#include "js_app_installer_i.h"
#include "js_app_installer_paths.h"

#include <toolbox/tar/tar_archive.h>
#include <storage/storage.h>
#include <web_server/web_server.h>
#include <js_app/js_app.h>
#include <js_app/js_app_registry.h>
#include <storage_utils/dir_walk.h>

#define TAG "JsAppInstaller"

#define MAX_MESSAGES   4
#define MAX_OPERATIONS 100

typedef enum {
    JsAppInstallerCustomEventOperationFinished = 1UL << 0,
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
    JsAppInstallerError error = JsAppInstallerErrorManifest;
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
                continue;
            }

            FURI_LOG_I(TAG, "Found app at %s", furi_string_get_cstr(path));

            JsAppInfo app_info;
            if(!js_app_get_info(app, &app_info)) {
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
    JsAppInstallerError error = JsAppInstallerErrorInstall;
    bool cleanup = false;
    do {
        if(message->install_staged.install_key == 0 ||
           message->install_staged.install_key != instance->staged_install_key) {
            break;
        }

        cleanup = true;
        JsApp* new_app = js_app_alloc();
        do {
            if(!js_app_load_from_directory(
                   new_app, furi_string_get_cstr(instance->staged_app_path))) {
                break;
            }
            JsAppInfo new_app_info;
            if(!js_app_get_info(new_app, &new_app_info)) {
                break;
            }
            FuriString* target_path = js_app_registry_get_app_path(new_app_info.manifest.id);
            furi_assert(target_path);

            do {
                if(!storage_simply_remove_recursive(storage, furi_string_get_cstr(target_path))) {
                    FURI_LOG_E(TAG, "Cannot delete app with id = %s", new_app_info.manifest.id);
                    break;
                }
                if(storage_common_rename(
                       storage,
                       furi_string_get_cstr(instance->staged_app_path),
                       furi_string_get_cstr(target_path)) != FSE_OK) {
                    FURI_LOG_E(TAG, "Cannot move directory");
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

    if(event->type == JsAppInstallerOperationEventTypeFinished) {
        furi_event_loop_set_custom_event(
            instance->event_loop, JsAppInstallerCustomEventOperationFinished);
    }
}

static void run_next_operation(JsAppInstaller* instance) {
    furi_check(instance->current_operation == NULL);

    if(JsAppInstallerOperationQueue_empty_p(instance->pending_queue)) {
        FURI_LOG_D(TAG, "No more pending operations");
        return;
    }

    JsAppInstallerQueueItem* item =
        JsAppInstallerOperationQueue_pop_front(instance->pending_queue);
    JsAppInstallerOperation* operation = js_app_installer_operation_alloc();
    js_app_installer_operation_set_callback(operation, operation_event_callback, instance);
    js_app_installer_operation_start(operation);

    instance->current_item = item;
    instance->current_operation = operation;

    FURI_LOG_D(
        TAG,
        "Next operation: %s %s (%lu)",
        js_app_installer_get_operation_name(item->type),
        furi_string_get_cstr(item->app_id),
        item->id);
}

static void handle_operation_finished(JsAppInstaller* instance) {
    JsAppInstallerQueueItem* item = instance->current_item;
    furi_check(item != NULL);

    JsAppInstallerOperation* operation = instance->current_operation;
    furi_check(operation != NULL);

    FURI_LOG_D(
        TAG,
        "Operation finished: %s %s (%lu)",
        js_app_installer_get_operation_name(item->type),
        furi_string_get_cstr(item->app_id),
        item->id);

    js_app_installer_queue_item_free(item);
    js_app_installer_operation_free(operation);

    instance->current_item = NULL;
    instance->current_operation = NULL;

    run_next_operation(instance);
}

static JsAppInstallerError
    enqueue_operation(JsAppInstaller* instance, JsAppInstallerQueueItem* item) {
    JsAppInstallerError error = JsAppInstallerErrorNone;

    if(JsAppInstallerOperationQueue_size(instance->pending_queue) < MAX_OPERATIONS) {
        item->id = instance->operation_id_counter++;
        JsAppInstallerOperationQueue_push_back(instance->pending_queue, item);

        if(instance->current_operation == NULL) {
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
    JsAppInstallerOperation* current_operation = instance->current_operation;

    if(current_operation != NULL) {
        furi_assert(instance->current_item != NULL);
        js_app_installer_operation_cancel(current_operation);
    }
}

static bool cancel_current_operation_with_id(JsAppInstaller* instance, uint32_t item_id) {
    bool is_found = false;

    JsAppInstallerOperation* current_operation = instance->current_operation;

    if(current_operation != NULL) {
        const JsAppInstallerQueueItem* current_item = instance->current_item;
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
    const JsAppInstallerQueueItem* current_item = instance->current_item;

    if(current_item != NULL) {
        JsAppInstallerOperationInfo operation_info;
        js_app_installer_queue_item_get_partial_info(current_item, &operation_info);

        operation_info.status = JsAppInstallerOperationStatusInProgress;
        operation_info.detail = instance->current_detail;

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
        furi_check(handler);

        *msg.error = handler(instance, &msg);

        if(msg.api_lock != NULL) {
            api_lock_unlock(msg.api_lock);
        }
    }
}

static void custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    JsAppInstaller* instance = context;

    if(events & JsAppInstallerCustomEventOperationFinished) {
        handle_operation_finished(instance);
    }
}

static JsAppInstaller* js_app_installer_alloc(void) {
    JsAppInstaller* instance = malloc(sizeof(JsAppInstaller));

    instance->event_loop = furi_event_loop_alloc();
    instance->msg_queue = furi_message_queue_alloc(MAX_MESSAGES, sizeof(JsAppInstallerMsg));

    instance->staged_install_key = 0;
    instance->staged_app_path = NULL;
    instance->current_operation = NULL;

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
