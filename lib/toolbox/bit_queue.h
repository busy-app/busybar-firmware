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

void bit_queue_init(BitQ* bit_queue, const uint8_t* buffer, size_t size_bits);

size_t bit_queue_read(BitQ* bit_queue, size_t width);

bool bit_queue_end(BitQ* bit_queue);

#ifdef __cplusplus
}
#endif
