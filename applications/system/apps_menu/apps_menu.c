#include "apps_menu.h"
#include "apps_menu_i.h"
#include "scenes/apps_menu_scenes.h"
#include "app_list.h"

#include <storage/storage.h>
#include <gui/modules/submenu.h>
#include <js_app_launcher/js_app_launcher.h>

#define TAG "AppsMenu"

#define APPS_MENU_APP_ID          "apps_menu"
#define APPS_MENU_ARG_RESET       "reset"
#define APPS_MENU_ARG_SKIP_MENU   "-s"
#define APPS_MENU_ACTIVE_APP_NONE ""

#define APPS_MENU_JS_APPS_ENABLE_FLAG_PATH APP_DATA_PATH("js_apps_enabled")

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

    AppsMenu* instance = context;

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort) {
            if(event.key == InputKeyBack) {
                scene_manager_handle_back_event(instance->scene_manager);
            }
        }
    }
}

static void apps_menu_event_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    AppsMenu* instance = context;

    uint32_t event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        scene_manager_handle_custom_event(instance->scene_manager, event);
    }
}

static bool apps_menu_gui_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    AppsMenu* instance = context;

    bool consumed = false;
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            consumed = true;
        }
    }

    if(consumed) {
        furi_check(
            furi_message_queue_put(instance->input_queue, event, FuriWaitForever) == FuriStatusOk);
    }

    return consumed;
}

static AppsMenuMode apps_menu_get_mode(const char* arg_str) {
    AppsMenuMode mode = AppsMenuModeResume;

    if(arg_str != NULL) {
        if(strcmp(APPS_MENU_ARG_RESET, arg_str) == 0) {
            mode = AppsMenuModeShowMenu;
        }
    }

    return mode;
}

static bool apps_menu_has_active_application(const AppsMenuSettings* settings) {
    return strnlen(settings->active_application, sizeof(settings->active_application)) > 0;
}

static AppsMenu* apps_menu_alloc(void* arg) {
    FuriThread* thread = furi_thread_get_current();
    const AppsMenuMode mode = apps_menu_get_mode(arg);

    AppsMenuSettings settings;
    apps_menu_settings_load(&settings);

    if(mode == AppsMenuModeShowMenu) {
        apps_menu_set_active_application(&settings, APPS_MENU_ACTIVE_APP_NONE);

    } else if(apps_menu_has_active_application(&settings)) {
        if(apps_menu_start_application(settings.active_application, true)) {
            return NULL;
        }
    }

    AppsMenu* instance = malloc(sizeof(*instance));

    instance->settings = settings;

    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(1, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(1, sizeof(AppsMenuCustomEvent));
    furi_thread_set_signal_callback(thread, apps_menu_thread_signal_callback, instance);

    instance->scene_manager =
        scene_manager_alloc(apps_menu_scenes, COUNT_OF(apps_menu_scenes), instance);
    instance->gui = furi_record_open(RECORD_GUI);
    instance->desktop = furi_record_open(RECORD_DESKTOP);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        apps_menu_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        apps_menu_event_queue_callback,
        instance);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, apps_menu_gui_input_callback, instance);

        Widget* front_root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_scene_window = widget_alloc(front_root);

        Widget* back_root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_container = flex_layout_alloc(back_root, FlexLayoutTypeColumn);
        flex_layout_set_spacing(instance->back_container, 2);

        instance->back_nav_bar = nav_bar_alloc(flex_layout_get_base(instance->back_container));
        nav_bar_set_header_image(
            instance->back_nav_bar, SHARED_IMG_PATH("apps_menu_back_12x12.image"));
        nav_bar_set_header_text(instance->back_nav_bar, "APPS");
        widget_set_height(nav_bar_get_base(instance->back_nav_bar), 14);
        widget_set_padding(nav_bar_get_base(instance->back_nav_bar), 1, 0, 0, 0);
        widget_set_visible(nav_bar_get_base(instance->back_nav_bar), false);

        instance->back_scene_window = widget_alloc(flex_layout_get_base(instance->back_container));
        flex_layout_set_child_widget_grow(
            instance->back_container, instance->back_scene_window, 1);
    });

    if(mode == AppsMenuModeShowMenu) {
        static const uint32_t scenes[] = {AppsMenuSceneIdStart, AppsMenuSceneIdMain};
        scene_manager_next_scenes(instance->scene_manager, scenes, COUNT_OF(scenes));
    } else {
        scene_manager_next_scene(instance->scene_manager, AppsMenuSceneIdStart);
    }

    return instance;
}

static void apps_menu_free(AppsMenu* instance) {
    FuriThread* thread = furi_thread_get_current();
    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, apps_menu_gui_input_callback);

        widget_free(instance->front_scene_window);
        flex_layout_free(instance->back_container);
    });

    furi_record_close(RECORD_DESKTOP);
    furi_record_close(RECORD_GUI);

    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);

    furi_thread_set_signal_callback(thread, NULL, NULL);
    furi_message_queue_free(instance->input_queue);
    furi_message_queue_free(instance->event_queue);
    furi_event_loop_free(instance->event_loop);

    free(instance);
}

void apps_menu_send_custom_event(AppsMenu* app, AppsMenuCustomEvent event) {
    furi_assert(app);
    furi_check(furi_message_queue_put(app->event_queue, &event, FuriWaitForever) == FuriStatusOk);
}

void apps_menu_set_active_application(AppsMenuSettings* settings, const char* app_id) {
    furi_assert(settings);
    furi_assert(app_id);

    strlcpy(settings->active_application, app_id, sizeof(settings->active_application));
    apps_menu_settings_save(settings);
}

bool apps_menu_start_application(const char* app_id, bool is_skip_menu) {
    bool success = false;

    const char* id;
    const char* args;

    if(apps_list_contains(app_id)) {
        id = app_id;
        args = is_skip_menu ? APPS_MENU_ARG_SKIP_MENU : NULL;

    } else if(apps_menu_is_js_apps_enabled()) {
        id = JS_APP_LAUNCHER_APP_ID;
        args = app_id;

    } else {
        id = NULL;
        args = NULL;
    }

    if(id != NULL) {
        Desktop* desktop = furi_record_open(RECORD_DESKTOP);

        if(desktop_replace_current_app(desktop, id, args)) {
            success = true;
        }

        furi_record_close(RECORD_DESKTOP);
    }

    return success;
}

bool apps_menu_is_js_apps_enabled(void) {
    bool is_enabled = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);

    FileInfo file_info;
    if(storage_common_stat(storage, APPS_MENU_JS_APPS_ENABLE_FLAG_PATH, &file_info) == FSE_OK) {
        if((file_info.flags & FSF_DIRECTORY) == 0) {
            is_enabled = true;
        }
    }

    furi_record_close(RECORD_STORAGE);

    return is_enabled;
}

bool apps_menu_start(AppsMenuMode mode) {
    bool success;
    const char* args;

    if(mode == AppsMenuModeResume) {
        args = NULL;
    } else if(mode == AppsMenuModeShowMenu) {
        args = APPS_MENU_ARG_RESET;
    } else {
        furi_crash("Invalid AppsMenuMode value");
    }

    Desktop* desktop = furi_record_open(RECORD_DESKTOP);
    success = desktop_replace_current_app(desktop, APPS_MENU_APP_ID, args);
    furi_record_close(RECORD_DESKTOP);

    return success;
}

int32_t apps_menu_app(void* arg) {
    UNUSED(arg);

    AppsMenu* instance = apps_menu_alloc(arg);

    if(instance) {
        furi_event_loop_run(instance->event_loop);
        apps_menu_free(instance);
    }

    return 0;
}
