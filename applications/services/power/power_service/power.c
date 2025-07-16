#include "power_i.h"
#include <toolbox/dsp.h>
#include <drivers/bq25798/bq25798.h>

#define TAG "Power"

#define POWER_IRQ_GPIO (&gpio_bq25798_irq)
#define POWER_I2C      (&furi_hal_i2c_handle_1)

static void power_print_interrupt_flags(uint32_t flags) {
    FURI_LOG_D(TAG, "Charger Interrupt flags: %08lX", flags);
    if(flags & Bq25798ChargerFlagVbusPresent) FURI_LOG_D(TAG, "\tVbus present");
    if(flags & Bq25798ChargerFlagAC1Present) FURI_LOG_D(TAG, "\tAC1 present");
    if(flags & Bq25798ChargerFlagAC2Present) FURI_LOG_D(TAG, "\tAC2 present");
    if(flags & Bq25798ChargerFlagPowerGood) FURI_LOG_D(TAG, "\tPower good");
    if(flags & Bq25798ChargerFlagPoorSrc) FURI_LOG_D(TAG, "\tPoor source");
    if(flags & Bq25798ChargerFlagWd) FURI_LOG_D(TAG, "\tWatchdog");
    if(flags & Bq25798ChargerFlagVindpmVotg) FURI_LOG_D(TAG, "\tVindpm Votg");
    if(flags & Bq25798ChargerFlagIindpmIotg) FURI_LOG_D(TAG, "\tIindpm Iotg");

    if(flags & Bq25798ChargerFlagBC12Done) FURI_LOG_D(TAG, "\tBC1.2 done");
    if(flags & Bq25798ChargerFlagVbatPresent) FURI_LOG_D(TAG, "\tVbat present");
    if(flags & Bq25798ChargerFlagThermReg) FURI_LOG_D(TAG, "\tThermal regulation");
    if(flags & Bq25798ChargerFlagVbusStatus) FURI_LOG_D(TAG, "\tVbus status");
    if(flags & Bq25798ChargerFlagICOStatus) FURI_LOG_D(TAG, "\tICO status");
    if(flags & Bq25798ChargerFlagChargeStatus) FURI_LOG_D(TAG, "\tCharge status");

    if(flags & Bq25798ChargerFlagTopOffTmr) FURI_LOG_D(TAG, "\tTop off timer");
    if(flags & Bq25798ChargerFlagPreChgTmr) FURI_LOG_D(TAG, "\tPre charge timer");
    if(flags & Bq25798ChargerFlagTrickleChgTmr) FURI_LOG_D(TAG, "\tTrickle charge timer");
    if(flags & Bq25798ChargerFlagFastChgTmr) FURI_LOG_D(TAG, "\tFast charge timer");
    if(flags & Bq25798ChargerFlagVsysMinReg) FURI_LOG_D(TAG, "\tVsys min regulation");
    if(flags & Bq25798ChargerFlagAdcDone) FURI_LOG_D(TAG, "\tADC done");
    if(flags & Bq25798ChargerFlagDpDmDone) FURI_LOG_D(TAG, "\tDP DM done");

    if(flags & Bq25798ChargerFlagTsHot) FURI_LOG_D(TAG, "\tThermal shutdown");
    if(flags & Bq25798ChargerFlagTsWarm) FURI_LOG_D(TAG, "\tThermal warm");
    if(flags & Bq25798ChargerFlagTsCool) FURI_LOG_D(TAG, "\tThermal cool");
    if(flags & Bq25798ChargerFlagTsCold) FURI_LOG_D(TAG, "\tThermal cold");
    if(flags & Bq25798ChargerFlagVbatOtgLow) FURI_LOG_D(TAG, "\tVbat OTG low");
}

static void power_on_interrupt(FuriEventLoopObject* object, void* context) {
    Power* power = context;

    furi_assert(power);
    furi_assert(power->gpio_semaphore == object);

    furi_check(furi_semaphore_acquire(power->gpio_semaphore, 0) == FuriStatusOk);

    furi_hal_i2c_acquire(POWER_I2C);
    uint32_t irq_flags = 0;
    bq25798_get_charger_irq_flags(POWER_I2C, &irq_flags);
    power_print_interrupt_flags(irq_flags);

    Bq25798ChargerStatus status = {};
    bq25798_get_charger_status(POWER_I2C, &status);

    if(irq_flags & Bq25798ChargerFlagVbusPresent) {
        if(status.vbus_present) {
            bq25798_set_input_current_limit(POWER_I2C, power->input_current_limit);
        }
        power->state.usb_connected = status.vbus_present;
    }

    // ADC can be disabled by internal BQ25798 mechanism
    bq25798_adc_enable(POWER_I2C, true);

    furi_hal_i2c_release(POWER_I2C);
}

static void power_gpio_isr(void* context) {
    Power* power = context;
    furi_assert(power);
    furi_semaphore_release(power->gpio_semaphore);
}

static void power_handle_shutdown(Power* power, bool full_shutdown) {
    UNUSED(power);
    furi_hal_i2c_acquire(POWER_I2C);

    if(full_shutdown) {
        bq25798_power_switch(POWER_I2C, Bq25798PowerShutdown);
    } else {
        bq25798_power_switch(POWER_I2C, Bq25798PowerOff);
    }
    furi_hal_i2c_release(POWER_I2C);
}

static void power_handle_reboot(Power* power, PowerRebootMode mode) {
    UNUSED(power);
    if(mode == PowerRebootHardware) {
        furi_hal_i2c_acquire(POWER_I2C);
        bq25798_power_switch(POWER_I2C, Bq25798PowerReset);
        furi_hal_i2c_release(POWER_I2C);
        furi_delay_ms(100);
        furi_crash("Should never happen");
    }

    if((mode == PowerRebootNormal) || (mode == PowerRebootNormal917)) {
        furi_hal_power_reset_917(false);
    } else if(mode == PowerRebootDfu917) {
        furi_hal_power_reset_917(true);
    }

    if((mode == PowerRebootNormal) || (mode == PowerRebootNormalU5)) {
        furi_hal_cortex_system_reset();
        furi_crash("Should never happen");
    } else if(mode == PowerRebootDfuU5) {
        // TODO: set RTC flag & reboot
        furi_hal_deinit_early();
        furi_hal_cortex_jump(FuriHalCortexJumpDFU);
        furi_crash("Should never happen");
    }
}

static void power_dump_pd_capabilities(PowerUsbPdCapability* caps) {
    FURI_LOG_I(
        TAG,
        "PD Capabilities: %u (default %.3fA)",
        caps->cap_number,
        caps->passive_mode_current / 1000.f);
    for(size_t i = 0; i < caps->cap_number; i++) {
        if(caps->cap[i].is_fixed) {
            FURI_LOG_I(
                TAG,
                "[%u] fixed %.3fV %.3fA",
                caps->cap[i].pdo_id,
                caps->cap[i].voltage_max / 1000.f,
                caps->cap[i].current_max / 1000.f);
        } else {
            FURI_LOG_I(
                TAG,
                "[%u] PPS %.3f-%.3fV %.3fA",
                caps->cap[i].pdo_id,
                caps->cap[i].voltage_min / 1000.f,
                caps->cap[i].voltage_max / 1000.f,
                caps->cap[i].current_max / 1000.f);
        }
    }
}

static void power_handle_pd_update(Power* power, uint32_t voltage, uint32_t current) {
    FURI_LOG_I(TAG, "USB PD Mode: %.3fV %.3fA", voltage / 1000.f, current / 1000.f);

    PowerUsbPdCapability pd_capabilities;
    power_usb_pd_get_capabilities(power->usb_pd, &pd_capabilities);
    power_dump_pd_capabilities(&pd_capabilities);

    power->pd_info.voltage_set = voltage;
    power->pd_info.current_max = current;
    power->pd_info.cc_line = pd_capabilities.cc_line;
    power->pd_info.cap_id = pd_capabilities.cap_id_current;
    power->pd_info.passive_mode_current = pd_capabilities.passive_mode_current;
    power->pd_info.cap_number = pd_capabilities.cap_number;
    for(uint8_t i = 0; i < power->pd_info.cap_number; i++) {
        power->pd_info.cap[i].voltage_min = pd_capabilities.cap[i].voltage_min;
        power->pd_info.cap[i].voltage_max = pd_capabilities.cap[i].voltage_max;
        power->pd_info.cap[i].current_max = pd_capabilities.cap[i].current_max;
        power->pd_info.cap[i].pdo_id = pd_capabilities.cap[i].pdo_id;
        power->pd_info.cap[i].is_fixed = pd_capabilities.cap[i].is_fixed;
    }

    // TODO: check capabilities before
    if((voltage == 5000) && (pd_capabilities.cap_number > 1)) {
        if(power->state.pd_initialized) {
            power_usb_pd_request_power(power->usb_pd, 9000, 0);
        }
    }

    furi_hal_i2c_acquire(POWER_I2C);
    power->input_current_limit = current;
    bq25798_set_input_current_limit(POWER_I2C, power->input_current_limit);
    furi_hal_i2c_release(POWER_I2C);
}

static void power_message_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    Power* power = context;

    furi_assert(object == power->message_queue);

    PowerMessage msg;
    furi_check(furi_message_queue_get(power->message_queue, &msg, 0) == FuriStatusOk);

    switch(msg.type) {
    case PowerMessageTypeOff:
        power_handle_shutdown(power, false);
        break;
    case PowerMessageTypeReboot:
        power_handle_reboot(power, msg.reboot_mode);
        break;
    case PowerMessageTypeIsUsbConnected:
        *(msg.param_bool) = power->state.usb_connected;
        break;
    case PowerMessageTypeIsBatteryReady:
        *(msg.param_bool) = power->state.battery_ready;
        break;
    case PowerMessageTypeGetInfo:
        furi_assert(msg.power_info);
        memcpy(msg.power_info, &(power->info), sizeof(PowerInfo));
        break;
    case PowerMessageTypeChargeEnable:
        power->charger_enabled = *(msg.param_bool);
        furi_hal_i2c_acquire(POWER_I2C);
        bq25798_charge_enable(POWER_I2C, power->charger_enabled);
        furi_hal_i2c_release(POWER_I2C);
        break;
    case PowerMessageTypeSetChargeCurrent:
        power->charger_current_limit = *(msg.param_int);
        furi_hal_i2c_acquire(POWER_I2C);
        bq25798_set_charge_current_limit(POWER_I2C, power->charger_current_limit);
        furi_hal_i2c_release(POWER_I2C);
        break;
    case PowerMessageTypePdGetInfo:
        furi_assert(msg.power_info);
        memcpy(msg.pd_info, &(power->pd_info), sizeof(PowerPdInfo));
        break;
    case PowerMessageTypePdRequest:
        if(power->state.battery_ready && power->state.pd_initialized) {
            power_usb_pd_request_power(power->usb_pd, *(msg.param_int), 0);
        }
        break;
    case PowerMessageTypeUsbPdUpdate:
        power_handle_pd_update(power, msg.pd_mode.voltage, msg.pd_mode.current);
        break;
    default:
        furi_crash();
    }

    if(msg.lock) {
        api_lock_unlock(msg.lock);
    }
}

static void power_pubsub_publish(Power* power, PowerEventType type) {
    PowerEvent event = {.type = type};
    furi_pubsub_publish(power->event_pubsub, &event);
}

static const char* power_battery_state_to_string(PowerBatteryState state) {
    switch(state) {
    case PowerBatteryStateNormal:
        return "Normal";
    case PowerBatteryStateLow:
        return "Low";
    case PowerBatteryStateCritical:
        return "Critical";
    default:
        return "Unknown";
    }
}

static void power_battery_state_transition(Power* power, PowerBatteryState state) {
    if(power->battery_state != state) {
        FURI_LOG_D(
            TAG,
            "Battery state transition: %s -> %s",
            power_battery_state_to_string(power->battery_state),
            power_battery_state_to_string(state));

        switch(power->battery_state) {
        case PowerBatteryStateNormal:
            power_pubsub_publish(power, PowerEventBatteryNormalStop);
            break;
        case PowerBatteryStateLow:
            power_pubsub_publish(power, PowerEventBatteryLowStop);
            break;
        case PowerBatteryStateCritical:
            power_pubsub_publish(power, PowerEventBatteryCriticalStop);
            break;
        }

        power->battery_state = state;

        switch(power->battery_state) {
        case PowerBatteryStateNormal:
            power_pubsub_publish(power, PowerEventBatteryNormalStart);
            break;
        case PowerBatteryStateLow:
            power_pubsub_publish(power, PowerEventBatteryLowStart);
            break;
        case PowerBatteryStateCritical:
            power_pubsub_publish(power, PowerEventBatteryCriticalStart);
            break;
        }
    }
}

static void power_process_battery_state(Power* power, uint32_t voltage_mv, bool charging) {
    if(charging) {
        power_battery_state_transition(power, PowerBatteryStateNormal);
    } else {
        if(power->battery_state == PowerBatteryStateNormal) {
            if(voltage_mv < (POWER_VOLTAGE_CRITICAL - POWER_VOLTAGE_HYSTERESIS)) {
                power_battery_state_transition(power, PowerBatteryStateCritical);
            } else if(voltage_mv < (POWER_VOLTAGE_LOW - POWER_VOLTAGE_HYSTERESIS)) {
                power_battery_state_transition(power, PowerBatteryStateLow);
            }
        } else if(power->battery_state == PowerBatteryStateLow) {
            if(voltage_mv > (POWER_VOLTAGE_LOW + POWER_VOLTAGE_HYSTERESIS)) {
                power_battery_state_transition(power, PowerBatteryStateNormal);
            } else if(voltage_mv < (POWER_VOLTAGE_CRITICAL - POWER_VOLTAGE_HYSTERESIS)) {
                power_battery_state_transition(power, PowerBatteryStateCritical);
            }
        } else if(power->battery_state == PowerBatteryStateCritical) {
            if(voltage_mv > (POWER_VOLTAGE_LOW + POWER_VOLTAGE_HYSTERESIS)) {
                power_battery_state_transition(power, PowerBatteryStateNormal);
            } else if(voltage_mv > (POWER_VOLTAGE_CRITICAL + POWER_VOLTAGE_HYSTERESIS)) {
                power_battery_state_transition(power, PowerBatteryStateLow);
            }
        }
    }
}

static bool power_charger_is_charging(Bq25798ChargerStatusChargeStat status) {
    return (status == Bq25798ChargerStatusChargeStatTrickle) ||
           (status == Bq25798ChargerStatusChargeStatPre) ||
           (status == Bq25798ChargerStatusChargeStatFast) ||
           (status == Bq25798ChargerStatusChargeStatTaper) ||
           (status == Bq25798ChargerStatusChargeStatTopOff);
}

static bool power_charger_is_charged(Bq25798ChargerStatusChargeStat status) {
    return (status == Bq25798ChargerStatusChargeStatTermination);
}

static void power_update_info(Power* power) {
    UNUSED(power);
    furi_hal_i2c_acquire(POWER_I2C);
    Bq25798ChargerStatus status = {0};
    assert(bq25798_get_charger_status(POWER_I2C, &status) == true);

    assert(bq25798_get_charger_fault(POWER_I2C, &power->info.debug.charger_fault) == true);
    memcpy(&power->info.debug.charger_status, &status, sizeof(Bq25798ChargerStatus));

    Bq25798AdcValues adc_val = {0};
    assert(bq25798_get_adc_values(POWER_I2C, &adc_val) == true);

    furi_hal_i2c_release(POWER_I2C);

#if 0
    {
        // Debug battery status

        FURI_LOG_I(
            TAG,
            "ADC values: %u %d %u %u %u %.2f %.2f (%.1f C)",
            adc_val.bat_v,
            adc_val.bat_i,
            adc_val.usb_v,
            adc_val.usb_i,
            adc_val.sys_v,
            adc_val.temp_charger,
            adc_val.temp_bat_pct,
            ntc_temp);

        if(status.treg_stat) FURI_LOG_I(TAG, "Thermal regulation");
        if(status.prechg_timer_stat) FURI_LOG_I(TAG, "Precharge timer expired");
        if(status.trichg_timer_stat) FURI_LOG_I(TAG, "Trickle charge timer expired");
        if(status.chg_timer_stat) FURI_LOG_I(TAG, "Charge timer expired");

        if(status.ts_hot_stat) FURI_LOG_I(TAG, "Thermal sensor hot");
        if(status.ts_warm_stat) FURI_LOG_I(TAG, "Thermal sensor warm");
        if(status.ts_cool_stat) FURI_LOG_I(TAG, "Thermal sensor cool");
        if(status.ts_cold_stat) FURI_LOG_I(TAG, "Thermal sensor cold");
    }
#endif

    if((power->state.battery_ready == false) && (status.vbat_present_stat == true)) {
        power->state.battery_ready = true;
        power_pubsub_publish(power, PowerEventBatteryPresent);
        FURI_LOG_I(TAG, "Battery is ready");
    } else if((power->state.battery_ready == true) && (status.vbat_present_stat == false)) {
        power->state.battery_ready = false;
        power_pubsub_publish(power, PowerEventBatteryNotPresent);
        FURI_LOG_I(TAG, "Battery is not present");
    }

    power->state.usb_connected = status.vbus_present;

    if(power->info.voltage_battery < 2500.0f) {
        power->info.voltage_battery = adc_val.bat_v;
    } else {
        // TODO: tune low pass filter
        power->info.voltage_battery =
            dsp_low_pass(adc_val.bat_v, power->info.voltage_battery, 0.90f);
    }

    FURI_LOG_I(
        TAG,
        "Battery voltage: %.3fV, real %.3fV",
        power->info.voltage_battery / 1000.f,
        adc_val.bat_v / 1000.f);

    power->info.current_battery = adc_val.bat_i;
    power->info.current_usb = adc_val.usb_i;
    power->info.voltage_usb = adc_val.usb_v;
    power->info.temperature_charger = adc_val.temp_charger;
    power->info.temperature_battery = adc_val.temp_bat_pct;

    power->info.is_charging = power_charger_is_charging(status.chg_stat);
    power->info.is_full_charged = power_charger_is_charged(status.chg_stat);
    power->info.charge = power_get_battery_charge(
        power->info.voltage_battery, power->info.current_battery, power->info.is_charging);

    power->info.charge_ilim_usb = power->input_current_limit;
    power->info.charge_ilim_battery = power->charger_current_limit;
    power->info.charge_enabled = power->charger_enabled;

    // do not init USB PD if we dont need it
    if(status.chg_stat == Bq25798ChargerStatusChargeStatFast ||
       status.chg_stat == Bq25798ChargerStatusChargeStatTaper ||
       status.chg_stat == Bq25798ChargerStatusChargeStatTopOff ||
       status.chg_stat == Bq25798ChargerStatusChargeStatTermination) {
        if(!power->state.pd_initialized) {
            FURI_LOG_I(TAG, "Initializing USB PD");
            power->state.pd_initialized = true;
            power_usb_pd_start(power->usb_pd);
        }
    }

    power_process_battery_state(power, power->info.voltage_battery, power->info.is_charging);
}

static void power_tick_callback(void* context) {
    furi_assert(context);
    Power* power = context;
    power_update_info(power);
}

static Power* power_alloc(void) {
    Power* power = malloc(sizeof(Power));
    power->event_loop = furi_event_loop_alloc();
    power->gpio_semaphore = furi_semaphore_alloc(1, 0);
    power->message_queue = furi_message_queue_alloc(4, sizeof(PowerMessage));
    power->input_current_limit = 500;
    power->charger_current_limit = POWER_CHARGE_CURRENT_MAX;
    power->charger_enabled = true;
    power->state.battery_ready = false;

    furi_event_loop_subscribe_message_queue(
        power->event_loop,
        power->message_queue,
        FuriEventLoopEventIn,
        power_message_callback,
        power);

    FuriMessageQueue* pd_queue = NULL;
    power->usb_pd = power_usb_pd_alloc(&pd_queue);
    furi_event_loop_subscribe_message_queue(
        power->event_loop, pd_queue, FuriEventLoopEventIn, power_usb_pd_msg_handler, power->usb_pd);

    furi_event_loop_tick_set(power->event_loop, 1000, power_tick_callback, power);

    power->event_pubsub = furi_pubsub_alloc();

    return power;
}

void power_run(Power* power) {
    furi_assert(power);

    FURI_LOG_I(TAG, "Configuring interrupt source");
    furi_event_loop_subscribe_semaphore(
        power->event_loop, power->gpio_semaphore, FuriEventLoopEventIn, power_on_interrupt, power);
    furi_hal_gpio_init(POWER_IRQ_GPIO, GpioModeInterruptFall, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_add_int_callback(POWER_IRQ_GPIO, power_gpio_isr, power);

    FURI_LOG_I(TAG, "Initializing charger");
    furi_hal_i2c_acquire(POWER_I2C);
    power->state.charger_alive = bq25798_init(POWER_I2C);
    if(power->state.charger_alive) {
        FURI_LOG_I(TAG, "Charger is ready");
        bq25798_reset(POWER_I2C);
        bq25798_set_cfg(POWER_I2C);
    } else {
        FURI_LOG_E(TAG, "Charger is absent");
    }

    Bq25798ChargerStatus status = {0};
    bq25798_get_charger_status(POWER_I2C, &status);
    if(status.vbat_present_stat) {
        power->state.battery_ready = true;
        power_pubsub_publish(power, PowerEventBatteryPresent);
        FURI_LOG_I(TAG, "Battery is present");
    } else {
        power->state.battery_ready = false;
        power_pubsub_publish(power, PowerEventBatteryNotPresent);
        FURI_LOG_I(TAG, "Battery is not present");
    }
    power->state.pd_initialized = false;
    power->info.voltage_battery = 0.0f;

    bq25798_set_charge_current_limit(POWER_I2C, power->charger_current_limit);
    bq25798_set_charge_voltage_limit(POWER_I2C, POWER_CHARGE_VOLTAGE);
    furi_hal_i2c_release(POWER_I2C);

    furi_record_create(RECORD_POWER, power);

    FURI_LOG_I(TAG, "Running event loop");
    furi_event_loop_run(power->event_loop);
}

void power_on_usb_pd_update(Power* power, uint32_t voltage, uint32_t current) {
    furi_check(power);

    PowerMessage msg = {
        .type = PowerMessageTypeUsbPdUpdate,
        .pd_mode = {.voltage = voltage, .current = current},
    };

    furi_check(
        furi_message_queue_put(power->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
}

int32_t power_srv_app(void* p) {
    UNUSED(p);

    Power* power = power_alloc();
    power_run(power);

    return 0;
}

FuriPubSub* power_get_pubsub(Power* power) {
    furi_check(power);
    return power->event_pubsub;
}
