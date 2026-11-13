/**
 * @file js_app_installer.h
 * @brief Javascript application installer.
 */
#pragma once

#include <core/pubsub.h>

#include <js_app/js_app.h>

#include "js_app_installer_common.h"

#define RECORD_JS_APP_INSTALLER "js_app_installer"

#define JS_APP_INSTALLER_MAX_QUEUE_ITEMS (100)

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

typedef struct {
    JsAppInstallerError error;
    uint32_t item_id;
} JsAppInstallerOperationResult;

typedef void (
    *JsAppInstallerOperationInfoCallback)(const JsAppInstallerOperationInfo* info, void* context);

typedef enum {
    JsAppInstallerEventTypeOperationDetail,
    JsAppInstallerEventTypeOperationFinished,
    JsAppInstallerEventTypeMax,
} JsAppInstallerEventType;

typedef struct {
    JsAppInstallerOperationDetail value;
} JsAppInstallerEventOperationDetail;

typedef struct {
    JsAppInstallerError error;
} JsAppInstallerEventOperationFinished;

typedef struct {
    JsAppInstallerEventType type;
    uint32_t item_id;
    union {
        JsAppInstallerEventOperationDetail detail;
        JsAppInstallerEventOperationFinished finished;
    };
} JsAppInstallerEvent;

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

JsAppInstallerError js_app_installer_cancel_operation(JsAppInstaller* instance, uint32_t item_id);

JsAppInstallerError js_app_installer_cancel_all(JsAppInstaller* instance);
