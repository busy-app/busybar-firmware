/**
 * @file temp_file.h
 * @brief Library for easy creation and management of temporary files.
 */
#pragma once

#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque TempFile structure declaration.
 */
typedef struct TempFile TempFile;

/**
 * @brief Create a TempFile instance.
 *
 * This function does not create any files, only allocates a container
 * that can later be used to create a file via temp_file_create().
 *
 * @param[in] storage pointer to the Storage instance.
 * @returns pointer to the created instance
 */
TempFile* temp_file_alloc(Storage* storage);

/**
 * @brief Delete a TempFile instance.
 *
 * This function does not remove any files it might have created.
 *
 * @param[in,out] instance pointer to the instance to be deleted
 */
void temp_file_free(TempFile* instance);

/**
 * @brief Create an underlying file and open it for writing.
 *
 * This function will create necessary paths if they do not exist.
 *
 * @warning This function will silently overwrite conflicting files.
 *
 * @param[in,out] instance pointer to the instance to be used
 * @param[in] path full path to the file to be created
 * @returns @c true if the file could be created, @c false otherwise
 */
bool temp_file_create(TempFile* instance, const char* path);

/**
 * @brief Write data to the underlying file.
 *
 * @param[in,out] instance pointer to the instance to be written to
 * @param[in] data pointer to arbitrary data to be written to the file
 * @param[in] data_len length of the data to be written
 * @returns @c true if the data was written successfully, @c false otherwise
 */
bool temp_file_write(TempFile* instance, const void* data, size_t data_len);

/**
 * @brief Remove the underlying file.
 *
 * @param[in,out] instance pointer to the instance to be removed.
 * @returns @c true if the file could be removed, @c false otherwise
 */
bool temp_file_remove(TempFile* instance);

#ifdef __cplusplus
}
#endif
