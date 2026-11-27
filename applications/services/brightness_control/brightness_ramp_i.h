#pragma once

#include "brightness_conv.h"
#include <stdbool.h>

#define BRIGHTNESS_RAMP_STEP (1.f / 128.f)

typedef struct {
    InternalBrightness current;
    InternalBrightness target;
    bool is_active;
} BrightnessRamp;

static inline void brightness_ramp_set(BrightnessRamp* instance, InternalBrightness value) {
    instance->current = value;
    instance->target = value;
    instance->is_active = false;
}

static inline bool brightness_ramp_target(BrightnessRamp* instance, InternalBrightness target) {
    const bool has_changed = target.val != instance->target.val;
    if(has_changed) {
        instance->target = target;
        instance->is_active = target.val != instance->current.val;
    }
    return has_changed;
}

static inline void brightness_ramp_step(BrightnessRamp* instance) {
    if(instance->is_active) {
        const float remaining = instance->target.val - instance->current.val;
        if(remaining > BRIGHTNESS_RAMP_STEP) {
            instance->current.val += BRIGHTNESS_RAMP_STEP;
        } else if(remaining < -BRIGHTNESS_RAMP_STEP) {
            instance->current.val -= BRIGHTNESS_RAMP_STEP;
        } else {
            instance->current = instance->target;
            instance->is_active = false;
        }
    }
}
