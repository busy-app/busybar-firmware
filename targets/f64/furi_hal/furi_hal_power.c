#include "furi_hal_power.h"
#include "furi_hal_cortex.h"
#include "furi_hal_bus.h"
#include "furi_hal_qspi_flash.h"

#include <sli_wifi_utility.h>
#include <si91x_device.h>
#include <rsi_wisemcu_hardware_setup.h>
#include <rsi_rom_power_save.h>
#include <rsi_rom_clks.h>
#include <sli_si91x_power_manager.h>
#include <sli_si91x_clock_manager.h>

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
// #define IVT_OFFSET_ADDR        0x08202000
// #define WKP_RAM_USAGE_LOCATION 0x24061EFC
#define SL_SLEEP_STACK_USAGE_ADDRESS     0x1b000
#define SL_SLEEP_VECTOR_OFFSET           0x0
#define SL_SLEEP_WAKEUP_CALLBACK_ADDRESS 0x1e000

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

static void furi_hal_power_trigger_nwp_sleep(void) {
    sl_wifi_performance_profile_v2_t wifi_profile = {
        .profile = SL_WIFI_SYSTEM_DEEP_SLEEP_WITHOUT_RAM_RETENTION};
    sl_bt_performance_profile_t bt_profile = {
        .profile = SL_WIFI_SYSTEM_DEEP_SLEEP_WITHOUT_RAM_RETENTION};
    sl_status_t status = sli_wifi_send_power_save_request(&wifi_profile, &bt_profile);
    if(status != SL_STATUS_OK) {
        FURI_LOG_E("sleep", "err: 0x%lx", status);
        furi_crash("failed to put NWP to sleep");
    }
}

/**
 * @brief Equivalent to `trigger_sleep` in sli_si91x_power_manager.c, but one
 * that we can call directly
 */
static void furi_hal_power_trigger_m4_sleep(sli_power_sleep_config_t* config, uint8_t sleep_type) {
    sl_si91x_clock_manager_control_pll(SOC_PLL, ENABLE);
    RSI_PS_RetentionSleepConfig(
        config->stack_address,
        config->wakeup_callback_address,
        config->vector_offset,
        config->mode);
    sl_si91x_clock_manager_control_pll(SOC_PLL, DISABLE);
    sl_si91x_clock_manager_control_pll(INTF_PLL, DISABLE);
    sl_si91x_clock_manager_control_pll(I2S_PLL, DISABLE);
    rsi_error_t status = RSI_PS_EnterDeepSleep(sleep_type, config->low_freq_clock);
    if(status != SL_STATUS_OK) {
        FURI_LOG_E("sleep", "err: 0x%x", status);
        furi_crash("failed to put M4 to sleep");
    }
}

void furi_hal_power_deep_sleep(void) {
    furi_thread_set_current_priority(FuriThreadPriorityHighest);

    furi_hal_power_trigger_nwp_sleep();

    FURI_CRITICAL_ENTER();

    furi_check(furi_hal_qspi_flash_clk_config(M4CLK, QSPI_ULPREFCLK, 0, 0, 1) == RSI_OK);
    sli_si91x_power_manager_low_power_hw_config(true);
    sli_si91x_clock_manager_config_clks_on_ps_change(
        SL_SI91X_POWER_MANAGER_SLEEP, SL_SI91X_POWER_MANAGER_POWERSAVE);

    sli_power_sleep_config_t config = {
        .low_freq_clock = SLI_SI91X_POWER_MANAGER_DISABLE_LF_MODE,
        .mode = SLI_SI91X_POWER_MANAGER_WAKEUP_FROM_FLASH_MODE,
        .stack_address = SL_SLEEP_STACK_USAGE_ADDRESS,
        .vector_offset = SL_SLEEP_VECTOR_OFFSET,
        .wakeup_callback_address = SL_SLEEP_WAKEUP_CALLBACK_ADDRESS,
    };
    furi_hal_power_trigger_m4_sleep(&config, SLEEP_WITHOUT_RETENTION);

    furi_crash("failed to enter sleep");
    FURI_CRITICAL_EXIT();
}

FuriHalPowerResetSource furi_hal_power_get_reset_source(void) {
    return furi_hal_power.reset_source;
}
