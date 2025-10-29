/**
 * @brief Brightness model
 */

#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BRIGHTNESS_RANGE_MIN 0
#define BRIGHTNESS_RANGE_MAX 100
#define BRIGHTNESS_STEP      5

typedef struct BrightnessModel BrightnessModel;

typedef enum {
    BrightnessModeManual,
    BrightnessModeAuto,

    BrightnessModesCount,
} BrightnessMode;

BrightnessModel* brightness_model_alloc(void);
void brightness_model_free(BrightnessModel* model);

void brightness_model_set_auto_mode(BrightnessModel* model);
BrightnessMode brightness_model_get_mode(BrightnessModel* model);

void brightness_model_set(BrightnessModel* model, uint8_t brightness);
uint8_t brightness_model_get(BrightnessModel* model);
void brightness_model_format(BrightnessModel* model, FuriString* string);

#ifdef __cplusplus
}
#endif
