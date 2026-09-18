#include <furi_hal.h>
#include <stm32u5xx_ll_pwr.h>
#include <stm32u5xx_ll_cortex.h>

typedef struct {
    volatile uint8_t insomnia;
    volatile FuriHalPowerResetSource reset_source;
} FuriHalPower;

static volatile FuriHalPower furi_hal_power = {
    .insomnia = 0,
    .reset_source = FuriHalPowerResetSourceUnknown,
};

void furi_hal_power_init_super_early(void) {
    volatile uint32_t* const status_register = &PWR->WUSR;
    if(*status_register) {
        furi_hal_power.reset_source = FuriHalPowerResetSourceWakeup;
    } else {
        // no way to differentiate between POR and non-power-on reset on STM
        furi_hal_power.reset_source = FuriHalPowerResetSourceResetLine;
    }

    volatile uint32_t* const status_clear_register = &PWR->WUSCR;
    *status_clear_register |= *status_register;
}

void furi_hal_power_reset(void) {
    furi_hal_cortex_system_reset();
}

void furi_hal_power_reset_917(bool to_dfu) {
    furi_hal_gpio_write(&gpio_917_swo, true);
    furi_hal_gpio_write(&gpio_917_rst, true);

    furi_hal_gpio_init_simple(&gpio_917_swo, GpioModeOutputPushPull);
    furi_hal_gpio_init_simple(&gpio_917_rst, GpioModeOutputPushPull);

    if(to_dfu) {
        furi_hal_gpio_write(&gpio_917_swo, false);
    }
    furi_hal_gpio_write(&gpio_917_rst, false);
    furi_delay_ms(20);
    furi_hal_gpio_write(&gpio_917_rst, true);

    if(to_dfu) {
        furi_delay_ms(150);
    }

    furi_hal_gpio_init_simple(&gpio_917_swo, GpioModeAnalog);
    furi_hal_gpio_init_simple(&gpio_917_rst, GpioModeAnalog);
}

uint16_t furi_hal_power_insomnia_level(void) {
    return furi_hal_power.insomnia;
}

void furi_hal_power_insomnia_enter(void) {
    FURI_CRITICAL_ENTER();
    furi_check(furi_hal_power.insomnia < UINT8_MAX);
    furi_hal_power.insomnia++;
    FURI_CRITICAL_EXIT();
}

void furi_hal_power_insomnia_exit(void) {
    FURI_CRITICAL_ENTER();
    furi_check(furi_hal_power.insomnia > 0);
    furi_hal_power.insomnia--;
    FURI_CRITICAL_EXIT();
}

bool furi_hal_power_sleep_available(void) {
    return furi_hal_power.insomnia == 0;
}

void furi_hal_power_sleep_wakeup_clear(void) {
    FURI_CRITICAL_ENTER();
    LL_PWR_ClearFlag_WU();
    FURI_CRITICAL_EXIT();
}

void furi_hal_power_sleep_wakeup_gpio(const GpioPin* wakeup_pin, GpioMode condition) {
    furi_check(wakeup_pin);
    furi_check((condition == GpioModeInterruptRise) || (condition == GpioModeInterruptFall));
    furi_check(wakeup_pin->wakeup_line != GpioWakeupLineNone);
    furi_check(wakeup_pin->wakeup_mux != GpioWakeupMultiplexNone);

    FURI_CRITICAL_ENTER();

    furi_hal_gpio_init(wakeup_pin, GpioModeInput, GpioPullNo, GpioSpeedVeryHigh);

    const size_t index = wakeup_pin->wakeup_line - 1;
    volatile uint32_t* const enable_register = &PWR->WUCR1;
    volatile uint32_t* const multiplex_register = &PWR->WUCR3;

    uint32_t enable_mask = 1 << index;
    furi_check(!(*enable_register & enable_mask), "wakeup line conflict");

    if(condition == GpioModeInterruptFall) {
        LL_PWR_SetWakeUpPinPolarityLow(1 << index);
    } else {
        LL_PWR_SetWakeUpPinPolarityHigh(1 << index);
    }

    MODIFY_REG(
        *multiplex_register,
        PWR_WUCR3_WUSEL1_Msk << (index * 2),
        wakeup_pin->wakeup_mux << (index * 2));

    *enable_register |= enable_mask;

    FURI_LOG_D("sleep", "WUCR1 %032lb", PWR->WUCR1);
    FURI_LOG_D("sleep", "WUCR2 %032lb", PWR->WUCR2);
    FURI_LOG_D("sleep", "WUCR3 %032lb", PWR->WUCR3);

    NVIC_SetPriority(PWR_S3WU_IRQn, 0);
    NVIC_EnableIRQ(PWR_S3WU_IRQn);
    NVIC_SetPriority(RCC_IRQn, 0);
    NVIC_EnableIRQ(RCC_IRQn);

    LL_PWR_EnablePUPDConfig();

    FURI_CRITICAL_EXIT();
}

void furi_hal_power_deep_sleep(void) {
    FURI_CRITICAL_ENTER();

    LL_PWR_DisableBkUpRegulator();
    while(LL_PWR_IsEnabledBkUpRegulator())
        ;
    LL_PWR_ClearFlag_WU();
    LL_PWR_SetPowerMode(LL_PWR_STANDBY_MODE);

    SysTick->CTRL = DISABLE;
    NVIC_ClearPendingIRQ(SysTick_IRQn);

    __disable_irq();
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    __asm__ volatile("wfi\n"
                     "nop\n"
                     "nop\n");

    furi_crash("failed to enter deep sleep");
    FURI_CRITICAL_EXIT();
}

FuriHalPowerResetSource furi_hal_power_get_reset_source(void) {
    return furi_hal_power.reset_source;
}
