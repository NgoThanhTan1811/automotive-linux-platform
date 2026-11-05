#pragma once

#include <string>

namespace automotive {

class UnixDomainSocket {
public:
    UnixDomainSocket() = default;
    ~UnixDomainSocket();

    UnixDomainSocket(const UnixDomainSocket&) = delete;
    UnixDomainSocket& operator=(const UnixDomainSocket&) = delete;
    UnixDomainSocket(UnixDomainSocket&& other) noexcept;
    UnixDomainSocket& operator=(UnixDomainSocket&& other) noexcept;

    static UnixDomainSocket listen_on(const std::string& path);
    static UnixDomainSocket connect_to(const std::string& path);

    UnixDomainSocket accept_client() const;
    bool send_line(const std::string& line) const;
    bool receive_line(std::string& line) const;
    bool valid() const;

private:
    explicit UnixDomainSocket(int file_descriptor);
    int file_descriptor_ = -1;
};

}  // namespace automotive
