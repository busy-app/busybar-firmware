#include "status_bar.h"

#include "widgets/ble_status_indicator.h"
#include "widgets/wifi_status_indicator.h"
#include "widgets/audio_status_indicator.h"
#include "widgets/usb_status_indicator.h"
#include "widgets/battery_status_indicator.h"

#include <gui/gui_i.h>
#include <gui/modules/image.h>
#include <gui/modules/flex_layout.h>

#include <power/power_service/power.h>
#include <audio/audio.h>

#define TAG "StatusBar"

struct StatusBar {
    FuriEventLoop* event_loop;
    Gui* gui;
    Power* power;
    Audio* audio;
    BleStatusIndicator* ble_status_indicator;
    WifiStatusIndicator* wifi_status_indicator;
    AudioStatusIndicator* audio_status_indicator;
    UsbStatusIndicator* usb_status_indicator;
    BatteryStatusIndicator* battery_status_indicator;
};

typedef enum {
    StatusBarUpdateEventPowerChargingState = 1 << 0,
    StatusBarUpdateEventPowerChargeAmount = 1 << 1,

    StatusBarUpdateEventAudioVolume = 1 << 2,

    StatusBarUpdateEventUsbConnectionState = 1 << 3,

    StatusBarUpdateEventAnyPower = StatusBarUpdateEventPowerChargingState |
                                   StatusBarUpdateEventPowerChargeAmount
} StatusBarUpdateEvent;

static void power_events_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    PowerEvent* event = (PowerEvent*)message;
    StatusBar* instance = context;
    StatusBarUpdateEvent update_event;

    switch(event->type) {
    case PowerEventChargingStateUpdate:
        update_event = StatusBarUpdateEventPowerChargingState;
        break;

    case PowerEventChargeAmountUpdate:
        update_event = StatusBarUpdateEventPowerChargeAmount;
        break;

    case PowerEventUsbConnectionStateUpdate:
        update_event = StatusBarUpdateEventUsbConnectionState;
        break;

    default:
        return;
    }

    furi_event_loop_set_custom_event(instance->event_loop, update_event);
}

static void audio_events_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    AudioEvent* event = (AudioEvent*)message;
    StatusBar* instance = context;

    if(event->type == AudioEventVolumeUpdate) {
        furi_event_loop_set_custom_event(instance->event_loop, StatusBarUpdateEventAudioVolume);
    }
}

static void status_bar_custom_event_callback(uint32_t events, void* context) {
    StatusBar* instance = context;
    PowerInfo* power_info = NULL;
    bool is_usb_connected;
    float audio_volume;

    if(READ_BIT(events, StatusBarUpdateEventAnyPower)) {
        power_info = alloca(sizeof(*power_info));
        power_get_info(instance->power, power_info);
    }

    if(READ_BIT(events, StatusBarUpdateEventAudioVolume)) {
        audio_volume = audio_get_volume(instance->audio);
    }

    if(READ_BIT(events, StatusBarUpdateEventUsbConnectionState)) {
        is_usb_connected = power_is_usb_connected(instance->power);
    }

    with_gui(instance->gui, {
        if(READ_BIT(events, StatusBarUpdateEventPowerChargingState)) {
            battery_status_indicator_set_charging_state(
                instance->battery_status_indicator, power_info->is_charging);
        }

        if(READ_BIT(events, StatusBarUpdateEventPowerChargeAmount)) {
            battery_status_indicator_set_charge_amount(
                instance->battery_status_indicator,
                (power_info->is_full_charged) ? BATTERY_STATUS_INDICATOR_MAX_CHARGE_AMOUNT :
                                                power_info->charge);
        }

        if(READ_BIT(events, StatusBarUpdateEventUsbConnectionState)) {
            usb_status_indicator_set_connection_state(
                instance->usb_status_indicator, is_usb_connected);
        }

        if(READ_BIT(events, StatusBarUpdateEventAudioVolume)) {
            audio_status_indicator_set_volume(instance->audio_status_indicator, audio_volume);
        }
    })
}

static StatusBar* status_bar_alloc(void) {
    StatusBar* instance = malloc(sizeof(*instance));

    instance->event_loop = furi_event_loop_alloc();
    instance->gui = furi_record_open(RECORD_GUI);
    instance->power = furi_record_open(RECORD_POWER);
    instance->audio = furi_record_open(RECORD_AUDIO);

    PowerInfo power_info;
    power_get_info(instance->power, &power_info);

    bool is_usb_connected = power_is_usb_connected(instance->power);
    float audio_volume = audio_get_volume(instance->audio);

    with_gui(instance->gui, {
        GuiLayer* system_layer = gui_get_layer(instance->gui, GuiLayerIdSystem);
        Widget* root = gui_layer_get_root_widget(system_layer, GuiDisplayIdBack);
        FlexLayout* status_bar = flex_layout_alloc(root, FlexLayoutTypeColumn);
        widget_set_align(flex_layout_get_base(status_bar), AlignRightMid);
        widget_set_width(flex_layout_get_base(status_bar), BACK_STATUS_BAR_WIDTH);
        widget_set_padding(flex_layout_get_base(status_bar), 0, 0, 2, 1);
        flex_layout_set_align(
            status_bar, FlexLayoutAlignStart, FlexLayoutAlignCenter, FlexLayoutAlignCenter);

        instance->ble_status_indicator =
            ble_status_indicator_alloc(flex_layout_get_base(status_bar));
        widget_set_margin(
            ble_status_indicator_get_base(instance->ble_status_indicator), 0, 0, 2, 2);

        instance->wifi_status_indicator =
            wifi_status_indicator_alloc(flex_layout_get_base(status_bar));
        widget_set_margin(
            wifi_status_indicator_get_base(instance->wifi_status_indicator), 0, 0, 2, 2);

        instance->audio_status_indicator =
            audio_status_indicator_alloc(flex_layout_get_base(status_bar));
        widget_set_margin(
            audio_status_indicator_get_base(instance->audio_status_indicator), 0, 0, 2, 2);
        audio_status_indicator_set_volume(instance->audio_status_indicator, audio_volume);

        instance->usb_status_indicator =
            usb_status_indicator_alloc(flex_layout_get_base(status_bar));
        widget_set_margin(
            usb_status_indicator_get_base(instance->usb_status_indicator), 0, 0, 2, 2);
        usb_status_indicator_set_connection_state(
            instance->usb_status_indicator, is_usb_connected);

        instance->battery_status_indicator =
            battery_status_indicator_alloc(flex_layout_get_base(status_bar));
        widget_set_ignore_layout(
            battery_status_indicator_get_base(instance->battery_status_indicator), true);
        widget_set_align(
            battery_status_indicator_get_base(instance->battery_status_indicator), AlignBottomMid);
        battery_status_indicator_set_error_state(instance->battery_status_indicator, false);
        battery_status_indicator_set_charging_state(
            instance->battery_status_indicator, power_info.is_charging);
        battery_status_indicator_set_charge_amount(
            instance->battery_status_indicator, power_info.charge);
    });

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, status_bar_custom_event_callback, instance);

    furi_pubsub_subscribe(power_get_pubsub(instance->power), power_events_callback, instance);
    furi_pubsub_subscribe(audio_get_pubsub(instance->audio), audio_events_callback, instance);

    furi_record_create(RECORD_STATUS_BAR, instance);
    return instance;
}

int32_t status_bar_srv(void* arg) {
    UNUSED(arg);

    StatusBar* instance = status_bar_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
