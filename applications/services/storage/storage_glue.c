#include "storage_glue.h"
#include <furi_hal.h>

#define TAG "StorageGlue"

/************ storage file shared part ************/

static StorageFileShared* storage_file_shared_alloc(const FuriString* path) {
    StorageFileShared* shared = malloc(sizeof(StorageFileShared));
    shared->path = furi_string_alloc_set(path);
    shared->ref_count = 0;
    return shared;
}

static void storage_file_shared_free(StorageFileShared* shared) {
    furi_check(shared->ref_count == 0);

    furi_string_free(shared->path);
    free(shared);
}

/****************** storage file ******************/

void storage_file_init(StorageFile* obj) {
    obj->file = NULL;
    obj->file_data = NULL;
    obj->shared = NULL;
}

void storage_file_init_set(StorageFile* obj, const StorageFile* src) {
    UNUSED(obj);
    UNUSED(src);
    furi_crash(__FUNCTION__);
}

void storage_file_set(StorageFile* obj, const StorageFile* src) { //-V524
    UNUSED(obj);
    UNUSED(src);
    furi_crash(__FUNCTION__);
}

void storage_file_clear(StorageFile* obj) {
    if(obj->shared) {
        if(--obj->shared->ref_count == 0) {
            storage_file_shared_free(obj->shared);
        }
    }
}

/****************** storage data ******************/

void storage_data_init(StorageData* storage) {
    storage->data = NULL;
    storage->status = StorageStatusNotReady;
    StorageFileList_init(storage->files);
}

StorageStatus storage_data_status(StorageData* storage) {
    return storage->status;
}

const char* storage_data_status_text(StorageData* storage) {
    const char* result = "unknown";
    switch(storage->status) {
    case StorageStatusOK:
        result = "ok";
        break;
    case StorageStatusNotReady:
        result = "not ready";
        break;
    case StorageStatusNotMounted:
        result = "not mounted";
        break;
    case StorageStatusNoFS:
        result = "no filesystem";
        break;
    case StorageStatusNotAccessible:
        result = "not accessible";
        break;
    case StorageStatusErrorInternal:
        result = "internal";
        break;
    }

    return result;
}

void storage_data_timestamp(StorageData* storage) {
    storage->timestamp = furi_hal_rtc_get_timestamp();
}

uint32_t storage_data_get_timestamp(StorageData* storage) {
    return storage->timestamp;
}

/****************** storage glue ******************/

static StorageFile* storage_get_file(const File* file, StorageData* storage) {
    StorageFile* storage_file_ref = NULL;

    StorageFileList_it_t it;
    for(StorageFileList_it(it, storage->files); !StorageFileList_end_p(it);
        StorageFileList_next(it)) {
        StorageFile* storage_file = StorageFileList_ref(it);

        if(storage_file->file->file_id == file->file_id) {
            storage_file_ref = storage_file;
            break;
        }
    }

    return storage_file_ref;
}

static StorageFileShared* storage_get_file_shared(const FuriString* path, StorageData* storage) {
    StorageFileShared* storage_file_shared_ref = NULL;

    StorageFileList_it_t it;
    for(StorageFileList_it(it, storage->files); !StorageFileList_end_p(it);
        StorageFileList_next(it)) {
        StorageFile* storage_file = StorageFileList_ref(it);

        if(furi_string_equal(storage_file->shared->path, path)) {
            storage_file_shared_ref = storage_file->shared;
            break;
        }
    }

    return storage_file_shared_ref;
}

bool storage_has_file(const File* file, StorageData* storage) {
    return storage_get_file(file, storage) != NULL;
}

bool storage_path_already_open(FuriString* path, StorageData* storage) {
    return storage_get_file_shared(path, storage) != NULL;
}

void storage_set_storage_file_data(const File* file, void* file_data, StorageData* storage) {
    StorageFile* storage_file_ref = storage_get_file(file, storage);
    furi_check(storage_file_ref != NULL);
    storage_file_ref->file_data = file_data;
}

void* storage_get_storage_file_data(const File* file, StorageData* storage) {
    StorageFile* storage_file_ref = storage_get_file(file, storage);
    furi_check(storage_file_ref != NULL);
    return storage_file_ref->file_data;
}

void storage_push_storage_file(File* file, FuriString* path, StorageData* storage) {
    StorageFileShared* storage_file_shared = storage_get_file_shared(path, storage);

    if(storage_file_shared == NULL) {
        storage_file_shared = storage_file_shared_alloc(path);
    }

    ++storage_file_shared->ref_count;

    StorageFile* storage_file = StorageFileList_push_new(storage->files);
    file->file_id = (uint32_t)storage_file;
    storage_file->file = file;
    storage_file->shared = storage_file_shared;
}

bool storage_pop_storage_file(File* file, StorageData* storage) {
    StorageFileList_it_t it;
    bool result = false;

    for(StorageFileList_it(it, storage->files); !StorageFileList_end_p(it);
        StorageFileList_next(it)) {
        if(StorageFileList_cref(it)->file->file_id == file->file_id) {
            result = true;
            break;
        }
    }

    if(result) {
        StorageFileList_remove(storage->files, it);
    }

    return result;
}

size_t storage_open_files_count(StorageData* storage) {
    size_t count = StorageFileList_size(storage->files);
    return count;
}

void storage_set_read_only(StorageData* storage, bool read_only) {
    storage->read_only = read_only;
}

bool storage_is_read_only(StorageData* storage) {
    return storage->read_only;
}
