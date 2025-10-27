#include "../busy.h"
#include <gui/modules/image.h>
#include <gui/modules/anim_image_i.h>
#include "../widgets/timer_label.h"

#define ANIM_START_X     73
#define ANIM_END_X       43
#define ANIM_DURATION_MS (40 * 1000 / 60)

typedef struct {
    Widget* root;
    AnimImage* anim_image;
    TimerLabel* timer_label;
    BusyTimerTime timer_time;
} BusySceneTimerOffToSimple;

static bool busy_scene_timer_off_to_simple_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BusyApp* instance = context;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyUp) {
            busy_send_custom_event(instance, BusyCustomEventTimeIncrement);
            return true;
        } else if(event->key == InputKeyDown) {
            busy_send_custom_event(instance, BusyCustomEventTimeDecrement);
            return true;
        } else if(event->key == InputKeyStart) {
            busy_send_custom_event(instance, BusyCustomEventTimerTogglePause);
            return true;
        }
    }

    return false;
}

static void busy_scene_timer_off_to_simple_anim_image_completed_callback(
    AnimImage* instance,
    void* context) {
    UNUSED(instance);
    furi_assert(context);
    busy_send_custom_event((BusyApp*)context, BusyCustomEventOffToSimple);
}
static void
    busy_scene_timer_off_to_simple_event_callback(const BusyTimerEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneTimerOffToSimple* data =
        scene_manager_get_current_scene_data(instance->scene_manager);

    if(event->type == BusyTimerEventTypeTick) {
        data->timer_time = event->time;
        busy_send_custom_event(instance, BusyCustomEventTimerTick);
    } else if(event->type == BusyTimerEventTypeIntervalEnded) {
        busy_send_custom_event(instance, BusyCustomEventTimerIntervalEnded);
    }
}

static void busy_scene_timer_off_to_simple_lvgl_anim_callback(void* context, int32_t value) {
    furi_assert(context);

    // lv_obj_t* instance = context;
    // lv_obj_set_width(instance, value);
    Widget* instance = context;
    widget_set_pos(instance, value, 1);
}

void busy_scene_timer_off_to_simple_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneTimerOffToSimple* data =
        scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        gui_layer_add_input_callback(
            gui_get_layer(instance->gui, GuiLayerIdMain),
            busy_scene_timer_off_to_simple_input_callback,
            instance);

        data->root = widget_alloc(instance->front_window);

        data->anim_image = anim_image_alloc(data->root);
        widget_set_pos(anim_image_get_base(data->anim_image), 1, 1);
        anim_image_set_source(
            data->anim_image, "/ext/apps_assets/busy/animations/busy_label_transition_70x14.anim");
        anim_image_set_loop(data->anim_image, false);
        anim_image_set_completed_callback(
            data->anim_image,
            busy_scene_timer_off_to_simple_anim_image_completed_callback,
            instance);
        anim_image_start(data->anim_image);

        data->timer_label = timer_label_alloc(data->root);
        BusyTimerTime time = {0};
        busy_timer_get_time(instance->busy_timer, &time);
        timer_label_set_time(data->timer_label, time.remain_s);
        widget_set_pos(timer_label_get_base(data->timer_label), ANIM_START_X, 1);

        {
            lv_anim_t anim;
            lv_anim_init(&anim);

            lv_anim_set_values(&anim, ANIM_START_X, ANIM_END_X);
            lv_anim_set_duration(&anim, ANIM_DURATION_MS);

            lv_anim_set_bezier3_param(
                &anim,
                LV_BEZIER_VAL_FLOAT(0.3F),
                LV_BEZIER_VAL_FLOAT(0.0F),
                LV_BEZIER_VAL_FLOAT(0.3F),
                LV_BEZIER_VAL_FLOAT(1.0F));

            lv_anim_set_path_cb(&anim, lv_anim_path_custom_bezier3);
            lv_anim_set_exec_cb(&anim, busy_scene_timer_off_to_simple_lvgl_anim_callback);
            // lv_anim_set_completed_cb(&anim, busy_scene_timer_off_to_simple_lvgl_anim_completed_callback);
            lv_anim_set_var(&anim, timer_label_get_base(data->timer_label));

            lv_anim_start(&anim);
        }

        widget_set_visible(timer_card_get_base(instance->timer_card), true);
        timer_card_set_time(instance->timer_card, data->timer_time.remain_s);
        timer_card_show_header(instance->timer_card, true);
        timer_card_show_time(instance->timer_card, true);
    });

    busy_timer_set_callback(
        instance->busy_timer, busy_scene_timer_off_to_simple_event_callback, instance);
}

void busy_scene_timer_off_to_simple_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;

    busy_set_status_lights(instance, BusyStatusLightsTypeOff);

    BusySceneTimerOffToSimple* data =
        scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        gui_layer_remove_input_callback(
            gui_get_layer(instance->gui, GuiLayerIdMain),
            busy_scene_timer_off_to_simple_input_callback);

        timer_card_show_header(instance->timer_card, false);
        timer_card_show_time(instance->timer_card, false);

        widget_free(data->root);
    });
}

static bool
    busy_scene_timer_off_to_simple_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusyCustomEventTimerTick) {
            const BusySceneTimerOffToSimple* data =
                scene_manager_get_current_scene_data(instance->scene_manager);

            with_gui(instance->gui, {
                timer_label_set_time(data->timer_label, data->timer_time.remain_s);
                timer_card_set_time(instance->timer_card, data->timer_time.remain_s);
            });
        } else if(event->event == BusyCustomEventOffToSimple) {
            scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdTimerSimple);
        } else if(event->event == BusyCustomEventTimeIncrement) {
            busy_timer_add_time(instance->busy_timer, BUSY_TIMER_TIME_INCREMENT_MN);
        } else if(event->event == BusyCustomEventTimeDecrement) {
            busy_timer_add_time(instance->busy_timer, -BUSY_TIMER_TIME_INCREMENT_MN);
        }

        consumed = true;
    }

    return consumed;
}

const Scene busy_scene_timer_off_to_simple = {
    .enter_callback = busy_scene_timer_off_to_simple_on_enter,
    .exit_callback = busy_scene_timer_off_to_simple_on_exit,
    .event_callback = busy_scene_timer_off_to_simple_on_event,
    .data_size = sizeof(BusySceneTimerOffToSimple),
};
