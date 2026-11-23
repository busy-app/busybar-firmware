#include <furi.h>
#include <sl_log.h>

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

void assertEFM(const char* file, int line) {
    UNUSED(file);
    UNUSED(line);
    furi_crash("AssertEFM");
}

// TODO: Find out what's trying to link with write()
int __wrap__write_r(void) {
    furi_crash("write_r()");
}

sl_log_level_t sl_log_get_loglevel(void) {
    return SL_LOG_ENUM_CONFIG_DEBUG;
}

void sl_log_send_no_args(uint32_t event_id, uint8_t log_level) {
    UNUSED(log_level);
    FURI_LOG_D(TAG, (const char*)event_id);
}

void sl_log_send_arg1(uint32_t event_id, uint8_t log_level, uint32_t arg1) {
    UNUSED(log_level);
    FURI_LOG_D(TAG, (const char*)event_id, arg1);
}

void sl_log_send_arg2(uint32_t event_id, uint8_t log_level, uint32_t arg1,
                      uint32_t arg2) {
    UNUSED(log_level);
    FURI_LOG_D(TAG, (const char*)event_id, arg1, arg2);
}

void sl_log_send_arg3(uint32_t event_id, uint8_t log_level, uint32_t arg1,
                      uint32_t arg2, uint32_t arg3) {
    UNUSED(log_level);
    FURI_LOG_D(TAG, (const char*)event_id, arg1, arg2, arg3);
}

sl_status_t sl_log_sync_timestamp(uint8_t core_id, void *args) {
    UNUSED(core_id);
    UNUSED(args);
    return SL_STATUS_OK;
}

static uint32_t timesync;
uint32_t sl_si91x_log_host_timesync_address = (uint32_t)&timesync;
