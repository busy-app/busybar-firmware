#pragma once

#include "js_fetch.h"

typedef struct JsReadableStream JsReadableStream;

jerry_value_t js_readable_stream_alloc(JsFetch* parent);
