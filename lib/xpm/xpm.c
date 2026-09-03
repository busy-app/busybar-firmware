#include "xpm.h"

#include <furi.h>
#include <toolbox/color.h>

#define TAG                        "Xpm"
#define XPM_TOKEN_DELIMITERS       "\x20\t\v\f\r"
#define XPM_KEYWORD_SYMBOLIC       "s"
#define XPM_KEYWORD_SIGNATURE_MARK "!"
#define XPM_KEYWORD_SIGNATURE_NAME "XPM2"
#define XPM_BITS_PER_HEX_CHAR      4u /* number of bits that digit of hex number represents */
#define XPM_HEADER_INVALID_VALUE   0u /* any header's field value of 0 is meaningless */

typedef Color XpmColors[XpmPixelFormatsCount];

typedef struct {
    const char* key;
    XpmColors colors;
} XpmColorTableItem;

struct Xpm {
    const char* data;
    const char* header_section_stop;
    const char* colors_section_stop;

    const char* cursor;

    XpmHeaderData header_data;

    XpmColorTableItem* color_table;
};

typedef struct {
    const char* value;
    size_t length;
} XpmToken;

typedef struct {
    const char* name;
    Color color;
} XpmPresetColor;

typedef struct {
    void (*write)(Color color, uint8_t* out);
    size_t pixel_size;
} XpmPixelWriter;

typedef enum {
    XpmColorTypeColored,
    XpmColorTypeGrayscale,
    XpmColorTypeGrayscale4Bit,
    XpmColorTypeMonochrome,

    XpmColorTypesCount,
} XpmColorType;

static const char* const xpm_color_type_keys[] = {
    [XpmColorTypeColored] = "c",
    [XpmColorTypeGrayscale] = "g",
    [XpmColorTypeGrayscale4Bit] = "g4",
    [XpmColorTypeMonochrome] = "m",
};

static_assert(COUNT_OF(xpm_color_type_keys) == XpmColorTypesCount);

static const unsigned int xpm_color_type_ranks[][XpmColorTypesCount] = {
    [XpmPixelFormatBGRA8888] =
        {
            [XpmColorTypeColored] = 0,
            [XpmColorTypeGrayscale] = 1,
            [XpmColorTypeGrayscale4Bit] = 2,
            [XpmColorTypeMonochrome] = 3,
        },
    [XpmPixelFormatLA88] =
        {
            [XpmColorTypeGrayscale] = 0,
            [XpmColorTypeGrayscale4Bit] = 1,
            [XpmColorTypeColored] = 2,
            [XpmColorTypeMonochrome] = 3,
        },
};

static_assert(COUNT_OF(xpm_color_type_ranks) == XpmPixelFormatsCount);

static const XpmPresetColor xpm_preset_colors[] = {
    {"none", COLOR_MAKE_RGBA(0x00, 0x00, 0x00, 0x00)},
    {"black", COLOR_MAKE_RGB(0x00, 0x00, 0x00)},
    {"white", COLOR_MAKE_RGB(0xFF, 0xFF, 0xFF)},
    {"red", COLOR_MAKE_RGB(0xFF, 0x00, 0x00)},
    {"green", COLOR_MAKE_RGB(0x00, 0xFF, 0x00)},
    {"blue", COLOR_MAKE_RGB(0x00, 0x00, 0xFF)},
    {"yellow", COLOR_MAKE_RGB(0xFF, 0xFF, 0x00)},
    {"cyan", COLOR_MAKE_RGB(0x00, 0xFF, 0xFF)},
    {"magenta", COLOR_MAKE_RGB(0xFF, 0x00, 0xFF)},
    {"gray", COLOR_MAKE_RGB(0x80, 0x80, 0x80)},
    {"grey", COLOR_MAKE_RGB(0x80, 0x80, 0x80)},
};

static void xpm_write_pixel_bgra8888(Color color, uint8_t* out);
static void xpm_write_pixel_la88(Color color, uint8_t* out);

static const XpmPixelWriter xpm_pixel_writers[] = {
    [XpmPixelFormatBGRA8888] =
        {
            .write = xpm_write_pixel_bgra8888,
            .pixel_size = 4,
        },
    [XpmPixelFormatLA88] =
        {
            .write = xpm_write_pixel_la88,
            .pixel_size = 2,
        },
};

static_assert(COUNT_OF(xpm_pixel_writers) == XpmPixelFormatsCount);

/*  Common Utilities */

static bool xpm_is_header_data_valid(const XpmHeaderData* header_data) {
    return header_data->width != XPM_HEADER_INVALID_VALUE && header_data->width <= XPM_MAX_WIDTH &&
           header_data->height != XPM_HEADER_INVALID_VALUE &&
           header_data->height <= XPM_MAX_HEIGHT &&
           header_data->colors_count != XPM_HEADER_INVALID_VALUE &&
           header_data->colors_count <= XPM_MAX_COLORS_COUNT &&
           header_data->chars_per_pixel != XPM_HEADER_INVALID_VALUE &&
           header_data->chars_per_pixel <= XPM_MAX_CHARS_PER_PIXEL;
}

static bool xpm_lookup_color(Xpm* instance, const char* key, XpmPixelFormat format, Color* color) {
    XpmColorTableItem* color_table = instance->color_table;
    unsigned int colors_count = instance->header_data.colors_count;
    unsigned int chars_per_pixel = instance->header_data.chars_per_pixel;

    bool is_found = false;
    for(unsigned int i = 0; i < colors_count; i++) {
        XpmColorTableItem* item = &color_table[i];

        if(memcmp(item->key, key, chars_per_pixel) == 0) {
            *color = item->colors[format];
            is_found = true;
            break;
        }
    }

    return is_found;
}

static void xpm_write_pixel_bgra8888(Color color, uint8_t* out) {
    *out++ = color.b;
    *out++ = color.g;
    *out++ = color.r;
    *out = color.a;
}

static void xpm_write_pixel_la88(Color color, uint8_t* out) {
    *out++ = color_rgb_to_l8(color);
    *out = color.a;
}

/* Parsing Utilities */

static bool xpm_skip_in_line(Xpm* instance, size_t length) {
    const char* cursor = instance->cursor;

    bool is_successful = true;
    for(; length > 0; length--, cursor++) {
        char value = *cursor;
        if(value == '\n' || value == '\0') {
            is_successful = false;
            break;
        }
    }

    instance->cursor = cursor;
    return is_successful;
}

static bool xpm_next_token(Xpm* instance, XpmToken* token) {
    const char* cursor = instance->cursor;
    cursor += strspn(cursor, XPM_TOKEN_DELIMITERS);

    bool is_found;
    char value = *cursor;
    if(value != '\n' && value != '\0') {
        size_t token_length = strcspn(cursor, XPM_TOKEN_DELIMITERS "\n");

        *token = (XpmToken){
            .value = cursor,
            .length = token_length,
        };

        cursor += token_length;
        is_found = true;
    } else {
        is_found = false;
    }

    instance->cursor = cursor;
    return is_found;
}

static bool xpm_next_line(Xpm* instance) {
    const char* cursor = instance->cursor;

    static const char* new_line_sequences[] = {"\n", "\r\n"};

    bool is_successful = false;
    for(size_t i = 0; i < COUNT_OF(new_line_sequences); i++) {
        const char* new_line_sequence = new_line_sequences[i];
        size_t length = strlen(new_line_sequence);

        if(memcmp(cursor, new_line_sequence, length) == 0) {
            instance->cursor = cursor + length;
            is_successful = true;
            break;
        }
    }

    return is_successful;
}

static bool xpm_token_equals(XpmToken token, const char* string) {
    return token.length == strlen(string) && strncasecmp(token.value, string, token.length) == 0;
}

/* Parsing */

static bool xpm_parse_signature(Xpm* instance) {
    bool is_successful = false;
    do {
        XpmToken mark_token;
        if(!xpm_next_token(instance, &mark_token)) {
            break;
        }

        if(!xpm_token_equals(mark_token, XPM_KEYWORD_SIGNATURE_MARK)) {
            break;
        }

        XpmToken name_token;
        if(!xpm_next_token(instance, &name_token)) {
            break;
        }

        if(!xpm_token_equals(name_token, XPM_KEYWORD_SIGNATURE_NAME)) {
            break;
        }

        is_successful = xpm_next_line(instance);
    } while(false);

    return is_successful;
}

static bool xpm_parse_preset_color_value(XpmToken token, Color* color) {
    bool is_successful = false;
    for(size_t i = 0; i < COUNT_OF(xpm_preset_colors); i++) {
        const XpmPresetColor* preset = &xpm_preset_colors[i];

        if(xpm_token_equals(token, preset->name)) {
            *color = preset->color;
            is_successful = true;
            break;
        }
    }

    return is_successful;
}

static bool xpm_parse_literal_color_value(XpmToken token, Color* color) {
    bool is_successful = false;
    do {
        static const char* const formats[] = {
            "%1x%1x%1x%n",
            "%2x%2x%2x%n",
            "%3x%3x%3x%n",
            "%4x%4x%4x%n",
        };

        unsigned int digits_per_channel = token.length / strlen("RGB");
        if(digits_per_channel < 1 || digits_per_channel > COUNT_OF(formats)) {
            break;
        }

        unsigned int r, g, b;
        int parsed_length;
        int parsed_count =
            sscanf(token.value, formats[digits_per_channel - 1], &r, &g, &b, &parsed_length);
        if(parsed_count == 3 && (size_t)parsed_length == token.length) {
            unsigned int channel_max = (1u << (digits_per_channel * XPM_BITS_PER_HEX_CHAR)) - 1;
            *color = (Color)COLOR_MAKE_RGB(
                r * UINT8_MAX / channel_max,
                g * UINT8_MAX / channel_max,
                b * UINT8_MAX / channel_max);

            is_successful = true;
            break;
        }
    } while(false);

    return is_successful;
}

static bool xpm_parse_color_value_token(XpmToken token, Color* color) {
    return (*token.value == '#') ? xpm_parse_literal_color_value(
                                       (XpmToken){
                                           .value = token.value + strlen("#"),
                                           .length = token.length - strlen("#"),
                                       },
                                       color) :
                                   xpm_parse_preset_color_value(token, color);
}

static bool xpm_parse_color_type_token(XpmToken token, XpmColorType* type) {
    bool is_successful = false;
    for(XpmColorType t = 0; t < XpmColorTypesCount; t++) {
        if(xpm_token_equals(token, xpm_color_type_keys[t])) {
            *type = t;
            is_successful = true;
            break;
        }
    }

    return is_successful;
}

static bool xpm_parse_color_entries(Xpm* instance, XpmColors colors) {
    size_t best_ranks[XpmPixelFormatsCount] = {
        [0 ... XpmPixelFormatsCount - 1] = XpmColorTypesCount,
    };

    bool is_successful = false;
    for(XpmToken type_token; xpm_next_token(instance, &type_token);) {
        XpmToken value_token;
        if(!xpm_next_token(instance, &value_token)) {
            FURI_LOG_E(
                TAG, "Missing value for color type '%.*s'", type_token.length, type_token.value);
            is_successful = false;
            break;
        }

        XpmColorType type;
        if(xpm_parse_color_type_token(type_token, &type)) {
            Color value;
            if(!xpm_parse_color_value_token(value_token, &value)) {
                FURI_LOG_E(
                    TAG,
                    "Failed to parse color value '%.*s' for type '%.*s'",
                    value_token.length,
                    value_token.value,
                    type_token.length,
                    type_token.value);
                is_successful = false;
                break;
            }

            for(size_t format = 0; format < XpmPixelFormatsCount; format++) {
                unsigned int rank = xpm_color_type_ranks[format][type];
                if(rank <= best_ranks[format]) {
                    best_ranks[format] = rank;
                    colors[format] = value;
                }
            }

            is_successful = true;
        } else if(xpm_token_equals(type_token, XPM_KEYWORD_SYMBOLIC)) {
            /* symbolic key processing entry */
        } else {
            FURI_LOG_E(TAG, "Unknown color type '%.*s'", type_token.length, type_token.value);
            is_successful = false;
            break;
        }
    }

    return is_successful;
}

static bool xpm_parse_colors(Xpm* instance, XpmColorTableItem* color_table) {
    unsigned int colors_count = instance->header_data.colors_count;
    unsigned int chars_per_pixel = instance->header_data.chars_per_pixel;

    bool is_successful = true;
    for(size_t i = 0; i < colors_count; i++) {
        if(!xpm_next_line(instance)) {
            FURI_LOG_E(TAG, "Expected newline before color entry %zu", i);
            is_successful = false;
            break;
        }

        XpmColorTableItem* item = &color_table[i];
        item->key = instance->cursor;

        if(!xpm_skip_in_line(instance, chars_per_pixel)) {
            FURI_LOG_E(
                TAG,
                "Color entry %zu: expected key of %u chars, got '%.*s'",
                i,
                chars_per_pixel,
                instance->cursor - item->key,
                item->key);
            is_successful = false;
            break;
        }

        if(!xpm_parse_color_entries(instance, item->colors)) {
            is_successful = false;
            break;
        }
    }

    return is_successful;
}

static bool xpm_parse_pixels(Xpm* instance, XpmPixelFormat format, void* pixel_data) {
    const XpmPixelWriter* pixel_writer = &xpm_pixel_writers[format];
    unsigned int height = instance->header_data.height;
    unsigned int width = instance->header_data.width;
    unsigned int chars_per_pixel = instance->header_data.chars_per_pixel;

    bool is_successful = true;
    for(size_t y = 0; y < height && is_successful; y++) {
        if(!xpm_next_line(instance)) {
            FURI_LOG_E(TAG, "Expected newline before pixel row %zu", y);
            is_successful = false;
            break;
        }

        for(size_t x = 0; x < width; x++) {
            const char* key = instance->cursor;
            if(!xpm_skip_in_line(instance, chars_per_pixel)) {
                FURI_LOG_E(
                    TAG,
                    "Pixel row %zu ended early at column %zu (expected %u pixel keys)",
                    y,
                    x,
                    width);
                is_successful = false;
                break;
            }

            Color color;
            if(!xpm_lookup_color(instance, key, format, &color)) {
                FURI_LOG_E(
                    TAG,
                    "Unknown pixel key '%.*s' at row %zu, column %zu",
                    chars_per_pixel,
                    key,
                    y,
                    x);
                is_successful = false;
                break;
            }

            pixel_writer->write(color, pixel_data);
            pixel_data += pixel_writer->pixel_size;
        }
    }

    if(is_successful) {
        /* tolerate exactly one optional line at the end of file */
        xpm_next_line(instance);

        if(*instance->cursor != '\0') {
            FURI_LOG_E(TAG, "Unexpected data after pixel rows");
            is_successful = false;
        }
    }

    return is_successful;
}

/* Public API Implementation */

Xpm* xpm_alloc(const char* xpm_string) {
    furi_check(xpm_string);

    Xpm* instance = malloc(sizeof(*instance));

    instance->data = xpm_string;
    instance->header_section_stop = NULL;
    instance->colors_section_stop = NULL;
    instance->cursor = xpm_string;

    instance->header_data = (XpmHeaderData){
        .width = XPM_HEADER_INVALID_VALUE,
        .height = XPM_HEADER_INVALID_VALUE,
        .colors_count = XPM_HEADER_INVALID_VALUE,
        .chars_per_pixel = XPM_HEADER_INVALID_VALUE,
    };

    instance->color_table = NULL;

    return instance;
}

void xpm_free(Xpm* instance) {
    furi_check(instance);

    free(instance->color_table);
    free(instance);
}

bool xpm_decode_header(Xpm* instance) {
    furi_check(instance);

    bool is_successful = false;
    do {
        if(xpm_is_header_data_valid(&instance->header_data)) {
            is_successful = true;
            break;
        }

        furi_check(instance->data == instance->cursor);

        if(!xpm_parse_signature(instance)) {
            FURI_LOG_E(TAG, "Missing or malformed XPM2 signature line");
            break;
        }

        XpmHeaderData header_data;
        int parsed_length;
        int parsed_count = sscanf(
            instance->cursor,
            "%u%u%u%u%n",
            &header_data.width,
            &header_data.height,
            &header_data.colors_count,
            &header_data.chars_per_pixel,
            &parsed_length);

        if(parsed_count != 4) {
            FURI_LOG_E(TAG, "Expected 4 header values, got %d", parsed_count);
            break;
        }

        if(!xpm_is_header_data_valid(&header_data)) {
            FURI_LOG_E(TAG, "Header contains invalid value(s)");
            break;
        }

        instance->cursor += parsed_length;

        XpmToken invalid_token;
        if(xpm_next_token(instance, &invalid_token)) {
            FURI_LOG_E(
                TAG,
                "Unexpected token after header: '%.*s'",
                invalid_token.length,
                invalid_token.value);
            break;
        }

        instance->header_data = header_data;
        instance->header_section_stop = instance->cursor;
        is_successful = true;
    } while(false);

    return is_successful;
}

XpmHeaderData xpm_get_header_data(Xpm* instance) {
    furi_check(instance);
    furi_check(xpm_is_header_data_valid(&instance->header_data));

    return instance->header_data;
}

bool xpm_decode_colors(Xpm* instance) {
    furi_check(instance);
    furi_check(xpm_is_header_data_valid(&instance->header_data));

    bool is_successful = false;
    do {
        if(instance->color_table) {
            is_successful = true;
            break;
        }

        furi_check(instance->header_section_stop == instance->cursor);

        XpmColorTableItem* color_table =
            calloc(instance->header_data.colors_count, sizeof(XpmColorTableItem));

        if(xpm_parse_colors(instance, color_table)) {
            instance->color_table = color_table;
            instance->colors_section_stop = instance->cursor;
            is_successful = true;
        } else {
            free(color_table);
        }
    } while(false);

    return is_successful;
}

void* xpm_decode_pixels(Xpm* instance, XpmPixelFormat format, size_t* size) {
    furi_check(instance);
    furi_check(xpm_is_header_data_valid(&instance->header_data));
    furi_check(instance->color_table);
    furi_check(instance->colors_section_stop);
    furi_check(format < XpmPixelFormatsCount);

    size_t pixel_buffer_size = instance->header_data.width * instance->header_data.height *
                               xpm_pixel_writers[format].pixel_size;
    void* pixels = malloc(pixel_buffer_size);

    instance->cursor = instance->colors_section_stop;
    if(!xpm_parse_pixels(instance, format, pixels)) {
        free(pixels);
        pixels = NULL;
    } else if(size) {
        *size = pixel_buffer_size;
    }

    return pixels;
}
