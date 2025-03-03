#include "../status_lights_preset_base.h"

#include <furi/furi.h>

typedef struct {
    uint8_t tick;
} WhiteFade;

static WhiteFade* white_fade_alloc(void) {
    WhiteFade* instance = malloc(sizeof(WhiteFade));

    return instance;
}

static void white_fade_free(WhiteFade* instance) {
    furi_check(instance);

    free(instance);
}

static void white_fade_run(WhiteFade* instance, StatusLightsColor* color) {
    furi_check(instance);
    furi_check(color);

    uint8_t brightness = instance->tick++ < 128 ? instance->tick : 256 - instance->tick;
    color->r = brightness;
    color->g = brightness;
    color->b = brightness;
}

const StatusLightsPresetBase status_ligth_preset_white_fade = {
    .period_ms = 10,
    .alloc = (StatusLightsPresetAlloc)white_fade_alloc,
    .free = (StatusLightsPresetFree)white_fade_free,
    .run = (StatusLightsPresetRun)white_fade_run,
};
