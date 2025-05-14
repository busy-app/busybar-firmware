#include <furi_hal.h>
#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/submenu.h>
#include <gui/modules/label.h>
#include <gui/modules/canvas.h>
#include <power/power_service/power.h>
#include <front_display/front_display.h>

#define TAG "PowerTest"

#define INFO_UPDATE_PERIOD 500

typedef enum {
    PowerTestSceneMenu,
    PowerTestSceneInfo,
    PowerTestSceneCanvas,
} PowerTestScene;

typedef enum {
    PowerTestBack,
    PowerTestOK,
    PowerTestInfo,
    PowerTestOff,
    PowerTestReboot,
    PowerTestTick,
} PowerTestEvent;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* queue;
    Gui* gui;
    Submenu* submenu_front;
    Submenu* submenu_back;
    Canvas* canvas_front;
    Label* label_front;
    Label* label_back;
    PowerTestScene scene;
} PowerTest;

static void power_test_submenu_callback(uint32_t index, void* context) {
    furi_assert(context);
    PowerTest* instance = context;

    furi_check(furi_message_queue_put(instance->queue, &index, FuriWaitForever) == FuriStatusOk);
}

static void power_test_menu_enter(PowerTest* instance) {
    furi_assert(instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);

        instance->submenu_front =
            submenu_alloc(gui_layer_get_root_widget(main_layer, GuiDisplayIdFront));

        submenu_add_item(
            instance->submenu_front, "Off", PowerTestOff, power_test_submenu_callback, instance);
        submenu_add_item(
            instance->submenu_front,
            "Reboot",
            PowerTestReboot,
            power_test_submenu_callback,
            instance);
        submenu_add_item(
            instance->submenu_front, "Info", PowerTestInfo, power_test_submenu_callback, instance);

        instance->submenu_back =
            submenu_alloc(gui_layer_get_root_widget(main_layer, GuiDisplayIdBack));

        submenu_add_item(instance->submenu_back, "Off", PowerTestOff, NULL, NULL);
        submenu_add_item(instance->submenu_back, "Reboot", PowerTestReboot, NULL, NULL);
        submenu_add_item(instance->submenu_back, "Info", PowerTestInfo, NULL, NULL);
    });
}

static void power_test_menu_exit(PowerTest* instance) {
    furi_assert(instance);

    with_gui(instance->gui, {
        submenu_free(instance->submenu_front);
        submenu_free(instance->submenu_back);

        instance->submenu_front = NULL;
        instance->submenu_back = NULL;
    });
}

static void power_test_canvas_enter(PowerTest* instance) {
    furi_assert(instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        Widget* root = gui_layer_get_root_widget(main_layer, GuiDisplayIdFront);

        instance->canvas_front =
            canvas_alloc(root, widget_get_width(root), widget_get_height(root));
        canvas_set_fill_color(instance->canvas_front, (Color){255, 255, 255});
        canvas_fill(instance->canvas_front);
    });

    FrontDisplaySrv* srv = furi_record_open(RECORD_FRONT_DISPLAY);
    front_display_set_brightness(srv, FRONT_DISPLAY_BRIGHTNESS_MAX);
    furi_record_close(RECORD_FRONT_DISPLAY);
}

static void power_test_canvas_exit(PowerTest* instance) {
    furi_assert(instance);

    with_gui(instance->gui, {
        canvas_free(instance->canvas_front);
        instance->canvas_front = NULL;
    });

    FrontDisplaySrv* srv = furi_record_open(RECORD_FRONT_DISPLAY);
    front_display_set_brightness(srv, FRONT_DISPLAY_BRIGHTNESS_AUTO);
    furi_record_close(RECORD_FRONT_DISPLAY);
}

static void power_test_info_update(PowerTest* instance) {
    Power* power = furi_record_open(RECORD_POWER);
    PowerInfo info;
    power_get_info(power, &info);
    furi_record_close(RECORD_POWER);

    char* state = NULL;
    if(info.is_charging) {
        switch(info.debug.charger_status.chg_stat) {
        case Bq25798ChargerStatusChargeStatTrickle:
            state = "Tr chrg";
            break;
        case Bq25798ChargerStatusChargeStatPre:
            state = "Prechrg";
            break;
        case Bq25798ChargerStatusChargeStatFast:
            state = "Fast chrg";
            break;
        case Bq25798ChargerStatusChargeStatTaper:
            state = "Tap chrg";
            break;
        case Bq25798ChargerStatusChargeStatTopOff:
            state = "TopOff chrg";
            break;
        default:
            state = (info.is_full_charged) ? "Chrgd" : "Chrg";
            break;
        }
    } else {
        state = "Idle";
    }

    Bq25798ChargerStatus status = info.debug.charger_status;

    const char* flags_charger = "";
    if(status.treg_stat) FURI_LOG_I(TAG, "r");
    if(status.prechg_timer_stat) FURI_LOG_I(TAG, "p");
    if(status.trichg_timer_stat) FURI_LOG_I(TAG, "t");
    if(status.chg_timer_stat) FURI_LOG_I(TAG, "c");

    const char* flags_battery = "";
    if(status.ts_hot_stat) flags_battery = "H";
    if(status.ts_warm_stat) flags_battery = "W";
    if(status.ts_cool_stat) flags_battery = "C";
    if(status.ts_cold_stat) flags_battery = "-";

    with_gui(instance->gui, {
        label_set_text_fmt(
            instance->label_front,
            "%s %u%% %s%s\nB%.2fV U%.2fV %.2fC",
            state,
            info.charge,
            flags_battery,
            flags_charger,
            info.voltage_battery / 1000.f,
            info.voltage_usb / 1000.f,
            info.temperature_battery_celsius);
        label_set_text_fmt(
            instance->label_back,
            "%s %u%% %s%s\n\nBattery: %.2fV  %.2fA %.2fC\n\nUSB: %.2fV  %.2fA",
            state,
            info.charge,
            flags_battery,
            flags_charger,
            info.voltage_battery / 1000.f,
            info.current_battery / 1000.f,
            info.temperature_battery_celsius,
            info.voltage_usb / 1000.f,
            info.current_usb / 1000.f);
    });
}

static void power_test_info_tick_callback(void* context) {
    furi_assert(context);
    PowerTest* instance = context;

    uint32_t event = PowerTestTick;
    furi_check(furi_message_queue_put(instance->queue, &event, FuriWaitForever) == FuriStatusOk);
}

static void power_test_info_enter(PowerTest* instance) {
    furi_assert(instance);

    with_gui(instance->gui, {
        gui_display_set_theme(instance->gui, GuiDisplayIdBack, GuiThemeIdDebug);
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        Widget* root_front = gui_layer_get_root_widget(main_layer, GuiDisplayIdFront);
        instance->label_front = label_alloc(root_front);

        Widget* root_back = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);
        instance->label_back = label_alloc(root_back);

        widget_set_align(label_get_base(instance->label_front), AlignLeftMid);
        widget_set_align(label_get_base(instance->label_back), AlignCenter);
    });
    power_test_info_update(instance);
    furi_event_loop_tick_set(
        instance->event_loop, INFO_UPDATE_PERIOD, power_test_info_tick_callback, instance);
}

static void power_test_info_exit(PowerTest* instance) {
    furi_assert(instance);
    with_gui(instance->gui, {
        gui_display_set_theme(instance->gui, GuiDisplayIdBack, GuiThemeIdDefault);
        label_free(instance->label_front);
        label_free(instance->label_back);
    });
    furi_event_loop_tick_set(instance->event_loop, 0, NULL, instance);
}

static bool power_test_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    PowerTest* instance = context;

    bool consumed = false;

    if((event->type == InputTypeShort) && (event->key == InputKeyBack)) {
        uint32_t event = PowerTestBack;
        furi_check(
            furi_message_queue_put(instance->queue, &event, FuriWaitForever) == FuriStatusOk);
        consumed = true;
    } else if((event->type == InputTypeShort) && (event->key == InputKeyOk)) {
        uint32_t event = PowerTestOK;
        furi_check(
            furi_message_queue_put(instance->queue, &event, FuriWaitForever) == FuriStatusOk);
        // do not consume here
    }
    return consumed;
}

static void power_test_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    PowerTest* instance = context;

    furi_assert(object == instance->queue);

    uint32_t index;
    furi_check(furi_message_queue_get(instance->queue, &index, 0) == FuriStatusOk);
    if(index == PowerTestOK) {
        if(instance->scene == PowerTestSceneInfo) {
            // do not exit info scene here, to show info on the back display
            instance->scene = PowerTestSceneCanvas;
            power_test_canvas_enter(instance);
        }
    } else if(index == PowerTestBack) {
        if(instance->scene == PowerTestSceneMenu) {
            furi_event_loop_stop(instance->event_loop);
        } else if(instance->scene == PowerTestSceneInfo) {
            power_test_info_exit(instance);
            instance->scene = PowerTestSceneMenu;
            power_test_menu_enter(instance);
        } else if(instance->scene == PowerTestSceneCanvas) {
            power_test_canvas_exit(instance);
            instance->scene = PowerTestSceneInfo;
            // do not enter info scene here, since we are not exit from it
        }
    } else if(index == PowerTestInfo) {
        power_test_menu_exit(instance);
        instance->scene = PowerTestSceneInfo;
        power_test_info_enter(instance);
    } else if(index == PowerTestOff) {
        Power* power = furi_record_open(RECORD_POWER);
        power_off(power);
        furi_record_close(RECORD_POWER);
    } else if(index == PowerTestReboot) {
        Power* power = furi_record_open(RECORD_POWER);
        power_reboot(power, PowerRebootNormal);
        furi_record_close(RECORD_POWER);
    } else if(index == PowerTestTick) {
        if(instance->scene == PowerTestSceneInfo || instance->scene == PowerTestSceneCanvas) {
            power_test_info_update(instance);
        }
    } else {
        furi_crash();
    }
}

PowerTest* power_test_alloc(void) {
    PowerTest* instance = malloc(sizeof(PowerTest));

    instance->event_loop = furi_event_loop_alloc();
    instance->queue = furi_message_queue_alloc(1, sizeof(uint32_t));
    instance->gui = furi_record_open(RECORD_GUI);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->queue,
        FuriEventLoopEventIn,
        power_test_queue_callback,
        instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, power_test_input_callback, instance);
    });

    instance->scene = PowerTestSceneMenu;
    power_test_menu_enter(instance);

    return instance;
}

void power_test_free(PowerTest* instance) {
    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, power_test_input_callback);
    });
    power_test_menu_exit(instance);

    furi_record_close(RECORD_GUI);

    furi_event_loop_unsubscribe(instance->event_loop, instance->queue);
    furi_message_queue_free(instance->queue);
    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t power_test_app(void* arg) {
    UNUSED(arg);

    PowerTest* instance = power_test_alloc();
    furi_event_loop_run(instance->event_loop);
    power_test_free(instance);

    return 0;
}
