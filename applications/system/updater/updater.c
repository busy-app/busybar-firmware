#include "updater.h"
#include "updater_paths.h"
#include "updater_settings.h"
#include "update_checker/update_checker.h"
#include "session/session_config.h"

#include <storage/storage.h>
#include <power/power_service/power.h>

#include <furi_hal_nvm.h>
#include <furi_hal_power.h>
#include <furi_hal_version.h>
#include <toolbox/api_lock.h>
#include <toolbox/path.h>
#include <toolbox/tar/tar_archive.h>
#include <toolbox/fetch/fetch_loader.h>

#define TAG "Updater"

#define MESSAGE_QUEUE_ITEMS_COUNT 8

#define UPDATE_START_MIN_BATTERY_CHARGE        40
#define UPDATE_INSTALLATION_APPLY_REBOOT_DELAY 100

struct Updater {
    Storage* storage;
    Power* power;

    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    UpdaterSettings settings;

    FuriSemaphore* update_lock;
    FuriState* update_state;
    FuriString* update_detail;

    FetchLoader* download_loader;
    FuriMessageQueue* download_queue;

    UpdateChecker* update_checker;
    FuriState* check_state;
    FuriEventLoopTimer* check_timer;
    FuriString* check_url;
    FuriString* check_id;
    FuriString* check_version;
    FuriString* check_sha256;
    FuriString* check_changelog;
};

typedef struct {
    bool is_abort_request;
    UpdaterStatus status;
} DownloadQueueMessage;

typedef enum {
    MessageTypeSessionStart,
    MessageTypeSessionStop,
    MessageTypeDownload,
    MessageTypeUnpack,
    MessageTypeInstallationPrepare,
    MessageTypeInstallationApply,
    MessageTypeCheckForUpdate,

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
        } as_installation_prepare;
    };

    FuriApiLock api_lock;
    UpdaterStatus* result_status;
    MessageType type;
} UpdaterMessage;

typedef struct {
    UpdaterStatus (*callback)(Updater* instance, UpdaterMessage* message);
    UpdaterUpdateAction action;
} MessageHandler;

typedef enum {
    CustomEventUpdateCheckSuccess = 1 << 0,
    CustomEventUpdateCheckFailure = 1 << 1,
} CustomEvent;

static const char* const status_strings[];
static const MessageHandler message_handlers[];

static void custom_event_callback(uint32_t events, void* context) {
    Updater* instance = context;

    if(events & CustomEventUpdateCheckSuccess) {
        furi_event_loop_timer_start(instance->check_timer, instance->settings.check_interval);
    } else if(events & CustomEventUpdateCheckFailure) {
        furi_event_loop_timer_restart(instance->check_timer);
    }
}

static void check_done_callback(bool is_success, UpdaterCheckerInfo* update_info, void* context) {
    Updater* instance = context;

    furi_event_loop_set_custom_event(
        instance->event_loop,
        (is_success) ? CustomEventUpdateCheckSuccess : CustomEventUpdateCheckFailure);

    UpdaterCheckState* check_state = furi_state_acquire(instance->check_state);

    if(is_success) {
        if(furi_string_cmp_str(update_info->version, updater_get_active_version(instance))) {
            furi_string_set(instance->check_url, update_info->url);
            furi_string_set(instance->check_id, update_info->id);
            furi_string_set(instance->check_version, update_info->version);
            furi_string_set(instance->check_sha256, update_info->sha256);
            furi_string_set(instance->check_changelog, update_info->changelog);

            check_state->status = UpdaterCheckStatusAvailable;
        } else {
            check_state->status = UpdaterCheckStatusNotAvailable;
        }

    } else {
        check_state->status = UpdaterCheckStatusFailure;
    }

    check_state->event = UpdaterCheckEventStop;

    furi_state_release(instance->check_state);
}

static void check_timer_callback(void* context) {
    updater_check_for_update(context);
}

static UpdaterStatus do_check_for_update(Updater* instance, UpdaterMessage* message) {
    UNUSED(message);

    bool is_check_start_successful = update_checker_run(
        instance->update_checker,
        instance->settings.check_url,
        instance->settings.check_channel_id);

    if(is_check_start_successful) {
        UpdaterCheckState* check_state = furi_state_acquire(instance->check_state);
        check_state->event = UpdaterCheckEventStart;
        furi_state_release(instance->check_state);
    }

    return UpdaterStatusOk;
}

static UpdaterStatus do_session_start(Updater* instance, UpdaterMessage* message) {
    UNUSED(message);

    UpdaterUpdateState* update_state = furi_state_acquire(instance->update_state);
    update_state->event = UpdaterUpdateEventSessionStart;
    furi_state_release(instance->update_state);

    return UpdaterStatusOk;
}

static UpdaterStatus do_session_stop(Updater* instance, UpdaterMessage* message) {
    UNUSED(message);

    UpdaterUpdateState* update_state = furi_state_acquire(instance->update_state);
    update_state->event = UpdaterUpdateEventSessionStop;
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

static UpdaterStatus do_installation_prepare(Updater* instance, UpdaterMessage* message) {
    const char* manifest_path =
        furi_string_get_cstr(message->as_installation_prepare.manifest_path);

    FURI_LOG_D(TAG, "Preparing update bundle for installation using manifest %s", manifest_path);

    UpdateConfig* config = update_config_alloc();

    UpdaterStatus update_status;
    do {
        FURI_LOG_D(TAG, "Checking for manifest...");

        if(!storage_file_exists(instance->storage, manifest_path)) {
            FURI_LOG_E(TAG, "Manifest file not found: %s", manifest_path);
            update_status = UpdaterStatusInstallationPrepareManifestNotFound;
            break;
        }

        FURI_LOG_D(TAG, "Loading manifest configuration...");

        UpdateConfigValidation validation_status = update_config_load(config, manifest_path);
        if(validation_status != UpdateConfigValidationOK) {
            FURI_LOG_E(
                TAG,
                "Failed to load updater configuration: %s",
                update_config_validation_get_error_str(validation_status));

            update_status = UpdaterStatusInstallationPrepareManifestInvalid;
            break;
        }

        FURI_LOG_D(TAG, "Setting up session config...");

        UpdaterSessionConfig session_config;
        const UpdateManifest* manifest = update_config_get_manifest(config);
        updater_session_config_compose(manifest, &session_config);
        if(!updater_session_config_save(&session_config)) {
            FURI_LOG_E(TAG, "Failed to set up session config");
            update_status = UpdaterStatusInstallationPrepareSessionConfigSetupFailure;
            break;
        }

        FURI_LOG_D(TAG, "Setting up pointer file...");

        if(!update_config_write_pointer_file(instance->storage, manifest_path)) {
            FURI_LOG_E(TAG, "Failed to set up pointer file");
            update_status = UpdaterStatusInstallationPreparePointerSetupFailure;

            updater_session_config_delete();

            break;
        }

        FURI_LOG_D(TAG, "Update bundle prepared for installation successfully");

        update_status = UpdaterStatusOk;
    } while(false);

    update_config_free(config);
    furi_string_free(message->as_installation_prepare.manifest_path);

    return update_status;
}

static UpdaterStatus do_installation_apply(Updater* instance, UpdaterMessage* message) {
    UNUSED(instance);
    UNUSED(message);

    furi_hal_nvm_set_boot_mode(FuriHalNvmBootModeUpdate);

    FURI_LOG_D(TAG, "Boot mode set to \"update\", device will reboot...");

    furi_delay_ms(UPDATE_INSTALLATION_APPLY_REBOOT_DELAY);
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

    update_state = furi_state_acquire(instance->update_state);

    if(handler->action != UpdaterUpdateActionNone) {
        update_state->event = UpdaterUpdateEventActionBegin;
        update_state->status = UpdaterStatusBusy;
        update_state->action = handler->action;
    }

    furi_string_reset(instance->update_detail);
    furi_state_release(instance->update_state);

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

FuriState* updater_get_check_state(Updater* instance) {
    furi_check(instance);

    return instance->check_state;
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

UpdaterStatus updater_session_start(Updater* instance) {
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

        invoke_async(instance, &(UpdaterMessage){.type = MessageTypeSessionStart});

        result_status = UpdaterStatusOk;
    } while(false);

    return result_status;
}

void updater_session_stop(Updater* instance) {
    furi_check(instance);

    invoke_async(instance, &(UpdaterMessage){.type = MessageTypeSessionStop});

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
                .path = furi_string_alloc_set_str((path) ?: UPDATER_DEFAULT_DOWNLOAD_PATH),
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
                .tar_path = furi_string_alloc_set_str((tar_path) ?: UPDATER_DEFAULT_DOWNLOAD_PATH),
                .staging_path =
                    furi_string_alloc_set_str((staging_path) ?: UPDATER_DEFAULT_STAGING_PATH),
                .manifest_path = manifest_path,
            },
        .type = MessageTypeUnpack,
    };

    return (do_wait) ? invoke_sync(instance, &message) : invoke_async(instance, &message);
}

UpdaterStatus
    updater_installation_prepare(Updater* instance, const char* manifest_path, bool do_wait) {
    furi_check(instance);
    furi_check(furi_semaphore_get_space(instance->update_lock) > 0);

    UpdaterMessage message = {
        .as_installation_prepare =
            {
                .manifest_path =
                    furi_string_alloc_set_str((manifest_path) ?: UPDATER_DEFAULT_MANIFEST_PATH),
            },
        .type = MessageTypeInstallationPrepare,
    };

    return (do_wait) ? invoke_sync(instance, &message) : invoke_async(instance, &message);
}

void updater_installation_apply(Updater* instance, bool do_wait) {
    furi_check(instance);
    furi_check(furi_semaphore_get_space(instance->update_lock) > 0);

    UpdaterMessage message = {
        .type = MessageTypeInstallationApply,
    };

    if(do_wait) {
        invoke_sync(instance, &message);
    } else {
        invoke_async(instance, &message);
    }
}

void updater_check_for_update(Updater* instance) {
    furi_check(instance);

    invoke_sync(instance, &(UpdaterMessage){.type = MessageTypeCheckForUpdate});
}

const char* updater_get_active_version(Updater* instance) {
    furi_check(instance);

    const Version* version = furi_hal_version_get_firmware_version();
    return (strcmp(version_get_version(version), "unknown") == 0) ? version_get_githash(version) :
                                                                    version_get_version(version);
}

static Updater* updater_alloc(void) {
    Updater* instance = malloc(sizeof(*instance));

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->power = furi_record_open(RECORD_POWER);

    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue =
        furi_message_queue_alloc(MESSAGE_QUEUE_ITEMS_COUNT, sizeof(UpdaterMessage));
    updater_settings_load(&instance->settings);

    instance->update_lock = furi_semaphore_alloc(1, 1);
    instance->update_state = furi_state_alloc(sizeof(UpdaterUpdateState));
    instance->update_detail = furi_string_alloc();

    instance->download_loader = NULL;
    instance->download_queue = furi_message_queue_alloc(1, sizeof(DownloadQueueMessage));

    instance->update_checker = update_checker_alloc();
    instance->check_state = furi_state_alloc(sizeof(UpdaterCheckState));
    instance->check_timer = furi_event_loop_timer_alloc(
        instance->event_loop, check_timer_callback, FuriEventLoopTimerTypeOnce, instance);
    instance->check_url = furi_string_alloc();
    instance->check_id = furi_string_alloc();
    instance->check_version = furi_string_alloc();
    instance->check_sha256 = furi_string_alloc();
    instance->check_changelog = furi_string_alloc();

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        message_queue_callback,
        instance);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, custom_event_callback, instance);

    furi_state_set(
        instance->update_state,
        &(const UpdaterUpdateState){
            .event = UpdaterUpdateEventNone,
            .action = UpdaterUpdateActionNone,
            .detail = instance->update_detail,
        });

    furi_state_set(
        instance->check_state,
        &(const UpdaterCheckState){
            .url = instance->check_url,
            .id = instance->check_id,
            .version = instance->check_version,
            .sha256 = instance->check_sha256,
            .changelog = instance->check_changelog,
            .status = UpdaterCheckStatusNone,
            .event = UpdaterCheckEventNone,
        });

    update_checker_set_done_callback(instance->update_checker, check_done_callback, instance);
    furi_event_loop_timer_start(instance->check_timer, instance->settings.check_startup_interval);

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
    [UpdaterStatusInstallationPrepareManifestNotFound] = "Manifest not found",
    [UpdaterStatusInstallationPrepareManifestInvalid] = "Failed to validate manifest",
    [UpdaterStatusInstallationPrepareSessionConfigSetupFailure] = "Failed to save session config",
    [UpdaterStatusInstallationPreparePointerSetupFailure] = "Failed to write pointer file",
    [UpdaterStatusUnknownFailure] = "Unknown error",
};

static_assert(COUNT_OF(status_strings) == UpdaterStatusesCount);

static const MessageHandler message_handlers[] = {
    [MessageTypeSessionStart] =
        {
            .callback = do_session_start,
            .action = UpdaterUpdateActionNone,
        },
    [MessageTypeSessionStop] =
        {
            .callback = do_session_stop,
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
    [MessageTypeInstallationPrepare] =
        {
            .callback = do_installation_prepare,
            .action = UpdaterUpdateActionInstallationPrepare,
        },
    [MessageTypeInstallationApply] =
        {
            .callback = do_installation_apply,
            .action = UpdaterUpdateActionInstallationApply,
        },
    [MessageTypeCheckForUpdate] =
        {
            .callback = do_check_for_update,
            .action = UpdaterUpdateActionNone,
        },
};

static_assert(COUNT_OF(message_handlers) == MessageTypesCount);
