#include "../../settings.h"

#include <gui/modules/flex_layout.h>
#include <gui/modules/qr_code.h>
#include <gui/modules/label.h>

#include <matter/matter.h>

typedef struct {
    FlexLayout* layouts[GuiDisplayIdMax];
    QRCode* qr_code;
    Label* manual_codes[GuiDisplayIdMax];
    Label* timers[GuiDisplayIdMax];

    FuriEventLoopTimer* pairing_timer;
    size_t pairing_deadline; // absolute furi tick

    _Atomic size_t menu_idx;
} SettingsSceneDebugApps;

static void settings_scene_matter_pairing_update_timer(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneDebugApps* scene = scene_manager_get_current_scene_data(app->scene_manager);

    int32_t ticks_left = (int32_t)scene->pairing_deadline - (int32_t)furi_get_tick();
    furi_check(furi_kernel_get_tick_frequency() == 1000); // TODO: furi_ticks_to_ms
    size_t secs_left = (ticks_left > 0) ? (ticks_left / 1000) : 0;

    size_t mins_left = secs_left / 60;
    secs_left %= 60;

    with_gui(app->gui, {
        for(GuiDisplayId display = 0; display < GuiDisplayIdMax; display++) {
            label_set_text_fmt(scene->timers[display], "%02d:%02d left", mins_left, secs_left);
        }
    });
}

static void settings_scene_matter_pairing_on_enter(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneDebugApps* scene = scene_manager_get_current_scene_data(app->scene_manager);

    FuriString* qr_code = furi_string_alloc();
    FuriString* man_code = furi_string_alloc();

    size_t window_secs = matter_enable_commissioning(app->matter, qr_code, man_code);

    with_gui(app->gui, {
        widget_set_visible(nav_bar_get_base(app->back_nav_bar), true);

        for(GuiDisplayId display = 0; display < GuiDisplayIdMax; display++) {
            Widget* window = (display == GuiDisplayIdFront) ? app->front_scene_window :
                                                              app->back_scene_window;
            scene->layouts[display] = flex_layout_alloc(window, FlexLayoutTypeColumn);
            Widget* layout_base = flex_layout_get_base(scene->layouts[display]);
            scene->manual_codes[display] = label_alloc(layout_base);
            scene->timers[display] = label_alloc(layout_base);

            label_set_text(scene->manual_codes[display], furi_string_get_cstr(man_code));
        }

        scene->qr_code = qr_code_alloc(flex_layout_get_base(scene->layouts[GuiDisplayIdBack]));
        qr_code_set_size(scene->qr_code, 31);
        qr_code_set_data(scene->qr_code, furi_string_get_cstr(qr_code));
    });

    scene->pairing_deadline = furi_get_tick() + furi_ms_to_ticks(window_secs * 1000);
    scene->pairing_timer = furi_event_loop_timer_alloc(
        app->event_loop,
        settings_scene_matter_pairing_update_timer,
        FuriEventLoopTimerTypePeriodic,
        app);
    furi_event_loop_timer_start(scene->pairing_timer, furi_ms_to_ticks(1000));
    settings_scene_matter_pairing_update_timer(app);

    furi_string_free(qr_code);
    furi_string_free(man_code);
}

static void settings_scene_matter_pairing_on_exit(void* context) {
    furi_assert(context);
    SettingsApp* app = context;
    SettingsSceneDebugApps* scene = scene_manager_get_current_scene_data(app->scene_manager);

    furi_event_loop_timer_stop(scene->pairing_timer);
    furi_event_loop_timer_free(scene->pairing_timer);

    with_gui(app->gui, {
        for(GuiDisplayId display = 0; display < GuiDisplayIdMax; display++) {
            flex_layout_free(scene->layouts[display]);
            label_free(scene->manual_codes[display]);
            label_free(scene->timers[display]);
        }
        qr_code_free(scene->qr_code);
    });
}

static bool settings_scene_matter_pairing_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SettingsApp* app = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeBack) {
        settings_pop_location(app);
    }

    return consumed;
}

const Scene settings_scene_matter_pairing = {
    .enter_callback = settings_scene_matter_pairing_on_enter,
    .exit_callback = settings_scene_matter_pairing_on_exit,
    .event_callback = settings_scene_matter_pairing_on_event,
    .data_size = sizeof(SettingsSceneDebugApps),
};
