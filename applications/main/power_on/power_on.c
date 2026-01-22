#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/anim_player.h>
#include <gui/modules/label.h>

#include <storage/storage.h>
#include <back_display/back_display.h>
#include <front_display/front_display.h>
#include <power/power_service/power.h>
#include <intercom/intercom.h>

#define TAG "PowerON"

#define POWER_ON_START_TIMEOUT_TICKS furi_ms_to_ticks(500)
#define POWER_ON_APP_TIMEOUT_MIN     (15)

#define MIN_TO_MS(minutes)    (minutes * 60U * 1000U)
#define POWER_ON_LOOP_SECTION "loop"

#define POWER_ON_ANIM_PATH(path) APP_ASSETS_PATH("animations") "/" path
#define POWER_ON_DONE_PATH       APP_DATA_PATH("done.txt")

typedef enum {
    PowerOnAppThreadFlagExitToMenu = 1 << 0,
    PowerOnAppThreadFlagExitToTransportMode = 1 << 1,
    PowerOnAppThreadFlagDeviceStarted = 1 << 2,
} PowerOnAppThreadFlag;

#define POWER_ON_APP_ANIMATION_FLAGS \
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

static bool power_on_thread_signal(uint32_t signal, void* arg, void* context) {
    UNUSED(arg);
    furi_assert(context);

    PowerOnApp* instance = context;

    if(signal == FuriSignalExit) {
        // Desktop has received the initial switch state and wants to close us
        furi_check(
            !(furi_thread_flags_set(instance->thread, PowerOnAppThreadFlagDeviceStarted) &
              FuriFlagError));
        return true;
    }

    return false;
}

static PowerOnApp* power_on_app_alloc(void) {
    PowerOnApp* instance = malloc(sizeof(PowerOnApp));

    instance->gui = furi_record_open(RECORD_GUI);
    instance->front_display = furi_record_open(RECORD_FRONT_DISPLAY);
    instance->back_display = furi_record_open(RECORD_BACK_DISPLAY);
    instance->power = furi_record_open(RECORD_POWER);
    instance->storage = furi_record_open(RECORD_STORAGE);

    instance->thread = furi_thread_get_current();
    instance->back_to_transport_timer =
        furi_timer_alloc(back_to_transport_timer_callback, FuriTimerTypeOnce, instance);
    furi_timer_start(
        instance->back_to_transport_timer, furi_ms_to_ticks(MIN_TO_MS(POWER_ON_APP_TIMEOUT_MIN)));

    furi_thread_set_signal_callback(furi_thread_get_current(), power_on_thread_signal, instance);

    return instance;
}

static void power_on_app_free(PowerOnApp* instance) {
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_POWER);
    furi_record_close(RECORD_BACK_DISPLAY);
    furi_record_close(RECORD_FRONT_DISPLAY);
    furi_record_close(RECORD_GUI);

    furi_timer_free(instance->back_to_transport_timer);
    free(instance);
}

static inline bool power_on_done_flag_present(PowerOnApp* instance) {
    return storage_file_exists(instance->storage, POWER_ON_DONE_PATH);
}

static inline void power_on_done_flag_create(PowerOnApp* instance) {
    File* file = storage_file_alloc(instance->storage);

    if(!storage_file_open(file, POWER_ON_DONE_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS))
        FURI_LOG_W(TAG, "Failed to create file");

    storage_file_close(file);
    storage_file_free(file);
}

static AnimPlayer* power_on_animation_alloc(Widget* widget, const char* anim_path) {
    AnimPlayer* anim = anim_player_alloc(widget);

    do {
        if(!anim_player_set_source(anim, anim_path)) break;
        AnimFile* file = anim_player_get_file(anim);
        furi_assert(file);

        if(!anim_file_set_section(
               file, AnimFilePlayFlagFinishCurrent | AnimFilePlayFlagLoop, POWER_ON_LOOP_SECTION))
            break;
    } while(0);

    return anim;
}

int32_t power_on_app(void* arg) {
    UNUSED(arg);

    PowerOnApp* instance = power_on_app_alloc();

    GuiLayer* layer_main = gui_get_layer(instance->gui, GuiLayerIdMain);
    Widget* front_root = gui_layer_get_root_widget(layer_main, GuiDisplayIdFront);
    Widget* back_root = gui_layer_get_root_widget(layer_main, GuiDisplayIdBack);

    AnimPlayer* front_anim = NULL;
    AnimPlayer* back_anim = NULL;

    Label* front_label = NULL;
    Label* back_label = NULL;

    // avoid showing text for <500ms
    uint32_t wanted_flags = PowerOnAppThreadFlagDeviceStarted;
    uint32_t flags =
        furi_thread_flags_wait(wanted_flags, FuriFlagWaitAny, POWER_ON_START_TIMEOUT_TICKS);

    if(flags == FuriFlagErrorTimeout) {
        with_gui(instance->gui, {
            front_label = label_alloc(front_root);
            back_label = label_alloc(back_root);

            label_set_text(front_label, "Starting...");
            label_set_text(back_label, "Starting...");

            widget_set_align(label_get_base(front_label), AlignCenter);
            widget_set_align(label_get_base(back_label), AlignCenter);
        });

        furi_thread_flags_wait(wanted_flags, FuriFlagWaitAny, FuriWaitForever);

    } else {
        furi_check(!(flags & FuriFlagError));
    }

    do {
        if(power_on_done_flag_present(instance)) break;

        with_gui(instance->gui, {
            gui_layer_add_input_callback(layer_main, power_on_input_callback, instance);

            if(front_label) label_free(front_label);
            if(back_label) label_free(back_label);

            front_anim = power_on_animation_alloc(
                front_root, POWER_ON_ANIM_PATH("front_power_on_72x16.anim"));
            back_anim = power_on_animation_alloc(
                back_root, POWER_ON_ANIM_PATH("back_power_on_148x80.anim"));
        });

        uint32_t flags =
            furi_thread_flags_wait(POWER_ON_APP_ANIMATION_FLAGS, FuriFlagWaitAny, FuriWaitForever);

        if(flags & PowerOnAppThreadFlagExitToTransportMode) {
            power_off(instance->power);
        }

        if(flags & PowerOnAppThreadFlagExitToMenu) {
            furi_timer_stop(instance->back_to_transport_timer);
            power_on_done_flag_create(instance);
        }
    } while(0);

    with_gui(instance->gui, {
        if(front_anim) anim_player_free(front_anim);
        if(back_anim) anim_player_free(back_anim);

        if(front_label) label_free(front_label);
        if(back_label) label_free(back_label);

        gui_layer_remove_input_callback(layer_main, power_on_input_callback);
    });

    power_on_app_free(instance);
    return 0;
}
