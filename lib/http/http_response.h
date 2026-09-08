/**
 * @file http_response.h
 * @brief Utility functions for parsing HTTP responses.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "slice.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Parsed HTTP response structure.
 */
typedef struct {
    uint32_t status; /**< Numeric HTTP status code */
    StringSlice status_text; /**< HTTP status text slice (optional) */
    StringSlice headers; /**< HTTP headers slice (optional) */
} HttpResponse;

/**
 * @brief Initialise a HttpResponse instance with default values
 *
 * After calling this function, all string slices are guaranteed
 * to be zero-length and with non-@c NULL pointers.
 *
 * @param[in,out] instance pointer to the instance to be initialised
 */
void http_response_init(HttpResponse* instance);

/**
 * @brief Parse the response from a raw string representation.
 *
 * @note In case of failure, the value pointed to by @p instance
 *       is left untouched. Therefore, if a valid value is needed at all times,
 *       `http_response_init` must be called first to ensure that the instance
 *       is valid before calling this function.
 *
 * @param[in,out] instance pointer to the instance to be parsed
 */
bool http_response_parse(HttpResponse* instance, const char* data, size_t data_len);

#ifdef __cplusplus
}
#endif
