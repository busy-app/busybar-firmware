#include "animation_player_app_i.h"

#include <furi.h>

#define TAG "AnimationPlayer"

#define ANIMATION_PLAYER_FILE_PATH EXT_PATH("animations/test.anim")

static bool animation_player_app_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    AnimationPlayerApp* instance = context;
    AnimationPlayerAppEvent app_event;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            consumed = true;
            app_event = AnimationPlayerAppEventExit;
            furi_check(
                furi_message_queue_put(instance->event_queue, &app_event, FuriWaitForever) ==
                FuriStatusOk);
        }
    }

    return consumed;
}

static void animation_player_app_event_queue_callback(FuriEventLoopObject* object, void* context) {
    AnimationPlayerApp* instance = context;
    furi_check(object == instance->event_queue);

    AnimationPlayerAppEvent event;
    furi_check(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk);

    if(event == AnimationPlayerAppEventExit) {
        furi_event_loop_stop(instance->event_loop);
    }
}

static AnimationPlayerApp* animation_player_app_alloc(void* args) {
    AnimationPlayerApp* instance = malloc(sizeof(AnimationPlayerApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->event_queue = furi_message_queue_alloc(16, sizeof(AnimationPlayerAppEvent));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        animation_player_app_event_queue_callback,
        instance);

    instance->gui = furi_record_open(RECORD_GUI);

    FuriString* args_str = furi_string_alloc_set_str(args ? args : "");
    size_t colon_index = furi_string_search_char(args_str, ':');

    GuiDisplayId display_arg = GuiDisplayIdFront;

    if(colon_index != FURI_STRING_FAILURE) {
        if(furi_string_start_with_str(args_str, "back:")) {
            display_arg = GuiDisplayIdBack;
        }
        furi_string_right(args_str, colon_index + 1);
    }

    const char* path_arg = furi_string_get_cstr(args_str);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, animation_player_app_input_callback, instance);

        Widget* root = gui_layer_get_root_widget(main_layer, display_arg);
        instance->anim_player = anim_player_alloc(root);

        anim_player_set_source(instance->anim_player, path_arg);
    });

    furi_string_free(args_str);

    return instance;
}

static void animation_player_app_free(AnimationPlayerApp* instance) {
    furi_check(instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, animation_player_app_input_callback);

        anim_player_free(instance->anim_player);
    });

    furi_record_close(RECORD_GUI);

    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_message_queue_free(instance->event_queue);
    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t animation_player_app(void* args) {
    AnimationPlayerApp* instance = animation_player_app_alloc(args);
    furi_event_loop_run(instance->event_loop);
    animation_player_app_free(instance);

    return 0;
}
