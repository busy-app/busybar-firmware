#include "power.h"
#include "power_usb_pd_i.h"
#include <furi.h>
#include <furi_hal.h>
#include <toolbox/api_lock.h>
#include "bq25798.h"

#define TAG "Power"

#define POWER_IRQ_GPIO (&gpio_bq25798_irq)
#define POWER_I2C      (&furi_hal_i2c_handle_1)

typedef enum {
    PowerMessageTypeShutdown,
    PowerMessageTypeOff,
    PowerMessageTypeReboot,

    // TODO: separate queue for internal messages?
    PowerMessageTypeUsbPdUpdate,
} PowerMessageType;

typedef struct {
    bool charger_alive;
} PowerState;

typedef struct {
    PowerMessageType type;
    FuriApiLock lock;
    union {
        struct {
            uint32_t voltage;
            uint32_t current;
        } pd_mode;
    };
} PowerMessage;

struct Power {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    FuriSemaphore* gpio_semaphore;
    PowerState state;
    PowerUsbPd* usb_pd;
    PowerUsbPdCapability pd_capabilities;
    float input_current_limit;
};

static void power_on_interrupt(FuriEventLoopObject* object, void* context) {
    Power* instance = context;

    furi_assert(instance);
    furi_assert(instance->gpio_semaphore == object);

    furi_check(furi_semaphore_acquire(instance->gpio_semaphore, 0) == FuriStatusOk);

    furi_hal_i2c_acquire(POWER_I2C);
    uint32_t irq_flags = 0;
    bq25798_get_charger_flags(POWER_I2C, &irq_flags);
    FURI_LOG_I(TAG, "Charger Interrupt flags: %08lX", irq_flags);

    if(irq_flags & Bq25987ChargerFlagVbusPresent) {
        bq25798_set_input_current_limit(POWER_I2C, instance->input_current_limit);
    }

    furi_hal_i2c_release(POWER_I2C);
}

static void power_gpio_isr(void* context) {
    Power* instance = context;
    furi_assert(instance);
    furi_semaphore_release(instance->gpio_semaphore);
}

static void power_handle_shutdown(Power* instance, bool full_shutdown) {
    UNUSED(instance);
    furi_hal_i2c_acquire(POWER_I2C);
    // TODO: check if USB is not plugged

    if(full_shutdown) {
        FURI_LOG_I(TAG, "shutdown");
        bq25798_power_switch(POWER_I2C, Bq25987PowerShutdown);
    } else {
        FURI_LOG_I(TAG, "off");
        bq25798_power_switch(POWER_I2C, Bq25987PowerOff);
    }
    furi_hal_i2c_release(POWER_I2C);
}

static void power_handle_reboot(Power* instance) {
    UNUSED(instance);
    // TODO: normal reboot, DFU, ...
    FURI_LOG_I(TAG, "reset");
    furi_hal_i2c_acquire(POWER_I2C);
    bq25798_power_switch(POWER_I2C, Bq25987PowerReset);
    furi_hal_i2c_release(POWER_I2C);
}

static void power_dump_pd_capabilities(Power* instance) {
    FURI_LOG_I(
        TAG,
        "PD Capabilities: %u (default %.3fA)",
        instance->pd_capabilities.cap_number,
        instance->pd_capabilities.passive_mode_current / 1000.f);
    for(size_t i = 0; i < instance->pd_capabilities.cap_number; i++) {
        if(instance->pd_capabilities.cap[i].is_fixed) {
            FURI_LOG_I(
                TAG,
                "[%u] fixed %.3fV %.3fA",
                instance->pd_capabilities.cap[i].pdo_id,
                instance->pd_capabilities.cap[i].voltage_max / 1000.f,
                instance->pd_capabilities.cap[i].current_max / 1000.f);
        } else {
            FURI_LOG_I(
                TAG,
                "[%u] PPS %.3f-%.3fV %.3fA",
                instance->pd_capabilities.cap[i].pdo_id,
                instance->pd_capabilities.cap[i].voltage_min / 1000.f,
                instance->pd_capabilities.cap[i].voltage_max / 1000.f,
                instance->pd_capabilities.cap[i].current_max / 1000.f);
        }
    }
}

static void power_message_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    Power* instance = context;

    furi_assert(object == instance->message_queue);

    PowerMessage msg;
    furi_check(furi_message_queue_get(instance->message_queue, &msg, 0) == FuriStatusOk);

    switch(msg.type) {
    case PowerMessageTypeShutdown:
        power_handle_shutdown(instance, true);
        break;
    case PowerMessageTypeOff:
        power_handle_shutdown(instance, false);
        break;
    case PowerMessageTypeReboot:
        power_handle_reboot(instance);
        break;
    case PowerMessageTypeUsbPdUpdate:
        FURI_LOG_I(
            TAG,
            "USB PD Mode: %.3fV %.3fA",
            msg.pd_mode.voltage / 1000.f,
            msg.pd_mode.current / 1000.f);

        power_usb_pd_get_capabilities(instance->usb_pd, &instance->pd_capabilities);
        power_dump_pd_capabilities(instance);

        // TODO: check capabilities before
        if((msg.pd_mode.voltage == 5000) && (instance->pd_capabilities.cap_number > 1)) {
            power_usb_pd_request_power(instance->usb_pd, 9000, 0); // Request 9v, max current
            // power_usb_pd_request_power(instance->usb_pd, 6666, 1234); //PPS
        }

        furi_hal_i2c_acquire(POWER_I2C);
        instance->input_current_limit = msg.pd_mode.current / 1000.f;
        bq25798_set_input_current_limit(POWER_I2C, instance->input_current_limit);
        furi_hal_i2c_release(POWER_I2C);

        break;
    default:
        furi_crash();
    }

    if(msg.lock) {
        api_lock_unlock(msg.lock);
    }
}

static bool power_battery_wait(Power* instance) {
    UNUSED(instance);

    bool ret = false;
    do {
        Bq25987ChargerStatus status = {};
        if(!bq25798_get_charger_status(POWER_I2C, &status)) {
            FURI_LOG_E(TAG, "Failed to get status");
            break;
        }
        if(!status.vbat_present_stat) {
            FURI_LOG_E(TAG, "VBAT is not ready");
            break;
        }
        ret = true;
    } while(false);

    return ret;
}

Power* power_alloc(void) {
    Power* instance = malloc(sizeof(Power));
    instance->event_loop = furi_event_loop_alloc();
    instance->gpio_semaphore = furi_semaphore_alloc(1, 0);
    instance->message_queue = furi_message_queue_alloc(4, sizeof(PowerMessage));
    instance->input_current_limit = 0.5f;

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        power_message_callback,
        instance);

    FuriMessageQueue* pd_queue = NULL;
    instance->usb_pd = power_usb_pd_alloc(&pd_queue);
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        pd_queue,
        FuriEventLoopEventIn,
        power_usb_pd_msg_handler,
        instance->usb_pd);

    return instance;
}

void power_run(Power* instance) {
    furi_assert(instance);

    FURI_LOG_I(TAG, "Configuring interrupt source");
    furi_event_loop_subscribe_semaphore(
        instance->event_loop,
        instance->gpio_semaphore,
        FuriEventLoopEventIn,
        power_on_interrupt,
        instance);
    furi_hal_gpio_init(POWER_IRQ_GPIO, GpioModeInterruptFall, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_add_int_callback(POWER_IRQ_GPIO, power_gpio_isr, instance);

    FURI_LOG_I(TAG, "Initializing charger");
    furi_hal_i2c_acquire(POWER_I2C);
    instance->state.charger_alive = bq25798_init(POWER_I2C);
    if(instance->state.charger_alive) {
        FURI_LOG_I(TAG, "Charger is ready");
        bq25798_reset(POWER_I2C);
        bq25798_set_cfg(POWER_I2C);
    } else {
        FURI_LOG_E(TAG, "Charger is absent");
    }
    FURI_LOG_I(TAG, "Waiting for battery to arrive");
    while(!power_battery_wait(instance)) {
        furi_delay_ms(1000);
    }
    FURI_LOG_I(TAG, "Initializing PD");
    power_usb_pd_start(instance->usb_pd);
    // TODO: update charge current limit based on temperature
    bq25798_set_charge_current_limit(POWER_I2C, 3.3f);
    furi_hal_i2c_release(POWER_I2C);

    furi_record_create(RECORD_POWER, instance);

    FURI_LOG_I(TAG, "Running event loop");
    furi_event_loop_run(instance->event_loop);
}

void power_off(Power* instance) {
    furi_check(instance);

    PowerMessage msg = {
        .type = PowerMessageTypeOff,
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
}

void power_shutdown(Power* instance) {
    furi_check(instance);

    PowerMessage msg = {
        .type = PowerMessageTypeShutdown,
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
}

void power_reboot(Power* instance) {
    PowerMessage msg = {
        .type = PowerMessageTypeReboot,
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
}

void power_on_usb_pd_update(Power* instance, uint32_t voltage, uint32_t current) {
    furi_check(instance);

    PowerMessage msg = {
        .type = PowerMessageTypeUsbPdUpdate,
        .pd_mode = {.voltage = voltage, .current = current},
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
}
