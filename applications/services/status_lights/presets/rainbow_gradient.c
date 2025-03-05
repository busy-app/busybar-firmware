#include "../status_lights_preset_base.h"

#include <furi/furi.h>

typedef struct {
    ColorHsv color;
} RainbowGradient;

RainbowGradient* rainbow_gradient_alloc(const Color* color) {
    UNUSED(color);

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

void rainbow_gradient_run(RainbowGradient* instance, Color* color) {
    furi_check(instance);
    furi_check(color);

    instance->color.h++;
    *color = color_hsv_to_rgb(instance->color);
}

const StatusLightsPresetBase status_ligth_preset_rainbow_gradient = {
    .period_ms = 16,
    .alloc = (StatusLightsPresetAlloc)rainbow_gradient_alloc,
    .free = (StatusLightsPresetFree)rainbow_gradient_free,
    .run = (StatusLightsPresetRun)rainbow_gradient_run,
};
