#include "../about.h"

#include <gui/modules/label.h>

#define GREY_TEXT(text) "#888888 " text "#"

typedef enum {
    SceneEventPowerInfoStatusChangedEvent = AppEventSceneEventsStart,
} SceneEventPowerInfoEvent;

typedef struct {
    Label* complience_info[GuiDisplayIdMax];
    FuriString* complience_info_str;
} AboutSceneComplience;

static void about_scene_complience_on_enter(void* context) {
    furi_assert(context);
    About* instance = context;

    AboutSceneComplience* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdComplience);
    scene->complience_info_str = furi_string_alloc();
    furi_string_printf(
        scene->complience_info_str, GREY_TEXT("Complience:") " %s%%\n", "Hui ego znaet");

    Widget* const windows[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = instance->front_scene_window,
        [GuiDisplayIdBack] = instance->back_scene_window,
    };

    with_gui(instance->gui, {
        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            widget_set_scrollbar_mode(windows[disp], WidgetScrollBarModeAuto);
            scene->complience_info[disp] = label_alloc(windows[disp]);
            label_set_inline_text_color_formatting(scene->complience_info[disp], true);
            label_set_text(
                scene->complience_info[disp], furi_string_get_cstr(scene->complience_info_str));
        }
    });
}

static void about_scene_complience_on_exit(void* context) {
    furi_assert(context);
    About* instance = context;
    AboutSceneComplience* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdComplience);

    furi_string_free(scene->complience_info_str);
    furi_event_loop_tick_set(instance->event_loop, 0, NULL, NULL);

    with_gui(instance->gui, {
        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            label_free(scene->complience_info[disp]);
        }
    });
}

static bool about_scene_complience_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    About* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeBack) {
        about_pop_location(instance);
        consumed = scene_manager_previous_scene(instance->scene_manager);
    }

    return consumed;
}

const Scene about_scene_complience = {
    .enter_callback = about_scene_complience_on_enter,
    .exit_callback = about_scene_complience_on_exit,
    .event_callback = about_scene_complience_on_event,
    .data_size = sizeof(AboutSceneComplience),
};
