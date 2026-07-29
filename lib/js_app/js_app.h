/**
 * @file js_app.h
 */
#pragma once

#include "js_app_manifest.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JsApp JsApp;

typedef struct {
    const char* front;
    const char* back;
} JsAppIconPath;

typedef struct {
    JsAppIconPath icon;
} JsAppPathInfo;

typedef struct {
    JsAppManifestInfo manifest;
    JsAppPathInfo path;
} JsAppInfo;

JsApp* js_app_alloc(void);

void js_app_free(JsApp* instance);

bool js_app_parse_from_dir(JsApp* instance, const char* dir_path);

bool js_app_get_info(const JsApp* instance, JsAppInfo* info);

#ifdef __cplusplus
}
#endif
