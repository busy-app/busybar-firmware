#include "busy_i.h"

#include "scenes/busy_scene_start.h"
#include "scenes/busy_scene_busy.h"

static const BusyAppScene* busy_scenes[BusyAppSceneIdMax] = {
    [BusyAppSceneIdStart] = &busy_scene_start,
    [BusyAppSceneIdBusy] = &busy_scene_busy,
};

void busy_switch_to_scene(BusyApp* instance, BusyAppSceneId scene_id) {
    if(instance->current_scene) {
        instance->current_scene->on_exit(instance);
    }

    instance->current_scene = busy_scenes[scene_id];
    instance->current_scene->on_enter(instance);
}

void busy_send_custom_event(BusyApp* instance, uint32_t event) {
    furi_check(
        furi_message_queue_put(instance->event_queue, &event, FuriWaitForever) == FuriStatusOk);
}

static void busy_event_queue_callback(FuriEventLoopObject* object, void* context) {
    BusyApp* instance = context;
    furi_check(object == instance->event_queue);

    uint32_t event;
    furi_check(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk);

    if(instance->current_scene) {
        instance->current_scene->on_event(event, instance);
    }
}

static BusyApp* busy_alloc(void) {
    BusyApp* instance = malloc(sizeof(BusyApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->event_queue = furi_message_queue_alloc(16, sizeof(uint32_t));
    instance->gui = furi_record_open(RECORD_GUI_LVGL);
    instance->busy_interval_s = BUSY_INTERVAL_DEFAULT_S;
    instance->rest_interval_s = REST_INTERVAL_DEFAULT_S;

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        busy_event_queue_callback,
        instance);

    busy_switch_to_scene(instance, BusyAppSceneIdStart);

    return instance;
}

static void busy_free(BusyApp* instance) {
    furi_record_close(RECORD_GUI_LVGL);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t busy_app(void* arg) {
    UNUSED(arg);

    BusyApp* instance = busy_alloc();
    furi_event_loop_run(instance->event_loop);
    busy_free(instance);

    return 0;
}
