#include "js_app_installer_common.h"

#include <core/check.h>

const char* js_app_installer_get_operation_name(JsAppInstallerOperationType type) {
    furi_check(type < JsAppInstallerOperationTypeMax);

    static const char* const operation_names[] = {
        [JsAppInstallerOperationTypeInstall] = "install",
        [JsAppInstallerOperationTypeRemove] = "remove",
    };

    static_assert(COUNT_OF(operation_names) == JsAppInstallerOperationTypeMax);

    return operation_names[type];
}
