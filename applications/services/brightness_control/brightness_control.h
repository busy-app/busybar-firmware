#pragma once

#include <stdint.h>

#include <furi/core/state.h>
#include "brightness_conv.h"

#define RECORD_BRIGHTNESS_CONTROL "brightness_control"

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
    InternalBrightness effective_brightness;

    /**
    * Brightness value as previously set by user.
    * In automatic mode this value is stored, but not used to control brightness.
    */
    UserBrightness brightness_setting;
} BrightnessControlState;

typedef struct BrightnessControl BrightnessControl;

// uint8_t brightness_control_get_brightness(BrightnessControl* instance);

void brightness_control_set_auto_brightness(BrightnessControl* instance);

void brightness_control_set_manual_brightness(
    BrightnessControl* instance,
    UserBrightness brightness);

/**
 * @brief enable or disable temporary brightness override for a module.
 *
 * Brightness of selected module will be set to a fixed value and unaffected by auto/manual brightness set otherwise.
 *
 * @param override The desired brightness or NULL to disable override.
 */
void brightness_control_set_brightness_override(
    BrightnessControl* instance,
    BrightnessControlModule module,
    const UserBrightness* override);

FuriState* brightness_control_get_state(const BrightnessControl* instance);

// NOTE: Brightness control manages settings for both displays and status lights in one place
