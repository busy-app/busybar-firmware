#include "updater.h"
#include "session/session_config.h"

#include <storage/storage.h>
#include <power/power_service/power.h>
#include <toolbox/fetch/fetch_loader.h>

#include <furi_hal_nvm.h>
#include <furi_hal_power.h>
#include <toolbox/api_lock.h>
#include <toolbox/path.h>
#include <toolbox/tar/tar_archive.h>
#include <toolbox/update_lib/common_vals.h>

#define TAG "Updater"

#define MESSAGE_QUEUE_ITEMS_COUNT 8

#define UPDATE_START_MIN_BATTERY_CHARGE 40
#define UPDATE_REBOOT_INSTALL_DELAY     100

#define DEFAULT_DOWNLOAD_PATH EXT_PATH("update/bundle.tar")
#define DEFAULT_STAGING_PATH  EXT_PATH("update/staging")
#define DEFAULT_MANIFEST_PATH DEFAULT_STAGING_PATH "/" UPDATE_CONFIG_FILENAME

struct Updater {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    FuriSemaphore* update_lock;
    FuriState* update_state;

    Storage* storage;
    Power* power;

    FuriString* update_detail;

    FetchLoader* download_loader;
    FuriMessageQueue* download_queue;
};

typedef struct {
    bool is_abort_request;
    UpdaterStatus status;
} DownloadQueueMessage;

typedef enum {
    MessageTypeStartUpdate,
    MessageTypeStopUpdate,
    MessageTypeDownload,
    MessageTypeUnpack,
    MessageTypePrepareInstall,
    MessageTypeRebootInstall,

    MessageTypesCount
} MessageType;

typedef struct {
    union {
        struct {
            FuriString* url;
            FuriString* path;
        } as_download;

        struct {
            FuriString* tar_path;
            FuriString* staging_path;
            FuriString* manifest_path;
        } as_unpack;

        struct {
            FuriString* manifest_path;
        } as_prepare_install;
    };

    FuriApiLock api_lock;
    UpdaterStatus* result_status;
    MessageType type;
} UpdaterMessage;

typedef struct {
    UpdaterStatus (*callback)(Updater* instance, UpdaterMessage* message);
    UpdaterUpdateAction action;
} MessageHandler;

static const char* const status_strings[];
static const MessageHandler message_handlers[];

static UpdaterStatus do_start_update(Updater* instance, UpdaterMessage* message) {
    UNUSED(message);

    UpdaterUpdateState* update_state = furi_state_acquire(instance->update_state);
    update_state->event = UpdaterUpdateEventStart;
    furi_state_release(instance->update_state);

    return UpdaterStatusOk;
}

static UpdaterStatus do_stop_update(Updater* instance, UpdaterMessage* message) {
    UNUSED(message);

    UpdaterUpdateState* update_state = furi_state_acquire(instance->update_state);
    update_state->event = UpdaterUpdateEventStop;
    furi_state_release(instance->update_state);

    return UpdaterStatusOk;
}

static void download_status_callback(const FetchLoaderStatus* status, void* context) {
    furi_assert(context);

    Updater* instance = context;

    UpdaterUpdateState* update_state = furi_state_acquire(instance->update_state);
    update_state->event = UpdaterUpdateEventActionProgress;
    update_state->as_download.total_size = status->total_download_size;
    update_state->as_download.received_size = status->received_download_size;
    update_state->as_download.speed_bytes_per_sec = status->speed_bytes_per_sec;
    furi_state_release(instance->update_state);
}

static void download_state_callback(const FuriString* state, void* context) {
    furi_assert(context);

    Updater* instance = context;

    UpdaterUpdateState* update_state = furi_state_acquire(instance->update_state);
    update_state->event = UpdaterUpdateEventDetailChange;
    furi_string_set(instance->update_detail, state);
    furi_state_release(instance->update_state);
}

static void download_done_callback(FetchLoaderDoneStatus done_status, void* context) {
    furi_assert(context);

    Updater* instance = context;

    UpdaterStatus update_status;
    switch(done_status) {
    case FetchLoaderDoneStatusSuccess:
        update_status = UpdaterStatusOk;
        break;

    case FetchLoaderDoneStatusFailure:
        update_status = UpdaterStatusDownloadFailure;
        break;

    case FetchLoaderDoneStatusAbort:
        update_status = UpdaterStatusDownloadAbort;
        break;

    default:
        update_status = UpdaterStatusUnknownFailure;
        break;
    }

    furi_message_queue_put(
        instance->download_queue,
        &(const DownloadQueueMessage){
            .is_abort_request = false,
            .status = update_status,
        },
        FuriWaitForever);
}

static UpdaterStatus do_download(Updater* instance, UpdaterMessage* message) {
    const char* url = furi_string_get_cstr(message->as_download.url);
    const char* path = furi_string_get_cstr(message->as_download.path);

    FURI_LOG_D(TAG, "Downloading update bundle from %s into %s", url, path);

    instance->download_loader = fetch_loader_alloc();

    fetch_loader_set_status_callback(
        instance->download_loader, download_status_callback, instance);
    fetch_loader_set_state_callback(instance->download_loader, download_state_callback, instance);
    fetch_loader_set_done_callback(instance->download_loader, download_done_callback, instance);

    fetch_loader_run(instance->download_loader, url, path);

    DownloadQueueMessage download_message;
    furi_message_queue_get(instance->download_queue, &download_message, FuriWaitForever);

    if(download_message.is_abort_request) {
        fetch_loader_forced_done(instance->download_loader);

        do {
            furi_message_queue_get(instance->download_queue, &download_message, FuriWaitForever);
        } while(download_message.is_abort_request);
    }

    switch(download_message.status) {
    case UpdaterStatusOk:
        FURI_LOG_D(TAG, "Update bundle downloaded successfully");
        break;

    case UpdaterStatusDownloadFailure:
        FURI_LOG_E(TAG, "Failed to download update bundle from %s into %s", url, path);
        break;

    case UpdaterStatusDownloadAbort:
        FURI_LOG_D(TAG, "Update bundle download aborted");
        break;

    case UpdaterStatusUnknownFailure:
    /* fall-through */
    default:
        FURI_LOG_D(TAG, "Update bundle download caused unknown failure");
        break;
    }

    fetch_loader_free(instance->download_loader);
    furi_string_free(message->as_download.url);
    furi_string_free(message->as_download.path);

    instance->download_loader = NULL;

    return download_message.status;
}

static UpdaterStatus do_unpack(Updater* instance, UpdaterMessage* message) {
    const char* tar_path = furi_string_get_cstr(message->as_unpack.tar_path);
    const char* staging_path = furi_string_get_cstr(message->as_unpack.staging_path);

    FURI_LOG_D(TAG, "Unpacking update bundle from %s into %s", tar_path, staging_path);

    if(storage_dir_exists(instance->storage, staging_path)) {
        FURI_LOG_D(TAG, "Cleaning up staging directory recursively...");
        storage_simply_remove_recursive(instance->storage, staging_path);
    }

    TarArchive* tar_archive = tar_archive_alloc(instance->storage);

    UpdaterStatus update_status;
    do {
        FURI_LOG_D(TAG, "Creating staging directory...");

        if(storage_common_mkdir(instance->storage, staging_path) != FSE_OK) {
            FURI_LOG_E(TAG, "Failed to create staging directory %s", staging_path);
            update_status = UpdaterStatusUnpackCreateStagingDirectoryFailure;
            break;
        }

        if(!tar_archive_open(tar_archive, tar_path, TarOpenModeRead)) {
            FURI_LOG_E(TAG, "Failed to open %s as .tar archive", tar_path);
            update_status = UpdaterStatusUnpackArchiveOpenFailure;
            break;
        }

        if(!tar_archive_unpack_to(tar_archive, staging_path, NULL)) {
            FURI_LOG_E(TAG, "Failed to unpack %s contents into %s", tar_path, staging_path);
            update_status = UpdaterStatusUnpackArchiveUnpackFailure;
            break;
        }

        if(message->as_unpack.manifest_path) {
            path_concat(staging_path, UPDATE_CONFIG_FILENAME, message->as_unpack.manifest_path);
        }

        FURI_LOG_D(TAG, "Update bundle unpacked successfully");

        update_status = UpdaterStatusOk;
    } while(false);

    tar_archive_free(tar_archive);
    furi_string_free(message->as_unpack.tar_path);
    furi_string_free(message->as_unpack.staging_path);

    return update_status;
}

static UpdaterStatus do_prepare_install(Updater* instance, UpdaterMessage* message) {
    const char* manifest_path = furi_string_get_cstr(message->as_prepare_install.manifest_path);

    FURI_LOG_D(TAG, "Preparing update bundle for installation using manifest %s", manifest_path);

    UpdateConfig* config = update_config_alloc();

    UpdaterStatus update_status;
    do {
        FURI_LOG_D(TAG, "Checking for manifest...");

        if(!storage_file_exists(instance->storage, manifest_path)) {
            FURI_LOG_E(TAG, "Manifest file not found: %s", manifest_path);
            update_status = UpdaterStatusPrepareInstallManifestNotFound;
            break;
        }

        FURI_LOG_D(TAG, "Loading manifest configuration...");

        UpdateConfigValidation validation_status = update_config_load(config, manifest_path);
        if(validation_status != UpdateConfigValidationOK) {
            FURI_LOG_E(
                TAG,
                "Failed to load updater configuration: %s",
                update_config_validation_get_error_str(validation_status));

            update_status = UpdaterStatusPrepareInstallManifestInvalid;
            break;
        }

        FURI_LOG_D(TAG, "Setting up session config...");

        UpdaterSessionConfig session_config;
        const UpdateManifest* manifest = update_config_get_manifest(config);
        updater_session_config_compose(manifest, &session_config);
        if(!updater_session_config_save(&session_config)) {
            FURI_LOG_E(TAG, "Failed to set up session config");
            update_status = UpdaterStatusPrepareInstallSessionConfigSetupFailure;
            break;
        }

        FURI_LOG_D(TAG, "Setting up pointer file...");

        if(!update_config_write_pointer_file(instance->storage, manifest_path)) {
            FURI_LOG_E(TAG, "Failed to set up pointer file");
            update_status = UpdaterStatusPrepareInstallPointerSetupFailure;

            updater_session_config_delete();

            break;
        }

        FURI_LOG_D(TAG, "Update bundle prepared for installation successfully");

        update_status = UpdaterStatusOk;
    } while(false);

    update_config_free(config);
    furi_string_free(message->as_prepare_install.manifest_path);

    return update_status;
}

static UpdaterStatus do_reboot_install(Updater* instance, UpdaterMessage* message) {
    UNUSED(instance);
    UNUSED(message);

    furi_hal_nvm_set_boot_mode(FuriHalNvmBootModeUpdate);

    FURI_LOG_D(TAG, "Boot mode set to \"update\", device will reboot...");

    furi_delay_ms(UPDATE_REBOOT_INSTALL_DELAY);
    furi_hal_power_reset();

    furi_crash();
}

static void message_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    Updater* instance = context;
    UpdaterUpdateState* update_state;

    UpdaterMessage message;
    furi_message_queue_get(instance->message_queue, &message, FuriWaitForever);
    const MessageHandler* handler = &message_handlers[message.type];

    if(handler->action != UpdaterUpdateActionNone) {
        update_state = furi_state_acquire(instance->update_state);
        update_state->event = UpdaterUpdateEventActionBegin;
        update_state->status = UpdaterStatusBusy;
        update_state->action = handler->action;

        furi_string_reset(instance->update_detail);

        furi_state_release(instance->update_state);
    }

    UpdaterStatus result_status = handler->callback(instance, &message);

    if(message.result_status) {
        *message.result_status = result_status;
    }

    if(message.api_lock) {
        api_lock_unlock(message.api_lock);
    }

    if(handler->action != UpdaterUpdateActionNone) {
        update_state = furi_state_acquire(instance->update_state);
        update_state->event = UpdaterUpdateEventActionDone;
        update_state->status = result_status;
        furi_state_release(instance->update_state);
    }
}

static UpdaterStatus invoke_async(Updater* instance, UpdaterMessage* message) {
    message->result_status = NULL;
    message->api_lock = NULL;

    furi_check(
        furi_message_queue_put(instance->message_queue, message, FuriWaitForever) == FuriStatusOk);

    return UpdaterStatusOk;
}

static UpdaterStatus invoke_sync(Updater* instance, UpdaterMessage* message) {
    UpdaterStatus update_status;

    message->result_status = &update_status;
    message->api_lock = api_lock_alloc_locked();

    furi_check(
        furi_message_queue_put(instance->message_queue, message, FuriWaitForever) == FuriStatusOk);

    api_lock_wait_unlock_and_free(message->api_lock);

    return update_status;
}

const char* updater_get_status_string(UpdaterStatus status) {
    return (status < UpdaterStatusesCount) ? status_strings[status] : "Unknown error code";
}

FuriState* updater_get_update_state(Updater* instance) {
    furi_check(instance);

    return instance->update_state;
}

UpdaterStatus updater_get_allowance_status(Updater* instance) {
    PowerInfo power_info;
    power_get_info(instance->power, &power_info);

    return (power_info.charge >= UPDATE_START_MIN_BATTERY_CHARGE ||
            (furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug) &&
             power_is_usb_connected(instance->power))) ?
               UpdaterStatusOk :
               UpdaterStatusBatteryLow;
}

UpdaterStatus updater_start_update(Updater* instance) {
    furi_check(instance);

    UpdaterStatus result_status;
    do {
        result_status = updater_get_allowance_status(instance);
        if(result_status != UpdaterStatusOk) {
            break;
        }

        if(furi_semaphore_acquire(instance->update_lock, 0) != FuriStatusOk) {
            result_status = UpdaterStatusBusy;
            break;
        }

        invoke_async(instance, &(UpdaterMessage){.type = MessageTypeStartUpdate});

        result_status = UpdaterStatusOk;
    } while(false);

    return result_status;
}

void updater_stop_update(Updater* instance) {
    furi_check(instance);

    invoke_async(instance, &(UpdaterMessage){.type = MessageTypeStopUpdate});

    furi_semaphore_release(instance->update_lock);
}

UpdaterStatus
    updater_download(Updater* instance, const char* url, const char* path, bool do_wait) {
    furi_check(instance);
    furi_check(url);
    furi_check(furi_semaphore_get_space(instance->update_lock) > 0);

    furi_message_queue_reset(instance->download_queue);

    UpdaterMessage message = {
        .as_download =
            {
                .url = furi_string_alloc_set_str(url),
                .path = furi_string_alloc_set_str((path) ?: DEFAULT_DOWNLOAD_PATH),
            },
        .type = MessageTypeDownload,
    };

    return (do_wait) ? invoke_sync(instance, &message) : invoke_async(instance, &message);
}

void updater_abort_download(Updater* instance) {
    furi_check(instance);
    furi_check(furi_semaphore_get_space(instance->update_lock) > 0);

    furi_message_queue_put(
        instance->download_queue,
        &(const DownloadQueueMessage){
            .is_abort_request = true,
        },
        FuriWaitForever);
}

UpdaterStatus updater_unpack(
    Updater* instance,
    const char* tar_path,
    const char* staging_path,
    FuriString* manifest_path,
    bool do_wait) {
    furi_check(instance);
    furi_check(furi_semaphore_get_space(instance->update_lock) > 0);

    UpdaterMessage message = {
        .as_unpack =
            {
                .tar_path = furi_string_alloc_set_str((tar_path) ?: DEFAULT_DOWNLOAD_PATH),
                .staging_path = furi_string_alloc_set_str((staging_path) ?: DEFAULT_STAGING_PATH),
                .manifest_path = manifest_path,
            },
        .type = MessageTypeUnpack,
    };

    return (do_wait) ? invoke_sync(instance, &message) : invoke_async(instance, &message);
}

UpdaterStatus updater_prepare_install(Updater* instance, const char* manifest_path, bool do_wait) {
    furi_check(instance);
    furi_check(furi_semaphore_get_space(instance->update_lock) > 0);

    UpdaterMessage message = {
        .as_prepare_install =
            {
                .manifest_path =
                    furi_string_alloc_set_str((manifest_path) ?: DEFAULT_MANIFEST_PATH),
            },
        .type = MessageTypePrepareInstall,
    };

    return (do_wait) ? invoke_sync(instance, &message) : invoke_async(instance, &message);
}

void updater_reboot_install(Updater* instance, bool do_wait) {
    furi_check(instance);
    furi_check(furi_semaphore_get_space(instance->update_lock) > 0);

    UpdaterMessage message = {
        .type = MessageTypeRebootInstall,
    };

    if(do_wait) {
        invoke_sync(instance, &message);
    } else {
        invoke_async(instance, &message);
    }
}

static Updater* updater_alloc(void) {
    Updater* instance = malloc(sizeof(*instance));

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->power = furi_record_open(RECORD_POWER);

    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue =
        furi_message_queue_alloc(MESSAGE_QUEUE_ITEMS_COUNT, sizeof(UpdaterMessage));
    instance->update_lock = furi_semaphore_alloc(1, 1);
    instance->update_state = furi_state_alloc(sizeof(UpdaterUpdateState));

    instance->update_detail = furi_string_alloc();

    instance->download_queue = furi_message_queue_alloc(1, sizeof(DownloadQueueMessage));

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        message_queue_callback,
        instance);

    furi_state_set(
        instance->update_state,
        &(const UpdaterUpdateState){
            .event = UpdaterUpdateEventNone,
            .action = UpdaterUpdateActionNone,
            .detail = instance->update_detail,
        });

    furi_record_create(RECORD_UPDATER, instance);

    return instance;
}

int32_t updater_srv(void* p) {
    UNUSED(p);

    Updater* instance = updater_alloc();

    furi_event_loop_run(instance->event_loop);

    return 0;
}

static const char* const status_strings[] = {
    [UpdaterStatusOk] = "Success",
    [UpdaterStatusBatteryLow] = "Battery level too low",
    [UpdaterStatusBusy] = "Update already in progress",
    [UpdaterStatusDownloadFailure] = "Failed to download update bundle",
    [UpdaterStatusDownloadAbort] = "Download aborted",
    [UpdaterStatusUnpackCreateStagingDirectoryFailure] = "Failed to create staging directory",
    [UpdaterStatusUnpackArchiveOpenFailure] = "Failed to open tar file",
    [UpdaterStatusUnpackArchiveUnpackFailure] = "Failed to unpack tar file",
    [UpdaterStatusPrepareInstallManifestNotFound] = "Manifest not found",
    [UpdaterStatusPrepareInstallManifestInvalid] = "Failed to validate manifest",
    [UpdaterStatusPrepareInstallSessionConfigSetupFailure] = "Failed to save session config",
    [UpdaterStatusPrepareInstallPointerSetupFailure] = "Failed to write pointer file",
    [UpdaterStatusUnknownFailure] = "Unknown error",
};

static_assert(COUNT_OF(status_strings) == UpdaterStatusesCount);

static const MessageHandler message_handlers[] = {
    [MessageTypeStartUpdate] =
        {
            .callback = do_start_update,
            .action = UpdaterUpdateActionNone,
        },
    [MessageTypeStopUpdate] =
        {
            .callback = do_stop_update,
            .action = UpdaterUpdateActionNone,
        },
    [MessageTypeDownload] =
        {
            .callback = do_download,
            .action = UpdaterUpdateActionDownload,
        },
    [MessageTypeUnpack] =
        {
            .callback = do_unpack,
            .action = UpdaterUpdateActionUnpack,
        },
    [MessageTypePrepareInstall] =
        {
            .callback = do_prepare_install,
            .action = UpdaterUpdateActionPrepareInstall,
        },
    [MessageTypeRebootInstall] =
        {
            .callback = do_reboot_install,
            .action = UpdaterUpdateActionRebootInstall,
        },
};

static_assert(COUNT_OF(message_handlers) == MessageTypesCount);
