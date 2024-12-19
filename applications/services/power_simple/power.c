#include "power.h"

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
} PowerMessageType;

typedef struct {
    bool charger_alive;
} PowerState;

typedef struct {
    PowerMessageType type;
    FuriApiLock lock;
} PowerMessage;

struct Power {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    FuriSemaphore* gpio_semaphore;
    PowerState state;
};

static void power_on_interrupt(FuriEventLoopObject* object, void* context) {
    Power* instance = context;

    furi_assert(instance);
    furi_assert(instance->gpio_semaphore == object);

    furi_check(furi_semaphore_acquire(instance->gpio_semaphore, 0) == FuriStatusOk);

    furi_hal_i2c_acquire(POWER_I2C);
    uint32_t irq_flags = 0;
    bq25798_get_irq_flags(POWER_I2C, &irq_flags);
    FURI_LOG_I(TAG, "Charger Interrupt flags: %08lX", irq_flags);

    if(irq_flags & Bq25987IrqFlagVbusPresent) {
        bq25798_set_input_current_limit(POWER_I2C, 1.f); // TODO: update limit after PD negotiation
    }

    furi_hal_i2c_release(POWER_I2C);
}

static void power_gpio_isr(void* context) {
    Power* instance = context;
    furi_assert(instance);
    furi_semaphore_release(instance->gpio_semaphore);
}

static void power_handle_shutdown(Power* power, bool full_shutdown) {
    UNUSED(power);
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

static void power_handle_reboot(Power* power) {
    UNUSED(power);
    // TODO: normal reboot, DFU, ...
    FURI_LOG_I(TAG, "reset");
    furi_hal_i2c_acquire(POWER_I2C);
    bq25798_power_switch(POWER_I2C, Bq25987PowerReset);
    furi_hal_i2c_release(POWER_I2C);
}

static void power_message_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    Power* power = context;

    furi_assert(object == power->message_queue);

    PowerMessage msg;
    furi_check(furi_message_queue_get(power->message_queue, &msg, 0) == FuriStatusOk);

    switch(msg.type) {
    case PowerMessageTypeShutdown:
        power_handle_shutdown(power, true);
        break;
    case PowerMessageTypeOff:
        power_handle_shutdown(power, false);
        break;
    case PowerMessageTypeReboot:
        power_handle_reboot(power);
        break;
    default:
        furi_crash();
    }

    if(msg.lock) {
        api_lock_unlock(msg.lock);
    }
}

Power* power_alloc(void) {
    Power* instance = malloc(sizeof(Power));
    instance->event_loop = furi_event_loop_alloc();
    instance->gpio_semaphore = furi_semaphore_alloc(1, 0);
    instance->message_queue = furi_message_queue_alloc(4, sizeof(PowerMessage));

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        power_message_callback,
        instance);
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
    furi_hal_i2c_release(POWER_I2C);

    furi_record_create(RECORD_POWER, instance);
    FURI_LOG_I(TAG, "Running event loop");
    furi_event_loop_run(instance->event_loop);
}

void power_off(Power* power) {
    furi_check(power);

    PowerMessage msg = {
        .type = PowerMessageTypeOff,
    };

    furi_check(
        furi_message_queue_put(power->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
}

void power_shutdown(Power* power) {
    furi_check(power);

    PowerMessage msg = {
        .type = PowerMessageTypeShutdown,
    };

    furi_check(
        furi_message_queue_put(power->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
}

void power_reboot(Power* power) {
    PowerMessage msg = {
        .type = PowerMessageTypeReboot,
    };

    furi_check(
        furi_message_queue_put(power->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
}
