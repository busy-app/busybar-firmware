/**
 * @file fetch_common.h
 */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    size_t total_download_size;
    size_t received_download_size;
    uint32_t speed_bytes_per_sec;
} FetchStatus;

#ifdef __cplusplus
}
#endif
