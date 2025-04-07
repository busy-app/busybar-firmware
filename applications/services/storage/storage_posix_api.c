#include "storage_posix_api.h"

#include <furi.h>
#include <storage/storage.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <m-bptree.h>

#define TAG "StoragePosix"

#define FD_TREE_RANK (4)

BPTREE_DEF2(FileTree, FD_TREE_RANK, uint16_t, M_DEFAULT_OPLIST, File*, M_PTR_OPLIST) // NOLINT

typedef struct {
    Storage* storage;
    FileTree_t files;
} StoragePosix;

static StoragePosix* instance;

static inline uint16_t file_get_free_fd(void) {
    furi_check(FileTree_size(instance->files) < UINT16_MAX);

    uint16_t ret = 0;

    FileTree_it_ct it;
    for(FileTree_it(it, instance->files); !FileTree_end_p(it); FileTree_next(it)) {
        if(ret != *FileTree_cref(it)->key_ptr) {
            break;
        }
        ret += 1;
    }

    return ret;
}

static inline int file_add(File* file) {
    furi_assert(instance);

    const uint16_t fd = file_get_free_fd();
    FileTree_set_at(instance->files, fd, file);

    return fd;
}

static inline File* file_get(int fd) {
    furi_assert(instance);
    return *FileTree_get(instance->files, fd);
}

static inline void file_remove(int fd) {
    furi_assert(instance);
    furi_check(FileTree_erase(instance->files, fd));
}

extern int __wrap_fflush(FILE* stream);
extern int __wrap_vsnprintf(char* str, size_t size, const char* format, va_list args);

int __wrap_sniprintf(char* str, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = __wrap_vsnprintf(str, size, format, args);
    va_end(args);

    return ret;
}

int __wrap__fflush_r(struct _reent* reent, FILE* stream) {
    UNUSED(reent);
    return __wrap_fflush(stream);
}

int _isatty(int fd) {
    UNUSED(fd);
    return 0;
}

int _open(const char* filename, int oflag) {
    furi_assert(instance);
    File* file = storage_file_alloc(instance->storage);

    FS_AccessMode access_mode = FSAM_READ;
    if(oflag & O_WRONLY) {
        access_mode = FSAM_WRITE;
    } else if(oflag & O_RDWR) {
        access_mode = FSAM_READ_WRITE;
    }

    FS_OpenMode open_mode = FSOM_OPEN_EXISTING;
    if(oflag & O_CREAT) {
        open_mode = FSOM_CREATE_ALWAYS;
    } else if(oflag & O_TRUNC) {
        open_mode = FSOM_CREATE_NEW;
    } else if(oflag & O_APPEND) {
        open_mode = FSOM_OPEN_APPEND;
    }

    bool result = storage_file_open(file, filename, access_mode, open_mode);

    if(!result) {
        FURI_LOG_T(
            TAG,
            "Failed to open file %s, flags in %d, access_mode %d, open_mode %d, error %d",
            filename,
            oflag,
            access_mode,
            open_mode,
            storage_file_get_error(file));
        storage_file_free(file);
        return -1;
    }

    int fd = file_add(file);

    FURI_LOG_T(TAG, "Opened file %s, file %p", filename, file);
    return fd;
}

int _close(int fd) {
    File* file = file_get(fd);
    FURI_LOG_T(TAG, "close %p", file);
    storage_file_close(file);
    storage_file_free(file);
    file_remove(fd);
    return 0;
}

int _fstat(int fd, struct stat* buf) {
    File* file = file_get(fd);
    FURI_LOG_T(TAG, "fstat %p", file);

    UNUSED(fd);
    buf->st_mode = S_IFCHR;
    return 0;
}

int _lseek(int fd, int pos, int whence) {
    File* file_p = file_get(fd);
    FURI_LOG_T(TAG, "lseek %p, pos %d, whence %d", file_p, pos, whence);

    int seek_position = 0;
    int current_position = storage_file_tell(file_p);
    int size = storage_file_size(file_p);

    if(whence == SEEK_SET) {
        seek_position = pos;
    } else if(whence == SEEK_CUR) {
        seek_position = current_position + pos;
    } else if(whence == SEEK_END) {
        if(pos > size) {
            seek_position = 0;
        } else {
            seek_position = size - pos;
        }
    }

    bool storage_res = storage_file_seek(file_p, seek_position, true);

    return storage_res ? seek_position : -1;
}

ssize_t _read(int fd, void* buf, size_t count) {
    File* file = file_get(fd);
    FURI_LOG_T(TAG, "read %p, count %d", file, count);

    return storage_file_read(file, buf, count);
}

ssize_t _write(int fd, const void* buf, size_t count) {
    File* file = file_get(fd);
    FURI_LOG_T(TAG, "write %p, count %d", file, count);

    return storage_file_write(file, buf, count);
}

void storage_posix_api_init(Storage* storage) {
    furi_assert(instance == NULL);

    instance = malloc(sizeof(StoragePosix));
    instance->storage = storage;
    FileTree_init(instance->files);
}
