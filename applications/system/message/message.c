#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/status_view.h>
#include <storage/storage.h>

typedef struct {
    FuriEventLoop* event_loop;
    Gui* gui;
    StatusView* front_status;
    StatusView* back_status;
} MessageApp;

static bool message_app_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    MessageApp* instance = context;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            furi_event_loop_stop(instance->event_loop);
            consumed = true;
        }
    }

    return consumed;
}

static MessageApp* message_app_alloc(const char* message) {
    MessageApp* instance = malloc(sizeof(MessageApp));
    instance->event_loop = furi_event_loop_alloc();
    instance->gui = furi_record_open(RECORD_GUI);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, message_app_input_callback, instance);

        instance->front_status =
            status_view_alloc(gui_layer_get_root_widget(main_layer, GuiDisplayIdFront));
        status_view_set_icon(
            instance->front_status, SHARED_IMG_PATH("info_front_8x8.image"), false);
        status_view_set_primary_text(instance->front_status, message ? message : "Hello There");

        instance->back_status =
            status_view_alloc(gui_layer_get_root_widget(main_layer, GuiDisplayIdBack));
        status_view_set_icon(
            instance->back_status, SHARED_IMG_PATH("info_back_11x11.image"), false);
        status_view_set_primary_text(instance->back_status, message ? message : "Hello There");
    });

    return instance;
}

static void message_app_free(MessageApp* instance) {
    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, message_app_input_callback);
        status_view_free(instance->front_status);
        status_view_free(instance->back_status);
    });

    furi_record_close(RECORD_GUI);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t message_app(void* arg) {
    MessageApp* instance = message_app_alloc(arg);
    furi_event_loop_run(instance->event_loop);
    message_app_free(instance);

    return 0;
}
