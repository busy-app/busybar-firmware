#include "js_app_installer_operations.h"

#include <core/check.h>

extern const JsAppInstallerOperationInterface js_app_installer_operation_install;
extern const JsAppInstallerOperationInterface js_app_installer_operation_remove;

/* clang-format off */
const JsAppInstallerOperationInterface* const js_app_installer_operations[] = {
    [JsAppInstallerOperationTypeInstall] = &js_app_installer_operation_install,
    [JsAppInstallerOperationTypeRemove] = &js_app_installer_operation_remove,
};
/* clang-format on */

static_assert(COUNT_OF(js_app_installer_operations) == JsAppInstallerOperationTypeMax);
