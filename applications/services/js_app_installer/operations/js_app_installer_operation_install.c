#include "../js_app_installer_operation_i.h"

#include <core/check.h>
#include <core/kernel.h>
#include <core/semaphore.h>
#include <core/log.h>

#include <fetch/fetch_loader.h>

#include <storage_utils/dir_walk.h>
#include <toolbox/tar/tar_archive.h>

#include "../js_app_installer_paths.h"

#define TAG "JsAppInstallerOperationInstall"

typedef enum {
    JsAppInstallerOperationInstallStepIdDownload,
    JsAppInstallerOperationInstallStepIdUnpack,
    JsAppInstallerOperationInstallStepIdInstall,
    JsAppInstallerOperationInstallStepIdMax,
} JsAppInstallerOperationInstallStepId;

typedef JsAppInstallerError (*JsAppInstallerOperationInstallStepHandler)(
    JsAppInstallerOperation* instance);

typedef struct {
    const char* const name;
    JsAppInstallerOperationInstallStepHandler handler;
} JsAppInstallerOperationInstallStep;

typedef struct {
    FuriString* url;
    FuriSemaphore* semaphore;
    _Atomic FetchStatus fetch_status;
} JsAppInstallerOperationInstall;

static const JsAppInstallerOperationInstallStep js_app_installer_operation_install_steps[];

static JsAppInstallerOperationData*
    js_app_installer_operation_install_alloc(const JsAppInstallerQueueItem* item) {
    furi_assert(item);
    furi_assert(item->type == JsAppInstallerOperationTypeInstall);

    JsAppInstallerOperationInstall* data = malloc(sizeof(JsAppInstallerOperationInstall));

    data->url = furi_string_alloc_set(item->url);
    data->semaphore = furi_semaphore_alloc(1, 0);

    return data;
}

static void js_app_installer_operation_install_free(JsAppInstallerOperationData* operation_data) {
    furi_assert(operation_data);
    JsAppInstallerOperationInstall* data = operation_data;

    furi_string_free(data->url);
    furi_semaphore_free(data->semaphore);

    free(data);
}

static void js_app_installer_operation_install_loader_progress_callback(
    const FetchProgress* progress,
    void* context) {
    furi_assert(context);
    JsAppInstallerOperation* instance = context;

    uint32_t percent_complete = 0;
    const bool has_progress = progress->has_total_download_size;
    if(has_progress) {
        percent_complete =
            ((float)progress->received_download_size / progress->total_download_size) * 100;
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
    js_app_installer_operation_emit(instance, &event);
}

static void
    js_app_installer_operation_install_loader_done_callback(FetchStatus status, void* context) {
    furi_assert(context);

    JsAppInstallerOperation* instance = context;
    JsAppInstallerOperationInstall* data = instance->data;

    data->fetch_status = status;
    furi_check(furi_semaphore_release(data->semaphore) == FuriStatusOk);
}

static JsAppInstallerError
    js_app_installer_operation_install_translate_fetch_status(FetchStatus status) {
    static const JsAppInstallerError error_table[] = {
        [FetchStatusOk] = JsAppInstallerErrorNone,
        [FetchStatusError] = JsAppInstallerErrorInstall,
        [FetchStatusAborted] = JsAppInstallerErrorInstall,
    };
    return error_table[status];
}

static JsAppInstallerError
    js_app_installer_operation_install_handler_download(JsAppInstallerOperation* instance) {
    JsAppInstallerOperationInstall* data = instance->data;
    FetchLoader* loader = fetch_loader_alloc();

    fetch_loader_set_progress_callback(
        loader, js_app_installer_operation_install_loader_progress_callback);
    fetch_loader_set_done_callback(
        loader, js_app_installer_operation_install_loader_done_callback);
    fetch_loader_set_callback_context(loader, instance);

    fetch_loader_start(loader, furi_string_get_cstr(data->url), JS_APP_DOWNLOAD_PATH);
    furi_check(furi_semaphore_acquire(data->semaphore, FuriWaitForever) == FuriStatusOk);

    fetch_loader_free(loader);

    return js_app_installer_operation_install_translate_fetch_status(data->fetch_status);
}

static JsAppInstallerError
    js_app_installer_operation_install_handler_unpack(JsAppInstallerOperation* instance) {
    UNUSED(instance);

    JsAppInstallerError error = JsAppInstallerErrorUnpack;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    TarArchive* tar = tar_archive_alloc(storage);

    do {
        if(!storage_simply_remove_recursive(storage, JS_APP_STAGING_PATH)) {
            FURI_LOG_E(TAG, "Failed to remove staging directory: %s", JS_APP_STAGING_PATH);
            break;
        }

        if(!tar_archive_open(tar, JS_APP_DOWNLOAD_PATH, TarOpenModeReadAuto)) {
            FURI_LOG_E(TAG, "Failed to open file: %s", JS_APP_DOWNLOAD_PATH);
            break;
        }

        if(!tar_archive_unpack_to(tar, JS_APP_STAGING_PATH, NULL)) {
            FURI_LOG_E(TAG, "Failed to unpack file: %s", JS_APP_DOWNLOAD_PATH);
            break;
        }

        error = JsAppInstallerErrorNone;
    } while(false);

    tar_archive_free(tar);
    furi_record_close(RECORD_STORAGE);

    return error;
}

static JsAppInstallerError
    js_app_installer_operation_install_handler_install(JsAppInstallerOperation* instance) {
    UNUSED(instance);
    return JsAppInstallerErrorNone;
}

static JsAppInstallerError
    js_app_installer_operation_install_run(JsAppInstallerOperation* instance) {
    furi_assert(instance);

    const char* app_id = furi_string_get_cstr(instance->app_id);
    FURI_LOG_D(TAG, "Installing app: %s", app_id);

    JsAppInstallerError error;

    for(size_t i = 0; i < JsAppInstallerOperationInstallStepIdMax; ++i) {
        const JsAppInstallerOperationInstallStep* step =
            &js_app_installer_operation_install_steps[i];
        /* clang-format off */
        const JsAppInstallerOperationEvent event = {
            .type = JsAppInstallerOperationEventTypeProgress,
            .progress.detail = {
                .step = step->name,
                .has_progress = false,
            },
        };
        /* clang-format on */
        js_app_installer_operation_emit(instance, &event);

        error = step->handler(instance);
        if(error != JsAppInstallerErrorNone) {
            FURI_LOG_E(TAG, "Step %s failed with error 0x%X", step->name, error);
            break;
        }
    }

    return error;
}

const JsAppInstallerOperationInterface js_app_installer_operation_install = {
    .alloc = js_app_installer_operation_install_alloc,
    .free = js_app_installer_operation_install_free,
    .run = js_app_installer_operation_install_run,
};

/* clang-format off */
static const JsAppInstallerOperationInstallStep js_app_installer_operation_install_steps[] = {
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
};
/* clang-format on */
