#pragma once

#include "common_vals.h"

#include <stdint.h>
#include <stdbool.h>
#include <core/string.h>
#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UpdateManifest UpdateManifest; // Opaque type declaration
typedef enum {
    UpdateManifestPathStage,
    UpdateManifestPathResources,
    UpdateManifestPathSilFw,
    UpdateManifestPathSilRadioFw,
    UpdateManifestPathDfu,
} UpdateManifestPath;

uint32_t updater_manifest_get_updater_stage_crc32(const UpdateManifest* config);
const FuriString*
    updater_manifest_get_path(const UpdateManifest* config, UpdateManifestPath field);
uint8_t updater_manifest_get_target(const UpdateManifest* config);

UpdateManifest* updater_manifest_alloc(void);
void updater_manifest_free(UpdateManifest* config);
void updater_manifest_prefix_paths(UpdateManifest* config, const char* prefix);
bool updater_manifest_init_from_memory(
    UpdateManifest* config,
    const char* json_data,
    size_t json_size);
bool updater_manifest_init_from_file(UpdateManifest* config, File* file);

#ifdef __cplusplus
}
#endif
