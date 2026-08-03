/**
 * @file js_app_manifest.h
 * @brief JavaScript application manifest library.
 *
 * A JS application manifest contains the properties
 * necessary to properly display and run an application.
 *
 * The manifest file is stored using the following JSON format:
 *
 * ```json
 * {
 *     "format_version": 1,
 *     "id": "app_id",
 *     "name": "Free form app name",
 *     "version": "1.2.3",
 *     "description": "Free form description", // Optional, default: ""
 *     "author": "Application author",         // Optional, default: ""
 *     "heap_size_kib": 32,                    // Optional, default: 32
 *     "debug": false                          // Optional, default: false
 * }
 * ```
 *
 * Fields marked as optional may be omitted, in which case
 * they will be assigned their respective default values.
 *
 * @see @ref javascript-applications for more info on the application manifests.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief JsAppManifest opaque type.
 */
typedef struct JsAppManifest JsAppManifest;

/**
 * @brief JsAppManifest information structure.
 *
 * All string fields are zero-terminated C strings.
 *
 * The data pointers are guaranteed to remain valid until
 * the manifest is loaded from file again or is deleted.
 */
typedef struct {
    const char* id; /**< Unique application identifier */
    const char* name; /**< Display name (e.g. will show up in menus) */
    const char* version; /**< Application version in semver format (x.y.z) */
    const char* description; /**< Application description (free form) */
    const char* author; /**< Application author (free form) */
    uint32_t heap_size; /**< Heap size for the JS process, in bytes */
    bool is_debug; /**< If @c true, hide the application when not in developer mode */
} JsAppManifestInfo;

/**
 * @brief Create a JsAppManifest instance.
 *
 * @returns pointer to the created instance
 */
JsAppManifest* js_app_manifest_alloc(void);

/**
 * @brief Delete a JsAppManifest instance.
 *
 * @warning All @ref JsAppManifestInfo objects associated with the instance
 *          will become invalid after a call to this function.
 *
 * @param[in,out] instance pointer to the instance to be deleted
 */
void js_app_manifest_free(JsAppManifest* instance);

/**
 * @brief Load a JsAppManifest instance from a JSON description file.
 *
 * @warning All @ref JsAppManifestInfo objects associated with the instance
 *          will become invalid after a call to this function.
 *
 * @param[in,out] instance pointer to the instance to be loaded
 * @param[in] file_path zero-terminated string containing the path to the manifest file
 * @returns @c true if the manifest was loaded successfully, @c false otherwise
 */
bool js_app_manifest_load_from_file(JsAppManifest* instance, const char* file_path);

/**
 * @brief Get the manifest information (its contents).
 *
 * @param[in] instance pointer to the instance to be queried
 * @param[out] info pointer to the output structure to be filled out with info (must be allocated)
 * @returns @c true if valid information was present, @c false otherwise
 */
bool js_app_manifest_get_info(const JsAppManifest* instance, JsAppManifestInfo* info);

#ifdef __cplusplus
}
#endif
