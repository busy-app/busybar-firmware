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

typedef void (*FetchLoaderCallbackStatus)(const FetchStatus* status, void* context);
typedef void (*FetchLoaderCallbackState)(const char* state, void* context);
typedef void (*FetchLoaderCallbackDone)(FetchLoaderDoneStatus done_status, void* context);

FetchLoader* fetch_loader_alloc(void);

void fetch_loader_free(FetchLoader* instance);

void fetch_loader_start(FetchLoader* instance, const char* url, const char* path);

void fetch_loader_stop(FetchLoader* instance);

void fetch_loader_set_status_callback(
    FetchLoader* instance,
    FetchLoaderCallbackStatus callback,
    void* context);

void fetch_loader_set_state_callback(
    FetchLoader* instance,
    FetchLoaderCallbackState callback,
    void* context);

void fetch_loader_set_done_callback(
    FetchLoader* instance,
    FetchLoaderCallbackDone callback,
    void* context);

#ifdef __cplusplus
}
#endif
