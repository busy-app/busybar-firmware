#include <furi.h>

#include <audio/audio.h>
#include <storage/storage.h>
#include <gui/gui.h>
#include <gui/modules/image.h>

typedef enum {
    HttpViewerCustomEventExit = 1UL << 0,
} HttpViewerCustomEvent;

typedef struct {
    FuriEventLoop* event_loop;
    Audio* audio;
    Gui* gui;
    Image* image;
} HttpViewer;

static bool http_viewer_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    HttpViewer* instance = context;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            furi_event_loop_set_custom_event(instance->event_loop, HttpViewerCustomEventExit);
            consumed = true;
        }
    }

    return consumed;
}

static void http_viewer_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    HttpViewer* instance = context;

    if(events & HttpViewerCustomEventExit) {
        furi_event_loop_stop(instance->event_loop);
    }
}

static HttpViewer* http_viewer_alloc() {
    HttpViewer* instance = malloc(sizeof(HttpViewer));
    instance->event_loop = furi_event_loop_alloc();
    instance->audio = furi_record_open(RECORD_AUDIO);
    instance->gui = furi_record_open(RECORD_GUI);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, http_viewer_custom_event_callback, instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, http_viewer_input_callback, instance);

        Widget* root = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);
        instance->image = image_alloc(root);
        image_set_source(instance->image, "/ext/http_viewer/test2.png");
    });

    return instance;
}

static void http_viewer_free(HttpViewer* instance) {
    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, http_viewer_input_callback);
        image_free(instance->image);
    });

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_AUDIO);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t http_viewer_app(void* arg) {
    UNUSED(arg);
    HttpViewer* instance = http_viewer_alloc(/* TODO: add params here */);
    furi_event_loop_run(instance->event_loop);
    http_viewer_free(instance);

    return 0;
}
