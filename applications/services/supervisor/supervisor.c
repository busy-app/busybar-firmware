#include <furi/furi.h>
#include <furi_hal_nvm.h>
#include <power/power_service/power.h>
#include <gui/gui.h>
#include <gui/modules/label.h>
#include <storage/storage.h>
#include <storage/storage_backup.h>

#define TAG "Supervisor"

// TODO: battery low, overheat

typedef struct Supervisor Supervisor;

typedef void (*SupervisorGuiOkCb)(Supervisor* supervisor);

typedef struct {
    Gui* gui;
    Label* front_label;
    Label* back_label;
    bool input_locked;
    SupervisorGuiOkCb ok_callback;
    uint32_t current_warnings;
} SupervisorGui;

struct Supervisor {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    Power* power;
    Storage* storage;

    SupervisorGui gui;
};

typedef enum {
    SupervisorEventTypeBatteryLow,
    SupervisorEventTypeOKPressed,
} SupervisorEventType;

typedef struct {
    SupervisorEventType type;
} SupervisorEvent;

typedef struct {
    const char* front_text;
    const char* back_text;
    bool input_locked;
    SupervisorGuiOkCb ok_callback;
} SupervisorWarning;

// if multiple warnings are active, the one with the lowest enum value is shown
typedef enum {
    SupervisorWarningTypeStorageNoPartitions,
    SupervisorWarningTypeStorageNoBackup,
    SupervisorWarningTypeStorageNoExternal,
    SupervisorWarningTypeBatteryNotReady,
    SupervisorWarningTypeBatteryLow,

    SupervisorWarningTypeMax, // must be last
} SupervisorWarningType;

static void supervisor_make_filesystem(Supervisor* supervisor);
static void supervisor_format_backup(Supervisor* supervisor);
static void supervisor_format_external(Supervisor* supervisor);

static const SupervisorWarning supervisor_warnings[] = {
    [SupervisorWarningTypeStorageNoPartitions] =
        {
            .front_text = "No partitions\nPress OK to format",
            .back_text = "Incorrect partitions\nPress OK to format\nDevice will reboot",
            .input_locked = true,
            .ok_callback = supervisor_make_filesystem,
        },
    [SupervisorWarningTypeStorageNoBackup] =
        {
            .front_text = "Backup corrupted\nPress OK to format",
            .back_text = "Backup partition corrupted\nPress OK to format\nDevice will reboot",
            .input_locked = true,
            .ok_callback = supervisor_format_backup,
        },
    [SupervisorWarningTypeStorageNoExternal] =
        {
            .front_text = "Partition corrupted\nPress OK to format",
            .back_text = "Main partition corrupted\nPress OK to format\nDevice will reboot",
            .input_locked = true,
            .ok_callback = supervisor_format_external,
        },
    [SupervisorWarningTypeBatteryNotReady] =
        {
            .front_text = "Connect battery\nAnd reset me",
            .back_text = "Connect battery\nand reset me\n>_<",
            .input_locked = true,
            .ok_callback = NULL,
        },
    [SupervisorWarningTypeBatteryLow] =
        {
            .front_text = "Battery low",
            .back_text = "Battery low",
            .input_locked = true,
            .ok_callback = NULL,
        },
};

#define SUPERVISOR_WARNINGS_SIZE COUNT_OF(supervisor_warnings)

static_assert(
    SUPERVISOR_WARNINGS_SIZE == SupervisorWarningTypeMax,
    "SupervisorWarningType enum must match the number of SupervisorWarning entries");

static_assert(
    SUPERVISOR_WARNINGS_SIZE < 32,
    "SupervisorWarningType enum must fit into a 32-bit integer");

static void supervisor_send_event(Supervisor* instance, SupervisorEventType type) {
    furi_check(instance);
    SupervisorEvent event;
    event.type = type;

    furi_check(
        furi_message_queue_put(instance->message_queue, &event, FuriWaitForever) == FuriStatusOk);
}

static void supervisor_sub_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const PowerEvent* event = message;
    Supervisor* instance = context;

    if(event->type == PowerEventBatteryLow) {
        supervisor_send_event(instance, SupervisorEventTypeBatteryLow);
    }
}

static int32_t supervisor_get_topmost_warning(SupervisorGui* gui) {
    for(uint32_t i = 0; i < SUPERVISOR_WARNINGS_SIZE; ++i) {
        if((gui->current_warnings & (1 << i)) != 0) {
            return i;
        }
    }
    return -1; // No warnings
}

static void
    supervisor_update_warning(SupervisorGui* gui, SupervisorWarningType warning_type, bool add) {
    furi_check(warning_type < SUPERVISOR_WARNINGS_SIZE);
    const SupervisorWarning* warning = &supervisor_warnings[warning_type];
    with_gui(gui->gui, {
        if(add) {
            gui->current_warnings |= (1 << warning_type);
        } else {
            gui->current_warnings &= ~(1 << warning_type);
        }

        int32_t topmost_warning_type = supervisor_get_topmost_warning(gui);

        if(topmost_warning_type >= 0) {
            gui->input_locked = warning->input_locked;
            gui->ok_callback = warning->ok_callback;

            GuiLayer* main_layer = gui_get_layer(gui->gui, GuiLayerIdSystem);

            // back display label
            {
                Widget* root = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);
                if(!gui->back_label) {
                    gui->back_label = label_alloc(root);
                }
                Widget* widget = label_get_base(gui->back_label);

                size_t screen_width_half = widget_get_width(root) / 2;
                size_t screen_height_half = widget_get_height(root) / 2;

                widget_set_padding(
                    widget,
                    screen_width_half,
                    screen_width_half,
                    screen_height_half,
                    screen_height_half);
                widget_set_align(widget, AlignCenter);
                widget_set_background_color(widget, (Color){0, 0, 0}, 0.90f);

                label_set_text_fmt(gui->back_label, warning->back_text);
                label_set_text_align(gui->back_label, TextAlignCenter);
                label_set_line_spacing(gui->back_label, 4);
            }

            // front display label
            {
                Widget* root = gui_layer_get_root_widget(main_layer, GuiDisplayIdFront);
                if(!gui->front_label) {
                    gui->front_label = label_alloc(root);
                }
                Widget* widget = label_get_base(gui->front_label);

                size_t screen_width_half = widget_get_width(root) / 2;
                size_t screen_height_half = widget_get_height(root) / 2;

                widget_set_padding(
                    widget,
                    screen_width_half,
                    screen_width_half,
                    screen_height_half,
                    screen_height_half);
                widget_set_align(widget, AlignCenter);
                widget_set_background_color(widget, (Color){0, 0, 0}, 0.90f);

                label_set_text_fmt(gui->front_label, warning->front_text);
                label_set_text_align(gui->front_label, TextAlignCenter);
            }
        } else {
            // No warnings, remove labels
            label_free(gui->front_label);
            label_free(gui->back_label);
            gui->front_label = NULL;
            gui->back_label = NULL;
            gui->input_locked = false;
            gui->ok_callback = NULL;
        }
    });
}

static bool supervisor_input(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Supervisor* instance = context;

    if(instance->gui.ok_callback) {
        if(event->type == InputTypePress && event->key == InputKeyOk) {
            supervisor_send_event(instance, SupervisorEventTypeOKPressed);
            return true;
        }
    }

    if(instance->gui.input_locked) {
        return true;
    }

    return false;
}

static void supervisor_reset(void) {
    furi_hal_nvm_reset();
    FURI_LOG_I(TAG, "Rebooting...");
    furi_delay_ms(100);
    Power* pwr = furi_record_open(RECORD_POWER);
    power_reboot(pwr, PowerRebootNormal);
    furi_record_close(RECORD_POWER);
}

static void supervisor_make_filesystem(Supervisor* supervisor) {
    SupervisorGui* gui = &supervisor->gui;
    with_gui(gui->gui, {
        label_set_text_fmt(gui->front_label, "Creating filesystem...\nPlease wait");
        label_set_text_fmt(gui->back_label, "Creating filesystem...\nPlease wait");
    });

    FURI_LOG_I(TAG, "Creating filesystem...");
    FS_Error error = storage_sd_make_filesystem(supervisor->storage, STORAGE_ROOT_PREFIX);
    if(error != FSE_OK) {
        FURI_LOG_E(TAG, "Failed to make filesystem: %s", storage_error_get_desc(error));
    } else {
        FURI_LOG_I(TAG, "Filesystem was successfully created");
    }

    supervisor_reset();
}

static void supervisor_format_backup(Supervisor* supervisor) {
    SupervisorGui* gui = &supervisor->gui;
    with_gui(gui->gui, {
        label_set_text_fmt(gui->front_label, "Formatting backup...\nPlease wait");
        label_set_text_fmt(gui->back_label, "Formatting backup partition...\nPlease wait");
    });

    FURI_LOG_I(TAG, "Formatting backup partition...");

    FS_Error error = storage_sd_format(supervisor->storage, STORAGE_BACKUP_PATH_PREFIX);
    if(error != FSE_OK) {
        FURI_LOG_E(TAG, "Failed to format backup partition: %s", storage_error_get_desc(error));
    } else {
        FURI_LOG_I(TAG, "Backup partition formatted successfully");
    }

    supervisor_reset();
}

static void supervisor_format_external(Supervisor* supervisor) {
    SupervisorGui* gui = &supervisor->gui;
    with_gui(gui->gui, {
        label_set_text_fmt(gui->front_label, "Formatting external...\nPlease wait");
        label_set_text_fmt(gui->back_label, "Formatting external partition...\nPlease wait");
    });

    FURI_LOG_I(TAG, "Formatting external partition...");

    FS_Error error = storage_sd_format(supervisor->storage, STORAGE_EXT_PATH_PREFIX);
    if(error != FSE_OK) {
        FURI_LOG_E(TAG, "Failed to format external partition: %s", storage_error_get_desc(error));
    } else {
        FURI_LOG_I(TAG, "External partition formatted successfully");
    }

    supervisor_reset();
}

static void supervisor_process(FuriEventLoopObject* object, void* context) {
    Supervisor* instance = context;
    furi_assert(object == instance->message_queue);

    SupervisorEvent event;
    if(furi_message_queue_get(instance->message_queue, &event, 0) != FuriStatusOk) {
        return;
    }

    switch(event.type) {
    case SupervisorEventTypeBatteryLow:
        FURI_LOG_I(TAG, "Battery low event received");
        if(!instance->gui.input_locked) {
            supervisor_update_warning(&instance->gui, SupervisorWarningTypeBatteryLow, true);
        }
        break;
    case SupervisorEventTypeOKPressed:
        FURI_LOG_I(TAG, "OK pressed event received");
        if(instance->gui.ok_callback) {
            instance->gui.ok_callback(instance);
        }
    }
}

int32_t supervisor_start(void* p) {
    UNUSED(p);

    Supervisor* instance = malloc(sizeof(Supervisor));
    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue = furi_message_queue_alloc(8, sizeof(SupervisorEvent));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        supervisor_process,
        instance);

    instance->power = furi_record_open(RECORD_POWER);

    UNUSED(supervisor_sub_callback);

    instance->gui.gui = furi_record_open(RECORD_GUI);
    instance->storage = furi_record_open(RECORD_STORAGE);

    gui_layer_add_input_callback(
        gui_get_layer(instance->gui.gui, GuiLayerIdSystem), supervisor_input, instance);

    if(!power_is_battery_ready(instance->power)) {
        supervisor_update_warning(&instance->gui, SupervisorWarningTypeBatteryNotReady, true);
    }

    // Check storage partitions
    Storage* storage = instance->storage;
    bool backup_exists = storage_sd_status(storage, STORAGE_BACKUP_PATH_PREFIX) == FSE_OK;
    bool external_exists = storage_sd_status(storage, STORAGE_EXT_PATH_PREFIX) == FSE_OK;

    if(!backup_exists && !external_exists) {
        FURI_LOG_E(TAG, "No partitions found");
        supervisor_update_warning(&instance->gui, SupervisorWarningTypeStorageNoPartitions, true);
    } else if(!backup_exists) {
        FURI_LOG_E(TAG, "Backup partition not found");
        supervisor_update_warning(&instance->gui, SupervisorWarningTypeStorageNoBackup, true);
    } else if(!external_exists) {
        FURI_LOG_E(TAG, "External partition not found");
        supervisor_update_warning(&instance->gui, SupervisorWarningTypeStorageNoExternal, true);
    } else {
        FURI_LOG_I(TAG, "All partitions are OK");
    }

    furi_event_loop_run(instance->event_loop);

    return 0;
}
