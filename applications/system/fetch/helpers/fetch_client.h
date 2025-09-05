#pragma once

#include <furi.h>

typedef struct FetchClient FetchClient;

typedef void (*FetchClientCallbackRawData)(uint8_t* data, size_t data_size, void* context);
typedef void (*FetchClientCallbackHeader)(uint8_t* data, size_t data_size, void* context);
typedef void (*FetchClientCallbackError)(const char* error, void* context);

FetchClient* fetch_client_alloc(FuriString* url);
void fetch_client_free(FetchClient* instance);
void fetch_client_run(FetchClient* instance);
bool fetch_client_is_done(FetchClient* instance);
void fetch_client_set_callback_raw_data(FetchClient* instance, FetchClientCallbackRawData callback, void* context);
void fetch_client_set_callback_header(FetchClient* instance, FetchClientCallbackHeader callback, void* context);
void fetch_client_set_callback_error(FetchClient* instance, FetchClientCallbackError callback, void* context);
