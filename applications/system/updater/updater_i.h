#pragma once

#include "updater.h"
#include "update_checker/update_checker.h"

#include <storage/storage.h>
#include <power/power_service/power.h>

#include <toolbox/fetch/fetch_loader.h>
#include <toolbox/api_lock.h>

#define TAG "Updater"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MessageTypeSessionStart,
    MessageTypeSessionStop,
    MessageTypeDownload,
    MessageTypeVerifyBundleSha,
    MessageTypeUnpack,
    MessageTypeInstallationPrepare,
    MessageTypeInstallationApply,
    MessageTypeCheckForUpdate,
    MessageTypeGetSettings,
    MessageTypeSetSettings,

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

        struct {
            UpdaterSettings* get_settings;
        } as_get_settings;

        struct {
            const UpdaterSettings* set_settings;
        } as_set_settings;
    };

    FuriApiLock api_lock;
    UpdaterStatus* result_status;
    MessageType type;
} UpdaterMessage;

struct Updater {
    Storage* storage;
    Power* power;

    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    UpdaterSettings settings;

    FuriPubSub* pubsub;

    FuriSemaphore* update_lock;
    FuriState* update_state;

#ifndef FW_CFG_recovery
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

#ifdef SRV_TIME
    FuriEventLoopTimer* autoupdate_timer;
    FuriSemaphore* autoupdate_semaphore;
#endif /* SRV_TIME */
#endif /* FW_CFG_recovery */
};

UpdaterStatus updater_internal_invoke_async(Updater* instance, UpdaterMessage* message);
UpdaterStatus updater_internal_invoke_sync(Updater* instance, UpdaterMessage* message);

UpdaterStatus updater_internal_do_check_for_update(Updater* instance, UpdaterMessage* message);
UpdaterStatus updater_internal_do_download(Updater* instance, UpdaterMessage* message);
UpdaterStatus updater_internal_do_verify_bundle_sha(Updater* instance, UpdaterMessage* message);
UpdaterStatus updater_internal_do_unpack(Updater* instance, UpdaterMessage* message);

void updater_internal_settings_change_build_specific(Updater* instance);

void updater_internal_setup_build_specific(Updater* instance);

#ifdef __cplusplus
}
#endif
