/**
 * @file js_app_installer_common.h
 * @brief Common types for the JsAppInstaller service.
 */
#pragma once

#include <toolbox/semver.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum JsAppInstallerError {
    JsAppInstallerErrorNone = 0,
    JsAppInstallerErrorUnpack,
    JsAppInstallerErrorManifest,
    JsAppInstallerErrorApiVersion,
    JsAppInstallerErrorInstall,
    JsAppInstallerErrorTimeout,
    JsAppInstallerErrorNotFound,
    JsAppInstallerErrorQueueFull,
    JsAppInstallerErrorCancelled,
    JsAppInstallerErrorMax,
} JsAppInstallerError;

typedef enum {
    JsAppInstallerOperationTypeInstall,
    JsAppInstallerOperationTypeRemove,
    JsAppInstallerOperationTypeMax,
} JsAppInstallerOperationType;

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
    JsAppInstallerOperationStatusPending,
    JsAppInstallerOperationStatusInProgress,
} JsAppInstallerOperationStatus;

typedef struct {
    const char* step;
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

const char* js_app_installer_get_operation_name(JsAppInstallerOperationType type);

#ifdef __cplusplus
}
#endif
