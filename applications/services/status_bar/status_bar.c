#include "status_bar.h"
#include "storage_macros.h"

#include <lvgl.h>

#include <gui/gui.h>
#include <gui/modules/image.h>
#include <gui/modules/flex_layout.h>

#define TAG "StatusBar"

#define STATUS_BAR_WIDTH 12

struct StatusBar {
    FuriEventLoop* event_loop;
    Gui* gui;
};

static StatusBar* status_bar_alloc(void) {
    StatusBar* instance = malloc(sizeof(*instance));

    instance->event_loop = furi_event_loop_alloc();
    instance->gui = furi_record_open(RECORD_GUI);

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

        Image* battery = image_alloc(flex_layout_get_base(status_bar));
        widget_add_flag(image_get_base(battery), LV_OBJ_FLAG_IGNORE_LAYOUT);
        widget_set_align(image_get_base(battery), AlignBottomMid);
        image_set_source(battery, STATUS_BAR_IMG_PATH("battery_8x18.bin"));
        widget_set_margin(image_get_base(battery), 0, 0, 2, 2);
        widget_set_size(image_get_base(battery), LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    });

    furi_record_create(RECORD_STATUS_BAR, instance);
    return instance;
}

int32_t status_bar_srv(void* arg) {
    UNUSED(arg);

    StatusBar* instance = status_bar_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}
