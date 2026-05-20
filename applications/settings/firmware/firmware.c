#include "firmware_i.h"

#include <settings_helpers/app_desc.h>
#include <settings_helpers/gui_params.h>

#define INPUT_QUEUE_CAPACITY   8
#define INPUT_QUEUE_TIMEOUT_MS 3000

#define EVENT_QUEUE_CAPACITY   8
#define EVENT_QUEUE_TIMEOUT_MS 3000

static bool firmware_settings_input_callback(const InputEvent* event, void* context) {
    FirmwareSettings* instance = context;

    bool is_consumed = false;
    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        is_consumed = true;
    }

    if(is_consumed) {
        FuriStatus input_queue_status = furi_message_queue_put(
            instance->input_queue, event, furi_ms_to_ticks(INPUT_QUEUE_TIMEOUT_MS));

        if(input_queue_status != FuriStatusOk) {
            FURI_LOG_E(TAG, "Failed to put an item into input queue.");
        }
    }

    return is_consumed;
}

static void firmware_settings_input_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    FirmwareSettings* instance = context;

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort && event.key == InputKeyBack) {
            if(!scene_manager_handle_back_event(instance->scene_manager)) {
                desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_APP_NAME);
            }
        }
    }
}

static void firmware_settings_event_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    FirmwareSettings* instance = context;

    uint32_t event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        scene_manager_handle_custom_event(instance->scene_manager, event);
    }
}

static FirmwareSettings* firmware_settings_alloc(void) {
    FirmwareSettings* instance = malloc(sizeof(*instance));

    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(INPUT_QUEUE_CAPACITY, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(EVENT_QUEUE_CAPACITY, sizeof(uint32_t));
    instance->scene_manager = scene_manager_alloc(
        firmware_settings_internal_scenes, FirmwareSettingsSceneIdxsCount, instance);

    instance->gui = furi_record_open(RECORD_GUI);
    instance->desktop = furi_record_open(RECORD_DESKTOP);
    instance->updater = furi_record_open(RECORD_UPDATER);
    instance->power = furi_record_open(RECORD_POWER);

    instance->update_info = (UpdateCheckInfo){
        .version = furi_string_alloc(),
        .url = furi_string_alloc(),
        .id = NULL,
        .sha256 = furi_string_alloc(),
        .changelog = NULL,
    };

    instance->check_result_preset.front_text = furi_string_alloc();
    instance->check_result_preset.back_primary_text = furi_string_alloc();
    instance->check_result_preset.back_detail_text = furi_string_alloc();

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, firmware_settings_input_callback, instance);

        Widget* front_root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_scene_window = widget_alloc(front_root);

        Widget* back_root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_container = flex_layout_alloc(back_root, FlexLayoutTypeColumn);

        instance->back_nav_bar = nav_bar_alloc(flex_layout_get_base(instance->back_container));
        nav_bar_set_header_image(instance->back_nav_bar, SETTINGS_ICON_BACK);
        nav_bar_push_location(instance->back_nav_bar, "FIRMWARE");
        widget_set_height(nav_bar_get_base(instance->back_nav_bar), SETTINGS_NAV_BAR_HEIGHT);
        widget_set_padding(nav_bar_get_base(instance->back_nav_bar), 1, 0, 0, 2);

        instance->back_scene_window = widget_alloc(flex_layout_get_base(instance->back_container));
        flex_layout_set_child_widget_grow(
            instance->back_container, instance->back_scene_window, 1);
    });

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        firmware_settings_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        firmware_settings_event_queue_callback,
        instance);

    UpdaterCheckState updater_check_state;
    furi_state_get(updater_get_check_state(instance->updater), &updater_check_state);

    scene_manager_next_scene(
        instance->scene_manager,
        (updater_check_state.result == UpdaterCheckResultAvailable) ?
            FirmwareSettingsSceneIdxDialog :
            FirmwareSettingsSceneIdxMain);

    return instance;
}

static void firmware_settings_free(FirmwareSettings* instance) {
    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, firmware_settings_input_callback);

        widget_free(instance->front_scene_window);
        flex_layout_free(instance->back_container);
    });

    furi_string_free(instance->check_result_preset.back_detail_text);
    furi_string_free(instance->check_result_preset.back_primary_text);
    furi_string_free(instance->check_result_preset.front_text);

    furi_string_free(instance->update_info.sha256);
    furi_string_free(instance->update_info.url);
    furi_string_free(instance->update_info.version);

    furi_record_close(RECORD_POWER);
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

static void firmware_settings_setup_descriptor(SettingsAppDescriptor* descriptor) {
    furi_string_set_str(descriptor->front_title, "Firmware");
    furi_string_set_str(descriptor->back_title, "Firmware");
    furi_string_set_str(descriptor->front_icon, THIS_IMG_PATH("microchip_front_8x8.image"));
    furi_string_set_str(descriptor->back_icon, THIS_IMG_PATH("microchip_back_11x11.image"));

    Updater* updater = furi_record_open(RECORD_UPDATER);
    UpdaterCheckState updater_check_state;
    furi_state_get(updater_get_check_state(updater), &updater_check_state);
    furi_record_close(RECORD_UPDATER);

    if(updater_check_state.result == UpdaterCheckResultAvailable) {
        furi_string_set_str(descriptor->menu_extra, "New");
    }
}

int32_t firmware_settings_entry(void* argument) {
    if(argument) {
        firmware_settings_setup_descriptor(argument);
    } else {
        FirmwareSettings* instance = firmware_settings_alloc();

        furi_event_loop_run(instance->event_loop);

        firmware_settings_free(instance);
    }

    return 0;
}

void firmware_settings_internal_fire_event(FirmwareSettings* instance, uint32_t event) {
    FuriStatus event_queue_status = furi_message_queue_put(
        instance->event_queue, &event, furi_ms_to_ticks(EVENT_QUEUE_TIMEOUT_MS));

    if(event_queue_status != FuriStatusOk) {
        FURI_LOG_E(TAG, "Failed to put an item into event queue.");
    }
}
