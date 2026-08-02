/**
 * @file js_app.h
 * @brief JavaScript application library.
 *
 * A JS application type contains the entirety of the information
 * that is needed to properly display and run a JavaScript application.
 *
 * The JS application is loaded from a directory with the following structure:
 * ```
 * + app_id
 * |
 * |-+ appmeta
 * | |- manifest.json
 * | |- config.json
 * | |- icon_front_8x8.png
 * | |- icon_back_11x11.png
 * |
 * |-+ scripts
 * | |- main.js
 * | |- module1.js
 * | |- module2.js
 * |
 * |-+ resources
 *   |- smile.png
 *   |- alert.snd
 *   |- spinner.anim
 * ```
 *
 * The only two required files are:
 *
 * - `appmeta/manifest.json` - The application manifest.
 * - `scripts/main.js` - The application entry point script.
 *
 * Additionally, the top-level directory name MUST be
 * the same as the `id` field in `manifest.json`.
 *
 * All sub-folders besides `appmeta` and `scripts` are free-form
 * and may or may not be present depending on the application.
 *
 * @see @ref js_app_manifest.h for more info on JS application manifests.
 * @see @ref javascript-applications for more info on JS applications.
 */
#pragma once

#include "js_app_manifest.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief JsApp opaque type.
 */
typedef struct JsApp JsApp;

/**
 * @brief Application icon path descriptor structure.
 *
 * All string fields are zero-terminated C strings.
 */
typedef struct {
    const char* front; /**< Path to the icon to be shown on Front display */
    const char* back; /**< Path to the icon to be shown on Back display */
} JsAppIconPath;

/**
 * @brief Application paths descriptor structure.
 *
 * All string fields are zero-terminated C strings.
 */
typedef struct {
    const char* entry; /**< Path to the entry JS script file (`main.js`) */
    JsAppIconPath icon; /**< Application icon paths descriptor */
} JsAppPathInfo;

/**
 * @brief JsApp information structure.
 *
 * The data pointers are guaranteed to remain valid until
 * the application is loaded from directory again or is deleted.
 */
typedef struct {
    JsAppManifestInfo manifest; /**< JS application manifest contents */
    JsAppPathInfo path; /**< JS application paths descriptor */
} JsAppInfo;

/**
 * @brief Create a JsApp instance.
 *
 * @returns pointer to the created instance
 */
JsApp* js_app_alloc(void);

/**
 * @brief Delete a JsApp instance.
 *
 * @warning All @ref JsAppInfo objects associated with the instance
 *          will become invalid after a call to this function.
 *
 * @param[in,out] instance pointer to the instance to be deleted
 */
void js_app_free(JsApp* instance);

/**
 * @brief Load a JsApp instance from a special directory structure.

 * @warning All @ref JsAppInfo objects associated with the instance
 *          will become invalid after a call to this function.
 *
 * @param[in,out] instance pointer to the instance to be loaded
 * @param[in] dir_path zero-terminated string containing the path to the directory
 * @returns @c true if the application was loaded successfully, @c false otherwise
 */
bool js_app_load_from_directory(JsApp* instance, const char* dir_path);

/**
 * @brief Get the application information.
 *
 * @param[in] instance pointer to the instance to be queried
 * @param[out] info pointer to the output structure to be filled out with info (must be allocated)
 * @returns @c true if valid information was present, @c false otherwise
 */
bool js_app_get_info(const JsApp* instance, JsAppInfo* info);

#ifdef __cplusplus
}
#endif
