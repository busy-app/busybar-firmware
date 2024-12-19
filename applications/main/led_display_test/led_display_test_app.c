#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>

#include "led_display_test.h"
#include <led_display/led_display.h>

#define TAG "LedDisplayTest"

typedef struct {
    FuriMutex* mutex;
    FuriMessageQueue* event_queue;
    FuriString* tmp_str;

    LedDisplayTestColor color;
    LedDisplayTestPattern pattern;
} LedDisplayTest;

typedef enum {
    LedDisplayTestEventExit,
    LedDisplayTestEventUpdateDisplay,
} LedDisplayTestEventType;

typedef struct {
    LedDisplayTestEventType type;
} LedDisplayTestEvent;

static void led_display_test_render_callback(Canvas* canvas, void* ctx) {
    LedDisplayTest* instance = ctx;
    furi_mutex_acquire(instance->mutex, FuriWaitForever);
    canvas_clear(canvas);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 12, "Press Start / OK to change Pattern");
    canvas_draw_str(canvas, 0, 24, "Rotate Encoder to change color ");

    furi_string_printf(
        instance->tmp_str, "Pattern: %s", led_display_get_pattern_str(instance->pattern));
    canvas_draw_str(canvas, 10, 40, furi_string_get_cstr(instance->tmp_str));

    furi_string_printf(instance->tmp_str, "Color: %s", led_display_get_color_str(instance->color));
    canvas_draw_str(canvas, 10, 52, furi_string_get_cstr(instance->tmp_str));

    furi_mutex_release(instance->mutex);
}

static void led_display_test_input_callback(InputEvent* input_event, void* ctx) {
    LedDisplayTest* instance = ctx;
    furi_mutex_acquire(instance->mutex, FuriWaitForever);

    if((input_event->key == InputKeyBack) && (input_event->type == InputTypeShort)) {
        LedDisplayTestEvent event = {.type = LedDisplayTestEventExit};
        furi_message_queue_put(instance->event_queue, &event, FuriWaitForever);
    } else if(
        ((input_event->key == InputKeyStart) || (input_event->key == InputKeyOk)) &&
        (input_event->type == InputTypeShort)) {
        instance->pattern = (instance->pattern + 1) % LedDisplayTestPatternNum;
        LedDisplayTestEvent event = {.type = LedDisplayTestEventUpdateDisplay};
        furi_message_queue_put(instance->event_queue, &event, FuriWaitForever);
    } else if((input_event->key == InputKeyUp) && (input_event->type == InputTypeShort)) {
        instance->color = (instance->color + 1) % LedDisplayTestColorNum;
        LedDisplayTestEvent event = {.type = LedDisplayTestEventUpdateDisplay};
        furi_message_queue_put(instance->event_queue, &event, FuriWaitForever);
    } else if((input_event->key == InputKeyDown) && (input_event->type == InputTypeShort)) {
        if(instance->color == 0) {
            instance->color = LedDisplayTestColorNum - 1;
        } else {
            instance->color = (instance->color - 1);
        }
        LedDisplayTestEvent event = {.type = LedDisplayTestEventUpdateDisplay};
        furi_message_queue_put(instance->event_queue, &event, FuriWaitForever);
    }

    furi_mutex_release(instance->mutex);
}

int32_t led_display_test_app(void* p) {
    UNUSED(p);

    LedDisplayTest* instance = malloc(sizeof(LedDisplayTest));
    instance->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->event_queue = furi_message_queue_alloc(32, sizeof(LedDisplayTestEvent));
    instance->tmp_str = furi_string_alloc();

    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, led_display_test_render_callback, instance);
    view_port_input_callback_set(view_port, led_display_test_input_callback, instance);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    led_display_test_set(instance->pattern, instance->color);

    LedDisplayTestEvent event;
    while(true) {
        furi_message_queue_get(instance->event_queue, &event, FuriWaitForever);
        furi_mutex_acquire(instance->mutex, FuriWaitForever);

        if(event.type == LedDisplayTestEventExit) {
            furi_mutex_release(instance->mutex);
            led_display_set_default_img();
            break;
        } else if(event.type == LedDisplayTestEventUpdateDisplay) {
            led_display_test_set(instance->pattern, instance->color);
        }

        furi_mutex_release(instance->mutex);
        view_port_update(view_port);
    }

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(instance->event_queue);
    furi_mutex_free(instance->mutex);
    furi_string_free(instance->tmp_str);
    free(instance);

    furi_record_close(RECORD_GUI);

    return 0;
}
