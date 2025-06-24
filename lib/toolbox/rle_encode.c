#include "rle_encode.h"
#include <furi.h>

#define MAX_BLOCKS_PER_BYTE (127)

static inline size_t get_repeat_count(const uint8_t* data, size_t data_len, size_t blk_size) {
    if(data_len < blk_size) return 0;

    const size_t max_repeats = data_len / blk_size;
    size_t count = 1;
    for(size_t i = count; i < max_repeats && count < MAX_BLOCKS_PER_BYTE; i++) {
        if(memcmp(data, data + i * blk_size, blk_size) != 0) break;
        count++;
    }

    return count;
}

static inline size_t
    get_nonrepeat_count(const uint8_t* data, size_t data_len, size_t blk_size, size_t threshold) {
    if(data_len < blk_size) return 0;

    size_t nonrepeat_count = 0;
    size_t repeat_count = 0;
    size_t index = 0;

    while(true) {
        if(memcmp(data + index, data + index + blk_size, blk_size) == 0) {
            repeat_count += 1;
            if(repeat_count > threshold) break;
        } else {
            nonrepeat_count += 1 + repeat_count;
            repeat_count = 0;
            if(nonrepeat_count >= MAX_BLOCKS_PER_BYTE) {
                nonrepeat_count = MAX_BLOCKS_PER_BYTE;
                break;
            }
        }

        index += blk_size;
        if(index >= data_len) {
            nonrepeat_count += repeat_count;
            break;
        }
    }

    return nonrepeat_count;
}

bool rle_compress(
    const uint8_t* src,
    size_t src_len,
    uint8_t* dest,
    size_t dest_len,
    size_t blk_size,
    size_t* result_len) {
    furi_assert(src);
    furi_assert(dest);
    furi_assert(dest_len > src_len);

    size_t index = 0;
    size_t dest_index = 0;

    const uint8_t threshold = 3;
    bool error = false;
    while(index < src_len) {
        size_t remaining = src_len - index;
        size_t repeat_count = get_repeat_count(src + index, remaining, blk_size);

        if(repeat_count == 0) break;

        if(repeat_count < threshold) {
            size_t non_repeat_count =
                get_nonrepeat_count(src + index, remaining, blk_size, threshold);
            uint8_t ctrl_byte = (uint8_t)(non_repeat_count | 0x80);

            size_t byte_size = non_repeat_count * blk_size;
            if(dest_index + byte_size + 1 >= dest_len) {
                error = true;
                break;
            }

            dest[dest_index++] = ctrl_byte;
            memcpy(dest + dest_index, src + index, byte_size);
            dest_index += byte_size;
            index += byte_size;
        } else {
            uint8_t ctrl_byte = (uint8_t)repeat_count;

            if(dest_index + blk_size + 1 >= dest_len) {
                error = true;
                break;
            }

            dest[dest_index++] = ctrl_byte;
            memcpy(dest + dest_index, src + index, blk_size);
            dest_index += blk_size;
            index += repeat_count * blk_size;
        }
    }

    *result_len = error ? 0 : dest_index;
    return !error;
}
