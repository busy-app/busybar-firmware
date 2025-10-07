/*******************************************************************************
 * Size: 12 px
 * Bpp: 1
 * Opts: --font /home/portasynthinca3/Downloads/BSB_Ark_BusyAppFont_Regular.ttf --bpp 1 --size 12 --no-compress --symbols ▶▹◃∞ --range 32-127 --format lvgl -o lib/lvgl_addons/fonts/lv_font_ark_regular_12.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_FONT_ARK_REGULAR_12
#define LV_FONT_ARK_REGULAR_12 1
#endif

#if LV_FONT_ARK_REGULAR_12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfd,

    /* U+0022 "\"" */
    0xb4,

    /* U+0023 "#" */
    0x53, 0xf5, 0x14, 0x51, 0x4f, 0xd4,

    /* U+0024 "$" */
    0x10, 0x47, 0xa5, 0x91, 0xe1, 0x5, 0x95, 0xe1,
    0x0,

    /* U+0025 "%" */
    0x42, 0x94, 0x44, 0x8, 0x12, 0x10, 0x15, 0x42,

    /* U+0026 "&" */
    0x10, 0x48, 0x48, 0x73, 0x8b, 0x80, 0x80, 0x7b,

    /* U+0027 "'" */
    0xc0,

    /* U+0028 "(" */
    0x21, 0x29, 0x20, 0x48, 0x80,

    /* U+0029 ")" */
    0x81, 0x22, 0x48, 0x4a, 0x0,

    /* U+002A "*" */
    0x12, 0x57, 0xa5, 0x10, 0x40,

    /* U+002B "+" */
    0x10, 0x41, 0x3f, 0x10, 0x40,

    /* U+002C "," */
    0x58,

    /* U+002D "-" */
    0xfc,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x8, 0x40, 0x21, 0x21, 0x8, 0x4, 0x20,

    /* U+0030 "0" */
    0x7a, 0x18, 0xed, 0xb7, 0x18, 0x5e,

    /* U+0031 "1" */
    0x11, 0xcd, 0x4, 0x10, 0x41, 0x3f,

    /* U+0032 "2" */
    0x7a, 0x10, 0x46, 0x40, 0x8, 0x3f,

    /* U+0033 "3" */
    0x7a, 0x10, 0x46, 0x0, 0x18, 0x5e,

    /* U+0034 "4" */
    0x39, 0x24, 0xa2, 0x8a, 0x2f, 0xc2,

    /* U+0035 "5" */
    0xfe, 0xf, 0x81, 0x6, 0x18, 0x1e,

    /* U+0036 "6" */
    0x7a, 0x18, 0x3e, 0x82, 0x18, 0x5e,

    /* U+0037 "7" */
    0xdc, 0x10, 0x84, 0x10, 0x4, 0x10,

    /* U+0038 "8" */
    0x7a, 0x18, 0x5e, 0x2, 0x18, 0x5e,

    /* U+0039 "9" */
    0x7a, 0x18, 0x5f, 0x4, 0x18, 0x5e,

    /* U+003A ":" */
    0x84,

    /* U+003B ";" */
    0x40, 0x16,

    /* U+003C "<" */
    0x2a, 0x22,

    /* U+003D "=" */
    0xfc, 0xf, 0xc0,

    /* U+003E ">" */
    0x88, 0xa8,

    /* U+003F "?" */
    0x7a, 0x10, 0x42, 0x10, 0x0, 0x4,

    /* U+0040 "@" */
    0x7a, 0x18, 0xe5, 0x96, 0x59, 0xe0, 0x78,

    /* U+0041 "A" */
    0x10, 0x44, 0x92, 0x78, 0x8, 0x61,

    /* U+0042 "B" */
    0xfa, 0x18, 0x7e, 0x82, 0x18, 0x7e,

    /* U+0043 "C" */
    0x7a, 0x18, 0x20, 0x82, 0x8, 0x5e,

    /* U+0044 "D" */
    0xfa, 0x18, 0x61, 0x86, 0x18, 0x7e,

    /* U+0045 "E" */
    0xfe, 0x8, 0x3e, 0x82, 0x8, 0x3f,

    /* U+0046 "F" */
    0xfe, 0x8, 0x3e, 0x82, 0x8, 0x20,

    /* U+0047 "G" */
    0x7a, 0x18, 0x20, 0x8e, 0x18, 0x5f,

    /* U+0048 "H" */
    0x86, 0x18, 0x7f, 0x86, 0x18, 0x61,

    /* U+0049 "I" */
    0xf4, 0x44, 0x44, 0x4f,

    /* U+004A "J" */
    0x8, 0x42, 0x10, 0xc6, 0x2c,

    /* U+004B "K" */
    0x86, 0x29, 0x38, 0x92, 0x8, 0xa1,

    /* U+004C "L" */
    0x84, 0x21, 0x8, 0x42, 0x1f,

    /* U+004D "M" */
    0x81, 0xe0, 0xf0, 0x72, 0xf9, 0x7c, 0xe, 0x27,
    0x13,

    /* U+004E "N" */
    0x87, 0x1c, 0x65, 0x86, 0x38, 0xe1,

    /* U+004F "O" */
    0x7a, 0x18, 0x61, 0x86, 0x18, 0x5e,

    /* U+0050 "P" */
    0xfa, 0x18, 0x7e, 0x82, 0x8, 0x20,

    /* U+0051 "Q" */
    0xfe, 0x18, 0x61, 0xa6, 0x8, 0x99,

    /* U+0052 "R" */
    0xfa, 0x18, 0x7e, 0x8a, 0x8, 0x61,

    /* U+0053 "S" */
    0x7a, 0x18, 0x1e, 0x0, 0x18, 0x5e,

    /* U+0054 "T" */
    0xfc, 0x41, 0x4, 0x10, 0x41, 0x4,

    /* U+0055 "U" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0x5e,

    /* U+0056 "V" */
    0x86, 0x14, 0x92, 0x48, 0x1, 0x4,

    /* U+0057 "W" */
    0x89, 0x89, 0x89, 0x95, 0x95, 0x0, 0x42, 0x42,

    /* U+0058 "X" */
    0x86, 0x14, 0x84, 0x48, 0x8, 0x61,

    /* U+0059 "Y" */
    0x86, 0x14, 0x92, 0x0, 0x41, 0x4,

    /* U+005A "Z" */
    0xfc, 0x10, 0x84, 0x40, 0x8, 0x3f,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x93, 0x80,

    /* U+005C "\\" */
    0x84, 0x0, 0x84, 0x8, 0x42, 0x0, 0x42,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x27, 0x80,

    /* U+005E "^" */
    0x54,

    /* U+005F "_" */
    0xf8,

    /* U+0060 "`" */
    0x90,

    /* U+0061 "a" */
    0x60, 0xde, 0x18, 0xbc,

    /* U+0062 "b" */
    0x84, 0x3d, 0x18, 0xc6, 0x1e,

    /* U+0063 "c" */
    0x64, 0x61, 0x8, 0xb0,

    /* U+0064 "d" */
    0x8, 0x5f, 0x18, 0xc4, 0x2f,

    /* U+0065 "e" */
    0x64, 0x7f, 0x0, 0x30,

    /* U+0066 "f" */
    0x1a, 0x3e, 0x84, 0x21, 0x8,

    /* U+0067 "g" */
    0x7c, 0x63, 0x10, 0xbc, 0x6c,

    /* U+0068 "h" */
    0x84, 0x3d, 0x18, 0xc6, 0x31,

    /* U+0069 "i" */
    0x40, 0xc4, 0x44, 0x4f,

    /* U+006A "j" */
    0x10, 0xf1, 0x11, 0x11, 0x1c,

    /* U+006B "k" */
    0x84, 0x23, 0x2e, 0x42, 0x51,

    /* U+006C "l" */
    0xc9, 0x24, 0x93,

    /* U+006D "m" */
    0xea, 0x59, 0x65, 0x96, 0x50,

    /* U+006E "n" */
    0xf4, 0x63, 0x18, 0xc4,

    /* U+006F "o" */
    0x64, 0x63, 0x10, 0x30,

    /* U+0070 "p" */
    0xf4, 0x63, 0x18, 0x7a, 0x10,

    /* U+0071 "q" */
    0x7c, 0x63, 0x10, 0xbc, 0x21,

    /* U+0072 "r" */
    0x9f, 0x21, 0x8, 0x40,

    /* U+0073 "s" */
    0x7c, 0x18, 0x0, 0xf0,

    /* U+0074 "t" */
    0x42, 0x3e, 0x84, 0x21, 0x3,

    /* U+0075 "u" */
    0x8c, 0x63, 0x18, 0xbc,

    /* U+0076 "v" */
    0x86, 0x10, 0x12, 0x48, 0x40,

    /* U+0077 "w" */
    0x96, 0x59, 0x40, 0x49, 0x20,

    /* U+0078 "x" */
    0x85, 0x21, 0x0, 0x4a, 0x10,

    /* U+0079 "y" */
    0x86, 0x14, 0x92, 0x0, 0x41, 0x30,

    /* U+007A "z" */
    0xf8, 0x90, 0x8, 0x7c,

    /* U+007B "{" */
    0x30, 0x44, 0x48, 0x4, 0x44, 0x30,

    /* U+007C "|" */
    0xff, 0xe0,

    /* U+007D "}" */
    0xc0, 0x8, 0x42, 0x4, 0x4, 0x21, 0x30,

    /* U+007E "~" */
    0x42, 0x50, 0x80,

    /* U+25B6 "▶" */
    0xc0, 0x30, 0xf, 0x3, 0xf8, 0xff, 0xbf, 0xff,
    0xff, 0xfe, 0xfe, 0x3c, 0xc, 0x0,

    /* U+25B9 "▹" */
    0xc2, 0xe8, 0x6e, 0xbb, 0x0,

    /* U+25C3 "◃" */
    0xd, 0xd8, 0x5d, 0x74, 0x30
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 19, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 77, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 115, .box_w = 3, .box_h = 2, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 3, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 9, .adv_w = 115, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 18, .adv_w = 154, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 26, .adv_w = 134, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 34, .adv_w = 77, .box_w = 1, .box_h = 2, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 35, .adv_w = 115, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 40, .adv_w = 115, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 45, .adv_w = 115, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 50, .adv_w = 115, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 55, .adv_w = 77, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 56, .adv_w = 115, .box_w = 6, .box_h = 1, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 57, .adv_w = 77, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 58, .adv_w = 96, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 65, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 71, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 77, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 83, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 89, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 95, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 101, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 107, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 113, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 119, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 125, .adv_w = 38, .box_w = 1, .box_h = 6, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 126, .adv_w = 77, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 128, .adv_w = 77, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 130, .adv_w = 115, .box_w = 6, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 133, .adv_w = 77, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 135, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 141, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 148, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 160, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 166, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 172, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 178, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 184, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 190, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 77, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 200, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 205, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 216, .adv_w = 154, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 231, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 237, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 243, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 249, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 255, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 261, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 267, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 273, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 279, .adv_w = 154, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 287, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 293, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 299, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 305, .adv_w = 115, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 310, .adv_w = 96, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 317, .adv_w = 115, .box_w = 3, .box_h = 11, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 322, .adv_w = 77, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 323, .adv_w = 96, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 324, .adv_w = 96, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 325, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 329, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 334, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 338, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 343, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 347, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 352, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 357, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 362, .adv_w = 77, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 366, .adv_w = 77, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 371, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 376, .adv_w = 77, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 379, .adv_w = 115, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 384, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 388, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 392, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 397, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 402, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 406, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 410, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 415, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 419, .adv_w = 115, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 424, .adv_w = 115, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 429, .adv_w = 115, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 434, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 440, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 444, .adv_w = 134, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 450, .adv_w = 77, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 452, .adv_w = 134, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 459, .adv_w = 115, .box_w = 6, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 462, .adv_w = 192, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 476, .adv_w = 192, .box_w = 6, .box_h = 6, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 481, .adv_w = 192, .box_w = 6, .box_h = 6, .ofs_x = 3, .ofs_y = 1}
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
const lv_font_t lv_font_ark_regular_12 = {
#else
lv_font_t lv_font_ark_regular_12 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 12,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if LV_FONT_ARK_REGULAR_12*/

