#include "http_api.h"

#include <apps_menu/apps_menu.h>
#include <js_app/js_app_installer.h>
#include <js_app/js_app_registry.h>
#include <js_app/js_app_settings.h>
#include <storage_utils/temp_file.h>
#include <toolbox/timers.h>

#define TAG "HttpApps"

#define APP_NAME_PARAM_KEY    "application_name"
#define APP_NAME_LEN_MAX      (32)
#define SETTINGS_BODY_LEN_MAX (32 * 1024)

#define APPS_INSTALL_WORK_DIR        EXT_PATH("tmp/app_install")
#define APPS_INSTALL_BODY_LEN_MAX    (8 * 1024 * 1024)
#define APPS_INSTALL_IDLE_TIMEOUT_MS (5000)

typedef struct {
    HttpHandlersList_t handlers;
} ApiAppsCtx;

typedef struct {
    Storage* storage;
    TempFile* archive_file;
    // Named per connection: two concurrent uploads sharing one path means the
    // second truncates the first and both installs get a corrupt archive.
    FuriString* archive_path;
    size_t total_size;
    size_t received_size;
    CoarseTimer timeout_timer;
    FuriThreadPriority original_thread_priority;
} HttpAppsInstallContext;

static bool api_apps_get_app_name(const struct mg_str* query, char* output, size_t output_size) {
    const int length = mg_http_get_var(query, APP_NAME_PARAM_KEY, output, output_size);
    return (length > 0) && ((size_t)length < output_size) && js_app_id_is_valid(output);
}

static JsAppSettings* api_apps_load_settings(const char* app_name) {
    JsAppSettings* settings = NULL;
    JsApp* app = js_app_registry_get_app(app_name);
    if(!app) return NULL;

    JsAppInfo info;
    if(js_app_get_info(app, &info) && info.path.settings) {
        settings = js_app_settings_alloc();
        if(!js_app_settings_load(settings, info.manifest.id, info.path.settings)) {
            js_app_settings_free(settings);
            settings = NULL;
        }
    }

    js_app_free(app);
    return settings;
}

static void api_apps_get_settings(struct mg_connection* conn, JsAppSettings* settings) {
    cJSON* response = cJSON_CreateObject();
    cJSON_AddItemToObject(
        response, "schema", cJSON_Duplicate(js_app_settings_get_schema_json(settings), true));
    cJSON_AddItemToObject(
        response, "values", cJSON_Duplicate(js_app_settings_get_values_json(settings), true));

    char* serialized = cJSON_PrintUnformatted(response);
    MG_REPLY_OK_BODY(conn, "%s\n", serialized);
    free(serialized);
    cJSON_Delete(response);
}

static void api_apps_put_settings(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    JsAppSettings* settings) {
    if(msg->body.len > SETTINGS_BODY_LEN_MAX) {
        MG_REPLY_PAYLOAD_TOO_LARGE(conn);
        return;
    }

    cJSON* values = cJSON_ParseWithLength(msg->body.buf, msg->body.len);
    if(!cJSON_IsObject(values) || !js_app_settings_update_json(settings, values)) {
        MG_REPLY_ERROR(conn, 400, "Invalid settings");
    } else {
        char* serialized = cJSON_PrintUnformatted(js_app_settings_get_values_json(settings));
        MG_REPLY_OK_BODY(conn, "%s\n", serialized);
        free(serialized);
    }
    cJSON_Delete(values);
}

static bool api_apps_settings_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);
    if(!IS_HTTP_ENDPOINT(path)) return false;

    char app_name[APP_NAME_LEN_MAX + 1];
    if(!api_apps_get_app_name(&msg->query, app_name, sizeof(app_name))) {
        MG_REPLY_BAD_REQUEST(conn);
        return true;
    }

    JsAppSettings* settings = api_apps_load_settings(app_name);
    if(!settings) {
        MG_REPLY_NOT_FOUND(conn);
        return true;
    }

    if(method == HttpMethodGet) {
        api_apps_get_settings(conn, settings);
    } else if(method == HttpMethodPut) {
        api_apps_put_settings(conn, msg, settings);
    }

    js_app_settings_free(settings);
    return true;
}

static void api_apps_list_add(const JsAppInfo* info, void* context) {
    cJSON* applications = context;
    cJSON* application = cJSON_CreateObject();
    cJSON_AddStringToObject(application, "id", info->manifest.id);
    cJSON_AddStringToObject(application, "name", info->manifest.name);
    cJSON_AddStringToObject(application, "version", info->manifest.version);
    cJSON_AddStringToObject(application, "description", info->manifest.description);
    cJSON_AddStringToObject(application, "author", info->manifest.author);
    cJSON_AddNumberToObject(application, "heap_size_kib", info->manifest.heap_size / 1024);
    cJSON_AddBoolToObject(application, "debug", info->manifest.is_debug);
    cJSON_AddBoolToObject(application, "has_settings", info->path.settings != NULL);
    cJSON_AddItemToArray(applications, application);
}

static bool api_apps_list_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(msg);
    UNUSED(ctx);
    if(!IS_HTTP_ENDPOINT(path)) return false;

    cJSON* response = cJSON_CreateObject();
    cJSON* applications = cJSON_AddArrayToObject(response, "applications");
    js_app_registry_list_apps(api_apps_list_add, applications);

    char* serialized = cJSON_PrintUnformatted(response);
    MG_REPLY_OK_BODY(conn, "%s\n", serialized);
    free(serialized);
    cJSON_Delete(response);
    return true;
}

static bool api_apps_launch_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);
    if(!IS_HTTP_ENDPOINT(path)) return false;

    char app_name[APP_NAME_LEN_MAX + 1];
    if(!api_apps_get_app_name(&msg->query, app_name, sizeof(app_name))) {
        MG_REPLY_BAD_REQUEST(conn);
        return true;
    }

    JsApp* app = js_app_registry_get_app(app_name);
    if(!app) {
        MG_REPLY_NOT_FOUND(conn);
        return true;
    }
    js_app_free(app);

    if(apps_menu_start_application(app_name, true)) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_SERVICE_UNAVAILABLE(conn, "Failed to launch application");
    }
    return true;
}

static bool api_apps_remove_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(method);
    UNUSED(ctx);
    if(!IS_HTTP_ENDPOINT(path)) return false;

    char app_name[APP_NAME_LEN_MAX + 1];
    if(!api_apps_get_app_name(&msg->query, app_name, sizeof(app_name))) {
        MG_REPLY_BAD_REQUEST(conn);
        return true;
    }

    JsApp* app = js_app_registry_get_app(app_name);
    if(!app) {
        MG_REPLY_NOT_FOUND(conn);
        return true;
    }
    js_app_free(app);

    if(js_app_registry_remove_app(app_name)) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_SERVICE_UNAVAILABLE(conn, "Failed to remove application");
    }
    return true;
}

static void api_apps_install_free_context(HttpAppsInstallContext* context) {
    if(!context) return;

    furi_thread_set_current_priority(context->original_thread_priority);

    if(context->archive_file) {
        temp_file_remove(context->archive_file);
        temp_file_free(context->archive_file);
    } else if(context->storage && context->archive_path) {
        storage_simply_remove(context->storage, furi_string_get_cstr(context->archive_path));
    }
    if(context->archive_path) furi_string_free(context->archive_path);
    if(context->storage) furi_record_close(RECORD_STORAGE);
    free(context);
}

static void api_apps_install_reply(
    struct mg_connection* conn,
    JsAppInstallResult result,
    const FuriString* app_id) {
    switch(result) {
    case JsAppInstallResultOk:
        mg_http_reply(
            conn,
            200,
            DEFAULT_JSON_HEADERS "Connection: close\r\n",
            "{\"result\":\"OK\",\"application_name\":\"%M\"}\n",
            MG_ESC(furi_string_get_cstr(app_id)));
        break;
    case JsAppInstallResultInvalidArchive:
        MG_REPLY_ERROR_CLOSE(conn, 400, "Invalid application archive");
        break;
    case JsAppInstallResultInvalidApplication:
        MG_REPLY_ERROR_CLOSE(conn, 400, "Invalid application");
        break;
    case JsAppInstallResultVersionConflict:
        mg_http_reply(
            conn,
            409,
            DEFAULT_JSON_HEADERS "Connection: close\r\n",
            "{\"error\":\"Installed application version is newer or equal\"}\n");
        break;
    case JsAppInstallResultTooLarge:
        mg_http_reply(
            conn,
            413,
            DEFAULT_JSON_HEADERS "Connection: close\r\n",
            "{\"error\":\"Application package is too large\"}\n");
        break;
    case JsAppInstallResultStorageError:
        // 508 is what every other upload endpoint here returns for a storage
        // failure; returning 503 from this one path made the same condition
        // look like two different problems.
        mg_http_reply(
            conn,
            508,
            DEFAULT_JSON_HEADERS "Connection: close\r\n",
            "{\"error\":\"Application installation failed\"}\n");
        break;
    case JsAppInstallResultMax:
    default:
        MG_REPLY_ERROR_CLOSE(conn, 500, "Application installation failed");
        break;
    }
    conn->is_draining = 1;
}

static void api_apps_install_on_data(struct mg_connection* conn, struct mg_iobuf* io) {
    ConnectionContext* connection_context = (void*)conn->data;
    HttpAppsInstallContext* context = connection_context->context;
    if(!context || !context->archive_file) {
        // Data arriving after the upload already completed: a malformed
        // request, not the version conflict that 409 means on this endpoint.
        MG_REPLY_ERROR_CLOSE(conn, 400, "Application upload context is invalid");
        mg_iobuf_del(io, 0, io->len);
        conn->is_draining = 1;
        return;
    }

    context->timeout_timer = coarse_timer_create(APPS_INSTALL_IDLE_TIMEOUT_MS);
    if(context->received_size + io->len > context->total_size) {
        MG_REPLY_ERROR_CLOSE(conn, 413, "Payload Too Large");
        mg_iobuf_del(io, 0, io->len);
        conn->is_draining = 1;
        return;
    }

    if(io->len && !temp_file_write(context->archive_file, io->buf, io->len)) {
        MG_REPLY_ERROR_CLOSE(conn, 508, "Failed to save application package");
        mg_iobuf_del(io, 0, io->len);
        conn->is_draining = 1;
        return;
    }

    context->received_size += io->len;
    mg_iobuf_del(io, 0, io->len);

    if(context->received_size == context->total_size) {
        temp_file_free(context->archive_file);
        context->archive_file = NULL;

        FuriString* app_id = furi_string_alloc();
        const JsAppInstallResult result =
            js_app_installer_install(furi_string_get_cstr(context->archive_path), app_id);
        api_apps_install_reply(conn, result, app_id);
        furi_string_free(app_id);
    }
}

static void api_apps_install_on_poll(struct mg_connection* conn) {
    ConnectionContext* connection_context = (void*)conn->data;
    HttpAppsInstallContext* context = connection_context->context;
    if(context && coarse_timer_is_expired(context->timeout_timer)) {
        MG_REPLY_TIMEOUT(conn, "Upload timeout");
        conn->is_draining = 1;
    }
}

static void api_apps_install_on_close(struct mg_connection* conn) {
    ConnectionContext* connection_context = (void*)conn->data;
    api_apps_install_free_context(connection_context->context);
    connection_context->context = NULL;
    connection_context->raw.on_data = NULL;
    connection_context->raw.on_poll = NULL;
    connection_context->on_close = NULL;
}

static bool api_apps_install_hdr_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);
    if(!IS_HTTP_ENDPOINT(path)) return false;
    if(method == HttpMethodOptions) return false;
    if(method != HttpMethodPost) {
        http_reply_405_method_not_allowed(conn, HttpMethodPost, true);
        conn->is_draining = 1;
        return true;
    }
    if((msg->body.len == 0) || (msg->body.len > APPS_INSTALL_BODY_LEN_MAX)) {
        MG_REPLY_ERROR_CLOSE(
            conn, msg->body.len ? 413 : 400, msg->body.len ? "Payload Too Large" : "Bad Request");
        conn->is_draining = 1;
        return true;
    }

    HttpAppsInstallContext* context = malloc(sizeof(*context));
    memset(context, 0, sizeof(*context));
    context->storage = furi_record_open(RECORD_STORAGE);
    context->archive_file = temp_file_alloc(context->storage);
    context->archive_path =
        furi_string_alloc_printf("%s/upload-%lu.tgz", APPS_INSTALL_WORK_DIR, conn->id);
    context->total_size = msg->body.len;
    context->timeout_timer = coarse_timer_create(APPS_INSTALL_IDLE_TIMEOUT_MS);
    context->original_thread_priority = furi_thread_get_current_priority();

    ConnectionContext* connection_context = (void*)conn->data;
    connection_context->context = context;
    connection_context->raw.on_data = api_apps_install_on_data;
    connection_context->raw.on_poll = api_apps_install_on_poll;
    connection_context->on_close = api_apps_install_on_close;

    // Writing the upload and then untarring it both run on the single mongoose
    // poll thread. Yield to the rest of the system for the duration, the way
    // the firmware update endpoint does; the context restores this on close.
    furi_thread_set_current_priority(FuriThreadPriorityLow);

    if(!temp_file_create(context->archive_file, furi_string_get_cstr(context->archive_path))) {
        MG_REPLY_ERROR_CLOSE(conn, 508, "Failed to create application package");
        conn->is_draining = 1;
        return true;
    }

    mg_iobuf_del(&conn->recv, 0, msg->head.len);
    conn->pfn = NULL;
    api_apps_install_on_data(conn, &conn->recv);
    return true;
}

static bool api_apps_install_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(path);
    UNUSED(msg);
    UNUSED(ctx);
    if(method == HttpMethodOptions) {
        http_reply_cors_preflight(conn, HttpMethodPost);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }
    return true;
}

static const HttpHandler handlers_apps[] = {
    {
        .uri = "install",
        .method = HttpMethodAny,
        .type = HttpHandlerCustom,
        .on_request = api_apps_install_callback,
        .on_headers = api_apps_install_hdr_callback,
    },
    {
        .uri = "list",
        .method = HttpMethodGet,
        .type = HttpHandlerCustom,
        .on_request = api_apps_list_callback,
    },
    {
        .uri = "launch",
        .method = HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = api_apps_launch_callback,
    },
    {
        .uri = "remove",
        .method = HttpMethodDelete,
        .type = HttpHandlerCustom,
        .on_request = api_apps_remove_callback,
    },
    {
        .uri = "settings",
        .method = HttpMethodGet | HttpMethodPut,
        .type = HttpHandlerCustom,
        .on_request = api_apps_settings_callback,
    },
};

void* http_api_apps_alloc(void) {
    ApiAppsCtx* context = malloc(sizeof(*context));
    HttpHandlersList_init(context->handlers);
    for(size_t i = COUNT_OF(handlers_apps); i > 0; --i) {
        http_handler_add(context->handlers, &handlers_apps[i - 1]);
    }
    return context;
}

void http_api_apps_free(void* ctx) {
    furi_assert(ctx);
    ApiAppsCtx* context = ctx;
    HttpHandlersList_clear(context->handlers);
    free(context);
}

bool http_api_apps_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiAppsCtx* context = ctx;
    return http_handle_request(path, method, context->handlers, conn, msg);
}

bool http_api_apps_hdr_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiAppsCtx* context = ctx;
    return http_handle_headers(path, method, context->handlers, conn, msg);
}
