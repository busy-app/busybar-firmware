#include "js_app_settings_storage.h"

#include <storage/storage.h>

#define TAG "JsAppSettingsStorage"

#define JS_APP_SETTINGS_STORAGE_SCHEMA_PATH_FORMAT \
    (EXT_PATH("user_assets") "/%s/appmeta/settings.json")
#define JS_APP_SETTINGS_STORAGE_DATA_PATH_FORMAT \
    (EXT_PATH("apps_data/jsrunner") "/%s.settings.json")

#define JS_APP_SETTINGS_STORAGE_ALIGN(value) \
    (((value) + (_Alignof(max_align_t) - 1)) & ~(_Alignof(max_align_t) - 1))

struct JsAppSettingsStorage {
    JsAppSettings* settings;
    SettingProvider* provider;

    SettingProviderSetting root;
    SettingProviderStructInterface root_interface;
    SettingProviderSetting* root_fields;

    void* values;
};

typedef struct {
    const JsAppSettingsNode* node;
    void* value;
} JsAppSettingsStorageSettingContext;

typedef struct {
    void* (*build)(const JsAppSettingsNode* node);
    SettingProviderSettingType setting_type;
} JsAppSettingsStorageInterfaceBuilder;

static const JsAppSettingsStorageInterfaceBuilder js_app_settings_storage_interface_builders[];
static const size_t js_app_settings_storage_sizes[];

/* Layout */

static size_t
    js_app_settings_storage_compute_size(const JsAppSettingsNode* nodes, size_t nodes_count);

static size_t js_app_settings_storage_compute_node_size(const JsAppSettingsNode* node) {
    size_t node_size;

    if(node->type == JsAppSettingsNodeTypeString) {
        const JsAppSettingsStringData* node_data = node->data;
        node_size = node_data->max_length + 1;
    } else if(node->type == JsAppSettingsNodeTypeGroup) {
        const JsAppSettingsGroupData* node_data = node->data;
        node_size = js_app_settings_storage_compute_size(node_data->nodes, node_data->nodes_count);
    } else {
        node_size = js_app_settings_storage_sizes[node->type];
    }

    return node_size;
}

static size_t
    js_app_settings_storage_compute_size(const JsAppSettingsNode* nodes, size_t nodes_count) {
    size_t nodes_total_size = 0;

    for(; nodes_count > 0; nodes_count--, nodes++) {
        nodes_total_size +=
            JS_APP_SETTINGS_STORAGE_ALIGN(js_app_settings_storage_compute_node_size(nodes));
    }

    return nodes_total_size;
}

/* Validation */

static bool
    js_app_settings_storage_is_valid_int(const SettingProviderSetting* setting, int value) {
    const JsAppSettingsStorageSettingContext* context = setting->context;

    return js_app_settings_validate_value(context->node, &value);
}

static bool js_app_settings_storage_is_valid_string(
    const SettingProviderSetting* setting,
    const char* value) {
    const JsAppSettingsStorageSettingContext* context = setting->context;

    return js_app_settings_validate_value(context->node, value);
}

static bool js_app_settings_storage_is_valid_value(
    const SettingProviderSetting* setting,
    const void* value) {
    const JsAppSettingsStorageSettingContext* context = setting->context;

    return js_app_settings_validate_value(context->node, value);
}

/* Serialization */

static bool js_app_settings_storage_color_serialize(
    const SettingProviderSetting* setting,
    const void* value,
    FuriString* string) {
    UNUSED(setting);

    js_app_settings_color_format(value, string);

    return true;
}

static bool js_app_settings_storage_color_deserialize(
    const SettingProviderSetting* setting,
    const char* string,
    void* value) {
    UNUSED(setting);

    return js_app_settings_color_parse(string, value);
}

static bool js_app_settings_storage_time_serialize(
    const SettingProviderSetting* setting,
    const void* value,
    FuriString* string) {
    const JsAppSettingsStorageSettingContext* context = setting->context;

    bool is_valid = false;
    if(js_app_settings_validate_value(context->node, value)) {
        js_app_settings_time_format(value, string);
        is_valid = true;
    }

    return is_valid;
}

static bool js_app_settings_storage_time_deserialize(
    const SettingProviderSetting* setting,
    const char* string,
    void* value) {
    UNUSED(setting);

    return js_app_settings_time_parse(string, value);
}

static bool js_app_settings_storage_geo_serialize(
    const SettingProviderSetting* setting,
    const void* value,
    cJSON* json) {
    const JsAppSettingsStorageSettingContext* context = setting->context;
    const JsAppSettingsGeoValue* geo = value;

    bool is_valid = false;
    if(js_app_settings_validate_value(context->node, geo)) {
        cJSON_AddStringToObject(json, "mode", js_app_settings_geo_mode_format(geo->mode));
        cJSON_AddStringToObject(json, "name", geo->name);

        if(geo->mode == JsAppSettingsGeoModeFixed) {
            cJSON_AddNumberToObject(json, "lat", geo->latitude);
            cJSON_AddNumberToObject(json, "lon", geo->longitude);
        }

        is_valid = true;
    }

    return is_valid;
}

static bool js_app_settings_storage_geo_deserialize(
    const SettingProviderSetting* setting,
    const cJSON* json,
    void* value) {
    const JsAppSettingsStorageSettingContext* context = setting->context;
    JsAppSettingsGeoValue* geo = value;

    bool is_successful = false;

    do {
        cJSON* mode_json = cJSON_GetObjectItem(json, "mode");
        if(!cJSON_IsString(mode_json)) {
            break;
        }

        if(!js_app_settings_geo_mode_parse(mode_json->valuestring, &geo->mode)) {
            break;
        }

        cJSON* name_json = cJSON_GetObjectItem(json, "name");
        if(!cJSON_IsString(name_json)) {
            break;
        }

        const char* name = name_json->valuestring;
        if(strlen(name) > sizeof(geo->name) - 1) {
            break;
        }

        strcpy(geo->name, name);

        cJSON* lat_json = cJSON_GetObjectItem(json, "lat");
        geo->latitude = cJSON_GetNumberValue(lat_json);

        cJSON* lon_json = cJSON_GetObjectItem(json, "lon");
        geo->longitude = cJSON_GetNumberValue(lon_json);

        is_successful = js_app_settings_validate_value(context->node, geo);
    } while(false);

    return is_successful;
}

/* Construction */

static void* js_app_settings_storage_build_interface_bool(const JsAppSettingsNode* node) {
    const JsAppSettingsBoolData* node_data = node->data;

    SettingProviderBoolInterface* interface = malloc(sizeof(*interface));
    interface->default_value_callback = NULL;
    interface->default_value = node_data->default_value;

    return interface;
}

static void* js_app_settings_storage_build_interface_int(const JsAppSettingsNode* node) {
    const JsAppSettingsIntData* node_data = node->data;

    SettingProviderIntInterface* interface = malloc(sizeof(*interface));
    interface->is_valid_callback = js_app_settings_storage_is_valid_int;
    interface->default_value = node_data->default_value;

    return interface;
}

static void* js_app_settings_storage_build_interface_string(const JsAppSettingsNode* node) {
    const JsAppSettingsStringData* node_data = node->data;

    SettingProviderStringInterface* interface = malloc(sizeof(*interface));
    interface->is_valid_callback = js_app_settings_storage_is_valid_string;
    interface->default_value = node_data->default_value;
    interface->max_size = node_data->max_length + 1;

    return interface;
}

static void* js_app_settings_storage_build_interface_enum(const JsAppSettingsNode* node) {
    const JsAppSettingsEnumData* node_data = node->data;

    const char** enum_string_map = calloc(node_data->options_count, sizeof(const char*));
    for(size_t i = 0; i < node_data->options_count; i++) {
        enum_string_map[i] = node_data->options[i].value;
    }

    SettingProviderEnumInterface* interface = malloc(sizeof(*interface));
    interface->string_map = enum_string_map;
    interface->string_map_length = node_data->options_count;
    interface->type_size = sizeof(node_data->default_index);
    interface->default_value = &node_data->default_index;

    return interface;
}

static void* js_app_settings_storage_build_interface_color(const JsAppSettingsNode* node) {
    const JsAppSettingsColorData* node_data = node->data;

    SettingProviderCustomInterface* interface = malloc(sizeof(*interface));
    interface->is_valid_callback = NULL;
    interface->serialize_callback = js_app_settings_storage_color_serialize;
    interface->deserialize_callback = js_app_settings_storage_color_deserialize;
    interface->default_value_callback = NULL;
    interface->default_value = &node_data->default_value;
    interface->default_value_size = sizeof(node_data->default_value);

    return interface;
}

static void* js_app_settings_storage_build_interface_time(const JsAppSettingsNode* node) {
    const JsAppSettingsTimeData* node_data = node->data;

    SettingProviderCustomInterface* interface = malloc(sizeof(*interface));
    interface->is_valid_callback = js_app_settings_storage_is_valid_value;
    interface->serialize_callback = js_app_settings_storage_time_serialize;
    interface->deserialize_callback = js_app_settings_storage_time_deserialize;
    interface->default_value_callback = NULL;
    interface->default_value = &node_data->default_value;
    interface->default_value_size = sizeof(node_data->default_value);

    return interface;
}

static void* js_app_settings_storage_build_interface_geo(const JsAppSettingsNode* node) {
    const JsAppSettingsGeoData* node_data = node->data;

    SettingProviderRawInterface* interface = malloc(sizeof(*interface));
    interface->is_valid_callback = js_app_settings_storage_is_valid_value;
    interface->serialize_callback = js_app_settings_storage_geo_serialize;
    interface->deserialize_callback = js_app_settings_storage_geo_deserialize;
    interface->default_value = &node_data->default_value;
    interface->default_value_size = sizeof(node_data->default_value);

    return interface;
}

static void js_app_settings_storage_build_group(
    JsAppSettingsStorage* instance,
    const JsAppSettingsNode* nodes,
    size_t nodes_count,
    SettingProviderSetting* settings,
    size_t initial_offset) {
    for(size_t cursor = initial_offset; nodes_count > 0; nodes++, settings++, nodes_count--) {
        JsAppSettingsStorageSettingContext* context = malloc(sizeof(*context));
        context->node = nodes;
        context->value = instance->values + cursor;

        const JsAppSettingsStorageInterfaceBuilder* builder =
            &js_app_settings_storage_interface_builders[nodes->type];

        settings->name = nodes->id;
        settings->context = context;
        settings->field_offset = cursor - initial_offset;
        settings->type = builder->setting_type;

        if(nodes->type == JsAppSettingsNodeTypeGroup) {
            const JsAppSettingsGroupData* node_data = nodes->data;
            size_t inner_nodes_count = node_data->nodes_count;

            SettingProviderStructInterface* interface = malloc(sizeof(*interface));
            SettingProviderSetting* inner_settings =
                calloc(inner_nodes_count, sizeof(*inner_settings));

            interface->is_valid_callback = NULL;
            interface->inner_settings = inner_settings;
            interface->inner_settings_count = inner_nodes_count;

            js_app_settings_storage_build_group(
                instance, node_data->nodes, inner_nodes_count, inner_settings, cursor);

            settings->interface = interface;
        } else {
            settings->interface = builder->build(nodes);
        }

        cursor += JS_APP_SETTINGS_STORAGE_ALIGN(js_app_settings_storage_compute_node_size(nodes));
    }
}

/* Destruction */

static void
    js_app_settings_storage_free_group(const SettingProviderSetting* settings, size_t count) {
    for(; count > 0; settings++, count--) {
        SettingProviderSettingType type = settings->type;

        if(type == SettingProviderSettingTypeStruct) {
            const SettingProviderStructInterface* interface = settings->interface;
            js_app_settings_storage_free_group(
                interface->inner_settings, interface->inner_settings_count);
            free((void*)interface->inner_settings);
        } else if(type == SettingProviderSettingTypeEnum) {
            const SettingProviderEnumInterface* interface = settings->interface;
            free((void*)interface->string_map);
        }

        free((void*)settings->context);
        free((void*)settings->interface);
    }
}

/* Schema Parsing */

static JsAppSettings* js_app_settings_storage_parse_schema(const char* app_id) {
    FuriString* schema_path_builder =
        furi_string_alloc_printf(JS_APP_SETTINGS_STORAGE_SCHEMA_PATH_FORMAT, app_id);

    JsAppSettings* settings = NULL;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        const char* schema_path = furi_string_get_cstr(schema_path_builder);
        if(!storage_file_open(file, schema_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            if(storage_file_get_error(file) != FSE_NOT_EXIST) {
                FURI_LOG_E(TAG, "Failed to open schema file %s", schema_path);
            }

            break;
        }

        uint64_t file_size = storage_file_size(file);
        if(file_size == 0) {
            FURI_LOG_E(TAG, "Schema file %s is empty", schema_path);
            break;
        }

        char* file_buffer = malloc(file_size);
        if(storage_file_read(file, file_buffer, file_size) == file_size) {
            settings = js_app_settings_parse(file_buffer, file_size);
        } else {
            FURI_LOG_E(TAG, "Failed to read schema file %s", schema_path);
        }

        free(file_buffer);
    } while(false);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    furi_string_free(schema_path_builder);

    return settings;
}

/* Public API Implementation */

JsAppSettingsStorage* js_app_settings_storage_alloc(const char* app_id) {
    furi_check(app_id);

    JsAppSettingsStorage* instance = NULL;

    do {
        JsAppSettings* settings = js_app_settings_storage_parse_schema(app_id);
        if(!settings) {
            break;
        }

        JsAppSettingsNode* nodes = settings->nodes;
        size_t nodes_count = settings->nodes_count;

        instance = malloc(sizeof(*instance));
        instance->settings = settings;
        instance->values = malloc(js_app_settings_storage_compute_size(nodes, nodes_count));

        instance->root_fields = calloc(nodes_count, sizeof(*instance->root_fields));

        instance->root_interface = (SettingProviderStructInterface){
            .inner_settings = instance->root_fields,
            .inner_settings_count = nodes_count,
        };

        instance->root = (SettingProviderSetting){
            .interface = &instance->root_interface,
            .type = SettingProviderSettingTypeStruct,
        };

        js_app_settings_storage_build_group(
            instance, nodes, nodes_count, instance->root_fields, 0);

        FuriString* data_path =
            furi_string_alloc_printf(JS_APP_SETTINGS_STORAGE_DATA_PATH_FORMAT, app_id);
        instance->provider =
            setting_provider_alloc(furi_string_get_cstr(data_path), settings->version, NULL, 0);
        furi_string_free(data_path);
    } while(false);

    return instance;
}

void js_app_settings_storage_free(JsAppSettingsStorage* instance) {
    furi_check(instance);

    setting_provider_free(instance->provider);

    js_app_settings_storage_free_group(instance->root_fields, instance->settings->nodes_count);

    free(instance->root_fields);
    free(instance->values);

    js_app_settings_free(instance->settings);
    free(instance);
}

bool js_app_settings_storage_load(JsAppSettingsStorage* instance) {
    furi_check(instance);

    return setting_provider_load(instance->provider, &instance->root, instance->values);
}

bool js_app_settings_storage_save(JsAppSettingsStorage* instance) {
    furi_check(instance);

    return setting_provider_save(instance->provider, &instance->root, instance->values);
}

const SettingProviderSetting* js_app_settings_storage_get_root(JsAppSettingsStorage* instance) {
    furi_check(instance);

    return &instance->root;
}

const JsAppSettingsNode* js_app_settings_storage_get_node(const SettingProviderSetting* instance) {
    furi_check(instance);
    furi_check(instance->context);

    const JsAppSettingsStorageSettingContext* context = instance->context;

    return context->node;
}

void* js_app_settings_storage_get_value(const SettingProviderSetting* instance) {
    furi_check(instance);
    furi_check(instance->context);

    const JsAppSettingsStorageSettingContext* context = instance->context;

    return context->value;
}

/* Tables */

static const JsAppSettingsStorageInterfaceBuilder js_app_settings_storage_interface_builders[] = {
    [JsAppSettingsNodeTypeBool] =
        {
            .build = js_app_settings_storage_build_interface_bool,
            .setting_type = SettingProviderSettingTypeBool,
        },
    [JsAppSettingsNodeTypeInt] =
        {
            .build = js_app_settings_storage_build_interface_int,
            .setting_type = SettingProviderSettingTypeInt,
        },
    [JsAppSettingsNodeTypeString] =
        {
            .build = js_app_settings_storage_build_interface_string,
            .setting_type = SettingProviderSettingTypeString,
        },
    [JsAppSettingsNodeTypeEnum] =
        {
            .build = js_app_settings_storage_build_interface_enum,
            .setting_type = SettingProviderSettingTypeEnum,
        },
    [JsAppSettingsNodeTypeColor] =
        {
            .build = js_app_settings_storage_build_interface_color,
            .setting_type = SettingProviderSettingTypeCustom,
        },
    [JsAppSettingsNodeTypeTime] =
        {
            .build = js_app_settings_storage_build_interface_time,
            .setting_type = SettingProviderSettingTypeCustom,
        },
    [JsAppSettingsNodeTypeGeo] =
        {
            .build = js_app_settings_storage_build_interface_geo,
            .setting_type = SettingProviderSettingTypeRaw,
        },
    [JsAppSettingsNodeTypeGroup] =
        {
            .build = NULL,
            .setting_type = SettingProviderSettingTypeStruct,
        },
};

static_assert(COUNT_OF(js_app_settings_storage_interface_builders) == JsAppSettingsNodeTypesCount);

static const size_t js_app_settings_storage_sizes[] = {
    [JsAppSettingsNodeTypeBool] = sizeof(bool),
    [JsAppSettingsNodeTypeInt] = sizeof(int),
    [JsAppSettingsNodeTypeString] = 0,
    [JsAppSettingsNodeTypeEnum] = sizeof(int),
    [JsAppSettingsNodeTypeColor] = sizeof(Color),
    [JsAppSettingsNodeTypeTime] = sizeof(JsAppSettingsTimeValue),
    [JsAppSettingsNodeTypeGeo] = sizeof(JsAppSettingsGeoValue),
    [JsAppSettingsNodeTypeGroup] = 0,
};

static_assert(COUNT_OF(js_app_settings_storage_sizes) == JsAppSettingsNodeTypesCount);
