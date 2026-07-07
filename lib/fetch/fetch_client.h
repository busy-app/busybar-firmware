/**
 * @file fetch_client.h
 */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct FetchClient FetchClient;

typedef struct {
    const char* url;
    const char* method;
    const char** headers;
    uint32_t headers_count;
    const char* body;
} FetchClientRequest;

typedef struct {
    size_t total_download_size;
    size_t received_download_size;
    uint32_t speed_bytes_per_sec;
} FetchClientStatus;

typedef void (*FetchClientCallbackRawData)(uint8_t* data, size_t data_size, void* context);
typedef void (*FetchClientCallbackHeader)(uint8_t* data, size_t data_size, void* context);
typedef void (*FetchClientCallbackError)(const char* error, void* context);
typedef void (*FetchClientCallbackStatus)(FetchClientStatus status, void* context);
typedef void (*FetchClientCallbackFinished)(void* context);

/**
* Allocates a FetchClient instance.
* @returns Pointer to the allocated FetchClient instance.
*/
FetchClient* fetch_client_alloc(void);

/**
* Free FetchClient instance. The instance must not be processing.
* @param[in] instance Pointer to the FetchClient instance to free.
*/
void fetch_client_free(FetchClient* instance);

/**
* Starts the fetch process with the given URL.
*
* @warning The data pointed to by the @p request parameter
*          must NOT be deleted until the FetchClient instance finishes.
*
* @param[in] instance Pointer to the FetchClient instance.
* @param[in] request Pointer to the request to be executed.
*/
void fetch_client_start(FetchClient* instance, const FetchClientRequest* request);

/**
* Stop the ongoing request and mark it as done.
* @param[in] instance Pointer to the FetchClient instance.
*/
void fetch_client_stop(FetchClient* instance);

/**
* Checks if the fetch process is done.
* @param[in] instance Pointer to the FetchClient instance.
* @return true if the fetch process is done, false otherwise.
*/
bool fetch_client_is_finished(FetchClient* instance);

/**
* Sets the context for the FetchClient instance.
* @param[in] instance Pointer to the FetchClient instance.
* @param[in] context Pointer to the context to set.
*/
void fetch_client_set_context(FetchClient* instance, void* context);

/**
* Sets the callback for raw data received.
* @param[in] instance Pointer to the FetchClient instance.
* @param[in] callback Function pointer to the callback function.
*/
void fetch_client_set_callback_raw_data(FetchClient* instance, FetchClientCallbackRawData callback);

/**
* Sets the callback for header data received.
* @param[in] instance Pointer to the FetchClient instance.
* @param[in] callback Function pointer to the callback function.
*/
void fetch_client_set_callback_header(FetchClient* instance, FetchClientCallbackHeader callback);

/**
* Sets the callback for error data received.
* @param[in] instance Pointer to the FetchClient instance.
* @param[in] callback Function pointer to the callback function.
*/
void fetch_client_set_callback_error(FetchClient* instance, FetchClientCallbackError callback);

/**
* Sets the callback for status updates.
* @param[in] instance Pointer to the FetchClient instance.
* @param[in] callback Function pointer to the callback function.
*/
void fetch_client_set_callback_status(FetchClient* instance, FetchClientCallbackStatus callback);

/**
* Sets the callback for finish notification.
* @param[in] instance Pointer to the FetchClient instance.
* @param[in] callback Function pointer to the callback function.
*/
void fetch_client_set_callback_finished(
    FetchClient* instance,
    FetchClientCallbackFinished callback);
