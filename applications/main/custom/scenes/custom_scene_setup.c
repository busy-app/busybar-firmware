#include "../custom.h"

#include <gui/modules/menu.h>
#include <gui/modules/anim_image.h>

typedef struct {
    Menu* front_menu;
    Menu* back_menu;
} CustomSceneSetup;

typedef enum {
    CustomSceneSetupMenuIndexTheme,
    CustomSceneSetupMenuIndexMax,
} CustomSceneSetupMenuIndex;

static void custom_scene_setup_menu_callback(uint32_t index, void* context) {
    furi_assert(index < CustomSceneSetupMenuIndexMax);
    furi_assert(context);

    CustomApp* instance = context;
    custom_send_custom_event(instance, index);
}

static void custom_scene_setup_on_enter(void* context) {
    furi_assert(context);

    CustomApp* instance = context;
    CustomSceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, CustomAppSceneIdSetup);

    with_gui(instance->gui, {
        data->front_menu = menu_alloc(instance->front_window);
        menu_add_item(
            data->front_menu,
            "Theme",
            "",
            CUSTOM_IMG_PATH("theme_8x8.bin"),
            CustomSceneSetupMenuIndexTheme,
            custom_scene_setup_menu_callback,
            instance);

        data->back_menu = menu_alloc(instance->back_window);
        menu_add_item(
            data->back_menu, "THEME", "", CUSTOM_IMG_PATH("theme_12x12.bin"), 0, NULL, NULL);
    });
}

static void custom_scene_setup_on_exit(void* context) {
    furi_assert(context);

    CustomApp* instance = context;
    CustomSceneSetup* data =
        scene_manager_get_scene_data(instance->scene_manager, CustomAppSceneIdSetup);

    with_gui(instance->gui, {
        menu_free(data->front_menu);
        menu_free(data->back_menu);
    });
}

static bool custom_scene_setup_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    CustomApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == CustomSceneSetupMenuIndexTheme) {
            custom_push_location(instance, "THEME");
            scene_manager_next_scene(instance->scene_manager, CustomAppSceneIdSetupTheme);
        }

        consumed = true;

    } else if(event->type == SceneManagerEventTypeBack) {
        custom_pop_location(instance);
    }

    return consumed;
}

const Scene custom_scene_setup = {
    .enter_callback = custom_scene_setup_on_enter,
    .exit_callback = custom_scene_setup_on_exit,
    .event_callback = custom_scene_setup_on_event,
    .data_size = sizeof(CustomSceneSetup),
};
