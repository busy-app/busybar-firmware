#include "js_app_installer_i.h"

#define API_TIMEOUT_MS (1000)

static void
    js_app_installer_api_send_message(JsAppInstaller* instance, JsAppInstallerMsg* message) {
    message->api_lock = api_lock_alloc_locked();

    const FuriStatus status =
        furi_message_queue_put(instance->msg_queue, message, furi_ms_to_ticks(API_TIMEOUT_MS));

    if(status == FuriStatusOk) {
        api_lock_wait_unlock_and_free(message->api_lock);
    } else {
        furi_check(status == FuriStatusErrorTimeout);
        *message->error = JsAppInstallerErrorTimeout;
        api_lock_free(message->api_lock);
    }
}

JsAppInstallerStageResult js_app_installer_stage(JsAppInstaller* instance) {
    furi_check(instance);

    JsAppInstallerError error;
    JsAppInstallerStageInfo stage_info = {0};

    /* clang-format off */
    JsAppInstallerMsg msg = {
        .cmd = JsAppInstallerCmdStage,
        .error = &error,
        .stage = {
            .stage_info = &stage_info,
        },
    };
    /* clang-format on */

    js_app_installer_api_send_message(instance, &msg);

    const JsAppInstallerStageResult result = {
        .error = error,
        .install_key = stage_info.install_key,
        .staged_app = stage_info.staged_app,
        .installed_app = stage_info.installed_app,
    };

    return result;
}

JsAppInstallerError
    js_app_installer_install_staged(JsAppInstaller* instance, uint32_t install_key) {
    furi_check(instance);

    JsAppInstallerError error;
    /* clang-format off */
    JsAppInstallerMsg msg = {
        .cmd = JsAppInstallerCmdInstallStaged,
        .error = &error,
        .install_staged = {
            .install_key = install_key,
        },
    };
    /* clang-format on */

    js_app_installer_api_send_message(instance, &msg);
    return error;
}

JsAppInstallerOperationResult js_app_installer_install(
    JsAppInstaller* instance,
    const char* app_id,
    const JsAppInstallerSourceInfo* source_info) {
    furi_check(app_id);
    furi_check(source_info);

    JsAppInstallerOperationResult result;
    /* clang-format off */
    JsAppInstallerMsg msg = {
        .cmd = JsAppInstallerCmdInstall,
        .error = &result.error,
        .install = {
            .app_id = app_id,
            .source_info = source_info,
            .item_id = &result.item_id,
        },
    };
    /* clang-format on */

    js_app_installer_api_send_message(instance, &msg);
    return result;
}

JsAppInstallerOperationResult
    js_app_installer_remove(JsAppInstaller* instance, const char* app_id) {
    furi_check(instance);
    furi_check(app_id);

    JsAppInstallerOperationResult result;
    /* clang-format off */
    JsAppInstallerMsg msg = {
        .cmd = JsAppInstallerCmdRemove,
        .error = &result.error,
        .remove = {
            .app_id = app_id,
            .item_id = &result.item_id,
        },
    };
    /* clang-format on */

    js_app_installer_api_send_message(instance, &msg);
    return result;
}

JsAppInstallerError js_app_installer_get_operation_queue(
    JsAppInstaller* instance,
    JsAppInstallerOperationInfoCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(callback);

    JsAppInstallerError error;
    /* clang-format off */
    JsAppInstallerMsg msg = {
        .cmd = JsAppInstallerCmdGetQueue,
        .error = &error,
        .get_queue = {
            .callback = callback,
            .callback_context = context,
        },
    };
    /* clang-format on */

    js_app_installer_api_send_message(instance, &msg);
    return error;
}

JsAppInstallerError js_app_installer_cancel_operation(JsAppInstaller* instance, uint32_t item_id) {
    furi_check(instance);

    JsAppInstallerError error;
    /* clang-format off */
    JsAppInstallerMsg msg = {
        .cmd = JsAppInstallerCmdCancel,
        .error = &error,
        .cancel = {
            .item_id = item_id,
            .is_clear_all = false,
        },
    };
    /* clang-format on */

    js_app_installer_api_send_message(instance, &msg);
    return error;
}

JsAppInstallerError js_app_installer_cancel_all(JsAppInstaller* instance) {
    furi_check(instance);

    JsAppInstallerError error;
    /* clang-format off */
    JsAppInstallerMsg msg = {
        .cmd = JsAppInstallerCmdCancel,
        .error = &error,
        .cancel = {
            .is_clear_all = true,
        },
    };
    /* clang-format on */

    js_app_installer_api_send_message(instance, &msg);
    return error;
}
