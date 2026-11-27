/**
 * @file fetch.h
 * @brief Library for building fetch-like HTTP clients.
 */
#pragma once

#include "fetch_common.h"
#include "fetch_request.h"

#define RECORD_FETCH "fetch"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Fetch service
 */
typedef struct Fetch Fetch;

/**
 * @brief A single fetch request handle
 */
typedef struct FetchHandle FetchHandle;

/**
 * @brief Received HTTP data callback function type.
 *
 * @param[in] data pointer to received data
 * @param[in] data_size length of the received data
 * @param[in] context pointer to a user-specific object (may be @c NULL)
 */
typedef void (*FetchRxDataCallback)(const void* data, size_t data_size, void* context);

/**
 * @brief Received HTTP headers callback function type.
 *
 * @param[in] data pointer to header data
 * @param[in] data_size length of the header data
 * @param[in] context pointer to a user-specific object (may be @c NULL)
 */
typedef void (*FetchHeaderCallback)(const void* data, size_t data_size, void* context);

/**
 * @brief Progress reporting callback function type.
 *
 * @param[in] progress pointer to a structure containing the current progress information
 * @param[in] context pointer to a user-specific object (may be @c NULL)
 */
typedef void (*FetchProgressCallback)(const FetchProgress* progress, void* context);

/**
 * @brief Error reporting callback function type.
 *
 * @param[in] error pointer to a zero-terminated string containing the error description
 * @param[in] context pointer to a user-specific object (may be @c NULL)
 */
typedef void (*FetchErrorCallback)(const char* error, void* context);

/**
 * @brief Fetch done callback function type.
 *
 * @param[in] handle handle of the fetch process
 * @param[in] context pointer to a user-specific object (may be @c NULL)
 */
typedef void (*FetchDoneCallback)(FetchHandle* handle, void* context);

/**
 * @brief Callbacks description
 */
typedef struct FetchCallbacks {
    FetchRxDataCallback rx_data_callback;
    FetchHeaderCallback header_callback;
    FetchErrorCallback error_callback;
    FetchProgressCallback progress_callback;
    FetchDoneCallback done_callback;
    void* callback_context;
} FetchCallbacks;

/**
 * @brief Run the fetch process.
 *
 * The callbacks will be executed in the fetch service thread's context.
 *
 * @param[in] instance fetch service instance.
 * @param[in] request Pointer to the request to be executed. Must be valid through the entire execution time (until fetch_join returns).
 * @param[in] callbacks Pointer to the callbacks description. Unused callbacks can be set to NULL.
 * @returns fetch handle on success, NULL if resources could not be allocated.
 */
FetchHandle*
    fetch_run(Fetch* instance, const FetchRequest* request, const FetchCallbacks* callbacks);

/**
 * @brief Wait until fetch request execution stops and free the handle.
 *
 * @param handle fetch handle
 * @param timeout operation timeout in ticks.
 *
 * @return operation result. If FetchStatusTimeout is returned, handle remains valid.
 */
FetchStatus fetch_join(FetchHandle* handle, uint32_t timeout);

/**
 * @brief Request the ongoing fetch process to stop.
 *
 * This function is non-blocking. The execution thread must
 * wait until `fetch_exec()` returns.
 *
 * @param handle fetch handle.
 */
void fetch_stop(FetchHandle* handle);

#ifdef __cplusplus
}
#endif
