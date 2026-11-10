#include "../js_app_installer_operation_i.h"

#include <core/check.h>
#include <core/kernel.h>
#include <core/log.h>

#define TAG "JsAppInstallerOperationRemove"

JsAppInstallerOperationData*
    js_app_installer_operation_remove_alloc(const JsAppInstallerQueueItem* item) {
    furi_assert(item);
    return NULL;
}

void js_app_installer_operation_remove_free(JsAppInstallerOperationData* data) {
    UNUSED(data);
}

void js_app_installer_operation_remove_run(JsAppInstallerOperation* instance) {
    furi_assert(instance);

    FURI_LOG_W(TAG, "Removing app: %s", furi_string_get_cstr(instance->app_id));
    furi_delay_ms(3000);
}

const JsAppInstallerOperationInterface js_app_installer_operation_remove = {
    .alloc = js_app_installer_operation_remove_alloc,
    .free = js_app_installer_operation_remove_free,
    .run = js_app_installer_operation_remove_run,
};
