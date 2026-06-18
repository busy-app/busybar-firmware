/**
 * @file busy.h
 * @brief BUSY application -- frontend for the @ref BusyTimer service.
 */
#pragma once

#include "busy_common.h"

/**
 * @brief The string key for BusyApp instance access
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_BUSY_APP);`
 *
 * @note The record is only available if the BUSY app is running.
 */
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

/**
 * @brief Enumeration of possible statuses returned by the BusyApp APIs.
 */
typedef enum {
    BusyStatusOk, /**< Command executed normally, no error occurred */
    BusyStatusAborted, /**< Command was aborted (e.g. when app was exiting) */
    BusyStatusTimeout, /**< Command was taking too long to execute */
} BusyStatus;

/**
 * @brief BusyApp opaque type.
 */
typedef struct BusyApp BusyApp;

/**
 * @brief Set application configuration (e.g. theme, Smart home behaviour)
 *
 * It is safe to delete the value pointed to by
 * @p config after this function has returned.
 *
 * @param[in,out] instance pointer to the BusyApp instance
 * @param[in] config pointer to the structure containing the configuration
 *
 * @returns @c BusyStatusOk on success, any other value from @ref BusyStatus on failure
 */
BusyStatus busy_set_config(BusyApp* instance, const BusyAppConfig* config);

/**
 * @brief Force the running app to transition to the timer scene.
 *
 * This function is blocking, i.e. it only returns when the transition has been completed.
 *
 * @param[in,out] instance pointer to the BusyApp instance
 *
 * @returns @c BusyStatusOk on success, any other value from @ref BusyStatus on failure
 */
BusyStatus busy_show_timer(BusyApp* instance);

/**
 * @brief Request the running app to exit.
 *
 * Depending on the way the app was started, it will result in different behaviour:
 *
 * - If the app was started in regular or custom mode, it will return to the start scene.
 * - If the app was started in the timer mode, it will exit.
 *
 * This function is nonblocking (will return sooner than the command will actually execute).
 *
 * @param[in,out] instance pointer to the BusyApp instance
 *
 * @returns @c BusyStatusOk on success, any other value from @ref BusyStatus on failure
 */
BusyStatus busy_request_exit(BusyApp* instance);

#ifdef __cplusplus
}
#endif
