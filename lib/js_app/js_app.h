/**
 * @file js_app.h
 */
#pragma once

#include "js_app_common.h"
#include "js_app_manifest.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JsApp JsApp;

JsApp* js_app_alloc(void);

void js_app_free(JsApp* instance);

bool js_app_parse_from_dir(JsApp* instance, const char* dir_path);

bool js_app_get_info(const JsApp* instance, JsAppInfo* info);

const JsAppManifest* js_app_get_manifest(const JsApp* instance);

#ifdef __cplusplus
}
#endif
