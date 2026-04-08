#pragma once

#include <stdbool.h>

typedef struct DesktopStartRequest DesktopStartRequest;

DesktopStartRequest*
    desktop_start_request_alloc(const char* name, const char* args, bool is_default);

void desktop_start_request_free(DesktopStartRequest* request);

const char* desktop_start_request_get_name(const DesktopStartRequest* request);

const char* desktop_start_request_get_args(const DesktopStartRequest* request);

bool desktop_start_request_is_default(const DesktopStartRequest* request);
