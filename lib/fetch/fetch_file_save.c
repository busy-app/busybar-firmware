#include "fetch_file_save.h"
#include <storage/storage.h>
#include <toolbox/path.h>

struct FetchFileSave {
    Storage* storage;
    File* file_handle;
    FuriString* file_path;
};

#define TAG "FetchFileSave"

FetchFileSave* fetch_file_save_alloc(void) {
    FetchFileSave* instance = malloc(sizeof(FetchFileSave));

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->file_handle = storage_file_alloc(instance->storage);
    instance->file_path = furi_string_alloc();

    return instance;
}

void fetch_file_save_free(FetchFileSave* instance) {
    furi_check(instance);
    furi_check(instance->storage);

    furi_string_free(instance->file_path);
    storage_file_free(instance->file_handle);

    free(instance);

    furi_record_close(RECORD_STORAGE);
}

bool fetch_file_save_open(FetchFileSave* instance, const char* file_path) {
    furi_check(instance);
    furi_check(file_path);

    bool ret = false;

    FuriString* basedir = furi_string_alloc();

    do {
        path_extract_dirname(file_path, basedir);

        const bool is_valid_prefix =
            (furi_string_start_with(basedir, STORAGE_EXT_PATH_PREFIX) ||
             furi_string_start_with(basedir, STORAGE_BACKUP_PATH_PREFIX));

        if(!is_valid_prefix) {
            FURI_LOG_E(
                TAG,
                "File path must be within %s or %s: %s",
                STORAGE_EXT_PATH_PREFIX,
                STORAGE_BACKUP_PATH_PREFIX,
                furi_string_get_cstr(basedir));
            break;
        }

        if(path_recursive_create_dir(instance->storage, basedir) != FSE_OK) {
            FURI_LOG_E(TAG, "Failed to create directory: %s", furi_string_get_cstr(basedir));
            break;
        }

        if(!storage_simply_remove(instance->storage, file_path) != FSE_OK) {
            FURI_LOG_E(TAG, "Failed to remove existing temporary file: %s", file_path);
            break;
        }

        if(!storage_file_open(
               instance->file_handle,
               file_path,
               FSAM_WRITE,
               FSOM_CREATE_ALWAYS | FSOM_NONBLOCKING)) {
            FURI_LOG_E(TAG, "Failed to open file for writing: %s", file_path);
            break;
        }

        furi_string_set(instance->file_path, file_path);

        FURI_LOG_D(TAG, "Created file at: %s", file_path);

        ret = true;
    } while(false);

    furi_string_free(basedir);

    return ret;
}

bool fetch_file_save_write(FetchFileSave* instance, const void* data, size_t size) {
    furi_check(instance);
    furi_check(data);
    furi_check(size > 0);

    if(!storage_file_is_open(instance->file_handle)) {
        FURI_LOG_E(TAG, "File is not open for writing");
        return false;
    }

    const size_t written = storage_file_write(instance->file_handle, data, size);
    if(written != size) {
        FURI_LOG_E(TAG, "Failed to write all data to file. Wrote %zu of %zu", written, size);
        return false;
    }

    return true;
}

void fetch_file_save_remove(FetchFileSave* instance) {
    furi_check(instance);
    furi_check(instance->storage);

    if(storage_file_is_open(instance->file_handle)) {
        storage_file_close(instance->file_handle);
    }

    const char* file_path = furi_string_get_cstr(instance->file_path);

    if(storage_simply_remove(instance->storage, file_path)) {
        FURI_LOG_D(TAG, "Removed file: %s", file_path);
    } else {
        FURI_LOG_E(TAG, "Failed to remove file: %s", file_path);
    }
}
