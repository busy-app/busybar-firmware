/**
 * @file js_app_common.h
 */
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* name;
    bool is_debug;
} JsAppInfo;

#ifdef __cplusplus
}
#endif
