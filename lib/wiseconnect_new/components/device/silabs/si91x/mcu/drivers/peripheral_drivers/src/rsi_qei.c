/******************************************************************************
* @file  rsi_qei.c
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

// Include Files
#include "rsi_qei.h"
#include "rsi_ccp_common.h"

/** @addtogroup SOC25
* @{
*/
/*==============================================*/
/**
 * @fn          void RSI_QEI_Enable(volatile QEI_Type *pstcQei)      
 * @brief	    Enables the clock to the QEI module
 * @param[in]   pstcQei - Pointer to the QEI register instance
 * @return 	    none
 */
void RSI_QEI_Enable(QEI_Type *pstcQei)
{
  UNUSED_PARAMETER(pstcQei);
  // QEI Direction pin mux
  if (RTE_QEI_DIR_PIN >= 25 && RTE_QEI_DIR_PIN <= 30) {
    RSI_EGPIO_HostPadsGpioModeEnable(RTE_QEI_DIR_PIN);
  } else if (RTE_QEI_DIR_PIN > 63) {
    RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(RTE_QEI_DIR_PIN - 64));
    RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(RTE_QEI_DIR_PIN - 64), 6);
  }
  RSI_EGPIO_PadSelectionEnable(RTE_QEI_DIR_PAD);

  RSI_EGPIO_PadReceiverEnable(RTE_QEI_DIR_PIN);

  RSI_EGPIO_SetPinMux(EGPIO, RTE_QEI_DIR_PORT, RTE_QEI_DIR_PIN, RTE_QEI_DIR_MUX);

  // QEI Index pin mux
  if (RTE_QEI_IDX_PIN >= 25 && RTE_QEI_IDX_PIN <= 30) {
    RSI_EGPIO_HostPadsGpioModeEnable(RTE_QEI_IDX_PIN);
  } else if (RTE_QEI_IDX_PIN > 63) {
    RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(RTE_QEI_IDX_PIN - 64));
    RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(RTE_QEI_IDX_PIN - 64), 6);
  }
  RSI_EGPIO_PadSelectionEnable(RTE_QEI_IDX_PAD);

  RSI_EGPIO_PadReceiverEnable(RTE_QEI_IDX_PIN);

  RSI_EGPIO_SetPinMux(EGPIO, RTE_QEI_IDX_PORT, RTE_QEI_IDX_PIN, RTE_QEI_IDX_MUX);

  // QEI PhaseA pin mux
  if (RTE_QEI_PHA_PIN >= 25 && RTE_QEI_PHA_PIN <= 30) {
    RSI_EGPIO_HostPadsGpioModeEnable(RTE_QEI_PHA_PIN);
  } else if (RTE_QEI_PHA_PIN > 63) {
    RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(RTE_QEI_PHA_PIN - 64));
    RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(RTE_QEI_PHA_PIN - 64), 6);
  }
  RSI_EGPIO_PadSelectionEnable(RTE_QEI_PHA_PAD);

  RSI_EGPIO_PadReceiverEnable(RTE_QEI_PHA_PIN);

  RSI_EGPIO_SetPinMux(EGPIO, RTE_QEI_PHA_PORT, RTE_QEI_PHA_PIN, RTE_QEI_PHA_MUX);

  // QEI PhaseB pin mux
  if (RTE_QEI_PHB_PIN >= 25 && RTE_QEI_PHB_PIN <= 30) {
    RSI_EGPIO_HostPadsGpioModeEnable(RTE_QEI_PHB_PIN);
  } else if (RTE_QEI_PHB_PIN > 63) {
    RSI_EGPIO_UlpPadReceiverEnable((uint8_t)(RTE_QEI_PHB_PIN - 64));
    RSI_EGPIO_SetPinMux(EGPIO1, 0, (uint8_t)(RTE_QEI_PHB_PIN - 64), 6);
  }

  RSI_EGPIO_PadSelectionEnable(RTE_QEI_PHB_PAD);

  RSI_EGPIO_SetPinMux(EGPIO, RTE_QEI_PHB_PORT, RTE_QEI_PHB_PIN, RTE_QEI_PHB_MUX);

  RSI_EGPIO_PadReceiverEnable(RTE_QEI_PHB_PIN);

  // QEI clock enable routine here
  RSI_CLK_PeripheralClkEnable2(M4CLK, QE_PCLK_ENABLE);

  return;
}

/*==============================================*/
/**
 * @fn          void RSI_QEI_Disable(volatile QEI_Type *pstcQei)
 * @brief	    Disables the clock to the QEI module
 * @param[in]   pstcQei - Pointer to the QEI register instance
 * @return 	    none
 */
void RSI_QEI_Disable(QEI_Type *pstcQei)
{
  UNUSED_PARAMETER(pstcQei);
  // clock disable to QEI module
  RSI_CLK_PeripheralClkDisable2(M4CLK, QE_PCLK_ENABLE);
  return;
}
/** @} */
