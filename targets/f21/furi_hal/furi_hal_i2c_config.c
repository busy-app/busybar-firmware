#include <furi_hal_i2c_config.h>
#include <furi_hal_resources.h>
// #include <furi_hal_version.h>
#include <furi_hal_bus.h>

#include <stm32u5xx_ll_rcc.h>

/** Timing register value is computed with the STM32CubeMX Tool,
  * Standard Mode @100kHz with I2CCLK = 64 MHz,
  * rise time = 0ns, fall time = 0ns
  */
#define FURI_HAL_I2C_CONFIG_I2C_TIMINGS_100 // TODO:

/** Timing register value is computed with the STM32CubeMX Tool,
  * Fast Mode @400kHz with I2CCLK = 64 MHz,
  * rise time = 0ns, fall time = 0ns
  */
#define FURI_HAL_I2C_CONFIG_I2C_TIMINGS_400 0x10F02A89 // TODO: now is ~370khz

FuriMutex* furi_hal_i2c_bus_1_mutex = NULL;

static void furi_hal_i2c_bus_1_event(FuriHalI2cBus* bus, FuriHalI2cBusEvent event) {
    if(event == FuriHalI2cBusEventInit) {
        furi_hal_i2c_bus_1_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
        bus->current_handle = NULL;
    } else if(event == FuriHalI2cBusEventDeinit) {
        furi_mutex_free(furi_hal_i2c_bus_1_mutex);
    } else if(event == FuriHalI2cBusEventLock) {
        furi_check(furi_mutex_acquire(furi_hal_i2c_bus_1_mutex, FuriWaitForever) == FuriStatusOk);
    } else if(event == FuriHalI2cBusEventUnlock) {
        furi_check(furi_mutex_release(furi_hal_i2c_bus_1_mutex) == FuriStatusOk);
    } else if(event == FuriHalI2cBusEventActivate) {
        FURI_CRITICAL_ENTER();
        furi_hal_bus_enable(FuriHalBusI2C1);
        LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_PCLK1);
        FURI_CRITICAL_EXIT();
    } else if(event == FuriHalI2cBusEventDeactivate) {
        furi_hal_bus_disable(FuriHalBusI2C1);
    }
}

FuriHalI2cBus furi_hal_i2c_bus_1 = {
    .i2c = I2C1,
    .callback = furi_hal_i2c_bus_1_event,
};

void furi_hal_i2c_bus_handle_1_event(FuriHalI2cBusHandle* handle, FuriHalI2cBusHandleEvent event) {
    if(event == FuriHalI2cBusHandleEventActivate) {
        furi_hal_gpio_init_ex(
            &gpio_i2c_sda, GpioModeAltFunctionOpenDrain, GpioPullNo, GpioSpeedLow, GpioAltFn4I2C1);
        furi_hal_gpio_init_ex(
            &gpio_i2c_scl, GpioModeAltFunctionOpenDrain, GpioPullNo, GpioSpeedLow, GpioAltFn4I2C1);

        LL_I2C_InitTypeDef I2C_InitStruct;
        I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
        I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
        I2C_InitStruct.DigitalFilter = 0;
        I2C_InitStruct.OwnAddress1 = 0;
        I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
        I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
        I2C_InitStruct.Timing = FURI_HAL_I2C_CONFIG_I2C_TIMINGS_400;
        LL_I2C_Init(handle->bus->i2c, &I2C_InitStruct);
        // I2C is enabled at this point
        LL_I2C_EnableAutoEndMode(handle->bus->i2c);
        LL_I2C_SetOwnAddress2(handle->bus->i2c, 0, LL_I2C_OWNADDRESS2_NOMASK);
        LL_I2C_DisableOwnAddress2(handle->bus->i2c);
        LL_I2C_DisableGeneralCall(handle->bus->i2c);
        LL_I2C_EnableClockStretching(handle->bus->i2c);
    } else if(event == FuriHalI2cBusHandleEventDeactivate) {
        LL_I2C_Disable(handle->bus->i2c);
        furi_hal_gpio_write(&gpio_i2c_sda, 1);
        furi_hal_gpio_write(&gpio_i2c_scl, 1);
        furi_hal_gpio_init_ex(
            &gpio_i2c_sda, GpioModeAnalog, GpioPullNo, GpioSpeedLow, GpioAltFnUnused);
        furi_hal_gpio_init_ex(
            &gpio_i2c_scl, GpioModeAnalog, GpioPullNo, GpioSpeedLow, GpioAltFnUnused);
    }
}

FuriHalI2cBusHandle furi_hal_i2c_handle_1 = {
    .bus = &furi_hal_i2c_bus_1,
    .callback = furi_hal_i2c_bus_handle_1_event,
};
