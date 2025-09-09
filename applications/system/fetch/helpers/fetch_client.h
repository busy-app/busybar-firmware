#pragma once

#include <furi.h>

typedef struct FetchClient FetchClient;

typedef struct {
    size_t total_download_size;
    size_t received_download_size;
    uint32_t speed_bytes_per_sec;
} FetchClientStatus;

typedef void (*FetchClientCallbackRawData)(uint8_t* data, size_t data_size, void* context);
typedef void (*FetchClientCallbackHeader)(uint8_t* data, size_t data_size, void* context);
typedef void (*FetchClientCallbackError)(const char* error, void* context);
typedef void (*FetchClientCallbackStatus)(FetchClientStatus status, void* context);

FetchClient* fetch_client_alloc(void);
void fetch_client_free(FetchClient* instance);
void fetch_client_run(FetchClient* instance, FuriString* url);
bool fetch_client_is_processing_done(FetchClient* instance);
void fetch_client_set_context(FetchClient* instance, void* context);
void fetch_client_set_callback_raw_data(FetchClient* instance, FetchClientCallbackRawData callback);
void fetch_client_set_callback_header(FetchClient* instance, FetchClientCallbackHeader callback);
void fetch_client_set_callback_error(FetchClient* instance, FetchClientCallbackError callback);
void fetch_client_set_callback_status(FetchClient* instance, FetchClientCallbackStatus callback);
