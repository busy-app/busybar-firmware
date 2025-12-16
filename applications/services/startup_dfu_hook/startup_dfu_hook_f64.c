#include <status_lights/status_lights.h>
#include <input/input_f64.h>
#include <intercom/intercom.h>

#include <furi.h>
#include <furi_hal_gpio.h>
#include <furi_hal_resources.h>

#define DFU_WARN_COLOR  0xFFA500
#define DFU_ENTER_COLOR 0xFF0000

#define DFU_WARN_TIMEOUT_MS 3000

typedef enum {
    StartupDfuHookFlagStartRelease = 1 << 0,
    StartupDfuHookFlagIntercomSync = 1 << 1,

    StartupDfuHookFlagAny = StartupDfuHookFlagStartRelease | StartupDfuHookFlagIntercomSync
} StartupDfuHookFlag;

typedef enum {
    StartupDfuHookStateDfuWarn,
    StartupDfuHookStateDfuEnter,
    StartupDfuHookStateAbort,
    StartupDfuHookStateExit,
} StartupDfuHookState;

static void startup_dfu_hook_input_events_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const InputCommonEvent* event = message;
    FuriThreadId* thread_id = context;

    if(event->device == InputDeviceButton) {
        if(event->button_event.button == InputButtonStart &&
           event->button_event.action == InputActionRelease) {
            furi_thread_flags_set(thread_id, StartupDfuHookFlagStartRelease);
        }
    }
}

static void startup_dfu_hook_intercom_events_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const IntercomEvent* event = message;
    FuriThreadId* thread_id = context;

    if(event->type == IntercomEventTypeSyncStateChanged) {
        if(event->is_in_sync) {
            furi_thread_flags_set(thread_id, StartupDfuHookFlagIntercomSync);
        }
    }
}

static int32_t startup_dfu_hook_thread(void* arg) {
    furi_assert(arg);

    Input* input = arg;
    FuriPubSub* input_events = furi_record_open(RECORD_INPUT_EVENTS);
    FuriPubSubSubscription* input_events_subscription = furi_pubsub_subscribe(
        input_events, startup_dfu_hook_input_events_callback, furi_thread_get_current_id());
    // TODO [FW-503]: use furi_state
    InputAbsoluteState input_state = input_get_absolute_state(input);

    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    FuriPubSub* intercom_events = intercom_get_pubsub(intercom);
    FuriPubSubSubscription* intercom_events_subscription = furi_pubsub_subscribe(
        intercom_events, startup_dfu_hook_intercom_events_callback, furi_thread_get_current_id());
    bool is_intercom_in_sync = intercom_is_in_sync(intercom);

    if(input_state.buttons & InputButtonMaskStart && !is_intercom_in_sync) {
        StatusLights* status_lights = furi_record_open(RECORD_STATUS_LIGHTS);
        StartupDfuHookState state = StartupDfuHookStateDfuWarn;

        do {
            switch(state) {
            case StartupDfuHookStateDfuWarn: {
                status_lights_run_preset(
                    status_lights, StatusLightsPresetBlink, (Color)COLOR_MAKE_HEX(DFU_WARN_COLOR));

                uint32_t flags = furi_thread_flags_wait(
                    StartupDfuHookFlagAny, FuriFlagWaitAny, DFU_WARN_TIMEOUT_MS);

                state = (flags & FuriFlagError) ? StartupDfuHookStateDfuEnter :
                                                  StartupDfuHookStateAbort;
                break;
            }

            case StartupDfuHookStateDfuEnter:
                status_lights_run_preset(
                    status_lights,
                    StatusLightsPresetStaticColor,
                    (Color)COLOR_MAKE_HEX(DFU_ENTER_COLOR));

                furi_thread_flags_wait(
                    StartupDfuHookFlagIntercomSync, FuriFlagWaitAny, FuriWaitForever);

                state = StartupDfuHookStateAbort;
                break;

            case StartupDfuHookStateAbort:
                status_lights_run_preset(
                    status_lights, StatusLightsPresetOff, (Color)COLOR_MAKE_RGB(0, 0, 0));
                state = StartupDfuHookStateExit;
                break;

            case StartupDfuHookStateExit:
                break;
            }
        } while(state != StartupDfuHookStateExit);

        furi_record_close(RECORD_STATUS_LIGHTS);
    }

    furi_pubsub_unsubscribe(intercom_events, intercom_events_subscription);
    furi_record_close(RECORD_INTERCOM);

    furi_pubsub_unsubscribe(input_events, input_events_subscription);
    furi_record_close(RECORD_INPUT_EVENTS);

    return 0;
}

void startup_dfu_hook_on_system_start(void) {
    Input* input = furi_record_open(RECORD_INPUT);
    // TODO [FW-503]: use furi_state
    InputAbsoluteState input_state = input_get_absolute_state(input);

    if(input_state.buttons & InputButtonMaskStart) {
        furi_thread_start(
            furi_thread_alloc_ex("StartupDfuHook", 1024, startup_dfu_hook_thread, input));
    } else {
        furi_record_close(RECORD_INPUT);
    }
}
