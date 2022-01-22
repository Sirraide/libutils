#ifndef UTILS_FILE_H
#define UTILS_FILE_H
#include "./coroutine.h"
#include "./utils.h"

#include <cstring>
#include <fcntl.h>
#include <functional>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>

enum struct FileMode : char8_t {
    R  = 1,
    W  = 2,
    RW = 3,
};
template <FileMode m>
concept Writable = requires { (char8_t(m) & char8_t(FileMode::W)) != 0; };

template <FileMode m>
concept Readable = requires { (char8_t(m) & char8_t(FileMode::R)) != 0; };

using FD = int;

template <FileMode mode = FileMode::RW>
struct File {
    FD                          fd       = -1;
    static constexpr inline U64 _bufsize = 8192;
    _err_handler_t              err_handler;

    void Close() {
        /// Try to close whether open or not to get the error from errno
        if (fd == -1) err_handler("Close: invalid file descriptor");
        if (close(fd) < 0) err_handler(std::string{"Close: "} + strerror(errno));
    }

    [[nodiscard]] std::string Drain() const requires Readable<mode> {
        std::string out;
        void*       buf = malloc(_bufsize);
        U64         n_read;
        do {
            n_read = read(fd, buf, _bufsize);
            if (n_read < 0) goto err;
            out.append((char*) buf, n_read);
        } while (n_read == _bufsize);
        free(buf);
        return out;
    err:
        free(buf);
        err_handler(std::strerror(errno));
    }

    [[nodiscard]] co_generator<std::string> Lines() const requires Readable<mode> {
        char*       buf = (char*) malloc(4096);
        U64         n_read;
        std::string out;
        do {
            n_read = read(fd, buf, _bufsize);
            if (n_read < 0) goto err;

            char *pos = buf, *start;
            for (;;) {
                start = pos;
                pos   = (char*) memchr(pos, '\n', n_read);
                if (!pos) {
                    out.append(start, n_read);
                    break;
                }
                out.append(start, U64(pos - start));
                co_yield out;
                out.clear();
                n_read -= U64(pos - start) + 1;
                pos++;
            }
        } while (n_read == _bufsize);
        free(buf);
        if (!out.empty()) co_yield out;
        co_return;
    err:
        free(buf);
        err_handler(std::strerror(errno));
        co_return;
    }

    [[nodiscard]] std::string Read(U64 n = UINT64_MAX) const requires Readable<mode> {
        (void) n;
        ConstexprNotImplemented("File::Read()");
        return "";
    }

    void Write(const std::string& str, U64 n = UINT64_MAX) const requires Writable<mode> {
        U64 sz = std::min(str.size(), n);
        write(fd, str.c_str(), sz);
    }

    explicit File(_err_handler_t _err_handler = _libutils_terminate)
        : err_handler(std::move(_err_handler)) {}

    explicit File(FD _fd, _err_handler_t _err_handler = _libutils_terminate)
        : fd(_fd), err_handler(std::move(_err_handler)) {}

    explicit File(const std::string& path, _err_handler_t _err_handler = _libutils_terminate)
        : err_handler(std::move(_err_handler)) {
        if constexpr (mode == FileMode::R) fd = open(path.c_str(), O_RDONLY);
        else if constexpr (mode == FileMode::W) fd = open(path.c_str(), O_WRONLY);
        else if constexpr (mode == FileMode::RW) fd = open(path.c_str(), O_RDWR);
        else ConstexprNotImplemented("File::File(const std::string&, _err_handler_t)");
        if (fd < 0) {
            err_handler(std::string{"open(): "} + std::strerror(errno));
            return;
        }
    }

    ~File() {
        if (fd > 2) close(fd);
    }

    LIBUTILS_NON_COPYABLE_NON_MOVABLE(File);
};

using OFile = File<FileMode::W>;
using IFile = File<FileMode::R>;

struct SynchronousPipe {
    OFile in;
    IFile out;
    IFile err;
    bool  valid = false;
    int   status;

    explicit SynchronousPipe(const std::string& command, _err_handler_t = _libutils_terminate);
    LIBUTILS_NON_COPYABLE_NON_MOVABLE(SynchronousPipe);
};

#endif // UTILS_FILE_H
