/**
 * @file js_app_launcher.h
 * @brief JS Application Launcher public API.
 */
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Possible modes for launching JS applications
 */
typedef enum {
    JsAppLauncherModeNormal, /**< Normal mode, the start menu will be shown always */
    JsAppLauncherModeSkipMenu, /**< Skip the start menu on startup, show before exit */
    JsAppLauncherModeMax, /**< Special value, internal use */
} JsAppLauncherMode;

/**
 * @brief Start a JS application by its application ID.
 *
 * @param[in] app_id zero-terminated string containing the ID of the app to be started
 * @param[in] mode mode to start the JS application in
 * @returns @c true if the app could be started, @c false otherwise
 */
bool js_app_launcher_start(const char* app_id, JsAppLauncherMode mode);

/**
 * @brief Stop the currently running JS application, if any
 *
 * @returns @c true if a JS app was running and could be stopped, @c false otherwise
 */
bool js_app_launcher_stop(void);

#ifdef __cplusplus
}
#endif
