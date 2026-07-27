#include "../apps_menu_i.h"
#include "../storage_macros.h"
#include "apps_menu_scenes.h"
#include "../app_list.h"

#include <furi_hal_nvm.h>

#include <desktop/desktop.h>
#include <gui/modules/menu.h>

#include <js_app/js_app_registry.h>

typedef enum {
    SceneCustomEventMenuItemClicked = AppsMenuCustomEventSceneEventsStart,
} SceneCustomEvent;

typedef struct {
    Menu* front_menu;
    Menu* back_menu;

    uint32_t item_count;
    _Atomic uint32_t menu_idx;
} AppsMenuSceneMain;

typedef struct {
    AppsMenu* instance;
    AppsMenuSceneMain* data;
} AppsMenuSceneMainContext;

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
    menu_add_item(
        data->front_menu,
        "Clock",
        "",
        APPS_MENU_IMG_PATH("clock_front_8x8.image"),
        AppsMenuEntryIdxClock,
        apps_scene_setup_menu_callback,
        instance);

    menu_add_item(
        data->back_menu,
        "Clock",
        "",
        APPS_MENU_IMG_PATH("clock_back_11x11.image"),
        AppsMenuEntryIdxClock,
        NULL,
        NULL);

    data->item_count = AppsMenuEntryIdxsCount;
}

static void app_menu_scene_main_js_app_list_callback(const JsAppInfo* info, void* context) {
    furi_assert(info);
    furi_assert(context);

    const JsAppManifestInfo* manifest_info = &info->manifest;

    if(manifest_info->is_debug && !furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug)) {
        return;
    }

    const AppsMenuSceneMainContext* ctx = context;

    AppsMenu* instance = ctx->instance;
    AppsMenuSceneMain* data = ctx->data;

    const char* app_name = manifest_info->name;
    const JsAppIconInfo* icon_info = &info->icons;

    menu_add_item(
        data->front_menu,
        app_name,
        "",
        icon_info->front_path,
        data->item_count,
        apps_scene_setup_menu_callback,
        instance);

    menu_add_item(
        data->back_menu, app_name, "", icon_info->back_path, data->item_count, NULL, NULL);

    ++data->item_count;
}

static void apps_menu_scene_main_list_js_apps(AppsMenu* instance, AppsMenuSceneMain* data) {
    AppsMenuSceneMainContext ctx = {
        .instance = instance,
        .data = data,
    };

    js_app_registry_list_apps(app_menu_scene_main_js_app_list_callback, &ctx);
}

static void apps_menu_scene_main_on_enter(void* context) {
    furi_assert(context);
    AppsMenu* instance = context;
    AppsMenuSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, AppsMenuSceneIdMain);

    with_gui(instance->gui, {
        data->front_menu = menu_alloc(instance->front_scene_window);
        data->back_menu = menu_alloc(instance->back_scene_window);

        apps_menu_scene_main_list_native_apps(instance, data);
        apps_menu_scene_main_list_js_apps(instance, data);

        menu_set_selected_item_index(data->front_menu, data->menu_idx);
        menu_set_selected_item_index(data->back_menu, data->menu_idx);

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
}

static void apps_menu_scene_main_start_native_app(AppsMenu* instance, uint32_t app_idx) {
    AppsMenuSettings* settings = &instance->settings;
    const char* app_name = apps_menu_entries[app_idx];

    strlcpy(settings->active_application, app_name, sizeof(settings->active_application));

    apps_menu_settings_save(settings);

    Desktop* desktop = furi_record_open(RECORD_DESKTOP);
    desktop_replace_current_app(desktop, app_name, NULL);
    furi_record_close(RECORD_DESKTOP);
}

static void apps_menu_scene_main_start_js_app(AppsMenu* instance, uint32_t app_idx) {
    UNUSED(instance);

    // TODO: Implementation
    FURI_LOG_I("AppsMenu", "Running JS application with index %lu...", app_idx);
}

static bool apps_menu_scene_main_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    bool consumed = false;

    AppsMenu* instance = context;
    AppsMenuSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, AppsMenuSceneIdMain);

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneCustomEventMenuItemClicked) {
            if(data->menu_idx < AppsMenuEntryIdxsCount) {
                apps_menu_scene_main_start_native_app(instance, data->menu_idx);
            } else {
                apps_menu_scene_main_start_js_app(instance, data->menu_idx);
            }

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
