#include "../unit_tests.h"
#include <xpm/xpm.h>

static void* xpm_test_decode_full(const char* src, XpmPixelFormat format, size_t* size) {
    Xpm* xpm = xpm_alloc(src);
    void* pixels = NULL;

    do {
        if(!xpm_decode_header(xpm)) {
            break;
        }

        if(!xpm_decode_colors(xpm)) {
            break;
        }

        pixels = xpm_decode_pixels(xpm, format, size);
    } while(false);

    xpm_free(xpm);
    return pixels;
}

MU_TEST(xpm_header_valid) {
    Xpm* xpm = xpm_alloc("! XPM2\n8 8 2 1\n. c #FF0000\n# c #00FF00\n.#.#.#.#\n#.#.#.#.\n");

    mu_check(xpm_decode_header(xpm));

    XpmHeaderData header = xpm_get_header_data(xpm);
    mu_assert_int_eq(8, header.width);
    mu_assert_int_eq(8, header.height);
    mu_assert_int_eq(2, header.colors_count);
    mu_assert_int_eq(1, header.chars_per_pixel);

    xpm_free(xpm);
}

MU_TEST(xpm_header_too_few_numbers) {
    Xpm* xpm = xpm_alloc("! XPM2\n8 8 2\n.c #FF0000\n#c #00FF00\n");
    mu_check(!xpm_decode_header(xpm));
    xpm_free(xpm);
}

MU_TEST(xpm_header_invalid_value) {
    Xpm* xpm = xpm_alloc("! XPM2\n8 8 0 1\n.c #FF0000\n");
    mu_check(!xpm_decode_header(xpm));
    xpm_free(xpm);
}

MU_TEST(xpm_header_garbage_after_numbers) {
    Xpm* xpm = xpm_alloc("! XPM2\n8 8 2 1 garbage\n.c #FF0000\n");
    mu_check(!xpm_decode_header(xpm));
    xpm_free(xpm);
}

MU_TEST(xpm_header_exceeds_max) {
    Xpm* xpm = xpm_alloc("! XPM2\n99999 1 1 1\n. c #FF0000\n.\n");
    mu_check(!xpm_decode_header(xpm));
    xpm_free(xpm);
}

MU_TEST(xpm_signature_missing) {
    Xpm* xpm = xpm_alloc("2 1 2 1\n. c #FF0000\n# c #00FF00\n.#\n");
    mu_check(!xpm_decode_header(xpm));
    xpm_free(xpm);
}

MU_TEST(xpm_colors_single_hex) {
    Xpm* xpm = xpm_alloc("! XPM2\n1 1 1 1\n. c #FF0000\n.\n");
    mu_check(xpm_decode_header(xpm));
    mu_check(xpm_decode_colors(xpm));
    xpm_free(xpm);
}

MU_TEST(xpm_colors_precedence) {
    Xpm* xpm = xpm_alloc("! XPM2\n1 1 1 1\n. c #FF0000 g #00FF00\n.\n");
    mu_check(xpm_decode_header(xpm));
    mu_check(xpm_decode_colors(xpm));

    size_t size = 0;
    uint8_t* pixels = xpm_decode_pixels(xpm, XpmPixelFormatBGRA8888, &size);
    mu_assert_not_null(pixels);
    mu_assert_int_eq(4, size);
    mu_assert_int_eq(0x00, pixels[0]);
    mu_assert_int_eq(0x00, pixels[1]);
    mu_assert_int_eq(0xFF, pixels[2]);
    mu_assert_int_eq(0xFF, pixels[3]);
    free(pixels);

    pixels = xpm_decode_pixels(xpm, XpmPixelFormatLA88, &size);
    mu_assert_not_null(pixels);
    mu_assert_int_eq(2, size);
    mu_assert_int_eq(0xFF, pixels[1]);
    free(pixels);

    xpm_free(xpm);
}

MU_TEST(xpm_colors_all_types_and_symbolic) {
    Xpm* xpm = xpm_alloc("! XPM2\n1 1 1 1\n. c #FF0000 g #00FF00 g4 #0000FF m black s dot\n.\n");
    mu_check(xpm_decode_header(xpm));
    mu_check(xpm_decode_colors(xpm));
    xpm_free(xpm);
}

MU_TEST(xpm_colors_presets) {
    Xpm* xpm = xpm_alloc("! XPM2\n2 1 2 1\n. c none\n# c red\n.#\n");
    mu_check(xpm_decode_header(xpm));
    mu_check(xpm_decode_colors(xpm));

    size_t size = 0;
    uint8_t* pixels = xpm_decode_pixels(xpm, XpmPixelFormatBGRA8888, &size);
    mu_assert_not_null(pixels);
    mu_assert_int_eq(8, size);
    mu_assert_int_eq(0x00, pixels[0]);
    mu_assert_int_eq(0x00, pixels[1]);
    mu_assert_int_eq(0x00, pixels[2]);
    mu_assert_int_eq(0x00, pixels[3]);
    mu_assert_int_eq(0x00, pixels[4]);
    mu_assert_int_eq(0x00, pixels[5]);
    mu_assert_int_eq(0xFF, pixels[6]);
    mu_assert_int_eq(0xFF, pixels[7]);
    free(pixels);

    xpm_free(xpm);
}

MU_TEST(xpm_colors_hex_variants) {
    Xpm* xpm = xpm_alloc("! XPM2\n4 1 4 1\n"
                         "a c #F00\n"
                         "b c #FF0000\n"
                         "c c #FFF000000\n"
                         "d c #FFFF00000000\n"
                         "abcd\n");
    mu_check(xpm_decode_header(xpm));
    mu_check(xpm_decode_colors(xpm));

    size_t size = 0;
    uint8_t* pixels = xpm_decode_pixels(xpm, XpmPixelFormatBGRA8888, &size);
    mu_assert_not_null(pixels);
    mu_assert_int_eq(16, size);
    mu_assert_mem_eq(pixels + 0, pixels + 4, 4);
    mu_assert_mem_eq(pixels + 0, pixels + 8, 4);
    mu_assert_mem_eq(pixels + 0, pixels + 12, 4);
    mu_assert_int_eq(0xFF, pixels[2]);
    mu_assert_int_eq(0xFF, pixels[3]);
    free(pixels);

    xpm_free(xpm);
}

MU_TEST(xpm_colors_invalid_value) {
    Xpm* xpm = xpm_alloc("! XPM2\n1 1 1 1\n. c xyz\n.\n");
    mu_check(xpm_decode_header(xpm));
    mu_check(!xpm_decode_colors(xpm));
    xpm_free(xpm);
}

MU_TEST(xpm_colors_unknown_type) {
    Xpm* xpm = xpm_alloc("! XPM2\n1 1 1 1\n. x #FF0000\n.\n");
    mu_check(xpm_decode_header(xpm));
    mu_check(!xpm_decode_colors(xpm));
    xpm_free(xpm);
}

MU_TEST(xpm_colors_missing_value) {
    Xpm* xpm = xpm_alloc("! XPM2\n1 1 1 1\n. c\n.\n");
    mu_check(xpm_decode_header(xpm));
    mu_check(!xpm_decode_colors(xpm));
    xpm_free(xpm);
}

MU_TEST(xpm_colors_duplicate_type_last_wins) {
    Xpm* xpm = xpm_alloc("! XPM2\n1 1 1 1\n. c #FF0000 c #00FF00\n.\n");
    mu_check(xpm_decode_header(xpm));
    mu_check(xpm_decode_colors(xpm));

    size_t size = 0;
    uint8_t* pixels = xpm_decode_pixels(xpm, XpmPixelFormatBGRA8888, &size);
    mu_assert_not_null(pixels);
    mu_assert_int_eq(4, size);
    mu_assert_int_eq(0x00, pixels[0]);
    mu_assert_int_eq(0xFF, pixels[1]);
    mu_assert_int_eq(0x00, pixels[2]);
    mu_assert_int_eq(0xFF, pixels[3]);
    free(pixels);

    xpm_free(xpm);
}

MU_TEST(xpm_pixels_valid) {
    size_t size = 0;
    uint8_t* pixels = xpm_test_decode_full(
        "! XPM2\n2 1 2 1\n"
        ". c #FF0000\n"
        "# c #00FF00\n"
        ".#\n",
        XpmPixelFormatBGRA8888,
        &size);

    mu_assert_not_null(pixels);
    mu_assert_int_eq(8, size);
    mu_assert_int_eq(0x00, pixels[0]);
    mu_assert_int_eq(0x00, pixels[1]);
    mu_assert_int_eq(0xFF, pixels[2]);
    mu_assert_int_eq(0xFF, pixels[3]);
    mu_assert_int_eq(0x00, pixels[4]);
    mu_assert_int_eq(0xFF, pixels[5]);
    mu_assert_int_eq(0x00, pixels[6]);
    mu_assert_int_eq(0xFF, pixels[7]);
    free(pixels);
}

MU_TEST(xpm_pixels_unknown_key) {
    void* pixels = xpm_test_decode_full(
        "! XPM2\n1 1 1 1\n"
        ". c #FF0000\n"
        "#\n",
        XpmPixelFormatBGRA8888,
        NULL);
    mu_assert_null(pixels);
}

MU_TEST(xpm_pixels_row_too_short) {
    void* pixels = xpm_test_decode_full(
        "! XPM2\n3 1 1 1\n"
        ". c #FF0000\n"
        "..\n",
        XpmPixelFormatBGRA8888,
        NULL);
    mu_assert_null(pixels);
}

MU_TEST(xpm_pixels_overlong_row) {
    void* pixels = xpm_test_decode_full(
        "! XPM2\n2 1 2 1\n"
        ". c #FF0000\n"
        "# c #00FF00\n"
        ".#\x20",
        XpmPixelFormatBGRA8888,
        NULL);
    mu_assert_null(pixels);
}

MU_TEST(xpm_pixels_trailing_garbage) {
    void* pixels = xpm_test_decode_full(
        "! XPM2\n1 1 1 1\n"
        ". c #FF0000\n"
        ".\n"
        "garbage\n",
        XpmPixelFormatBGRA8888,
        NULL);
    mu_assert_null(pixels);
}

MU_TEST(xpm_pixels_multi_format) {
    Xpm* xpm = xpm_alloc("! XPM2\n1 1 1 1\n"
                         ". c #FFFFFF\n"
                         ".\n");
    mu_check(xpm_decode_header(xpm));
    mu_check(xpm_decode_colors(xpm));

    size_t size = 0;
    uint8_t* bgra_pixels = xpm_decode_pixels(xpm, XpmPixelFormatBGRA8888, &size);
    mu_assert_not_null(bgra_pixels);
    mu_assert_int_eq(4, size);
    mu_assert_int_eq(0xFF, bgra_pixels[0]);
    mu_assert_int_eq(0xFF, bgra_pixels[1]);
    mu_assert_int_eq(0xFF, bgra_pixels[2]);
    mu_assert_int_eq(0xFF, bgra_pixels[3]);
    free(bgra_pixels);

    uint8_t* la_pixels = xpm_decode_pixels(xpm, XpmPixelFormatLA88, &size);
    mu_assert_not_null(la_pixels);
    mu_assert_int_eq(2, size);
    mu_assert_int_eq(0xFF, la_pixels[0]);
    mu_assert_int_eq(0xFF, la_pixels[1]);
    free(la_pixels);

    xpm_free(xpm);
}

MU_TEST(xpm_pixels_transparency) {
    size_t size = 0;
    uint8_t* pixels = xpm_test_decode_full(
        "! XPM2\n1 1 1 1\n"
        ". c none\n"
        ".\n",
        XpmPixelFormatBGRA8888,
        &size);

    mu_assert_not_null(pixels);
    mu_assert_int_eq(4, size);
    mu_assert_int_eq(0x00, pixels[3]);
    free(pixels);
}

MU_TEST(xpm_integration_minimal) {
    size_t size = 0;
    uint8_t* pixels = xpm_test_decode_full(
        "! XPM2\n1 1 1 1\n"
        ". c #FF0000\n"
        ".\n",
        XpmPixelFormatBGRA8888,
        &size);

    mu_assert_not_null(pixels);
    mu_assert_int_eq(4, size);
    mu_assert_int_eq(0x00, pixels[0]);
    mu_assert_int_eq(0x00, pixels[1]);
    mu_assert_int_eq(0xFF, pixels[2]);
    mu_assert_int_eq(0xFF, pixels[3]);
    free(pixels);
}

MU_TEST(xpm_integration_multicolor) {
    size_t size = 0;
    uint8_t* pixels = xpm_test_decode_full(
        "! XPM2\n3 3 2 1\n"
        ". c #FF0000 g #808080\n"
        "# c #00FF00 m black\n"
        ".#.\n"
        "#.#\n"
        ".#.\n",
        XpmPixelFormatBGRA8888,
        &size);

    mu_assert_not_null(pixels);
    mu_assert_int_eq(36, size);

    mu_assert_int_eq(0xFF, pixels[0 * 4 + 2]);
    mu_assert_int_eq(0xFF, pixels[1 * 4 + 1]);
    mu_assert_int_eq(0xFF, pixels[4 * 4 + 2]);

    free(pixels);
}

MU_TEST_SUITE(xpm_test_suite) {
    MU_RUN_TEST(xpm_header_valid);
    MU_RUN_TEST(xpm_header_too_few_numbers);
    MU_RUN_TEST(xpm_header_invalid_value);
    MU_RUN_TEST(xpm_header_garbage_after_numbers);
    MU_RUN_TEST(xpm_header_exceeds_max);
    MU_RUN_TEST(xpm_signature_missing);

    MU_RUN_TEST(xpm_colors_single_hex);
    MU_RUN_TEST(xpm_colors_precedence);
    MU_RUN_TEST(xpm_colors_all_types_and_symbolic);
    MU_RUN_TEST(xpm_colors_presets);
    MU_RUN_TEST(xpm_colors_hex_variants);
    MU_RUN_TEST(xpm_colors_invalid_value);
    MU_RUN_TEST(xpm_colors_unknown_type);
    MU_RUN_TEST(xpm_colors_missing_value);
    MU_RUN_TEST(xpm_colors_duplicate_type_last_wins);

    MU_RUN_TEST(xpm_pixels_valid);
    MU_RUN_TEST(xpm_pixels_unknown_key);
    MU_RUN_TEST(xpm_pixels_row_too_short);
    MU_RUN_TEST(xpm_pixels_overlong_row);
    MU_RUN_TEST(xpm_pixels_trailing_garbage);
    MU_RUN_TEST(xpm_pixels_multi_format);
    MU_RUN_TEST(xpm_pixels_transparency);

    MU_RUN_TEST(xpm_integration_minimal);
    MU_RUN_TEST(xpm_integration_multicolor);
}

int run_minunit_xpm_test(void) {
    MU_RUN_SUITE(xpm_test_suite);
    return MU_EXIT_CODE;
}
