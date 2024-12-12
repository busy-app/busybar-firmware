#pragma once

#include <stdint.h>

typedef enum {
    InputDeviceButton,
    InputDeviceSwitch,
    InputDeviceEncoder,
    InputDeviceMAX,
} InputDevice;

typedef enum {
    InputButtonOk,
    InputButtonBack,
    InputButtonStart,
    InputButtonMAX,
} InputButton;

typedef enum {
    InputActionPress,
    InputActionRelease,
} InputAction;

typedef enum {
    InputSwitchPositionBusy,
    InputSwitchPositionStatus,
    InputSwitchPositionOff,
    InputSwitchPositionApps,
    InputSwitchPositionSettings,
    InputSwitchPositionMAX,
} InputSwitchPosition;

typedef struct {
    InputDevice device;
    union {
        struct {
            InputButton button;
            InputAction action;
        } button_event;
        InputSwitchPosition switch_position;
        int16_t encoder_delta;
    };
} InputCommonEvent;
