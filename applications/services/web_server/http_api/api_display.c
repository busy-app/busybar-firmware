#include "http_api.h"
#include <gui/gui.h>
#include <xpm/xpm.h>
#include <toolbox/path.h>
#include <toolbox/value_index.h>
#include <canvas/canvas.h>
#include <back_display/back_display.h>
#include <front_display/front_display.h>
#include <furi_hal_rtc.h>
#include <font_registry/fonts.h>
#include <brightness_control/brightness_control.h>
#include <status_lights/status_lights.h>
#include <lvgl.h>
#include <cjson/cJSON.h>

#define TAG "HttpDisplay"

#define DISPLAY_ASSETS_DIR           EXT_PATH("user_assets")
#define DISPLAY_API_DEFAULT_PRIORITY (50)

#define XPM_API_MAX_COLORS_COUNT    32u
#define XPM_API_MAX_CHARS_PER_PIXEL 4u

#define FIELD_MISSING_ERROR(field_name)            ".%s is required", (field_name)
#define FIELD_XOR_ERROR(field_names)               ".{%s}: exactly one of these must be present", (field_names)
#define FIELD_INVALID_ERROR_NO_DETAILS(field_name) ".%s value is invalid", (field_name)
#define FIELD_INVALID_ERROR(field_name, expectation) \
    ".%s value is invalid; must %s", (field_name), (expectation)

static bool api_display_draw_parse_text_element(
    CanvasElement* canvas_element,
    const char* app_name,
    struct mg_str json_element,
    FuriString* error) {
    UNUSED(app_name);

    bool result = false;
    do {
        canvas_element->type = CanvasElementTypeText;
        canvas_element->text.text_str = mg_json_get_str(json_element, "$.text");
        if(!canvas_element->text.text_str) {
            furi_string_cat_printf(error, FIELD_MISSING_ERROR("text"));
            break;
        }

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
            "superscript",
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
            FONT_BUSY_SUPERSCRIPT_7,
        };

        const char* font_path =
            value_index_map_string(font_names, font_paths, COUNT_OF(font_names), font_name);
        if(!font_path) {
            furi_string_cat_printf(error, FIELD_INVALID_ERROR_NO_DETAILS("font"));
            free(font_name);
            break;
        }
        if(strcmp(font_path, font_paths[0]) == 0 && strcmp(font_name, font_names[0]) != 0) {
            // Unknown font name mapped to default — reject
            furi_string_cat_printf(error, FIELD_INVALID_ERROR_NO_DETAILS("font"));
            free(font_name);
            break;
        }
        canvas_element->text.font_path = strdup(font_path);
        free(font_name);

        char* color_hex = mg_json_get_str(json_element, "$.color");
        if(color_hex) {
            bool color_parsed = color_parse_hexa_string(color_hex, &canvas_element->text.color);
            free(color_hex);
            if(!color_parsed) {
                furi_string_cat_printf(
                    error, FIELD_INVALID_ERROR("color", "be in #RRGGBBAA format"));
                break;
            }
        }

        double number;
        if(mg_json_get_num(json_element, "$.width", &number)) {
            if(number < __DBL_EPSILON__) { // <= 0
                furi_string_cat_printf(error, FIELD_INVALID_ERROR("width", "be >= 1"));
                break;
            }
            canvas_element->text.width = (size_t)number;
        }

        if(mg_json_get_num(json_element, "$.scroll_rate", &number)) {
            if(number < -__DBL_EPSILON__) { // < 0
                furi_string_cat_printf(error, FIELD_INVALID_ERROR("scroll_rate", "be >= 0"));
                break;
            }
            canvas_element->text.scroll_rate_cpm = (size_t)number;
        }

        if(mg_json_get_num(json_element, "$.scroll_start_delay", &number)) {
            if(number < -__DBL_EPSILON__) { // < 0
                furi_string_cat_printf(
                    error, FIELD_INVALID_ERROR("scroll_start_delay", "be >= 0"));
                break;
            }
            canvas_element->text.scroll_start_delay = (size_t)number;
        }

        if(mg_json_get_num(json_element, "$.scroll_repeat_delay", &number)) {
            if(number < -__DBL_EPSILON__) { // < 0
                furi_string_cat_printf(
                    error, FIELD_INVALID_ERROR("scroll_repeat_delay", "be >= 0"));
                break;
            }
            canvas_element->text.scroll_repeat_delay = (size_t)number;
        }

        result = true;
    } while(0);
    return result;
}

static bool api_display_draw_parse_countdown_element(
    CanvasElement* canvas_element,
    const char* app_name,
    struct mg_str json_element,
    FuriString* error) {
    UNUSED(app_name);
    UNUSED(error);

    bool result = false;
    do {
        canvas_element->type = CanvasElementTypeCountdown;
        canvas_element->countdown.color = (Color)COLOR_MAKE_HEXA(0xFFFFFFFF);

        char* color_hex = mg_json_get_str(json_element, "$.color");
        if(color_hex) {
            bool color_parsed =
                color_parse_hexa_string(color_hex, &canvas_element->countdown.color);
            free(color_hex);
            if(!color_parsed) {
                furi_string_cat_printf(
                    error, FIELD_INVALID_ERROR("color", "be in #RRGGBBAA format"));
                break;
            }
        }

        // numeric representation in string: JS and mg_json have precision issues
        // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number/MAX_SAFE_INTEGER
        char* timestamp_str = mg_json_get_str(json_element, "$.timestamp");
        if(!timestamp_str) {
            furi_string_cat_printf(error, FIELD_MISSING_ERROR("timestamp"));
            break;
        }
        canvas_element->countdown.timestamp = atoll(timestamp_str);
        free(timestamp_str);

        char* direction_str = mg_json_get_str(json_element, "$.direction");
        if(!direction_str) {
            furi_string_cat_printf(error, FIELD_MISSING_ERROR("direction"));
            break;
        }
        static const char* const direction_lut[CountdownDirectionMAX] = {
            [CountdownDirectionTimeLeft] = "time_left",
            [CountdownDirectionTimeSince] = "time_since",
        };
        size_t direction_temp =
            value_index_string(direction_str, direction_lut, COUNT_OF(direction_lut));
        free(direction_str);
        if(direction_temp >= CountdownDirectionMAX) {
            furi_string_cat_printf(error, FIELD_INVALID_ERROR("direction", "be one of: "));
            for(size_t i = 0; i < COUNT_OF(direction_lut); i++) {
                furi_string_cat_printf(error, "\"%s\"", direction_lut[i]);
                if(i != COUNT_OF(direction_lut) - 1) {
                    furi_string_cat_str(error, ", ");
                }
            }
            break;
        }
        canvas_element->countdown.direction = direction_temp;

        char* hours_str = mg_json_get_str(json_element, "$.show_hours");
        if(!hours_str) {
            furi_string_cat_printf(error, FIELD_MISSING_ERROR("show_hours"));
            break;
        }
        static const char* const hours_lut[CountdownShowHourMAX] = {
            [CountdownShowHourWhenNonZero] = "when_non_zero",
            [CountdownShowHourAlways] = "always",
        };
        size_t hours_temp = value_index_string(hours_str, hours_lut, COUNT_OF(hours_lut));
        free(hours_str);
        if(hours_temp >= CountdownShowHourMAX) {
            furi_string_cat_printf(error, FIELD_INVALID_ERROR("show_hours", "be one of:"));
            for(size_t i = 0; i < COUNT_OF(hours_lut); i++) {
                furi_string_cat_printf(error, "\"%s\"", hours_lut[i]);
                if(i != COUNT_OF(hours_lut) - 1) {
                    furi_string_cat_str(error, ", ");
                }
            }
            break;
        }
        canvas_element->countdown.hours = hours_temp;

        result = true;
    } while(0);
    return result;
}

static bool api_display_draw_parse_image_path(
    FuriString** file_path,
    const char* app_name,
    struct mg_str json_element,
    CanvasElementType type,
    FuriString* error) {
    furi_check((type == CanvasElementTypeImage) || (type == CanvasElementTypeAnimPlayer));
    bool is_animated = type == CanvasElementTypeAnimPlayer;

    bool result = false;

    char* uploaded = mg_json_get_str(json_element, "$.path");
    char* stock = mg_json_get_str(json_element, "$.stock_path");

    do {
        if(uploaded && stock) {
            furi_string_cat_printf(error, FIELD_XOR_ERROR("path,stock_path"));
            break;
        }

        if(uploaded) {
            *file_path =
                furi_string_alloc_printf("%s/%s/%s", DISPLAY_ASSETS_DIR, app_name, uploaded);
            if(!mg_path_is_sane(mg_str(furi_string_get_cstr(*file_path)))) {
                furi_string_free(*file_path);
                *file_path = NULL;
                furi_string_cat_printf(
                    error, FIELD_INVALID_ERROR("path", "be clean, without path escape sequences"));
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

            if(!image_name || *image_name == '\0') {
                furi_string_cat_printf(error, FIELD_INVALID_ERROR_NO_DETAILS("stock_path"));
                break;
            }

            *file_path = furi_string_alloc_printf(
                is_animated ? SHARED_ANIM_PATH("%s") : SHARED_IMG_PATH("%s"), image_name);
            result = true;
            break;
        }

        furi_string_cat_printf(error, FIELD_XOR_ERROR("path,stock_path"));
    } while(0);

    if(uploaded) free(uploaded);
    if(stock) free(stock);
    return result;
}

static bool
    api_display_validate_image(const char* file_path, GuiDisplayId display, FuriString* error) {
    lv_image_header_t header;
    if(lv_image_decoder_get_info(file_path, &header) != LV_RESULT_OK) {
        furi_string_cat_printf(error, ": failed to decode image %s", file_path);
        return false;
    }

    const GuiDisplayParameters* display_parameters = gui_display_get_parameters(display);

    if(header.w > display_parameters->width || header.h > display_parameters->height) {
        furi_string_cat_printf(
            error,
            ": image %s exceeds display dimensions %zux%zu.",
            file_path,
            display_parameters->width,
            display_parameters->height);
        return false;
    }

    return true;
}

static bool api_display_draw_parse_image_element(
    CanvasElement* canvas_element,
    const char* app_name,
    struct mg_str json_element,
    FuriString* error) {
    bool result = false;

    do {
        canvas_element->type = CanvasElementTypeImage;

        long opacity = mg_json_get_long(json_element, "$.opacity", 100);
        if(opacity < 0 || opacity > 100) {
            furi_string_cat_printf(error, FIELD_INVALID_ERROR("opacity", "be in range [0; 100]"));
            break;
        }
        canvas_element->image.opacity = opacity * 255 / 100;

        if(!api_display_draw_parse_image_path(
               &canvas_element->image.file_path,
               app_name,
               json_element,
               canvas_element->type,
               error))
            break;

        if(!api_display_validate_image(
               furi_string_get_cstr(canvas_element->image.file_path),
               canvas_element->display,
               error))
            break;

        result = true;
    } while(0);

    return result;
}

static bool api_display_draw_parse_xpm_element(
    CanvasElement* canvas_element,
    const char* app_name,
    struct mg_str json_element,
    FuriString* error) {
    UNUSED(app_name);

    bool result = false;

    char* data_str = NULL;
    Xpm* xpm = NULL;

    do {
        long opacity = mg_json_get_long(json_element, "$.opacity", 100);
        if(opacity < 0 || opacity > 100) {
            furi_string_cat_printf(error, FIELD_INVALID_ERROR("opacity", "be in range [0; 100]"));
            break;
        }

        data_str = mg_json_get_str(json_element, "$.data");
        if(!data_str) {
            furi_string_cat_printf(error, FIELD_MISSING_ERROR("data"));
            break;
        }

        xpm = xpm_alloc(data_str);
        if(!xpm_decode_header(xpm)) {
            furi_string_cat_printf(error, FIELD_INVALID_ERROR("data", "have valid XPM header"));
            break;
        }

        XpmHeaderData header = xpm_get_header_data(xpm);
        if(header.colors_count > XPM_API_MAX_COLORS_COUNT ||
           header.chars_per_pixel > XPM_API_MAX_CHARS_PER_PIXEL) {
            furi_string_cat_printf(
                error, FIELD_INVALID_ERROR("data", "have <= 32 colors and <= 4 chars per pixel"));
            break;
        }

        const GuiDisplayParameters* display_parameters =
            gui_display_get_parameters(canvas_element->display);
        if(header.width > display_parameters->width ||
           header.height > display_parameters->height) {
            furi_string_cat_printf(
                error, FIELD_INVALID_ERROR("data", "be smaller than the display"));
            furi_string_cat_printf(
                error, " (%zux%zu)", display_parameters->width, display_parameters->height);
            break;
        }

        if(!xpm_decode_colors(xpm)) {
            furi_string_cat_printf(
                error, FIELD_INVALID_ERROR("data", "have valid XPM color table"));
            break;
        }

        XpmPixelFormat xpm_format;
        ImageColorFormat image_format;
        if(canvas_element->display == GuiDisplayIdFront) {
            xpm_format = XpmPixelFormatBGRA8888;
            image_format = ImageColorFormatBGRA8888;
        } else {
            xpm_format = XpmPixelFormatLA88;
            image_format = ImageColorFormatLA88;
        }

        size_t pixels_buffer_size = 0;
        void* pixels = xpm_decode_pixels(xpm, xpm_format, &pixels_buffer_size);
        if(!pixels) {
            furi_string_cat_printf(
                error, FIELD_INVALID_ERROR("data", "have valid XPM pixel data"));
            break;
        }

        canvas_element->type = CanvasElementTypeRawImage;
        canvas_element->raw_image.data = pixels;
        canvas_element->raw_image.data_size = pixels_buffer_size;
        canvas_element->raw_image.width = header.width;
        canvas_element->raw_image.height = header.height;
        canvas_element->raw_image.format = image_format;
        canvas_element->raw_image.opacity = opacity * 255 / 100;

        result = true;
    } while(0);

    if(data_str) {
        free(data_str);
    }

    if(xpm) {
        xpm_free(xpm);
    }

    return result;
}

static bool api_display_draw_parse_anim_player_element(
    CanvasElement* canvas_element,
    const char* app_name,
    struct mg_str json_element,
    FuriString* error) {
    UNUSED(error);
    bool result = false;

    do {
        canvas_element->type = CanvasElementTypeAnimPlayer;

        if(!api_display_draw_parse_image_path(
               &canvas_element->anim_player.file_path,
               app_name,
               json_element,
               CanvasElementTypeAnimPlayer,
               error))
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

        long opacity = mg_json_get_long(json_element, "$.opacity", 100);
        if(opacity < 0 || opacity > 100) {
            furi_string_cat_printf(error, FIELD_INVALID_ERROR("opacity", "be in range [0; 100]"));
            break;
        }
        canvas_element->anim_player.opacity = opacity * 255 / 100;

        result = true;
    } while(0);

    return result;
}

static bool api_display_draw_parse_rect_fill(
    CanvasElement* canvas_element,
    struct mg_str json_element,
    FuriString* error) {
    bool result = false;

    do {
        char* fill_type = mg_json_get_str(json_element, "$.fill");
        size_t fill = RectangleFillNone;
        if(fill_type) {
            static const char* const fill_types[] = {
                [RectangleFillNone] = "none",
                [RectangleFillSolid] = "solid",
                [RectangleFillGradientH] = "gradient_h",
                [RectangleFillGradientV] = "gradient_v",
            };
            fill = value_index_string(fill_type, fill_types, COUNT_OF(fill_types));
            free(fill_type);
            if(fill >= COUNT_OF(fill_types)) {
                furi_string_cat_printf(error, FIELD_INVALID_ERROR("fill", "be one of: "));
                for(size_t i = 0; i < COUNT_OF(fill_types); i++) {
                    furi_string_cat_printf(error, "\"%s\"", fill_types[i]);
                    if(i != COUNT_OF(fill_types) - 1) {
                        furi_string_cat_str(error, ", ");
                    }
                }
                break;
            }
        }

        Color fill_color[2] = {
            (Color)COLOR_MAKE_HEXA(0xFFFFFFFF), (Color)COLOR_MAKE_HEXA(0x00000000)};
        bool color_parsed[2] = {false, false};

        char* color_hex = mg_json_get_str(json_element, "$.fill_colors[0]");
        if(color_hex) {
            color_parsed[0] = color_parse_hexa_string(color_hex, &fill_color[0]);
            free(color_hex);
            if(!color_parsed[0]) {
                furi_string_cat_printf(
                    error, FIELD_INVALID_ERROR("fill_colors[0]", "be in #RRGGBBAA format"));
                break;
            }
        }
        color_hex = mg_json_get_str(json_element, "$.fill_colors[1]");
        if(color_hex) {
            color_parsed[1] = color_parse_hexa_string(color_hex, &fill_color[1]);
            free(color_hex);
            if(!color_parsed[1]) {
                furi_string_cat_printf(
                    error, FIELD_INVALID_ERROR("fill_colors[1]", "be in #RRGGBBAA format"));
                break;
            }
        }

        if(fill == RectangleFillGradientH || fill == RectangleFillGradientV) {
            // Gradient fill requires two colors or defaults to white and black
            if(color_parsed[0] != color_parsed[1]) {
                furi_string_cat_printf(
                    error,
                    FIELD_INVALID_ERROR(
                        "fill_colors", "have exactly 2 elements with \"gradient_X\" fill value"));
                break;
            }
        } else if(fill == RectangleFillSolid) {
            // Solid fill requires only one color or defaults to white
            if(color_parsed[1]) {
                furi_string_cat_printf(
                    error,
                    FIELD_INVALID_ERROR(
                        "fill_colors", "have only one element with \"solid\" fill value"));
                break;
            }
        }

        canvas_element->rectangle.fill = fill;
        canvas_element->rectangle.fill_color[0] = fill_color[0];
        canvas_element->rectangle.fill_color[1] = fill_color[1];
        result = true;
    } while(0);

    return result;
}

static bool api_display_draw_parse_rect_border(
    CanvasElement* canvas_element,
    struct mg_str json_element,
    FuriString* error) {
    bool result = false;

    do {
        long border_width = mg_json_get_long(json_element, "$.border_width", 1);
        if(border_width < 0) {
            furi_string_cat_printf(error, FIELD_INVALID_ERROR("border_width", "be >= 0"));
            break;
        }
        long radius = mg_json_get_long(json_element, "$.radius", 0);
        if(radius < 0) {
            furi_string_cat_printf(error, FIELD_INVALID_ERROR("radius", "be >= 0"));
            break;
        }

        Color border_color = (Color)COLOR_MAKE_HEXA(0xFFFFFFFF);
        char* color_hex = mg_json_get_str(json_element, "$.border_color");
        if(color_hex) {
            bool color_parsed = color_parse_hexa_string(color_hex, &border_color);
            free(color_hex);
            if(!color_parsed) {
                furi_string_cat_printf(
                    error, FIELD_INVALID_ERROR("border_color", "be in #RRGGBBAA format"));
                break;
            }
        }

        canvas_element->rectangle.radius = radius;
        canvas_element->rectangle.border_width = border_width;
        canvas_element->rectangle.border_color = border_color;
        result = true;
    } while(0);

    return result;
}

static bool api_display_draw_parse_rectangle_element(
    CanvasElement* canvas_element,
    const char* app_name,
    struct mg_str json_element,
    FuriString* error) {
    UNUSED(app_name);
    UNUSED(error);
    bool result = false;

    do {
        canvas_element->type = CanvasElementTypeRectangle;

        long width = mg_json_get_long(json_element, "$.width", -1);
        if(width <= 0) {
            furi_string_cat_printf(error, FIELD_INVALID_ERROR("width", "be >= 1"));
            break;
        }
        long height = mg_json_get_long(json_element, "$.height", -1);
        if(height <= 0) {
            furi_string_cat_printf(error, FIELD_INVALID_ERROR("height", "be >= 1"));
            break;
        }
        canvas_element->rectangle.width = width;
        canvas_element->rectangle.height = height;

        if(!api_display_draw_parse_rect_fill(canvas_element, json_element, error)) {
            break;
        }

        if(!api_display_draw_parse_rect_border(canvas_element, json_element, error)) {
            break;
        }

        result = true;
    } while(0);

    return result;
}

typedef bool (*ApiDisplayElementTypeParser)(
    CanvasElement*,
    const char* app_name,
    struct mg_str element,
    FuriString* error);

typedef struct {
    const char* type;
    ApiDisplayElementTypeParser parser;
} ApiDisplayElementTypeAssoc;

static bool api_display_draw_parse_element(
    CanvasElementsArray_t elements_array,
    char* app_name,
    struct mg_str element,
    FuriString* error,
    int32_t* default_z_index) {
    bool success = false;
    char* element_type = NULL;
    CanvasElement* canvas_element = CanvasElementsArray_push_new(elements_array);

    do {
        canvas_element->id = mg_json_get_str(element, "$.id");
        if(!canvas_element->id) {
            furi_string_cat_printf(error, FIELD_MISSING_ERROR("id"));
            break;
        }

        int32_t temp_val = mg_json_get_long(element, "$.timeout", -1);
        canvas_element->timeout = (temp_val > 0) ? temp_val : 0;

        struct mg_str z_index_token = mg_json_get_tok(element, "$.z_index");
        if(z_index_token.buf) {
            int32_t z_index = mg_json_get_long(element, "$.z_index", -1);
            if(z_index < 0) {
                furi_string_cat_printf(error, FIELD_INVALID_ERROR("z_index", "be >= 0"));
                break;
            }
            canvas_element->z_index = z_index;
        } else {
            canvas_element->z_index = *default_z_index;
            *default_z_index += 10;
        }

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
            if(align >= COUNT_OF(alignments)) {
                furi_string_cat_printf(error, FIELD_INVALID_ERROR("align", "be one of: "));
                for(size_t i = 0; i < COUNT_OF(alignments); i++) {
                    furi_string_cat_printf(error, "\"%s\"", alignments[i]);
                    if(i != COUNT_OF(alignments) - 1) {
                        furi_string_cat_str(error, ", ");
                    }
                }
            }
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
                furi_string_cat_printf(
                    error, FIELD_INVALID_ERROR("display", "be one of: \"front\", \"back\""));
                free(display_id_str);
                break;
            }
            free(display_id_str);
        }

        element_type = mg_json_get_str(element, "$.type");
        if(!element_type) {
            furi_string_cat_printf(error, FIELD_MISSING_ERROR("type"));
            break;
        }

        bool type_valid = false;
        static const ApiDisplayElementTypeAssoc element_parsers[] = {
            {"text", api_display_draw_parse_text_element},
            {"image", api_display_draw_parse_image_element},
            {"animation", api_display_draw_parse_anim_player_element},
            {"rectangle", api_display_draw_parse_rectangle_element},
            {"countdown", api_display_draw_parse_countdown_element},
            {"xpmbitmap", api_display_draw_parse_xpm_element},
        };
        for(size_t i = 0; i < COUNT_OF(element_parsers); i++) {
            const ApiDisplayElementTypeAssoc* association = &element_parsers[i];
            if(strcmp(element_type, association->type) == 0) {
                type_valid = true;
                success = association->parser(canvas_element, app_name, element, error);
                break;
            }
        }

        if(!type_valid) {
            furi_string_cat_printf(error, FIELD_INVALID_ERROR("type", "be one of: "));
            for(size_t i = 0; i < COUNT_OF(element_parsers); i++) {
                furi_string_cat_printf(error, "\"%s\"", element_parsers[i].type);
                if(i != COUNT_OF(element_parsers) - 1) {
                    furi_string_cat_str(error, ", ");
                }
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
    [CanvasResultBadParameters] = {400, "Unspecified request error"},
    [CanvasResultLowPriority] = {409, "Not drawn due to low priority"},
    [CanvasResultEmptyScreen] = {400, "Nothing to display"},
    [CanvasResultTooManyElements] = {400, "Elements number limit exceeded"},
};
_Static_assert(
    COUNT_OF(draw_errors) == CanvasResultMax,
    "draw_errors table must cover all CanvasResult values");

typedef struct {
    unsigned long conn_id;
    CanvasResult result;
    Color led_color;
    bool blink_led;
} CanvasDrawCtx;

static void canvas_draw_wakeup_callback(struct mg_connection* conn, void* data, size_t len) {
    UNUSED(data);
    UNUSED(len);
    ConnectionContext* conn_ctx = (void*)conn->data;
    CanvasDrawCtx* ctx = conn_ctx->context;
    conn_ctx->on_wakeup = NULL;
    conn_ctx->on_close = NULL;
    conn_ctx->context = NULL;
    if(ctx->result != CanvasResultOk) {
        MG_REPLY_ERROR(conn, draw_errors[ctx->result].code, draw_errors[ctx->result].message);
    } else {
        if(ctx->blink_led) {
            StatusLights* status_lights = furi_record_open(RECORD_STATUS_LIGHTS);
            status_lights_run_preset(
                status_lights, StatusLightsPresetNotification, ctx->led_color);
            furi_record_close(RECORD_STATUS_LIGHTS);
        }
        MG_REPLY_OK(conn);
    }
    free(ctx);
}

static void canvas_draw_close_callback(struct mg_connection* conn) {
    ConnectionContext* conn_ctx = (void*)conn->data;
    CanvasDrawCtx* ctx = conn_ctx->context;
    ctx->conn_id = 0;
    conn_ctx->on_wakeup = NULL;
    conn_ctx->on_close = NULL;
    conn_ctx->context = NULL;
}

static void canvas_draw_done_callback(CanvasResult result, void* ctx_ptr) {
    CanvasDrawCtx* ctx = ctx_ptr;
    if(ctx->conn_id) {
        ctx->result = result;
        mg_wakeup(web_srv_get_mgr(), ctx->conn_id, NULL, 0);
    } else {
        free(ctx);
    }
}

static void api_display_canvas_draw(struct mg_connection* conn, struct mg_http_message* msg) {
    CanvasElementsArray_t elements_array;
    CanvasElementsArray_init(elements_array);

    char* app_name = NULL;
    double json_num = 0;
    int priority = DISPLAY_API_DEFAULT_PRIORITY;
    FuriString* error = furi_string_alloc();

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

        bool blink_led = false;
        Color led_color;
        char* led_color_hex = mg_json_get_str(msg->body, "$.led_notification_color");
        if(led_color_hex) {
            bool color_parsed = color_parse_hexa_string(led_color_hex, &led_color);
            free(led_color_hex);
            if(!color_parsed) {
                MG_REPLY_ERROR(conn, 400, "Invalid LED notification color");
                break;
            } else {
                blink_led = true;
            }
        }

        struct mg_str elements_obj = mg_json_get_tok(msg->body, "$.elements");
        if(!elements_obj.buf || elements_obj.len < 2 || elements_obj.buf[0] != '[') {
            MG_REPLY_ERROR(conn, 400, "Missing or invalid elements array");
            break;
        }

        size_t offset = 0;
        struct mg_str element;
        bool ok = true;
        size_t elements_count = 0;
        int32_t default_z_index = 0;
        while((offset = mg_json_next(elements_obj, offset, NULL, &element)) > 0) {
            furi_string_printf(error, "elements[%zu]", elements_count);
            ok = api_display_draw_parse_element(
                elements_array, app_name, element, error, &default_z_index);
            if(!ok) break;
            elements_count++;
            if(elements_count > CANVAS_MAX_ELEMENTS) {
                furi_string_printf(error, "%s", draw_errors[CanvasResultTooManyElements].message);
                ok = false;
                break;
            }
        }
        if(!ok) {
            furi_assert(!furi_string_empty(error));
            MG_REPLY_ERROR(conn, 400, furi_string_get_cstr(error));
            break;
        }
        if(CanvasElementsArray_size(elements_array) == 0) {
            MG_REPLY_ERROR(conn, 400, "Elements array is empty");
            break;
        }

        CanvasDrawCtx* ctx = malloc(sizeof(*ctx));
        *ctx = (CanvasDrawCtx){
            .conn_id = conn->id,
            .blink_led = blink_led,
            .led_color = led_color,
        };

        ConnectionContext* conn_ctx = (void*)conn->data;
        conn_ctx->on_wakeup = canvas_draw_wakeup_callback;
        conn_ctx->on_close = canvas_draw_close_callback;
        conn_ctx->context = ctx;

        CanvasSrv* canvas = furi_record_open(RECORD_CANVAS);
        canvas_show_elements_async(
            canvas, app_name, priority, elements_array, canvas_draw_done_callback, ctx);
        furi_record_close(RECORD_CANVAS);
    } while(0);

    CanvasElementsArray_clear(elements_array);
    furi_string_free(error);
    if(app_name) free(app_name);
}

static void api_display_canvas_clear(struct mg_connection* conn, struct mg_http_message* msg) {
    FuriString* error = furi_string_alloc();
    bool request_valid = false;
    cJSON* body = NULL;
    char** element_ids = NULL;

    do {
        cJSON* json_app_name = NULL;
        if(msg->body.buf && msg->body.len) {
            body = cJSON_ParseWithLength(msg->body.buf, msg->body.len);
            if(!body) break;
            if(!cJSON_IsObject(body)) break;

            cJSON* json_element_ids = cJSON_GetObjectItem(body, "element_ids");

            if(json_element_ids) {
                if(!cJSON_IsArray(json_element_ids)) break;

                bool all_element_ids_valid = true;
                element_ids = malloc(sizeof(char*) * (cJSON_GetArraySize(json_element_ids) + 1));
                size_t i = 0;
                cJSON* json_element_id = NULL;

                cJSON_ArrayForEach(json_element_id, json_element_ids) {
                    if(!cJSON_IsString(json_element_id)) {
                        all_element_ids_valid = false;
                        furi_string_printf(error, "element_ids must be an array of strings");
                        break;
                    }
                    element_ids[i] = cJSON_GetStringValue(json_element_id);
                    i++;
                }

                if(!all_element_ids_valid) break;
            }

            cJSON* json_app_name = cJSON_GetObjectItem(body, "application_name");
            if(json_app_name && !cJSON_IsString(json_app_name)) {
                furi_string_printf(error, "application_name must be a string");
                break;
            }
        }
        char* body_app_name = cJSON_GetStringValue(json_app_name);

        char query_app_name_buf[64];
        int query_app_name_len = mg_http_get_var(
            &msg->query, "application_name", query_app_name_buf, sizeof(query_app_name_buf));

        if((query_app_name_len >= 1) && body_app_name) {
            furi_string_printf(
                error,
                "application_name present in both the body and query parameters; exactly one is required");
            break;
        }

        const char* app_name = (query_app_name_len >= 1) ? query_app_name_buf : body_app_name;
        CanvasSrv* canvas = furi_record_open(RECORD_CANVAS);
        CanvasResult res =
            canvas_delete_elements(canvas, app_name, (const char* const*)element_ids);
        furi_record_close(RECORD_CANVAS);

        if(res == CanvasResultOk) {
            MG_REPLY_OK(conn);
            request_valid = true;
        } else if(res == CanvasResultNonexistentElementId) {
            furi_string_set_str(error, "one of element_ids is non-existent");
        } else if(res == CanvasResultWrongAppId) {
            furi_string_set_str(
                error, "this application_name is currently not displaying anything");
        } else {
            furi_crash("no error translation for canvas result");
        }
    } while(0);

    if(element_ids) free(element_ids);
    if(body) cJSON_Delete(body);

    if(!request_valid) {
        MG_REPLY_ERROR(conn, 400, furi_string_get_cstr(error));
    }

    furi_string_free(error);
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
