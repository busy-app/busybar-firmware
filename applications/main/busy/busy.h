#pragma once

#define RECORD_BUSY_APP "busy_app"

/**
 * @brief Special argument to launch in timer mode.
 *
 * Passing this string as the application argument
 * indicates that the app was run by the BUSY timer.
 *
 * In this mode, the app will go directly to the
 * BusyAppSceneIdTimer scene and will exit if "back"
 * is pressed on any of the following scenes:
 * - BusyAppSceneIdTimer,
 * - BusyAppSceneIdNext,
 * - BusyAppSceneIdProgress.
 */
#define BUSY_APP_TIMER_MODE "timer"

/**
 * @brief Special argument to launch in Custom mode.
 *
 * Passing this string as the application argument
 * indicates that the app is to be run in Custom mode.
 *
 * In this mode, the app will show different graphics
 * and additionally, use a different settings profile.
 */
#define BUSY_APP_CUSTOM_MODE "custom"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BusyApp BusyApp;

void busy_show_timer(BusyApp* instance);

void busy_request_exit(BusyApp* instance);

#ifdef __cplusplus
}
#endif
