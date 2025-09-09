#include "apps_menu_i.h"
#include "storage_macros.h"
#include "scenes/apps_menu_scenes.h"

#include <applications.h>

#include <gui/modules/submenu.h>

#define TAG "AppsMenu"

#define APPS_MENU_NAV_BAR_HEIGHT 20

static bool apps_menu_thread_signal_callback(uint32_t signal, void* arg, void* context) {
    UNUSED(arg);

    AppsMenu* instance = context;

    switch(signal) {
    case FuriSignalExit:
        furi_event_loop_stop(instance->event_loop);
        return true;

    case FuriSignalAboutToExit:
        apps_menu_send_custom_event(instance, AppsMenuCustomEventAboutToExit);
        return true;

    default:
        return false;
    }
}

static void apps_menu_input_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    AppsMenu* app = context;

    InputEvent event;
    while(furi_message_queue_get(app->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort) {
            if(event.key == InputKeyBack) {
                AppsMenuSceneId current_scene_id =
                    scene_manager_get_current_scene_id(app->scene_manager);

                if(current_scene_id != AppsMenuSceneIdStart) {
                    scene_manager_handle_back_event(app->scene_manager);
                }
            }
        }
    }
}

static void apps_menu_event_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    AppsMenu* app = context;

    uint32_t event;
    while(furi_message_queue_get(app->event_queue, &event, 0) == FuriStatusOk) {
        scene_manager_handle_custom_event(app->scene_manager, event);
    }
}

static bool apps_menu_gui_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    AppsMenu* app = context;

    bool consumed = false;
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            consumed = true;
        }
    }

    if(consumed) {
        furi_check(
            furi_message_queue_put(app->input_queue, event, FuriWaitForever) == FuriStatusOk);
    }

    return consumed;
}

static AppsMenu* apps_menu_alloc(void) {
    AppsMenu* app = malloc(sizeof(AppsMenu));

    app->event_loop = furi_event_loop_alloc();
    app->input_queue = furi_message_queue_alloc(1, sizeof(InputEvent));
    app->event_queue = furi_message_queue_alloc(1, sizeof(AppsMenuCustomEvent));
    app->scene_manager = scene_manager_alloc(apps_menu_scenes, COUNT_OF(apps_menu_scenes), app);
    app->gui = furi_record_open(RECORD_GUI);
    app->desktop = furi_record_open(RECORD_DESKTOP);

    furi_event_loop_subscribe_message_queue(
        app->event_loop,
        app->input_queue,
        FuriEventLoopEventIn,
        apps_menu_input_queue_callback,
        app);

    furi_event_loop_subscribe_message_queue(
        app->event_loop,
        app->event_queue,
        FuriEventLoopEventIn,
        apps_menu_event_queue_callback,
        app);

    with_gui(app->gui, {
        GuiLayer* layer = gui_get_layer(app->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, apps_menu_gui_input_callback, app);

        Widget* front_root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        app->front_scene_window = widget_alloc(front_root);

        Widget* back_root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        app->back_container = flex_layout_alloc(back_root, FlexLayoutTypeColumn);
        flex_layout_set_spacing(app->back_container, 2);

        app->back_nav_bar = nav_bar_alloc(flex_layout_get_base(app->back_container));
        nav_bar_set_header_image(app->back_nav_bar, APPS_MENU_IMG_PATH("apps_menu_back_7x7.bin"));
        nav_bar_set_header_text(app->back_nav_bar, "APPS");
        widget_set_height(nav_bar_get_base(app->back_nav_bar), APPS_MENU_NAV_BAR_HEIGHT);
        widget_set_padding(nav_bar_get_base(app->back_nav_bar), 6, 6, 0, 0);

        app->back_scene_window = widget_alloc(flex_layout_get_base(app->back_container));
        flex_layout_set_child_widget_grow(app->back_container, app->back_scene_window, 1);
    });

    scene_manager_next_scene(app->scene_manager, AppsMenuSceneIdStart);

    return app;
}

static void apps_menu_free(AppsMenu* app) {
    scene_manager_free(app->scene_manager);

    with_gui(app->gui, {
        GuiLayer* layer = gui_get_layer(app->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, apps_menu_gui_input_callback);

        widget_free(app->front_scene_window);
        flex_layout_free(app->back_container);
    });

    furi_record_close(RECORD_DESKTOP);
    furi_record_close(RECORD_GUI);

    furi_event_loop_unsubscribe(app->event_loop, app->input_queue);
    furi_event_loop_unsubscribe(app->event_loop, app->event_queue);
    furi_message_queue_free(app->input_queue);
    furi_message_queue_free(app->event_queue);
    furi_event_loop_free(app->event_loop);

    free(app);
}

void apps_menu_send_custom_event(AppsMenu* app, AppsMenuCustomEvent event) {
    furi_assert(app);
    furi_check(furi_message_queue_put(app->event_queue, &event, FuriWaitForever) == FuriStatusOk);
}

int32_t apps_menu_app(void* arg) {
    UNUSED(arg);

    AppsMenu* app = apps_menu_alloc();
    FuriThread* thread = furi_thread_get_current();
    furi_thread_set_signal_callback(thread, apps_menu_thread_signal_callback, app);
    furi_event_loop_run(app->event_loop);
    furi_thread_set_signal_callback(thread, NULL, NULL);
    apps_menu_free(app);

    return 0;
}
