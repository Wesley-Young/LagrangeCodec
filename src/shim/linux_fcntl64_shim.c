//
// Provides missing symbols when linking with Linux that lacks fcntl64.
//

#if defined(__linux__)

#include <fcntl.h>
#include <stdarg.h>

__attribute__((weak))
int fcntl64(int fd, int cmd, ...) {
    switch (cmd) {
        case F_GETFD:
        case F_GETFL:
            return fcntl(fd, cmd);
        default: {
            va_list ap;
            va_start(ap, cmd);
            /* On Linux (LP64), both pointers and ints are safely representable in long. */
            long arg = va_arg(ap, long);
            va_end(ap);
            return fcntl(fd, cmd, arg);
        }
    }
}

#endif /* __linux__ */

