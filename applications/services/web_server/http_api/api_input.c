#include "http_api.h"
#include <power/power_service/power.h>
#include <version.h>
#include <input/input.h>

#define TAG "HttpInput"

#define KEY_NAME_LEN_MAX 10

static const struct {
    char* name;
    InputKey key;
} input_keys[] = {
    {"up", InputKeyUp},
    {"down", InputKeyDown},
    {"ok", InputKeyOk},
    {"back", InputKeyBack},
    {"start", InputKeyStart},
    {"busy", InputKeyBusy},
    {"custom", InputKeyCustom},
    {"off", InputKeyOff},
    {"apps", InputKeyApps},
    {"settings", InputKeySettings},
};

static const char* const switch_position_names[] = {
    [InputSwitchPositionBusy] = "busy",
    [InputSwitchPositionStatus] = "custom",
    [InputSwitchPositionOff] = "off",
    [InputSwitchPositionApps] = "apps",
    [InputSwitchPositionSettings] = "settings",
};
static_assert(COUNT_OF(switch_position_names) == InputSwitchPositionMAX);

bool http_api_input_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);
    UNUSED(method);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    char key_name[KEY_NAME_LEN_MAX];
    bool success = false;

    do {
        if(msg->query.len == 0) break;

        int var_len = mg_http_get_var(&msg->query, "key", key_name, sizeof(key_name));
        if(var_len <= 0) break;

        Input* input = furi_record_open(RECORD_INPUT);
        for(size_t i = 0; i < COUNT_OF(input_keys); i++) {
            if(strcmp(input_keys[i].name, key_name) == 0) {
                input_key_toggle(input, input_keys[i].key);
                success = true;
                break;
            }
        }
        furi_record_close(RECORD_INPUT);
    } while(0);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    return true;
}

bool http_api_input_switch_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    Input* input = furi_record_open(RECORD_INPUT);
    InputSwitchPosition position;
    furi_state_get(input_get_switch_pos(input), &position);
    furi_record_close(RECORD_INPUT);

    const char* name = "unknown";
    if(position < InputSwitchPositionMAX) {
        name = switch_position_names[position];
    }

    MG_REPLY_OK_BODY(conn, "{\"position\":\"%s\"}\n", name);

    return true;
}
