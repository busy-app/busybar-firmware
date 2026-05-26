#include "../power_on_i.h"

#include <gui/modules/anim_player.h>
#include <gui/modules/label.h>

#define POWER_ON_ANIM_PATH(path) BACKUP_PATH("recovery/resources/power_on/animations") "/" path

static const char* const power_on_anim_paths[GuiDisplayIdMax] = {
    POWER_ON_ANIM_PATH("front_power_on_72x16.anim"),
    POWER_ON_ANIM_PATH("back_power_on_148x80.anim"),
};

#define POWER_ON_ANIM_SECTION "loop"
#define POWER_ON_ANIM_FLAGS   (AnimFilePlayFlagFinishCurrent | AnimFilePlayFlagLoop)

#define SHUTDOWN_TIMER_INTERVAL_MSECS (15 * 60 * 1000)

typedef struct {
    FuriEventLoopTimer* shutdown_timer;
    AnimPlayer* anims[GuiDisplayIdMax];
} SceneAnimation;

static void power_on_scene_animation_shutdown_timer_fired(void* context) {
    furi_assert(context);
    PowerOnApp* app = context;
    power_off(app->power);
}

static bool power_on_scene_animation_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    PowerOnApp* app = context;

    return power_on_handle_generic_input(app, event);
}

static void power_on_scene_animation_on_enter(void* context) {
    furi_assert(context);

    PowerOnApp* app = context;
    SceneAnimation* scene = scene_manager_get_scene_data(app->scene_manager, SceneIdAnimation);

    scene->shutdown_timer = furi_event_loop_timer_alloc(
        app->event_loop,
        power_on_scene_animation_shutdown_timer_fired,
        FuriEventLoopTimerTypeOnce,
        app);
    furi_event_loop_timer_start(scene->shutdown_timer, SHUTDOWN_TIMER_INTERVAL_MSECS);

    with_gui(app->gui, {
        GuiLayer* layer = gui_get_layer(app->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, power_on_scene_animation_input_callback, app);

        for(GuiDisplayId display = 0; display < GuiDisplayIdMax; display++) {
            Widget* root = (display == GuiDisplayIdFront) ? app->front_root : app->back_root;

            AnimPlayer* anim = anim_player_alloc(root);
            if(anim_player_set_source(anim, power_on_anim_paths[display])) {
                anim_player_set_section(anim, POWER_ON_ANIM_FLAGS, POWER_ON_ANIM_SECTION);
            }
            scene->anims[display] = anim;
        }
    });
}

static void power_on_scene_animation_on_exit(void* context) {
    furi_assert(context);

    PowerOnApp* app = context;
    SceneAnimation* scene = scene_manager_get_scene_data(app->scene_manager, SceneIdAnimation);

    with_gui(app->gui, {
        for(GuiDisplayId display = 0; display < GuiDisplayIdMax; display++) {
            anim_player_free(scene->anims[display]);
        }

        GuiLayer* layer = gui_get_layer(app->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, power_on_scene_animation_input_callback);
    });

    furi_event_loop_timer_free(scene->shutdown_timer);
}

static bool power_on_scene_animation_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    PowerOnApp* app = context;
    SceneAnimation* scene = scene_manager_get_scene_data(app->scene_manager, SceneIdAnimation);
    UNUSED(scene);

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case PowerOnAppEventUserInteracted:
            power_on_done_flag_create(app);
            scene_manager_replace_current_scene(app->scene_manager, SceneIdUpdateFw);
            consumed = true;
            break;
        default:
            break;
        }
    }

    return consumed;
}

const Scene power_on_scene_animation = {
    .enter_callback = power_on_scene_animation_on_enter,
    .exit_callback = power_on_scene_animation_on_exit,
    .event_callback = power_on_scene_animation_on_event,
    .data_size = sizeof(SceneAnimation),
};
