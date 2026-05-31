#include "http_api.h"
#include <gui/gui.h>
#include <toolbox/path.h>
#include <toolbox/value_index.h>
#include <canvas/canvas.h>
#include <back_display/back_display.h>
#include <front_display/front_display.h>
#include <furi_hal_rtc.h>
#include <font_registry/fonts.h>
#include <brightness_control/brightness_control.h>

#define TAG "HttpDisplay"

#define DISPLAY_ASSETS_DIR           EXT_PATH("user_assets")
#define DISPLAY_API_DEFAULT_PRIORITY (50)

static bool api_display_draw_parse_text_element(
    CanvasElement* canvas_element,
    const char* app_name,
    struct mg_str json_element) {
    UNUSED(app_name);
    bool result = false;
    do {
        canvas_element->type = CanvasElementTypeText;
        canvas_element->text.text_str = mg_json_get_str(json_element, "$.text");
        if(!canvas_element->text.text_str) break;

        canvas_element->text.color = (Color)COLOR_MAKE_HEXA(0xFFFFFFFF);

        char* font_name = mg_json_get_str(json_element, "$.font");
        if(!font_name) break;

        static const char* const font_names[] = {
            "tiny",
            "small",
            "normal",
            "condensed",
            "bold",
            "large",
            "extra_large",
            "global",
        };

        static const char* const font_paths[] = {
            FONT_BUSY_TINY,
            FONT_BUSY_REGULAR_5,
            FONT_BUSY_REGULAR_7,
            FONT_BUSY_CONDENSED_7,
            FONT_BUSY_BOLD_7,
            FONT_BUSY_REGULAR_9,
            FONT_BUSY_BOLD_10,
            FONT_LANA_PIXEL_REGULAR_11,
        };

        const char* font_path =
            value_index_map_string(font_names, font_paths, COUNT_OF(font_names), font_name);
        if(strcmp(font_path, font_paths[0]) == 0 && strcmp(font_name, font_names[0]) != 0) {
            // Unknown font name mapped to default — reject
            free(font_name);
            break;
        }
        canvas_element->text.font_path = strdup(font_path);
        free(font_name);

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
    const char* app_name,
    struct mg_str json_element) {
    UNUSED(app_name);
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
    const char* app_name,
    struct mg_str json_element,
    CanvasElementType type) {
    furi_check((type == CanvasElementTypeImage) || (type == CanvasElementTypeAnimPlayer));
    bool is_animated = type == CanvasElementTypeAnimPlayer;

    bool result = false;

    char* uploaded = mg_json_get_str(json_element, "$.path");
    char* stock = mg_json_get_str(json_element, "$.stock_path");

    do {
        if(uploaded && stock) break;

        if(uploaded) {
            *file_path =
                furi_string_alloc_printf("%s/%s/%s", DISPLAY_ASSETS_DIR, app_name, uploaded);
            if(!mg_path_is_sane(mg_str(furi_string_get_cstr(*file_path)))) {
                furi_string_free(*file_path);
                *file_path = NULL;
                break;
            }
            result = true;
            break;
        }

        if(stock) {
            char* image_name = NULL;

            for(char* c = stock; *c != 0; c++) {
                if(*c == '/') {
                    *c = '\0';
                    image_name = c + 1;
                }
            }

            if(!image_name || *image_name == '\0') break;

            *file_path = furi_string_alloc_printf(
                is_animated ? SHARED_ANIM_PATH("%s") : SHARED_IMG_PATH("%s"), image_name);
            result = true;
            break;
        }
    } while(0);

    if(uploaded) free(uploaded);
    if(stock) free(stock);
    return result;
}

static bool api_display_draw_parse_image_element(
    CanvasElement* canvas_element,
    const char* app_name,
    struct mg_str json_element) {
    bool result = false;

    do {
        canvas_element->type = CanvasElementTypeImage;
        if(!api_display_draw_parse_image_path(
               &canvas_element->image.file_path, app_name, json_element, canvas_element->type))
            break;

        result = true;
    } while(0);

    return result;
}

static bool api_display_draw_parse_anim_player_element(
    CanvasElement* canvas_element,
    const char* app_name,
    struct mg_str json_element) {
    bool result = false;

    do {
        if(!api_display_draw_parse_image_path(
               &canvas_element->anim_player.file_path,
               app_name,
               json_element,
               CanvasElementTypeAnimPlayer))
            break;

        bool json_bool;
        char* json_str;

        if((json_str = mg_json_get_str(json_element, "$.section"))) {
            canvas_element->anim_player.section = furi_string_alloc_set_str(json_str);
            free(json_str);
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
    *ApiDisplayElementTypeParser)(CanvasElement*, const char* app_name, struct mg_str element);

typedef struct {
    const char* type;
    ApiDisplayElementTypeParser parser;
} ApiDisplayElementTypeAssoc;

static bool api_display_draw_parse_element(
    CanvasElementsArray_t elements_array,
    char* app_name,
    struct mg_str element) {
    bool success = false;
    char* element_type = NULL;
    CanvasElement* canvas_element = CanvasElementsArray_push_new(elements_array);

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

        canvas_element->display = GuiDisplayIdFront;
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
            {"animation", api_display_draw_parse_anim_player_element},
            {"countdown", api_display_draw_parse_countdown_element},
        };
        for(size_t i = 0; i < COUNT_OF(element_parsers); i++) {
            const ApiDisplayElementTypeAssoc* association = &element_parsers[i];
            if(strcmp(element_type, association->type) == 0) {
                success = association->parser(canvas_element, app_name, element);
                break;
            }
        }
    } while(0);

    if(element_type) free(element_type);

    return success;
}

static const struct {
    uint32_t code;
    const char* message;
} draw_errors[CanvasResultMax] = {
    [CanvasResultOk] = {0, NULL},
    [CanvasResultBadParameters] = {400, "Bad request"},
    [CanvasResultLowPriority] = {409, "Not drawn due to low priority"},
    [CanvasResultEmptyScreen] = {400, "Nothing to display"},
};
_Static_assert(
    COUNT_OF(draw_errors) == CanvasResultMax,
    "draw_errors table must cover all CanvasResult values");

static void api_display_canvas_draw(struct mg_connection* conn, struct mg_http_message* msg) {
    CanvasElementsArray_t elements_array;
    CanvasElementsArray_init(elements_array);

    char* app_name = NULL;
    bool success = false;
    double json_num = 0;
    int priority = DISPLAY_API_DEFAULT_PRIORITY;

    do {
        app_name = mg_json_get_str(msg->body, "$.application_name");
        if(!app_name) {
            MG_REPLY_ERROR(conn, 400, "Missing application_name");
            break;
        }

        if(mg_json_get_num(msg->body, "$.priority", &json_num)) {
            priority = json_num;
        }
        if(priority <= 0) {
            MG_REPLY_ERROR(conn, 400, "Priority must be >= 1");
            break;
        }
        if((size_t)priority > CANVAS_MAX_PRIORITY) {
            MG_REPLY_ERROR(conn, 400, "Priority must be <= 100");
            break;
        }

        struct mg_str elements_obj = mg_json_get_tok(msg->body, "$.elements");
        if(!elements_obj.buf || elements_obj.len < 2 || elements_obj.buf[0] != '[') {
            MG_REPLY_ERROR(conn, 400, "Missing or invalid elements array");
            break;
        }

        size_t offset = 0;
        struct mg_str element;
        success = true;
        while((offset = mg_json_next(elements_obj, offset, NULL, &element)) > 0) {
            success = api_display_draw_parse_element(elements_array, app_name, element);
            if(!success) break;
        }

        if(!success) {
            MG_REPLY_BAD_REQUEST(conn);
            break;
        }

        if(CanvasElementsArray_size(elements_array) == 0) {
            MG_REPLY_ERROR(conn, 400, "Elements array is empty");
            break;
        }

        CanvasSrv* canvas = furi_record_open(RECORD_CANVAS);
        CanvasResult result = canvas_show_elements(canvas, app_name, priority, elements_array);
        furi_record_close(RECORD_CANVAS);

        if(result != CanvasResultOk) {
            furi_assert(result < CanvasResultMax);
            MG_REPLY_ERROR(conn, draw_errors[result].code, draw_errors[result].message);
            break;
        }

        MG_REPLY_OK(conn);
    } while(0);

    CanvasElementsArray_clear(elements_array);
    if(app_name) free(app_name);
}

static void api_display_canvas_clear(struct mg_connection* conn, struct mg_http_message* msg) {
    char app_name_buf[64];
    int app_name_len =
        mg_http_get_var(&msg->query, "application_name", app_name_buf, sizeof(app_name_buf));
    const char* app_name = (app_name_len >= 1) ? app_name_buf : NULL;

    CanvasSrv* canvas = furi_record_open(RECORD_CANVAS);
    CanvasResult res = canvas_delete_elements(canvas, app_name);
    furi_record_close(RECORD_CANVAS);

    if(res == CanvasResultOk) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }
}

static bool api_display_draw_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    if(method == HttpMethodPost) {
        api_display_canvas_draw(conn, msg);
    } else if(method == HttpMethodDelete) {
        api_display_canvas_clear(conn, msg);
    }

    return true;
}

static void api_display_get_brightness(struct mg_connection* conn, struct mg_http_message* msg) {
    UNUSED(msg);

    FuriString* json_str = furi_string_alloc();

    BrightnessControl* brightness_ctrl = furi_record_open(RECORD_BRIGHTNESS_CONTROL);
    FuriState* fstate = brightness_control_get_state(brightness_ctrl);
    BrightnessControlState state;
    furi_state_get(fstate, &state);

    if(state.mode == BrightnessControlBrightnessModeAuto) {
        furi_string_cat_printf(json_str, "\"value\":\"auto\"");
    } else {
        furi_string_cat_printf(json_str, "\"value\":\"%hhu\"", state.brightness_setting);
    }

    MG_REPLY_OK_BODY(conn, "{%s}\n", furi_string_get_cstr(json_str));
    furi_string_free(json_str);
}

static void api_display_set_brightness(struct mg_connection* conn, struct mg_http_message* msg) {
    bool success = false;
    do {
        if(msg->query.len == 0) break;

        char value_str[5];
        int brightness_value = 0;
        bool is_auto = false;

        int value_len = mg_http_get_var(&msg->query, "value", value_str, sizeof(value_str));

        if(value_len <= 0) break;

        if(strcmp(value_str, "auto") == 0) {
            is_auto = true;
        } else if(!mg_str_to_num(
                      mg_str_n(value_str, value_len),
                      10,
                      &brightness_value,
                      sizeof(brightness_value))) {
            break;
        } else if(brightness_value < BRIGHTNESS_MIN || brightness_value > BRIGHTNESS_MAX) {
            break;
        }

        BrightnessControl* srv = furi_record_open(RECORD_BRIGHTNESS_CONTROL);

        if(is_auto) {
            brightness_control_set_auto_brightness(srv);
        } else {
            brightness_control_set_manual_brightness(srv, brightness_value);
        }
        success = true;

        furi_record_close(RECORD_BRIGHTNESS_CONTROL);

    } while(0);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }
}

static bool api_display_brightness_callback(
    FuriString* path,
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    if(method == HttpMethodGet) {
        api_display_get_brightness(conn, msg);
    } else if(method == HttpMethodPost) {
        api_display_set_brightness(conn, msg);
    }

    return true;
}

static const HttpHandler handlers_display[] = {
    {
        .uri = "draw",
        .method = HttpMethodPost | HttpMethodDelete,
        .type = HttpHandlerCustom,
        .on_request = api_display_draw_callback,
    },
    {
        .uri = "brightness",
        .method = HttpMethodGet | HttpMethodPost,
        .type = HttpHandlerCustom,
        .on_request = api_display_brightness_callback,
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
    HttpMethod method,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiDisplayCtx* context = ctx;
    return http_handle_request(path, method, context->handlers, conn, msg);
}
