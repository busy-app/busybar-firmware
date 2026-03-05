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

typedef struct {
    Input* input;
    Intercom* intercom;
    StatusLights* status_lights;

    FuriPubSub* input_events;

    FuriPubSubSubscription* input_events_subscription;
    FuriStateSub* intercom_state_subscription;
} StartupDfuHook;

static void startup_dfu_hook_input_events_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    const InputCommonEvent* event = message;
    FuriThreadId thread_id = context;

    if(event->device == InputDeviceButton) {
        if(event->button_event.button == InputButtonStart &&
           event->button_event.action == InputActionRelease) {
            furi_thread_flags_set(thread_id, StartupDfuHookFlagStartRelease);
        }
    }
}

static void startup_dfu_hook_intercom_state_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    FuriThreadId thread_id = context;

    const IntercomStatus intercom_status = *(IntercomStatus*)message;

    if(intercom_status == IntercomStatusOk) {
        furi_thread_flags_set(thread_id, StartupDfuHookFlagIntercomSync);
    }
}

static void startup_dfu_hook_run(StartupDfuHook* instance) {
    StatusLights* status_lights = instance->status_lights;
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

        default:
            furi_crash("Invalid StartupDfuHookState value");
        }

    } while(state != StartupDfuHookStateExit);
}

static void startup_dfu_hook_construct(StartupDfuHook* instance) {
    instance->input = furi_record_open(RECORD_INPUT);
    instance->intercom = furi_record_open(RECORD_INTERCOM);
    instance->status_lights = furi_record_open(RECORD_STATUS_LIGHTS);

    instance->input_events = furi_record_open(RECORD_INPUT_EVENTS);
    instance->input_events_subscription = furi_pubsub_subscribe(
        instance->input_events,
        startup_dfu_hook_input_events_callback,
        furi_thread_get_current_id());

    FuriState* intercom_state = intercom_get_state(instance->intercom);
    instance->intercom_state_subscription = furi_state_subscribe(
        intercom_state, startup_dfu_hook_intercom_state_callback, furi_thread_get_current_id());
}

static void startup_dfu_hook_teardown(StartupDfuHook* instance) {
    furi_pubsub_unsubscribe(instance->input_events, instance->input_events_subscription);
    furi_state_unsubscribe(instance->intercom_state_subscription);

    furi_record_close(RECORD_INPUT);
    furi_record_close(RECORD_INPUT_EVENTS);
    furi_record_close(RECORD_INTERCOM);
    furi_record_close(RECORD_STATUS_LIGHTS);
}

static bool startup_dfu_hook_should_run(const StartupDfuHook* instance) {
    // TODO [FW-503]: use furi_state
    const InputAbsoluteState input_state = input_get_absolute_state(instance->input);
    return (input_state.buttons & InputButtonMaskStart);
}

void startup_dfu_hook_on_system_start(void) {
    // Using stack allocation to conserve heap
    StartupDfuHook instance = {0};
    startup_dfu_hook_construct(&instance);

    if(startup_dfu_hook_should_run(&instance)) {
        startup_dfu_hook_run(&instance);
    }

    startup_dfu_hook_teardown(&instance);
}
