#include <furi.h>

#include <audio/audio.h>
#include <storage/storage.h>

#include <gui/gui.h>
#include <gui/modules/label.h>

#include <lvgl.h>

typedef enum {
    LottieTestCustomEventExit = 1UL << 0,
    LottieTestCustomEventSound = 1UL << 1,
} LottieTestCustomEvent;

typedef struct {
    FuriEventLoop* event_loop;
    Audio* audio;
    Gui* gui;
    lv_obj_t* lottie;
    uint8_t draw_buf[72 * 16 * 4];
} LottieTest;

static bool lottie_test_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    LottieTest* instance = context;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            furi_event_loop_set_custom_event(instance->event_loop, LottieTestCustomEventExit);
            consumed = true;
        } else if(event->key == InputKeyStart) {
            furi_event_loop_set_custom_event(instance->event_loop, LottieTestCustomEventSound);
            consumed = true;
        }
    }

    return consumed;
}

static void lottie_test_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    LottieTest* instance = context;

    if(events & LottieTestCustomEventExit) {
        furi_event_loop_stop(instance->event_loop);
    }
}

static LottieTest* lottie_test_alloc(void) {
    LottieTest* instance = malloc(sizeof(LottieTest));
    instance->event_loop = furi_event_loop_alloc();
    instance->audio = furi_record_open(RECORD_AUDIO);
    instance->gui = furi_record_open(RECORD_GUI);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, lottie_test_custom_event_callback, instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, lottie_test_input_callback, instance);

        Widget* root = gui_layer_get_root_widget(main_layer, GuiDisplayIdFront);

        instance->lottie = lv_lottie_create((lv_obj_t*)root);
        lv_lottie_set_buffer(instance->lottie, 72, 16, instance->draw_buf);
        lv_lottie_set_src_file(instance->lottie, "/ext/lottietest.json");
    });

    return instance;
}

static void lottie_test_free(LottieTest* instance) {
    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, lottie_test_input_callback);
        lv_obj_delete(instance->lottie);
    });

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_AUDIO);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t lottie_test_app(void* arg) {
    UNUSED(arg);

    LottieTest* instance = lottie_test_alloc();
    furi_event_loop_run(instance->event_loop);
    lottie_test_free(instance);

    return 0;
}
