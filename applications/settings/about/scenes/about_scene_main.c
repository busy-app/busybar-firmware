#include "../about.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/var_item_list.h>
#include <gui/modules/submenu.h>

typedef enum {
    SceneEventGeneral = AppEventSceneEventsStart,
    SceneEventFirmware,
    SceneEventComplianceInfo,
    SceneEventLibsList,
} SceneEvent;

typedef struct {
    Submenu* front_menu;
    Submenu* back_menu;
    uint32_t menu_index;
} AboutSceneMain;

static void about_scene_main_menu_item_callback(uint32_t index, void* context) {
    furi_assert(context);

    about_send_custom_event(context, index);
}

static void about_scene_main_on_enter(void* context) {
    furi_assert(context);

    About* instance = context;
    AboutSceneMain* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    with_gui(instance->gui, {
        data->front_menu = submenu_alloc(instance->front_scene_window);
        submenu_add_item(
            data->front_menu,
            "General",
            NULL,
            SceneEventGeneral,
            about_scene_main_menu_item_callback,
            instance);
        submenu_add_item(
            data->front_menu,
            "Firmware",
            NULL,
            SceneEventFirmware,
            about_scene_main_menu_item_callback,
            instance);
        submenu_add_item(
            data->front_menu,
            "Compliance Info",
            NULL,
            SceneEventComplianceInfo,
            about_scene_main_menu_item_callback,
            instance);
        submenu_add_item(
            data->front_menu,
            "Open-Source Libs",
            NULL,
            SceneEventLibsList,
            about_scene_main_menu_item_callback,
            instance);
        submenu_set_selected_item_index(data->front_menu, data->menu_index);

        data->back_menu = submenu_alloc(instance->back_scene_window);
        submenu_add_item(data->back_menu, "General", NULL, SceneEventGeneral, NULL, instance);
        submenu_add_item(data->back_menu, "Firmware", NULL, SceneEventFirmware, NULL, instance);
        submenu_add_item(
            data->back_menu, "Compliance Info", NULL, SceneEventComplianceInfo, NULL, instance);
        submenu_add_item(
            data->back_menu, "Open-Source Libs", NULL, SceneEventLibsList, NULL, instance);
        submenu_set_selected_item_index(data->back_menu, data->menu_index);

        widget_set_scrollbar_enabled(submenu_get_base(data->front_menu), true);
        widget_set_scrollbar_enabled(submenu_get_base(data->back_menu), true);
    });
}

static void about_scene_main_on_exit(void* context) {
    furi_assert(context);

    About* instance = context;
    AboutSceneMain* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    with_gui(instance->gui, {
        submenu_free(data->front_menu);
        submenu_free(data->back_menu);
    });
}

static bool about_scene_main_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    About* instance = context;
    AboutSceneMain* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdMain);

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == SceneEventGeneral) {
            scene_manager_next_scene(instance->scene_manager, SceneIdGeneral);
            about_push_location(instance, "GENERAL");
            consumed = true;
        } else if(event->event == SceneEventFirmware) {
            scene_manager_next_scene(instance->scene_manager, SceneIdFirmware);
            about_push_location(instance, "FIRMWARE");
            consumed = true;
        } else if(event->event == SceneEventComplianceInfo) {
            scene_manager_next_scene(instance->scene_manager, SceneIdCompliance);
            about_push_location(instance, "COMPLIANCE INFO");
            consumed = true;
        } else if(event->event == SceneEventLibsList) {
            instance->license_lib_index = 0;
            scene_manager_next_scene(instance->scene_manager, SceneIdLibsList);
            about_push_location(instance, "OPEN-SOURCE LIBS");
            consumed = true;
        }
        data->menu_index = event->event;
    } else if(event->type == SceneManagerEventTypeBack) {
        desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
    }

    return consumed;
}

const Scene about_scene_main = {
    .enter_callback = about_scene_main_on_enter,
    .exit_callback = about_scene_main_on_exit,
    .event_callback = about_scene_main_on_event,
    .data_size = sizeof(AboutSceneMain),
};
