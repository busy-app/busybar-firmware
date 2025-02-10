#include <core/log.h>
#include <core/check.h>
#include <furi_hal_bus.h>
#include <furi_hal_clock.h>
#include <stm32u5xx_ll_cortex.h>
#include <stm32u5xx_ll_system.h>
#include <stm32u5xx_ll_pwr.h>
#include <stm32u5xx_ll_utils.h>

#define TAG "FuriHalClock"

#define CPU_CLOCK_EARLY_HZ 4000000U
#define TICK_INT_PRIORITY  15U

// Common PLL1 settings
#define FURI_CLOCK_PLL1_M 1
#define FURI_CLOCK_PLL1_N 10
// PLL1R is used for system clock
#define FURI_CLOCK_PLL1_R 1
// PLL1P is used for SDMMC12 and SAI1 blocks
#define FURI_CLOCK_PLL1_P 1

void furi_hal_clock_init_early(void) {
    LL_SetSystemCoreClock(CPU_CLOCK_EARLY_HZ);
    LL_InitTick(SystemCoreClock, 1000U);
}

void furi_hal_clock_deinit_early(void) {
}

void furi_hal_clock_init(void) {
    // TODO: here lies the dragon
    // We need to figure out how to properly initialize the clocks

    LL_SYSTICK_DisableIT();
    furi_hal_bus_enable(FuriHalBusPWR);
    furi_hal_bus_enable(FuriHalBusSYSCFG);

    // Todo move to furi_hal_bus? furi_hal_cortex?
    LL_PWR_EnableVddIO2();

    LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
    while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_4) {
    }

    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    while(LL_PWR_IsActiveFlag_VOS() == 0) {
    }

    LL_RCC_HSE_Enable();
    while(LL_RCC_HSE_IsReady() != 1) {
    }

    // PLL1R used for system clock, 160 MHz
    LL_RCC_PLL1_ConfigDomain_SYS(
        LL_RCC_PLL1SOURCE_HSE, FURI_CLOCK_PLL1_M, FURI_CLOCK_PLL1_N, FURI_CLOCK_PLL1_R);

    // PLL1P used for SDMMC block
    // ST has very weird naming conventions for the PLLs
    LL_RCC_PLL1_ConfigDomain_SAI(
        LL_RCC_PLL1SOURCE_HSE, FURI_CLOCK_PLL1_M, FURI_CLOCK_PLL1_N, FURI_CLOCK_PLL1_P);

    LL_RCC_PLL1_EnableDomain_SYS();
    LL_RCC_PLL1_EnableDomain_SAI();
    LL_RCC_SetPll1EPodPrescaler(LL_RCC_PLL1MBOOST_DIV_1);
    LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_8_16);
    LL_RCC_PLL1_Enable();

    while(LL_RCC_PLL1_IsReady() != 1) {
    }

    /* Intermediate AHB prescaler 2 when target frequency clock is higher than 80 MHz */
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL1) {
    }

    /* Insure 1us transition state at intermediate medium speed clock*/
    for(__IO uint32_t i = (160 >> 1); i != 0; i--)
        ;

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_1);

    LL_SetSystemCoreClock(160000000);
    LL_InitTick(SystemCoreClock, 1000U);
    LL_SYSTICK_EnableIT();

    // Enable LSI for LPTimer
    LL_PWR_EnableBkUpAccess();
    while(LL_PWR_IsEnabledBkUpAccess() == 0U) {
    }

    // LL_RCC_LSI_Enable();
    // while(LL_RCC_LSI_IsReady() != 1) {
    // }

    LL_RCC_HSI_Enable();
    while(LL_RCC_HSI_IsReady() != 1) {
    }

    LL_RCC_MSI_EnableRangeSelection();
    LL_RCC_MSI_SetCalibTrimming(16, LL_RCC_MSI_OSCILLATOR_0);
    LL_RCC_MSIK_SetRange(LL_RCC_MSIKRANGE_15);
    LL_RCC_MSIK_Enable();
    while(LL_RCC_MSIK_IsReady() != 1) {
    }

    FURI_LOG_I(TAG, "Init OK");
}

uint32_t furi_hal_clock_get_freq(FuriHalClockHW hw) {
    switch(hw) {
    case FuriHalClockHwSdMmc12:
        furi_check(
            LL_RCC_GetSDMMCKernelClockSource(LL_RCC_SDMMC_KERNELCLKSOURCE) ==
            LL_RCC_SDMMC12_KERNELCLKSOURCE_PLL1);
        furi_check(LL_RCC_PLL1_GetMainSource() == LL_RCC_PLL1SOURCE_HSE);
        furi_check(LL_RCC_PLL1_GetDivider() == FURI_CLOCK_PLL1_M);
        furi_check(LL_RCC_PLL1_GetN() == FURI_CLOCK_PLL1_N);
        furi_check(LL_RCC_PLL1_GetP() == FURI_CLOCK_PLL1_P);
        return (HSE_VALUE / FURI_CLOCK_PLL1_M * FURI_CLOCK_PLL1_N / FURI_CLOCK_PLL1_P);
    case FuriHalClockHwSAI1:
        furi_check(LL_RCC_GetSAIClockSource(LL_RCC_SAI1_CLKSOURCE) == LL_RCC_SAI1_CLKSOURCE_PLL1);
        furi_check(LL_RCC_PLL1_GetMainSource() == LL_RCC_PLL1SOURCE_HSE);
        furi_check(LL_RCC_PLL1_GetDivider() == FURI_CLOCK_PLL1_M);
        furi_check(LL_RCC_PLL1_GetN() == FURI_CLOCK_PLL1_N);
        furi_check(LL_RCC_PLL1_GetP() == FURI_CLOCK_PLL1_P);
        return (HSE_VALUE / FURI_CLOCK_PLL1_M * FURI_CLOCK_PLL1_N / FURI_CLOCK_PLL1_P);

    default:
        furi_crash("Unknown hardware");
        break;
    }

    return 0;
}
