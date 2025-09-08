#include "../apps_menu_i.h"
#include "apps_menu_scenes.h"

#include <gui/modules/app_title_card.h>

typedef struct {
    AppTitleCard* front_card;
    AppTitleCard* back_card;
} AppsMenuSceneStart;

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

static void apps_menu_scene_start_on_enter(void* context) {
    furi_assert(context);
    AppsMenu* app = context;
    AppsMenuSceneStart* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        widget_set_visible(nav_bar_get_base(app->back_nav_bar), false);

        GuiLayer* layer = gui_get_layer(app->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, apps_menu_scene_start_input_callback, app);

        scene->front_card = app_title_card_alloc(app->front_scene_window);
        app_title_card_set_text(scene->front_card, "APPS");
        app_title_card_set_image(scene->front_card, APPS_MENU_IMG("apps_menu_front_13x13"));

        scene->back_card = app_title_card_alloc(app->back_scene_window);
        app_title_card_set_text(scene->back_card, "APPS");
        app_title_card_set_image(scene->back_card, APPS_MENU_IMG("apps_menu_back_18x18"));
    });
}

static void apps_menu_scene_start_on_exit(void* context) {
    furi_assert(context);
    AppsMenu* app = context;
    AppsMenuSceneStart* scene = scene_manager_get_current_scene_data(app->scene_manager);

    with_gui(app->gui, {
        GuiLayer* layer = gui_get_layer(app->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, apps_menu_scene_start_input_callback);

        app_title_card_free(scene->front_card);
        app_title_card_free(scene->back_card);
        scene->front_card = NULL;
        scene->back_card = NULL;
    });
}

static bool apps_menu_scene_start_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    AppsMenu* app = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == AppsMenuCustomEventLaunchMain) {
            scene_manager_next_scene(app->scene_manager, AppsMenuSceneIdMain);
        }

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
