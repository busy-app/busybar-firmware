#pragma once

#include <stm32u5xx_ll_rcc.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Early initialization */
void furi_hal_clock_init_early(void);

/** Early deinitialization */
void furi_hal_clock_deinit_early(void);

/** Initialize clocks */
void furi_hal_clock_init(void);

/** Stop SysTick counter without resetting */
void furi_hal_clock_suspend_tick(void);

/** Continue SysTick counter operation */
void furi_hal_clock_resume_tick(void);

/** Switch clock from HSE to HSI */
void furi_hal_clock_switch_hse2hsi(void);

/** Switch clock from HSI to HSE */
void furi_hal_clock_switch_hsi2hse(void);

/** Switch clock from HSE to PLL
 *
 * @return     true if changed, false if failed or not possible at this moment
 */
bool furi_hal_clock_switch_hse2pll(void);

/** Switch clock from PLL to HSE
 *
 * @return     true if changed, false if failed or not possible at this moment
 */
bool furi_hal_clock_switch_pll2hse(void);

typedef enum {
    FuriHalClockHwSdMmc12,
    FuriHalClockHwSAI1,
} FuriHalClockHW;

uint32_t furi_hal_clock_get_freq(FuriHalClockHW hw);

#ifdef __cplusplus
}
#endif
