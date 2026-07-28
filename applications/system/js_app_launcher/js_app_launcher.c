#include "js_app_launcher_i.h"

#include <storage/storage.h>

#include "scenes/js_app_launcher_scenes.h"

#define INPUT_QUEUE_SIZE (8)
#define EVENT_QUEUE_SIZE (8)

#define NAV_BAR_HEIGHT (14)

static bool js_app_launcher_gui_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    JsAppLauncher* instance = context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            furi_check(
                furi_message_queue_put(instance->input_queue, event, FuriWaitForever) ==
                FuriStatusOk);
            consumed = true;
        }
    }

    return consumed;
}

static void js_app_launcher_input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    JsAppLauncher* instance = context;
    furi_assert(object == instance->input_queue);

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort) {
            if(event.key == InputKeyBack) {
                if(!scene_manager_handle_back_event(instance->scene_manager)) {
                    desktop_replace_current_app(instance->desktop, "apps_menu", "a");
                }
            }
        }
    }
}

static void js_app_launcher_event_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    JsAppLauncher* instance = context;
    furi_assert(object == instance->event_queue);

    uint32_t event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        scene_manager_handle_custom_event(instance->scene_manager, event);
    }
}

static JsAppLauncher* js_app_launcher_alloc(const char* app_id) {
    JsAppLauncher* instance = malloc(sizeof(JsAppLauncher));

    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(INPUT_QUEUE_SIZE, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(EVENT_QUEUE_SIZE, sizeof(uint32_t));
    instance->scene_manager =
        scene_manager_alloc(js_app_launcher_scenes, JsAppLauncherSceneIdMax, instance);
    instance->gui = furi_record_open(RECORD_GUI);
    instance->desktop = furi_record_open(RECORD_DESKTOP);
    instance->app_id = app_id;

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, js_app_launcher_gui_input_callback, instance);

        instance->front_window = widget_alloc(gui_layer_get_root_widget(layer, GuiDisplayIdFront));
        instance->back_container = flex_layout_alloc(
            gui_layer_get_root_widget(layer, GuiDisplayIdBack), FlexLayoutTypeColumn);

        instance->nav_bar = nav_bar_alloc(flex_layout_get_base(instance->back_container));
        widget_set_height(nav_bar_get_base(instance->nav_bar), NAV_BAR_HEIGHT);
        widget_set_margin(nav_bar_get_base(instance->nav_bar), 1, 0, 0, 2);
        nav_bar_set_header_image(instance->nav_bar, SHARED_IMG_PATH("apps_menu_back_12x12.image"));
        flex_layout_set_child_widget_grow(
            instance->back_container, nav_bar_get_base(instance->nav_bar), 0);
        nav_bar_push_location(instance->nav_bar, instance->app_id);

        instance->back_window = widget_alloc(flex_layout_get_base(instance->back_container));
        flex_layout_set_child_widget_grow(instance->back_container, instance->back_window, 1);
    });

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        js_app_launcher_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        js_app_launcher_event_queue_callback,
        instance);

    scene_manager_next_scene(instance->scene_manager, JsAppLauncherSceneIdStart);

    return instance;
}

static void js_app_launcher_free(JsAppLauncher* instance) {
    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, js_app_launcher_gui_input_callback);

        widget_free(instance->front_window);
        flex_layout_free(instance->back_container);
    });

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_DESKTOP);

    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);

    furi_message_queue_free(instance->input_queue);
    furi_message_queue_free(instance->event_queue);

    furi_event_loop_free(instance->event_loop);
    scene_manager_free(instance->scene_manager);

    free(instance);
}

int32_t js_app_launcher_app(void* arg) {
    UNUSED(arg);

    JsAppLauncher* instance = js_app_launcher_alloc(arg);
    furi_event_loop_run(instance->event_loop);
    js_app_launcher_free(instance);

    return 0;
}

void js_app_launcher_send_custom_event(JsAppLauncher* instance, uint32_t event) {
    furi_assert(instance);
    furi_check(
        furi_message_queue_put(instance->event_queue, &event, FuriWaitForever) == FuriStatusOk);
}
