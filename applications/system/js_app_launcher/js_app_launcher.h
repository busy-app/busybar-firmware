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
 * @brief Status codes returned by the JsApplication APIs.
 */
typedef enum {
    JsAppLauncherStatusOk, /**< Operation successful, no error reported */
    JsAppLauncherStatusTimeout, /**< Operation timed out */
    JsAppLauncherStatusNotRunning, /**< Js application is not running */
    JsAppLauncherStatusInvalidAppId, /**< Application ID is invalid */
} JsAppLauncherStatus;

/**
 * @brief Modes for starting JS applications.
 */
typedef enum {
    JsAppLauncherStartModeShowMenu, /**< Always show start menu */
    JsAppLauncherStartModeResume, /**< Skip start menu on startup, show before exit */
    JsAppLauncherStartModeMax, /**< Special value, internal use */
} JsAppLauncherStartMode;

/**
 * @brief Modes for stopping JS applications.
 */
typedef enum {
    JsAppLauncherStopModeNormal, /**< Simply exit from the application */
    JsAppLauncherStopModeForget, /**< Exit and request AppsMenu to forget the most recent app */
    JsAppLauncherStopModeMax, /**< Special value, internal use */
} JsAppLauncherStopMode;

/**
 * @brief Start a JS application by its application ID.
 *
 * @param[in] app_id zero-terminated string containing the ID of the app to be started
 * @param[in] start_mode mode to be used to start the JS application
 * @returns @c JsAppLauncherStatusOk the application was successfully launched
 * @returns @c JsAppLauncherStatusError failed to launch the application
 */
JsAppLauncherStatus js_app_launcher_start(const char* app_id, JsAppLauncherStartMode start_mode);

/**
 * @brief Stop the currently running JS application, if applicable.
 *
 * The start menu will not be shown upon exit.
 *
 * @param[in] stop_mode mode to be used to exit from the JS application
 * @returns @c true if a JS app was running and could be stopped, @c false otherwise
 */
JsAppLauncherStatus js_app_launcher_stop(JsAppLauncherStopMode stop_mode);

#ifdef __cplusplus
}
#endif
