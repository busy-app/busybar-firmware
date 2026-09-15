#include "http_api.h"

#include <js_app/js_app_settings_storage.h>

#define TAG "HttpApps"

#define API_APPS_ID_MAX_LENGTH (32)

static void http_api_apps_settings_get(struct mg_connection* conn, JsAppSettingsStorage* storage) {
    FuriString* buffer = furi_string_alloc();

    do {
        if(!js_app_settings_storage_load(storage)) {
            MG_REPLY_ERROR(conn, 508, "Failed to load settings");
            break;
        }

        if(!js_app_settings_storage_export(storage, buffer)) {
            MG_REPLY_SERVICE_UNAVAILABLE(conn, "Failed to build settings document");
            break;
        }

        MG_REPLY_OK_BODY(conn, "%s\n", furi_string_get_cstr(buffer));
    } while(false);

    furi_string_free(buffer);
}

static void http_api_apps_settings_put(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    JsAppSettingsStorage* storage) {
    do {
        if(!js_app_settings_storage_import(storage, msg->body.buf, msg->body.len)) {
            MG_REPLY_BAD_REQUEST(conn);
            break;
        }

        if(!js_app_settings_storage_save(storage)) {
            MG_REPLY_ERROR(conn, 508, "Failed to save settings");
            break;
        }

        MG_REPLY_OK(conn);
    } while(false);
}

static void
    http_api_apps_settings_delete(struct mg_connection* conn, JsAppSettingsStorage* storage) {
    if(js_app_settings_storage_reset(storage)) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_ERROR(conn, 508, "Failed to reset settings");
    }
}

bool http_api_apps_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) {
        return false;
    }

    char app_id[API_APPS_ID_MAX_LENGTH + 1];
    if(mg_http_get_var(&msg->query, "app_id", app_id, sizeof(app_id)) <= 0) {
        MG_REPLY_BAD_REQUEST(conn);
        return true;
    }

    JsAppSettingsStorageStatus status;
    JsAppSettingsStorage* storage = js_app_settings_storage_alloc(app_id, &status);

    if(!storage) {
        if(status == JsAppSettingsStorageStatusSchemaMissing) {
            MG_REPLY_ERROR(conn, 404, "Application has no settings schema");
        } else if(status == JsAppSettingsStorageStatusSchemaInvalid) {
            MG_REPLY_SERVICE_UNAVAILABLE(conn, "Application settings schema is invalid");
        } else if(status == JsAppSettingsStorageStatusStorageFailure) {
            MG_REPLY_ERROR(conn, 508, "Failed to read settings schema");
        }
    } else {
        if(method == HttpMethodGet) {
            FURI_LOG_D(TAG, "Loading \"%s\" application settings", app_id);
            http_api_apps_settings_get(conn, storage);
        } else if(method == HttpMethodPut) {
            FURI_LOG_D(TAG, "Saving \"%s\" application settings", app_id);
            http_api_apps_settings_put(conn, msg, storage);
        } else if(method == HttpMethodDelete) {
            FURI_LOG_D(TAG, "Resetting \"%s\" application settings", app_id);
            http_api_apps_settings_delete(conn, storage);
        }

        js_app_settings_storage_free(storage);
    }

    return true;
}
