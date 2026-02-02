#include "../apps_menu_i.h"
#include "../storage_macros.h"
#include "apps_menu_scenes.h"

#include <gui/modules/title_card.h>
#include <gui/modules/anim_title_card.h>

#include <lvgl.h>

#define STANDBY_ANIM_INITIAL_DELAY_MS 1000
#define STANDBY_ANIM_DELAY_MS         5000

typedef struct {
    AnimTitleCard* front_card;
    TitleCard* back_card;

    FuriEventLoopTimer* timer;

    bool is_not_first_enter;
    bool is_timer_initial_run;
} AppsMenuSceneStart;

#define ICON_SECTION_ENTER  "enter"
#define ICON_SECTION_STEADY "steady"
#define ICON_SECTION_EXIT   "exit"

static bool apps_menu_scene_start_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    AppsMenu* app = context;

    bool consumed = false;
    AppsMenuCustomEvent custom_event;
    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyStart:
        /* fall-through */
        case InputKeyOk:
            custom_event = AppsMenuCustomEventLaunchMain;
            consumed = true;
            break;

        default:
            break;
        }
    }

    if(consumed) {
        apps_menu_send_custom_event(app, custom_event);
    }

    return consumed;
}

static void apps_menu_scene_start_timer_callback(void* context) {
    furi_assert(context);

    AppsMenu* app = context;
    AppsMenuSceneStart* scene =
        scene_manager_get_scene_data(app->scene_manager, AppsMenuSceneIdStart);

    with_gui(app->gui, { anim_title_card_run_background_anim(scene->front_card); });

    if(scene->is_timer_initial_run) {
        furi_event_loop_timer_start(scene->timer, STANDBY_ANIM_DELAY_MS);
    }
}

static void apps_menu_start_run_icon_anim(AppsMenu* app, const char* section) {
    AppsMenuSceneStart* scene =
        scene_manager_get_scene_data(app->scene_manager, AppsMenuSceneIdStart);
    anim_title_card_run_icon_anim(scene->front_card, section);
}

static void apps_menu_scene_start_on_enter(void* context) {
    furi_assert(context);
    AppsMenu* app = context;
    AppsMenuSceneStart* scene =
        scene_manager_get_scene_data(app->scene_manager, AppsMenuSceneIdStart);

    with_gui(app->gui, {
        GuiLayer* layer = gui_get_layer(app->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, apps_menu_scene_start_input_callback, app);

        scene->front_card = anim_title_card_alloc(app->front_scene_window);
        anim_title_card_set_title(scene->front_card, "APPS");
        anim_title_card_set_icon(
            scene->front_card, APPS_MENU_ANIM_PATH("apps_menu_front_13x13.anim"));

        if(scene->is_not_first_enter) {
            apps_menu_start_run_icon_anim(app, ICON_SECTION_STEADY);
        } else {
            apps_menu_start_run_icon_anim(app, ICON_SECTION_ENTER);
            scene->is_not_first_enter = true;
        }

        scene->back_card = title_card_alloc(app->back_scene_window);
        title_card_set_title(scene->back_card, "APPS");
        title_card_set_icon(scene->back_card, APPS_MENU_IMG_PATH("apps_menu_back_18x18.bin"));

        widget_set_visible(nav_bar_get_base(app->back_nav_bar), false);
    });

    scene->is_timer_initial_run = true;
    scene->timer = furi_event_loop_timer_alloc(
        app->event_loop, apps_menu_scene_start_timer_callback, FuriEventLoopTimerTypePeriodic, app);

    furi_event_loop_timer_start(scene->timer, STANDBY_ANIM_INITIAL_DELAY_MS);
}

static void apps_menu_scene_start_on_exit(void* context) {
    furi_assert(context);
    AppsMenu* app = context;
    AppsMenuSceneStart* scene =
        scene_manager_get_scene_data(app->scene_manager, AppsMenuSceneIdStart);

    with_gui(app->gui, {
        GuiLayer* layer = gui_get_layer(app->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, apps_menu_scene_start_input_callback);

        furi_event_loop_timer_free(scene->timer);

        anim_title_card_free(scene->front_card);
        title_card_free(scene->back_card);
    });
}

static bool apps_menu_scene_start_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    AppsMenu* app = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case AppsMenuCustomEventLaunchMain:
            scene_manager_next_scene(app->scene_manager, AppsMenuSceneIdMain);
            consumed = true;
            break;

        case AppsMenuCustomEventAboutToExit:
            with_gui(app->gui, { apps_menu_start_run_icon_anim(app, ICON_SECTION_EXIT); });
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

const Scene apps_menu_scene_start = {
    .enter_callback = apps_menu_scene_start_on_enter,
    .exit_callback = apps_menu_scene_start_on_exit,
    .event_callback = apps_menu_scene_start_on_event,
    .data_size = sizeof(AppsMenuSceneStart),
};
