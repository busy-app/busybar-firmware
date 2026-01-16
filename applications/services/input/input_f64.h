/**
 * @file input.h
 * Input: f64 side API
 */
#pragma once

#include "input_common_f64.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_INPUT        "input"
#define RECORD_INPUT_EVENTS "input_events"

// TODO [FW-503]: replace this w/ furi_state
/**
 * @brief Gets the state of all absolute controls, i.e. the buttons and the mode
 *        switch
 * 
 * @param [in] input Input instance
 * 
 * @returns State of all absolute controls
 */
InputAbsoluteState FURI_DEPRECATED input_get_absolute_state(Input* instance);

#ifdef __cplusplus
}
#endif
