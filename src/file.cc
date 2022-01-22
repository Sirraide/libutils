#include "../include/file.h"

struct PipePair {
    int _pair[2] = {-1, -1};
    PipePair() { pipe(_pair); }
    ~PipePair() {
        if (_pair[0] != -1) close(_pair[0]);
        if (_pair[1] != -1) close(_pair[1]);
    }
    LIBUTILS_NON_COPYABLE_NON_MOVABLE(PipePair);
};

SynchronousPipe::SynchronousPipe(const std::string& command, _err_handler_t handler) {
    PipePair p_in, p_out, p_err;
    pid_t    pid = vfork();
    if (pid < 0) return;
    if (pid == 0) {
        dup2(p_in._pair[0], STDIN_FILENO);
        dup2(p_out._pair[1], STDOUT_FILENO);
        dup2(p_err._pair[1], STDERR_FILENO);
        p_in.~PipePair();
        p_out.~PipePair();
        p_err.~PipePair();
        execlp("/bin/sh", "sh", "-c", command.c_str(), nullptr);
        exit(127);
    }
    in.fd         = p_in._pair[1];
    p_in._pair[1] = -1;

    out.fd         = p_out._pair[0];
    p_out._pair[0] = -1;

    err.fd         = p_err._pair[0];
    p_err._pair[0] = -1;

    in.err_handler  = handler;
    out.err_handler = handler;
    err.err_handler = handler;

    int wstatus{};
    do
        if (waitpid(pid, &wstatus, 0) < 0) {
            handler(std::string{"waitpid() failed: "} + std::strerror(errno));
            return;
        }
    while (!WIFEXITED(wstatus));
    status = WEXITSTATUS(wstatus);
    valid  = true;
}
