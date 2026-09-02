#pragma once

#include <stdbool.h>

#include "slice.h"

typedef struct Url Url;

typedef enum {
    UrlPartHref,
    UrlPartOrigin,
    UrlPartProtocol,
    UrlPartHost,
    UrlPartHostname,
    UrlPartPort,
    UrlPartPathname,
    UrlPartSearch,
    UrlPartMax,
} UrlPart;

Url* url_alloc(void);

void url_free(Url* instance);

bool url_parse(Url* instance, const char* source);

void url_get_part(const Url* instance, UrlPart part, StringSlice* out);
