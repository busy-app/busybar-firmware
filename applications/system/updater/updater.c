#include "updater_i.h"

#include "updater_paths.h"
#include "settings/settings_i.h"
#include "session/session_config.h"

#include <furi_hal_nvm.h>
#include <furi_hal_power.h>
#include <version.h>

#if defined(SRV_SL_INFO)
#include <sl_info/sl_info.h>
#endif // SRV_SL_INFO

#define TAG "Updater"

#if defined(SRV_SL_INFO)
typedef struct {
    const char* key;
    const char* enabled_value; // value that means "flag is active"
    uint32_t flag;
} SlInfoSecurityMapping;

static const SlInfoSecurityMapping sl_security_mappings[] = {
    {"sl_nwp_signature", "true", UpdateManifestSecurityFlagNwpSigned},
    {"sl_m4_signature", "true", UpdateManifestSecurityFlagM4Signed},
    {"sl_nwp_encrypt", NULL, UpdateManifestSecurityFlagNwpEncrypted},
    {"sl_m4_encrypt", "true", UpdateManifestSecurityFlagM4Encrypted},
};

static bool updater_get_device_security_flags(uint32_t* flags_out) {
    uint32_t device_flags = 0;
    bool is_ready = true;
    const SlInfo* sl_info = furi_record_open(RECORD_SL_INFO);

    for(size_t i = 0; i < COUNT_OF(sl_security_mappings); i++) {
        const char* value = NULL;
        SlInfoStatus status = sl_info_get_value(sl_info, sl_security_mappings[i].key, &value);
        if(status == SlInfoStatusNotReady) {
            is_ready = false;
            break;
        }
        if(status == SlInfoStatusOk) {
            const char* expected = sl_security_mappings[i].enabled_value;
            if(expected) {
                if(strcmp(value, expected) == 0) {
                    device_flags |= sl_security_mappings[i].flag;
                }
            } else {
                // For mode-style values (e.g. "none"/"ctr"/"xts"), any value other than "none" means enabled
                if(strcmp(value, "none") != 0) {
                    device_flags |= sl_security_mappings[i].flag;
                }
            }
        }
    }

    furi_record_close(RECORD_SL_INFO);
    *flags_out = device_flags;
    return is_ready;
}

static UpdaterStatus updater_verify_security_flags(const UpdateManifest* manifest) {
    const uint32_t manifest_flags = updater_manifest_get_security_flags(manifest);

    uint32_t device_flags = 0;
    if(!updater_get_device_security_flags(&device_flags)) {
        FURI_LOG_W(TAG, "SL info not ready, skipping security check");
        return UpdaterStatusOk;
    }

    const uint32_t check_mask =
        UpdateManifestSecurityFlagNwpSigned | UpdateManifestSecurityFlagM4Signed |
        UpdateManifestSecurityFlagNwpEncrypted | UpdateManifestSecurityFlagM4Encrypted;

    if((manifest_flags & check_mask) != (device_flags & check_mask)) {
        FURI_LOG_E(
            TAG, "Security mismatch: manifest=0x%lx, device=0x%lx", manifest_flags, device_flags);
        return UpdaterStatusInstallationPrepareSecurityMismatch;
    }

    FURI_LOG_I(TAG, "Security flags OK (0x%lx)", manifest_flags);
    return UpdaterStatusOk;
}
#endif // SRV_SL_INFO

#define MESSAGE_QUEUE_ITEMS_COUNT 8

#define UPDATE_START_MIN_BATTERY_CHARGE        40
#define UPDATE_INSTALLATION_APPLY_REBOOT_DELAY 100

typedef struct {
    UpdaterStatus (*callback)(Updater* instance, UpdaterMessage* message);
    UpdaterUpdateAction action;
} MessageHandler;

static const char* const status_strings[];
static const MessageHandler message_handlers[];

UpdaterStatus updater_internal_invoke_async(Updater* instance, UpdaterMessage* message) {
    message->result_status = NULL;
    message->api_lock = NULL;

    furi_check(
        furi_message_queue_put(instance->message_queue, message, FuriWaitForever) == FuriStatusOk);

    return UpdaterStatusOk;
}

UpdaterStatus updater_internal_invoke_sync(Updater* instance, UpdaterMessage* message) {
    UpdaterStatus update_status;

    message->result_status = &update_status;
    message->api_lock = api_lock_alloc_locked();

    furi_check(
        furi_message_queue_put(instance->message_queue, message, FuriWaitForever) == FuriStatusOk);

    api_lock_wait_unlock_and_free(message->api_lock);

    return update_status;
}

static UpdaterStatus updater_do_session_start(Updater* instance, UpdaterMessage* message) {
    UNUSED(message);

    UpdaterUpdateState* update_state = furi_state_acquire(instance->update_state);
    update_state->event = UpdaterUpdateEventSessionStart;
    furi_state_release(instance->update_state);

    return UpdaterStatusOk;
}

static UpdaterStatus updater_do_session_stop(Updater* instance, UpdaterMessage* message) {
    UNUSED(message);

    UpdaterUpdateState* update_state = furi_state_acquire(instance->update_state);
    update_state->event = UpdaterUpdateEventSessionStop;
    furi_state_release(instance->update_state);

    return UpdaterStatusOk;
}

static UpdaterStatus updater_do_get_settings(Updater* instance, UpdaterMessage* message) {
    *message->as_get_settings.get_settings = instance->settings;
    return UpdaterStatusOk;
}

static UpdaterStatus updater_do_set_settings(Updater* instance, UpdaterMessage* message) {
    if(!updater_settings_save(message->as_set_settings.set_settings))
        return UpdaterStatusUnknownFailure;

    instance->settings = *message->as_set_settings.set_settings;

    updater_internal_settings_change_build_specific(instance);

    furi_pubsub_publish(
        instance->pubsub, &(UpdaterEvent){.type = UpdaterEventTypeSettingsChanged});

    return UpdaterStatusOk;
}

static UpdaterStatus updater_do_installation_prepare(Updater* instance, UpdaterMessage* message) {
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

        const UpdateManifest* manifest = update_config_get_manifest(config);

#if defined(SRV_SL_INFO)
        FURI_LOG_D(TAG, "Checking security flags...");
        update_status = updater_verify_security_flags(manifest);
        if(update_status != UpdaterStatusOk) {
            break;
        }
#endif // SRV_SL_INFO

        FURI_LOG_D(TAG, "Setting up session config...");

        UpdaterSessionConfig session_config;
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

static UpdaterStatus updater_do_installation_apply(Updater* instance, UpdaterMessage* message) {
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

        updater_internal_invoke_async(
            instance, &(UpdaterMessage){.type = MessageTypeSessionStart});

        result_status = UpdaterStatusOk;
    } while(false);

    return result_status;
}

void updater_session_stop(Updater* instance) {
    furi_check(instance);

    updater_internal_invoke_async(instance, &(UpdaterMessage){.type = MessageTypeSessionStop});

    furi_semaphore_release(instance->update_lock);
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

    return (do_wait) ? updater_internal_invoke_sync(instance, &message) :
                       updater_internal_invoke_async(instance, &message);
}

void updater_installation_apply(Updater* instance, bool do_wait) {
    furi_check(instance);
    furi_check(furi_semaphore_get_space(instance->update_lock) > 0);

    UpdaterMessage message = {
        .type = MessageTypeInstallationApply,
    };

    if(do_wait) {
        updater_internal_invoke_sync(instance, &message);
    } else {
        updater_internal_invoke_async(instance, &message);
    }
}

const char* updater_get_active_version(void) {
    const Version* version = version_get();
    const char* version_str = version_get_version(version);
    if((strlen(version_str) > 0) && (version_str[0] != 'r')) {
        return version_str;
    }

    return version_get_githash(version);
}

void updater_get_settings(const Updater* instance, UpdaterSettings* settings) {
    furi_check(instance);
    furi_check(settings);

    UpdaterMessage message = {
        .as_get_settings.get_settings = settings,
        .type = MessageTypeGetSettings,
    };

    updater_internal_invoke_sync((Updater*)instance, &message);
}

bool updater_set_settings(Updater* instance, const UpdaterSettings* settings) {
    furi_check(instance);
    furi_check(settings);

    UpdaterMessage message = {
        .as_set_settings.set_settings = settings,
        .type = MessageTypeSetSettings,
    };

    return updater_internal_invoke_sync(instance, &message) == UpdaterStatusOk;
}

FuriPubSub* updater_get_pubsub(Updater* instance) {
    furi_check(instance);

    return instance->pubsub;
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
    instance->pubsub = furi_pubsub_alloc();

    updater_internal_setup_build_specific(instance);

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
            .detail = "",
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
    [UpdaterStatusBusy] = "Operation already in progress",

    [UpdaterStatusDownloadFailure] = "Failed to download update bundle",
    [UpdaterStatusDownloadAbort] = "Download aborted",

    [UpdaterStatusShaMismatch] = "SHA256 checksum verification failed",

    [UpdaterStatusUnpackCreateStagingDirectoryFailure] = "Failed to create staging directory",
    [UpdaterStatusUnpackArchiveOpenFailure] = "Failed to open tar file",
    [UpdaterStatusUnpackArchiveUnpackFailure] = "Failed to unpack tar file",

    [UpdaterStatusInstallationPrepareManifestNotFound] = "Manifest not found",
    [UpdaterStatusInstallationPrepareManifestInvalid] = "Failed to validate manifest",
    [UpdaterStatusInstallationPrepareSecurityMismatch] = "Bundle security mismatch",
    [UpdaterStatusInstallationPrepareSessionConfigSetupFailure] = "Failed to save session config",
    [UpdaterStatusInstallationPreparePointerSetupFailure] = "Failed to write pointer file",

    [UpdaterStatusUnknownFailure] = "Unknown error",
};

static_assert(COUNT_OF(status_strings) == UpdaterStatusesCount);

static const MessageHandler message_handlers[] = {
    [MessageTypeSessionStart] =
        {
            .callback = updater_do_session_start,
            .action = UpdaterUpdateActionNone,
        },
    [MessageTypeSessionStop] =
        {
            .callback = updater_do_session_stop,
            .action = UpdaterUpdateActionNone,
        },
    [MessageTypeDownload] =
        {
            .callback = updater_internal_do_download,
            .action = UpdaterUpdateActionDownload,
        },
    [MessageTypeVerifyBundleSha] =
        {
            .callback = updater_internal_do_verify_bundle_sha,
            .action = UpdaterUpdateActionShaVerification,
        },
    [MessageTypeUnpack] =
        {
            .callback = updater_internal_do_unpack,
            .action = UpdaterUpdateActionUnpack,
        },
    [MessageTypeInstallationPrepare] =
        {
            .callback = updater_do_installation_prepare,
            .action = UpdaterUpdateActionInstallationPrepare,
        },
    [MessageTypeInstallationApply] =
        {
            .callback = updater_do_installation_apply,
            .action = UpdaterUpdateActionInstallationApply,
        },
    [MessageTypeCheckForUpdate] =
        {
            .callback = updater_internal_do_check_for_update,
            .action = UpdaterUpdateActionNone,
        },
    [MessageTypeGetSettings] =
        {
            .callback = updater_do_get_settings,
            .action = UpdaterUpdateActionNone,
        },
    [MessageTypeSetSettings] =
        {
            .callback = updater_do_set_settings,
            .action = UpdaterUpdateActionNone,
        },
};

static_assert(COUNT_OF(message_handlers) == MessageTypesCount);
