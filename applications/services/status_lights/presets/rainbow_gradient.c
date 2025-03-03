#include "../status_lights_preset_base.h"

#include <furi/furi.h>

typedef struct {
    uint8_t h;
    uint8_t s;
    uint8_t v;
} RainbowGradientColor;

typedef struct {
    RainbowGradientColor color;
} RainbowGradient;

// https://stackoverflow.com/questions/24152553/hsv-to-rgb-and-back-without-floating-point-math-in-python
static void status_lights_hsv_to_rgb(const RainbowGradientColor* hsv, StatusLightsColor* rgb) {
    if(hsv->s == 0) {
        rgb->r = hsv->v;
        rgb->g = hsv->v;
        rgb->b = hsv->v;
        return;
    }

    const uint8_t region = hsv->h / 43;
    const uint8_t remainder = (hsv->h % 43) * 6;

    const uint16_t s = hsv->s;
    const uint16_t v = hsv->v;

    const uint16_t p = (v * (255 - hsv->s)) >> 8;
    const uint16_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    const uint16_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch(region) {
    case 0:
        rgb->r = v;
        rgb->g = t;
        rgb->b = p;
        break;

    case 1:
        rgb->r = q;
        rgb->g = v;
        rgb->b = p;
        break;

    case 2:
        rgb->r = p;
        rgb->g = v;
        rgb->b = t;
        break;

    case 3:
        rgb->r = p;
        rgb->g = q;
        rgb->b = v;
        break;

    case 4:
        rgb->r = t;
        rgb->g = p;
        rgb->b = v;
        break;

    default:
        rgb->r = v;
        rgb->g = p;
        rgb->b = q;
        break;
    }
}

RainbowGradient* rainbow_gradient_alloc(void) {
    RainbowGradient* instance = malloc(sizeof(RainbowGradient));
    instance->color.s = 255;
    instance->color.v = 16;
    instance->color.h = 0;

    return instance;
}

void rainbow_gradient_free(RainbowGradient* instance) {
    furi_check(instance);

    free(instance);
}

void rainbow_gradient_run(RainbowGradient* instance, StatusLightsColor* color) {
    furi_check(instance);
    furi_check(color);

    status_lights_hsv_to_rgb(&instance->color, color);

    instance->color.h++;
}

const StatusLightsPresetBase status_ligth_preset_rainbow_gradient = {
    .period_ms = 16,
    .alloc = (StatusLightsPresetAlloc)rainbow_gradient_alloc,
    .free = (StatusLightsPresetFree)rainbow_gradient_free,
    .run = (StatusLightsPresetRun)rainbow_gradient_run,
};
