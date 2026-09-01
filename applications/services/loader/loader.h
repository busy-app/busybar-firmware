/**
 * @file loader.h
 * @brief Loader service API.
 *
 * @see @ref loader for more information.
 */
#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup loader Loader service
 *
 * Loader is a system @ref services "service" that
 * loads and manages @ref applications "applications".
 *
 * @{
 */

#define RECORD_LOADER            "loader"
#define LOADER_APPLICATIONS_NAME "Apps"

/**
 * @defgroup loader-priority Loader application priority
 *
 * The loader tracks a "priority" for the currently running application.  The
 * canvas HTTP API (POST /api/display/draw) uses this value to decide whether
 * an incoming draw request is allowed to update the display.
 *
 * Priority scale:
 *   STUB (0)          – never gates draws; any draw priority beats it.
 *   PASSTHROUGH (9)   – threshold that admits draws at ≥ DEFAULT (10);
 *                       canvas empty-canvas check uses strict-less-than.
 *   DEFAULT (10)      – set on app start; minimum priority for useful draws.
 *   MAX_APP (90)      – maximum value accepted by loader_set_priority().
 *   MAX / CANVAS_MAX (100) – HTTP API ceiling; values above → 400.
 *   BLOCKING (101)    – above API ceiling; all HTTP draws rejected.
 *
 * @note Only the thread that owns the currently running app may call
 *       loader_set_priority(); the loader verifies the caller's app-id.
 * @{
 */

#define LOADER_MAX_PRIORITY         100
#define LOADER_DEFAULT_APP_PRIORITY 10
#define LOADER_MAX_APP_PRIORITY     90
#define LOADER_STUB_APP_PRIORITY    0
#define LOADER_PASSTHROUGH_PRIORITY (LOADER_DEFAULT_APP_PRIORITY - 1)
#define LOADER_BLOCKING_PRIORITY    (LOADER_MAX_PRIORITY + 1)

/** @} loader-priority */

typedef struct Loader Loader;

typedef enum {
    LoaderStatusOk,
    LoaderStatusErrorAppStarted,
    LoaderStatusErrorAppNotRunning,
    LoaderStatusErrorUnknownApp,
    LoaderStatusErrorNoSignalHandler,
} LoaderStatus;

typedef enum {
    LoaderEventTypeApplicationBeforeLoad,
    LoaderEventTypeApplicationLoadFailed,
    LoaderEventTypeApplicationStopped,
    LoaderEventTypePriorityChanged,
} LoaderEventType;

typedef struct {
    LoaderEventType type;
    size_t priority;
    /**
     * @brief App ID of the application the event refers to.
     *
     * Set for @c LoaderEventTypeApplicationBeforeLoad and @c LoaderEventTypeApplicationStopped.
     * NULL for @c LoaderEventTypePriorityChanged and @c LoaderEventTypeApplicationLoadFailed.
     * The pointer is valid only within the event delivery callback.
     */
    const char* appid;
} LoaderEvent;

/**
 * @brief Start application
 * @param[in] instance loader instance
 * @param[in] name application name or id
 * @param[in] args application arguments
 * @param[out] error_message detailed error message, can be NULL
 * @return LoaderStatus
 */
LoaderStatus
    loader_start(Loader* instance, const char* name, const char* args, FuriString* error_message);

/**
 * @brief Stop the currently running application
 *
 * @param[in] instance loader instance
 * @return LoaderStatusOk on success, any other error code on failure
 */
LoaderStatus loader_stop(Loader* instance);

/**
 * @brief Lock application start
 * @param[in] instance loader instance
 * @return true on success
 */
bool loader_lock(Loader* instance);

/**
 * @brief Unlock application start
 * @param[in] instance loader instance
 */
void loader_unlock(Loader* instance);

/**
 * @brief Check if loader is locked
 * @param[in] instance loader instance
 * @return true if locked
 */
bool loader_is_locked(Loader* instance);

/**
 * @brief Get loader pubsub
 * @param[in] instance loader instance
 * @return FuriPubSub* 
 */
FuriPubSub* loader_get_pubsub(Loader* instance);

/**
 * @brief Get the name of the currently running application
 *
 * @param[in] instance pointer to the loader instance
 * @param[in,out] name pointer to the string to contain the name (must be allocated)
 * @return true if it was possible to get an application name, false otherwise
 */
bool loader_get_application_name(Loader* instance, FuriString* name);

/**
 * @brief Send a signal to the currently running application
 * @param[in] instance pointer to the loader instance
 * @param[in] signal signal value
 * @param[in] arg optional argument to pass with the signal
 * @return true if signal was sent and consumed, false otherwise
 */
bool loader_send_signal(Loader* instance, uint32_t signal, void* arg);

/**
 * @brief Set the priority level for the currently running app.
 *
 * The priority gates HTTP canvas draw requests: see the LoaderPriority group
 * for the full scale and interaction rules.
 *
 * The call is synchronous and is processed by the loader task.  It succeeds
 * only when an application is running **and** the calling thread belongs to
 * that same application (verified by app-id comparison).
 *
 * @param[in] instance loader instance
 * @param[in] priority new priority (0 … LOADER_BLOCKING_PRIORITY)
 * @return true  priority was updated
 * @return false no app is running, or the caller is not the running app
 */
bool loader_set_priority(Loader* instance, size_t priority);

/**
 * @brief Get the priority level of the currently running app.
 *
 * The canvas service calls this on every draw request to compute the effective
 * rejection threshold.  Returns 0 if no application is currently running.
 *
 * @param[in] instance loader instance
 * @return Current app priority, or 0 if no app is running.
 */
size_t loader_get_priority(Loader* instance);

/** @} loader */

#ifdef __cplusplus
}
#endif
