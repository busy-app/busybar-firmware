#include <furi/furi.h>
#include <furi_hal_nvm.h>
#include <power/power_service/power.h>
#include <gui/gui.h>
#include <gui/modules/label.h>
#include <settings_helpers/status_view.h>
#include <storage/storage.h>
#include <storage/storage_backup.h>
#include <intercom/intercom.h>
#include <matter/matter.h>

#define TAG "Supervisor"

#define SUPERVISOR_BATTERY_LOW_TIMEOUT_MS 5000
#define SUPERVISOR_BATTERY_TIME_TO_DIE_S  30

#define SUPERVISOR_REBOOT_GRACE_PERIOD_MS (30000)

typedef struct Supervisor Supervisor;

typedef void (*SupervisorGuiOkCb)(Supervisor* supervisor);

typedef struct {
    Gui* gui;
    struct {
        Label* front;
        Label* back;
    } label;
    struct {
        StatusView* front;
        StatusView* back;
    } status_view;
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
    Matter* matter;

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
    SupervisorEventTypeIntercomStatusChanged,
    SupervisorEventTypeOKPressed,
    SupervisorEventTypeWillReboot,
} SupervisorEventType;

typedef struct {
    SupervisorEventType type;
    union {
        IntercomStatus intercom_status;
    };
} SupervisorEvent;

typedef enum {
    SupervisorWarningStyleTextOnly,
    SupervisorWarningStyleTextAndImage,
} SupervisorWarningStyle;

typedef struct {
    SupervisorWarningStyle style;
    struct {
        const char* front;
        const char* back;
    } text;
    union {
        struct {
            const char* front;
            const char* back;
        } image;
    };
    bool input_locked;
    SupervisorGuiOkCb ok_callback;
} SupervisorWarning;

// if multiple warnings are active, the one with the lowest enum value is shown
typedef enum {
    SupervisorWarningTypeBatteryNotReady,
    SupervisorWarningTypeBatteryCritical, // must be higher than others to power off properly
    SupervisorWarningTypeRebooting,
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
            .style = SupervisorWarningStyleTextOnly,
            .text.front = "No partitions\nPress OK to format",
            .text.back = "Incorrect partitions\nPress OK to format\nDevice will reboot",
            .input_locked = true,
            .ok_callback = supervisor_make_filesystem,
        },
    [SupervisorWarningTypeRebooting] =
        {
            .style = SupervisorWarningStyleTextAndImage,
            .image.front = SHARED_ANIM_PATH("spinner_front_8x8.anim"),
            .image.back = SHARED_ANIM_PATH("spinner_back_16x16.anim"),
            .text.front = "Restarting device...",
            .text.back = "Restarting device...",
            .input_locked = true,
            .ok_callback = NULL,
        },
    [SupervisorWarningTypeStorageNoBackup] =
        {
            .style = SupervisorWarningStyleTextOnly,
            .text.front = "Backup corrupted\nPress OK to format",
            .text.back = "Backup partition corrupted\nPress OK to format\nDevice will reboot",
            .input_locked = true,
            .ok_callback = supervisor_format_backup,
        },
    [SupervisorWarningTypeStorageNoExternal] =
        {
            .style = SupervisorWarningStyleTextOnly,
            .text.front = "Partition corrupted\nPress OK to format",
            .text.back = "Main partition corrupted\nPress OK to format\nDevice will reboot",
            .input_locked = true,
            .ok_callback = supervisor_format_external,
        },
    [SupervisorWarningTypeBatteryNotReady] =
        {
            .style = SupervisorWarningStyleTextOnly,
            .text.front = "Battery not present\nConnect battery",
            .text.back = "Battery not present\nPlease connect battery\n>_<",
            .input_locked = true,
            .ok_callback = NULL,
        },
    [SupervisorWarningTypeBatteryCritical] =
        {
            .style = SupervisorWarningStyleTextOnly,
            .text.front = "Connect charger\nPower off in 30 sec.",
            .text.back = "Battery critical\nPlease connect charger\nPower off in 30 sec.",
            .input_locked = false,
            .ok_callback = NULL,
        },
    [SupervisorWarningTypeBatteryLow] =
        {
            .style = SupervisorWarningStyleTextOnly,
            .text.front = "Battery low",
            .text.back = "Battery low",
            .input_locked = false,
            .ok_callback = NULL,
        },
    [SupervisorWarningTypeIntercomError] =
        {
            .style = SupervisorWarningStyleTextOnly,
            .text.front = "Intercom error\nReboot device",
            .text.back = "Intercom error\nPlease reboot the device",
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

static void supervisor_send_event_ex(Supervisor* instance, const SupervisorEvent* event) {
    furi_check(
        furi_message_queue_put(instance->message_queue, event, FuriWaitForever) == FuriStatusOk);
}

static void supervisor_send_event(Supervisor* instance, SupervisorEventType type) {
    furi_check(instance);

    const SupervisorEvent event = {
        .type = type,
    };

    supervisor_send_event_ex(instance, &event);
}

static void supervisor_intercom_state_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    Supervisor* instance = context;

    const SupervisorEvent event = {
        .type = SupervisorEventTypeIntercomStatusChanged,
        .intercom_status = *(IntercomStatus*)message,
    };

    supervisor_send_event_ex(instance, &event);
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
    case PowerEventChargeAmountUpdate:
        /* fall-through */
    case PowerEventUsbConnectionStateUpdate:
        /* fall-through */
    case PowerEventShutdown:
        break;
    }
}

static void supervisor_matter_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const MatterEvent* event = message;
    Supervisor* instance = context;

    switch(event->type) {
    case MatterEventTypeWillReboot:
        supervisor_send_event(instance, SupervisorEventTypeWillReboot);
        break;
    default:
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

/**
 * @note `warning` allowed to be `NULL`
 */
static void supervisor_set_label(
    SupervisorGui* gui,
    GuiDisplayId display,
    const SupervisorWarning* warning) {
    furi_assert(gui);

    bool labels_needed = false;
    if(warning) labels_needed = warning->style == SupervisorWarningStyleTextOnly;
    Label** label = (display == GuiDisplayIdFront) ? &gui->label.front : &gui->label.back;

    GuiLayer* main_layer = gui_get_layer(gui->gui, GuiLayerIdSystem);
    Widget* root = gui_layer_get_root_widget(main_layer, display);

    if(labels_needed && !*label) *label = label_alloc(root);
    if(!labels_needed && *label) {
        label_free(*label);
        *label = NULL;
    }

    if(!labels_needed) return;
    furi_assert(warning);
    furi_assert(warning->text.front);
    furi_assert(warning->text.back);

    const char* text = (display == GuiDisplayIdFront) ? warning->text.front : warning->text.back;

    Widget* widget = label_get_base(*label);
    Color background = COLOR_MAKE_HEXA(0x000000E5);

    size_t screen_width_half = widget_get_width(root) / 2;
    size_t screen_height_half = widget_get_height(root) / 2;

    widget_set_padding(
        widget, screen_width_half, screen_width_half, screen_height_half, screen_height_half);
    widget_set_align(widget, AlignCenter);
    widget_set_background_color(widget, background);

    label_set_text(*label, text);
    label_set_text_align(*label, TextAlignCenter);

    if(display == GuiDisplayIdBack) {
        label_set_line_spacing(*label, 4);
    }
}

/**
 * @note `warning` allowed to be `NULL`
 */
static void supervisor_set_status_view(
    SupervisorGui* gui,
    GuiDisplayId display,
    const SupervisorWarning* warning) {
    furi_assert(gui);

    bool status_needed = false;
    if(warning) status_needed = warning->style == SupervisorWarningStyleTextAndImage;
    StatusView** status = (display == GuiDisplayIdFront) ? &gui->status_view.front :
                                                           &gui->status_view.back;

    GuiLayer* main_layer = gui_get_layer(gui->gui, GuiLayerIdSystem);
    Widget* root = gui_layer_get_root_widget(main_layer, display);

    if(status_needed && !*status) *status = status_view_alloc(root);
    if(!status_needed && *status) {
        status_view_free(*status);
        *status = NULL;
    }

    if(!status_needed) return;
    furi_assert(warning);
    furi_assert(warning->text.front);
    furi_assert(warning->text.back);
    furi_assert(warning->image.front);
    furi_assert(warning->image.back);

    const char* text = (display == GuiDisplayIdFront) ? warning->text.front : warning->text.back;
    const char* image = (display == GuiDisplayIdFront) ? warning->image.front :
                                                         warning->image.back;

    status_view_set_icon(*status, image);
    status_view_set_header(*status, text);

    Widget* widget = status_view_get_base(*status);
    Color background = COLOR_MAKE_HEXA(0x000000E5);
    widget_set_background_color(widget, background);
}

/**
 * @note `warning` allowed to be `NULL`
 */
static void supervisor_render_warning(SupervisorGui* gui, const SupervisorWarning* warning) {
    furi_assert(gui);
    for(GuiDisplayId display = GuiDisplayIdFront; display < GuiDisplayIdMax; display++) {
        supervisor_set_label(gui, display, warning);
        supervisor_set_status_view(gui, display, warning);
    }
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

            supervisor_render_warning(gui, warning);

        } else {
            gui->input_locked = false;
            gui->ok_callback = NULL;
            supervisor_render_warning(gui, NULL);
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
        label_set_text_fmt(gui->label.front, "Creating filesystem...\nPlease wait");
        label_set_text_fmt(gui->label.back, "Creating filesystem...\nPlease wait");
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
        label_set_text_fmt(gui->label.front, "Formatting backup...\nPlease wait");
        label_set_text_fmt(gui->label.back, "Formatting backup partition...\nPlease wait");
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
        label_set_text_fmt(gui->label.front, "Formatting external...\nPlease wait");
        label_set_text_fmt(gui->label.back, "Formatting external partition...\nPlease wait");
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
        if(gui->label.front && gui->label.back) {
            label_set_text_fmt(
                gui->label.front, "Connect charger\nPower off in %zu sec.", seconds);
            label_set_text_fmt(
                gui->label.back,
                "Battery critical\nPlease connect charger\nPower off in %zu sec.",
                seconds);
        }
    });
}

static void supervisor_handle_intercom_status(Supervisor* instance, IntercomStatus status) {
    if(status == IntercomStatusUnknown || status == IntercomStatusOk) {
        return;
    }

    FURI_LOG_E(TAG, "Intercom error received: 0x%X", status);

    if(status != IntercomStatusErrorSync && !furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug)) {
        if(furi_get_tick() > furi_ms_to_ticks(SUPERVISOR_REBOOT_GRACE_PERIOD_MS)) {
            FURI_LOG_I(TAG, "Rebooting...");
            furi_delay_ms(100);

            power_reboot(instance->power, PowerRebootNormal);
        }
    }

    supervisor_update_warning(&instance->gui, SupervisorWarningTypeIntercomError, true);
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
    case SupervisorEventTypeIntercomStatusChanged: {
        supervisor_handle_intercom_status(instance, event.intercom_status);
    } break;
    case SupervisorEventTypeWillReboot:
        FURI_LOG_I(TAG, "Will Reboot event received");
        supervisor_update_warning(&instance->gui, SupervisorWarningTypeRebooting, true);
        break;
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
    instance->matter = furi_record_open(RECORD_MATTER);

    furi_pubsub_subscribe(power_get_pubsub(instance->power), supervisor_power_callback, instance);
    furi_state_subscribe(
        intercom_get_state(instance->intercom), supervisor_intercom_state_callback, instance);

    gui_layer_add_input_callback(
        gui_get_layer(instance->gui.gui, GuiLayerIdSystem), supervisor_input, instance);

    furi_pubsub_subscribe(
        matter_get_pubsub(instance->matter), supervisor_matter_callback, instance);

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
