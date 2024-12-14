/**
 * @brief Common declarations for the Input services.
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of device types used for user input.
 */
typedef enum {
    InputDeviceButton, /**< Regular button (can be pressed or released) */
    InputDeviceSwitch, /**< Positional switch (can be set) */
    InputDeviceEncoder, /**< Rotational encoder (can be turned) */
    InputDeviceMAX, /**< Special value for internal use */
} InputDevice;

/**
 * @brief Enumeration of available buttons.
 */
typedef enum {
    InputButtonOk,
    InputButtonBack,
    InputButtonStart,
    InputButtonMAX, /**< Special value for internal use */
} InputButton;

/**
 * @brief Enumeration of possible button acitons.
 */
typedef enum {
    InputActionPress, /**< A button has been pressed */
    InputActionRelease, /**< A button has been released */
} InputAction;

/**
 * @brief Enumeration of possible mode switch positions.
 */
typedef enum {
    InputSwitchPositionBusy,
    InputSwitchPositionStatus,
    InputSwitchPositionOff,
    InputSwitchPositionApps,
    InputSwitchPositionSettings,
    InputSwitchPositionMAX, /**< Special value for internal use */
} InputSwitchPosition;

/**
 * @brief Input event coming from the input controller chip.
 */
typedef struct {
    InputDevice device; /**< Identifier of the device that emitted the event */
    union {
        struct {
            InputButton button; /**< Identifier of the button interacted with */
            InputAction action; /**< Button action that occurred */
        } button_event; /**< Button event */
        InputSwitchPosition switch_position; /**< New mode switch position */
        int16_t encoder_delta; /**< Speed and direction of encoder rotation */
    };
} InputCommonEvent;

#ifdef __cplusplus
}
#endif
