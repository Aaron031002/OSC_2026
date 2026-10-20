#ifndef MINI_OS_ENDIAN_H
#define MINI_OS_ENDIAN_H

#include <stdint.h>

static inline uint32_t bswap32(uint32_t x) {
    return __builtin_bswap32(x);
}

static inline uint64_t bswap64(uint64_t x) {
    return __builtin_bswap64(x);
}

#endif