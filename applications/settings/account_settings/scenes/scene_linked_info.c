#include "../account_settings_i.h"
#include <settings_helpers/gui_params.h>
#include "../widgets/account_info_view.h"

typedef struct {
    AccountInfoView* front_view;
    AccountInfoView* back_view;
} SceneLinkedInfo;

typedef enum {
    SceneEventConfirm = AppEventSceneEventsStart,
} SceneEvent;

static bool account_scene_linked_info_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    AccountSettings* instance = context;

    bool consumed = false;
    SceneEvent custom_event;
    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyStart:
        /* fall-through */
        case InputKeyOk:
            custom_event = SceneEventConfirm;
            consumed = true;
            break;

        default:
            break;
        }
    }

    if(consumed) {
        account_settings_send_custom_event(instance, custom_event);
    }

    return consumed;
}

static void account_scene_linked_info_on_enter(void* context) {
    furi_assert(context);

    AccountSettings* instance = context;
    SceneLinkedInfo* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdLinkedInfo);

    FuriString* mail_short_str = furi_string_alloc();
    account_settings_get_short_email(instance, mail_short_str);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, account_scene_linked_info_input_callback, instance);

        data->front_view = account_info_view_front_alloc(instance->front_scene_window);
        data->back_view = account_info_view_back_alloc(instance->back_scene_window);

        account_info_view_set_state(data->front_view, furi_string_get_cstr(mail_short_str));
        account_info_view_set_state(data->back_view, furi_string_get_cstr(mail_short_str));
    });

    furi_string_free(mail_short_str);
}

static void account_scene_linked_info_on_exit(void* context) {
    furi_assert(context);

    AccountSettings* instance = context;

    SceneLinkedInfo* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdLinkedInfo);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, account_scene_linked_info_input_callback);

        account_info_view_free(data->back_view);
        account_info_view_free(data->front_view);
    });
}

static bool account_scene_linked_info_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    AccountSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SceneEventConfirm:
            scene_manager_next_scene(instance->scene_manager, SceneIdLinkedMenu);
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

const Scene account_scene_linked_info = {
    .enter_callback = account_scene_linked_info_on_enter,
    .exit_callback = account_scene_linked_info_on_exit,
    .event_callback = account_scene_linked_info_on_event,
    .data_size = sizeof(SceneLinkedInfo),
};
