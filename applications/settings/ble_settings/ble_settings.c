#include "ble_settings.h"
#include <settings_helpers/app_desc.h>
#include <settings_helpers/gui_params.h>

#define TAG "BleSettings"

static void ble_settings_input_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    BleSettings* instance = context;

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort) {
            if(event.key == InputKeyBack) {
                scene_manager_handle_back_event(instance->scene_manager);
            }
        }
    }
}

static void ble_settings_event_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    BleSettings* instance = context;

    uint32_t event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        scene_manager_handle_custom_event(instance->scene_manager, event);
    }
}

static bool ble_settings_gui_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    BleSettings* instance = context;

    bool consumed = false;
    if(event->type == InputTypeShort) {
        consumed = true;
    }

    if(consumed) {
        furi_check(
            furi_message_queue_put(instance->input_queue, event, FuriWaitForever) == FuriStatusOk);
    }

    return consumed;
}

static BleSettings* ble_settings_alloc() {
    BleSettings* instance = malloc(sizeof(BleSettings));
    instance->ble = furi_record_open(RECORD_BLE);
    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(4, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(4, sizeof(uint32_t));
    instance->scene_manager =
        scene_manager_alloc(ble_settings_scenes, COUNT_OF(ble_settings_scenes), instance);

    instance->status_lights = furi_record_open(RECORD_STATUS_LIGHTS);
    instance->desktop = furi_record_open(RECORD_DESKTOP);
    instance->gui = furi_record_open(RECORD_GUI);
    instance->front_display = furi_record_open(RECORD_FRONT_DISPLAY);
    instance->back_display = furi_record_open(RECORD_BACK_DISPLAY);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, ble_settings_gui_input_callback, instance);

        Widget* front_root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_scene_window = widget_alloc(front_root);

        Widget* back_root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_container = flex_layout_alloc(back_root, FlexLayoutTypeColumn);

        instance->back_nav_bar = nav_bar_alloc(flex_layout_get_base(instance->back_container));
        nav_bar_set_header_image(instance->back_nav_bar, SETTINGS_ICON_BACK);
        nav_bar_push_location(instance->back_nav_bar, "Bluetooth");
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
        ble_settings_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        ble_settings_event_queue_callback,
        instance);

    BleStatus status = {0};
    bool result = ble_get_status(instance->ble, &status);
    furi_check(result);
    const bool not_paired = status.pairing == BlePairingStateNotPaired;
    scene_manager_next_scene(
        instance->scene_manager, not_paired ? SceneIdPairingMode : SceneIdForgetDevice);
    return instance;
}

static void ble_settings_free(BleSettings* instance) {
    furi_assert(instance);
    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, ble_settings_gui_input_callback);

        widget_free(instance->front_scene_window);
        widget_free(instance->back_scene_window);
        flex_layout_free(instance->back_container);
    });

    furi_record_close(RECORD_DESKTOP);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_FRONT_DISPLAY);
    furi_record_close(RECORD_BACK_DISPLAY);
    furi_record_close(RECORD_STATUS_LIGHTS);
    furi_record_close(RECORD_BLE);

    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_message_queue_free(instance->input_queue);
    furi_message_queue_free(instance->event_queue);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

static void ble_settings_set_icon_by_status(
    SettingsAppDescriptor* const descriptor,
    const BleStatus* const status) {
    struct {
        const char* front;
        const char* back;
    } icon;

    if(status->state == BleServiceStateReady) {
        icon.front = "ble_front_gray_8x8.bin";
        icon.back = "ble_back_12x12.bin";
    } else if(status->state == BleServiceStateAdvertising) {
        const bool paired = status->pairing == BlePairingStatePaired;
        icon.front = paired ? "ble_front_paired_8x8.bin" : "ble_front_8x8.bin";
        icon.back = paired ? "ble_back_paired_12x12.bin" : "ble_back_12x12.bin";
    } else if(status->state == BleServiceStateConnected) {
        icon.front = "ble_front_checkmark_8x8.bin";
        icon.back = "ble_back_paired_12x12.bin";
    } else {
        FURI_LOG_W(TAG, "Wrong state!");
        icon.front = "ble_front_gray_8x8.bin";
        icon.back = "ble_back_12x12.bin";
    }

    furi_string_printf(descriptor->front_icon, IMG_PATH("%s"), icon.front);
    furi_string_printf(descriptor->back_icon, IMG_PATH("%s"), icon.back);
}

int32_t ble_settings_entry(void* arg) {
    if(arg) {
        SettingsAppDescriptor* descriptor = arg;
        furi_string_set_str(descriptor->front_title, "Bluetooth");
        furi_string_set_str(descriptor->back_title, "Bluetooth");

        Ble* ble = furi_record_open(RECORD_BLE);

        BleStatus status = {0};
        bool result = ble_get_status(ble, &status);
        furi_check(result);
        ///TODO: rework this part when proper init sequence will be done for BLE
        if(status.state == BleServiceStateReset) {
            ble_stop(ble);
            result = ble_get_status(ble, &status);
            furi_check(result);
        }

        ble_settings_set_icon_by_status(descriptor, &status);
        furi_record_close(RECORD_BLE);

        return 0;
    }

    BleSettings* instance = ble_settings_alloc();
    furi_event_loop_run(instance->event_loop);
    ble_settings_free(instance);

    return 0;
}

void ble_settings_send_custom_event(BleSettings* instance, uint32_t event) {
    furi_assert(instance);

    furi_check(
        furi_message_queue_put(instance->event_queue, &event, FuriWaitForever) == FuriStatusOk);
}
