#include "../busy.h"
#include "../widgets/nav_header.h"

#include <gui/modules/var_item_list.h>
#include <gui/modules/anim_image.h>
#include <gui/modules/flex_layout.h>

typedef struct {
    FlexLayout* back_layout;
    NavHeader* back_header;
    VarItemList* back_list;
} BusySceneSetupTimer;

static void busy_scene_setup_timer_simple_changed_callback(VarItem* item, void* context) {
    UNUSED(item);

    BusyApp* instance = context;
    UNUSED(instance);
}

static void busy_scene_setup_timer_on_enter(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetupTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        data->back_layout = flex_layout_alloc(instance->back_window, FlexLayoutTypeColumn);

        data->back_header = nav_header_alloc(flex_layout_get_base(data->back_layout));
        nav_header_set_image(data->back_header, (const void*)&I_header_40x16);
        nav_header_push_location(data->back_header, "SETUP");
        nav_header_push_location(data->back_header, "TIMER");

        data->back_list = var_item_list_alloc(flex_layout_get_base(data->back_layout));
        // TODO: Fix the layout to set appropriate sizes for children
        widget_set_height(var_item_list_get_base(data->back_list), 56);

        VarItem* item;

        item = var_item_list_add_timebox(
            data->back_list,
            "SIMPLE",
            5,
            H_TO_M(24),
            5,
            busy_scene_setup_timer_simple_changed_callback,
            instance);
        item = var_item_list_add_switch(
            data->back_list, "INTERVAL", busy_scene_setup_timer_simple_changed_callback, instance);
        item = var_item_list_add_switch(
            data->back_list, "OFF", busy_scene_setup_timer_simple_changed_callback, instance);

        UNUSED(item);
    });
}

static void busy_scene_setup_timer_on_exit(void* context) {
    furi_assert(context);

    BusyApp* instance = context;
    BusySceneSetupTimer* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, { flex_layout_free(data->back_layout); });
}

static bool busy_scene_setup_timer_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    BusyApp* instance = context;
    UNUSED(instance);

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        // TODO: Handle custom events
        consumed = true;
    }

    return consumed;
}

const Scene busy_scene_setup_timer = {
    .enter_callback = busy_scene_setup_timer_on_enter,
    .exit_callback = busy_scene_setup_timer_on_exit,
    .event_callback = busy_scene_setup_timer_on_event,
    .data_size = sizeof(BusySceneSetupTimer),
};
