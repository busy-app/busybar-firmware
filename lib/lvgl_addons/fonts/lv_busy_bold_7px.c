/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --bpp 1 --size 16 --no-compress --font .\busy_bold_7px.ttf --symbols ▶▹◃∞ --range 32-127 --format lvgl -o lv_busy_bold_7px.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_BUSY_BOLD_7PX
#define LV_BUSY_BOLD_7PX 1
#endif

#if LV_BUSY_BOLD_7PX

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xcc,

    /* U+0022 "\"" */
    0xb4,

    /* U+0023 "#" */
    0x6d, 0xfd, 0xb3, 0x66, 0xdf, 0xdb, 0x0,

    /* U+0024 "$" */
    0x18, 0x7e, 0xdb, 0xd8, 0x7e, 0x1b, 0xdb, 0x7e,
    0x18,

    /* U+0025 "%" */
    0xc3, 0xc6, 0xc, 0x18, 0x30, 0x63, 0xc3,

    /* U+0026 "&" */
    0x71, 0xb3, 0x6b, 0xad, 0x9b, 0xbc, 0x80,

    /* U+0027 "'" */
    0xc0,

    /* U+0028 "(" */
    0x7b, 0x6d, 0x98,

    /* U+0029 ")" */
    0xcd, 0xb6, 0xf0,

    /* U+002A "*" */
    0x5d, 0x50,

    /* U+002B "+" */
    0x30, 0xcf, 0xcc, 0x30,

    /* U+002C "," */
    0x58,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x18, 0xcc, 0x66, 0x33, 0x0,

    /* U+0030 "0" */
    0x7b, 0x3d, 0xfb, 0xcf, 0x37, 0x80,

    /* U+0031 "1" */
    0x31, 0xcf, 0xc, 0x30, 0xcf, 0xc0,

    /* U+0032 "2" */
    0x7b, 0x30, 0xce, 0x63, 0xf, 0xc0,

    /* U+0033 "3" */
    0x7b, 0x30, 0xce, 0xf, 0x37, 0x80,

    /* U+0034 "4" */
    0x39, 0xe5, 0xb6, 0xdb, 0xf1, 0x80,

    /* U+0035 "5" */
    0xff, 0xc, 0x3e, 0xc, 0x3f, 0x80,

    /* U+0036 "6" */
    0x7b, 0x3c, 0x3e, 0xcf, 0x37, 0x80,

    /* U+0037 "7" */
    0xfc, 0x31, 0x8c, 0x31, 0x86, 0x0,

    /* U+0038 "8" */
    0x7b, 0x3c, 0xde, 0xcf, 0x37, 0x80,

    /* U+0039 "9" */
    0x7b, 0x3c, 0xdf, 0xf, 0x37, 0x80,

    /* U+003A ":" */
    0x88,

    /* U+003B ";" */
    0x40, 0x58,

    /* U+003C "<" */
    0x36, 0xc6, 0x30,

    /* U+003D "=" */
    0xfc, 0xf, 0xc0,

    /* U+003E ">" */
    0xc6, 0x36, 0xc0,

    /* U+003F "?" */
    0x7b, 0x30, 0xc6, 0x30, 0x3, 0x0,

    /* U+0040 "@" */
    0x7f, 0x60, 0xf7, 0x7a, 0xbd, 0xfe, 0x1, 0xf8,

    /* U+0041 "A" */
    0x30, 0xc7, 0x96, 0x7b, 0x3c, 0xc0,

    /* U+0042 "B" */
    0xfb, 0x3c, 0xfe, 0xcf, 0x3f, 0x80,

    /* U+0043 "C" */
    0x7b, 0x3c, 0x30, 0xc3, 0x37, 0x80,

    /* U+0044 "D" */
    0xf3, 0x6c, 0xf3, 0xcf, 0x6f, 0x0,

    /* U+0045 "E" */
    0xfe, 0x31, 0xec, 0x63, 0xe0,

    /* U+0046 "F" */
    0xfe, 0x31, 0xec, 0x63, 0x0,

    /* U+0047 "G" */
    0x7b, 0x3c, 0x37, 0xcf, 0x37, 0x40,

    /* U+0048 "H" */
    0xcf, 0x3c, 0xff, 0xcf, 0x3c, 0xc0,

    /* U+0049 "I" */
    0xf6, 0x66, 0x66, 0xf0,

    /* U+004A "J" */
    0x18, 0xc6, 0x3d, 0xed, 0xc0,

    /* U+004B "K" */
    0xcf, 0x6f, 0x38, 0xf3, 0x6c, 0xc0,

    /* U+004C "L" */
    0xc6, 0x31, 0x8c, 0x63, 0xe0,

    /* U+004D "M" */
    0xc1, 0xf1, 0xfd, 0xff, 0xfd, 0xde, 0x4f, 0x6,

    /* U+004E "N" */
    0xc7, 0xcf, 0xdf, 0xfd, 0xf9, 0xf1, 0x80,

    /* U+004F "O" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x37, 0x80,

    /* U+0050 "P" */
    0xfb, 0x3c, 0xfe, 0xc3, 0xc, 0x0,

    /* U+0051 "Q" */
    0x7b, 0x3c, 0xf3, 0xd7, 0x27, 0x40,

    /* U+0052 "R" */
    0xfb, 0x3c, 0xfe, 0xcf, 0x3c, 0xc0,

    /* U+0053 "S" */
    0x7b, 0x3c, 0x1e, 0xf, 0x37, 0x80,

    /* U+0054 "T" */
    0xfc, 0xc3, 0xc, 0x30, 0xc3, 0x0,

    /* U+0055 "U" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x37, 0x80,

    /* U+0056 "V" */
    0xc7, 0x8f, 0x1b, 0x66, 0xc7, 0xe, 0x0,

    /* U+0057 "W" */
    0xc0, 0xf3, 0x3c, 0xcf, 0x7b, 0xff, 0xdc, 0xe6,
    0x18,

    /* U+0058 "X" */
    0xcf, 0x37, 0x8c, 0x7b, 0x3c, 0xc0,

    /* U+0059 "Y" */
    0xcf, 0x37, 0x9e, 0x30, 0xc3, 0x0,

    /* U+005A "Z" */
    0xfc, 0x31, 0x8c, 0x63, 0xf, 0xc0,

    /* U+005B "[" */
    0xfb, 0x6d, 0xb8,

    /* U+005C "\\" */
    0xc6, 0x18, 0xc3, 0x18, 0x60,

    /* U+005D "]" */
    0xed, 0xb6, 0xf8,

    /* U+005E "^" */
    0x31, 0xec, 0xc0,

    /* U+005F "_" */
    0xf0,

    /* U+0061 "a" */
    0x70, 0xdf, 0xb7, 0x80,

    /* U+0062 "b" */
    0xc6, 0x3d, 0xbd, 0xef, 0xc0,

    /* U+0063 "c" */
    0x7e, 0x31, 0x87, 0x80,

    /* U+0064 "d" */
    0x18, 0xdf, 0xbd, 0xed, 0xe0,

    /* U+0065 "e" */
    0x76, 0xff, 0x87, 0x0,

    /* U+0066 "f" */
    0x33, 0xd9, 0xf6, 0x31, 0x80,

    /* U+0067 "g" */
    0x7e, 0xf7, 0xb7, 0x8d, 0xc0,

    /* U+0068 "h" */
    0xc6, 0x3d, 0xbd, 0xef, 0x60,

    /* U+0069 "i" */
    0x60, 0xe6, 0x66, 0xf0,

    /* U+006A "j" */
    0x30, 0xf3, 0x33, 0x33, 0xe0,

    /* U+006B "k" */
    0xc6, 0x37, 0xee, 0x7b, 0x60,

    /* U+006C "l" */
    0xe6, 0x66, 0x66, 0x70,

    /* U+006D "m" */
    0xfd, 0xaf, 0x5e, 0xbd, 0x60,

    /* U+006E "n" */
    0xf6, 0xf7, 0xbd, 0x80,

    /* U+006F "o" */
    0x76, 0xf7, 0xb7, 0x0,

    /* U+0070 "p" */
    0xf6, 0xf7, 0xbf, 0x63, 0x0,

    /* U+0071 "q" */
    0x7e, 0xf7, 0xb7, 0x8c, 0x60,

    /* U+0072 "r" */
    0xde, 0xcc, 0xc0,

    /* U+0073 "s" */
    0x7e, 0x1c, 0x3f, 0x0,

    /* U+0074 "t" */
    0x63, 0x3e, 0xc6, 0x30, 0xe0,

    /* U+0075 "u" */
    0xde, 0xf7, 0xb7, 0x80,

    /* U+0076 "v" */
    0xcf, 0x37, 0x9e, 0x30,

    /* U+0077 "w" */
    0xc7, 0xaf, 0x5b, 0x66, 0xc0,

    /* U+0078 "x" */
    0xcd, 0xe3, 0x1e, 0xcc,

    /* U+0079 "y" */
    0xde, 0xf7, 0xb7, 0x8d, 0xc0,

    /* U+007A "z" */
    0xf9, 0x99, 0x8f, 0x80,

    /* U+007B "{" */
    0x36, 0x6c, 0x66, 0x30,

    /* U+007C "|" */
    0xff, 0xfc,

    /* U+007D "}" */
    0xc6, 0x63, 0x66, 0xc0,

    /* U+007E "~" */
    0x47, 0x6e, 0x20,

    /* U+221E "∞" */
    0x73, 0xb7, 0xbc, 0xcf, 0x7b, 0x73, 0x80
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 80, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 64, .box_w = 2, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 64, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 4, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 11, .adv_w = 144, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 20, .adv_w = 144, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 27, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 34, .adv_w = 32, .box_w = 1, .box_h = 2, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 35, .adv_w = 64, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 38, .adv_w = 64, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 41, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 43, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 47, .adv_w = 48, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 48, .adv_w = 64, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 49, .adv_w = 64, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 50, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 55, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 61, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 67, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 73, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 79, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 85, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 91, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 97, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 103, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 109, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 115, .adv_w = 32, .box_w = 1, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 116, .adv_w = 48, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 118, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 121, .adv_w = 112, .box_w = 6, .box_h = 3, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 124, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 127, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 133, .adv_w = 160, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 141, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 147, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 153, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 159, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 165, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 170, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 175, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 181, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 191, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 202, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 207, .adv_w = 160, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 215, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 222, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 228, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 234, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 240, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 246, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 252, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 258, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 264, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 271, .adv_w = 176, .box_w = 10, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 280, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 286, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 292, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 298, .adv_w = 64, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 301, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 306, .adv_w = 64, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 309, .adv_w = 112, .box_w = 6, .box_h = 3, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 312, .adv_w = 80, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 313, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 317, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 322, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 326, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 331, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 335, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 340, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 345, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 350, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 354, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 359, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 364, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 368, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 373, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 377, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 381, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 386, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 391, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 394, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 398, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 403, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 407, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 411, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 416, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 420, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 425, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 429, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 433, .adv_w = 64, .box_w = 2, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 435, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 439, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 442, .adv_w = 176, .box_w = 10, .box_h = 5, .ofs_x = 0, .ofs_y = 1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 64, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 97, .range_length = 30, .glyph_id_start = 65,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 8734, .range_length = 1, .glyph_id_start = 95,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
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
    .cmap_num = 3,
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
const lv_font_t lv_busy_bold_7px = {
#else
lv_font_t lv_busy_bold_7px = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 10,          /*The maximum line height required by the font*/
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



#endif /*#if LV_BUSY_BOLD_7PX*/

