/**
 * @file js_fetch_request.h
 * @brief Internal request parsing methods for fetch functionality.
 */
#pragma once

#include <lib/fetch/fetch.h>
#include <lib/jerryscript/amalgam/jerryscript.h>

/**
 * @brief Parse an incoming JS Request object into its native representation.
 *
 * @param[in] obj value representing the input JS object to be parsed
 * @param[out] request pointer to the native fetch request object (must be allocated)
 * @returns @c undefined on success
 * @returns @c exception on failure (the exception will contain an error string)
 */
jerry_value_t js_fetch_request_parse(jerry_value_t obj, FetchRequest* request);

/**
 * @brief Delete allocated fields inside the request object.
 *
 * The memory pointed to by the @p request parameter is not freed.
 *
 * @param[in,out] request pointer to the native fetch request to be cleared.
 */
void js_fetch_request_clear(FetchRequest* request);
