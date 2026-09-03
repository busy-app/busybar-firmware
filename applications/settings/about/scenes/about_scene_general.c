#include "../about.h"

#include <gui/modules/label.h>
#include <device_name/device_name.h>
#include <front_display/front_display.h>
#include <back_display/back_display.h>

#include <furi_hal_version.h>
#include <sl_info/sl_info.h>

#define SERIAL_NUMBER_NEW_LINE_POS (8)
#define GREY_TEXT(text)            "#888888 " text "#"

typedef struct {
    Label* front_label;
    Label* back_label;
} AboutSceneGeneral;

static void about_scene_general_fill_name(FuriString* info) {
    DeviceName* device_name = furi_record_open(RECORD_DEVICE_NAME);
    device_name_get(device_name, info);
    furi_string_replace_all_str(info, "#", "##"); // Escape all # chracters
    furi_record_close(RECORD_DEVICE_NAME);
}

static void about_scene_general_fill_serial_number(FuriString* info) {
    size_t serial_num_arr_len = furi_hal_version_uid_size();
    const uint8_t* serial_num_arr = furi_hal_version_uid();
    furi_string_reset(info);
    for(size_t i = 0; i < serial_num_arr_len; i++) {
        furi_string_cat_printf(info, "%02x", serial_num_arr[i]);
        if(i == SERIAL_NUMBER_NEW_LINE_POS) {
            furi_string_cat_str(info, "\n");
        }
    }
}

static void about_scene_general_fill_hardware_version(FuriString* info) {
    furi_string_printf(
        info,
        "%u.F%uB%uC%u",
        furi_hal_version_get_hw_version(),
        furi_hal_version_get_hw_target(),
        furi_hal_version_get_hw_body(),
        furi_hal_version_get_hw_connect());
}

static void about_scene_general_fill_mac_address(FuriString* info) {
    const uint8_t* usb_mac = furi_hal_version_get_usb_mac();
    furi_string_printf(info, "%02x", usb_mac[0]);
    for(size_t i = 1; i < FURI_HAL_VERSION_MAC_LENGTH; i++) {
        furi_string_cat_printf(info, ":%02x", usb_mac[i]);
    }
}

static void about_scene_general_on_enter(void* context) {
    furi_assert(context);
    About* instance = context;

    AboutSceneGeneral* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdGeneral);
    FuriString* general_info_string = furi_string_alloc();
    FuriString* temp_str = furi_string_alloc();

    // Device name
    about_scene_general_fill_name(temp_str);
    furi_string_printf(
        general_info_string, GREY_TEXT("Name:") " %s\n", furi_string_get_cstr(temp_str));

    // Device serial number
    about_scene_general_fill_serial_number(temp_str);
    furi_string_cat_printf(
        general_info_string, GREY_TEXT("Serial number:") "\n%s\n", furi_string_get_cstr(temp_str));

    // Device hardware version
    about_scene_general_fill_hardware_version(temp_str);
    furi_string_cat_printf(
        general_info_string,
        GREY_TEXT("Hardware version:") "\n%s\n",
        furi_string_get_cstr(temp_str));

    // Device WiFi MAC address
    SlInfo* sl_info = furi_record_open(RECORD_SL_INFO);
    const char* sl_mac_addr = NULL;

    SlInfoStatus sl_status = sl_info_get_value(sl_info, "sl_wifi_mac", &sl_mac_addr);
    if(sl_status == SlInfoStatusOk) {
        furi_string_cat_printf(
            general_info_string, GREY_TEXT("Mac address [Wi-Fi]:") "\n%s\n", sl_mac_addr);
    }

    // Device BLE MAC address
    sl_status = sl_info_get_value(sl_info, "sl_ble_mac", &sl_mac_addr);
    if(sl_status == SlInfoStatusOk) {
        furi_string_cat_printf(
            general_info_string, GREY_TEXT("Mac address [BLE]:") "\n%s\n", sl_mac_addr);
    }

    furi_record_close(RECORD_SL_INFO);

    // Device USB MAC address
    about_scene_general_fill_mac_address(temp_str);
    furi_string_cat_printf(
        general_info_string,
        GREY_TEXT("Mac address [USB]:") "\n%s\n",
        furi_string_get_cstr(temp_str));

    // Device Front display info
    furi_string_cat_printf(
        general_info_string,
        GREY_TEXT("Front display:") "\n%dx%d (LED)\n",
        FRONT_DISPLAY_W,
        FRONT_DISPLAY_H);

    // Device Back display info
    furi_string_cat_printf(
        general_info_string,
        GREY_TEXT("Back display:") "\n%dx%d (OLED)",
        BACK_DISPLAY_W,
        BACK_DISPLAY_H);

    furi_string_free(temp_str);

    with_gui(instance->gui, {
        scene->front_label = label_alloc(instance->front_scene_window);
        widget_set_padding(label_get_base(scene->front_label), 0, 2, 0, 0);
        label_set_inline_text_color_formatting(scene->front_label, true);
        label_set_text(scene->front_label, furi_string_get_cstr(general_info_string));
        label_set_font(scene->front_label, FONT_BUSY_REGULAR_5);
        label_set_line_spacing(scene->front_label, -2);

        scene->back_label = label_alloc(instance->back_scene_window);
        widget_set_padding(label_get_base(scene->back_label), 2, 4, 0, 0);
        label_set_inline_text_color_formatting(scene->back_label, true);
        label_set_text(scene->back_label, furi_string_get_cstr(general_info_string));
        label_set_font(scene->back_label, FONT_BUSY_REGULAR_7);

        widget_set_scrollbar_enabled(instance->front_scene_window, true);
        widget_set_scrollbar_enabled(instance->back_scene_window, true);
    });

    furi_string_free(general_info_string);
}

static void about_scene_general_on_exit(void* context) {
    furi_assert(context);
    About* instance = context;
    AboutSceneGeneral* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdGeneral);

    with_gui(instance->gui, {
        label_free(scene->front_label);
        widget_set_scrollbar_enabled(instance->front_scene_window, false);

        label_free(scene->back_label);
        widget_set_scrollbar_enabled(instance->back_scene_window, false);
    });
}

static bool about_scene_general_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);
    About* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeBack) {
        about_pop_location(instance);
        consumed = scene_manager_previous_scene(instance->scene_manager);
    }

    return consumed;
}

const Scene about_scene_general = {
    .enter_callback = about_scene_general_on_enter,
    .exit_callback = about_scene_general_on_exit,
    .event_callback = about_scene_general_on_event,
    .data_size = sizeof(AboutSceneGeneral),
};
