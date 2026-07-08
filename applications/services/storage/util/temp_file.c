#include "temp_file.h"

#include "../storage.h"

#include <toolbox/path.h>

struct TempFile {
    Storage* storage;
    File* file;
    FuriString* file_path;
    bool is_keep;
};

static bool temp_file_cleanup(TempFile* instance) {
    bool success = false;

    do {
        if(storage_file_is_open(instance->file)) {
            if(!storage_file_close(instance->file)) {
                break;
            }
        }

        if(!(furi_string_empty(instance->file_path) || instance->is_keep)) {
            const char* file_path = furi_string_get_cstr(instance->file_path);
            if(!storage_simply_remove(instance->storage, file_path)) {
                break;
            }
        }

        success = true;
    } while(false);

    return success;
}

TempFile* temp_file_alloc(void) {
    TempFile* instance = malloc(sizeof(TempFile));

    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->file = storage_file_alloc(instance->storage);
    instance->file_path = furi_string_alloc();

    return instance;
}

void temp_file_free(TempFile* instance) {
    furi_check(instance);

    temp_file_cleanup(instance);

    furi_string_free(instance->file_path);
    storage_file_free(instance->file);

    free(instance);

    furi_record_close(RECORD_STORAGE);
}

bool temp_file_create(TempFile* instance, const char* path) {
    furi_check(instance);
    furi_check(path);

    bool success = false;

    FuriString* tmp = furi_string_alloc();

    do {
        if(!temp_file_cleanup(instance)) {
            break;
        }

        furi_string_set(instance->file_path, path);

        path_extract_dirname(path, tmp);

        if(!storage_simply_mkpath(instance->storage, furi_string_get_cstr(tmp))) {
            break;
        }

        if(!storage_file_open(
               instance->file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS | FSOM_NONBLOCKING)) {
            break;
        }

        success = true;
    } while(false);

    furi_string_free(tmp);

    return success;
}

bool temp_file_write(TempFile* instance, const void* data, size_t data_len) {
    furi_check(instance);
    furi_check(data);

    bool success = false;

    if(storage_file_is_open(instance->file)) {
        success = (storage_file_write(instance->file, data, data_len) == data_len);
    }

    return success;
}

void temp_file_set_keep(TempFile* instance, bool keep) {
    furi_check(instance);
    instance->is_keep = keep;
}
