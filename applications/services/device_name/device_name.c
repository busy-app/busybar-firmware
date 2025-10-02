#include "device_name.h"

#include <storage/storage.h>
#include <toolbox/path.h>

#define TAG "Name"

#define DEFAULT_NAME "BUSY Bar"

#define MAX_NAME_LENGTH (10UL)

#define SETTINGS_PATH  EXT_PATH("apps_data/settings")
#define NAME_FILE_PATH SETTINGS_PATH "/name.txt"

#define DEVICE_NAME_SET_ERROR(error, text)          \
    ({                                              \
        if(error) furi_string_set_str(error, text); \
    })

struct DeviceName {
    FuriMutex* lock;
};

static bool settings_dir_create_if_not_exist(Storage* storage) {
    FuriString* dir_path = furi_string_alloc_set(SETTINGS_PATH);
    bool result = path_recursive_create_dir(storage, dir_path) == FSE_OK;
    furi_string_free(dir_path);
    return result;
}

static bool device_name_save_config(Storage* storage, FuriString* name) {
    bool result = false;
    File* file = storage_file_alloc(storage);
    do {
        if(!settings_dir_create_if_not_exist(storage)) {
            FURI_LOG_W(TAG, "Unable to create settings dir");
            break;
        }

        if(!storage_file_open(file, NAME_FILE_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            FURI_LOG_W(TAG, "Unable to create name file");
            break;
        }

        if(!storage_file_write(file, furi_string_get_cstr(name), furi_string_size(name))) {
            FURI_LOG_W(TAG, "Unable to write name");
            break;
        }

        result = true;
    } while(false);
    storage_file_free(file);

    return result;
}

static bool device_name_read_config(Storage* storage, FuriString* name) {
    bool result = false;
    File* file = storage_file_alloc(storage);
    do {
        if(!storage_file_open(file, NAME_FILE_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_W(TAG, "Unable to open file config");
            break;
        }

        uint64_t file_size = storage_file_size(file);
        if(file_size == 0) {
            FURI_LOG_W(TAG, "File is empty");
            break;
        }
        size_t name_size = MIN(file_size, MAX_NAME_LENGTH);
        char* buf = malloc(name_size + 1);
        buf[name_size] = 0;

        if(!storage_file_read(file, buf, name_size)) {
            FURI_LOG_W(TAG, "Unable to read name from file");
        } else
            result = true;

        furi_string_set_str(name, buf);
        free(buf);
    } while(false);
    storage_file_free(file);
    return result;
}

void device_name_get(DeviceName* instance, FuriString* name) {
    furi_assert(instance);
    furi_assert(name);

    furi_mutex_acquire(instance->lock, FuriWaitForever);
    Storage* storage = furi_record_open(RECORD_STORAGE);

    if(!storage_file_exists(storage, NAME_FILE_PATH) || !device_name_read_config(storage, name)) {
        furi_string_set_str(name, DEFAULT_NAME);
        FURI_LOG_I(TAG, "Default name used");

        if(!device_name_save_config(storage, name)) {
            FURI_LOG_E(TAG, "Failed to save name");
        }
    }

    furi_record_close(RECORD_STORAGE);
    furi_mutex_release(instance->lock);
}

bool device_name_set(DeviceName* instance, FuriString* name, FuriString* error) {
    furi_assert(instance);
    furi_assert(name);

    bool result = false;
    furi_mutex_acquire(instance->lock, FuriWaitForever);

    do {
        if(furi_string_empty(name)) {
            DEVICE_NAME_SET_ERROR(error, "Name is empty");
            break;
        }

        Storage* storage = furi_record_open(RECORD_STORAGE);
        if(device_name_save_config(storage, name)) {
            FURI_LOG_I(TAG, "New name: %s", furi_string_get_cstr(name));
            result = true;
        } else {
            DEVICE_NAME_SET_ERROR(error, "Failed to save name");
        }
        furi_record_close(RECORD_STORAGE);
    } while(false);

    furi_mutex_release(instance->lock);
    return result;
}

int device_name_startup(void* arg) {
    UNUSED(arg);

    DeviceName* instance = malloc(sizeof(DeviceName));
    instance->lock = furi_mutex_alloc(FuriMutexTypeNormal);

    furi_record_create(RECORD_DEVICE_NAME, instance);

    FuriString* name = furi_string_alloc();
    device_name_get(instance, name);
    FURI_LOG_I(TAG, "Device name: %s", furi_string_get_cstr(name));
    furi_string_free(name);

    return 0;
}
