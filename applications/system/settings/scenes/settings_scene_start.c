#include "../settings.h"
#include "settings_scenes.h"
#include "../storage_macros.h"

#include <gui/modules/title_card.h>
#include <gui/modules/anim_title_card.h>

#define STANDBY_ANIM_INITIAL_DELAY_MS 1000
#define STANDBY_ANIM_DELAY_MS         5000

typedef enum {
    SceneCustomEventStartOrOkClicked = SettingsCustomEventSceneEventsStart,
} SceneCustomEvent;

typedef struct {
    AnimTitleCard* front_card;
    TitleCard* back_card;

    FuriEventLoopTimer* timer;

    bool is_not_first_enter;
    bool is_timer_initial_run;
} SettingsSceneStart;

typedef enum {
    SettingsSceneStartInOutAnimTypeNone,

    SettingsSceneStartInOutAnimTypeEnter,
    SettingsSceneStartInOutAnimTypeExit,
} SettingsSceneStartInOutAnimType;

typedef struct {
    uint32_t icon_start;
    uint32_t icon_stop;
} SettingsSceneStartInOutAnimInfo;

static const SettingsSceneStartInOutAnimInfo in_out_anim_infos[] = {
    [SettingsSceneStartInOutAnimTypeNone] =
        {
            .icon_start = 59,
            .icon_stop = 59,
        },
    [SettingsSceneStartInOutAnimTypeEnter] =
        {
            .icon_start = 0,
            .icon_stop = 59,
        },
    [SettingsSceneStartInOutAnimTypeExit] =
        {
            .icon_start = 60,
            .icon_stop = 67,
        },
};

static bool settings_scene_start_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    SettingsApp* instance = context;

    bool consumed = false;
    SceneCustomEvent custom_event;
    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyStart:
        /* fall-through */
        case InputKeyOk:
            custom_event = SceneCustomEventStartOrOkClicked;
            consumed = true;
            break;

        default:
            break;
        }
    }

    if(consumed) {
        settings_send_custom_event(instance, custom_event);
    }

    return consumed;
}

static void settings_scene_start_timer_callback(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneStart* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, { anim_title_card_run_background_anim(data->front_card); });

    if(data->is_timer_initial_run) {
        furi_event_loop_timer_start(data->timer, STANDBY_ANIM_DELAY_MS);
    }
}

static void
    settings_start_run_in_out_anim(SettingsApp* instance, SettingsSceneStartInOutAnimType type) {
    SettingsSceneStart* data = scene_manager_get_current_scene_data(instance->scene_manager);

    const SettingsSceneStartInOutAnimInfo* anim_info = &in_out_anim_infos[type];
    anim_title_card_run_icon_anim(data->front_card, anim_info->icon_start, anim_info->icon_stop);
}

static void settings_scene_start_on_enter(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneStart* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, settings_scene_start_input_callback, instance);

        data->front_card = anim_title_card_alloc(instance->front_scene_window);
        anim_title_card_set_title(data->front_card, "SETTINGS");
        anim_title_card_set_icon(
            data->front_card, SETTINGS_ANIM_PATH("settings_front_13x13.anim"));

        if(data->is_not_first_enter) {
            settings_start_run_in_out_anim(instance, SettingsSceneStartInOutAnimTypeNone);
        } else {
            settings_start_run_in_out_anim(instance, SettingsSceneStartInOutAnimTypeEnter);
            data->is_not_first_enter = true;
        }

        data->back_card = title_card_alloc(instance->back_scene_window);
        title_card_set_title(data->back_card, "SETTINGS");
        title_card_set_icon(data->back_card, SETTINGS_IMG_PATH("settings_back_18x18.bin"));

        widget_set_visible(nav_bar_get_base(instance->back_nav_bar), false);
    });

    data->is_timer_initial_run = true;
    data->timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        settings_scene_start_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);

    furi_event_loop_timer_start(data->timer, STANDBY_ANIM_INITIAL_DELAY_MS);
}

static void settings_scene_start_on_exit(void* context) {
    furi_assert(context);

    SettingsApp* instance = context;
    SettingsSceneStart* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, settings_scene_start_input_callback);

        furi_event_loop_timer_free(data->timer);

        anim_title_card_free(data->front_card);
        title_card_free(data->back_card);
    });
}

static bool settings_scene_start_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    SettingsApp* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SceneCustomEventStartOrOkClicked:
            scene_manager_next_scene(instance->scene_manager, SettingsAppSceneIdMain);
            consumed = true;
            break;

        case SettingsCustomEventAboutToExit:
            with_gui(instance->gui, {
                settings_start_run_in_out_anim(instance, SettingsSceneStartInOutAnimTypeExit);
            });
            consumed = true;
            break;

        default:
            break;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        consumed = true;
    }

    return consumed;
}

const Scene settings_scene_start = {
    .enter_callback = settings_scene_start_on_enter,
    .exit_callback = settings_scene_start_on_exit,
    .event_callback = settings_scene_start_on_event,
    .data_size = sizeof(SettingsSceneStart),
};
