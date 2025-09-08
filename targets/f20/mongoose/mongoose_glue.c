#include <furi.h>
#include <furi_hal_random.h>
#include <mongoose.h>

uint64_t mg_millis(void) {
    return furi_get_tick();
}

void mg_log_prefix(int level, const char* file, int line, const char* fname) {
    UNUSED(file);
    FuriString* string = furi_string_alloc();

    const char* color = _FURI_LOG_CLR_RESET;
    const char* log_letter = " ";
    switch(level) {
    case MG_LL_ERROR:
        color = _FURI_LOG_CLR_E;
        log_letter = "E";
        break;
    case MG_LL_INFO:
        color = _FURI_LOG_CLR_I;
        log_letter = "I";
        break;
    case MG_LL_DEBUG:
        color = _FURI_LOG_CLR_D;
        log_letter = "D";
        break;
    case MG_LL_VERBOSE:
        color = _FURI_LOG_CLR_T;
        log_letter = "T";
        break;
    default:
        break;
    }

    furi_string_printf(
        string,
        "%lu %s[%s][%s] " _FURI_LOG_CLR_RESET,
        furi_get_tick(),
        color,
        log_letter,
        "Mongoose");

    furi_string_cat_printf(string, "%s:%u ", fname, line);
    furi_log_puts(furi_string_get_cstr(string));

    furi_string_free(string);
}

void mg_log(const char* fmt, ...) {
    FuriString* string = furi_string_alloc();

    va_list args;
    va_start(args, fmt);
    furi_string_vprintf(string, fmt, args);
    va_end(args);

    furi_string_cat_str(string, "\r\n");
    furi_log_puts(furi_string_get_cstr(string));
    furi_string_free(string);
}

int _gettimeofday(struct timeval* tv, void* tz) {
    uint64_t now = mg_now();
    (void)tz;
    tv->tv_sec = (time_t)(now / 1000);
    tv->tv_usec = (unsigned long)((now % 1000) * 1000);
    return 0;
}

bool mg_random(void* buf, size_t len) {
    furi_hal_random_fill_buf(buf, len);
    return true;
}
