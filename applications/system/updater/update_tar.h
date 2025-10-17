#pragma once

#include <stdbool.h>

typedef enum {
    UpdaterTarStatusSuccess = 0,
    UpdaterTarStatusErrorCreateStagingDir,
    UpdaterTarStatusErrorOpenTar,
    UpdaterTarStatusErrorUnpackTar,
    UpdaterTarStatusErrorManifestNotFound,
    UpdaterTarStatusErrorValidateManifest,
    UpdaterTarStatusErrorWritePointerFile,
    UpdaterTarStatusErrorUnknown,
} UpdaterTarStatus;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Gets a string description for an UpdaterTarStatus error code.
 * @param error_code The UpdaterTarStatus error code.
 * @return A constant string describing the error.
 */
const char* updater_tar_install_get_error_str(UpdaterTarStatus error_code);

/**
 * @brief Execute the installation of a firmware update from a TAR file.
 *
 * @param path The path to the TAR file.
 * @param auto_reboot If true, automatically reboot after successful installation.
 * @return The status of the update installation.
 */
UpdaterTarStatus updater_tar_install(const char* path, bool auto_reboot);

#ifdef __cplusplus
}
#endif
