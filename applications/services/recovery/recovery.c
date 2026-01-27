#include <furi.h>
#include <core/thread.h>

#include <gui/modules/label.h>

#include <gui/gui.h>

#include <power/power_service/power.h>

#define TAG "Recovery"

typedef struct {
    Gui* gui;

    FuriEventLoop* event_loop;

    Widget* back_container;
    Widget* front_container;

    Label *back_status_label, *front_status_label;

} RecoveryApp;

RecoveryApp* recovery_app_alloc(void) {
    RecoveryApp* instance = malloc(sizeof(*instance));

    instance->gui = furi_record_open(RECORD_GUI);
    instance->event_loop = furi_event_loop_alloc();

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);

        Widget* root_back = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_container = widget_alloc(root_back);

        instance->back_status_label = label_alloc(instance->back_container);
        label_set_text_font_size(instance->back_status_label, LabelFontSizeLarge);
        label_set_text(instance->back_status_label, "Recovering...");
        widget_set_size_content(label_get_base(instance->back_status_label));

        Widget* root_front = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_container = widget_alloc(root_front);
        instance->front_status_label = label_alloc(instance->front_container);
        label_set_text(instance->front_status_label, "Recovering...");
        widget_set_size_content(label_get_base(instance->front_status_label));
    });

    return instance;
}

void recovery_app_free(RecoveryApp* instance) {
    with_gui(instance->gui, {
        widget_free(instance->back_container);
        widget_free(instance->front_container);
    });

    furi_event_loop_free(instance->event_loop);
    furi_record_close(RECORD_GUI);
    free(instance);
}

int32_t recovery_srv(void* context) {
    UNUSED(context);
    FURI_LOG_I(TAG, "Recovery service started");

    RecoveryApp* app = recovery_app_alloc();

    furi_event_loop_run(app->event_loop);

    recovery_app_free(app);

    while(1) {
        FURI_LOG_I(TAG, "Device recovery completed, rebooting...");
        furi_delay_ms(100);
    }

    return 0;
}
