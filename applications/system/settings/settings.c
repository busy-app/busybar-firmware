#include "settings.h"
#include "storage_macros.h"
#include "scenes/settings_scenes.h"

#define SETTINGS_NAV_BAR_HEIGHT 20

static bool settings_thread_signal_callback(uint32_t signal, void* arg, void* context) {
    UNUSED(arg);

    SettingsApp* instance = context;

    switch(signal) {
    case FuriSignalExit:
        furi_event_loop_stop(instance->event_loop);
        return true;

    case FuriSignalAboutToExit:
        settings_send_custom_event(instance, SettingsCustomEventAboutToExit);
        return true;

    default:
        return false;
    }
}

static void settings_input_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    SettingsApp* instance = context;

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort) {
            if(event.key == InputKeyBack) {
                scene_manager_handle_back_event(instance->scene_manager);
            }
        }
    }
}

static void settings_event_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    SettingsApp* instance = context;

    uint32_t event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        scene_manager_handle_custom_event(instance->scene_manager, event);
    }
}

static bool settings_gui_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    SettingsApp* instance = context;

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

static SettingsApp* settings_alloc(void) {
    SettingsApp* instance = malloc(sizeof(SettingsApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(8, sizeof(uint32_t));
    instance->scene_manager =
        scene_manager_alloc(settings_scenes, COUNT_OF(settings_scenes), instance);

    instance->l10n_service = furi_record_open(RECORD_L10N);
    instance->l10n =
        l10n_context_open(instance->l10n_service, SETTINGS_ASSETS_PATH("l10n"), L10nSourceStorage);

    instance->gui = furi_record_open(RECORD_GUI);
    instance->audio = furi_record_open(RECORD_AUDIO);
    instance->front_display = furi_record_open(RECORD_FRONT_DISPLAY);
    instance->back_display = furi_record_open(RECORD_BACK_DISPLAY);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, settings_gui_input_callback, instance);

        Widget* front_root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_scene_window = widget_alloc(front_root);

        Widget* back_root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_container = flex_layout_alloc(back_root, FlexLayoutTypeColumn);
        flex_layout_set_spacing(instance->back_container, 2);

        instance->back_nav_bar = nav_bar_alloc(flex_layout_get_base(instance->back_container));
        nav_bar_set_header_image(
            instance->back_nav_bar, SETTINGS_IMG_PATH("settings_back_7x7.bin"));
        nav_bar_set_header_text(
            instance->back_nav_bar, l10n_get(instance->l10n, L10N_KEY_SETTINGS_TITLE_NAVBAR));
        widget_set_height(nav_bar_get_base(instance->back_nav_bar), SETTINGS_NAV_BAR_HEIGHT);
        widget_set_padding(nav_bar_get_base(instance->back_nav_bar), 6, 6, 0, 0);

        instance->back_scene_window = widget_alloc(flex_layout_get_base(instance->back_container));
        flex_layout_set_child_widget_grow(
            instance->back_container, instance->back_scene_window, 1);
    });

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        settings_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        settings_event_queue_callback,
        instance);

    scene_manager_next_scene(instance->scene_manager, SettingsAppSceneIdStart);

    return instance;
}

static void settings_free(SettingsApp* instance) {
    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, settings_gui_input_callback);

        widget_free(instance->front_scene_window);
        flex_layout_free(instance->back_container);
    });

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_AUDIO);
    furi_record_close(RECORD_FRONT_DISPLAY);
    furi_record_close(RECORD_BACK_DISPLAY);

    l10n_context_close(instance->l10n);
    furi_record_close(RECORD_L10N);

    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_message_queue_free(instance->input_queue);
    furi_message_queue_free(instance->event_queue);
    furi_event_loop_free(instance->event_loop);

    free(instance);
}

int32_t settings_app(void* arg) {
    UNUSED(arg);

    SettingsApp* instance = settings_alloc();
    FuriThread* thread = furi_thread_get_current();
    furi_thread_set_signal_callback(thread, settings_thread_signal_callback, instance);
    furi_event_loop_run(instance->event_loop);
    furi_thread_set_signal_callback(thread, NULL, NULL);
    settings_free(instance);

    return 0;
}

void settings_send_custom_event(SettingsApp* instance, uint32_t event) {
    furi_assert(instance);

    furi_check(
        furi_message_queue_put(instance->event_queue, &event, FuriWaitForever) == FuriStatusOk);
}

void settings_push_location(SettingsApp* instance, const char* location_name) {
    furi_assert(instance);
    furi_assert(location_name);

    with_gui(instance->gui, { nav_bar_push_location(instance->back_nav_bar, location_name); });
}

void settings_pop_location(SettingsApp* instance) {
    furi_assert(instance);

    with_gui(instance->gui, { nav_bar_pop_location(instance->back_nav_bar); });
}
