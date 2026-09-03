/** @file http_headers.h
 * @brief Utility functions for dealing with HTTP headers
 */
#pragma once

#include <furi/core/string.h>
#include <m-array.h>

typedef struct HttpHeader {
    FuriString* key;
    FuriString* value;
} HttpHeader;

typedef struct HttpHeaders HttpHeaders;

HttpHeaders* http_headers_alloc(void);

bool http_headers_parse(HttpHeaders* instance, const char* data, size_t size);

uint32_t http_headers_get_status(const HttpHeaders* instance);

const char* http_headers_get_status_text(const HttpHeaders* instance);

size_t http_headers_get_header_count(const HttpHeaders* instance);

const HttpHeader* http_headers_get_header(const HttpHeaders* instance, size_t index);

void http_headers_free(HttpHeaders* headers);
