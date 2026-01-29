#include "../account_settings.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/var_item_list.h>
#include <gui/modules/submenu.h>

typedef enum {
    SceneEventMenuGetCode = AppEventSceneEventsStart,
} SceneEvent;

typedef struct {
    Submenu* front_menu;
    Submenu* back_menu;
} SceneAccountMenu;

void account_scene_menu_item_callback(uint32_t index, void* context) {
    account_settings_send_custom_event(context, index);
}

static void account_scene_menu_on_enter(void* context) {
    furi_assert(context);

    AccountSettings* instance = context;
    SceneAccountMenu* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdNotLinked);

    with_gui(instance->gui, {
        data->front_menu = submenu_alloc(instance->front_scene_window);
        submenu_add_item(
            data->front_menu,
            "Get pairing code",
            SceneEventMenuGetCode,
            account_scene_menu_item_callback,
            instance);

        data->back_menu = submenu_alloc(instance->back_scene_window);
        submenu_add_item(
            data->back_menu, "Get pairing code", SceneEventMenuGetCode, NULL, instance);
    });
}

static void account_scene_menu_on_exit(void* context) {
    furi_assert(context);

    AccountSettings* instance = context;
    SceneAccountMenu* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdNotLinked);

    with_gui(instance->gui, {
        submenu_free(data->front_menu);
        submenu_free(data->back_menu);
    });
}

static bool account_scene_menu_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    AccountSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SceneEventMenuGetCode:
            if(account_model_is_linked(instance->model)) {
                scene_manager_replace_current_scene(instance->scene_manager, SceneIdLinked);
            } else if(account_model_get_state(instance->model) == AccountModelStateNotConnected) {
                scene_manager_replace_current_scene(instance->scene_manager, SceneIdConnecting);
            } else {
                scene_manager_next_scene(instance->scene_manager, SceneIdLinkPin);
            }
            consumed = true;
            break;
        case AppEventAccountStateChange:
            AccountModelState state = account_model_get_state(instance->model);
            if(state == AccountModelStateConnectedLinked) {
                scene_manager_replace_current_scene(instance->scene_manager, SceneIdLinked);
            }
            consumed = true;
            break;
        default:
            break;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
        consumed = true;
    }

    return consumed;
}

const Scene account_scene_not_linked = {
    .enter_callback = account_scene_menu_on_enter,
    .exit_callback = account_scene_menu_on_exit,
    .event_callback = account_scene_menu_on_event,
    .data_size = sizeof(SceneAccountMenu),
};
