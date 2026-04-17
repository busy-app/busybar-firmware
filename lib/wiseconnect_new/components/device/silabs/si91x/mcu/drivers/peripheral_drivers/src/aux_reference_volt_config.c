/******************************************************************************
* @file  aux_reference_volt_config.c
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

#include "si91x_device.h"
#include "aux_reference_volt_config.h"

/*==============================================*/
/**
 * @fn        rsi_error_t RSI_AUX_RefVoltageConfig(float verf_val, float chip_vltg)
 * @brief     This API used for configure reference voltage for ADC operation.
 * @param[in] verf_val     - Reference voltage for ADC.
 * @param[in] chip_vltg    - Chip voltage 3.2V or 1.8V,If chip voltage to chip is 1.8V then. 
 *							\n chip voltage(bypass LDO) act as reference voltage to analog peripheral.
 * @return    Success      - Returns 'RSI_OK' on successful execution.
 */
rsi_error_t RSI_AUX_RefVoltageConfig(float verf_val, float chip_vltg)
{
  uint8_t temp                                 = 0;
  float cntrl_val                              = 0;
  AUX_ADC_DAC_COMP->AUX_LDO_b.ENABLE_LDO       = 1;
  AUX_ADC_DAC_COMP->AUX_LDO_b.LDO_DEFAULT_MODE = 1;
  if ((verf_val > (float)2.8) || (chip_vltg < (float)3.0)) {
    AUX_ADC_DAC_COMP->AUX_LDO_b.BYPASS_LDO = 1;
  } else {
    if (verf_val < (float)1.6) {
      AUX_ADC_DAC_COMP->AUX_LDO_b.LDO_CTRL         = 0;
      AUX_ADC_DAC_COMP->AUX_LDO_b.ENABLE_LDO       = 0;
      AUX_ADC_DAC_COMP->AUX_LDO_b.LDO_DEFAULT_MODE = 0;
    } else {
      cntrl_val = ((verf_val - (float)1.6) / (float)(0.08));
      temp      = (uint8_t)(cntrl_val + 1);
      cntrl_val = (cntrl_val + (float)0.50);
      if ((float)temp > cntrl_val) {
        temp = (uint8_t)cntrl_val;
      }
      AUX_ADC_DAC_COMP->AUX_LDO_b.LDO_CTRL = (unsigned int)(temp & 0x0F);
    }
  }
  return RSI_OK;
}
