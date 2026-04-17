/******************************************************************************
* @file  rsi_wwdt.c
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

#include "si91x_device.h"
#include "rsi_power_save.h"
#include "rsi_wwdt.h"

/** @addtogroup SOC27
 * @{
 */
/*==============================================*/
/**
 * @fn           void RSI_WWDT_Init(const MCU_WDT_Type *pstcWDT)
 * @brief        This API is used to initialize the Watch dog timer
 * @param[in]    pstcWDT : pointer to the WDT register instance
 * @return       none
 */

void RSI_WWDT_Init(const MCU_WDT_Type *pstcWDT)
{
  UNUSED_PARAMETER(pstcWDT);
  RSI_PS_NpssPeriPowerUp(SLPSS_PWRGATE_ULP_MCUWDT | SLPSS_PWRGATE_ULP_TIMEPERIOD);
}

/*==============================================*/
/**
 * @fn           void RSI_WWDT_IntrClear(void)
 * @brief        This API is used to clear the WDT interrupt
 * @return       none
 */

void RSI_WWDT_IntrClear(void)
{
  NPSS_INTR_CLEAR_REG                  = NPSS_TO_MCU_WDT_INTR;
  MCU_FSM->MCU_FSM_WAKEUP_STATUS_CLEAR = WWD_INTERRUPT_STATUS_CLEAR;
  return;
}

/*==============================================*/
/**
 * @fn           uint8_t RSI_WWDT_GetIntrStatus(void)
 * @brief        This API is used to get the WDT interrupt status
 * @return       returns the WDT interrupt status.
 */

uint8_t RSI_WWDT_GetIntrStatus(void)
{
  if (NPSS_INTR_STATUS_REG & NPSS_TO_MCU_WDT_INTR) {
    return 1;
  } else {
    return 0;
  }
}

/*==============================================*/
/**
 * @fn           void RSI_WWDT_DeInit(MCU_WDT_Type *pstcWDT)
 * @brief        This API is used to disable the WDT.
 * @param[in]    pstcWDT : pointer to the WDT register instance
 * @return       none
 */

void RSI_WWDT_DeInit(MCU_WDT_Type *pstcWDT)
{
  RSI_WWDT_Disable(pstcWDT);
  return;
}

/*==============================================*/
/**
 * @fn           void RSI_WWDT_Start(MCU_WDT_Type *pstcWDT)
 * @brief        This API is used to start the WDT.
 * @param[in]    pstcWDT : pointer to the WDT register instance
 * @return       none
 */

void RSI_WWDT_Start(MCU_WDT_Type *pstcWDT)
{
  /*CLEAR MASK*/
  /*0xAA to enable the watch dog */
  pstcWDT->MCU_WWD_MODE_AND_RSTART_b.WWD_MODE_EN_STATUS = 0xAA;
  pstcWDT->MCU_WWD_MODE_AND_RSTART_b.WWD_MODE_RSTART    = 1;
  return;
}
/** @} */
/*End of file not truncated*/
