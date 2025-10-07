#include "http_api.h"
#include <loader/loader.h>
#include <desktop/desktop.h>
#include <gui/gui.h>
#include <toolbox/path.h>
#include <canvas/canvas.h>
#include <back_display/back_display.h>
#include <front_display/front_display.h>

#define TAG "HttpDisplay"

#define DISPLAY_ASSETS_DIR EXT_PATH("assets")

#define DISPLAY_BRIGHTNESS_MAX (100)

static bool api_display_draw_parse_text_element(
    CanvasElement* canvas_element,
    const char* app_id,
    struct mg_str json_element) {
    UNUSED(app_id);
    bool result = false;
    do {
        canvas_element->type = CanvasElementTypeText;
        canvas_element->text.text_str = mg_json_get_str(json_element, "$.text");
        if(!canvas_element->text.text_str) break;

        canvas_element->text.font = GuiFontTiny5_8;
        canvas_element->text.color = (Color)COLOR_MAKE_HEXA(0xFFFFFFFF);

        char* font_name = mg_json_get_str(json_element, "$.font");
        if(font_name) {
            static const char* const font_names[GuiFontMax] = {
                [GuiFontArkNumeralsCondensed10] = "numerals_condensed_10",
                [GuiFontArkNumeralsRegular10] = "numerals_regular_10",
                [GuiFontArkNumeralsSmall10] = "numerals_small_10",
                [GuiFontArkRegular10] = "ark_regular_10",
                [GuiFontArkRegular20] = "ark_regular_20",
                [GuiFontCubic12] = "cubic_12",
                [GuiFontSomybmp7] = "sonybmp_7",
                [GuiFontTiny6] = "tiny_6",
                [GuiFontTiny5_8] = "tiny5_8",
            };
            canvas_element->text.font = GuiFontMax;
            for(GuiFont i = 0; i < GuiFontMax; i++) {
                if(strcmp(font_names[i], font_name) == 0) {
                    canvas_element->text.font = i;
                    break;
                }
            }
            free(font_name);
            if(canvas_element->text.font == GuiFontMax) break;
        }

        char* color_hex = mg_json_get_str(json_element, "$.color");
        if(color_hex) {
            bool color_parsed = color_parse_hexa_string(color_hex, &canvas_element->text.color);
            free(color_hex);
            if(!color_parsed) break;
        }

        result = true;
    } while(0);
    return result;
}

static bool api_display_draw_parse_image_element(
    CanvasElement* canvas_element,
    const char* app_id,
    struct mg_str json_element) {
    bool result = false;
    do {
        canvas_element->type = CanvasElementTypeImage;
        char* image_file = mg_json_get_str(json_element, "$.path");
        if(!image_file) break;
        canvas_element->image.file_path =
            furi_string_alloc_printf("%s/%s/%s", DISPLAY_ASSETS_DIR, app_id, image_file);

        free(image_file);
        result = true;
    } while(0);
    return result;
}

typedef bool (
    *ApiDisplayElementTypeParser)(CanvasElement*, const char* app_id, struct mg_str element);

typedef struct {
    const char* type;
    ApiDisplayElementTypeParser parser;
} ApiDisplayElementTypeAssoc;

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

        static const ApiDisplayElementTypeAssoc element_parsers[] = {
            {"text", api_display_draw_parse_text_element},
            {"image", api_display_draw_parse_image_element}};
        for(size_t i = 0; i < COUNT_OF(element_parsers); i++) {
            const ApiDisplayElementTypeAssoc* association = &element_parsers[i];
            if(strcmp(element_type, association->type) == 0) {
                success = association->parser(canvas_element, app_id, element);
                break;
            }
        }
    } while(0);

    if(element_type) free(element_type);

    return success;
}

static bool api_display_draw_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

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
                MG_REPLY_ERROR(conn, 423, "Loader is busy with another app");
            } else {
                Desktop* desktop = furi_record_open(RECORD_DESKTOP);
                if(!desktop_replace_current_app(desktop, "canvas", "")) {
                    MG_REPLY_ERROR(conn, 503, "Failed to load app");
                } else {
                    app_running = true;
                }
                furi_record_close(RECORD_DESKTOP);
            }
        }

        if(app_running) {
            CanvasApp* canvas = furi_record_open(RECORD_CANVAS);
            if(canvas_show_elements(canvas, app_id, elements_array)) {
                MG_REPLY_OK(conn);
            } else {
                MG_REPLY_BAD_REQUEST(conn);
            }
            furi_record_close(RECORD_CANVAS);
        }

    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    CanvasElementsArray_clear(elements_array);
    if(app_id) free(app_id);
    return true;
}

static bool api_display_delete_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FuriString* app_name = furi_string_alloc();
    Loader* loader = furi_record_open(RECORD_LOADER);
    if(loader_get_application_name(loader, app_name)) {
        if(furi_string_cmp(app_name, "Canvas") == 0) {
            loader_stop(loader);
        }
    }
    furi_record_close(RECORD_LOADER);
    MG_REPLY_OK(conn);
    furi_string_free(app_name);
    return true;
}

static bool api_display_get_brightness_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    FuriString* json_str = furi_string_alloc();

    FrontDisplaySrv* front_srv = furi_record_open(RECORD_FRONT_DISPLAY);
    uint8_t brightness_value = front_display_get_brightness_setting(front_srv);
    furi_record_close(RECORD_FRONT_DISPLAY);

    if(brightness_value == FRONT_DISPLAY_BRIGHTNESS_AUTO) {
        furi_string_cat_printf(json_str, "\"front\":\"auto\",");
    } else {
        furi_string_cat_printf(json_str, "\"front\":\"%u\",", brightness_value);
    }

    BackDisplaySrv* back_srv = furi_record_open(RECORD_BACK_DISPLAY);
    brightness_value = back_display_get_brightness(back_srv);
    furi_record_close(RECORD_BACK_DISPLAY);

    if(brightness_value == FRONT_DISPLAY_BRIGHTNESS_AUTO) {
        furi_string_cat_printf(json_str, "\"back\":\"auto\"");
    } else {
        furi_string_cat_printf(json_str, "\"back\":\"%u\"", brightness_value);
    }

    MG_REPLY_OK_BODY(conn, "{%s}\n", furi_string_get_cstr(json_str));
    furi_string_free(json_str);
    return true;
}

static bool api_display_set_brightness_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    bool success = false;
    do {
        if(msg->query.len == 0) break;

        char front_value_str[5];
        char back_value_str[5];

        int front_value = 0;
        int back_value = 0;

        int front_len =
            mg_http_get_var(&msg->query, "front", front_value_str, sizeof(front_value_str));
        int back_len =
            mg_http_get_var(&msg->query, "back", back_value_str, sizeof(back_value_str));

        if((front_len <= 0) && (back_len <= 0)) break;

        if(front_len > 0) {
            if(strcmp(front_value_str, "auto") == 0) {
                front_value = FRONT_DISPLAY_BRIGHTNESS_AUTO;
            } else if(sscanf(front_value_str, "%u", &front_value) == 1) {
                if(front_value > DISPLAY_BRIGHTNESS_MAX) break;
            } else {
                break;
            }
            FrontDisplaySrv* srv = furi_record_open(RECORD_FRONT_DISPLAY);
            front_display_set_brightness(srv, front_value);
            furi_record_close(RECORD_FRONT_DISPLAY);
        }

        if(back_len > 0) {
            if(strcmp(back_value_str, "auto") == 0) {
                back_value = BACK_DISPLAY_BRIGHTNESS_AUTO;
            } else if(sscanf(back_value_str, "%u", &back_value) == 1) {
                if(back_value > DISPLAY_BRIGHTNESS_MAX) break;
            } else {
                break;
            }
            BackDisplaySrv* srv = furi_record_open(RECORD_BACK_DISPLAY);
            back_display_set_brightness(srv, back_value);
            furi_record_close(RECORD_BACK_DISPLAY);
        }
        success = true;
    } while(0);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    return true;
}

static const HttpHandler handlers_display[] = {
    {
        .uri = "draw",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_display_draw_callback,
    },
    {
        .uri = "draw",
        .method = "DELETE",
        .type = HttpHandlerCustom,
        .on_request = api_display_delete_callback,
    },
    {
        .uri = "brightness",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = api_display_get_brightness_callback,
    },
    {
        .uri = "brightness",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_display_set_brightness_callback,
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

bool http_api_display_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiDisplayCtx* context = ctx;
    return http_handle_request(path, context->handlers, conn, msg);
}
