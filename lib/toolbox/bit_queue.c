#include "bit_queue.h"

void bit_queue_init(BitQ* bit_queue, const uint8_t* buffer, size_t size_bits) {
    furi_check(bit_queue);
    furi_check(buffer);

    memset(bit_queue, 0, sizeof(*bit_queue));

    bit_queue->buffer = buffer;
    bit_queue->size_bits = size_bits;
    bit_queue->size_bytes = ROUND_UP_TO(size_bits, 8);
}

size_t bit_queue_read(BitQ* bit_queue, size_t width) {
    const size_t word_width = sizeof(size_t) * 8;
    furi_check(bit_queue);
    furi_check(width <= word_width);

    size_t piece = 0;

    while(width) {
        if(bit_queue->bit_pos_total >= bit_queue->size_bits) break;

        size_t left_in_current_byte = 8 - bit_queue->bit_pos;
        if(!left_in_current_byte) {
            bit_queue->byte_pos++;
            bit_queue->bit_pos = 0;
            left_in_current_byte = 8;
        }

        size_t piece_size = MIN(width, left_in_current_byte);
        piece <<= piece_size;

        size_t original_byte = bit_queue->buffer[bit_queue->byte_pos];
        original_byte >>= 8 - bit_queue->bit_pos - piece_size;
        original_byte &= (1 << piece_size) - 1;
        piece |= original_byte;

        bit_queue->bit_pos += piece_size;
        bit_queue->bit_pos_total += piece_size;
        width -= piece_size;
    }

    return piece;
}

bool bit_queue_end(BitQ* bit_queue) {
    furi_check(bit_queue);
    return bit_queue->bit_pos_total >= bit_queue->size_bits;
}
