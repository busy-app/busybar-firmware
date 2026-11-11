#include "../js_app_installer_operation_i.h"

#include <core/check.h>
#include <core/kernel.h>
#include <core/log.h>

#include <js_app/js_app_registry.h>

#define TAG "JsAppInstallerOperationRemove"

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

static void js_app_installer_operation_remove_clear(JsAppInstallerOperation* base_instance) {
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
    js_app_installer_operation_remove_run(JsAppInstallerOperation* base_instance) {
    furi_assert(base_instance);

    const char* app_id = furi_string_get_cstr(base_instance->app_id);
    FURI_LOG_D(TAG, "Removing app: %s", app_id);
    /* clang-format off */
    const JsAppInstallerOperationEvent event = {
        .type = JsAppInstallerOperationEventTypeProgress,
        .progress.detail = {
            .step = "removing",
            .has_progress = false,
        },
    };
    /* clang-format on */
    js_app_installer_operation_emit(base_instance, &event);

    const JsAppRegistryAppUninstallResult result = js_app_registry_uninstall_app(app_id);
    return js_app_installer_operation_translate_registry_result(result);
}

const JsAppInstallerOperationInterface js_app_installer_operation_remove = {
    .init = js_app_installer_operation_remove_init,
    .clear = js_app_installer_operation_remove_clear,
    .run = js_app_installer_operation_remove_run,
    .data_size = sizeof(JsAppInstallerOperationRemove),
};
