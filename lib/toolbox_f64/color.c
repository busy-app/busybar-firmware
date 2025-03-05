#include "color.h"

// https://stackoverflow.com/questions/24152553/hsv-to-rgb-and-back-without-floating-point-math-in-python
Color color_hsv_to_rgb(ColorHsv hsv) {
    Color rgb = {};

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
