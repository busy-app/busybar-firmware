#include "../account_settings.h"
#include <settings_helpers/gui_params.h>
#include "../widgets/link_pin_view.h"

typedef enum {
    SceneEventRequestPin = AppEventSceneEventsStart,
} SceneEvent;

typedef struct {
    LinkPinView* front_view;
    LinkPinView* back_view;
} SceneLinkPin;

static bool account_scene_link_pin_input_callback(const InputEvent* event, void* context) {
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
            custom_event = SceneEventRequestPin;
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

static void
    account_scene_link_pin_update(AccountSettings* instance, SceneLinkPin* data, char* pin_code) {
    with_gui(instance->gui, {
        link_pin_view_set_state(data->back_view, pin_code);
        link_pin_view_set_state(data->front_view, pin_code);
    });
}

static void account_scene_link_pin_on_enter(void* context) {
    furi_assert(context);

    AccountSettings* instance = context;
    SceneLinkPin* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdLinkPin);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, account_scene_link_pin_input_callback, instance);

        data->front_view = link_pin_view_front_alloc(instance->front_scene_window);
        data->back_view = link_pin_view_back_alloc(instance->back_scene_window);
    });

    account_scene_link_pin_update(instance, data, NULL);
    account_model_request_link_pin(instance->model);
}

static void account_scene_link_pin_on_exit(void* context) {
    furi_assert(context);

    AccountSettings* instance = context;

    SceneLinkPin* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdLinkPin);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, account_scene_link_pin_input_callback);

        link_pin_view_free(data->back_view);
        link_pin_view_free(data->front_view);
    });
}

static bool account_scene_link_pin_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    AccountSettings* instance = context;
    SceneLinkPin* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdLinkPin);

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SceneEventRequestPin:
            account_scene_link_pin_update(instance, data, NULL);
            account_model_request_link_pin(instance->model);
            consumed = true;
            break;
        case AppEventAccountLinkPin:
            account_scene_link_pin_update(instance, data, instance->link_pin);
            consumed = true;
            break;
        case AppEventAccountLinkPinTimeout:
            scene_manager_replace_current_scene(instance->scene_manager, SceneIdError);
            consumed = true;
            break;
        case AppEventAccountLinkDone:
            scene_manager_replace_current_scene(instance->scene_manager, SceneIdConnecting);
            consumed = true;
            break;

        default:
            break;
        }
    }

    return consumed;
}

const Scene account_scene_link_pin = {
    .enter_callback = account_scene_link_pin_on_enter,
    .exit_callback = account_scene_link_pin_on_exit,
    .event_callback = account_scene_link_pin_on_event,
    .data_size = sizeof(SceneLinkPin),
};
