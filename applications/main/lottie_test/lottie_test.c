#include <furi.h>

#include <storage/storage.h>

#include <gui/gui.h>
#include <gui/modules/lottie_animation.h>

#define TAG "LottieTest"

#define DEFAULT_FILE_PATH EXT_PATH("waves_test.json")

#define OFFSET_STEP 0.2F

#define SLOT_TEMPLATE \
    "{"               \
    " \"%s\": {"      \
    "  \"p\": {"      \
    "   \"a\": 0,"    \
    "   \"k\": ["     \
    "     %f,"        \
    "     %f"         \
    "   ]"            \
    "}}}"

#define SLOT_NAME "wave_offset"

typedef enum {
    LottieTestEventExit,
    LottieTestEventIncOffset,
    LottieTestEventDecOffset,
} LottieTestEvent;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    FuriString* text_store;
    Gui* gui;
    LottieAnimation* lottie;
    float offset;
} LottieTest;

static bool lottie_test_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    LottieTest* instance = context;

    LottieTestEvent e;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            e = LottieTestEventExit;
            consumed = true;
        } else if(event->key == InputKeyUp) {
            e = LottieTestEventIncOffset;
            consumed = true;
        } else if(event->key == InputKeyDown) {
            e = LottieTestEventDecOffset;
            consumed = true;
        }
    }

    if(consumed) {
        furi_check(
            furi_message_queue_put(instance->event_queue, &e, FuriWaitForever) == FuriStatusOk);
    }

    return consumed;
}

static void lottie_test_override_offset(LottieTest* instance) {
    furi_string_printf(instance->text_store, SLOT_TEMPLATE, SLOT_NAME, 0.F, instance->offset);

    if(!lottie_animation_override_slot(
           instance->lottie, furi_string_get_cstr(instance->text_store))) {
        FURI_LOG_E(TAG, "Failed to override slot");
    }
}

static void lottie_test_custom_event_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    LottieTest* instance = context;
    furi_assert(instance->event_queue == object);

    LottieTestEvent event;
    furi_check(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk);

    if(event == LottieTestEventExit) {
        furi_event_loop_stop(instance->event_loop);
    } else if(event == LottieTestEventIncOffset) {
        instance->offset += OFFSET_STEP;
        lottie_test_override_offset(instance);
    } else if(event == LottieTestEventDecOffset) {
        instance->offset -= OFFSET_STEP;
        lottie_test_override_offset(instance);
    }
}

static LottieTest* lottie_test_alloc(void) {
    LottieTest* instance = malloc(sizeof(LottieTest));
    instance->event_loop = furi_event_loop_alloc();
    instance->event_queue = furi_message_queue_alloc(16, sizeof(LottieTestEvent));
    instance->text_store = furi_string_alloc();
    instance->gui = furi_record_open(RECORD_GUI);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        lottie_test_custom_event_callback,
        instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, lottie_test_input_callback, instance);

        Widget* root = gui_layer_get_root_widget(main_layer, GuiDisplayIdFront);

        instance->lottie = lottie_animation_alloc(root);
        if(!lottie_animation_set_source(instance->lottie, DEFAULT_FILE_PATH)) {
            FURI_LOG_E(TAG, "Failed to load animation");
        }
    });

    return instance;
}

static void lottie_test_free(LottieTest* instance) {
    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, lottie_test_input_callback);
        lottie_animation_free(instance->lottie);
    });

    furi_record_close(RECORD_GUI);

    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);

    furi_string_free(instance->text_store);
    furi_message_queue_free(instance->event_queue);
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
