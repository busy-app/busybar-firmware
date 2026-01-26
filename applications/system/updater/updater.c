#include "updater.h"
#include "updater_paths.h"
#include "settings/settings.h"
#include "update_checker/update_checker.h"
#include "session/session_config.h"

#include <storage/storage.h>
#include <power/power_service/power.h>
#include <sntp/sntp.h>

#include <furi_hal_nvm.h>
#include <furi_hal_power.h>
#include <version.h>
#include <datetime.h>
#include <toolbox/api_lock.h>
#include <toolbox/path.h>
#include <toolbox/tar/tar_archive.h>
#include <toolbox/fetch/fetch_loader.h>
#include <toolbox/sha256_calc.h>

#define TAG "Updater"

#define MESSAGE_QUEUE_ITEMS_COUNT 8

#define UPDATE_START_MIN_BATTERY_CHARGE        40
#define UPDATE_INSTALLATION_APPLY_REBOOT_DELAY 100

#define INSTALL_FROM_URL_THREAD_NAME       "UpdateInstall"
#define INSTALL_FROM_URL_THREAD_STACK_SIZE (2 * 1024)

#define AUTOUPDATE_TIMER_INTERVAL (5 * 60 * 1000)

struct Updater {
    Storage* storage;
    Power* power;

    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    UpdaterSettings settings;

    FuriSemaphore* update_lock;
    FuriState* update_state;

    FetchLoader* download_loader;
    FuriMessageQueue* download_queue;

    UpdateChecker* update_checker;
    FuriState* check_state;
    FuriEventLoopTimer* check_timer;
    FuriMutex* check_info_mutex;
    FuriString* check_version;
    FuriString* check_url;
    FuriString* check_id;
    FuriString* check_sha256;
    FuriString* check_changelog;

    FuriString* install_url;
    FuriString* install_sha256;
    bool install_is_autoupdate;

    FuriEventLoopTimer* autoupdate_timer;
    FuriSemaphore* autoupdate_semaphore;
};

typedef struct {
    bool is_abort_request;
    UpdaterStatus status;
} DownloadQueueMessage;

typedef enum {
    MessageTypeSessionStart,
    MessageTypeSessionStop,
    MessageTypeDownload,
    MessageTypeVerifyBundleSha,
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
            FuriString* sha;
        } as_verify_bundle_sha;

        struct {
            FuriString* tar_path;
            FuriString* staging_path;
            FuriString* manifest_path;
        } as_unpack;

        struct {
            FuriString* manifest_path;
        } as_installation_prepare;

        struct {
            UpdateCheckInfo* info;
        } as_get_check_info;
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

static UpdaterStatus install_from_url_internal(
    Updater* instance,
    const char* url,
    const char* sha256,
    bool is_autoupdate);

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

static void custom_event_callback(uint32_t events, void* context) {
    Updater* instance = context;

    if(events & CustomEventUpdateCheckSuccess) {
        furi_event_loop_timer_start(
            instance->check_timer, furi_ms_to_ticks(instance->settings.check_interval));
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
        if(furi_string_cmp_str(update_info->version, updater_get_active_version())) {
            furi_mutex_acquire(instance->check_info_mutex, FuriWaitForever);
            furi_string_set(instance->check_version, update_info->version);
            furi_string_set(instance->check_url, update_info->url);
            furi_string_set(instance->check_id, update_info->id);
            furi_string_set(instance->check_sha256, update_info->sha256);
            furi_string_set(instance->check_changelog, update_info->changelog);
            furi_mutex_release(instance->check_info_mutex);

            check_state->result = UpdaterCheckResultAvailable;
        } else {
            check_state->result = UpdaterCheckResultNotAvailable;
        }
    } else {
        check_state->result = UpdaterCheckResultFailure;
    }

    check_state->event = UpdaterCheckEventStop;

    furi_state_release(instance->check_state);
}

static void check_timer_callback(void* context) {
    invoke_async(context, &(UpdaterMessage){.type = MessageTypeCheckForUpdate});
}

static void autoupdate_timer_callback(void* context) {
    furi_assert(context);

    Updater* instance = context;

    FURI_LOG_D(TAG, "Autoupdate: starting check...");

    if(furi_semaphore_get_space(instance->autoupdate_semaphore) > 0) {
        FURI_LOG_D(TAG, "Autoupdate: skipped, on pause");
        return;
    }

    Sntp* sntp = furi_record_open(RECORD_SNTP);
    DateTime datetime;
    sntp_get_local_datetime(sntp, &datetime);
    furi_record_close(RECORD_SNTP);

    int time_minutes = datetime.hour * 60 + datetime.minute;
    int interval_start = instance->settings.autoupdate_interval_start;
    int interval_end = instance->settings.autoupdate_interval_end;
    bool is_time_in_interval =
        (interval_start <= interval_end) ?
            (time_minutes >= interval_start) && (time_minutes < interval_end) :
            (time_minutes >= interval_start) || (time_minutes < interval_end);

    if(!is_time_in_interval) {
        FURI_LOG_D(
            TAG,
            "Autoupdate: skipped, outside time window (%02d:%02d)",
            datetime.hour,
            datetime.minute);
        return;
    }

    UpdaterCheckState check_state;
    furi_state_get(instance->check_state, &check_state);

    if(check_state.result != UpdaterCheckResultAvailable) {
        FURI_LOG_D(TAG, "Autoupdate: skipped, no update available");
        return;
    }

    if(check_state.event != UpdaterCheckEventStop) {
        FURI_LOG_D(TAG, "Autoupdate: skipped, check for update is running");
        return;
    }

    UpdaterStatus install_status = install_from_url_internal(
        instance,
        furi_string_get_cstr(instance->check_url),
        furi_string_get_cstr(instance->check_sha256),
        true);

    if(install_status == UpdaterStatusOk) {
        FURI_LOG_I(TAG, "Autoupdate: installation started");
    } else {
        FURI_LOG_W(
            TAG, "Autoupdate: failed to start (%s)", updater_get_status_string(install_status));
    }
}

static UpdaterStatus do_check_for_update(Updater* instance, UpdaterMessage* message) {
    UNUSED(message);

    bool is_check_start_successful = update_checker_run(
        instance->update_checker,
        furi_string_get_cstr(instance->settings.check_url),
        furi_string_get_cstr(instance->settings.check_channel_id));

    if(is_check_start_successful) {
        UpdaterCheckState* check_state = furi_state_acquire(instance->check_state);
        check_state->event = UpdaterCheckEventStart;
        furi_state_release(instance->check_state);
    }

    return (is_check_start_successful) ? UpdaterStatusOk : UpdaterStatusBusy;
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
    strncpy(update_state->detail, furi_string_get_cstr(state), sizeof(update_state->detail));
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

static UpdaterStatus do_verify_bundle_sha(Updater* instance, UpdaterMessage* message) {
    const char* tar_path = furi_string_get_cstr(message->as_verify_bundle_sha.tar_path);
    const char* sha = furi_string_get_cstr(message->as_verify_bundle_sha.sha);

    FURI_LOG_D(TAG, "Verifying SHA256 checksum of %s", tar_path);

    FuriString* sha256_calc = furi_string_alloc();

    FS_Error file_status = FSE_OK;
    File* file = storage_file_alloc(instance->storage);

    sha256_string_calc_file(file, tar_path, sha256_calc, &file_status);

    storage_file_free(file);

    UpdaterStatus update_status =
        (file_status == FSE_OK && furi_string_cmp(sha256_calc, sha) == 0) ?
            UpdaterStatusOk :
            UpdaterStatusShaMismatch;

    furi_string_free(sha256_calc);
    furi_string_free(message->as_verify_bundle_sha.tar_path);
    furi_string_free(message->as_verify_bundle_sha.sha);

    if(update_status == UpdaterStatusOk) {
        FURI_LOG_D(TAG, "SHA256 checksum verified successfully");
    } else {
        FURI_LOG_E(TAG, "SHA256 checksum verification failed for %s", tar_path);
    }

    return update_status;
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

        if(!tar_archive_open(tar_archive, tar_path, TarOpenModeReadAuto)) {
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

    if(handler->action != UpdaterUpdateActionNone) {
        update_state = furi_state_acquire(instance->update_state);
        update_state->event = UpdaterUpdateEventActionBegin;
        update_state->status = UpdaterStatusBusy;
        update_state->action = handler->action;
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

static int32_t install_from_url_thread_callback(void* context) {
    Updater* instance = context;

    UpdaterStatus status;
    do {
        const char* url = furi_string_get_cstr(instance->install_url);
        status = updater_download(instance, url, NULL, true);
        if(status != UpdaterStatusOk) {
            break;
        }

        if(furi_string_size(instance->install_sha256) > 0) {
            const char* sha = furi_string_get_cstr(instance->install_sha256);
            status = updater_verify_bundle_sha(instance, NULL, sha, true);
            if(status != UpdaterStatusOk) {
                break;
            }
        }

        status = updater_unpack(instance, NULL, NULL, NULL, true);
        if(status != UpdaterStatusOk) {
            break;
        }

        status = updater_installation_prepare(instance, NULL, true);
        if(status != UpdaterStatusOk) {
            break;
        }

        if(instance->install_is_autoupdate) {
            if(furi_semaphore_get_space(instance->autoupdate_semaphore) > 0) {
                FURI_LOG_I(TAG, "Autoupdate: installation aborted, paused by user");
                break;
            }
        }

        updater_installation_apply(instance, true);
    } while(false);

    updater_session_stop(instance);

    return 0;
}

static void install_from_url_thread_state_callback(
    FuriThread* thread,
    FuriThreadState state,
    void* context) {
    UNUSED(context);

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
    }
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

void updater_get_check_info(Updater* instance, UpdateCheckInfo* info) {
    furi_check(instance);
    furi_check(info);

    furi_mutex_acquire(instance->check_info_mutex, FuriWaitForever);

    if(info->version) {
        furi_string_set(info->version, instance->check_version);
    }

    if(info->url) {
        furi_string_set(info->url, instance->check_url);
    }

    if(info->id) {
        furi_string_set(info->id, instance->check_id);
    }

    if(info->sha256) {
        furi_string_set(info->sha256, instance->check_sha256);
    }

    if(info->changelog) {
        furi_string_set(info->changelog, instance->check_changelog);
    }

    furi_mutex_release(instance->check_info_mutex);
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

    furi_message_queue_put(
        instance->download_queue,
        &(const DownloadQueueMessage){
            .is_abort_request = true,
        },
        0);
}

UpdaterStatus updater_verify_bundle_sha(
    Updater* instance,
    const char* tar_path,
    const char* sha,
    bool do_wait) {
    furi_check(instance);
    furi_check(sha);
    furi_check(furi_semaphore_get_space(instance->update_lock) > 0);

    UpdaterMessage message = {
        .as_verify_bundle_sha =
            {
                .tar_path = furi_string_alloc_set_str((tar_path) ?: UPDATER_DEFAULT_DOWNLOAD_PATH),
                .sha = furi_string_alloc_set_str(sha),
            },
        .type = MessageTypeVerifyBundleSha,
    };

    return (do_wait) ? invoke_sync(instance, &message) : invoke_async(instance, &message);
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

static UpdaterStatus install_from_url_internal(
    Updater* instance,
    const char* url,
    const char* sha256,
    bool is_autoupdate) {
    UpdaterStatus session_start_status = updater_session_start(instance);

    if(session_start_status == UpdaterStatusOk) {
        furi_string_set(instance->install_url, url);

        if(sha256) {
            furi_string_set(instance->install_sha256, sha256);
        } else {
            furi_string_reset(instance->install_sha256);
        }

        instance->install_is_autoupdate = is_autoupdate;

        FuriThread* thread = furi_thread_alloc_ex(
            INSTALL_FROM_URL_THREAD_NAME,
            INSTALL_FROM_URL_THREAD_STACK_SIZE,
            install_from_url_thread_callback,
            instance);

        furi_thread_set_state_context(thread, instance);
        furi_thread_set_state_callback(thread, install_from_url_thread_state_callback);
        furi_thread_start(thread);
    }

    return session_start_status;
}

UpdaterStatus updater_install_from_url(Updater* instance, const char* url, const char* sha256) {
    furi_check(instance);
    furi_check(url);

    return install_from_url_internal(instance, url, sha256, false);
}

UpdaterStatus updater_check_for_update(Updater* instance) {
    furi_check(instance);

    return invoke_sync(instance, &(UpdaterMessage){.type = MessageTypeCheckForUpdate});
}

void updater_pause_autoupdates(Updater* instance) {
    furi_check(instance);

    furi_check(furi_semaphore_acquire(instance->autoupdate_semaphore, 0) == FuriStatusOk);
}

void updater_resume_autoupdates(Updater* instance) {
    furi_check(instance);

    furi_semaphore_release(instance->autoupdate_semaphore);
}

const char* updater_get_active_version(void) {
    const Version* version = version_get();
    const char* version_str = version_get_version(version);
    if((strlen(version_str) > 0) && (version_str[0] != 'r')) {
        return version_str;
    }

    return version_get_githash(version);
}

static Updater* updater_alloc(void) {
    Updater* instance = malloc(sizeof(*instance));

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->power = furi_record_open(RECORD_POWER);

    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue =
        furi_message_queue_alloc(MESSAGE_QUEUE_ITEMS_COUNT, sizeof(UpdaterMessage));

    instance->settings.check_url = furi_string_alloc();
    instance->settings.check_channel_id = furi_string_alloc();
    updater_settings_load(&instance->settings);

    instance->update_lock = furi_semaphore_alloc(1, 1);
    instance->update_state = furi_state_alloc(sizeof(UpdaterUpdateState));

    instance->download_loader = NULL;
    instance->download_queue = furi_message_queue_alloc(1, sizeof(DownloadQueueMessage));

    instance->update_checker = update_checker_alloc();
    instance->check_state = furi_state_alloc(sizeof(UpdaterCheckState));
    instance->check_timer = furi_event_loop_timer_alloc(
        instance->event_loop, check_timer_callback, FuriEventLoopTimerTypeOnce, instance);
    instance->check_info_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->check_version = furi_string_alloc();
    instance->check_url = furi_string_alloc();
    instance->check_id = furi_string_alloc();
    instance->check_sha256 = furi_string_alloc();
    instance->check_changelog = furi_string_alloc();

    instance->install_url = furi_string_alloc();
    instance->install_sha256 = furi_string_alloc();

#ifdef SRV_SNTP
    instance->autoupdate_timer = furi_event_loop_timer_alloc(
        instance->event_loop, autoupdate_timer_callback, FuriEventLoopTimerTypePeriodic, instance);
#else
    UNUSED(autoupdate_timer_callback);
#endif // SRV_SNTP
    instance->autoupdate_semaphore = furi_semaphore_alloc(UINT32_MAX, UINT32_MAX);

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
            .detail = "",
        });

    furi_state_set(
        instance->check_state,
        &(const UpdaterCheckState){
            .result = UpdaterCheckResultNone,
            .event = UpdaterCheckEventNone,
        });

    update_checker_set_done_callback(instance->update_checker, check_done_callback, instance);
    furi_event_loop_timer_start(
        instance->check_timer, furi_ms_to_ticks(instance->settings.check_startup_interval));

    if(instance->settings.autoupdate_enabled) {
        furi_event_loop_timer_start(
            instance->autoupdate_timer, furi_ms_to_ticks(AUTOUPDATE_TIMER_INTERVAL));
    }

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
    [UpdaterStatusBusy] = "Operation already in progress",

    [UpdaterStatusDownloadFailure] = "Failed to download update bundle",
    [UpdaterStatusDownloadAbort] = "Download aborted",

    [UpdaterStatusShaMismatch] = "SHA256 checksum verification failed",

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
    [MessageTypeVerifyBundleSha] =
        {
            .callback = do_verify_bundle_sha,
            .action = UpdaterUpdateActionShaVerification,
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
