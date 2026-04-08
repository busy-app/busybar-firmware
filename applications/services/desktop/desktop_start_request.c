#include "desktop_start_request.h"

#include <furi.h>

struct DesktopStartRequest {
    FuriString* name;
    FuriString* args;
    bool is_default;
};

DesktopStartRequest*
    desktop_start_request_alloc(const char* name, const char* args, bool is_default) {
    furi_assert(name);

    DesktopStartRequest* request = malloc(sizeof(DesktopStartRequest));

    request->name = furi_string_alloc_set(name);
    request->args = args ? furi_string_alloc_set(args) : furi_string_alloc();
    request->is_default = is_default;

    return request;
}

void desktop_start_request_free(DesktopStartRequest* request) {
    furi_string_free(request->name);
    furi_string_free(request->args);
    free(request);
}

const char* desktop_start_request_get_name(const DesktopStartRequest* request) {
    return furi_string_get_cstr(request->name);
}

const char* desktop_start_request_get_args(const DesktopStartRequest* request) {
    return furi_string_empty(request->args) ? NULL : furi_string_get_cstr(request->args);
}

bool desktop_start_request_is_default(const DesktopStartRequest* request) {
    return request->is_default;
}
