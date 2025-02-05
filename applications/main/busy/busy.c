#include <furi.h>

#include <gui_lvgl/gui_lvgl.h>

#define TAG "Busy"

extern const lv_image_dsc_t I_pending_39x16;
extern const lv_image_dsc_t I_busy_39x14;

typedef struct {
    FuriEventLoop* event_loop;
    GuiLvgl* gui;
} BusyApp;

BusyApp* busy_alloc(void) {
    BusyApp* instance = malloc(sizeof(BusyApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->gui = furi_record_open(RECORD_GUI_LVGL);

    gui_lvgl_acquire(instance->gui);

    lv_obj_t* active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);

    lv_obj_t* image = lv_image_create(active);
    lv_image_set_src(image, &I_pending_39x16);

    lv_obj_t* list = lv_list_create(active);
    lv_obj_set_size(list, 33, lv_obj_get_height(active));
    lv_obj_set_style_text_font(list, &lv_font_tiny5_8, LV_PART_MAIN);
    lv_obj_set_pos(list, 39, 0);
    lv_list_add_text(list, "START");
    lv_list_add_text(list, "SETUP");

    gui_lvgl_release(instance->gui);

    FURI_LOG_I(TAG, "is running!");

    return instance;
}

void busy_free(BusyApp* instance) {
    furi_record_close(RECORD_GUI_LVGL);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t busy_app(void* arg) {
    UNUSED(arg);

    BusyApp* instance = busy_alloc();
    furi_event_loop_run(instance->event_loop);
    busy_free(instance);

    return 0;
}
