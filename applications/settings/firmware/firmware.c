#include "firmware_i.h"

#include <settings_helpers/app_desc.h>
#include <settings_helpers/gui_params.h>

#define INPUT_QUEUE_CAPACITY 8
#define EVENT_QUEUE_CAPACITY 8

static bool thread_signal_callback(uint32_t signal, void* argument, void* context) {
    UNUSED(argument);

    furi_assert(context);

    ThisInstance* instance = context;

    switch(signal) {
    case FuriSignalExit:
        furi_event_loop_stop(instance->event_loop);
        return true;

    default:
        return false;
    }
}

static void input_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    ThisInstance* instance = context;

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort && event.key == InputKeyBack) {
            if(!scene_manager_handle_back_event(instance->scene_manager)) {
                desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_APP_NAME);
            }
        }
    }
}

static void event_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    ThisInstance* instance = context;

    uint32_t event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        scene_manager_handle_custom_event(instance->scene_manager, event);
    }
}

static bool gui_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    ThisInstance* instance = context;

    bool is_consumed = false;
    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        is_consumed = true;
    }

    if(is_consumed) furi_message_queue_put(instance->input_queue, event, FuriWaitForever);

    return is_consumed;
}

static ThisInstance* this_alloc(void) {
    ThisInstance* instance = malloc(sizeof(*instance));

    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(INPUT_QUEUE_CAPACITY, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(EVENT_QUEUE_CAPACITY, sizeof(uint32_t));
    instance->scene_manager =
        scene_manager_alloc(settings_firmware_app_scenes, ThisSceneIdxsCount, instance);

    instance->gui = furi_record_open(RECORD_GUI);
    instance->desktop = furi_record_open(RECORD_DESKTOP);
    instance->updater = furi_record_open(RECORD_UPDATER);

    instance->update_info = (UpdateCheckInfo){
        .version = furi_string_alloc(),
        .url = furi_string_alloc(),
        .id = NULL,
        .sha256 = furi_string_alloc(),
        .changelog = NULL,
    };

    instance->result_preset.front_text = furi_string_alloc();
    instance->result_preset.back_primary_text = furi_string_alloc();
    instance->result_preset.back_auxiliary_text = furi_string_alloc();

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, gui_input_callback, instance);

        Widget* front_root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_scene_window = widget_alloc(front_root);

        Widget* back_root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_container = flex_layout_alloc(back_root, FlexLayoutTypeColumn);

        instance->back_nav_bar = nav_bar_alloc(flex_layout_get_base(instance->back_container));
        nav_bar_set_header_image(instance->back_nav_bar, SETTINGS_ICON_BACK);
        nav_bar_push_location(instance->back_nav_bar, "FIRMWARE");
        widget_set_height(nav_bar_get_base(instance->back_nav_bar), SETTINGS_NAV_BAR_HEIGHT);
        widget_set_padding(nav_bar_get_base(instance->back_nav_bar), 2, 2, 0, 0);

        instance->back_scene_window = widget_alloc(flex_layout_get_base(instance->back_container));
        flex_layout_set_child_widget_grow(
            instance->back_container, instance->back_scene_window, 1);
    });

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        event_queue_callback,
        instance);

    UpdaterCheckState updater_check_state;
    furi_state_get(updater_get_check_state(instance->updater), &updater_check_state);

    scene_manager_next_scene(
        instance->scene_manager,
        (updater_check_state.result == UpdaterCheckResultAvailable) ? ThisSceneIdxDialog :
                                                                      ThisSceneIdxMain);

    return instance;
}

static void this_free(ThisInstance* instance) {
    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, gui_input_callback);

        widget_free(instance->front_scene_window);
        flex_layout_free(instance->back_container);
    });

    furi_string_free(instance->result_preset.back_auxiliary_text);
    furi_string_free(instance->result_preset.back_primary_text);
    furi_string_free(instance->result_preset.front_text);

    furi_string_free(instance->update_info.sha256);
    furi_string_free(instance->update_info.url);
    furi_string_free(instance->update_info.version);

    furi_record_close(RECORD_UPDATER);
    furi_record_close(RECORD_DESKTOP);
    furi_record_close(RECORD_GUI);

    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);

    furi_message_queue_free(instance->input_queue);
    furi_message_queue_free(instance->event_queue);

    furi_event_loop_free(instance->event_loop);

    free(instance);
}

static void this_setup_app_descriptor(SettingsAppDescriptor* descriptor) {
    furi_string_set_str(descriptor->front_title, "Firmware");
    furi_string_set_str(descriptor->back_title, "Firmware");
    furi_string_set_str(descriptor->front_icon, THIS_IMG_PATH("microchip_front_8x8.bin"));
    furi_string_set_str(descriptor->back_icon, THIS_IMG_PATH("microchip_back_11x11.bin"));

    Updater* updater = furi_record_open(RECORD_UPDATER);
    UpdaterCheckState updater_check_state;
    furi_state_get(updater_get_check_state(updater), &updater_check_state);
    furi_record_close(RECORD_UPDATER);

    if(updater_check_state.result == UpdaterCheckResultAvailable) {
        furi_string_set_str(descriptor->menu_extra, "New");
    }
}

int32_t settings_firmware_app_entry(void* argument) {
    if(argument) {
        this_setup_app_descriptor(argument);
    } else {
        ThisInstance* instance = this_alloc();

        FuriThread* thread = furi_thread_get_current();
        furi_thread_set_signal_callback(thread, thread_signal_callback, instance);
        furi_event_loop_run(instance->event_loop);

        furi_thread_set_signal_callback(thread, NULL, NULL);

        this_free(instance);
    }

    return 0;
}

void settings_firmware_app_fire_event(ThisInstance* instance, uint32_t event) {
    furi_assert(instance);

    furi_message_queue_put(instance->event_queue, &event, FuriWaitForever);
}
