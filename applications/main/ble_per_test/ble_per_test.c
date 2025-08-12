#include <furi.h>

#include "ble_per_test_i.h"

#include <gui/gui.h>
#include <gui/modules/var_item_list.h>
#include <gui/modules/label.h>
#include <gui/modules/image.h>
#include <storage/storage.h>
#include "helpers/ble_per_cli.h"

#define TAG "BlePerTest"

#define IMAGE_FRONT_PATH EXT_PATH("apps_assets/debug/images/LabTestFrontDisplay.bin")

typedef enum {
    BlePerTestCustomEventExit = (1UL << 0),
    BlePerTestCustomEventStartTest = (1UL << 1),
    BlePerTestCustomEventStopTest = (1UL << 2),
} BlePerTestCustomEvent;

typedef enum {
    BLEPerTestStateStop,
    BLEPerTestStateLoading,
    BLEPerTestStateRunning,
    BLEPerTestStateStoping,
} BLEPerTestState;

struct BlePerTest {
    FuriEventLoop* event_loop;
    Gui* gui;
    VarItemList* var_list;
    Label* label_status;
    bool exit_on_back;
    Label* label;
    BlePerCliSettings settings;
    BLEPerTestState test_state;
    Image* image_front;
};

static const char* ble_per_test_mode_text[] = {
    "Tx",
    "Rx",
};

static const VarItemKeyValue ble_per_test_channel[] = {
    {"2402", 0},  {"2404", 1},  {"2406", 2},  {"2408", 3},  {"2410", 4},  {"2412", 5},
    {"2414", 6},  {"2416", 7},  {"2418", 8},  {"2420", 9},  {"2422", 10}, {"2424", 11},
    {"2426", 12}, {"2428", 13}, {"2430", 14}, {"2432", 15}, {"2434", 16}, {"2436", 17},
    {"2438", 18}, {"2440", 19}, {"2442", 20}, {"2444", 21}, {"2446", 22}, {"2448", 23},
    {"2450", 24}, {"2452", 25}, {"2454", 26}, {"2456", 27}, {"2458", 28}, {"2460", 29},
    {"2462", 30}, {"2464", 31}, {"2466", 32}, {"2468", 33}, {"2470", 34}, {"2472", 35},
    {"2474", 36}, {"2476", 37}, {"2478", 38}, {"2480", 39},
};

static const VarItemKeyValue ble_per_test_rate[] = {
    {"1Mbps", 0},
    {"2Mbps", 1},
    {"125Kbps", 2},
    {"500Kbps", 3},
};

#define PAYLOAD_LEN_MIN  (1)
#define PAYLOAD_LEN_MAX  (255)
#define PAYLOAD_LEN_STEP (1)

static const VarItemKeyValue ble_per_test_payload_type[] = {
    {"PRBS9", 0},
    {"11110000", 1},
    {"10101010", 2},
    {"PRBS15", 3},
    {"11111111", 4},
    {"00000000", 5},
    {"00001111", 6},
    {"01010101", 7},
};

static VarItemKeyValue ble_per_test_mode_work[] = {
    {"Burst", 0},
    {"Continuous", 1},
    {"Carrier", 2},
};

static VarItemKeyValue ble_per_test_hopping[] = {
    {"No hopping", 0},
    {"Fixed hopping", 1},
    {"Random hopping", 2},
};

static VarItemKeyValue ble_per_test_tx_power[] = {
    {"1dBm", 1},
    {"2dBm", 2},
    {"3dBm", 3},
    {"4dBm", 4},
    {"5dBm", 5},
    {"6dBm", 6},
    {"7dBm", 7},
    {"8dBm", 8},
    {"9dBm", 9},
    {"10dBm", 10},
    {"Max Power", 127},
};

void ble_per_test_set_default_settings(BlePerTest* instance) {
    instance->settings.mode_work = 0; //Burst
    instance->settings.mode = BLEPerCliSettingsModeTx;
    instance->settings.channel = 0; //2402
    instance->settings.rate = 1; //2Mbps
    instance->settings.payload_len = 32; //32 bytes
    instance->settings.payload_type = 0; //PRBS9
    instance->settings.hopping = 0; //No hopping
    instance->settings.tx_power = 127; //Max Power
    instance->test_state = BLEPerTestStateStop;
}
static void ble_per_test_mode_work_changed_callback(VarItem* item, void* context) {
    BlePerTest* instance = context;
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Mode work set: %s", ble_per_test_mode_work[index].key);
    FURI_LOG_I(TAG, "Mode work value: %ld", ble_per_test_mode_work[index].value);
    instance->settings.mode_work = ble_per_test_mode_work[index].value;
}

static void ble_per_test_mode_changed_callback(VarItem* item, void* context) {
    BlePerTest* instance = context;
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Mode set: %s", ble_per_test_mode_text[index]);
    instance->settings.mode = index;
}

static void ble_per_test_hopping_changed_callback(VarItem* item, void* context) {
    BlePerTest* instance = context;
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Hopping set: %s", ble_per_test_hopping[index].key);
    FURI_LOG_I(TAG, "Hopping value: %ld", ble_per_test_hopping[index].value);
    instance->settings.hopping = ble_per_test_hopping[index].value;
}

static void ble_per_test_channel_changed_callback(VarItem* item, void* context) {
    BlePerTest* instance = context;
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Channel set: %s", ble_per_test_channel[index].key);
    FURI_LOG_I(TAG, "Channel value: %ld", ble_per_test_channel[index].value);
    instance->settings.channel = ble_per_test_channel[index].value;
}

static void ble_per_test_power_changed_callback(VarItem* item, void* context) {
    BlePerTest* instance = context;
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Power set: %s", ble_per_test_tx_power[index].key);
    FURI_LOG_I(TAG, "Power value: %ld", ble_per_test_tx_power[index].value);
    instance->settings.tx_power = ble_per_test_tx_power[index].value;
}

static void ble_per_test_payload_len_changed_callback(VarItem* item, void* context) {
    BlePerTest* instance = context;
    const int32_t value = var_item_get_value(item);
    FURI_LOG_I(TAG, "Payload length set: %ld", value);
    instance->settings.payload_len = value;
}

static void ble_per_test_payload_type_changed_callback(VarItem* item, void* context) {
    BlePerTest* instance = context;
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Payload type set: %s", ble_per_test_payload_type[index].key);
    FURI_LOG_I(TAG, "Payload type value: %ld", ble_per_test_payload_type[index].value);
    instance->settings.payload_type = ble_per_test_payload_type[index].value;
}

static void ble_per_test_rate_changed_callback(VarItem* item, void* context) {
    BlePerTest* instance = context;
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Rate set: %s", ble_per_test_rate[index].key);
    FURI_LOG_I(TAG, "Rate value: %ld", ble_per_test_rate[index].value);
    instance->settings.rate = ble_per_test_rate[index].value;
}

static bool ble_per_test_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    BlePerTest* instance = context;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            if(instance->test_state == BLEPerTestStateRunning) {
                furi_event_loop_set_custom_event(
                    instance->event_loop, BlePerTestCustomEventStopTest);
            } else {
                furi_event_loop_set_custom_event(instance->event_loop, BlePerTestCustomEventExit);
                instance->exit_on_back = true;
            }
            consumed = true;
        }

    } else if(event->type == InputTypeLong) {
        if(event->key == InputKeyStart) {
            if(instance->test_state == BLEPerTestStateRunning) {
                furi_event_loop_set_custom_event(
                    instance->event_loop, BlePerTestCustomEventStopTest);
            } else if(instance->test_state == BLEPerTestStateStop) {
                furi_event_loop_set_custom_event(
                    instance->event_loop, BlePerTestCustomEventStartTest);
                instance->test_state = BLEPerTestStateLoading;
            }
            consumed = true;
        }
    }

    if(instance->test_state != BLEPerTestStateStop) {
        consumed = true;
    }

    return consumed;
}

static void ble_per_test_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    BlePerTest* instance = context;

    if(events & BlePerTestCustomEventExit) {
        if(instance->exit_on_back) {
            FURI_LOG_I(TAG, "Exit test");
            if(ble_per_cli_is_running()) {
                ble_per_cli_stop();
                ble_per_cli_deinit();
            }
            furi_event_loop_stop(instance->event_loop);
        }
    }

    if(events & BlePerTestCustomEventStartTest) {
        furi_check(instance->test_state == BLEPerTestStateLoading);

        if(!ble_per_cli_is_running()) {
            if(ble_per_cli_init(instance)) {
                FURI_LOG_I(TAG, "Init test");
                ble_per_cli_start(instance->settings);
                with_gui(instance->gui, {
                    widget_set_visible(var_item_list_get_base(instance->var_list), false);
                    widget_set_visible(label_get_base(instance->label_status), true);
                });
                instance->test_state = BLEPerTestStateRunning;
            } else {
                FURI_LOG_E(TAG, "Init test failed");
                ble_per_cli_stop();
                instance->test_state = BLEPerTestStateStop;
            }
        } else {
            FURI_LOG_I(TAG, "Start test");
            ble_per_cli_start(instance->settings);
            with_gui(instance->gui, {
                widget_set_visible(var_item_list_get_base(instance->var_list), false);
                widget_set_visible(label_get_base(instance->label_status), true);
            });
            instance->test_state = BLEPerTestStateRunning;
        }
    }

    if(events & BlePerTestCustomEventStopTest) {
        furi_check(instance->test_state == BLEPerTestStateRunning);
        instance->test_state = BLEPerTestStateStoping;
        FURI_LOG_I(TAG, "Stop test");
        ble_per_cli_stop();
        with_gui(instance->gui, {
            widget_set_visible(var_item_list_get_base(instance->var_list), true);
            widget_set_visible(label_get_base(instance->label_status), false);
        });
        instance->test_state = BLEPerTestStateStop;
    }
}

void ble_per_test_update(
    BlePerTest* instance,
    uint32_t tx_dones,
    uint32_t crc_fail_cnt,
    uint32_t crc_pass_cnt,
    int32_t rssi) {
    FuriString* str = furi_string_alloc();
    furi_string_printf(
        str,
        "TxDones %ld\nRssi %ld\nCrcFallCnt %ld\nCrcPassCnt %ld",
        tx_dones,
        rssi,
        crc_fail_cnt,
        crc_pass_cnt);
    with_gui(instance->gui, {
        label_set_text(instance->label_status, furi_string_get_cstr(str));
    });
    furi_string_free(str);
}

static BlePerTest* ble_per_test_alloc(void) {
    BlePerTest* instance = malloc(sizeof(BlePerTest));
    ble_per_test_set_default_settings(instance);

    instance->event_loop = furi_event_loop_alloc();
    instance->gui = furi_record_open(RECORD_GUI);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, ble_per_test_custom_event_callback, instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, ble_per_test_input_callback, instance);

        GuiLayer* top_layer = gui_get_layer(instance->gui, GuiLayerIdTop);
        Widget* top_layer_root = gui_layer_get_root_widget(top_layer, GuiDisplayIdBack);

        Widget* root = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);
        instance->var_list = var_item_list_alloc(root);
        widget_set_pos_y(var_item_list_get_base(instance->var_list), 15);
        widget_set_height(var_item_list_get_base(instance->var_list), 65);

        instance->label_status = label_alloc(root);
        widget_set_pos_y(label_get_base(instance->label_status), 20);
        widget_set_height(label_get_base(instance->label_status), 50);
        widget_set_visible(label_get_base(instance->label_status), false);

        instance->label = label_alloc(top_layer_root);
        label_set_text(instance->label, "BlePerTest");
        widget_set_pos(label_get_base(instance->label), 10, 0);

        // GuiDisplayIdFront
        Widget* root_front = gui_layer_get_root_widget(main_layer, GuiDisplayIdFront);
        instance->image_front = image_alloc(root_front);
        image_set_source(instance->image_front, IMAGE_FRONT_PATH);
        widget_set_align(image_get_base(instance->image_front), AlignCenter);

        VarItem* item;
        item = var_item_list_add_selector_key_value(
            instance->var_list,
            "Mode Work",
            NULL,
            ble_per_test_mode_work,
            COUNT_OF(ble_per_test_mode_work),
            ble_per_test_mode_work_changed_callback,
            instance);
        var_item_set_value_key_value(item, ble_per_test_mode_work, instance->settings.mode_work);

        item = var_item_list_add_selector(
            instance->var_list,
            "Mode",
            NULL,
            ble_per_test_mode_text,
            COUNT_OF(ble_per_test_mode_text),
            ble_per_test_mode_changed_callback,
            instance);
        var_item_set_value(item, instance->settings.mode);

        item = var_item_list_add_selector_key_value(
            instance->var_list,
            "Hopping",
            NULL,
            ble_per_test_hopping,
            COUNT_OF(ble_per_test_hopping),
            ble_per_test_hopping_changed_callback,
            instance);
        var_item_set_value_key_value(item, ble_per_test_hopping, instance->settings.hopping);

        item = var_item_list_add_selector_key_value(
            instance->var_list,
            "Channel",
            NULL,
            ble_per_test_channel,
            COUNT_OF(ble_per_test_channel),
            ble_per_test_channel_changed_callback,
            instance);
        var_item_set_value_key_value(item, ble_per_test_channel, instance->settings.channel);

        item = var_item_list_add_selector_key_value(
            instance->var_list,
            "Power",
            NULL,
            ble_per_test_tx_power,
            COUNT_OF(ble_per_test_tx_power),
            ble_per_test_power_changed_callback,
            instance);
        var_item_set_value_key_value(item, ble_per_test_tx_power, instance->settings.tx_power);

        item = var_item_list_add_spinbox(
            instance->var_list,
            "Payload Length",
            NULL,
            PAYLOAD_LEN_MIN,
            PAYLOAD_LEN_MAX,
            PAYLOAD_LEN_STEP,
            ble_per_test_payload_len_changed_callback,
            instance);
        var_item_set_value(item, instance->settings.payload_len);

        item = var_item_list_add_selector_key_value(
            instance->var_list,
            "Payload Type",
            NULL,
            ble_per_test_payload_type,
            COUNT_OF(ble_per_test_payload_type),
            ble_per_test_payload_type_changed_callback,
            instance);
        var_item_set_value_key_value(
            item, ble_per_test_payload_type, instance->settings.payload_type);

        item = var_item_list_add_selector_key_value(
            instance->var_list,
            "Rate",
            NULL,
            ble_per_test_rate,
            COUNT_OF(ble_per_test_rate),
            ble_per_test_rate_changed_callback,
            instance);
        var_item_set_value_key_value(item, ble_per_test_rate, instance->settings.rate);
    });

    return instance;
}

static void ble_per_test_free(BlePerTest* instance) {
    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, ble_per_test_input_callback);
        label_free(instance->label);
        label_free(instance->label_status);
        var_item_list_free(instance->var_list);
        image_free(instance->image_front);
    });

    furi_record_close(RECORD_GUI);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t ble_per_test_app(void* arg) {
    UNUSED(arg);
    BlePerTest* instance = ble_per_test_alloc();
    furi_event_loop_run(instance->event_loop);
    ble_per_test_free(instance);

    return 0;
}
