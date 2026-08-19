#pragma once
#include <light_sensor/light_sensor.h>
#include <light_sensor/light_sensor_common.h>
#include <front_display/front_display.h>
#include <back_display/back_display.h>
#if defined(SRV_STATUS_LIGHTS)
#include <status_lights/status_lights.h>
#endif
#include "brightness_control.h"

/// Brightness value as set by user (0-100)
typedef struct UserBrightness {
    uint8_t val;
} UserBrightness;

/// Brightness value used internally (0-1)
typedef struct InternalBrightness {
    float val;
} InternalBrightness;

/// Clamp and convert an integer to user brightness.
UserBrightness brightness_conv_int_to_user_clamped(int brightness);

/// Convert user brightness to internal brightness.
InternalBrightness brightness_conv_user_to_internal(UserBrightness v);

/// Convert internal brightness to user brightness.
UserBrightness brightness_conv_internal_to_user(InternalBrightness v);

/// Convert light sensor level to internal brightness.
InternalBrightness brightness_conv_light_sensor_to_internal(LightSensorLevel v);

/// Produce appropriate front display brightness for given internal brightness.
FrontDisplayBrightness brightness_conv_internal_to_front(InternalBrightness v);

/// Produce appropriate back display contrast for given internal brightness.
BackDisplayContrast brightness_conv_internal_to_back(InternalBrightness v);

#if defined(SRV_STATUS_LIGHTS)
/// Produce appropriate status lights brightness for given internal brightness.
StatusLightsBrightness brightness_conv_internal_to_status(InternalBrightness v);
#endif
