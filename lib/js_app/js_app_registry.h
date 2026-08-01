/**
 * @file js_app_registry.h
 * @brief Facilities for querying installed JavaScript applications.
 *
 * @see @ref js_app.h for more info on JavaScript apps.
 */
#pragma once

#include "js_app.h"

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

#ifdef __cplusplus
}
#endif
