#pragma once

#include <furi.h>

typedef struct FetchFileSave FetchFileSave;

/*
* Allocates a FetchFileSave instance.
* @param[in] file_path Pointer to the FuriString containing the file path to save
* @returns Pointer to the allocated FetchFileSave instance, or NULL on failure.
*/
FetchFileSave* fetch_file_save_alloc(FuriString* file_path);

/*
* Free FetchFileSave instance.
* @param[in] instance Pointer to the FetchFileSave instance to free.
*/
void fetch_file_save_free(FetchFileSave* instance);

/*
* Write chunk data to the file.
* @param[in] instance Pointer to the FetchFileSave instance.
* @param[in] data Pointer to the data to write.
* @param[in] size Size of the data to write.
* @returns true on success, false on failure.
*/
bool fetch_file_save_write(FetchFileSave* instance, uint8_t* data, size_t size);

/*
* Close the file associated with the FetchFileSave instance.
* @param[in] instance Pointer to the FetchFileSave instance.
*/
void fetch_file_save_close(FetchFileSave* instance);

/*
* Remove the file associated with the FetchFileSave instance.
* @param[in] instance Pointer to the FetchFileSave instance.
*/
void fetch_file_save_remove(FetchFileSave* instance);
