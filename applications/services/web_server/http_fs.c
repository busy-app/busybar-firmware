#include <furi.h>
#include <storage/storage.h>
#include "mongoose.h"

#define TAG "HTTP FS"

#define MAX_FILENAME_LEN 255

static int fs_stat(const char* path, size_t* size, time_t* mtime) {
    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    FileInfo file_info;
    FS_Error result = storage_common_stat(fs_api, path, &file_info);
    uint32_t ts = 0;
    if(result == FSE_OK) {
        storage_common_timestamp(fs_api, path, &ts);
    } else {
        furi_record_close(RECORD_STORAGE);
        return 0;
    }
    furi_record_close(RECORD_STORAGE);

    if(size) {
        *size = (size_t)file_info.size;
    }
    if(mtime) {
        *mtime = ts;
    }

    return MG_FS_READ | MG_FS_WRITE | ((file_info.flags & FSF_DIRECTORY) ? MG_FS_DIR : 0);
}

static void fs_list(const char* path, void (*fn)(const char*, void*), void* userdata) {
    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(fs_api);

    if(storage_dir_open(file, path)) {
        FileInfo fileinfo;
        char* name = malloc(MAX_FILENAME_LEN);

        while(storage_dir_read(file, &fileinfo, name, MAX_FILENAME_LEN)) {
            fn(name, userdata);
        }

        free(name);
    }

    storage_dir_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

static void* fs_open(const char* path, int flags) {
    if(flags & MG_FS_DIR) {
        FURI_LOG_W(TAG, "TODO: %s MG_FS_DIR", __func__);
        return NULL;
    }

    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(fs_api);

    FS_AccessMode access = (flags & MG_FS_WRITE) ? FSAM_READ_WRITE : FSAM_READ;
    FS_OpenMode open_mode = (flags & MG_FS_WRITE) ? FSOM_OPEN_APPEND : FSOM_OPEN_EXISTING;

    if(!storage_file_open(file, path, access, open_mode)) {
        storage_file_close(file);
        storage_file_free(file);
        file = NULL;
    }

    furi_record_close(RECORD_STORAGE);

    return file;
}

static void fs_close(void* fp) {
    storage_file_close(fp);
    storage_file_free(fp);
}

static size_t fs_read(void* fd, void* buf, size_t len) {
    return storage_file_read(fd, buf, len);
}

static size_t fs_write(void* fd, const void* buf, size_t len) {
    uint8_t* temp_buf = malloc(len);
    memcpy(temp_buf, buf, len);
    // TODO: fix sdmmc buffer alignment bug to get rid of temp buffer
    size_t ret = storage_file_write(fd, temp_buf, len);
    free(temp_buf);
    return ret;
}

static size_t fs_seek(void* fd, size_t offset) {
    return storage_file_seek(fd, offset, true);
}

static bool fs_rename(const char* from, const char* to) {
    (void)from, (void)to;
    FURI_LOG_W(TAG, "TODO: %s", __func__);
    return false;
}

static bool fs_remove(const char* path) {
    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    FS_Error error = storage_common_remove(fs_api, path);
    furi_record_close(RECORD_STORAGE);
    return (error == FSE_OK);
}

static bool fs_mkdir(const char* path) {
    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    FS_Error error = storage_common_mkdir(fs_api, path);
    furi_record_close(RECORD_STORAGE);
    return (error == FSE_OK);
}

static const struct mg_fs mg_fs_flipper = {
    .st = fs_stat,
    .ls = fs_list,
    .op = fs_open,
    .cl = fs_close,
    .rd = fs_read,
    .wr = fs_write,
    .sk = fs_seek,
    .mv = fs_rename,
    .rm = fs_remove,
    .mkd = fs_mkdir,
};

const struct mg_fs* http_fs_get(void) {
    return &mg_fs_flipper;
}
