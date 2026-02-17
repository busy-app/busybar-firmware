#include "http_api.h"
#include <loader/loader.h>
#include <desktop/desktop.h>
#include <gui/gui.h>
#include <toolbox/path.h>
#include <toolbox/value_index.h>
#include <canvas/canvas.h>
#include <back_display/back_display.h>
#include <front_display/front_display.h>

#define TAG "HttpDisplay"

#define DISPLAY_ASSETS_DIR               EXT_PATH("assets")
#define DISPLAY_BUILTIN_IMAGES_FORMATTER EXT_PATH("apps_assets/%s/images/%s.bin")
#define DISPLAY_BUILTIN_ANIM_FORMATTER   EXT_PATH("apps_assets/%s/animations/%s.anim")

#define DISPLAY_BRIGHTNESS_MAX  (100)
#define DISPLAY_BRIGHTNESS_AUTO (FRONT_DISPLAY_BRIGHTNESS_AUTO)

#define BUILTIN_APP_PRIORITY     5
#define DEFAULT_ELEMENT_PRIORITY 6

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
        if(!font_name) break;
        static const char* const font_names[GuiFontMax] = {
            [GuiFontBf4x5] = "small",
            [GuiFontBf5x7] = "medium",
            [GuiFontBf5x7CondensedNumerals] = "medium_condensed",
            [GuiFontBf7x10] = "big",
        };
        size_t font = value_index_string(font_name, font_names, COUNT_OF(font_names));
        canvas_element->text.font = font;
        free(font_name);
        if(font == 0) break;

        char* color_hex = mg_json_get_str(json_element, "$.color");
        if(color_hex) {
            bool color_parsed = color_parse_hexa_string(color_hex, &canvas_element->text.color);
            free(color_hex);
            if(!color_parsed) break;
        }

        double number;
        if(mg_json_get_num(json_element, "$.width", &number)) {
            if(number < __DBL_EPSILON__) break; // <= 0
            canvas_element->text.width = (size_t)number;
        }

        if(mg_json_get_num(json_element, "$.scroll_rate", &number)) {
            if(number < -__DBL_EPSILON__) break; // < 0
            canvas_element->text.scroll_rate_cpm = (size_t)number;
        }

        result = true;
    } while(0);
    return result;
}

static bool api_display_draw_parse_countdown_element(
    CanvasElement* canvas_element,
    const char* app_id,
    struct mg_str json_element) {
    UNUSED(app_id);
    bool result = false;
    do {
        canvas_element->type = CanvasElementTypeCountdown;
        canvas_element->countdown.color = (Color)COLOR_MAKE_HEXA(0xFFFFFFFF);

        char* color_hex = mg_json_get_str(json_element, "$.color");
        if(color_hex) {
            bool color_parsed =
                color_parse_hexa_string(color_hex, &canvas_element->countdown.color);
            free(color_hex);
            if(!color_parsed) break;
        }

        // numeric representation in string: JS and mg_json have precision issues
        // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number/MAX_SAFE_INTEGER
        char* timestamp_str = mg_json_get_str(json_element, "$.timestamp");
        if(!timestamp_str) break;
        canvas_element->countdown.timestamp = atoll(timestamp_str);
        free(timestamp_str);

        char* direction_str = mg_json_get_str(json_element, "$.direction");
        if(!direction_str) break;
        static const char* const direction_lut[CountdownDirectionMAX] = {
            [CountdownDirectionTimeLeft] = "time_left",
            [CountdownDirectionTimeSince] = "time_since",
        };
        canvas_element->countdown.direction =
            value_index_string(direction_str, direction_lut, COUNT_OF(direction_lut));
        free(direction_str);

        char* hours_str = mg_json_get_str(json_element, "$.show_hours");
        if(!hours_str) break;
        static const char* const hours_lut[CountdownShowHourMAX] = {
            [CountdownShowHourWhenNonZero] = "when_non_zero",
            [CountdownShowHourAlways] = "always",
        };
        canvas_element->countdown.hours =
            value_index_string(hours_str, hours_lut, COUNT_OF(hours_lut));
        free(hours_str);

        result = true;
    } while(0);
    return result;
}

static bool api_display_draw_parse_image_path(
    FuriString** file_path,
    const char* app_id,
    struct mg_str json_element,
    CanvasElementType type) {
    furi_check((type == CanvasElementTypeImage) || (type == CanvasElementTypeAnimPlayer));
    bool is_animated = type == CanvasElementTypeAnimPlayer;

    bool result = false;

    char* uploaded = mg_json_get_str(json_element, "$.path");
    char* builtin =
        mg_json_get_str(json_element, is_animated ? "$.builtin_anim" : "$.builtin_image");

    do {
        if(uploaded && builtin) break;

        if(uploaded) {
            *file_path =
                furi_string_alloc_printf("%s/%s/%s", DISPLAY_ASSETS_DIR, app_id, uploaded);

            result = true;
            break;
        }

        if(builtin) {
            char* app_name = builtin;
            char* image_name = NULL;

            for(char* c = builtin; *c != 0; c++) {
                if(*c == '/') {
                    *c = '\0';
                    image_name = c + 1;
                }
            }

            if(!image_name) break;

            *file_path = furi_string_alloc_printf(
                is_animated ? DISPLAY_BUILTIN_ANIM_FORMATTER : DISPLAY_BUILTIN_IMAGES_FORMATTER,
                app_name,
                image_name);
            result = true;
            break;
        }
    } while(0);

    free(uploaded);
    free(builtin);
    return result;
}

static bool api_display_draw_parse_image_element(
    CanvasElement* canvas_element,
    const char* app_id,
    struct mg_str json_element) {
    bool result = false;

    do {
        canvas_element->type = CanvasElementTypeImage;
        if(!api_display_draw_parse_image_path(
               &canvas_element->image.file_path, app_id, json_element, canvas_element->type))
            break;

        result = true;
    } while(0);

    return result;
}

static bool api_display_draw_parse_anim_player_element(
    CanvasElement* canvas_element,
    const char* app_id,
    struct mg_str json_element) {
    bool result = false;

    do {
        if(!api_display_draw_parse_image_path(
               &canvas_element->anim_player.file_path,
               app_id,
               json_element,
               CanvasElementTypeAnimPlayer))
            break;

        bool json_bool;
        char* json_str;

        if((json_str = mg_json_get_str(json_element, "$.section"))) {
            canvas_element->anim_player.section = furi_string_alloc_set_str(json_str);
        } else {
            canvas_element->anim_player.section =
                furi_string_alloc_set_str(ANIM_FILE_DEFAULT_SECTION);
        }

        canvas_element->anim_player.flags = AnimFilePlayFlagNone;

        if(mg_json_get_bool(json_element, "$.loop", &json_bool)) {
            if(json_bool) canvas_element->anim_player.flags |= AnimFilePlayFlagLoop;
        }
        if(mg_json_get_bool(json_element, "$.await_previous_end", &json_bool)) {
            if(json_bool) canvas_element->anim_player.flags |= AnimFilePlayFlagFinishCurrent;
        }

        canvas_element->type = CanvasElementTypeAnimPlayer;
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
        canvas_element->id = mg_json_get_str(element, "$.id");
        if(!canvas_element->id) break;

        int32_t temp_val = mg_json_get_long(element, "$.timeout", -1);
        canvas_element->timeout = (temp_val > 0) ? temp_val : 0;

        char* disp_until = mg_json_get_str(element, "$.display_until");
        if(disp_until) {
            canvas_element->display_until = atoll(disp_until);
            free(disp_until);
        }

        if((canvas_element->timeout > 0) && (canvas_element->display_until > 0)) break;

        canvas_element->x = mg_json_get_long(element, "$.x", 0);
        canvas_element->y = mg_json_get_long(element, "$.y", 0);

        char* alignment = mg_json_get_str(element, "$.align");
        if(alignment) {
            static const char* const alignments[AlignMax] = {
                [AlignTopLeft] = "top_left",
                [AlignTopMid] = "top_mid",
                [AlignTopRight] = "top_right",
                [AlignLeftMid] = "mid_left",
                [AlignCenter] = "center",
                [AlignRightMid] = "mid_right",
                [AlignBottomLeft] = "bottom_left",
                [AlignBottomMid] = "bottom_mid",
                [AlignBottomRight] = "bottom_right",
            };
            size_t align = value_index_string(alignment, alignments, COUNT_OF(alignments));
            canvas_element->align = align;
            free(alignment);
            if(align == 0) break;
        } else {
            canvas_element->align = AlignDefault;
        }

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
            {"image", api_display_draw_parse_image_element},
            {"anim", api_display_draw_parse_anim_player_element},
            {"countdown", api_display_draw_parse_countdown_element},
        };
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

static int api_display_active_priority(void) {
    int priority = INT_MIN;

    Loader* loader = furi_record_open(RECORD_LOADER);
    FuriString* app_name = furi_string_alloc();

    do {
        bool canvas_running = furi_record_exists(RECORD_CANVAS);
        if(canvas_running) {
            CanvasApp* canvas = furi_record_open(RECORD_CANVAS);
            priority = MAX(priority, canvas_active_priority(canvas));
            furi_record_close(RECORD_CANVAS);
        }

        if(!loader_get_application_name(loader, app_name)) break;
        if(furi_string_search_str(app_name, "Settings") != FURI_STRING_FAILURE) break;
        if(furi_string_cmp_str(app_name, "Software Power Off") == 0) break;
        priority = MAX(priority, BUILTIN_APP_PRIORITY);
    } while(0);

    furi_string_free(app_name);
    furi_record_close(RECORD_LOADER);

    return priority;
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
    double json_num = 0;
    int priority = DEFAULT_ELEMENT_PRIORITY;

    do {
        app_id = mg_json_get_str(msg->body, "$.app_id");
        if(!app_id) break;

        if(mg_json_get_num(msg->body, "$.priority", &json_num)) {
            priority = json_num;
        }
        if(priority <= 0) break;
        if(priority > 10) break;

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

        if(!success) {
            MG_REPLY_BAD_REQUEST(conn);
            break;
        }

        int active_priority = api_display_active_priority();
        if(priority < active_priority) {
            MG_REPLY_ERROR(conn, 409, "Not drawn due to low priority");
            break;
        }

        bool canvas_running = furi_record_exists(RECORD_CANVAS);
        if(!canvas_running) {
            Desktop* desktop = furi_record_open(RECORD_DESKTOP);
            if(desktop_replace_current_app(desktop, "canvas", "")) {
                canvas_running = true;
            } else {
                MG_REPLY_ERROR(conn, 503, "Failed to load canvas app");
            }
            furi_record_close(RECORD_DESKTOP);
            if(!canvas_running) break;
        }

        CanvasApp* canvas = furi_record_open(RECORD_CANVAS);
        if(!canvas_show_elements(canvas, app_id, priority, elements_array)) {
            MG_REPLY_BAD_REQUEST(conn);
        }
        furi_record_close(RECORD_CANVAS);

        MG_REPLY_OK(conn);
    } while(0);

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

    char app_id_buf[64];
    int app_id_len = mg_http_get_var(&msg->query, "app_id", app_id_buf, sizeof(app_id_buf));
    const char* app_id = (app_id_len >= 1) ? app_id_buf : NULL;

    FuriString* app_name = furi_string_alloc();
    Loader* loader = furi_record_open(RECORD_LOADER);

    if(loader_get_application_name(loader, app_name)) {
        if(furi_string_cmp(app_name, "Canvas") == 0) {
            CanvasApp* canvas = furi_record_open(RECORD_CANVAS);
            canvas_delete_elements(canvas, app_id);
            furi_record_close(RECORD_CANVAS);
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
    uint8_t brightness_front = front_display_get_brightness_setting(front_srv);
    furi_record_close(RECORD_FRONT_DISPLAY);

    BackDisplaySrv* back_srv = furi_record_open(RECORD_BACK_DISPLAY);
    uint8_t brightness_back = back_display_get_brightness(back_srv);
    if(brightness_front != brightness_back) {
        back_display_set_brightness(back_srv, brightness_front);
    }
    furi_record_close(RECORD_BACK_DISPLAY);

    if(brightness_front == DISPLAY_BRIGHTNESS_AUTO) {
        furi_string_cat_printf(json_str, "\"value\":\"auto\"");
    } else {
        furi_string_cat_printf(json_str, "\"value\":\"%u\"", brightness_front);
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

        char value_str[5];
        int brightness_value = 0;

        int value_len = mg_http_get_var(&msg->query, "value", value_str, sizeof(value_str));

        if(value_len <= 0) break;

        if(strcmp(value_str, "auto") == 0) {
            brightness_value = DISPLAY_BRIGHTNESS_AUTO;
        } else if(sscanf(value_str, "%u", &brightness_value) == 1) {
            if(brightness_value > DISPLAY_BRIGHTNESS_MAX) break;
        } else {
            break;
        }
        FrontDisplaySrv* srv = furi_record_open(RECORD_FRONT_DISPLAY);
        BackDisplaySrv* back_srv = furi_record_open(RECORD_BACK_DISPLAY);

        front_display_set_brightness(srv, brightness_value);
        back_display_set_brightness(back_srv, brightness_value);

        furi_record_close(RECORD_FRONT_DISPLAY);
        furi_record_close(RECORD_BACK_DISPLAY);

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
