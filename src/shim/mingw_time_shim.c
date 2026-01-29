/*
 * Kotlin/Native's mingwX64 toolchain uses lld and a MinGW-w64 sysroot that may
 * not provide the same time64 symbols that FFmpeg (built via vcpkg/MSYS2) ends
 * up referencing (e.g. clock_gettime64/nanosleep64).
 *
 * Provide lightweight shims so static linking succeeds. These are only enabled
 * for MinGW builds.
 */

#if defined(__MINGW32__) || defined(__MINGW64__)

#include <errno.h>
#include <stdint.h>
#include <time.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

static void qpc_now(uint64_t* out_ns) {
    static LARGE_INTEGER freq = {0};
    static int freq_init = 0;
    LARGE_INTEGER counter;

    if (!freq_init) {
        QueryPerformanceFrequency(&freq);
        freq_init = 1;
    }

    QueryPerformanceCounter(&counter);
    *out_ns = (uint64_t)((counter.QuadPart * 1000000000ULL) / (uint64_t)freq.QuadPart);
}

int clock_gettime64(int clock_id, struct timespec* tp) {
    if (!tp) {
        errno = EINVAL;
        return -1;
    }

    if (clock_id == CLOCK_MONOTONIC) {
        uint64_t ns = 0;
        qpc_now(&ns);
        tp->tv_sec = (time_t)(ns / 1000000000ULL);
        tp->tv_nsec = (long)(ns % 1000000000ULL);
        return 0;
    }

    /* Best-effort wall clock. */
    FILETIME ft;
    ULARGE_INTEGER ui;
    GetSystemTimeAsFileTime(&ft);
    ui.LowPart = ft.dwLowDateTime;
    ui.HighPart = ft.dwHighDateTime;

    /* Windows FILETIME is 100ns since 1601-01-01. Convert to Unix epoch. */
    const uint64_t EPOCH_DIFF_100NS = 116444736000000000ULL;
    uint64_t t100 = (ui.QuadPart > EPOCH_DIFF_100NS) ? (ui.QuadPart - EPOCH_DIFF_100NS) : 0;
    tp->tv_sec = (time_t)(t100 / 10000000ULL);
    tp->tv_nsec = (long)((t100 % 10000000ULL) * 100ULL);
    return 0;
}

int nanosleep64(const struct timespec* req, struct timespec* rem) {
    (void)rem;
    if (!req) {
        errno = EINVAL;
        return -1;
    }

    /* Sleep() takes milliseconds; round up to avoid undersleeping. */
    uint64_t ms = 0;
    if (req->tv_sec > 0) {
        ms += (uint64_t)req->tv_sec * 1000ULL;
    }
    if (req->tv_nsec > 0) {
        ms += (uint64_t)((req->tv_nsec + 999999L) / 1000000L);
    }

    if (ms > 0xFFFFFFFFULL) {
        /* Sleep() max is DWORD; clamp. */
        ms = 0xFFFFFFFFULL;
    }

    Sleep((DWORD)ms);
    return 0;
}

#endif /* __MINGW32__ || __MINGW64__ */

