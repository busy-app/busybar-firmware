/**
 * @file fetch_common.h
 * @brief Common definitions for Fetch and files that use it.
 */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Fetch download progress structure.
 *
 * @note @p total_download_size is valid ONLY if @p has_total_download_size is @c true.
 */
typedef struct {
    size_t total_download_size; /**< Total file size, in bytes */
    size_t received_download_size; /**< Number of bytes downloaded so far */
    uint32_t speed_bytes_per_sec; /**< Donwload speed, in bytes per second */
    bool has_total_download_size; /**< Whether the total download size is known */
} FetchProgress;

/**
 * @brief Enumeration of possible Fetch completion statuses.
 */
typedef enum {
    FetchStatusOk, /**< Operation completed successfully */
    FetchStatusError, /**< Operation failed due to an error */
    FetchStatusAborted, /**< Operation was aborted (cancelled) */
} FetchStatus;

#ifdef __cplusplus
}
#endif
