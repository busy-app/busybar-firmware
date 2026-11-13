#include "js_app_installer_i.h"
#include "js_app_installer_paths.h"

#include <toolbox/tar/tar_archive.h>
#include <storage/storage.h>
#include <web_server/web_server.h>
#include <js_app/js_app.h>
#include <js_app/js_app_registry.h>
#include <storage_utils/dir_walk.h>

#define TAG "JsAppInstaller"

#define API_QUEUE_SIZE 1

typedef enum {
    JsAppInstallerCustomEventOperationProgress = 1UL << 0,
    JsAppInstallerCustomEventOperationFinished = 1UL << 1,
} JsAppInstallerCustomEvent;

typedef JsAppInstallerError (*MessageHandler)(JsAppInstaller* instance, JsAppInstallerMsg* message);

static bool prepare_staging_folder(Storage* storage) {
    if(!storage_simply_remove_recursive(storage, JS_APP_STAGING_PATH)) {
        FURI_LOG_E(TAG, "Cannot remove " JS_APP_STAGING_PATH);
        return false;
    }

    if(!storage_simply_mkpath(storage, JS_APP_STAGING_PATH)) {
        FURI_LOG_E(TAG, "Cannot create " JS_APP_STAGING_PATH);
        return false;
    }
    return true;
}

static uint32_t gen_install_key(void) {
    uint32_t result = 0;
    while(result == 0) {
        result = rand();
    }
    return result;
}

static JsAppInstallerError find_app_folder(
    JsAppInstaller* instance,
    Storage* storage,
    JsAppInstallerStageInfo* stage_info) {
    JsAppInstallerError error = JsAppInstallerErrorStorage;
    DirWalk* walk = dir_walk_alloc(storage);

    dir_walk_set_recursive(walk, false);
    dir_walk_set_filter_cb(walk, dir_walk_is_dir_callback, NULL);

    do {
        if(!dir_walk_open(walk, JS_APP_STAGING_PATH)) {
            FURI_LOG_E(TAG, "Cannot open " JS_APP_STAGING_PATH);
            break;
        }

        FuriString* path = furi_string_alloc();
        JsApp* app = js_app_alloc();

        while(dir_walk_read(walk, path, NULL) == DirWalkOK) {
            if(!js_app_load_from_directory(app, furi_string_get_cstr(path))) {
                error = JsAppInstallerErrorManifest;
                continue;
            }

            FURI_LOG_I(TAG, "Found app at %s", furi_string_get_cstr(path));

            JsAppInfo app_info;
            if(!js_app_get_info(app, &app_info)) {
                error = JsAppInstallerErrorManifest;
                break;
            }

            if(!web_server_is_api_version_compatible(&app_info.manifest.api_version)) {
                error = JsAppInstallerErrorApiVersion;
                break;
            }

            instance->staged_install_key = gen_install_key();
            instance->staged_app_path = furi_string_alloc_set(path);

            stage_info->staged_app = app;
            app = NULL;

            stage_info->install_key = instance->staged_install_key;
            stage_info->installed_app = js_app_registry_get_app(app_info.manifest.id);

            error = JsAppInstallerErrorNone;
            break;
        }

        furi_string_free(path);

        if(app) {
            js_app_free(app);
        }

    } while(false);

    dir_walk_free(walk);

    return error;
}

static bool is_queue_active(const JsAppInstaller* instance) {
    return (instance->current_work.item != NULL) ||
           (!JsAppInstallerOperationQueue_empty_p(instance->pending_queue));
}

static JsAppInstallerError handle_stage(JsAppInstaller* instance, JsAppInstallerMsg* message) {
    furi_assert(instance);
    furi_assert(message);
    JsAppInstallerError error;

    instance->staged_install_key = 0;
    if(instance->staged_app_path) {
        furi_string_free(instance->staged_app_path);
        instance->staged_app_path = NULL;
    }

    Storage* storage = furi_record_open(RECORD_STORAGE);
    do {
        if(is_queue_active(instance)) {
            error = JsAppInstallerErrorQueueFull;
            break;
        }

        if(!prepare_staging_folder(storage)) {
            error = JsAppInstallerErrorUnpack;
            break;
        }

        TarArchive* tar = tar_archive_alloc(storage);
        do {
            if(!tar_archive_open(tar, JS_APP_DOWNLOAD_PATH, TarOpenModeReadAuto)) {
                FURI_LOG_E(TAG, "Cannot open " JS_APP_DOWNLOAD_PATH);
                error = JsAppInstallerErrorUnpack;
                break;
            }

            if(!tar_archive_unpack_to(tar, JS_APP_STAGING_PATH, NULL)) {
                FURI_LOG_E(TAG, "Cannot unpack " JS_APP_DOWNLOAD_PATH);
                error = JsAppInstallerErrorUnpack;
                break;
            }

            JsAppInstallerStageInfo stage_info;

            error = find_app_folder(instance, storage, &stage_info);
            if(error != JsAppInstallerErrorNone) {
                FURI_LOG_E(TAG, "Cannot find a valid application folder");
                break;
            }

            *message->stage.stage_info = stage_info;
        } while(false);
        tar_archive_free(tar);

    } while(false);
    furi_record_close(RECORD_STORAGE);

    return error;
}

static JsAppInstallerError
    handle_install_staged(JsAppInstaller* instance, JsAppInstallerMsg* message) {
    furi_assert(instance);
    furi_assert(message);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    JsAppInstallerError error;
    bool cleanup = false;
    do {
        if(is_queue_active(instance)) {
            error = JsAppInstallerErrorQueueFull;
            break;
        }

        if(message->install_staged.install_key == 0 ||
           message->install_staged.install_key != instance->staged_install_key) {
            error = JsAppInstallerErrorParameter;
            break;
        }

        cleanup = true;
        JsApp* new_app = js_app_alloc();
        do {
            if(!js_app_load_from_directory(
                   new_app, furi_string_get_cstr(instance->staged_app_path))) {
                error = JsAppInstallerErrorManifest;
                break;
            }
            JsAppInfo new_app_info;
            if(!js_app_get_info(new_app, &new_app_info)) {
                error = JsAppInstallerErrorManifest;
                break;
            }
            FuriString* target_path = js_app_registry_get_app_path(new_app_info.manifest.id);
            furi_assert(target_path);

            do {
                if(!storage_simply_remove_recursive(storage, furi_string_get_cstr(target_path))) {
                    FURI_LOG_E(TAG, "Cannot delete app with id = %s", new_app_info.manifest.id);
                    error = JsAppInstallerErrorStorage;
                    break;
                }
                if(storage_common_rename(
                       storage,
                       furi_string_get_cstr(instance->staged_app_path),
                       furi_string_get_cstr(target_path)) != FSE_OK) {
                    FURI_LOG_E(TAG, "Cannot move directory");
                    error = JsAppInstallerErrorStorage;
                    break;
                }
                error = JsAppInstallerErrorNone;
            } while(false);
            furi_string_free(target_path);
        } while(false);
        js_app_free(new_app);
    } while(false);

    if(cleanup) {
        instance->staged_install_key = 0;
        furi_string_free(instance->staged_app_path);
        instance->staged_app_path = NULL;
        prepare_staging_folder(storage);
    }
    furi_record_close(RECORD_STORAGE);

    return error;
}

static void operation_event_callback(const JsAppInstallerOperationEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    JsAppInstaller* instance = context;

    uint32_t custom_event;

    JsAppInstallerOperationEventType event_type = event->type;
    if(event_type == JsAppInstallerOperationEventTypeProgress) {
        FURI_CRITICAL_ENTER();
        // OK to overwrite previous undelivered progress
        instance->current_work.detail = event->progress.detail;
        FURI_CRITICAL_EXIT();
        custom_event = JsAppInstallerCustomEventOperationProgress;

    } else if(event_type == JsAppInstallerOperationEventTypeFinished) {
        instance->current_work.error = event->finished.error;
        custom_event = JsAppInstallerCustomEventOperationFinished;

    } else {
        furi_crash("Invalid JsAppInstallerOperationEventType value");
    }

    furi_event_loop_set_custom_event(instance->event_loop, custom_event);
}

static void run_next_operation(JsAppInstaller* instance) {
    JsAppInstallerCurrentWork* current_work = &instance->current_work;
    furi_assert(current_work->operation == NULL);

    if(JsAppInstallerOperationQueue_empty_p(instance->pending_queue)) {
        FURI_LOG_D(TAG, "No more pending operations");
        return;
    }

    JsAppInstallerOperationDetail* current_detail = &current_work->detail;
    current_detail->step = "starting";
    current_detail->has_progress = false;

    JsAppInstallerQueueItem* item =
        JsAppInstallerOperationQueue_pop_front(instance->pending_queue);
    JsAppInstallerOperation* operation = js_app_installer_operation_alloc(item);
    js_app_installer_operation_set_callback(operation, operation_event_callback, instance);

    current_work->item = item;
    current_work->operation = operation;
    current_work->error = JsAppInstallerErrorMax;

    js_app_installer_operation_start(operation);

    // TODO: Emit update event

    FURI_LOG_D(
        TAG,
        "Operation %s %s (%lu) started",
        js_app_installer_get_operation_name(item->type),
        furi_string_get_cstr(item->app_id),
        item->id);
}

static void handle_operation_progress(JsAppInstaller* instance) {
    const JsAppInstallerCurrentWork* current_work = &instance->current_work;
    const JsAppInstallerQueueItem* current_item = current_work->item;
    furi_assert(current_item != NULL);

    FURI_CRITICAL_ENTER();
    const JsAppInstallerOperationDetail detail = current_work->detail;
    FURI_CRITICAL_EXIT();

    if(detail.has_progress) {
        FURI_LOG_D(
            TAG,
            "Operation status: step: %s, progress: %lu%% (%lu)",
            detail.step,
            detail.progress,
            current_item->id);
    } else {
        FURI_LOG_D(TAG, "Operation status: step: %s (%lu)", detail.step, current_item->id);
    }

    // TODO: Emit update event
}

static void handle_operation_finished(JsAppInstaller* instance) {
    JsAppInstallerCurrentWork* current_work = &instance->current_work;
    JsAppInstallerQueueItem* current_item = current_work->item;
    furi_assert(current_item != NULL);

    JsAppInstallerOperation* current_operation = current_work->operation;
    furi_assert(current_operation != NULL);

    const JsAppInstallerError error = current_work->error;
    furi_assert(error < JsAppInstallerErrorMax);

    const uint32_t operation_id = current_item->id;
    const char* operation_name = js_app_installer_get_operation_name(current_item->type);
    const char* app_id = furi_string_get_cstr(current_item->app_id);

    if(error == JsAppInstallerErrorNone) {
        FURI_LOG_D(
            TAG,
            "Operation %s %s (%lu) finished successfully",
            operation_name,
            app_id,
            operation_id);

    } else {
        FURI_LOG_E(
            TAG,
            "Operation %s %s (%lu) finished with error 0x%X",
            operation_name,
            app_id,
            operation_id,
            error);
    }

    js_app_installer_queue_item_free(current_item);
    js_app_installer_operation_free(current_operation);

    current_work->item = NULL;
    current_work->operation = NULL;

    run_next_operation(instance);
}

static JsAppInstallerError
    enqueue_operation(JsAppInstaller* instance, JsAppInstallerQueueItem* item) {
    JsAppInstallerError error = JsAppInstallerErrorNone;

    const size_t items_count = JsAppInstallerOperationQueue_size(instance->pending_queue);
    if(items_count < JS_APP_INSTALLER_MAX_QUEUE_ITEMS) {
        item->id = instance->operation_id_counter++;
        JsAppInstallerOperationQueue_push_back(instance->pending_queue, item);

        if(instance->current_work.operation == NULL) {
            run_next_operation(instance);
        }

    } else {
        error = JsAppInstallerErrorQueueFull;
    }

    return error;
}

static JsAppInstallerError handle_install(JsAppInstaller* instance, JsAppInstallerMsg* message) {
    furi_assert(instance);
    furi_assert(message);

    const JsAppInstallerMsgInstall* msg = &message->install;
    JsAppInstallerQueueItem* item = js_app_installer_queue_item_alloc(
        msg->app_id, JsAppInstallerOperationTypeInstall, msg->source_info);

    const JsAppInstallerError error = enqueue_operation(instance, item);
    if(error == JsAppInstallerErrorNone) {
        *msg->item_id = item->id;
    } else {
        js_app_installer_queue_item_free(item);
    }

    return error;
}

static JsAppInstallerError handle_remove(JsAppInstaller* instance, JsAppInstallerMsg* message) {
    furi_assert(instance);
    furi_assert(message);

    JsAppInstallerMsgRemove* msg = &message->remove;
    JsAppInstallerQueueItem* item =
        js_app_installer_queue_item_alloc(msg->app_id, JsAppInstallerOperationTypeRemove);

    const JsAppInstallerError error = enqueue_operation(instance, item);
    if(error == JsAppInstallerErrorNone) {
        *msg->item_id = item->id;
    } else {
        js_app_installer_queue_item_free(item);
    }

    return error;
}

static void cancel_current_operation(JsAppInstaller* instance) {
    JsAppInstallerCurrentWork* current_work = &instance->current_work;
    JsAppInstallerOperation* current_operation = current_work->operation;

    if(current_operation != NULL) {
        furi_assert(current_work->item != NULL);
        js_app_installer_operation_cancel(current_operation);
    }
}

static bool cancel_current_operation_with_id(JsAppInstaller* instance, uint32_t item_id) {
    bool is_found = false;

    JsAppInstallerCurrentWork* current_work = &instance->current_work;
    JsAppInstallerOperation* current_operation = current_work->operation;

    if(current_operation != NULL) {
        const JsAppInstallerQueueItem* current_item = current_work->item;
        furi_assert(current_item != NULL);

        if(current_item->id == item_id) {
            js_app_installer_operation_cancel(current_operation);
            is_found = true;
        }
    }

    return is_found;
}

static bool cancel_pending_operation(JsAppInstaller* instance, uint32_t item_id) {
    bool is_found = false;

    JsAppInstallerOperationQueue_it_t it;

    for(JsAppInstallerOperationQueue_it(it, instance->pending_queue);
        !JsAppInstallerOperationQueue_end_p(it);
        JsAppInstallerOperationQueue_next(it)) {
        const JsAppInstallerQueueItem* item = JsAppInstallerOperationQueue_cref(it);

        if(item->id == item_id) {
            JsAppInstallerOperationQueue_remove(instance->pending_queue, it);
            is_found = true;
            break;
        }
    }

    return is_found;
}

static JsAppInstallerError handle_cancel(JsAppInstaller* instance, JsAppInstallerMsg* message) {
    furi_assert(instance);
    furi_assert(message);

    JsAppInstallerError error = JsAppInstallerErrorNone;
    const JsAppInstallerMsgCancel* msg = &message->cancel;

    if(msg->is_clear_all) {
        cancel_current_operation(instance);
        JsAppInstallerOperationQueue_reset(instance->pending_queue);

    } else if(
        !cancel_current_operation_with_id(instance, msg->item_id) &&
        !cancel_pending_operation(instance, msg->item_id)) {
        error = JsAppInstallerErrorNotFound;
    }

    return error;
}

static JsAppInstallerError handle_get_queue(JsAppInstaller* instance, JsAppInstallerMsg* message) {
    furi_assert(instance);
    furi_assert(message);

    const JsAppInstallerMsgGetQueue* msg = &message->get_queue;

    JsAppInstallerCurrentWork* current_work = &instance->current_work;
    const JsAppInstallerQueueItem* current_item = current_work->item;

    if(current_item != NULL) {
        JsAppInstallerOperationInfo operation_info;
        js_app_installer_queue_item_get_partial_info(current_item, &operation_info);

        operation_info.status = JsAppInstallerOperationStatusInProgress;
        operation_info.detail = current_work->detail;

        msg->callback(&operation_info, msg->callback_context);
    }

    JsAppInstallerOperationQueue_it_ct it;
    for(JsAppInstallerOperationQueue_it(it, instance->pending_queue);
        !JsAppInstallerOperationQueue_end_p(it);
        JsAppInstallerOperationQueue_next(it)) {
        const JsAppInstallerQueueItem* item = JsAppInstallerOperationQueue_ref(it);

        JsAppInstallerOperationInfo operation_info;
        js_app_installer_queue_item_get_partial_info(item, &operation_info);

        msg->callback(&operation_info, msg->callback_context);
    }

    return JsAppInstallerErrorNone;
}

static const MessageHandler msg_handlers[] = {
    [JsAppInstallerCmdStage] = handle_stage,
    [JsAppInstallerCmdInstall] = handle_install,
    [JsAppInstallerCmdInstallStaged] = handle_install_staged,
    [JsAppInstallerCmdRemove] = handle_remove,
    [JsAppInstallerCmdCancel] = handle_cancel,
    [JsAppInstallerCmdGetQueue] = handle_get_queue,
};

static_assert(COUNT_OF(msg_handlers) == JsAppInstallerCmdMax);

static void message_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    JsAppInstaller* instance = context;
    furi_assert(object == instance->msg_queue);

    JsAppInstallerMsg msg;
    while(furi_message_queue_get(instance->msg_queue, &msg, 0) == FuriStatusOk) {
        const MessageHandler handler = msg_handlers[msg.cmd];

        furi_assert(handler);
        furi_assert(msg.error);
        *msg.error = handler(instance, &msg);

        if(msg.api_lock != NULL) {
            api_lock_unlock(msg.api_lock);
        }
    }
}

static void custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);

    JsAppInstaller* instance = context;

    if(events & JsAppInstallerCustomEventOperationProgress) {
        handle_operation_progress(instance);
    }
    if(events & JsAppInstallerCustomEventOperationFinished) {
        handle_operation_finished(instance);
    }
}

static JsAppInstaller* js_app_installer_alloc(void) {
    JsAppInstaller* instance = malloc(sizeof(JsAppInstaller));

    instance->event_loop = furi_event_loop_alloc();
    instance->msg_queue = furi_message_queue_alloc(API_QUEUE_SIZE, sizeof(JsAppInstallerMsg));

    JsAppInstallerOperationQueue_init(instance->pending_queue);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->msg_queue,
        FuriEventLoopEventIn,
        message_queue_callback,
        instance);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, custom_event_callback, instance);

    furi_record_create(RECORD_JS_APP_INSTALLER, instance);

    return instance;
}

int32_t js_app_installer_srv(void* arg) {
    UNUSED(arg);

    JsAppInstaller* instance = js_app_installer_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
