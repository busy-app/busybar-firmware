#include "js_app_installer.h"

#include "js_app.h"
#include "js_app_manifest.h"
#include "js_app_registry.h"

#include <storage/storage.h>
#include <storage_utils/dir_walk.h>
#include <toolbox/path.h>
#include <toolbox/tar/tar_archive.h>

#include <ctype.h>
#include <limits.h>

#define TAG "JsAppInstaller"

#define JS_APPS_PATH EXT_PATH("user_assets")

#define JS_APP_INSTALL_WORK_PATH     EXT_PATH("tmp/app_install")
#define JS_APP_INSTALL_INCOMING_PATH JS_APP_INSTALL_WORK_PATH "/incoming"

#define JS_APP_INSTALL_ARCHIVE_SIZE_MAX   (8 * 1024 * 1024)
#define JS_APP_INSTALL_EXTRACTED_SIZE_MAX (16 * 1024 * 1024)
#define JS_APP_INSTALL_ENTRY_COUNT_MAX    (256)
#define JS_APP_INSTALL_ENTRY_PATH_MAX     (255)

#define JS_APP_INSTALL_MANIFEST_PATH "appmeta/manifest.json"
#define JS_APP_INSTALL_ENTRY_PATH    "scripts/main.js"

typedef struct {
    bool is_valid;
} JsAppInstallArchiveContext;

typedef struct {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    const char* prerelease;
    size_t prerelease_length;
} JsAppInstallVersion;

static bool js_app_installer_parse_number(const char** cursor, const char* end, uint32_t* value) {
    const char* begin = *cursor;
    if((begin == end) || !isdigit((unsigned char)*begin)) return false;
    if((*begin == '0') && ((begin + 1 != end) && isdigit((unsigned char)begin[1]))) return false;

    uint32_t result = 0;
    while((*cursor != end) && isdigit((unsigned char)**cursor)) {
        const uint32_t digit = (uint32_t)(**cursor - '0');
        if(result > ((UINT32_MAX - digit) / 10)) return false;
        result = result * 10 + digit;
        ++*cursor;
    }

    *value = result;
    return true;
}

static bool js_app_installer_validate_identifiers(
    const char* begin,
    const char* end,
    bool reject_numeric_leading_zero) {
    if(begin == end) return false;

    const char* identifier = begin;
    bool numeric = true;
    for(const char* cursor = begin;; ++cursor) {
        if((cursor == end) || (*cursor == '.')) {
            if(cursor == identifier) return false;
            if(reject_numeric_leading_zero && numeric && (*identifier == '0') &&
               (cursor - identifier > 1)) {
                return false;
            }
            if(cursor == end) return true;
            identifier = cursor + 1;
            numeric = true;
            continue;
        }

        const unsigned char value = *cursor;
        if(!isalnum(value) && (value != '-')) return false;
        if(!isdigit(value)) numeric = false;
    }
}

static bool js_app_installer_parse_version(const char* value, JsAppInstallVersion* version) {
    if(!value || !*value) return false;

    const char* cursor = value;
    const char* end = value + strlen(value);
    if(!js_app_installer_parse_number(&cursor, end, &version->major) || (cursor == end) ||
       (*cursor++ != '.') || !js_app_installer_parse_number(&cursor, end, &version->minor) ||
       (cursor == end) || (*cursor++ != '.') ||
       !js_app_installer_parse_number(&cursor, end, &version->patch)) {
        return false;
    }

    version->prerelease = NULL;
    version->prerelease_length = 0;

    if((cursor != end) && (*cursor == '-')) {
        const char* prerelease = ++cursor;
        while((cursor != end) && (*cursor != '+'))
            ++cursor;
        if(!js_app_installer_validate_identifiers(prerelease, cursor, true)) return false;
        version->prerelease = prerelease;
        version->prerelease_length = cursor - prerelease;
    }

    if((cursor != end) && (*cursor == '+')) {
        const char* build = ++cursor;
        if(!js_app_installer_validate_identifiers(build, end, false)) return false;
        cursor = end;
    }

    return cursor == end;
}

static int32_t js_app_installer_compare_identifier(
    const char* left,
    size_t left_length,
    const char* right,
    size_t right_length) {
    bool left_numeric = true;
    bool right_numeric = true;
    for(size_t i = 0; i < left_length; ++i)
        left_numeric &= isdigit((unsigned char)left[i]);
    for(size_t i = 0; i < right_length; ++i)
        right_numeric &= isdigit((unsigned char)right[i]);

    if(left_numeric != right_numeric) return left_numeric ? -1 : 1;
    if(left_numeric && (left_length != right_length)) return left_length < right_length ? -1 : 1;

    const size_t common_length = MIN(left_length, right_length);
    const int32_t comparison = strncmp(left, right, common_length);
    if(comparison != 0) return comparison < 0 ? -1 : 1;
    if(left_length == right_length) return 0;
    return left_length < right_length ? -1 : 1;
}

static int32_t js_app_installer_compare_prerelease(
    const JsAppInstallVersion* left,
    const JsAppInstallVersion* right) {
    if(!left->prerelease && !right->prerelease) return 0;
    if(!left->prerelease) return 1;
    if(!right->prerelease) return -1;

    size_t left_offset = 0;
    size_t right_offset = 0;
    while((left_offset < left->prerelease_length) && (right_offset < right->prerelease_length)) {
        const char* left_value = left->prerelease + left_offset;
        const char* right_value = right->prerelease + right_offset;
        const char* left_dot = memchr(left_value, '.', left->prerelease_length - left_offset);
        const char* right_dot = memchr(right_value, '.', right->prerelease_length - right_offset);
        const size_t left_length = left_dot ? (size_t)(left_dot - left_value) :
                                              left->prerelease_length - left_offset;
        const size_t right_length = right_dot ? (size_t)(right_dot - right_value) :
                                                right->prerelease_length - right_offset;

        const int32_t comparison = js_app_installer_compare_identifier(
            left_value, left_length, right_value, right_length);
        if(comparison != 0) return comparison;

        left_offset += left_length + (left_dot ? 1 : 0);
        right_offset += right_length + (right_dot ? 1 : 0);
    }

    if(left_offset == left->prerelease_length && right_offset == right->prerelease_length) {
        return 0;
    }
    return left_offset == left->prerelease_length ? -1 : 1;
}

static bool js_app_installer_version_is_newer(const char* candidate, const char* installed) {
    JsAppInstallVersion candidate_version;
    JsAppInstallVersion installed_version;
    if(!js_app_installer_parse_version(candidate, &candidate_version) ||
       !js_app_installer_parse_version(installed, &installed_version)) {
        return false;
    }

    if(candidate_version.major != installed_version.major) {
        return candidate_version.major > installed_version.major;
    }
    if(candidate_version.minor != installed_version.minor) {
        return candidate_version.minor > installed_version.minor;
    }
    if(candidate_version.patch != installed_version.patch) {
        return candidate_version.patch > installed_version.patch;
    }
    return js_app_installer_compare_prerelease(&candidate_version, &installed_version) > 0;
}

static bool
    js_app_installer_archive_path_is_valid(const char* name, bool is_directory, void* context) {
    JsAppInstallArchiveContext* archive_context = context;
    const size_t length = name ? strlen(name) : 0;
    if((length == 0) || (length > JS_APP_INSTALL_ENTRY_PATH_MAX) || (name[0] == '/') ||
       strchr(name, '\\') || strchr(name, ':')) {
        archive_context->is_valid = false;
        return false;
    }

    const char* segment = name;
    for(size_t i = 0; i <= length; ++i) {
        const unsigned char value = name[i];
        if((value != 0) && (value < 0x20)) {
            archive_context->is_valid = false;
            return false;
        }

        if((value == '/') || (value == 0)) {
            const size_t segment_length = &name[i] - segment;
            const bool trailing_directory_slash = is_directory && (value == 0) &&
                                                  (segment_length == 0) && (i > 0) &&
                                                  (name[i - 1] == '/');
            if((segment_length == 0 && !trailing_directory_slash) ||
               ((segment_length == 1) && (segment[0] == '.')) ||
               ((segment_length == 2) && (segment[0] == '.') && (segment[1] == '.'))) {
                archive_context->is_valid = false;
                return false;
            }
            segment = &name[i + 1];
        }
    }

    return true;
}

static bool js_app_installer_extracted_size_is_valid(Storage* storage) {
    bool success = false;
    uint64_t total_size = 0;
    DirWalk* walk = dir_walk_alloc(storage);
    FuriString* path = furi_string_alloc();
    FileInfo file_info;

    if(dir_walk_open(walk, JS_APP_INSTALL_INCOMING_PATH)) {
        DirWalkResult result;
        while((result = dir_walk_read(walk, path, &file_info)) == DirWalkOK) {
            if(!file_info_is_dir(&file_info)) {
                total_size += file_info.size;
                if(total_size > JS_APP_INSTALL_EXTRACTED_SIZE_MAX) break;
            }
        }
        success = (result == DirWalkLast) && (total_size <= JS_APP_INSTALL_EXTRACTED_SIZE_MAX);
    }

    dir_walk_close(walk);
    dir_walk_free(walk);
    furi_string_free(path);
    return success;
}

static JsAppInstallResult js_app_installer_unpack(Storage* storage, const char* archive_path) {
    FileInfo archive_info;
    if((storage_common_stat(storage, archive_path, &archive_info) != FSE_OK) ||
       file_info_is_dir(&archive_info)) {
        return JsAppInstallResultInvalidArchive;
    }
    if((archive_info.size == 0) || (archive_info.size > JS_APP_INSTALL_ARCHIVE_SIZE_MAX)) {
        return JsAppInstallResultTooLarge;
    }

    if(!storage_simply_remove_recursive(storage, JS_APP_INSTALL_INCOMING_PATH) ||
       !storage_simply_mkpath(storage, JS_APP_INSTALL_INCOMING_PATH)) {
        return JsAppInstallResultStorageError;
    }

    JsAppInstallResult result = JsAppInstallResultInvalidArchive;
    TarArchive* archive = tar_archive_alloc(storage);
    if(tar_archive_open(archive, archive_path, TarOpenModeReadAuto)) {
        const int32_t entries = tar_archive_get_entries_count(archive, false);
        const int32_t normal_entries = tar_archive_get_entries_count(archive, true);
        if((entries > 0) && (entries <= JS_APP_INSTALL_ENTRY_COUNT_MAX) &&
           (entries == normal_entries)) {
            // Sum the headers before extracting. A well-compressed archive can
            // sit under the 8 MiB upload cap and still expand to gigabytes; by
            // the time a post-extraction check notices, the card is full and
            // every other subsystem's writes have been failing.
            if(!tar_archive_get_unpacked_size(archive, JS_APP_INSTALL_EXTRACTED_SIZE_MAX, NULL)) {
                result = JsAppInstallResultTooLarge;
            } else {
                JsAppInstallArchiveContext archive_context = {.is_valid = true};
                tar_archive_set_file_callback(
                    archive, js_app_installer_archive_path_is_valid, &archive_context);
                if(tar_archive_unpack_to(archive, JS_APP_INSTALL_INCOMING_PATH, NULL) &&
                   archive_context.is_valid && js_app_installer_extracted_size_is_valid(storage)) {
                    result = JsAppInstallResultOk;
                }
            }
        } else if(entries > JS_APP_INSTALL_ENTRY_COUNT_MAX) {
            result = JsAppInstallResultTooLarge;
        }
    }
    tar_archive_free(archive);

    if(result != JsAppInstallResultOk) {
        storage_simply_remove_recursive(storage, JS_APP_INSTALL_INCOMING_PATH);
    }
    return result;
}

#define JS_APP_INSTALL_BACKUP_SUFFIX ".backup"
#define JS_APP_INSTALL_STAGED_SUFFIX ".staged"
#define JS_APP_INSTALL_RECOVER_MAX   (16)

void js_app_installer_recover(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);

    FuriString* path = furi_string_alloc();
    FuriString* name = furi_string_alloc();
    FuriString* final_path = furi_string_alloc();
    FuriString* work_path = furi_string_alloc();

    // Collect first, act second: renaming and deleting entries underneath an
    // open DirWalk is not something it promises to survive.
    FuriString* pending[JS_APP_INSTALL_RECOVER_MAX];
    size_t pending_count = 0;

    DirWalk* walk = dir_walk_alloc(storage);
    dir_walk_set_recursive(walk, false);
    if(dir_walk_open(walk, JS_APP_INSTALL_WORK_PATH)) {
        while((pending_count < JS_APP_INSTALL_RECOVER_MAX) &&
              (dir_walk_read(walk, path, NULL) == DirWalkOK)) {
            path_extract_filename(path, name, false);
            if(furi_string_end_with_str(name, JS_APP_INSTALL_BACKUP_SUFFIX) ||
               furi_string_end_with_str(name, JS_APP_INSTALL_STAGED_SUFFIX)) {
                pending[pending_count++] = furi_string_alloc_set(name);
            }
        }
    }
    dir_walk_close(walk);
    dir_walk_free(walk);

    for(size_t i = 0; i < pending_count; ++i) {
        const bool is_backup = furi_string_end_with_str(pending[i], JS_APP_INSTALL_BACKUP_SUFFIX);
        const size_t suffix_length =
            strlen(is_backup ? JS_APP_INSTALL_BACKUP_SUFFIX : JS_APP_INSTALL_STAGED_SUFFIX);

        furi_string_printf(
            work_path, "%s/%s", JS_APP_INSTALL_WORK_PATH, furi_string_get_cstr(pending[i]));
        furi_string_left(pending[i], furi_string_size(pending[i]) - suffix_length);
        path_concat(JS_APPS_PATH, furi_string_get_cstr(pending[i]), final_path);

        // An update that lost power between the two renames left the app with
        // no live directory at all. Put the backup back; anything else here is
        // scratch that outlived its install.
        if(is_backup && !storage_common_exists(storage, furi_string_get_cstr(final_path))) {
            FURI_LOG_W(
                TAG, "Restoring interrupted update of %s", furi_string_get_cstr(pending[i]));
            if(storage_common_rename(
                   storage, furi_string_get_cstr(work_path), furi_string_get_cstr(final_path)) !=
               FSE_OK) {
                FURI_LOG_E(TAG, "Failed to restore %s", furi_string_get_cstr(pending[i]));
            }
        } else {
            storage_simply_remove_recursive(storage, furi_string_get_cstr(work_path));
        }

        furi_string_free(pending[i]);
    }

    storage_simply_remove_recursive(storage, JS_APP_INSTALL_INCOMING_PATH);

    furi_string_free(work_path);
    furi_string_free(final_path);
    furi_string_free(name);
    furi_string_free(path);
    furi_record_close(RECORD_STORAGE);
}

JsAppInstallResult js_app_installer_install(const char* archive_path, FuriString* app_id) {
    furi_check(archive_path);
    furi_check(app_id);
    furi_string_reset(app_id);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    JsAppInstallResult result = js_app_installer_unpack(storage, archive_path);
    if(result != JsAppInstallResultOk) {
        furi_record_close(RECORD_STORAGE);
        return result;
    }

    FuriString* manifest_path = furi_string_alloc();
    FuriString* entry_path = furi_string_alloc();
    FuriString* staged_path = furi_string_alloc();
    FuriString* final_path = furi_string_alloc();
    FuriString* backup_path = furi_string_alloc();
    FuriString* candidate_version = furi_string_alloc();
    JsAppManifest* manifest = js_app_manifest_alloc();
    JsApp* installed_app = NULL;

    do {
        if(!storage_simply_mkpath(storage, JS_APPS_PATH)) {
            result = JsAppInstallResultStorageError;
            break;
        }

        path_concat(JS_APP_INSTALL_INCOMING_PATH, JS_APP_INSTALL_MANIFEST_PATH, manifest_path);
        path_concat(JS_APP_INSTALL_INCOMING_PATH, JS_APP_INSTALL_ENTRY_PATH, entry_path);
        if(!js_app_manifest_load_from_file(manifest, furi_string_get_cstr(manifest_path)) ||
           !storage_file_exists(storage, furi_string_get_cstr(entry_path))) {
            result = JsAppInstallResultInvalidApplication;
            break;
        }

        JsAppManifestInfo manifest_info;
        if(!js_app_manifest_get_info(manifest, &manifest_info) ||
           !js_app_id_is_valid(manifest_info.id)) {
            result = JsAppInstallResultInvalidApplication;
            break;
        }

        JsAppInstallVersion parsed_version;
        if(!js_app_installer_parse_version(manifest_info.version, &parsed_version)) {
            result = JsAppInstallResultInvalidApplication;
            break;
        }

        furi_string_set(app_id, manifest_info.id);
        furi_string_set(candidate_version, manifest_info.version);
        furi_string_printf(
            staged_path, "%s/%s.staged", JS_APP_INSTALL_WORK_PATH, manifest_info.id);
        path_concat(JS_APPS_PATH, manifest_info.id, final_path);
        furi_string_printf(
            backup_path, "%s/%s.backup", JS_APP_INSTALL_WORK_PATH, manifest_info.id);

        if(storage_common_exists(storage, furi_string_get_cstr(backup_path))) {
            if(storage_common_exists(storage, furi_string_get_cstr(final_path))) {
                if(!storage_simply_remove_recursive(storage, furi_string_get_cstr(backup_path))) {
                    result = JsAppInstallResultStorageError;
                    break;
                }
            } else if(
                storage_common_rename(
                    storage,
                    furi_string_get_cstr(backup_path),
                    furi_string_get_cstr(final_path)) != FSE_OK) {
                result = JsAppInstallResultStorageError;
                break;
            }
        }

        installed_app = js_app_registry_get_app(manifest_info.id);
        JsAppInfo installed_info;
        const bool is_update = installed_app && js_app_get_info(installed_app, &installed_info);

        if(is_update) {
            // An installed copy carrying a non-semver version (hand-copied dev
            // builds routinely say "1.0") counts as older rather than wedging
            // the id on InvalidApplication for good.
            JsAppInstallVersion installed_version;
            if(js_app_installer_parse_version(
                   installed_info.manifest.version, &installed_version) &&
               !js_app_installer_version_is_newer(
                   furi_string_get_cstr(candidate_version), installed_info.manifest.version)) {
                result = JsAppInstallResultVersionConflict;
                break;
            }
        } else if(storage_common_exists(storage, furi_string_get_cstr(final_path))) {
            // A directory that will not load as an application — left by a
            // half-finished removal or a partial write — otherwise blocks this
            // id permanently. Replace it instead of refusing forever.
            FURI_LOG_W(TAG, "Replacing unreadable application directory %s", manifest_info.id);
            if(!storage_simply_remove_recursive(storage, furi_string_get_cstr(final_path))) {
                result = JsAppInstallResultStorageError;
                break;
            }
        }

        storage_simply_remove_recursive(storage, furi_string_get_cstr(staged_path));
        if(storage_common_rename(
               storage, JS_APP_INSTALL_INCOMING_PATH, furi_string_get_cstr(staged_path)) !=
           FSE_OK) {
            result = JsAppInstallResultStorageError;
            break;
        }

        if(is_update &&
           (storage_common_rename(
                storage, furi_string_get_cstr(final_path), furi_string_get_cstr(backup_path)) !=
            FSE_OK)) {
            result = JsAppInstallResultStorageError;
            break;
        }

        if(storage_common_rename(
               storage, furi_string_get_cstr(staged_path), furi_string_get_cstr(final_path)) !=
           FSE_OK) {
            if(is_update) {
                storage_common_rename(
                    storage, furi_string_get_cstr(backup_path), furi_string_get_cstr(final_path));
            }
            result = JsAppInstallResultStorageError;
            break;
        }

        if(is_update) {
            storage_simply_remove_recursive(storage, furi_string_get_cstr(backup_path));
        }
        result = JsAppInstallResultOk;
    } while(false);

    if(installed_app) js_app_free(installed_app);
    js_app_manifest_free(manifest);
    furi_string_free(candidate_version);
    furi_string_free(backup_path);
    furi_string_free(final_path);

    // Reclaimed here rather than only on a retry of the same id, which may
    // never come.
    if(!furi_string_empty(staged_path)) {
        storage_simply_remove_recursive(storage, furi_string_get_cstr(staged_path));
    }
    furi_string_free(staged_path);
    furi_string_free(entry_path);
    furi_string_free(manifest_path);

    storage_simply_remove_recursive(storage, JS_APP_INSTALL_INCOMING_PATH);
    furi_record_close(RECORD_STORAGE);

    if(result != JsAppInstallResultOk) furi_string_reset(app_id);
    return result;
}
