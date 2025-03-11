#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/var_item_list.h>

#define TAG "GuiTest"

#define POWER_MIN  (0)
#define POWER_MAX  (6)
#define POWER_STEP (2)

#define DATA_RATE_MIN    (1)
#define DATA_RATE_MAX    (2)
#define DATA_RATE_STEP   (1)
#define DATA_RATE_SUFFIX "Mbps"

#define TIME_MIN  (0)
#define TIME_MAX  (3 * 60)
#define TIME_STEP (5)

#define VALUE_MIN  (0)
#define VALUE_MAX  (11)
#define VALUE_STEP (1)

typedef struct {
    FuriEventLoop* event_loop;
    Gui* gui;
    VarItemList* var_list;
} GuiTestApp;

static const char* gui_test_mode_text[] = {
    "Rx",
    "Tx",
    "Hopping",
};

static const char* gui_test_channel_text[] = {
    "2402",
    "2440",
    "2480",
};

static void gui_test_mode_changed_callback(VarItem* item, void* context) {
    UNUSED(context);
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Mode set: %s", gui_test_mode_text[index]);
}

static void gui_test_channel_changed_callback(VarItem* item, void* context) {
    UNUSED(context);
    const int32_t index = var_item_get_value(item);
    FURI_LOG_I(TAG, "Channel set: %s", gui_test_channel_text[index]);
}

static void gui_test_power_changed_callback(VarItem* item, void* context) {
    UNUSED(context);
    const int32_t power = var_item_get_value(item);
    FURI_LOG_I(TAG, "Power set: %ld", power);
}

static void gui_test_data_rate_changed_callback(VarItem* item, void* context) {
    UNUSED(context);
    const int32_t data_rate = var_item_get_value(item);
    FURI_LOG_I(TAG, "Data rate set: %ld %s", data_rate, DATA_RATE_SUFFIX);
}

static void gui_test_time_changed_callback(VarItem* item, void* context) {
    UNUSED(context);
    const int32_t time_minutes = var_item_get_value(item);
    const int32_t hh = time_minutes / 60;
    const int32_t mm = time_minutes % 60;
    FURI_LOG_I(TAG, "Time set: %ld:%02ld", hh, mm);
}

static void gui_test_spinbox_changed_callback(VarItem* item, void* context) {
    UNUSED(context);
    const int32_t value = var_item_get_value(item);
    FURI_LOG_I(TAG, "Value set: %ld", value);
}

static void gui_test_switch_changed_callback(VarItem* item, void* context) {
    UNUSED(context);
    const int32_t value = var_item_get_value(item);
    FURI_LOG_I(TAG, "Switch set: %s", value ? "ON" : "OFF");
}

GuiTestApp* gui_test_alloc(void) {
    GuiTestApp* instance = malloc(sizeof(GuiTestApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->gui = furi_record_open(RECORD_GUI);

    with_gui(instance->gui, {
        Widget* root = gui_get_root_widget(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);
        instance->var_list = var_item_list_alloc(root);

        VarItem* item;

        item = var_item_list_add_selector(
            instance->var_list,
            "Mode",
            NULL,
            gui_test_mode_text,
            COUNT_OF(gui_test_mode_text),
            gui_test_mode_changed_callback,
            NULL);

        item = var_item_list_add_selector(
            instance->var_list,
            "Channel",
            NULL,
            gui_test_channel_text,
            COUNT_OF(gui_test_channel_text),
            gui_test_channel_changed_callback,
            NULL);

        item = var_item_list_add_spinbox(
            instance->var_list,
            "Power",
            NULL,
            POWER_MIN,
            POWER_MAX,
            POWER_STEP,
            gui_test_power_changed_callback,
            NULL);

        item = var_item_list_add_spinbox(
            instance->var_list,
            "Data Rate",
            DATA_RATE_SUFFIX,
            DATA_RATE_MIN,
            DATA_RATE_MAX,
            DATA_RATE_STEP,
            gui_test_data_rate_changed_callback,
            NULL);

        item = var_item_list_add_timebox(
            instance->var_list,
            "Time",
            TIME_MIN,
            TIME_MAX,
            TIME_STEP,
            gui_test_time_changed_callback,
            NULL);
        var_item_set_flags(item, VarItemFlagMinIsInf);

        item = var_item_list_add_spinbox(
            instance->var_list,
            "Value",
            NULL,
            VALUE_MIN,
            VALUE_MAX,
            VALUE_STEP,
            gui_test_spinbox_changed_callback,
            NULL);
        var_item_set_flags(item, VarItemFlagMaxIsInf);

        item = var_item_list_add_switch(
            instance->var_list, "Switch", gui_test_switch_changed_callback, NULL);

        gui_set_active_widget(instance->gui, (Widget*)instance->var_list);
    });

    return instance;
}

void gui_test_free(GuiTestApp* instance) {
    with_gui(instance->gui, { var_item_list_free(instance->var_list); });

    furi_record_close(RECORD_GUI);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t gui_test_app(void* arg) {
    UNUSED(arg);

    GuiTestApp* instance = gui_test_alloc();
    furi_event_loop_run(instance->event_loop);
    gui_test_free(instance);

    return 0;
}
