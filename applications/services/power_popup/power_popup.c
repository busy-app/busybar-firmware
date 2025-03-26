#include <furi/furi.h>
#include <power/power_service/power.h>
#include <gui/gui.h>
#include <gui/modules/label.h>

#define TAG "PowerPopup"

// TODO: battery low, overheat

typedef struct {
    FuriEventLoop* event_loop;
    FuriSemaphore* ready_semaphore;
    Gui* gui;
    Label* label;
} PowerPopup;

static void power_popup_sub_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const PowerEvent* event = message;
    PowerPopup* instance = context;

    if(event->type == PowerEventReady) {
        furi_semaphore_release(instance->ready_semaphore);
    }
}

static void power_popup_ready(FuriEventLoopObject* object, void* context) {
    PowerPopup* instance = context;
    furi_assert(instance);
    furi_assert(instance->ready_semaphore == object);

    furi_check(furi_semaphore_acquire(object, 0) == FuriStatusOk);
    with_gui(instance->gui, { label_free(instance->label); });
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
    instance->ready_semaphore = furi_semaphore_alloc(1, 0);
    furi_event_loop_subscribe_semaphore(
        instance->event_loop,
        instance->ready_semaphore,
        FuriEventLoopEventIn,
        power_popup_ready,
        instance);

    Power* power = furi_record_open(RECORD_POWER);
    furi_pubsub_subscribe(power_get_pubsub(power), power_popup_sub_callback, instance);
    if(!power_is_battery_ready(power)) {
        instance->gui = furi_record_open(RECORD_GUI);
        power_popup_show_battery_warning(instance);
    }

    furi_event_loop_run(instance->event_loop);

    return 0;
}
