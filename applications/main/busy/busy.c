#include <furi.h>

#include <gui_lvgl/gui_lvgl.h>

#define TAG "Busy"

extern const lv_image_dsc_t I_pending_39x16;
extern const lv_image_dsc_t I_busy_39x14;

typedef struct {
    FuriEventLoop* event_loop;
    GuiLvgl* gui;
} BusyApp;

static void busy_button_event_callback(lv_event_t* event) {
    lv_obj_t* button = lv_event_get_target_obj(event);
    FURI_LOG_D(
        TAG, "Button %p, event: %s", button, lv_event_code_get_name(lv_event_get_code(event)));
}

static void busy_list_event_callback(lv_event_t* event) {
    lv_anim_t* anim = lv_event_get_scroll_anim(event);
    if(anim) anim->duration = 16 * 4;
}

BusyApp* busy_alloc(void) {
    BusyApp* instance = malloc(sizeof(BusyApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->gui = furi_record_open(RECORD_GUI_LVGL);

    gui_lvgl_acquire(instance->gui);

    lv_obj_t* active = gui_lvgl_get_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive);

    lv_obj_t* image = lv_image_create(active);
    lv_image_set_src(image, &I_pending_39x16);

    lv_obj_t* list = lv_list_create(active);
    lv_obj_set_pos(list, 39, 0);
    lv_obj_set_size(list, 33, lv_obj_get_height(active));
    lv_obj_set_style_text_font(list, &lv_font_tiny5_8, LV_PART_MAIN);
    lv_obj_set_style_pad_left(list, 8, LV_PART_MAIN);
    // TODO: Speed up animation the right way
    lv_obj_add_event_cb(list, busy_list_event_callback, LV_EVENT_SCROLL_BEGIN, NULL);

    lv_obj_t* start_button = lv_list_add_button(list, NULL, "START");
    lv_obj_set_style_text_color(start_button, lv_color_hex(0x033013), LV_PART_MAIN);
    lv_obj_set_style_text_color(
        start_button, lv_color_hex(0x13F562), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_add_event_cb(start_button, busy_button_event_callback, LV_EVENT_CLICKED, instance);

    lv_obj_t* setup_button = lv_list_add_button(list, NULL, "SETUP");
    lv_obj_add_event_cb(setup_button, busy_button_event_callback, LV_EVENT_CLICKED, instance);

    gui_lvgl_release(instance->gui);

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
