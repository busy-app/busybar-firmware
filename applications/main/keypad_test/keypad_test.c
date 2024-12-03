#include <furi.h>

#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_holder.h>

#define TAG "KeypadTest"

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    ViewHolder* view_holder;
    View* view;
} KeypadTestApp;

typedef struct {
    uint32_t ok;
    uint32_t start;
    int32_t encoder;
    int32_t switch_pos;
} KeypadTestViewModel;

static void keypad_test_app_reset_view_model(KeypadTestViewModel* model) {
    memset(model, 0, sizeof(KeypadTestViewModel));
    model->switch_pos = -1;
}

static void keypad_test_app_handle_short_press(InputKey key, KeypadTestViewModel* model) {
    if(key == InputKeyUp) {
        model->encoder++;
    } else if(key == InputKeyDown) {
        model->encoder--;
    } else if(key == InputKeyOk) {
        model->ok++;
    } else if(key == InputKeyBack) {
        keypad_test_app_reset_view_model(model);
    } else if(key == InputKeyStart) {
        model->start++;
    } else if(key >= InputKeyBusy && key < InputKeyMAX) {
        model->switch_pos = key - InputKeyBusy;
    }
}

static void keypad_test_app_input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    KeypadTestApp* instance = context;

    furi_assert(instance->input_queue == object);

    InputEvent event;
    furi_check(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk);

    if(event.type == InputTypeShort) {
        with_view_model(instance->view,
                        KeypadTestViewModel * model,
                        keypad_test_app_handle_short_press(event.key, model);
                        , true);

    } else if(event.type == InputTypeLong) {
        if(event.key == InputKeyBack) {
            furi_event_loop_stop(instance->event_loop);
        }
    }

    FURI_LOG_I(
        TAG, "Key: %s Type: %s", input_get_key_name(event.key), input_get_type_name(event.type));
}

static bool keypad_test_app_view_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    KeypadTestApp* instance = context;

    furi_check(
        furi_message_queue_put(instance->input_queue, event, FuriWaitForever) == FuriStatusOk);
    return true;
}

static void keypad_test_app_view_draw_callback(Canvas* canvas, void* _model) {
    furi_assert(_model);
    KeypadTestViewModel* model = _model;

    canvas_clear(canvas);

    canvas_set_color(canvas, ColorWhite4);
    canvas_draw_box(canvas, 0, 0, canvas_width(canvas), 16);

    canvas_set_color(canvas, ColorWhiteMax);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 4, 12, "Keypad Test");

    canvas_set_font(canvas, FontSecondary);

    char str_temp[40];
    snprintf(str_temp, 40, "OK: %lu  Start: %lu", model->ok, model->start);
    canvas_draw_str(canvas, 4, 30, str_temp);

    snprintf(str_temp, 40, "Encoder: %ld", model->encoder);
    canvas_draw_str(canvas, 4, 44, str_temp);

#if 0
    if(model->switch_pos >= 0) {
        snprintf(str_temp, 40, "Switch: %ld", model->switch_pos);
        canvas_draw_str(canvas, 4, 58, str_temp);
    }
#endif

    canvas_draw_str(canvas, 10, canvas_height(canvas) - 1, "[back] - reset, hold to exit");
}

static KeypadTestApp* keypad_test_app_alloc(void) {
    KeypadTestApp* instance = malloc(sizeof(KeypadTestApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(32, sizeof(InputEvent));
    instance->view_holder = view_holder_alloc();
    instance->view = view_alloc();

    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(KeypadTestViewModel));
    with_view_model(
        instance->view, KeypadTestViewModel * model, keypad_test_app_reset_view_model(model);
        , false);

    view_set_input_callback(instance->view, keypad_test_app_view_input_callback);
    view_set_draw_callback(instance->view, keypad_test_app_view_draw_callback);
    view_set_context(instance->view, instance);

    Gui* gui = furi_record_open(RECORD_GUI);

    view_holder_set_view(instance->view_holder, instance->view);
    view_holder_attach_to_gui(instance->view_holder, gui);
    view_holder_send_to_front(instance->view_holder);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        keypad_test_app_input_queue_callback,
        instance);

    return instance;
}

static void keypad_test_app_free(KeypadTestApp* instance) {
    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    view_holder_set_view(instance->view_holder, NULL);

    furi_message_queue_free(instance->input_queue);
    furi_event_loop_free(instance->event_loop);
    view_holder_free(instance->view_holder);
    view_free(instance->view);

    furi_record_close(RECORD_GUI);

    free(instance);
}

int32_t keypad_test_app(void* p) {
    UNUSED(p);

    KeypadTestApp* instance = keypad_test_app_alloc();
    furi_event_loop_run(instance->event_loop);
    keypad_test_app_free(instance);

    return 0;
}
