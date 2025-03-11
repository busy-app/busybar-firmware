#include <furi_hal_clock.h>
#include <furi.h>

#include <rsi_rom_clks.h>

#include <FreeRTOS.h>

#include <furi_hal_gpio.h>
#include <furi_hal_resources.h>

#define TAG "FuriHalClock"

#define XTAL_FREQ_HZ     40000000UL
#define CPU_CLOCK_PLL_HZ 180000000UL

#define FURI_SWITCH_QSPI_TO_INTF_PLL
#define INTF_PLL_CLK 160000000UL

#define TICK_INT_PRIORITY 15U

void furi_hal_clock_init_early(void) {
    SystemCoreClockUpdate();
}

void furi_hal_clock_deinit_early(void) {
}

void furi_hal_clock_init(void) {
    furi_check(RSI_CLK_M4SocClkConfig(M4CLK, M4_ULPREFCLK, 0) == RSI_OK);
#if CPU_CLOCK_PLL_HZ > 120000000UL
    RSI_PS_PS4SetRegisters();
#endif
    furi_check(RSI_CLK_SetSocPllFreq(M4CLK, CPU_CLOCK_PLL_HZ, XTAL_FREQ_HZ) == RSI_OK);
    furi_check(RSI_CLK_M4SocClkConfig(M4CLK, M4_SOCPLLCLK, 0) == RSI_OK);

    SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);

#ifdef FURI_SWITCH_QSPI_TO_INTF_PLL
    furi_check(RSI_CLK_SetIntfPllFreq(M4CLK, INTF_PLL_CLK, XTAL_FREQ_HZ) == RSI_OK);
    // QSPI clock config INTF_PLL_CLK / 2 = 80 MHz
    RSI_CLK_QspiClkConfig(M4CLK, QSPI_INTFPLLCLK, 0, 0, 1);
#endif /* FURI_SWITCH_QSPI_TO_INTF_PLL */

    NVIC_SetPriority(
        SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), TICK_INT_PRIORITY, 0));
    NVIC_EnableIRQ(SysTick_IRQn);

    // Start 32KHz xtal oscillator
    MCU_FSM->MCU_FSM_CLK_ENS_AND_FIRST_BOOTUP_b.MCU_ULP_32KHZ_XTAL_CLK_EN_b = 1;

#ifdef FURI_HAL_CLOCK_MCO
    furi_hal_gpio_init_ex(&gpio_12, GpioModeOutputPushPull, GpioPullNo, GpioSpeedHigh, GpioAltFn8);
    M4CLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_ENABLE = 0;
    M4CLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_SEL = FURI_HAL_CLOCK_MCO; // 8 - 32k XTAL, 2 - 40m XTAL
    M4CLK->CLK_CONFIG_REG3_b.MCU_CLKOUT_ENABLE = 1;
#endif

    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_clock_suspend_tick(void) {
    FURI_BIT_CLEAR(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
}

void furi_hal_clock_resume_tick(void) {
    FURI_BIT_SET(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
}
