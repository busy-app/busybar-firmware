#include "../firmware_i.h"

#include <gui/modules/status_view.h>

#define SCENE_EXIT_TIMEOUT_MS 3000

typedef struct {
    StatusView* front_status;
    StatusView* back_status;

    FuriEventLoopTimer* timeout_timer;
} FirmwareSettingsCheckResultScene;

static inline FirmwareSettingsCheckResultScene*
    firmware_settings_check_result_scene_get(FirmwareSettings* instance) {
    return scene_manager_get_scene_data(
        instance->scene_manager, FirmwareSettingsSceneIdxCheckResult);
}

static void firmware_settings_check_result_scene_timeout_callback(void* context) {
    FirmwareSettings* instance = context;
    scene_manager_previous_scene(instance->scene_manager);
}

static void firmware_settings_check_result_scene_on_enter(void* context) {
    furi_assert(context);

    FirmwareSettings* instance = context;
    FirmwareSettingsCheckResultScene* scene = firmware_settings_check_result_scene_get(instance);

    with_gui(instance->gui, {
        /* front layout setup */
        scene->front_status = status_view_alloc(instance->front_scene_window);
        status_view_set_icon(
            scene->front_status, instance->check_result_preset.front_image_path, false);
        status_view_set_primary_text(
            scene->front_status, furi_string_get_cstr(instance->check_result_preset.front_text));

        /* back layout setup */
        scene->back_status = status_view_alloc(instance->back_scene_window);
        status_view_set_icon(
            scene->back_status, instance->check_result_preset.back_image_path, false);
        status_view_set_primary_text(
            scene->back_status,
            furi_string_get_cstr(instance->check_result_preset.back_primary_text));
        status_view_set_auxiliary_text(
            scene->back_status,
            furi_string_get_cstr(instance->check_result_preset.back_detail_text));
    });

    scene->timeout_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        firmware_settings_check_result_scene_timeout_callback,
        FuriEventLoopTimerTypeOnce,
        instance);
    furi_event_loop_timer_start(scene->timeout_timer, furi_ms_to_ticks(SCENE_EXIT_TIMEOUT_MS));
}

static void firmware_settings_check_result_scene_on_exit(void* context) {
    furi_assert(context);

    FirmwareSettings* instance = context;
    FirmwareSettingsCheckResultScene* scene = firmware_settings_check_result_scene_get(instance);

    furi_event_loop_timer_free(scene->timeout_timer);

    with_gui(instance->gui, {
        status_view_free(scene->back_status);
        status_view_free(scene->front_status);
    });
}

static bool
    firmware_settings_check_result_scene_on_event(const SceneManagerEvent* event, void* context) {
    UNUSED(event);
    UNUSED(context);

    return false;
}

const Scene firmware_settings_internal_scene_check_result = {
    .enter_callback = firmware_settings_check_result_scene_on_enter,
    .exit_callback = firmware_settings_check_result_scene_on_exit,
    .event_callback = firmware_settings_check_result_scene_on_event,
    .data_size = sizeof(FirmwareSettingsCheckResultScene),
};
