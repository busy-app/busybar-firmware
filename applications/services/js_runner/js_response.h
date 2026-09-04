#pragma once

#include "js_runner_i.h"

#include <toolbox/slice.h>

void js_setup_response(void);

jerry_value_t js_response_alloc(uint32_t status, StringSlice status_text);
