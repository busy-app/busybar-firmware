#include "../unit_tests.h"
#include "test_files.h"

#include <anim_file/anim_file.h>
#include <storage/storage.h>
#include <toolbox/crc32_calc.h>

typedef enum {
    /*
     * 1x1 RGB pixel, 4 frames.
     * The only pixel has equal R, G and B values in a single frame.
     * 
     * Frame 0   Frame 1   Frame 2   Frame 3
     *    0         1         2         3
     */
    AnimTestFile_Color_Ramp_1x1x4,
    /*
     * 2x2 RGB pixels, 10 frames.
     * R, G and B values within an individual pixel are equal.
     * 
     * Frame 0   Frame 1   Frame 2   Frame 3
     *   0 1       1 2       1 2       2 3
     *   2 3       3 4       3 4       4 5
     * 
     * Frame 4   Frame 5   Frame 6   Frame 7
     *   2 3       2 3       3 4       3 4
     *   4 5       4 5       5 6       5 6
     * 
     * Frame 8   Frame 9
     *   3 4       3 4
     *   5 6       5 6
     */
    AnimTestFile_Color_ComplexDuration_2x2x10,
    /*
     * 2x2 4bpp grayscale pixels, 4 frames.
     * 
     * Frame 0   Frame 1   Frame 2   Frame 3
     *   0 1       1 2       2 3       3 4
     *   2 3       3 4       4 5       5 6
     * (all px vals shifted left by 4 bits)
     */
    AnimTestFile_Gray_Ramp_2x2x4,
    /*
     * 10x10 4bpp grayscale pixels, 10 frames.
     * R, G and B values within an individual pixel are equal.
     * All pixels within a frame have the same color.
     * 
     * Frame 0   Frame 1         Frame 8   Frame 9
     *    0         1     . . .     8         9
     */
    AnimTestFile_Gray_RLE_Ramp_10x10x10,
} AnimTestFile;

const char* anim_test_prepare_file(AnimTestFile type) {
    static const char* const path = UNIT_TESTS_PATH("anim.anim");
    Storage* storage = furi_record_open(RECORD_STORAGE);

    File* file = storage_file_alloc(storage);
    storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS);

    typedef struct {
        const uint8_t* data;
        size_t size;
    } TestArray;

#define TEST_FILE_DEF(name) [AnimTestFile_##name] = {test_file_##name, COUNT_OF(test_file_##name)}
    static const TestArray arrays[] = {
        TEST_FILE_DEF(Color_Ramp_1x1x4),
        TEST_FILE_DEF(Color_ComplexDuration_2x2x10),
        TEST_FILE_DEF(Gray_Ramp_2x2x4),
        TEST_FILE_DEF(Gray_RLE_Ramp_10x10x10),
    };
    TestArray array = arrays[type];
#undef TEST_FILE_DEF

    storage_file_write(file, array.data, array.size);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return path;
}

/**
 * Basic non-repeating playback
 */
MU_TEST(anim_test_play_whole_oneshot) {
    const char* path = anim_test_prepare_file(AnimTestFile_Color_Ramp_1x1x4);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    AnimFile* anim = anim_file_alloc(storage, path);
    mu_assert_not_null(anim);

    AnimFileInfo info = anim_file_info(anim);
    mu_assert_int_eq(1, info.width);
    mu_assert_int_eq(1, info.height);
    mu_assert_int_eq(10, info.fps);

    uint8_t buffer[3];

    for(size_t i = 0; i < 10; i++) {
        for(size_t j = 0; j < 10; j++) {
            AnimFileFrameFlag flags = anim_file_frame(anim, buffer);

            size_t expected = MIN(j, 3u);
            mu_assert_int_eq(expected, buffer[0]);
            mu_assert_int_eq(expected, buffer[1]);
            mu_assert_int_eq(expected, buffer[2]);

            AnimFileFrameFlag exp_flags =
                (j >= 3) ? (AnimFileFrameFlagLast | AnimFileFrameFlagFinished) : 0;
            mu_assert_int_eq(exp_flags, flags);
        }

        if(i % 2 == 0) {
            mu_assert_int_eq(
                1,
                anim_file_set_section_indexed(
                    anim, AnimFilePlayFlagNone, ANIM_FILE_WHOLE_SECTION_INDEX));
        } else {
            mu_assert_int_eq(
                1,
                anim_file_set_section_named(
                    anim, AnimFilePlayFlagNone, ANIM_FILE_WHOLE_SECTION_NAME));
        }
    }

    anim_file_free(anim);
    furi_record_close(RECORD_STORAGE);
}

/**
 * Basic repeating playback
 */
MU_TEST(anim_test_play_whole_loop) {
    const char* path = anim_test_prepare_file(AnimTestFile_Color_Ramp_1x1x4);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    AnimFile* anim = anim_file_alloc(storage, path);
    mu_assert_not_null(anim);
    mu_assert_int_eq(
        1,
        anim_file_set_section_indexed(anim, AnimFilePlayFlagLoop, ANIM_FILE_WHOLE_SECTION_INDEX));

    uint8_t buffer[3];

    for(size_t j = 0; j < 100; j++) {
        AnimFileFrameFlag flags = anim_file_frame(anim, buffer);

        size_t expected = j % 4;
        mu_assert_int_eq(expected, buffer[0]);
        mu_assert_int_eq(expected, buffer[1]);
        mu_assert_int_eq(expected, buffer[2]);

        AnimFileFrameFlag exp_flags =
            ((j % 4) == 3) ? (AnimFileFrameFlagLast | AnimFileFrameFlagLooping) : 0;
        mu_assert_int_eq(exp_flags, flags);
    }

    anim_file_free(anim);
    furi_record_close(RECORD_STORAGE);
}

/**
 * Basic non-repeating playback with a complexly encoded animation
 */
MU_TEST(anim_test_play_complex_oneshot) {
    const char* path = anim_test_prepare_file(AnimTestFile_Color_ComplexDuration_2x2x10);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    AnimFile* anim = anim_file_alloc(storage, path);
    mu_assert_not_null(anim);

    uint8_t buffer[12];

    for(size_t i = 0; i < 10; i++) {
        if(i % 3 == 0) {
            mu_assert_int_eq(1, anim_file_set_section_manual(anim, AnimFilePlayFlagNone, 2, 6));
        } else if(i % 3 == 1) {
            mu_assert_int_eq(1, anim_file_set_section_named(anim, AnimFilePlayFlagNone, "2-6"));
        } else if(i % 3 == 2) {
            mu_assert_int_eq(1, anim_file_set_section_indexed(anim, AnimFilePlayFlagNone, 1));
        }
#define FRAMES_IN_RANGE ((6u - 2u) + 1u) // 5

        for(size_t j = 0; j < 10; j++) {
            AnimFileFrameFlag flags = anim_file_frame(anim, buffer);

            for(size_t k = 0; k < 4; k++) {
                const size_t bases[FRAMES_IN_RANGE] = {1, 2, 2, 2, 3};
                size_t expected = bases[MIN(j, FRAMES_IN_RANGE - 1)] + k;
                mu_assert_int_eq(expected, buffer[(k * 3) + 0]);
                mu_assert_int_eq(expected, buffer[(k * 3) + 1]);
                mu_assert_int_eq(expected, buffer[(k * 3) + 2]);
            }

            AnimFileFrameFlag exp_flags = (j >= (FRAMES_IN_RANGE - 1)) ?
                                              (AnimFileFrameFlagLast | AnimFileFrameFlagFinished) :
                                              0;
            mu_assert_int_eq(exp_flags, flags);
        }

#undef FRAMES_IN_RANGE
    }

    anim_file_free(anim);
    furi_record_close(RECORD_STORAGE);
}

/**
 * Whole animation, then complexly encoded intermediate range
 */
MU_TEST(anim_test_play_complex_pend) {
    const char* path = anim_test_prepare_file(AnimTestFile_Color_ComplexDuration_2x2x10);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    AnimFile* anim = anim_file_alloc(storage, path);
    mu_assert_not_null(anim);

    const size_t bases[] = {
        0, 1, 1, 2, 2, 2, 3, 3, 3, 3, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3,
    };
    uint8_t buffer[12];

    for(size_t i = 0; i < COUNT_OF(bases); i++) {
        AnimFileFrameFlag flags = anim_file_frame(anim, buffer);

        for(size_t k = 0; k < 4; k++) {
            size_t expected = bases[i] + k;
            mu_assert_int_eq(expected, buffer[(k * 3) + 0]);
            mu_assert_int_eq(expected, buffer[(k * 3) + 1]);
            mu_assert_int_eq(expected, buffer[(k * 3) + 2]);
        }

        if(i == 1) {
            mu_assert_int_eq(
                1, anim_file_set_section_named(anim, AnimFilePlayFlagFinishCurrentSection, "2-6"));
        }

        AnimFileFrameFlag exp_flags;
        if(i == 9) {
            exp_flags = AnimFileFrameFlagLast | AnimFileFrameFlagSwitchToRequested;
        } else if(i >= 14) {
            exp_flags = AnimFileFrameFlagLast | AnimFileFrameFlagFinished;
        } else {
            exp_flags = 0;
        }
        mu_assert_int_eq(exp_flags, flags);
    }

    anim_file_free(anim);
    furi_record_close(RECORD_STORAGE);
}

/**
 * Whole non-repeating grayscale animation
 */
MU_TEST(anim_test_grayscale) {
    const char* path = anim_test_prepare_file(AnimTestFile_Gray_Ramp_2x2x4);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    AnimFile* anim = anim_file_alloc(storage, path);
    mu_assert_not_null(anim);

    const size_t bases[] = {0x00, 0x10, 0x20, 0x30};
    uint8_t buffer[12];

    for(size_t i = 0; i < COUNT_OF(bases); i++) {
        AnimFileFrameFlag flags = anim_file_frame(anim, buffer);

        for(size_t k = 0; k < 4; k++) {
            size_t expected = bases[i] + (k << 4);
            mu_assert_int_eq(expected, buffer[(k * 3) + 0]);
            mu_assert_int_eq(expected, buffer[(k * 3) + 1]);
            mu_assert_int_eq(expected, buffer[(k * 3) + 2]);
        }

        AnimFileFrameFlag exp_flags =
            (i >= 3) ? (AnimFileFrameFlagLast | AnimFileFrameFlagFinished) : 0;
        mu_assert_int_eq(exp_flags, flags);
    }

    anim_file_free(anim);
    furi_record_close(RECORD_STORAGE);
}

/**
 * Run-length encoded frame
 */
MU_TEST(anim_test_rle) {
    const char* path = anim_test_prepare_file(AnimTestFile_Gray_RLE_Ramp_10x10x10);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    AnimFile* anim = anim_file_alloc(storage, path);
    mu_assert_not_null(anim);

    uint8_t buffer[300];

    for(size_t i = 0; i < 10; i++) {
        AnimFileFrameFlag flags = anim_file_frame(anim, buffer);

        for(size_t k = 0; k < 100; k++) {
            size_t expected = i << 4;
            mu_assert_int_eq(expected, buffer[(k * 3) + 0]);
            mu_assert_int_eq(expected, buffer[(k * 3) + 1]);
            mu_assert_int_eq(expected, buffer[(k * 3) + 2]);
        }

        AnimFileFrameFlag exp_flags =
            (i == 9) ? (AnimFileFrameFlagLast | AnimFileFrameFlagFinished) : 0;
        mu_assert_int_eq(exp_flags, flags);
    }

    anim_file_free(anim);
    furi_record_close(RECORD_STORAGE);
}

/**
 * Performance testing
 */
MU_TEST(anim_test_perf) {
    const size_t iterations = 10;
    const char* path =
        "/ext/test.anim"; // TODO: /ext/apps_assets/power_on/back_power_on_160x80.anim

    Storage* storage = furi_record_open(RECORD_STORAGE);
    AnimFile* anim = anim_file_alloc(storage, path);
    mu_assert_not_null(anim);

    AnimFileInfo info = anim_file_info(anim);
    uint8_t* buffer = malloc(info.width * info.height * 3);

    size_t start = furi_get_tick();
    for(size_t i = 0; i < iterations; i++) {
        while(1) {
            AnimFileFrameFlag flags = anim_file_frame(anim, buffer);
            if(flags & AnimFileFrameFlagFinished) break;
        }
    }
    size_t end = furi_get_tick();
    size_t delta = end - start;

    size_t frames = iterations * info.frames;
    printf("%zu fps (%zu frames in %zu ms)\r\n", 1000 * frames / delta, frames, delta);

    free(buffer);

    anim_file_free(anim);
    furi_record_close(RECORD_STORAGE);
}

MU_TEST_SUITE(anim_basic_test_suite) {
    MU_RUN_TEST(anim_test_play_whole_oneshot);
    MU_RUN_TEST(anim_test_play_whole_loop);
    MU_RUN_TEST(anim_test_play_complex_oneshot);
    MU_RUN_TEST(anim_test_play_complex_pend);
    MU_RUN_TEST(anim_test_grayscale);
    MU_RUN_TEST(anim_test_rle);
    MU_RUN_TEST(anim_test_perf);
}

int run_minunit_anim_test(void) {
    MU_RUN_SUITE(anim_basic_test_suite);
    return MU_EXIT_CODE;
}
