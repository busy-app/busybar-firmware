#pragma once

#include "js_fetch.h"

jerry_value_t js_fetch_array_buffer(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count);

jerry_value_t js_fetch_blob(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count);

jerry_value_t js_fetch_bytes(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count);

jerry_value_t js_fetch_form_data(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count);

jerry_value_t js_fetch_json(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count);

jerry_value_t js_fetch_text(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count);
