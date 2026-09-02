#pragma once

#include <stdbool.h>

#include "slice.h"

typedef struct Url Url;

typedef enum {
    UrlPartIdHref,
    UrlPartIdOrigin,
    UrlPartIdProtocol,
    UrlPartIdHost,
    UrlPartIdHostname,
    UrlPartIdPort,
    UrlPartIdPathname,
    UrlPartIdSearch,
    UrlPartIdMax,
} UrlPartId;

Url* url_alloc(void);

void url_free(Url* instance);

bool url_parse(Url* instance, const char* source);

void url_get_part(const Url* instance, UrlPartId part_id, StringSlice* part);
