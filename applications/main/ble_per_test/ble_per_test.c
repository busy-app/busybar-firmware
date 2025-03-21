#include <furi.h>

//#include <audio/audio.h>
//#include <storage/storage.h>
#include <gui/gui.h>
#include <gui/modules/var_item_list.h>
#include <gui/modules/label.h>
#include "helpers/ble_per_cli.h"

#define TAG "BlePerTest"

#define POWER_MIN  (0)
#define POWER_MAX  (6)
#define POWER_STEP (2)

typedef enum {
    BlePerTestCustomEventExit = (1UL << 0),
    BlePerTestCustomEventStartTest = (1UL << 1),
    BlePerTestCustomEventStopTest = (1UL << 2),
} BlePerTestCustomEvent;

typedef struct {
    FuriEventLoop* event_loop;
    Gui* gui;
    VarItemList* var_list;
    bool exit_on_back;
    Label* label;
} BlePerTest;

typedef struct {
    uint8_t mode_work;
    uint8_t mode;
    uint8_t channel;
    uint8_t tx_power;
    uint8_t rate;
    uint8_t payload_type;
    bool start_test;
} BlePerTestSettings;

static const char* ble_per_test_mode_work_text[] = {
    "Carrier",
    "Packet",
};

static const char* ble_per_test_mode_text[] = {
    "Rx",
    "Tx",
    "Hopping",
};

#define CANNEL_MAX 3
static const VarItemKeyValue ble_per_test_channel[CANNEL_MAX] = {
    {"2402", 0},
    {"2440", 19},
    {"2480", 39},
};

#define PAYLOAD_TYPE_KEY_MAX 8
static const VarItemKeyValue ble_per_test_payload_type[PAYLOAD_TYPE_KEY_MAX] = {
    {"PRBS9", 0},
    {"11110000", 1},
    {"10101010", 2},
    {"PRBS15", 3}, 
    {"11111111", 4},
    {"00000000", 5},
    {"00001111", 6},
    {"01010101", 7},
};

#define RATE_MAX 5
static const VarItemKeyValue ble_per_test_rate[RATE_MAX] = {
    {"1Mbps", 0},
    {"2Mbps", 1},
    {"125Kbps", 2},
    {"1Mbps", 3},
    {"500Kbps", 4},
};

static void ble_per_test_mode_work_changed_callback(VarItem* item, void* context) {
    UNUSED(context);
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Mode work set: %s", ble_per_test_mode_work_text[index]);
}

static void ble_per_test_mode_changed_callback(VarItem* item, void* context) {
    UNUSED(context);
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Mode set: %s", ble_per_test_mode_text[index]);
}

static void ble_per_test_channel_changed_callback(VarItem* item, void* context) {
    UNUSED(context);
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Channel set: %s", ble_per_test_channel[index].key);
    FURI_LOG_I(TAG, "Channel value: %ld", ble_per_test_channel[index].value);
}

static void ble_per_test_power_changed_callback(VarItem* item, void* context) {
    UNUSED(context);
    const int32_t power = var_item_get_value(item);
    FURI_LOG_I(TAG, "Power set: %ld", power);
}

static void ble_per_test_payload_type_changed_callback(VarItem* item, void* context) {
    UNUSED(context);
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Payload type set: %s", ble_per_test_payload_type[index].key);
    FURI_LOG_I(TAG, "Payload type value: %ld", ble_per_test_payload_type[index].value);
}

static void ble_per_test_rate_changed_callback(VarItem* item, void* context) {
    UNUSED(context);
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Rate set: %s", ble_per_test_rate[index].key);
    FURI_LOG_I(TAG, "Rate value: %ld", ble_per_test_rate[index].value);
}

static void ble_per_test_switch_changed_callback(VarItem* item, void* context) {

    BlePerTest* instance = context;
    const int32_t value = var_item_get_value(item);
    FURI_LOG_I(TAG, "Start test set: %s", value ? "ON" : "OFF");
    if(value) {
        furi_event_loop_set_custom_event(instance->event_loop, BlePerTestCustomEventStartTest);
    } else {
        furi_event_loop_set_custom_event(instance->event_loop, BlePerTestCustomEventStopTest);
    }
}

static bool ble_per_test_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    BlePerTest* instance = context;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            furi_event_loop_set_custom_event(instance->event_loop, BlePerTestCustomEventExit);
            instance->exit_on_back = true;
            consumed = true;
        } 
        //else if(event->key == InputKeyStart) {
        //     furi_event_loop_set_custom_event(instance->event_loop, BlePerTestCustomEventSound);
        //     consumed = true;
        // }
    }

    return consumed;
}

static void ble_per_test_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    BlePerTest* instance = context;

    if(events & BlePerTestCustomEventExit) {
        if(instance->exit_on_back) {
            furi_event_loop_stop(instance->event_loop);
        }
    }

    if(events & BlePerTestCustomEventStartTest) {
        FURI_LOG_I(TAG, "Start test");
        ble_per_cli_start();
    }

    if(events & BlePerTestCustomEventStopTest) {
        FURI_LOG_I(TAG, "Stop test");
        ble_per_cli_stop();
    }

    // if(events & BlePerTestCustomEventSound) {
    //     audio_play_file(instance->audio, EXT_PATH("audio/test.snd"));
    // }
}

static BlePerTest* ble_per_test_alloc(void) {
    BlePerTest* instance = malloc(sizeof(BlePerTest));
    instance->event_loop = furi_event_loop_alloc();
    instance->gui = furi_record_open(RECORD_GUI);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, ble_per_test_custom_event_callback, instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, ble_per_test_input_callback, instance);

        Widget* root = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);
        instance->var_list = var_item_list_alloc(root);
        widget_set_pos_y(var_item_list_get_base(instance->var_list), 15);
        
        instance->label = label_alloc(root);
        label_set_text(instance->label, "BlePerTest");
        widget_set_pos(label_get_base(instance->label),30, 0);

        VarItem* item;
        item = var_item_list_add_selector(
            instance->var_list,
            "Mode Work",
            NULL,
            ble_per_test_mode_work_text,
            COUNT_OF(ble_per_test_mode_work_text),
            ble_per_test_mode_work_changed_callback,
            NULL);

        item = var_item_list_add_selector(
            instance->var_list,
            "Mode",
            NULL,
            ble_per_test_mode_text,
            COUNT_OF(ble_per_test_mode_text),
            ble_per_test_mode_changed_callback,
            NULL);
        item = var_item_list_add_selector_key_value(
            instance->var_list,
            "Channel",
            NULL,
            ble_per_test_channel,
            CANNEL_MAX,
            ble_per_test_channel_changed_callback,
            NULL);
        item = var_item_list_add_spinbox(
            instance->var_list,
            "Power",
            NULL,
            POWER_MIN,
            POWER_MAX,
            POWER_STEP,
            ble_per_test_power_changed_callback,
            NULL);   
        
        item = var_item_list_add_selector_key_value(
            instance->var_list,
            "Payload Type",
            NULL,
            ble_per_test_payload_type,
            PAYLOAD_TYPE_KEY_MAX,
            ble_per_test_payload_type_changed_callback,
            NULL);
        item = var_item_list_add_selector_key_value(
            instance->var_list,
            "Rate",
            NULL,
            ble_per_test_rate,
            RATE_MAX,
            ble_per_test_rate_changed_callback,
            NULL);

        item = var_item_list_add_switch(
            instance->var_list, "Start test", ble_per_test_switch_changed_callback, instance); 

        UNUSED(item);
    });

    return instance;
}

static void ble_per_test_free(BlePerTest* instance) {
    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, ble_per_test_input_callback);
        label_free(instance->label);
        var_item_list_free(instance->var_list);
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