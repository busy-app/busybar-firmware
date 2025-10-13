#pragma once

#include <stdbool.h>

typedef enum {
    UpdateFwTarStatusSuccess = 0,
    UpdateFwTarStatusErrorCreateStagingDir,
    UpdateFwTarStatusErrorOpenTar,
    UpdateFwTarStatusErrorUnpackTar,
    UpdateFwTarStatusErrorManifestNotFound,
    UpdateFwTarStatusErrorValidateManifest,
    UpdateFwTarStatusErrorWritePointerFile,
    UpdateFwTarStatusErrorUnknown,
} UpdateFwTarStatus;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Gets a string description for an UpdateFwTarStatus error code.
 * @param error_code The UpdateFwTarStatus error code.
 * @return A constant string describing the error.
 */
const char* update_fw_tar_install_get_error_str(UpdateFwTarStatus error_code);

/**
 * @brief Execute the installation of a firmware update from a TAR file.
 *
 * @param path The path to the TAR file.
 * @return The status of the update installation.
 */
UpdateFwTarStatus update_fw_tar_install(const char* path);

#ifdef __cplusplus
}
#endif
