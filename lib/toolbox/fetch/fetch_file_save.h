/**
 * @file fetch_file_save.h
 */
#pragma once

#include <furi.h>

typedef struct FetchFileSave FetchFileSave;

typedef enum {
    FetchFileSaveFlagNone = 0,
    FetchFileSaveFlagNonblocking = 1 << 0,
} FetchFileSaveFlag;

/**
* Allocates a FetchFileSave instance.
* @returns Pointer to the allocated FetchFileSave instance
*/
FetchFileSave* fetch_file_save_alloc(void);

/**
* Free FetchFileSave instance.
* @param[in] instance Pointer to the FetchFileSave instance to free.
*/
void fetch_file_save_free(FetchFileSave* instance);

/**
 * Open the file for saving.
 * @param[in,out] instance Pointer to the FetchFileSave instance to open.
 * @returns @c true if the file could be opened, @c false otherwise
 */
bool fetch_file_save_open(FetchFileSave* instance, FetchFileSaveFlag flags, const char* file_path);

/**
* Write chunk data to the file.
* @param[in] instance Pointer to the FetchFileSave instance.
* @param[in] data Pointer to the data to write.
* @param[in] size Size of the data to write.
* @returns true on success, false on failure.
*/
bool fetch_file_save_write(FetchFileSave* instance, const void* data, size_t size);

/**
* Remove the file associated with the FetchFileSave instance.
* @param[in] instance Pointer to the FetchFileSave instance.
*/
void fetch_file_save_remove(FetchFileSave* instance);
