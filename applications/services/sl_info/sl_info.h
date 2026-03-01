/**
 * @file sl_info.h
 * @brief
 */
#pragma once

#include <toolbox/property.h>

#define RECORD_SL_INFO "sl_info"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SlInfo SlInfo;

const char* sl_info_get_value(const SlInfo* instance, const char* key);

bool sl_info_get_values(const SlInfo* instance, PropertyValueCallback callback, void* context);

#ifdef __cplusplus
}
#endif
