#include "../about.h"

#include <gui/modules/label.h>
#include <device_name/device_name.h>
#include <front_display/front_display.h>
#include <back_display/back_display.h>

#include <furi_hal_version.h>

#define MAC_ADDRESS_LEN            (6)
#define SERIAL_NUMBER_NEW_LINE_POS (8)
#define GREY_TEXT(text)            "#888888 " text "#"

typedef struct {
    Label* general_info[GuiDisplayIdMax];
    FuriString* general_info_str;
} AboutSceneGeneral;

static void about_scene_general_fill_name(FuriString* info) {
    DeviceName* device_name = furi_record_open(RECORD_DEVICE_NAME);
    device_name_get(device_name, info);
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
    for(size_t i = 1; i < MAC_ADDRESS_LEN; i++) {
        furi_string_cat_printf(info, ":%02x", usb_mac[i]);
    }
}

static void about_scene_general_on_enter(void* context) {
    furi_assert(context);
    About* instance = context;

    AboutSceneGeneral* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdGeneral);
    scene->general_info_str = furi_string_alloc();
    FuriString* temp_str = furi_string_alloc();

    about_scene_general_fill_name(temp_str);
    furi_string_printf(
        scene->general_info_str, GREY_TEXT("Name:") " %s\n", furi_string_get_cstr(temp_str));

    about_scene_general_fill_serial_number(temp_str);
    furi_string_cat_printf(
        scene->general_info_str,
        GREY_TEXT("Serial number:") "\n%s\n",
        furi_string_get_cstr(temp_str));

    about_scene_general_fill_hardware_version(temp_str);
    furi_string_cat_printf(
        scene->general_info_str,
        GREY_TEXT("Hardware version:") "\n%s\n",
        furi_string_get_cstr(temp_str));

    // TODO add BLE and WiFi MAC addresses info
    about_scene_general_fill_mac_address(temp_str);
    furi_string_cat_printf(
        scene->general_info_str,
        GREY_TEXT("Mac address [USB]:") "\n%s\n",
        furi_string_get_cstr(temp_str));

    furi_string_cat_printf(
        scene->general_info_str,
        GREY_TEXT("Front display:") "\n%dx%d (LED)\n",
        FRONT_DISPLAY_W,
        FRONT_DISPLAY_H);
    furi_string_cat_printf(
        scene->general_info_str,
        GREY_TEXT("Back display:") "\n%dx%d (OLED)\n",
        BACK_DISPLAY_W,
        BACK_DISPLAY_H);

    furi_string_free(temp_str);

    Widget* const windows[GuiDisplayIdMax] = {
        [GuiDisplayIdFront] = instance->front_scene_window,
        [GuiDisplayIdBack] = instance->back_scene_window,
    };

    with_gui(instance->gui, {
        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            widget_set_scrollbar_mode(windows[disp], WidgetScrollBarModeAuto);
            scene->general_info[disp] = label_alloc(windows[disp]);
            label_set_inline_text_color_formatting(scene->general_info[disp], true);
            label_set_text(
                scene->general_info[disp], furi_string_get_cstr(scene->general_info_str));
        }
    });
}

static void about_scene_general_on_exit(void* context) {
    furi_assert(context);
    About* instance = context;
    AboutSceneGeneral* scene =
        scene_manager_get_scene_data(instance->scene_manager, SceneIdGeneral);

    furi_string_free(scene->general_info_str);

    with_gui(instance->gui, {
        for(GuiDisplayId disp = 0; disp < GuiDisplayIdMax; disp++) {
            label_free(scene->general_info[disp]);
        }
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
