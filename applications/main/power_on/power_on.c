
#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/anim_image.h>

#include <storage/storage.h>
#include <back_display/back_display.h>
#include <front_display/front_display.h>
#include <power/power_service/power.h>

#define POWER_ON_APP_TIMEOUT_MIN (15)

#define MIN_TO_MS(minutes) (minutes * 60U * 1000U)

#define POWER_ON_ANIMATION_LOOP_START_FRAME (90)
#define POWER_ON_ANIMATION_LOOP_END_FRAME   (630)

#define POWER_ON_ANIM_PATH(path) APP_ASSETS_PATH("animations") "/" path
#define POWER_ON_DONE_PATH       APP_DATA_PATH("done.txt")

typedef enum {
    PowerOnAppThreadFlagExitToMenu = 1 << 0,
    PowerOnAppThreadFlagExitToTransportMode = 1 << 1,
} PowerOnAppThreadFlag;

#define POWER_ON_APP_FLAGS \
    (PowerOnAppThreadFlagExitToMenu | PowerOnAppThreadFlagExitToTransportMode)

typedef struct {
    Gui* gui;
    FrontDisplaySrv* front_display;
    BackDisplaySrv* back_display;
    Input* input;
    Power* power;
    Storage* storage;

    FuriThread* thread;
    FuriTimer* back_to_transport_timer;
} PowerOnApp;

static bool power_on_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    PowerOnApp* instance = context;

    bool consumed = false;
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyStart)
            furi_thread_flags_set(instance->thread, PowerOnAppThreadFlagExitToMenu);
        consumed = true;
    } else if(event->type == InputTypeLong) {
        if(event->key == InputKeyBack)
            furi_thread_flags_set(instance->thread, PowerOnAppThreadFlagExitToTransportMode);
        consumed = true;
    }

    return consumed;
}

static void back_to_transport_timer_callback(void* ctx) {
    PowerOnApp* instance = ctx;
    furi_thread_flags_set(instance->thread, PowerOnAppThreadFlagExitToTransportMode);
}

static PowerOnApp* power_on_app_alloc() {
    PowerOnApp* instance = malloc(sizeof(PowerOnApp));

    instance->gui = furi_record_open(RECORD_GUI);
    instance->front_display = furi_record_open(RECORD_FRONT_DISPLAY);
    instance->back_display = furi_record_open(RECORD_BACK_DISPLAY);
    instance->input = furi_record_open(RECORD_INPUT);
    instance->power = furi_record_open(RECORD_POWER);
    instance->storage = furi_record_open(RECORD_STORAGE);

    instance->thread = furi_thread_get_current();
    instance->back_to_transport_timer =
        furi_timer_alloc(back_to_transport_timer_callback, FuriTimerTypeOnce, instance);
    furi_timer_start(
        instance->back_to_transport_timer, furi_ms_to_ticks(MIN_TO_MS(POWER_ON_APP_TIMEOUT_MIN)));
    return instance;
}

static void power_on_app_free(PowerOnApp* instance) {
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_POWER);
    furi_record_close(RECORD_INPUT);
    furi_record_close(RECORD_BACK_DISPLAY);
    furi_record_close(RECORD_FRONT_DISPLAY);
    furi_record_close(RECORD_GUI);

    furi_timer_free(instance->back_to_transport_timer);
    free(instance);
}

int32_t power_on_app(void* arg) {
    UNUSED(arg);

    PowerOnApp* instance = power_on_app_alloc();

    do {
        if(storage_file_exists(instance->storage, POWER_ON_DONE_PATH)) break;

        back_display_sleep_mode(instance->back_display, true);

        AnimImage* anim_image;
        with_gui(instance->gui, {
            GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
            gui_layer_add_input_callback(layer, power_on_input_callback, instance);
            Widget* root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);

            anim_image = anim_image_alloc(root);
            anim_image_set_source(anim_image, POWER_ON_ANIM_PATH("power_on_72x16.anim"));
            anim_image_set_range(
                anim_image,
                POWER_ON_ANIMATION_LOOP_START_FRAME,
                POWER_ON_ANIMATION_LOOP_END_FRAME,
                true,
                true);
        });

        uint32_t flags =
            furi_thread_flags_wait(POWER_ON_APP_FLAGS, FuriFlagWaitAny, FuriWaitForever);
        if(flags & PowerOnAppThreadFlagExitToTransportMode) {
            power_off(instance->power);
        }

        if(flags & PowerOnAppThreadFlagExitToMenu) {
            furi_timer_stop(instance->back_to_transport_timer);

            File* file = storage_file_alloc(instance->storage);

            if(!storage_file_open(file, POWER_ON_DONE_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS))
                FURI_LOG_W("Power on", "Failed to create file");

            storage_file_close(file);
            storage_file_free(file);
        }

        with_gui(instance->gui, {
            anim_image_free(anim_image);
            GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
            gui_layer_remove_input_callback(layer, power_on_input_callback);
        });

        back_display_sleep_mode(instance->back_display, false);
    } while(false);

    power_on_app_free(instance);
    return 0;
}
