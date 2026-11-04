/**
 * @file js_app_installer.h
 * @brief Javascript application installer.
 */
#pragma once

#include <js_app/js_app.h>

#define RECORD_JS_APP_INSTALLER "js_app_installer"

typedef struct JsAppInstaller JsAppInstaller;

typedef enum JsAppInstallerError {
    JsAppInstallerErrorNone = 0,
    JsAppInstallerErrorUnpack,
    JsAppInstallerErrorManifest,
    JsAppInstallerErrorApiVersion,
    JsAppInstallerErrorInstall,
    JsAppInstallerErrorTimeout,
    JsAppInstallerErrorQueueFull,
    JsAppInstallerErrorMax,
} JsAppInstallerError;

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

typedef struct {
    JsAppInstallerError error;
    uint32_t operation_id;
} JsAppInstallerOperationResult;

typedef struct {
    SemVer value;
    bool is_present;
} JsAppInstallerAppVersion;

typedef struct {
    const char* app_id;
    JsAppInstallerAppVersion version;
} JsAppInstallerAppInfo;

typedef struct {
    const char* url;
    JsAppInstallerAppVersion version;
} JsAppInstallerSourceInfo;

typedef enum {
    JsAppInstallerOperationTypeInstall,
    JsAppInstallerOperationTypeRemove,
} JsAppInstallerOperationType;

typedef enum {
    JsAppInstallerOperationStatusPending,
    JsAppInstallerOperationStatusInProgress,
} JsAppInstallerOperationStatus;

typedef enum {
    JsAppInstallerOperationStepTypeDownloading,
    JsAppInstallerOperationStepTypeUnpacking,
    JsAppInstallerOperationStepTypeRemoving,
    // TODO: More step types
    JsAppInstallerOperationStepTypeMax,
} JsAppInstallerOperationStepType;

typedef struct {
    JsAppInstallerOperationStepType step;
    bool has_progress;
    uint32_t progress;
} JsAppInstallerOperationDetail;

typedef struct {
    uint32_t item_id;
    JsAppInstallerAppInfo app_info;
    JsAppInstallerOperationType type;
    JsAppInstallerOperationStatus status;
    JsAppInstallerOperationDetail detail;
} JsAppInstallerOperationInfo;

typedef void (
    *JsAppInstallerOperationInfoCallback)(const JsAppInstallerOperationInfo* info, void* context);

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

JsAppInstallerOperationResult js_app_installer_install(
    JsAppInstaller* instance,
    const char* app_id,
    const JsAppInstallerSourceInfo* info);

JsAppInstallerOperationResult
    js_app_installer_remove(JsAppInstaller* instance, const char* app_id);

JsAppInstallerError js_app_installer_get_operation_queue(
    JsAppInstaller* instance,
    JsAppInstallerOperationInfoCallback callback,
    void* context);

JsAppInstallerError
    js_app_installer_cancel_operation(JsAppInstaller* instance, uint32_t operation_id);

JsAppInstallerError js_app_installer_cancel_all(JsAppInstaller* instance);
