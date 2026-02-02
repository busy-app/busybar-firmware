#pragma once
#include <stdint.h>
#include <stdbool.h>

static inline bool furi_hal_bits_is_set(uint32_t value, uint32_t mask) {
    return (value & mask) == mask;
}

static inline bool furi_hal_bits_is_not_set(uint32_t value, uint32_t mask) {
    return (value & mask) == 0;
}

static inline void furi_hal_bits_set(volatile uint32_t* value, uint32_t mask) {
    *value |= mask;
}

static inline void furi_hal_bits_clear(volatile uint32_t* value, uint32_t mask) {
    *value &= ~mask;
}
