#include "../matter_settings_i.h"
#include "../widgets/matter_code_view.h"
#include <settings_helpers/status_view.h>

#include <matter/matter.h>

#define STATUS_LIGHTS_COLOR ((Color)COLOR_MAKE_RGB(0xFF, 0xFF, 0xFF))

typedef struct {
    bool ui_initialized;

    StatusView* front_prompt;
    MatterCodeView* back_codes;
} MatterScenePairing;

static void matter_scene_pairing_on_enter(void* context) {
    furi_assert(context);
    MatterSettings* app = context;
    MatterScenePairing* scene = scene_manager_get_scene_data(app->scene_manager, SceneIdPairing);

    scene->ui_initialized = false;
    if(!matter_settings_check_wifi_connectivity(app)) return;

    MatterCommissioningInfo info;
    const MatterStatus status = matter_enable_commissioning(app->matter, &info);

    if(status != MatterStatusOk) {
        // TODO: Better way of handling errors at this point
        furi_event_loop_stop(app->event_loop);
        return;
    }

    with_gui(app->gui, {
        scene->front_prompt = status_view_alloc(app->front_scene_window);
        status_view_set_icon(scene->front_prompt, SETTINGS_IMG_PATH("info_front_7x7.image"));
        status_view_set_header(scene->front_prompt, "Look at back\nscreen");

        GuiLayer* top_layer = gui_get_layer(app->gui, GuiLayerIdSystem);
        Widget* top_back_layer_root = gui_layer_get_root_widget(top_layer, GuiDisplayIdBack);
        scene->back_codes = matter_code_view_alloc(top_back_layer_root);
        matter_code_view_set_logo_path(scene->back_codes, IMG_PATH("matter_back_21x21.image"));
        matter_code_view_set_codes(scene->back_codes, info.qr_code, info.manual_code);
    });

    brightness_control_set_brightness_override(
        app->brightness_control, BrightnessControlModuleStatusLights, BRIGHTNESS_MAX);
    status_lights_run_preset(app->status_lights, StatusLightsPresetBlink, STATUS_LIGHTS_COLOR);

    scene->ui_initialized = true;
}

static void matter_scene_pairing_on_exit(void* context) {
    furi_assert(context);
    MatterSettings* app = context;
    MatterScenePairing* scene = scene_manager_get_scene_data(app->scene_manager, SceneIdPairing);

    if(!scene->ui_initialized) return;

    with_gui(app->gui, {
        status_view_free(scene->front_prompt);
        matter_code_view_free(scene->back_codes);
    });

    status_lights_run_preset(app->status_lights, StatusLightsPresetOff, (Color){});
    brightness_control_reset_brightness_override(
        app->brightness_control, BrightnessControlModuleStatusLights);
}

static bool matter_scene_pairing_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    MatterSettings* app = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeBack) {
        matter_settings_exit_if_last(app);
    }

    return consumed;
}

const Scene matter_scene_pairing = {
    .enter_callback = matter_scene_pairing_on_enter,
    .exit_callback = matter_scene_pairing_on_exit,
    .event_callback = matter_scene_pairing_on_event,
    .data_size = sizeof(MatterScenePairing),
};
