#include "busy_timer_i.h"

#define TIMER_SMART_HOME_PROFILE_ID (BusyTimerProfileIdBusy)

static bool busy_timer_smart_home_initial_state_received(const BusyTimer* instance) {
    return instance->matter_switch_state != MatterSwitchStateUnknown;
}

static void
    busy_timer_smart_home_handle_timer_event(BusyTimer* instance, const BusyTimerEvent* event) {
    const BusyTimerEventType event_type = event->type;

    MatterSwitchState switch_state = MatterSwitchStateMax;

    if(event_type == BusyTimerEventTypeStateChanged) {
        if(instance->is_timer_running) {
            const bool is_work = (event->state_changed.state == BusyTimerStateWork);
            switch_state = is_work ? MatterSwitchStateOn : MatterSwitchStateOff;
        } else {
            switch_state = MatterSwitchStateOff;
        }

    } else if(event_type == BusyTimerEventTypePaused) {
        if(instance->state == BusyTimerStateWork) {
            const bool is_paused = event->paused.is_paused;
            switch_state = is_paused ? MatterSwitchStateOff : MatterSwitchStateOn;
        }

    } else if(event_type == BusyTimerEventTypeIntervalEnded) {
        switch_state = MatterSwitchStateOff;
    }

    if((switch_state != MatterSwitchStateMax) && (switch_state != instance->matter_switch_state)) {
        if(matter_set_switch_state(instance->matter, switch_state) == MatterStatusOk) {
            if(busy_timer_smart_home_initial_state_received(instance)) {
                instance->matter_switch_state = switch_state;
            }
        } else {
            FURI_LOG_E(TAG, "Failed to set matter switch state");
        }
    }
}

static void busy_timer_smart_home_timer_pubsub_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);

    BusyTimer* instance = context;
    const BusyTimerEvent* event = message;

    if(instance->app_config.is_smart_home_enabled) {
        busy_timer_smart_home_handle_timer_event(instance, event);
    }
}

static void busy_timer_matter_switch_state_callback(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    BusyTimer* instance = context;

    const MatterSwitchState switch_state = *(MatterSwitchState*)item;
    furi_assert(switch_state < MatterSwitchStateMax);

    busy_timer_handle_matter(instance, switch_state);
}

static void busy_timer_smart_home_start_app(BusyTimer* instance) {
    if(instance->state == BusyTimerStateIdle) {
        busy_timer_apply_profile_settings(instance, TIMER_SMART_HOME_PROFILE_ID);
    }

    BusyAppConfig* app_config = &instance->app_config;
    // NOTE: "Trigger smart home" setting is forced ON only for the current session.
    //       The main profile setting will remain unchanged.
    app_config->is_smart_home_enabled = true;

    instance->session_source = BusyTimerSessionSourceIntegrationMatter;
    instance->active_profile_id = TIMER_SMART_HOME_PROFILE_ID;

    busy_timer_start_app(app_config);
    busy_timer_start_internal(instance);
}

static MatterSwitchState busy_timer_smart_home_process_switch_on(BusyTimer* instance) {
    MatterSwitchState result = MatterSwitchStateMax;

    if(instance->state == BusyTimerStateIdle) {
        busy_timer_smart_home_start_app(instance);

    } else if(instance->state == BusyTimerStateWork) {
        if(!busy_timer_is_running(instance)) {
            if(instance->time_elapsed_s == 0) {
                busy_timer_smart_home_start_app(instance);
            } else {
                busy_timer_toggle_internal(instance);
            }
        } else {
            result = MatterSwitchStateOn;
        }

    } else if(instance->state == BusyTimerStateRest) {
        if(!busy_timer_is_running(instance)) {
            if(instance->time_elapsed_s == 0) {
                busy_timer_smart_home_start_app(instance);
                result = MatterSwitchStateOff;

            } else {
                busy_timer_toggle_internal(instance);
                busy_timer_skip_internal(instance);
            }

        } else {
            busy_timer_skip_internal(instance);
        }
    }

    return result;
}

static MatterSwitchState busy_timer_smart_home_process_switch_off(BusyTimer* instance) {
    if(instance->state != BusyTimerStateIdle) {
        busy_timer_stop_internal(instance);
        busy_timer_exit_app();
    }

    return MatterSwitchStateOff;
}

static MatterSwitchState busy_timer_smart_home_process_switch_state(
    BusyTimer* instance,
    MatterSwitchState switch_state) {
    MatterSwitchState result = MatterSwitchStateMax;

    if(switch_state == MatterSwitchStateOn) {
        result = busy_timer_smart_home_process_switch_on(instance);
    } else if(switch_state == MatterSwitchStateOff) {
        result = busy_timer_smart_home_process_switch_off(instance);
    }

    return result;
}

void busy_timer_smart_home_handle_switch_state(BusyTimer* instance, MatterSwitchState switch_state) {
    MatterSwitchState new_switch_state = MatterSwitchStateMax;

    if(instance->matter_switch_state != switch_state) {
        if(busy_timer_smart_home_initial_state_received(instance)) {
            new_switch_state = busy_timer_smart_home_process_switch_state(instance, switch_state);
        } else {
            new_switch_state = switch_state;
        }
    }

    if(new_switch_state != MatterSwitchStateMax) {
        instance->matter_switch_state = new_switch_state;

        if(new_switch_state != switch_state) {
            matter_set_switch_state(instance->matter, new_switch_state);
        }
    }
}

void busy_timer_smart_home_init(BusyTimer* instance) {
    instance->matter = furi_record_open(RECORD_MATTER);

    furi_pubsub_subscribe(
        instance->event_pubsub, busy_timer_smart_home_timer_pubsub_callback, instance);

    furi_state_subscribe(
        matter_get_switch_state(instance->matter),
        busy_timer_matter_switch_state_callback,
        instance);
}
