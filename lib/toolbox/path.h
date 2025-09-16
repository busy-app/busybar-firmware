#pragma once
#include <furi.h>
#include <storage/storage.h>
#include <storage/filesystem_api_defines.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Extract filename without extension from path.
 * 
 * @param path path string
 * @param filename output filename string. Must be initialized before.
 */
void path_extract_filename_no_ext(const char* path, FuriString* filename);

/**
 * @brief Extract filename string from path.
 * 
 * @param path path string
 * @param filename output filename string. Must be initialized before.
 * @param trim_ext true - get filename without extension
 */
void path_extract_filename(FuriString* path, FuriString* filename, bool trim_ext);

/**
 * @brief Extract file extension from path.
 * 
 * @param path path string
 * @param ext output extension string
 * @param ext_len_max maximum extension string length
 */
void path_extract_extension(FuriString* path, char* ext, size_t ext_len_max);

/**
 * @brief Extract last path component
 * 
 * @param path path string
 * @param filename output string. Must be initialized before.
 */
void path_extract_basename(const char* path, FuriString* basename);

/**
 * @brief Extract path, except for last component
 * 
 * @param path path string
 * @param filename output string. Must be initialized before.
 */
void path_extract_dirname(const char* path, FuriString* dirname);

/**
 * @brief Appends new component to path, adding path delimiter
 * 
 * @param path path string
 * @param suffix path part to apply
 */
void path_append(FuriString* path, const char* suffix);

/**
 * @brief Appends new component to path, adding path delimiter
 * 
 * @param path first path part
 * @param suffix second path part
 * @param out_path output string to combine parts into. Must be initialized
 */
void path_concat(const char* path, const char* suffix, FuriString* out_path);

/**
 * @brief Check that path contains only ascii characters
 * 
 * @param path 
 * @return true 
 * @return false 
 */
bool path_contains_only_ascii(const char* path);

/**
 * @brief Recursively create directories for the given path.
 * 
 * If any part of the path already exists, it will be skipped.
 * 
 * @param storage pointer to a storage API instance.
 * @param path pointer to a zero-terminated string containing the path in question.
 * @return FSE_OK if the directories were successfully created or already exist, any other error code on failure.
 */
FS_Error path_recursive_create_dir(Storage* storage, FuriString* path);

#ifdef __cplusplus
}
#endif
