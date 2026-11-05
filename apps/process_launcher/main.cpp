#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace {

pid_t start_process(const std::string& executable_path) {
    const pid_t child_pid = fork();

    if (child_pid == -1) {
        std::cerr << "fork failed: " << std::strerror(errno) << '\n';
        return -1;
    }

    if (child_pid == 0) {
        execl(executable_path.c_str(), executable_path.c_str(),
              static_cast<char*>(nullptr));

        // The child must not continue as the launcher if exec fails.
        std::cerr << "exec failed for " << executable_path << ": "
                  << std::strerror(errno) << '\n';
        _exit(127);
    }

    std::cout << "launcher started " << executable_path << " with PID "
              << child_pid << '\n';
    return child_pid;
}

int wait_for_process(pid_t child_pid) {
    int status = 0;
    if (waitpid(child_pid, &status, 0) == -1) {
        std::cerr << "waitpid failed: " << std::strerror(errno) << '\n';
        return 1;
    }

    if (WIFEXITED(status)) {
        const int exit_code = WEXITSTATUS(status);
        std::cout << "child PID " << child_pid << " exited with code "
                  << exit_code << '\n';
        return exit_code;
    }

    if (WIFSIGNALED(status)) {
        std::cerr << "child PID " << child_pid << " terminated by signal "
                  << WTERMSIG(status) << '\n';
    }
    return 1;
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "usage: " << argv[0]
                  << " <sensor_service> <vehicle_middleware>\n";
        return 2;
    }

    const std::vector<pid_t> child_pids{
        start_process(argv[1]),
        start_process(argv[2]),
    };

    int result = 0;
    for (const pid_t child_pid : child_pids) {
        if (child_pid == -1 || wait_for_process(child_pid) != 0) {
            result = 1;
        }
    }
    return result;
}
