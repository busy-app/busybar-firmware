#include "brightness_conv.h"
#include <assert.h>
#include <furi/core/core_defines.h>

#define INTERNAL_BRIGHTNESS_MAX 10

/* BarMetal: the stock table was {25, 25, 28, ...}: brightness 0 did NOT turn the front
   display off (it sat at 25 %), and steps 0 and 1 were identical, so every user value
   from 0 to ~14 looked the same. That made the documented 0-100 control appear inert.
   Step 0 is now truly off and the low end is spread out. Auto-brightness never uses
   step 0 (see brightness_conv_light_sensor_to_internal), so a dark room does not
   black out the display. */
static const uint8_t brightness_conv_front_table[] = {0, 12, 18, 24, 31, 39, 48, 58, 70, 84, 100};

// Exponential curve: y = k * b ^ x
// b = 7.143377489
// k = 1.324264735
// static const uint8_t brightness_conv_back_table[] = {7, 9, 12, 16, 21, 29, 38, 51, 67, 89, 118};

// k = 0.6 * 1.324264735
/* BarMetal: step 0 turns the back display off to match the front table. */
static const uint8_t brightness_conv_back_table[] = {0, 5, 7, 9, 13, 17, 23, 30, 40, 53, 71};

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
    /* BarMetal: clamp auto-brightness to at least one step so that "off" stays a
       deliberate manual choice and a dark room never blanks the display. */
    return (InternalBrightness){MAX(v.val, (uint8_t)1)};
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
