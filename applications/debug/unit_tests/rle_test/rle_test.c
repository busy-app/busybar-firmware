#include "../unit_tests.h"
#include <toolbox/rle_encode.h>

static void rle_test(size_t blk_size) {
    const size_t default_sz = 512;
    uint8_t original[default_sz - (default_sz % blk_size)];

    for(size_t i = 0; i < sizeof(original); i++) {
        original[i] = rand() % UINT8_MAX;
    }

    // insert repeating data so RLE has a chance to activate
    for(size_t i = 0; i < 3; i++) {
        size_t length_blks = 5 + (rand() % 10);
        size_t length_bytes = length_blks * blk_size;
        size_t insertion_point = rand() % (sizeof(original) - length_bytes);
        for(size_t j = 0; j < length_bytes; j++) {
            original[insertion_point + j] = j % blk_size;
        }
    }

    uint8_t compressed[sizeof(original) + 32];
    size_t compressed_sz = 0;
    mu_assert_int_eq(
        true,
        rle_compress(
            original, sizeof(original), compressed, sizeof(compressed), blk_size, &compressed_sz));

    uint8_t decompressed[sizeof(original)];
    size_t decompressed_sz = 0;
    mu_assert_int_eq(
        true,
        rle_decompress(
            compressed,
            compressed_sz,
            decompressed,
            sizeof(decompressed),
            blk_size,
            &decompressed_sz));

    mu_assert_int_eq(decompressed_sz, sizeof(original));
    mu_assert_mem_eq(original, decompressed, decompressed_sz);
}

MU_TEST(rle_test_random) {
    for(size_t blk_size = 1; blk_size <= 8; blk_size++) {
        rle_test(blk_size);
    }
}

MU_TEST_SUITE(rle_test_suite) {
    MU_RUN_TEST(rle_test_random);
}

int run_minunit_rle_test(void) {
    MU_RUN_SUITE(rle_test_suite);
    return MU_EXIT_CODE;
}
