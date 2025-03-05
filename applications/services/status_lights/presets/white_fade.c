#include "../status_lights_preset_base.h"

#include <furi/furi.h>

typedef struct {
    uint8_t tick;
    Color color;
} WhiteFade;

static WhiteFade* white_fade_alloc(const Color* color) {
    furi_check(color);

    WhiteFade* instance = malloc(sizeof(WhiteFade));
    instance->color = *color;

    return instance;
}

static void white_fade_free(WhiteFade* instance) {
    furi_check(instance);

    free(instance);
}

static void white_fade_run(WhiteFade* instance, Color* color) {
    furi_check(instance);
    furi_check(color);

    uint8_t brightness = instance->tick++ < 128 ? instance->tick : 256 - instance->tick;
    color->r = instance->color.r / 128 * brightness;
    color->g = instance->color.g / 128 * brightness;
    color->b = instance->color.b / 128 * brightness;
}

const StatusLightsPresetBase status_ligth_preset_white_fade = {
    .period_ms = 10,
    .alloc = (StatusLightsPresetAlloc)white_fade_alloc,
    .free = (StatusLightsPresetFree)white_fade_free,
    .run = (StatusLightsPresetRun)white_fade_run,
};
