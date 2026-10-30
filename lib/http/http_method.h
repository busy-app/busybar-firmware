/**
 * @file http_method.h
 * @brief Utility functions for dealing with HTTP method strings.
 */
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Check whether the HTTP method string is valid.
 *
 * @param[in] method_str Pointer to the zero-terminated method string
 * @returns @c true if the method string is valid, @c false otherwise
 */
bool http_method_is_valid(const char* method_str);

/**
 * @brief Normalize the method string in-place.
 *
 * Normalization is performed according to
 * https://fetch.spec.whatwg.org/#concept-method-normalize
 *
 * @param[in,out] method_str Pointer to the zero-terminated method string
 */
void http_method_normalize(char* method_str);

#ifdef __cplusplus
}
#endif
