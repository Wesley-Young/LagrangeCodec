/*
 * Some toolchains/sysroots used during Kotlin/Native's linuxX64 link step can
 * end up without the legacy fcntl64 symbol even though dependent archives
 * (e.g. FFmpeg) reference it. Provide a weak shim so static linking succeeds.
 *
 * This is intentionally minimal: FFmpeg uses fcntl64 for F_GETFL/F_SETFL in
 * ff_socket_nonblock(), so forwarding to fcntl() is sufficient.
 */

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

