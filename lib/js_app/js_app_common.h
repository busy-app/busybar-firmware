/**
 * @file js_app_common.h
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} JsAppVersion;

#ifdef __cplusplus
}
#endif
