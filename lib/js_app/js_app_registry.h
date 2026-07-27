/**
 * @file js_app_registry.h
 */
#pragma once

#include "js_app_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*JsAppRegistryListCallback)(const JsAppInfo* info, void* context);

void js_app_registry_list_apps(JsAppRegistryListCallback callback, void* context);

#ifdef __cplusplus
}
#endif
