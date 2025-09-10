#include "brightness.h"
#include "common.h"

#define BACK_BRIGHTNESS_RANGE_MIN 0
#define BACK_BRIGHTNESS_RANGE_MAX 100

#define FRONT_BRIGHTNESS_RANGE_MIN 25
#define FRONT_BRIGHTNESS_RANGE_MAX 100

static uint8_t settings_brightness_to_model(uint8_t brightness, uint8_t min, uint8_t max) {
    uint8_t input_range = max - min;
    uint8_t output_range = SETTINGS_BRIGHTNESS_RANGE_MAX - SETTINGS_BRIGHTNESS_RANGE_MIN;

    return CEILING_MULTIPLE_OF(
        SETTINGS_BRIGHTNESS_RANGE_MIN +
            ((CLAMP(brightness, max, min) - min) * output_range) / input_range,
        SETTINGS_BRIGHTNESS_STEP);
}

static uint8_t settings_brightness_from_model(uint8_t brightness, uint8_t min, uint8_t max) {
    uint8_t input_range = SETTINGS_BRIGHTNESS_RANGE_MAX - SETTINGS_BRIGHTNESS_RANGE_MIN;
    uint8_t output_range = max - min;

    return min + ((brightness - SETTINGS_BRIGHTNESS_RANGE_MIN) * output_range) / input_range;
}

void settings_brightness_set_auto_mode(SettingsApp* instance) {
    furi_assert(instance);

    back_display_set_brightness(instance->back_display, BACK_DISPLAY_BRIGHTNESS_AUTO);
    front_display_set_brightness(instance->front_display, FRONT_DISPLAY_BRIGHTNESS_AUTO);
}

SettingsBrightnessMode settings_brightness_get_mode(SettingsApp* instance) {
    furi_assert(instance);

    uint8_t brightness = back_display_get_brightness(instance->back_display);
    return (brightness == BACK_DISPLAY_BRIGHTNESS_AUTO) ? SettingsBrightnessModeAuto :
                                                          SettingsBrightnessModeManual;
}

void settings_brightness_set(SettingsApp* instance, uint8_t brightness) {
    furi_assert(instance);
    furi_assert(brightness >= SETTINGS_BRIGHTNESS_RANGE_MIN);
    furi_assert(brightness <= SETTINGS_BRIGHTNESS_RANGE_MAX);

    uint8_t back_brightness = settings_brightness_from_model(
        brightness, BACK_BRIGHTNESS_RANGE_MIN, BACK_BRIGHTNESS_RANGE_MAX);
    back_display_set_brightness(instance->back_display, back_brightness);

    uint8_t front_brightness = settings_brightness_from_model(
        brightness, FRONT_BRIGHTNESS_RANGE_MIN, FRONT_BRIGHTNESS_RANGE_MAX);
    front_display_set_brightness(instance->front_display, front_brightness);
}

uint8_t settings_brightness_get(SettingsApp* instance) {
    furi_assert(instance);

    uint8_t brightness = back_display_get_brightness(instance->back_display);
    return settings_brightness_to_model(
        brightness, BACK_BRIGHTNESS_RANGE_MIN, BACK_BRIGHTNESS_RANGE_MAX);
}
