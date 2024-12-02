/**
 * @file input.h
 * Input: main API
 */
#pragma once

#include <furi_hal_resources.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Input Types */
typedef enum {
    InputTypePress, /**< Press event, emitted after debounce */
    InputTypeRelease, /**< Release event, emitted after debounce */
    InputTypeMAX, /**< Special value for exceptional */
} InputType;

/** Input Event*/
typedef struct {
    InputKey key;
    union {
        InputType type;
        InputSwitchPosition position;
        int16_t delta;
    };
} InputEvent;

#ifdef __cplusplus
}
#endif
