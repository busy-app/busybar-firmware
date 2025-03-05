#include "../status_lights_preset_base.h"

#include <furi/furi.h>

typedef struct {
    uint8_t tick;
    Color color;
} Fade;

static Fade* fade_alloc(const Color* color) {
    furi_check(color);

    Fade* instance = malloc(sizeof(Fade));
    instance->color = *color;

    return instance;
}

static void fade_free(Fade* instance) {
    furi_check(instance);

    free(instance);
}

static void fade_run(Fade* instance, Color* color) {
    furi_check(instance);
    furi_check(color);

    uint8_t brightness = instance->tick++ < 128 ? instance->tick : 256 - instance->tick;
    color->r = instance->color.r / 128 * brightness;
    color->g = instance->color.g / 128 * brightness;
    color->b = instance->color.b / 128 * brightness;
}

const StatusLightsPresetBase status_lights_preset_fade = {
    .period_ms = 10,
    .alloc = (StatusLightsPresetAlloc)fade_alloc,
    .free = (StatusLightsPresetFree)fade_free,
    .run = (StatusLightsPresetRun)fade_run,
};
