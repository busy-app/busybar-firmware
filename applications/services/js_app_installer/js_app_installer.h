/**
 * @file js_app_installer.h
 * @brief Javascript application installer service API.
 */
#pragma once

#include <core/pubsub.h>
#include <js_app/js_app.h>

#include "js_app_installer_common.h"

/**
 * @brief String key for JsAppInstaller instance access
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_JS_APP_INSTALLER)`
 */
#define RECORD_JS_APP_INSTALLER "js_app_installer"

/**
 * @brief Maximum number of pending operations.
 */
#define JS_APP_INSTALLER_MAX_QUEUE_ITEMS (100)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief JsAppInstaller opaque type.
 */
typedef struct JsAppInstaller JsAppInstaller;

/**
 * @brief Stage operation result structure.
 *
 * Members other than @p error are valid only if `error == JsAppInstallerErrorNone`.
 */
typedef struct JsAppInstallerStageResult {
    JsAppInstallerError error; ///< Error code (always valid)
    uint32_t install_key; ///< Unique key used to install the application
    JsApp* staged_app; ///< Staged application
    JsApp* installed_app; ///< Application to be replaced or @c NULL if none present
} JsAppInstallerStageResult;

/**
 * @brief Queue-based operation result structure.
 *
 * Members other than @p error are valid only if `error == JsAppInstallerErrorNone`.
 */
typedef struct {
    JsAppInstallerError error; /**< Error code (always valid) */
    uint32_t item_id; /**< Unique numeric operation ID */
} JsAppInstallerOperationResult;

/**
 * @brief Operation info callback function type.
 *
 * Data pointed to by @p info is only valid within the callback.
 * Callers must make a deep copy of the data they intend to use
 * outside of the callback call.
 *
 * @param[in] info pointer to the current operation info structure
 * @param[in,out] context pointer to the user-specified object
 */
typedef void (
    *JsAppInstallerOperationInfoCallback)(const JsAppInstallerOperationInfo* info, void* context);

/**
 * @brief Events emitted by the JsAppInstaller service.
 */
typedef enum {
    JsAppInstallerEventTypeOperationDetail, /**< A new operation detail is available */
    JsAppInstallerEventTypeOperationFinished, /**< An operation has finished */
    JsAppInstallerEventTypeMax, /**< Special value for bounds checking */
} JsAppInstallerEventType;

/**
 * @brief Operation detail event structure.
 */
typedef struct {
    JsAppInstallerOperationDetail value; /**< Operation detail value */
} JsAppInstallerEventOperationDetail;

/**
 * @brief Operation finished event structure.
 */
typedef struct {
    JsAppInstallerError error; /**< Error value the operation finished with */
} JsAppInstallerEventOperationFinished;

/**
 * @brief Event structure emitted by the JsAppInstaller service.
 */
typedef struct {
    /** Type of the event, determines which union member should be used */
    JsAppInstallerEventType type;
    uint32_t item_id; /**< Unique numeric ID of the operation that produced the event */
    union {
        JsAppInstallerEventOperationDetail detail; /**< Detail event value */
        JsAppInstallerEventOperationFinished finished; /**< Finished event value */
    };
} JsAppInstallerEvent;

/**
 * @brief Get the @c FuriPubSub object used for event notification.
 *
 * Events received through the pubsub callback will be of @c JsAppInstallerEvent type.
 *
 * @param[in] instance pointer to the service instance
 * @returns pointer to the @c FuriPubSub object
 */
FuriPubSub* js_app_installer_get_pubsub(JsAppInstaller* instance);

/**
 * @brief Unpack and check downloaded app package.
 *
 * The package to unpack is located at JS_APP_DOWNLOAD_PATH.
 * The package is unpacked to the staging folder (JS_APP_STAGING_PATH).
 *
 * @param instance service instance.
 * @return operation result.
 */
JsAppInstallerStageResult js_app_installer_stage(JsAppInstaller* instance);

/**
 * @brief Install previously staged app.
 *
 * @param instance service instance.
 * @param install_key installation key previously returned with js_app_installer_stage.
 * @return operation result.
 */
JsAppInstallerError
    js_app_installer_install_staged(JsAppInstaller* instance, uint32_t install_key);

/**
 * @brief Install an app via the application management queue.
 *
 * This call only enqueues the request (if possible).
 * Completion and progress will be delivered asynchronously
 * via the event system.
 *
 * @param[in,out] instance pointer to the service instance
 * @param[in] app_id application ID of the app to be installed
 * @param[in] info app installation source information
 * @returns result structure containing the error and queue item ID if successful
 */
JsAppInstallerOperationResult js_app_installer_install(
    JsAppInstaller* instance,
    const char* app_id,
    const JsAppInstallerSourceInfo* info);

/**
 * @brief Remove an app via the application management queue.
 *
 * This call only enqueues the request (if possible).
 * Completion and progress will be delivered asynchronously
 * via the event system.
 *
 * @param[in,out] instance pointer to the service instance
 * @param[in] app_id application ID of the app to be removed
 * @returns result structure containing the error and queue item ID if successful
 */
JsAppInstallerOperationResult
    js_app_installer_remove(JsAppInstaller* instance, const char* app_id);

/**
 * @brief List all pending and active operations in the application management queue.
 *
 * Queue elements will be delivered via the callback function one-by one.
 *
 * @param[in,out] instance pointer to the service instance
 * @param[in] callback pointer to the function to be called once per queue item
 * @param[in,out] context pointer to the user-specified object (will be passed to callback)
 * @returns @c JsAppInstallerErrorNone operation queue listed successfully
 * @returns @c JsAppInstallerErrorTimeout service was taking too long to respond
 */
JsAppInstallerError js_app_installer_get_operation_queue(
    JsAppInstaller* instance,
    JsAppInstallerOperationInfoCallback callback,
    void* context);

/**
 * @brief Cancel a specific operation by its ID.
 *
 * If the requested operation is already running, it may still run
 * to completion depending on the time it has been requested to stop.
 *
 * @param[in,out] instance pointer to the service instance
 * @param[in] item_id numeric ID of the operation to be cancelled
 * @returns @c JsAppInstallerErrorNone operation cancelled successfully
 * @returns @c JsAppInstallerErrorTimeout service was taking too long to respond
 * @returnc @c JsAppInstallerErrorNotFound operation with the given ID was not found
 */
JsAppInstallerError js_app_installer_cancel_operation(JsAppInstaller* instance, uint32_t item_id);

/**
 * @brief Cancel all pending and current operations.
 *
 * If there is an operation already running, it may still run
 * to completion depending on the time it has been requested to stop.
 *
 * @param[in,out] instance pointer to the service instance
 * @returns @c JsAppInstallerErrorNone all operations cancelled successfully
 * @returns @c JsAppInstallerErrorTimeout service was taking too long to respond
 */
JsAppInstallerError js_app_installer_cancel_all(JsAppInstaller* instance);

#ifdef __cplusplus
}
#endif
