/**
 * @brief Common declarations for the Input services.
 */
#pragma once

#include <furi.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Input Input;

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
 * @brief Bitmask of available buttons.
 */
typedef enum {
    InputButtonMaskOk = (1 << InputButtonOk),
    InputButtonMaskBack = (1 << InputButtonBack),
    InputButtonMaskStart = (1 << InputButtonStart),
} InputButtonMask;

/**
 * @brief Enumeration of possible button actions.
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
 * @brief State of all absolute controls (i.e. buttons and the mode switch)
 */
typedef struct {
    InputButtonMask buttons; /**< Bitmask of buttons that are currently pressed */
    InputSwitchPosition switch_position; /**< Current mode switch position */
} InputAbsoluteState;

#ifdef __cplusplus
}
#endif
