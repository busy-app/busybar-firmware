#include "http_api.h"
#include <loader/loader.h>
#include <desktop/desktop.h>
#include <gui/gui.h>
#include <toolbox/path.h>
#include <canvas/canvas.h>

#define TAG "HttpDisplay"

#define DISPLAY_ASSETS_DIR EXT_PATH("assets")

static bool api_display_draw_parse_element(
    CanvasElementsArray_t elements_array,
    char* app_id,
    struct mg_str element) {
    bool success = false;
    char* element_type = NULL;
    CanvasElement* canvas_element = CanvasElementsArray_push_new(elements_array);
    canvas_element->display = GuiDisplayIdFront;

    do {
        canvas_element->element_id = mg_json_get_str(element, "$.id");
        if(!canvas_element->element_id) break;

        int32_t temp_val = mg_json_get_long(element, "$.timeout", -1);
        canvas_element->timeout = (temp_val > 0) ? temp_val : 0;

        temp_val = mg_json_get_long(element, "$.x", -1);
        if(temp_val < 0) break;
        canvas_element->x = temp_val;

        temp_val = mg_json_get_long(element, "$.y", -1);
        if(temp_val < 0) break;
        canvas_element->y = temp_val;

        char* display_id_str = mg_json_get_str(element, "$.display");
        if(display_id_str) {
            if(strcmp(display_id_str, "front") == 0) {
                canvas_element->display = GuiDisplayIdFront;
            } else if(strcmp(display_id_str, "back") == 0) {
                canvas_element->display = GuiDisplayIdBack;
            } else {
                free(display_id_str);
                break;
            }
            free(display_id_str);
        }

        element_type = mg_json_get_str(element, "$.type");
        if(!element_type) break;
        if(strcmp(element_type, "image") == 0) {
            canvas_element->type = CanvasElementTypeImage;
            char* image_file = mg_json_get_str(element, "$.path");
            if(!image_file) break;
            canvas_element->image.file_path =
                furi_string_alloc_printf("%s/%s/%s", DISPLAY_ASSETS_DIR, app_id, image_file);

            free(image_file);
            success = true;
        } else if(strcmp(element_type, "text") == 0) {
            canvas_element->type = CanvasElementTypeText;
            canvas_element->text.text_str = mg_json_get_str(element, "$.text");
            if(!canvas_element->text.text_str) break;

            success = true;
        }
    } while(0);

    if(element_type) free(element_type);

    return success;
}

static bool
    api_display_draw_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(ctx);

    CanvasElementsArray_t elements_array;
    CanvasElementsArray_init(elements_array);

    char* app_id = NULL;
    bool success = false;
    do {
        app_id = mg_json_get_str(msg->body, "$.app_id");
        if(!app_id) break;

        struct mg_str elements_obj = mg_json_get_tok(msg->body, "$.elements");
        if(!elements_obj.buf) break;
        if(elements_obj.len < 2) break;
        if(elements_obj.buf[0] != '[') break;

        size_t offset = 0;
        struct mg_str element;
        while((offset = mg_json_next(elements_obj, offset, NULL, &element)) > 0) {
            success = api_display_draw_parse_element(elements_array, app_id, element);
            if(!success) break;
        }
    } while(0);

    if(success) {
        bool app_running = furi_record_exists(RECORD_CANVAS);
        if(!app_running) {
            Loader* loader = furi_record_open(RECORD_LOADER);
            FuriString* app_name = furi_string_alloc();
            bool loader_busy = false;
            if(loader_get_application_name(loader, app_name)) {
                if(furi_string_cmp(app_name, "Busy") == 0) {
                    loader_busy = true;
                }
            }
            furi_string_free(app_name);
            furi_record_close(RECORD_LOADER);

            if(loader_busy) {
                mg_http_reply(conn, 403, "", "Forbidden");
            } else {
                Desktop* desktop = furi_record_open(RECORD_DESKTOP);
                if(!desktop_replace_current_app(desktop, "canvas", "")) {
                    mg_http_reply(conn, 400, "", "Failed to load app");
                } else {
                    app_running = true;
                }
                furi_record_close(RECORD_DESKTOP);
            }
        }

        if(app_running) {
            CanvasApp* canvas = furi_record_open(RECORD_CANVAS);
            if(canvas_show_elements(canvas, app_id, elements_array)) {
                mg_http_reply(conn, 200, "", "OK");
            } else {
                mg_http_reply(conn, 400, "", "Bad Request");
            }
            furi_record_close(RECORD_CANVAS);
        }

    } else {
        mg_http_reply(conn, 400, "", "Bad Request");
    }

    CanvasElementsArray_clear(elements_array);
    if(app_id) free(app_id);
    return true;
}

static bool api_display_delete_callback(
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);
    FURI_LOG_I(TAG, "DELETE");

    FuriString* app_name = furi_string_alloc();
    Loader* loader = furi_record_open(RECORD_LOADER);
    if(loader_get_application_name(loader, app_name)) {
        if(furi_string_cmp(app_name, "Canvas") == 0) {
            loader_stop(loader);
        }
    }
    furi_record_close(RECORD_LOADER);
    mg_http_reply(conn, 200, "", "OK");
    furi_string_free(app_name);
    return true;
}

static const HttpHandler handlers_display[] = {
    {
        .uri = "/*/*/draw",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_display_draw_callback,
    },
    {
        .uri = "/*/*/draw",
        .method = "DELETE",
        .type = HttpHandlerCustom,
        .on_request = api_display_delete_callback,
    },
};

typedef struct {
    HttpHandlersList_t handlers;
} ApiDisplayCtx;

void* http_api_display_alloc(void) {
    ApiDisplayCtx* context = malloc(sizeof(ApiDisplayCtx));
    HttpHandlersList_init(context->handlers);

    for(size_t i = COUNT_OF(handlers_display); i > 0; i--) {
        http_handler_add(context->handlers, &handlers_display[i - 1]);
    }
    return context;
}

void http_api_display_free(void* ctx) {
    furi_assert(ctx);
    ApiDisplayCtx* context = ctx;
    HttpHandlersList_clear(context->handlers);
    free(context);
}

bool http_api_display_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    ApiDisplayCtx* context = ctx;
    return http_handle_request(context->handlers, conn, msg);
}
