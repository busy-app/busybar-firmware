#!/usr/bin/env python3

# Run-Length encoding and decoding
# Compatible with `lib/toolbox/rle_encode`
# For the format specification, see `lib/toolbox/rle_encode.c`

from random import randbytes, randint

MAX_BLOCKS_PER_BYTE = 127
RLE_BLOCK_THRESHOLD = 3

def compress(source: bytes, blk_size: int) -> bytes:
    src_i = 0
    src_len = len(source)
    assert src_len % blk_size == 0
    dest = bytearray()    

    while src_i < src_len:
        repeat_count = 0
        for i in range(src_i, src_len, blk_size):
            if source[i : i + blk_size] == source[src_i : src_i + blk_size]:
                repeat_count += 1
            else:
                break
        repeat_count = min(repeat_count, MAX_BLOCKS_PER_BYTE)

        if repeat_count == 0:
            break

        elif repeat_count < RLE_BLOCK_THRESHOLD:
            repeat_count = 0
            verbatim_count = 0
            for i in range(src_i, src_len, blk_size):
                if source[i : i + blk_size] == source[i + blk_size : i + (blk_size * 2)]:
                    repeat_count += 1
                    if repeat_count > RLE_BLOCK_THRESHOLD:
                        break
                else:
                    verbatim_count += 1 + repeat_count
                    repeat_count = 0
            verbatim_count += repeat_count
            verbatim_count = min(verbatim_count, MAX_BLOCKS_PER_BYTE)

            opcode = 0x80 | verbatim_count
            dest.append(opcode)
            dest.extend(source[src_i : src_i + (verbatim_count * blk_size)])
            src_i += verbatim_count * blk_size

        else:
            opcode = repeat_count
            dest.append(opcode)
            dest.extend(source[src_i : src_i + blk_size])
            src_i += repeat_count * blk_size

    return bytes(dest)

def decompress(source: bytes, blk_size: int) -> bytes:
    src_i = 0
    src_len = len(source)
    dest = bytearray()

    while src_i < src_len:
        opcode = source[src_i]
        count = int(opcode) & 0x7F
        src_i += 1

        if int(opcode) & 0x80:
            verbatim = source[src_i : src_i + count * blk_size]
            dest.extend(verbatim)
            src_i += count * blk_size
        else:
            repeated = source[src_i : src_i + blk_size]
            dest.extend(repeated * count)
            src_i += blk_size

    return bytes(dest)

def test_rle_single(blk_size):
    default_len = 512
    original_len = default_len - (default_len % blk_size)
    original = bytearray(randbytes(original_len))

    # insert repeating data so RLE has a chance to kick in
    for i in range(3):
        len_blks = randint(5, 10)
        len_bytes = len_blks * blk_size
        insertion_point = randint(0, original_len - len_bytes - 1)
        for j in range(len_bytes):
            original[insertion_point + j] = j % blk_size

    original = bytes(original)

    compressed = compress(original, blk_size)
    decompressed = decompress(compressed, blk_size)

    assert decompressed == original

def test_rle():
    for blk_size in range(1, 9):
        test_rle_single(blk_size)
    print("Tests passed")

if __name__ == "__main__":
    test_rle()
