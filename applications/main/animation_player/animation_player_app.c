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

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        instance->input_events = gui_layer_subscribe_to_input_events(
            main_layer, animation_player_app_input_callback, instance);

        Widget* root;
        root = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);
        instance->label = label_alloc(root);

        widget_set_align(label_get_base(instance->label), AlignCenter);

        root = gui_layer_get_root_widget(main_layer, GuiDisplayIdFront);
        instance->anim_image = anim_image_alloc(root);

        const char* path = (args == NULL) ? ANIMATION_PLAYER_FILE_PATH : args;

        if(!anim_image_set_source(instance->anim_image, path)) {
            FURI_LOG_E(TAG, "Failed to load animation");
            label_set_text(instance->label, "Failed to load animation");

        } else {
            label_set_text(instance->label, "Running animation");
            anim_image_start(instance->anim_image);
        }
    });

    return instance;
}

static void animation_player_app_free(AnimationPlayerApp* instance) {
    furi_check(instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_unsubscribe_from_input_events(main_layer, instance->input_events);

        anim_image_stop(instance->anim_image);
        anim_image_free(instance->anim_image);
        label_free(instance->label);
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
