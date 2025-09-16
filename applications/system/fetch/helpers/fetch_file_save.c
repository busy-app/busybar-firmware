#include "fetch_file_save.h"
#include <storage/storage.h>
#include <toolbox/path.h>

struct FetchFileSave {
    Storage* storage;
    File* file_handle;
    FuriString* file_path;
};

#define TAG "FetchFileSave"

FetchFileSave* fetch_file_save_alloc(FuriString* file_path) {
    furi_check(file_path);

    FetchFileSave* instance = malloc(sizeof(FetchFileSave));

    bool ret = false;

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->file_path = furi_string_alloc_set(furi_string_get_cstr(file_path));
    instance->file_handle = storage_file_alloc(instance->storage);

    FuriString* path = furi_string_alloc();

    do {
        // Ensure staging directory exists
        path_extract_dirname(furi_string_get_cstr(instance->file_path), path);

        if(!(strncmp(
                 furi_string_get_cstr(path),
                 STORAGE_EXT_PATH_PREFIX,
                 strlen(STORAGE_EXT_PATH_PREFIX)) == 0 ||
             strncmp(
                 furi_string_get_cstr(path),
                 STORAGE_BACKUP_PATH_PREFIX,
                 strlen(STORAGE_BACKUP_PATH_PREFIX)) == 0)) {
            FURI_LOG_E(
                TAG,
                "File path must be within %s or %s: %s",
                STORAGE_EXT_PATH_PREFIX,
                STORAGE_BACKUP_PATH_PREFIX,
                furi_string_get_cstr(path));
            break;
        }

        if(path_recursive_create_dir(instance->storage, path) != FSE_OK) {
            FURI_LOG_E(TAG, "Failed to create directory: %s", furi_string_get_cstr(path));
            break;
        }

        // Ensure existing file is removed
        if(storage_file_exists(instance->storage, furi_string_get_cstr(instance->file_path))) {
            if(!storage_simply_remove(
                   instance->storage, furi_string_get_cstr(instance->file_path)) != FSE_OK) {
                FURI_LOG_E(
                    TAG,
                    "Failed to remove existing temporary file: %s",
                    furi_string_get_cstr(instance->file_path));
                break;
            }
        }

        // Create and open file for writing
        if(!storage_file_open(
               instance->file_handle,
               furi_string_get_cstr(instance->file_path),
               FSAM_WRITE,
               FSOM_CREATE_ALWAYS)) {
            FURI_LOG_E(
                TAG,
                "Failed to open file for writing: %s",
                furi_string_get_cstr(instance->file_path));
            break;
        }
        FURI_LOG_D(TAG, "Created file at: %s", furi_string_get_cstr(instance->file_path));
        ret = true;
    } while(0);

    if(!ret) {
        fetch_file_save_free(instance);
        return NULL;
    }

    furi_string_free(path);
    return instance;
}

void fetch_file_save_free(FetchFileSave* instance) {
    furi_check(instance);
    furi_check(instance->storage);
    furi_check(instance->file_path);

    if(storage_file_is_open(instance->file_handle)) {
        storage_file_close(instance->file_handle);
    }
    storage_file_free(instance->file_handle);

    furi_string_free(instance->file_path);
    furi_record_close(RECORD_STORAGE);

    free(instance);
}

bool fetch_file_save_write(FetchFileSave* instance, uint8_t* data, size_t size) {
    furi_check(instance);
    furi_check(data);
    furi_check(size > 0);

    if(!storage_file_is_open(instance->file_handle)) {
        FURI_LOG_E(TAG, "File is not open for writing");
        return false;
    }

    size_t written = storage_file_write(instance->file_handle, data, size);
    if(written != size) {
        FURI_LOG_E(TAG, "Failed to write all data to file. Wrote %zu of %zu", written, size);
        return false;
    }

    return true;
}

void fetch_file_save_remove(FetchFileSave* instance) {
    furi_check(instance);
    furi_check(instance->storage);
    furi_check(instance->file_path);

    if(storage_file_is_open(instance->file_handle)) {
        storage_file_close(instance->file_handle);
    }

    if(storage_file_exists(instance->storage, furi_string_get_cstr(instance->file_path))) {
        if(!storage_simply_remove(instance->storage, furi_string_get_cstr(instance->file_path)) !=
           FSE_OK) {
            FURI_LOG_E(
                TAG, "Failed to remove file: %s", furi_string_get_cstr(instance->file_path));
        } else {
            FURI_LOG_D(TAG, "Removed file: %s", furi_string_get_cstr(instance->file_path));
        }
    }
}
