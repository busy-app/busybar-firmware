#include "js_app_installer_common_i.h"

#include <core/check.h>

#include <js_app/js_app_registry.h>

JsAppInstallerError js_app_installer_common_remove_app(const char* app_id) {
    const JsAppRegistryAppUninstallResult result = js_app_registry_uninstall_app(app_id);

    static const JsAppInstallerError error_lookup[] = {
        [JsAppRegistryAppUninstallResultOk] = JsAppInstallerErrorNone,
        [JsAppRegistryAppUninstallResultNotFound] = JsAppInstallerErrorNotFound,
        [JsAppRegistryAppUninstallResultStorageError] = JsAppInstallerErrorStorage,
    };

    static_assert(COUNT_OF(error_lookup) == JsAppRegistryAppUninstallResultMax);

    return error_lookup[result];
}
