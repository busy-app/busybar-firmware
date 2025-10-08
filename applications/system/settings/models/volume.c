#include "volume.h"
#include "common.h"

static uint8_t settings_volume_to_model(float volume) {
    return CEILING_MULTIPLE_OF((uint32_t)(volume * 100.f), SETTINGS_VOLUME_STEP);
}

static float settings_volume_from_model(uint8_t volume) {
    return volume * 0.01f;
}

void settings_volume_set(SettingsApp* instance, uint8_t volume) {
    furi_assert(instance);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    furi_assert(volume >= SETTINGS_VOLUME_RANGE_MIN);
    furi_assert(volume <= SETTINGS_VOLUME_RANGE_MAX);
#pragma GCC diagnostic pop

    audio_set_volume(instance->audio, settings_volume_from_model(volume));
}

uint8_t settings_volume_get(SettingsApp* instance) {
    furi_assert(instance);

    return settings_volume_to_model(audio_get_volume(instance->audio));
}
