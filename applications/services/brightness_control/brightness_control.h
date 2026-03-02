#pragma once

#include <stdint.h>

#include <furi/core/state.h>

#define RECORD_BRIGHTNESS_CONTROL "brightness_control"

#define BRIGHTNESS_MIN (0)
#define BRIGHTNESS_MAX (100)

typedef enum {
    BrightnessControlBrightnessModeAuto,
    BrightnessControlBrightnessModeManual,

    BrightnessControlBrightnessModeMax,
} BrightnessControlBrightnessMode;

typedef enum BrightnessControlModule {
    BrightnessControlModuleFrontDisplay,
    BrightnessControlModuleBackDisplay,
    BrightnessControlModuleStatusLights,

    BrightnessControlModuleMax,
} BrightnessControlModule;

typedef struct {
    /**
    * Brightness mode (auto or manual)
    */
    BrightnessControlBrightnessMode mode;

    /**
    * Actual brightness value.
    * In auto mode this value is controlled by the light sensor.
    * In manual mode this is the user-set value.
    */
    uint8_t effective_brightness;

    /**
    * Brightness value as previously set by user.
    * In automatic mode this value is stored, but not used to control brightness.
    */
    uint8_t brightness_setting;
} BrightnessControlState;

typedef struct BrightnessControl BrightnessControl;

void brightness_control_set_auto_brightness(BrightnessControl* instance);

/**
 * @brief Set manual brightness, clamping the input value into range.
 *
 * @param brightness brightness value (0-100).
 */
void brightness_control_set_manual_brightness(BrightnessControl* instance, uint8_t brightness);

/**
 * @brief enable temporary brightness override for a module.
 *
 * Brightness of selected module will be set to a fixed value and unaffected by auto/manual brightness set otherwise.
 *
 * @param override The desired brightness.
 */
void brightness_control_set_brightness_override(
    BrightnessControl* instance,
    BrightnessControlModule module,
    uint8_t override);

/**
 * @brief disable temporary brightness override for a module.
 */
void brightness_control_reset_brightness_override(
    BrightnessControl* instance,
    BrightnessControlModule module);

FuriState* brightness_control_get_state(const BrightnessControl* instance);
