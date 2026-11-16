/**
 * @file js_app_installer_common.h
 * @brief Common types for the JsAppInstaller service.
 */
#pragma once

#include <toolbox/semver.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Errors that can occur during JsAppInstaller operation.
 */
typedef enum JsAppInstallerError {
    JsAppInstallerErrorNone = 0, /**< Operation successful, no error occurred */
    JsAppInstallerErrorDownload, /**< Failed to download a file */
    JsAppInstallerErrorUnpack, /**< Failed to unpak an archive file */
    JsAppInstallerErrorManifest, /**< Application manifest is invalid or missing */
    JsAppInstallerErrorApiVersion, /**< App requires an incompatible HTTP API version */
    JsAppInstallerErrorStorage, /**< A storage operation has failed */
    JsAppInstallerErrorTimeout, /**< The operation is taking too long */
    JsAppInstallerErrorNotFound, /**< Requested item was not found */
    JsAppInstallerErrorQueueFull, /**< Too many pending operations */
    JsAppInstallerErrorCancelled, /**< Operation was cancelled */
    JsAppInstallerErrorAppRunning, /**< Impossible to update/remove a running app */
    JsAppInstallerErrorParameter, /**< Invalid parameter value was provided */
    JsAppInstallerErrorInternal, /**< Unknown internal error */
    JsAppInstallerErrorMax, /**< Special value for bounds checking */
} JsAppInstallerError;

/**
 * @brief Enumeration of supported operation types.
 */
typedef enum {
    JsAppInstallerOperationTypeInstall, /**< Install operation tag */
    JsAppInstallerOperationTypeRemove, /**< Remove operation tag */
    JsAppInstallerOperationTypeMax, /**< Special value for bounds checking */
} JsAppInstallerOperationType;

/**
 * @brief Optional application version structure.
 *
 * @p value is only valid if `has_value == true`
 */
typedef struct {
    SemVer value; /**< Application version value */
    bool has_value; /**< Whether the structure contains a valid version */
} JsAppInstallerAppVersion;

/**
 * @brief Application information structure.
 */
typedef struct {
    const char* app_id; /**< Zero-terminated string containing the application ID */
    JsAppInstallerAppVersion version; /**< Optional application version */
} JsAppInstallerAppInfo;

/**
 * @brief Application source information structure.
 */
typedef struct {
    const char* url; /**< Remote URL to install the app from */
    JsAppInstallerAppVersion version; /**< Optional application version */
} JsAppInstallerSourceInfo;

/**
 * @brief Enumeration of possible operation statuses.
 */
typedef enum {
    JsAppInstallerOperationStatusPending, /**< Operation is in the queue, not started yet */
    JsAppInstallerOperationStatusInProgress, /**< Operation is currently in progress */
} JsAppInstallerOperationStatus;

/**
 * @brief Detailed information about the operation's progress.
 *
 * @p progress is only valid if `has_progress == true`
 */
typedef struct {
    const char* step; /**< Zero-terminated string containing the step name */
    bool has_progress; /**< Whehter the structure contains a valid progress value */
    uint8_t progress; /**< Operation progress value (0-100) */
} JsAppInstallerOperationDetail;

/**
 * @brief Operation information structure.
 */
typedef struct {
    uint32_t item_id; /**< Unique numeric ID of the operation */
    JsAppInstallerAppInfo app_info; /**< Application information */
    JsAppInstallerOperationType type; /**< Operation type */
    JsAppInstallerOperationStatus status; /**< Operation status */
    JsAppInstallerOperationDetail detail; /**< Detailed progress information */
} JsAppInstallerOperationInfo;

const char* js_app_installer_get_operation_name(JsAppInstallerOperationType type);

#ifdef __cplusplus
}
#endif
