#include "bit_q.h"

void bit_q_init(BitQ* bit_q, const uint8_t* buffer, size_t size_bits) {
    furi_check(bit_q);
    furi_check(buffer);

    memset(bit_q, 0, sizeof(*bit_q));

    bit_q->buffer = buffer;
    bit_q->size_bits = size_bits;
    bit_q->size_bytes = ROUND_UP_TO(size_bits, 8);
}

size_t bit_q_read(BitQ* bit_q, size_t width) {
    const size_t word_width = sizeof(size_t) * 8;
    furi_check(bit_q);
    furi_check(width <= word_width);

    size_t piece = 0;

    while(width) {
        if(bit_q->bit_pos_total >= bit_q->size_bits) break;

        size_t left_in_current_byte = 8 - bit_q->bit_pos;
        if(!left_in_current_byte) {
            bit_q->byte_pos++;
            bit_q->bit_pos = 0;
            left_in_current_byte = 8;
        }

        size_t piece_size = MIN(width, left_in_current_byte);
        piece <<= piece_size;

        size_t original_byte = bit_q->buffer[bit_q->byte_pos];
        original_byte >>= 8 - bit_q->bit_pos - piece_size;
        original_byte &= (1 << piece_size) - 1;
        piece |= original_byte;

        bit_q->bit_pos += piece_size;
        bit_q->bit_pos_total += piece_size;
        width -= piece_size;
    }

    return piece;
}

bool bit_q_end(BitQ* bit_q) {
    furi_check(bit_q);
    return bit_q->bit_pos_total >= bit_q->size_bits;
}
