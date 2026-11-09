/**
 * @file js_app_installer_common.h
 * @brief Common types for the JsAppInstaller service.
 */
#pragma once

#include <toolbox/semver.h>

#ifdef __cplusplus
extern "C" {
#endif

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

const char* js_app_installer_get_operation_name(JsAppInstallerOperationType type);

#ifdef __cplusplus
}
#endif
