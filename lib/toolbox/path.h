/**
 * @file path.h
 * @brief Filesystem path utilities.
 */
#pragma once

#include <core/string.h>

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
 * @param basename output string. Must be initialized before.
 */
void path_extract_basename(const char* path, FuriString* basename);

/**
 * @brief Extract path, except for last component
 * 
 * @param path path string
 * @param dirname output string. Must be initialized before.
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
 * @returns @c true if the paths contains only ASCII characters, @c false otherwise
 */
bool path_contains_only_ascii(const char* path);

/**
 * @brief Normalizes path (resolves current and parent directory references).
 *
 * @param path path to normalize
 * @param out_path output string to normalize path into. Must be initialized
 * @param allow_escape_root if true, resulting paths can reference parents of current directory.
 *
 * @code
 * path_normalize("a/../../b", out_path, true); // produces "../b"
 * path_normalize("a/../../b", out_path, false); // produces "b"
 * @endcode
 */
void path_normalize(const char* path, FuriString* out_path, bool allow_escape_root);
#ifdef __cplusplus
}
#endif
