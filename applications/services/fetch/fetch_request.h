/** @file fetch_request.h
 * @brief HTTP request data structure
 */
#pragma once
#include "fetch_common.h"
#include <toolbox/tls_config.h>

/** Maximum number of additional request headers. */
#define FETCH_HEADERS_COUNT_MAX (12)

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif
