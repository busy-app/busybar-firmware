#include <m-dict.h>
#include <m-list.h>

#include <storage/storage_processing.h>

#define TAG "StorageProcessing"

#define STORAGE_PATH_PREFIX_LEN 4u
_Static_assert(
    sizeof(STORAGE_EXT_PATH_PREFIX) == STORAGE_PATH_PREFIX_LEN + 1,
    "Ext path prefix len mismatch");

#define FS_CALL(_storage, _fn) ret = _storage->fs_api->_fn;

static bool storage_type_is_valid(StorageType type) {
    return type < ST_MAX;
}

static StorageData* get_storage_by_file(File* file, StorageData* storages) {
    StorageData* storage_data = NULL;

    for(uint8_t i = 0; i < STORAGE_COUNT; i++) {
        if(storage_has_file(file, &storages[i])) {
            storage_data = &storages[i];
        }
    }

    return storage_data;
}

static StorageType storage_get_type_by_path(FuriString* path) {
    StorageType type = ST_MAX;
    const char* path_cstr = furi_string_get_cstr(path);

    if(furi_string_size(path) > STORAGE_PATH_PREFIX_LEN) {
        if(path_cstr[STORAGE_PATH_PREFIX_LEN] != '/') {
            return ST_MAX;
        }
    }

    if(memcmp(path_cstr, STORAGE_EXT_PATH_PREFIX, strlen(STORAGE_EXT_PATH_PREFIX)) == 0) {
        type = ST_EXT;
    } else if(memcmp(path_cstr, STORAGE_BACKUP_PATH_PREFIX, strlen(STORAGE_BACKUP_PATH_PREFIX)) == 0) {
        type = ST_BKP;
    }

    return type;
}

static const char* cstr_storage_path(Storage* app, StorageType type, FuriString* path) {
    if(type >= ST_MAX) {
        furi_crash("Invalid storage type");
    }

    furi_string_set(
        app->path_storage, app->storage[type].fs_api->storage.prefix(&app->storage[type]));

    const size_t prefix_len = MIN(STORAGE_PATH_PREFIX_LEN, furi_string_size(path));
    const char* clear_path = furi_string_get_cstr(path) + prefix_len;

    furi_string_cat(app->path_storage, clear_path);

    return furi_string_get_cstr(app->path_storage);
}

static FS_Error storage_get_data(Storage* app, StorageType type, StorageData** storage) {
    if(storage_type_is_valid(type)) {
        if(storage_data_status(&app->storage[type]) != StorageStatusOK) {
            return FSE_NOT_READY;
        }

        *storage = &app->storage[type];

        return FSE_OK;
    } else {
        return FSE_INVALID_NAME;
    }
}

static void storage_path_trim_trailing_slashes(FuriString* path) {
    while(furi_string_end_with(path, "/")) {
        furi_string_left(path, furi_string_size(path) - 1);
    }
}

/******************* File Functions *******************/

bool storage_process_file_open(
    Storage* app,
    File* file,
    FuriString* path,
    FS_AccessMode access_mode,
    FS_OpenMode open_mode) {
    bool ret = false;
    StorageData* storage;
    StorageType type = storage_get_type_by_path(path);
    file->error_id = storage_get_data(app, type, &storage);

    do {
        if(file->error_id != FSE_OK) break;

        if(storage_path_already_open(path, storage)) {
            file->error_id = FSE_ALREADY_OPEN;
            break;
        }

        if(access_mode & FSAM_WRITE) {
            if(storage_is_read_only(storage)) {
                file->error_id = FSE_DENIED;
                break;
            }

            storage_data_timestamp(storage);
        }

        storage_push_storage_file(file, path, storage);

        const char* path_cstr_no_vfs = cstr_storage_path(app, type, path);
        FS_CALL(storage, file.open(storage, file, path_cstr_no_vfs, access_mode, open_mode));
    } while(false);

    return ret;
}

bool storage_process_file_close(Storage* app, File* file) {
    bool ret = false;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        FS_CALL(storage, file.close(storage, file));
        storage_pop_storage_file(file, storage);

        StorageEvent event = {.type = StorageEventTypeFileClose};
        furi_pubsub_publish(app->pubsub, &event);
    }

    return ret;
}

static uint16_t
    storage_process_file_read(Storage* app, File* file, void* buff, uint16_t const bytes_to_read) {
    uint16_t ret = 0;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        FS_CALL(storage, file.read(storage, file, buff, bytes_to_read));
    }

    return ret;
}

static uint16_t storage_process_file_write(
    Storage* app,
    File* file,
    const void* buff,
    uint16_t const bytes_to_write) {
    uint16_t ret = 0;
    StorageData* storage = get_storage_by_file(file, app->storage);

    do {
        if(storage == NULL) {
            file->error_id = FSE_INVALID_PARAMETER;
            break;
        }

        if(storage_is_read_only(storage)) {
            file->error_id = FSE_DENIED;
            break;
        }

        storage_data_timestamp(storage);

        FS_CALL(storage, file.write(storage, file, buff, bytes_to_write));
    } while(false);

    return ret;
}

static bool storage_process_file_seek(
    Storage* app,
    File* file,
    const uint32_t offset,
    const bool from_start) {
    bool ret = false;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        FS_CALL(storage, file.seek(storage, file, offset, from_start));
    }

    return ret;
}

static uint64_t storage_process_file_tell(Storage* app, File* file) {
    uint64_t ret = 0;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        FS_CALL(storage, file.tell(storage, file));
    }

    return ret;
}

static bool storage_process_file_truncate(Storage* app, File* file) {
    bool ret = false;
    StorageData* storage = get_storage_by_file(file, app->storage);

    do {
        if(storage == NULL) {
            file->error_id = FSE_INVALID_PARAMETER;
            break;
        }

        if(storage_is_read_only(storage)) {
            file->error_id = FSE_DENIED;
            break;
        }

        storage_data_timestamp(storage);
        FS_CALL(storage, file.truncate(storage, file));
    } while(false);

    return ret;
}

static bool storage_process_file_sync(Storage* app, File* file) {
    bool ret = false;
    StorageData* storage = get_storage_by_file(file, app->storage);

    do {
        if(storage == NULL) {
            file->error_id = FSE_INVALID_PARAMETER;
            break;
        }

        if(storage_is_read_only(storage)) {
            file->error_id = FSE_DENIED;
            break;
        }

        storage_data_timestamp(storage);
        FS_CALL(storage, file.sync(storage, file));
    } while(false);

    return ret;
}

static uint64_t storage_process_file_size(Storage* app, File* file) {
    uint64_t ret = 0;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        FS_CALL(storage, file.size(storage, file));
    }

    return ret;
}

static bool storage_process_file_eof(Storage* app, File* file) {
    bool ret = false;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        FS_CALL(storage, file.eof(storage, file));
    }

    return ret;
}

/******************* Dir Functions *******************/

bool storage_process_dir_open(Storage* app, File* file, FuriString* path) {
    bool ret = false;
    StorageData* storage;
    StorageType type = storage_get_type_by_path(path);
    file->error_id = storage_get_data(app, type, &storage);

    if(file->error_id == FSE_OK) {
        if(storage_path_already_open(path, storage)) {
            file->error_id = FSE_ALREADY_OPEN;
        } else {
            storage_push_storage_file(file, path, storage);
            FS_CALL(storage, dir.open(storage, file, cstr_storage_path(app, type, path)));
        }
    }

    return ret;
}

bool storage_process_dir_close(Storage* app, File* file) {
    bool ret = false;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        FS_CALL(storage, dir.close(storage, file));
        storage_pop_storage_file(file, storage);

        StorageEvent event = {.type = StorageEventTypeDirClose};
        furi_pubsub_publish(app->pubsub, &event);
    }

    return ret;
}

bool storage_process_dir_read(
    Storage* app,
    File* file,
    FileInfo* fileinfo,
    char* name,
    const uint16_t name_length) {
    bool ret = false;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        FS_CALL(storage, dir.read(storage, file, fileinfo, name, name_length));
    }

    return ret;
}

bool storage_process_dir_rewind(Storage* app, File* file) {
    bool ret = false;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        FS_CALL(storage, dir.rewind(storage, file));
    }

    return ret;
}

/******************* Common FS Functions *******************/

static FS_Error
    storage_process_common_timestamp(Storage* app, FuriString* path, uint32_t* timestamp) {
    StorageData* storage;
    StorageType type = storage_get_type_by_path(path);
    FS_Error ret = storage_get_data(app, type, &storage);

    if(ret == FSE_OK) {
        *timestamp = storage_data_get_timestamp(storage);
    }

    return ret;
}

static FS_Error storage_process_common_stat(Storage* app, FuriString* path, FileInfo* fileinfo) {
    StorageData* storage;
    StorageType type = storage_get_type_by_path(path);
    FS_Error ret = storage_get_data(app, type, &storage);

    if(ret == FSE_OK) {
        FS_CALL(storage, common.stat(storage, cstr_storage_path(app, type, path), fileinfo));
    }

    return ret;
}

static FS_Error storage_process_common_remove(Storage* app, FuriString* path) {
    FS_Error ret;

    do {
        StorageData* storage;
        StorageType type = storage_get_type_by_path(path);
        ret = storage_get_data(app, type, &storage);

        if(ret != FSE_OK) break;

        if(storage_path_already_open(path, storage)) {
            ret = FSE_ALREADY_OPEN;
            break;
        }

        if(storage_is_read_only(storage)) {
            ret = FSE_DENIED;
            break;
        }

        storage_data_timestamp(storage);
        FS_CALL(storage, common.remove(storage, cstr_storage_path(app, type, path)));
    } while(false);

    return ret;
}

static FS_Error storage_process_common_mkdir(Storage* app, FuriString* path) {
    FS_Error ret;

    do {
        StorageData* storage;
        StorageType type = storage_get_type_by_path(path);
        ret = storage_get_data(app, type, &storage);

        if(ret != FSE_OK) break;

        if(storage_is_read_only(storage)) {
            ret = FSE_DENIED;
            break;
        }

        storage_data_timestamp(storage);
        FS_CALL(storage, common.mkdir(storage, cstr_storage_path(app, type, path)));
    } while(false);

    return ret;
}

static FS_Error storage_process_common_fs_info(
    Storage* app,
    FuriString* path,
    uint64_t* total_space,
    uint64_t* free_space,
    bool* is_read_only) {
    StorageData* storage;
    StorageType type = storage_get_type_by_path(path);
    FS_Error ret = storage_get_data(app, type, &storage);

    if(ret == FSE_OK) {
        *is_read_only = storage_is_read_only(storage);
        FS_CALL(
            storage,
            common.fs_info(storage, cstr_storage_path(app, type, path), total_space, free_space));
    }

    return ret;
}

static bool
    storage_process_common_equivalent_path(Storage* app, FuriString* path1, FuriString* path2) {
    bool ret = false;

    do {
        const StorageType storage_type1 = storage_get_type_by_path(path1);
        const StorageType storage_type2 = storage_get_type_by_path(path2);

        // Paths on different storages are of course not equal
        if(storage_type1 != storage_type2) break;

        StorageData* storage;
        const FS_Error status = storage_get_data(app, storage_type1, &storage);

        if(status != FSE_OK) break;

        FS_CALL(
            storage,
            common.equivalent_path(furi_string_get_cstr(path1), furi_string_get_cstr(path2)));

    } while(false);

    return ret;
}

/****************** Raw SD API ******************/
#include "storages/storage_ext_sdmmc.h"

static FS_Error storage_process_sd_mkfs(Storage* app, FuriString* path) {
    FS_Error ret;

    do {
        if(furi_string_cmp(path, "/") != 0) {
            ret = FSE_INVALID_NAME;
            break;
        }

        if(storage_is_read_only(&app->storage[ST_BKP])) {
            ret = FSE_DENIED;
            break;
        }

        if(storage_is_read_only(&app->storage[ST_EXT])) {
            ret = FSE_DENIED;
            break;
        }

        ret = storage_ext_mk_partititons();

        storage_data_timestamp(&app->storage[ST_BKP]);
        storage_data_timestamp(&app->storage[ST_EXT]);
    } while(false);

    return ret;
}

static FS_Error storage_process_sd_format(Storage* app, FuriString* path) {
    FS_Error ret;

    do {
        StorageData* storage;
        StorageType type = storage_get_type_by_path(path);
        ret = storage_get_data(app, type, &storage);

        if(ret != FSE_OK) break;

        if(storage_is_read_only(storage)) {
            ret = FSE_DENIED;
            break;
        }

        FS_CALL(storage, storage.format(storage));
        storage_data_timestamp(storage);
    } while(false);

    return ret;
}

static FS_Error storage_process_sd_unmount(Storage* app, FuriString* path) {
    FS_Error ret;

    do {
        StorageData* storage;
        StorageType type = storage_get_type_by_path(path);
        ret = storage_get_data(app, type, &storage);

        if(ret != FSE_OK) break;

        if(storage_is_read_only(storage)) {
            ret = FSE_DENIED;
            break;
        }

        FS_CALL(storage, storage.unmount(storage));
        storage_data_timestamp(storage);
    } while(false);

    return ret;
}

static FS_Error storage_process_sd_mount(Storage* app, FuriString* path) {
    FS_Error ret;

    do {
        StorageData* storage;
        StorageType type = storage_get_type_by_path(path);
        ret = storage_get_data(app, type, &storage);

        if(ret != FSE_OK) break;

        if(storage_is_read_only(storage)) {
            ret = FSE_DENIED;
            break;
        }

        FS_CALL(storage, storage.mount(storage));
        storage_data_timestamp(storage);
    } while(false);

    return ret;
}

static FS_Error storage_process_sd_info(Storage* app, FuriString* path, SDInfo* info) {
    FS_Error ret;

    do {
        StorageData* storage;
        StorageType type = storage_get_type_by_path(path);
        ret = storage_get_data(app, type, &storage);

        if(ret != FSE_OK) break;

        if(storage_is_read_only(storage)) {
            ret = FSE_DENIED;
            break;
        }

        FS_CALL(storage, storage.info(storage, info));
        storage_data_timestamp(storage);
    } while(false);

    return ret;
}

static FS_Error storage_process_sd_status(Storage* app, FuriString* path) {
    FS_Error ret;

    do {
        StorageData* storage;
        StorageType type = storage_get_type_by_path(path);
        ret = storage_get_data(app, type, &storage);

        if(ret != FSE_OK) break;

        StorageStatus status = storage_data_status(storage);

        switch(status) {
        case StorageStatusOK:
            ret = FSE_OK;
            break;
        case StorageStatusNotReady:
            ret = FSE_NOT_READY;
            break;
        default:
            ret = FSE_INTERNAL;
            break;
        }
    } while(false);

    return ret;
}

/******************** Aliases processing *******************/

void storage_process_alias(
    Storage* app,
    FuriString* path,
    FuriThreadId thread_id,
    bool create_folders) {
    if(furi_string_start_with(path, STORAGE_APP_DATA_PATH_PREFIX)) {
        FuriString* apps_data_path_with_appsid = furi_string_alloc_set(APPS_DATA_PATH "/");
        furi_string_cat(apps_data_path_with_appsid, furi_thread_get_appid(thread_id));

        // "/data" -> "/ext/apps_data/appsid"
        furi_string_replace_at(
            path,
            0,
            strlen(STORAGE_APP_DATA_PATH_PREFIX),
            furi_string_get_cstr(apps_data_path_with_appsid));

        // Create app data folder if not exists
        if(create_folders &&
           storage_process_common_stat(app, apps_data_path_with_appsid, NULL) != FSE_OK) {
            furi_string_set(apps_data_path_with_appsid, APPS_DATA_PATH);
            storage_process_common_mkdir(app, apps_data_path_with_appsid);
            furi_string_cat(apps_data_path_with_appsid, "/");
            furi_string_cat(apps_data_path_with_appsid, furi_thread_get_appid(thread_id));
            storage_process_common_mkdir(app, apps_data_path_with_appsid);
        }

        furi_string_free(apps_data_path_with_appsid);
    } else if(furi_string_start_with(path, STORAGE_APP_ASSETS_PATH_PREFIX)) {
        FuriString* apps_assets_path_with_appsid = furi_string_alloc_set(APPS_ASSETS_PATH "/");
        furi_string_cat(apps_assets_path_with_appsid, furi_thread_get_appid(thread_id));

        // "/assets" -> "/ext/apps_assets/appsid"
        furi_string_replace_at(
            path,
            0,
            strlen(STORAGE_APP_ASSETS_PATH_PREFIX),
            furi_string_get_cstr(apps_assets_path_with_appsid));

        furi_string_free(apps_assets_path_with_appsid);
    }
}

/****************** SD Presence ******************/

void storage_process_message_internal(Storage* app, StorageMessage* message) {
    switch(message->command) {
    // File operations
    case StorageCommandFileOpen:
        furi_string_set(app->path_aliased, message->data->fopen.path);
        storage_process_alias(app, app->path_aliased, message->data->fopen.thread_id, true);
        message->return_data->bool_value = storage_process_file_open(
            app,
            message->data->fopen.file,
            app->path_aliased,
            message->data->fopen.access_mode,
            message->data->fopen.open_mode);
        break;
    case StorageCommandFileClose:
        message->return_data->bool_value =
            storage_process_file_close(app, message->data->fopen.file);
        break;
    case StorageCommandFileRead:
        message->return_data->uint16_value = storage_process_file_read(
            app,
            message->data->fread.file,
            message->data->fread.buff,
            message->data->fread.bytes_to_read);
        break;
    case StorageCommandFileWrite:
        message->return_data->uint16_value = storage_process_file_write(
            app,
            message->data->fwrite.file,
            message->data->fwrite.buff,
            message->data->fwrite.bytes_to_write);
        break;
    case StorageCommandFileSeek:
        message->return_data->bool_value = storage_process_file_seek(
            app,
            message->data->fseek.file,
            message->data->fseek.offset,
            message->data->fseek.from_start);
        break;
    case StorageCommandFileTell:
        message->return_data->uint64_value =
            storage_process_file_tell(app, message->data->file.file);
        break;
    case StorageCommandFileTruncate:
        message->return_data->bool_value =
            storage_process_file_truncate(app, message->data->file.file);
        break;
    case StorageCommandFileSync:
        message->return_data->bool_value =
            storage_process_file_sync(app, message->data->file.file);
        break;
    case StorageCommandFileSize:
        message->return_data->uint64_value =
            storage_process_file_size(app, message->data->file.file);
        break;
    case StorageCommandFileEof:
        message->return_data->bool_value = storage_process_file_eof(app, message->data->file.file);
        break;

    // Dir operations
    case StorageCommandDirOpen:
        furi_string_set(app->path_aliased, message->data->dopen.path);
        storage_process_alias(app, app->path_aliased, message->data->dopen.thread_id, true);
        message->return_data->bool_value =
            storage_process_dir_open(app, message->data->dopen.file, app->path_aliased);
        break;
    case StorageCommandDirClose:
        message->return_data->bool_value =
            storage_process_dir_close(app, message->data->file.file);
        break;
    case StorageCommandDirRead:
        message->return_data->bool_value = storage_process_dir_read(
            app,
            message->data->dread.file,
            message->data->dread.fileinfo,
            message->data->dread.name,
            message->data->dread.name_length);
        break;
    case StorageCommandDirRewind:
        message->return_data->bool_value =
            storage_process_dir_rewind(app, message->data->file.file);
        break;

    // Common operations
    case StorageCommandCommonTimestamp:
        furi_string_set(app->path_aliased, message->data->ctimestamp.path);
        storage_process_alias(app, app->path_aliased, message->data->ctimestamp.thread_id, false);
        message->return_data->error_value = storage_process_common_timestamp(
            app, app->path_aliased, message->data->ctimestamp.timestamp);
        break;
    case StorageCommandCommonStat:
        furi_string_set(app->path_aliased, message->data->cstat.path);
        storage_process_alias(app, app->path_aliased, message->data->cstat.thread_id, false);
        message->return_data->error_value =
            storage_process_common_stat(app, app->path_aliased, message->data->cstat.fileinfo);
        break;
    case StorageCommandCommonRemove:
        furi_string_set(app->path_aliased, message->data->path.path);
        storage_process_alias(app, app->path_aliased, message->data->path.thread_id, false);
        message->return_data->error_value = storage_process_common_remove(app, app->path_aliased);
        break;
    case StorageCommandCommonMkDir:
        furi_string_set(app->path_aliased, message->data->path.path);
        storage_process_alias(app, app->path_aliased, message->data->path.thread_id, true);
        message->return_data->error_value = storage_process_common_mkdir(app, app->path_aliased);
        break;
    case StorageCommandCommonFSInfo:
        furi_string_set(app->path_aliased, message->data->cfsinfo.fs_path);
        storage_process_alias(app, app->path_aliased, message->data->cfsinfo.thread_id, false);
        message->return_data->error_value = storage_process_common_fs_info(
            app,
            app->path_aliased,
            message->data->cfsinfo.total_space,
            message->data->cfsinfo.free_space,
            message->data->cfsinfo.is_read_only);
        break;
    case StorageCommandCommonResolvePath:
        storage_process_alias(
            app, message->data->cresolvepath.path, message->data->cresolvepath.thread_id, true);
        break;

    case StorageCommandCommonEquivalentPath: {
        FuriString* path1 = furi_string_alloc_set(message->data->cequivpath.path1);
        FuriString* path2 = furi_string_alloc_set(message->data->cequivpath.path2);
        storage_path_trim_trailing_slashes(path1);
        storage_path_trim_trailing_slashes(path2);
        storage_process_alias(app, path1, message->data->cequivpath.thread_id, false);
        storage_process_alias(app, path2, message->data->cequivpath.thread_id, false);
        if(message->data->cequivpath.check_subdir) {
            // by appending slashes at the end and then truncating the second path, we can
            // effectively check for shared path components:
            // example 1:
            //   path1: "/ext/blah"      -> "/ext/blah/"      -> "/ext/blah/"
            //   path2: "/ext/blah-blah" -> "/ect/blah-blah/" -> "/ext/blah-"
            //   results unequal, conclusion: path2 is not a subpath of path1
            // example 2:
            //   path1: "/ext/blah"      -> "/ext/blah/"      -> "/ext/blah/"
            //   path2: "/ext/blah/blah" -> "/ect/blah/blah/" -> "/ext/blah/"
            //   results equal, conclusion: path2 is a subpath of path1
            // example 3:
            //   path1: "/ext/blah/blah" -> "/ect/blah/blah/" -> "/ext/blah/blah/"
            //   path2: "/ext/blah"      -> "/ext/blah/"      -> "/ext/blah/"
            //   results unequal, conclusion: path2 is not a subpath of path1
            furi_string_push_back(path1, '/');
            furi_string_push_back(path2, '/');
            furi_string_left(path2, furi_string_size(path1));
        }
        message->return_data->bool_value =
            storage_process_common_equivalent_path(app, path1, path2);
        furi_string_free(path1);
        furi_string_free(path2);
        break;
    }

    // SD operations
    case StorageCommandSDFormat:
        furi_string_set(app->path_aliased, message->data->path.path);
        storage_process_alias(app, app->path_aliased, message->data->path.thread_id, false);
        message->return_data->error_value = storage_process_sd_format(app, app->path_aliased);
        break;
    case StorageCommandSDMakePartitions:
        furi_string_set(app->path_aliased, message->data->path.path);
        storage_process_alias(app, app->path_aliased, message->data->path.thread_id, false);
        message->return_data->error_value = storage_process_sd_mkfs(app, app->path_aliased);
        break;
    case StorageCommandSDUnmount:
        furi_string_set(app->path_aliased, message->data->path.path);
        storage_process_alias(app, app->path_aliased, message->data->path.thread_id, false);
        message->return_data->error_value = storage_process_sd_unmount(app, app->path_aliased);
        break;
    case StorageCommandSDMount:
        furi_string_set(app->path_aliased, message->data->path.path);
        storage_process_alias(app, app->path_aliased, message->data->path.thread_id, false);
        message->return_data->error_value = storage_process_sd_mount(app, app->path_aliased);
        break;
    case StorageCommandSDInfo:
        furi_string_set(app->path_aliased, message->data->sdinfo.path);
        storage_process_alias(app, app->path_aliased, message->data->sdinfo.thread_id, false);
        message->return_data->error_value =
            storage_process_sd_info(app, app->path_aliased, message->data->sdinfo.info);
        break;
    case StorageCommandSDStatus:
        furi_string_set(app->path_aliased, message->data->path.path);
        storage_process_alias(app, app->path_aliased, message->data->path.thread_id, false);
        message->return_data->error_value = storage_process_sd_status(app, app->path_aliased);
        break;
    case StorageCommandBackupReadOnly:
        StorageData* storage = &app->storage[ST_BKP];
        storage_set_read_only(storage, message->data->readonly.readonly);
        message->return_data->bool_value = true;
        break;
    }

    furi_string_set(app->path_aliased, "");
    furi_string_set(app->path_storage, "");

    api_lock_unlock(message->lock);
}

void storage_process_message(Storage* app, StorageMessage* message) {
    storage_process_message_internal(app, message);
}
