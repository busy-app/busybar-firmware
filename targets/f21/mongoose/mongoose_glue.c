#include "mongoose_glue.h"
#include <furi.h>
#include <furi_hal_random.h>
#include <storage/storage.h>

#define TAG              "MongooseGlue"
#define MAX_FILENAME_LEN 255

// Uncomment to enable verbose per-call logging for the FS glue layer
// (fs_stat, fs_open, fs_close, fs_read, fs_write, fs_seek).
// Normally silent to avoid flooding the log on every HTTP static-file request.
// #define MONGOOSE_FS_VERBOSE_LOG

#ifdef MONGOOSE_FS_VERBOSE_LOG
#define MG_FS_LOG_V(...) FURI_LOG_T(TAG, __VA_ARGS__)
#else
#define MG_FS_LOG_V(...)
#endif

uint64_t mg_millis(void) {
    return furi_get_tick();
}

void mg_log_prefix(int level, const char* file, int line, const char* fname) {
    UNUSED(file);
    FuriString* string = furi_string_alloc();

    const char* color = _FURI_LOG_CLR_RESET;
    const char* log_letter = " ";
    switch(level) {
    case MG_LL_ERROR:
        color = _FURI_LOG_CLR_E;
        log_letter = "E";
        break;
    case MG_LL_INFO:
        color = _FURI_LOG_CLR_I;
        log_letter = "I";
        break;
    case MG_LL_DEBUG:
        color = _FURI_LOG_CLR_D;
        log_letter = "D";
        break;
    case MG_LL_VERBOSE:
        color = _FURI_LOG_CLR_T;
        log_letter = "T";
        break;
    default:
        break;
    }

    furi_string_printf(
        string,
        "%lu %s[%s][%s] " _FURI_LOG_CLR_RESET,
        furi_get_tick(),
        color,
        log_letter,
        "Mongoose");

    furi_string_cat_printf(string, "%s:%u ", fname, line);
    furi_log_puts(furi_string_get_cstr(string));

    furi_string_free(string);
}

void mg_log(const char* fmt, ...) {
    FuriString* string = furi_string_alloc();

    va_list args;
    va_start(args, fmt);
    furi_string_vprintf(string, fmt, args);
    va_end(args);

    furi_string_cat_str(string, "\r\n");
    furi_log_puts(furi_string_get_cstr(string));
    furi_string_free(string);
}

int _gettimeofday(struct timeval* tv, void* tz) {
    uint64_t now = mg_now();
    (void)tz;
    tv->tv_sec = (time_t)(now / 1000);
    tv->tv_usec = (unsigned long)((now % 1000) * 1000);
    return 0;
}

bool mg_random(void* buf, size_t len) {
    furi_hal_random_fill_buf(buf, len);
    return true;
}

static int fs_stat(const char* path, size_t* size, time_t* mtime) {
    MG_FS_LOG_V("fs_stat: %s", path);
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
    MG_FS_LOG_V("fs_list: %s", path);
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
    MG_FS_LOG_V("fs_open: %s (flags: 0x%x)", path, flags);
    if(flags & MG_FS_DIR) {
        FURI_LOG_W(TAG, "TODO: %s MG_FS_DIR", __func__);
        return NULL;
    }

    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(fs_api);

    FS_AccessMode access = (flags & MG_FS_WRITE) ? FSAM_READ_WRITE : FSAM_READ;
    FS_OpenMode open_mode = (flags & MG_FS_WRITE) ? FSOM_OPEN_APPEND : FSOM_OPEN_EXISTING;
    if(flags & MG_FS_WRITE) {
        open_mode |= FSOM_NONBLOCKING;
    }

    if(!storage_file_open(file, path, access, open_mode)) {
        storage_file_close(file);
        storage_file_free(file);
        file = NULL;
    }

    furi_record_close(RECORD_STORAGE);

    return file;
}

static void fs_close(void* fp) {
    MG_FS_LOG_V("fs_close: fd=%p", fp);
    storage_file_close(fp);
    storage_file_free(fp);
}

static size_t fs_read(void* fd, void* buf, size_t len) {
    size_t result = storage_file_read(fd, buf, len);
    MG_FS_LOG_V("fs_read: fd=%p, len=%zu, result=%zu", fd, len, result);
    return result;
}

static size_t fs_write(void* fd, const void* buf, size_t len) {
    size_t ret = storage_file_write(fd, buf, len);
    MG_FS_LOG_V("fs_write: fd=%p, len=%zu, result=%zu", fd, len, ret);
    return ret;
}

static size_t fs_seek(void* fd, size_t offset) {
    size_t result = storage_file_seek(fd, offset, true);
    MG_FS_LOG_V("fs_seek: fd=%p, offset=%zu, result=%zu", fd, offset, result);
    return result;
}

static bool fs_rename(const char* from, const char* to) {
    FURI_LOG_D(TAG, "fs_rename: %s -> %s", from, to);
    (void)from, (void)to;
    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    FS_Error error = storage_common_rename(fs_api, from, to);
    furi_record_close(RECORD_STORAGE);
    return (error == FSE_OK);
}

static bool fs_remove(const char* path) {
    FURI_LOG_D(TAG, "fs_remove: %s", path);
    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    const bool error = storage_simply_remove(fs_api, path);
    furi_record_close(RECORD_STORAGE);
    return error;
}

static bool fs_mkdir(const char* path) {
    FURI_LOG_D(TAG, "fs_mkdir: %s", path);
    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    const bool error = storage_simply_mkpath(fs_api, path);
    furi_record_close(RECORD_STORAGE);
    return error;
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

struct mg_fs* http_fs_get(void) {
    return (struct mg_fs*)&mg_fs_flipper;
}

void mg_init_early(void) {
    mg_log_set(MG_LOG_LEVEL);
}
