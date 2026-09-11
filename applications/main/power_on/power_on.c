#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/status_view.h>
#include <gui/modules/anim_player.h>

#include <storage/storage.h>
#include <power/power_service/power.h>

#include <busy_timer/time_macros.h>

#define TAG "PowerOn"

#define POWER_ON_START_TIMEOUT_MS (500)
#define POWER_ON_APP_TIMEOUT_MIN  (15)

#define POWER_ON_ANIM_SECTION "loop"
#define POWER_ON_ANIM_FLAGS   (AnimFilePlayFlagFinishCurrent | AnimFilePlayFlagLoop)

#define POWER_ON_ANIM_PATH(path) APP_ASSETS_PATH("animations/") "/" path
#define POWER_ON_DONE_PATH       APP_DATA_PATH("done.txt")

typedef enum {
    PowerOnAppFlagStartupComplete = 1UL << 0,
    PowerOnAppFlagUserInteracted = 1UL << 1,
    PowerOnAppFlagShutdownRequired = 1UL << 2,
} PowerOnAppFlag;

#define POWER_ON_APP_STARTUP_FLAGS (PowerOnAppFlagStartupComplete)
#define POWER_ON_APP_ANIMATION_FLAGS \
    (PowerOnAppFlagUserInteracted | PowerOnAppFlagShutdownRequired)

typedef struct {
    Gui* gui;
    Power* power;
    Storage* storage;
    FuriThreadId thread_id;
    FuriTimer* shutdown_timer;
    StatusView* status_views[GuiDisplayIdMax];
    AnimPlayer* anims[GuiDisplayIdMax];
} PowerOnApp;

static const char* const power_on_anim_paths[] = {
    [GuiDisplayIdFront] = POWER_ON_ANIM_PATH("front_power_on_72x16.anim"),
    [GuiDisplayIdBack] = POWER_ON_ANIM_PATH("back_power_on_148x80.anim"),
};

static_assert(COUNT_OF(power_on_anim_paths) == GuiDisplayIdMax);

static const char* const power_on_spinner_paths[] = {
    [GuiDisplayIdFront] = SHARED_ANIM_PATH("spinner_front_8x8.anim"),
    [GuiDisplayIdBack] = SHARED_ANIM_PATH("spinner_back_16x16.anim"),
};

static_assert(COUNT_OF(power_on_spinner_paths) == GuiDisplayIdMax);

static bool power_on_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    PowerOnApp* instance = context;

    bool consumed = false;
    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyOk:
        case InputKeyBack:
        case InputKeyStart:
        case InputKeyBusy:
        case InputKeyCustom:
        case InputKeyOff:
        case InputKeyApps:
        case InputKeySettings:
            furi_thread_flags_set(instance->thread_id, PowerOnAppFlagUserInteracted);
            consumed = true;
            break;
        default:
            break;
        }
    }

    return consumed;
}

static void power_on_shutdown_timer_callback(void* ctx) {
    PowerOnApp* instance = ctx;
    furi_thread_flags_set(instance->thread_id, PowerOnAppFlagShutdownRequired);
}

static bool power_on_thread_signal_callback(uint32_t signal, void* arg, void* context) {
    UNUSED(arg);
    furi_assert(context);

    PowerOnApp* instance = context;

    if(signal == FuriSignalExit) {
        // Desktop has received the initial switch state and wants to close us
        const uint32_t flags =
            furi_thread_flags_set(instance->thread_id, PowerOnAppFlagStartupComplete);
        furi_check((flags & FuriFlagError) == 0);
        return true;
    }

    return false;
}

static inline bool power_on_is_done_flag_present(PowerOnApp* instance) {
    return storage_file_exists(instance->storage, POWER_ON_DONE_PATH);
}

static inline void power_on_done_flag_create(PowerOnApp* instance) {
    File* file = storage_file_alloc(instance->storage);

    if(!storage_file_open(file, POWER_ON_DONE_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        FURI_LOG_W(TAG, "Failed to create file");
    }

    storage_file_close(file);
    storage_file_free(file);
}

static AnimPlayer* power_on_animation_alloc(Widget* widget, GuiDisplayId display_id) {
    AnimPlayer* anim = anim_player_alloc(widget);

    if(anim_player_set_source(anim, power_on_anim_paths[display_id])) {
        anim_player_set_section(anim, POWER_ON_ANIM_FLAGS, POWER_ON_ANIM_SECTION);
    }

    return anim;
}

static void power_on_show_startup_message(PowerOnApp* instance) {
    with_gui(instance->gui, {
        GuiLayer* layer_main = gui_get_layer(instance->gui, GuiLayerIdMain);

        for(GuiDisplayId display = GuiDisplayIdFront; display < GuiDisplayIdMax; display++) {
            Widget* root = gui_layer_get_root_widget(layer_main, display);

            StatusView* status_view = status_view_alloc(root);
            status_view_set_icon(status_view, power_on_spinner_paths[display], true);
            status_view_set_primary_text(status_view, "Starting...");

            instance->status_views[display] = status_view;
        }
    });
}

static void power_on_show_first_boot_animation(PowerOnApp* instance) {
    with_gui(instance->gui, {
        GuiLayer* layer_main = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer_main, power_on_input_callback, instance);

        for(GuiDisplayId id = 0; id < GuiDisplayIdMax; ++id) {
            Widget* root = gui_layer_get_root_widget(layer_main, id);
            instance->anims[id] = power_on_animation_alloc(root, id);
        }
    });
}

static void power_on_wait_for_start_condition(PowerOnApp* instance) {
    // avoid showing text for < 500ms
    uint32_t flags;

    flags = furi_thread_flags_wait(
        POWER_ON_APP_STARTUP_FLAGS, FuriFlagWaitAny, furi_ms_to_ticks(POWER_ON_START_TIMEOUT_MS));

    if(flags == FuriFlagErrorTimeout) {
        power_on_show_startup_message(instance);
        flags =
            furi_thread_flags_wait(POWER_ON_APP_STARTUP_FLAGS, FuriFlagWaitAny, FuriWaitForever);
    }

    furi_check((flags & FuriFlagError) == 0);
}

static void power_on_wait_for_exit_condition(PowerOnApp* instance) {
    const uint32_t flags =
        furi_thread_flags_wait(POWER_ON_APP_ANIMATION_FLAGS, FuriFlagWaitAny, FuriWaitForever);

    if(flags & PowerOnAppFlagShutdownRequired) {
        power_off(instance->power);
    }

    if(flags & PowerOnAppFlagUserInteracted) {
        furi_timer_stop(instance->shutdown_timer);
        power_on_done_flag_create(instance);
    }
}

static PowerOnApp* power_on_app_alloc(void) {
    PowerOnApp* instance = malloc(sizeof(PowerOnApp));

    instance->gui = furi_record_open(RECORD_GUI);
    instance->power = furi_record_open(RECORD_POWER);
    instance->storage = furi_record_open(RECORD_STORAGE);

    instance->thread_id = furi_thread_get_current_id();
    instance->shutdown_timer =
        furi_timer_alloc(power_on_shutdown_timer_callback, FuriTimerTypeOnce, instance);
    furi_timer_start(
        instance->shutdown_timer, furi_ms_to_ticks(M_TO_MS(POWER_ON_APP_TIMEOUT_MIN)));

    furi_thread_set_signal_callback(
        furi_thread_get_current(), power_on_thread_signal_callback, instance);

    return instance;
}

static void power_on_app_free(PowerOnApp* instance) {
    furi_thread_set_signal_callback(furi_thread_get_current(), NULL, NULL);

    furi_timer_free(instance->shutdown_timer);

    with_gui(instance->gui, {
        for(GuiDisplayId id = 0; id < GuiDisplayIdMax; ++id) {
            if(instance->status_views[id]) {
                status_view_free(instance->status_views[id]);
            }

            if(instance->anims[id]) {
                anim_player_free(instance->anims[id]);
            }
        }

        GuiLayer* layer_main = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer_main, power_on_input_callback);
    });

    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_POWER);
    furi_record_close(RECORD_GUI);

    free(instance);
}

int32_t power_on_app(void* arg) {
    UNUSED(arg);

    PowerOnApp* instance = power_on_app_alloc();

    power_on_wait_for_start_condition(instance);

    if(!power_on_is_done_flag_present(instance)) {
        power_on_show_first_boot_animation(instance);
        power_on_wait_for_exit_condition(instance);
    }

    power_on_app_free(instance);

    return 0;
}
