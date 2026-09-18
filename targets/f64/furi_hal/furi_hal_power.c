#include "furi_hal_power.h"
#include "furi_hal_cortex.h"
#include "furi_hal_bus.h"

#include <si91x_device.h>
#include <rsi_wisemcu_hardware_setup.h>
#include <rsi_rom_power_save.h>
#include <rsi_rom_clks.h>
#include <rsi_m4.h>

typedef struct {
    volatile FuriHalPowerResetSource reset_source;
} FuriHalPower;

FuriHalPower furi_hal_power;

#define MCU_FIRST_POWERUP_RESET_N BIT(17)
#define MCU_FIRST_POWERUP_POR     BIT(16)
#define WAKEUP_STATUS_INDICATION  BIT(0)

static FuriHalPowerResetSource furi_hal_power_parse_reset_source(void) {
    const uint32_t status_reg = MCU_FSM->GPIO_WAKEUP_REGISTER;

    if(status_reg & MCU_FIRST_POWERUP_POR) return FuriHalPowerResetSourcePowerOn;
    if(status_reg & MCU_FIRST_POWERUP_RESET_N) return FuriHalPowerResetSourceResetLine;
    if(status_reg & WAKEUP_STATUS_INDICATION) return FuriHalPowerResetSourceWakeup;

    return FuriHalPowerResetSourceUnknown;
}

void furi_hal_power_init_super_early(void) {
    furi_hal_power.reset_source = furi_hal_power_parse_reset_source();
}

void furi_hal_power_reset(void) {
    furi_hal_cortex_system_reset();
}

#define ALL_WAKEUP_SOURCES                                                                   \
    (WDT_INTR_BASED_WAKEUP | MSEC_BASED_WAKEUP | SEC_BASED_WAKEUP | ALARM_BASED_WAKEUP |     \
     SDCSS_BASED_WAKEUP | ULPSS_BASED_WAKEUP | SYSRTC_BASED_WAKEUP | COMPR_BASED_WAKEUP |    \
     GPIO_BASED_WAKEUP | M4_PROCS_BASED_WAKEUP | WIRELESS_BASED_WAKEUP | HOST_BASED_WAKEUP | \
     DST_BASED_WAKEUP | WIC_BASED_WAKEUP)
#define IVT_OFFSET_ADDR        0x08202000
#define WKP_RAM_USAGE_LOCATION 0x24061EFC

void furi_hal_power_sleep_wakeup_clear(void) {
    MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE &= ~ALL_WAKEUP_SOURCES;
}

static void furi_hal_power_dummy_interrupt_handler(void* context) {
    UNUSED(context);
}

void furi_hal_power_sleep_wakeup_gpio(const GpioPin* wakeup_pin, GpioCondition wakeup_condition) {
    furi_check(wakeup_pin);
    furi_check(wakeup_pin->type == GpioTypeUulp); // HP and ULP are disabled in PS0 (deep sleep)

    furi_hal_gpio_init_ex(
        wakeup_pin, GpioModeInput, GpioPullNo, GpioSpeedLow, GpioAltFn2UulpWakeup);
    furi_hal_gpio_add_int_callback(
        wakeup_pin, wakeup_condition, furi_hal_power_dummy_interrupt_handler, NULL);
    furi_hal_gpio_enable_int_callback(wakeup_pin);

    MCU_FSM->GPIO_WAKEUP_REGISTER |= 1 << wakeup_pin->pin;
    MCU_FSM->MCU_FSM_SLEEP_CTRLS_AND_WAKEUP_MODE_b.GPIO_BASED_WAKEUP_b = 1;
}

void my_trigger_sleep(
    SLEEP_TYPE_T sleepType,
    uint8_t lf_clk_mode,
    uint32_t stack_address,
    uint32_t jump_cb_address,
    uint32_t vector_offset,
    uint32_t mode) {
    // Turn on the ULPSS RAM domains and retain ULPSS RAMs
    if((mode != RSI_WAKEUP_WITH_RETENTION_WO_ULPSS_RAM) ||
       (mode != RSI_WAKEUP_WO_RETENTION_WO_ULPSS_RAM)) {
        /* Turn on ULPSS SRAM domains*/
        RSI_PS_UlpssRamBanksPowerUp(
            ULPSS_2K_BANK_0 | ULPSS_2K_BANK_1 | ULPSS_2K_BANK_2 | ULPSS_2K_BANK_3);

        /* Turn on ULPSS SRAM Core/Periphery domains*/
        RSI_PS_UlpssRamBanksPeriPowerUp(
            ULPSS_2K_BANK_0 | ULPSS_2K_BANK_1 | ULPSS_2K_BANK_2 | ULPSS_2K_BANK_3);

        if((mode == RSI_WAKEUP_FROM_FLASH_MODE) || (mode == RSI_WAKEUP_WITH_RETENTION)
#if defined SLI_SI917B0
           || (mode == SL_SI91X_MCU_WAKEUP_PSRAM_MODE)
#endif // SLI_SI917B0
        ) {
            /* Retain ULPSS RAM*/
            RSI_PS_SetRamRetention(ULPSS_RAM_RETENTION_MODE_EN);
        }
    }

    // Peripherals needed on Wake-up (without RAM retention) needs to be powered up before going to sleep
    if((mode == RSI_WAKEUP_WITH_OUT_RETENTION) || (mode == RSI_WAKEUP_WO_RETENTION_WO_ULPSS_RAM)) {
        RSI_PS_M4ssPeriPowerUp(M4SS_PWRGATE_ULP_M4_DEBUG_FPU);
    }

#if(SL_SI91X_TICKLESS_MODE == 0)

    // Disabling the interrupts & clearing m4_is_active as m4 is going to sleep
    __disable_irq();

    // Indicate M4 is Inactive
    P2P_STATUS_REG &= ~M4_is_active;
    P2P_STATUS_REG;
    // Adding delay to sync m4 with NWP
    for(volatile uint8_t delay = 0; delay < 10; delay++) {
        __ASM("NOP");
    }

    //Disbling systick & clearing interrupt as systick is non-maskable interrupt
    SysTick->CTRL = DISABLE;
    NVIC_ClearPendingIRQ(SysTick_IRQn);

    //!Clear RX_BUFFER_VALID
    M4SS_P2P_INTR_CLR_REG = RX_BUFFER_VALID;
    M4SS_P2P_INTR_CLR_REG;
#endif // SL_SI91X_TICKLESS_MODE  == 0

#ifndef ENABLE_DEBUG_MODULE
    RSI_PS_M4ssPeriPowerDown(M4SS_PWRGATE_ULP_M4_DEBUG_FPU);
#endif // ENABLE_DEBUG_MODULE

    /* Define 'SLI_SI91X_MCU_ENABLE_FLASH_BASED_EXECUTION' macro if FLASH execution is needed*/
#ifndef SLI_SI91X_MCU_ENABLE_FLASH_BASED_EXECUTION
    RSI_PS_M4ssPeriPowerDown(M4SS_PWRGATE_ULP_QSPI_ICACHE);
    // Remove this if MCU is executing from Flash
#endif //SLI_SI91X_MCU_ENABLE_FLASH_BASED_EXECUTION

    // Move M4 SOC clock to ULP reference clock before going to PowerSave
    if(RSI_CLK_M4SocClkConfig(M4CLK, M4_ULPREFCLK, 0) != RSI_OK) {
        printf("RSI_CLK_M4SocClkConfig failed\n");
    }

    /* Check whether M4 is using XTAL */
    if(sli_si91x_is_xtal_in_use_by_m4() == true) {
        /* If M4 is using XTAL then request NWP to turn OFF XTAL as M4 is going to sleep */
        sli_si91x_raise_xtal_interrupt_to_ta(TURN_OFF_XTAL_REQUEST);
    }

    // Configure sleep parameters required by bootloader upon Wake-up
    RSI_PS_RetentionSleepConfig(stack_address, jump_cb_address, vector_offset, mode);

    // Trigger M4 to sleep
    RSI_PS_EnterDeepSleep(sleepType, lf_clk_mode);
}

void furi_hal_power_deep_sleep(void) {
    furi_delay_ms(10); // allow previous logs to flush

    FURI_CRITICAL_ENTER();

    static const FuriHalBus blocklist[] = {
        FuriHalBusUSART1_PCLK,
        FuriHalBusUSART1_SCLK,
        FuriHalBusRPDMA_HCLK,
        FuriHalBusUDMA_HCLK,
        FuriHalBusQEI_PCLK,
        FuriHalBusMCPWM_PCLK,
    };
    for(size_t i = 0; i < COUNT_OF(blocklist); i++) {
        FuriHalBus bus = blocklist[i];
        if(furi_hal_bus_is_enabled(bus)) {
            furi_hal_bus_disable(bus);
        }
    }

    sl_si91x_configure_ram_retention(
        WISEMCU_320KB_RAM_IN_USE, WISEMCU_RETAIN_DEFAULT_RAM_DURING_SLEEP);
    my_trigger_sleep(
        SLEEP_WITHOUT_RETENTION,
        DISABLE_LF_MODE,
        WKP_RAM_USAGE_LOCATION,
        (uint32_t)0,
        IVT_OFFSET_ADDR,
        RSI_WAKEUP_FROM_FLASH_MODE);

    furi_crash("failed to enter sleep");
    FURI_CRITICAL_EXIT();
}

FuriHalPowerResetSource furi_hal_power_get_reset_source(void) {
    return furi_hal_power.reset_source;
}
