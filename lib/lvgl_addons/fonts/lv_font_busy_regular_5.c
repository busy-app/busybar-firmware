/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --font ..\..\assets\shared\fonts\ttf\busy_regular_5.ttf -o ..\..\lib\lvgl_addons\fonts\lv_font_busy_regular_5.c --bpp 1 --size 16 --no-compress --format lvgl --range 0-65535
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



#ifndef LV_FONT_BUSY_REGULAR_5
#define LV_FONT_BUSY_REGULAR_5 1
#endif

#if LV_FONT_BUSY_REGULAR_5

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x00,

    /* U+0021 "!" */
    0xe8,

    /* U+0022 "\"" */
    0xb4,

    /* U+0023 "#" */
    0x57, 0xd5, 0xf5, 0x00,

    /* U+0024 "$" */
    0x4f, 0x1c, 0x80,

    /* U+0025 "%" */
    0x92, 0x49,

    /* U+0026 "&" */
    0x4a, 0x4a, 0x50,

    /* U+0027 "'" */
    0xc0,

    /* U+0028 "(" */
    0x6a, 0x40,

    /* U+0029 ")" */
    0x95, 0x80,

    /* U+002A "*" */
    0x5d, 0x50,

    /* U+002B "+" */
    0x5d, 0x00,

    /* U+002C "," */
    0xc0,

    /* U+002D "-" */
    0xc0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x5a, 0x80,

    /* U+0030 "0" */
    0x56, 0xd4,

    /* U+0031 "1" */
    0x75, 0x40,

    /* U+0032 "2" */
    0xc5, 0x4e,

    /* U+0033 "3" */
    0xc5, 0x1c,

    /* U+0034 "4" */
    0x2e, 0xf2,

    /* U+0035 "5" */
    0xf3, 0x1c,

    /* U+0036 "6" */
    0x73, 0x54,

    /* U+0037 "7" */
    0xe5, 0x24,

    /* U+0038 "8" */
    0x55, 0x54,

    /* U+0039 "9" */
    0x55, 0x9c,

    /* U+003A ":" */
    0x90,

    /* U+003B ";" */
    0x41, 0x80,

    /* U+003C "<" */
    0x2a, 0x22,

    /* U+003D "=" */
    0xe3, 0x80,

    /* U+003E ">" */
    0x88, 0xa8,

    /* U+003F "?" */
    0xc5, 0x04,

    /* U+0040 "@" */
    0x79, 0xb8, 0x70,

    /* U+0041 "A" */
    0x69, 0xf9, 0x90,

    /* U+0042 "B" */
    0xe9, 0xe9, 0xe0,

    /* U+0043 "C" */
    0x69, 0x89, 0x60,

    /* U+0044 "D" */
    0xe9, 0x99, 0xe0,

    /* U+0045 "E" */
    0xf8, 0xe8, 0xf0,

    /* U+0046 "F" */
    0xf8, 0xe8, 0x80,

    /* U+0047 "G" */
    0x78, 0xb9, 0x70,

    /* U+0048 "H" */
    0x99, 0xf9, 0x90,

    /* U+0049 "I" */
    0xf8,

    /* U+004A "J" */
    0x24, 0xd4,

    /* U+004B "K" */
    0x9a, 0xca, 0x90,

    /* U+004C "L" */
    0x92, 0x4e,

    /* U+004D "M" */
    0x8e, 0xeb, 0x18, 0x80,

    /* U+004E "N" */
    0x9d, 0xb9, 0x90,

    /* U+004F "O" */
    0x69, 0x99, 0x60,

    /* U+0050 "P" */
    0xe9, 0xe8, 0x80,

    /* U+0051 "Q" */
    0x69, 0x9a, 0x50,

    /* U+0052 "R" */
    0xe9, 0xe9, 0x90,

    /* U+0053 "S" */
    0x78, 0x61, 0xe0,

    /* U+0054 "T" */
    0xe9, 0x24,

    /* U+0055 "U" */
    0x99, 0x99, 0x60,

    /* U+0056 "V" */
    0xb6, 0xa4,

    /* U+0057 "W" */
    0xad, 0x6a, 0xa5, 0x00,

    /* U+0058 "X" */
    0xb5, 0x5a,

    /* U+0059 "Y" */
    0xb5, 0x24,

    /* U+005A "Z" */
    0xe5, 0x4e,

    /* U+005B "[" */
    0xea, 0xc0,

    /* U+005C "\\" */
    0xa5, 0x40,

    /* U+005D "]" */
    0xd5, 0xc0,

    /* U+005E "^" */
    0x54,

    /* U+005F "_" */
    0xe0,

    /* U+0061 "a" */
    0x76, 0xb0,

    /* U+0062 "b" */
    0x9a, 0xdc,

    /* U+0063 "c" */
    0x72, 0x30,

    /* U+0064 "d" */
    0x2e, 0xd6,

    /* U+0065 "e" */
    0x5e, 0x30,

    /* U+0066 "f" */
    0x6e, 0x80,

    /* U+0067 "g" */
    0x76, 0xbc,

    /* U+0068 "h" */
    0x9a, 0xda,

    /* U+0069 "i" */
    0xb8,

    /* U+006A "j" */
    0x45, 0x58,

    /* U+006B "k" */
    0x97, 0x5a,

    /* U+006C "l" */
    0xf8,

    /* U+006D "m" */
    0xf5, 0x6b, 0x50,

    /* U+006E "n" */
    0xd6, 0xd0,

    /* U+006F "o" */
    0x56, 0xa0,

    /* U+0070 "p" */
    0xd6, 0xe8,

    /* U+0071 "q" */
    0x76, 0xb2,

    /* U+0072 "r" */
    0xea,

    /* U+0073 "s" */
    0x78, 0xe0,

    /* U+0074 "t" */
    0xba, 0x40,

    /* U+0075 "u" */
    0xb6, 0xb0,

    /* U+0076 "v" */
    0xb5, 0x20,

    /* U+0077 "w" */
    0xad, 0x54, 0xa0,

    /* U+0078 "x" */
    0xa9, 0x50,

    /* U+0079 "y" */
    0xb6, 0xa8,

    /* U+007A "z" */
    0xea, 0x70,

    /* U+007B "{" */
    0x6a, 0x26,

    /* U+007C "|" */
    0xf8,

    /* U+007D "}" */
    0xc8, 0xac,

    /* U+007E "~" */
    0x5a,

    /* U+00A0 " " */
    0x00,

    /* U+00A1 "¡" */
    0xb8,

    /* U+00A3 "£" */
    0x25, 0xe4, 0xf0,

    /* U+00A5 "¥" */
    0xb5, 0x74,

    /* U+00A6 "¦" */
    0xd8,

    /* U+00A7 "§" */
    0x6c, 0xb6, 0x40,

    /* U+00A9 "©" */
    0x38, 0x8a, 0x4d, 0x19, 0x28, 0x8e, 0x00,

    /* U+00AB "«" */
    0x4c, 0x92,

    /* U+00AE "®" */
    0x38, 0x8a, 0xed, 0x9a, 0xa8, 0x8e, 0x00,

    /* U+00B0 "°" */
    0x55, 0x00,

    /* U+00B1 "±" */
    0x5d, 0x0e,

    /* U+00B2 "²" */
    0xc5, 0x70,

    /* U+00B3 "³" */
    0xe8, 0xe0,

    /* U+00B6 "¶" */
    0x7f, 0x5a, 0x52, 0x80,

    /* U+00B7 "·" */
    0x80,

    /* U+00B9 "¹" */
    0x75,

    /* U+00BB "»" */
    0x92, 0x64,

    /* U+00BF "¿" */
    0x41, 0x46,

    /* U+00D7 "×" */
    0xaa, 0x80,

    /* U+00F7 "÷" */
    0x43, 0x84,

    /* U+0401 "Ё" */
    0x90, 0xf8, 0xe8, 0xf0,

    /* U+0410 "А" */
    0x69, 0xf9, 0x90,

    /* U+0411 "Б" */
    0xf8, 0xe9, 0xe0,

    /* U+0412 "В" */
    0xe9, 0xe9, 0xe0,

    /* U+0413 "Г" */
    0xf2, 0x48,

    /* U+0414 "Д" */
    0x72, 0x94, 0xaf, 0xc4,

    /* U+0415 "Е" */
    0xf8, 0xe8, 0xf0,

    /* U+0416 "Ж" */
    0x92, 0xa8, 0xe2, 0xa9, 0x20,

    /* U+0417 "З" */
    0xc5, 0x1c,

    /* U+0418 "И" */
    0x9b, 0xd9, 0x90,

    /* U+0419 "Й" */
    0x60, 0x9b, 0xd9, 0x90,

    /* U+041A "К" */
    0x9a, 0xca, 0x90,

    /* U+041B "Л" */
    0x75, 0x55, 0x90,

    /* U+041C "М" */
    0x8e, 0xeb, 0x18, 0x80,

    /* U+041D "Н" */
    0x99, 0xf9, 0x90,

    /* U+041E "О" */
    0x69, 0x99, 0x60,

    /* U+041F "П" */
    0xf9, 0x99, 0x90,

    /* U+0420 "Р" */
    0xe9, 0xe8, 0x80,

    /* U+0421 "С" */
    0x69, 0x89, 0x60,

    /* U+0422 "Т" */
    0xe9, 0x24,

    /* U+0423 "У" */
    0x99, 0x62, 0xc0,

    /* U+0424 "Ф" */
    0x75, 0x6b, 0x57, 0x00,

    /* U+0425 "Х" */
    0xb5, 0x5a,

    /* U+0426 "Ц" */
    0x94, 0xa5, 0x2f, 0x84,

    /* U+0427 "Ч" */
    0x99, 0x71, 0x10,

    /* U+0428 "Ш" */
    0xad, 0x6b, 0x5f, 0x80,

    /* U+0429 "Щ" */
    0xaa, 0xaa, 0xaa, 0xfc, 0x10,

    /* U+042A "Ъ" */
    0xc2, 0x1c, 0x97, 0x00,

    /* U+042B "Ы" */
    0x86, 0x1e, 0x65, 0xe4,

    /* U+042C "Ь" */
    0x88, 0xe9, 0xe0,

    /* U+042D "Э" */
    0xe1, 0x71, 0xe0,

    /* U+042E "Ю" */
    0x9a, 0x9e, 0x69, 0x98,

    /* U+042F "Я" */
    0x79, 0x79, 0x90,

    /* U+0430 "а" */
    0x76, 0xb0,

    /* U+0431 "б" */
    0x73, 0x54,

    /* U+0432 "в" */
    0xde, 0xe0,

    /* U+0433 "г" */
    0xf2, 0x40,

    /* U+0434 "д" */
    0x72, 0x95, 0xf8, 0x80,

    /* U+0435 "е" */
    0x5e, 0x30,

    /* U+0436 "ж" */
    0xab, 0x9d, 0x50,

    /* U+0437 "з" */
    0xc8, 0xe0,

    /* U+0438 "и" */
    0x9b, 0xd9,

    /* U+0439 "й" */
    0x60, 0x9b, 0xd9,

    /* U+043A "к" */
    0xba, 0xd0,

    /* U+043B "л" */
    0x75, 0x59,

    /* U+043C "м" */
    0x8e, 0xeb, 0x10,

    /* U+043D "н" */
    0xbe, 0xd0,

    /* U+043E "о" */
    0x56, 0xa0,

    /* U+043F "п" */
    0xf6, 0xd0,

    /* U+0440 "р" */
    0xd6, 0xe8,

    /* U+0441 "с" */
    0x72, 0x30,

    /* U+0442 "т" */
    0xe9, 0x20,

    /* U+0443 "у" */
    0xb6, 0xa8,

    /* U+0444 "ф" */
    0x23, 0xab, 0x57, 0x10,

    /* U+0445 "х" */
    0xa9, 0x50,

    /* U+0446 "ц" */
    0xaa, 0xaf, 0x10,

    /* U+0447 "ч" */
    0xb5, 0x90,

    /* U+0448 "ш" */
    0xad, 0x6b, 0xf0,

    /* U+0449 "щ" */
    0xaa, 0xaa, 0xbf, 0x04,

    /* U+044A "ъ" */
    0xc6, 0x56,

    /* U+044B "ы" */
    0x8e, 0x6b, 0x90,

    /* U+044C "ь" */
    0x9a, 0xe0,

    /* U+044D "э" */
    0xcc, 0xe0,

    /* U+044E "ю" */
    0x97, 0x6b, 0x20,

    /* U+044F "я" */
    0x75, 0xd0,

    /* U+0451 "ё" */
    0xa1, 0x78, 0xc0,

    /* U+2013 "–" */
    0xe0,

    /* U+2014 "—" */
    0xf0,

    /* U+2018 "‘" */
    0x60,

    /* U+2019 "’" */
    0x60,

    /* U+201A "‚" */
    0xc0,

    /* U+201C "“" */
    0xb4,

    /* U+201D "”" */
    0xb4,

    /* U+201E "„" */
    0xb4,

    /* U+2020 "†" */
    0x5d, 0x20,

    /* U+2021 "‡" */
    0x5d, 0x74,

    /* U+2022 "•" */
    0xf0,

    /* U+2026 "…" */
    0xa8,

    /* U+2039 "‹" */
    0x64,

    /* U+203A "›" */
    0x98,

    /* U+205F " " */
    0x00,

    /* U+2070 "⁰" */
    0x56, 0xa0,

    /* U+2074 "⁴" */
    0x37, 0x90,

    /* U+2075 "⁵" */
    0xf0, 0xe0,

    /* U+2076 "⁶" */
    0x73, 0xf0,

    /* U+2077 "⁷" */
    0xe5, 0x40,

    /* U+2078 "⁸" */
    0x5e, 0xa0,

    /* U+2079 "⁹" */
    0xfc, 0xe0,

    /* U+2080 "₀" */
    0x56, 0xa0,

    /* U+2081 "₁" */
    0x75,

    /* U+2082 "₂" */
    0xc5, 0x70,

    /* U+2083 "₃" */
    0xe8, 0xe0,

    /* U+2084 "₄" */
    0x37, 0x90,

    /* U+2085 "₅" */
    0xf0, 0xe0,

    /* U+2086 "₆" */
    0x73, 0xf0,

    /* U+2087 "₇" */
    0xe5, 0x40,

    /* U+2088 "₈" */
    0x5e, 0xa0,

    /* U+2089 "₉" */
    0xfc, 0xe0,

    /* U+20AC "€" */
    0x69, 0xc9, 0x60,

    /* U+20BD "₽" */
    0xe9, 0xe8, 0xc0,

    /* U+2122 "™" */
    0xef, 0xe7, 0xd2, 0xe9, 0x10,

    /* U+2190 "←" */
    0x21, 0x86, 0x3f, 0x70, 0xc0,

    /* U+2191 "↑" */
    0x10, 0xff, 0xb1, 0x08,

    /* U+2192 "→" */
    0x30, 0xe1, 0xbf, 0x30, 0xc0,

    /* U+2193 "↓" */
    0x10, 0x86, 0xf7, 0x88,

    /* U+2194 "↔" */
    0x0c, 0x6e, 0x66, 0xff, 0x7c, 0x3c,

    /* U+2195 "↕" */
    0x10, 0xff, 0xbd, 0x7d, 0xe2,

    /* U+2196 "↖" */
    0xf3, 0xcb, 0xa7, 0x0c,

    /* U+2197 "↗" */
    0x3c, 0xf7, 0x79, 0xc0,

    /* U+2198 "↘" */
    0xc3, 0x87, 0x4f, 0x1c, 0xf0,

    /* U+2199 "↙" */
    0x0c, 0x7b, 0xbc, 0xe3, 0xc0,

    /* U+2211 "∑" */
    0xfb, 0x9c, 0xcf, 0x80,

    /* U+2212 "−" */
    0xe0,

    /* U+221A "√" */
    0x74, 0x4c, 0x40,

    /* U+221E "∞" */
    0x7e, 0xff, 0xdb, 0xff, 0x7e,

    /* U+2248 "≈" */
    0x7f, 0xfc, 0x4f, 0x68,

    /* U+2260 "≠" */
    0x3d, 0x78,

    /* U+2264 "≤" */
    0x2f, 0x66, 0x47,

    /* U+2265 "≥" */
    0x99, 0xbd, 0x07,

    /* U+FF0F "／" */
    0x0c, 0x73, 0x9c, 0xe3, 0x00
};

/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 32, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 32, .box_w = 1, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 64, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 3, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 7, .adv_w = 64, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 10, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 12, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 15, .adv_w = 32, .box_w = 1, .box_h = 2, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 16, .adv_w = 48, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 18, .adv_w = 48, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 20, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 22, .adv_w = 64, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 24, .adv_w = 32, .box_w = 1, .box_h = 2, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 25, .adv_w = 48, .box_w = 2, .box_h = 1, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 26, .adv_w = 32, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 27, .adv_w = 48, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 29, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 31, .adv_w = 48, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 33, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 35, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 37, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 39, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 41, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 43, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 45, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 47, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 49, .adv_w = 32, .box_w = 1, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 50, .adv_w = 48, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 52, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 54, .adv_w = 64, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 56, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 58, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 60, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 63, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 66, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 69, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 72, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 75, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 78, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 81, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 84, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 87, .adv_w = 32, .box_w = 1, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 88, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 90, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 93, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 95, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 99, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 105, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 108, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 111, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 114, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 117, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 119, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 122, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 124, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 128, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 130, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 132, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 134, .adv_w = 48, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 136, .adv_w = 48, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 138, .adv_w = 48, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 140, .adv_w = 64, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 141, .adv_w = 64, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 142, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 144, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 146, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 148, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 150, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 152, .adv_w = 48, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 156, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 158, .adv_w = 32, .box_w = 1, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 159, .adv_w = 48, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 161, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 32, .box_w = 1, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 164, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 167, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 169, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 171, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 173, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 175, .adv_w = 48, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 176, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 178, .adv_w = 48, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 180, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 182, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 184, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 189, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 191, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 193, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 195, .adv_w = 32, .box_w = 1, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 198, .adv_w = 80, .box_w = 4, .box_h = 2, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 199, .adv_w = 32, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 200, .adv_w = 32, .box_w = 1, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 201, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 204, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 206, .adv_w = 32, .box_w = 1, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 207, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 210, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 217, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 219, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 226, .adv_w = 64, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 228, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 230, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 232, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 234, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 238, .adv_w = 32, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 239, .adv_w = 48, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 240, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 242, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 244, .adv_w = 64, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 246, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 248, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 252, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 255, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 258, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 261, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 263, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 267, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 270, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 275, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 277, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 280, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 287, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 290, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 294, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 297, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 300, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 303, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 306, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 309, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 311, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 314, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 318, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 320, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 324, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 327, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 331, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 336, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 340, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 344, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 347, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 350, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 354, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 357, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 359, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 361, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 363, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 365, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 369, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 371, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 374, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 376, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 378, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 381, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 383, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 385, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 388, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 390, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 392, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 394, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 396, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 398, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 400, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 402, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 406, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 408, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 411, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 413, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 416, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 420, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 422, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 425, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 427, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 429, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 432, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 434, .adv_w = 64, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 437, .adv_w = 64, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 438, .adv_w = 80, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 439, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 440, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 441, .adv_w = 32, .box_w = 1, .box_h = 2, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 442, .adv_w = 64, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 443, .adv_w = 64, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 444, .adv_w = 64, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 445, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 447, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 449, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 450, .adv_w = 96, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 451, .adv_w = 48, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 452, .adv_w = 48, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 453, .adv_w = 32, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 454, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 456, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 458, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 460, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 462, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 464, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 466, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 468, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 470, .adv_w = 48, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 471, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 473, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 475, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 477, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 479, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 481, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 483, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 485, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 487, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 490, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 493, .adv_w = 160, .box_w = 9, .box_h = 4, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 498, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 503, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 507, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 512, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 516, .adv_w = 128, .box_w = 8, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 522, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 527, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 531, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 535, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 540, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 545, .adv_w = 80, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 549, .adv_w = 64, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 550, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 553, .adv_w = 128, .box_w = 8, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 558, .adv_w = 80, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 562, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 564, .adv_w = 64, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 567, .adv_w = 64, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 570, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0}
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
    0x00, 0x1bc2, 0x1bc3, 0x1bc7, 0x1bc8, 0x1bc9, 0x1bcb, 0x1bcc,
    0x1bcd, 0x1bcf, 0x1bd0, 0x1bd1, 0x1bd5, 0x1be8, 0x1be9, 0x1c0e,
    0x1c1f, 0x1c23, 0x1c24, 0x1c25, 0x1c26, 0x1c27, 0x1c28, 0x1c2f,
    0x1c30, 0x1c31, 0x1c32, 0x1c33, 0x1c34, 0x1c35, 0x1c36, 0x1c37,
    0x1c38, 0x1c5b, 0x1c6c, 0x1cd1, 0x1d3f, 0x1d40, 0x1d41, 0x1d42,
    0x1d43, 0x1d44, 0x1d45, 0x1d46, 0x1d47, 0x1d48, 0x1dc0, 0x1dc1,
    0x1dc9, 0x1dcd, 0x1df7, 0x1e0f, 0x1e13, 0x1e14, 0xfabe
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
        .unicode_list = unicode_list_4, .glyph_id_ofs_list = NULL, .list_length = 55, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
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
const lv_font_t lv_font_busy_regular_5 = {
#else
lv_font_t lv_font_busy_regular_5 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,         /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,         /*Function pointer to get glyph's bitmap*/
    .line_height = 9, /*The maximum line height required by the font*/
    .base_line = 2,                     /*Baseline measured from the bottom of the line*/
#if LV_VERSION_CHECK(9, 6, 0) || LVGL_VERSION_MAJOR >= 10
    .cap_height = 5,                       /*Cap height of the font*/
    .x_height = 4,                           /*x-height of the font*/
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



#endif /*#if LV_FONT_BUSY_REGULAR_5*/
