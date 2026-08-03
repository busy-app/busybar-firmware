#include "http_api.h"
#include <power/power_service/power.h>

#define TAG "HttpSystem"

#define REBOOT_TARGET_LEN_MAX 8
/* Give mongoose a moment to flush the response before the device goes down. */
#define REBOOT_DELAY_MS 250

static const struct {
    char* name;
    PowerRebootMode mode;
} reboot_targets[] = {
    {"all", PowerRebootNormal},
    {"mcu", PowerRebootNormalU5},
    {"wifi", PowerRebootNormal917},
};

static void system_reboot_timer_callback(void* context) {
    PowerRebootMode mode = (PowerRebootMode)(uintptr_t)context;

    Power* power = furi_record_open(RECORD_POWER);
    power_reboot(power, mode);
    furi_record_close(RECORD_POWER);
}

/* BarMetal addition.
 *
 * POST /api/system/reboot[?target=all|mcu|wifi]
 *
 * The stock firmware exposes no software reboot, so a device whose Wi-Fi
 * co-processor has stopped responding can only be recovered physically.
 * "wifi" restarts just the radio, which clears that failure without
 * interrupting anything running on the main MCU.
 */
bool http_api_system_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!furi_string_equal_str(path, "reboot")) return false;

    if(method == HttpMethodOptions) {
        http_reply_cors_preflight(conn, HttpMethodPost);
        return true;
    }

    if(method != HttpMethodPost) {
        http_reply_405_method_not_allowed(conn, HttpMethodPost, false);
        return true;
    }

    PowerRebootMode mode = PowerRebootNormal;
    bool success = true;

    if(msg->query.len > 0) {
        char target[REBOOT_TARGET_LEN_MAX];
        const int var_len = mg_http_get_var(&msg->query, "target", target, sizeof(target));

        if(var_len > 0) {
            success = false;
            for(size_t i = 0; i < COUNT_OF(reboot_targets); i++) {
                if(strcmp(reboot_targets[i].name, target) == 0) {
                    mode = reboot_targets[i].mode;
                    success = true;
                    break;
                }
            }
        }
    }

    if(!success) {
        MG_REPLY_BAD_REQUEST(conn);
        return true;
    }

    /* Answer first, then reboot, so the caller learns the request was accepted. */
    MG_REPLY_OK(conn);

    FuriTimer* timer = furi_timer_alloc(
        system_reboot_timer_callback, FuriTimerTypeOnce, (void*)(uintptr_t)mode);
    furi_timer_start(timer, furi_ms_to_ticks(REBOOT_DELAY_MS));

    return true;
}
