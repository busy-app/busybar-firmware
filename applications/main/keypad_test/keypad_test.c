#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
// #include <input/input_new.h> // TODO:

#define TAG "KeypadTest"

typedef struct {
    int32_t encoder;
    uint16_t ok;
    uint16_t menu;
    int16_t sw_pos;
    FuriMutex* mutex;
} KeypadTestState;

static void keypad_test_reset_state(KeypadTestState* state) {
    state->encoder = 0;
    state->ok = 0;
    state->menu = 0;
    state->sw_pos = -1;
}

static void keypad_test_render_callback(Canvas* canvas, void* ctx) {
    KeypadTestState* state = ctx;
    furi_mutex_acquire(state->mutex, FuriWaitForever);
    canvas_clear(canvas);

    canvas_set_color(canvas, ColorWhite4);
    canvas_draw_box(canvas, 0, 0, canvas_width(canvas), 16);

    canvas_set_color(canvas, ColorWhiteMax);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 4, 12, "Keypad test");

    canvas_set_font(canvas, FontSecondary);

    char str_temp[40];
    snprintf(str_temp, 40, "Encoder: %ld", state->encoder);
    canvas_draw_str(canvas, 4, 34, str_temp);

    snprintf(str_temp, 40, "OK: %d    Menu: %d", state->ok, state->menu);
    canvas_draw_str(canvas, 4, 46, str_temp);

    if(state->sw_pos >= 0) {
        snprintf(str_temp, 40, "Switch: %d", state->sw_pos);
        canvas_draw_str(canvas, 4, 58, str_temp);
    }

    // canvas_draw_str(canvas, 35, 24, strings[2]);
    // canvas_draw_str(canvas, 0, 36, strings[3]);
    // canvas_draw_str(canvas, 35, 36, strings[4]);
    // canvas_draw_str(canvas, 0, 48, strings[0]);

    canvas_draw_str(canvas, 10, canvas_height(canvas) - 1, "[back] - reset, hold to exit");

    furi_mutex_release(state->mutex);
}

static void keypad_test_input_callback(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t keypad_test_app(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(32, sizeof(InputEvent));
    furi_check(event_queue);

    KeypadTestState state = {0};
    state.sw_pos = -1;
    state.mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    if(!state.mutex) {
        FURI_LOG_E(TAG, "cannot create mutex");
        return 0;
    }

    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, keypad_test_render_callback, &state);
    view_port_input_callback_set(view_port, keypad_test_input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    InputEvent event;
    while(furi_message_queue_get(event_queue, &event, FuriWaitForever) == FuriStatusOk) {
        furi_mutex_acquire(state.mutex, FuriWaitForever);
        FURI_LOG_I(
            TAG,
            "key: %s type: %s",
            input_get_key_name(event.key),
            input_get_type_name(event.type));

        // if(event.key == InputEncoder) {
        //     if(event.type == InputTypeEncoderTurn) {
        //         state.encoder += event.click_count;
        //     }
        // } else
        if(event.key == InputKeyOk) {
            if(event.type == InputTypeShort) {
                ++state.ok;
            }
            // } else if(event.key == InputKeyMenu) {
            //     if(event.type == InputTypeShort) {
            //         ++state.menu;
            //     }
            // } else if(event.key == InputSwitch) {
            //     state.sw_pos = event.switch_position;
        } else if(event.key == InputKeyBack) {
            if(event.type == InputTypeLong) {
                furi_mutex_release(state.mutex);
                break;
            } else if(event.type == InputTypeShort) {
                keypad_test_reset_state(&state);
            }
        }

        furi_mutex_release(state.mutex);
        view_port_update(view_port);
    }

    // remove & free all stuff created by app
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_mutex_free(state.mutex);

    furi_record_close(RECORD_GUI);

    return 0;
}
