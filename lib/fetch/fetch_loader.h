/**
 * @file fetch_loader.h
 */
#pragma once

#include "fetch_common.h"

typedef struct FetchLoader FetchLoader;

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FetchLoaderDoneStatusSuccess,
    FetchLoaderDoneStatusFailure,
    FetchLoaderDoneStatusAbort,
} FetchLoaderDoneStatus;

typedef void (*FetchLoaderProgressCallback)(const FetchProgress* progress, void* context);
typedef void (*FetchLoaderStateCallback)(const char* error, void* context);
typedef void (*FetchLoaderDoneCallback)(FetchLoaderDoneStatus done_status, void* context);

FetchLoader* fetch_loader_alloc(void);

void fetch_loader_free(FetchLoader* instance);

void fetch_loader_start(FetchLoader* instance, const char* remote_url, const char* file_path);

void fetch_loader_stop(FetchLoader* instance);

void fetch_loader_set_callback_context(FetchLoader* instance, void* context);

void fetch_loader_set_progress_callback(
    FetchLoader* instance,
    FetchLoaderProgressCallback callback);

void fetch_loader_set_state_callback(FetchLoader* instance, FetchLoaderStateCallback callback);

void fetch_loader_set_done_callback(FetchLoader* instance, FetchLoaderDoneCallback callback);

#ifdef __cplusplus
}
#endif
