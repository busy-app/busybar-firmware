/**
 * @brief Common declarations for the Input services.
 */
#pragma once

#include "input_common.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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
