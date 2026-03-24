#include "http_api.h"

#include <furi.h>

#include <device_name/device_name.h>
#include <cjson/cJSON.h>

#define TAG "HttpName"

static const char* device_name_error_message[DeviceNameErrorMax] = {
    [DeviceNameErrorNone] = "Ok",
    [DeviceNameErrorEmpty] = "Name is empty",
    [DeviceNameErrorTooLong] = "Name is too long",
    [DeviceNameErrorIllegalChar] = "Name contains disallowed character",
    [DeviceNameErrorOnlySpaces] = "Name consists of only spaces",
};

static bool http_api_name_parse(const char* payload, FuriString* output) {
    cJSON* json_root = cJSON_Parse(payload);

    bool is_successful = false;
    if(cJSON_IsObject(json_root)) {
        cJSON* name_item = cJSON_GetObjectItem(json_root, "name");

        if(cJSON_IsString(name_item)) {
            furi_string_set_str(output, name_item->valuestring);
            is_successful = true;
        }
    }

    cJSON_Delete(json_root);

    return is_successful;
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

        MG_REPLY_OK_BODY(conn, "{\"name\":%m}\n", mg_print_esc, 0, furi_string_get_cstr(name));
        furi_string_free(name);
    } else if(mg_match(msg->method, mg_str("POST"), NULL)) {
        FuriString* name = furi_string_alloc();
        do {
            if(!http_api_name_parse(msg->body.buf, name)) {
                MG_REPLY_BAD_REQUEST(conn);
                break;
            }

            DeviceName* dev_name = furi_record_open(RECORD_DEVICE_NAME);
            DeviceNameError error = device_name_set(dev_name, name);
            furi_assert(error < DeviceNameErrorMax);

            furi_record_close(RECORD_DEVICE_NAME);

            if(error == DeviceNameErrorNone) {
                MG_REPLY_OK(conn);
            } else {
                MG_REPLY_ERROR(conn, 400, device_name_error_message[error]);
            }
        } while(false);

        furi_string_free(name);
    } else
        MG_REPLY_METHOD_NOT_ALLOWED(conn);

    return true;
}
