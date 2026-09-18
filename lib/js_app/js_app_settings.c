#include "js_app_settings.h"

#include <cjson/cJSON.h>

#define TAG "JsAppSettings"

#define JS_APP_SETTINGS_FORMAT_VERSION (1)

#define JS_APP_SETTINGS_MAX_DEPTH (4u)

#define JS_APP_SETTINGS_INT_DEFAULT_MIN  INT_MIN
#define JS_APP_SETTINGS_INT_DEFAULT_MAX  INT_MAX
#define JS_APP_SETTINGS_INT_DEFAULT_STEP (1)

#define JS_APP_SETTINGS_STRING_ABSOLUTE_MAX_LENGTH (255u)
#define JS_APP_SETTINGS_STRING_DEFAULT_MIN_LENGTH  (0u)
#define JS_APP_SETTINGS_STRING_DEFAULT_MAX_LENGTH  (64u)

#define JS_APP_SETTINGS_ENUM_MAX_OPTIONS (32u)

typedef struct JsAppSettingsCursor JsAppSettingsCursor;

struct JsAppSettingsCursor {
    const JsAppSettingsCursor* parent;
    const char* id;
    cJSON* json;
};

typedef struct {
    const char* key;
    bool (*parse)(const JsAppSettingsCursor* cursor, JsAppSettingsNode* node);
    bool (*is_valid)(const JsAppSettingsNode* node, const void* value);
    void (*free)(JsAppSettingsNode* node);
} JsAppSettingsNodeTypeEntry;

typedef enum {
    JsAppSettingsJsonTypeBool,
    JsAppSettingsJsonTypeNumber,
    JsAppSettingsJsonTypeString,
    JsAppSettingsJsonTypeArray,
    JsAppSettingsJsonTypeObject,
    JsAppSettingsJsonTypeNull,

    JsAppSettingsJsonTypesCount,
} JsAppSettingsJsonType;

typedef struct {
    const char* name;
    cJSON_bool (*is_type)(const cJSON* item);
} JsAppSettingsJsonTypeInfo;

static const JsAppSettingsNodeTypeEntry js_app_settings_node_types[];
static const JsAppSettingsJsonTypeInfo js_app_settings_json_types[];

/* JSON Logging */

static size_t js_app_settings_cursor_depth(const JsAppSettingsCursor* cursor) {
    size_t depth = 0;

    for(; cursor->parent; cursor = cursor->parent) {
        depth++;
    }

    return depth;
}

static void
    js_app_settings_json_cat_cursor_path(const JsAppSettingsCursor* cursor, FuriString* path) {
    if(cursor->parent) {
        js_app_settings_json_cat_cursor_path(cursor->parent, path);
        furi_string_cat(path, ".");
    }

    furi_string_cat(path, cursor->id);
}

static void
    js_app_settings_json_log_error(const JsAppSettingsCursor* cursor, const char* format, ...) {
    FuriString* message = furi_string_alloc();

    furi_string_cat(message, "Field '");
    js_app_settings_json_cat_cursor_path(cursor, message);
    furi_string_cat(message, "': ");

    va_list arguments;
    va_start(arguments, format);
    furi_string_cat_vprintf(message, format, arguments);
    va_end(arguments);

    FURI_LOG_E(TAG, "%s", furi_string_get_cstr(message));

    furi_string_free(message);
}

static const char* js_app_settings_json_type_name(cJSON* json) {
    const char* name = "unknown";

    for(JsAppSettingsJsonType type = 0; type < JsAppSettingsJsonTypesCount; type++) {
        const JsAppSettingsJsonTypeInfo* type_info = &js_app_settings_json_types[type];

        if(type_info->is_type(json)) {
            name = type_info->name;
            break;
        }
    }

    return name;
}

/* JSON Reading */

static char* js_app_settings_string_steal(char** string) {
    char* value = *string;
    *string = NULL;

    return value;
}

static bool js_app_settings_json_get(
    const JsAppSettingsCursor* cursor,
    const char* key,
    bool is_optional,
    JsAppSettingsJsonType type,
    cJSON** json_out) {
    cJSON* json = cJSON_GetObjectItem(cursor->json, key);

    bool is_successful = true;
    do {
        if(json) {
            const JsAppSettingsJsonTypeInfo* type_info = &js_app_settings_json_types[type];

            if(!type_info->is_type(json)) {
                js_app_settings_json_log_error(
                    cursor,
                    "key '%s' has type %s (%s was expected)",
                    key,
                    js_app_settings_json_type_name(json),
                    type_info->name);
                is_successful = false;
            } else {
                *json_out = json;
            }

            break;
        }

        if(is_optional) {
            *json_out = NULL;
            break;
        }

        js_app_settings_json_log_error(cursor, "key '%s' is missing", key);
        is_successful = false;
    } while(false);

    return is_successful;
}

static bool js_app_settings_json_read_bool(
    const JsAppSettingsCursor* cursor,
    const char* key,
    bool is_optional,
    bool* value_out) {
    cJSON* json;
    bool is_successful =
        js_app_settings_json_get(cursor, key, is_optional, JsAppSettingsJsonTypeBool, &json);

    if(is_successful && json) {
        *value_out = cJSON_IsTrue(json);
    }

    return is_successful;
}

static bool js_app_settings_json_read_float(
    const JsAppSettingsCursor* cursor,
    const char* key,
    bool is_optional,
    float* value_out) {
    cJSON* json;
    bool is_successful =
        js_app_settings_json_get(cursor, key, is_optional, JsAppSettingsJsonTypeNumber, &json);

    if(is_successful && json) {
        *value_out = cJSON_GetNumberValue(json);
    }

    return is_successful;
}

static bool js_app_settings_json_read_int(
    const JsAppSettingsCursor* cursor,
    const char* key,
    bool is_optional,
    int* value_out) {
    cJSON* json;
    bool is_successful =
        js_app_settings_json_get(cursor, key, is_optional, JsAppSettingsJsonTypeNumber, &json);

    if(is_successful && json) {
        *value_out = json->valueint;
    }

    return is_successful;
}

static bool js_app_settings_json_read_uint(
    const JsAppSettingsCursor* cursor,
    const char* key,
    bool is_optional,
    unsigned int* value_out) {
    cJSON* json;
    bool is_successful =
        js_app_settings_json_get(cursor, key, is_optional, JsAppSettingsJsonTypeNumber, &json);

    if(is_successful && json) {
        int value = json->valueint;

        if(value >= 0) {
            *value_out = value;
        } else {
            js_app_settings_json_log_error(cursor, "key '%s' must be a non-negative integer", key);
            is_successful = false;
        }
    }

    return is_successful;
}

static bool js_app_settings_json_read_string(
    const JsAppSettingsCursor* cursor,
    const char* key,
    bool is_optional,
    const char** value_out) {
    cJSON* json;
    bool is_successful =
        js_app_settings_json_get(cursor, key, is_optional, JsAppSettingsJsonTypeString, &json);

    if(is_successful && json) {
        *value_out = json->valuestring;
    }

    return is_successful;
}

static bool js_app_settings_json_steal_string(
    const JsAppSettingsCursor* cursor,
    const char* key,
    bool is_optional,
    char** value_out) {
    cJSON* json;
    bool is_successful =
        js_app_settings_json_get(cursor, key, is_optional, JsAppSettingsJsonTypeString, &json);

    if(is_successful && json) {
        *value_out = js_app_settings_string_steal(&json->valuestring);
    }

    return is_successful;
}

/* Node Value Validation */

static bool js_app_settings_is_id_valid(const char* id) {
    bool is_id_valid = islower((unsigned char)*id);

    for(unsigned char character; is_id_valid && (character = *++id) != '\0';) {
        is_id_valid = islower(character) || isdigit(character) || character == '_';
    }

    return is_id_valid;
}

static bool js_app_settings_is_id_unique(
    const JsAppSettingsNode* nodes,
    size_t nodes_count,
    const char* id) {
    bool is_id_unique = true;

    for(; nodes_count > 0; nodes_count--, nodes++) {
        if(strcmp(nodes->id, id) == 0) {
            is_id_unique = false;
            break;
        }
    }

    return is_id_unique;
}

static bool js_app_settings_is_int_valid(const JsAppSettingsNode* node, const void* value) {
    const JsAppSettingsIntData* data = node->data;
    int integer = *(const int*)value;

    return integer >= data->min_value && integer <= data->max_value;
}

static bool js_app_settings_is_string_valid(const JsAppSettingsNode* node, const void* value) {
    const JsAppSettingsStringData* data = node->data;
    size_t length = strlen(value);

    return length >= data->min_length && length <= data->max_length;
}

static bool js_app_settings_is_enum_valid(const JsAppSettingsNode* node, const void* value) {
    const JsAppSettingsEnumData* data = node->data;
    int index = *(const int*)value;

    return index >= 0 && (size_t)index < data->options_count;
}

static bool js_app_settings_is_time_valid(const JsAppSettingsNode* node, const void* value) {
    UNUSED(node);

    const JsAppSettingsTimeValue* time_value = value;

    return utz_time_init_checked(
        time_value->time.hour, time_value->time.minute, time_value->time.second, &(utz_time_t){});
}

static inline bool js_app_settings_is_geo_auto_valid(const JsAppSettingsGeoValue* geo) {
    return isnan(geo->latitude) && isnan(geo->longitude);
}

static inline bool js_app_settings_is_geo_fixed_valid(const JsAppSettingsGeoValue* geo) {
    return !isnan(geo->latitude) && !isnan(geo->longitude) && geo->latitude >= -90.0f &&
           geo->latitude <= 90.0f && geo->longitude >= -180.0f && geo->longitude <= 180.0f;
}

static bool js_app_settings_is_geo_valid(const JsAppSettingsNode* node, const void* value) {
    UNUSED(node);

    const JsAppSettingsGeoValue* geo = value;
    return (geo->mode < JsAppSettingsGeoModesCount) && *geo->name != '\0' &&
           ((geo->mode == JsAppSettingsGeoModeAuto) ? js_app_settings_is_geo_auto_valid(geo) :
                                                      js_app_settings_is_geo_fixed_valid(geo));
}

/* Node Parsing */

static bool
    js_app_settings_parse_bool(const JsAppSettingsCursor* cursor, JsAppSettingsNode* node) {
    JsAppSettingsBoolData* data = malloc(sizeof(*data));
    node->data = data;

    return js_app_settings_json_read_bool(cursor, "default", false, &data->default_value);
}

static bool js_app_settings_parse_int(const JsAppSettingsCursor* cursor, JsAppSettingsNode* node) {
    JsAppSettingsIntData* data = malloc(sizeof(*data));
    node->data = data;

    data->min_value = JS_APP_SETTINGS_INT_DEFAULT_MIN;
    data->max_value = JS_APP_SETTINGS_INT_DEFAULT_MAX;
    data->value_step = JS_APP_SETTINGS_INT_DEFAULT_STEP;

    bool is_successful = false;
    do {
        if(!js_app_settings_json_read_int(cursor, "default", false, &data->default_value)) {
            break;
        }

        if(!js_app_settings_json_read_int(cursor, "min", true, &data->min_value)) {
            break;
        }

        if(!js_app_settings_json_read_int(cursor, "max", true, &data->max_value)) {
            break;
        }

        if(!js_app_settings_json_read_uint(cursor, "step", true, &data->value_step)) {
            break;
        }

        if(data->min_value > data->max_value) {
            js_app_settings_json_log_error(cursor, "'min' is greater than 'max'");
            break;
        }

        if(data->value_step == 0) {
            js_app_settings_json_log_error(cursor, "key 'step' must be a non-zero integer");
            break;
        }

        unsigned int range = (unsigned int)data->max_value - (unsigned int)data->min_value;
        if(range % data->value_step != 0) {
            js_app_settings_json_log_error(
                cursor, "'max' - 'min' must be evenly divisible by 'step'");
            break;
        }

        if(!js_app_settings_is_int_valid(node, &data->default_value)) {
            js_app_settings_json_log_error(cursor, "default is out of bounds");
            break;
        }

        is_successful = true;
    } while(false);

    return is_successful;
}

static bool
    js_app_settings_parse_string(const JsAppSettingsCursor* cursor, JsAppSettingsNode* node) {
    JsAppSettingsStringData* data = malloc(sizeof(*data));
    node->data = data;

    data->default_value = NULL;
    data->min_length = JS_APP_SETTINGS_STRING_DEFAULT_MIN_LENGTH;
    data->max_length = JS_APP_SETTINGS_STRING_DEFAULT_MAX_LENGTH;
    data->is_sensitive = false;

    bool is_successful = false;
    do {
        if(!js_app_settings_json_steal_string(cursor, "default", false, &data->default_value)) {
            break;
        }

        if(!js_app_settings_json_read_bool(cursor, "sensitive", true, &data->is_sensitive)) {
            break;
        }

        if(!js_app_settings_json_read_uint(cursor, "min_length", true, &data->min_length)) {
            break;
        }

        if(!js_app_settings_json_read_uint(cursor, "max_length", true, &data->max_length)) {
            break;
        }

        if(data->min_length > data->max_length) {
            js_app_settings_json_log_error(cursor, "'min_length' is greater than 'max_length'");
            break;
        }

        if(data->max_length > JS_APP_SETTINGS_STRING_ABSOLUTE_MAX_LENGTH) {
            js_app_settings_json_log_error(
                cursor,
                "'max_length' is greater than %u",
                JS_APP_SETTINGS_STRING_ABSOLUTE_MAX_LENGTH);
            break;
        }

        if(!js_app_settings_is_string_valid(node, data->default_value)) {
            js_app_settings_json_log_error(cursor, "default length is out of bounds");
            break;
        }

        is_successful = true;
    } while(false);

    return is_successful;
}

static bool js_app_settings_enum_find_index(
    const JsAppSettingsEnumData* data,
    const char* value,
    int* index) {
    bool is_found = false;

    for(size_t i = 0; i < data->options_count; i++) {
        if(strcmp(data->options[i].value, value) == 0) {
            *index = i;
            is_found = true;
            break;
        }
    }

    return is_found;
}

static bool js_app_settings_parse_enum_options(
    const JsAppSettingsCursor* cursor,
    cJSON* options_json,
    JsAppSettingsEnumData* data) {
    bool is_successful = false;

    do {
        size_t options_count = cJSON_GetArraySize(options_json);
        if(options_count == 0 || options_count > JS_APP_SETTINGS_ENUM_MAX_OPTIONS) {
            js_app_settings_json_log_error(
                cursor,
                "key 'options' must contain between 1 and %u items",
                JS_APP_SETTINGS_ENUM_MAX_OPTIONS);
            break;
        }

        data->options = calloc(options_count, sizeof(*data->options));
        data->options_count = options_count;

        is_successful = true;
        size_t index = 0;
        for(cJSON* json = options_json->child; json && is_successful; json = json->next, index++) {
            JsAppSettingsEnumOption* option = &data->options[index];
            JsAppSettingsCursor option_cursor = *cursor;
            option_cursor.json = json;

            is_successful =
                js_app_settings_json_steal_string(
                    &option_cursor, "value", false, &option->value) &&
                js_app_settings_json_steal_string(&option_cursor, "label", false, &option->label);

            for(size_t i = 0; is_successful && i < index; i++) {
                if(strcmp(option->value, data->options[i].value) == 0) {
                    js_app_settings_json_log_error(
                        cursor, "duplicate enum option value '%s'", option->value);
                    is_successful = false;
                }
            }
        }
    } while(false);

    return is_successful;
}

static bool
    js_app_settings_parse_enum(const JsAppSettingsCursor* cursor, JsAppSettingsNode* node) {
    JsAppSettingsEnumData* data = malloc(sizeof(*data));
    node->data = data;

    data->default_index = 0;
    data->options = NULL;
    data->options_count = 0;

    bool is_successful = false;
    do {
        const char* default_value;
        if(!js_app_settings_json_read_string(cursor, "default", false, &default_value)) {
            break;
        }

        cJSON* options_json;
        if(!js_app_settings_json_get(
               cursor, "options", false, JsAppSettingsJsonTypeArray, &options_json)) {
            break;
        }

        if(!js_app_settings_parse_enum_options(cursor, options_json, data)) {
            break;
        }

        if(!js_app_settings_enum_find_index(data, default_value, &data->default_index)) {
            js_app_settings_json_log_error(
                cursor, "default '%s' is not among the options", default_value);
            break;
        }

        is_successful = true;
    } while(false);

    return is_successful;
}

static bool
    js_app_settings_parse_color(const JsAppSettingsCursor* cursor, JsAppSettingsNode* node) {
    JsAppSettingsColorData* data = malloc(sizeof(*data));
    node->data = data;

    bool is_successful = false;
    do {
        const char* default_value;
        if(!js_app_settings_json_read_string(cursor, "default", false, &default_value)) {
            break;
        }

        if(!js_app_settings_color_parse(default_value, &data->default_value)) {
            js_app_settings_json_log_error(
                cursor, "default must be a '#RRGGBB' or '#RRGGBBAA' color");
            break;
        }

        is_successful = true;
    } while(false);

    return is_successful;
}

static bool
    js_app_settings_parse_time(const JsAppSettingsCursor* cursor, JsAppSettingsNode* node) {
    JsAppSettingsTimeData* data = malloc(sizeof(*data));
    node->data = data;

    bool is_successful = false;
    do {
        const char* default_value;
        if(!js_app_settings_json_read_string(cursor, "default", false, &default_value)) {
            break;
        }

        if(!js_app_settings_time_parse(default_value, &data->default_value)) {
            js_app_settings_json_log_error(cursor, "default must be a 'HH:MM' or 'HH:MM:SS' time");
            break;
        }

        is_successful = true;
    } while(false);

    return is_successful;
}

static bool js_app_settings_parse_geo(const JsAppSettingsCursor* cursor, JsAppSettingsNode* node) {
    JsAppSettingsGeoData* data = malloc(sizeof(*data));
    node->data = data;
    JsAppSettingsGeoValue* value = &data->default_value;

    value->mode = JsAppSettingsGeoModeAuto;
    *value->name = '\0';
    value->latitude = NAN;
    value->longitude = NAN;

    bool is_successful = false;
    do {
        cJSON* default_json;
        if(!js_app_settings_json_get(
               cursor, "default", false, JsAppSettingsJsonTypeObject, &default_json)) {
            break;
        }

        JsAppSettingsCursor default_cursor = *cursor;
        default_cursor.json = default_json;

        const char* mode;
        if(!js_app_settings_json_read_string(&default_cursor, "mode", false, &mode)) {
            break;
        }

        if(!js_app_settings_geo_mode_parse(mode, &value->mode)) {
            js_app_settings_json_log_error(
                &default_cursor, "key 'mode' must be 'auto' or 'fixed', got '%s'", mode);
            break;
        }

        const char* name;
        if(!js_app_settings_json_read_string(&default_cursor, "name", false, &name)) {
            break;
        }

        size_t name_length = strlen(name);
        if(name_length == 0 || name_length > JS_APP_SETTINGS_GEO_NAME_MAX_LENGTH) {
            js_app_settings_json_log_error(
                &default_cursor,
                "key 'name' must be between 1 and %u characters",
                JS_APP_SETTINGS_GEO_NAME_MAX_LENGTH);
            break;
        }

        strcpy(value->name, name);
        if(!js_app_settings_json_read_float(&default_cursor, "lat", true, &value->latitude)) {
            break;
        }

        if(!js_app_settings_json_read_float(&default_cursor, "lon", true, &value->longitude)) {
            break;
        }

        if(!js_app_settings_is_geo_valid(node, value)) {
            js_app_settings_json_log_error(&default_cursor, "invalid default value");
            break;
        }

        is_successful = true;
    } while(false);

    return is_successful;
}

static bool js_app_settings_parse_nodes(
    cJSON* fields_json,
    const JsAppSettingsCursor* parent,
    JsAppSettingsNode* nodes);

static bool
    js_app_settings_parse_group(const JsAppSettingsCursor* cursor, JsAppSettingsNode* node) {
    JsAppSettingsGroupData* data = malloc(sizeof(*data));
    node->data = data;

    data->nodes = NULL;
    data->nodes_count = 0;

    bool is_successful = false;
    do {
        cJSON* fields_json;
        if(!js_app_settings_json_get(
               cursor, "fields", false, JsAppSettingsJsonTypeObject, &fields_json)) {
            break;
        }

        size_t nodes_count = cJSON_GetArraySize(fields_json);
        if(nodes_count == 0) {
            js_app_settings_json_log_error(cursor, "group has no fields");
            break;
        }

        if(js_app_settings_cursor_depth(cursor) >= JS_APP_SETTINGS_MAX_DEPTH) {
            js_app_settings_json_log_error(
                cursor, "maximum group nesting depth (%u) reached", JS_APP_SETTINGS_MAX_DEPTH);
            break;
        }

        data->nodes_count = nodes_count;
        data->nodes = calloc(nodes_count, sizeof(*data->nodes));

        is_successful = js_app_settings_parse_nodes(fields_json, cursor, data->nodes);
    } while(false);

    return is_successful;
}

static bool js_app_settings_parse_node_type(const char* string, JsAppSettingsNodeType* node_type) {
    bool is_successful = false;

    for(JsAppSettingsNodeType type = 0; type < JsAppSettingsNodeTypesCount; type++) {
        const JsAppSettingsNodeTypeEntry* type_entry = &js_app_settings_node_types[type];

        if(strcmp(type_entry->key, string) == 0) {
            *node_type = type;
            is_successful = true;
            break;
        }
    }

    return is_successful;
}

static bool
    js_app_settings_parse_node(const JsAppSettingsCursor* cursor, JsAppSettingsNode* node) {
    bool is_successful = false;

    do {
        if(!js_app_settings_json_steal_string(cursor, "label", false, &node->label)) {
            break;
        }

        if(!js_app_settings_json_steal_string(cursor, "description", true, &node->description)) {
            break;
        }

        const char* type_key;
        if(!js_app_settings_json_read_string(cursor, "type", false, &type_key)) {
            break;
        }

        if(!js_app_settings_parse_node_type(type_key, &node->type)) {
            js_app_settings_json_log_error(cursor, "unknown type '%s'", type_key);
            break;
        }

        is_successful = js_app_settings_node_types[node->type].parse(cursor, node);
    } while(false);

    return is_successful;
}

static bool js_app_settings_parse_nodes(
    cJSON* fields_json,
    const JsAppSettingsCursor* parent,
    JsAppSettingsNode* nodes) {
    bool is_successful = true;

    size_t index = 0;
    for(cJSON* json = fields_json->child; json; json = json->next, index++) {
        JsAppSettingsNode* node = &nodes[index];

        node->id = js_app_settings_string_steal(&json->string);
        JsAppSettingsCursor cursor = {.parent = parent, .json = json, .id = node->id};
        if(!js_app_settings_is_id_valid(cursor.id)) {
            js_app_settings_json_log_error(&cursor, "invalid id format");
            is_successful = false;
            break;
        }

        if(!js_app_settings_is_id_unique(nodes, index, cursor.id)) {
            js_app_settings_json_log_error(&cursor, "duplicate id on the same level");
            is_successful = false;
            break;
        }

        if(!js_app_settings_parse_node(&cursor, node)) {
            is_successful = false;
            break;
        }
    }

    return is_successful;
}

/* Node Destruction */

static void js_app_settings_free_nodes(JsAppSettingsNode* nodes, size_t nodes_count) {
    for(JsAppSettingsNode* node = nodes; nodes_count > 0; nodes_count--, node++) {
        const JsAppSettingsNodeTypeEntry* type_entry = &js_app_settings_node_types[node->type];

        if(type_entry->free) {
            type_entry->free(node);
        }

        free(node->data);

        cJSON_free(node->id);
        cJSON_free(node->label);
        cJSON_free(node->description);
    }

    free(nodes);
}

static void js_app_settings_free_string(JsAppSettingsNode* node) {
    JsAppSettingsStringData* data = node->data;

    cJSON_free(data->default_value);
}

static void js_app_settings_free_enum(JsAppSettingsNode* node) {
    JsAppSettingsEnumData* data = node->data;

    for(size_t i = 0; i < data->options_count; i++) {
        JsAppSettingsEnumOption* option = &data->options[i];

        cJSON_free(option->value);
        cJSON_free(option->label);
    }

    free(data->options);
}

static void js_app_settings_free_group(JsAppSettingsNode* node) {
    JsAppSettingsGroupData* data = node->data;

    js_app_settings_free_nodes(data->nodes, data->nodes_count);
}

/* Public API Implementation */

bool js_app_settings_validate_value(const JsAppSettingsNode* node, const void* value) {
    furi_check(node);
    furi_check(value);

    const JsAppSettingsNodeTypeEntry* type_entry = &js_app_settings_node_types[node->type];
    return type_entry->is_valid ? type_entry->is_valid(node, value) : true;
}

JsAppSettings* js_app_settings_parse(const char* data, uint32_t length) {
    furi_check(data);

    cJSON* root_json;
    JsAppSettings* settings = NULL;

    do {
        root_json = cJSON_ParseWithLength(data, length);
        if(!cJSON_IsObject(root_json)) {
            FURI_LOG_E(TAG, "Malformed JSON or root item is not an object");
            break;
        }

        cJSON* format_version_json = cJSON_GetObjectItem(root_json, "format_version");
        if(!cJSON_IsNumber(format_version_json)) {
            FURI_LOG_E(TAG, "Root key 'format_version' is missing or is not a number");
            break;
        }

        int format_version = format_version_json->valueint;
        if(format_version != JS_APP_SETTINGS_FORMAT_VERSION) {
            FURI_LOG_E(
                TAG,
                "Unsupported format version: expected %u, got %d",
                JS_APP_SETTINGS_FORMAT_VERSION,
                format_version);
            break;
        }

        cJSON* version_json = cJSON_GetObjectItem(root_json, "version");
        if(!cJSON_IsNumber(version_json)) {
            FURI_LOG_E(TAG, "Root key 'version' is missing or is not a number");
            break;
        }

        int version = version_json->valueint;
        if(version <= 0) {
            FURI_LOG_E(TAG, "Root key 'version' must be a positive number");
            break;
        }

        cJSON* fields_json = cJSON_GetObjectItem(root_json, "fields");
        if(!cJSON_IsObject(fields_json)) {
            FURI_LOG_E(TAG, "Root key 'fields' is missing or is not an object");
            break;
        }

        size_t nodes_count = cJSON_GetArraySize(fields_json);
        if(nodes_count == 0) {
            FURI_LOG_E(TAG, "Root key 'fields' must not be empty");
            break;
        }

        settings = malloc(sizeof(*settings));
        settings->version = version;
        settings->nodes_count = nodes_count;
        settings->nodes = calloc(nodes_count, sizeof(*settings->nodes));

        if(!js_app_settings_parse_nodes(fields_json, NULL, settings->nodes)) {
            js_app_settings_free(settings);
            settings = NULL;
        }
    } while(false);

    cJSON_Delete(root_json);

    return settings;
}

void js_app_settings_free(JsAppSettings* settings) {
    furi_check(settings);

    js_app_settings_free_nodes(settings->nodes, settings->nodes_count);

    free(settings);
}

/* Tables */

static const JsAppSettingsNodeTypeEntry js_app_settings_node_types[] = {
    [JsAppSettingsNodeTypeBool] =
        {
            .key = "boolean",
            .parse = js_app_settings_parse_bool,
            .is_valid = NULL,
            .free = NULL,
        },
    [JsAppSettingsNodeTypeInt] =
        {
            .key = "integer",
            .parse = js_app_settings_parse_int,
            .is_valid = js_app_settings_is_int_valid,
            .free = NULL,
        },
    [JsAppSettingsNodeTypeString] =
        {
            .key = "string",
            .parse = js_app_settings_parse_string,
            .is_valid = js_app_settings_is_string_valid,
            .free = js_app_settings_free_string,
        },
    [JsAppSettingsNodeTypeEnum] =
        {
            .key = "enum",
            .parse = js_app_settings_parse_enum,
            .is_valid = js_app_settings_is_enum_valid,
            .free = js_app_settings_free_enum,
        },
    [JsAppSettingsNodeTypeColor] =
        {
            .key = "color",
            .parse = js_app_settings_parse_color,
            .is_valid = NULL,
            .free = NULL,
        },
    [JsAppSettingsNodeTypeTime] =
        {
            .key = "time",
            .parse = js_app_settings_parse_time,
            .is_valid = js_app_settings_is_time_valid,
            .free = NULL,
        },
    [JsAppSettingsNodeTypeGeo] =
        {
            .key = "geolocation",
            .parse = js_app_settings_parse_geo,
            .is_valid = js_app_settings_is_geo_valid,
            .free = NULL,
        },
    [JsAppSettingsNodeTypeGroup] =
        {
            .key = "group",
            .parse = js_app_settings_parse_group,
            .is_valid = NULL,
            .free = js_app_settings_free_group,
        },
};

static_assert(COUNT_OF(js_app_settings_node_types) == JsAppSettingsNodeTypesCount);

static const JsAppSettingsJsonTypeInfo js_app_settings_json_types[] = {
    [JsAppSettingsJsonTypeBool] = {.name = "boolean", .is_type = cJSON_IsBool},
    [JsAppSettingsJsonTypeNumber] = {.name = "number", .is_type = cJSON_IsNumber},
    [JsAppSettingsJsonTypeString] = {.name = "string", .is_type = cJSON_IsString},
    [JsAppSettingsJsonTypeArray] = {.name = "array", .is_type = cJSON_IsArray},
    [JsAppSettingsJsonTypeObject] = {.name = "object", .is_type = cJSON_IsObject},
    [JsAppSettingsJsonTypeNull] = {.name = "null", .is_type = cJSON_IsNull},
};

static_assert(COUNT_OF(js_app_settings_json_types) == JsAppSettingsJsonTypesCount);
