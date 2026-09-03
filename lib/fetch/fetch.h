/**
 * @file fetch.h
 * @brief Library for building fetch-like HTTP clients.
 */
#pragma once

#include "fetch_common.h"

#include <toolbox/tls_config.h>

/** Maximum number of additional request headers. */
#define FETCH_HEADERS_COUNT_MAX (10)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque Fetch structure declaration.
 */
typedef struct Fetch Fetch;

/**
 * @brief Request headers structure.
 */
typedef struct {
    /**
     * @brief Array of pointers to individual header strings.
     *
     * Each header must be a zero-terminated string.
     * Maximum number of the headers is limited to @c FETCH_HEADERS_COUNT_MAX.
     */
    const char* data[FETCH_HEADERS_COUNT_MAX];
    uint32_t count; /**< Number of provided headers. */
} FetchRequestHeaders;

/**
 * @brief Request body structure.
 */
typedef struct {
    const void* data; /**< Pointer to the data to be sent. */
    uint32_t length; /**< Length of the data to be sent, in bytes. */
} FetchRequestBody;

/**
 * @brief Request structure.
 *
 * If @p method is @c NULL, a `GET` request is assumed.
 */
typedef struct {
    /**
      * @brief Pointer to the remote URL string.
      *
      * @note The string MUST begin with a valid protocol prefix,
      *       e.g. `http://` or `https://`.
      */
    const char* url;
    const char* method; /**< Pointer to the HTTP method string. */
    FetchRequestHeaders headers; /**< Request headers (optional). */
    FetchRequestBody body; /**< Request body (optional).*/
    TlsConfig tls_config; /**< TLS configuration (optional). */
} FetchRequest;

/**
 * @brief Received HTTP data callback function type.
 *
 * @param[in] data pointer to received data
 * @param[in] data_size length of the received data
 * @param[in,out] context pointer to a user-specific object (may be @c NULL)
 */
typedef void (*FetchRxDataCallback)(const void* data, size_t data_size, void* context);

/**
 * @brief Received HTTP headers callback function type.
 *
 * @param[in] data pointer to header data
 * @param[in] data_size length of the header data
 * @param[in,out] context pointer to a user-specific object (may be @c NULL)
 */
typedef void (*FetchHeaderCallback)(const void* data, size_t data_size, void* context);

/**
 * @brief Progress reporting callback function type.
 *
 * @param[in] progress pointer to a structure containing the current progress information
 * @param[in,out] context pointer to a user-specific object (may be @c NULL)
 */
typedef void (*FetchProgressCallback)(const FetchProgress* progress, void* context);

/**
 * @brief Error reporting callback function type.
 *
 * @param[in] error pointer to a zero-terminated string containing the error description
 * @param[in,out] context pointer to a user-specific object (may be @c NULL)
 */
typedef void (*FetchErrorCallback)(const char* error, void* context);

/**
 * @brief Create a Fetch instance.
 *
 * @returns Pointer to the allocated Fetch instance.
 */
Fetch* fetch_alloc(void);

/**
 * @brief Delete the Fetch instance.
 *
 * @warning The instance must be not running.
 *
 * @param[in,out] instance Pointer to the Fetch instance to free.
 */
void fetch_free(Fetch* instance);

/**
 * @brief Run the fetch process in the current thread.
 *
 * This call will block the calling thread
 * until the fetch process is complete.
 *
 * The callbacks will be executed in the calling thread's context.
 *
 * @warning The caller must ensure that at least 8 KB of stack
 *          is available within the context.
 *
 * @param[in,out] instance Pointer to the Fetch instance.
 * @param[in] request Pointer to the request to be executed.
 * @returns @c FetchStatusOk on success, any other @ref FetchStatus value otherwise
 */
FetchStatus fetch_run(Fetch* instance, const FetchRequest* request);

/**
 * @brief Request the ongoing fetch process to stop.
 *
 * This function is non-blocking. The execution thread must
 * wait until `fetch_exec()` returns.
 *
 * @param[in,out] instance Pointer to the Fetch instance.
 */
void fetch_stop(Fetch* instance);

/**
 * @brief Set the callback context for the Fetch instance.
 *
 * @param[in,out] instance Pointer to the Fetch instance.
 * @param[in,out] context Pointer to the context to set.
 */
void fetch_set_callback_context(Fetch* instance, void* context);

/**
 * @brief Set the callback for received data.
 *
 * @param[in,out] instance Pointer to the Fetch instance.
 * @param[in] callback Function pointer to the callback function.
 */
void fetch_set_rx_data_callback(Fetch* instance, FetchRxDataCallback callback);

/**
 * @brief  Set the callback for the header data.
 *
 * @param[in,out] instance Pointer to the Fetch instance.
 * @param[in] callback Function pointer to the callback function.
 */
void fetch_set_header_callback(Fetch* instance, FetchHeaderCallback callback);

/**
 * @brief Set the callback for error reporting.
 *
 * @param[in,out] instance Pointer to the Fetch instance.
 * @param[in] callback Function pointer to the callback function.
 */
void fetch_set_error_callback(Fetch* instance, FetchErrorCallback callback);

/**
 * @brief  Sets the callback for status updates.
 *
 * @param[in,out] instance Pointer to the Fetch instance.
 * @param[in] callback Function pointer to the callback function.
 */
void fetch_set_progress_callback(Fetch* instance, FetchProgressCallback callback);

#ifdef __cplusplus
}
#endif
