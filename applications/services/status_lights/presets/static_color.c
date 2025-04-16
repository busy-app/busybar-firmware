#include "../status_lights_preset_base.h"

#include <furi/furi.h>

typedef struct {
    Color color;
} StaticColor;

static StaticColor* static_color_alloc(const Color* color) {
    furi_check(color);

    StaticColor* instance = malloc(sizeof(StaticColor));
    instance->color = *color;

    return instance;
}

static void static_color_free(StaticColor* instance) {
    furi_check(instance);

    free(instance);
}

static void static_color_run(StaticColor* instance, Color* color) {
    furi_check(instance);
    furi_check(color);

    *color = instance->color;
}

const StatusLightsPresetBase status_lights_preset_static_color = {
    .period_ms = 1000,
    .alloc = (StatusLightsPresetAlloc)static_color_alloc,
    .free = (StatusLightsPresetFree)static_color_free,
    .run = (StatusLightsPresetRun)static_color_run,
};
