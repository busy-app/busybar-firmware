/**
 * @file js_app_manifest.h
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JsAppManifest JsAppManifest;

typedef struct {
    const char* id;
    const char* name;
    const char* version;
    const char* descritption;
    const char* author;
    uint32_t heap_size;
    bool is_debug;
} JsAppManifestInfo;

JsAppManifest* js_app_manifest_alloc(void);

void js_app_manifest_free(JsAppManifest* instance);

bool js_app_manifest_parse_from_file(JsAppManifest* instance, const char* file_path);

bool js_app_manifest_get_info(const JsAppManifest* instance, JsAppManifestInfo* info);

#ifdef __cplusplus
}
#endif
