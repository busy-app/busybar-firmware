#include "animation_player_app_i.h"

#include <input/input.h>
#include <lib/lvgl/src/widgets/canvas/lv_canvas.h>

#include <furi.h>

#define TAG "AnimationPlayer"

#define ANIMATION_PLAYER_FILE_PATH "/ext/animations/test.anim"

static void animation_player_app_keypad_callback(lv_event_t* event) {
    AnimationPlayerApp* instance = lv_event_get_user_data(event);

    // TODO Fix focus and differ OK and START buttons
    const lv_event_code_t code = lv_event_get_code(event);
    if(code == LV_EVENT_KEY) {
        AnimationPlayerAppEvent app_event;
        const uint32_t key = *((uint32_t*)lv_event_get_param(event));
        if(key == LV_KEY_ESC) {
            app_event = AnimationPlayerAppEventExit;
            furi_check(
                furi_message_queue_put(instance->event_queue, &app_event, FuriWaitForever) ==
                FuriStatusOk);
        }
    }
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

    instance->gui = furi_record_open(RECORD_GUI_LVGL);
    gui_lvgl_acquire(instance->gui);

    lv_obj_t* active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdBack, GuiLayerIdActive);
    instance->label = lv_label_create(active);

    lv_obj_set_pos(instance->label, 10, 30);
    lv_obj_set_style_text_color(instance->label, lv_color_white(), LV_PART_MAIN);

    lv_group_add_obj(lv_group_get_default(), instance->label);
    lv_group_focus_obj(instance->label);
    lv_obj_add_event_cb(
        instance->label, animation_player_app_keypad_callback, LV_EVENT_KEY, instance);

    active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);
    instance->image_animation = image_animation_alloc(active);
    const char* path = (args == NULL) ? ANIMATION_PLAYER_FILE_PATH : args;
    if(!image_animation_set_source(instance->image_animation, path)) {
        FURI_LOG_E(TAG, "Failed to launch animation");
        lv_label_set_text(instance->label, "Failed to load animation");
    } else {
        lv_label_set_text(instance->label, "Running animation");
        image_animation_start(instance->image_animation);
    }

    gui_lvgl_release(instance->gui);
    return instance;
}

static void animation_player_app_free(AnimationPlayerApp* instance) {
    furi_check(instance);

    gui_lvgl_acquire(instance->gui);

    image_animation_stop(instance->image_animation);
    image_animation_free(instance->image_animation);
    lv_obj_delete(instance->label);

    gui_lvgl_release(instance->gui);

    furi_record_close(RECORD_GUI_LVGL);

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
