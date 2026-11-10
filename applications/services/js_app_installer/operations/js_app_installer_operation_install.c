#include "../js_app_installer_operation_i.h"

#include <core/check.h>
#include <core/kernel.h>
#include <core/log.h>

#define TAG "JsAppInstallerOperationInstall"

static JsAppInstallerOperationData*
    js_app_installer_operation_install_alloc(const JsAppInstallerQueueItem* item) {
    furi_assert(item);
    return NULL;
}

static void js_app_installer_operation_install_free(JsAppInstallerOperationData* data) {
    UNUSED(data);
}

static JsAppInstallerError
    js_app_installer_operation_install_run(JsAppInstallerOperation* instance) {
    furi_assert(instance);

    FURI_LOG_W(TAG, "Installing app: %s", furi_string_get_cstr(instance->app_id));
    furi_delay_ms(3000);

    return JsAppInstallerErrorNone;
}

const JsAppInstallerOperationInterface js_app_installer_operation_install = {
    .alloc = js_app_installer_operation_install_alloc,
    .free = js_app_installer_operation_install_free,
    .run = js_app_installer_operation_install_run,
};
