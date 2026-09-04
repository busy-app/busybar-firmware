#pragma once

#include "js_runner_i.h"

#include <toolbox/slice.h>

void js_setup_headers(void);

jerry_value_t js_headers_alloc(StringSlice headers_text);
