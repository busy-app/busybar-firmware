/**
 * @file js_app_installer.h
 * @brief Installation of packaged JavaScript applications.
 */
#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    JsAppInstallResultOk,
    JsAppInstallResultInvalidArchive,
    JsAppInstallResultInvalidApplication,
    JsAppInstallResultVersionConflict,
    JsAppInstallResultStorageError,
    JsAppInstallResultTooLarge,
    JsAppInstallResultMax,
} JsAppInstallResult;

/**
 * @brief Install a JavaScript application from a TAR or TGZ archive.
 *
 * The archive must contain the application files directly at its root.
 * Existing applications are replaced only by a newer semantic version.
 *
 * @param[in] archive_path path to the uploaded archive
 * @param[out] app_id receives the installed application identifier on success
 * @returns installation result
 */
JsAppInstallResult js_app_installer_install(const char* archive_path, FuriString* app_id);

/**
 * @brief Clean up after an install that was cut short.
 *
 * Restores an application whose directory was already moved aside when power
 * was lost mid-update, and discards leftover scratch directories. Intended to
 * run once at startup, before anything lists installed applications.
 */
void js_app_installer_recover(void);

#ifdef __cplusplus
}
#endif
