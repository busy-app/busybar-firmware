#include "../busy.h"
#include "../widgets/nav_header.h"

#include <gui/modules/menu.h>
#include <gui/modules/anim_image.h>
#include <gui/modules/flex_layout.h>

typedef struct {
    FlexLayout* back_layout;
    NavHeader* back_header;
    Menu* back_menu;
} BusySceneSetup;

typedef enum {
    BusySceneSetupMenuIndexTimer,
    BusySceneSetupMenuIndexTheme,
    BusySceneSetupMenuIndexMax,
} BusySceneStartMenuIndex;

static void busy_scene_setup_menu_callback(uint32_t index, void* context) {
    furi_assert(index < BusySceneSetupMenuIndexMax);
    furi_assert(context);

    BusyApp* instance = context;
    busy_send_custom_event(instance, index);
}

static void busy_scene_setup_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetup* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        data->back_layout = flex_layout_alloc(instance->back_window, FlexLayoutTypeColumn);

        data->back_header = nav_header_alloc(flex_layout_get_base(data->back_layout));
        nav_header_set_image(data->back_header, (const void*)&I_header_40x16);
        nav_header_push_location(data->back_header, "SETUP");

        data->back_menu = menu_alloc(flex_layout_get_base(data->back_layout));
        menu_add_item(
            data->back_menu,
            "TIMER",
            "Interval",
            (const void*)&I_timer_12x12,
            BusySceneSetupMenuIndexTimer,
            busy_scene_setup_menu_callback,
            instance);
        menu_add_item(
            data->back_menu,
            "THEME",
            "",
            (const void*)&I_theme_12x12,
            BusySceneSetupMenuIndexTheme,
            busy_scene_setup_menu_callback,
            instance);
    });
}

static void busy_scene_setup_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetup* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, { flex_layout_free(data->back_layout); });
}

static bool busy_scene_setup_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BusyApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == BusySceneSetupMenuIndexTimer) {
            FURI_LOG_D(TAG, "Timer selected!");
        } else if(event->event == BusySceneSetupMenuIndexTheme) {
            FURI_LOG_D(TAG, "Theme selected!");
        }
        consumed = true;
    } else if(event->type == SceneManagerEventTypeBack) {
        // TODO: React to unhandled back events
        scene_manager_switch_to_scene(instance->scene_manager, BusyAppSceneIdStart);
        consumed = true;
    }

    return consumed;
}

const Scene busy_scene_setup = {
    .enter_callback = busy_scene_setup_on_enter,
    .exit_callback = busy_scene_setup_on_exit,
    .event_callback = busy_scene_setup_on_event,
    .data_size = sizeof(BusySceneSetup),
};
