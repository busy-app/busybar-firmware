#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __THUMBEL__ // Thumb Endian Little
#error "Platform not supported because of hardcoded endianness conversion functions"
#endif

// Host To Big-Endian 16
// we're little endian
static inline uint16_t __htobe16(uint16_t input) {
    uint16_t hi = input >> 8;
    uint16_t lo = input & 0xFF;
    return (lo << 8) | hi;
}

// Host To Big-Endian 32
// we're little endian
static inline uint32_t __htobe32(uint32_t input) {
    uint32_t b0 = input >> 24;
    uint32_t b1 = (input >> 16) & 0xFF;
    uint32_t b2 = (input >> 8) & 0xFF;
    uint32_t b3 = input & 0xFF;
    return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}

#ifndef htons
#define htons(x) __htobe16(x)
#define htonl(x) __htobe32(x)
#define ntohs(x) __htobe16(x)
#define ntohl(x) __htobe32(x)
#endif

#ifdef __cplusplus
}
#endif
