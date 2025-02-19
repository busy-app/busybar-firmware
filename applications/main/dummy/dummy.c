#include <furi.h>

#include <gui_lvgl/gui_lvgl.h>

typedef struct {
    FuriEventLoop* event_loop;
    GuiLvgl* gui;
    lv_obj_t* label;
} Dummy;

Dummy* dummy_alloc(const char* message) {
    Dummy* instance = malloc(sizeof(Dummy));
    instance->event_loop = furi_event_loop_alloc();
    instance->gui = furi_record_open(RECORD_GUI_LVGL);

    gui_lvgl_acquire(instance->gui);

    lv_obj_t* active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);

    instance->label = lv_label_create(active);
    lv_label_set_text(instance->label, message);
    lv_obj_set_style_text_color(instance->label, lv_color_white(), LV_PART_MAIN);
    lv_obj_center(instance->label);

    gui_lvgl_release(instance->gui);

    return instance;
}

void dummy_free(Dummy* instance) {
    gui_lvgl_acquire(instance->gui);

    lv_obj_delete(instance->label);

    gui_lvgl_release(instance->gui);

    furi_record_close(RECORD_GUI_LVGL);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t dummy_app(void* arg) {
    furi_assert(arg);

    Dummy* instance = dummy_alloc(arg);
    furi_event_loop_run(instance->event_loop);
    dummy_free(instance);

    return 0;
}
