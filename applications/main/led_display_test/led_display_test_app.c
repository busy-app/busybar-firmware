#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>

#define TAG "LedDisplayTest"

typedef struct {
    FuriMutex* mutex;
    FuriMessageQueue* event_queue;
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
    canvas_draw_str(canvas, 0, 12, "Press Start to change Pattern");
    canvas_draw_str(canvas, 0, 22, "Rotate Encoder to change color ");

    furi_mutex_release(instance->mutex);
}

static void led_display_test_input_callback(InputEvent* input_event, void* ctx) {
    LedDisplayTest* instance = ctx;
    if((input_event->key == InputKeyBack) && (input_event->type == InputTypeShort)) {
        LedDisplayTestEvent event = {.type = LedDisplayTestEventExit};
        furi_message_queue_put(instance->event_queue, &event, FuriWaitForever);
    }
}

int32_t led_display_test_app(void* p) {
    UNUSED(p);

    LedDisplayTest* instance = malloc(sizeof(LedDisplayTest));
    instance->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->event_queue = furi_message_queue_alloc(32, sizeof(LedDisplayTestEvent));

    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, led_display_test_render_callback, instance);
    view_port_input_callback_set(view_port, led_display_test_input_callback, instance);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    LedDisplayTestEvent event;
    while(true) {
        furi_message_queue_get(instance->event_queue, &event, FuriWaitForever);
        furi_mutex_acquire(instance->mutex, FuriWaitForever);

        if(event.type == LedDisplayTestEventExit) {
            furi_mutex_release(instance->mutex);
            break;
        }

        furi_mutex_release(instance->mutex);
        view_port_update(view_port);
    }

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(instance->event_queue);
    furi_mutex_free(instance->mutex);
    furi_record_close(RECORD_GUI);

    free(instance);

    return 0;
}
