#include "../js_app_launcher_i.h"
#include "js_app_launcher_scenes.h"

#include <gui/modules/smenu.h>
#include <gui/modules/smenu_item_editable.h>
#include <gui/modules/smenu_item_simple.h>

typedef struct {
    SMenu* front_menu;
    SMenu* back_menu;

    const SettingProviderSetting* _Atomic pending_setting;

    FuriMutex* settings_mutex;
} JsAppLauncherSceneSetup;

typedef struct {
    SMenuItem* (*build)(SMenu* menu, const SettingProviderSetting* setting);
    void (*store)(void* value, int32_t item_value);
} JsAppLauncherSceneSetupItemTypeApi;

static const JsAppLauncherSceneSetupItemTypeApi js_app_launcher_scene_setup_item_type_apis[];

static SMenuItem*
    js_app_launcher_scene_setup_build_bool(SMenu* menu, const SettingProviderSetting* setting) {
    const JsAppSettingsNode* node = js_app_settings_storage_get_node(setting);

    SMenuItemEditable* item = smenu_item_editable_switch_alloc(smenu_get_base(menu));
    smenu_item_editable_set_primary_text(item, node->label);
    smenu_item_editable_set_value(item, *(bool*)js_app_settings_storage_get_value(setting));

    return smenu_item_editable_get_base(item);
}

static SMenuItem*
    js_app_launcher_scene_setup_build_integer(SMenu* menu, const SettingProviderSetting* setting) {
    const JsAppSettingsNode* node = js_app_settings_storage_get_node(setting);
    const JsAppSettingsIntData* node_data = node->data;

    SMenuItemEditable* item = smenu_item_editable_spinbox_alloc(
        smenu_get_base(menu),
        NULL,
        node_data->min_value,
        node_data->max_value,
        node_data->value_step);
    smenu_item_editable_set_primary_text(item, node->label);
    smenu_item_editable_set_value(item, *(int*)js_app_settings_storage_get_value(setting));

    return smenu_item_editable_get_base(item);
}

static SMenuItem*
    js_app_launcher_scene_setup_build_enum(SMenu* menu, const SettingProviderSetting* setting) {
    const JsAppSettingsNode* node = js_app_settings_storage_get_node(setting);
    const JsAppSettingsEnumData* node_data = node->data;

    const char** labels = calloc(node_data->options_count, sizeof(*labels));
    for(size_t i = 0; i < node_data->options_count; i++) {
        labels[i] = node_data->options[i].label;
    }

    SMenuItemEditable* item =
        smenu_item_editable_selector_alloc(smenu_get_base(menu), labels, node_data->options_count);

    free(labels);

    smenu_item_editable_set_primary_text(item, node->label);
    smenu_item_editable_set_value(item, *(int*)js_app_settings_storage_get_value(setting));

    return smenu_item_editable_get_base(item);
}

static SMenuItem*
    js_app_launcher_scene_setup_build_string(SMenu* menu, const SettingProviderSetting* setting) {
    const JsAppSettingsNode* node = js_app_settings_storage_get_node(setting);
    const JsAppSettingsStringData* node_data = node->data;

    SMenuItemSimple* item = smenu_item_simple_alloc(smenu_get_base(menu));
    smenu_item_simple_set_primary_text(item, node->label);
    smenu_item_simple_set_auxiliary_text(
        item, node_data->is_sensitive ? "***" : js_app_settings_storage_get_value(setting));

    return smenu_item_simple_get_base(item);
}

static SMenuItem*
    js_app_launcher_scene_setup_build_color(SMenu* menu, const SettingProviderSetting* setting) {
    const JsAppSettingsNode* node = js_app_settings_storage_get_node(setting);

    FuriString* aux_text = furi_string_alloc();
    js_app_settings_color_format(js_app_settings_storage_get_value(setting), aux_text);

    SMenuItemSimple* item = smenu_item_simple_alloc(smenu_get_base(menu));
    smenu_item_simple_set_primary_text(item, node->label);
    smenu_item_simple_set_auxiliary_text(item, furi_string_get_cstr(aux_text));

    furi_string_free(aux_text);

    return smenu_item_simple_get_base(item);
}

static SMenuItem*
    js_app_launcher_scene_setup_build_time(SMenu* menu, const SettingProviderSetting* setting) {
    const JsAppSettingsNode* node = js_app_settings_storage_get_node(setting);

    FuriString* string = furi_string_alloc();
    js_app_settings_time_format(js_app_settings_storage_get_value(setting), string);

    SMenuItemSimple* item = smenu_item_simple_alloc(smenu_get_base(menu));
    smenu_item_simple_set_primary_text(item, node->label);
    smenu_item_simple_set_auxiliary_text(item, furi_string_get_cstr(string));

    furi_string_free(string);

    return smenu_item_simple_get_base(item);
}

static SMenuItem*
    js_app_launcher_scene_setup_build_geo(SMenu* menu, const SettingProviderSetting* setting) {
    const JsAppSettingsNode* node = js_app_settings_storage_get_node(setting);

    const JsAppSettingsGeoValue* value = js_app_settings_storage_get_value(setting);
    const char* aux_text = (*value->name != '\0') ? value->name :
                                                    js_app_settings_geo_mode_format(value->mode);

    SMenuItemSimple* item = smenu_item_simple_alloc(smenu_get_base(menu));
    smenu_item_simple_set_primary_text(item, node->label);
    smenu_item_simple_set_auxiliary_text(item, aux_text);

    return smenu_item_simple_get_base(item);
}

static SMenuItem*
    js_app_launcher_scene_setup_build_group(SMenu* menu, const SettingProviderSetting* setting) {
    const JsAppSettingsNode* node = js_app_settings_storage_get_node(setting);

    SMenuItemSimple* item = smenu_item_simple_alloc(smenu_get_base(menu));
    smenu_item_simple_set_primary_text(item, node->label);

    return smenu_item_simple_get_base(item);
}

static void js_app_launcher_scene_setup_store_bool(void* value, int32_t item_value) {
    *(bool*)value = item_value;
}

static void js_app_launcher_scene_setup_store_integer(void* value, int32_t item_value) {
    *(int*)value = item_value;
}

static void js_app_launcher_scene_setup_value_changed_callback(SMenuItem* item) {
    JsAppLauncher* instance = widget_get_context(smenu_get_base(smenu_get_from_item(item)));
    JsAppLauncherSceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdSetup);

    const SettingProviderSetting* setting = widget_get_context(smenu_item_get_base(item));
    const JsAppSettingsNode* node = js_app_settings_storage_get_node(setting);

    int32_t value = smenu_item_editable_get_value((SMenuItemEditable*)item);

    furi_mutex_acquire(data->settings_mutex, FuriWaitForever);
    js_app_launcher_scene_setup_item_type_apis[node->type].store(
        js_app_settings_storage_get_value(setting), value);
    furi_mutex_release(data->settings_mutex);

    js_app_launcher_send_custom_event(instance, JsAppLauncherCustomEventSettingsChanged);
}

static void js_app_launcher_scene_setup_item_selected_callback(SMenuItem* item) {
    JsAppLauncher* instance = widget_get_context(smenu_get_base(smenu_get_from_item(item)));
    JsAppLauncherSceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdSetup);

    data->pending_setting = widget_get_context(smenu_item_get_base(item));

    js_app_launcher_send_custom_event(instance, JsAppLauncherCustomEventSettingsItemSelected);
}

static void js_app_launcher_scene_setup_build_items(
    SMenu* menu,
    const JsAppLauncherSceneSetupLevel* level,
    bool is_interactive) {
    for(size_t i = 0; i < level->settings_count; i++) {
        const SettingProviderSetting* setting = &level->settings[i];
        const JsAppSettingsNode* node = js_app_settings_storage_get_node(setting);
        const JsAppLauncherSceneSetupItemTypeApi* type_api =
            &js_app_launcher_scene_setup_item_type_apis[node->type];

        SMenuItem* item = type_api->build(menu, setting);

        if(is_interactive) {
            smenu_item_set_callback(
                item,
                type_api->store ? js_app_launcher_scene_setup_value_changed_callback :
                                  js_app_launcher_scene_setup_item_selected_callback);
            widget_set_context(smenu_item_get_base(item), (void*)setting);
        }

        if(setting == level->selected_setting) {
            smenu_set_selected_item_index(menu, i);
        }
    }
}

static void js_app_launcher_scene_setup_rebuild_level(JsAppLauncher* instance) {
    JsAppLauncherSceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdSetup);

    const JsAppLauncherSceneSetupLevel* level =
        JsAppLauncherSceneSetupLevels_back(instance->setup_levels);

    data->pending_setting = NULL;

    smenu_reset(data->front_menu);
    smenu_reset(data->back_menu);

    js_app_launcher_scene_setup_build_items(data->front_menu, level, true);
    js_app_launcher_scene_setup_build_items(data->back_menu, level, false);
}

static void
    js_app_launcher_scene_setup_raise_error(JsAppLauncher* instance, JsAppLauncherError error) {
    instance->error = error;
    JsAppLauncherSceneSetupLevels_reset(instance->setup_levels);
    with_gui(instance->gui, { nav_bar_pop_location(instance->nav_bar); });
    scene_manager_replace_current_scene(instance->scene_manager, JsAppLauncherSceneIdError);
}

static void js_app_launcher_scene_setup_on_enter(void* context) {
    furi_assert(context);

    JsAppLauncher* instance = context;
    JsAppLauncherSceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdSetup);

    data->front_menu = NULL;
    data->back_menu = NULL;
    data->settings_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    do {
        if(!instance->settings_storage) {
            js_app_launcher_scene_setup_raise_error(
                instance, JsAppLauncherErrorSettingsSchemaMissing);
            break;
        }

        if(!js_app_settings_storage_load(instance->settings_storage)) {
            js_app_launcher_scene_setup_raise_error(
                instance, JsAppLauncherErrorSettingsLoadFailed);
            break;
        }

        if(JsAppLauncherSceneSetupLevels_size(instance->setup_levels) == 0) {
            const SettingProviderSetting* root =
                js_app_settings_storage_get_root(instance->settings_storage);
            const SettingProviderStructInterface* interface = root->interface;

            JsAppLauncherSceneSetupLevels_push_back(
                instance->setup_levels,
                (JsAppLauncherSceneSetupLevel){
                    .settings = interface->inner_settings,
                    .settings_count = interface->inner_settings_count,
                    .selected_setting = NULL,
                });
        }

        with_gui(instance->gui, {
            data->front_menu = smenu_alloc(instance->front_window, true);
            widget_set_context(smenu_get_base(data->front_menu), instance);

            data->back_menu = smenu_alloc(instance->back_window, false);
            widget_set_context(smenu_get_base(data->back_menu), instance);

            js_app_launcher_scene_setup_rebuild_level(instance);
        });
    } while(false);
}

static void js_app_launcher_scene_setup_on_exit(void* context) {
    furi_assert(context);

    JsAppLauncher* instance = context;
    JsAppLauncherSceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdSetup);

    with_gui(instance->gui, {
        if(data->front_menu) {
            smenu_free(data->front_menu);
        }

        if(data->back_menu) {
            smenu_free(data->back_menu);
        }
    });

    furi_mutex_free(data->settings_mutex);
}

static bool js_app_launcher_scene_setup_on_settings_changed(JsAppLauncher* instance) {
    JsAppLauncherSceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdSetup);

    furi_mutex_acquire(data->settings_mutex, FuriWaitForever);
    js_app_settings_storage_save(instance->settings_storage);
    furi_mutex_release(data->settings_mutex);

    return true;
}

static bool js_app_launcher_scene_setup_on_item_selected(JsAppLauncher* instance) {
    JsAppLauncherSceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdSetup);
    JsAppLauncherSceneSetupLevel* level =
        JsAppLauncherSceneSetupLevels_back(instance->setup_levels);
    const SettingProviderSetting* setting = data->pending_setting;

    if(setting) {
        level->selected_setting = setting;

        if(setting->type == SettingProviderSettingTypeStruct) {
            const SettingProviderStructInterface* interface = setting->interface;
            JsAppLauncherSceneSetupLevels_push_back(
                instance->setup_levels,
                (JsAppLauncherSceneSetupLevel){
                    .settings = interface->inner_settings,
                    .settings_count = interface->inner_settings_count,
                    .selected_setting = NULL,
                });

            with_gui(instance->gui, { js_app_launcher_scene_setup_rebuild_level(instance); });
        } else {
            scene_manager_next_scene(instance->scene_manager, JsAppLauncherSceneIdWebSetup);
        }
    }

    return true;
}

static bool js_app_launcher_scene_setup_on_back(JsAppLauncher* instance) {
    JsAppLauncherSceneSetupLevels_pop_back(NULL, instance->setup_levels);

    bool is_consumed = false;
    if(JsAppLauncherSceneSetupLevels_size(instance->setup_levels) > 0) {
        with_gui(instance->gui, { js_app_launcher_scene_setup_rebuild_level(instance); });

        is_consumed = true;
    } else {
        with_gui(instance->gui, { nav_bar_pop_location(instance->nav_bar); });
    }

    return is_consumed;
}

static bool js_app_launcher_scene_setup_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    JsAppLauncher* instance = context;

    bool is_consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == JsAppLauncherCustomEventSettingsChanged) {
            is_consumed = js_app_launcher_scene_setup_on_settings_changed(instance);
        } else if(event->event == JsAppLauncherCustomEventSettingsItemSelected) {
            is_consumed = js_app_launcher_scene_setup_on_item_selected(instance);
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        is_consumed = js_app_launcher_scene_setup_on_back(instance);
    }

    return is_consumed;
}

const Scene js_app_launcher_scene_setup = {
    .data_size = sizeof(JsAppLauncherSceneSetup),
    .enter_callback = js_app_launcher_scene_setup_on_enter,
    .exit_callback = js_app_launcher_scene_setup_on_exit,
    .event_callback = js_app_launcher_scene_setup_on_event,
};

static const JsAppLauncherSceneSetupItemTypeApi js_app_launcher_scene_setup_item_type_apis[] = {
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
            .store = js_app_launcher_scene_setup_store_integer,
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
            .build = js_app_launcher_scene_setup_build_group,
            .store = NULL,
        },
};

static_assert(COUNT_OF(js_app_launcher_scene_setup_item_type_apis) == JsAppSettingsNodeTypesCount);
