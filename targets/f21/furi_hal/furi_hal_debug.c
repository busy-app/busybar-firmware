#include <furi_hal_debug.h>
#include <furi_hal_resources.h>
#include <stm32u5xx_ll_system.h>
#include <stm32u5xx_ll_exti.h>

volatile bool furi_hal_debug_gdb_session_active = false;

void furi_hal_debug_enable(void) {
    // Low power mode debug
    LL_DBGMCU_EnableDBGStopMode();
    LL_DBGMCU_EnableDBGStandbyMode();
    // // SWD GPIO
    // furi_hal_gpio_init_ex(
    //     &gpio_swdio, GpioModeAltFunctionPushPull, GpioPullUp, GpioSpeedVeryHigh, GpioAltFn0SWJ);
    // furi_hal_gpio_init_ex(
    //     &gpio_swclk, GpioModeAltFunctionPushPull, GpioPullDown, GpioSpeedLow, GpioAltFn0SWJ);
}

void furi_hal_debug_disable(void) {
    // Low power mode debug
    LL_DBGMCU_DisableDBGStopMode();
    LL_DBGMCU_DisableDBGStandbyMode();
    // // SWD GPIO
    // furi_hal_gpio_init_simple(&gpio_swdio, GpioModeAnalog);
    // furi_hal_gpio_init_simple(&gpio_swclk, GpioModeAnalog);
}

bool furi_hal_debug_is_gdb_session_active(void) {
    return furi_hal_debug_gdb_session_active;
}
