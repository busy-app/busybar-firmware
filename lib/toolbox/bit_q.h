/**
 * @brief Bit Queue
 * Interprets packed binary data as a series of variable-length tokens with
 * size granularity of 1 bit
 */

#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const uint8_t* buffer;

    size_t size_bytes;
    size_t size_bits;

    size_t byte_pos;
    size_t bit_pos;
    size_t bit_pos_total;
} BitQ;

void bit_q_init(BitQ* bit_q, const uint8_t* buffer, size_t size_bits);

size_t bit_q_read(BitQ* bit_q, size_t width);

bool bit_q_end(BitQ* bit_q);

#ifdef __cplusplus
}
#endif
