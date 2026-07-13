#include "temp_file.h"

#include <toolbox/path.h>

#define FILE_ACCESS_MODE (FSAM_WRITE)
#define FILE_OPEN_MODE   (FSOM_CREATE_ALWAYS | FSOM_NONBLOCKING)

struct TempFile {
    Storage* storage;
    File* file;
    FuriString* file_path;
};

static bool temp_file_close_internal(TempFile* instance) {
    bool success = true;

    if(storage_file_is_open(instance->file)) {
        success = storage_file_close(instance->file);
    }

    return success;
}

static bool temp_file_remove_internal(TempFile* instance) {
    bool success = true;

    const FuriString* file_path = instance->file_path;

    if(!furi_string_empty(file_path)) {
        success = storage_simply_remove(instance->storage, furi_string_get_cstr(file_path));
    }

    return success;
}

TempFile* temp_file_alloc(Storage* storage) {
    furi_check(storage);

    TempFile* instance = malloc(sizeof(TempFile));

    instance->storage = storage;
    instance->file = storage_file_alloc(storage);
    instance->file_path = furi_string_alloc();

    return instance;
}

void temp_file_free(TempFile* instance) {
    furi_check(instance);

    temp_file_close_internal(instance);

    furi_string_free(instance->file_path);
    storage_file_free(instance->file);

    free(instance);
}

bool temp_file_create(TempFile* instance, const char* path) {
    furi_check(instance);
    furi_check(path);

    bool success = false;

    FuriString* tmp = furi_string_alloc();

    do {
        if(!temp_file_close_internal(instance)) {
            break;
        }

        furi_string_set(instance->file_path, path);

        path_extract_dirname(path, tmp);

        if(!storage_simply_mkpath(instance->storage, furi_string_get_cstr(tmp))) {
            break;
        }

        if(!storage_file_open(instance->file, path, FILE_ACCESS_MODE, FILE_OPEN_MODE)) {
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

bool temp_file_remove(TempFile* instance) {
    furi_check(instance);
    return temp_file_close_internal(instance) && temp_file_remove_internal(instance);
}
