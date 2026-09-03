#include "clock_i.h"

#include <settings_helpers/gui_params.h>

#include <furi.h>
#include <cli/args.h>
#include <apps_menu/apps_menu.h>

#define TAG "clock"

#define INPUT_QUEUE_CAPACITY   8
#define INPUT_QUEUE_TIMEOUT_MS 3000

#define EVENT_QUEUE_CAPACITY   8
#define EVENT_QUEUE_TIMEOUT_MS 3000

#define TIMER_INTERVAL_MS 100

static const ClockArguments clock_default_arguments = {
    .do_skip_menu = false,
};

static void clock_parse_arguments(const char* string, ClockArguments* arguments) {
    *arguments = clock_default_arguments;

    if(!string) return;

    FuriString* _string = furi_string_alloc_set(string);
    FuriString* argument = furi_string_alloc();
    while(args_read_string_and_trim(_string, argument)) {
        if(furi_string_equal_str(argument, "-s") ||
           furi_string_equal_str(argument, "--skip-menu")) {
            arguments->do_skip_menu = true;
        } else {
            FURI_LOG_W(TAG, "Unknown argument: %s.", furi_string_get_cstr(argument));
        }
    }

    furi_string_free(argument);
}

static void clock_input_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    Clock* instance = context;

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort && event.key == InputKeyBack) {
            if(!scene_manager_handle_back_event(instance->scene_manager)) {
                if(!apps_menu_start(AppsMenuModeShowMenu)) {
                    FURI_LOG_E(TAG, "Failed to exit to apps menu");
                }
            }
        }
    }
}

static void clock_event_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    Clock* instance = context;

    uint32_t event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        scene_manager_handle_custom_event(instance->scene_manager, event);
    }
}

static bool clock_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    Clock* instance = context;

    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        FuriStatus queue_status =
            furi_message_queue_put(instance->input_queue, event, INPUT_QUEUE_TIMEOUT_MS);

        if(queue_status != FuriStatusOk) {
            FURI_LOG_E(TAG, "Failed to put an item into input queue.");
        }

        return true;
    }

    return false;
}

static void clock_timer_callback(void* context) {
    Clock* instance = context;

    clock_internal_fire_event(instance, ClockEventTimerUpdate);
}

static Clock* clock_alloc(const char* arguments) {
    Clock* instance = malloc(sizeof(*instance));

    clock_parse_arguments(arguments, &instance->arguments);

    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(INPUT_QUEUE_CAPACITY, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(EVENT_QUEUE_CAPACITY, sizeof(uint32_t));
    instance->scene_manager =
        scene_manager_alloc(clock_internal_scenes, ClockSceneIdxsCount, instance);

    instance->gui = furi_record_open(RECORD_GUI);
    instance->time = furi_record_open(RECORD_TIME);
    instance->updater = furi_record_open(RECORD_UPDATER);

    instance->timer = furi_event_loop_timer_alloc(
        instance->event_loop, clock_timer_callback, FuriEventLoopTimerTypePeriodic, instance);

    clock_settings_load(&instance->settings);

    updater_pause_autoupdates(instance->updater);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, clock_input_callback, instance);

        Widget* front_root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_scene_window = widget_alloc(front_root);

        instance->front_transition_overlay = transition_overlay_alloc(front_root);
        transition_overlay_set_pressed_widget(
            instance->front_transition_overlay, instance->front_scene_window);

        Widget* back_root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_container = flex_layout_alloc(back_root, FlexLayoutTypeColumn);

        instance->back_nav_bar = nav_bar_alloc(flex_layout_get_base(instance->back_container));
        nav_bar_set_header_image(
            instance->back_nav_bar, SHARED_IMG_PATH("apps_menu_back_12x12.image"));
        nav_bar_push_location(instance->back_nav_bar, "CLOCK");
        widget_set_height(nav_bar_get_base(instance->back_nav_bar), 14);
        widget_set_margin(nav_bar_get_base(instance->back_nav_bar), 1, 0, 0, 2);

        instance->back_scene_window = widget_alloc(flex_layout_get_base(instance->back_container));
        flex_layout_set_child_widget_grow(
            instance->back_container, instance->back_scene_window, 1);
    });

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        clock_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        clock_event_queue_callback,
        instance);

    furi_event_loop_timer_start(instance->timer, furi_ms_to_ticks(TIMER_INTERVAL_MS));

    if(instance->arguments.do_skip_menu) {
        with_gui(instance->gui, {
            widget_set_visible(nav_bar_get_base(instance->back_nav_bar), false);
        });

        static const uint32_t scenes[] = {ClockSceneIdxMain, ClockSceneIdxClock};
        scene_manager_next_scenes(instance->scene_manager, scenes, COUNT_OF(scenes));
    } else {
        scene_manager_next_scene(instance->scene_manager, ClockSceneIdxMain);
    }

    return instance;
}

static void clock_free(Clock* instance) {
    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, clock_input_callback);

        transition_overlay_free(instance->front_transition_overlay);

        widget_free(instance->front_scene_window);
        flex_layout_free(instance->back_container);
    });

    updater_resume_autoupdates(instance->updater);

    furi_record_close(RECORD_UPDATER);
    furi_record_close(RECORD_TIME);
    furi_record_close(RECORD_GUI);

    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);

    furi_message_queue_free(instance->input_queue);
    furi_message_queue_free(instance->event_queue);

    furi_event_loop_timer_free(instance->timer);
    furi_event_loop_free(instance->event_loop);

    free(instance);
}

int32_t clock_entry(void* argument) {
    Clock* instance = clock_alloc(argument);

    furi_event_loop_run(instance->event_loop);

    clock_free(instance);

    return 0;
}

void clock_internal_fire_event(Clock* instance, uint32_t event) {
    furi_assert(instance);

    FuriStatus queue_status =
        furi_message_queue_put(instance->event_queue, &event, EVENT_QUEUE_TIMEOUT_MS);

    if(queue_status != FuriStatusOk) {
        FURI_LOG_E(TAG, "Failed to put an item into event queue.");
    }
}
