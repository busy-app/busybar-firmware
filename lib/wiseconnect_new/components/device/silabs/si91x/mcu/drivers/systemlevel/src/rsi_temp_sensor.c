/******************************************************************************
* @file  rsi_temp_sensor.c
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
#include "base_types.h"
#include "si91x_device.h"
#include "rsi_ipmu.h"
/*==============================================*/
/** 
 * @fn          void RSI_TS_SetCntFreez(MCU_TEMP_Type *pstcTempSens, uint32_t u32CountFreez)
 * @brief       This API is used to set the count of reference clock on which ptat clock counts
 * @param[in]   pstcTempSens  : pointer to the temperature sensor register instance
 * @param[in]   u32CountFreez : count of reference clock on which ptat clock counts
 * @return      none
 */

void RSI_TS_SetCntFreez(MCU_TEMP_Type *pstcTempSens, uint32_t u32CountFreez)
{
  pstcTempSens->TS_ENABLE_AND_TEMPERATURE_DONE_b.CONT_COUNT_FREEZE = (unsigned int)(u32CountFreez & 0x03FF);
  return;
}

/*==============================================*/
/** 
 * @fn         void RSI_TS_RefClkSel(MCU_TEMP_Type *pstcTempSens, boolean_t bRefClk)
 * @brief      This API is used to select the reference clock to the temperature sensor
 * @param[in]  pstcTempSens : pointer to the temperature sensor register instance
 * @param[in]  bRefClk      : reference clock selection
 *                - 0: reference RO clock from analog
 *                - 1: MCU FSM clock
 * @return      none
 */

void RSI_TS_RefClkSel(MCU_TEMP_Type *pstcTempSens, boolean_t bRefClk)
{

  pstcTempSens->TS_ENABLE_AND_TEMPERATURE_DONE_b.REF_CLK_SEL = (unsigned int)(bRefClk & 0x01);
  return;
}

/*==============================================*/
/** 
 * @fn          void RSI_TS_Enable(MCU_TEMP_Type *pstcTempSens, boolean_t bEn)
 * @brief       This API is used to enable / disable the temperature sensor
 * @param[in]    pstcTempSens  : pointer to the temperature sensor register instance
 * @param[in]    bEn           : enable / disable parameter
 *                - 0: Disable
 *                - 1: Enable
 * @return      none
 */

void RSI_TS_Enable(MCU_TEMP_Type *pstcTempSens, boolean_t bEn)
{
  pstcTempSens->TS_ENABLE_AND_TEMPERATURE_DONE_b.TEMP_SENS_EN = (unsigned int)(bEn & 0x01);
  return;
}

/*==============================================*/
/**
 * @fn           void RSI_TS_Config(MCU_TEMP_Type *pstcTempSens, uint32_t u32Nomial)   
 * @brief        This API is used to set the slope of the temperature sensor
 * @param[in]    pstcTempSens  : pointer to the temperature sensor register instance
 * @param[in]    u32Slope      : slope value to be programmed
 * @param[in]    u32Nomial     : calibrated temperature value
 * @param[in]    u32F2Nominal  : ptat clock count during calibration. This will vary with chip to chip
 * @return       none
 */

void RSI_TS_Config(MCU_TEMP_Type *pstcTempSens, uint32_t u32Nomial)
{
  volatile uint32_t reg_write_data                                  = 0;
  reg_write_data                                                    = RSI_IPMU_RO_TsConfig();
  pstcTempSens->TS_SLOPE_SET_b.SLOPE                                = (unsigned int)(reg_write_data & 0x03FF);
  pstcTempSens->TS_FE_COUNTS_NOMINAL_SETTINGS_b.NOMINAL_TEMPERATURE = (unsigned int)(u32Nomial & 0x7F);
  reg_write_data                                                    = RSI_IPMU_RO_TsEfuse();
  pstcTempSens->TS_FE_COUNTS_NOMINAL_SETTINGS_b.F2_NOMINAL          = (unsigned int)(reg_write_data & 0x03FF);
  return;
}

/*==============================================*/
/**
 * @fn          uint32_t RSI_TS_ReadTemp(const MCU_TEMP_Type *pstcTempSens)    
 * @brief       This API is used to read the temperature value
 * @param[in]   pstcTempSens   : pointer to the temperature sensor register instance
 * @return      returns the temperature value
 */

uint32_t RSI_TS_ReadTemp(const MCU_TEMP_Type *pstcTempSens)
{
  /*Wait for done */
  while (pstcTempSens->TS_ENABLE_AND_TEMPERATURE_DONE_b.TEMP_MEASUREMENT_DONE != 1)
    ;
  /*Return the temperature value*/
  return pstcTempSens->TEMPERATURE_READ_b.TEMPERATURE_RD;
}

/*==============================================*/
/** 
 * @fn          void RSI_TS_RoBjtEnable(MCU_TEMP_Type *pstcTempSens, boolean_t enable)
 * @brief       This API is used to read the temperature value
 * @param[in]   pstcTempSens   : pointer to the temperature sensor register instance
 * @param[in]   enable         : enable or disable bjt based temp sensor
 *                - 0: Disable
 *                - 1: Enable
 * @return      returns the temperature value
 */

void RSI_TS_RoBjtEnable(MCU_TEMP_Type *pstcTempSens, boolean_t enable)
{
  if (pstcTempSens->TS_SLOPE_SET_b.BJT_BASED_TEMP == 1U) {
    if (enable == 1U) {
      return;
    }
    pstcTempSens->TS_SLOPE_SET_b.BJT_BASED_TEMP = (unsigned int)(enable & 0x01); // 0
    return;
  }

  if (pstcTempSens->TS_SLOPE_SET_b.BJT_BASED_TEMP == 0U && enable == 1U) {
    pstcTempSens->TS_SLOPE_SET_b.BJT_BASED_TEMP = 1;
    for (uint32_t i = 100; i; i--)
      ; // wait for 100 us
    return;
  }

  pstcTempSens->TS_SLOPE_SET_b.BJT_BASED_TEMP = (unsigned int)(enable & 0x01); // 0
}

/*==============================================*/
/**
 * @fn          void RSI_TS_LoadBjt(MCU_TEMP_Type *pstcTempSens, uint8_t temp)
 * @brief       This API is used to read the temperature value
 * @param[in]   pstcTempSens   : pointer to the temperature sensor register instance
 * @param[in]   temp           : known temprature
 * @return      returns the temperature value
 */

void RSI_TS_LoadBjt(MCU_TEMP_Type *pstcTempSens, uint8_t temp)
{
  pstcTempSens->TS_SLOPE_SET_b.TEMPERATURE_SPI = temp; // update the temp value
  pstcTempSens->TS_SLOPE_SET_b.TEMP_UPDATED    = 1;
}

/*==============================================*/
/**
 * @fn         uint32_t RSI_TS_GetRefClkCnt(const MCU_TEMP_Type *pstcTempSens)     
 * @brief      This API is used to read the reference clock count
 * @param[in]  pstcTempSens   : pointer to the temperature sensor register instance
 * @return     returns the reference clock count
 */

uint32_t RSI_TS_GetRefClkCnt(const MCU_TEMP_Type *pstcTempSens)
{
  /*Return the count value*/
  return pstcTempSens->TS_COUNTS_READ_b.COUNT_F1;
}

/*==============================================*/
/**
 * @fn         uint32_t RSI_TS_GetPtatClkCnt(const MCU_TEMP_Type *pstcTempSens)
 * @brief      This API is used to read the ptat clock count
 * @param[in]  pstcTempSens   : pointer to the temperature sensor register instance
 * @return     returns the ptat clock count
 */

uint32_t RSI_TS_GetPtatClkCnt(const MCU_TEMP_Type *pstcTempSens)
{
  /*Return the count value*/
  return pstcTempSens->TS_COUNTS_READ_b.COUNT_F2;
}

/*==============================================*/
/**
 * @fn         void RSI_Periodic_TempUpdate(TIME_PERIOD_Type *temp, uint8_t enable, uint8_t trigger_time)
 * @brief      This API is used to update the temp periodically after some time
 * @parm[in]   temp         : pointer to the timeperiod register instance
 * @parm[in]   enable       : enable periodic checking of temp
 * @parm[in]   trigger_time : periodic check time in sec
 *             0 -1 sec
 *             1 -2 sec
 *             2 -4 sec
 *             3 -5 sec
 * @return     none
 */

void RSI_Periodic_TempUpdate(TIME_PERIOD_Type *temp, uint8_t enable, uint8_t trigger_time)
{
  temp->MCU_CAL_TEMP_PROG_REG_b.PERIODIC_TEMP_CALIB_EN = (unsigned int)(enable & 0x01);
  temp->MCU_CAL_TEMP_PROG_REG_b.TEMP_TRIGGER_TIME_SEL  = (unsigned int)(trigger_time & 0x03);
  return;
}

/*End of file not truncated */
