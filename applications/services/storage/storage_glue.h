#pragma once

#include <furi.h>
#include "filesystem_api_internal.h"
#include <m-list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ST_BKP = 0,
    ST_EXT = 1,
    ST_MAX
} StorageType;

typedef struct StorageData StorageData;

typedef struct {
    void (*tick)(StorageData* storage);
} StorageApi;

typedef struct {
    FuriString* path;
    uint8_t ref_count;
    FS_AccessMode access_mode;
} StorageFileShared;

typedef struct {
    File* file;
    void* file_data;
    StorageFileShared* shared;
} StorageFile;

typedef enum {
    StorageStatusOK, /**< storage ok */
    StorageStatusNotReady, /**< storage not ready (not initialized or waiting for data storage to appear) */
    StorageStatusNotMounted, /**< datastore appeared, but we cannot mount it */
    StorageStatusNoFS, /**< datastore appeared and mounted, but does not have a file system */
    StorageStatusNotAccessible, /**< datastore appeared and mounted, but not available */
    StorageStatusErrorInternal, /**< any other internal error */
} StorageStatus;

void storage_file_init(StorageFile* obj);
void storage_file_init_set(StorageFile* obj, const StorageFile* src);
void storage_file_set(StorageFile* obj, const StorageFile* src);
void storage_file_clear(StorageFile* obj);

void storage_data_init(StorageData* storage);
StorageStatus storage_data_status(StorageData* storage);
const char* storage_data_status_text(StorageData* storage);
void storage_data_timestamp(StorageData* storage);
uint32_t storage_data_get_timestamp(StorageData* storage);

LIST_DEF(
    StorageFileList,
    StorageFile,
    (INIT(API_2(storage_file_init)),
     SET(API_6(storage_file_init_set)),
     INIT_SET(API_6(storage_file_set)),
     CLEAR(API_2(storage_file_clear))))

struct StorageData {
    const FS_Api* fs_api;
    StorageApi api;
    void* data;
    StorageStatus status;
    StorageFileList_t files;
    uint32_t timestamp;
    bool read_only;
};

bool storage_has_file(const File* file, StorageData* storage_data);
bool storage_path_already_open(
    FuriString* path,
    FS_AccessMode access_mode,
    StorageData* storage_data);

void storage_set_storage_file_data(const File* file, void* file_data, StorageData* storage);
void* storage_get_storage_file_data(const File* file, StorageData* storage);

void storage_push_storage_file(
    File* file,
    FuriString* path,
    FS_AccessMode access_mode,
    StorageData* storage);
bool storage_pop_storage_file(File* file, StorageData* storage);

size_t storage_open_files_count(StorageData* storage);

void storage_set_read_only(StorageData* storage, bool read_only);
bool storage_is_read_only(StorageData* storage);

#ifdef __cplusplus
}
#endif
