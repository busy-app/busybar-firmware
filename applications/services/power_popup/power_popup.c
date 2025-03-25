#include <furi/furi.h>
#include <power/power_service/power.h>
#include <gui/gui.h>
#include <gui/modules/label.h>

#define TAG "PowerPopup"

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
    Gui* gui;
    Label* label;
} PowerPopup;

static void power_popup_sub_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const PowerEvent* event = message;
    // PowerPopup* instance = context;

    FURI_LOG_I(TAG, "EVT %u", event->type);
}

static void power_popup_show_battery_warning(PowerPopup* instance) {
    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdSystem);
        Widget* root = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);
        instance->label = label_alloc(root);
        widget_set_align(label_get_base(instance->label), AlignCenter);
        label_set_text_fmt(instance->label, "Battery not ready");
    });
}

int32_t power_popup_start(void* p) {
    UNUSED(p);

    PowerPopup* instance = malloc(sizeof(PowerPopup));
    instance->event_loop = furi_event_loop_alloc();

    Power* power = furi_record_open(RECORD_POWER);
    furi_pubsub_subscribe(power_get_pubsub(power), power_popup_sub_callback, instance);

    instance->gui = furi_record_open(RECORD_GUI);

    power_popup_show_battery_warning(instance);

    furi_event_loop_run(instance->event_loop);

    return 0;
}
