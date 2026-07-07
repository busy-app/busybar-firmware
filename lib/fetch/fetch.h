/**
 * @file fetch.h
 */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define FETCH_HEADERS_COUNT_MAX (10)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Fetch Fetch;

typedef struct {
    const char* data[FETCH_HEADERS_COUNT_MAX];
    uint32_t count;
} FetchRequestHeaders;

typedef struct {
    const void* data;
    uint32_t length;
} FetchRequestBody;

typedef struct {
    const char* url;
    const char* method;
    FetchRequestHeaders headers;
    FetchRequestBody body;
} FetchRequest;

typedef struct {
    size_t total_download_size;
    size_t received_download_size;
    uint32_t speed_bytes_per_sec;
} FetchStatus;

typedef void (*FetchCallbackRawData)(uint8_t* data, size_t data_size, void* context);
typedef void (*FetchCallbackHeader)(uint8_t* data, size_t data_size, void* context);
typedef void (*FetchCallbackError)(const char* error, void* context);
typedef void (*FetchCallbackStatus)(FetchStatus status, void* context);
typedef void (*FetchCallbackFinished)(void* context);

/**
* Allocates a Fetch instance.
* @returns Pointer to the allocated Fetch instance.
*/
Fetch* fetch_alloc(void);

/**
* Free Fetch instance. The instance must not be processing.
* @param[in] instance Pointer to the Fetch instance to free.
*/
void fetch_free(Fetch* instance);

/**
* Starts the fetch process with the given URL.
*
* @warning The data pointed to by the @p request parameter and its members
*          must stay valid until the Fetch instance finishes.
*
* @param[in] instance Pointer to the Fetch instance.
* @param[in] request Pointer to the request to be executed.
*/
void fetch_start(Fetch* instance, const FetchRequest* request);

/**
* Stop the ongoing request and mark it as done.
* @param[in] instance Pointer to the Fetch instance.
*/
void fetch_stop(Fetch* instance);

/**
* Checks if the fetch process is done.
* @param[in] instance Pointer to the Fetch instance.
* @return true if the fetch process is done, false otherwise.
*/
bool fetch_is_finished(Fetch* instance);

/**
* Sets the context for the Fetch instance.
* @param[in] instance Pointer to the Fetch instance.
* @param[in] context Pointer to the context to set.
*/
void fetch_set_callback_context(Fetch* instance, void* context);

/**
* Sets the callback for raw data received.
* @param[in] instance Pointer to the Fetch instance.
* @param[in] callback Function pointer to the callback function.
*/
void fetch_set_callback_raw_data(Fetch* instance, FetchCallbackRawData callback);

/**
* Sets the callback for header data received.
* @param[in] instance Pointer to the Fetch instance.
* @param[in] callback Function pointer to the callback function.
*/
void fetch_set_callback_header(Fetch* instance, FetchCallbackHeader callback);

/**
* Sets the callback for error data received.
* @param[in] instance Pointer to the Fetch instance.
* @param[in] callback Function pointer to the callback function.
*/
void fetch_set_callback_error(Fetch* instance, FetchCallbackError callback);

/**
* Sets the callback for status updates.
* @param[in] instance Pointer to the Fetch instance.
* @param[in] callback Function pointer to the callback function.
*/
void fetch_set_callback_status(Fetch* instance, FetchCallbackStatus callback);

/**
* Sets the callback for finish notification.
* @param[in] instance Pointer to the Fetch instance.
* @param[in] callback Function pointer to the callback function.
*/
void fetch_set_callback_finished(
    Fetch* instance,
    FetchCallbackFinished callback);

#ifdef __cplusplus
}
#endif
