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

void assertEFM(const char *file, int line){
    UNUSED(file);
    UNUSED(line);
    furi_crash("AssertEFM");
}
