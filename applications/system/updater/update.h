/**
 * @file update.h
 * @brief Firmware update service API.
 */
#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Updater operation status codes.
 */
typedef enum {
    UpdaterStatusSuccess, /**< Operation completed successfully. */
    UpdaterStatusErrorCreateStagingDir, /**< Failed to create staging directory. */
    UpdaterStatusErrorOpenTar, /**< Failed to open archive file. */
    UpdaterStatusErrorUnpackTar, /**< Failed to extract archive contents. */
    UpdaterStatusErrorLowBatteryLevel, /**< Too low battery level for operation */
    UpdaterStatusErrorManifestNotFound, /**< Manifest file not found. */
    UpdaterStatusErrorValidateManifest, /**< Manifest validation failed. */
    UpdaterStatusErrorSaveSessionConfig, /**< Failed to save session config. */
    UpdaterStatusErrorWritePointerFile, /**< Failed to write pointer file. */
    UpdaterStatusErrorUnknown, /**< Unknown error occurred. */

    UpdaterStatusesCount,
} UpdaterStatus;

/**
 * @brief Get human-readable status description.
 *
 * @param[in] status the status code.
 * @return pointer to a statically allocated string.
 */
const char* updater_get_status_string(UpdaterStatus status);

/**
 * @brief Unpack update archive to staging directory.
 *
 * @param[in] tar_path path to the update archive.
 * @param[in] staging_path path where files should be extracted.
 * @param[out] manifest_path will be filled with path to the manifest file.
 * @return UpdaterStatusSuccess on success, error code otherwise.
 */
UpdaterStatus
    updater_unpack_tar(const char* tar_path, const char* staging_path, FuriString* manifest_path);

/**
 * @brief Check if update is ready to install on reboot.
 *
 * @return true if update is prepared, false otherwise.
 */
bool updater_is_install_prepared(void);

/**
 * @brief Cancel previously prepared update.
 */
void updater_cancel_prepared_install(void);

/**
 * @brief Prepare update for installation on next reboot.
 *
 * @param[in] manifest_path path to the update manifest file.
 * @return UpdaterStatusSuccess on success, error code otherwise.
 */
UpdaterStatus updater_prepare_install(const char* manifest_path);

/**
 * @brief Reboot device and apply prepared update.
 *
 * @return UpdaterStatusSuccess on success, error code otherwise.
 */
UpdaterStatus updater_reboot_install(void);

#ifdef __cplusplus
}
#endif
