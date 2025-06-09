#pragma once

#include "common_vals.h"

#include <stdint.h>
#include <stdbool.h>
#include <core/string.h>
#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UpdateManifest UpdateManifest;
typedef enum {
    UpdateManifestPathStage,
    UpdateManifestPathResources,
    UpdateManifestPathSilFw,
    UpdateManifestPathSilRadioFw,
    UpdateManifestPathDfu,
} UpdateManifestPath;

/**
 * @brief Manifest version.
 */
#define UPDATE_MANIFEST_VERSION (1)

/**
 * @brief Allocates a new UpdateManifest structure.
 * @return Pointer to the allocated UpdateManifest, or NULL on failure.
 */
UpdateManifest* updater_manifest_alloc(void);

/**
 * @brief Frees an UpdateManifest structure.
 * @param config Pointer to the UpdateManifest to free.
 */
void updater_manifest_free(UpdateManifest* config);

/**
 * @brief Prefixes all relevant paths within the manifest with a given prefix.
 *        This is typically used to make paths absolute to a new base.
 * @param config Pointer to the UpdateManifest to modify.
 * @param prefix The string prefix to add to the paths.
 */
void updater_manifest_prefix_paths(UpdateManifest* config, const char* prefix);

/**
 * @brief Initializes an UpdateManifest from a JSON string in memory.
 * @param config Pointer to the UpdateManifest to initialize.
 * @param json_data Pointer to the character buffer containing JSON data.
 * @param json_size Size of the JSON data in bytes.
 * @return True if initialization was successful, false otherwise.
 */
bool updater_manifest_init_from_memory(
    UpdateManifest* config,
    const char* json_data,
    size_t json_size);

/**
 * @brief Initializes an UpdateManifest from a JSON file.
 * @param config Pointer to the UpdateManifest to initialize.
 * @param file Pointer to an open File object representing the JSON manifest file.
 * @return True if initialization was successful, false otherwise.
 */
bool updater_manifest_init_from_file(UpdateManifest* config, File* file);

/**
 * @brief Gets the CRC32 checksum of the updater stage binary, as specified in the manifest.
 * @param config Pointer to the UpdateManifest.
 * @return The CRC32 checksum.
 */
uint32_t updater_manifest_get_updater_stage_crc32(const UpdateManifest* config);

/**
 * @brief Gets a specific path from the manifest.
 * @param config Pointer to the UpdateManifest.
 * @param field Enum value specifying which path to retrieve (e.g., stage, DFU).
 * @return Constant pointer to a FuriString containing the path, or NULL if not found/applicable.
 */
const FuriString*
    updater_manifest_get_path(const UpdateManifest* config, UpdateManifestPath field);

/**
 * @brief Gets the target hardware identifier from the manifest.
 * @param config Pointer to the UpdateManifest.
 * @return The target hardware identifier.
 */
uint8_t updater_manifest_get_target(const UpdateManifest* config);

/**
 * @brief Gets the version of the manifest file format.
 * @param config Pointer to the UpdateManifest.
 * @return The manifest version.
 */
uint32_t updater_manifest_get_version(const UpdateManifest* config);

#ifdef __cplusplus
}
#endif
