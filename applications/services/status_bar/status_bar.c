#include "status_bar.h"
#include "storage_macros.h"

#include "widgets/battery_status_indicator.h"

#include <lvgl.h>

#include <gui/gui.h>
#include <gui/modules/image.h>
#include <gui/modules/flex_layout.h>

#include <power/power_service/power.h>

#define TAG "StatusBar"

#define STATUS_BAR_WIDTH 12

struct StatusBar {
    FuriEventLoop* event_loop;
    Gui* gui;
    Power* power;
    BatteryStatusIndicator* battery_status_indicator;
};

typedef enum {
    StatusBarUpdateEventPowerChargingState = 1 << 0,
    StatusBarUpdateEventPowerCharge = 1 << 1,

    StatusBarUpdateEventAnyPower = StatusBarUpdateEventPowerChargingState |
                                   StatusBarUpdateEventPowerCharge
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

    case PowerEventChargeUpdate:
        update_event = StatusBarUpdateEventPowerCharge;
        break;

    default:
        return;
    }

    furi_event_loop_set_custom_event(instance->event_loop, update_event);
}

static void status_bar_custom_event_callback(uint32_t events, void* context) {
    StatusBar* instance = context;
    PowerInfo* power_info = NULL;

    if(READ_BIT(events, StatusBarUpdateEventAnyPower)) {
        power_info = alloca(sizeof(*power_info));
        power_get_info(instance->power, power_info);
    }

    with_gui(instance->gui, {
        if(READ_BIT(events, StatusBarUpdateEventPowerChargingState)) {
            battery_status_indicator_set_charging_state(
                instance->battery_status_indicator, power_info->is_charging);
        }

        if(READ_BIT(events, StatusBarUpdateEventPowerCharge)) {
            battery_status_indicator_set_charge_amount(
                instance->battery_status_indicator,
                (power_info->is_full_charged) ? BATTERY_STATUS_INDICATOR_MAX_CHARGE :
                                                power_info->charge);
        }
    })
}

static StatusBar* status_bar_alloc(void) {
    StatusBar* instance = malloc(sizeof(*instance));

    instance->event_loop = furi_event_loop_alloc();
    instance->gui = furi_record_open(RECORD_GUI);
    instance->power = furi_record_open(RECORD_POWER);

    PowerInfo power_info;
    power_get_info(instance->power, &power_info);

    with_gui(instance->gui, {
        GuiLayer* system_layer = gui_get_layer(instance->gui, GuiLayerIdSystem);
        Widget* root = gui_layer_get_root_widget(system_layer, GuiDisplayIdBack);
        FlexLayout* status_bar = flex_layout_alloc(root, FlexLayoutTypeColumn);
        widget_set_align(flex_layout_get_base(status_bar), AlignRightMid);
        widget_set_width(flex_layout_get_base(status_bar), STATUS_BAR_WIDTH);
        widget_set_padding(flex_layout_get_base(status_bar), 0, 0, 2, 1);
        flex_layout_set_align(
            status_bar, FlexLayoutAlignStart, FlexLayoutAlignCenter, FlexLayoutAlignCenter);

        Image* ble = image_alloc(flex_layout_get_base(status_bar));
        image_set_source(ble, STATUS_BAR_IMG_PATH("ble_6x8.bin"));
        widget_set_margin(image_get_base(ble), 0, 0, 2, 2);
        widget_set_size(image_get_base(ble), LV_SIZE_CONTENT, LV_SIZE_CONTENT);

        Image* wifi = image_alloc(flex_layout_get_base(status_bar));
        image_set_source(wifi, STATUS_BAR_IMG_PATH("wifi_8x8.bin"));
        widget_set_margin(image_get_base(wifi), 0, 0, 2, 2);
        widget_set_size(image_get_base(wifi), LV_SIZE_CONTENT, LV_SIZE_CONTENT);

        Image* volume = image_alloc(flex_layout_get_base(status_bar));
        image_set_source(volume, STATUS_BAR_IMG_PATH("sound_on_8x8.bin"));
        widget_set_margin(image_get_base(volume), 0, 0, 2, 2);
        widget_set_size(image_get_base(volume), LV_SIZE_CONTENT, LV_SIZE_CONTENT);

        Image* usb = image_alloc(flex_layout_get_base(status_bar));
        image_set_source(usb, STATUS_BAR_IMG_PATH("usb_8x8.bin"));
        widget_set_margin(image_get_base(usb), 0, 0, 2, 2);
        widget_set_size(image_get_base(usb), LV_SIZE_CONTENT, LV_SIZE_CONTENT);

        instance->battery_status_indicator =
            battery_status_indicator_alloc(flex_layout_get_base(status_bar));
        widget_add_flag(
            battery_status_indicator_get_base(instance->battery_status_indicator),
            LV_OBJ_FLAG_IGNORE_LAYOUT);
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

    furi_record_create(RECORD_STATUS_BAR, instance);
    return instance;
}

int32_t status_bar_srv(void* arg) {
    UNUSED(arg);

    StatusBar* instance = status_bar_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
