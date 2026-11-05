#include "ipc/unix_domain_socket.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <system_error>
#include <cstddef>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace automotive {
namespace {

sockaddr_un make_address(const std::string& path) {
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    const bool is_abstract = !path.empty() && path.front() == '@';
    const std::size_t name_offset = is_abstract ? 1 : 0;
    const std::size_t name_size = path.size() - name_offset;
    if (name_size >= sizeof(address.sun_path)) {
        throw std::invalid_argument("Unix socket path is too long");
    }
    if (is_abstract) {
        address.sun_path[0] = '\0';
        std::memcpy(address.sun_path + 1, path.data() + 1, name_size);
    } else {
        std::strncpy(address.sun_path, path.c_str(), sizeof(address.sun_path) - 1);
    }
    return address;
}

socklen_t address_length(const std::string& path) {
    if (!path.empty() && path.front() == '@') {
        return static_cast<socklen_t>(offsetof(sockaddr_un, sun_path) +
                                      1 + path.size() - 1);
    }
    return sizeof(sockaddr_un);
}

}  // namespace

UnixDomainSocket::UnixDomainSocket(int file_descriptor)
    : file_descriptor_(file_descriptor) {}

UnixDomainSocket::~UnixDomainSocket() {
    if (file_descriptor_ != -1) {
        close(file_descriptor_);
    }
}

UnixDomainSocket::UnixDomainSocket(UnixDomainSocket&& other) noexcept
    : file_descriptor_(other.file_descriptor_) {
    other.file_descriptor_ = -1;
}

UnixDomainSocket& UnixDomainSocket::operator=(UnixDomainSocket&& other) noexcept {
    if (this != &other) {
        if (file_descriptor_ != -1) {
            close(file_descriptor_);
        }
        file_descriptor_ = other.file_descriptor_;
        other.file_descriptor_ = -1;
    }
    return *this;
}

UnixDomainSocket UnixDomainSocket::listen_on(const std::string& path) {
    const int file_descriptor = socket(AF_UNIX, SOCK_STREAM, 0);
    if (file_descriptor == -1) {
        throw std::system_error(errno, std::generic_category(), "socket");
    }

    const sockaddr_un address = make_address(path);
    if (path.empty() || path.front() != '@') {
        unlink(path.c_str());
    }
    if (bind(file_descriptor, reinterpret_cast<const sockaddr*>(&address),
             address_length(path)) == -1 ||
        listen(file_descriptor, 1) == -1) {
        const int saved_errno = errno;
        close(file_descriptor);
        throw std::system_error(saved_errno, std::generic_category(),
                                "bind/listen");
    }
    return UnixDomainSocket(file_descriptor);
}

UnixDomainSocket UnixDomainSocket::connect_to(const std::string& path) {
    const int file_descriptor = socket(AF_UNIX, SOCK_STREAM, 0);
    if (file_descriptor == -1) {
        throw std::system_error(errno, std::generic_category(), "socket");
    }

    const sockaddr_un address = make_address(path);
    if (connect(file_descriptor, reinterpret_cast<const sockaddr*>(&address),
                address_length(path)) == -1) {
        const int saved_errno = errno;
        close(file_descriptor);
        throw std::system_error(saved_errno, std::generic_category(),
                                "connect");
    }
    return UnixDomainSocket(file_descriptor);
}

UnixDomainSocket UnixDomainSocket::accept_client() const {
    const int client_file_descriptor = accept(file_descriptor_, nullptr, nullptr);
    if (client_file_descriptor == -1) {
        throw std::system_error(errno, std::generic_category(), "accept");
    }
    return UnixDomainSocket(client_file_descriptor);
}

bool UnixDomainSocket::send_line(const std::string& line) const {
    const std::string framed_line = line + '\n';
    std::size_t bytes_sent = 0;
    while (bytes_sent < framed_line.size()) {
        const ssize_t result = send(file_descriptor_, framed_line.data() + bytes_sent,
                                    framed_line.size() - bytes_sent, MSG_NOSIGNAL);
        if (result == -1 && errno == EINTR) {
            continue;
        }
        if (result <= 0) {
            return false;
        }
        bytes_sent += static_cast<std::size_t>(result);
    }
    return true;
}

bool UnixDomainSocket::receive_line(std::string& line) const {
    line.clear();
    char byte = '\0';
    while (true) {
        const ssize_t result = recv(file_descriptor_, &byte, 1, 0);
        if (result == 0) {
            return !line.empty();
        }
        if (result == -1 && errno == EINTR) {
            continue;
        }
        if (result == -1) {
            return false;
        }
        if (byte == '\n') {
            return true;
        }
        line.push_back(byte);
    }
}

bool UnixDomainSocket::valid() const {
    return file_descriptor_ != -1;
}

}  // namespace automotive
