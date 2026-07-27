/**
 * @file js_app_manifest.h
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JsAppManifest JsAppManifest;

JsAppManifest* js_app_manifest_alloc(void);

void js_app_manifest_free(JsAppManifest* instance);

bool js_app_manifest_parse_from_file(JsAppManifest* instance, const char* file_path);

#ifdef __cplusplus
}
#endif
