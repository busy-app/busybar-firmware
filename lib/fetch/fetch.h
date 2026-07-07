/**
 * @file fetch.h
 */
#pragma once

#include "fetch_common.h"

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

typedef void (*FetchCallbackRawData)(const void* data, size_t data_size, void* context);
typedef void (*FetchCallbackHeader)(const void* data, size_t data_size, void* context);
typedef void (*FetchCallbackError)(const char* error, void* context);
typedef void (*FetchCallbackStatus)(const FetchStatus* status, void* context);

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
 * @brief Execute the fetch process in the current thread.
 *
 * This call will block the calling thread
 * until the fetch process is complete.
 *
 * The callbacks will be executed in the calling thread's context.
 *
 * @warning The caller must ensure that at least 8 KB of stack
 *          is available within the context.
 *
 * @param[in] instance Pointer to the Fetch instance.
 * @param[in] request Pointer to the request to be executed.
 */
void fetch_exec(Fetch* instance, const FetchRequest* request);

/**
* Stop the ongoing request and mark it as done.
* @param[in] instance Pointer to the Fetch instance.
*/
void fetch_stop(Fetch* instance);

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

#ifdef __cplusplus
}
#endif
