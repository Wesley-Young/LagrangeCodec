//
// Provides missing symbols when linking with Linux on AArch64 that lacks atomic operations.
//

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
