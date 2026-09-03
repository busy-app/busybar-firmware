#pragma once
#include "js_runner_i.h"

void js_setup_request(void);
jerry_value_t js_request_construct(jerry_value_t url, jerry_value_t init);
