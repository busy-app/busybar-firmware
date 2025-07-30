#include <furi/furi.h>
#include <furi_hal_nvm.h>
#include <power/power_service/power.h>
#include <gui/gui.h>
#include <gui/modules/label.h>
#include <storage/storage.h>
#include <storage/storage_backup.h>
#include <intercom/intercom.h>

#define TAG "Supervisor"

#define SUPERVISOR_BATTERY_LOW_TIMEOUT_MS 5000
#define SUPERVISOR_BATTERY_TIME_TO_DIE_S  30

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
    FuriEventLoopTimer* battery_low_timer;
    FuriEventLoopTimer* battery_critical_timer;
    size_t battery_critical_counter;
    Power* power;
    Storage* storage;
    Intercom* intercom;

    SupervisorGui gui;
};

typedef enum {
    SupervisorEventTypeBatteryCriticalStart,
    SupervisorEventTypeBatteryCriticalStop,
    SupervisorEventTypeBatteryLowStart,
    SupervisorEventTypeBatteryLowStop,
    SupervisorEventTypeBatteryNotPresent,
    SupervisorEventTypeBatteryPresent,
    SupervisorEventTypeTickToDie,
    SupervisorEventTypeIntercomError,
    SupervisorEventTypeOKPressed,
} SupervisorEventType;

typedef struct {
    SupervisorEventType type;
    union {
        FuriString* message; // must be deallocated by the receiver
    };
} SupervisorEvent;

typedef struct {
    const char* front_text;
    const char* back_text;
    bool input_locked;
    SupervisorGuiOkCb ok_callback;
} SupervisorWarning;

// if multiple warnings are active, the one with the lowest enum value is shown
typedef enum {
    SupervisorWarningTypeBatteryNotReady,
    SupervisorWarningTypeBatteryCritical, // must be higher than others to power off properly
    SupervisorWarningTypeStorageNoPartitions,
    SupervisorWarningTypeStorageNoBackup,
    SupervisorWarningTypeStorageNoExternal,
    SupervisorWarningTypeBatteryLow,
    SupervisorWarningTypeIntercomError, // must be last

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
            .front_text = "Battery not present\nConnect battery",
            .back_text = "Battery not present\nPlease connect battery\n>_<",
            .input_locked = true,
            .ok_callback = NULL,
        },
    [SupervisorWarningTypeBatteryCritical] =
        {
            .front_text = "Connect charger\nPower off in 30 sec.",
            .back_text = "Battery critical\nPlease connect charger\nPower off in 30 sec.",
            .input_locked = false,
            .ok_callback = NULL,
        },
    [SupervisorWarningTypeBatteryLow] =
        {
            .front_text = "Battery low",
            .back_text = "Battery low",
            .input_locked = false,
            .ok_callback = NULL,
        },
    [SupervisorWarningTypeIntercomError] =
        {
            .front_text = "Intercom error\nUpdate firmware",
            .back_text = "Intercom error\nPlease update firmware",
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

static void supervisor_send_event_with_message(
    Supervisor* instance,
    SupervisorEventType type,
    const char* message) {
    furi_check(instance);
    SupervisorEvent event;
    event.type = type;
    event.message = furi_string_alloc_set(message);

    furi_check(
        furi_message_queue_put(instance->message_queue, &event, FuriWaitForever) == FuriStatusOk);
}

static void supervisor_intercom_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const IntercomEvent* event = message;
    Supervisor* instance = context;

    if(event->type == IntercomEventTypeError) {
        supervisor_send_event_with_message(
            instance, SupervisorEventTypeIntercomError, event->message);
    }
}

static void supervisor_power_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const PowerEvent* event = message;
    Supervisor* instance = context;

    switch(event->type) {
    case PowerEventBatteryLowStart:
        supervisor_send_event(instance, SupervisorEventTypeBatteryLowStart);
        break;
    case PowerEventBatteryLowStop:
        supervisor_send_event(instance, SupervisorEventTypeBatteryLowStop);
        break;
    case PowerEventBatteryCriticalStart:
        supervisor_send_event(instance, SupervisorEventTypeBatteryCriticalStart);
        break;
    case PowerEventBatteryCriticalStop:
        supervisor_send_event(instance, SupervisorEventTypeBatteryCriticalStop);
        break;
    case PowerEventBatteryNotPresent:
        supervisor_send_event(instance, SupervisorEventTypeBatteryNotPresent);
        break;
    case PowerEventBatteryPresent:
        supervisor_send_event(instance, SupervisorEventTypeBatteryPresent);
        break;
    case PowerEventBatteryNormalStart:
        /* fall-through */
    case PowerEventBatteryNormalStop:
        /* fall-through */
    case PowerEventChargingStateUpdate:
        /* fall-through */
    case PowerEventChargeUpdate:
        /* fall-through */
    case PowerEventUsbConnectionStateUpdate:
        break;
    }
}

static void supervisor_timer_bat_low_callback(void* context) {
    Supervisor* instance = context;
    supervisor_send_event(instance, SupervisorEventTypeBatteryLowStop);
}

static void supervisor_timer_bat_critical_callback(void* context) {
    Supervisor* instance = context;
    supervisor_send_event(instance, SupervisorEventTypeTickToDie);
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
    with_gui(gui->gui, {
        if(add) {
            gui->current_warnings |= (1 << warning_type);
        } else {
            gui->current_warnings &= ~(1 << warning_type);
        }

        int32_t topmost_warning_type = supervisor_get_topmost_warning(gui);

        if(topmost_warning_type >= 0) {
            const SupervisorWarning* warning = &supervisor_warnings[topmost_warning_type];

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
            if(gui->front_label) {
                label_free(gui->front_label);
                gui->front_label = NULL;
            }

            if(gui->back_label) {
                label_free(gui->back_label);
                gui->back_label = NULL;
            }

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

static void supervisor_update_time_to_die(Supervisor* supervisor, size_t seconds) {
    SupervisorGui* gui = &supervisor->gui;
    with_gui(gui->gui, {
        if(gui->front_label && gui->back_label) {
            label_set_text_fmt(
                gui->front_label, "Connect charger\nPower off in %zu sec.", seconds);
            label_set_text_fmt(
                gui->back_label,
                "Battery critical\nPlease connect charger\nPower off in %zu sec.",
                seconds);
        }
    });
}

static void supervisor_process(FuriEventLoopObject* object, void* context) {
    Supervisor* instance = context;
    furi_assert(object == instance->message_queue);

    SupervisorEvent event;
    if(furi_message_queue_get(instance->message_queue, &event, 0) != FuriStatusOk) {
        return;
    }

    switch(event.type) {
    case SupervisorEventTypeBatteryLowStart:
        FURI_LOG_I(TAG, "Battery low warning received");
        supervisor_update_warning(&instance->gui, SupervisorWarningTypeBatteryLow, true);
        furi_event_loop_timer_start(
            instance->battery_low_timer, SUPERVISOR_BATTERY_LOW_TIMEOUT_MS);
        break;
    case SupervisorEventTypeBatteryLowStop:
        FURI_LOG_I(TAG, "Clearing battery low warning");
        furi_event_loop_timer_stop(instance->battery_low_timer);
        supervisor_update_warning(&instance->gui, SupervisorWarningTypeBatteryLow, false);
        break;
    case SupervisorEventTypeBatteryCriticalStart:
        FURI_LOG_I(TAG, "Battery critical warning received");
        supervisor_update_warning(&instance->gui, SupervisorWarningTypeBatteryCritical, true);
        furi_event_loop_timer_start(instance->battery_critical_timer, 1000); // 1 second interval
        instance->battery_critical_counter = 0;
        break;
    case SupervisorEventTypeBatteryCriticalStop:
        FURI_LOG_I(TAG, "Clearing battery critical warning");
        furi_event_loop_timer_stop(instance->battery_critical_timer);
        supervisor_update_warning(&instance->gui, SupervisorWarningTypeBatteryCritical, false);
        break;
    case SupervisorEventTypeBatteryNotPresent:
        FURI_LOG_I(TAG, "Battery not present warning received");
        supervisor_update_warning(&instance->gui, SupervisorWarningTypeBatteryNotReady, true);
        break;
    case SupervisorEventTypeBatteryPresent:
        FURI_LOG_I(TAG, "Battery present event received");
        supervisor_update_warning(&instance->gui, SupervisorWarningTypeBatteryNotReady, false);
        break;
    case SupervisorEventTypeOKPressed:
        FURI_LOG_I(TAG, "OK pressed event received");
        if(instance->gui.ok_callback) {
            instance->gui.ok_callback(instance);
        }
        break;
    case SupervisorEventTypeTickToDie: {
        size_t topmost_warning_type = supervisor_get_topmost_warning(&instance->gui);

        // with the assumption that only BatteryNotPresent warning has higher priority
        // this will garantee that we will power off in any other state
        if(topmost_warning_type == SupervisorWarningTypeBatteryCritical) {
            instance->battery_critical_counter++;

            supervisor_update_time_to_die(
                instance, SUPERVISOR_BATTERY_TIME_TO_DIE_S - instance->battery_critical_counter);

            if(instance->battery_critical_counter >= SUPERVISOR_BATTERY_TIME_TO_DIE_S) {
                FURI_LOG_I(TAG, "Battery critical timeout reached");
                if(!power_off(instance->power)) {
                    FURI_LOG_E(TAG, "Power off failed");
                }
            }
        }
    } break;
    case SupervisorEventTypeIntercomError: {
        FURI_LOG_E(TAG, "Intercom error received: %s", furi_string_get_cstr(event.message));
        supervisor_update_warning(&instance->gui, SupervisorWarningTypeIntercomError, true);
        size_t topmost_warning_type = supervisor_get_topmost_warning(&instance->gui);
        if(topmost_warning_type == SupervisorWarningTypeIntercomError) {
            with_gui(instance->gui.gui, {
                label_set_text_fmt(
                    instance->gui.back_label,
                    "Intercom error\nPlease update firmware\n\"%s\"",
                    furi_string_get_cstr(event.message));
            });
        }
        furi_string_free(event.message);
    } break;
    }
}

int32_t supervisor_start(void* p) {
    UNUSED(p);

    Supervisor* instance = malloc(sizeof(Supervisor));
    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue = furi_message_queue_alloc(8, sizeof(SupervisorEvent));
    instance->battery_low_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        supervisor_timer_bat_low_callback,
        FuriEventLoopTimerTypeOnce,
        instance);
    instance->battery_critical_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        supervisor_timer_bat_critical_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        supervisor_process,
        instance);

    instance->power = furi_record_open(RECORD_POWER);

    instance->gui.gui = furi_record_open(RECORD_GUI);
    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    furi_pubsub_subscribe(power_get_pubsub(instance->power), supervisor_power_callback, instance);
    furi_pubsub_subscribe(
        intercom_get_pubsub(instance->intercom), supervisor_intercom_callback, instance);

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
