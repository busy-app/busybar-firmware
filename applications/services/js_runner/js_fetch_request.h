#pragma once

#include <lib/fetch/fetch.h>
#include <lib/jerryscript/amalgam/jerryscript.h>

jerry_value_t js_fetch_request_parse(jerry_value_t obj, FetchRequest* request);

void js_fetch_request_clear(FetchRequest* request);
