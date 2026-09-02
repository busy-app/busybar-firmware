#include "../apps_menu_i.h"
#include "apps_menu_scenes.h"
#include "../app_list.h"

#include <furi_hal_nvm.h>

#include <desktop/desktop.h>
#include <gui/modules/menu.h>

#include <js_app/js_app_registry.h>
#include <js_app_launcher/js_app_launcher.h>

#include <m-array.h>
#include <toolbox/m_cstr_dup.h>

ARRAY_DEF(JsAppStrArray, const char*, M_CSTR_DUP_OPLIST)

typedef enum {
    SceneCustomEventMenuItemClicked = AppsMenuCustomEventSceneEventsStart,
} SceneCustomEvent;

typedef struct {
    Menu* front_menu;
    Menu* back_menu;

    /* Parallel arrays, one element per listed JS app, filled by the registry
     * walk and consumed when the menu items are built. Kept apart from the
     * menu because the walk hits the SD card and must not run under the GUI
     * lock. */
    JsAppStrArray_t js_app_ids;
    JsAppStrArray_t js_app_names;
    JsAppStrArray_t js_app_icons_front;
    JsAppStrArray_t js_app_icons_back;

    uint32_t next_item_idx;
    _Atomic uint32_t menu_idx;
} AppsMenuSceneMain;

static void apps_scene_setup_menu_callback(uint32_t index, void* context) {
    furi_assert(context);

    AppsMenu* instance = context;
    AppsMenuSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, AppsMenuSceneIdMain);

    data->menu_idx = index;
    furi_message_queue_put(
        instance->event_queue, &(uint32_t){SceneCustomEventMenuItemClicked}, FuriWaitForever);
}

static void apps_menu_scene_main_list_native_apps(AppsMenu* instance, AppsMenuSceneMain* data) {
    for(uint32_t i = 0; i < AppsMenuEntryIdxComingSoon; ++i) {
        const AppsMenuEntry* const entry = apps_list_get_item(i);

        menu_add_item(
            data->front_menu,
            entry->name,
            NULL,
            entry->icon_path.front,
            i,
            apps_scene_setup_menu_callback,
            instance);

        menu_add_item(data->back_menu, entry->name, NULL, entry->icon_path.back, i, NULL, NULL);
    }

    /* NOTE: next_item_idx is not an array index, but a unique numeric id
     * to distinguish a menu item. It is safe to always start enumerating
     * JS apps from AppsMenuEntryIdxMax. */
    data->next_item_idx = AppsMenuEntryIdxMax;
}

static void app_menu_scene_main_js_app_list_callback(const JsAppInfo* info, void* context) {
    furi_assert(info);
    furi_assert(context);

    const JsAppManifestInfo* manifest_info = &info->manifest;

    if(manifest_info->is_debug && !furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug)) {
        return;
    }

    AppsMenuSceneMain* data = context;
    const JsAppPathInfo* paths = &info->path;

    // info is only valid inside this callback, so everything needed later is
    // copied here.
    JsAppStrArray_push_back(data->js_app_ids, manifest_info->id);
    JsAppStrArray_push_back(data->js_app_names, manifest_info->name);
    JsAppStrArray_push_back(data->js_app_icons_front, paths->icon.front);
    JsAppStrArray_push_back(data->js_app_icons_back, paths->icon.back);
}

static void apps_menu_scene_main_collect_js_apps(AppsMenuSceneMain* data) {
    js_app_registry_list_apps(app_menu_scene_main_js_app_list_callback, data);
}

static void apps_menu_scene_main_list_js_apps(AppsMenu* instance, AppsMenuSceneMain* data) {
    for(size_t i = 0; i < JsAppStrArray_size(data->js_app_ids); ++i) {
        const char* app_name = *JsAppStrArray_cget(data->js_app_names, i);

        menu_add_item(
            data->front_menu,
            app_name,
            NULL,
            *JsAppStrArray_cget(data->js_app_icons_front, i),
            data->next_item_idx,
            apps_scene_setup_menu_callback,
            instance);

        menu_add_item(
            data->back_menu,
            app_name,
            NULL,
            *JsAppStrArray_cget(data->js_app_icons_back, i),
            data->next_item_idx,
            NULL,
            NULL);

        ++data->next_item_idx;
    }
}

static void apps_menu_scene_main_start_selected_app(AppsMenu* instance) {
    AppsMenuSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, AppsMenuSceneIdMain);

    const char* app_id;

    if(data->menu_idx < AppsMenuEntryIdxMax) {
        const AppsMenuEntry* entry = apps_list_get_item(data->menu_idx);
        app_id = entry->id;
    } else {
        const uint32_t array_idx = data->menu_idx - AppsMenuEntryIdxMax;
        app_id = *JsAppStrArray_cget(data->js_app_ids, array_idx);
    }

    if(app_id != NULL) {
        if(apps_menu_start_application(app_id, false)) {
            apps_menu_set_active_application(&instance->settings, app_id);
        }
    } else {
        scene_manager_next_scene(instance->scene_manager, AppsMenuSceneIdComingSoon);
    }
}

static void apps_menu_scene_main_on_enter(void* context) {
    furi_assert(context);
    AppsMenu* instance = context;
    AppsMenuSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, AppsMenuSceneIdMain);

    JsAppStrArray_init(data->js_app_ids);
    JsAppStrArray_init(data->js_app_names);
    JsAppStrArray_init(data->js_app_icons_front);
    JsAppStrArray_init(data->js_app_icons_back);

    // Walks /ext/user_assets and parses a manifest per app. Done before taking
    // the GUI lock: holding it across SD reads freezes both displays and all
    // input for every app on the device.
    apps_menu_scene_main_collect_js_apps(data);

    with_gui(instance->gui, {
        data->front_menu = menu_alloc(instance->front_scene_window);
        data->back_menu = menu_alloc(instance->back_scene_window);

        apps_menu_scene_main_list_native_apps(instance, data);
        apps_menu_scene_main_list_js_apps(instance, data);

        widget_set_scrollbar_enabled(menu_get_base(data->front_menu), true);
        widget_set_scrollbar_enabled(menu_get_base(data->back_menu), true);

        widget_set_visible(nav_bar_get_base(instance->back_nav_bar), true);
    });
}

static void apps_menu_scene_main_on_exit(void* context) {
    furi_assert(context);
    AppsMenu* instance = context;
    AppsMenuSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, AppsMenuSceneIdMain);

    with_gui(instance->gui, {
        menu_free(data->front_menu);
        menu_free(data->back_menu);
    });

    JsAppStrArray_clear(data->js_app_ids);
    JsAppStrArray_clear(data->js_app_names);
    JsAppStrArray_clear(data->js_app_icons_front);
    JsAppStrArray_clear(data->js_app_icons_back);
}

static bool apps_menu_scene_main_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    bool consumed = false;

    AppsMenu* instance = context;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneCustomEventMenuItemClicked) {
            apps_menu_scene_main_start_selected_app(instance);
            consumed = true;
        }
    }

    return consumed;
}

const Scene apps_menu_scene_main = {
    .enter_callback = apps_menu_scene_main_on_enter,
    .exit_callback = apps_menu_scene_main_on_exit,
    .event_callback = apps_menu_scene_main_on_event,
    .data_size = sizeof(AppsMenuSceneMain),
};
