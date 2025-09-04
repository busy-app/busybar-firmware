#pragma once

#include <furi.h>

typedef struct CurlClient CurlClient;

CurlClient* curl_client_alloc(FuriString* url);
void curl_client_free(CurlClient* instance);
void curl_client_run(CurlClient* instance);
bool curl_client_is_done(CurlClient* instance);
