/**
 * @file input.h
 * Input: main API
 */
#pragma once

#include "input_common.h"

#include <furi_hal_resources.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_INPUT        "input"
#define RECORD_INPUT_EVENTS "input_events"

#define INPUT_SEQUENCE_SOURCE_HARDWARE (0U)
#define INPUT_SEQUENCE_SOURCE_SOFTWARE (1U)

/** Input Types
 * Some of them are physical events and some logical
 */
typedef enum {
    InputTypePress, /**< Press event, emitted after debounce */
    InputTypeRelease, /**< Release event, emitted after debounce */
    InputTypeShort, /**< Short event, emitted after InputTypeRelease done within INPUT_LONG_PRESS interval */
    InputTypeLong, /**< Long event, emitted after INPUT_LONG_PRESS_COUNTS interval, asynchronous to InputTypeRelease  */
    InputTypeRepeat, /**< Repeat event, emitted with INPUT_LONG_PRESS_COUNTS period after InputTypeLong event */
    InputTypeMAX, /**< Special value, internal use */
} InputType;

/** Input Event, dispatches with FuriPubSub */
typedef struct {
    InputKey key;
    InputType type;
    uint8_t sequence_source;
    uint32_t sequence_number;
} InputEvent;

/** Get human readable input key name
 * @param key - InputKey
 * @return string
 */
const char* input_get_key_name(InputKey key);

/** Get human readable input type name
 * @param type - InputType
 * @return string
 */
const char* input_get_type_name(InputType type);

/**
 * @brief Set the pressed state of a key
 * 
 * @param [in] input Input instance
 * @param [in] key   Key
 */
void input_key_press(Input* input, InputKey key);

/**
 * @brief Set the released state of a key
 * 
 * @param [in] input Input instance
 * @param [in] key   Key
 */
void input_key_release(Input* input, InputKey key);

/**
 * @brief Toggle the state of a key
 *
 * Set the pressed and the released state of a key in a quick succession
 *
 * @param [in] input Input instance
 * @param [in] key   Key
 */
void input_key_toggle(Input* input, InputKey key);

/**
 * @brief Get the reference to a FuriState with the mode switch position.
 * 
 * The FuriState stores a `InputSwitchPosition` type. `InputSwitchPositionMAX`
 * means that state is unknown yet.
 * 
 * @param [in] input Input instance
 * @returns `FuriState` as described above.
 */
FuriState* input_get_switch_pos(Input* input);

#ifdef __cplusplus
}
#endif
