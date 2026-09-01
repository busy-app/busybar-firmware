#include "http_api.h"

#include <telemetry/telemetry.h>
#include <cjson/cJSON.h>

#define TAG "HttpTelemetry"

bool http_api_telemetry_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);
    if(!IS_HTTP_ENDPOINT(path)) return false;

    if(method == HttpMethodGet) {
        Telemetry* telemetry = furi_record_open(RECORD_TELEMETRY);
        const bool enabled = telemetry_is_enabled(telemetry);
        furi_record_close(RECORD_TELEMETRY);

        MG_REPLY_OK_BODY(conn, "{\"enabled\":%s}\n", enabled ? "true" : "false");
    } else if(method == HttpMethodPut) {
        bool success = false;
        bool enabled = false;

        do {
            cJSON* json_root = cJSON_Parse(msg->body.buf);

            if(!cJSON_IsObject(json_root)) {
                cJSON_Delete(json_root);
                MG_REPLY_BAD_REQUEST(conn);
                break;
            }

            // Reject bodies with unexpected properties (schema: additionalProperties: false)
            cJSON* enabled_item = cJSON_GetObjectItem(json_root, "enabled");
            if(!cJSON_IsBool(enabled_item) || cJSON_GetArraySize(json_root) != 1) {
                cJSON_Delete(json_root);
                MG_REPLY_BAD_REQUEST(conn);
                break;
            }

            enabled = cJSON_IsTrue(enabled_item);

            Telemetry* telemetry = furi_record_open(RECORD_TELEMETRY);
            telemetry_set_enabled(telemetry, enabled);
            enabled = telemetry_is_enabled(telemetry);
            furi_record_close(RECORD_TELEMETRY);

            cJSON_Delete(json_root);
            success = true;
        } while(false);

        if(success) {
            MG_REPLY_OK_BODY(conn, "{\"enabled\":%s}\n", enabled ? "true" : "false");
        }
    } else {
        http_reply_405_method_not_allowed(conn, HttpMethodGet | HttpMethodPut, false);
    }

    return true;
}
