#include "brightness_conv.h"
#include <assert.h>
#include <furi/core/core_defines.h>

#define INTERNAL_BRIGHTNESS_MAX 10

static const uint8_t brightness_conv_front_table[] = {25, 25, 28, 31, 37, 43, 52, 61, 73, 85, 100};

// Exponential curve: y = k * b ^ x
// b = 7.143377489
// k = 1.324264735
// static const uint8_t brightness_conv_back_table[] = {7, 9, 12, 16, 21, 29, 38, 51, 67, 89, 118};

// k = 0.6 * 1.324264735
static const uint8_t brightness_conv_back_table[] = {4, 5, 7, 9, 13, 17, 23, 30, 40, 53, 71};

static_assert(COUNT_OF(brightness_conv_front_table) == INTERNAL_BRIGHTNESS_MAX + 1);
static_assert(COUNT_OF(brightness_conv_back_table) == INTERNAL_BRIGHTNESS_MAX + 1);

#if defined(SRV_STATUS_LIGHTS)
static const uint8_t brightness_conv_status_table[] = {5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 90};
static_assert(COUNT_OF(brightness_conv_status_table) == INTERNAL_BRIGHTNESS_MAX + 1);
#endif

static_assert(BRIGHTNESS_MIN == 0);
static_assert(LIGHT_SENSOR_LIGHT_LEVEL_MIN == 0);
static_assert(LIGHT_SENSOR_LIGHT_LEVEL_MAX == INTERNAL_BRIGHTNESS_MAX);

UserBrightness brightness_conv_int_to_user_clamped(int brightness) {
    brightness = CLAMP(brightness, BRIGHTNESS_MAX, BRIGHTNESS_MIN);
    return (UserBrightness){brightness};
}

InternalBrightness brightness_conv_user_to_internal(UserBrightness v) {
    return (InternalBrightness){v.val * INTERNAL_BRIGHTNESS_MAX / BRIGHTNESS_MAX};
}

UserBrightness brightness_conv_internal_to_user(InternalBrightness v) {
    return (UserBrightness){v.val * BRIGHTNESS_MAX / INTERNAL_BRIGHTNESS_MAX};
}

InternalBrightness brightness_conv_light_sensor_to_internal(LightSensorLevel v) {
    return (InternalBrightness){v.val};
}

FrontDisplayBrightness brightness_conv_internal_to_front(InternalBrightness v) {
    return (FrontDisplayBrightness){brightness_conv_front_table[v.val]};
}

BackDisplayContrast brightness_conv_internal_to_back(InternalBrightness v) {
    return (BackDisplayContrast){brightness_conv_back_table[v.val]};
}

#if defined(SRV_STATUS_LIGHTS)
StatusLightsBrightness brightness_conv_internal_to_status(InternalBrightness v) {
    return (StatusLightsBrightness){brightness_conv_status_table[v.val]};
}
#endif
