#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <toolbox/update_lib/update_manifest.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UpdateConfigValidationOK,
    UpdateConfigValidationManifestPathInvalid,
    UpdateConfigValidationManifestFolderNotFound,
    UpdateConfigValidationManifestInvalid,
    UpdateConfigValidationStageMissing,
    UpdateConfigValidationStageIntegrityError,
    UpdateConfigValidationManifestPointerCreateError,
    UpdateConfigValidationManifestPointerCheckError,
    UpdateConfigValidationTargetMismatch,
    UpdateConfigValidationOutdatedManifestVersion,
    UpdateConfigValidationUnspecifiedError,
} UpdateConfigValidation;

// Opaque updater state struct
typedef struct UpdateConfig UpdateConfig;

/**
 * @brief Allocate and initialize updater configuration.
 * This function allocates memory for the UpdateConfig structure and initializes its members.
 * The caller is responsible for freeing the allocated memory using update_config_free.
 * @return Pointer to the allocated UpdateConfig structure
 */
UpdateConfig* update_config_alloc(void);

/**
 * @brief Free updater configuration and associated resources.
 * This function frees the memory allocated for the UpdateConfig structure and any
 * resources it holds (e.g., the loaded manifest).
 * @param state Pointer to the UpdateConfig structure to free
 */
void update_config_free(UpdateConfig* state);

/**
 * @brief Load and validate the update configuration from a manifest file.
 * This function reads an update manifest file from the given path, parses it,
 * and validates its contents. Validation includes checking the target hardware,
 * manifest version, and the integrity of the stage file (existence and CRC).
 * The manifest base path is also set within the UpdateConfig state.
 * @param state Pointer to the UpdateConfig structure to populate.
 * @param update_manifest_path Full path to the update manifest file (e.g., "/ext/my_update/update.json").
 * @return UpdateConfigValidation enum value indicating the result of the operation.
 *         UpdateConfigValidationOK on success, or an error code on failure.
 */
UpdateConfigValidation update_config_load(UpdateConfig* state, const char* update_manifest_path);

/**
 * @brief Get a pointer to the loaded update manifest.
 * This function returns a constant pointer to the UpdateManifest structure
 * contained within the UpdateConfig state. The manifest should have been loaded
 * previously by calling update_config_load_config.
 * @param state Pointer to the UpdateConfig structure.
 * @return Constant pointer to the UpdateManifest
 */
const UpdateManifest* update_config_get_manifest(const UpdateConfig* state);

/**
 * @brief Gets a string description for an UpdateConfigValidation error code.
 * @param error_code The UpdateConfigValidation error code.
 * @return A constant string describing the error.
 */
const char* update_config_validation_get_error_str(UpdateConfigValidation error_code);

/**
 * @brief Writes the provided manifest path to the update pointer file.
 * This function creates or overwrites the update pointer file with the full
 * path to the update manifest.
 * @param storage Pointer to the Storage instance.
 * @param manifest_path The full path to the update manifest file to be written.
 * @return True if the path was successfully written to the pointer file, false otherwise.
 */
bool update_config_write_pointer_file(Storage* storage, const char* manifest_path);

/**
 * @brief Reads the manifest path from the update pointer file.
 * This function reads the content of the update pointer file into the provided FuriString.
 * @param storage Pointer to the Storage instance.
 * @param manifest_path_output Pointer to a FuriString where the read path will be stored.
 *                             The string will be cleared if the file cannot be read or is empty.
 * @return True if the path was successfully read and is not empty, false otherwise.
 */
bool update_config_read_pointer_file(Storage* storage, FuriString* manifest_path_output);

#ifdef __cplusplus
}
#endif
