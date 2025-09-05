#pragma once

#include <furi.h>

typedef struct FetchClient FetchClient;

typedef enum {
    FetchClientEventNone = 0,
    FetchClientEventRawData,
    FetchClientEventProgress,
    FetchClientEventDone,
} FetchClientEvent;

typedef struct {
    union {
        struct {
            uint8_t* data;
            size_t size;
        } raw;
        struct {
            size_t total_file_size;
            size_t received_file_size;
        } progress;
    };
} FetchClientData;

typedef void (*FetchClientCallback)(FetchClientEvent event, FetchClientData* data, void* context);

FetchClient* fetch_client_alloc(FuriString* url, FuriString* file_path);
void fetch_client_free(FetchClient* instance);
void fetch_client_run(FetchClient* instance);
bool fetch_client_is_done(FetchClient* instance);
void fetch_client_set_callback(FetchClient* instance, FetchClientCallback callback, void* context);
