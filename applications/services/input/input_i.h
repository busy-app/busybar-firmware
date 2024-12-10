#pragma once

#include <stdint.h>

// TODO: Unified enums with f64

/* Input Keys */
typedef enum {
    InputRawKeyOk,
    InputRawKeyBack,
    InputRawKeyStart,
    InputRawKeySwitch,
    InputRawKeyEncoder,
} InputRawKey;

typedef enum {
    InputSwitchPositionBusy,
    InputSwitchPositionStatus,
    InputSwitchPositionOff,
    InputSwitchPositionApps,
    InputSwitchPositionSettings,
} InputSwitchPosition;

typedef enum {
    InputButtonActionPress,
    InputButtonActionRelease,
} InputButtonAction;

typedef struct {
    InputRawKey key;
    union {
        InputButtonAction button_action;
        InputSwitchPosition switch_position;
        int16_t encoder_delta;
    };
} InputRawEvent;
