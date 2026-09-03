#include <gui/gui_i.h>
#include <gui/modules/status_view.h>

#include <low_power/low_power.h>
#include <intercom/intercom.h>
#include <matter/matter.h>

#ifdef SRV_LOG_STORAGE
#include <log_storage/log_storage.h>
#endif

#include <furi_hal_nvm.h>

#include <assets_images.h>

#define TAG "Supervisor"

#define SUPERVISOR_BATTERY_TIME_TO_DIE_S  30
#define SUPERVISOR_REBOOT_GRACE_PERIOD_MS 30000
#define SUPERVISOR_FAILURE_LOG_DUMP_PATH  "/ext/intercom_failure_log.txt"

typedef struct Supervisor Supervisor;

typedef struct {
    const char* primary_text;
    const char* auxiliary_text;

    union {
        const void* as_any;
        const char* as_path;
        const lv_image_dsc_t* as_image;
    } icon;

    bool is_icon_animated;
} SupervisorUiPreset;

typedef struct {
    SupervisorUiPreset ui_presets[GuiDisplayIdMax];
    void (*ok_callback)(Supervisor* supervisor, const void* context);
    const void* context;
    bool do_lock_input;
} SupervisorWarning;

struct Supervisor {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* battery_critical_timer;
    FuriMessageQueue* message_queue;

    Power* power;
    Storage* storage;
    Intercom* intercom;
    Matter* matter;
    Gui* gui;
    LowPower* low_power;

    StatusView* status_views[GuiDisplayIdMax];

    uint32_t active_warnings;
    const SupervisorWarning* _Atomic displayed_warning;

    size_t battery_critical_counter;
};

typedef enum {
    SupervisorEventTypeBatteryCriticalStart,
    SupervisorEventTypeBatteryCriticalStop,
    SupervisorEventTypeBatteryNotPresent,
    SupervisorEventTypeBatteryPresent,
    SupervisorEventTypeTickToDie,
    SupervisorEventTypeIntercomStatusChanged,
    SupervisorEventTypeOkPressed,
    SupervisorEventTypeWillReboot,
} SupervisorEventType;

typedef struct {
    SupervisorEventType type;
    union {
        IntercomStatus intercom_status;
    };
} SupervisorEvent;

// if multiple warnings are active, the one with the lowest enum value is shown
typedef enum {
    SupervisorWarningTypeBatteryNotReady,
    SupervisorWarningTypeBatteryCritical, // must be higher than others to power off properly
    SupervisorWarningTypeRebooting,
    SupervisorWarningTypeStorageNoPartitions,
    SupervisorWarningTypeStorageNoBackup,
    SupervisorWarningTypeStorageNoExternal,
    SupervisorWarningTypeIntercomError, // must be last

    SupervisorWarningTypesCount,
} SupervisorWarningType;

static void supervisor_make_filesystem(Supervisor* supervisor, const void* context);
static void supervisor_format_partition(Supervisor* supervisor, const void* context);

static const SupervisorWarning supervisor_warnings[] = {
    [SupervisorWarningTypeBatteryNotReady] =
        {
            .ui_presets =
                {
                    [GuiDisplayIdFront] =
                        {
                            .primary_text = "Battery issue,\ncontact support",
                            .auxiliary_text = NULL,
                            .icon.as_path = SHARED_IMG_PATH("missing_battery_front_8x8.image"),
                            .is_icon_animated = false,
                        },
                    [GuiDisplayIdBack] =
                        {
                            .primary_text = "Battery issue",
                            .auxiliary_text = "Please contact support",
                            .icon.as_path = SHARED_IMG_PATH("error_back_11x11.image"),
                            .is_icon_animated = false,
                        },
                },
            .ok_callback = NULL,
            .do_lock_input = true,
        },
    [SupervisorWarningTypeBatteryCritical] =
        {
            .ui_presets =
                {
                    [GuiDisplayIdFront] =
                        {
                            .primary_text = "Connect charger",
                            .auxiliary_text = "Power off in 00:30",
                            .icon.as_path = SHARED_IMG_PATH("low_battery_front_8x8.image"),
                            .is_icon_animated = false,
                        },
                    [GuiDisplayIdBack] =
                        {
                            .primary_text = "Connect charger",
                            .auxiliary_text = "Power off in 00:30",
                            .icon.as_path = SHARED_IMG_PATH("error_back_11x11.image"),
                            .is_icon_animated = false,
                        },
                },
            .ok_callback = NULL,
            .do_lock_input = false,
        },
    [SupervisorWarningTypeRebooting] =
        {
            .ui_presets =
                {
                    [GuiDisplayIdFront] =
                        {
                            .primary_text = "Restarting device",
                            .auxiliary_text = NULL,
                            .icon.as_path = SHARED_ANIM_PATH("spinner_front_8x8.anim"),
                            .is_icon_animated = true,
                        },
                    [GuiDisplayIdBack] =
                        {
                            .primary_text = "Restarting device",
                            .auxiliary_text = NULL,
                            .icon.as_path = SHARED_ANIM_PATH("spinner_back_16x16.anim"),
                            .is_icon_animated = true,
                        },
                },
            .ok_callback = NULL,
            .do_lock_input = true,
        },
    [SupervisorWarningTypeStorageNoPartitions] =
        {
            .ui_presets =
                {
                    [GuiDisplayIdFront] =
                        {
                            .primary_text = "Storage error\nOK = reset device",
                            .auxiliary_text = NULL,
                            .icon.as_image = &I_error_front_8x8,
                            .is_icon_animated = false,
                        },
                    [GuiDisplayIdBack] =
                        {
                            .primary_text = "Storage error,\npress OK to reset device",
                            .auxiliary_text = NULL,
                            .icon.as_image = &I_error_back_11x11,
                            .is_icon_animated = false,
                        },
                },
            .ok_callback = supervisor_make_filesystem,
            .do_lock_input = true,
        },
    [SupervisorWarningTypeStorageNoBackup] =
        {
            .ui_presets =
                {
                    [GuiDisplayIdFront] =
                        {
                            .primary_text = "Storage error\nOK = reset device",
                            .auxiliary_text = NULL,
                            .icon.as_path = SHARED_IMG_PATH("error_front_8x8.image"),
                            .is_icon_animated = false,
                        },
                    [GuiDisplayIdBack] =
                        {
                            .primary_text = "Storage error,\npress OK to reset device",
                            .auxiliary_text = NULL,
                            .icon.as_path = SHARED_IMG_PATH("error_back_11x11.image"),
                            .is_icon_animated = false,
                        },
                },
            .ok_callback = supervisor_format_partition,
            .context = STORAGE_BACKUP_PATH_PREFIX,
            .do_lock_input = true,
        },
    [SupervisorWarningTypeStorageNoExternal] =
        {
            .ui_presets =
                {
                    [GuiDisplayIdFront] =
                        {
                            .primary_text = "Storage error\nOK = reset device",
                            .auxiliary_text = NULL,
                            .icon.as_image = &I_error_front_8x8,
                            .is_icon_animated = false,
                        },
                    [GuiDisplayIdBack] =
                        {
                            .primary_text = "Storage error,\npress OK to reset device",
                            .auxiliary_text = NULL,
                            .icon.as_image = &I_error_back_11x11,
                            .is_icon_animated = false,
                        },
                },
            .ok_callback = supervisor_format_partition,
            .context = STORAGE_EXT_PATH_PREFIX,
            .do_lock_input = true,
        },
    [SupervisorWarningTypeIntercomError] =
        {
            .ui_presets =
                {
                    [GuiDisplayIdFront] =
                        {
                            .primary_text = "System error,\nrestart device",
                            .auxiliary_text = NULL,
                            .icon.as_path = SHARED_IMG_PATH("error_front_8x8.image"),
                            .is_icon_animated = false,
                        },
                    [GuiDisplayIdBack] =
                        {
                            .primary_text = "System error",
                            .auxiliary_text = "To restart device, hold\nBACK + START buttons",
                            .icon.as_path = SHARED_IMG_PATH("error_back_11x11.image"),
                            .is_icon_animated = false,
                        },
                },
            .ok_callback = NULL,
            .do_lock_input = true,
        },
};

static_assert(
    COUNT_OF(supervisor_warnings) == SupervisorWarningTypesCount,
    "SupervisorWarningType enum must match the number of SupervisorWarning entries");

static_assert(
    COUNT_OF(supervisor_warnings) < sizeof(uint32_t) * CHAR_BIT,
    "SupervisorWarningType enum must fit into a 32-bit integer");

static void supervisor_send_event_ex(Supervisor* instance, const SupervisorEvent* event) {
    furi_check(
        furi_message_queue_put(instance->message_queue, event, FuriWaitForever) == FuriStatusOk);
}

static void supervisor_send_event(Supervisor* instance, SupervisorEventType type) {
    furi_check(instance);

    supervisor_send_event_ex(
        instance,
        &(SupervisorEvent){
            .type = type,
        });
}

static void supervisor_intercom_state_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    Supervisor* instance = context;

    supervisor_send_event_ex(
        instance,
        &(SupervisorEvent){
            .type = SupervisorEventTypeIntercomStatusChanged,
            .intercom_status = *(IntercomStatus*)message,
        });
}

static void supervisor_power_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const PowerEvent* event = message;
    Supervisor* instance = context;

    SupervisorEventType supervisor_event;
    switch(event->type) {
    case PowerEventBatteryCriticalStart:
        supervisor_event = SupervisorEventTypeBatteryCriticalStart;
        break;

    case PowerEventBatteryCriticalStop:
        supervisor_event = SupervisorEventTypeBatteryCriticalStop;
        break;

    case PowerEventBatteryNotPresent:
        supervisor_event = SupervisorEventTypeBatteryNotPresent;
        break;

    case PowerEventBatteryPresent:
        supervisor_event = SupervisorEventTypeBatteryPresent;
        break;

    default:
        return;
    }

    supervisor_send_event(instance, supervisor_event);
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

static void supervisor_timer_bat_critical_callback(void* context) {
    furi_assert(context);

    Supervisor* instance = context;

    supervisor_send_event(instance, SupervisorEventTypeTickToDie);
}

static int32_t supervisor_get_topmost_warning(Supervisor* instance) {
    furi_assert(instance);

    if(instance->active_warnings == 0) return -1;

    uint32_t idx = __builtin_ctz(instance->active_warnings);
    return (idx < COUNT_OF(supervisor_warnings)) ? (int32_t)idx : -1;
}

static void supervisor_update_warning(Supervisor* instance, SupervisorWarningType type, bool add) {
    furi_check(type < COUNT_OF(supervisor_warnings));

    if(add) {
        if(instance->active_warnings == 0) {
            low_power_lock(instance->low_power);
        }
        instance->active_warnings |= (1 << type);
    } else {
        instance->active_warnings &= ~(1 << type);
        if(instance->active_warnings == 0) {
            low_power_unlock(instance->low_power);
        }
    }

    int32_t topmost_warning_idx = supervisor_get_topmost_warning(instance);
    const SupervisorWarning* topmost_warning =
        (topmost_warning_idx >= 0) ? &supervisor_warnings[topmost_warning_idx] : NULL;

    with_gui(instance->gui, {
        if(topmost_warning) {
            for(GuiDisplayId display = GuiDisplayIdFront; display < GuiDisplayIdMax; display++) {
                StatusView* status_view = instance->status_views[display];
                const SupervisorUiPreset* ui_preset = &topmost_warning->ui_presets[display];

                widget_set_visible(status_view_get_base(status_view), true);
                status_view_set_primary_text(status_view, ui_preset->primary_text);
                status_view_set_auxiliary_text(status_view, ui_preset->auxiliary_text);
                status_view_set_icon(
                    status_view, ui_preset->icon.as_any, ui_preset->is_icon_animated);
            }
        } else {
            for(GuiDisplayId display = GuiDisplayIdFront; display < GuiDisplayIdMax; display++) {
                StatusView* status_view = instance->status_views[display];

                widget_set_visible(status_view_get_base(status_view), false);
                status_view_set_primary_text(status_view, NULL);
                status_view_set_auxiliary_text(status_view, NULL);
                status_view_set_icon(status_view, NULL, false);
            }
        }
    });

    instance->displayed_warning = topmost_warning;
}

static bool supervisor_input(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    Supervisor* instance = context;

    /* thread-safe: `displayed_warning` is an atomic pointer to a constant */
    const SupervisorWarning* warning = instance->displayed_warning;

    if(!warning) return false;

    if(warning->ok_callback && event->type == InputTypePress) {
        switch(event->key) {
        case InputKeyOk:
        /* fall-through */
        case InputKeyStart:
            supervisor_send_event(instance, SupervisorEventTypeOkPressed);
            return true;

        default:
            break;
        }
    }

    return warning->do_lock_input;
}

static void supervisor_reset(Supervisor* supervisor) {
    furi_hal_nvm_reset();

    FURI_LOG_I(TAG, "Rebooting...");

    furi_delay_ms(100);
    power_reboot(supervisor->power, PowerRebootNormal);
}

static void supervisor_make_filesystem(Supervisor* instance, const void* context) {
    UNUSED(context);

    furi_assert(instance);

    with_gui(instance->gui, {
        for(GuiDisplayId display = GuiDisplayIdFront; display < GuiDisplayIdMax; display++) {
            StatusView* status_view = instance->status_views[display];

            status_view_set_primary_text(status_view, "Resetting device...\nPlease wait");
            status_view_set_auxiliary_text(status_view, NULL);
            status_view_set_icon(status_view, NULL, false);
        }
    });

    FURI_LOG_I(TAG, "Creating filesystem...");

    FS_Error fs_error = storage_sd_make_filesystem(instance->storage, STORAGE_ROOT_PREFIX);
    if(fs_error != FSE_OK) {
        FURI_LOG_E(TAG, "Failed to create filesystem: %s", storage_error_get_desc(fs_error));
    } else {
        FURI_LOG_I(TAG, "Filesystem was successfully created");
    }

    supervisor_reset(instance);
}

static void supervisor_format_partition(Supervisor* instance, const void* context) {
    furi_assert(instance);
    furi_assert(context);

    const char* path = context;

    with_gui(instance->gui, {
        for(GuiDisplayId display = GuiDisplayIdFront; display < GuiDisplayIdMax; display++) {
            StatusView* status_view = instance->status_views[display];

            status_view_set_primary_text(status_view, "Resetting device...\nPlease wait");
            status_view_set_auxiliary_text(status_view, NULL);
            status_view_set_icon(status_view, NULL, false);
        }
    });

    FURI_LOG_I(TAG, "Formatting %s partition...", path);

    FS_Error fs_error = storage_sd_format(instance->storage, path);
    if(fs_error != FSE_OK) {
        FURI_LOG_E(
            TAG, "Failed to format %s partition: %s", path, storage_error_get_desc(fs_error));
    } else {
        FURI_LOG_I(TAG, "Partition %s formatted successfully", path);
    }

    supervisor_reset(instance);
}

static void supervisor_update_time_to_die(Supervisor* instance, size_t seconds) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Power off in %02zu:%02zu", seconds / 60, seconds % 60);

    with_gui(instance->gui, {
        for(GuiDisplayId display = GuiDisplayIdFront; display < GuiDisplayIdMax; display++) {
            status_view_set_auxiliary_text(instance->status_views[display], buffer);
        }
    });
}

static void supervisor_handle_intercom_status(Supervisor* instance, IntercomStatus status) {
    if(status == IntercomStatusUnknown || status == IntercomStatusOk) {
        return;
    }

    FURI_LOG_E(TAG, "Intercom error received: 0x%X", status);

#ifdef SRV_LOG_STORAGE
    LogStorage* log_storage = furi_record_open(RECORD_LOG_STORAGE);
    log_storage_dump(log_storage, SUPERVISOR_FAILURE_LOG_DUMP_PATH);
    log_storage_suspend_remote(log_storage);
    furi_record_close(RECORD_LOG_STORAGE);
#endif

    if(status != IntercomStatusErrorSync && !furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug)) {
        if(furi_get_tick() > furi_ms_to_ticks(SUPERVISOR_REBOOT_GRACE_PERIOD_MS)) {
            FURI_LOG_I(TAG, "Rebooting...");
            furi_delay_ms(100);

            power_reboot(instance->power, PowerRebootNormal);
        }
    }

    supervisor_update_warning(instance, SupervisorWarningTypeIntercomError, true);
}

static void supervisor_process(FuriEventLoopObject* object, void* context) {
    Supervisor* instance = context;
    furi_assert(object == instance->message_queue);

    SupervisorEvent event;
    if(furi_message_queue_get(instance->message_queue, &event, 0) != FuriStatusOk) {
        return;
    }

    switch(event.type) {
    case SupervisorEventTypeBatteryCriticalStart:
        FURI_LOG_I(TAG, "Battery critical warning received");
        supervisor_update_warning(instance, SupervisorWarningTypeBatteryCritical, true);
        furi_event_loop_timer_start(instance->battery_critical_timer, 1000); // 1 second interval
        instance->battery_critical_counter = 0;
        break;

    case SupervisorEventTypeBatteryCriticalStop:
        FURI_LOG_I(TAG, "Clearing battery critical warning");
        furi_event_loop_timer_stop(instance->battery_critical_timer);
        supervisor_update_warning(instance, SupervisorWarningTypeBatteryCritical, false);
        break;

    case SupervisorEventTypeBatteryNotPresent:
        FURI_LOG_I(TAG, "Battery not present warning received");
        supervisor_update_warning(instance, SupervisorWarningTypeBatteryNotReady, true);
        break;

    case SupervisorEventTypeBatteryPresent:
        FURI_LOG_I(TAG, "Battery present event received");
        supervisor_update_warning(instance, SupervisorWarningTypeBatteryNotReady, false);
        break;

    case SupervisorEventTypeOkPressed: {
        FURI_LOG_I(TAG, "OK pressed event received");

        const SupervisorWarning* displayed_warning = instance->displayed_warning;
        if(displayed_warning && displayed_warning->ok_callback) {
            displayed_warning->ok_callback(instance, displayed_warning->context);
        }

        break;
    }

    case SupervisorEventTypeTickToDie:
        // with the assumption that only BatteryNotPresent warning has higher priority
        // this will guarantee that we will power off in any other state
        if(supervisor_get_topmost_warning(instance) == SupervisorWarningTypeBatteryCritical) {
            instance->battery_critical_counter++;

            size_t time_to_off_seconds;
            if(instance->battery_critical_counter >= SUPERVISOR_BATTERY_TIME_TO_DIE_S) {
                FURI_LOG_I(TAG, "Battery critical timeout reached");

                if(!power_off(instance->power)) {
                    FURI_LOG_E(TAG, "Power off failed");

                    furi_event_loop_timer_stop(instance->battery_critical_timer);
                }

                time_to_off_seconds = 0;
            } else {
                time_to_off_seconds =
                    SUPERVISOR_BATTERY_TIME_TO_DIE_S - instance->battery_critical_counter;
            }

            supervisor_update_time_to_die(instance, time_to_off_seconds);
        }
        break;

    case SupervisorEventTypeIntercomStatusChanged:
        supervisor_handle_intercom_status(instance, event.intercom_status);
        break;

    case SupervisorEventTypeWillReboot:
        FURI_LOG_I(TAG, "Will Reboot event received");
        supervisor_update_warning(instance, SupervisorWarningTypeRebooting, true);
        break;
    }
}

int32_t supervisor_start(void* argument) {
    UNUSED(argument);

    Supervisor* instance = malloc(sizeof(*instance));
    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue = furi_message_queue_alloc(8, sizeof(SupervisorEvent));
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
    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->intercom = furi_record_open(RECORD_INTERCOM);
    instance->matter = furi_record_open(RECORD_MATTER);
    instance->gui = furi_record_open(RECORD_GUI);
    instance->low_power = furi_record_open(RECORD_LOW_POWER);

    instance->displayed_warning = NULL;

    with_gui(instance->gui, {
        GuiLayer* gui_system_layer = gui_get_layer(instance->gui, GuiLayerIdSystem);
        gui_layer_add_input_callback(gui_system_layer, supervisor_input, instance);

        for(GuiDisplayId display = GuiDisplayIdFront; display < GuiDisplayIdMax; display++) {
            Widget* root_widget = gui_layer_get_root_widget(gui_system_layer, display);

            StatusView* status_view = status_view_alloc(root_widget);
            widget_set_visible(status_view_get_base(status_view), false);
            widget_set_background_color(
                status_view_get_base(status_view), (Color)COLOR_MAKE_RGB(0x00, 0x00, 0x00));

            instance->status_views[display] = status_view;
        }

        StatusView* back_status_view = instance->status_views[GuiDisplayIdBack];
        widget_set_padding(status_view_get_base(back_status_view), 0, BACK_STATUS_BAR_WIDTH, 0, 0);
    });

    furi_pubsub_subscribe(power_get_pubsub(instance->power), supervisor_power_callback, instance);
    furi_state_subscribe(
        intercom_get_state(instance->intercom), supervisor_intercom_state_callback, instance);

    furi_pubsub_subscribe(
        matter_get_pubsub(instance->matter), supervisor_matter_callback, instance);

    if(!power_is_battery_ready(instance->power)) {
        supervisor_update_warning(instance, SupervisorWarningTypeBatteryNotReady, true);
    }

    // Check storage partitions
    Storage* storage = instance->storage;
    bool backup_exists = storage_sd_status(storage, STORAGE_BACKUP_PATH_PREFIX) == FSE_OK;
    bool external_exists = storage_sd_status(storage, STORAGE_EXT_PATH_PREFIX) == FSE_OK;

    if(!backup_exists && !external_exists) {
        FURI_LOG_E(TAG, "No partitions found");
        supervisor_update_warning(instance, SupervisorWarningTypeStorageNoPartitions, true);
    } else if(!backup_exists) {
        FURI_LOG_E(TAG, "Backup partition not found");
        supervisor_update_warning(instance, SupervisorWarningTypeStorageNoBackup, true);
    } else if(!external_exists) {
        FURI_LOG_E(TAG, "External partition not found");
        supervisor_update_warning(instance, SupervisorWarningTypeStorageNoExternal, true);
    } else {
        FURI_LOG_I(TAG, "All partitions are OK");
    }

    furi_event_loop_run(instance->event_loop);

    return 0;
}
