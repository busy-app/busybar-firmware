#include "js_app_settings.h"

#include <furi.h>
#include <storage/storage.h>

#include <ctype.h>
#include <limits.h>

#define TAG "JsAppSettings"

#define JS_APP_SETTINGS_FORMAT_VERSION        (1)
#define JS_APP_SETTINGS_VALUES_FORMAT_VERSION (1)

#define JS_APP_SETTINGS_MAX_FILE_SIZE (32 * 1024)
#define JS_APP_SETTINGS_MAX_DEPTH     (4)
#define JS_APP_SETTINGS_MAX_COUNT     (64)
#define JS_APP_SETTINGS_MAX_OPTIONS   (32)

#define JS_APP_SETTINGS_DATABASE_DIR EXT_PATH("apps_data/jsrunner")

#define KEY_FORMAT_VERSION    "format_version"
#define KEY_SETTINGS          "settings"
#define KEY_NAME              "name"
#define KEY_LABEL             "label"
#define KEY_TYPE              "type"
#define KEY_SUB_LABEL_SETTING "sub_label_setting"
#define KEY_OPTIONS           "options"
#define KEY_VALUE             "value"
#define KEY_DEFAULT           "default"
#define KEY_MIN               "min"
#define KEY_MAX               "max"
#define KEY_STEP              "step"
#define KEY_SUFFIX            "suffix"
#define KEY_VISIBLE_IF        "visible_if"
#define KEY_SETTING           "setting"
#define KEY_EQUALS            "equals"
#define KEY_VERSION           "version"
#define KEY_VALUES            "values"

typedef struct {
    const char* value;
    const char* label;
} JsAppSettingOption;

struct JsAppSetting {
    JsAppSettingType type;
    const char* name;
    const char* label;
    const char* suffix;

    JsAppSetting* parent;
    JsAppSetting* settings;
    size_t settings_count;
    const char* sub_label_setting_name;
    JsAppSetting* sub_label_setting;

    JsAppSettingOption* options;
    size_t options_count;

    int32_t min;
    int32_t max;
    int32_t step;

    const char* visible_setting_name;
    JsAppSetting* visible_setting;
    const cJSON* visible_equals;

    const cJSON* default_value;
    cJSON* value;
};

struct JsAppSettings {
    cJSON* schema_json;
    cJSON* values_json;
    JsAppSetting* settings;
    size_t settings_count;
    int32_t schema_version;
    FuriString* file_path;
    bool dirty;
};

static const struct {
    const char* name;
    JsAppSettingType type;
} setting_types[] = {
    {.name = "group", .type = JsAppSettingTypeGroup},
    {.name = "switch", .type = JsAppSettingTypeSwitch},
    {.name = "selector", .type = JsAppSettingTypeSelector},
    {.name = "spinbox", .type = JsAppSettingTypeSpinbox},
    {.name = "timebox", .type = JsAppSettingTypeTimebox},
};

static void js_app_setting_clear(JsAppSetting* setting) {
    for(size_t i = 0; i < setting->settings_count; ++i) {
        js_app_setting_clear(&setting->settings[i]);
    }
    free(setting->settings);
    free(setting->options);
}

static void js_app_settings_reset(JsAppSettings* instance) {
    for(size_t i = 0; i < instance->settings_count; ++i) {
        js_app_setting_clear(&instance->settings[i]);
    }
    free(instance->settings);
    instance->settings = NULL;
    instance->settings_count = 0;
    instance->schema_version = 0;

    cJSON_Delete(instance->schema_json);
    instance->schema_json = NULL;
    cJSON_Delete(instance->values_json);
    instance->values_json = NULL;
    furi_string_reset(instance->file_path);
    instance->dirty = false;
}

static bool js_app_setting_name_is_valid(const char* name) {
    if(!name || !islower((unsigned char)name[0])) return false;

    for(const char* cursor = name + 1; *cursor; ++cursor) {
        if(!islower((unsigned char)*cursor) && !isdigit((unsigned char)*cursor) &&
           (*cursor != '_')) {
            return false;
        }
    }

    return true;
}

static bool js_app_setting_json_get_int(const cJSON* object, const char* key, int32_t* value) {
    const cJSON* item = cJSON_GetObjectItem(object, key);
    if(!cJSON_IsNumber(item)) return false;

    const double number = cJSON_GetNumberValue(item);
    if((number < INT32_MIN) || (number > INT32_MAX) || (number != (int32_t)number)) return false;

    *value = number;
    return true;
}

static bool js_app_setting_parse_type(const cJSON* json, JsAppSettingType* type) {
    const cJSON* item = cJSON_GetObjectItem(json, KEY_TYPE);
    if(!cJSON_IsString(item)) return false;

    for(size_t i = 0; i < COUNT_OF(setting_types); ++i) {
        if(strcmp(item->valuestring, setting_types[i].name) == 0) {
            *type = setting_types[i].type;
            return true;
        }
    }

    return false;
}

static bool js_app_setting_parse_options(JsAppSetting* setting, const cJSON* json) {
    const cJSON* options = cJSON_GetObjectItem(json, KEY_OPTIONS);
    if(!cJSON_IsArray(options)) return false;

    const size_t count = cJSON_GetArraySize(options);
    if((count == 0) || (count > JS_APP_SETTINGS_MAX_OPTIONS)) return false;

    setting->options = malloc(sizeof(JsAppSettingOption) * count);
    setting->options_count = count;

    for(size_t i = 0; i < count; ++i) {
        const cJSON* option = cJSON_GetArrayItem(options, i);
        const cJSON* value = cJSON_GetObjectItem(option, KEY_VALUE);
        const cJSON* label = cJSON_GetObjectItem(option, KEY_LABEL);
        if(!cJSON_IsObject(option) || !cJSON_IsString(value) || !cJSON_IsString(label)) {
            return false;
        }

        setting->options[i].value = value->valuestring;
        setting->options[i].label = label->valuestring;

        for(size_t previous = 0; previous < i; ++previous) {
            if(strcmp(setting->options[previous].value, value->valuestring) == 0) return false;
        }
    }

    return true;
}

static bool js_app_setting_value_is_valid(const JsAppSetting* setting, const cJSON* value) {
    if(setting->type == JsAppSettingTypeSwitch) {
        return cJSON_IsBool(value);
    }

    if((setting->type == JsAppSettingTypeSpinbox) || (setting->type == JsAppSettingTypeTimebox)) {
        if(!cJSON_IsNumber(value)) return false;
        const double number = cJSON_GetNumberValue(value);
        if((number < setting->min) || (number > setting->max) || (number != (int32_t)number)) {
            return false;
        }
        return (((int32_t)number - setting->min) % setting->step) == 0;
    }

    if(setting->type == JsAppSettingTypeSelector) {
        if(!cJSON_IsString(value)) return false;
        for(size_t i = 0; i < setting->options_count; ++i) {
            if(strcmp(setting->options[i].value, value->valuestring) == 0) return true;
        }
    }

    return false;
}

static bool js_app_setting_parse_value_properties(JsAppSetting* setting, const cJSON* json) {
    setting->default_value = cJSON_GetObjectItem(json, KEY_DEFAULT);
    if(!setting->default_value) return false;

    if(setting->type == JsAppSettingTypeSelector) {
        if(!js_app_setting_parse_options(setting, json)) return false;
    } else if(
        (setting->type == JsAppSettingTypeSpinbox) || (setting->type == JsAppSettingTypeTimebox)) {
        int64_t range;
        if(!js_app_setting_json_get_int(json, KEY_MIN, &setting->min) ||
           !js_app_setting_json_get_int(json, KEY_MAX, &setting->max) ||
           !js_app_setting_json_get_int(json, KEY_STEP, &setting->step) ||
           (setting->min > setting->max) || (setting->step <= 0)) {
            return false;
        }
        range = (int64_t)setting->max - setting->min;
        if((range > INT32_MAX) || (range % setting->step != 0) ||
           (setting->min % setting->step != 0)) {
            return false;
        }
    }

    const cJSON* suffix = cJSON_GetObjectItem(json, KEY_SUFFIX);
    if(suffix && !cJSON_IsString(suffix)) return false;
    setting->suffix = suffix ? suffix->valuestring : NULL;

    return js_app_setting_value_is_valid(setting, setting->default_value);
}

static bool js_app_settings_parse_array(
    JsAppSettings* instance,
    JsAppSetting* parent,
    const cJSON* settings_json,
    JsAppSetting** settings_out,
    size_t* settings_count_out,
    size_t depth,
    size_t* total_count);

static bool js_app_setting_parse(
    JsAppSettings* instance,
    JsAppSetting* setting,
    JsAppSetting* parent,
    const cJSON* json,
    size_t depth,
    size_t* total_count) {
    UNUSED(instance);
    memset(setting, 0, sizeof(*setting));
    setting->parent = parent;

    const cJSON* name = cJSON_GetObjectItem(json, KEY_NAME);
    const cJSON* label = cJSON_GetObjectItem(json, KEY_LABEL);
    if(!cJSON_IsObject(json) || !cJSON_IsString(name) || !cJSON_IsString(label) ||
       !js_app_setting_name_is_valid(name->valuestring) ||
       !js_app_setting_parse_type(json, &setting->type)) {
        return false;
    }

    setting->name = name->valuestring;
    setting->label = label->valuestring;

    const cJSON* visible_if = cJSON_GetObjectItem(json, KEY_VISIBLE_IF);
    if(visible_if) {
        const cJSON* visible_setting = cJSON_GetObjectItem(visible_if, KEY_SETTING);
        const cJSON* visible_equals = cJSON_GetObjectItem(visible_if, KEY_EQUALS);
        if(!cJSON_IsObject(visible_if) || !cJSON_IsString(visible_setting) || !visible_equals) {
            return false;
        }
        setting->visible_setting_name = visible_setting->valuestring;
        setting->visible_equals = visible_equals;
    }

    if(setting->type == JsAppSettingTypeGroup) {
        const cJSON* children = cJSON_GetObjectItem(json, KEY_SETTINGS);
        if(!js_app_settings_parse_array(
               instance,
               setting,
               children,
               &setting->settings,
               &setting->settings_count,
               depth + 1,
               total_count)) {
            return false;
        }

        const cJSON* sub_label = cJSON_GetObjectItem(json, KEY_SUB_LABEL_SETTING);
        if(sub_label && !cJSON_IsString(sub_label)) return false;
        setting->sub_label_setting_name = sub_label ? sub_label->valuestring : NULL;
    } else if(!js_app_setting_parse_value_properties(setting, json)) {
        return false;
    }

    return true;
}

static bool js_app_settings_parse_array(
    JsAppSettings* instance,
    JsAppSetting* parent,
    const cJSON* settings_json,
    JsAppSetting** settings_out,
    size_t* settings_count_out,
    size_t depth,
    size_t* total_count) {
    if(!cJSON_IsArray(settings_json) || (depth > JS_APP_SETTINGS_MAX_DEPTH)) return false;

    const size_t count = cJSON_GetArraySize(settings_json);
    if((count == 0) || (*total_count + count > JS_APP_SETTINGS_MAX_COUNT)) return false;

    JsAppSetting* settings = malloc(sizeof(JsAppSetting) * count);
    memset(settings, 0, sizeof(JsAppSetting) * count);
    *settings_out = settings;
    *settings_count_out = count;
    *total_count += count;

    bool are_groups = false;
    for(size_t i = 0; i < count; ++i) {
        if(!js_app_setting_parse(
               instance,
               &settings[i],
               parent,
               cJSON_GetArrayItem(settings_json, i),
               depth,
               total_count)) {
            return false;
        }

        if(i == 0) {
            are_groups = settings[i].type == JsAppSettingTypeGroup;
        } else if(are_groups != (settings[i].type == JsAppSettingTypeGroup)) {
            FURI_LOG_E(TAG, "Settings level mixes groups and values");
            return false;
        }

        for(size_t previous = 0; previous < i; ++previous) {
            if(strcmp(settings[previous].name, settings[i].name) == 0) return false;
        }
    }

    return true;
}

static JsAppSetting*
    js_app_settings_find_sibling(JsAppSettings* instance, JsAppSetting* setting, const char* name) {
    JsAppSetting* siblings = setting->parent ? setting->parent->settings : instance->settings;
    const size_t count = setting->parent ? setting->parent->settings_count :
                                           instance->settings_count;

    for(size_t i = 0; i < count; ++i) {
        if(strcmp(siblings[i].name, name) == 0) return &siblings[i];
    }
    return NULL;
}

static bool js_app_settings_resolve_references(
    JsAppSettings* instance,
    JsAppSetting* settings,
    size_t count) {
    for(size_t i = 0; i < count; ++i) {
        JsAppSetting* setting = &settings[i];

        if(setting->visible_setting_name) {
            setting->visible_setting =
                js_app_settings_find_sibling(instance, setting, setting->visible_setting_name);
            if(!setting->visible_setting || (setting->visible_setting == setting) ||
               (setting->visible_setting->type == JsAppSettingTypeGroup) ||
               !js_app_setting_value_is_valid(setting->visible_setting, setting->visible_equals)) {
                return false;
            }
        }

        if(setting->sub_label_setting_name) {
            setting->sub_label_setting = js_app_settings_find_sibling(
                instance, setting->settings, setting->sub_label_setting_name);
            if(!setting->sub_label_setting ||
               (setting->sub_label_setting->type == JsAppSettingTypeGroup)) {
                return false;
            }
        }

        if((setting->type == JsAppSettingTypeGroup) &&
           !js_app_settings_resolve_references(
               instance, setting->settings, setting->settings_count)) {
            return false;
        }
    }
    return true;
}

static cJSON* js_app_settings_read_json(const char* path) {
    cJSON* json = NULL;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) break;
        const size_t size = storage_file_size(file);
        if((size == 0) || (size > JS_APP_SETTINGS_MAX_FILE_SIZE)) break;

        char* data = malloc(size);
        if(storage_file_read(file, data, size) == size) {
            json = cJSON_ParseWithLength(data, size);
        }
        free(data);
    } while(false);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return json;
}

static bool js_app_settings_save(const JsAppSettings* instance) {
    bool success = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* temp_path =
        furi_string_alloc_printf("%s.tmp", furi_string_get_cstr(instance->file_path));
    char* data = NULL;

    do {
        if(!storage_simply_mkpath(storage, JS_APP_SETTINGS_DATABASE_DIR)) break;

        data = cJSON_Print(instance->values_json);
        if(!data) break;
        const size_t size = strlen(data);

        File* file = storage_file_alloc(storage);
        bool written = false;
        if(storage_file_open(
               file, furi_string_get_cstr(temp_path), FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            written = storage_file_write(file, data, size) == size;
        }
        storage_file_free(file);
        if(!written) break;

        // Swap the finished file in rather than truncating the live one, so a
        // power loss mid-write cannot leave the user with an empty settings
        // file that silently reads back as all-defaults.
        success = storage_common_rename(
                      storage,
                      furi_string_get_cstr(temp_path),
                      furi_string_get_cstr(instance->file_path)) == FSE_OK;
    } while(false);

    if(!success) storage_simply_remove(storage, furi_string_get_cstr(temp_path));

    free(data);
    furi_string_free(temp_path);
    furi_record_close(RECORD_STORAGE);
    return success;
}

static cJSON* js_app_setting_normalize_value(JsAppSetting* setting, const cJSON* source) {
    cJSON* value;
    if(setting->type == JsAppSettingTypeGroup) {
        value = cJSON_CreateObject();
        const cJSON* source_object = cJSON_IsObject(source) ? source : NULL;
        for(size_t i = 0; i < setting->settings_count; ++i) {
            JsAppSetting* child = &setting->settings[i];
            const cJSON* source_child =
                source_object ? cJSON_GetObjectItem(source_object, child->name) : NULL;
            cJSON_AddItemToObject(
                value, child->name, js_app_setting_normalize_value(child, source_child));
        }
    } else {
        value = cJSON_Duplicate(
            js_app_setting_value_is_valid(setting, source) ? source : setting->default_value,
            true);
    }
    setting->value = value;
    return value;
}

static const cJSON*
    js_app_settings_stored_values(const JsAppSettings* instance, const cJSON* stored) {
    if(!cJSON_IsObject(stored)) return NULL;

    const cJSON* format_version = cJSON_GetObjectItem(stored, KEY_FORMAT_VERSION);
    const cJSON* version = cJSON_GetObjectItem(stored, KEY_VERSION);
    if(!cJSON_IsNumber(format_version) ||
       (cJSON_GetNumberValue(format_version) != JS_APP_SETTINGS_VALUES_FORMAT_VERSION) ||
       !cJSON_IsNumber(version) || (cJSON_GetNumberValue(version) != instance->schema_version)) {
        return NULL;
    }
    return cJSON_GetObjectItem(stored, KEY_VALUES);
}

static void js_app_settings_normalize_values(JsAppSettings* instance, const cJSON* stored_values) {
    instance->values_json = cJSON_CreateObject();
    cJSON_AddNumberToObject(
        instance->values_json, KEY_FORMAT_VERSION, JS_APP_SETTINGS_VALUES_FORMAT_VERSION);
    cJSON_AddNumberToObject(instance->values_json, KEY_VERSION, instance->schema_version);
    cJSON* values = cJSON_AddObjectToObject(instance->values_json, KEY_VALUES);

    for(size_t i = 0; i < instance->settings_count; ++i) {
        JsAppSetting* setting = &instance->settings[i];
        const cJSON* source = cJSON_IsObject(stored_values) ?
                                  cJSON_GetObjectItem(stored_values, setting->name) :
                                  NULL;
        cJSON_AddItemToObject(
            values, setting->name, js_app_setting_normalize_value(setting, source));
    }
}

JsAppSettings* js_app_settings_alloc(void) {
    JsAppSettings* instance = malloc(sizeof(*instance));
    memset(instance, 0, sizeof(*instance));
    instance->file_path = furi_string_alloc();
    return instance;
}

void js_app_settings_free(JsAppSettings* instance) {
    furi_check(instance);
    js_app_settings_reset(instance);
    furi_string_free(instance->file_path);
    free(instance);
}

bool js_app_settings_load(JsAppSettings* instance, const char* app_id, const char* schema_path) {
    furi_check(instance);
    furi_check(app_id);
    furi_check(schema_path);
    js_app_settings_reset(instance);

    bool success = false;
    do {
        instance->schema_json = js_app_settings_read_json(schema_path);
        if(!cJSON_IsObject(instance->schema_json)) break;

        const cJSON* format_version =
            cJSON_GetObjectItem(instance->schema_json, KEY_FORMAT_VERSION);
        const cJSON* settings_json = cJSON_GetObjectItem(instance->schema_json, KEY_SETTINGS);
        if(!cJSON_IsNumber(format_version) ||
           (cJSON_GetNumberValue(format_version) != JS_APP_SETTINGS_FORMAT_VERSION) ||
           !js_app_setting_json_get_int(
               instance->schema_json, KEY_VERSION, &instance->schema_version) ||
           (instance->schema_version < 1)) {
            break;
        }
        size_t total_count = 0;
        if(!js_app_settings_parse_array(
               instance,
               NULL,
               settings_json,
               &instance->settings,
               &instance->settings_count,
               0,
               &total_count) ||
           !js_app_settings_resolve_references(
               instance, instance->settings, instance->settings_count)) {
            break;
        }

        furi_string_printf(
            instance->file_path, "%s/%s.json", JS_APP_SETTINGS_DATABASE_DIR, app_id);

        cJSON* stored = js_app_settings_read_json(furi_string_get_cstr(instance->file_path));
        js_app_settings_normalize_values(
            instance, js_app_settings_stored_values(instance, stored));

        // Loading runs on every app launch and on every HTTP settings read, so
        // it must not depend on the card being writable and must not rewrite
        // flash when the stored file already says exactly this. Persisting the
        // normalized form is a repair, not part of the read.
        const bool needs_repair = !cJSON_Compare(stored, instance->values_json, true);
        cJSON_Delete(stored);
        if(needs_repair && !js_app_settings_save(instance)) {
            FURI_LOG_W(TAG, "Could not persist normalized settings for %s", app_id);
        }
        success = true;
    } while(false);

    if(!success) js_app_settings_reset(instance);
    return success;
}

bool js_app_settings_remove(const char* app_id) {
    furi_check(app_id);

    FuriString* file_path =
        furi_string_alloc_printf("%s/%s.json", JS_APP_SETTINGS_DATABASE_DIR, app_id);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    const bool success = storage_simply_remove(storage, furi_string_get_cstr(file_path));
    furi_record_close(RECORD_STORAGE);
    furi_string_free(file_path);
    return success;
}

size_t js_app_settings_get_count(const JsAppSettings* instance, const JsAppSetting* group) {
    furi_check(instance);
    return group ? group->settings_count : instance->settings_count;
}

const JsAppSetting*
    js_app_settings_get(const JsAppSettings* instance, const JsAppSetting* group, size_t index) {
    furi_check(instance);
    const JsAppSetting* settings = group ? group->settings : instance->settings;
    const size_t count = group ? group->settings_count : instance->settings_count;
    return index < count ? &settings[index] : NULL;
}

const JsAppSetting* js_app_settings_find(const JsAppSettings* instance, const char* path) {
    furi_check(instance);
    furi_check(path);

    const JsAppSetting* group = NULL;
    const char* segment = path;
    while(*segment) {
        const char* dot = strchr(segment, '.');
        const size_t length = dot ? (size_t)(dot - segment) : strlen(segment);
        const JsAppSetting* found = NULL;
        const size_t count = js_app_settings_get_count(instance, group);
        for(size_t i = 0; i < count; ++i) {
            const JsAppSetting* candidate = js_app_settings_get(instance, group, i);
            if((strlen(candidate->name) == length) &&
               (strncmp(candidate->name, segment, length) == 0)) {
                found = candidate;
                break;
            }
        }
        if(!found) return NULL;
        if(!dot) return found;
        if(found->type != JsAppSettingTypeGroup) return NULL;
        group = found;
        segment = dot + 1;
    }
    return NULL;
}

JsAppSettingType js_app_setting_get_type(const JsAppSetting* setting) {
    furi_check(setting);
    return setting->type;
}

const char* js_app_setting_get_name(const JsAppSetting* setting) {
    furi_check(setting);
    return setting->name;
}

const char* js_app_setting_get_label(const JsAppSetting* setting) {
    furi_check(setting);
    return setting->label;
}

const char* js_app_setting_get_suffix(const JsAppSetting* setting) {
    furi_check(setting);
    return setting->suffix;
}

const JsAppSetting* js_app_setting_get_sub_label_setting(const JsAppSetting* setting) {
    furi_check(setting);
    return setting->sub_label_setting;
}

bool js_app_setting_is_visible(const JsAppSetting* setting) {
    furi_check(setting);
    if(!setting->visible_setting) return true;
    return cJSON_Compare(setting->visible_setting->value, setting->visible_equals, true);
}

int32_t js_app_setting_get_min(const JsAppSetting* setting) {
    furi_check(setting);
    return setting->min;
}

int32_t js_app_setting_get_max(const JsAppSetting* setting) {
    furi_check(setting);
    return setting->max;
}

int32_t js_app_setting_get_step(const JsAppSetting* setting) {
    furi_check(setting);
    return setting->step;
}

size_t js_app_setting_get_option_count(const JsAppSetting* setting) {
    furi_check(setting);
    return setting->options_count;
}

const char* js_app_setting_get_option_value(const JsAppSetting* setting, size_t index) {
    furi_check(setting);
    return index < setting->options_count ? setting->options[index].value : NULL;
}

const char* js_app_setting_get_option_label(const JsAppSetting* setting, size_t index) {
    furi_check(setting);
    return index < setting->options_count ? setting->options[index].label : NULL;
}

bool js_app_setting_get_bool(const JsAppSetting* setting) {
    furi_check(setting);
    return cJSON_IsTrue(setting->value);
}

int32_t js_app_setting_get_int(const JsAppSetting* setting) {
    furi_check(setting);
    return cJSON_GetNumberValue(setting->value);
}

const char* js_app_setting_get_string(const JsAppSetting* setting) {
    furi_check(setting);
    return cJSON_GetStringValue(setting->value);
}

static void js_app_setting_assign_value(cJSON* target, const cJSON* value) {
    if(cJSON_IsBool(value)) {
        cJSON_SetBoolValue(target, cJSON_IsTrue(value));
    } else if(cJSON_IsNumber(value)) {
        cJSON_SetNumberValue(target, cJSON_GetNumberValue(value));
    } else if(cJSON_IsString(value)) {
        cJSON_SetValuestring(target, value->valuestring);
    }
}

static bool js_app_settings_set_value(
    JsAppSettings* instance,
    const JsAppSetting* setting,
    const cJSON* value) {
    if(!js_app_setting_value_is_valid(setting, value)) return false;

    js_app_setting_assign_value(setting->value, value);
    instance->dirty = true;
    return true;
}

bool js_app_settings_set_bool(JsAppSettings* instance, const JsAppSetting* setting, bool value) {
    cJSON* json = cJSON_CreateBool(value);
    const bool success = js_app_settings_set_value(instance, setting, json);
    cJSON_Delete(json);
    return success;
}

bool js_app_settings_set_int(JsAppSettings* instance, const JsAppSetting* setting, int32_t value) {
    cJSON* json = cJSON_CreateNumber(value);
    const bool success = js_app_settings_set_value(instance, setting, json);
    cJSON_Delete(json);
    return success;
}

bool js_app_settings_set_string(
    JsAppSettings* instance,
    const JsAppSetting* setting,
    const char* value) {
    cJSON* json = cJSON_CreateString(value);
    const bool success = js_app_settings_set_value(instance, setting, json);
    cJSON_Delete(json);
    return success;
}

const cJSON* js_app_settings_get_schema_json(const JsAppSettings* instance) {
    furi_check(instance);
    return instance->schema_json;
}

const cJSON* js_app_settings_get_config_json(const JsAppSettings* instance) {
    furi_check(instance);
    return instance->values_json;
}

const cJSON* js_app_settings_get_values_json(const JsAppSettings* instance) {
    furi_check(instance);
    return cJSON_GetObjectItem(instance->values_json, KEY_VALUES);
}

static bool js_app_settings_validate_update(
    const JsAppSettings* instance,
    const JsAppSetting* group,
    const cJSON* values) {
    if(!cJSON_IsObject(values)) return false;

    const cJSON* value;
    cJSON_ArrayForEach(value, values) {
        const JsAppSetting* setting = NULL;
        const size_t count = js_app_settings_get_count(instance, group);
        for(size_t i = 0; i < count; ++i) {
            const JsAppSetting* candidate = js_app_settings_get(instance, group, i);
            if(strcmp(candidate->name, value->string) == 0) {
                setting = candidate;
                break;
            }
        }
        if(!setting) return false;

        if(setting->type == JsAppSettingTypeGroup) {
            if(!js_app_settings_validate_update(instance, setting, value)) return false;
        } else if(!js_app_setting_value_is_valid(setting, value)) {
            return false;
        }
    }
    return true;
}

static void js_app_settings_apply_update(
    JsAppSettings* instance,
    const JsAppSetting* group,
    const cJSON* values) {
    const cJSON* value;
    cJSON_ArrayForEach(value, values) {
        const size_t count = js_app_settings_get_count(instance, group);
        for(size_t i = 0; i < count; ++i) {
            const JsAppSetting* setting = js_app_settings_get(instance, group, i);
            if(strcmp(setting->name, value->string) != 0) continue;

            if(setting->type == JsAppSettingTypeGroup) {
                js_app_settings_apply_update(instance, setting, value);
            } else {
                js_app_setting_assign_value(setting->value, value);
            }
            break;
        }
    }
}

bool js_app_settings_commit(JsAppSettings* instance) {
    furi_check(instance);
    if(!instance->dirty) return true;

    const bool success = js_app_settings_save(instance);
    if(success) instance->dirty = false;
    return success;
}

bool js_app_settings_update_json(JsAppSettings* instance, const cJSON* values) {
    furi_check(instance);
    if(!js_app_settings_validate_update(instance, NULL, values)) return false;

    cJSON* previous = cJSON_Duplicate(js_app_settings_get_values_json(instance), true);
    js_app_settings_apply_update(instance, NULL, values);

    const bool success = js_app_settings_save(instance);
    if(success) {
        instance->dirty = false;
    } else {
        js_app_settings_apply_update(instance, NULL, previous);
    }
    cJSON_Delete(previous);
    return success;
}

bool js_app_settings_reload(JsAppSettings* instance) {
    furi_check(instance);
    if(furi_string_empty(instance->file_path)) return false;

    cJSON* stored = js_app_settings_read_json(furi_string_get_cstr(instance->file_path));
    const cJSON* stored_values = js_app_settings_stored_values(instance, stored);

    // normalize_values() builds a fresh values_json and re-points every
    // setting->value into it, so the old tree has to go first.
    cJSON_Delete(instance->values_json);
    js_app_settings_normalize_values(instance, stored_values);
    instance->dirty = false;

    cJSON_Delete(stored);
    return true;
}
