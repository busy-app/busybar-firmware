#include "brightness_conv.h"

#include <math.h>

#include <core/core_defines.h>

#define INTERNAL_BRIGHTNESS_MAX UINT8_MAX

typedef struct {
    uint8_t min;
    uint8_t max;
    float power;
} BrightnessCurve;

static const BrightnessCurve front_curve = {
    .min = 1,
    .max = 100,
    .power = 2.f,
};

static const BrightnessCurve back_curve = {
    .min = 1,
    .max = 71,
    .power = 4.f,
};

#if defined(SRV_STATUS_LIGHTS)
static const BrightnessCurve lights_curve = {
    .min = 5,
    .max = 90,
};
#endif

static uint8_t brightness_conv_map_curve(const BrightnessCurve* curve, InternalBrightness v) {
    const float val_norm = (float)v.val / INTERNAL_BRIGHTNESS_MAX;

    float val_exp;
    const float power = curve->power;

    if(fabsf(power) < 1e-2f) {
        val_exp = val_norm;
    } else {
        val_exp = (expf(power * val_norm) - 1.f) / (expf(power) - 1.f);
    }

    return roundf(curve->min + (curve->max - curve->min) * val_exp);
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
    return (FrontDisplayBrightness){brightness_conv_map_curve(&front_curve, v)};
}

BackDisplayContrast brightness_conv_internal_to_back(InternalBrightness v) {
    return (BackDisplayContrast){brightness_conv_map_curve(&back_curve, v)};
}

#if defined(SRV_STATUS_LIGHTS)
StatusLightsBrightness brightness_conv_internal_to_status(InternalBrightness v) {
    return (StatusLightsBrightness){brightness_conv_map_curve(&lights_curve, v)};
}
#endif
