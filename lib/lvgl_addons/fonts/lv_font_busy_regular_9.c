/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --font busy_regular_9px.ttf -o ../../../../lib/lvgl_addons/fonts/lv_font_busy_regular_9.c --bpp 1 --size 16 --no-compress --format lvgl --range 0-65535
 ******************************************************************************/

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif



#ifndef LV_FONT_BUSY_REGULAR_9
#define LV_FONT_BUSY_REGULAR_9 1
#endif

#if LV_FONT_BUSY_REGULAR_9

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x00,

    /* U+0021 "!" */
    0xfc, 0x80,

    /* U+0022 "\"" */
    0xb4,

    /* U+0023 "#" */
    0x24, 0x24, 0xff, 0x24, 0x24, 0x24, 0xff, 0x24,
    0x24,

    /* U+0024 "$" */
    0x10, 0xfa, 0x4c, 0x87, 0xc2, 0x64, 0xbe, 0x10,

    /* U+0025 "%" */
    0x61, 0x48, 0xa4, 0x8c, 0x80, 0x80, 0x98, 0x92,
    0x89, 0x43, 0x00,

    /* U+0026 "&" */
    0x30, 0x48, 0x48, 0x27, 0x32, 0x4a, 0x84, 0x8a,
    0x71,

    /* U+0027 "'" */
    0xc0,

    /* U+0028 "(" */
    0x29, 0x49, 0x12, 0x20,

    /* U+0029 ")" */
    0x89, 0x12, 0x52, 0x80,

    /* U+002A "*" */
    0x25, 0x5d, 0x52, 0x00,

    /* U+002B "+" */
    0x21, 0x3e, 0x42, 0x00,

    /* U+002C "," */
    0x58,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x08, 0x84, 0x42, 0x21, 0x10, 0x80,

    /* U+0030 "0" */
    0x7a, 0x18, 0x61, 0x86, 0x18, 0x61, 0x78,

    /* U+0031 "1" */
    0x13, 0x59, 0x11, 0x11, 0x10,

    /* U+0032 "2" */
    0x7a, 0x10, 0x42, 0x10, 0x84, 0x20, 0xfc,

    /* U+0033 "3" */
    0xfc, 0x10, 0x84, 0x38, 0x10, 0x61, 0x78,

    /* U+0034 "4" */
    0x18, 0xa2, 0x92, 0x4a, 0x28, 0xbf, 0x08,

    /* U+0035 "5" */
    0xfe, 0x08, 0x20, 0x78, 0x10, 0x61, 0x78,

    /* U+0036 "6" */
    0x7a, 0x18, 0x20, 0xfa, 0x18, 0x61, 0x78,

    /* U+0037 "7" */
    0xfc, 0x10, 0x84, 0x10, 0x82, 0x08, 0x20,

    /* U+0038 "8" */
    0x7a, 0x18, 0x52, 0x31, 0x28, 0x61, 0x78,

    /* U+0039 "9" */
    0x7a, 0x18, 0x61, 0x7c, 0x10, 0x61, 0x78,

    /* U+003A ":" */
    0x88,

    /* U+003B ";" */
    0x40, 0x58,

    /* U+003C "<" */
    0x12, 0x48, 0x42, 0x10,

    /* U+003D "=" */
    0xf8, 0x3e,

    /* U+003E ">" */
    0x84, 0x21, 0x24, 0x80,

    /* U+003F "?" */
    0x74, 0x62, 0x22, 0x10, 0x00, 0x20,

    /* U+0040 "@" */
    0x3e, 0x20, 0xa7, 0x34, 0x9a, 0x4d, 0x26, 0x7c,
    0x80, 0x3e, 0x00,

    /* U+0041 "A" */
    0x10, 0x20, 0xa1, 0x44, 0x48, 0x9f, 0x41, 0x82,

    /* U+0042 "B" */
    0xfa, 0x18, 0x61, 0xfa, 0x18, 0x61, 0xf8,

    /* U+0043 "C" */
    0x39, 0x18, 0x20, 0x82, 0x08, 0x11, 0x38,

    /* U+0044 "D" */
    0xf9, 0x0a, 0x0c, 0x18, 0x30, 0x60, 0xc2, 0xf8,

    /* U+0045 "E" */
    0xfe, 0x08, 0x20, 0xfa, 0x08, 0x20, 0xfc,

    /* U+0046 "F" */
    0xfe, 0x08, 0x20, 0xfa, 0x08, 0x20, 0x80,

    /* U+0047 "G" */
    0x3c, 0x86, 0x04, 0x08, 0xf0, 0x60, 0xa3, 0x3a,

    /* U+0048 "H" */
    0x83, 0x06, 0x0c, 0x1f, 0xf0, 0x60, 0xc1, 0x82,

    /* U+0049 "I" */
    0xe9, 0x24, 0x92, 0xe0,

    /* U+004A "J" */
    0x04, 0x10, 0x41, 0x04, 0x18, 0x61, 0x78,

    /* U+004B "K" */
    0x86, 0x29, 0x28, 0xc2, 0x89, 0x22, 0x84,

    /* U+004C "L" */
    0x82, 0x08, 0x20, 0x82, 0x08, 0x20, 0xfc,

    /* U+004D "M" */
    0x80, 0xe0, 0xe8, 0xb4, 0x59, 0x4c, 0xa6, 0x23,
    0x11, 0x80, 0x80,

    /* U+004E "N" */
    0x83, 0x07, 0x0d, 0x19, 0x31, 0x61, 0xc1, 0x82,

    /* U+004F "O" */
    0x38, 0x8a, 0x0c, 0x18, 0x30, 0x60, 0xa2, 0x38,

    /* U+0050 "P" */
    0xfa, 0x18, 0x61, 0xfa, 0x08, 0x20, 0x80,

    /* U+0051 "Q" */
    0x38, 0x8a, 0x0c, 0x18, 0x32, 0x62, 0xa2, 0x3a,

    /* U+0052 "R" */
    0xfa, 0x18, 0x61, 0xfa, 0x48, 0xa1, 0x84,

    /* U+0053 "S" */
    0x7a, 0x18, 0x20, 0x78, 0x10, 0x61, 0x78,

    /* U+0054 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x02, 0x04, 0x08, 0x10,

    /* U+0055 "U" */
    0x83, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xa2, 0x38,

    /* U+0056 "V" */
    0x83, 0x05, 0x12, 0x24, 0x45, 0x0a, 0x08, 0x10,

    /* U+0057 "W" */
    0x88, 0xc4, 0x62, 0x29, 0x25, 0x52, 0xa8, 0x88,
    0x44, 0x22, 0x00,

    /* U+0058 "X" */
    0x82, 0x89, 0x11, 0x41, 0x05, 0x11, 0x22, 0x82,

    /* U+0059 "Y" */
    0x83, 0x05, 0x12, 0x22, 0x82, 0x04, 0x08, 0x10,

    /* U+005A "Z" */
    0xfc, 0x10, 0x84, 0x21, 0x08, 0x20, 0xfc,

    /* U+005B "[" */
    0xea, 0xaa, 0xc0,

    /* U+005C "\\" */
    0x82, 0x10, 0x42, 0x08, 0x41, 0x08,

    /* U+005D "]" */
    0xd5, 0x55, 0xc0,

    /* U+005E "^" */
    0x22, 0xa2,

    /* U+005F "_" */
    0xf8,

    /* U+0061 "a" */
    0x70, 0x5f, 0x18, 0xbc,

    /* U+0062 "b" */
    0x84, 0x21, 0xe8, 0xc6, 0x31, 0xf0,

    /* U+0063 "c" */
    0x74, 0x61, 0x08, 0xb8,

    /* U+0064 "d" */
    0x08, 0x42, 0xf8, 0xc6, 0x31, 0x78,

    /* U+0065 "e" */
    0x74, 0x7f, 0x08, 0xb8,

    /* U+0066 "f" */
    0x34, 0x4f, 0x44, 0x44, 0x40,

    /* U+0067 "g" */
    0x7c, 0x63, 0x18, 0xbc, 0x2e,

    /* U+0068 "h" */
    0x84, 0x21, 0x6c, 0xc6, 0x31, 0x88,

    /* U+0069 "i" */
    0x40, 0x64, 0x92, 0xe0,

    /* U+006A "j" */
    0x20, 0x72, 0x49, 0x27, 0x00,

    /* U+006B "k" */
    0x84, 0x21, 0x2a, 0x62, 0x92, 0x88,

    /* U+006C "l" */
    0xc9, 0x24, 0x92, 0x60,

    /* U+006D "m" */
    0xfd, 0x26, 0x4c, 0x99, 0x32, 0x40,

    /* U+006E "n" */
    0xb6, 0x63, 0x18, 0xc4,

    /* U+006F "o" */
    0x74, 0x63, 0x18, 0xb8,

    /* U+0070 "p" */
    0xb6, 0x63, 0x1c, 0xda, 0x10,

    /* U+0071 "q" */
    0x6c, 0xe3, 0x19, 0xb4, 0x21,

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
    0x88, 0xc4, 0x55, 0x4a, 0xa2, 0x21, 0x10,

    /* U+0078 "x" */
    0x8a, 0x88, 0x45, 0x44,

    /* U+0079 "y" */
    0x8c, 0x62, 0xa5, 0x10, 0x98,

    /* U+007A "z" */
    0xf8, 0x88, 0x88, 0x7c,

    /* U+007B "{" */
    0x29, 0x28, 0x92, 0x20,

    /* U+007C "|" */
    0xff, 0x80,

    /* U+007D "}" */
    0x89, 0x22, 0x92, 0x80,

    /* U+007E "~" */
    0x61, 0x24, 0x30,

    /* U+00A0 " " */
    0x00,

    /* U+00A1 "¡" */
    0x9f, 0x80,

    /* U+00A3 "£" */
    0x39, 0x14, 0x10, 0x43, 0xe4, 0x10, 0xbc,

    /* U+00A5 "¥" */
    0x8c, 0x62, 0xa5, 0x13, 0xe4, 0x20,

    /* U+00A6 "¦" */
    0xf7, 0x80,

    /* U+00A7 "§" */
    0x74, 0x60, 0xe8, 0xb8, 0x31, 0x70,

    /* U+00A9 "©" */
    0x3e, 0x20, 0xa7, 0x34, 0x5a, 0x0d, 0x16, 0x72,
    0x82, 0x3e, 0x00,

    /* U+00AB "«" */
    0x12, 0x49, 0x24, 0x84, 0x84, 0x84, 0x80,

    /* U+00AE "®" */
    0x3e, 0x20, 0xaf, 0x34, 0x5b, 0xcd, 0x16, 0x8a,
    0x82, 0x3e, 0x00,

    /* U+00B0 "°" */
    0x69, 0x96,

    /* U+00B1 "±" */
    0x21, 0x3e, 0x42, 0x03, 0xe0,

    /* U+00B2 "²" */
    0xe7, 0xce,

    /* U+00B3 "³" */
    0xe7, 0x9e,

    /* U+00B6 "¶" */
    0x1f, 0x31, 0x71, 0xf1, 0x71, 0x31, 0x11, 0x11,
    0x11,

    /* U+00B7 "·" */
    0x80,

    /* U+00B9 "¹" */
    0x59, 0x2e,

    /* U+00BB "»" */
    0x90, 0x90, 0x90, 0x92, 0x49, 0x24, 0x00,

    /* U+00BF "¿" */
    0x20, 0x00, 0x42, 0x22, 0x31, 0x70,

    /* U+00D7 "×" */
    0x8a, 0x88, 0xa8, 0x80,

    /* U+00F7 "÷" */
    0x20, 0x3e, 0x02, 0x00,

    /* U+0401 "Ё" */
    0x84, 0x0f, 0xe0, 0x82, 0x0f, 0xa0, 0x82, 0x0f,
    0xc0,

    /* U+0410 "А" */
    0x10, 0x20, 0xa1, 0x44, 0x48, 0x9f, 0x41, 0x82,

    /* U+0411 "Б" */
    0xfe, 0x08, 0x20, 0xfa, 0x18, 0x61, 0xf8,

    /* U+0412 "В" */
    0xfa, 0x18, 0x61, 0xfa, 0x18, 0x61, 0xf8,

    /* U+0413 "Г" */
    0xfc, 0x21, 0x08, 0x42, 0x10, 0x80,

    /* U+0414 "Д" */
    0x3c, 0x89, 0x12, 0x24, 0x48, 0x91, 0x22, 0xff,
    0x04,

    /* U+0415 "Е" */
    0xfe, 0x08, 0x20, 0xfa, 0x08, 0x20, 0xfc,

    /* U+0416 "Ж" */
    0x88, 0xa4, 0x92, 0x45, 0x41, 0xc1, 0x51, 0x25,
    0x11, 0x88, 0x80,

    /* U+0417 "З" */
    0x7a, 0x10, 0x41, 0x38, 0x10, 0x61, 0x78,

    /* U+0418 "И" */
    0x83, 0x06, 0x1c, 0x59, 0x34, 0x70, 0xc1, 0x82,

    /* U+0419 "Й" */
    0x44, 0x72, 0x0c, 0x18, 0x71, 0x64, 0xd1, 0xc3,
    0x06, 0x08,

    /* U+041A "К" */
    0x86, 0x29, 0x28, 0xc2, 0x89, 0x22, 0x84,

    /* U+041B "Л" */
    0x3d, 0x14, 0x51, 0x45, 0x14, 0x51, 0x84,

    /* U+041C "М" */
    0x80, 0xe0, 0xe8, 0xb4, 0x59, 0x4c, 0xa6, 0x23,
    0x11, 0x80, 0x80,

    /* U+041D "Н" */
    0x83, 0x06, 0x0c, 0x1f, 0xf0, 0x60, 0xc1, 0x82,

    /* U+041E "О" */
    0x38, 0x8a, 0x0c, 0x18, 0x30, 0x60, 0xa2, 0x38,

    /* U+041F "П" */
    0xfe, 0x18, 0x61, 0x86, 0x18, 0x61, 0x84,

    /* U+0420 "Р" */
    0xf2, 0x28, 0x61, 0x8b, 0xc8, 0x20, 0x80,

    /* U+0421 "С" */
    0x39, 0x18, 0x20, 0x82, 0x08, 0x11, 0x38,

    /* U+0422 "Т" */
    0xfe, 0x20, 0x40, 0x81, 0x02, 0x04, 0x08, 0x10,

    /* U+0423 "У" */
    0x83, 0x05, 0x12, 0x22, 0x85, 0x04, 0x08, 0x60,

    /* U+0424 "Ф" */
    0x08, 0x1f, 0x12, 0x51, 0x18, 0x8c, 0x45, 0x24,
    0x7c, 0x08, 0x00,

    /* U+0425 "Х" */
    0x82, 0x89, 0x11, 0x41, 0x05, 0x11, 0x22, 0x82,

    /* U+0426 "Ц" */
    0x85, 0x0a, 0x14, 0x28, 0x50, 0xa1, 0x42, 0xfe,
    0x04,

    /* U+0427 "Ч" */
    0x86, 0x18, 0x63, 0x74, 0x10, 0x41, 0x04,

    /* U+0428 "Ш" */
    0x88, 0xc4, 0x62, 0x31, 0x18, 0x8c, 0x46, 0x23,
    0x11, 0xff, 0x80,

    /* U+0429 "Щ" */
    0x88, 0xa2, 0x28, 0x8a, 0x22, 0x88, 0xa2, 0x28,
    0x8a, 0x22, 0xff, 0xc0, 0x10,

    /* U+042A "Ъ" */
    0xc0, 0x81, 0x03, 0xc4, 0x48, 0x50, 0xa2, 0x78,

    /* U+042B "Ы" */
    0x81, 0x81, 0x81, 0xf1, 0x89, 0x85, 0x85, 0x89,
    0xf1,

    /* U+042C "Ь" */
    0x82, 0x08, 0x3c, 0x8a, 0x18, 0x62, 0xf0,

    /* U+042D "Э" */
    0x72, 0x20, 0x41, 0x3c, 0x10, 0x62, 0x70,

    /* U+042E "Ю" */
    0x87, 0x22, 0x29, 0x06, 0x41, 0xf0, 0x64, 0x19,
    0x06, 0x22, 0x87, 0x00,

    /* U+042F "Я" */
    0x7e, 0x18, 0x61, 0x7c, 0x94, 0x61, 0x84,

    /* U+0430 "а" */
    0x70, 0x5f, 0x18, 0xbc,

    /* U+0431 "б" */
    0x0b, 0xa1, 0xe8, 0xc6, 0x31, 0x70,

    /* U+0432 "в" */
    0xf4, 0x7d, 0x18, 0xf8,

    /* U+0433 "г" */
    0xf8, 0x88, 0x88,

    /* U+0434 "д" */
    0x79, 0x24, 0x92, 0x8b, 0xf8, 0x40,

    /* U+0435 "е" */
    0x74, 0x7f, 0x08, 0xb8,

    /* U+0436 "ж" */
    0x92, 0xa8, 0xe2, 0xa9, 0x32, 0x40,

    /* U+0437 "з" */
    0x74, 0x4c, 0x18, 0xb8,

    /* U+0438 "и" */
    0x8c, 0xeb, 0x98, 0xc4,

    /* U+0439 "й" */
    0x8b, 0x81, 0x19, 0xd7, 0x31, 0x88,

    /* U+043A "к" */
    0x8d, 0xb1, 0x49, 0x44,

    /* U+043B "л" */
    0x7a, 0x52, 0x94, 0xc4,

    /* U+043C "м" */
    0x83, 0x8e, 0xad, 0x59, 0x32, 0x40,

    /* U+043D "н" */
    0x8c, 0x7f, 0x18, 0xc4,

    /* U+043E "о" */
    0x74, 0x63, 0x18, 0xb8,

    /* U+043F "п" */
    0xfc, 0x63, 0x18, 0xc4,

    /* U+0440 "р" */
    0xb6, 0x63, 0x1c, 0xda, 0x10,

    /* U+0441 "с" */
    0x74, 0x61, 0x08, 0xb8,

    /* U+0442 "т" */
    0xf9, 0x08, 0x42, 0x10,

    /* U+0443 "у" */
    0x8c, 0x54, 0xa2, 0x13, 0x00,

    /* U+0444 "ф" */
    0x10, 0x21, 0xf4, 0x99, 0x32, 0x64, 0xbe, 0x10,
    0x20,

    /* U+0445 "х" */
    0x8a, 0x88, 0x45, 0x44,

    /* U+0446 "ц" */
    0x8a, 0x28, 0xa2, 0x8b, 0xf0, 0x40,

    /* U+0447 "ч" */
    0x8c, 0x66, 0xd0, 0x84,

    /* U+0448 "ш" */
    0x93, 0x26, 0x4c, 0x99, 0x3f, 0xc0,

    /* U+0449 "щ" */
    0x92, 0x92, 0x92, 0x92, 0x92, 0xff, 0x01,

    /* U+044A "ъ" */
    0xc2, 0x1c, 0x94, 0xb8,

    /* U+044B "ы" */
    0x86, 0x1e, 0x65, 0x97, 0x90,

    /* U+044C "ь" */
    0x88, 0xe9, 0x9e,

    /* U+044D "э" */
    0x74, 0x4e, 0x18, 0xb8,

    /* U+044E "ю" */
    0x9d, 0x47, 0x8d, 0x1a, 0x33, 0x80,

    /* U+044F "я" */
    0x7c, 0x62, 0xf8, 0xc4,

    /* U+0451 "ё" */
    0x50, 0x1d, 0x1f, 0xc2, 0x2e,

    /* U+200A " " */
    0x00,

    /* U+2013 "–" */
    0xf0,

    /* U+2014 "—" */
    0xf8,

    /* U+2018 "‘" */
    0xc0,

    /* U+2019 "’" */
    0xc0,

    /* U+201A "‚" */
    0x60,

    /* U+201C "“" */
    0xb4,

    /* U+201D "”" */
    0xb4,

    /* U+201E "„" */
    0xb4,

    /* U+2020 "†" */
    0x5d, 0x24,

    /* U+2021 "‡" */
    0x5d, 0x74,

    /* U+2022 "•" */
    0xf0,

    /* U+2026 "…" */
    0xa8,

    /* U+2039 "‹" */
    0x12, 0x48, 0x42, 0x10,

    /* U+203A "›" */
    0x84, 0x21, 0x24, 0x80,

    /* U+205F " " */
    0x00,

    /* U+2070 "⁰" */
    0xf6, 0xde,

    /* U+2074 "⁴" */
    0xb7, 0x92,

    /* U+2075 "⁵" */
    0xf3, 0x9e,

    /* U+2076 "⁶" */
    0xf3, 0xde,

    /* U+2077 "⁷" */
    0xe5, 0x24,

    /* U+2078 "⁸" */
    0xf7, 0xde,

    /* U+2079 "⁹" */
    0xf7, 0x9e,

    /* U+2080 "₀" */
    0xf6, 0xde,

    /* U+2081 "₁" */
    0x59, 0x2e,

    /* U+2082 "₂" */
    0xe7, 0xce,

    /* U+2083 "₃" */
    0xe7, 0x9e,

    /* U+2084 "₄" */
    0xb7, 0x92,

    /* U+2085 "₅" */
    0xf3, 0x9e,

    /* U+2086 "₆" */
    0xf3, 0xde,

    /* U+2087 "₇" */
    0xe5, 0x24,

    /* U+2088 "₈" */
    0xf7, 0xde,

    /* U+2089 "₉" */
    0xf7, 0x9e,

    /* U+20AC "€" */
    0x1c, 0x45, 0x07, 0xe4, 0x1f, 0x90, 0x11, 0x1c,

    /* U+20BD "₽" */
    0x79, 0x14, 0x51, 0x79, 0x0f, 0x90, 0x40,

    /* U+2122 "™" */
    0x08, 0xf7, 0xd3, 0xe9, 0x74, 0x88,

    /* U+2190 "←" */
    0x18, 0x1c, 0x1c, 0x0c, 0x0f, 0xfb, 0x80, 0xc0,
    0x00,

    /* U+2191 "↑" */
    0x08, 0x0c, 0x7e, 0xcb, 0xcb, 0x08, 0x08, 0x08,
    0x08, 0x08,

    /* U+2192 "→" */
    0x06, 0x03, 0x80, 0x40, 0x2f, 0xf8, 0x1c, 0x1c,
    0x0c,

    /* U+2193 "↓" */
    0x08, 0x08, 0x08, 0x08, 0x08, 0xc8, 0xea, 0x7e,
    0x3c, 0x08,

    /* U+2194 "↔" */
    0x03, 0x06, 0x71, 0x87, 0x30, 0x6f, 0xfe, 0xe1,
    0x04, 0xe0, 0x18,

    /* U+2195 "↕" */
    0x0c, 0x2e, 0xca, 0xc8, 0x08, 0x0b, 0x6b, 0x3e,
    0x3c, 0x08,

    /* U+2196 "↖" */
    0xfc, 0x78, 0x2e, 0x13, 0x88, 0xe4, 0x38, 0x0e,
    0x03,

    /* U+2197 "↗" */
    0x1f, 0x83, 0xc3, 0xa3, 0x93, 0x8b, 0x87, 0x81,
    0x80,

    /* U+2198 "↘" */
    0xc0, 0x70, 0x1c, 0x07, 0x11, 0xc8, 0x74, 0x1e,
    0x07, 0x1f, 0x80,

    /* U+2199 "↙" */
    0x01, 0x81, 0xc1, 0xd1, 0xc9, 0xc5, 0xc3, 0xc1,
    0xc0, 0xfc, 0x00,

    /* U+2211 "∑" */
    0xfd, 0xc1, 0xc1, 0xc3, 0x8e, 0x38, 0x60, 0xfe,

    /* U+2212 "−" */
    0xf8,

    /* U+221A "√" */
    0x0f, 0x1f, 0x18, 0x18, 0x18, 0x18, 0xd8, 0xf8,
    0x78, 0x30,

    /* U+221E "∞" */
    0x00, 0x70, 0xe3, 0xec, 0x79, 0xf1, 0xe3, 0xc7,
    0x8d, 0x91, 0x31, 0xc7, 0x86, 0x0c,

    /* U+2248 "≈" */
    0x70, 0xfb, 0xdf, 0x3e, 0xf8, 0xce, 0x06,

    /* U+2260 "≠" */
    0x08, 0x61, 0x1f, 0x13, 0xfe, 0x30,

    /* U+2264 "≤" */
    0x2f, 0x66, 0x47,

    /* U+2265 "≥" */
    0x99, 0xbd, 0x07,

    /* U+25B6 "▶" */
    0x8c, 0xef, 0xec, 0x80,

    /* U+25C0 "◀" */
    0x13, 0x7f, 0x73, 0x10,

    /* U+FF0F "／" */
    0x0c, 0x71, 0x8e, 0x31, 0xc6, 0x38, 0xc3, 0x00
};

/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 96, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 48, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 64, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 4, .adv_w = 144, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 13, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 21, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 32, .adv_w = 144, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 41, .adv_w = 32, .box_w = 1, .box_h = 2, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 42, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 46, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 50, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 54, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 58, .adv_w = 48, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 59, .adv_w = 64, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 60, .adv_w = 64, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 61, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 67, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 79, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 93, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 100, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 107, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 114, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 121, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 128, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 135, .adv_w = 32, .box_w = 1, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 136, .adv_w = 48, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 138, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 142, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 144, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 148, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 165, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 173, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 180, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 195, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 202, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 209, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 217, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 229, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 236, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 243, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 250, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 261, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 269, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 277, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 292, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 299, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 306, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 314, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 322, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 330, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 341, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 349, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 357, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 364, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 367, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 373, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 376, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 378, .adv_w = 96, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 379, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 383, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 389, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 393, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 399, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 403, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 408, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 413, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 419, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 423, .adv_w = 64, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 428, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 434, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 438, .adv_w = 128, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 444, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 448, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 452, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 457, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 462, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 465, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 469, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 474, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 478, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 482, .adv_w = 160, .box_w = 9, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 489, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 493, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 498, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 502, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 506, .adv_w = 32, .box_w = 1, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 508, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 512, .adv_w = 128, .box_w = 7, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 515, .adv_w = 96, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 516, .adv_w = 48, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 518, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 525, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 531, .adv_w = 32, .box_w = 1, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 533, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 539, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 550, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 557, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 568, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 570, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 575, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 577, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 579, .adv_w = 144, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 588, .adv_w = 32, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 589, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 591, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 598, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 604, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 608, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 612, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 621, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 629, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 636, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 643, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 649, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 658, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 665, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 676, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 683, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 691, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 701, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 708, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 715, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 726, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 734, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 742, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 749, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 756, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 763, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 771, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 779, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 790, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 798, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 807, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 814, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 825, .adv_w = 176, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 838, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 846, .adv_w = 144, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 855, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 862, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 869, .adv_w = 176, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 881, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 888, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 892, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 898, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 902, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 905, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 911, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 915, .adv_w = 128, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 921, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 925, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 929, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 935, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 939, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 943, .adv_w = 128, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 949, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 953, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 957, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 961, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 966, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 970, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 974, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 979, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 988, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 992, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 998, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1002, .adv_w = 128, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1008, .adv_w = 144, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1015, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1019, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1024, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1027, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1031, .adv_w = 128, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1037, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1041, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1046, .adv_w = 16, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1047, .adv_w = 80, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 1048, .adv_w = 96, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 1049, .adv_w = 32, .box_w = 1, .box_h = 2, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 1050, .adv_w = 32, .box_w = 1, .box_h = 2, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 1051, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1052, .adv_w = 64, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 1053, .adv_w = 64, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 1054, .adv_w = 64, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1055, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 1057, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 1059, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 1060, .adv_w = 96, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1061, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1065, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1069, .adv_w = 64, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1070, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 1072, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 1074, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 1076, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 1078, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 1080, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 1082, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 1084, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1086, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1088, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1090, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1092, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1094, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1096, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1098, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1100, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1102, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1104, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1112, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1119, .adv_w = 160, .box_w = 9, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 1125, .adv_w = 160, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1134, .adv_w = 128, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1144, .adv_w = 160, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1153, .adv_w = 128, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1163, .adv_w = 176, .box_w = 11, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1174, .adv_w = 128, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1184, .adv_w = 144, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1193, .adv_w = 144, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1202, .adv_w = 144, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1213, .adv_w = 144, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1224, .adv_w = 112, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1232, .adv_w = 96, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 1233, .adv_w = 128, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1243, .adv_w = 224, .box_w = 14, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1257, .adv_w = 128, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1264, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1270, .adv_w = 64, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1273, .adv_w = 64, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1276, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1280, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1284, .adv_w = 96, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_2[] = {
    0x00, 0x01, 0x03, 0x05, 0x06, 0x07, 0x09, 0x0b,
    0x0e, 0x10, 0x11, 0x12, 0x13, 0x16, 0x17, 0x19,
    0x1b, 0x1f, 0x37, 0x57, 0x361
};

static const uint16_t unicode_list_4[] = {
    0x00, 0x1bb9, 0x1bc2, 0x1bc3, 0x1bc7, 0x1bc8, 0x1bc9, 0x1bcb,
    0x1bcc, 0x1bcd, 0x1bcf, 0x1bd0, 0x1bd1, 0x1bd5, 0x1be8, 0x1be9,
    0x1c0e, 0x1c1f, 0x1c23, 0x1c24, 0x1c25, 0x1c26, 0x1c27, 0x1c28,
    0x1c2f, 0x1c30, 0x1c31, 0x1c32, 0x1c33, 0x1c34, 0x1c35, 0x1c36,
    0x1c37, 0x1c38, 0x1c5b, 0x1c6c, 0x1cd1, 0x1d3f, 0x1d40, 0x1d41,
    0x1d42, 0x1d43, 0x1d44, 0x1d45, 0x1d46, 0x1d47, 0x1d48, 0x1dc0,
    0x1dc1, 0x1dc9, 0x1dcd, 0x1df7, 0x1e0f, 0x1e13, 0x1e14, 0x2165,
    0x216f, 0xfabe
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
        .range_start = 160, .range_length = 866, .glyph_id_start = 95,
        .unicode_list = unicode_list_2, .glyph_id_ofs_list = NULL, .list_length = 21, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    },
    {
        .range_start = 1040, .range_length = 64, .glyph_id_start = 116,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 1105, .range_length = 64191, .glyph_id_start = 180,
        .unicode_list = unicode_list_4, .glyph_id_ofs_list = NULL, .list_length = 58, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
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
const lv_font_t lv_font_busy_regular_9 = {
#else
lv_font_t lv_font_busy_regular_9 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,         /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,         /*Function pointer to get glyph's bitmap*/
    .line_height = 13, /*The maximum line height required by the font*/
    .base_line = 2,                     /*Baseline measured from the bottom of the line*/
#if LV_VERSION_CHECK(9, 6, 0) || LVGL_VERSION_MAJOR >= 10
    .cap_height = 9,                       /*Cap height of the font*/
    .x_height = 6,                           /*x-height of the font*/
#endif
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



#endif /*#if LV_FONT_BUSY_REGULAR_9*/
