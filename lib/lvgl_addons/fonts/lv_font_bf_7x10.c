/*******************************************************************************
 * Size: 14 px
 * Bpp: 1
 * Opts: --font BF-7x10.ttf --bpp 1 --size 14 --no-compress --symbols ⁰¹²³⁴⁵⁶⁷⁸⁹…¡•–—‚„“”‘’«»‹»°€₽£¥×÷±≈∞↑→↓← --range 32-127,1040-1103 --format lvgl -o lib/lvgl_addons/fonts/lv_font_bf_7x10.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_FONT_BF_7X10
#define LV_FONT_BF_7X10 1
#endif

#if LV_FONT_BF_7X10

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xfc, 0xf0,

    /* U+0022 "\"" */
    0xde, 0xd3, 0x20,

    /* U+0023 "#" */
    0x33, 0xc, 0xcf, 0xff, 0xff, 0x33, 0xc, 0xcf,
    0xff, 0xff, 0x33, 0xc, 0xc0,

    /* U+0024 "$" */
    0x18, 0x7e, 0xff, 0xd8, 0xfe, 0x7f, 0x1b, 0xff,
    0x7e, 0x18,

    /* U+0025 "%" */
    0x60, 0xfc, 0x7f, 0x39, 0x9c, 0xe, 0x7, 0x3,
    0x99, 0xcf, 0xe3, 0xf0, 0x60,

    /* U+0026 "&" */
    0x7c, 0x7f, 0x31, 0x9d, 0xb7, 0xff, 0xfb, 0x39,
    0x8e, 0xff, 0xbe, 0xc0,

    /* U+0027 "'" */
    0xf6,

    /* U+0028 "(" */
    0x37, 0x6c, 0xcc, 0xc6, 0x73,

    /* U+0029 ")" */
    0xce, 0x63, 0x33, 0x36, 0xec,

    /* U+002A "*" */
    0xcf, 0x33, 0xc, 0xcf, 0x30,

    /* U+002B "+" */
    0x30, 0xcf, 0xff, 0x30, 0xc0,

    /* U+002C "," */
    0xf6,

    /* U+002D "-" */
    0xfc,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x18, 0xcc, 0x63, 0x31, 0x8c, 0xc6, 0x0,

    /* U+0030 "0" */
    0x7d, 0xff, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xfe,
    0xf8,

    /* U+0031 "1" */
    0x6e, 0xe6, 0x66, 0x66, 0xff,

    /* U+0032 "2" */
    0x7d, 0xff, 0x18, 0x71, 0xc7, 0x1c, 0x70, 0xff,
    0xfc,

    /* U+0033 "3" */
    0x7d, 0xff, 0x18, 0x31, 0xc3, 0xc1, 0xe3, 0xfe,
    0xf8,

    /* U+0034 "4" */
    0x3c, 0xf9, 0xb7, 0x6c, 0xd9, 0xb3, 0x7f, 0xfe,
    0x18,

    /* U+0035 "5" */
    0xff, 0xff, 0x6, 0xf, 0xcf, 0xc1, 0xe3, 0xfe,
    0xf8,

    /* U+0036 "6" */
    0x7d, 0xff, 0x1e, 0xf, 0xdf, 0xf1, 0xe3, 0xfe,
    0xf8,

    /* U+0037 "7" */
    0xff, 0xfc, 0x18, 0x61, 0xc3, 0xc, 0x18, 0x30,
    0x60,

    /* U+0038 "8" */
    0x7d, 0xff, 0x1e, 0x37, 0xdf, 0xf1, 0xe3, 0xfe,
    0xf8,

    /* U+0039 "9" */
    0x7d, 0xff, 0x1e, 0x3f, 0xef, 0xc1, 0xe3, 0xfe,
    0xf8,

    /* U+003A ":" */
    0xf0, 0xf0,

    /* U+003B ";" */
    0xf0, 0xf6,

    /* U+003C "<" */
    0x33, 0xcc, 0x33,

    /* U+003D "=" */
    0xff, 0xf0, 0x0, 0xff, 0xf0,

    /* U+003E ">" */
    0xcc, 0x33, 0xcc,

    /* U+003F "?" */
    0x7b, 0xfc, 0xc3, 0x18, 0xe3, 0x0, 0x30, 0xc0,

    /* U+0040 "@" */
    0x3f, 0xcf, 0xff, 0x81, 0xe7, 0xbd, 0xf7, 0xb6,
    0xf7, 0xde, 0x7f, 0x60, 0xc7, 0xc0,

    /* U+0041 "A" */
    0x7d, 0xff, 0x1e, 0x3f, 0xff, 0xf1, 0xe3, 0xc7,
    0x8c,

    /* U+0042 "B" */
    0xfd, 0xff, 0x1e, 0x3f, 0xdf, 0xf1, 0xe3, 0xff,
    0xf8,

    /* U+0043 "C" */
    0x7d, 0xff, 0x1e, 0xc, 0x18, 0x30, 0x63, 0xfe,
    0xf8,

    /* U+0044 "D" */
    0xf9, 0xfb, 0x3e, 0x3c, 0x78, 0xf1, 0xe7, 0xfd,
    0xf0,

    /* U+0045 "E" */
    0xff, 0xfc, 0x30, 0xfb, 0xec, 0x30, 0xff, 0xf0,

    /* U+0046 "F" */
    0xff, 0xfc, 0x30, 0xfb, 0xec, 0x30, 0xc3, 0x0,

    /* U+0047 "G" */
    0x7d, 0xff, 0x1e, 0xc, 0xf9, 0xf1, 0xe3, 0xfe,
    0xf8,

    /* U+0048 "H" */
    0xc7, 0x8f, 0x1e, 0x3f, 0xff, 0xf1, 0xe3, 0xc7,
    0x8c,

    /* U+0049 "I" */
    0xff, 0x66, 0x66, 0x66, 0xff,

    /* U+004A "J" */
    0xc, 0x30, 0xc3, 0xc, 0x3c, 0xf3, 0xfd, 0xe0,

    /* U+004B "K" */
    0xc7, 0x8f, 0x3e, 0xef, 0x9f, 0xb3, 0xe3, 0xc7,
    0x8c,

    /* U+004C "L" */
    0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xff, 0xf0,

    /* U+004D "M" */
    0xc3, 0xc3, 0xe7, 0xff, 0xff, 0xdb, 0xc3, 0xc3,
    0xc3, 0xc3,

    /* U+004E "N" */
    0xc7, 0x8f, 0x9f, 0xbf, 0xfb, 0xf3, 0xe3, 0xc7,
    0x8c,

    /* U+004F "O" */
    0x7d, 0xff, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xfe,
    0xf8,

    /* U+0050 "P" */
    0xfd, 0xff, 0x1e, 0x3f, 0xff, 0xb0, 0x60, 0xc1,
    0x80,

    /* U+0051 "Q" */
    0x7d, 0xff, 0x1e, 0x3c, 0x78, 0xf1, 0xe6, 0xfe,
    0xec,

    /* U+0052 "R" */
    0xfd, 0xff, 0x1e, 0x3f, 0xdf, 0xf1, 0xe3, 0xc7,
    0x8c,

    /* U+0053 "S" */
    0x7d, 0xff, 0x1e, 0xf, 0xcf, 0xc1, 0xe3, 0xfe,
    0xf8,

    /* U+0054 "T" */
    0xff, 0xf3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc0,

    /* U+0055 "U" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xfe,
    0xf8,

    /* U+0056 "V" */
    0xc3, 0xc3, 0xc3, 0xe7, 0x66, 0x66, 0x66, 0x66,
    0x3c, 0x3c,

    /* U+0057 "W" */
    0xc3, 0xc3, 0xdb, 0xdb, 0xdb, 0xdb, 0xff, 0xff,
    0x66, 0x66,

    /* U+0058 "X" */
    0xc7, 0x8f, 0x1f, 0x77, 0xcf, 0xbb, 0xe3, 0xc7,
    0x8c,

    /* U+0059 "Y" */
    0xc7, 0x8f, 0x1e, 0x3f, 0xef, 0xc1, 0xe3, 0xfe,
    0xf8,

    /* U+005A "Z" */
    0xff, 0xf0, 0xc7, 0x39, 0xce, 0x30, 0xff, 0xf0,

    /* U+005B "[" */
    0xff, 0xcc, 0xcc, 0xcc, 0xff,

    /* U+005C "\\" */
    0xc6, 0x18, 0xc6, 0x18, 0xc6, 0x18, 0xc0,

    /* U+005D "]" */
    0xff, 0x33, 0x33, 0x33, 0xff,

    /* U+005E "^" */
    0x38, 0xfb, 0xbe, 0x30,

    /* U+005F "_" */
    0xff,

    /* U+0061 "a" */
    0x7d, 0xff, 0x1e, 0x3f, 0xff, 0xf1, 0xe3, 0xc7,
    0x8c,

    /* U+0062 "b" */
    0xfd, 0xff, 0x1e, 0x3f, 0xdf, 0xf1, 0xe3, 0xff,
    0xf8,

    /* U+0063 "c" */
    0x7d, 0xff, 0x1e, 0xc, 0x18, 0x30, 0x63, 0xfe,
    0xf8,

    /* U+0064 "d" */
    0xf9, 0xfb, 0x3e, 0x3c, 0x78, 0xf1, 0xe7, 0xfd,
    0xf0,

    /* U+0065 "e" */
    0xff, 0xfc, 0x30, 0xfb, 0xec, 0x30, 0xff, 0xf0,

    /* U+0066 "f" */
    0xff, 0xfc, 0x30, 0xfb, 0xec, 0x30, 0xc3, 0x0,

    /* U+0067 "g" */
    0x7d, 0xff, 0x1e, 0xc, 0xf9, 0xf1, 0xe3, 0xfe,
    0xf8,

    /* U+0068 "h" */
    0xc7, 0x8f, 0x1e, 0x3f, 0xff, 0xf1, 0xe3, 0xc7,
    0x8c,

    /* U+0069 "i" */
    0xff, 0x66, 0x66, 0x66, 0xff,

    /* U+006A "j" */
    0xc, 0x30, 0xc3, 0xc, 0x3c, 0xf3, 0xfd, 0xe0,

    /* U+006B "k" */
    0xc7, 0x8f, 0x3e, 0xef, 0x9f, 0xb3, 0xe3, 0xc7,
    0x8c,

    /* U+006C "l" */
    0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xff, 0xf0,

    /* U+006D "m" */
    0xc3, 0xc3, 0xe7, 0xff, 0xff, 0xdb, 0xc3, 0xc3,
    0xc3, 0xc3,

    /* U+006E "n" */
    0xc7, 0x8f, 0x9f, 0xbf, 0xfb, 0xf3, 0xe3, 0xc7,
    0x8c,

    /* U+006F "o" */
    0x7d, 0xff, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xfe,
    0xf8,

    /* U+0070 "p" */
    0xfd, 0xff, 0x1e, 0x3f, 0xff, 0xb0, 0x60, 0xc1,
    0x80,

    /* U+0071 "q" */
    0x7d, 0xff, 0x1e, 0x3c, 0x78, 0xf1, 0xe6, 0xfe,
    0xec,

    /* U+0072 "r" */
    0xfd, 0xff, 0x1e, 0x3f, 0xdf, 0xf1, 0xe3, 0xc7,
    0x8c,

    /* U+0073 "s" */
    0x7d, 0xff, 0x1e, 0xf, 0xcf, 0xc1, 0xe3, 0xfe,
    0xf8,

    /* U+0074 "t" */
    0xff, 0xf3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc0,

    /* U+0075 "u" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xfe,
    0xf8,

    /* U+0076 "v" */
    0xc3, 0xc3, 0xc3, 0xe7, 0x66, 0x66, 0x66, 0x66,
    0x3c, 0x3c,

    /* U+0077 "w" */
    0xc3, 0xc3, 0xdb, 0xdb, 0xdb, 0xdb, 0xff, 0xff,
    0x66, 0x66,

    /* U+0078 "x" */
    0xc7, 0x8f, 0x1f, 0x77, 0xcf, 0xbb, 0xe3, 0xc7,
    0x8c,

    /* U+0079 "y" */
    0xc7, 0x8f, 0x1e, 0x3f, 0xef, 0xc1, 0xe3, 0xfe,
    0xf8,

    /* U+007A "z" */
    0xff, 0xf0, 0xc7, 0x39, 0xce, 0x30, 0xff, 0xf0,

    /* U+007B "{" */
    0x37, 0x66, 0xcc, 0x66, 0x73,

    /* U+007C "|" */
    0xff, 0xff, 0xf0,

    /* U+007D "}" */
    0xce, 0x66, 0x33, 0x66, 0xec,

    /* U+007E "~" */
    0x77, 0xff, 0x70,

    /* U+00A1 "¡" */
    0xf3, 0xff, 0xf0,

    /* U+00A3 "£" */
    0x7d, 0xff, 0x1f, 0x7, 0x1f, 0xdc, 0x30, 0xff,
    0xfc,

    /* U+00A5 "¥" */
    0xc3, 0xc3, 0xe7, 0x66, 0x7e, 0x3c, 0x18, 0x7e,
    0x18, 0x18,

    /* U+00AB "«" */
    0x66, 0xcf, 0x36, 0x66, 0x6c, 0xc0,

    /* U+00B0 "°" */
    0x77, 0xf7, 0xf7, 0x0,

    /* U+00B1 "±" */
    0x30, 0xcf, 0xff, 0x30, 0xcf, 0xff,

    /* U+00B2 "²" */
    0xe1, 0x68, 0xf0,

    /* U+00B3 "³" */
    0xe1, 0x61, 0xe0,

    /* U+00B9 "¹" */
    0x6a, 0x22, 0xf0,

    /* U+00BB "»" */
    0xcd, 0x99, 0x9b, 0x3c, 0xd9, 0x80,

    /* U+00D7 "×" */
    0xcf, 0x33, 0xc, 0xcf, 0x30,

    /* U+00F7 "÷" */
    0x30, 0xc0, 0x3f, 0xfc, 0x3, 0xc,

    /* U+0410 "А" */
    0x7d, 0xff, 0x1e, 0x3f, 0xff, 0xf1, 0xe3, 0xc7,
    0x8c,

    /* U+0411 "Б" */
    0xfd, 0xfb, 0x6, 0xf, 0xdf, 0xf1, 0xe3, 0xff,
    0xf8,

    /* U+0412 "В" */
    0xfd, 0xff, 0x1e, 0x3f, 0xdf, 0xf1, 0xe3, 0xff,
    0xf8,

    /* U+0413 "Г" */
    0xff, 0xfc, 0x30, 0xc3, 0xc, 0x30, 0xc3, 0x0,

    /* U+0414 "Д" */
    0x3f, 0x1f, 0x8c, 0xc6, 0x63, 0x31, 0x99, 0x8c,
    0xc6, 0xff, 0xff, 0xf0, 0x60,

    /* U+0415 "Е" */
    0xff, 0xfc, 0x30, 0xfb, 0xec, 0x30, 0xff, 0xf0,

    /* U+0416 "Ж" */
    0xdb, 0xdb, 0xdb, 0xff, 0x7e, 0xdb, 0xdb, 0xdb,
    0xdb, 0xdb,

    /* U+0417 "З" */
    0x7d, 0xff, 0x18, 0x33, 0xc7, 0xc1, 0xe3, 0xfe,
    0xf8,

    /* U+0418 "И" */
    0xc7, 0x8f, 0x3e, 0xff, 0xfe, 0xf9, 0xe3, 0xc7,
    0x8c,

    /* U+0419 "Й" */
    0x38, 0x73, 0x1e, 0x3c, 0xfb, 0xff, 0xfb, 0xe7,
    0x8f, 0x1e, 0x30,

    /* U+041A "К" */
    0xc7, 0x8f, 0x3e, 0xef, 0x9f, 0xb3, 0xe3, 0xc7,
    0x8c,

    /* U+041B "Л" */
    0x3f, 0x7f, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
    0xe3, 0xc3,

    /* U+041C "М" */
    0xc3, 0xc3, 0xe7, 0xff, 0xff, 0xdb, 0xc3, 0xc3,
    0xc3, 0xc3,

    /* U+041D "Н" */
    0xc7, 0x8f, 0x1e, 0x3f, 0xff, 0xf1, 0xe3, 0xc7,
    0x8c,

    /* U+041E "О" */
    0x7d, 0xff, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xfe,
    0xf8,

    /* U+041F "П" */
    0xff, 0xff, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc7,
    0x8c,

    /* U+0420 "Р" */
    0xfd, 0xff, 0x1e, 0x3f, 0xff, 0xb0, 0x60, 0xc1,
    0x80,

    /* U+0421 "С" */
    0x7d, 0xff, 0x1e, 0xc, 0x18, 0x30, 0x63, 0xfe,
    0xf8,

    /* U+0422 "Т" */
    0xff, 0xf3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc0,

    /* U+0423 "У" */
    0xc7, 0x8f, 0x1e, 0x3f, 0xef, 0xc1, 0xe3, 0xfe,
    0xf8,

    /* U+0424 "Ф" */
    0x7f, 0xbf, 0xfc, 0xcf, 0x33, 0xcc, 0xf3, 0x3f,
    0xfd, 0xfe, 0xc, 0x3, 0x0,

    /* U+0425 "Х" */
    0xc7, 0x8f, 0x1f, 0x77, 0xcf, 0xbb, 0xe3, 0xc7,
    0x8c,

    /* U+0426 "Ц" */
    0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6,
    0xff, 0xff, 0x3,

    /* U+0427 "Ч" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x7f, 0xdf, 0x83, 0x6,
    0xc,

    /* U+0428 "Ш" */
    0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb,
    0xff, 0xff,

    /* U+0429 "Щ" */
    0xdb, 0x6d, 0xb6, 0xdb, 0x6d, 0xb6, 0xdb, 0x6d,
    0xb6, 0xff, 0xff, 0xc0, 0x60,

    /* U+042A "Ъ" */
    0xe0, 0xe0, 0x60, 0x60, 0x7e, 0x7f, 0x63, 0x63,
    0x7f, 0x7e,

    /* U+042B "Ы" */
    0xc0, 0xf0, 0x3c, 0xf, 0x3, 0xfc, 0xff, 0xbc,
    0x6f, 0x1b, 0xfe, 0xff, 0x30,

    /* U+042C "Ь" */
    0xc1, 0x83, 0x6, 0xf, 0xdf, 0xf1, 0xe3, 0xff,
    0xf8,

    /* U+042D "Э" */
    0x7d, 0xff, 0x18, 0x33, 0xe7, 0xc1, 0xe3, 0xfe,
    0xf8,

    /* U+042E "Ю" */
    0xcf, 0xb7, 0xfd, 0x8f, 0x63, 0xf8, 0xfe, 0x3d,
    0x8f, 0x63, 0xdf, 0xf3, 0xe0,

    /* U+042F "Я" */
    0x7f, 0xff, 0x1e, 0x3f, 0xef, 0xf9, 0xe3, 0xc7,
    0x8c,

    /* U+0430 "а" */
    0x7d, 0xff, 0x1e, 0x3f, 0xff, 0xf1, 0xe3, 0xc7,
    0x8c,

    /* U+0431 "б" */
    0xfd, 0xfb, 0x6, 0xf, 0xdf, 0xf1, 0xe3, 0xff,
    0xf8,

    /* U+0432 "в" */
    0xfd, 0xff, 0x1e, 0x3f, 0xdf, 0xf1, 0xe3, 0xff,
    0xf8,

    /* U+0433 "г" */
    0xff, 0xfc, 0x30, 0xc3, 0xc, 0x30, 0xc3, 0x0,

    /* U+0434 "д" */
    0x3f, 0x1f, 0x8c, 0xc6, 0x63, 0x31, 0x99, 0x8c,
    0xc6, 0xff, 0xff, 0xf0, 0x60,

    /* U+0435 "е" */
    0xff, 0xfc, 0x30, 0xfb, 0xec, 0x30, 0xff, 0xf0,

    /* U+0436 "ж" */
    0xdb, 0xdb, 0xdb, 0xff, 0x7e, 0xdb, 0xdb, 0xdb,
    0xdb, 0xdb,

    /* U+0437 "з" */
    0x7d, 0xff, 0x18, 0x33, 0xc7, 0xc1, 0xe3, 0xfe,
    0xf8,

    /* U+0438 "и" */
    0xc7, 0x8f, 0x3e, 0xff, 0xfe, 0xf9, 0xe3, 0xc7,
    0x8c,

    /* U+0439 "й" */
    0x38, 0x73, 0x1e, 0x3c, 0xfb, 0xff, 0xfb, 0xe7,
    0x8f, 0x1e, 0x30,

    /* U+043A "к" */
    0xc7, 0x8f, 0x3e, 0xef, 0x9f, 0xb3, 0xe3, 0xc7,
    0x8c,

    /* U+043B "л" */
    0x3f, 0x7f, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
    0xe3, 0xc3,

    /* U+043C "м" */
    0xc3, 0xc3, 0xe7, 0xff, 0xff, 0xdb, 0xc3, 0xc3,
    0xc3, 0xc3,

    /* U+043D "н" */
    0xc7, 0x8f, 0x1e, 0x3f, 0xff, 0xf1, 0xe3, 0xc7,
    0x8c,

    /* U+043E "о" */
    0x7d, 0xff, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xfe,
    0xf8,

    /* U+043F "п" */
    0xff, 0xff, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc7,
    0x8c,

    /* U+0440 "р" */
    0xfd, 0xff, 0x1e, 0x3f, 0xff, 0xb0, 0x60, 0xc1,
    0x80,

    /* U+0441 "с" */
    0x7d, 0xff, 0x1e, 0xc, 0x18, 0x30, 0x63, 0xfe,
    0xf8,

    /* U+0442 "т" */
    0xff, 0xf3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc0,

    /* U+0443 "у" */
    0xc7, 0x8f, 0x1e, 0x3f, 0xef, 0xc1, 0xe3, 0xfe,
    0xf8,

    /* U+0444 "ф" */
    0x7f, 0xbf, 0xfc, 0xcf, 0x33, 0xcc, 0xf3, 0x3f,
    0xfd, 0xfe, 0xc, 0x3, 0x0,

    /* U+0445 "х" */
    0xc7, 0x8f, 0x1f, 0x77, 0xcf, 0xbb, 0xe3, 0xc7,
    0x8c,

    /* U+0446 "ц" */
    0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6,
    0xff, 0xff, 0x3,

    /* U+0447 "ч" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x7f, 0xdf, 0x83, 0x6,
    0xc,

    /* U+0448 "ш" */
    0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb,
    0xff, 0xff,

    /* U+0449 "щ" */
    0xdb, 0x6d, 0xb6, 0xdb, 0x6d, 0xb6, 0xdb, 0x6d,
    0xb6, 0xff, 0xff, 0xc0, 0x60,

    /* U+044A "ъ" */
    0xe0, 0xe0, 0x60, 0x60, 0x7e, 0x7f, 0x63, 0x63,
    0x7f, 0x7e,

    /* U+044B "ы" */
    0xc0, 0xf0, 0x3c, 0xf, 0x3, 0xfc, 0xff, 0xbc,
    0x6f, 0x1b, 0xfe, 0xff, 0x30,

    /* U+044C "ь" */
    0xc1, 0x83, 0x6, 0xf, 0xdf, 0xf1, 0xe3, 0xff,
    0xf8,

    /* U+044D "э" */
    0x7d, 0xff, 0x18, 0x33, 0xe7, 0xc1, 0xe3, 0xfe,
    0xf8,

    /* U+044E "ю" */
    0xcf, 0xb7, 0xfd, 0x8f, 0x63, 0xf8, 0xfe, 0x3d,
    0x8f, 0x63, 0xdf, 0xf3, 0xe0,

    /* U+044F "я" */
    0x7f, 0xff, 0x1e, 0x3f, 0xef, 0xf9, 0xe3, 0xc7,
    0x8c,

    /* U+2013 "–" */
    0xff,

    /* U+2014 "—" */
    0xff, 0xf0,

    /* U+2018 "‘" */
    0xf6,

    /* U+2019 "’" */
    0xf6,

    /* U+201A "‚" */
    0xf6,

    /* U+201C "“" */
    0xde, 0xd3, 0x20,

    /* U+201D "”" */
    0xde, 0xd3, 0x20,

    /* U+201E "„" */
    0xde, 0xd3, 0x20,

    /* U+2022 "•" */
    0xf0,

    /* U+2026 "…" */
    0xdb, 0xdb,

    /* U+2039 "‹" */
    0x37, 0xec, 0xe7, 0x30,

    /* U+2070 "⁰" */
    0x69, 0x99, 0x60,

    /* U+2074 "⁴" */
    0x6a, 0xaf, 0x20,

    /* U+2075 "⁵" */
    0xf8, 0xe1, 0xe0,

    /* U+2076 "⁶" */
    0x78, 0xe9, 0x60,

    /* U+2077 "⁷" */
    0xf1, 0x24, 0x40,

    /* U+2078 "⁸" */
    0x69, 0x69, 0x60,

    /* U+2079 "⁹" */
    0x69, 0x71, 0xe0,

    /* U+20AC "€" */
    0x3e, 0x7f, 0x63, 0x60, 0xf8, 0x60, 0xf8, 0x63,
    0x7f, 0x3e,

    /* U+20BD "₽" */
    0x7e, 0x7f, 0x63, 0x63, 0xff, 0xfe, 0x60, 0xfc,
    0x60, 0x60,

    /* U+2190 "←" */
    0x18, 0x1c, 0x1c, 0x1f, 0xff, 0xfb, 0x80, 0xe0,
    0x30,

    /* U+2191 "↑" */
    0x18, 0x3c, 0x7e, 0xff, 0xdb, 0x18, 0x18, 0x18,
    0x18, 0x18,

    /* U+2192 "→" */
    0xc, 0x7, 0x1, 0xdf, 0xff, 0xf8, 0x38, 0x38,
    0x18,

    /* U+2193 "↓" */
    0x18, 0x18, 0x18, 0x18, 0x18, 0xdb, 0xff, 0x7e,
    0x3c, 0x18,

    /* U+221E "∞" */
    0x3c, 0x1e, 0x3f, 0x1f, 0xb9, 0xdc, 0xf8, 0x7c,
    0x3c, 0x3e, 0x1f, 0x3b, 0x9d, 0xf8, 0xfc, 0x78,
    0x3c,

    /* U+2248 "≈" */
    0x77, 0xff, 0x70, 0x0, 0xe, 0xff, 0xee
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 64, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 48, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 7, .adv_w = 176, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 20, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 30, .adv_w = 160, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 43, .adv_w = 160, .box_w = 9, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 55, .adv_w = 48, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 56, .adv_w = 80, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 61, .adv_w = 80, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 66, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 71, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 76, .adv_w = 48, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 77, .adv_w = 64, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 78, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 79, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 95, .adv_w = 80, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 100, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 109, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 118, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 127, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 136, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 145, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 172, .adv_w = 48, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 174, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 176, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 179, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 184, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 187, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 195, .adv_w = 192, .box_w = 11, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 209, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 218, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 227, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 236, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 245, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 253, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 261, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 270, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 279, .adv_w = 80, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 292, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 301, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 309, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 319, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 328, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 337, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 346, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 355, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 364, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 373, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 381, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 390, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 400, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 410, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 419, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 428, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 436, .adv_w = 80, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 441, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 448, .adv_w = 80, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 453, .adv_w = 128, .box_w = 7, .box_h = 4, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 457, .adv_w = 80, .box_w = 4, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 458, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 467, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 476, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 485, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 494, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 502, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 510, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 519, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 528, .adv_w = 80, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 533, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 541, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 550, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 558, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 568, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 577, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 586, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 595, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 604, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 613, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 622, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 630, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 639, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 649, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 659, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 668, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 677, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 685, .adv_w = 80, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 690, .adv_w = 48, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 693, .adv_w = 80, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 698, .adv_w = 128, .box_w = 7, .box_h = 3, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 701, .adv_w = 192, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 704, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 713, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 723, .adv_w = 128, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 729, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 733, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 739, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 742, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 745, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 748, .adv_w = 128, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 754, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 759, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 765, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 774, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 783, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 792, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 800, .adv_w = 160, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 813, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 821, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 831, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 840, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 849, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 860, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 869, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 879, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 889, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 898, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 907, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 916, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 925, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 934, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 942, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 951, .adv_w = 176, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 964, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 973, .adv_w = 144, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 984, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 993, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1003, .adv_w = 160, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1016, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1026, .adv_w = 176, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1039, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1048, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1057, .adv_w = 176, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1070, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1079, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1088, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1097, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1106, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1114, .adv_w = 160, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1127, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1135, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1145, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1154, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1163, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1174, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1183, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1193, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1203, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1212, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1221, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1230, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1239, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1248, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1256, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1265, .adv_w = 176, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1278, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1287, .adv_w = 144, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1298, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1307, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1317, .adv_w = 160, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1330, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1340, .adv_w = 176, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1353, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1362, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1371, .adv_w = 176, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1384, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1393, .adv_w = 80, .box_w = 4, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 1394, .adv_w = 112, .box_w = 6, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 1396, .adv_w = 48, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 1397, .adv_w = 48, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 1398, .adv_w = 48, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1399, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 1402, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 1405, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1408, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 1409, .adv_w = 144, .box_w = 8, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1411, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1415, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 1418, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 1421, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 1424, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 1427, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 1430, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 1433, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 1436, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1446, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1456, .adv_w = 160, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1465, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1475, .adv_w = 160, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1484, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1494, .adv_w = 288, .box_w = 17, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1511, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_2[] = {
    0x0, 0x2, 0x4, 0xa, 0xf, 0x10, 0x11, 0x12,
    0x18, 0x1a, 0x36, 0x56
};

static const uint16_t unicode_list_4[] = {
    0x0, 0x1, 0x5, 0x6, 0x7, 0x9, 0xa, 0xb,
    0xf, 0x13, 0x26, 0x5d, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x99, 0xaa, 0x17d, 0x17e, 0x17f, 0x180,
    0x20b, 0x235
};

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
        .range_start = 161, .range_length = 87, .glyph_id_start = 95,
        .unicode_list = unicode_list_2, .glyph_id_ofs_list = NULL, .list_length = 12, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    },
    {
        .range_start = 1040, .range_length = 64, .glyph_id_start = 107,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 8211, .range_length = 566, .glyph_id_start = 171,
        .unicode_list = unicode_list_4, .glyph_id_ofs_list = NULL, .list_length = 26, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
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
    .cmap_num = 5,
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
const lv_font_t lv_font_bf_7x10 = {
#else
lv_font_t lv_font_bf_7x10 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 14,          /*The maximum line height required by the font*/
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



#endif /*#if LV_FONT_BF_7X10*/

