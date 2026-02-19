#pragma once

#include <stdint.h>

#include <furi/core/state.h>

#define RECORD_BRIGHTNESS_CONTROL "brightness_control"

typedef enum {
   BrightnessControlBrightnessModeAuto,
   BrightnessControlBrightnessModeManual,

   BrightnessControlBrightnessModeMax,
} BrightnessControlBrightnessMode;

typedef struct {
   /**
    * Brightness mode (auto or manual)
    */
   BrightnessControlBrightnessMode mode;

   /**
    * Actual brightness value.
    * In auto mode this value is controlled by the light sensor.
    * In manual mode this is the (clamped) user-set value.
    */
   uint8_t brightness;
} BrightnessControlState;

typedef struct BrightnessControl BrightnessControl;

uint8_t brightness_control_get_brightness(BrightnessControl* instance);

void brightness_control_set_auto_brightness(BrightnessControl* instance);

void brightness_control_set_manual_brightness(BrightnessControl* instance, uint8_t brightness);

FuriState* brightness_control_get_state(BrightnessControl* instance);

// NOTE: Brightness control manages settings for both displays and status lights in one place
