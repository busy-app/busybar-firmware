/*******************************************************************************
 * Size: 12 px
 * Bpp: 1
 * Opts: --bpp 1 --size 12 --no-compress --font ark-pixel-12px-proportional-zh_cn.ttf --symbols ▶▹◃∞ --range 32-127 --format lvgl -o lv_font_ark_pixel_12.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_FONT_ARK_PIXEL_12
#define LV_FONT_ARK_PIXEL_12 1
#endif

#if LV_FONT_ARK_PIXEL_12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfe, 0x80,

    /* U+0022 "\"" */
    0xb6, 0x80,

    /* U+0023 "#" */
    0x49, 0x2f, 0xd2, 0x49, 0x2f, 0xd2, 0x48,

    /* U+0024 "$" */
    0x23, 0xab, 0x4a, 0x38, 0xa5, 0xab, 0x88,

    /* U+0025 "%" */
    0x45, 0x49, 0x20, 0x41, 0x4, 0x9, 0x25, 0x44,

    /* U+0026 "&" */
    0x31, 0x24, 0x8c, 0x66, 0x58, 0xa2, 0x74,

    /* U+0027 "'" */
    0xe0,

    /* U+0028 "(" */
    0x29, 0x49, 0x24, 0x48, 0x80,

    /* U+0029 ")" */
    0x89, 0x12, 0x49, 0x4a, 0x0,

    /* U+002A "*" */
    0x25, 0x5d, 0x52, 0x0,

    /* U+002B "+" */
    0x21, 0x3e, 0x42, 0x0,

    /* U+002C "," */
    0x58,

    /* U+002D "-" */
    0xf8,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x8, 0x44, 0x22, 0x10, 0x88, 0x44, 0x20,

    /* U+0030 "0" */
    0x74, 0x63, 0x3a, 0xe6, 0x31, 0x70,

    /* U+0031 "1" */
    0x23, 0x28, 0x42, 0x10, 0x84, 0xf8,

    /* U+0032 "2" */
    0x74, 0x42, 0x11, 0x11, 0x10, 0xf8,

    /* U+0033 "3" */
    0x74, 0x42, 0x13, 0x4, 0x31, 0x70,

    /* U+0034 "4" */
    0x32, 0x94, 0xa9, 0x4b, 0xe2, 0x10,

    /* U+0035 "5" */
    0xfc, 0x21, 0xf, 0x4, 0x31, 0x70,

    /* U+0036 "6" */
    0x74, 0x61, 0xf, 0x46, 0x31, 0x70,

    /* U+0037 "7" */
    0xf8, 0x42, 0x21, 0x10, 0x84, 0x20,

    /* U+0038 "8" */
    0x74, 0x63, 0x17, 0x46, 0x31, 0x70,

    /* U+0039 "9" */
    0x74, 0x63, 0x17, 0x84, 0x31, 0x70,

    /* U+003A ":" */
    0x84,

    /* U+003B ";" */
    0x40, 0x16,

    /* U+003C "<" */
    0x12, 0x48, 0x42, 0x10,

    /* U+003D "=" */
    0xf8, 0x3e,

    /* U+003E ">" */
    0x84, 0x21, 0x24, 0x80,

    /* U+003F "?" */
    0x74, 0x42, 0x11, 0x10, 0x80, 0x20,

    /* U+0040 "@" */
    0x38, 0x8a, 0x6d, 0x5a, 0xb5, 0x6a, 0xdb, 0x40,
    0x78,

    /* U+0041 "A" */
    0x10, 0x20, 0xa1, 0x44, 0x4f, 0x91, 0x41, 0x82,

    /* U+0042 "B" */
    0xfa, 0x18, 0x61, 0xfa, 0x18, 0x61, 0xf8,

    /* U+0043 "C" */
    0x39, 0x18, 0x20, 0x82, 0x8, 0x11, 0x38,

    /* U+0044 "D" */
    0xf2, 0x28, 0x61, 0x86, 0x18, 0x62, 0xf0,

    /* U+0045 "E" */
    0xfe, 0x8, 0x20, 0xfa, 0x8, 0x20, 0xfc,

    /* U+0046 "F" */
    0xfe, 0x8, 0x20, 0xfa, 0x8, 0x20, 0x80,

    /* U+0047 "G" */
    0x39, 0x18, 0x20, 0x82, 0x38, 0x51, 0x3c,

    /* U+0048 "H" */
    0x86, 0x18, 0x61, 0xfe, 0x18, 0x61, 0x84,

    /* U+0049 "I" */
    0xe9, 0x24, 0x92, 0xe0,

    /* U+004A "J" */
    0x8, 0x42, 0x10, 0x86, 0x31, 0x70,

    /* U+004B "K" */
    0x86, 0x29, 0x28, 0xc2, 0x89, 0x22, 0x84,

    /* U+004C "L" */
    0x84, 0x21, 0x8, 0x42, 0x10, 0xf8,

    /* U+004D "M" */
    0x83, 0x7, 0x1e, 0x3a, 0xb5, 0x64, 0xc9, 0x92,

    /* U+004E "N" */
    0x87, 0x1a, 0x69, 0x96, 0x58, 0xe3, 0x84,

    /* U+004F "O" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x60, 0xa2, 0x38,

    /* U+0050 "P" */
    0xfa, 0x18, 0x61, 0xfa, 0x8, 0x20, 0x80,

    /* U+0051 "Q" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x62, 0xa2, 0x3a,

    /* U+0052 "R" */
    0xfa, 0x18, 0x61, 0xfa, 0x48, 0xa2, 0x84,

    /* U+0053 "S" */
    0x7a, 0x18, 0x10, 0x30, 0x20, 0x61, 0x78,

    /* U+0054 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x2, 0x4, 0x8, 0x10,

    /* U+0055 "U" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0x61, 0x78,

    /* U+0056 "V" */
    0x83, 0x5, 0x12, 0x24, 0x45, 0xa, 0x8, 0x10,

    /* U+0057 "W" */
    0x88, 0xc4, 0x62, 0x2a, 0xa5, 0x52, 0xa8, 0x88,
    0x44, 0x22, 0x0,

    /* U+0058 "X" */
    0x82, 0x89, 0x11, 0x41, 0x5, 0x11, 0x22, 0x82,

    /* U+0059 "Y" */
    0x82, 0x89, 0x11, 0x42, 0x82, 0x4, 0x8, 0x10,

    /* U+005A "Z" */
    0xfc, 0x10, 0x84, 0x20, 0x84, 0x20, 0xfc,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x93, 0x80,

    /* U+005C "\\" */
    0x84, 0x10, 0x82, 0x10, 0x82, 0x10, 0x42,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x27, 0x80,

    /* U+005E "^" */
    0x22, 0xa2,

    /* U+005F "_" */
    0xf8,

    /* U+0060 "`" */
    0x90,

    /* U+0061 "a" */
    0x70, 0x5f, 0x18, 0xbc,

    /* U+0062 "b" */
    0x84, 0x21, 0xe8, 0xc6, 0x31, 0xf0,

    /* U+0063 "c" */
    0x74, 0x61, 0x8, 0xb8,

    /* U+0064 "d" */
    0x8, 0x42, 0xf8, 0xc6, 0x31, 0x78,

    /* U+0065 "e" */
    0x74, 0x7f, 0x8, 0xb8,

    /* U+0066 "f" */
    0x34, 0x4f, 0x44, 0x44, 0x40,

    /* U+0067 "g" */
    0x7c, 0x63, 0x18, 0xbc, 0x2e,

    /* U+0068 "h" */
    0x84, 0x21, 0x6c, 0xc6, 0x31, 0x88,

    /* U+0069 "i" */
    0x40, 0x64, 0x92, 0xe0,

    /* U+006A "j" */
    0x20, 0x72, 0x49, 0x27, 0x0,

    /* U+006B "k" */
    0x84, 0x21, 0x1b, 0x62, 0x92, 0x88,

    /* U+006C "l" */
    0xc9, 0x24, 0x92, 0x60,

    /* U+006D "m" */
    0xed, 0x26, 0x4c, 0x99, 0x32, 0x40,

    /* U+006E "n" */
    0xb6, 0x63, 0x18, 0xc4,

    /* U+006F "o" */
    0x74, 0x63, 0x18, 0xb8,

    /* U+0070 "p" */
    0xf4, 0x63, 0x18, 0xfa, 0x10,

    /* U+0071 "q" */
    0x7c, 0x63, 0x18, 0xbc, 0x21,

    /* U+0072 "r" */
    0xbc, 0x88, 0x88,

    /* U+0073 "s" */
    0x74, 0x58, 0x28, 0xb8,

    /* U+0074 "t" */
    0x44, 0x4f, 0x44, 0x44, 0x30,

    /* U+0075 "u" */
    0x8c, 0x63, 0x19, 0xb4,

    /* U+0076 "v" */
    0x8c, 0x54, 0xa2, 0x10,

    /* U+0077 "w" */
    0x93, 0x26, 0xad, 0x54, 0x48, 0x80,

    /* U+0078 "x" */
    0x8a, 0x88, 0x45, 0x44,

    /* U+0079 "y" */
    0x8c, 0x62, 0xa5, 0x10, 0x98,

    /* U+007A "z" */
    0xf8, 0x88, 0x88, 0x7c,

    /* U+007B "{" */
    0x34, 0x44, 0x48, 0x44, 0x44, 0x30,

    /* U+007C "|" */
    0xff, 0xe0,

    /* U+007D "}" */
    0xc2, 0x22, 0x21, 0x22, 0x22, 0xc0,

    /* U+007E "~" */
    0x45, 0x44,

    /* U+25B6 "▶" */
    0xc0, 0x78, 0x3f, 0x1f, 0xef, 0xff, 0xfb, 0xf1,
    0xe0, 0xc0, 0x0,

    /* U+25B9 "▹" */
    0xc5, 0xa3, 0x6c, 0x0,

    /* U+25C3 "◃" */
    0x1b, 0x62, 0xd1, 0x80
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 96, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 64, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 96, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 5, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 12, .adv_w = 96, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 19, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 27, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 34, .adv_w = 64, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 35, .adv_w = 96, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 40, .adv_w = 96, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 45, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 49, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 53, .adv_w = 64, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 54, .adv_w = 96, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 55, .adv_w = 64, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 56, .adv_w = 96, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 63, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 69, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 75, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 81, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 87, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 93, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 99, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 105, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 111, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 117, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 123, .adv_w = 64, .box_w = 1, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 124, .adv_w = 64, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 126, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 130, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 132, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 136, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 142, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 151, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 159, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 166, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 173, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 180, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 194, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 201, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 212, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 218, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 231, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 239, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 246, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 254, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 261, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 269, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 276, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 283, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 291, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 298, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 306, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 317, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 325, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 333, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 340, .adv_w = 96, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 345, .adv_w = 96, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 352, .adv_w = 96, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 357, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 359, .adv_w = 96, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 360, .adv_w = 80, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 361, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 365, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 371, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 375, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 381, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 385, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 390, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 395, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 401, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 405, .adv_w = 64, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 410, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 416, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 420, .adv_w = 128, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 426, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 430, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 434, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 439, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 444, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 447, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 451, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 456, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 460, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 464, .adv_w = 128, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 470, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 474, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 479, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 483, .adv_w = 112, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 489, .adv_w = 64, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 491, .adv_w = 112, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 497, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 499, .adv_w = 192, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 510, .adv_w = 192, .box_w = 5, .box_h = 5, .ofs_x = 3, .ofs_y = 2},
    {.bitmap_index = 514, .adv_w = 192, .box_w = 5, .box_h = 5, .ofs_x = 3, .ofs_y = 2}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {
    0x0, 0x3, 0xd
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 9654, .range_length = 14, .glyph_id_start = 96,
        .unicode_list = unicode_list_1, .glyph_id_ofs_list = NULL, .list_length = 3, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t lv_font_ark_pixel_12 = {
#else
lv_font_t lv_font_ark_pixel_12 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 12,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = 0,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if LV_FONT_ARK_PIXEL_12*/

