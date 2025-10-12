#pragma once
#include <furi.h>

typedef struct FetchLoader FetchLoader;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    size_t total_download_size;
    size_t received_download_size;
    uint32_t speed_bytes_per_sec;
} FetchLoaderStatus;

typedef void (*FetchLoaderCallbackStatus)(FetchLoaderStatus status, void* context);
typedef void (*FetchLoaderCallbackState)(FuriString* state, void* context);
typedef void (*FetchLoaderCallbackDone)(void* context);

FetchLoader* fetch_loader_alloc(void);
void fetch_loader_free(FetchLoader* instance);
void fetch_loader_forced_done(FetchLoader* instance);
void fetch_loader_run(FetchLoader* instance, const char* url, const char* path);
bool fetch_loader_is_processing_done(FetchLoader* instance);
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
