/***************************************************************************//**
 * @file
 * @brief Security Management Unit (SMU) peripheral API
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef SL_HAL_SMU_H
#define SL_HAL_SMU_H

#include "em_device.h"
#if defined(SMU_COUNT) && (SMU_COUNT > 0)

#include "sl_assert.h"
#include "sl_hal_bus.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup smu SMU - Security Management Unit
 * @brief Security Management Unit (SMU) Peripheral API
 *
 * @details
 *   SMU forms the control and status/reporting component of bus-level
 *   security in EFM32/EFR32 devices.
 *
 *   Peripheral-level protection is provided via the Peripheral Protection Unit
 *   (PPU). PPU provides hardware access barrier to any peripheral that is
 *   configured to be protected. When an attempt is made to access a peripheral
 *   without the required privilege/security level, PPU detects the fault
 *   and intercepts the access. No write or read of the peripheral register
 *   space occurs, and an all-zero value is returned if the access is a read.
 * @{
 ******************************************************************************/

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

/// SMU peripheral identifiers.
SL_ENUM(sl_hal_smu_peripheral_id_t) {
#if defined(_SILICON_LABS_32B_SERIES_2)
#if defined(SMU_PPUPATD0_SCRATCHPAD)
  SL_HAL_SMU_SCRATCHPAD   = _SMU_PPUPATD0_SCRATCHPAD_SHIFT,     /**< SMU peripheral identifier for SCRATCHPAD */
#endif
  SL_HAL_SMU_EMU          = _SMU_PPUPATD0_EMU_SHIFT,            /**< SMU peripheral identifier for EMU       */
  SL_HAL_SMU_CMU          = _SMU_PPUPATD0_CMU_SHIFT,            /**< SMU peripheral identifier for CMU       */
#if defined(_SMU_PPUPATD0_HFXO0_SHIFT)
  SL_HAL_SMU_HFXO         = _SMU_PPUPATD0_HFXO0_SHIFT,          /**< SMU peripheral identifier for HFXO0     */
#endif
#if defined(_SMU_PPUPATD1_HFXO0_SHIFT)
  SL_HAL_SMU_HFXO         = 32 + _SMU_PPUPATD1_HFXO0_SHIFT,     /**< SMU peripheral identifier for HFXO0     */
#endif
  SL_HAL_SMU_HFRCO0       = _SMU_PPUPATD0_HFRCO0_SHIFT,         /**< SMU peripheral identifier for HFRCO0    */
  SL_HAL_SMU_FSRCO        = _SMU_PPUPATD0_FSRCO_SHIFT,          /**< SMU peripheral identifier for FSRCO     */
  SL_HAL_SMU_DPLL0        = _SMU_PPUPATD0_DPLL0_SHIFT,          /**< SMU peripheral identifier for DPLL0     */
  SL_HAL_SMU_LFXO         = _SMU_PPUPATD0_LFXO_SHIFT,           /**< SMU peripheral identifier for LFXO      */
  SL_HAL_SMU_LFRCO        = _SMU_PPUPATD0_LFRCO_SHIFT,          /**< SMU peripheral identifier for LFRCO     */
  SL_HAL_SMU_ULFRCO       = _SMU_PPUPATD0_ULFRCO_SHIFT,         /**< SMU peripheral identifier for ULFRCO    */
  SL_HAL_SMU_MSC          = _SMU_PPUPATD0_MSC_SHIFT,            /**< SMU peripheral identifier for MSC       */
  SL_HAL_SMU_ICACHE0      = _SMU_PPUPATD0_ICACHE0_SHIFT,        /**< SMU peripheral identifier for ICACHE0   */
  SL_HAL_SMU_PRS          = _SMU_PPUPATD0_PRS_SHIFT,            /**< SMU peripheral identifier for PRS       */
  SL_HAL_SMU_GPIO         = _SMU_PPUPATD0_GPIO_SHIFT,           /**< SMU peripheral identifier for GPIO      */
  SL_HAL_SMU_LDMA         = _SMU_PPUPATD0_LDMA_SHIFT,           /**< SMU peripheral identifier for LDMA      */
  SL_HAL_SMU_LDMAXBAR     = _SMU_PPUPATD0_LDMAXBAR_SHIFT,       /**< SMU peripheral identifier for LDMAXBAR  */
  SL_HAL_SMU_TIMER0       = _SMU_PPUPATD0_TIMER0_SHIFT,         /**< SMU peripheral identifier for TIMER0    */
  SL_HAL_SMU_TIMER1       = _SMU_PPUPATD0_TIMER1_SHIFT,         /**< SMU peripheral identifier for TIMER1    */
  SL_HAL_SMU_TIMER2       = _SMU_PPUPATD0_TIMER2_SHIFT,         /**< SMU peripheral identifier for TIMER2    */
  SL_HAL_SMU_TIMER3       = _SMU_PPUPATD0_TIMER3_SHIFT,         /**< SMU peripheral identifier for TIMER3    */
#if defined(_SMU_PPUPATD0_TIMER4_SHIFT)
  SL_HAL_SMU_TIMER4       = _SMU_PPUPATD0_TIMER4_SHIFT,         /**< SMU peripheral identifier for TIMER4    */
#endif
#if defined(_SMU_PPUPATD0_TIMER5_SHIFT)
  SL_HAL_SMU_TIMER5       = _SMU_PPUPATD0_TIMER5_SHIFT,         /**< SMU peripheral identifier for TIMER5    */
#endif
#if defined(_SMU_PPUPATD0_TIMER6_SHIFT)
  SL_HAL_SMU_TIMER6       = _SMU_PPUPATD0_TIMER6_SHIFT,         /**< SMU peripheral identifier for TIMER6    */
#endif
#if defined(_SMU_PPUPATD0_TIMER7_SHIFT)
  SL_HAL_SMU_TIMER7       = _SMU_PPUPATD0_TIMER7_SHIFT,         /**< SMU peripheral identifier for TIMER7    */
#endif
#if defined(_SMU_PPUPATD0_USART0_SHIFT)
  SL_HAL_SMU_USART0       = _SMU_PPUPATD0_USART0_SHIFT,         /**< SMU peripheral identifier for USART0    */
#endif
#if defined(_SMU_PPUPATD0_USART1_SHIFT)
  SL_HAL_SMU_USART1       = _SMU_PPUPATD0_USART1_SHIFT,         /**< SMU peripheral identifier for USART1    */
#endif
#if defined(_SMU_PPUPATD0_USART2_SHIFT)
  SL_HAL_SMU_USART2       = _SMU_PPUPATD0_USART2_SHIFT,         /**< SMU peripheral identifier for USART2    */
#endif
  SL_HAL_SMU_BURTC        = _SMU_PPUPATD0_BURTC_SHIFT,          /**< SMU peripheral identifier for BURTC     */
#if defined(_SMU_PPUPATD0_I2C1_SHIFT)
  SL_HAL_SMU_I2C1         = _SMU_PPUPATD0_I2C1_SHIFT,           /**< SMU peripheral identifier for I2C1      */
#endif
#if defined(_SMU_PPUPATD0_CHIPTESTCTRL_SHIFT)
  SL_HAL_SMU_CHIPTESTCTRL = _SMU_PPUPATD0_CHIPTESTCTRL_SHIFT,   /**< SMU peripheral identifier for CHIPTESTCTRL */
#endif
#if defined(_SMU_PPUPATD0_SYSCFGCFGNS_SHIFT)
  SL_HAL_SMU_SYSCFGCFGNS  = _SMU_PPUPATD0_SYSCFGCFGNS_SHIFT,    /**< SMU peripheral identifier for SYSCFGCFGNS */
#endif

#if defined(SMU_PPUPATD0_LVGD)
  SL_HAL_SMU_LVGD         = _SMU_PPUPATD0_LVGD_SHIFT,           /**< SMU peripheral identifier for LVGD      */
#endif
  SL_HAL_SMU_SYSCFG       = _SMU_PPUPATD0_SYSCFG_SHIFT,         /**< SMU peripheral identifier for SYSCFG    */
#if defined(_SMU_PPUPATD0_BURAM_SHIFT)
  SL_HAL_SMU_BURAM        = _SMU_PPUPATD0_BURAM_SHIFT,          /**< SMU peripheral identifier for BURAM     */
#endif
#if defined(_SMU_PPUPATD1_BURAM_SHIFT)
  SL_HAL_SMU_BURAM        = _SMU_PPUPATD1_BURAM_SHIFT,          /**< SMU peripheral identifier for BURAM     */
#endif
#if defined(_SMU_PPUPATD0_IFADCDEBUG_SHIFT)
  SL_HAL_SMU_IFADCDEBUG   = _SMU_PPUPATD0_IFADCDEBUG_SHIFT,     /**< SMU peripheral identifier for IFADCDEBUG*/
#endif
#if defined(_SMU_PPUPATD0_GPCRC_SHIFT)
  SL_HAL_SMU_GPCRC        = _SMU_PPUPATD0_GPCRC_SHIFT,          /**< SMU peripheral identifier for GPCRC     */
#endif
#if defined(_SMU_PPUPATD1_GPCRC_SHIFT)
  SL_HAL_SMU_GPCRC        = 32 + _SMU_PPUPATD1_GPCRC_SHIFT,     /**< SMU peripheral identifier for GPCRC     */
#endif
#if defined(_SMU_PPUPATD0_DCDC_SHIFT)
  SL_HAL_SMU_DCDC         = _SMU_PPUPATD0_DCDC_SHIFT,           /**< SMU peripheral identifier for DCDC      */
#endif
#if defined(_SMU_PPUPATD0_RTCC_SHIFT)
  SL_HAL_SMU_RTCC         = _SMU_PPUPATD0_RTCC_SHIFT,           /**< SMU peripheral identifier for RTCC      */
#endif
#if defined(_SMU_PPUPATD0_HOSTMAILBOX_SHIFT)
  SL_HAL_SMU_HOSTMAILBOX  = _SMU_PPUPATD0_HOSTMAILBOX_SHIFT,   /**< SMU peripheral identifier for HOSTMAILBOX */
#endif
#if defined(_SMU_PPUPATD1_EUSART0_SHIFT)
  SL_HAL_SMU_EUSART0      = 32 + _SMU_PPUPATD1_EUSART0_SHIFT,    /**< SMU peripheral identifier for EUSART0   */
#endif
#if defined(_SMU_PPUPATD0_EUSART1_SHIFT)
  SL_HAL_SMU_EUSART1      = _SMU_PPUPATD0_EUSART1_SHIFT,        /**< SMU peripheral identifier for EUSART1   */
#endif
#if defined(_SMU_PPUPATD1_EUSART1_SHIFT)
  SL_HAL_SMU_EUSART1      = 32 + _SMU_PPUPATD1_EUSART1_SHIFT,   /**< SMU peripheral identifier for EUSART1   */
#endif
#if defined(_SMU_PPUPATD0_EUSART2_SHIFT)
  SL_HAL_SMU_EUSART2      = _SMU_PPUPATD0_EUSART2_SHIFT,        /**< SMU peripheral identifier for EUSART2   */
#endif
#if defined(_SMU_PPUPATD1_EUSART2_SHIFT)
  SL_HAL_SMU_EUSART2      = 32 + _SMU_PPUPATD1_EUSART2_SHIFT,   /**< SMU peripheral identifier for EUSART2   */
#endif
#if defined(_SMU_PPUPATD1_EUSART3_SHIFT)
  SL_HAL_SMU_EUSART3      = 32 + _SMU_PPUPATD1_EUSART3_SHIFT,   /**< SMU peripheral identifier for EUSART3   */
#endif
#if defined(_SMU_PPUPATD1_EUSART4_SHIFT)
  SL_HAL_SMU_EUSART4      = 32 + _SMU_PPUPATD1_EUSART4_SHIFT,   /**< SMU peripheral identifier for EUSART4   */
#endif
#if defined(_SMU_PPUPATD1_RTCC_SHIFT)
  SL_HAL_SMU_RTCC         = 32 + _SMU_PPUPATD1_RTCC_SHIFT,      /**< SMU peripheral identifier for RTCC      */
#endif
#if defined(_SMU_PPUPATD1_SYSRTC_SHIFT)
  SL_HAL_SMU_SYSRTC       = 32 + _SMU_PPUPATD1_SYSRTC_SHIFT,    /**< SMU peripheral identifier for SYSRTC    */
#endif
#if defined(_SMU_PPUPATD1_LCD_SHIFT)
  SL_HAL_SMU_LCD          = 32 + _SMU_PPUPATD1_LCD_SHIFT,       /**< SMU peripheral identifier for LCD       */
#endif
#if defined(_SMU_PPUPATD1_KEYSCAN_SHIFT)
  SL_HAL_SMU_KEYSCAN      = 32 + _SMU_PPUPATD1_KEYSCAN_SHIFT,   /**< SMU peripheral identifier for KEYSCAN   */
#endif
#if defined(_SMU_PPUPATD1_DMEM_SHIFT)
  SL_HAL_SMU_DMEM         = 32 + _SMU_PPUPATD1_DMEM_SHIFT,      /**< SMU peripheral identifier for DMEM      */
#endif
#if defined(_SMU_PPUPATD1_LCDRF_SHIFT)
  SL_HAL_SMU_LCDRF        = 32 + _SMU_PPUPATD1_LCDRF_SHIFT,     /**< SMU peripheral identifier for LCDRF     */
#endif
#if defined(_SMU_PPUPATD1_PFMXPPRF_SHIFT)
  SL_HAL_SMU_PFMXPPRF     = 32 + _SMU_PPUPATD1_PFMXPPRF_SHIFT,  /**< SMU peripheral identifier for PFMXPPRF  */
#endif
#if defined(_SMU_PPUPATD1_RFFPLL0_SHIFT)
  SL_HAL_SMU_RFFPLL0      = 32 + _SMU_PPUPATD1_RFFPLL0_SHIFT,   /**< SMU peripheral identifier for RFFPLL0   */
#endif
#if defined(_SMU_PPUPATD1_ETAMPDET_SHIFT)
  SL_HAL_SMU_ETAMPDET     = 32 + _SMU_PPUPATD1_ETAMPDET_SHIFT,  /**< SMU peripheral identifier for ETAMPDET  */
#endif
#if defined(_SMU_PPUPATD1_VDAC0_SHIFT)
  SL_HAL_SMU_VDAC0        = 32 + _SMU_PPUPATD1_VDAC0_SHIFT,     /**< SMU peripheral identifier for VDAC0     */
#endif
#if defined(_SMU_PPUPATD1_VDAC1_SHIFT)
  SL_HAL_SMU_VDAC1        = 32 + _SMU_PPUPATD1_VDAC1_SHIFT,     /**< SMU peripheral identifier for VDAC1     */
#endif
#if defined(_SMU_PPUPATD1_PCNT_SHIFT)
  SL_HAL_SMU_PCNT         = 32 + _SMU_PPUPATD1_PCNT_SHIFT,      /**< SMU peripheral identifier for PCNT      */
#endif
#if defined(_SMU_PPUPATD1_LESENSE_SHIFT)
  SL_HAL_SMU_LESENSE      = 32 + _SMU_PPUPATD1_LESENSE_SHIFT,   /**< SMU peripheral identifier for LESENSE   */
#endif
#if defined(_SMU_PPUPATD1_HFRCO1_SHIFT)
  SL_HAL_SMU_HFRCO1       = 32 + _SMU_PPUPATD1_HFRCO1_SHIFT,    /**< SMU peripheral identifier for HFRCO1    */
#endif
#if defined(_SMU_PPUPATD1_HFXO0_SHIFT)
  SL_HAL_SMU_HFXO0        = 32 + _SMU_PPUPATD1_HFXO0_SHIFT,     /**< SMU peripheral identifier for HFXO0     */
#endif
#if defined(_SMU_PPUPATD1_DCDC_SHIFT)
  SL_HAL_SMU_DCDC         = 32 + _SMU_PPUPATD1_DCDC_SHIFT,      /**< SMU peripheral identifier for DCDC      */
#endif
#if defined(_SMU_PPUPATD1_PDM_SHIFT)
  SL_HAL_SMU_PDM          = 32 + _SMU_PPUPATD1_PDM_SHIFT,       /**< SMU peripheral identifier for PDM       */
#endif
#if defined(_SMU_PPUPATD1_RFSENSE_SHIFT)
  SL_HAL_SMU_RFSENSE      = 32 + _SMU_PPUPATD1_RFSENSE_SHIFT,   /**< SMU peripheral identifier for RFSENSE   */
#endif
#if defined(_SMU_PPUPATD1_SEPUF_SHIFT)
  SL_HAL_SMU_SEPUF        = 32 + _SMU_PPUPATD1_SEPUF_SHIFT,     /**< SMU peripheral identifier for SEPUF     */
#endif
  SL_HAL_SMU_LETIMER0     = 32 + _SMU_PPUPATD1_LETIMER0_SHIFT,  /**< SMU peripheral identifier for LETIMER   */
#if defined(_SMU_PPUPATD1_IADC0_SHIFT)
  SL_HAL_SMU_IADC0        = 32 + _SMU_PPUPATD1_IADC0_SHIFT,     /**< SMU peripheral identifier for IADC0     */
#endif
#if defined(_SMU_PPUPATD1_ACMP0_SHIFT)
  SL_HAL_SMU_ACMP0        = 32 + _SMU_PPUPATD1_ACMP0_SHIFT,     /**< SMU peripheral identifier for ACMP0     */
#endif
#if defined(_SMU_PPUPATD1_ACMP1_SHIFT)
  SL_HAL_SMU_ACMP1        = 32 + _SMU_PPUPATD1_ACMP1_SHIFT,     /**< SMU peripheral identifier for ACMP1     */
#endif
#if defined(_SMU_PPUPATD1_I2C0_SHIFT)
  SL_HAL_SMU_I2C0         = 32 + _SMU_PPUPATD1_I2C0_SHIFT,      /**< SMU peripheral identifier for I2C0      */
#endif
#if defined(_SMU_PPUPATD1_HFRCOEM23_SHIFT)
  SL_HAL_SMU_HFRCOEM23    = 32 + _SMU_PPUPATD1_HFRCOEM23_SHIFT, /**< SMU peripheral identifier for HFRCOEM23 */
#endif
  SL_HAL_SMU_WDOG0        = 32 + _SMU_PPUPATD1_WDOG0_SHIFT,     /**< SMU peripheral identifier for WDOG0     */
#if defined(_SMU_PPUPATD1_WDOG1_SHIFT)
  SL_HAL_SMU_WDOG1        = 32 + _SMU_PPUPATD1_WDOG1_SHIFT,     /**< SMU peripheral identifier for WDOG1     */
#endif
  SL_HAL_SMU_AMUXCP0      = 32 + _SMU_PPUPATD1_AMUXCP0_SHIFT,   /**< SMU peripheral identifier for AMUXCP0   */
#if defined(_SMU_PPUPATD1_RADIOAES_SHIFT)
  SL_HAL_SMU_RADIOAES     = 32 + _SMU_PPUPATD1_RADIOAES_SHIFT,  /**< SMU peripheral identifier for RADIOAES  */
#endif
#if defined(_SMU_PPUPATD1_EUART0_SHIFT)
  SL_HAL_SMU_EUART0       = 32 + _SMU_PPUPATD1_EUART0_SHIFT,    /**< SMU peripheral identifier for EUART0    */
#endif
#if defined(_SMU_PPUPATD1_BUFC_SHIFT)
  SL_HAL_SMU_BUFC         = 32 + _SMU_PPUPATD1_BUFC_SHIFT,      /**< SMU peripheral identifier for BUFC      */
#endif
  SL_HAL_SMU_SMU          = 32 + _SMU_PPUPATD1_SMU_SHIFT,       /**< SMU peripheral identifier for SMU       */
#if defined(_SMU_PPUPATD1_SMUCFGNS_SHIFT)
  SL_HAL_SMU_SMUCFGNS     = 32 + _SMU_PPUPATD1_SMUCFGNS_SHIFT,  /**< SMU peripheral identifier for SMUCFGNS  */
#endif
#if defined(_SMU_PPUPATD1_AHBUSBSYS_SHIFT)
  SL_HAL_SMU_AHBUSBSYS    = 32 + _SMU_PPUPATD1_AHBUSBSYS_SHIFT, /**< SMU peripheral identifier for AHBUSBSYS */
#endif
#if defined(_SMU_PPUPATD1_AHBRADIO_SHIFT)
  SL_HAL_SMU_AHBRADIO     = 32 + _SMU_PPUPATD1_AHBRADIO_SHIFT,  /**< SMU peripheral identifier for AHBRADIO  */
#endif
#if defined(_SMU_PPUPATD1_SEMAILBOX_SHIFT)
  SL_HAL_SMU_SEMAILBOX    = 32 + _SMU_PPUPATD1_SEMAILBOX_SHIFT, /**< SMU peripheral identifier for SEMAILBOX */
#endif
#if defined(_SMU_PPUPATD1_MVP_SHIFT)
  SL_HAL_SMU_MVP          = 32 + _SMU_PPUPATD1_MVP_SHIFT,       /**< SMU peripheral identifier for MVP       */
#endif
#if defined(_SMU_PPUPATD1_CRYPTOACC_SHIFT)
  SL_HAL_SMU_CRYPTOACC    = 32 + _SMU_PPUPATD1_CRYPTOACC_SHIFT, /**< SMU peripheral identifier for CRYPTOACC */
#endif
#else
#error "No peripherals defined for SMU for this device configuration."
#endif
  SL_HAL_SMU_END /**< SMU peripheral end. */
};

/** SMU peripheral privileged access enablers. */
typedef struct {
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)
  bool sl_hal_smu_privileged_reserved0    : 1;  ///< Reserved privileged access enabler.
  bool sl_hal_smu_privileged_emu          : 1;  ///< Privileged access enabler for EMU
  bool sl_hal_smu_privileged_cmu          : 1;  ///< Privileged access enabler for CMU
  bool sl_hal_smu_privileged_hfxo0        : 1;  ///< Privileged access enabler for HFXO0
  bool sl_hal_smu_privileged_hfrco0       : 1;  ///< Privileged access enabler for HFRCO0
  bool sl_hal_smu_privileged_fsrco        : 1;  ///< Privileged access enabler for FSRCO
  bool sl_hal_smu_privileged_dpll0        : 1;  ///< Privileged access enabler for DPLL0
  bool sl_hal_smu_privileged_lfxo         : 1;  ///< Privileged access enabler for LFXO
  bool sl_hal_smu_privileged_lfrco        : 1;  ///< Privileged access enabler for LFRCO
  bool sl_hal_smu_privileged_ulfrco       : 1;  ///< Privileged access enabler for ULFRCO
  bool sl_hal_smu_privileged_msc          : 1;  ///< Privileged access enabler for MSC
  bool sl_hal_smu_privileged_icache0      : 1;  ///< Privileged access enabler for ICACHE0
  bool sl_hal_smu_privileged_prs          : 1;  ///< Privileged access enabler for PRS
  bool sl_hal_smu_privileged_gpio         : 1;  ///< Privileged access enabler for GPIO
  bool sl_hal_smu_privileged_ldma         : 1;  ///< Privileged access enabler for LDMA
  bool sl_hal_smu_privileged_ldmaxbar     : 1;  ///< Privileged access enabler for LDMAXBAR
  bool sl_hal_smu_privileged_timer0       : 1;  ///< Privileged access enabler for TIMER0
  bool sl_hal_smu_privileged_timer1       : 1;  ///< Privileged access enabler for TIMER1
  bool sl_hal_smu_privileged_timer2       : 1;  ///< Privileged access enabler for TIMER2
  bool sl_hal_smu_privileged_timer3       : 1;  ///< Privileged access enabler for TIMER3
  bool sl_hal_smu_privileged_usart0       : 1;  ///< Privileged access enabler for USART0
  bool sl_hal_smu_privileged_usart1       : 1;  ///< Privileged access enabler for USART1
  bool sl_hal_smu_privileged_usart2       : 1;  ///< Privileged access enabler for USART2
  bool sl_hal_smu_privileged_burtc        : 1;  ///< Privileged access enabler for BURTC
  bool sl_hal_smu_privileged_i2c1         : 1;  ///< Privileged access enabler for I2C1
  bool sl_hal_smu_privileged_chiptestctrl : 1;  ///< Privileged access enabler for CHIPTESTCTRL
  bool sl_hal_smu_privileged_lvgd         : 1;  ///< Privileged access enabler for LVGD
  bool sl_hal_smu_privileged_syscfg       : 1;  ///< Privileged access enabler for SYSCFG
  bool sl_hal_smu_privileged_buram        : 1;  ///< Privileged access enabler for BURAM
  bool sl_hal_smu_privileged_ifadcdebug   : 1;  ///< Privileged access enabler for IFADCDEBUG
  bool sl_hal_smu_privileged_gpcrc        : 1;  ///< Privileged access enabler for GPCRC
  bool sl_hal_smu_privileged_rtcc         : 1;  ///< Privileged access enabler for RTCC
  bool sl_hal_smu_privileged_letimer0     : 1;  ///< Privileged access enabler for LETIMER
  bool sl_hal_smu_privileged_iadc0        : 1;  ///< Privileged access enabler for IADC0
  bool sl_hal_smu_privileged_acmp0        : 1;  ///< Privileged access enabler for ACMP0
  bool sl_hal_smu_privileged_acmp1        : 1;  ///< Privileged access enabler for ACMP1
  bool sl_hal_smu_privileged_i2c0         : 1;  ///< Privileged access enabler for I2C0
  bool sl_hal_smu_privileged_hfrcoem23    : 1;  ///< Privileged access enabler for HFRCOEM23
  bool sl_hal_smu_privileged_wdog0        : 1;  ///< Privileged access enabler for WDOG0
  bool sl_hal_smu_privileged_wdog1        : 1;  ///< Privileged access enabler for WDOG1
  bool sl_hal_smu_privileged_amuxcp0      : 1;  ///< Privileged access enabler for AMUXCP0
  bool sl_hal_smu_privileged_radioaes     : 1;  ///< Privileged access enabler for RADIOAES
  bool sl_hal_smu_privileged_bufc         : 1;  ///< Privileged access enabler for BUFC
  bool sl_hal_smu_privileged_smu          : 1;  ///< Privileged access enabler for SMU
  bool sl_hal_smu_privileged_ahbradio     : 1;  ///< Privileged access enabler for AHBRADIO
  bool sl_hal_smu_privileged_semailbox    : 1;  ///< Privileged access enabler for SEMAILBOX

#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2)
  bool sl_hal_smu_privileged_scratchpad   : 1;  ///< Privileged access enabler for SCRATCHPAD
  bool sl_hal_smu_privileged_emu          : 1;  ///< Privileged access enabler for EMU
  bool sl_hal_smu_privileged_cmu          : 1;  ///< Privileged access enabler for CMU
  bool sl_hal_smu_privileged_hfxo0        : 1;  ///< Privileged access enabler for HFXO0
  bool sl_hal_smu_privileged_hfrco0       : 1;  ///< Privileged access enabler for HFRCO0
  bool sl_hal_smu_privileged_fsrco        : 1;  ///< Privileged access enabler for FSRCO
  bool sl_hal_smu_privileged_dpll0        : 1;  ///< Privileged access enabler for DPLL0
  bool sl_hal_smu_privileged_lfxo         : 1;  ///< Privileged access enabler for LFXO
  bool sl_hal_smu_privileged_lfrco        : 1;  ///< Privileged access enabler for LFRCO
  bool sl_hal_smu_privileged_ulfrco       : 1;  ///< Privileged access enabler for ULFRCO
  bool sl_hal_smu_privileged_msc          : 1;  ///< Privileged access enabler for MSC
  bool sl_hal_smu_privileged_icache0      : 1;  ///< Privileged access enabler for ICACHE0
  bool sl_hal_smu_privileged_prs          : 1;  ///< Privileged access enabler for PRS0
  bool sl_hal_smu_privileged_gpio         : 1;  ///< Privileged access enabler for GPIO
  bool sl_hal_smu_privileged_ldma         : 1;  ///< Privileged access enabler for LDMA
  bool sl_hal_smu_privileged_ldmaxbar     : 1;  ///< Privileged access enabler for LDMAXBAR
  bool sl_hal_smu_privileged_timer0       : 1;  ///< Privileged access enabler for TIMER0
  bool sl_hal_smu_privileged_timer1       : 1;  ///< Privileged access enabler for TIMER1
  bool sl_hal_smu_privileged_timer2       : 1;  ///< Privileged access enabler for TIMER2
  bool sl_hal_smu_privileged_timer3       : 1;  ///< Privileged access enabler for TIMER3
  bool sl_hal_smu_privileged_timer4       : 1;  ///< Privileged access enabler for TIMER4
  bool sl_hal_smu_privileged_usart0       : 1;  ///< Privileged access enabler for USART0
  bool sl_hal_smu_privileged_usart1       : 1;  ///< Privileged access enabler for USART1
  bool sl_hal_smu_privileged_burtc        : 1;  ///< Privileged access enabler for BURTC
  bool sl_hal_smu_privileged_i2c1         : 1;  ///< Privileged access enabler for I2C1
  bool sl_hal_smu_privileged_chiptestctrl : 1;  ///< Privileged access enabler for CHIPTESTCTRL
  bool sl_hal_smu_privileged_syscfgcfgns  : 1;  ///< Privileged access enabler for SYSCFGCFGNS
  bool sl_hal_smu_privileged_syscfg       : 1;  ///< Privileged access enabler for SYSCFG
  bool sl_hal_smu_privileged_buram        : 1;  ///< Privileged access enabler for BURAM
  bool sl_hal_smu_privileged_ifadcdebug   : 1;  ///< Privileged access enabler for IFADCDEBUG
  bool sl_hal_smu_privileged_gpcrc        : 1;  ///< Privileged access enabler for GPCRC
  bool sl_hal_smu_privileged_dci          : 1;  ///< Privileged access enabler for DCI

  bool sl_hal_smu_privileged_rootcfg      : 1;  ///< Privileged access enabler for ROOTCFG
  bool sl_hal_smu_privileged_dcdc         : 1;  ///< Privileged access enabler for DCDC
  bool sl_hal_smu_privileged_pdm          : 1;  ///< Privileged access enabler for PDM
  bool sl_hal_smu_privileged_rfsense      : 1;  ///< Privileged access enabler for RFSENSE
  bool sl_hal_smu_privileged_radioaes     : 1;  ///< Privileged access enabler for RADIOAES
  bool sl_hal_smu_privileged_smu          : 1;  ///< Privileged access enabler for SMU
  bool sl_hal_smu_privileged_smucfgns     : 1;  ///< Privileged access enabler for SMUCFGNS
  bool sl_hal_smu_privileged_rtcc         : 1;  ///< Privileged access enabler for RTCC
  bool sl_hal_smu_privileged_letimer0     : 1;  ///< Privileged access enabler for LETIMER0
  bool sl_hal_smu_privileged_iadc0        : 1;  ///< Privileged access enabler for IADC0
  bool sl_hal_smu_privileged_i2c0         : 1;  ///< Privileged access enabler for I2C0
  bool sl_hal_smu_privileged_wdog0        : 1;  ///< Privileged access enabler for WDOG0
  bool sl_hal_smu_privileged_amuxcp0      : 1;  ///< Privileged access enabler for AMUXCP0
  bool sl_hal_smu_privileged_euart0       : 1;  ///< Privileged access enabler for EUART0
  bool sl_hal_smu_privileged_cryptoacc    : 1;  ///< Privileged access enabler for CRYPTOACC
  bool sl_hal_smu_privileged_ahbradio     : 1;  ///< Privileged access enabler for AHBRADIO

#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_3)
  bool sl_hal_smu_privileged_reserved0    : 1;  ///< Reserved privileged access enabler
  bool sl_hal_smu_privileged_emu          : 1;  ///< Privileged access enabler for EMU
  bool sl_hal_smu_privileged_cmu          : 1;  ///< Privileged access enabler for CMU
  bool sl_hal_smu_privileged_hfrco0       : 1;  ///< Privileged access enabler for HFRCO0
  bool sl_hal_smu_privileged_fsrco        : 1;  ///< Privileged access enabler for FSRCO
  bool sl_hal_smu_privileged_dpll0        : 1;  ///< Privileged access enabler for DPLL0
  bool sl_hal_smu_privileged_lfxo         : 1;  ///< Privileged access enabler for LFXO
  bool sl_hal_smu_privileged_lfrco        : 1;  ///< Privileged access enabler for LFRCO
  bool sl_hal_smu_privileged_ulfrco       : 1;  ///< Privileged access enabler for ULFRCO
  bool sl_hal_smu_privileged_msc          : 1;  ///< Privileged access enabler for MSC
  bool sl_hal_smu_privileged_icache0      : 1;  ///< Privileged access enabler for ICACHE0
  bool sl_hal_smu_privileged_prs          : 1;  ///< Privileged access enabler for PRS0
  bool sl_hal_smu_privileged_gpio         : 1;  ///< Privileged access enabler for GPIO
  bool sl_hal_smu_privileged_ldma         : 1;  ///< Privileged access enabler for LDMA
  bool sl_hal_smu_privileged_ldmaxbar     : 1;  ///< Privileged access enabler for LDMAXBAR
  bool sl_hal_smu_privileged_timer0       : 1;  ///< Privileged access enabler for TIMER0
  bool sl_hal_smu_privileged_timer1       : 1;  ///< Privileged access enabler for TIMER1
  bool sl_hal_smu_privileged_timer2       : 1;  ///< Privileged access enabler for TIMER2
  bool sl_hal_smu_privileged_timer3       : 1;  ///< Privileged access enabler for TIMER3
  bool sl_hal_smu_privileged_timer4       : 1;  ///< Privileged access enabler for TIMER4
  bool sl_hal_smu_privileged_usart0       : 1;  ///< Privileged access enabler for USART0
  bool sl_hal_smu_privileged_burtc        : 1;  ///< Privileged access enabler for BURTC
  bool sl_hal_smu_privileged_i2c1         : 1;  ///< Privileged access enabler for I2C1
  bool sl_hal_smu_privileged_chiptestctrl : 1;  ///< Privileged access enabler for CHIPTESTCTRL
  bool sl_hal_smu_privileged_syscfgcfgns  : 1;  ///< Privileged access enabler for SYSCFGCFGNS
  bool sl_hal_smu_privileged_syscfg       : 1;  ///< Privileged access enabler for SYSCFG
  bool sl_hal_smu_privileged_buram        : 1;  ///< Privileged access enabler for BURAM
  bool sl_hal_smu_privileged_gpcrc        : 1;  ///< Privileged access enabler for GPCRC
  bool sl_hal_smu_privileged_dcdc         : 1;  ///< Privileged access enabler for DCDC
  bool sl_hal_smu_privileged_hostmailbox  : 1;  ///< Privileged access enabler for HOSTMAILBOX
  bool sl_hal_smu_privileged_eusart1      : 1;  ///< Privileged access enabler for EUSART1
  bool sl_hal_smu_privileged_eusart2      : 1;  ///< Privileged access enabler for EUSART2

  bool sl_hal_smu_privileged_sysrtc       : 1;  ///< Privileged access enabler for SYSRTC
  bool sl_hal_smu_privileged_lcd          : 1;  ///< Privileged access enabler for LCD
  bool sl_hal_smu_privileged_keyscan      : 1;  ///< Privileged access enabler for KEYSCAN
  bool sl_hal_smu_privileged_dmem         : 1;  ///< Privileged access enabler for DMEM
  bool sl_hal_smu_privileged_lcdrf        : 1;  ///< Privileged access enabler for LCDRF
  bool sl_hal_smu_privileged_pfmxpprf     : 1;  ///< Privileged access enabler for PFMXPPRF
  bool sl_hal_smu_privileged_radioaes     : 1;  ///< Privileged access enabler for RADIOAES
  bool sl_hal_smu_privileged_smu          : 1;  ///< Privileged access enabler for SMU
  bool sl_hal_smu_privileged_smucfgns     : 1;  ///< Privileged access enabler for SMUCFGNS
  bool sl_hal_smu_privileged_letimer0     : 1;  ///< Privileged access enabler for LETIMER0
  bool sl_hal_smu_privileged_iadc0        : 1;  ///< Privileged access enabler for IADC0
  bool sl_hal_smu_privileged_acmp0        : 1;  ///< Privileged access enabler for ACMP0
  bool sl_hal_smu_privileged_acmp1        : 1;  ///< Privileged access enabler for ACMP1
  bool sl_hal_smu_privileged_amuxcp0      : 1;  ///< Privileged access enabler for AMUXCP0
  bool sl_hal_smu_privileged_vdac0        : 1;  ///< Privileged access enabler for VDAC0
  bool sl_hal_smu_privileged_pcnt         : 1;  ///< Privileged access enabler for PCNT
  bool sl_hal_smu_privileged_lesense      : 1;  ///< Privileged access enabler for LESENSE
  bool sl_hal_smu_privileged_hfrco1       : 1;  ///< Privileged access enabler for HFRCO1
  bool sl_hal_smu_privileged_hfxo0        : 1;  ///< Privileged access enabler for HFXO0
  bool sl_hal_smu_privileged_i2c0         : 1;  ///< Privileged access enabler for I2C0
  bool sl_hal_smu_privileged_wdog0        : 1;  ///< Privileged access enabler for WDOG0
  bool sl_hal_smu_privileged_wdog1        : 1;  ///< Privileged access enabler for WDOG1
  bool sl_hal_smu_privileged_eusart0      : 1;  ///< Privileged access enabler for EUSART0
  bool sl_hal_smu_privileged_semailbox    : 1;  ///< Privileged access enabler for SEMAILBOX
  bool sl_hal_smu_privileged_ahbradio     : 1;  ///< Privileged access enabler for AHBRADIO

#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_4)
  bool sl_hal_smu_privileged_reserved0    : 1;  ///< Reserved privileged access enabler
  bool sl_hal_smu_privileged_emu          : 1;  ///< Privileged access enabler for EMU
  bool sl_hal_smu_privileged_cmu          : 1;  ///< Privileged access enabler for CMU
  bool sl_hal_smu_privileged_hfrco0       : 1;  ///< Privileged access enabler for HFRCO0
  bool sl_hal_smu_privileged_fsrco        : 1;  ///< Privileged access enabler for FSRCO
  bool sl_hal_smu_privileged_dpll0        : 1;  ///< Privileged access enabler for DPLL0
  bool sl_hal_smu_privileged_lfxo         : 1;  ///< Privileged access enabler for LFXO
  bool sl_hal_smu_privileged_lfrco        : 1;  ///< Privileged access enabler for LFRCO
  bool sl_hal_smu_privileged_ulfrco       : 1;  ///< Privileged access enabler for ULFRCO
  bool sl_hal_smu_privileged_msc          : 1;  ///< Privileged access enabler for MSC
  bool sl_hal_smu_privileged_icache0      : 1;  ///< Privileged access enabler for ICACHE0
  bool sl_hal_smu_privileged_prs          : 1;  ///< Privileged access enabler for PRS0
  bool sl_hal_smu_privileged_gpio         : 1;  ///< Privileged access enabler for GPIO
  bool sl_hal_smu_privileged_ldma         : 1;  ///< Privileged access enabler for LDMA
  bool sl_hal_smu_privileged_ldmaxbar     : 1;  ///< Privileged access enabler for LDMAXBAR
  bool sl_hal_smu_privileged_timer0       : 1;  ///< Privileged access enabler for TIMER0
  bool sl_hal_smu_privileged_timer1       : 1;  ///< Privileged access enabler for TIMER1
  bool sl_hal_smu_privileged_timer2       : 1;  ///< Privileged access enabler for TIMER2
  bool sl_hal_smu_privileged_timer3       : 1;  ///< Privileged access enabler for TIMER3
  bool sl_hal_smu_privileged_timer4       : 1;  ///< Privileged access enabler for TIMER4
  bool sl_hal_smu_privileged_usart0       : 1;  ///< Privileged access enabler for USART0
  bool sl_hal_smu_privileged_burtc        : 1;  ///< Privileged access enabler for BURTC
  bool sl_hal_smu_privileged_i2c1         : 1;  ///< Privileged access enabler for I2C1
  bool sl_hal_smu_privileged_chiptestctrl : 1;  ///< Privileged access enabler for CHIPTESTCTRL
  bool sl_hal_smu_privileged_syscfgcfgns  : 1;  ///< Privileged access enabler for SYSCFGCFGNS
  bool sl_hal_smu_privileged_syscfg       : 1;  ///< Privileged access enabler for SYSCFG
  bool sl_hal_smu_privileged_buram        : 1;  ///< Privileged access enabler for BURAM
  bool sl_hal_smu_privileged_gpcrc        : 1;  ///< Privileged access enabler for GPCRC
  bool sl_hal_smu_privileged_dcdc         : 1;  ///< Privileged access enabler for DCDC
  bool sl_hal_smu_privileged_hostmailbox  : 1;  ///< Privileged access enabler for HOSTMAILBOX
  bool sl_hal_smu_privileged_eusart1      : 1;  ///< Privileged access enabler for EUSART1
  bool sl_hal_smu_privileged_sysrtc       : 1;  ///< Privileged access enabler for SYSRTC

  bool sl_hal_smu_privileged_keypad       : 1;  ///< Privileged access enabler for KEYPAD
  bool sl_hal_smu_privileged_dmem         : 1;  ///< Privileged access enabler for DMEM
  bool sl_hal_smu_privileged_radioaes     : 1;  ///< Privileged access enabler for RADIOAES
  bool sl_hal_smu_privileged_smu          : 1;  ///< Privileged access enabler for SMU
  bool sl_hal_smu_privileged_smucfgns     : 1;  ///< Privileged access enabler for SMUCFGNS
  bool sl_hal_smu_privileged_letimer0     : 1;  ///< Privileged access enabler for LETIMER0
  bool sl_hal_smu_privileged_iadc0        : 1;  ///< Privileged access enabler for IADC0
  bool sl_hal_smu_privileged_acmp0        : 1;  ///< Privileged access enabler for ACMP0
  bool sl_hal_smu_privileged_acmp1        : 1;  ///< Privileged access enabler for ACMP1
  bool sl_hal_smu_privileged_amuxcp0      : 1;  ///< Privileged access enabler for AMUXCP0
  bool sl_hal_smu_privileged_vdac0        : 1;  ///< Privileged access enabler for VDAC0
  bool sl_hal_smu_privileged_vdac1        : 1;  ///< Privileged access enabler for VDAC1
  bool sl_hal_smu_privileged_pcnt         : 1;  ///< Privileged access enabler for PCNT
  bool sl_hal_smu_privileged_hfrco1       : 1;  ///< Privileged access enabler for HFRCO1
  bool sl_hal_smu_privileged_hfxo0        : 1;  ///< Privileged access enabler for HFXO0
  bool sl_hal_smu_privileged_i2c0         : 1;  ///< Privileged access enabler for I2C0
  bool sl_hal_smu_privileged_wdog0        : 1;  ///< Privileged access enabler for WDOG0
  bool sl_hal_smu_privileged_wdog1        : 1;  ///< Privileged access enabler for WDOG1
  bool sl_hal_smu_privileged_eusart0      : 1;  ///< Privileged access enabler for EUSART0
  bool sl_hal_smu_privileged_semailbox    : 1;  ///< Privileged access enabler for SEMAILBOX
  bool sl_hal_smu_privileged_mvp          : 1;  ///< Privileged access enabler for MVP
  bool sl_hal_smu_privileged_ahbradio     : 1;  ///< Privileged access enabler for AHBRADIO

#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_5)
  bool sl_hal_smu_privileged_reserved0    : 1;  ///< Reserved privileged access enabler
  bool sl_hal_smu_privileged_emu          : 1;  ///< Privileged access enabler for EMU
  bool sl_hal_smu_privileged_cmu          : 1;  ///< Privileged access enabler for CMU
  bool sl_hal_smu_privileged_hfrco0       : 1;  ///< Privileged access enabler for HFRCO0
  bool sl_hal_smu_privileged_fsrco        : 1;  ///< Privileged access enabler for FSRCO
  bool sl_hal_smu_privileged_dpll0        : 1;  ///< Privileged access enabler for DPLL0
  bool sl_hal_smu_privileged_lfxo         : 1;  ///< Privileged access enabler for LFXO
  bool sl_hal_smu_privileged_lfrco        : 1;  ///< Privileged access enabler for LFRCO
  bool sl_hal_smu_privileged_ulfrco       : 1;  ///< Privileged access enabler for ULFRCO
  bool sl_hal_smu_privileged_msc          : 1;  ///< Privileged access enabler for MSC
  bool sl_hal_smu_privileged_icache0      : 1;  ///< Privileged access enabler for ICACHE0
  bool sl_hal_smu_privileged_prs          : 1;  ///< Privileged access enabler for PRS0
  bool sl_hal_smu_privileged_gpio         : 1;  ///< Privileged access enabler for GPIO
  bool sl_hal_smu_privileged_ldma         : 1;  ///< Privileged access enabler for LDMA
  bool sl_hal_smu_privileged_ldmaxbar     : 1;  ///< Privileged access enabler for LDMAXBAR
  bool sl_hal_smu_privileged_timer0       : 1;  ///< Privileged access enabler for TIMER0
  bool sl_hal_smu_privileged_timer1       : 1;  ///< Privileged access enabler for TIMER1
  bool sl_hal_smu_privileged_timer2       : 1;  ///< Privileged access enabler for TIMER2
  bool sl_hal_smu_privileged_timer3       : 1;  ///< Privileged access enabler for TIMER3
  bool sl_hal_smu_privileged_timer4       : 1;  ///< Privileged access enabler for TIMER4
  bool sl_hal_smu_privileged_timer5       : 1;  ///< Privileged access enabler for TIMER5
  bool sl_hal_smu_privileged_timer6       : 1;  ///< Privileged access enabler for TIMER6
  bool sl_hal_smu_privileged_timer7       : 1;  ///< Privileged access enabler for TIMER7
  bool sl_hal_smu_privileged_burtc        : 1;  ///< Privileged access enabler for BURTC
  bool sl_hal_smu_privileged_i2c1         : 1;  ///< Privileged access enabler for I2C1
  bool sl_hal_smu_privileged_chiptestctrl : 1;  ///< Privileged access enabler for CHIPTESTCTRL
  bool sl_hal_smu_privileged_syscfgcfgns  : 1;  ///< Privileged access enabler for SYSCFGCFGNS
  bool sl_hal_smu_privileged_syscfg       : 1;  ///< Privileged access enabler for SYSCFG
  bool sl_hal_smu_privileged_buram        : 1;  ///< Privileged access enabler for BURAM
  bool sl_hal_smu_privileged_gpcrc        : 1;  ///< Privileged access enabler for GPCRC
  bool sl_hal_smu_privileged_dcdc         : 1;  ///< Privileged access enabler for DCDC
  bool sl_hal_smu_privileged_hostmailbox  : 1;  ///< Privileged access enabler for HOSTMAILBOX

  bool sl_hal_smu_privileged_eusart1      : 1;  ///< Privileged access enabler for EUSART1
  bool sl_hal_smu_privileged_eusart2      : 1;  ///< Privileged access enabler for EUSART2
  bool sl_hal_smu_privileged_eusart3      : 1;  ///< Privileged access enabler for EUSART3
  bool sl_hal_smu_privileged_eusart4      : 1;  ///< Privileged access enabler for EUSART4
  bool sl_hal_smu_privileged_sysrtc       : 1;  ///< Privileged access enabler for SYSRTC
  bool sl_hal_smu_privileged_dmem         : 1;  ///< Privileged access enabler for DMEM
  bool sl_hal_smu_privileged_pfmxpprf     : 1;  ///< Privileged access enabler for PFMXPPRF
  bool sl_hal_smu_privileged_rffpll0      : 1;  ///< Privileged access enabler for RFFPLL0
  bool sl_hal_smu_privileged_etampdet     : 1;  ///< Privileged access enabler for ETAMPDET
  bool sl_hal_smu_privileged_bufc         : 1;  ///< Privileged access enabler for BUFC
  bool sl_hal_smu_privileged_radioaes     : 1;  ///< Privileged access enabler for RADIOAES
  bool sl_hal_smu_privileged_smu          : 1;  ///< Privileged access enabler for SMU
  bool sl_hal_smu_privileged_smucfgns     : 1;  ///< Privileged access enabler for SMUCFGNS
  bool sl_hal_smu_privileged_ahbusbsys    : 1;  ///< Privileged access enabler for AHBUSBSYS
  bool sl_hal_smu_privileged_letimer0     : 1;  ///< Privileged access enabler for LETIMER0
  bool sl_hal_smu_privileged_iadc0        : 1;  ///< Privileged access enabler for IADC0
  bool sl_hal_smu_privileged_acmp0        : 1;  ///< Privileged access enabler for ACMP0
  bool sl_hal_smu_privileged_acmp1        : 1;  ///< Privileged access enabler for ACMP1
  bool sl_hal_smu_privileged_amuxcp0      : 1;  ///< Privileged access enabler for AMUXCP0
  bool sl_hal_smu_privileged_vdac0        : 1;  ///< Privileged access enabler for VDAC0
  bool sl_hal_smu_privileged_pcnt         : 1;  ///< Privileged access enabler for PCNT
  bool sl_hal_smu_privileged_lesense      : 1;  ///< Privileged access enabler for LESENSE
  bool sl_hal_smu_privileged_hfrco1       : 1;  ///< Privileged access enabler for HFRCO1
  bool sl_hal_smu_privileged_hfxo0        : 1;  ///< Privileged access enabler for HFXO0
  bool sl_hal_smu_privileged_i2c0         : 1;  ///< Privileged access enabler for I2C0
  bool sl_hal_smu_privileged_wdog0        : 1;  ///< Privileged access enabler for WDOG0
  bool sl_hal_smu_privileged_wdog1        : 1;  ///< Privileged access enabler for WDOG1
  bool sl_hal_smu_privileged_eusart0      : 1;  ///< Privileged access enabler for EUSART0
  bool sl_hal_smu_privileged_semailbox    : 1;  ///< Privileged access enabler for SEMAILBOX
  bool sl_hal_smu_privileged_ahbradio     : 1;  ///< Privileged access enabler for AHBRADIO

#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_6)
  bool sl_hal_smu_privileged_reserved0    : 1;  ///< Reserved privileged access enabler
  bool sl_hal_smu_privileged_emu          : 1;  ///< Privileged access enabler for EMU
  bool sl_hal_smu_privileged_cmu          : 1;  ///< Privileged access enabler for CMU
  bool sl_hal_smu_privileged_burtc        : 1;  ///< Privileged access enabler for BURTC
  bool sl_hal_smu_privileged_hfrco0       : 1;  ///< Privileged access enabler for HFRCO0
  bool sl_hal_smu_privileged_fsrco        : 1;  ///< Privileged access enabler for FSRCO
  bool sl_hal_smu_privileged_dpll0        : 1;  ///< Privileged access enabler for DPLL0
  bool sl_hal_smu_privileged_lfxo         : 1;  ///< Privileged access enabler for LFXO
  bool sl_hal_smu_privileged_lfrco        : 1;  ///< Privileged access enabler for LFRCO
  bool sl_hal_smu_privileged_ulfrco       : 1;  ///< Privileged access enabler for ULFRCO
  bool sl_hal_smu_privileged_keyscan      : 1;  ///< Privileged access enabler for KEYSCAN
  bool sl_hal_smu_privileged_msc          : 1;  ///< Privileged access enabler for MSC
  bool sl_hal_smu_privileged_icache0      : 1;  ///< Privileged access enabler for ICACHE0
  bool sl_hal_smu_privileged_prs          : 1;  ///< Privileged access enabler for PRS0
  bool sl_hal_smu_privileged_gpio         : 1;  ///< Privileged access enabler for GPIO
  bool sl_hal_smu_privileged_ldma         : 1;  ///< Privileged access enabler for LDMA
  bool sl_hal_smu_privileged_ldmaxbar     : 1;  ///< Privileged access enabler for LDMAXBAR
  bool sl_hal_smu_privileged_timer0       : 1;  ///< Privileged access enabler for TIMER0
  bool sl_hal_smu_privileged_timer1       : 1;  ///< Privileged access enabler for TIMER1
  bool sl_hal_smu_privileged_timer2       : 1;  ///< Privileged access enabler for TIMER2
  bool sl_hal_smu_privileged_timer3       : 1;  ///< Privileged access enabler for TIMER3
  bool sl_hal_smu_privileged_timer4       : 1;  ///< Privileged access enabler for TIMER4
  bool sl_hal_smu_privileged_timer5       : 1;  ///< Privileged access enabler for TIMER5
  bool sl_hal_smu_privileged_timer6       : 1;  ///< Privileged access enabler for TIMER6
  bool sl_hal_smu_privileged_timer7       : 1;  ///< Privileged access enabler for TIMER7
  bool sl_hal_smu_privileged_timer8       : 1;  ///< Privileged access enabler for TIMER8
  bool sl_hal_smu_privileged_timer9       : 1;  ///< Privileged access enabler for TIMER9
  bool sl_hal_smu_privileged_chiptestctrl : 1;  ///< Privileged access enabler for CHIPTESTCTRL
  bool sl_hal_smu_privileged_dmem0        : 1;  ///< Privileged access enabler for DMEM0
  bool sl_hal_smu_privileged_dmem1        : 1;  ///< Privileged access enabler for DMEM1
  bool sl_hal_smu_privileged_syscfgcfgns  : 1;  ///< Privileged access enabler for SYSCFGCFGNS
  bool sl_hal_smu_privileged_syscfg       : 1;  ///< Privileged access enabler for SYSCFG

  bool sl_hal_smu_privileged_buram        : 1;  ///< Privileged access enabler for BURAM
  bool sl_hal_smu_privileged_gpcrc        : 1;  ///< Privileged access enabler for GPCRC
  bool sl_hal_smu_privileged_eusart1      : 1;  ///< Privileged access enabler for EUSART1
  bool sl_hal_smu_privileged_eusart2      : 1;  ///< Privileged access enabler for EUSART2
  bool sl_hal_smu_privileged_eusart3      : 1;  ///< Privileged access enabler for EUSART3
  bool sl_hal_smu_privileged_dcdc         : 1;  ///< Privileged access enabler for DCDC
  bool sl_hal_smu_privileged_hostmailbox  : 1;  ///< Privileged access enabler for HOSTMAILBOX
  bool sl_hal_smu_privileged_usart0       : 1;  ///< Privileged access enabler for USART0
  bool sl_hal_smu_privileged_usart1       : 1;  ///< Privileged access enabler for USART1
  bool sl_hal_smu_privileged_usart2       : 1;  ///< Privileged access enabler for USART2
  bool sl_hal_smu_privileged_sysrtc0      : 1;  ///< Privileged access enabler for SYSRTC0
  bool sl_hal_smu_privileged_i2c1         : 1;  ///< Privileged access enabler for I2C1
  bool sl_hal_smu_privileged_i2c2         : 1;  ///< Privileged access enabler for I2C2
  bool sl_hal_smu_privileged_i2c3         : 1;  ///< Privileged access enabler for I2C3
  bool sl_hal_smu_privileged_lcd          : 1;  ///< Privileged access enabler for LCD
  bool sl_hal_smu_privileged_lcdrf        : 1;  ///< Privileged access enabler for LCDRF
  bool sl_hal_smu_privileged_radioaes     : 1;  ///< Privileged access enabler for RADIOAES
  bool sl_hal_smu_privileged_smu          : 1;  ///< Privileged access enabler for SMU
  bool sl_hal_smu_privileged_smucfgns     : 1;  ///< Privileged access enabler for SMUCFGNS
  bool sl_hal_smu_privileged_letimer0     : 1;  ///< Privileged access enabler for LETIMER0
  bool sl_hal_smu_privileged_iadc0        : 1;  ///< Privileged access enabler for IADC0
  bool sl_hal_smu_privileged_acmp0        : 1;  ///< Privileged access enabler for ACMP0
  bool sl_hal_smu_privileged_acmp1        : 1;  ///< Privileged access enabler for ACMP1
  bool sl_hal_smu_privileged_amuxcp0      : 1;  ///< Privileged access enabler for AMUXCP0
  bool sl_hal_smu_privileged_vdac0        : 1;  ///< Privileged access enabler for VDAC0
  bool sl_hal_smu_privileged_vdac1        : 1;  ///< Privileged access enabler for VDAC1
  bool sl_hal_smu_privileged_pcnt0        : 1;  ///< Privileged access enabler for PCNT0
  bool sl_hal_smu_privileged_hfrco1       : 1;  ///< Privileged access enabler for HFRCO1
  bool sl_hal_smu_privileged_hfxo0        : 1;  ///< Privileged access enabler for HFXO0
  bool sl_hal_smu_privileged_i2c0         : 1;  ///< Privileged access enabler for I2C0
  bool sl_hal_smu_privileged_wdog0        : 1;  ///< Privileged access enabler for WDOG0
  bool sl_hal_smu_privileged_wdog1        : 1;  ///< Privileged access enabler for WDOG1

  bool sl_hal_smu_privileged_eusart0      : 1;  ///< Privileged access enabler for EUSART0
  bool sl_hal_smu_privileged_semailbox    : 1;  ///< Privileged access enabler for SEMAILBOX
  bool sl_hal_smu_privileged_mvp          : 1;  ///< Privileged access enabler for MVP
  bool sl_hal_smu_privileged_ahbradio     : 1;  ///< Privileged access enabler for AHBRADIO

#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_7)
  bool sl_hal_smu_privileged_scratchpad   : 1;  ///< Privileged access enabler for SCRATCHPAD
  bool sl_hal_smu_privileged_emu          : 1;  ///< Privileged access enabler for EMU
  bool sl_hal_smu_privileged_cmu          : 1;  ///< Privileged access enabler for CMU
  bool sl_hal_smu_privileged_hfxo0        : 1;  ///< Privileged access enabler for HFXO0
  bool sl_hal_smu_privileged_hfrco0       : 1;  ///< Privileged access enabler for HFRCO0
  bool sl_hal_smu_privileged_fsrco        : 1;  ///< Privileged access enabler for FSRCO
  bool sl_hal_smu_privileged_dpll0        : 1;  ///< Privileged access enabler for DPLL0
  bool sl_hal_smu_privileged_lfxo         : 1;  ///< Privileged access enabler for LFXO
  bool sl_hal_smu_privileged_lfrco        : 1;  ///< Privileged access enabler for LFRCO
  bool sl_hal_smu_privileged_ulfrco       : 1;  ///< Privileged access enabler for ULFRCO
  bool sl_hal_smu_privileged_msc          : 1;  ///< Privileged access enabler for MSC
  bool sl_hal_smu_privileged_icache0      : 1;  ///< Privileged access enabler for ICACHE0
  bool sl_hal_smu_privileged_prs          : 1;  ///< Privileged access enabler for PRS0
  bool sl_hal_smu_privileged_gpio         : 1;  ///< Privileged access enabler for GPIO
  bool sl_hal_smu_privileged_ldma         : 1;  ///< Privileged access enabler for LDMA
  bool sl_hal_smu_privileged_ldmaxbar     : 1;  ///< Privileged access enabler for LDMAXBAR
  bool sl_hal_smu_privileged_timer0       : 1;  ///< Privileged access enabler for TIMER0
  bool sl_hal_smu_privileged_timer1       : 1;  ///< Privileged access enabler for TIMER1
  bool sl_hal_smu_privileged_timer2       : 1;  ///< Privileged access enabler for TIMER2
  bool sl_hal_smu_privileged_timer3       : 1;  ///< Privileged access enabler for TIMER3
  bool sl_hal_smu_privileged_timer4       : 1;  ///< Privileged access enabler for TIMER4
  bool sl_hal_smu_privileged_usart0       : 1;  ///< Privileged access enabler for USART0
  bool sl_hal_smu_privileged_usart1       : 1;  ///< Privileged access enabler for USART1
  bool sl_hal_smu_privileged_burtc        : 1;  ///< Privileged access enabler for BURTC
  bool sl_hal_smu_privileged_i2c1         : 1;  ///< Privileged access enabler for I2C1
  bool sl_hal_smu_privileged_chiptestctrl : 1;  ///< Privileged access enabler for CHIPTESTCTRL
  bool sl_hal_smu_privileged_syscfgcfgns  : 1;  ///< Privileged access enabler for SYSCFGCFGNS
  bool sl_hal_smu_privileged_syscfg       : 1;  ///< Privileged access enabler for SYSCFG
  bool sl_hal_smu_privileged_buram        : 1;  ///< Privileged access enabler for BURAM
  bool sl_hal_smu_privileged_ifadcdebug   : 1;  ///< Privileged access enabler for IFADCDEBUG
  bool sl_hal_smu_privileged_gpcrc        : 1;  ///< Privileged access enabler for GPCRC
  bool sl_hal_smu_privileged_dci          : 1;  ///< Privileged access enabler for DCI

  bool sl_hal_smu_privileged_reserved0    : 1;  ///< Reserved privileged access enabler
  bool sl_hal_smu_privileged_dcdc         : 1;  ///< Privileged access enabler for DCDC
  bool sl_hal_smu_privileged_pdm          : 1;  ///< Privileged access enabler for PDM
  bool sl_hal_smu_privileged_rfsense      : 1;  ///< Privileged access enabler for RFSENSE
  bool sl_hal_smu_privileged_sepuf        : 1;  ///< Privileged access enabler for SEPUF
  bool sl_hal_smu_privileged_etampdet     : 1;  ///< Privileged access enabler for ETAMPDET
  bool sl_hal_smu_privileged_radioaes     : 1;  ///< Privileged access enabler for RADIOAES
  bool sl_hal_smu_privileged_smu          : 1;  ///< Privileged access enabler for SMU
  bool sl_hal_smu_privileged_smucfgns     : 1;  ///< Privileged access enabler for SMUCFGNS
  bool sl_hal_smu_privileged_rtcc         : 1;  ///< Privileged access enabler for RTCC
  bool sl_hal_smu_privileged_letimer0     : 1;  ///< Privileged access enabler for LETIMER0
  bool sl_hal_smu_privileged_iadc0        : 1;  ///< Privileged access enabler for IADC0
  bool sl_hal_smu_privileged_acmp0        : 1;  ///< Privileged access enabler for ACMP0
  bool sl_hal_smu_privileged_i2c0         : 1;  ///< Privileged access enabler for I2C0
  bool sl_hal_smu_privileged_wdog0        : 1;  ///< Privileged access enabler for WDOG0
  bool sl_hal_smu_privileged_amuxcp0      : 1;  ///< Privileged access enabler for AMUXCP0
  bool sl_hal_smu_privileged_eusart0      : 1;  ///< Privileged access enabler for EUSART0
  bool sl_hal_smu_privileged_cryptoacc    : 1;  ///< Privileged access enabler for CRYPTOACC
  bool sl_hal_smu_privileged_ahbradio     : 1;  ///< Privileged access enabler for AHBRADIO

#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_8)
  bool sl_hal_smu_privileged_reserved0    : 1;  ///< Reserved privileged access enabler
  bool sl_hal_smu_privileged_emu          : 1;  ///< Privileged access enabler for EMU
  bool sl_hal_smu_privileged_cmu          : 1;  ///< Privileged access enabler for CMU
  bool sl_hal_smu_privileged_hfrco0       : 1;  ///< Privileged access enabler for HFRCO0
  bool sl_hal_smu_privileged_fsrco        : 1;  ///< Privileged access enabler for FSRCO
  bool sl_hal_smu_privileged_dpll0        : 1;  ///< Privileged access enabler for DPLL0
  bool sl_hal_smu_privileged_lfxo         : 1;  ///< Privileged access enabler for LFXO
  bool sl_hal_smu_privileged_lfrco        : 1;  ///< Privileged access enabler for LFRCO
  bool sl_hal_smu_privileged_ulfrco       : 1;  ///< Privileged access enabler for ULFRCO
  bool sl_hal_smu_privileged_msc          : 1;  ///< Privileged access enabler for MSC
  bool sl_hal_smu_privileged_icache0      : 1;  ///< Privileged access enabler for ICACHE0
  bool sl_hal_smu_privileged_prs          : 1;  ///< Privileged access enabler for PRS0
  bool sl_hal_smu_privileged_gpio         : 1;  ///< Privileged access enabler for GPIO
  bool sl_hal_smu_privileged_ldma         : 1;  ///< Privileged access enabler for LDMA
  bool sl_hal_smu_privileged_ldmaxbar     : 1;  ///< Privileged access enabler for LDMAXBAR
  bool sl_hal_smu_privileged_timer0       : 1;  ///< Privileged access enabler for TIMER0
  bool sl_hal_smu_privileged_timer1       : 1;  ///< Privileged access enabler for TIMER1
  bool sl_hal_smu_privileged_timer2       : 1;  ///< Privileged access enabler for TIMER2
  bool sl_hal_smu_privileged_timer3       : 1;  ///< Privileged access enabler for TIMER3
  bool sl_hal_smu_privileged_timer4       : 1;  ///< Privileged access enabler for TIMER4
  bool sl_hal_smu_privileged_usart0       : 1;  ///< Privileged access enabler for USART0
  bool sl_hal_smu_privileged_burtc        : 1;  ///< Privileged access enabler for BURTC
  bool sl_hal_smu_privileged_i2c1         : 1;  ///< Privileged access enabler for I2C1
  bool sl_hal_smu_privileged_chiptestctrl : 1;  ///< Privileged access enabler for CHIPTESTCTRL
  bool sl_hal_smu_privileged_syscfgcfgns  : 1;  ///< Privileged access enabler for SYSCFGCFGNS
  bool sl_hal_smu_privileged_syscfg       : 1;  ///< Privileged access enabler for SYSCFG
  bool sl_hal_smu_privileged_buram        : 1;  ///< Privileged access enabler for BURAM
  bool sl_hal_smu_privileged_gpcrc        : 1;  ///< Privileged access enabler for GPCRC
  bool sl_hal_smu_privileged_dcdc         : 1;  ///< Privileged access enabler for DCDC
  bool sl_hal_smu_privileged_hostmailbox  : 1;  ///< Privileged access enabler for HOSTMAILBOX
  bool sl_hal_smu_privileged_eusart1      : 1;  ///< Privileged access enabler for EUSART1
  bool sl_hal_smu_privileged_eusart2      : 1;  ///< Privileged access enabler for EUSART2

  bool sl_hal_smu_privileged_sysrtc       : 1;  ///< Privileged access enabler for SYSRTC
  bool sl_hal_smu_privileged_lcd          : 1;  ///< Privileged access enabler for LCD
  bool sl_hal_smu_privileged_keyscan      : 1;  ///< Privileged access enabler for KEYSCAN
  bool sl_hal_smu_privileged_dmem         : 1;  ///< Privileged access enabler for DMEM
  bool sl_hal_smu_privileged_lcdrf        : 1;  ///< Privileged access enabler for LCDRF
  bool sl_hal_smu_privileged_pfmxpprf     : 1;  ///< Privileged access enabler for PFMXPPRF
  bool sl_hal_smu_privileged_radioaes     : 1;  ///< Privileged access enabler for RADIOAES
  bool sl_hal_smu_privileged_smu          : 1;  ///< Privileged access enabler for SMU
  bool sl_hal_smu_privileged_smucfgns     : 1;  ///< Privileged access enabler for SMUCFGNS
  bool sl_hal_smu_privileged_letimer0     : 1;  ///< Privileged access enabler for LETIMER0
  bool sl_hal_smu_privileged_iadc0        : 1;  ///< Privileged access enabler for IADC0
  bool sl_hal_smu_privileged_acmp0        : 1;  ///< Privileged access enabler for ACMP0
  bool sl_hal_smu_privileged_acmp1        : 1;  ///< Privileged access enabler for ACMP1
  bool sl_hal_smu_privileged_amuxcp0      : 1;  ///< Privileged access enabler for AMUXCP0
  bool sl_hal_smu_privileged_vdac0        : 1;  ///< Privileged access enabler for VDAC0
  bool sl_hal_smu_privileged_pcnt         : 1;  ///< Privileged access enabler for PCNT
  bool sl_hal_smu_privileged_lesense      : 1;  ///< Privileged access enabler for LESENSE
  bool sl_hal_smu_privileged_hfrco1       : 1;  ///< Privileged access enabler for HFRCO1
  bool sl_hal_smu_privileged_hfxo0        : 1;  ///< Privileged access enabler for HFXO0
  bool sl_hal_smu_privileged_i2c0         : 1;  ///< Privileged access enabler for I2C0
  bool sl_hal_smu_privileged_wdog0        : 1;  ///< Privileged access enabler for WDOG0
  bool sl_hal_smu_privileged_wdog1        : 1;  ///< Privileged access enabler for WDOG1
  bool sl_hal_smu_privileged_eusart0      : 1;  ///< Privileged access enabler for EUSART0
  bool sl_hal_smu_privileged_semailbox    : 1;  ///< Privileged access enabler for SEMAILBOX
  bool sl_hal_smu_privileged_mvp          : 1;  ///< Privileged access enabler for MVP
  bool sl_hal_smu_privileged_ahbradio     : 1;  ///< Privileged access enabler for AHBRADIO
#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_9)
  bool sl_hal_smu_privileged_reserved0    : 1;  ///< Reserved privileged access enabler
  bool sl_hal_smu_privileged_emu          : 1;  ///< Privileged access enabler for EMU
  bool sl_hal_smu_privileged_cmu          : 1;  ///< Privileged access enabler for CMU
  bool sl_hal_smu_privileged_hfxo0        : 1;  ///< Privileged access enabler for HFXO0
  bool sl_hal_smu_privileged_hfrco0       : 1;  ///< Privileged access enabler for HFRCO0
  bool sl_hal_smu_privileged_fsrco        : 1;  ///< Privileged access enabler for FSRCO
  bool sl_hal_smu_privileged_dpll0        : 1;  ///< Privileged access enabler for DPLL0
  bool sl_hal_smu_privileged_lfxo         : 1;  ///< Privileged access enabler for LFXO
  bool sl_hal_smu_privileged_lfrco        : 1;  ///< Privileged access enabler for LFRCO
  bool sl_hal_smu_privileged_ulfrco       : 1;  ///< Privileged access enabler for ULFRCO
  bool sl_hal_smu_privileged_msc          : 1;  ///< Privileged access enabler for MSC
  bool sl_hal_smu_privileged_icache0      : 1;  ///< Privileged access enabler for ICACHE0
  bool sl_hal_smu_privileged_prs          : 1;  ///< Privileged access enabler for PRS0
  bool sl_hal_smu_privileged_gpio         : 1;  ///< Privileged access enabler for GPIO
  bool sl_hal_smu_privileged_ldma         : 1;  ///< Privileged access enabler for LDMA
  bool sl_hal_smu_privileged_ldmaxbar     : 1;  ///< Privileged access enabler for LDMAXBAR
  bool sl_hal_smu_privileged_timer0       : 1;  ///< Privileged access enabler for TIMER0
  bool sl_hal_smu_privileged_timer1       : 1;  ///< Privileged access enabler for TIMER1
  bool sl_hal_smu_privileged_timer2       : 1;  ///< Privileged access enabler for TIMER2
  bool sl_hal_smu_privileged_timer3       : 1;  ///< Privileged access enabler for TIMER3
  bool sl_hal_smu_privileged_timer4       : 1;  ///< Privileged access enabler for TIMER4
  bool sl_hal_smu_privileged_usart0       : 1;  ///< Privileged access enabler for USART0
  bool sl_hal_smu_privileged_usart1       : 1;  ///< Privileged access enabler for USART1
  bool sl_hal_smu_privileged_burtc        : 1;  ///< Privileged access enabler for BURTC
  bool sl_hal_smu_privileged_i2c1         : 1;  ///< Privileged access enabler for I2C1
  bool sl_hal_smu_privileged_chiptestctrl : 1;  ///< Privileged access enabler for CHIPTESTCTRL
  bool sl_hal_smu_privileged_syscfgcfgns  : 1;  ///< Privileged access enabler for SYSCFGCFGNS
  bool sl_hal_smu_privileged_syscfg       : 1;  ///< Privileged access enabler for SYSCFG
  bool sl_hal_smu_privileged_buram        : 1;  ///< Privileged access enabler for BURAM
  bool sl_hal_smu_privileged_ifadcdebug   : 1;  ///< Privileged access enabler for IFADCDEBUG
  bool sl_hal_smu_privileged_gpcrc        : 1;  ///< Privileged access enabler for GPCRC
  bool sl_hal_smu_privileged_dcdc         : 1;  ///< Privileged access enabler for DCDC
  bool sl_hal_smu_privileged_pdm          : 1;  ///< Privileged access enabler for PDM
  bool sl_hal_smu_privileged_rfsense      : 1;  ///< Privileged access enabler for RFSENSE
  bool sl_hal_smu_privileged_etampdet     : 1;  ///< Privileged access enabler for ETAMPDET
  bool sl_hal_smu_privileged_dmem         : 1;  ///< Privileged access enabler for DMEM
  bool sl_hal_smu_privileged_eusart1      : 1;  ///< Privileged access enabler for EUSART1
  bool sl_hal_smu_privileged_radioaes     : 1;  ///< Privileged access enabler for RADIOAES
  bool sl_hal_smu_privileged_smu          : 1;  ///< Privileged access enabler for SMU
  bool sl_hal_smu_privileged_smucfgns     : 1;  ///< Privileged access enabler for SMUCFGNS
  bool sl_hal_smu_privileged_rtcc         : 1;  ///< Privileged access enabler for RTCC
  bool sl_hal_smu_privileged_wdog0        : 1;  ///< Privileged access enabler for WDOG0
  bool sl_hal_smu_privileged_letimer0     : 1;  ///< Privileged access enabler for LETIMER0
  bool sl_hal_smu_privileged_iadc0        : 1;  ///< Privileged access enabler for IADC0
  bool sl_hal_smu_privileged_acmp0        : 1;  ///< Privileged access enabler for ACMP0
  bool sl_hal_smu_privileged_i2c0         : 1;  ///< Privileged access enabler for I2C0
  bool sl_hal_smu_privileged_amuxcp0      : 1;  ///< Privileged access enabler for AMUXCP0
  bool sl_hal_smu_privileged_eusart0      : 1;  ///< Privileged access enabler for EUSART0
  bool sl_hal_smu_privileged_semailbox    : 1;  ///< Privileged access enabler for SEMAILBOX
  bool sl_hal_smu_privileged_ahbradio     : 1;  ///< Privileged access enabler for AHBRADIO
#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_11)
  bool sl_hal_smu_privileged_reserved0    : 1;  ///< Reserved privileged access enabler
  bool sl_hal_smu_privileged_emu          : 1;  ///< Privileged access enabler for EMU
  bool sl_hal_smu_privileged_cmu          : 1;  ///< Privileged access enabler for CMU
  bool sl_hal_smu_privileged_hfxo0        : 1;  ///< Privileged access enabler for HFXO0
  bool sl_hal_smu_privileged_hfrco0       : 1;  ///< Privileged access enabler for HFRCO0
  bool sl_hal_smu_privileged_fsrco        : 1;  ///< Privileged access enabler for FSRCO
  bool sl_hal_smu_privileged_dpll0        : 1;  ///< Privileged access enabler for DPLL0
  bool sl_hal_smu_privileged_lfxo         : 1;  ///< Privileged access enabler for LFXO
  bool sl_hal_smu_privileged_lfrco        : 1;  ///< Privileged access enabler for LFRCO
  bool sl_hal_smu_privileged_ulfrco       : 1;  ///< Privileged access enabler for ULFRCO
  bool sl_hal_smu_privileged_msc          : 1;  ///< Privileged access enabler for MSC
  bool sl_hal_smu_privileged_icache0      : 1;  ///< Privileged access enabler for ICACHE0
  bool sl_hal_smu_privileged_prs          : 1;  ///< Privileged access enabler for PRS0
  bool sl_hal_smu_privileged_gpio         : 1;  ///< Privileged access enabler for GPIO
  bool sl_hal_smu_privileged_ldma         : 1;  ///< Privileged access enabler for LDMA
  bool sl_hal_smu_privileged_ldmaxbar     : 1;  ///< Privileged access enabler for LDMAXBAR
  bool sl_hal_smu_privileged_timer0       : 1;  ///< Privileged access enabler for TIMER0
  bool sl_hal_smu_privileged_timer1       : 1;  ///< Privileged access enabler for TIMER1
  bool sl_hal_smu_privileged_timer2       : 1;  ///< Privileged access enabler for TIMER2
  bool sl_hal_smu_privileged_timer3       : 1;  ///< Privileged access enabler for TIMER3
  bool sl_hal_smu_privileged_timer4       : 1;  ///< Privileged access enabler for TIMER4
  bool sl_hal_smu_privileged_usart0       : 1;  ///< Privileged access enabler for USART0
  bool sl_hal_smu_privileged_usart1       : 1;  ///< Privileged access enabler for USART1
  bool sl_hal_smu_privileged_burtc        : 1;  ///< Privileged access enabler for BURTC
  bool sl_hal_smu_privileged_i2c1         : 1;  ///< Privileged access enabler for I2C1
  bool sl_hal_smu_privileged_chiptestctrl : 1;  ///< Privileged access enabler for CHIPTESTCTRL
  bool sl_hal_smu_privileged_syscfgcfgns  : 1;  ///< Privileged access enabler for SYSCFGCFGNS
  bool sl_hal_smu_privileged_syscfg       : 1;  ///< Privileged access enabler for SYSCFG
  bool sl_hal_smu_privileged_buram        : 1;  ///< Privileged access enabler for BURAM
  bool sl_hal_smu_privileged_ifadcdebug   : 1;  ///< Privileged access enabler for IFADCDEBUG
  bool sl_hal_smu_privileged_gpcrc        : 1;  ///< Privileged access enabler for GPCRC
  bool sl_hal_smu_privileged_dcdc         : 1;  ///< Privileged access enabler for DCDC
  bool sl_hal_smu_privileged_pdm          : 1;  ///< Privileged access enabler for PDM
  bool sl_hal_smu_privileged_rfsense      : 1;  ///< Privileged access enabler for RFSENSE
  bool sl_hal_smu_privileged_etampdet     : 1;  ///< Privileged access enabler for ETAMPDET
  bool sl_hal_smu_privileged_dmem         : 1;  ///< Privileged access enabler for DMEM
  bool sl_hal_smu_privileged_eusart1      : 1;  ///< Privileged access enabler for EUSART1
  bool sl_hal_smu_privileged_radioaes     : 1;  ///< Privileged access enabler for RADIOAES
  bool sl_hal_smu_privileged_smu          : 1;  ///< Privileged access enabler for SMU
  bool sl_hal_smu_privileged_smucfgns     : 1;  ///< Privileged access enabler for SMUCFGNS
  bool sl_hal_smu_privileged_rtcc         : 1;  ///< Privileged access enabler for RTCC
  bool sl_hal_smu_privileged_wdog0        : 1;  ///< Privileged access enabler for WDOG0
  bool sl_hal_smu_privileged_letimer0     : 1;  ///< Privileged access enabler for LETIMER0
  bool sl_hal_smu_privileged_iadc0        : 1;  ///< Privileged access enabler for IADC0
  bool sl_hal_smu_privileged_acmp0        : 1;  ///< Privileged access enabler for ACMP0
  bool sl_hal_smu_privileged_i2c0         : 1;  ///< Privileged access enabler for I2C0
  bool sl_hal_smu_privileged_amuxcp0      : 1;  ///< Privileged access enabler for AMUXCP0
  bool sl_hal_smu_privileged_eusart0      : 1;  ///< Privileged access enabler for EUSART0
  bool sl_hal_smu_privileged_semailbox    : 1;  ///< Privileged access enabler for SEMAILBOX
  bool sl_hal_smu_privileged_ahbradio     : 1;  ///< Privileged access enabler for AHBRADIO
#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_13)
  bool sl_hal_smu_privileged_reserved0    : 1;  ///< Reserved privileged access enabler
  bool sl_hal_smu_privileged_emu          : 1;  ///< Privileged access enabler for EMU
  bool sl_hal_smu_privileged_cmu          : 1;  ///< Privileged access enabler for CMU
  bool sl_hal_smu_privileged_hfrco0       : 1;  ///< Privileged access enabler for HFRCO0
  bool sl_hal_smu_privileged_fsrco        : 1;  ///< Privileged access enabler for FSRCO
  bool sl_hal_smu_privileged_dpll0        : 1;  ///< Privileged access enabler for DPLL0
  bool sl_hal_smu_privileged_lfxo         : 1;  ///< Privileged access enabler for LFXO
  bool sl_hal_smu_privileged_lfrco        : 1;  ///< Privileged access enabler for LFRCO
  bool sl_hal_smu_privileged_ulfrco       : 1;  ///< Privileged access enabler for ULFRCO
  bool sl_hal_smu_privileged_msc          : 1;  ///< Privileged access enabler for MSC
  bool sl_hal_smu_privileged_icache0      : 1;  ///< Privileged access enabler for ICACHE0
  bool sl_hal_smu_privileged_prs          : 1;  ///< Privileged access enabler for PRS0
  bool sl_hal_smu_privileged_gpio         : 1;  ///< Privileged access enabler for GPIO
  bool sl_hal_smu_privileged_ldma         : 1;  ///< Privileged access enabler for LDMA
  bool sl_hal_smu_privileged_ldmaxbar     : 1;  ///< Privileged access enabler for LDMAXBAR
  bool sl_hal_smu_privileged_timer0       : 1;  ///< Privileged access enabler for TIMER0
  bool sl_hal_smu_privileged_timer1       : 1;  ///< Privileged access enabler for TIMER1
  bool sl_hal_smu_privileged_timer2       : 1;  ///< Privileged access enabler for TIMER2
  bool sl_hal_smu_privileged_timer3       : 1;  ///< Privileged access enabler for TIMER3
  bool sl_hal_smu_privileged_timer4       : 1;  ///< Privileged access enabler for TIMER4
  bool sl_hal_smu_privileged_usart0       : 1;  ///< Privileged access enabler for USART0
  bool sl_hal_smu_privileged_burtc        : 1;  ///< Privileged access enabler for BURTC
  bool sl_hal_smu_privileged_i2c1         : 1;  ///< Privileged access enabler for I2C1
  bool sl_hal_smu_privileged_chiptestctrl : 1;  ///< Privileged access enabler for CHIPTESTCTRL
  bool sl_hal_smu_privileged_syscfgcfgns  : 1;  ///< Privileged access enabler for SYSCFGCFGNS
  bool sl_hal_smu_privileged_syscfg       : 1;  ///< Privileged access enabler for SYSCFG
  bool sl_hal_smu_privileged_buram        : 1;  ///< Privileged access enabler for BURAM
  bool sl_hal_smu_privileged_gpcrc        : 1;  ///< Privileged access enabler for GPCRC
  bool sl_hal_smu_privileged_dcdc         : 1;  ///< Privileged access enabler for DCDC
  bool sl_hal_smu_privileged_hostmailbox  : 1;  ///< Privileged access enabler for HOSTMAILBOX
  bool sl_hal_smu_privileged_eusart1      : 1;  ///< Privileged access enabler for EUSART1
  bool sl_hal_smu_privileged_eusart2      : 1;  ///< Privileged access enabler for EUSART2

  bool sl_hal_smu_privileged_sysrtc       : 1;  ///< Privileged access enabler for SYSRTC
  bool sl_hal_smu_privileged_lcd          : 1;  ///< Privileged access enabler for LCD
  bool sl_hal_smu_privileged_keyscan      : 1;  ///< Privileged access enabler for KEYSCAN
  bool sl_hal_smu_privileged_dmem         : 1;  ///< Privileged access enabler for DMEM
  bool sl_hal_smu_privileged_lcdrf        : 1;  ///< Privileged access enabler for LCDRF
  bool sl_hal_smu_privileged_pfmxpprf     : 1;  ///< Privileged access enabler for PFMXPPRF
  bool sl_hal_smu_privileged_wdog0        : 1;  ///< Privileged access enabler for WDOG0
  bool sl_hal_smu_privileged_radioaes     : 1;  ///< Privileged access enabler for RADIOAES
  bool sl_hal_smu_privileged_smu          : 1;  ///< Privileged access enabler for SMU
  bool sl_hal_smu_privileged_smucfgns     : 1;  ///< Privileged access enabler for SMUCFGNS
  bool sl_hal_smu_privileged_letimer0     : 1;  ///< Privileged access enabler for LETIMER0
  bool sl_hal_smu_privileged_iadc0        : 1;  ///< Privileged access enabler for IADC0
  bool sl_hal_smu_privileged_acmp0        : 1;  ///< Privileged access enabler for ACMP0
  bool sl_hal_smu_privileged_acmp1        : 1;  ///< Privileged access enabler for ACMP1
  bool sl_hal_smu_privileged_amuxcp0      : 1;  ///< Privileged access enabler for AMUXCP0
  bool sl_hal_smu_privileged_vdac0        : 1;  ///< Privileged access enabler for VDAC0
  bool sl_hal_smu_privileged_pcnt         : 1;  ///< Privileged access enabler for PCNT
  bool sl_hal_smu_privileged_lesense      : 1;  ///< Privileged access enabler for LESENSE
  bool sl_hal_smu_privileged_hfrco1       : 1;  ///< Privileged access enabler for HFRCO1
  bool sl_hal_smu_privileged_hfxo0        : 1;  ///< Privileged access enabler for HFXO0
  bool sl_hal_smu_privileged_i2c0         : 1;  ///< Privileged access enabler for I2C0
  bool sl_hal_smu_privileged_wdog1        : 1;  ///< Privileged access enabler for WDOG1
  bool sl_hal_smu_privileged_eusart0      : 1;  ///< Privileged access enabler for EUSART0
  bool sl_hal_smu_privileged_semailbox    : 1;  ///< Privileged access enabler for SEMAILBOX
  bool sl_hal_smu_privileged_ahbradio     : 1;  ///< Privileged access enabler for AHBRADIO
#else
#error "No peripherals defined for SMU for this device configuration"
#endif
} sl_hal_smu_privileged_t;

/*******************************************************************************
 ******************************   STRUCTS   ************************************
 ******************************************************************************/

/** SMU initialization structure. */
typedef struct {
  union {
#if defined(_SMU_PPUNSPATD2_MASK)
    uint32_t reg[3];                      ///< Peripheral access control array.
#else
    uint32_t reg[2];                      ///< Peripheral access control array.
#endif
    sl_hal_smu_privileged_t access;       ///< Peripheral access control array.
  } ppu;                                  ///< PPU init array.
  bool enable;                            ///< SMU enable flag. When set, SMU_Init() will enable SMU.
} sl_hal_smu_init_t;

/** Default SMU initialization structure settings. */
#define SMU_INIT_DEFAULT {                                        \
    { { 0 } },             /* No peripherals access protected. */ \
    true                   /* Enable SMU.*/                       \
}

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Enable or disable PPU of SMU.
 *
 * @param[in] enable
 *   Set to true to enable PPU; set to false otherwise.
 ******************************************************************************/
__STATIC_INLINE void sl_hal_smu_enable_ppu(bool enable)
{
#if defined (_SMU_PPUCTRL_ENABLE_SHIFT)
  sl_hal_bus_ram_write_bit(&SMU->PPUCTRL, _SMU_PPUCTRL_ENABLE_SHIFT, enable);
#else
  (void)enable;
#endif
}

/***************************************************************************//**
 * @brief
 *   Initialize PPU of SMU.
 *
 * @param[in] init
 *   Pointer to initialization structure that defines which peripherals should
 *   only be accessed from privileged mode, and if PPU should be enabled.
 ******************************************************************************/
__STATIC_INLINE void sl_hal_smu_init(const sl_hal_smu_init_t *init)
{
#if !defined (SL_TRUSTZONE_SECURE) && defined(_SILICON_LABS_32B_SERIES_2_CONFIG) \
  && (_SILICON_LABS_32B_SERIES_2_CONFIG >= 2)
  SMU_NS_CFGNS->PPUNSPATD0 = init->ppu.reg[0];
  SMU_NS_CFGNS->PPUNSPATD1 = init->ppu.reg[1];
#if defined(_SMU_PPUNSPATD2_MASK)
  SMU_NS_CFGNS->PPUNSPATD2 = init->ppu.reg[2];
#endif //defined(_SMU_PPUNSPATD2_MASK)
#else
  SMU->PPUPATD0 = init->ppu.reg[0];
  SMU->PPUPATD1 = init->ppu.reg[1];
#if defined(_SMU_PPUNSPATD2_MASK)
  SMU->PPUPATD2 = init->ppu.reg[2];
#endif //defined(_SMU_PPUNSPATD2_MASK)
#endif //SL_TRUSTZONE_SECURE

  sl_hal_smu_enable_ppu(init->enable);
}
/***************************************************************************//**
 * @brief
 *   Change access settings for a peripheral.
 *
 * @details
 *   Set to limit access of a peripheral from privileged mode.
 *
 * @param[in] peripheral
 *   ID of the peripheral to change access settings for.
 *
 * @param[in] privileged
 *   Set to true if the peripheral should only be accessed from privileged
 *   mode; set to false otherwise.
 ******************************************************************************/
__STATIC_INLINE void sl_hal_smu_set_privileged_access(sl_hal_smu_peripheral_id_t peripheral,
                                                      bool privileged)
{
  EFM_ASSERT(peripheral < SL_HAL_SMU_END);

#if !defined (SL_TRUSTZONE_SECURE) && defined(_SILICON_LABS_32B_SERIES_2_CONFIG) \
  && (_SILICON_LABS_32B_SERIES_2_CONFIG >= 2)
  if (peripheral < 32) {
    sl_hal_bus_ram_write_bit(&SMU_NS_CFGNS->PPUNSPATD0, peripheral, privileged);
  } else if (peripheral < 64) {
    sl_hal_bus_ram_write_bit(&SMU_NS_CFGNS->PPUNSPATD1, peripheral - 32, privileged);
  } else {
#if defined(_SMU_PPUNSPATD2_MASK)
    sl_hal_bus_ram_write_bit(&SMU_NS_CFGNS->PPUNSPATD2, peripheral - 64, privileged);
#else
    EFM_ASSERT(false);
#endif //defined(_SMU_PPUNSPATD2_MASK)
  }
#else
  if (peripheral < 32) {
    sl_hal_bus_ram_write_bit(&SMU->PPUPATD0, peripheral, privileged);
  } else if (peripheral < 64) {
    sl_hal_bus_ram_write_bit(&SMU->PPUPATD1, peripheral - 32, privileged);
  } else {
#if defined(_SMU_PPUNSPATD2_MASK)
    sl_hal_bus_ram_write_bit(&SMU_NS_CFGNS->PPUNSPATD2, peripheral - 64, privileged);
#else
    EFM_ASSERT(false);
#endif //defined(_SMU_PPUNSPATD2_MASK)
  }
#endif //SL_TRUSTZONE_SECURE
}

/***************************************************************************//**
 * @brief
 *   Get the ID of the peripheral that caused an access fault.
 *
 * @note
 *   The return value is only valid if SMU_IF_PPUPRIV interrupt flag
 *   is set.
 *
 * @return
 *   ID of the peripheral that caused an access fault.
 ******************************************************************************/
__STATIC_INLINE sl_hal_smu_peripheral_id_t sl_hal_smu_get_faulting_peripheral(void)
{
#if !defined (SL_TRUSTZONE_SECURE) && defined(_SILICON_LABS_32B_SERIES_2_CONFIG) \
  && (_SILICON_LABS_32B_SERIES_2_CONFIG >= 2)
  return (sl_hal_smu_peripheral_id_t)SMU_NS_CFGNS->PPUNSFS;
#else
  return (sl_hal_smu_peripheral_id_t)SMU->PPUFS;
#endif //SL_TRUSTZONE_SECURE
}

/***************************************************************************//**
 * @brief
 *   Clear one or more pending SMU interrupts.
 *
 * @param[in] flags
 *   Bitwise logic OR of SMU interrupt sources to clear.
 ******************************************************************************/
__STATIC_INLINE void sl_hal_smu_int_clear(uint32_t flags)
{
#if defined (SMU_HAS_SET_CLEAR)
#if !defined (SL_TRUSTZONE_SECURE) && defined(_SILICON_LABS_32B_SERIES_2_CONFIG) \
  && (_SILICON_LABS_32B_SERIES_2_CONFIG >= 2)
  SMU_NS_CFGNS->NSIF_CLR = flags;
#else
  SMU->IF_CLR = flags;
#endif //SL_TRUSTZONE_SECURE
#else
  SMU->IFC = flags;
#endif //SMU_HAS_SET_CLEAR
}

/***************************************************************************//**
 * @brief
 *   Disable one or more SMU interrupts.
 *
 * @param[in] flags
 *   SMU interrupt sources to disable.
 ******************************************************************************/
__STATIC_INLINE void sl_hal_smu_int_disable(uint32_t flags)
{
#if defined (SMU_HAS_SET_CLEAR)
#if !defined (SL_TRUSTZONE_SECURE) && defined(_SILICON_LABS_32B_SERIES_2_CONFIG) \
  && (_SILICON_LABS_32B_SERIES_2_CONFIG >= 2)
  SMU_NS_CFGNS->NSIEN_CLR = flags;
#else
  SMU->IEN_CLR = flags;
#endif //SL_TRUSTZONE_SECURE
#else
  SMU->IEN &= ~flags;
#endif //SMU_HAS_SET_CLEAR
}

/***************************************************************************//**
 * @brief
 *   Enable one or more SMU interrupts.
 *
 * @note
 *   Depending on the use, a pending interrupt may already be set prior to
 *   enabling the interrupt. To ignore a pending interrupt, consider using
 *   SMU_IntClear() prior to enabling the interrupt.
 *
 * @param[in] flags
 *   SMU interrupt sources to enable.
 ******************************************************************************/
__STATIC_INLINE void sl_hal_smu_int_enable(uint32_t flags)
{
#if defined (SMU_HAS_SET_CLEAR)
#if !defined (SL_TRUSTZONE_SECURE) && defined(_SILICON_LABS_32B_SERIES_2_CONFIG) \
  && (_SILICON_LABS_32B_SERIES_2_CONFIG >= 2)
  SMU_NS_CFGNS->NSIEN_SET = flags;
#else
  SMU->IEN_SET = flags;
#endif //SL_TRUSTZONE_SECURE
#else
  SMU->IEN |= flags;
#endif //SMU_HAS_SET_CLEAR
}

/***************************************************************************//**
 * @brief
 *   Get pending SMU interrupts.
 *
 * @return
 *   SMU interrupt sources pending.
 ******************************************************************************/
__STATIC_INLINE uint32_t sl_hal_smu_int_get(void)
{
#if !defined (SL_TRUSTZONE_SECURE) && defined(_SILICON_LABS_32B_SERIES_2_CONFIG) \
  && (_SILICON_LABS_32B_SERIES_2_CONFIG >= 2)
  return SMU_NS_CFGNS->NSIF;
#else
  return SMU->IF;
#endif //SL_TRUSTZONE_SECURE
}

/***************************************************************************//**
 * @brief
 *   Get enabled and pending SMU interrupt flags.
 *
 * @note
 *   Interrupt flags are not cleared by this function.
 *
 * @return
 *   Pending and enabled SMU interrupt sources.
 *   The return value is the bitwise AND combination of
 *   - the OR combination of enabled interrupt sources in SMU_IEN register
 *     and
 *   - the OR combination of valid interrupt flags in SMU_IF register.
 ******************************************************************************/
__STATIC_INLINE uint32_t sl_hal_smu_int_get_enabled(void)
{
  uint32_t tmp;

#if !defined (SL_TRUSTZONE_SECURE) && defined(_SILICON_LABS_32B_SERIES_2_CONFIG) \
  && (_SILICON_LABS_32B_SERIES_2_CONFIG >= 2)
  // Store SMU->IEN in temporary variable to define explicit order
  // of volatile accesses.
  tmp = SMU_NS_CFGNS->NSIEN;

  // Bitwise AND of pending and enabled interrupts.
  return SMU_NS_CFGNS->NSIF & tmp;
#else
  // Store SMU->IEN in temporary variable to define explicit order
  // of volatile accesses.
  tmp = SMU->IEN;

  // Bitwise AND of pending and enabled interrupts.
  return SMU->IF & tmp;
#endif //SL_TRUSTZONE_SECURE
}

/***************************************************************************//**
 * @brief
 *   Set one or more pending SMU interrupts from SW.
 *
 * @param[in] flags
 *   SMU interrupt sources to set to pending.
 ******************************************************************************/
__STATIC_INLINE void sl_hal_smu_int_set(uint32_t flags)
{
#if defined (SMU_HAS_SET_CLEAR)
#if !defined (SL_TRUSTZONE_SECURE) && defined(_SILICON_LABS_32B_SERIES_2_CONFIG) \
  && (_SILICON_LABS_32B_SERIES_2_CONFIG >= 2)
  SMU_NS_CFGNS->NSIF_SET = flags;
#else
  SMU->IF_SET = flags;
#endif //SL_TRUSTZONE_SECURE
#else
  SMU->IFS = flags;
#endif //SMU_HAS_SET_CLEAR
}

/**************************************************************************//**
* @brief
*   SMU secure IRQ Handler.
*
* @details
*   When a PPU detects an access to a secure peripheral at its non-secure
*   address or an access to a non-secure peripheral at its secure
*   address, PPUSECIF in SMU_IF is set and the ID of the peripheral being
*   accessed is written to SMU_PPUFS. If PPUSECIEN is set and the SMU's
*   Secure IRQ enabled, the CPU will be interrupted and SMU_SECURE_IRQHandler
*   Will handle the interrupt.
******************************************************************************/
#if !defined (SL_TRUSTZONE_SECURE) && defined (_SILICON_LABS_32B_SERIES_2)
void SMU_SECURE_IRQHandler(void)
{
  if (SMU_IF_PPUSEC) {
    EFM_ASSERT(SMU->IF & SMU_IF_PPUSEC);
  }

  if (SMU_IF_BMPUSEC) {
    EFM_ASSERT(SMU->IF & SMU_IF_BMPUSEC);
  }

  // PPUFS contains the ID of the peripheral caused the fault
  // The ID is ordered after the PPUSATD0-PPUSATD1 register bit fields.
  EFM_ASSERT(SMU->PPUFS);

  while (1) {
    // do nothing
  }
}
#endif //SL_TRUSTZONE_SECURE

/** @} (end addtogroup smu) */

#ifdef __cplusplus
}
#endif

#endif // defined(SMU_COUNT) && (SMU_COUNT > 0)
#endif // SL_HAL_SMU_H
