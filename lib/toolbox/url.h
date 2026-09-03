/**
 * @file url.h
 * @brief URL parsing library.
 */
#pragma once

#include <stdbool.h>

#include "slice.h"

/**
 * @brief Url opaque type.
 */
typedef struct Url Url;

/**
 * @brief Enumeration of available URL part identifiers.
 */
typedef enum {
    UrlPartIdHref, /**< Full URL */
    UrlPartIdOrigin, /**< Protocol + Hostname + Port (if specified) */
    UrlPartIdProtocol, /**< Protocol string */
    UrlPartIdHost, /**< Hostname + Port (if specified) */
    UrlPartIdHostname, /**< Hostname or IP address */
    UrlPartIdPort, /**< Port string (if specified) */
    UrlPartIdPathname, /**< Request path (if specified) */
    UrlPartIdSearch, /**< Request parameters (if specified) */
    UrlPartIdMax, /**< Special value, internal use */
} UrlPartId;

/**
 * @brief Create an URL instance.
 *
 * @returns pointer to the created instance
 */
Url* url_alloc(void);

/**
 * @brief Delete an URL instance.
 *
 * All part pointers obtained via @p url_get_part will become INVALID after this call.
 *
 * @param[in,out] instance pointer to the instance to be deleted
 */
void url_free(Url* instance);

/**
 * @brief Parse an URL from a string.
 *
 * The following URL parts are required:
 * - Protocol (e.g. "http://")
 * - Hostname (e.g. "10.0.4.20" or "my.awesome.site")
 *
 * All part pointers obtained via @p url_get_part will remain VALID after this call,
 * but their underlying values might silently change.
 *
 * @param[in,out] instance pointer to the instance to contain the parsed data
 * @param[in] source pointer to a zero-terminated URL string
 * @returns @c true if the URL could be parsed, @c false otherwise
 */
bool url_parse(Url* instance, const char* source);

/**
 * @brief Get a URL part via its identifier.
 *
 * The return value is guaranteed to be non-@c NULL and valid until @p url_free is called.
 *
 * If a part is not present, the returned slice will represent an empty string.
 *
 * @param[in] instance pointer to the instance to be queried
 * @param[in] part_id numeric identifier of the part in question
 * @returns pointer to the string slice representing the requested part
 */
const StringSlice* url_get_part(const Url* instance, UrlPartId part_id);
