#include "power.h"

#include <furi.h>
#include <furi_hal.h>
#include "bq25798.h"

#define TAG "Power"

#define POWER_GPIO (&gpio_bq25798_irq)
#define POWER_I2C  (&furi_hal_i2c_handle_1)

typedef struct {
    bool charger_alive;
} PowerState;

struct Power {
    FuriEventLoop* event_loop;
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

    if (irq_flags & Bq25987IrqFlagVbusPresent) {
        bq25798_set_input_current_limit(POWER_I2C, 1.f); // TODO: update limit after PD negotiation
    }

    // bq25798_dump_status(POWER_I2C);
    furi_hal_i2c_release(POWER_I2C);
}

static void power_gpio_isr(void* context) {
    Power* instance = context;
    furi_assert(instance);
    furi_semaphore_release(instance->gpio_semaphore);
}

Power* power_alloc(void) {
    Power* instance = malloc(sizeof(Power));
    instance->event_loop = furi_event_loop_alloc();
    instance->gpio_semaphore = furi_semaphore_alloc(1, 0);
    return instance;
}

void power_free(Power* instance) {
    furi_assert(instance);
    furi_event_loop_free(instance->event_loop);
    free(instance);
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
    furi_hal_gpio_init(POWER_GPIO, GpioModeInterruptFall, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_add_int_callback(POWER_GPIO, power_gpio_isr, instance);

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

    FURI_LOG_I(TAG, "Running event loop");
    furi_event_loop_run(instance->event_loop);

    FURI_LOG_I(TAG, "Releasing GPIO");
    furi_hal_gpio_init(POWER_GPIO, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_remove_int_callback(POWER_GPIO);
}
