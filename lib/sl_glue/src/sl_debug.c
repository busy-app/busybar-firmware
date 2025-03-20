#include <furi.h>

#define TAG "SlDebug"

void sl_debug_log(const char* format, ...) {
    va_list args;
    va_start(args, format);

    FuriString* message = furi_string_alloc_vprintf(format, args);
    furi_string_trim(message);

    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(message));
    furi_string_free(message);

    va_end(args);
}

// extern "C" 
void lwip_debug_log(const char* format, ...) {
    va_list args;
    va_start(args, format);

    FuriString* message = furi_string_alloc_vprintf(format, args);
    furi_string_trim(message);

    FURI_LOG_D("LWIP_Debug", "%s", furi_string_get_cstr(message));
    furi_string_free(message);

    va_end(args);
}

int __wrap_fprintf(FILE* stream, const char* format, ...) {
    UNUSED(stream);

    va_list args;
    va_start(args, format);

    FuriString* message = furi_string_alloc_vprintf(format, args);
    furi_string_trim(message);

    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(message));
    furi_string_free(message);

    va_end(args);
    return 0;
}

int __wrap__fflush_r(struct _reent* reent, FILE* stream) {
    UNUSED(reent);
    UNUSED(stream);
    return 0;
}
