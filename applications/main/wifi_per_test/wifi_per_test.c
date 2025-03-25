#include <furi.h>

#include "wifi_per_test_i.h"

#include <gui/gui.h>
#include <gui/modules/var_item_list.h>
#include <gui/modules/label.h>
#include "helpers/wifi_per_cli.h"

#define TAG "WifiPerTest"

typedef enum {
    WifiPerTestCustomEventExit = (1UL << 0),
    WifiPerTestCustomEventStartTest = (1UL << 1),
    WifiPerTestCustomEventStopTest = (1UL << 2),
} WifiPerTestCustomEvent;

struct WifiPerTest {
    FuriEventLoop* event_loop;
    Gui* gui;
    VarItemList* var_list;
    bool exit_on_back;
    Label* label;
    WifiPerCliSettings settings;
};

static const char* wifi_per_test_mode_text[] = {
    "Tx",
    "Rx",
};

#define CANNEL_MAX 14
static const VarItemKeyValue wifi_per_test_channel[CANNEL_MAX] = {
    {"2412", 1},
    {"2417", 2},
    {"2422", 3},
    {"2427", 4},
    {"2432", 5},
    {"2437", 6},
    {"2442", 7},
    {"2447", 8},
    {"2452", 9},
    {"2457", 10},
    {"2462", 11},
    {"2467", 12},
    {"2472", 13},
    {"2484", 14},
};

static const char* wifi_per_test_rate_text[] = {"1",    "2",    "5.5",    "11",   "6",    "9",
                                                "12",   "18",   "24",     "36",   "48",   "54",
                                                "MCS0", "MCS1", "MCS2",   "MCS3", "MCS4", "MCS5",
                                                "MCS6", "MCS7", "MCS7_SG"};

static const char* wifi_per_test_mode_work_text[] = {
    "burst",
    "continuous",
    "cw",
    "cw_low",
    "cw_high",
};

#define TX_POWER_MAX 18
static VarItemKeyValue wifi_per_test_tx_power[TX_POWER_MAX] = {
    {"2dBm", 2},
    {"3dBm", 3},
    {"4dBm", 4},
    {"5dBm", 5},
    {"6dBm", 6},
    {"7dBm", 7},
    {"8dBm", 8},
    {"9dBm", 9},
    {"10dBm", 10},
    {"11dBm", 11},
    {"12dBm", 12},
    {"13dBm", 13},
    {"14dBm", 14},
    {"15dBm", 15},
    {"16dBm", 16},
    {"17dBm", 17},
    {"18dBm", 18},
    {"Max Power", 127},
};

void wifi_per_test_set_default_settings(WifiPerTest* instance) {
    instance->settings.mode_work = "burst"; //Burst
    instance->settings.mode = BLEPerCliSettingsModeTx;
    instance->settings.channel = 1; //2412
    instance->settings.rate = "6"; //SL_WIFI_DATA_RATE_6
    instance->settings.tx_power = 127; //Max Power
    instance->settings.start_test = false;
}
static void wifi_per_test_mode_work_changed_callback(VarItem* item, void* context) {
    WifiPerTest* instance = context;
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Mode work set: %s", wifi_per_test_mode_work_text[index]);
    instance->settings.mode_work = (char*)wifi_per_test_mode_work_text[index];
}

static void wifi_per_test_mode_changed_callback(VarItem* item, void* context) {
    WifiPerTest* instance = context;
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Mode set: %s", wifi_per_test_mode_text[index]);
    instance->settings.mode = index;
}

static void wifi_per_test_channel_changed_callback(VarItem* item, void* context) {
    WifiPerTest* instance = context;
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Channel set: %s", wifi_per_test_channel[index].key);
    FURI_LOG_I(TAG, "Channel value: %ld", wifi_per_test_channel[index].value);
    instance->settings.channel = wifi_per_test_channel[index].value;
}

static void wifi_per_test_power_changed_callback(VarItem* item, void* context) {
    WifiPerTest* instance = context;
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Power set: %s", wifi_per_test_tx_power[index].key);
    FURI_LOG_I(TAG, "Power value: %ld", wifi_per_test_tx_power[index].value);
    instance->settings.tx_power = wifi_per_test_tx_power[index].value;
}

static void wifi_per_test_rate_changed_callback(VarItem* item, void* context) {
    WifiPerTest* instance = context;
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Rate set: %s", wifi_per_test_rate_text[index]);
    instance->settings.rate = (char*)wifi_per_test_rate_text[index];
}

static void wifi_per_test_switch_changed_callback(VarItem* item, void* context) {
    WifiPerTest* instance = context;
    const int32_t value = var_item_get_value(item);
    FURI_LOG_I(TAG, "Start test set: %s", value ? "ON" : "OFF");
    if(instance->settings.start_test != value) {
        instance->settings.start_test = value;
        if(value) {
            furi_event_loop_set_custom_event(
                instance->event_loop, WifiPerTestCustomEventStartTest);
        } else {
            furi_event_loop_set_custom_event(instance->event_loop, WifiPerTestCustomEventStopTest);
        }
    }
}

static bool wifi_per_test_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    WifiPerTest* instance = context;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            furi_event_loop_set_custom_event(instance->event_loop, WifiPerTestCustomEventExit);
            instance->exit_on_back = true;
            consumed = true;
        }
        //else if(event->key == InputKeyStart) {
        //     furi_event_loop_set_custom_event(instance->event_loop, WifiPerTestCustomEventSound);
        //     consumed = true;
        // }
    }

    return consumed;
}

static void wifi_per_test_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    WifiPerTest* instance = context;

    if(events & WifiPerTestCustomEventExit) {
        if(instance->exit_on_back) {
            if(instance->settings.start_test) {
                FURI_LOG_I(TAG, "Stop test");
                wifi_per_cli_stop();
                instance->settings.start_test = false;
            }
            furi_event_loop_stop(instance->event_loop);
        }
    }

    if(events & WifiPerTestCustomEventStartTest) {
        FURI_LOG_I(TAG, "Start test");
        wifi_per_cli_start(instance, instance->settings);
    }

    if(events & WifiPerTestCustomEventStopTest) {
        FURI_LOG_I(TAG, "Stop test");
        wifi_per_cli_stop();
    }
}

void wifi_per_test_update(
    WifiPerTest* instance,
    uint32_t tx_dones,
    uint32_t crc_fail_cnt,
    uint32_t crc_pass_cnt,
    int32_t rssi) {
    UNUSED(tx_dones);
    FuriString* str = furi_string_alloc();
    furi_string_printf(
        str,
        "rssi %ld \ncrc_fail_cnt %ld crc_pass_cnt %ld ",
        rssi,
        crc_fail_cnt,
        crc_pass_cnt);
    with_gui(instance->gui, { label_set_text(instance->label, furi_string_get_cstr(str)); });
    furi_string_free(str);
}

static WifiPerTest* wifi_per_test_alloc(void) {
    WifiPerTest* instance = malloc(sizeof(WifiPerTest));
    wifi_per_test_set_default_settings(instance);

    instance->event_loop = furi_event_loop_alloc();
    instance->gui = furi_record_open(RECORD_GUI);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, wifi_per_test_custom_event_callback, instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, wifi_per_test_input_callback, instance);

        GuiLayer* top_layer = gui_get_layer(instance->gui, GuiLayerIdTop);
        Widget* top_layer_root = gui_layer_get_root_widget(top_layer, GuiDisplayIdBack);

        Widget* root = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);
        instance->var_list = var_item_list_alloc(root);
        widget_set_pos_y(var_item_list_get_base(instance->var_list), 20);
        widget_set_height(var_item_list_get_base(instance->var_list), 50);

        instance->label = label_alloc(top_layer_root);
        label_set_text(instance->label, "WifiPerTest");
        widget_set_pos(label_get_base(instance->label), 10, 0);

        VarItem* item;
        item = var_item_list_add_selector(
            instance->var_list,
            "Mode Work",
            NULL,
            wifi_per_test_mode_work_text,
            COUNT_OF(wifi_per_test_mode_work_text),
            wifi_per_test_mode_work_changed_callback,
            instance);
        //Todo:
        var_item_set_value(item, 0); //Burst

        item = var_item_list_add_selector(
            instance->var_list,
            "Mode",
            NULL,
            wifi_per_test_mode_text,
            COUNT_OF(wifi_per_test_mode_text),
            wifi_per_test_mode_changed_callback,
            instance);
        var_item_set_value(item, instance->settings.mode);

        item = var_item_list_add_selector_key_value(
            instance->var_list,
            "Channel",
            NULL,
            wifi_per_test_channel,
            CANNEL_MAX,
            wifi_per_test_channel_changed_callback,
            instance);
        var_item_set_value_key_value(item, wifi_per_test_channel, instance->settings.channel);

        item = var_item_list_add_selector_key_value(
            instance->var_list,
            "Power",
            NULL,
            wifi_per_test_tx_power,
            TX_POWER_MAX,
            wifi_per_test_power_changed_callback,
            instance);
        var_item_set_value_key_value(item, wifi_per_test_tx_power, instance->settings.tx_power);

        item = var_item_list_add_selector(
            instance->var_list,
            "Rate",
            NULL,
            wifi_per_test_rate_text,
            COUNT_OF(wifi_per_test_rate_text),
            wifi_per_test_rate_changed_callback,
            instance);
        //Todo:
        var_item_set_value(item, 4); // SL_WIFI_DATA_RATE_6

        item = var_item_list_add_switch(
            instance->var_list, "Start test", wifi_per_test_switch_changed_callback, instance);
        instance->settings.start_test = var_item_get_value(item);
    });

    return instance;
}

static void wifi_per_test_free(WifiPerTest* instance) {
    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, wifi_per_test_input_callback);
        label_free(instance->label);
        var_item_list_free(instance->var_list);
    });

    furi_record_close(RECORD_GUI);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t wifi_per_test_app(void* arg) {
    UNUSED(arg);
    WifiPerTest* instance = wifi_per_test_alloc();
    furi_event_loop_run(instance->event_loop);
    wifi_per_test_free(instance);

    return 0;
}
