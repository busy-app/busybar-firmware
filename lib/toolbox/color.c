#include "color.h"

#include <core/check.h>
#include <limits.h>
#include "strint.h"

// https://stackoverflow.com/questions/24152553/hsv-to-rgb-and-back-without-floating-point-math-in-python
Color color_hsv_to_rgb(ColorHsv hsv) {
    Color rgb = {.a = hsv.a};

    if(hsv.s == 0) {
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }

    const uint8_t region = hsv.h / 43;
    const uint8_t remainder = (hsv.h % 43) * 6;

    const uint16_t s = hsv.s;
    const uint16_t v = hsv.v;

    const uint16_t p = (v * (255 - hsv.s)) >> 8;
    const uint16_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    const uint16_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch(region) {
    case 0:
        rgb.r = v;
        rgb.g = t;
        rgb.b = p;
        break;

    case 1:
        rgb.r = q;
        rgb.g = v;
        rgb.b = p;
        break;

    case 2:
        rgb.r = p;
        rgb.g = v;
        rgb.b = t;
        break;

    case 3:
        rgb.r = p;
        rgb.g = q;
        rgb.b = v;
        break;

    case 4:
        rgb.r = t;
        rgb.g = p;
        rgb.b = v;
        break;

    default:
        rgb.r = v;
        rgb.g = p;
        rgb.b = q;
        break;
    }

    return rgb;
}

Color color_hex_to_rgb(uint32_t hex) {
    furi_check(hex <= 0xFFFFFF);

    Color rgb = {
        .a = 255,
        .b = hex & 0xFF,
        .g = (hex >> 8) & 0xFF,
        .r = (hex >> 16) & 0xFF,
    };

    return rgb;
}

Color color_hexa_to_rgb(uint32_t hexa) {
    Color rgb = {
        .a = hexa & 0xFF,
        .b = (hexa >> 8) & 0xFF,
        .g = (hexa >> 16) & 0xFF,
        .r = (hexa >> 24) & 0xFF,
    };

    return rgb;
}

bool color_parse_hexa_string(const char* hexa, Color* color_out) {
    if(strlen(hexa) != strlen("#RRGGBBAA")) return false;
    hexa++;

    uint32_t hexa_int;
    if(strint_to_uint32(hexa, NULL, &hexa_int, 16) != StrintParseNoError) return false;

    *color_out = color_hexa_to_rgb(hexa_int);
    return true;
}

uint8_t color_rgb_to_l8(Color color) {
    /* 
     * BT.601 luma coefficients (0.299 * R + 0.587 * G + 0.114 * B) via
     * fixed-point arithmetic: L = (77 * R + 150 * G + 29 * B) >> 8. 
     */
    return (color.r * 77 + color.g * 150 + color.b * 29) >> CHAR_BIT;
}

void color_buf_l8_to_l4(void* dst, const void* src, size_t size) {
    const uint8_t* src_u8 = src;
    uint8_t* dst_u8 = dst;
    size_t dst_size = size / 2;
    for(uint32_t dst_i = 0; dst_i < dst_size; ++dst_i) {
        const size_t src_i = 2 * dst_i;
        dst_u8[dst_i] = (src_u8[src_i] >> 4) | (src_u8[src_i + 1] & 0xF0);
    }
}
