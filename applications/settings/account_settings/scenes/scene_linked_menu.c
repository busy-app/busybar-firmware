#include "../account_settings_i.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/var_item_list.h>
#include <gui/modules/submenu.h>

typedef enum {
    SceneEventMenuUnlink = AppEventSceneEventsStart,
} SceneEvent;

typedef struct {
    Submenu* front_menu;
    Submenu* back_menu;
} SceneLinkedMenu;

static void account_scene_linked_menu_item_callback(uint32_t index, void* context) {
    account_settings_send_custom_event(context, index);
}

static void account_scene_linked_menu_on_enter(void* context) {
    furi_assert(context);

    AccountSettings* instance = context;
    SceneLinkedMenu* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdLinkedMenu);

    with_gui(instance->gui, {
        data->front_menu = submenu_alloc(instance->front_scene_window);
        submenu_add_item(
            data->front_menu,
            "Unlink account",
            SceneEventMenuUnlink,
            account_scene_linked_menu_item_callback,
            instance);

        data->back_menu = submenu_alloc(instance->back_scene_window);
        submenu_add_item(data->back_menu, "Unlink account", SceneEventMenuUnlink, NULL, instance);
    });
}

static void account_scene_linked_menu_on_exit(void* context) {
    furi_assert(context);

    AccountSettings* instance = context;
    SceneLinkedMenu* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdLinkedMenu);

    with_gui(instance->gui, {
        submenu_free(data->front_menu);
        submenu_free(data->back_menu);
    });
}

static bool account_scene_linked_menu_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    AccountSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SceneEventMenuUnlink:
            scene_manager_next_scene(instance->scene_manager, SceneIdUnlink);
            consumed = true;
            break;
        case AppEventAccountUnlinked:
            scene_manager_replace_current_scene(instance->scene_manager, SceneIdConnecting);
            consumed = true;
            break;
        default:
            break;
        }
    }

    return consumed;
}

const Scene account_scene_linked_menu = {
    .enter_callback = account_scene_linked_menu_on_enter,
    .exit_callback = account_scene_linked_menu_on_exit,
    .event_callback = account_scene_linked_menu_on_event,
    .data_size = sizeof(SceneLinkedMenu),
};
