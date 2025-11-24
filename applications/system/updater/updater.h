/**
 * @file updater.h
 * @brief Firmware update system interface
 */

#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_UPDATER "updater"

typedef struct Updater Updater;

typedef enum {
    UpdaterStatusOk, /**< Operation completed successfully */
    UpdaterStatusBatteryLow, /**< Battery level too low */
    UpdaterStatusBusy, /**< Update already in progress */

    UpdaterStatusDownloadFailure, /**< Failed to download update bundle from URL */
    UpdaterStatusDownloadAbort, /**< Download was aborted by user */

    UpdaterStatusUnpackCreateStagingDirectoryFailure, /**< Failed to create staging directory */
    UpdaterStatusUnpackArchiveOpenFailure, /**< Failed to open .tar archive file */
    UpdaterStatusUnpackArchiveUnpackFailure, /**< Failed to extract .tar archive contents */

    UpdaterStatusPrepareInstallManifestNotFound, /**< Update manifest file not found in staging directory */
    UpdaterStatusPrepareInstallManifestInvalid, /**< Manifest file validation failed */
    UpdaterStatusPrepareInstallSessionConfigSetupFailure, /**< Failed to save session configuration */
    UpdaterStatusPrepareInstallPointerSetupFailure, /**< Failed to write pointer file */

    UpdaterStatusUnknownFailure, /**< Unknown error occurred */

    UpdaterStatusesCount,
} UpdaterStatus;

typedef enum {
    UpdaterUpdateActionDownload, /**< Downloading update bundle from URL */
    UpdaterUpdateActionUnpack, /**< Unpacking .tar archive to staging directory */
    UpdaterUpdateActionPrepareInstall, /**< Preparing update for installation (manifest validation, session setup) */
    UpdaterUpdateActionRebootInstall, /**< Rebooting device to install prepared update */

    UpdaterUpdateActionNone, /**< No action (initial/idle state) */
} UpdaterUpdateAction;

typedef enum {
    UpdaterUpdateEventStart, /**< Update process started (lock acquired) */
    UpdaterUpdateEventStop, /**< Update process stopped (lock released) */
    UpdaterUpdateEventActionBegin, /**< Action started (download/unpack/prepare/reboot) */
    UpdaterUpdateEventActionDone, /**< Action completed with result status */
    UpdaterUpdateEventDetailChange, /**< Status detail string changed (e.g., download state message) */
    UpdaterUpdateEventActionProgress, /**< Progress update (e.g., download bytes received) */

    UpdaterUpdateEventNone, /**< No event (initial/idle state) */
} UpdaterUpdateEvent;

/** Update state information accessible via FuriState */
typedef struct {
    union {
        struct {
            size_t total_size; /**< Total download size in bytes */
            size_t received_size; /**< Bytes received so far */
            uint32_t speed_bytes_per_sec; /**< Current download speed in bytes per second */
        } as_download; /**< Download-specific progress information */
    };

    UpdaterUpdateEvent event; /**< Current event type */
    UpdaterUpdateAction action; /**< Current action being performed */
    UpdaterStatus status; /**< Current or last operation status */
    const FuriString* detail; /**< Optional detail string (e.g., download state message) */
} UpdaterUpdateState;

/** Get human-readable string for a status code
 *
 * @param[in]  status  The status code
 *
 * @return     Status string or "Unknown error code" for invalid status
 */
const char* updater_get_status_string(UpdaterStatus status);

/** Get update state object for monitoring progress
 *
 * @param[in]  instance  Updater instance
 *
 * @return     FuriState pointer (acquire/release to access UpdaterUpdateState)
 */
FuriState* updater_get_update_state(Updater* instance);

/** Check if update is allowed to start
 *
 * @param[in]  instance  Updater instance
 *
 * @return     UpdaterStatusOk if allowed, UpdaterStatusBatteryLow if battery too low
 */
UpdaterStatus updater_get_allowance_status(Updater* instance);

/** Start update process
 *
 * Acquires update lock and emits UpdaterUpdateEventStart event.
 * Must be called before any update operations.
 *
 * @param[in]  instance  Updater instance
 *
 * @return     UpdaterStatusOk on success,
 *             UpdaterStatusBatteryLow if battery too low,
 *             UpdaterStatusBusy if update already in progress
 */
UpdaterStatus updater_start_update(Updater* instance);

/** Stop update process
 *
 * Releases update lock and emits UpdaterUpdateEventStop event.
 * Must be called after all update operations complete.
 *
 * @param[in]  instance  Updater instance
 */
void updater_stop_update(Updater* instance);

/** Download update bundle from URL
 *
 * Requires update lock to be acquired.
 *
 * @param[in]  instance  Updater instance
 * @param[in]  url       URL to download from
 * @param[in]  path      Local file path to save to (NULL for default)
 * @param[in]  do_wait   true to block until complete, false for async operation
 *
 * @return     UpdaterStatusOk on success,
 *             UpdaterStatusDownloadFailure on network/file error,
 *             UpdaterStatusDownloadAbort if aborted
 */
UpdaterStatus updater_download(Updater* instance, const char* url, const char* path, bool do_wait);

/** Abort ongoing download operation
 *
 * Signals download to abort. Download will complete with UpdaterStatusDownloadAbort.
 * Requires update lock to be acquired.
 *
 * @param[in]  instance  Updater instance
 */
void updater_abort_download(Updater* instance);

/** Unpack update bundle .tar archive
 *
 * Requires update lock to be acquired.
 *
 * @param[in]   instance       Updater instance
 * @param[in]   tar_path       Path to .tar archive (NULL for default)
 * @param[in]   staging_path   Directory to extract to (NULL for default)
 * @param[out]  manifest_path  FuriString to receive manifest path (or NULL if not needed)
 * @param[in]   do_wait        true to block until complete, false for async operation
 *
 * @return     UpdaterStatusOk on success,
 *             UpdaterStatusUnpackCreateStagingDirectoryFailure if mkdir fails,
 *             UpdaterStatusUnpackArchiveOpenFailure if .tar open fails,
 *             UpdaterStatusUnpackArchiveUnpackFailure if extraction fails
 */
UpdaterStatus updater_unpack(
    Updater* instance,
    const char* tar_path,
    const char* staging_path,
    FuriString* manifest_path,
    bool do_wait);

/** Prepare update for installation
 *
 * Requires update lock to be acquired.
 *
 * @param[in]  instance       Updater instance
 * @param[in]  manifest_path  Path to update manifest file (NULL for default: /ext/update/update.fuf)
 * @param[in]  do_wait        true to block until complete, false for async operation
 *
 * @return     UpdaterStatusOk on success,
 *             UpdaterStatusPrepareInstallManifestNotFound if manifest missing,
 *             UpdaterStatusPrepareInstallManifestInvalid if validation fails,
 *             UpdaterStatusPrepareInstallSessionConfigSetupFailure if session config save fails,
 *             UpdaterStatusPrepareInstallPointerSetupFailure if pointer file write fails
 */
UpdaterStatus updater_prepare_install(Updater* instance, const char* manifest_path, bool do_wait);

/** Reboot device to install prepared update
 *
 * Requires update lock to be acquired.
 *
 * @param[in]  instance  Updater instance
 * @param[in]  do_wait   true to block and reboot, false for async reboot
 */
void updater_reboot_install(Updater* instance, bool do_wait);

#ifdef __cplusplus
}
#endif
