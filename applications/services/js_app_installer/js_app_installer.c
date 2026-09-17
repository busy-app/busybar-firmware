#include "js_app_installer_i.h"
#include "js_app_installer_paths.h"

#include <toolbox/tar/tar_archive.h>
#include <storage/storage.h>
#include <js_app/js_app.h>
#include <js_app/js_app_registry.h>
#include <storage_utils/dir_walk.h>

#define TAG          "JsAppInstaller"
#define MAX_MESSAGES 4

typedef void (*MessageHandler)(JsAppInstaller* instance, JsAppInstallerMsg* message);

static void message_queue_callback(FuriEventLoopObject* object, void* context);

static void handle_stage(JsAppInstaller* instance, JsAppInstallerMsg* message);
static void handle_install(JsAppInstaller* instance, JsAppInstallerMsg* message);

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

static bool
    find_app_folder(JsAppInstaller* instance, Storage* storage, JsAppInstallerStageResult* result) {
    bool found = false;
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
        while(!found && dir_walk_read(walk, path, NULL) == DirWalkOK) {
            if(js_app_load_from_directory(app, furi_string_get_cstr(path))) {
                FURI_LOG_I(TAG, "Found app at %s", furi_string_get_cstr(path));

                JsAppInfo info;
                if(js_app_get_info(app, &info)) {
                    instance->staged_install_key = gen_install_key();
                    instance->staged_app_path = furi_string_alloc_set(path);

                    result->staged_app = app;
                    app = NULL;

                    result->install_key = instance->staged_install_key;
                    result->error = JsAppInstallerErrorNone;
                    result->installed_app = js_app_registry_get_app(info.manifest.id);

                    found = true;
                }
            }
        }
        furi_string_free(path);
        if(app) {
            js_app_free(app);
        }
    } while(false);
    dir_walk_free(walk);
    return found;
}

static void handle_stage(JsAppInstaller* instance, JsAppInstallerMsg* message) {
    furi_assert(instance);
    furi_assert(message);

    JsAppInstallerStageResult* result = message->stage.result;

    instance->staged_install_key = 0;
    if(instance->staged_app_path) {
        furi_string_free(instance->staged_app_path);
        instance->staged_app_path = NULL;
    }

    Storage* storage = furi_record_open(RECORD_STORAGE);
    do {
        if(!prepare_staging_folder(storage)) {
            result->error = JsAppInstallerErrorUnpack;
            break;
        }

        TarArchive* tar = tar_archive_alloc(storage);
        do {
            if(!tar_archive_open(tar, JS_APP_DOWNLOAD_PATH, TarOpenModeReadAuto)) {
                FURI_LOG_E(TAG, "Cannot open " JS_APP_DOWNLOAD_PATH);
                result->error = JsAppInstallerErrorUnpack;
                break;
            }

            if(!tar_archive_unpack_to(tar, JS_APP_STAGING_PATH, NULL)) {
                FURI_LOG_E(TAG, "Cannot unpack " JS_APP_DOWNLOAD_PATH);
                result->error = JsAppInstallerErrorUnpack;
                break;
            }

            if(!find_app_folder(instance, storage, result)) {
                FURI_LOG_E(TAG, "Cannot find a valid application folder");
                result->error = JsAppInstallerErrorManifest;
                break;
            }
        } while(false);
        tar_archive_free(tar);
    } while(false);
    furi_record_close(RECORD_STORAGE);
}

JsAppInstallerStageResult js_app_installer_stage(JsAppInstaller* instance) {
    JsAppInstallerStageResult result;

    bzero(&result, sizeof(result));

    JsAppInstallerMsg msg = {
        .cmd = JsAppInstallerCmdStage,
        .api_lock = api_lock_alloc_locked(),
        .stage = {.result = &result},
    };

    furi_check(furi_message_queue_put(instance->msg_queue, &msg, FuriWaitForever) == FuriStatusOk);

    api_lock_wait_unlock_and_free(msg.api_lock);
    return result;
}

static void handle_install(JsAppInstaller* instance, JsAppInstallerMsg* message) {
    furi_assert(instance);
    furi_assert(message);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    JsAppInstallerError result = JsAppInstallerErrorInstall;
    bool cleanup = false;
    do {
        if(message->install.install_key == 0 ||
           message->install.install_key != instance->staged_install_key) {
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
                result = JsAppInstallerErrorNone;
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

    *message->install.error = result;
}

JsAppInstallerError js_app_installer_install(JsAppInstaller* instance, uint32_t install_key) {
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

static JsAppInstaller* js_app_installer_alloc(void) {
    JsAppInstaller* instance = malloc(sizeof(JsAppInstaller));

    instance->event_loop = furi_event_loop_alloc();
    instance->msg_queue = furi_message_queue_alloc(MAX_MESSAGES, sizeof(JsAppInstallerMsg));

    instance->staged_install_key = 0;
    instance->staged_app_path = NULL;

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->msg_queue,
        FuriEventLoopEventIn,
        message_queue_callback,
        instance);

    furi_record_create(RECORD_JS_APP_INSTALLER, instance);

    return instance;
}

static void js_app_installer_free(JsAppInstaller* instance) {
    furi_event_loop_unsubscribe(instance->event_loop, instance->msg_queue);
    furi_message_queue_free(instance->msg_queue);
    furi_event_loop_free(instance->event_loop);

    if(instance->staged_app_path) {
        furi_string_free(instance->staged_app_path);
    }
    furi_record_destroy(RECORD_JS_APP_INSTALLER);
    free(instance);
}

static const MessageHandler msg_handlers[] = {
    [JsAppInstallerCmdStage] = handle_stage,
    [JsAppInstallerCmdInstall] = handle_install,
};

static_assert(COUNT_OF(msg_handlers) == JsAppInstallerCmdMax);

static void message_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    JsAppInstaller* instance = context;
    furi_assert(instance);

    JsAppInstallerMsg msg;
    while(furi_message_queue_get(instance->msg_queue, &msg, 0) == FuriStatusOk) {
        MessageHandler handler = msg_handlers[msg.cmd];
        furi_check(handler);
        handler(instance, &msg);
        if(msg.api_lock) {
            api_lock_unlock(msg.api_lock);
        }
    }
}

int32_t js_app_installer_app(void* arg) {
    UNUSED(arg);

    JsAppInstaller* instance = js_app_installer_alloc();
    furi_event_loop_run(instance->event_loop);
    js_app_installer_free(instance);

    return 0;
}
