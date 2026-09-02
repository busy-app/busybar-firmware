#include "../js_app_launcher_i.h"
#include "js_app_launcher_scenes.h"

#include <gui/modules/label.h>
#include <gui/modules/submenu.h>
#include <gui/modules/var_item_list.h>

#include <js_app/js_app_settings.h>

#define JS_APP_SETTINGS_MAX_DEPTH (4)

typedef enum {
    JsAppLauncherSettingsViewNone,
    JsAppLauncherSettingsViewLabel,
    JsAppLauncherSettingsViewGroups,
    JsAppLauncherSettingsViewValues,
} JsAppLauncherSettingsView;

typedef struct {
    JsAppSettings* settings;
    const JsAppSetting* group_stack[JS_APP_SETTINGS_MAX_DEPTH];
    size_t group_depth;

    JsAppLauncherSettingsView view;
    Label* front_label;
    Label* back_label;
    Submenu* front_submenu;
    Submenu* back_submenu;
    VarItemList* front_list;
    VarItemList* back_list;

    VarItem** front_items;
    VarItem** back_items;
    const JsAppSetting** item_settings;
    size_t item_count;
} JsAppLauncherSceneSetup;

static const JsAppSetting*
    js_app_launcher_scene_setup_get_group(const JsAppLauncherSceneSetup* data) {
    return data->group_depth ? data->group_stack[data->group_depth - 1] : NULL;
}

static void js_app_launcher_scene_setup_free_view(JsAppLauncherSceneSetup* data) {
    if(data->view == JsAppLauncherSettingsViewLabel) {
        label_free(data->front_label);
        label_free(data->back_label);
    } else if(data->view == JsAppLauncherSettingsViewGroups) {
        submenu_free(data->front_submenu);
        submenu_free(data->back_submenu);
    } else if(data->view == JsAppLauncherSettingsViewValues) {
        var_item_list_free(data->front_list);
        var_item_list_free(data->back_list);
        free(data->front_items);
        free(data->back_items);
        free(data->item_settings);
    }

    data->view = JsAppLauncherSettingsViewNone;
}

static size_t js_app_launcher_scene_setup_selector_index(const JsAppSetting* setting) {
    const char* selected = js_app_setting_get_string(setting);
    const size_t count = js_app_setting_get_option_count(setting);
    for(size_t i = 0; i < count; ++i) {
        if(strcmp(selected, js_app_setting_get_option_value(setting, i)) == 0) return i;
    }
    return 0;
}

static void
    js_app_launcher_scene_setup_format_value(const JsAppSetting* setting, FuriString* output) {
    const JsAppSettingType type = js_app_setting_get_type(setting);
    if(type == JsAppSettingTypeSwitch) {
        furi_string_set(output, js_app_setting_get_bool(setting) ? "On" : "Off");
    } else if(type == JsAppSettingTypeSelector) {
        furi_string_set(
            output,
            js_app_setting_get_option_label(
                setting, js_app_launcher_scene_setup_selector_index(setting)));
    } else if(type == JsAppSettingTypeTimebox) {
        const int32_t minutes = js_app_setting_get_int(setting);
        if(minutes < 60) {
            furi_string_printf(output, "%ld m", minutes);
        } else if((minutes % 60) == 0) {
            furi_string_printf(output, "%ld h", minutes / 60);
        } else {
            furi_string_printf(output, "%ld:%02ld", minutes / 60, minutes % 60);
        }
    } else {
        const char* suffix = js_app_setting_get_suffix(setting);
        furi_string_printf(
            output,
            "%ld%s%s",
            js_app_setting_get_int(setting),
            suffix ? " " : "",
            suffix ? suffix : "");
    }
}

static void js_app_launcher_scene_setup_group_callback(uint32_t index, void* context) {
    JsAppLauncher* instance = context;
    js_app_launcher_send_custom_event(instance, index);
}

static void js_app_launcher_scene_setup_add_groups(
    JsAppLauncher* instance,
    JsAppLauncherSceneSetup* data,
    const JsAppSetting* group) {
    data->front_submenu = submenu_alloc(instance->front_window);
    data->back_submenu = submenu_alloc(instance->back_window);
    data->view = JsAppLauncherSettingsViewGroups;

    FuriString* sub_label = furi_string_alloc();
    const size_t count = js_app_settings_get_count(data->settings, group);
    for(size_t i = 0; i < count; ++i) {
        const JsAppSetting* setting = js_app_settings_get(data->settings, group, i);
        if(!js_app_setting_is_visible(setting)) continue;

        const JsAppSetting* sub_label_setting = js_app_setting_get_sub_label_setting(setting);
        const char* sub_label_text = NULL;
        if(sub_label_setting) {
            js_app_launcher_scene_setup_format_value(sub_label_setting, sub_label);
            sub_label_text = furi_string_get_cstr(sub_label);
        }

        submenu_add_item(
            data->front_submenu,
            js_app_setting_get_label(setting),
            sub_label_text,
            i,
            js_app_launcher_scene_setup_group_callback,
            instance);
        submenu_add_item(
            data->back_submenu, js_app_setting_get_label(setting), sub_label_text, i, NULL, NULL);
    }
    furi_string_free(sub_label);

    widget_set_scrollbar_enabled(submenu_get_base(data->front_submenu), true);
    widget_set_scrollbar_enabled(submenu_get_base(data->back_submenu), true);
}

static VarItem* js_app_launcher_scene_setup_add_value(
    VarItemList* list,
    const JsAppSetting* setting,
    VarItemChangeCallback callback,
    void* context) {
    VarItem* item = NULL;
    const JsAppSettingType type = js_app_setting_get_type(setting);

    if(type == JsAppSettingTypeSwitch) {
        item =
            var_item_list_add_switch(list, js_app_setting_get_label(setting), callback, context);
        var_item_set_value(item, js_app_setting_get_bool(setting));
    } else if(type == JsAppSettingTypeSelector) {
        const size_t option_count = js_app_setting_get_option_count(setting);
        const char** labels = malloc(sizeof(char*) * option_count);
        for(size_t i = 0; i < option_count; ++i) {
            labels[i] = js_app_setting_get_option_label(setting, i);
        }
        item = var_item_list_add_selector(
            list,
            js_app_setting_get_label(setting),
            js_app_setting_get_suffix(setting),
            labels,
            option_count,
            callback,
            context);
        free(labels);
        var_item_set_value(item, js_app_launcher_scene_setup_selector_index(setting));
    } else if(type == JsAppSettingTypeSpinbox) {
        item = var_item_list_add_spinbox(
            list,
            js_app_setting_get_label(setting),
            js_app_setting_get_suffix(setting),
            js_app_setting_get_min(setting),
            js_app_setting_get_max(setting),
            js_app_setting_get_step(setting),
            callback,
            context);
        var_item_set_value(item, js_app_setting_get_int(setting));
    } else if(type == JsAppSettingTypeTimebox) {
        item = var_item_list_add_timebox(
            list,
            js_app_setting_get_label(setting),
            js_app_setting_get_min(setting),
            js_app_setting_get_max(setting),
            js_app_setting_get_step(setting),
            callback,
            context);
        var_item_set_value(item, js_app_setting_get_int(setting));
    }

    return item;
}

static void js_app_launcher_scene_setup_filter_values(JsAppLauncherSceneSetup* data) {
    for(size_t i = 0; i < data->item_count; ++i) {
        const bool visible = js_app_setting_is_visible(data->item_settings[i]);
        widget_set_visible((Widget*)data->front_items[i], visible);
        widget_set_visible((Widget*)data->back_items[i], visible);
    }
}

static void js_app_launcher_scene_setup_refresh_values(JsAppLauncherSceneSetup* data) {
    for(size_t i = 0; i < data->item_count; ++i) {
        const JsAppSetting* setting = data->item_settings[i];
        int32_t value;

        if(js_app_setting_get_type(setting) == JsAppSettingTypeSwitch) {
            value = js_app_setting_get_bool(setting);
        } else if(js_app_setting_get_type(setting) == JsAppSettingTypeSelector) {
            const char* current = js_app_setting_get_string(setting);
            value = 0;
            for(size_t option = 0; option < js_app_setting_get_option_count(setting); ++option) {
                if(strcmp(js_app_setting_get_option_value(setting, option), current) == 0) {
                    value = option;
                    break;
                }
            }
        } else {
            value = js_app_setting_get_int(setting);
        }

        var_item_set_value(data->front_items[i], value);
        var_item_set_value(data->back_items[i], value);
    }
    js_app_launcher_scene_setup_filter_values(data);
}

static void js_app_launcher_scene_setup_value_callback(VarItem* item, void* context) {
    JsAppLauncher* instance = context;
    JsAppLauncherSceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdSetup);

    for(size_t i = 0; i < data->item_count; ++i) {
        if(data->front_items[i] != item) continue;

        const JsAppSetting* setting = data->item_settings[i];
        const int32_t value = var_item_get_value(item);
        bool accepted;

        if(js_app_setting_get_type(setting) == JsAppSettingTypeSwitch) {
            accepted = js_app_settings_set_bool(data->settings, setting, value);
        } else if(js_app_setting_get_type(setting) == JsAppSettingTypeSelector) {
            accepted = js_app_settings_set_string(
                data->settings, setting, js_app_setting_get_option_value(setting, value));
        } else {
            accepted = js_app_settings_set_int(data->settings, setting, value);
        }

        if(accepted) {
            var_item_set_value(data->back_items[i], value);
            js_app_launcher_scene_setup_filter_values(data);
            // This runs on the GUI service thread with the LVGL lock held, so
            // the card write cannot happen here — one turn of the dial per
            // write would stall every display and every app. Hand it to the
            // launcher thread instead.
            js_app_launcher_send_custom_event(instance, JsAppLauncherCustomEventSettingsChanged);
        } else {
            FURI_LOG_E(TAG, "Rejected setting value: %s", js_app_setting_get_name(setting));
        }
        break;
    }
}

static void js_app_launcher_scene_setup_add_values(
    JsAppLauncher* instance,
    JsAppLauncherSceneSetup* data,
    const JsAppSetting* group) {
    data->front_list = var_item_list_alloc(instance->front_window);
    data->back_list = var_item_list_alloc(instance->back_window);
    data->view = JsAppLauncherSettingsViewValues;

    data->item_count = js_app_settings_get_count(data->settings, group);
    data->front_items = malloc(sizeof(VarItem*) * data->item_count);
    data->back_items = malloc(sizeof(VarItem*) * data->item_count);
    data->item_settings = malloc(sizeof(JsAppSetting*) * data->item_count);

    for(size_t i = 0; i < data->item_count; ++i) {
        const JsAppSetting* setting = js_app_settings_get(data->settings, group, i);
        data->item_settings[i] = setting;
        data->front_items[i] = js_app_launcher_scene_setup_add_value(
            data->front_list, setting, js_app_launcher_scene_setup_value_callback, instance);
        data->back_items[i] =
            js_app_launcher_scene_setup_add_value(data->back_list, setting, NULL, NULL);
    }

    js_app_launcher_scene_setup_filter_values(data);
    widget_set_scrollbar_enabled(var_item_list_get_base(data->front_list), true);
    widget_set_scrollbar_enabled(var_item_list_get_base(data->back_list), true);
}

static void js_app_launcher_scene_setup_show_label(
    JsAppLauncher* instance,
    JsAppLauncherSceneSetup* data,
    const char* text) {
    data->front_label = label_alloc(instance->front_window);
    label_set_text(data->front_label, text);
    widget_set_align(label_get_base(data->front_label), AlignCenter);

    data->back_label = label_alloc(instance->back_window);
    label_set_text(data->back_label, text);
    widget_set_align(label_get_base(data->back_label), AlignCenter);
    data->view = JsAppLauncherSettingsViewLabel;
}

static void
    js_app_launcher_scene_setup_build_view(JsAppLauncher* instance, JsAppLauncherSceneSetup* data) {
    js_app_launcher_scene_setup_free_view(data);

    const JsAppSetting* group = js_app_launcher_scene_setup_get_group(data);
    const JsAppSetting* first = js_app_settings_get(data->settings, group, 0);
    if(js_app_setting_get_type(first) == JsAppSettingTypeGroup) {
        js_app_launcher_scene_setup_add_groups(instance, data, group);
    } else {
        js_app_launcher_scene_setup_add_values(instance, data, group);
    }
}

static void js_app_launcher_scene_setup_push_location(JsAppLauncher* instance, const char* label) {
    FuriString* location = furi_string_alloc_set(label);
    furi_string_to_upper_in_place(location);
    nav_bar_push_location(instance->nav_bar, furi_string_get_cstr(location));
    furi_string_free(location);
}

static void js_app_launcher_scene_setup_on_enter(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;
    JsAppLauncherSceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdSetup);

    memset(data, 0, sizeof(*data));
    with_gui(instance->gui, { nav_bar_push_location(instance->nav_bar, "SETUP"); });

    JsAppInfo info;
    furi_check(js_app_get_info(instance->js_app, &info));

    if(!info.path.settings) {
        with_gui(instance->gui, {
            js_app_launcher_scene_setup_show_label(instance, data, "No settings");
        });
        return;
    }

    data->settings = js_app_settings_alloc();
    if(!js_app_settings_load(data->settings, info.manifest.id, info.path.settings)) {
        with_gui(instance->gui, {
            js_app_launcher_scene_setup_show_label(instance, data, "Invalid settings");
        });
        return;
    }

    with_gui(instance->gui, { js_app_launcher_scene_setup_build_view(instance, data); });
}

static void js_app_launcher_scene_setup_on_exit(void* context) {
    furi_assert(context);
    JsAppLauncher* instance = context;
    JsAppLauncherSceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdSetup);

    with_gui(instance->gui, {
        js_app_launcher_scene_setup_free_view(data);
        for(size_t i = 0; i <= data->group_depth; ++i) {
            nav_bar_pop_location(instance->nav_bar);
        }
    });

    if(data->settings) {
        // Catches a change made right before Back, whose deferred commit event
        // never got a chance to run.
        if(!js_app_settings_commit(data->settings)) {
            FURI_LOG_E(TAG, "Failed to save settings on exit");
        }
        js_app_settings_free(data->settings);
    }
}

static bool js_app_launcher_scene_setup_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    JsAppLauncher* instance = context;
    JsAppLauncherSceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, JsAppLauncherSceneIdSetup);

    if((event->type == SceneManagerEventTypeCustom) &&
       (event->event == JsAppLauncherCustomEventSettingsChanged)) {
        if(data->settings && !js_app_settings_commit(data->settings)) {
            // The card rejected the write. Show what is actually stored rather
            // than leaving the list asserting a value nobody saved.
            FURI_LOG_E(TAG, "Failed to save settings");
            js_app_settings_reload(data->settings);
            with_gui(instance->gui, { js_app_launcher_scene_setup_refresh_values(data); });
        }
        return true;
    }

    if((event->type == SceneManagerEventTypeCustom) &&
       (data->view == JsAppLauncherSettingsViewGroups)) {
        const JsAppSetting* group = js_app_launcher_scene_setup_get_group(data);
        const JsAppSetting* selected = js_app_settings_get(data->settings, group, event->event);
        if(selected && (js_app_setting_get_type(selected) == JsAppSettingTypeGroup) &&
           (data->group_depth < JS_APP_SETTINGS_MAX_DEPTH)) {
            data->group_stack[data->group_depth++] = selected;
            with_gui(instance->gui, {
                js_app_launcher_scene_setup_push_location(
                    instance, js_app_setting_get_label(selected));
                js_app_launcher_scene_setup_build_view(instance, data);
            });
        }
        return true;
    }

    if((event->type == SceneManagerEventTypeBack) && data->group_depth) {
        --data->group_depth;
        with_gui(instance->gui, {
            nav_bar_pop_location(instance->nav_bar);
            js_app_launcher_scene_setup_build_view(instance, data);
        });
        return true;
    }

    return false;
}

const Scene js_app_launcher_scene_setup = {
    .data_size = sizeof(JsAppLauncherSceneSetup),
    .enter_callback = js_app_launcher_scene_setup_on_enter,
    .exit_callback = js_app_launcher_scene_setup_on_exit,
    .event_callback = js_app_launcher_scene_setup_on_event,
};
