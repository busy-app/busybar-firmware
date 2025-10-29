#include "volume.h"
#include <toolbox/float_tools.h>
#include <audio/audio.h>

struct VolumeModel {
    Audio* audio;
};

static uint8_t volume_to_model(float volume) {
    return CEILING_MULTIPLE_OF((uint32_t)(volume * 100.f), SETTINGS_VOLUME_STEP);
}

static float volume_from_model(uint8_t volume) {
    return volume * 0.01f;
}

VolumeModel* volume_model_alloc(void) {
    VolumeModel* model = malloc(sizeof(VolumeModel));
    model->audio = furi_record_open(RECORD_AUDIO);
    return model;
}

void volume_model_free(VolumeModel* model) {
    furi_assert(model);
    furi_record_close(RECORD_AUDIO);
    free(model);
}

void volume_model_set(VolumeModel* model, uint8_t volume) {
    furi_assert(model);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    furi_assert(volume >= SETTINGS_VOLUME_RANGE_MIN);
    furi_assert(volume <= SETTINGS_VOLUME_RANGE_MAX);
#pragma GCC diagnostic pop

    audio_set_volume(model->audio, volume_from_model(volume));
}

uint8_t volume_model_get(VolumeModel* model) {
    furi_assert(model);

    return volume_to_model(audio_get_volume(model->audio));
}

void volume_model_format(VolumeModel* model, FuriString* string) {
    furi_assert(model);
    furi_assert(string);
    furi_string_printf(string, "%hhu%%", volume_model_get(model));
}
