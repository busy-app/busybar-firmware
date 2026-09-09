/**
 * @file js_app_installer.h
 * @brief Javascript application installer.
 */
#pragma once

#include <furi/furi.h>

#define RECORD_JS_APP_INSTALLER "js_app_installer"

typedef struct JsAppInstaller JsAppInstaller;

typedef enum JsAppInstallerError {
    JsAppInstallerErrorNone = 0,
    JsAppInstallerErrorUnpack,
    JsAppInstallerErrorManifest,

    JsAppInstallerErrorMax
} JsAppInstallerError;

typedef struct JsAppInstallerStageResult {
    JsAppInstallerError error;

    uint32_t
        install_id; ///< Unique id used to install the app (valid if error == JsAppInstallerErrorNone)
    FuriString* app_id; ///< Unpacked application id (NULL unless error == JsAppInstallerErrorNone)
    FuriString*
        version; ///< Unpacked application version (NULL unless error == JsAppInstallerErrorNone)
    FuriString* installed_version; ///< Currently installed version of this app or NULL
} JsAppInstallerStageResult;

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
 * @param install_id installation id previously returned with js_app_installer_stage.
 * @return operation result.
 */
JsAppInstallerError js_app_installer_install(JsAppInstaller* instance, uint32_t install_id);
