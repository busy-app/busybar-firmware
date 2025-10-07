#include "http_api.h"

#include <furi.h>

#include <device_name/device_name.h>
#include <cjson/cJSON.h>

#define TAG "HttpName"

static bool http_api_name_parse(const char* payload, FuriString* output) {
    bool result = false;
    cJSON* json = cJSON_Parse(payload);
    if(json != NULL) {
        cJSON* item = cJSON_GetObjectItem(json, "name");
        if(item != NULL) {
            furi_string_set_str(output, cJSON_GetStringValue(item));
            result = true;
        }
        cJSON_Delete(json);
    }
    return result;
}

bool http_api_name_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);
    if(!IS_HTTP_ENDPOINT(path)) return false;

    if(mg_match(msg->method, mg_str("GET"), NULL)) {
        FuriString* name = furi_string_alloc();

        DeviceName* dev_name = furi_record_open(RECORD_DEVICE_NAME);
        device_name_get(dev_name, name);
        furi_record_close(RECORD_DEVICE_NAME);

        MG_REPLY_OK_BODY(conn, "{\"name\":\"%s\"}\n", furi_string_get_cstr(name));
        furi_string_free(name);
    } else if(mg_match(msg->method, mg_str("POST"), NULL)) {
        FuriString* name = furi_string_alloc();
        FuriString* error = furi_string_alloc();
        do {
            if(!http_api_name_parse(msg->body.buf, name)) {
                MG_REPLY_BAD_REQUEST(conn);
                break;
            }

            DeviceName* dev_name = furi_record_open(RECORD_DEVICE_NAME);
            bool result = device_name_set(dev_name, name, error);
            furi_record_close(RECORD_DEVICE_NAME);

            if(result)
                MG_REPLY_OK(conn);
            else
                MG_REPLY_ERROR(conn, 400, furi_string_get_cstr(error));
        } while(false);

        furi_string_free(name);
        furi_string_free(error);
    } else
        MG_REPLY_METHOD_NOT_ALLOWED(conn);

    return true;
}
