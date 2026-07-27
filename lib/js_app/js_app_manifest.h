/**
 * @file js_app_manifest.h
 */
#pragma once

#include "js_app_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JsAppManifest JsAppManifest;

typedef struct {
    const char* name;
    const char* descritption;
    const char* author;
    JsAppVersion version;
    bool is_debug;
} JsAppManifestInfo;

JsAppManifest* js_app_manifest_alloc(void);

void js_app_manifest_free(JsAppManifest* instance);

bool js_app_manifest_parse_from_file(JsAppManifest* instance, const char* file_path);

bool js_app_manifest_get_info(const JsAppManifest* instance, JsAppManifestInfo* info);

#ifdef __cplusplus
}
#endif
