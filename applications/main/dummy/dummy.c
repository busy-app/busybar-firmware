#include <furi.h>

#include <audio/audio.h>
#include <gui_lvgl/gui_lvgl.h>

typedef enum {
    DummyCustomEventExit = 1UL << 0,
    DummyCustomEventSound = 1UL << 1,
} DummyCustomEvent;

typedef struct {
    FuriEventLoop* event_loop;
    Audio* audio;
    GuiLvgl* gui;
    lv_obj_t* label;
} Dummy;

static void dummy_key_callback(lv_event_t* event) {
    const lv_event_code_t code = lv_event_get_code(event);

    if(code == LV_EVENT_KEY) {
        Dummy* instance = lv_event_get_user_data(event);
        furi_assert(instance);

        const uint32_t key = *(uint32_t*)lv_event_get_param(event);

        if(key == LV_KEY_ESC) {
            furi_event_loop_set_custom_event(instance->event_loop, DummyCustomEventExit);
        } else if(key == LV_KEY_ENTER) {
            furi_event_loop_set_custom_event(instance->event_loop, DummyCustomEventSound);
        }
    }
}

static void dummy_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    Dummy* instance = context;

    if(events & DummyCustomEventExit) {
        furi_event_loop_stop(instance->event_loop);
    }
    if(events & DummyCustomEventSound) {
        audio_play_file(instance->audio, "/ext/audio/test.snd");
    }
}

static Dummy* dummy_alloc(const char* message) {
    Dummy* instance = malloc(sizeof(Dummy));
    instance->event_loop = furi_event_loop_alloc();
    instance->audio = furi_record_open(RECORD_AUDIO);
    instance->gui = furi_record_open(RECORD_GUI_LVGL);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, dummy_custom_event_callback, instance);

    with_gui_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive, {
        instance->label = lv_label_create(layer);

        lv_label_set_text(instance->label, message ? message : "Hello There");
        lv_obj_set_style_text_color(instance->label, lv_color_white(), LV_PART_MAIN);
        lv_obj_center(instance->label);

        if(message == NULL) {
            lv_group_add_obj(lv_group_get_default(), instance->label);
            lv_obj_add_event_cb(instance->label, dummy_key_callback, LV_EVENT_KEY, instance);
        }
    });

    return instance;
}

static void dummy_free(Dummy* instance) {
    with_gui(instance->gui, { lv_obj_delete(instance->label); });

    furi_record_close(RECORD_GUI_LVGL);
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
