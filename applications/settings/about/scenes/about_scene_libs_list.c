#include "../about.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/var_item_list.h>
#include <gui/modules/submenu.h>

typedef enum {
    SceneEventLibSelectStart = AppEventSceneEventsStart,
} SceneEvent;

typedef struct {
    Submenu* front_menu;
    Submenu* back_menu;
} AboutSceneLibsList;

static void about_scene_libs_list_menu_item_callback(uint32_t index, void* context) {
    furi_assert(context);

    about_send_custom_event(context, SceneEventLibSelectStart + index);
}

static void about_scene_libs_list_on_enter(void* context) {
    furi_assert(context);

    About* instance = context;
    AboutSceneLibsList* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdLibsList);

    with_gui(instance->gui, {
        data->front_menu = submenu_alloc(instance->front_scene_window);
        data->back_menu = submenu_alloc(instance->back_scene_window);

        for(size_t i = 0; i < COUNT_OF(about_libs_info); i++) {
            submenu_add_item(
                data->front_menu,
                about_libs_info[i].name,
                i,
                about_scene_libs_list_menu_item_callback,
                instance);
            submenu_add_item(data->back_menu, about_libs_info[i].name, i, NULL, instance);
        }
        submenu_set_selected_item_index(data->front_menu, instance->license_lib_index);
        submenu_set_selected_item_index(data->back_menu, instance->license_lib_index);
    });
}

static void about_scene_libs_list_on_exit(void* context) {
    furi_assert(context);

    About* instance = context;
    AboutSceneLibsList* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdLibsList);

    with_gui(instance->gui, {
        submenu_free(data->front_menu);
        submenu_free(data->back_menu);
    });
}

static bool about_scene_libs_list_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    About* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event >= SceneEventLibSelectStart &&
           event->event < SceneEventLibSelectStart + COUNT_OF(about_libs_info)) {
            size_t lib_index = event->event - SceneEventLibSelectStart;
            instance->license_lib_index = lib_index;
            scene_manager_next_scene(instance->scene_manager, SceneIdLibInfo);
            UNUSED(lib_index);
            consumed = true;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        about_pop_location(instance);
        consumed = scene_manager_previous_scene(instance->scene_manager);
    }

    return consumed;
}

const Scene about_scene_libs_list = {
    .enter_callback = about_scene_libs_list_on_enter,
    .exit_callback = about_scene_libs_list_on_exit,
    .event_callback = about_scene_libs_list_on_event,
    .data_size = sizeof(AboutSceneLibsList),
};
