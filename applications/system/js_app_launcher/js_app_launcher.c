#include "js_app_launcher_i.h"

#include <apps_menu/apps_menu.h>
#include <canvas/canvas.h>
#include <storage/storage.h>

#include <js_app/js_app_common.h>
#include <js_app/js_app_registry.h>

#include "scenes/js_app_launcher_scenes.h"

#define INPUT_QUEUE_SIZE (8)
#define EVENT_QUEUE_SIZE (8)
#define API_QUEUE_SIZE   (2)

#define EVENT_QUEUE_TIMEOUT_MS (3000)

#define NAV_BAR_HEIGHT (14)

static bool js_app_launcher_gui_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    JsAppLauncher* instance = context;
    bool consumed = false;

    if((event->type == InputTypeShort) && (event->key == InputKeyBack)) {
        furi_check(
            furi_message_queue_put(instance->input_queue, event, FuriWaitForever) == FuriStatusOk);
        consumed = true;
    }

    return consumed;
}

static void js_app_launcher_input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    JsAppLauncher* instance = context;
    furi_assert(object == instance->input_queue);

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if((event.type == InputTypeShort) && (event.key == InputKeyBack)) {
            if(!scene_manager_handle_back_event(instance->scene_manager)) {
                furi_event_loop_stop(instance->event_loop);
                apps_menu_forget_current_app();
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

static void js_app_launcher_api_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    JsAppLauncher* instance = context;
    furi_assert(object == instance->api_queue);

    JsAppLauncherApiMessage message;
    while(furi_message_queue_get(instance->api_queue, &message, 0) == FuriStatusOk) {
        if(message.type == JsAppLauncherApiMessageTypeStop) {
            const JsAppLauncherStopMode stop_mode = message.stop.mode;
            furi_check(stop_mode < JsAppLauncherStopModeMax);

            if(message.stop.mode == JsAppLauncherStopModeForget) {
                apps_menu_forget_current_app();
            }

            furi_event_loop_stop(instance->event_loop);

        } else {
            furi_crash("Invalid JsAppLauncherApiMessageType value");
        }
    }
}

static void js_app_launcher_init_current_app(JsAppLauncher* instance, const char* app_id) {
    JsAppLauncherStartMode mode = JsAppLauncherStartModeShowMenu;

    do {
        const size_t app_id_len = strlen(app_id);
        if((app_id_len == 0) ||
           (app_id_len > (JS_APP_ID_LEN_MAX + strlen(JS_APP_LAUNCHER_ARG_SKIP_MENU)))) {
            break;
        }

        char app_id_tmp[app_id_len + 1];
        strcpy(app_id_tmp, app_id);

        const size_t flag_idx = app_id_len - strlen(JS_APP_LAUNCHER_ARG_SKIP_MENU);

        if(strcmp(&app_id_tmp[flag_idx], JS_APP_LAUNCHER_ARG_SKIP_MENU) == 0) {
            app_id_tmp[flag_idx] = '\0';
            mode = JsAppLauncherStartModeResume;
        }

        instance->js_app = js_app_registry_get_app(app_id_tmp);

    } while(false);

    instance->mode = mode;
}

static JsAppLauncherError
    js_app_launcher_translate_from_settings_storage_status(JsAppSettingsStorageStatus status) {
    static const JsAppLauncherError status_map[] = {
        [JsAppSettingsStorageStatusOk] = JsAppLauncherErrorNone,
        [JsAppSettingsStorageStatusSchemaMissing] = JsAppLauncherErrorSettingsSchemaMissing,
        [JsAppSettingsStorageStatusSchemaInvalid] = JsAppLauncherErrorSettingsSchemaInvalid,
        [JsAppSettingsStorageStatusStorageFailure] = JsAppLauncherErrorSettingsStorageFailure,
    };

    static_assert(COUNT_OF(status_map) == JsAppSettingsStorageStatusesCount);
    furi_assert(status < JsAppSettingsStorageStatusesCount);

    return status_map[status];
}

static void js_app_launcher_init_settings_storage(JsAppLauncher* instance) {
    JsAppLauncherError error;

    do {
        if(instance->js_app == NULL) {
            error = JsAppLauncherErrorLoadFailed;
            break;
        }

        JsAppInfo app_info;
        if(!js_app_get_info(instance->js_app, &app_info)) {
            error = JsAppLauncherErrorLoadFailed;
            break;
        }

        JsAppSettingsStorageStatus status;
        instance->settings_storage = js_app_settings_storage_alloc(app_info.manifest.id, &status);

        if((instance->settings_storage == NULL) &&
           (status != JsAppSettingsStorageStatusSchemaMissing)) {
            error = js_app_launcher_translate_from_settings_storage_status(status);
            break;
        }

        error = JsAppLauncherErrorNone;
    } while(false);

    instance->error = error;
}

static void js_app_launcher_set_navbar_text(const JsAppLauncher* instance) {
    JsAppInfo info;

    if(js_app_get_info(instance->js_app, &info)) {
        FuriString* tmp = furi_string_alloc_set(info.manifest.name);
        furi_string_to_upper_in_place(tmp);

        nav_bar_push_location(instance->nav_bar, furi_string_get_cstr(tmp));
        furi_string_free(tmp);
    }
}

static void js_app_launcher_init_gui(JsAppLauncher* instance) {
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

        instance->back_window = widget_alloc(flex_layout_get_base(instance->back_container));
        flex_layout_set_child_widget_grow(instance->back_container, instance->back_window, 1);

        if(instance->js_app) {
            js_app_launcher_set_navbar_text(instance);
        }
    });
}

static void js_app_launcher_go_to_next_scene(const JsAppLauncher* instance) {
    uint32_t scene_ids[2];
    size_t scene_ids_count;

    if(instance->error == JsAppLauncherErrorNone) {
        scene_ids[0] = JsAppLauncherSceneIdStart;
        scene_ids_count = 1;

        if(instance->mode == JsAppLauncherStartModeResume) {
            scene_ids[1] = JsAppLauncherSceneIdRun;
            scene_ids_count = 2;
        }

    } else {
        scene_ids[0] = JsAppLauncherSceneIdError;
        scene_ids_count = 1;
    }

    scene_manager_next_scenes(instance->scene_manager, scene_ids, scene_ids_count);
}

static JsAppLauncher* js_app_launcher_alloc(const char* app_id) {
    JsAppLauncher* instance = malloc(sizeof(JsAppLauncher));

    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(INPUT_QUEUE_SIZE, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(EVENT_QUEUE_SIZE, sizeof(uint32_t));
    instance->api_queue =
        furi_message_queue_alloc(API_QUEUE_SIZE, sizeof(JsAppLauncherApiMessage));
    instance->scene_manager =
        scene_manager_alloc(js_app_launcher_scenes, JsAppLauncherSceneIdMax, instance);
    instance->gui = furi_record_open(RECORD_GUI);

    furi_record_create(RECORD_JS_APP_LAUNCHER, instance);

    js_app_launcher_init_current_app(instance, app_id);
    js_app_launcher_init_settings_storage(instance);
    js_app_launcher_init_gui(instance);

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

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->api_queue,
        FuriEventLoopEventIn,
        js_app_launcher_api_queue_callback,
        instance);

    js_app_launcher_go_to_next_scene(instance);

    return instance;
}

static void js_app_launcher_clear_canvas(JsAppLauncher* instance) {
    CanvasSrv* canvas = furi_record_open(RECORD_CANVAS);

    JsAppInfo info;
    if(js_app_get_info(instance->js_app, &info)) {
        const char* app_id = info.manifest.id;
        const CanvasResult result = canvas_delete_elements(canvas, app_id, NULL);
        if((result != CanvasResultOk) && (result != CanvasResultEmptyScreen)) {
            FURI_LOG_W(TAG, "Failed to clear canvas: %d", result);
        }
    }

    furi_record_close(RECORD_CANVAS);
}

static void js_app_launcher_free(JsAppLauncher* instance) {
    furi_record_destroy(RECORD_JS_APP_LAUNCHER);
    // TODO [FW-602]: scene_manager_free() MUST be called before
    //      all other free()s to avoid use-after-free.
    scene_manager_free(instance->scene_manager);

    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->api_queue);

    furi_message_queue_free(instance->input_queue);
    furi_message_queue_free(instance->event_queue);
    furi_message_queue_free(instance->api_queue);

    furi_event_loop_free(instance->event_loop);

    if(instance->settings_storage) {
        js_app_settings_storage_free(instance->settings_storage);
    }

    if(instance->js_app) {
        js_app_launcher_clear_canvas(instance);
        js_app_free(instance->js_app);
    }

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, js_app_launcher_gui_input_callback);

        widget_free(instance->front_window);
        flex_layout_free(instance->back_container);
    });

    furi_record_close(RECORD_GUI);

    free(instance);
}

int32_t js_app_launcher_app(void* arg) {
    furi_assert(arg);

    JsAppLauncher* instance = js_app_launcher_alloc(arg);
    furi_event_loop_run(instance->event_loop);
    js_app_launcher_free(instance);

    return 0;
}

void js_app_launcher_send_custom_event(JsAppLauncher* instance, uint32_t event) {
    furi_assert(instance);

    FuriStatus queue_status =
        furi_message_queue_put(instance->event_queue, &event, EVENT_QUEUE_TIMEOUT_MS);

    if(queue_status != FuriStatusOk) {
        FURI_LOG_E(TAG, "Failed to put an item into event queue.");
    }
}
