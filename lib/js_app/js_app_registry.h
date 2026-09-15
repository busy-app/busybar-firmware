/**
 * @file js_app_registry.h
 * @brief Facilities for querying installed JavaScript applications.
 *
 * @see @ref js_app.h for more info on JavaScript apps.
 */
#pragma once

#include "js_app.h"
#include <furi/core/string.h>

#define JS_APP_ID_LEN_MAX 32

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Type for a callback to be invoked for each JavaScript application found.
 *
 * @warning The `info` parameter is valid ONLY inside the callback function.
 *          If the calling code needs it for further processing, it must make
 *          a deep copy of the data.
 *
 * @param[in] info pointer to an application info structure associated with an app found
 * @param[in,out] context pointer to a user-specific object
 */
typedef void (*JsAppRegistryListCallback)(const JsAppInfo* info, void* context);

/**
 * @brief List all installed JavaScript applications.
 *
 * @param[in] callback pointer to the function to be called for each application found
 * @param[in,out] context pointer to a user-specific object (will be passed to the callback)
 */
void js_app_registry_list_apps(JsAppRegistryListCallback callback, void* context);

/**
 * @brief Get a JavaScript application by its ID.
 *
 * @note If the call succeeds, the user code is responsible for deleting
 *       the return value via `js_app_free()` when done with it.
 *
 * @param[in] app_id zero-terminated string containing the desired application's ID
 * @returns pointer to a JsApp instance associated with the found app, or @c NULL on failure
 */
JsApp* js_app_registry_get_app(const char* app_id);

/**
 * @brief Validate app ID and get installation path for a JavaScript application by its ID.
 *
 * Valid app IDs correspond to the following regular expression: [a-zA-Z0-9_\-][a-zA-Z0-9_\-.]*
 *
 * @param[in] app_id zero-terminated string containing the desired application's ID
 * @return path in filesystem or @c NULL if app_id is invalid
 */
FuriString* js_app_registry_get_app_path(const char* app_id);

/**
 * @brief Check an application ID is valid.
 *
 * Valid application IDs correspond to the following regular expression:
 * ^[a-zA-Z0-9_\-][a-zA-Z0-9_\-.]*$
 *
 * @param[in] app_id zero-terminated string to be validated
 * @return true if app_id is a valid application ID
 */
bool js_app_registry_validate_app_id(const char* app_id);

#ifdef __cplusplus
}
#endif
