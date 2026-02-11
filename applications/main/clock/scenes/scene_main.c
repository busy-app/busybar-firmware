#include "../clock_i.h"

#include <gui/modules/label.h>

#define TAG "ClockSceneMain"

#define CLOCK_INTERVAL_UPDATE_MS 500

typedef struct {
    Label* front_label;
    Label* back_label;
} ThisScene;

static inline ThisScene* this_get_scene(ThisInstance* instance) {
    return scene_manager_get_scene_data(instance->scene_manager, ThisSceneIdxMain);
}

const char* clock_get_time_string(ThisInstance* instance) {
    furi_assert(instance);

    LocalTime lt = sntp_get_local_time(instance->sntp);

    SntpSettings sntp_settings;
    sntp_get_settings(instance->sntp, &sntp_settings);

    switch(sntp_settings.time_format) {
    case SntpSettingTimeFormat24h:
        furi_string_printf(
            instance->time_string,
            "   %02hhu:%02hhu:%02hhu\n%02hhu-%02hhu-%04hu",
            lt.dt.hour,
            lt.dt.minute,
            lt.dt.second,
            lt.dt.dayofmonth,
            lt.dt.month,
            lt.dt.year);
        break;
    case SntpSettingTimeFormat12h: {
        uint8_t h = lt.dt.hour % 12;
        if(h == 0) {
            h = 12;
        }

        bool pm = lt.dt.hour / 12;

        furi_string_printf(
            instance->time_string,
            " %02hhu:%02hhu:%02hhu%s\n%02hhu-%02hhu-%04hu",
            h,
            lt.dt.minute,
            lt.dt.second,
            pm ? "pm" : "am",
            lt.dt.dayofmonth,
            lt.dt.month,
            lt.dt.year);
        break;
    }
    default:
        furi_string_set(instance->time_string, "ERROR");
        break;
    }

    return furi_string_get_cstr(instance->time_string);
}

static void this_scene_on_enter(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    with_gui(instance->gui, {
        scene->front_label = label_alloc(instance->front_scene_window);
        label_set_text(scene->front_label, clock_get_time_string(instance));
        widget_set_align(label_get_base(scene->front_label), AlignCenter);

        scene->back_label = label_alloc(instance->back_scene_window);
        label_set_text(scene->back_label, clock_get_time_string(instance));
        widget_set_align(label_get_base(scene->back_label), AlignCenter);

        nav_bar_push_location(instance->back_nav_bar, "CLOCK");

        label_set_text(scene->front_label, clock_get_time_string(instance));
        label_set_text(scene->back_label, clock_get_time_string(instance));
    });

    furi_event_loop_timer_start(instance->timer, CLOCK_INTERVAL_UPDATE_MS);
}

static void this_scene_on_exit(void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    furi_event_loop_timer_stop(instance->timer);

    with_gui(instance->gui, {
        if(scene->front_label) {
            label_free(scene->front_label);
            scene->front_label = NULL;
        }

        if(scene->back_label) {
            label_free(scene->back_label);
            scene->back_label = NULL;
        }

        nav_bar_pop_location(instance->back_nav_bar);
    });
}

static bool this_scene_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    ThisInstance* instance = context;
    ThisScene* scene = this_get_scene(instance);

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case ThisEventTimerUpdate:
            with_gui(instance->gui, {
                label_set_text(scene->front_label, clock_get_time_string(instance));
                label_set_text(scene->back_label, clock_get_time_string(instance));
            });
            return true;

        default:
            break;
        }
    }

    return false;
}

const Scene clock_app_scene_main = {
    .enter_callback = this_scene_on_enter,
    .exit_callback = this_scene_on_exit,
    .event_callback = this_scene_on_event,
    .data_size = sizeof(ThisScene),
};
