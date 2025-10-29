/**
 * @brief
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>

#define SETTINGS_VOLUME_RANGE_MIN 0
#define SETTINGS_VOLUME_RANGE_MAX 100
#define SETTINGS_VOLUME_STEP      5

typedef struct VolumeModel VolumeModel;

VolumeModel* volume_model_alloc(void);
void volume_model_free(VolumeModel* model);

void volume_model_set(VolumeModel* model, uint8_t volume);
uint8_t volume_model_get(VolumeModel* model);

void volume_model_format(VolumeModel* model, FuriString* string);

#ifdef __cplusplus
}
#endif
