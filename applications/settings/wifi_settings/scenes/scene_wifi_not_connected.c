#include "../wifi_settings.h"
#include "../widgets/wifi_not_connected_view.h"
#include <settings_helpers/gui_params.h>

#include <gui/modules/var_item_list.h>

#include <wifi/wifi.h>

typedef enum {
    SceneEventConnectDone = AppEventSceneEventsStart,
} SceneEvent;

typedef struct {
    FlexLayout* front_container;
    Label* front_label;
    Image* front_image;

    WifiNotConnectedView* back_view;
} SceneWifiNotConnected;

static void wifi_scene_not_connected_on_enter(void* context) {
    furi_assert(context);

    WifiSettings* instance = context;
    SceneWifiNotConnected* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdNotConnected);

    with_gui(instance->gui, {
        data->front_container = flex_layout_alloc(instance->front_scene_window, FlexLayoutTypeRow);
        Widget* flex_base = flex_layout_get_base(data->front_container);
        flex_layout_set_align(
            data->front_container,
            FlexLayoutAlignCenter,
            FlexLayoutAlignCenter,
            FlexLayoutAlignStart);
        flex_layout_set_spacing(data->front_container, 2);
        widget_set_size_content(flex_layout_get_base(data->front_container));
        widget_set_align(flex_layout_get_base(data->front_container), AlignLeftMid);

        data->front_image = image_alloc(flex_base);
        image_set_source(data->front_image, IMG_PATH("wifi_front_gray_8x8.bin"));
        widget_set_size_content(image_get_base(data->front_image));

        data->front_label = label_alloc(flex_base);
        label_set_text(data->front_label, "Connect Wi-Fi via\nPC or BUSY App");
        widget_set_size_content(label_get_base(data->front_label));

        GuiLayer* top_layer = gui_get_layer(instance->gui, GuiLayerIdSystem);
        Widget* top_back_layer_root = gui_layer_get_root_widget(top_layer, GuiDisplayIdBack);
        data->back_view = wifi_not_connected_view_back_alloc(top_back_layer_root);
    });
}

static void wifi_scene_not_connected_on_exit(void* context) {
    furi_assert(context);

    WifiSettings* instance = context;

    SceneWifiNotConnected* data =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdNotConnected);

    with_gui(instance->gui, {
        image_free(data->front_image);
        label_free(data->front_label);
        flex_layout_free(data->front_container);

        wifi_not_connected_view_free(data->back_view);
    });
}

static bool wifi_scene_not_connected_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    WifiSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case AppEventWifiStateChange:
            WifiModelState wifi_state = wifi_model_get_state(instance->model);
            if(wifi_state == WifiModelStateConnected) {
                scene_manager_replace_current_scene(instance->scene_manager, SceneIdState);
            }
            break;
        default:
            break;
        }

    } else if(event->type == SceneManagerEventTypeBack) {
        desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
    }

    return consumed;
}

const Scene wifi_scene_not_connected = {
    .enter_callback = wifi_scene_not_connected_on_enter,
    .exit_callback = wifi_scene_not_connected_on_exit,
    .event_callback = wifi_scene_not_connected_on_event,
    .data_size = sizeof(SceneWifiNotConnected),
};
