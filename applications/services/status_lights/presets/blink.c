#include "../status_lights_preset_base.h"

#include <furi/furi.h>

typedef struct {
    Color color;
    bool is_on;
} Blink;

static Blink* blink_alloc(const Color* color) {
    furi_check(color);

    Blink* instance = malloc(sizeof(*instance));
    instance->color = *color;
    instance->is_on = false;

    return instance;
}

static void blink_free(Blink* instance) {
    furi_check(instance);

    free(instance);
}

static void blink_run(Blink* instance, Color* color) {
    furi_check(instance);
    furi_check(color);

    *color = (instance->is_on) ? (Color)COLOR_MAKE_RGB(0, 0, 0) : instance->color;
    instance->is_on = !instance->is_on;
}

const StatusLightsPresetBase status_lights_preset_blink = {
    .period_ms = 500,
    .alloc = (StatusLightsPresetAlloc)blink_alloc,
    .free = (StatusLightsPresetFree)blink_free,
    .run = (StatusLightsPresetRun)blink_run,
};
