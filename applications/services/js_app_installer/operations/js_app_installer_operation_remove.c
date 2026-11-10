#include "../js_app_installer_operation_i.h"

#include <core/check.h>
#include <core/kernel.h>
#include <core/log.h>

#include <js_app/js_app_registry.h>

#define TAG "JsAppInstallerOperationRemove"

static JsAppInstallerOperationData*
    js_app_installer_operation_remove_alloc(const JsAppInstallerQueueItem* item) {
    furi_assert(item);
    furi_assert(item->type == JsAppInstallerOperationTypeRemove);
    // Empty implementation
    return NULL;
}

static void js_app_installer_operation_remove_free(JsAppInstallerOperationData* data) {
    UNUSED(data);
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
    js_app_installer_operation_remove_run(JsAppInstallerOperation* instance) {
    furi_assert(instance);

    const char* app_id = furi_string_get_cstr(instance->app_id);
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
    js_app_installer_operation_emit(instance, &event);

    const JsAppRegistryAppUninstallResult result = js_app_registry_uninstall_app(app_id);
    return js_app_installer_operation_translate_registry_result(result);
}

const JsAppInstallerOperationInterface js_app_installer_operation_remove = {
    .alloc = js_app_installer_operation_remove_alloc,
    .free = js_app_installer_operation_remove_free,
    .run = js_app_installer_operation_remove_run,
};
