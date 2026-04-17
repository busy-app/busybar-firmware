/*******************************************************************************
* @file  rsi_wwdt.h
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

/**
 * Includes
 */
#ifndef __RSI_WDT_H__
#define __RSI_WDT_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "rsi_ccp_common.h"
#include "base_types.h"
#include "rsi_power_save.h"

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif // UNUSED_PARAMETER

#define WDT_SYSTEM_RESET_TIMER 0x3
#define WDT_INTERRUPT_TIMER    0x1
#define WDT_IRQHandler         IRQ020_Handler            /*!<WDT IRQ Handler*/
#define NVIC_WDT               NPSS_TO_MCU_WDT_INTR_IRQn /*!<WDT NVIC IRQ Numb */

/**
 * @fn          void RSI_WWDT_ConfigWindowTimer(MCU_WDT_Type *pstcWDT , uint16_t u16IntrTimerVal)
 * @brief       This API is used to configure the window timer of the watch dog timer
 * @param[in]     pstcWDT          : pointer to the WDT register instance
 * @param[in]     u16WindowTimerVal  : window timer value
 * @return      None
 */
STATIC INLINE void RSI_WWDT_ConfigWindowTimer(MCU_WDT_Type *pstcWDT, uint8_t u8WindowTimerVal)
{
  pstcWDT->MCU_WWD_WINDOW_TIMER_b.WINDOW_TIMER = (unsigned int)(u8WindowTimerVal & 0x0F);
}

/**
 * @fn          void RSI_WWDT_ConfigIntrTimer(MCU_WDT_Type *pstcWDT , uint16_t u16IntrTimerVal)
 * @brief       This API is used to configure the interrupt timer of the watch dog timer
 * @param[in]     pstcWDT          : pointer to the WDT register instance
 * @param[in]     u16IntrTimerVal  : interrupt timer value
 * @return      None
 */
STATIC INLINE void RSI_WWDT_ConfigIntrTimer(MCU_WDT_Type *pstcWDT, uint16_t u16IntrTimerVal)
{
  pstcWDT->MCU_WWD_INTERRUPT_TIMER_b.WWD_INTERRUPT_TIMER = (unsigned int)(u16IntrTimerVal & 0x1F);
}

/**
 * @fn          void RSI_WWDT_ConfigSysRstTimer(MCU_WDT_Type *pstcWDT , uint16_t u16SysRstVal)  
 * @brief       This API is used to configure the system reset timer of the watch dog timer
 * @param[in]   pstcWDT         : pointer to the WDT register instance
 * @param[in]     u16SysRstVal    : reset value
 * @return      NONE
 */
STATIC INLINE void RSI_WWDT_ConfigSysRstTimer(MCU_WDT_Type *pstcWDT, uint16_t u16SysRstVal)
{
  pstcWDT->MCU_WWD_SYSTEM_RESET_TIMER_b.WWD_SYSTEM_RESET_TIMER = (unsigned int)(u16SysRstVal & 0x1F);
}

/**
 * @fn          void RSI_WWDT_Disable(MCU_WDT_Type *pstcWDT)
 * @brief       This API is used to Disable the Watch dog timer
 * @param[in]     pstcWDT   :pointer to the WDT register instance
 * @return      None
 */
STATIC INLINE void RSI_WWDT_Disable(MCU_WDT_Type *pstcWDT)
{
  /*0xF0 to Disable the watch dog */
  pstcWDT->MCU_WWD_MODE_AND_RSTART_b.WWD_MODE_EN_STATUS = 0xF0;
}

/**
 * @fn           void RSI_WWDT_ReStart(MCU_WDT_Type *pstcWDT)
 * @brief        This API is used to restart the Watch dog timer
 * @param[in]    pstcWDT  :pointer to the WDT register instance
 * @return       None
 */
STATIC INLINE void RSI_WWDT_ReStart(MCU_WDT_Type *pstcWDT)
{
  pstcWDT->MCU_WWD_MODE_AND_RSTART_b.WWD_MODE_RSTART = 1U;
}

/**
 * @fn           void RSI_WWDT_IntrUnMask(void)
 * @brief        This API is used to unmask the Watch dog timer
 * @return       None
 */
STATIC INLINE void RSI_WWDT_IntrUnMask(void)
{
  NPSS_INTR_MASK_CLR_REG = NPSS_TO_MCU_WDT_INTR;
}

/**
 * @fn           void RSI_WWDT_IntrMask(void)
 * @brief        This API is used to mask the Watch dog timer
 * @return       None
 */
STATIC INLINE void RSI_WWDT_IntrMask(void)
{
  NPSS_INTR_MASK_SET_REG = NPSS_TO_MCU_WDT_INTR;
}

/**
 * @fn           void RSI_WWDT_SysRstOnProcLockEnable(MCU_WDT_Type *pstcWDT)
 * @brief        This API is used to enable Watch dog timer to reset system on processor stuck
 * @return       None
 */
STATIC INLINE void RSI_WWDT_SysRstOnProcLockEnable(MCU_WDT_Type *pstcWDT)
{
  pstcWDT->MCU_WWD_ARM_STUCK_EN_b.PROCESSOR_STUCK_RESET_EN = ENABLE;
}

/**
 * @fn           void RSI_WWDT_SysRstOnProcLockDisable(MCU_WDT_Type *pstcWDT)
 * @brief        This API is used to disable Watch dog timer to reset system on processor stuck
 * @return       None
 */
STATIC INLINE void RSI_WWDT_SysRstOnProcLockDisable(MCU_WDT_Type *pstcWDT)
{
  pstcWDT->MCU_WWD_ARM_STUCK_EN_b.PROCESSOR_STUCK_RESET_EN = DISABLE;
}

/**
 * @fn           void RSI_WWDT_GetProcLockSignal(const MCU_WDT_Type *pstcWDT)
 * @brief        This API is used to read signal for processor stuck reset enable
 * @return       None
 */
STATIC INLINE uint8_t RSI_WWDT_GetProcLockRstEnableSignal(const MCU_WDT_Type *pstcWDT)
{
  if (pstcWDT->MCU_WWD_ARM_STUCK_EN_b.PROCESSOR_STUCK_RESET_EN_) {
    return 1;
  } else {
    return 0;
  }
}

/**
 * @fn         uint16_t RSI_WWDT_GetIntrTime(const MCU_WDT_Type *pstcWDT)
 * @brief      This API is used to read the interrupt time of the watch dog timer
 * @param[in]  pstcWDT          : pointer to the WDT register instance
 * @return     uint8_t  : interrupt timer value
 */
STATIC INLINE uint8_t RSI_WWDT_GetIntrTime(const MCU_WDT_Type *pstcWDT)
{
  uint8_t interrupt_time;
  interrupt_time = pstcWDT->MCU_WWD_INTERRUPT_TIMER_b.WWD_INTERRUPT_TIMER;
  return interrupt_time;
}

/**
 * @fn         uint16_t RSI_WWDT_GetSysRstTime(const MCU_WDT_Type *pstcWDT)
 * @brief      This API is used to read the system reset time of the watch dog timer
 * @param[in]  pstcWDT          : pointer to the WDT register instance
 * @return     uint8_t  : system reset timer value
 */
STATIC INLINE uint8_t RSI_WWDT_GetSysRstTime(const MCU_WDT_Type *pstcWDT)
{
  uint8_t system_reset_time;
  system_reset_time = pstcWDT->MCU_WWD_SYSTEM_RESET_TIMER_b.WWD_SYSTEM_RESET_TIMER;
  return system_reset_time;
}

/**
 * @fn         uint8_t RSI_WWDT_GetWindowTime(const MCU_WDT_Type *pstcWDT)
 * @brief      This API is used to read the system reset time of the watch dog timer
 * @param[in]  pstcWDT          : pointer to the WDT register instance
 * @return     uint8_t  : system reset timer value
 */
STATIC INLINE uint8_t RSI_WWDT_GetWindowTime(const MCU_WDT_Type *pstcWDT)
{
  uint8_t window_time;
  window_time = pstcWDT->MCU_WWD_WINDOW_TIMER_b.WINDOW_TIMER;
  return window_time;
}
// Function prototypes
void RSI_WWDT_IntrClear(void);

uint8_t RSI_WWDT_GetIntrStatus(void);

void RSI_WWDT_DeInit(MCU_WDT_Type *pstcWDT);

void RSI_WWDT_Start(MCU_WDT_Type *pstcWDT);

void RSI_WWDT_Init(const MCU_WDT_Type *pstcWDT);
void IRQ020_Handler(void);

#ifdef __cplusplus
}
#endif

/*End of file not truncated*/
#endif /*__RSI_WDT_H__*/
