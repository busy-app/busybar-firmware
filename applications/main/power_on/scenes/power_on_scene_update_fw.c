#include "../power_on_i.h"

#include <gui/modules/status_view.h>
#include <gui/modules/qr_docs.h>

typedef struct {
    StatusView* front_view;
    QrDocs* back_view;
} SceneUpdateFw;

static bool power_on_scene_update_fw_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    PowerOnApp* app = context;

    return power_on_handle_generic_input(app, event);
}

static void power_on_scene_update_fw_on_enter(void* context) {
    furi_assert(context);

    PowerOnApp* app = context;
    SceneUpdateFw* scene = scene_manager_get_scene_data(app->scene_manager, SceneIdUpdateFw);

    with_gui(app->gui, {
        GuiLayer* layer = gui_get_layer(app->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, power_on_scene_update_fw_input_callback, app);

        scene->front_view = status_view_alloc(app->front_root);
        status_view_set_icon(scene->front_view, SHARED_IMG_PATH("info_front_8x8.image"));
        status_view_set_primary_text(scene->front_view, "Look at back\nscreen");

        scene->back_view = qr_docs_alloc(app->back_root);
        qr_docs_set_image(scene->back_view, SHARED_IMG_PATH("microchip_back_11x11.image"));
        qr_docs_set_text(scene->back_view, "Update\nfirmware.\nPress any\nkey.");
        qr_docs_set_url(scene->back_view, "https://docs.busy.app/bar/basics/firmware-update");
    });
}

static void power_on_scene_update_fw_on_exit(void* context) {
    furi_assert(context);

    PowerOnApp* app = context;

    SceneUpdateFw* scene = scene_manager_get_scene_data(app->scene_manager, SceneIdUpdateFw);

    with_gui(app->gui, {
        status_view_free(scene->front_view);
        qr_docs_free(scene->back_view);

        GuiLayer* layer = gui_get_layer(app->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, power_on_scene_update_fw_input_callback);
    });
}

static bool power_on_scene_update_fw_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    PowerOnApp* app = context;
    SceneUpdateFw* scene = scene_manager_get_scene_data(app->scene_manager, SceneIdUpdateFw);
    UNUSED(scene);

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case PowerOnAppEventUserInteracted:
            furi_event_loop_stop(app->event_loop);
            consumed = true;
            break;
        default:
            break;
        }

    } else if(event->type == SceneManagerEventTypeBack) {
        furi_event_loop_stop(app->event_loop);
        consumed = true;
    }

    return consumed;
}

const Scene power_on_scene_update_fw = {
    .enter_callback = power_on_scene_update_fw_on_enter,
    .exit_callback = power_on_scene_update_fw_on_exit,
    .event_callback = power_on_scene_update_fw_on_event,
    .data_size = sizeof(SceneUpdateFw),
};
