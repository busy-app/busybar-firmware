/** @file http_headers.h
 * @brief Utility functions for dealing with HTTP headers
 */
#pragma once

#include <core/string.h>

typedef struct HttpHeader {
    FuriString* key;
    FuriString* value;
} HttpHeader;

typedef struct HttpHeaders HttpHeaders;

HttpHeaders* http_headers_alloc(void);

void http_headers_free(HttpHeaders* headers);

bool http_headers_parse(HttpHeaders* instance, const char* data, size_t data_len);

size_t http_headers_get_header_count(const HttpHeaders* instance);

const HttpHeader* http_headers_get_header(const HttpHeaders* instance, size_t index);

const HttpHeader* http_headers_get(const HttpHeaders* instance, const char* key);

void http_headers_set(HttpHeaders* instance, const char* key, const char* value);
