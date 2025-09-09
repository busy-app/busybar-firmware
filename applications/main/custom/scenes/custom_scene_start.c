#include "../custom.h"
#include <busy/widgets/anim_menu.h>

#include <gui/modules/menu.h>
#include <gui/modules/anim_image.h>
#include <gui/modules/flex_layout.h>

#include <lvgl.h>

#define ANIM_MENU_IDLE_FRAMES       (120)
#define ANIM_MENU_TRANSITION_FRAMES (10)

typedef struct {
    FlexLayout* front_layout;
    AnimImage* front_logo;
    AnimMenu* front_menu;
    Menu* back_menu;

    bool is_not_first_enter;
} CustomSceneStart;

typedef enum {
    CustomSceneStartMenuIndexStart,
    CustomSceneStartMenuIndexSetup,
    CustomSceneStartMenuIndexMax,
} CustomSceneStartMenuIndex;

typedef enum {
    CustomSceneStartInOutAnimTypeEnter,
    CustomSceneStartInOutAnimTypeExit
} CustomSceneStartInOutAnimType;

typedef struct {
    int32_t start;
    int32_t stop;
    uint32_t duration;
} CustomSceneStartInOutAnimInfo;

static const CustomSceneStartInOutAnimInfo in_out_anim_infos[][DesktopSwitchDirectionsCount] = {
    [CustomSceneStartInOutAnimTypeEnter] =
        {
            [DesktopSwitchDirectionUp] = {.start = 8, .stop = 0, .duration = 135},
            [DesktopSwitchDirectionDown] = {.start = -8, .stop = 0, .duration = 135},
        },
    [CustomSceneStartInOutAnimTypeExit] =
        {
            [DesktopSwitchDirectionUp] = {.start = 0, .stop = -8, .duration = 135},
            [DesktopSwitchDirectionDown] = {.start = 0, .stop = 8, .duration = 135},
        },
};

static void custom_scene_start_menu_callback(uint32_t index, void* context) {
    furi_assert(index < CustomSceneStartMenuIndexMax);
    furi_assert(context);

    CustomApp* instance = context;
    custom_send_custom_event(instance, index);
}

static void custom_scene_start_anim_exec_callback(void* var, int32_t value) {
    lv_obj_set_style_translate_x(var, value, LV_PART_MAIN);
}

static void
    custom_scene_start_run_in_out_anim(CustomApp* instance, CustomSceneStartInOutAnimType type) {
    const CustomSceneStartInOutAnimInfo* anim_info =
        &in_out_anim_infos[type][desktop_get_switch_direction(instance->desktop)];

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, instance->front_window);
    lv_anim_set_values(&anim, anim_info->start, anim_info->stop);
    lv_anim_set_duration(&anim, anim_info->duration);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_set_exec_cb(&anim, custom_scene_start_anim_exec_callback);
    lv_anim_start(&anim);
}

static void custom_scene_start_on_enter(void* context) {
    furi_assert(context);

    CustomApp* instance = context;
    CustomSceneStart* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        widget_set_visible(timer_card_get_base(instance->timer_card), false);
        widget_set_visible(nav_bar_get_base(instance->nav_bar), true);

        data->front_layout = flex_layout_alloc(instance->front_window, FlexLayoutTypeRow);

        data->front_logo = anim_image_alloc(flex_layout_get_base(data->front_layout));
        anim_image_set_source(data->front_logo, CUSTOM_ANIM_PATH("start_logo_custom_41x16.anim"));
        anim_image_set_loop(data->front_logo, false);

        data->front_menu = anim_menu_alloc(flex_layout_get_base(data->front_layout));
        anim_menu_set_callback(data->front_menu, custom_scene_start_menu_callback, instance);
        anim_menu_set_source(
            data->front_menu,
            CUSTOM_ANIM_PATH("start_menu_31x16.anim"),
            ANIM_MENU_IDLE_FRAMES,
            ANIM_MENU_TRANSITION_FRAMES);

        data->back_menu = menu_alloc(instance->back_window);
        menu_add_item(
            data->back_menu, "START", NULL, CUSTOM_IMG_PATH("start_12x12.bin"), 0, NULL, NULL);
        menu_add_item(
            data->back_menu, "SETUP", NULL, CUSTOM_IMG_PATH("setup_12x12.bin"), 0, NULL, NULL);

        if(!data->is_not_first_enter) {
            custom_scene_start_run_in_out_anim(instance, CustomSceneStartInOutAnimTypeEnter);
            data->is_not_first_enter = true;
        }
    });

    custom_start_transition(instance);
}

static void custom_scene_start_on_exit(void* context) {
    furi_assert(context);

    CustomApp* instance = context;
    CustomSceneStart* data = scene_manager_get_current_scene_data(instance->scene_manager);

    with_gui(instance->gui, {
        flex_layout_free(data->front_layout);
        menu_free(data->back_menu);
    });
}

static bool custom_scene_start_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    CustomApp* instance = context;

    bool consumed = false;

    if(event->type == SceneManagerEventTypeCustom) {
        if(event->event == CustomSceneStartMenuIndexStart) {
            with_gui(instance->gui, {
                widget_set_visible(nav_bar_get_base(instance->nav_bar), false);
                widget_set_visible(timer_card_get_base(instance->timer_card), true);

                timer_card_show_header(instance->timer_card, false);
                timer_card_show_time(instance->timer_card, false);
            });

            custom_prepare_transition(instance, CustomTransitionTypeSelect);
            scene_manager_next_scene(instance->scene_manager, CustomAppSceneIdTimer);
        } else if(event->event == CustomSceneStartMenuIndexSetup) {
            custom_push_location(instance, "SETUP");
            scene_manager_next_scene(instance->scene_manager, CustomAppSceneIdSetup);
        } else if(event->event == CustomCustomEventAboutToExit) {
            with_gui(instance->gui, {
                custom_scene_start_run_in_out_anim(instance, CustomSceneStartInOutAnimTypeExit);
            });
        }

        consumed = true;
    }

    return consumed;
}

const Scene custom_scene_start = {
    .enter_callback = custom_scene_start_on_enter,
    .exit_callback = custom_scene_start_on_exit,
    .event_callback = custom_scene_start_on_event,
    .data_size = sizeof(CustomSceneStart),
};
