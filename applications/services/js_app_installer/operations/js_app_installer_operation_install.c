#include "../js_app_installer_operation_i.h"

#include <math.h>

#include <core/check.h>
#include <core/kernel.h>
#include <core/semaphore.h>
#include <core/log.h>

#include <web_server/web_server.h>

#include <js_app/js_app.h>
#include <js_app/js_app_registry.h>

#include <fetch/fetch_loader.h>
#include <storage_utils/dir_walk.h>

#include <toolbox/path.h>
#include <toolbox/tar/tar_archive.h>

#include "../js_app_installer_paths.h"

#define TAG "JsAppInstallerOperationInstall"

typedef enum {
    JsAppInstallerOperationInstallStepIdDownload,
    JsAppInstallerOperationInstallStepIdUnpack,
    JsAppInstallerOperationInstallStepIdInstall,
    JsAppInstallerOperationInstallStepIdMax,
} JsAppInstallerOperationInstallStepId;

typedef struct {
    JsAppInstallerOperation base;
    Storage* storage;
    FuriString* url;
    FuriSemaphore* semaphore;
    FetchLoader* loader;
    _Atomic FetchStatus fetch_status;
} JsAppInstallerOperationInstall;

typedef JsAppInstallerError (*JsAppInstallerOperationInstallStepHandler)(
    JsAppInstallerOperationInstall* instance);

static void js_app_installer_operation_install_init(
    JsAppInstallerOperation* base_instance,
    const JsAppInstallerQueueItem* item) {
    furi_assert(base_instance);
    furi_assert(item);
    furi_assert(item->type == JsAppInstallerOperationTypeInstall);

    JsAppInstallerOperationInstall* instance = (JsAppInstallerOperationInstall*)base_instance;

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->url = furi_string_alloc_set(item->url);
    instance->semaphore = furi_semaphore_alloc(1, 0);
    instance->loader = fetch_loader_alloc();
}

static void js_app_installer_operation_install_deinit(JsAppInstallerOperation* base_instance) {
    furi_assert(base_instance);
    JsAppInstallerOperationInstall* instance = (JsAppInstallerOperationInstall*)base_instance;

    furi_string_free(instance->url);
    furi_semaphore_free(instance->semaphore);
    fetch_loader_free(instance->loader);

    furi_record_close(RECORD_STORAGE);
}

static void js_app_installer_operation_install_cancel(JsAppInstallerOperation* base_instance) {
    furi_assert(base_instance);
    JsAppInstallerOperationInstall* instance = (JsAppInstallerOperationInstall*)base_instance;
    fetch_loader_stop(instance->loader);
}

static void js_app_installer_operation_install_cleanup(JsAppInstallerOperation* base_instance) {
    furi_assert(base_instance);
    JsAppInstallerOperationInstall* instance = (JsAppInstallerOperationInstall*)base_instance;

    Storage* storage = instance->storage;
    storage_simply_remove_recursive(storage, JS_APP_STAGING_PATH);
    storage_simply_remove(storage, JS_APP_DOWNLOAD_PATH);
}

static void js_app_installer_operation_install_loader_progress_callback(
    const FetchProgress* progress,
    void* context) {
    furi_assert(context);
    JsAppInstallerOperation* base_instance = context;

    uint32_t percent_complete = 0;
    const bool has_progress = progress->has_total_download_size;
    if(has_progress) {
        const size_t total_size = progress->total_download_size;
        if(total_size != 0) {
            const float recv_size = progress->received_download_size;
            percent_complete = MIN(roundf((recv_size / total_size) * 100), 100.F);
        }
    }
    /* clang-format off */
    JsAppInstallerOperationEvent event = {
        .type = JsAppInstallerOperationEventTypeProgress,
        .progress.detail = {
            .step = "downloading",
            .has_progress = has_progress,
            .progress = percent_complete,
        }
    };
    /* clang-format on */
    js_app_installer_operation_notify(base_instance, &event);
}

static void
    js_app_installer_operation_install_loader_done_callback(FetchStatus status, void* context) {
    furi_assert(context);
    JsAppInstallerOperationInstall* instance = context;

    instance->fetch_status = status;
    furi_check(furi_semaphore_release(instance->semaphore) == FuriStatusOk);
}

static JsAppInstallerError
    js_app_installer_operation_install_translate_fetch_status(FetchStatus status) {
    furi_assert(status < FetchStatusMax);

    static const JsAppInstallerError error_table[] = {
        [FetchStatusOk] = JsAppInstallerErrorNone,
        [FetchStatusError] = JsAppInstallerErrorDownload,
        [FetchStatusAborted] = JsAppInstallerErrorCancelled,
    };

    static_assert(COUNT_OF(error_table) == FetchStatusMax);
    return error_table[status];
}

static JsAppInstallerError
    js_app_installer_operation_install_handler_download(JsAppInstallerOperation* base_instance) {
    JsAppInstallerOperationInstall* instance = (JsAppInstallerOperationInstall*)base_instance;
    FetchLoader* loader = instance->loader;

    fetch_loader_set_progress_callback(
        loader, js_app_installer_operation_install_loader_progress_callback);
    fetch_loader_set_done_callback(
        loader, js_app_installer_operation_install_loader_done_callback);
    fetch_loader_set_callback_context(loader, instance);

    fetch_loader_start(loader, furi_string_get_cstr(instance->url), JS_APP_DOWNLOAD_PATH);
    furi_check(furi_semaphore_acquire(instance->semaphore, FuriWaitForever) == FuriStatusOk);

    return js_app_installer_operation_install_translate_fetch_status(instance->fetch_status);
}

static JsAppInstallerError
    js_app_installer_operation_install_handler_unpack(JsAppInstallerOperation* base_instance) {
    JsAppInstallerOperationInstall* instance = (JsAppInstallerOperationInstall*)base_instance;

    JsAppInstallerError error;
    TarArchive* tar = tar_archive_alloc(instance->storage);

    do {
        if(!storage_simply_remove_recursive(instance->storage, JS_APP_STAGING_PATH)) {
            FURI_LOG_E(TAG, "Failed to remove staging directory: %s", JS_APP_STAGING_PATH);
            error = JsAppInstallerErrorStorage;
            break;
        }

        if(!tar_archive_open(tar, JS_APP_DOWNLOAD_PATH, TarOpenModeReadAuto)) {
            FURI_LOG_E(TAG, "Failed to open file: %s", JS_APP_DOWNLOAD_PATH);
            error = JsAppInstallerErrorUnpack;
            break;
        }

        if(!tar_archive_unpack_to(tar, JS_APP_STAGING_PATH, NULL)) {
            FURI_LOG_E(TAG, "Failed to unpack file: %s", JS_APP_DOWNLOAD_PATH);
            error = JsAppInstallerErrorUnpack;
            break;
        }

        error = JsAppInstallerErrorNone;
    } while(false);

    tar_archive_free(tar);
    return error;
}

static JsAppInstallerError js_app_installer_operation_install_find_staged_app(
    Storage* storage,
    const FuriString* app_id,
    JsApp* out_app,
    FuriString* out_path) {
    JsAppInstallerError error = JsAppInstallerErrorManifest;
    DirWalk* walk = dir_walk_alloc(storage);

    dir_walk_set_recursive(walk, false);
    dir_walk_set_filter_cb(walk, dir_walk_is_dir_callback, NULL);

    do {
        if(!dir_walk_open(walk, JS_APP_STAGING_PATH)) {
            error = JsAppInstallerErrorStorage;
            break;
        }

        while(dir_walk_read(walk, out_path, NULL) == DirWalkOK) {
            const char* path_str = furi_string_get_cstr(out_path);

            if(!js_app_load_from_directory(out_app, path_str)) {
                continue;
            }

            FURI_LOG_D(TAG, "Found app at: %s", path_str);

            JsAppInfo app_info;
            if(!js_app_get_info(out_app, &app_info)) {
                break;
            }

            const JsAppManifestInfo* manifest = &app_info.manifest;

            if(!furi_string_equal(app_id, manifest->id)) {
                continue;
            }

            const SemVer* api_version = &manifest->api_version;
            if(!web_server_is_api_version_compatible(api_version)) {
                error = JsAppInstallerErrorApiVersion;
                break;
            }

            error = JsAppInstallerErrorNone;
            break;
        }

    } while(false);

    dir_walk_free(walk);
    return error;
}

static JsAppInstallerError
    js_app_installer_operation_install_handler_install(JsAppInstallerOperation* base_instance) {
    JsAppInstallerOperationInstall* instance = (JsAppInstallerOperationInstall*)base_instance;

    JsAppInstallerError error;
    Storage* storage = instance->storage;

    JsApp* staged_app = js_app_alloc();
    FuriString* staged_path = furi_string_alloc();
    FuriString* target_path = NULL;

    do {
        error = js_app_installer_operation_install_find_staged_app(
            storage, instance->base.app_id, staged_app, staged_path);
        if(error != JsAppInstallerErrorNone) {
            break;
        }

        JsAppInfo app_info;
        if(!js_app_get_info(staged_app, &app_info)) {
            error = JsAppInstallerErrorManifest;
            break;
        }

        const char* staged_app_id = app_info.manifest.id;

        target_path = js_app_registry_get_app_path(staged_app_id);
        furi_assert(target_path);

        const char* target_path_str = furi_string_get_cstr(target_path);
        if(!storage_simply_remove_recursive(storage, target_path_str)) {
            FURI_LOG_E(TAG, "Failed to remove app with id: %s", staged_app_id);
            error = JsAppInstallerErrorStorage;
            break;
        }

        const char* staged_path_str = furi_string_get_cstr(staged_path);
        if(storage_common_rename(storage, staged_path_str, target_path_str) != FSE_OK) {
            FURI_LOG_E(
                TAG, "Failed to move app directory %s -> %s", staged_path_str, target_path_str);
            error = JsAppInstallerErrorStorage;
            break;
        }

        error = JsAppInstallerErrorNone;
    } while(false);

    js_app_free(staged_app);
    furi_string_free(staged_path);

    if(target_path != NULL) {
        furi_string_free(target_path);
    }

    return error;
}

const JsAppInstallerOperationInterface js_app_installer_operation_install = {
    .init = js_app_installer_operation_install_init,
    .deinit = js_app_installer_operation_install_deinit,
    .cancel = js_app_installer_operation_install_cancel,
    .cleanup = js_app_installer_operation_install_cleanup,
    .data_size = sizeof(JsAppInstallerOperationInstall),
    .step_count = JsAppInstallerOperationInstallStepIdMax,
    /* clang-format off */
    .steps = {
        [JsAppInstallerOperationInstallStepIdDownload] = {
            .name = "downloading",
            .handler = js_app_installer_operation_install_handler_download,
        },
        [JsAppInstallerOperationInstallStepIdUnpack] = {
            .name = "unpacking",
            .handler = js_app_installer_operation_install_handler_unpack,
        },
        [JsAppInstallerOperationInstallStepIdInstall] = {
            .name = "installing",
            .handler = js_app_installer_operation_install_handler_install,
        },
    },
    /* clang-format on */
};
