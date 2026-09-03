#include "../js_app_launcher_i.h"
#include "js_app_launcher_scenes.h"

#include <gui/modules/var_item_list.h>

typedef struct {
    JsAppLauncher* instance;
    const SettingProviderSetting* setting;
    const JsAppSettingsNode* node;
} JsAppLauncherSceneSetupItemContext;

typedef struct {
    VarItemList* front_list;
    VarItemList* back_list;
    JsAppLauncherSceneSetupItemContext* contexts;
    FuriMutex* settings_mutex;
} JsAppLauncherSceneSetup;

typedef struct {
    VarItem* (*build)(
        VarItemList* list,
        JsAppLauncherSceneSetupItemContext* context,
        VarItemChangeCallback callback);
    void (*store)(void* value, int32_t item_value);
} JsAppLauncherSceneSetupItemEntry;

static const JsAppLauncherSceneSetupItemEntry js_app_launcher_scene_setup_items[];

static VarItem* js_app_launcher_scene_setup_build_bool(
    VarItemList* list,
    JsAppLauncherSceneSetupItemContext* context,
    VarItemChangeCallback callback) {
    const JsAppSettingsNode* node = context->node;
    const bool* value = js_app_settings_storage_get_value(context->setting);

    VarItem* item = var_item_list_add_switch(list, node->label, callback, context);
    var_item_set_value(item, *value);

    return item;
}

static VarItem* js_app_launcher_scene_setup_build_integer(
    VarItemList* list,
    JsAppLauncherSceneSetupItemContext* context,
    VarItemChangeCallback callback) {
    const JsAppSettingsNode* node = context->node;
    const JsAppSettingsIntData* node_data = node->data;
    const int* value = js_app_settings_storage_get_value(context->setting);

    VarItem* item = var_item_list_add_spinbox(
        list,
        node->label,
        NULL,
        node_data->min_value,
        node_data->max_value,
        node_data->value_step,
        callback,
        context);
    var_item_set_value(item, *value);

    return item;
}

static VarItem* js_app_launcher_scene_setup_build_string(
    VarItemList* list,
    JsAppLauncherSceneSetupItemContext* context,
    VarItemChangeCallback callback) {
    const JsAppSettingsNode* node = context->node;
    const JsAppSettingsStringData* node_data = node->data;
    const char* value =
        node_data->is_sensitive ? "***" : js_app_settings_storage_get_value(context->setting);

    return var_item_list_add_selector(
        list, node->label, NULL, (const char*[]){value}, 1, callback, context);
}

static VarItem* js_app_launcher_scene_setup_build_enum(
    VarItemList* list,
    JsAppLauncherSceneSetupItemContext* context,
    VarItemChangeCallback callback) {
    const JsAppSettingsNode* node = context->node;
    const JsAppSettingsEnumData* node_data = node->data;

    const char** labels = calloc(node_data->options_count, sizeof(*labels));
    for(size_t i = 0; i < node_data->options_count; i++) {
        labels[i] = node_data->options[i].label;
    }

    VarItem* item = var_item_list_add_selector(
        list, node->label, NULL, labels, node_data->options_count, callback, context);

    free(labels);

    const int* index_value = js_app_settings_storage_get_value(context->setting);
    var_item_set_value(item, *index_value);

    return item;
}

static VarItem* js_app_launcher_scene_setup_build_color(
    VarItemList* list,
    JsAppLauncherSceneSetupItemContext* context,
    VarItemChangeCallback callback) {
    const JsAppSettingsNode* node = context->node;
    FuriString* string = furi_string_alloc();

    js_app_settings_color_format(js_app_settings_storage_get_value(context->setting), string);
    VarItem* item = var_item_list_add_selector(
        list,
        node->label,
        NULL,
        (const char*[]){furi_string_get_cstr(string)},
        1,
        callback,
        context);

    furi_string_free(string);

    return item;
}

static VarItem* js_app_launcher_scene_setup_build_time(
    VarItemList* list,
    JsAppLauncherSceneSetupItemContext* context,
    VarItemChangeCallback callback) {
    const JsAppSettingsNode* node = context->node;
    FuriString* string = furi_string_alloc();

    js_app_settings_time_format(js_app_settings_storage_get_value(context->setting), string);
    VarItem* item = var_item_list_add_selector(
        list,
        node->label,
        NULL,
        (const char*[]){furi_string_get_cstr(string)},
        1,
        callback,
        context);

    furi_string_free(string);

    return item;
}

static VarItem* js_app_launcher_scene_setup_build_geo(
    VarItemList* list,
    JsAppLauncherSceneSetupItemContext* context,
    VarItemChangeCallback callback) {
    const JsAppSettingsNode* node = context->node;

    const JsAppSettingsGeoValue* geo = js_app_settings_storage_get_value(context->setting);
    const char* value = (*geo->name != '\0') ? geo->name :
                                               js_app_settings_geo_mode_format(geo->mode);

    return var_item_list_add_selector(
        list, node->label, NULL, (const char*[]){value}, 1, callback, context);
}

static void js_app_launcher_scene_setup_store_bool(void* value, int32_t item_value) {
    *(bool*)value = item_value;
}

static void js_app_launcher_scene_setup_store_integer(void* value, int32_t item_value) {
    *(int*)value = item_value;
}

static void js_app_launcher_scene_setup_store_enum(void* value, int32_t item_value) {
    *(int*)value = item_value;
}

static size_t js_app_launcher_scene_setup_count_items(
    const SettingProviderSetting* settings,
    size_t settings_count) {
    size_t items_count = 0;

    for(size_t i = 0; i < settings_count; i++) {
        const SettingProviderSetting* setting = &settings[i];

        if(setting->type == SettingProviderSettingTypeStruct) {
            const SettingProviderStructInterface* interface = setting->interface;
            items_count += js_app_launcher_scene_setup_count_items(
                interface->inner_settings, interface->inner_settings_count);
        } else {
            items_count++;
        }
    }

    return items_count;
}

static void js_app_launcher_scene_setup_add_items(
    JsAppLauncher* instance,
    VarItemList* list,
    const SettingProviderSetting* settings,
    size_t settings_count,
    JsAppLauncherSceneSetupItemContext** contexts,
    VarItemChangeCallback callback) {
    for(size_t i = 0; i < settings_count; i++) {
        const SettingProviderSetting* setting = &settings[i];

        if(setting->type == SettingProviderSettingTypeStruct) {
            const SettingProviderStructInterface* interface = setting->interface;
            js_app_launcher_scene_setup_add_items(
                instance,
                list,
                interface->inner_settings,
                interface->inner_settings_count,
                contexts,
                callback);
        } else {
            const JsAppSettingsNode* node = js_app_settings_storage_get_node(setting);
            const JsAppLauncherSceneSetupItemEntry* entry =
                &js_app_launcher_scene_setup_items[node->type];

            JsAppLauncherSceneSetupItemContext* item_context = (*contexts)++;
            item_context->instance = instance;
            item_context->setting = setting;
            item_context->node = node;

            entry->build(list, item_context, entry->store ? callback : NULL);
        }
    }
}

static void js_app_launcher_scene_setup_value_changed(VarItem* item, void* context) {
    const JsAppLauncherSceneSetupItemContext* item_context = context;

    JsAppLauncher* instance = item_context->instance;
    JsAppLauncherSceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdSetup);

    furi_mutex_acquire(data->settings_mutex, FuriWaitForever);
    js_app_launcher_scene_setup_items[item_context->node->type].store(
        js_app_settings_storage_get_value(item_context->setting), var_item_get_value(item));
    furi_mutex_release(data->settings_mutex);

    uint32_t event = JsAppLauncherCustomEventSettingsChanged;
    furi_message_queue_put(instance->event_queue, &event, 0);
}

static void js_app_launcher_scene_setup_on_enter(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;

    JsAppLauncherSceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdSetup);

    data->front_list = NULL;
    data->back_list = NULL;
    data->contexts = NULL;
    data->settings_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    do {
        if(!instance->settings_storage) {
            instance->error = JsAppLauncherErrorSettingsMissing;
            scene_manager_replace_current_scene(
                instance->scene_manager, JsAppLauncherSceneIdError);
            break;
        };

        if(!js_app_settings_storage_load(instance->settings_storage)) {
            instance->error = JsAppLauncherErrorSettingsLoadFailed;
            scene_manager_replace_current_scene(
                instance->scene_manager, JsAppLauncherSceneIdError);
            break;
        };

        with_gui(instance->gui, {
            const SettingProviderSetting* root =
                js_app_settings_storage_get_root(instance->settings_storage);
            const SettingProviderStructInterface* interface = root->interface;

            data->front_list = var_item_list_alloc(instance->front_window);
            data->back_list = var_item_list_alloc(instance->back_window);

            data->contexts = calloc(
                js_app_launcher_scene_setup_count_items(
                    interface->inner_settings, interface->inner_settings_count),
                sizeof(*data->contexts));

            js_app_launcher_scene_setup_add_items(
                instance,
                data->front_list,
                interface->inner_settings,
                interface->inner_settings_count,
                &(JsAppLauncherSceneSetupItemContext*){data->contexts},
                js_app_launcher_scene_setup_value_changed);

            js_app_launcher_scene_setup_add_items(
                instance,
                data->back_list,
                interface->inner_settings,
                interface->inner_settings_count,
                &(JsAppLauncherSceneSetupItemContext*){data->contexts},
                NULL);

            widget_set_scrollbar_enabled(var_item_list_get_base(data->front_list), true);
            widget_set_scrollbar_enabled(var_item_list_get_base(data->back_list), true);
        });
    } while(false);
}

static void js_app_launcher_scene_setup_on_exit(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;

    JsAppLauncherSceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdSetup);

    with_gui(instance->gui, {
        nav_bar_pop_location(instance->nav_bar);

        if(data->front_list) {
            var_item_list_free(data->front_list);
        }

        if(data->back_list) {
            var_item_list_free(data->back_list);
        }
    });

    free(data->contexts);

    furi_mutex_free(data->settings_mutex);
}

static bool js_app_launcher_scene_setup_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    bool consumed = false;

    JsAppLauncher* instance = context;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == JsAppLauncherCustomEventSettingsChanged) {
            JsAppLauncherSceneSetup* data =
                scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdSetup);

            furi_mutex_acquire(data->settings_mutex, FuriWaitForever);
            js_app_settings_storage_save(instance->settings_storage);
            furi_mutex_release(data->settings_mutex);

            consumed = true;
        }
    }

    return consumed;
}

const Scene js_app_launcher_scene_setup = {
    .data_size = sizeof(JsAppLauncherSceneSetup),
    .enter_callback = js_app_launcher_scene_setup_on_enter,
    .exit_callback = js_app_launcher_scene_setup_on_exit,
    .event_callback = js_app_launcher_scene_setup_on_event,
};

static const JsAppLauncherSceneSetupItemEntry js_app_launcher_scene_setup_items[] = {
    [JsAppSettingsNodeTypeBool] =
        {
            .build = js_app_launcher_scene_setup_build_bool,
            .store = js_app_launcher_scene_setup_store_bool,
        },
    [JsAppSettingsNodeTypeInt] =
        {
            .build = js_app_launcher_scene_setup_build_integer,
            .store = js_app_launcher_scene_setup_store_integer,
        },
    [JsAppSettingsNodeTypeString] =
        {
            .build = js_app_launcher_scene_setup_build_string,
            .store = NULL,
        },
    [JsAppSettingsNodeTypeEnum] =
        {
            .build = js_app_launcher_scene_setup_build_enum,
            .store = js_app_launcher_scene_setup_store_enum,
        },
    [JsAppSettingsNodeTypeColor] =
        {
            .build = js_app_launcher_scene_setup_build_color,
            .store = NULL,
        },
    [JsAppSettingsNodeTypeTime] =
        {
            .build = js_app_launcher_scene_setup_build_time,
            .store = NULL,
        },
    [JsAppSettingsNodeTypeGeo] =
        {
            .build = js_app_launcher_scene_setup_build_geo,
            .store = NULL,
        },
    [JsAppSettingsNodeTypeGroup] =
        {
            .build = NULL,
            .store = NULL,
        },
};

static_assert(COUNT_OF(js_app_launcher_scene_setup_items) == JsAppSettingsNodeTypesCount);
