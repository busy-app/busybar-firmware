#include "../busy.h"

#include <gui/modules/label.h>

typedef struct {
    Label* front_label;
    uint32_t countdown;
} BusySceneProgress;

static void busy_scene_progress_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneProgress* data = scene_manager_get_current_scene_data(instance->scene_manager);

    data->countdown = 3;

    BusyTimerCycles cycles;
    busy_timer_get_cycles(instance->busy_timer, &cycles);

    with_gui(instance->gui, {
        data->front_label = label_alloc(instance->front_window);
        label_set_text_fmt(
            data->front_label, "DONE: %lu/%lu", cycles.done_count, cycles.total_count);
        widget_set_align(label_get_base(data->front_label), AlignCenter);
    });
}

static void busy_scene_progress_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneProgress* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, { label_free(data->front_label); });
}

static bool busy_scene_progress_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneProgress* data = scene_manager_get_current_scene_data(instance->scene_manager);

    bool consumed = false;

    if(event->type == SceneManagerEventTypeTick) {
        data->countdown -= 1;

        if(data->countdown == 0) {
            scene_manager_next_scene(instance->scene_manager, BusyAppSceneIdNext);
        }

        consumed = true;

    } else if(event->type == SceneManagerEventTypeBack) {
        consumed = true;
    }

    return consumed;
}

const Scene busy_scene_progress = {
    .enter_callback = busy_scene_progress_on_enter,
    .exit_callback = busy_scene_progress_on_exit,
    .event_callback = busy_scene_progress_on_event,
    .data_size = sizeof(BusySceneProgress),
};
