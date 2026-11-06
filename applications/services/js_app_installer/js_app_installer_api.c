#include "js_app_installer_i.h"

#define OPERATION_QUEUE_TIMEOUT_MS (1000)
#define OPERATION_QUEUE_MAX_SIZE   (100)

static bool js_app_installer_queue_lock(JsAppInstaller* instance) {
    bool success = true;

    const FuriStatus status =
        furi_mutex_acquire(instance->queue_mutex, furi_ms_to_ticks(OPERATION_QUEUE_TIMEOUT_MS));

    if(status != FuriStatusOk) {
        furi_check(status == FuriStatusErrorTimeout);
        success = false;
    }

    return success;
}

static void js_app_installer_queue_unlock(JsAppInstaller* instance) {
    furi_mutex_release(instance->queue_mutex);
}

static JsAppInstallerOperationResult
    js_app_installer_enqueue_operation(JsAppInstaller* instance, JsAppInstallerQueueItem* item) {
    JsAppInstallerOperationResult result;

    do {
        if(!js_app_installer_queue_lock(instance)) {
            result.error = JsAppInstallerErrorTimeout;
            break;
        }

        if(JsAppInstallerOperationQueue_size(instance->operation_queue) >=
           OPERATION_QUEUE_MAX_SIZE) {
            result.error = JsAppInstallerErrorQueueFull;
            break;
        }

        item->operation_id = instance->operation_id_counter++;
        JsAppInstallerOperationQueue_push_front(instance->operation_queue, item);

        result.error = JsAppInstallerErrorNone;
        result.operation_id = item->operation_id;

    } while(false);

    js_app_installer_queue_unlock(instance);

    if(result.error != JsAppInstallerErrorNone) {
        js_app_installer_queue_item_free(item);
    }

    return result;
}

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
    const JsAppInstallerSourceInfo* source_info) {
    furi_check(app_id);
    furi_check(source_info);

    JsAppInstallerQueueItem* item =
        js_app_installer_queue_item_alloc(app_id, JsAppInstallerOperationTypeInstall, source_info);
    const JsAppInstallerOperationResult result =
        js_app_installer_enqueue_operation(instance, item);
    return result;
}

JsAppInstallerOperationResult
    js_app_installer_remove(JsAppInstaller* instance, const char* app_id) {
    furi_check(instance);
    furi_check(app_id);

    JsAppInstallerQueueItem* item =
        js_app_installer_queue_item_alloc(app_id, JsAppInstallerOperationTypeRemove);
    const JsAppInstallerOperationResult result =
        js_app_installer_enqueue_operation(instance, item);
    return result;
}

JsAppInstallerError js_app_installer_get_operation_queue(
    JsAppInstaller* instance,
    JsAppInstallerOperationInfoCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(callback);

    if(!js_app_installer_queue_lock(instance)) {
        return JsAppInstallerErrorTimeout;
    }

    JsAppInstallerOperationQueue_it_ct it;
    for(JsAppInstallerOperationQueue_it(it, instance->operation_queue);
        !JsAppInstallerOperationQueue_end_p(it);
        JsAppInstallerOperationQueue_next(it)) {
        const JsAppInstallerQueueItem* item = JsAppInstallerOperationQueue_ref(it);
        /* clang-format off */
        const JsAppInstallerOperationInfo operation_info = {
            .type = item->type,
            .item_id = item->operation_id,
            .app_info = {
                .app_id = furi_string_get_cstr(item->app_id),
                .version = item->app_version,
            },
        };
        /* clang-format on */
        callback(&operation_info, context);
    }

    js_app_installer_queue_unlock(instance);
    return JsAppInstallerErrorNone;
}

JsAppInstallerError
    js_app_installer_cancel_operation(JsAppInstaller* instance, uint32_t operation_id) {
    furi_check(instance);
    UNUSED(operation_id);
    // TODO: Implementation
    return JsAppInstallerErrorNone;
}

JsAppInstallerError js_app_installer_cancel_all(JsAppInstaller* instance) {
    furi_check(instance);
    // TODO: Implementation
    return JsAppInstallerErrorNone;
}
