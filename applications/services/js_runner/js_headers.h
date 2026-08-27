#pragma once

#include "js_runner_i.h"

void js_setup_headers(void);

jerry_value_t js_headers_alloc(jerry_value_t response, const char* data, size_t data_size);
