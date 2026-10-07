#include "../apps_menu_i.h"
#include "apps_menu_scenes.h"

#include <furi_hal_nvm.h>

#include <desktop/desktop.h>
#include <gui/modules/menu.h>

#include <js_app/js_app_registry.h>
#include <js_app_launcher/js_app_launcher.h>

#include <m-array.h>
#include <toolbox/m_cstr_dup.h>

ARRAY_DEF(AppIdArray, const char*, M_CSTR_DUP_OPLIST)

typedef struct {
    Menu* front_menu;
    Menu* back_menu;
    AppIdArray_t app_ids;
} AppsMenuSceneMain;

typedef struct {
    AppsMenu* instance;
    AppsMenuSceneMain* data;
    uint32_t next_item_idx;
} AppsMenuSceneMainContext;

static void apps_scene_setup_menu_callback(uint32_t index, void* context) {
    furi_assert(context);
    AppsMenu* instance = context;
    apps_menu_send_custom_event(instance, index);
}

static void app_menu_scene_main_js_app_list_callback(const JsAppInfo* info, void* context) {
    furi_assert(info);
    furi_assert(context);

    const JsAppManifestInfo* manifest_info = &info->manifest;

    if(manifest_info->is_debug && !furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug)) {
        return;
    }

    AppsMenuSceneMainContext* ctx = context;

    AppsMenu* instance = ctx->instance;
    AppsMenuSceneMain* data = ctx->data;

    const char* app_name = manifest_info->name;
    const JsAppPathInfo* paths = &info->path;

    menu_add_item(
        data->front_menu,
        app_name,
        NULL,
        paths->icon.front,
        ctx->next_item_idx,
        apps_scene_setup_menu_callback,
        instance);

    menu_add_item(
        data->back_menu, app_name, NULL, paths->icon.back, ctx->next_item_idx, NULL, NULL);

    AppIdArray_push_back(data->app_ids, manifest_info->id);

    ++ctx->next_item_idx;
}

static void apps_menu_scene_main_list_apps(AppsMenu* instance, AppsMenuSceneMain* data) {
    AppsMenuSceneMainContext ctx = {
        .instance = instance,
        .data = data,
        .next_item_idx = 0,
    };

    js_app_registry_list_apps(app_menu_scene_main_js_app_list_callback, &ctx);
}

static void apps_menu_scene_main_start_app(AppsMenu* instance, uint32_t selection_idx) {
    AppsMenuSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, AppsMenuSceneIdMain);

    const char* app_id = *AppIdArray_cget(data->app_ids, selection_idx);

    if(apps_menu_start_application(app_id, AppsMenuModeShowMenu)) {
        apps_menu_set_active_application(&instance->settings, app_id);
    }
}

static void apps_menu_scene_main_on_enter(void* context) {
    furi_assert(context);
    AppsMenu* instance = context;
    AppsMenuSceneMain* data =
        scene_manager_get_scene_data(instance->scene_manager, AppsMenuSceneIdMain);

    with_gui(instance->gui, {
        data->front_menu = menu_alloc(instance->front_scene_window);
        data->back_menu = menu_alloc(instance->back_scene_window);

        AppIdArray_init(data->app_ids);
        apps_menu_scene_main_list_apps(instance, data);

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

    AppIdArray_clear(data->app_ids);
}

static bool apps_menu_scene_main_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    bool consumed = false;

    AppsMenu* instance = context;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event <= AppsMenuCustomEventIndexMax) {
            apps_menu_scene_main_start_app(instance, event->event);
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
