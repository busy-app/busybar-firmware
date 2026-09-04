/** @file http_headers.h
 * @brief Utility functions for dealing with HTTP headers
 */
#pragma once

#include <core/string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Single HTTP header structure (key: value).
 */
typedef struct HttpHeader {
    FuriString* key; /**< Header key (e.g. "Content-Type") */
    FuriString* value; /**< Header value (e.g. "application/json") */
} HttpHeader;

/**
 * @brief Opaque HttpHeaders type.
 */
typedef struct HttpHeaders HttpHeaders;

/**
 * @brief Create an empty HttpHeaders instance.
 *
 * @returns pointer to the created instance
 */
HttpHeaders* http_headers_alloc(void);

/**
 * @brief Delete the HttpHeaders instance.
 *
 * @param[in,out] instance pointer to the instance to be deleted
 */
void http_headers_free(HttpHeaders* instance);

/**
 * @brief Parse the HTTP headers from a string.
 *
 * @param[in,out] instance pointer to the instance to hold the parsing results
 * @param[in] data pointer to the source string (not required to be zero-terminated)
 * @param[in] data_len length of the source string in bytes
 *
 * @returns @c true if the input data could be parsed, @c false otherwise
 */
bool http_headers_parse(HttpHeaders* instance, const char* data, size_t data_len);

/**
 * @brief Get the number of header elements held in the instance.
 *
 * @param[in] instance pointer to the instance to be queried
 *
 * @returns number of elements currently held in the instance
 */
size_t http_headers_get_count(const HttpHeaders* instance);

/**
 * @brief Get the header element by its index.

 * @note The return value will become invalid after modfying
 *       the instance with `parse()` or `set()` functions
 *       or deleting it using `http_headers_free()`.
 *
 * @param[in] instance pointer to the instance to be queried
 * @param[in] index the desired element index (must be < total number of elements)
 *
 * @return pointer to the element at the requested index
 */
const HttpHeader* http_headers_get_by_index(const HttpHeaders* instance, size_t index);

/**
 * @brief Get the header element by its string key.
 *
 * The key string is case-insensitive (i.e. "KeY" and "key" are treated the same).
 *
 * @note The return value will become invalid after modfying
 *       the instance with `parse()` or `set()` functions
 *       or deleting it using `http_headers_free()`.
 *
 * @param[in] instance pointer to the instance to be queried
 * @param[in] key zero-terminated string containing the header name (key)
 *
 * @return pointer to the element at the requested key or @c NULL if none exists
 */
const HttpHeader* http_headers_get(const HttpHeaders* instance, const char* key);

/**
 * @brief Set the header element under the string key.
 *
 * The key string is case-insensitive (i.e. "KeY" and "key" are treated the same).
 *
 * If an element already exists under the provided key,
 * its value will be replaced with the new one, otherwise
 * a new element will be inserted.
 *
 * @param[in,out] instance pointer to the instance to be modified
 * @param[in] key zero-terminated string containing the header name (key)
 * @param[in] value zero-terminated string containing the value
 */
void http_headers_set(HttpHeaders* instance, const char* key, const char* value);

#ifdef __cplusplus
}
#endif
