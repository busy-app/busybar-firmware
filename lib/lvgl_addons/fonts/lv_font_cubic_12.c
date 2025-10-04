/*******************************************************************************
 * Size: 12 px
 * Bpp: 1
 * Opts: --font /home/portasynthinca3/Downloads/Cubic_11.ttf --bpp 1 --size 12 --no-compress --symbols ▶▹◃∞ --range 32-127,1040-1103 --format lvgl -o lib/lvgl_addons/fonts/lv_font_cubic_12.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_FONT_CUBIC_12
#define LV_FONT_CUBIC_12 1
#endif

#if LV_FONT_CUBIC_12

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
    0xb6, 0xd0,

    /* U+0023 "#" */
    0x49, 0x2f, 0xd2, 0x49, 0x2f, 0xd2, 0x48,

    /* U+0024 "$" */
    0x11, 0xe9, 0x64, 0x91, 0xe2, 0x49, 0xa5, 0xe2,
    0x0,

    /* U+0025 "%" */
    0x61, 0x26, 0x53, 0x41, 0x5, 0x94, 0xc9, 0xc,

    /* U+0026 "&" */
    0x72, 0x28, 0x94, 0x66, 0x58, 0xa2, 0x74,

    /* U+0027 "'" */
    0xf0,

    /* U+0028 "(" */
    0x29, 0x49, 0x24, 0x48, 0x80,

    /* U+0029 ")" */
    0x89, 0x12, 0x49, 0x4a, 0x0,

    /* U+002A "*" */
    0x25, 0x5c, 0x47, 0x54, 0x80,

    /* U+002B "+" */
    0x10, 0x20, 0x47, 0xf1, 0x2, 0x4, 0x0,

    /* U+002C "," */
    0x58,

    /* U+002D "-" */
    0xfc,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x8, 0x44, 0x22, 0x11, 0x8, 0x84, 0x0,

    /* U+0030 "0" */
    0x74, 0x63, 0x18, 0xc6, 0x31, 0x70,

    /* U+0031 "1" */
    0x2e, 0x92, 0x49, 0x20,

    /* U+0032 "2" */
    0x74, 0x62, 0x11, 0x11, 0x10, 0xf8,

    /* U+0033 "3" */
    0xf8, 0x44, 0x47, 0x4, 0x31, 0x70,

    /* U+0034 "4" */
    0x18, 0x62, 0x8a, 0x49, 0x2f, 0xc2, 0x8,

    /* U+0035 "5" */
    0xfc, 0x21, 0xe0, 0x86, 0x31, 0x70,

    /* U+0036 "6" */
    0x32, 0x21, 0xe8, 0xc6, 0x31, 0x70,

    /* U+0037 "7" */
    0xf8, 0x42, 0x21, 0x10, 0x88, 0x40,

    /* U+0038 "8" */
    0x74, 0x63, 0x17, 0x46, 0x31, 0x70,

    /* U+0039 "9" */
    0x74, 0x63, 0x18, 0xbc, 0x22, 0x60,

    /* U+003A ":" */
    0x84,

    /* U+003B ";" */
    0x40, 0x16,

    /* U+003C "<" */
    0x8, 0x88, 0x88, 0x20, 0x82, 0x8,

    /* U+003D "=" */
    0xfc, 0xf, 0xc0,

    /* U+003E ">" */
    0x82, 0x8, 0x20, 0x88, 0x88, 0x80,

    /* U+003F "?" */
    0x7a, 0x18, 0x42, 0x10, 0x82, 0x0, 0x20,

    /* U+0040 "@" */
    0x3c, 0x86, 0x7d, 0x1a, 0x34, 0x67, 0x20, 0x3c,

    /* U+0041 "A" */
    0x10, 0x20, 0xa1, 0x44, 0x48, 0x9f, 0x41, 0x82,

    /* U+0042 "B" */
    0xfa, 0x18, 0x61, 0xfa, 0x18, 0x61, 0xf8,

    /* U+0043 "C" */
    0x39, 0x18, 0x20, 0x82, 0x8, 0x11, 0x38,

    /* U+0044 "D" */
    0xf9, 0xa, 0xc, 0x18, 0x30, 0x60, 0xc2, 0xf8,

    /* U+0045 "E" */
    0xfe, 0x8, 0x20, 0xfa, 0x8, 0x20, 0xfc,

    /* U+0046 "F" */
    0xfe, 0x8, 0x20, 0xfa, 0x8, 0x20, 0x80,

    /* U+0047 "G" */
    0x3c, 0x86, 0x4, 0x8, 0xf0, 0x60, 0xa3, 0x3a,

    /* U+0048 "H" */
    0x83, 0x6, 0xc, 0x1f, 0xf0, 0x60, 0xc1, 0x82,

    /* U+0049 "I" */
    0xff, 0x80,

    /* U+004A "J" */
    0x4, 0x10, 0x41, 0x4, 0x18, 0x62, 0x70,

    /* U+004B "K" */
    0x86, 0x29, 0x28, 0xc2, 0x89, 0x22, 0x84,

    /* U+004C "L" */
    0x82, 0x8, 0x20, 0x82, 0x8, 0x20, 0xfc,

    /* U+004D "M" */
    0x80, 0xe0, 0xe8, 0xb2, 0x98, 0x8c, 0x6, 0x3,
    0x1, 0x80, 0x80,

    /* U+004E "N" */
    0x83, 0x86, 0x8c, 0x98, 0xb0, 0xe0, 0xc1, 0x82,

    /* U+004F "O" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x60, 0xa2, 0x38,

    /* U+0050 "P" */
    0xfa, 0x18, 0x61, 0xfa, 0x8, 0x20, 0x80,

    /* U+0051 "Q" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x60, 0xa2, 0x38,
    0x10, 0x18,

    /* U+0052 "R" */
    0xfa, 0x18, 0x61, 0xfa, 0x89, 0x22, 0x84,

    /* U+0053 "S" */
    0x7a, 0x18, 0x20, 0x78, 0x10, 0x61, 0x78,

    /* U+0054 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x2, 0x4, 0x8, 0x10,

    /* U+0055 "U" */
    0x83, 0x6, 0xc, 0x18, 0x30, 0x60, 0xa2, 0x38,

    /* U+0056 "V" */
    0x83, 0x5, 0x12, 0x24, 0x45, 0xa, 0x8, 0x10,

    /* U+0057 "W" */
    0x88, 0xc4, 0x62, 0x29, 0x25, 0x52, 0xa8, 0x88,
    0x44, 0x22, 0x0,

    /* U+0058 "X" */
    0x86, 0x14, 0x92, 0x31, 0x24, 0xa1, 0x84,

    /* U+0059 "Y" */
    0x83, 0x5, 0x12, 0x22, 0x82, 0x4, 0x8, 0x10,

    /* U+005A "Z" */
    0xfc, 0x10, 0x84, 0x21, 0x8, 0x20, 0xfc,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x93, 0x80,

    /* U+005C "\\" */
    0x84, 0x10, 0x82, 0x10, 0x42, 0x8, 0x40,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x27, 0x80,

    /* U+005E "^" */
    0x22, 0xa2,

    /* U+005F "_" */
    0xfc,

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
    0x19, 0x9, 0xf2, 0x10, 0x84, 0x20,

    /* U+0067 "g" */
    0x7c, 0x63, 0x18, 0xbc, 0x31, 0x70,

    /* U+0068 "h" */
    0x84, 0x21, 0x6c, 0xc6, 0x31, 0x88,

    /* U+0069 "i" */
    0x9f, 0x80,

    /* U+006A "j" */
    0x10, 0x1, 0x11, 0x11, 0x11, 0x2c,

    /* U+006B "k" */
    0x84, 0x21, 0x2a, 0x62, 0x92, 0x88,

    /* U+006C "l" */
    0xff, 0xc0,

    /* U+006D "m" */
    0xfd, 0x26, 0x4c, 0x99, 0x32, 0x40,

    /* U+006E "n" */
    0xb6, 0x63, 0x18, 0xc4,

    /* U+006F "o" */
    0x74, 0x63, 0x18, 0xb8,

    /* U+0070 "p" */
    0xf4, 0x63, 0x18, 0xfa, 0x10, 0x80,

    /* U+0071 "q" */
    0x7c, 0x63, 0x18, 0xbc, 0x21, 0x8,

    /* U+0072 "r" */
    0xbc, 0x88, 0x88,

    /* U+0073 "s" */
    0x74, 0x1c, 0x18, 0xb8,

    /* U+0074 "t" */
    0x44, 0x4f, 0x44, 0x44, 0x30,

    /* U+0075 "u" */
    0x8c, 0x63, 0x19, 0xb4,

    /* U+0076 "v" */
    0x8c, 0x54, 0xa2, 0x10,

    /* U+0077 "w" */
    0x88, 0xc4, 0x55, 0x4a, 0xa2, 0x21, 0x10,

    /* U+0078 "x" */
    0x8a, 0x88, 0x45, 0x44,

    /* U+0079 "y" */
    0x8c, 0x63, 0x18, 0xbc, 0x31, 0x70,

    /* U+007A "z" */
    0xf8, 0x88, 0x88, 0x7c,

    /* U+007B "{" */
    0x19, 0x8, 0x42, 0x60, 0x84, 0x21, 0x6,

    /* U+007C "|" */
    0xff, 0xe0,

    /* U+007D "}" */
    0xc1, 0x8, 0x42, 0xc, 0x84, 0x21, 0x30,

    /* U+007E "~" */
    0x66, 0x60,

    /* U+0410 "А" */
    0x1c, 0x0, 0x80, 0x28, 0x5, 0x1, 0x10, 0x22,
    0x7, 0xc1, 0x4, 0x20, 0x9e, 0x3c,

    /* U+0411 "Б" */
    0xfe, 0x10, 0x88, 0x24, 0x3, 0xe1, 0x8, 0x82,
    0x41, 0x21, 0x7f, 0x0,

    /* U+0412 "В" */
    0xfe, 0x10, 0x88, 0x44, 0x23, 0xe1, 0x8, 0x82,
    0x41, 0x21, 0x7f, 0x0,

    /* U+0413 "Г" */
    0xff, 0x21, 0x21, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0xf0,

    /* U+0414 "Д" */
    0x7f, 0x84, 0x41, 0x10, 0x44, 0x11, 0x4, 0x42,
    0x10, 0x84, 0x7f, 0x10, 0x28, 0x4,

    /* U+0415 "Е" */
    0xfe, 0x11, 0x8, 0x44, 0x83, 0xc1, 0x20, 0x80,
    0x41, 0x21, 0x7f, 0x80,

    /* U+0416 "Ж" */
    0x1f, 0x8, 0x8a, 0x92, 0x92, 0x41, 0xf0, 0x2a,
    0x9, 0x21, 0x24, 0x44, 0x5b, 0xec,

    /* U+0417 "З" */
    0xbc, 0xc2, 0x81, 0x2, 0xc, 0x2, 0x1, 0x81,
    0xc2, 0xbc,

    /* U+0418 "И" */
    0xf3, 0xc8, 0xc2, 0x50, 0x94, 0x29, 0xa, 0x43,
    0x10, 0xc4, 0x21, 0x38, 0xf0,

    /* U+0419 "Й" */
    0x12, 0x3, 0xe, 0x1c, 0x8c, 0x23, 0x9, 0x42,
    0x50, 0xa4, 0x29, 0xc, 0x4f, 0x3c,

    /* U+041A "К" */
    0xf1, 0x88, 0x92, 0x20, 0x90, 0x28, 0xe, 0x2,
    0x40, 0x88, 0x21, 0x3c, 0xf0,

    /* U+041B "Л" */
    0x3f, 0xc2, 0x40, 0x90, 0x24, 0x9, 0x2, 0x48,
    0x92, 0x24, 0x91, 0x18, 0xf0,

    /* U+041C "М" */
    0x71, 0xc2, 0x20, 0x44, 0x15, 0x42, 0xa8, 0x55,
    0x9, 0x21, 0x24, 0x24, 0x9e, 0x3c,

    /* U+041D "Н" */
    0xf3, 0xc8, 0x42, 0x10, 0x84, 0x3f, 0x8, 0x42,
    0x10, 0x84, 0x21, 0x3c, 0xf0,

    /* U+041E "О" */
    0x3e, 0x20, 0xa0, 0x30, 0x18, 0xc, 0x6, 0x3,
    0x1, 0x41, 0x1f, 0x0,

    /* U+041F "П" */
    0x7f, 0x88, 0x42, 0x10, 0x84, 0x21, 0x8, 0x42,
    0x10, 0x84, 0x21, 0x3c, 0xf0,

    /* U+0420 "Р" */
    0xfe, 0x10, 0x88, 0x24, 0x12, 0x11, 0xf0, 0x80,
    0x40, 0x20, 0x78, 0x0,

    /* U+0421 "С" */
    0x3d, 0x43, 0x81, 0x80, 0x80, 0x80, 0x80, 0x81,
    0x43, 0x3c,

    /* U+0422 "Т" */
    0x7f, 0xc8, 0x89, 0x11, 0x42, 0x10, 0x40, 0x8,
    0x1, 0x0, 0x20, 0x4, 0x3, 0xe0,

    /* U+0423 "У" */
    0xf3, 0xc8, 0x42, 0x10, 0x48, 0x12, 0x5, 0x1,
    0x40, 0x20, 0x88, 0x24, 0x6, 0x0,

    /* U+0424 "Ф" */
    0x3e, 0x4, 0xf, 0x89, 0x28, 0x8c, 0x45, 0x24,
    0x7c, 0x8, 0x1f, 0x0,

    /* U+0425 "Х" */
    0xf7, 0x91, 0x8, 0x82, 0x80, 0x80, 0x40, 0x50,
    0x44, 0x22, 0x7b, 0xc0,

    /* U+0426 "Ц" */
    0xf7, 0x88, 0x82, 0x20, 0x88, 0x22, 0x8, 0x82,
    0x20, 0x88, 0x22, 0x3f, 0xe0, 0x4,

    /* U+0427 "Ч" */
    0xf3, 0xc8, 0x42, 0x10, 0x84, 0x21, 0x8, 0xc1,
    0xd0, 0x4, 0x1, 0x0, 0xf0,

    /* U+0428 "Ш" */
    0xee, 0xe4, 0x90, 0x92, 0x12, 0x42, 0x48, 0x49,
    0x9, 0x21, 0x24, 0x24, 0x9f, 0xfc,

    /* U+0429 "Щ" */
    0xee, 0xe2, 0x48, 0x24, 0x82, 0x48, 0x24, 0x82,
    0x48, 0x24, 0x82, 0x48, 0x24, 0x8f, 0xfe, 0x0,
    0x10,

    /* U+042A "Ъ" */
    0xfc, 0x24, 0x9, 0x0, 0x40, 0x1f, 0x4, 0x21,
    0x4, 0x41, 0x10, 0x9f, 0xc0,

    /* U+042B "Ы" */
    0xfb, 0xe4, 0x10, 0x82, 0x10, 0x43, 0xc8, 0x45,
    0x8, 0xa1, 0x14, 0x22, 0x9f, 0xbc,

    /* U+042C "Ь" */
    0xf8, 0x10, 0x8, 0x4, 0x3, 0xe1, 0x8, 0x82,
    0x41, 0x21, 0x7f, 0x0,

    /* U+042D "Э" */
    0xbc, 0xc2, 0x81, 0x1, 0x19, 0x27, 0x1, 0x81,
    0xc2, 0xbc,

    /* U+042E "Ю" */
    0xf1, 0x84, 0x48, 0x90, 0x92, 0x13, 0xc2, 0x48,
    0x49, 0x9, 0x21, 0x22, 0x5e, 0x30,

    /* U+042F "Я" */
    0x1f, 0xc8, 0x44, 0x11, 0x4, 0x21, 0x7, 0xc0,
    0x90, 0x44, 0x21, 0x30, 0xf0,

    /* U+0430 "а" */
    0x79, 0x8, 0x11, 0xec, 0x50, 0xa3, 0x3b,

    /* U+0431 "б" */
    0x8, 0xc4, 0x20, 0xb3, 0x28, 0x61, 0x85, 0x23,
    0x0,

    /* U+0432 "в" */
    0xf8, 0x89, 0x13, 0xc4, 0x48, 0x51, 0x7c,

    /* U+0433 "г" */
    0xf8, 0x89, 0xa, 0x4, 0x8, 0x10, 0x78,

    /* U+0434 "д" */
    0x7f, 0x9, 0x4, 0x82, 0x41, 0x21, 0x10, 0xfc,
    0x81, 0x80, 0x80,

    /* U+0435 "е" */
    0x38, 0x8a, 0xf, 0xf8, 0x10, 0x10, 0x9e,

    /* U+0436 "ж" */
    0x9c, 0xc4, 0x5a, 0xc7, 0xc4, 0x92, 0x49, 0x25,
    0xbb,

    /* U+0437 "з" */
    0x7d, 0x4, 0x8, 0xe0, 0x30, 0x70, 0x9e,

    /* U+0438 "и" */
    0xe7, 0x46, 0x4a, 0x4a, 0x52, 0x52, 0x62, 0xe7,

    /* U+0439 "й" */
    0x24, 0x18, 0x0, 0xe7, 0x46, 0x4a, 0x4a, 0x52,
    0x52, 0x62, 0xe7,

    /* U+043A "к" */
    0xe6, 0x49, 0x48, 0x50, 0x68, 0x44, 0x42, 0xe7,

    /* U+043B "л" */
    0x7f, 0x12, 0x12, 0x12, 0x12, 0x92, 0x92, 0x67,

    /* U+043C "м" */
    0xe3, 0xb1, 0x95, 0x4a, 0xa5, 0x52, 0x49, 0x25,
    0xc7,

    /* U+043D "н" */
    0xe7, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42, 0xe7,

    /* U+043E "о" */
    0x3c, 0x42, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3c,

    /* U+043F "п" */
    0xff, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0xe7,

    /* U+0440 "р" */
    0xdc, 0x62, 0x41, 0x41, 0x41, 0x62, 0x5c, 0x40,
    0xe0,

    /* U+0441 "с" */
    0x3c, 0x86, 0xc, 0x8, 0x10, 0x10, 0x9e,

    /* U+0442 "т" */
    0x7f, 0x24, 0xa2, 0x21, 0x0, 0x80, 0x40, 0x20,
    0x38,

    /* U+0443 "у" */
    0xee, 0x89, 0x11, 0x42, 0x82, 0x24, 0x48, 0x60,

    /* U+0444 "ф" */
    0x18, 0x4, 0x1a, 0xd3, 0x98, 0x8c, 0x46, 0x72,
    0xd6, 0x8, 0xe, 0x0,

    /* U+0445 "х" */
    0xee, 0x88, 0xa0, 0x81, 0x5, 0x11, 0x77,

    /* U+0446 "ц" */
    0xee, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0xfe,
    0x1,

    /* U+0447 "ч" */
    0xee, 0x89, 0x12, 0x24, 0xc6, 0x81, 0x7,

    /* U+0448 "ш" */
    0xdd, 0xa4, 0x92, 0x49, 0x24, 0x92, 0x49, 0x25,
    0xff,

    /* U+0449 "щ" */
    0xdd, 0x92, 0x44, 0x91, 0x24, 0x49, 0x12, 0x44,
    0x93, 0xfe, 0x0, 0x40,

    /* U+044A "ъ" */
    0x78, 0x90, 0x90, 0x1e, 0x11, 0x11, 0x11, 0x7e,

    /* U+044B "ы" */
    0xe3, 0xa0, 0x90, 0x4f, 0x24, 0x52, 0x29, 0x15,
    0xf7,

    /* U+044C "ь" */
    0xe1, 0x4, 0x1e, 0x45, 0x14, 0x7e,

    /* U+044D "э" */
    0x79, 0x8, 0x9, 0x94, 0xe0, 0x61, 0x3c,

    /* U+044E "ю" */
    0xee, 0x51, 0x51, 0x71, 0x51, 0x51, 0x51, 0xee,

    /* U+044F "я" */
    0x7f, 0xa, 0x13, 0xe1, 0x44, 0x89, 0x67,

    /* U+221E "∞" */
    0x31, 0x89, 0x4a, 0x10, 0xc2, 0x14, 0xa4, 0x63,
    0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 64, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 64, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 3, .adv_w = 96, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 5, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 12, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 21, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 29, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 36, .adv_w = 64, .box_w = 1, .box_h = 4, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 37, .adv_w = 96, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 42, .adv_w = 96, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 47, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 52, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 59, .adv_w = 64, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 60, .adv_w = 112, .box_w = 6, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 61, .adv_w = 64, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 62, .adv_w = 128, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 69, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 75, .adv_w = 96, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 79, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 85, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 91, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 98, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 104, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 110, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 116, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 122, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 128, .adv_w = 64, .box_w = 1, .box_h = 6, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 129, .adv_w = 64, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 131, .adv_w = 128, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 137, .adv_w = 112, .box_w = 6, .box_h = 3, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 140, .adv_w = 128, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 146, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 153, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 161, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 169, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 176, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 183, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 191, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 198, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 205, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 213, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 221, .adv_w = 64, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 223, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 230, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 237, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 244, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 255, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 263, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 271, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 278, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 288, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 295, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 302, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 310, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 318, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 326, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 337, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 344, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 352, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 359, .adv_w = 96, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 364, .adv_w = 128, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 371, .adv_w = 96, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 376, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 378, .adv_w = 112, .box_w = 6, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 379, .adv_w = 64, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 380, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 384, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 390, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 394, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 400, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 404, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 410, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 416, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 422, .adv_w = 64, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 424, .adv_w = 64, .box_w = 4, .box_h = 12, .ofs_x = -2, .ofs_y = -4},
    {.bitmap_index = 430, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 436, .adv_w = 64, .box_w = 1, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 438, .adv_w = 128, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 444, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 448, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 452, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 458, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 464, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 467, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 471, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 476, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 480, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 484, .adv_w = 160, .box_w = 9, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 491, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 495, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 501, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 505, .adv_w = 128, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 512, .adv_w = 96, .box_w = 1, .box_h = 11, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 514, .adv_w = 128, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 521, .adv_w = 112, .box_w = 6, .box_h = 2, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 523, .adv_w = 208, .box_w = 11, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 537, .adv_w = 208, .box_w = 9, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 549, .adv_w = 208, .box_w = 9, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 561, .adv_w = 208, .box_w = 8, .box_h = 10, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 571, .adv_w = 208, .box_w = 10, .box_h = 11, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 585, .adv_w = 208, .box_w = 9, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 597, .adv_w = 208, .box_w = 11, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 611, .adv_w = 208, .box_w = 8, .box_h = 10, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 621, .adv_w = 208, .box_w = 10, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 634, .adv_w = 208, .box_w = 10, .box_h = 11, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 648, .adv_w = 208, .box_w = 10, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 661, .adv_w = 208, .box_w = 10, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 674, .adv_w = 208, .box_w = 11, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 688, .adv_w = 208, .box_w = 10, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 701, .adv_w = 208, .box_w = 9, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 713, .adv_w = 208, .box_w = 10, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 726, .adv_w = 208, .box_w = 9, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 738, .adv_w = 208, .box_w = 8, .box_h = 10, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 748, .adv_w = 208, .box_w = 11, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 762, .adv_w = 208, .box_w = 10, .box_h = 11, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 776, .adv_w = 208, .box_w = 9, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 788, .adv_w = 208, .box_w = 9, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 800, .adv_w = 208, .box_w = 10, .box_h = 11, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 814, .adv_w = 208, .box_w = 10, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 827, .adv_w = 208, .box_w = 11, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 841, .adv_w = 208, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 858, .adv_w = 208, .box_w = 10, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 871, .adv_w = 208, .box_w = 11, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 885, .adv_w = 208, .box_w = 9, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 897, .adv_w = 208, .box_w = 8, .box_h = 10, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 907, .adv_w = 208, .box_w = 11, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 921, .adv_w = 208, .box_w = 10, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 934, .adv_w = 208, .box_w = 7, .box_h = 8, .ofs_x = 4, .ofs_y = -1},
    {.bitmap_index = 941, .adv_w = 208, .box_w = 6, .box_h = 11, .ofs_x = 4, .ofs_y = -1},
    {.bitmap_index = 950, .adv_w = 208, .box_w = 7, .box_h = 8, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 957, .adv_w = 208, .box_w = 7, .box_h = 8, .ofs_x = 4, .ofs_y = -1},
    {.bitmap_index = 964, .adv_w = 208, .box_w = 9, .box_h = 9, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 975, .adv_w = 208, .box_w = 7, .box_h = 8, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 982, .adv_w = 208, .box_w = 9, .box_h = 8, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 991, .adv_w = 208, .box_w = 7, .box_h = 8, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 998, .adv_w = 208, .box_w = 8, .box_h = 8, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 1006, .adv_w = 208, .box_w = 8, .box_h = 11, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 1017, .adv_w = 208, .box_w = 8, .box_h = 8, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 1025, .adv_w = 208, .box_w = 8, .box_h = 8, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 1033, .adv_w = 208, .box_w = 9, .box_h = 8, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 1042, .adv_w = 208, .box_w = 8, .box_h = 8, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 1050, .adv_w = 208, .box_w = 8, .box_h = 8, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 1058, .adv_w = 208, .box_w = 8, .box_h = 8, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 1066, .adv_w = 208, .box_w = 8, .box_h = 9, .ofs_x = 3, .ofs_y = -2},
    {.bitmap_index = 1075, .adv_w = 208, .box_w = 7, .box_h = 8, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 1082, .adv_w = 208, .box_w = 9, .box_h = 8, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 1091, .adv_w = 208, .box_w = 7, .box_h = 9, .ofs_x = 4, .ofs_y = -2},
    {.bitmap_index = 1099, .adv_w = 208, .box_w = 9, .box_h = 10, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 1111, .adv_w = 208, .box_w = 7, .box_h = 8, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 1118, .adv_w = 208, .box_w = 8, .box_h = 9, .ofs_x = 3, .ofs_y = -2},
    {.bitmap_index = 1127, .adv_w = 208, .box_w = 7, .box_h = 8, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 1134, .adv_w = 208, .box_w = 9, .box_h = 8, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 1143, .adv_w = 208, .box_w = 10, .box_h = 9, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 1155, .adv_w = 208, .box_w = 8, .box_h = 8, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 1163, .adv_w = 208, .box_w = 9, .box_h = 8, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 1172, .adv_w = 208, .box_w = 6, .box_h = 8, .ofs_x = 4, .ofs_y = -1},
    {.bitmap_index = 1178, .adv_w = 208, .box_w = 7, .box_h = 8, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 1185, .adv_w = 208, .box_w = 8, .box_h = 8, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 1193, .adv_w = 208, .box_w = 7, .box_h = 8, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 1200, .adv_w = 208, .box_w = 11, .box_h = 6, .ofs_x = 1, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 1040, .range_length = 64, .glyph_id_start = 96,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 8734, .range_length = 1, .glyph_id_start = 160,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Map glyph_ids to kern left classes*/
static const uint8_t kern_left_class_mapping[] =
{
    0, 0, 0, 1, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 2, 0, 2,
    1, 3, 4, 3, 3, 3, 3, 3,
    3, 3, 3, 5, 5, 1, 0, 0,
    0, 0, 6, 0, 0, 7, 0, 8,
    0, 0, 7, 0, 0, 9, 0, 0,
    7, 10, 7, 0, 0, 11, 0, 12,
    12, 0, 12, 0, 0, 1, 0, 0,
    13, 0, 14, 14, 14, 15, 14, 15,
    13, 14, 16, 16, 14, 16, 14, 14,
    14, 14, 13, 17, 18, 16, 14, 14,
    14, 14, 13, 14, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0
};

/*Map glyph_ids to kern right classes*/
static const uint8_t kern_right_class_mapping[] =
{
    0, 0, 1, 2, 1, 0, 1, 1,
    2, 0, 3, 1, 1, 4, 1, 4,
    0, 5, 5, 5, 5, 0, 5, 5,
    5, 5, 5, 6, 6, 0, 1, 0,
    1, 1, 7, 3, 8, 3, 3, 3,
    8, 3, 8, 9, 3, 3, 3, 3,
    8, 3, 8, 3, 3, 10, 11, 12,
    12, 3, 12, 3, 0, 0, 3, 1,
    13, 0, 14, 3, 15, 15, 15, 16,
    15, 3, 16, 17, 3, 16, 15, 15,
    15, 15, 15, 15, 15, 16, 15, 15,
    15, 15, 15, 15, 0, 0, 3, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0
};

/*Kern values between classes*/
static const int8_t kern_class_values[] =
{
    0, 0, 0, 0, -16, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -16, 0, 0,
    0, 0, -32, 0, 0, 0, 0, 0,
    0, 0, 16, 0, 16, 0, 0, 0,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 0, 0, 16, 0, 0,
    0, 16, 16, 16, 16, 16, 16, 0,
    16, 16, 16, 16, 0, 0, 0, 0,
    -16, 0, 0, 0, 0, -16, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -16, 0,
    -16, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -16,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -16, 0, 0, -16, -16, -16,
    0, 0, 0, -16, -16, -16, -16, -32,
    0, -16, 0, 0, 0, 0, 0, -16,
    0, -32, -16, -16, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -16,
    0, -16, 0, 0, 0, -16, 0, 0,
    0, 0, 0, 0, 0, -32, 0, -16,
    -16, -16, -32, 0, 0, 0, -32, -32,
    -32, -16, -32, 0, 0, 0, 0, 0,
    0, -16, 0, -16, 0, 0, 0, 0,
    -16, 0, 0, -16, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -32, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -32, 0,
    0, 0, 0, 0, 0, -16, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -16, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -16, 0, 0, 0, 0, 0, 0, -16,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -32, 0, 0, 0, -16, 0, 0,
    -16, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -32, 0, -16, 0, 0, 0,
    0, -16
};


/*Collect the kern class' data in one place*/
static const lv_font_fmt_txt_kern_classes_t kern_classes =
{
    .class_pair_values   = kern_class_values,
    .left_class_mapping  = kern_left_class_mapping,
    .right_class_mapping = kern_right_class_mapping,
    .left_class_cnt      = 18,
    .right_class_cnt     = 17,
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
    .kern_dsc = &kern_classes,
    .kern_scale = 16,
    .cmap_num = 3,
    .bpp = 1,
    .kern_classes = 1,
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
const lv_font_t lv_font_cubic_12 = {
#else
lv_font_t lv_font_cubic_12 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 14,          /*The maximum line height required by the font*/
    .base_line = 4,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if LV_FONT_CUBIC_12*/

