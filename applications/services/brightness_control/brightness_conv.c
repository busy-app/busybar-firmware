#include "brightness_conv.h"

#include <math.h>

#include <core/core_defines.h>

#define INTERNAL_BRIGHTNESS_MAX UINT8_MAX

typedef struct {
    uint8_t min;
    uint8_t max;
} BrightnessRange;

static const BrightnessRange front_range = {
    .min = 1,
    .max = 100,
};

static const BrightnessRange back_range = {
    .min = 1,
    .max = 71,
};

#if defined(SRV_STATUS_LIGHTS)
static const BrightnessRange lights_range = {
    .min = 5,
    .max = 90,
};
#endif

static uint8_t brightness_conv_map_range(const BrightnessRange* range, InternalBrightness v) {
    return range->min + v.val * ((range->max - range->min) / 255.f);
}

UserBrightness brightness_conv_int_to_user_clamped(int brightness) {
    brightness = CLAMP(brightness, BRIGHTNESS_MAX, BRIGHTNESS_MIN);
    return (UserBrightness){brightness};
}

InternalBrightness brightness_conv_user_to_internal(UserBrightness v) {
    return (InternalBrightness){roundf(((float)INTERNAL_BRIGHTNESS_MAX / BRIGHTNESS_MAX) * v.val)};
}

UserBrightness brightness_conv_internal_to_user(InternalBrightness v) {
    return (UserBrightness){roundf(v.val / ((float)INTERNAL_BRIGHTNESS_MAX / BRIGHTNESS_MAX))};
}

InternalBrightness brightness_conv_light_sensor_to_internal(LightSensorLevel v) {
    return (InternalBrightness){
        roundf(v.val * ((float)INTERNAL_BRIGHTNESS_MAX / LIGHT_SENSOR_LIGHT_LEVEL_MAX))};
}

FrontDisplayBrightness brightness_conv_internal_to_front(InternalBrightness v) {
    return (FrontDisplayBrightness){brightness_conv_map_range(&front_range, v)};
}

BackDisplayContrast brightness_conv_internal_to_back(InternalBrightness v) {
    return (BackDisplayContrast){brightness_conv_map_range(&back_range, v)};
}

#if defined(SRV_STATUS_LIGHTS)
StatusLightsBrightness brightness_conv_internal_to_status(InternalBrightness v) {
    return (StatusLightsBrightness){brightness_conv_map_range(&lights_range, v)};
}
#endif
