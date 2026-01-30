#include "../account_settings.h"
#include <settings_helpers/gui_params.h>
#include <settings_helpers/status_view.h>

typedef struct {
    StatusView* front_status;
    StatusView* back_status;
} SceneLinkDone;

typedef enum {
    SceneEventConfirm = AppEventSceneEventsStart,
} SceneEvent;

static bool account_scene_link_done_input_callback(const InputEvent* event, void* context) {
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

static void account_scene_link_done_on_enter(void* context) {
    furi_assert(context);

    AccountSettings* instance = context;
    SceneLinkDone* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdLinkDone);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, account_scene_link_done_input_callback, instance);

        data->front_status = status_view_alloc(instance->front_scene_window);
        status_view_set_icon(data->front_status, SETTINGS_IMG_PATH("checkmark_front_8x6.bin"));
        status_view_set_header(data->front_status, "Connected");

        data->back_status = status_view_alloc(instance->back_scene_window);
        status_view_set_icon(data->back_status, SETTINGS_IMG_PATH("checkmark_back_12x10.bin"));
        status_view_set_header(data->back_status, "Connected");
    });
}

static void account_scene_link_done_on_exit(void* context) {
    furi_assert(context);

    AccountSettings* instance = context;

    SceneLinkDone* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdLinkDone);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, account_scene_link_done_input_callback);

        status_view_free(data->back_status);
        status_view_free(data->front_status);
    });
}

static bool account_scene_link_done_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    AccountSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SceneEventConfirm:
            scene_manager_replace_current_scene(instance->scene_manager, SceneIdLinked);
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

const Scene account_scene_link_done = {
    .enter_callback = account_scene_link_done_on_enter,
    .exit_callback = account_scene_link_done_on_exit,
    .event_callback = account_scene_link_done_on_event,
    .data_size = sizeof(SceneLinkDone),
};
