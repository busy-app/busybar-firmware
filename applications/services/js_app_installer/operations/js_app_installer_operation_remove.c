#include "../js_app_installer_operation_i.h"

#include <core/check.h>
#include <core/kernel.h>
#include <core/log.h>

#include <js_app/js_app_registry.h>

#define TAG "JsAppInstallerOperationRemove"

typedef enum {
    JsAppInstallerOperationRemoveStepIdRemove,
    JsAppInstallerOperationRemoveStepIdMax,
} JsAppInstallerOperationRemoveStepId;

typedef struct {
    JsAppInstallerOperation base;
} JsAppInstallerOperationRemove;

static void js_app_installer_operation_remove_init(
    JsAppInstallerOperation* base_instance,
    const JsAppInstallerQueueItem* item) {
    furi_assert(item);
    furi_assert(item->type == JsAppInstallerOperationTypeRemove);
    UNUSED(base_instance);
    // Empty implementation
}

static void js_app_installer_operation_remove_empty(JsAppInstallerOperation* base_instance) {
    UNUSED(base_instance);
    // Empty implementation
}

static JsAppInstallerError
    js_app_installer_operation_translate_registry_result(JsAppRegistryAppUninstallResult result) {
    furi_assert(result < JsAppRegistryAppUninstallResultMax);

    static const JsAppInstallerError error_table[] = {
        [JsAppRegistryAppUninstallResultOk] = JsAppInstallerErrorNone,
        [JsAppRegistryAppUninstallResultNotFound] = JsAppInstallerErrorNotFound,
        // TODO: fitting error
        [JsAppRegistryAppUninstallResultStorageError] = JsAppInstallerErrorInstall,
    };

    static_assert(COUNT_OF(error_table) == JsAppRegistryAppUninstallResultMax);
    return error_table[result];
}

static JsAppInstallerError
    js_app_installer_operation_remove_handler(JsAppInstallerOperation* base_instance) {
    furi_assert(base_instance);

    const char* app_id = furi_string_get_cstr(base_instance->app_id);
    const JsAppRegistryAppUninstallResult result = js_app_registry_uninstall_app(app_id);

    return js_app_installer_operation_translate_registry_result(result);
}

/* clang-format off */
const JsAppInstallerOperationInterface js_app_installer_operation_remove = {
    .init = js_app_installer_operation_remove_init,
    .deinit = js_app_installer_operation_remove_empty,
    .cancel = js_app_installer_operation_remove_empty,
    .cleanup = js_app_installer_operation_remove_empty,
    .data_size = sizeof(JsAppInstallerOperationRemove),
    .step_count = JsAppInstallerOperationRemoveStepIdMax,
    .steps = {
        [JsAppInstallerOperationRemoveStepIdRemove] = {
            .name = "removing",
            .handler = js_app_installer_operation_remove_handler,
        },
    },
};
/* clang-format on */
