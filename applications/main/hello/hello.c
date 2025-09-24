
#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/anim_image.h>

#include <storage/storage.h>
#include <back_display/back_display.h>
#include <front_display/front_display.h>
#include <power/power_service/power.h>

#define HELLO_APP_TIMEOUT_MIN (15)

#define MIN_TO_MS(minutes) (minutes * 60U * 1000U)

#define HELLO_ANIMATION_LOOP_START_FRAME (90)
#define HELLO_ANIMATION_LOOP_END_FRAME   (630)

#define HELLO_ASSETS_PATH(path) EXT_PATH("apps_assets/power_on") "/" path
#define HELLO_ANIM_PATH(path)   HELLO_ASSETS_PATH("animations") "/" path

typedef enum {
    HelloAppThreadFlagExitToMenu = 1 << 0,
    HelloAppThreadFlagExitToTransportMode = 1 << 1,
} HelloAppThreadFlag;

#define HELLO_APP_FLAGS (HelloAppThreadFlagExitToMenu | HelloAppThreadFlagExitToTransportMode)

typedef struct {
    Gui* gui;
    FrontDisplaySrv* front_display;
    BackDisplaySrv* back_display;
    Input* input;
    Power* power;

    FuriThread* thread;
    FuriTimer* back_to_transport_timer;
} HelloApp;

static bool hello_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    HelloApp* instance = context;

    bool consumed = false;
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyStart)
            furi_thread_flags_set(instance->thread, HelloAppThreadFlagExitToMenu);
        consumed = true;
    } else if(event->type == InputTypeLong) {
        if(event->key == InputKeyBack)
            furi_thread_flags_set(instance->thread, HelloAppThreadFlagExitToTransportMode);
        consumed = true;
    }

    return consumed;
}

static void back_to_transport_timer_callback(void* ctx) {
    HelloApp* instance = ctx;
    furi_thread_flags_set(instance->thread, HelloAppThreadFlagExitToTransportMode);
}

static HelloApp* hello_app_alloc() {
    HelloApp* instance = malloc(sizeof(HelloApp));

    instance->gui = furi_record_open(RECORD_GUI);
    instance->front_display = furi_record_open(RECORD_FRONT_DISPLAY);
    instance->back_display = furi_record_open(RECORD_BACK_DISPLAY);
    instance->input = furi_record_open(RECORD_INPUT);
    instance->power = furi_record_open(RECORD_POWER);

    instance->thread = furi_thread_get_current();
    instance->back_to_transport_timer =
        furi_timer_alloc(back_to_transport_timer_callback, FuriTimerTypeOnce, instance);
    furi_timer_start(
        instance->back_to_transport_timer, furi_ms_to_ticks(MIN_TO_MS(HELLO_APP_TIMEOUT_MIN)));
    return instance;
}

static void hello_app_free(HelloApp* instance) {
    furi_record_close(RECORD_POWER);
    furi_record_close(RECORD_INPUT);
    furi_record_close(RECORD_BACK_DISPLAY);
    furi_record_close(RECORD_FRONT_DISPLAY);
    furi_record_close(RECORD_GUI);

    furi_timer_free(instance->back_to_transport_timer);
    free(instance);
}

int32_t hello_app(void* arg) {
    UNUSED(arg);

    HelloApp* instance = hello_app_alloc();

    back_display_sleep_mode(instance->back_display, true);

    AnimImage* anim_image;
    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, hello_input_callback, instance);
        Widget* root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);

        anim_image = anim_image_alloc(root);
        anim_image_set_source(anim_image, HELLO_ANIM_PATH("power_on_72x16.anim"));
        anim_image_set_range(
            anim_image,
            HELLO_ANIMATION_LOOP_START_FRAME,
            HELLO_ANIMATION_LOOP_END_FRAME,
            true,
            true);
    });

    uint32_t flags = furi_thread_flags_wait(HELLO_APP_FLAGS, FuriFlagWaitAny, FuriWaitForever);
    if(flags & HelloAppThreadFlagExitToTransportMode) {
        power_off(instance->power);
    }

    if(flags & HelloAppThreadFlagExitToMenu) {
        furi_timer_stop(instance->back_to_transport_timer);
    }

    with_gui(instance->gui, {
        anim_image_free(anim_image);
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, hello_input_callback);
    });

    back_display_sleep_mode(instance->back_display, false);

    hello_app_free(instance);
    return 0;
}
