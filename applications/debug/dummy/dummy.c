#include <furi.h>

#include <audio/audio.h>
#include <storage/storage.h>
#include <gui/gui.h>
#include <gui/modules/label.h>

typedef enum {
    DummyCustomEventExit = 1UL << 0,
    DummyCustomEventSound = 1UL << 1,
} DummyCustomEvent;

typedef struct {
    FuriEventLoop* event_loop;
    Audio* audio;
    Gui* gui;
    Label* labels[GuiDisplayIdMax];
    bool exit_on_back;
} Dummy;

static bool dummy_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Dummy* instance = context;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            furi_event_loop_set_custom_event(instance->event_loop, DummyCustomEventExit);
            consumed = true;
        } else if(event->key == InputKeyStart) {
            furi_event_loop_set_custom_event(instance->event_loop, DummyCustomEventSound);
            consumed = true;
        }
    }

    return consumed;
}

static void dummy_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    Dummy* instance = context;

    if(events & DummyCustomEventExit) {
        if(instance->exit_on_back) {
            furi_event_loop_stop(instance->event_loop);
        }
    }
    if(events & DummyCustomEventSound) {
        audio_play_file(instance->audio, EXT_PATH("audio/test.snd"));
    }
}

static Dummy* dummy_alloc(const char* message) {
    Dummy* instance = malloc(sizeof(Dummy));
    instance->event_loop = furi_event_loop_alloc();
    instance->audio = furi_record_open(RECORD_AUDIO);
    instance->gui = furi_record_open(RECORD_GUI);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, dummy_custom_event_callback, instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, dummy_input_callback, instance);

        Widget* root;

        for(GuiDisplayId id = 0; id < GuiDisplayIdMax; ++id) {
            root = gui_layer_get_root_widget(main_layer, id);

            Label* label = label_alloc(root);
            label_set_text(label, message ? message : "Hello There");
            widget_set_align(label_get_base(label), AlignCenter);

            instance->labels[id] = label;
        }

        if(message == NULL) {
            instance->exit_on_back = true;
        }
    });

    return instance;
}

static void dummy_free(Dummy* instance) {
    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, dummy_input_callback);
        for(GuiDisplayId id = 0; id < GuiDisplayIdMax; ++id) {
            label_free(instance->labels[id]);
        }
    });

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_AUDIO);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t dummy_app(void* arg) {
    Dummy* instance = dummy_alloc(arg);
    furi_event_loop_run(instance->event_loop);
    dummy_free(instance);

    return 0;
}
