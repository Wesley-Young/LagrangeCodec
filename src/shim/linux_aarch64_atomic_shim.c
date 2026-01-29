/*
 * Kotlin/Native's linuxArm64 link step may use a sysroot/runtime that doesn't
 * provide the AArch64 LSE/outline-atomics helper symbols that some FFmpeg builds
 * reference (e.g. __aarch64_ldadd4_* / __aarch64_swp4_*).
 *
 * Provide weak shims so static linking succeeds. These implementations use
 * GCC-style __atomic builtins and are compiled into libLagrangeCodec.a.
 *
 * Note: These symbols are part of compiler-rt on many toolchains; marking them
 * weak avoids conflicts when the "real" implementation is available.
 */

#if defined(__linux__) && defined(__aarch64__)

#include <stdint.h>

__attribute__((weak))
uint32_t __aarch64_ldadd4_relax(uint32_t val, uint32_t* ptr) {
    return __atomic_fetch_add(ptr, val, __ATOMIC_RELAXED);
}

__attribute__((weak))
uint32_t __aarch64_ldadd4_rel(uint32_t val, uint32_t* ptr) {
    return __atomic_fetch_add(ptr, val, __ATOMIC_RELEASE);
}

__attribute__((weak))
uint32_t __aarch64_ldadd4_acq_rel(uint32_t val, uint32_t* ptr) {
    return __atomic_fetch_add(ptr, val, __ATOMIC_ACQ_REL);
}

__attribute__((weak))
uint32_t __aarch64_swp4_relax(uint32_t val, uint32_t* ptr) {
    return __atomic_exchange_n(ptr, val, __ATOMIC_RELAXED);
}

#endif /* __linux__ && __aarch64__ */
