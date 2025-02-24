#include <furi.h>
#include <applications.h>

#include <desktop/desktop.h>
#include <gui_lvgl/gui_lvgl.h>

#define TAG "AppsMenu"

#ifndef APPS_MENU_ERROR_TEST
#define APPS_MENU_ERROR_TEST 1
#endif

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* queue;
    Desktop* desktop;
    GuiLvgl* gui;
    lv_obj_t* list;
} AppsMenu;

// Speed up animations
static void apps_menu_list_event_callback(lv_event_t* event) {
    const lv_event_code_t code = lv_event_get_code(event);

    if(code == LV_EVENT_SCROLL_BEGIN) {
        lv_anim_t* anim = lv_event_get_scroll_anim(event);
        if(anim) anim->duration = 16 * 4;
    }
}

static void apps_menu_button_event_callback(lv_event_t* event) {
    const lv_event_code_t code = lv_event_get_code(event);

    if(code == LV_EVENT_SINGLE_CLICKED) {
        AppsMenu* instance = lv_event_get_user_data(event);
        lv_obj_t* button = lv_event_get_target(event);
        furi_check(
            furi_message_queue_put(instance->queue, &button, FuriWaitForever) == FuriStatusOk);
    }
}

static void apps_menu_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    AppsMenu* instance = context;

    furi_assert(object == instance->queue);

    lv_obj_t* button;
    furi_check(furi_message_queue_get(instance->queue, &button, 0) == FuriStatusOk);

    const char* app_name = lv_list_get_button_text(instance->list, button);

    if(desktop_replace_current_app(instance->desktop, app_name, NULL)) {
        FURI_LOG_D(TAG, "Selected app %s", app_name);
    } else {
        FURI_LOG_E(TAG, "Failed to select app %s", app_name);
    }
}

static AppsMenu* apps_menu_alloc(void) {
    AppsMenu* instance = malloc(sizeof(AppsMenu));

    instance->event_loop = furi_event_loop_alloc();
    instance->queue = furi_message_queue_alloc(1, sizeof(lv_obj_t*));
    instance->desktop = furi_record_open(RECORD_DESKTOP);
    instance->gui = furi_record_open(RECORD_GUI_LVGL);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->queue,
        FuriEventLoopEventIn,
        apps_menu_queue_callback,
        instance);

    with_gui_layer(instance->gui, GuiDisplayIdFront, GuiLayerIdActive, {
        instance->list = lv_list_create(layer);
        lv_obj_set_style_pad_left(instance->list, 6, LV_PART_MAIN);
        lv_obj_set_style_text_font(instance->list, &lv_font_tiny5_8, LV_PART_MAIN);
        lv_obj_set_size(instance->list, lv_obj_get_width(layer), lv_obj_get_height(layer));
        lv_obj_add_event_cb(
            instance->list, apps_menu_list_event_callback, LV_EVENT_SCROLL_BEGIN, NULL);

        for(uint32_t i = 0; i < FLIPPER_APPS_COUNT; ++i) {
            const FlipperInternalApplication* app = &FLIPPER_APPS[i];
            lv_obj_t* button = lv_list_add_button(instance->list, NULL, app->name);
            lv_obj_add_event_cb(
                button, apps_menu_button_event_callback, LV_EVENT_SINGLE_CLICKED, instance);
        }

#if APPS_MENU_ERROR_TEST
        lv_obj_t* error_test = lv_list_add_button(instance->list, NULL, "Error Test");
        lv_obj_add_event_cb(
            error_test, apps_menu_button_event_callback, LV_EVENT_SINGLE_CLICKED, instance);
#endif
    });

    return instance;
}

static void apps_menu_free(AppsMenu* instance) {
    with_gui(instance->gui, { lv_obj_delete(instance->list); });

    furi_record_close(RECORD_GUI_LVGL);
    furi_record_close(RECORD_DESKTOP);

    furi_event_loop_unsubscribe(instance->event_loop, instance->queue);
    furi_message_queue_free(instance->queue);
    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t apps_menu_app(void* arg) {
    UNUSED(arg);

    AppsMenu* instance = apps_menu_alloc();
    furi_event_loop_run(instance->event_loop);
    apps_menu_free(instance);

    return 0;
}
