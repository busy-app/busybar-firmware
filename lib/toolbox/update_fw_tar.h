#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Execute the installation of a firmware update from a TAR file.
 *
 * @param path The path to the TAR file.
 * @return true if the installation was successful, false otherwise.
 */
bool update_fw_tar(const char* path);

#ifdef __cplusplus
}
#endif
