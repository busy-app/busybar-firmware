#include "brightness_conv.h"

#include <math.h>

#include <core/core_defines.h>

#define BRIGHTNESS_CONV_POWER_MIN (1E-2f)

/**
 * User brightness (linear, 0-100) is converted to
 * device-specific brightness using the below parameters:
 *
 * - min: Lower limit of the device brightness, inclusive
 * - max: Upper limit of the device brightness, inclusive
 * - power: Exponential power that controls the brightness curve behaviour:
 *   - 0: Completely linear (straight line between min and max)
 *   - > 0 : Ease in (grows slowly near min, grows fast near max)
 *   - < 0 : Ease out (grows fast near min, grows slowly near max)
 *
 * Larger absolute power values result in a more pronounced exponential behaviour.
 * The output value is guaranteed to always stay between min and max.
 */
typedef struct {
    uint8_t min;
    uint8_t max;
    float power;
} BrightnessCurve;

static const BrightnessCurve front_display_curve = {
    .min = 1,
    .max = 100,
    .power = 0.5f,
};

static const BrightnessCurve back_display_curve = {
    .min = 1,
    .max = 71,
    .power = 1.f,
};

#if defined(SRV_STATUS_LIGHTS)
static const BrightnessCurve status_lights_curve = {
    .min = 5,
    .max = 90,
    .power = 0.f,
};
#endif

static uint8_t brightness_conv_map_curve(const BrightnessCurve* curve, InternalBrightness v) {
    float val_exp;
    const float power = curve->power;

    if(fabsf(power) < BRIGHTNESS_CONV_POWER_MIN) {
        val_exp = v.val;
    } else {
        val_exp = (expf(power * v.val) - 1.f) / (expf(power) - 1.f);
    }

    return roundf(curve->min + (curve->max - curve->min) * val_exp);
}

UserBrightness brightness_conv_int_to_user_clamped(int brightness) {
    brightness = CLAMP(brightness, BRIGHTNESS_MAX, BRIGHTNESS_MIN);
    return (UserBrightness){brightness};
}

InternalBrightness brightness_conv_user_to_internal(UserBrightness v) {
    return (InternalBrightness){(float)v.val / BRIGHTNESS_MAX};
}

UserBrightness brightness_conv_internal_to_user(InternalBrightness v) {
    return (UserBrightness){roundf(v.val * BRIGHTNESS_MAX)};
}

InternalBrightness brightness_conv_light_sensor_to_internal(LightSensorLevel v) {
    return (InternalBrightness){(float)v.val / LIGHT_SENSOR_LIGHT_LEVEL_MAX};
}

FrontDisplayBrightness brightness_conv_internal_to_front(InternalBrightness v) {
    return (FrontDisplayBrightness){brightness_conv_map_curve(&front_display_curve, v)};
}

BackDisplayContrast brightness_conv_internal_to_back(InternalBrightness v) {
    return (BackDisplayContrast){brightness_conv_map_curve(&back_display_curve, v)};
}

#if defined(SRV_STATUS_LIGHTS)
StatusLightsBrightness brightness_conv_internal_to_status(InternalBrightness v) {
    return (StatusLightsBrightness){brightness_conv_map_curve(&status_lights_curve, v)};
}
#endif
