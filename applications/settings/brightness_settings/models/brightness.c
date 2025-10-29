#include "brightness.h"
#include <front_display/front_display.h>
#include <back_display/back_display.h>
#include <status_lights/status_lights.h>
#include <toolbox/float_tools.h>

#define BACK_BRIGHTNESS_RANGE_MIN 5
#define BACK_BRIGHTNESS_RANGE_MAX 100

#define FRONT_BRIGHTNESS_RANGE_MIN 25
#define FRONT_BRIGHTNESS_RANGE_MAX 100

#define STATUS_LIGHTS_BRIGHTNESS_RANGE_MIN 1
#define STATUS_LIGHTS_BRIGHTNESS_RANGE_MAX 100

struct BrightnessModel {
    FrontDisplaySrv* front;
    BackDisplaySrv* back;
    StatusLights* lights;
};

static uint8_t brightness_to_model(uint8_t brightness, uint8_t min, uint8_t max) {
    uint8_t input_range = max - min;
    uint8_t output_range = BRIGHTNESS_RANGE_MAX - BRIGHTNESS_RANGE_MIN;

    return CEILING_MULTIPLE_OF(
        BRIGHTNESS_RANGE_MIN + ((CLAMP(brightness, max, min) - min) * output_range) / input_range,
        BRIGHTNESS_STEP);
}

static uint8_t brightness_from_model(uint8_t brightness, uint8_t min, uint8_t max) {
    uint8_t input_range = BRIGHTNESS_RANGE_MAX - BRIGHTNESS_RANGE_MIN;
    uint8_t output_range = max - min;

    return min + ((brightness - BRIGHTNESS_RANGE_MIN) * output_range) / input_range;
}

BrightnessModel* brightness_model_alloc(void) {
    BrightnessModel* model = malloc(sizeof(BrightnessModel));
    model->front = furi_record_open(RECORD_FRONT_DISPLAY);
    model->back = furi_record_open(RECORD_BACK_DISPLAY);
    model->lights = furi_record_open(RECORD_STATUS_LIGHTS);
    return model;
}

void brightness_model_free(BrightnessModel* model) {
    furi_assert(model);
    furi_record_close(RECORD_STATUS_LIGHTS);
    furi_record_close(RECORD_BACK_DISPLAY);
    furi_record_close(RECORD_FRONT_DISPLAY);
    free(model);
}

void brightness_model_set_auto_mode(BrightnessModel* model) {
    furi_assert(model);

    back_display_set_brightness(model->back, BACK_DISPLAY_BRIGHTNESS_AUTO);
    front_display_set_brightness(model->front, FRONT_DISPLAY_BRIGHTNESS_AUTO);
    status_lights_set_brightness(model->lights, STATUS_LIGHTS_BRIGHTNESS_AUTO);
}

BrightnessMode brightness_model_get_mode(BrightnessModel* model) {
    furi_assert(model);

    uint8_t brightness = back_display_get_brightness(model->back);
    return (brightness == BACK_DISPLAY_BRIGHTNESS_AUTO) ? BrightnessModeAuto :
                                                          BrightnessModeManual;
}

void brightness_model_set(BrightnessModel* model, uint8_t brightness) {
    furi_assert(model);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    furi_assert(brightness >= BRIGHTNESS_RANGE_MIN);
    furi_assert(brightness <= BRIGHTNESS_RANGE_MAX);
#pragma GCC diagnostic pop

    uint8_t back_brightness =
        brightness_from_model(brightness, BACK_BRIGHTNESS_RANGE_MIN, BACK_BRIGHTNESS_RANGE_MAX);
    back_display_set_brightness(model->back, back_brightness);

    uint8_t front_brightness =
        brightness_from_model(brightness, FRONT_BRIGHTNESS_RANGE_MIN, FRONT_BRIGHTNESS_RANGE_MAX);
    front_display_set_brightness(model->front, front_brightness);

    uint8_t status_lights_brightness = brightness_from_model(
        brightness, STATUS_LIGHTS_BRIGHTNESS_RANGE_MIN, STATUS_LIGHTS_BRIGHTNESS_RANGE_MAX);
    status_lights_set_brightness(model->lights, status_lights_brightness);
}

uint8_t brightness_model_get(BrightnessModel* model) {
    furi_assert(model);

    uint8_t brightness = back_display_get_brightness(model->back);
    return brightness_to_model(brightness, BACK_BRIGHTNESS_RANGE_MIN, BACK_BRIGHTNESS_RANGE_MAX);
}

void brightness_model_format(BrightnessModel* model, char* buffer) {
    furi_assert(model);
    furi_assert(buffer);

    if(brightness_model_get_mode(model) == BrightnessModeAuto) {
        strcpy(buffer, "Auto");
    } else {
        sprintf(buffer, "%hhu%%", brightness_model_get(model));
    }
}
