/**
 * @file http_headers.h
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "slice.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t status;
    StringSlice status_text;
    StringSlice headers;
} HttpResponse;

bool http_response_parse(HttpResponse* instance, const char* data, size_t data_len);

#ifdef __cplusplus
}
#endif
