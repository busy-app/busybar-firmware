/**
 * @file log_storage.h
 * @brief Log capture service API.
 */
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The string key for the LogStorage instance access.
 *
 * Get the instance pointer by calling `furi_record_open(RECORD_LOG_STORAGE)`.
 */
#define RECORD_LOG_STORAGE "log_storage"

/**
 * @brief Opaque log storage instance.
 *
 * Obtain it via `furi_record_open(RECORD_LOG_STORAGE)`.
 */
typedef struct LogStorage LogStorage;

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

/**
 * @brief Temporarily release the remote log UART.
 *
 * Releases the serial line used to receive remote logs so that another consumer
 * can acquire it. Reception is paused until log_storage_resume_remote is called.
 *
 * @param[in] instance pointer to the LogStorage instance
 */
void log_storage_suspend_remote(LogStorage* instance);

/**
 * @brief Resume remote log reception after log_storage_suspend_remote.
 *
 * @param[in] instance pointer to the LogStorage instance
 */
void log_storage_resume_remote(LogStorage* instance);

#ifdef __cplusplus
}
#endif
