/**
 * @file log_storage_u5.h
 * @brief Log capture service — U5 dump API.
 */
#pragma once

#include "common/log_storage.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Default file path used when log_storage_dump() is called with NULL.
 */
#define LOG_STORAGE_DUMP_DEFAULT_FILE_PATH "/ext/log.txt"

/**
 * @brief Snapshot the captured log buffer into a file on the /ext partition.
 *
 * The complete log lines held in the in-memory circular buffer are written to
 * @p path. Partial lines are never emitted: a leading fragment left by the
 * buffer wrap-around and a trailing in-progress line are both discarded, so
 * the file contains only whole log lines.
 *
 * @note The in-memory buffer is left untouched and keeps accumulating logs
 *       after the call (the snapshot is a copy, not a move).
 *
 * @param[in] instance pointer to the LogStorage instance
 * @param[in] path     destination file path, or NULL to use @ref LOG_STORAGE_DUMP_DEFAULT_FILE_PATH
 *
 * @returns true if the file was written successfully, false otherwise
 */
bool log_storage_dump(LogStorage* instance, const char* path);

#ifdef __cplusplus
}
#endif
