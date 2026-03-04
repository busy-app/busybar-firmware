#pragma once
#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_LOADER            "loader"
#define LOADER_APPLICATIONS_NAME "Apps"

#define LOADER_MAX_PRIORITY         100
#define LOADER_DEFAULT_APP_PRIORITY 10
#define LOADER_MAX_APP_PRIORITY     90
/** Priority 0 is reserved for system stub apps (e.g. poweroff, certain
 *  settings pages) that must never block HTTP draw requests. */
#define LOADER_STUB_APP_PRIORITY    0

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
    LoaderEventTypeApplicationStopped
} LoaderEventType;

typedef struct {
    LoaderEventType type;
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
 * @brief Sets the priority level for the currently running app
 * 
 * @param[in] instance pointer to the loader instance
 * @param[in] priority priority level to set. Max is `LOADER_MAX_PRIORITY`
 * 
 * @return `true` if set successfully, `false` if argument is out of range or
 *         the requesting thread does not belong to the running app
 */
bool loader_set_priority(Loader* instance, size_t priority);

/**
 * @brief Gets the priority level of the currently running app
 * 
 * @param[in] instance pointer to the loader instance
 * 
 * @return the active priority level, or `0` if no app is running
 */
size_t loader_get_priority(Loader* instance);

#ifdef __cplusplus
}
#endif
