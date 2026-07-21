/**
 * @file log_storage.h
 * @brief Log capture service — shared (U5 + 917) definitions.
 */
#pragma once

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

#ifdef __cplusplus
}
#endif
