#include "js_app_installer_i.h"

JsAppInstallerStageResult js_app_installer_stage(JsAppInstaller* instance) {
    JsAppInstallerStageResult result;

    bzero(&result, sizeof(result));

    JsAppInstallerMsg msg = {
        .cmd = JsAppInstallerCmdStage,
        .api_lock = api_lock_alloc_locked(),
        .stage =
            {
                .result = &result,
            },
    };

    furi_check(furi_message_queue_put(instance->msg_queue, &msg, FuriWaitForever) == FuriStatusOk);

    api_lock_wait_unlock_and_free(msg.api_lock);
    return result;
}

JsAppInstallerError
    js_app_installer_install_staged(JsAppInstaller* instance, uint32_t install_key) {
    JsAppInstallerError result;

    JsAppInstallerMsg msg = {
        .cmd = JsAppInstallerCmdInstall,
        .api_lock = api_lock_alloc_locked(),
        .install =
            {
                .install_key = install_key,
                .error = &result,
            },
    };

    furi_check(furi_message_queue_put(instance->msg_queue, &msg, FuriWaitForever) == FuriStatusOk);

    api_lock_wait_unlock_and_free(msg.api_lock);
    return result;
}

JsAppInstallerOperationResult js_app_installer_install(
    JsAppInstaller* instance,
    const char* app_id,
    const JsAppInstallerSourceInfo* info) {
    furi_check(instance);
    furi_check(app_id);
    furi_check(info);
    const JsAppInstallerOperationResult res = {
        .error = JsAppInstallerErrorNone,
        .operation_id = 42,
    };
    return res;
}

JsAppInstallerOperationResult
    js_app_installer_remove(JsAppInstaller* instance, const char* app_id) {
    furi_check(instance);
    furi_check(app_id);
    const JsAppInstallerOperationResult res = {
        .error = JsAppInstallerErrorNone,
        .operation_id = 67,
    };
    return res;
}

JsAppInstallerError js_app_installer_get_operation_queue(
    JsAppInstaller* instance,
    JsAppInstallerOperationInfoCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(callback);

    const JsAppInstallerOperationInfo infos[] = {
        {
            .item_id = 67,
            .app_info =
                {
                    .app_id = "com.example.my-app",
                    .version =
                        {
                            .value =
                                {
                                    .major = 1,
                                    .minor = 2,
                                    .patch = 3,
                                },
                            .is_present = true,
                        },
                },
            .type = JsAppInstallerOperationTypeInstall,
            .status = JsAppInstallerOperationStatusInProgress,
            .detail =
                {
                    .step = JsAppInstallerOperationStepTypeDownloading,
                    .has_progress = true,
                    .progress = 69,
                },
        },
        {
            .item_id = 42,
            .app_info =
                {
                    .app_id = "com.example.another-app",
                },
            .type = JsAppInstallerOperationTypeRemove,
            .status = JsAppInstallerOperationStatusPending,
        },
    };

    for(size_t i = 0; i < COUNT_OF(infos); ++i) {
        const JsAppInstallerOperationInfo* info = &infos[i];
        callback(info, context);
    }

    return JsAppInstallerErrorNone;
}

JsAppInstallerError
    js_app_installer_cancel_operation(JsAppInstaller* instance, uint32_t operation_id) {
    furi_check(instance);
    UNUSED(operation_id);
    return JsAppInstallerErrorNone;
}

JsAppInstallerError js_app_installer_cancel_all(JsAppInstaller* instance) {
    furi_check(instance);
    return JsAppInstallerErrorNone;
}
