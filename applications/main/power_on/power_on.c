
#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/anim_image.h>
#include <gui/modules/label.h>

#include <storage/storage.h>
#include <back_display/back_display.h>
#include <front_display/front_display.h>
#include <power/power_service/power.h>
#include <intercom/intercom.h>

#define TAG "PowerON"

#define POWER_ON_APP_TIMEOUT_MIN (15)

#define MIN_TO_MS(minutes) (minutes * 60U * 1000U)

#define POWER_ON_ANIMATION_LOOP_START_FRAME (90)
#define POWER_ON_ANIMATION_LOOP_END_FRAME   (329)

#define POWER_ON_ANIM_PATH(path) APP_ASSETS_PATH("animations") "/" path
#define POWER_ON_DONE_PATH       APP_DATA_PATH("done.txt")

typedef enum {
    PowerOnAppThreadFlagExitToMenu = 1 << 0,
    PowerOnAppThreadFlagExitToTransportMode = 1 << 1,
    PowerOnAppThreadFlagSyncDone = 1 << 2,
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

    Intercom* intercom;
    FuriPubSubSubscription* intercom_event_sub;
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

static void power_on_intercom_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const IntercomEvent* event = message;
    PowerOnApp* instance = context;

    if(event->type == IntercomEventTypeSyncStateChanged) {
        if(event->is_in_sync) {
            furi_thread_flags_set(instance->thread, PowerOnAppThreadFlagSyncDone);
        }
    }
}

static PowerOnApp* power_on_app_alloc(void) {
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

    instance->intercom = furi_record_open(RECORD_INTERCOM);
    instance->intercom_event_sub = furi_pubsub_subscribe(
        intercom_get_pubsub(instance->intercom), power_on_intercom_callback, instance);
    return instance;
}

static void power_on_app_free(PowerOnApp* instance) {
    furi_pubsub_unsubscribe(intercom_get_pubsub(instance->intercom), instance->intercom_event_sub);

    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_POWER);
    furi_record_close(RECORD_INPUT);
    furi_record_close(RECORD_BACK_DISPLAY);
    furi_record_close(RECORD_FRONT_DISPLAY);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_INTERCOM);

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

static AnimImage* power_on_animation_alloc(Widget* widget, const char* anim_path) {
    AnimImage* anim = anim_image_alloc(widget);
    anim_image_set_source(anim, anim_path);
    anim_image_set_range(
        anim, POWER_ON_ANIMATION_LOOP_START_FRAME, POWER_ON_ANIMATION_LOOP_END_FRAME, true, true);
    return anim;
}

int32_t power_on_app(void* arg) {
    UNUSED(arg);

    PowerOnApp* instance = power_on_app_alloc();

    bool sync_done = intercom_is_in_sync(instance->intercom);

    AnimImage* front_anim = NULL;
    AnimImage* back_anim = NULL;

    Label* label_front = NULL;
    Label* label_back = NULL;

    if(!sync_done) {
        with_gui(instance->gui, {
            GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
            Widget* front = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
            Widget* back = gui_layer_get_root_widget(layer, GuiDisplayIdBack);

            label_front = label_alloc(front);
            label_back = label_alloc(back);

            label_set_text(label_front, "Starting...");
            label_set_text(label_back, "Starting...");

            widget_set_align(label_get_base(label_front), AlignCenter);
            widget_set_align(label_get_base(label_back), AlignCenter);
        });

        furi_thread_flags_wait(PowerOnAppThreadFlagSyncDone, FuriFlagWaitAny, FuriWaitForever);
    }

    do {
        if(power_on_done_flag_present(instance)) break;

        with_gui(instance->gui, {
            GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
            gui_layer_add_input_callback(layer, power_on_input_callback, instance);

            if(label_front) label_free(label_front);
            if(label_back) label_free(label_back);

            Widget* front = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
            front_anim =
                power_on_animation_alloc(front, POWER_ON_ANIM_PATH("front_power_on_72x16.anim"));

            Widget* back = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
            back_anim =
                power_on_animation_alloc(back, POWER_ON_ANIM_PATH("back_power_on_160x80.anim"));
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
        if(front_anim) anim_image_free(front_anim);
        if(back_anim) anim_image_free(back_anim);

        if(label_front) label_free(label_front);
        if(label_back) label_free(label_back);

        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, power_on_input_callback);
    });

    power_on_app_free(instance);
    return 0;
}
