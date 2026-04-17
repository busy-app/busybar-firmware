/******************************************************************************
* @file  rsi_opamp.c
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

// Include files
#include "rsi_opamp.h"

/** @addtogroup SOC24
* @{
*/
/*==============================================*/
/**   
 * @fn          void RSI_OPAMP1_UGB(uint8_t vin_p_sel,uint8_t enable,uint8_t lp_mode,uint8_t out_mux_en,uint8_t out_mux_sel,
 *                                  uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configure the opamp1 in Unity gain buffer
 * @param[in]   vin_p_sel   : input selection (0 - ULP_GPIO_12,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 -ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8,6 - AUX_DAC_OUT)
 * @param[in]   enable      : opamp1 enable (1 - enable ,0 - disable)           
 * @param[in]   lp_mode     : opamp1 low power mode(1 - enable ,0 - disable)           
 * @param[in]   out_mux_en  : opamp1 output mux enable(1 - enable ,0 - disable)           
 * @param[in]   out_mux_sel : opamp1 output mux selection(0 - ULP_GPIO_4,1 - ULP_GPIO_15)           
 * @param[in]   dyn_en      : opamp1 dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel     : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP1_UGB(uint8_t vin_p_sel,
                    uint8_t enable,
                    uint8_t lp_mode,
                    uint8_t out_mux_en,
                    uint8_t out_mux_sel,
                    uint8_t dyn_en,
                    uint8_t channel)
{

  UNUSED_PARAMETER(out_mux_en);
  UNUSED_PARAMETER(out_mux_sel);

  OPAMP_CONFIG_T Config;

  Config.opamp1.opamp1_dyn_en         = dyn_en;
  Config.opamp1.opamp1_sel_p_mux      = vin_p_sel;
  Config.opamp1.opamp1_sel_n_mux      = 4;
  Config.opamp1.opamp1_out_mux_en     = 1;
  Config.opamp1.opamp1_out_mux_sel    = 1;
  Config.opamp1.opamp1_res_to_out_vdd = 0;
  Config.opamp1.opamp1_res_mux_sel    = 0;
  Config.opamp1.opamp1_en_res_bank    = 0;
  Config.opamp1.opamp1_r2_sel         = 0;
  Config.opamp1.opamp1_r1_sel         = 1;
  Config.opamp1.opamp1_lp_mode        = lp_mode;
  Config.opamp1.opamp1_enable         = enable;

  RSI_OPAMP1_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP2_UGB(uint8_t vin_p_sel,uint8_t enable,uint8_t lp_mode,uint8_t out_mux_en,
 *                                  uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configure the opamp2 in Unity gain buffer 
 * @param[in]   vin_p_sel   : input selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5,3 - AUX_DAC_OUT,6 - OPAMP1_OUT)
 * @param[in]   enable      : opamp2 enable (1 - enable ,0 - disable)           
 * @param[in]   lp_mode     : opamp2 low power mode(1 - enable ,0 - disable)           
 * @param[in]   out_mux_en  : opamp2 output mux enable(1 -opamp2 output on ULP_GPIO_9)           
 * @param[in]   dyn_en      : opamp2 dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel     : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP2_UGB(uint8_t vin_p_sel,
                    uint8_t enable,
                    uint8_t lp_mode,
                    uint8_t out_mux_en,
                    uint8_t dyn_en,
                    uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Config.opamp2.opamp2_dyn_en         = dyn_en;
  Config.opamp2.opamp2_sel_p_mux      = vin_p_sel;
  Config.opamp2.opamp2_sel_n_mux      = 3;
  Config.opamp2.opamp2_out_mux_en     = out_mux_en;
  Config.opamp2.opamp2_res_to_out_vdd = 0;
  Config.opamp2.opamp2_res_mux_sel    = 0;
  Config.opamp2.opamp2_en_res_bank    = 0;
  Config.opamp2.opamp2_r2_sel         = 0;
  Config.opamp2.opamp2_r1_sel         = 1;
  Config.opamp2.opamp2_lp_mode        = lp_mode;
  Config.opamp2.opamp2_enable         = enable;

  RSI_OPAMP2_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP3_UGB(uint8_t vin_p_sel,uint8_t enable,uint8_t lp_mode,uint8_t out_mux_en,
 *                                  uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configure the opamp3 in Unity gain buffer 
 * @param[in]   vin_p_sel   : input selection (0 - ULP_GPIO_10,1 - ULP_GPIO_14,2 -AUX_DAC_OUT,5 -OPAMP2_OUT)
 * @param[in]   enable      : opamp3 enable (1 - enable ,0 - disable)           
 * @param[in]   lp_mode     : opamp3 low power mode(1 - enable ,0 - disable)           
 * @param[in]   out_mux_en  : opamp3 output mux enable(GPIO27)(1 - enable ,0 - disable)           
 * @param[in]   dyn_en      : opamp3 dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel     : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP3_UGB(uint8_t vin_p_sel,
                    uint8_t enable,
                    uint8_t lp_mode,
                    uint8_t out_mux_en,
                    uint8_t dyn_en,
                    uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Config.opamp3.opamp3_dyn_en         = dyn_en;
  Config.opamp3.opamp3_sel_p_mux      = vin_p_sel;
  Config.opamp3.opamp3_sel_n_mux      = 3;
  Config.opamp3.opamp3_out_mux_en     = out_mux_en;
  Config.opamp3.opamp3_res_to_out_vdd = 0;
  Config.opamp3.opamp3_res_mux_sel    = 0;
  Config.opamp3.opamp3_en_res_bank    = 0;
  Config.opamp3.opamp3_r2_sel         = 0;
  Config.opamp3.opamp3_r1_sel         = 1;
  Config.opamp3.opamp3_lp_mode        = lp_mode;
  Config.opamp3.opamp3_enable         = enable;

  RSI_OPAMP3_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP1_TIA(uint8_t vref_sel,uint8_t iin_sel,uint8_t enable,uint8_t lp_mode,
 *                                  uint8_t r2_sel,uint8_t out_mux_en,uint8_t out_mux_sel,uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configure the opamp1 in Trans Impedance Amplifier 
 * @param[in]   vref_sel   : ref voltage gpio selection (0 - ULP_GPIO_12,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 -ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8,6 - AUX_DAC_OUT)
 * @param[in]   iin_sel    : input current gpio selection (0 - ULP_GPIO_12,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 -ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8)
 * @param[in]   r2_sel     : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   enable     : opamp1 enable (1 - enable ,0 - disable)           
 * @param[in]   lp_mode    : opamp1 low power mode(1 - enable ,0 - disable)           
 * @param[in]   out_mux_en : opamp1 output mux enable(1 - enable ,0 - disable)           
 * @param[in]   out_mux_sel: opamp1 output mux selection(0 - ULP_GPIO_4,1 - ULP_GPIO_15)           
 * @param[in]   dyn_en     : opamp1 dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel    : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP1_TIA(uint8_t vref_sel,
                    uint8_t iin_sel,
                    uint8_t enable,
                    uint8_t lp_mode,
                    uint8_t r2_sel,
                    uint8_t out_mux_en,
                    uint8_t out_mux_sel,
                    uint8_t dyn_en,
                    uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Config.opamp1.opamp1_dyn_en         = dyn_en;
  Config.opamp1.opamp1_sel_p_mux      = vref_sel;
  Config.opamp1.opamp1_sel_n_mux      = 3;
  Config.opamp1.opamp1_out_mux_en     = out_mux_en;
  Config.opamp1.opamp1_out_mux_sel    = out_mux_sel;
  Config.opamp1.opamp1_res_to_out_vdd = 0;
  Config.opamp1.opamp1_res_mux_sel    = iin_sel;
  Config.opamp1.opamp1_en_res_bank    = 1;
  Config.opamp1.opamp1_r2_sel         = r2_sel;
  Config.opamp1.opamp1_r1_sel         = 0;
  Config.opamp1.opamp1_lp_mode        = lp_mode;
  Config.opamp1.opamp1_enable         = enable;

  RSI_OPAMP1_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP2_TIA(uint8_t vref_sel,uint8_t iin_sel,uint8_t enable,uint8_t lp_mode,
 *                                  uint8_t r2_sel,uint8_t out_mux_en,uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configure the opamp2 in Trans Impedance Amplifier 
 * @param[in]   vref_sel   : ref voltage  gpio selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5,3 - AUX_DAC_OUT,6 - OPAMP1_OUT)
 * @param[in]   iin        : input current gpio selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5)
 * @param[in]   enable     : opamp2 enable (1 - enable ,0 - disable)           
 * @param[in]   lp_mode    : opamp2 low power mode(1 - enable ,0 - disable)          
 * @param[in]   r2_sel     : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm) 
 * @param[in]   out_mux_en : opamp2 output mux enable(1 -opamp2 output on ULP_GPIO_9)           
 * @param[in]   dyn_en     : opamp2 dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel    : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP2_TIA(uint8_t vref_sel,
                    uint8_t iin_sel,
                    uint8_t enable,
                    uint8_t lp_mode,
                    uint8_t r2_sel,
                    uint8_t out_mux_en,
                    uint8_t dyn_en,
                    uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Config.opamp2.opamp2_dyn_en         = dyn_en;
  Config.opamp2.opamp2_sel_p_mux      = vref_sel;
  Config.opamp2.opamp2_sel_n_mux      = 2;
  Config.opamp2.opamp2_out_mux_en     = out_mux_en;
  Config.opamp2.opamp2_res_to_out_vdd = 0;
  Config.opamp2.opamp2_res_mux_sel    = iin_sel;
  Config.opamp2.opamp2_en_res_bank    = 1;
  Config.opamp2.opamp2_r2_sel         = r2_sel;
  Config.opamp2.opamp2_r1_sel         = 0;
  Config.opamp2.opamp2_lp_mode        = lp_mode;
  Config.opamp2.opamp2_enable         = enable;

  RSI_OPAMP2_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP3_TIA(uint8_t vref_sel,uint8_t iin_sel,uint8_t enable,uint8_t lp_mode,
 *                                  uint8_t r2_sel,uint8_t out_mux_en,uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configure the opamp3 in Trans Impedance Amplifier 
 * @param[in]   vref_sel   : ref voltage gpio selection (0 - ULP_GPIO_10,1 - ULP_GPIO_14,2 - AUX_DAC_OUT,5 - OPAMP2_OUT)
 * @param[in]   iin        : input current gpio selection (0 - ULP_GPIO_10,1 - ULP_GPIO_14)
 * @param[in]   enable     : opamp3 enable (1 - enable ,0 - disable)           
 * @param[in]   lp_mode    : opamp3 low power mode(1 - enable ,0 - disable)           
 * @param[in]   r2_sel     : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en : opamp3 output mux enable(1 - enable ,0 - disable)           
 * @param[in]   out_mux_sel: opamp3 output mux selection(TGPIO2)           
 * @param[in]   dyn_en     : opamp3 dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel    : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP3_TIA(uint8_t vref_sel,
                    uint8_t iin_sel,
                    uint8_t enable,
                    uint8_t lp_mode,
                    uint8_t r2_sel,
                    uint8_t out_mux_en,
                    uint8_t dyn_en,
                    uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Config.opamp3.opamp3_dyn_en         = dyn_en;
  Config.opamp3.opamp3_sel_p_mux      = vref_sel;
  Config.opamp3.opamp3_sel_n_mux      = 2;
  Config.opamp3.opamp3_out_mux_en     = out_mux_en;
  Config.opamp3.opamp3_res_to_out_vdd = 0;
  Config.opamp3.opamp3_res_mux_sel    = iin_sel;
  Config.opamp3.opamp3_en_res_bank    = 1;
  Config.opamp3.opamp3_r2_sel         = r2_sel;
  Config.opamp3.opamp3_r1_sel         = 0;
  Config.opamp3.opamp3_lp_mode        = lp_mode;
  Config.opamp3.opamp3_enable         = enable;

  RSI_OPAMP3_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP1_InvPGA(uint8_t vref_sel,uint8_t vin_sel,uint8_t enable,uint8_t lp_mode,
 *                                   uint8_t r1_sel,uint8_t r2_sel,uint8_t out_mux_en,uint8_t out_mux_sel,uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configure the opamp1 in Inverting PGA 
 * @param[in]   vref_sel    : ref voltage gpio selection (0 - ULP_GPIO_12,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 -ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8,6 - AUX_DAC_OUT)
 * @param[in]   vin_sel     : input voltage gpio selection (0 - ULP_GPIO_12,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 -ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8,6 - AUX_ADC_DAC,7- GND)
 * @param[in]   enable      : opamp1 enable (1 - enable ,0 - disable)           
 * @param[in]   r1_sel      : selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   r2_sel      : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   lp_mode     : opamp1 low power mode(1 - enable ,0 - disable)           
 * @param[in]   out_mux_en  : opamp1 output mux enable(1 - enable ,0 - disable)           
 * @param[in]   out_mux_sel : opamp1 output mux selection(0 - ULP_GPIO_4,1 - ULP_GPIO_15)           
 * @param[in]   dyn_en      : opamp1 dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel     : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP1_InvPGA(uint8_t vref_sel,
                       uint8_t vin_sel,
                       uint8_t enable,
                       uint8_t lp_mode,
                       uint8_t r1_sel,
                       uint8_t r2_sel,
                       uint8_t out_mux_en,
                       uint8_t out_mux_sel,
                       uint8_t dyn_en,
                       uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Config.opamp1.opamp1_dyn_en         = dyn_en;
  Config.opamp1.opamp1_sel_p_mux      = vref_sel;
  Config.opamp1.opamp1_sel_n_mux      = 3;
  Config.opamp1.opamp1_out_mux_en     = out_mux_en;
  Config.opamp1.opamp1_out_mux_sel    = out_mux_sel;
  Config.opamp1.opamp1_res_to_out_vdd = 0;
  Config.opamp1.opamp1_res_mux_sel    = vin_sel;
  Config.opamp1.opamp1_en_res_bank    = 1;
  Config.opamp1.opamp1_r2_sel         = r2_sel;
  Config.opamp1.opamp1_r1_sel         = r1_sel;
  Config.opamp1.opamp1_lp_mode        = lp_mode;
  Config.opamp1.opamp1_enable         = enable;

  RSI_OPAMP1_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP2_InvPGA(uint8_t vref_sel,uint8_t vin_sel,uint8_t enable,uint8_t lp_mode,
 *                                     uint8_t r1_sel,uint8_t r2_sel,uint8_t out_mux_en,uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configure the opamp2 in Inverting PGA 
 * @param[in]   vref_sel  : ref voltage  gpio selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5,3 - AUX_DAC_OUT,6 - OPAMP1_OUT)
 * @param[in]   vin_sel   : input voltage gpio selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5,2 - AUX_DAC_OUT,4 - GND,5 - OPAMP1_OUT)
 * @param[in]   enable    : opamp2 enable (1 - enable ,0 - disable)           
 * @param[in]   lp_mode   : opamp2 low power mode(1 - enable ,0 - disable)           
 * @param[in]   r1_sel    : selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   r2_sel    : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en: opamp2 output mux enable(1 -opamp2 output on ULP_GPIO_9)           
 * @param[in]   dyn_en    : opamp2 dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel   : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP2_InvPGA(uint8_t vref_sel,
                       uint8_t vin_sel,
                       uint8_t enable,
                       uint8_t lp_mode,
                       uint8_t r1_sel,
                       uint8_t r2_sel,
                       uint8_t out_mux_en,
                       uint8_t dyn_en,
                       uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Config.opamp2.opamp2_dyn_en         = dyn_en;
  Config.opamp2.opamp2_sel_p_mux      = vref_sel;
  Config.opamp2.opamp2_sel_n_mux      = 2;
  Config.opamp2.opamp2_out_mux_en     = out_mux_en;
  Config.opamp2.opamp2_res_to_out_vdd = 0;
  Config.opamp2.opamp2_res_mux_sel    = vin_sel;
  Config.opamp2.opamp2_en_res_bank    = 1;
  Config.opamp2.opamp2_r2_sel         = r2_sel;
  Config.opamp2.opamp2_r1_sel         = r1_sel;
  Config.opamp2.opamp2_lp_mode        = lp_mode;
  Config.opamp2.opamp2_enable         = enable;

  RSI_OPAMP2_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP3_InvPGA(uint8_t vref_sel,uint8_t vin_sel,uint8_t enable,uint8_t lp_mode,
 *                                    uint8_t r1_sel,uint8_t r2_sel,uint8_t out_mux_en,uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configuration of an opamp3 in Inverting PGA
 * @param[in]   vref_sel  : ref voltage gpio selection (0 - ULP_GPIO_10,1 - ULP_GPIO_14,2 -AUX_DAC_OUT,5 -OPAMP2_OUT)
 * @param[in]   vin_sel   : input voltage gpio selection (0 - ULP_GPIO_10,1 - ULP_GPIO_14,2 - AUX_DAC_OUT,3 - GND,4 - OPAMP2_OUT)
 * @param[in]   enable    : opamp3 enable (1 - enable ,0 - disable)           
 * @param[in]   lp_mode   : opamp3 low power mode(1 - enable ,0 - disable)           
 * @param[in]   r1_sel    : selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   r2_sel    : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en: opamp3 output mux enable(1 - enable ,0 - disable output on GPIO27)           
 * @param[in]   dyn_en    : opamp3 dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel   : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP3_InvPGA(uint8_t vref_sel,
                       uint8_t vin_sel,
                       uint8_t enable,
                       uint8_t lp_mode,
                       uint8_t r1_sel,
                       uint8_t r2_sel,
                       uint8_t out_mux_en,
                       uint8_t dyn_en,
                       uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Config.opamp3.opamp3_dyn_en         = dyn_en;
  Config.opamp3.opamp3_sel_p_mux      = vref_sel;
  Config.opamp3.opamp3_sel_n_mux      = 2;
  Config.opamp3.opamp3_out_mux_en     = out_mux_en;
  Config.opamp3.opamp3_res_to_out_vdd = 0;
  Config.opamp3.opamp3_res_mux_sel    = vin_sel;
  Config.opamp3.opamp3_en_res_bank    = 1;
  Config.opamp3.opamp3_r2_sel         = r2_sel;
  Config.opamp3.opamp3_r1_sel         = r1_sel;
  Config.opamp3.opamp3_lp_mode        = lp_mode;
  Config.opamp3.opamp3_enable         = enable;

  RSI_OPAMP3_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP1_NonInvPGA(uint8_t vin_sel,uint8_t vref_sel,uint8_t enable,uint8_t lp_mode,
 *                                        uint8_t r1_sel,uint8_t r2_sel,uint8_t out_mux_en,uint8_t out_mux_sel,uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configure the opamp1 in Non inverting PGA
 * @param[in]   vin_sel    : input voltage gpio selection (0 - ULP_GPIO_12,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 -ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8,6 - AUX_ADC_DAC)
 * @param[in]   vref_sel   : ref voltage gpio selection (0 - ULP_GPIO_12,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 -ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8,6 - AUX_DAC_OUT,7 - GND)
 * @param[in]   enable     : opamp1 enable (1 - enable ,0 - disable)           
 * @param[in]   lp_mode    : opamp1 low power mode(1 - enable ,0 - disable)           
 * @param[in]   r1_sel     : selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   r2_sel     : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en : opamp1 output mux enable(1 - enable ,0 - disable)           
 * @param[in]   out_mux_sel: opamp1 output mux selection(0 - ULP_GPIO_4,1 - ULP_GPIO_15)           
 * @param[in]   dyn_en     : opamp1 dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel    : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP1_NonInvPGA(uint8_t vin_sel,
                          uint8_t vref_sel,
                          uint8_t enable,
                          uint8_t lp_mode,
                          uint8_t r1_sel,
                          uint8_t r2_sel,
                          uint8_t out_mux_en,
                          uint8_t out_mux_sel,
                          uint8_t dyn_en,
                          uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Config.opamp1.opamp1_dyn_en         = dyn_en;
  Config.opamp1.opamp1_sel_p_mux      = vin_sel;
  Config.opamp1.opamp1_sel_n_mux      = 3;
  Config.opamp1.opamp1_out_mux_en     = out_mux_en;
  Config.opamp1.opamp1_out_mux_sel    = out_mux_sel;
  Config.opamp1.opamp1_res_to_out_vdd = 0;
  Config.opamp1.opamp1_res_mux_sel    = vref_sel;
  Config.opamp1.opamp1_en_res_bank    = 1;
  Config.opamp1.opamp1_r2_sel         = r2_sel;
  Config.opamp1.opamp1_r1_sel         = r1_sel;
  Config.opamp1.opamp1_lp_mode        = lp_mode;
  Config.opamp1.opamp1_enable         = enable;

  RSI_OPAMP1_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP2_NonInvPGA(uint8_t vin_sel,uint8_t vref_sel,uint8_t enable,uint8_t lp_mode,
 *                                        uint8_t r1_sel,uint8_t r2_sel,uint8_t out_mux_en,uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configure the opamp2 in Non inverting PGA 
 * @param[in]   vin_sel    : input voltage gpio selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5,3 - AUX_DAC_OUT,6 - OPAMP1_OUT)
 * @param[in]   vref_sel   : ref voltage  gpio selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5,3 - AUX_DAC_OUT,4 - GND,5 - OPAMP1_OUT)
 * @param[in]   enable     : opamp2 enable (1 - enable ,0 - disable)           
 * @param[in]   lp_mode    : opamp2 low power mode(1 - enable ,0 - disable)           
 * @param[in]   r1_sel     : selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   r2_sel     : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en : opamp2 output mux enable(1 -opamp2 output on ULP_GPIO_9)           
 * @param[in]   dyn_en     : opamp2 dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel    : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP2_NonInvPGA(uint8_t vin_sel,
                          uint8_t vref_sel,
                          uint8_t enable,
                          uint8_t lp_mode,
                          uint8_t r1_sel,
                          uint8_t r2_sel,
                          uint8_t out_mux_en,
                          uint8_t dyn_en,
                          uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Config.opamp2.opamp2_dyn_en         = dyn_en;
  Config.opamp2.opamp2_sel_p_mux      = vin_sel;
  Config.opamp2.opamp2_sel_n_mux      = 2;
  Config.opamp2.opamp2_out_mux_en     = out_mux_en;
  Config.opamp2.opamp2_res_to_out_vdd = 0;
  Config.opamp2.opamp2_res_mux_sel    = vref_sel;
  Config.opamp2.opamp2_en_res_bank    = 1;
  Config.opamp2.opamp2_r2_sel         = r2_sel;
  Config.opamp2.opamp2_r1_sel         = r1_sel;
  Config.opamp2.opamp2_lp_mode        = lp_mode;
  Config.opamp2.opamp2_enable         = enable;

  RSI_OPAMP2_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP3_NonInvPGA(uint8_t vin_sel,uint8_t vref_sel,uint8_t enable,uint8_t lp_mode,
 *                                        uint8_t r1_sel,uint8_t r2_sel,uint8_t out_mux_en,uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configure the opamp3 in Non inverting PGA 
 * @param[in]   vin_sel   : input voltage gpio selection (0 - ULP_GPIO_10,1 - ULP_GPIO_14,2 - AUX_DAC_OUT,5 - OPAMP2_OUT)
 * @param[in]   vref_sel  : ref voltage gpio selection (0 - ULP_GPIO_10,1 - ULP_GPIO_14,2 -AUX_DAC_OUT,3 - GND,4 -OPAMP2_OUT)
 * @param[in]   enable    : opamp3 enable (1 - enable ,0 - disable)           
 * @param[in]   lp_mode   : opamp3 low power mode(1 - enable ,0 - disable)           
 * @param[in]   r1_sel    : selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   r2_sel    : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en: opamp3 output mux enable(1 - enable ,0 - disable output on GPIO27)           
 * @param[in]   dyn_en    : opamp3 dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel   : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP3_NonInvPGA(uint8_t vin_sel,
                          uint8_t vref_sel,
                          uint8_t enable,
                          uint8_t lp_mode,
                          uint8_t r1_sel,
                          uint8_t r2_sel,
                          uint8_t out_mux_en,
                          uint8_t dyn_en,
                          uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Config.opamp3.opamp3_dyn_en         = dyn_en;
  Config.opamp3.opamp3_sel_p_mux      = vin_sel;
  Config.opamp3.opamp3_sel_n_mux      = 2;
  Config.opamp3.opamp3_out_mux_en     = out_mux_en;
  Config.opamp3.opamp3_res_to_out_vdd = 0;
  Config.opamp3.opamp3_res_mux_sel    = vref_sel;
  Config.opamp3.opamp3_en_res_bank    = 1;
  Config.opamp3.opamp3_r2_sel         = r2_sel;
  Config.opamp3.opamp3_r1_sel         = r1_sel;
  Config.opamp3.opamp3_lp_mode        = lp_mode;
  Config.opamp3.opamp3_enable         = enable;

  RSI_OPAMP3_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP1_InvCMP(uint8_t vin_sel,uint8_t vref_sel,uint8_t enable,uint8_t lp_mode,
 *                                     uint8_t r1_sel,uint8_t r2_sel,uint8_t out_mux_en,uint8_t out_mux_sel,uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configuration of an opamp1 for inverting comparator with programmable hysteresis 
 * @param[in]   vin_sel   : input voltage gpio selection (0 - ULP_GPIO_12,1 - ULP_GPIO_7,2 -AUX_DAC_OUT)
 * @param[in]   vref_sel  : ref voltage gpio selection (0 - ULP_GPIO_12,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 -ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8,6 - AUX_DAC_OUT,7 - GND)
 * @param[in]   enable    : opamp1 enable (1 - enable ,0 - disable)           
 * @param[in]   lp_mode   : opamp1 low power mode(1 - enable ,0 - disable)           
 * @param[in]   r1_sel    : selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   r2_sel    : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en : opamp1 output mux enable(1 - enable ,0 - disable)           
 * @param[in]   out_mux_sel: opamp1 output mux selection(0 - ULP_GPIO_4,1 - ULP_GPIO_15)           
 * @param[in]   dyn_en     : opamp1 dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel    : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP1_InvCMP(uint8_t vin_sel,
                       uint8_t vref_sel,
                       uint8_t enable,
                       uint8_t lp_mode,
                       uint8_t r1_sel,
                       uint8_t r2_sel,
                       uint8_t out_mux_en,
                       uint8_t out_mux_sel,
                       uint8_t dyn_en,
                       uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Config.opamp1.opamp1_dyn_en         = dyn_en;
  Config.opamp1.opamp1_sel_p_mux      = 7;
  Config.opamp1.opamp1_sel_n_mux      = vin_sel;
  Config.opamp1.opamp1_out_mux_en     = out_mux_en;
  Config.opamp1.opamp1_out_mux_sel    = out_mux_sel;
  Config.opamp1.opamp1_res_to_out_vdd = 0;
  Config.opamp1.opamp1_res_mux_sel    = vref_sel;
  Config.opamp1.opamp1_en_res_bank    = 1;
  Config.opamp1.opamp1_r2_sel         = r2_sel;
  Config.opamp1.opamp1_r1_sel         = r1_sel;
  Config.opamp1.opamp1_lp_mode        = lp_mode;
  Config.opamp1.opamp1_enable         = enable;

  RSI_OPAMP1_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP2_InvCMP(uint8_t vin_sel,uint8_t vref_sel,uint8_t enable,uint8_t lp_mode,
 *                                     uint8_t r1_sel,uint8_t r2_sel,uint8_t out_mux_en,uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configuration of an opamp2 for inverting comparator with programmable hysteresis 
 * @param[in]   vin_sel   : input voltage selection (0 - ULP_GPIO_11,1 - AUX_DAC_OUT)
 * @param[in]   vref_sel  : reference voltage selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5,3 - AUX_DAC_OUT,4 - GND,5 - OPAMP1_OUT) 
 * @param[in]   enable    : opamp2 enable (1 - enable ,0 - disable)           
 * @param[in]   lp_mode   : opamp2 low power mode(1 - enable ,0 - disable)           
 * @param[in]   r1_sel    : selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   r2_sel    : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en: opamp2 output mux enable(1 -opamp2 output on ULP_GPIO_9)           
 * @param[in]   dyn_en    : opamp2 dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel   : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP2_InvCMP(uint8_t vin_sel,
                       uint8_t vref_sel,
                       uint8_t enable,
                       uint8_t lp_mode,
                       uint8_t r1_sel,
                       uint8_t r2_sel,
                       uint8_t out_mux_en,
                       uint8_t dyn_en,
                       uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Config.opamp2.opamp2_dyn_en         = dyn_en;
  Config.opamp2.opamp2_sel_p_mux      = 4;
  Config.opamp2.opamp2_sel_n_mux      = vin_sel;
  Config.opamp2.opamp2_out_mux_en     = out_mux_en;
  Config.opamp2.opamp2_res_to_out_vdd = 0;
  Config.opamp2.opamp2_res_mux_sel    = vref_sel;
  Config.opamp2.opamp2_en_res_bank    = 1;
  Config.opamp2.opamp2_r2_sel         = r2_sel;
  Config.opamp2.opamp2_r1_sel         = r1_sel;
  Config.opamp2.opamp2_lp_mode        = lp_mode;
  Config.opamp2.opamp2_enable         = enable;

  RSI_OPAMP2_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP3_InvCMP(uint8_t vin_sel,uint8_t vref_sel,uint8_t enable,uint8_t lp_mode,
 *                                     uint8_t r1_sel,uint8_t r2_sel,uint8_t out_mux_en,uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configuration of an opamp3 for inverting comparator with programmable hysteresis 
 * @param[in]   vin_sel   : input voltage gpio selection (0 - ULP_GPIO_10,1 - AUX_DAC_OUT)
 * @param[in]   vref_sel  : ref voltage gpio selection (0 - ULP_GPIO_10,1 - ULP_GPIO_14,2 -AUX_DAC_OUT,3 - GND,4 -OPAMP2_OUT) 
 * @param[in]   enable    : opamp3 enable (1 - enable ,0 - disable)           
 * @param[in]   lp_mode   : opamp3 low power mode(1 - enable ,0 - disable)           
 * @param[in]   r1_sel    : selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   r2_sel    : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en: opamp3 output mux enable(1 - enable ,0 - disable output on GPIO27)            
 * @param[in]   dyn_en    : opamp3 dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel   : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP3_InvCMP(uint8_t vin_sel,
                       uint8_t vref_sel,
                       uint8_t enable,
                       uint8_t lp_mode,
                       uint8_t r1_sel,
                       uint8_t r2_sel,
                       uint8_t out_mux_en,
                       uint8_t dyn_en,
                       uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Config.opamp3.opamp3_dyn_en         = dyn_en;
  Config.opamp3.opamp3_sel_p_mux      = 3;
  Config.opamp3.opamp3_sel_n_mux      = vin_sel;
  Config.opamp3.opamp3_out_mux_en     = out_mux_en;
  Config.opamp3.opamp3_res_to_out_vdd = 0;
  Config.opamp3.opamp3_res_mux_sel    = vref_sel;
  Config.opamp3.opamp3_en_res_bank    = 1;
  Config.opamp3.opamp3_r2_sel         = r2_sel;
  Config.opamp3.opamp3_r1_sel         = r1_sel;
  Config.opamp3.opamp3_lp_mode        = lp_mode;
  Config.opamp3.opamp3_enable         = enable;

  RSI_OPAMP3_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP1_NonInvCMP(uint8_t vin_sel,uint8_t vref_sel,uint8_t enable,uint8_t lp_mode,
 *                                        uint8_t r1_sel,uint8_t r2_sel,uint8_t out_mux_en,uint8_t out_mux_sel,uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configuration of an opamp1 for inverting comparator with programmable hysteresis 
 * @param[in]   vin_sel    : input voltage gpio selection (0 - ULP_GPIO_12,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 - ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8,6 - AUX_DAC_OUT)
 * @param[in]   vref_sel   : ref voltage gpio selection (0 - ULP_GPIO_12,1 - ULP_GPIO_7,2 - AUX_DAC_OUT)
 * @param[in]   enable     : opamp1 enable (1 - enable ,0 - disable)           
 * @param[in]   lp_mode    : opamp1 low power mode(1 - enable ,0 - disable)           
 * @param[in]   r1_sel     : selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   r2_sel     : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en : opamp1 output mux enable(1 - enable ,0 - disable)           
 * @param[in]   out_mux_sel: opamp1 output mux selection(0 - ULP_GPIO_4,1 - ULP_GPIO_15)           
 * @param[in]   dyn_en     : opamp1 dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel    : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP1_NonInvCMP(uint8_t vin_sel,
                          uint8_t vref_sel,
                          uint8_t enable,
                          uint8_t lp_mode,
                          uint8_t r1_sel,
                          uint8_t r2_sel,
                          uint8_t out_mux_en,
                          uint8_t out_mux_sel,
                          uint8_t dyn_en,
                          uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Config.opamp1.opamp1_dyn_en         = dyn_en;
  Config.opamp1.opamp1_sel_p_mux      = 7;
  Config.opamp1.opamp1_sel_n_mux      = vref_sel;
  Config.opamp1.opamp1_out_mux_en     = out_mux_en;
  Config.opamp1.opamp1_out_mux_sel    = out_mux_sel;
  Config.opamp1.opamp1_res_to_out_vdd = 0;
  Config.opamp1.opamp1_res_mux_sel    = vin_sel;
  Config.opamp1.opamp1_en_res_bank    = 1;
  Config.opamp1.opamp1_r2_sel         = r2_sel;
  Config.opamp1.opamp1_r1_sel         = r1_sel;
  Config.opamp1.opamp1_lp_mode        = lp_mode;
  Config.opamp1.opamp1_enable         = enable;

  RSI_OPAMP1_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP2_NonInvCMP(uint8_t vin_sel,uint8_t vref_sel,uint8_t enable,uint8_t lp_mode,
 *                                uint8_t r1_sel,uint8_t r2_sel,uint8_t out_mux_en,uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configuration of an opamp2 for inverting comparator with programmable hysteresis 
 * @param[in]   vin_sel   : input voltae gpio selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5,3 - AUX_DAC_OUT, 5 - OPAMP1_OUT)
 * @param[in]   vref_sel  : ref voltage gpio selection (0 - ULP_GPIO_11,1 - AUX_DAC_OUT) 
 * @param[in]   enable    : opamp2 enable (1 - enable ,0 - disable)           
 * @param[in]   lp_mode   : opamp2 low power mode(1 - enable ,0 - disable)           
 * @param[in]   r1_sel    : selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   r2_sel    : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en: opamp2 output mux enable(1 -opamp2 output on ULP_GPIO_9)           
 * @param[in]   dyn_en    : opamp2 dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel   : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP2_NonInvCMP(uint8_t vin_sel,
                          uint8_t vref_sel,
                          uint8_t enable,
                          uint8_t lp_mode,
                          uint8_t r1_sel,
                          uint8_t r2_sel,
                          uint8_t out_mux_en,
                          uint8_t dyn_en,
                          uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Config.opamp2.opamp2_dyn_en         = dyn_en;
  Config.opamp2.opamp2_sel_p_mux      = 4;
  Config.opamp2.opamp2_sel_n_mux      = vref_sel;
  Config.opamp2.opamp2_out_mux_en     = out_mux_en;
  Config.opamp2.opamp2_res_to_out_vdd = 0;
  Config.opamp2.opamp2_res_mux_sel    = vin_sel;
  Config.opamp2.opamp2_en_res_bank    = 1;
  Config.opamp2.opamp2_r2_sel         = r2_sel;
  Config.opamp2.opamp2_r1_sel         = r1_sel;
  Config.opamp2.opamp2_lp_mode        = lp_mode;
  Config.opamp2.opamp2_enable         = enable;

  RSI_OPAMP2_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP3_NonInvCMP(uint8_t vin_sel,uint8_t vref_sel,uint8_t enable,uint8_t lp_mode,
 *                                        uint8_t r1_sel,uint8_t r2_sel,uint8_t out_mux_en,uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configuration of an opamp3 for inverting comparator with programmable hysteresis 
 * @param[in]   vin_sel    : input voltage gpio selection (0 - ULP_GPIO_10,1 - ULP_GPIO_14,2 - AUX_DAC_OUT,4 - OPAMP2_OUT)
 * @param[in]   vref_sel   : ref voltage gpio selection (0 - ULP_GPIO_10,1 - AUX_DAC_OUT)
 * @param[in]   enable     : opamp3 enable (1 - enable ,0 - disable)           
 * @param[in]   lp_mode    : opamp3 low power mode(1 - enable ,0 - disable)           
 * @param[in]   r1_sel     : selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   r2_sel     : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en : opamp3 output mux enable(1 - enable ,0 - disable)           
 * @param[in]   out_mux_sel: opamp3 output mux selection(TGPIO2)           
 * @param[in]   dyn_en     : opamp3 dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel    : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP3_NonInvCMP(uint8_t vin_sel,
                          uint8_t vref_sel,
                          uint8_t enable,
                          uint8_t lp_mode,
                          uint8_t r1_sel,
                          uint8_t r2_sel,
                          uint8_t out_mux_en,
                          uint8_t dyn_en,
                          uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Config.opamp3.opamp3_dyn_en         = dyn_en;
  Config.opamp3.opamp3_sel_p_mux      = 3;
  Config.opamp3.opamp3_sel_n_mux      = vref_sel;
  Config.opamp3.opamp3_out_mux_en     = out_mux_en;
  Config.opamp3.opamp3_res_to_out_vdd = 0;
  Config.opamp3.opamp3_res_mux_sel    = vin_sel;
  Config.opamp3.opamp3_en_res_bank    = 1;
  Config.opamp3.opamp3_r2_sel         = r2_sel;
  Config.opamp3.opamp3_r1_sel         = r1_sel;
  Config.opamp3.opamp3_lp_mode        = lp_mode;
  Config.opamp3.opamp3_enable         = enable;

  RSI_OPAMP3_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP_InstrAMP(uint8_t vin_p_sel,uint8_t vin_n_sel,uint8_t enable,uint8_t lp_mode,
 *                                      uint8_t r1_sel,uint8_t r2_sel,uint8_t out_mux_en,uint8_t dyn_en,uint8_t channel)
 * @brief       This API is used to configuration of opamp as instrumentation amplifier
 * @param[in]   vin_p_sel   : input selection (0 - ULP_GPIO_12,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 -ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8)
 * @param[in]   vin_n_sel   : input selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5)
 * @param[in]   enable      : opamp1 enable (1 - enable ,0 - disable)           
 * @param[in]   lp_mode     : opamp1 low power mode(1 - enable ,0 - disable)           
 * @param[in]   r1_sel     : selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   r2_sel     : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm) 
 * @param[in]   out_mux_en  : opamp output mux enable(1 - enable ,0 - disable output on GPIO27)           
 * @param[in]   dyn_en      : opamp dynamic mode enable(1 - enable ,0 - disable)                       
 * @param[in]   channel     : channel no to set in dynamic mode           
 * @return      none
 */
void RSI_OPAMP_InstrAMP(uint8_t vin_p_sel,
                        uint8_t vin_n_sel,
                        uint8_t enable,
                        uint8_t lp_mode,
                        uint8_t r1_sel,
                        uint8_t r2_sel,
                        uint8_t out_mux_en,
                        uint8_t dyn_en,
                        uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Config.opamp1.opamp1_dyn_en         = dyn_en;
  Config.opamp1.opamp1_sel_p_mux      = vin_p_sel;
  Config.opamp1.opamp1_sel_n_mux      = 4;
  Config.opamp1.opamp1_out_mux_en     = 0;
  Config.opamp1.opamp1_out_mux_sel    = 0;
  Config.opamp1.opamp1_res_to_out_vdd = 0;
  Config.opamp1.opamp1_res_mux_sel    = 0;
  Config.opamp1.opamp1_en_res_bank    = 0;
  Config.opamp1.opamp1_r2_sel         = 0;
  Config.opamp1.opamp1_r1_sel         = 1;
  Config.opamp1.opamp1_lp_mode        = lp_mode;
  Config.opamp1.opamp1_enable         = enable;

  Config.opamp2.opamp2_dyn_en         = dyn_en;
  Config.opamp2.opamp2_sel_p_mux      = vin_n_sel;
  Config.opamp2.opamp2_sel_n_mux      = 3;
  Config.opamp2.opamp2_out_mux_en     = 0;
  Config.opamp2.opamp2_res_to_out_vdd = 2;
  Config.opamp2.opamp2_res_mux_sel    = 5;
  Config.opamp2.opamp2_en_res_bank    = 1;
  Config.opamp2.opamp2_r2_sel         = r2_sel;
  Config.opamp2.opamp2_r1_sel         = r1_sel;
  Config.opamp2.opamp2_lp_mode        = lp_mode;
  Config.opamp2.opamp2_enable         = enable;

  Config.opamp3.opamp3_dyn_en         = dyn_en;
  Config.opamp3.opamp3_sel_p_mux      = 6;
  Config.opamp3.opamp3_sel_n_mux      = 2;
  Config.opamp3.opamp3_out_mux_en     = out_mux_en;
  Config.opamp3.opamp3_res_to_out_vdd = 0;
  Config.opamp3.opamp3_res_mux_sel    = 4;
  Config.opamp3.opamp3_en_res_bank    = 1;
  Config.opamp3.opamp3_r2_sel         = r2_sel;
  Config.opamp3.opamp3_r1_sel         = r1_sel;
  Config.opamp3.opamp3_lp_mode        = lp_mode;
  Config.opamp3.opamp3_enable         = enable;

  RSI_OPAMP1_Config(OPAMP, channel, &Config);
  RSI_OPAMP2_Config(OPAMP, channel, &Config);
  RSI_OPAMP3_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP_Instrumentation_Amplifier(uint32_t channel, const OPAMP_CONFIG_T *Config)
 * @brief       This API is used to configure all three OPAMPs (OPAMP1, OPAMP2, and OPAMP3) for instrumentation amplifier operation
 * @param[in]   channel : channel number to set in dynamic mode           
 * @param[in]   Config  : pointer to OPAMP configuration structure containing settings for all three OPAMPs                       
 * @return      none
 */
void RSI_OPAMP_Instrumentation_Amplifier(uint32_t channel, const OPAMP_CONFIG_T *Config)
{
  RSI_OPAMP1_Config(OPAMP, channel, Config);
  RSI_OPAMP2_Config(OPAMP, channel, Config);
  RSI_OPAMP3_Config(OPAMP, channel, Config);
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP1_Config(OPAMP_type *Opamp, uint32_t channel, const OPAMP_CONFIG_T *config)
 * @brief       This API is used to configuration of an opamp1 
 * @param[in]   Opamp   : pointer to opamp
 * @param[in]   channel : channel no to set in dynamic mode           
 * @param[in]   config  : pointer to opamp configuration                       
 * @return      none
 */
void RSI_OPAMP1_Config(OPAMP_Type *Opamp, uint32_t channel, const OPAMP_CONFIG_T *config)
{

  if (config->opamp1.opamp1_dyn_en) {
    Opamp->OPAMP_1_b.OPAMP1_DYN_EN = 1;
    AUX_ADC_DAC_COMP->ADC_CH_BIT_MAP_CONFIG[channel].ADC_CH_BIT_MAP_CONFIG_1_b.CHANNEL_BITMAP =
      (uint32_t)(config->opamp1.opamp1_out_mux_sel << 29 | config->opamp1.opamp1_sel_p_mux << 25
                 | config->opamp1.opamp1_sel_n_mux << 22 | config->opamp1.opamp1_out_mux_en << 21
                 | config->opamp1.opamp1_res_to_out_vdd << 20 | config->opamp1.opamp1_res_mux_sel << 17
                 | config->opamp1.opamp1_en_res_bank << 16 | config->opamp1.opamp1_r2_sel << 13
                 | config->opamp1.opamp1_r1_sel << 11 | config->opamp1.opamp1_lp_mode << 10
                 | config->opamp1.opamp1_enable << 9);
  } else {
    Opamp->OPAMP_1_b.OPAMP1_EN_RES_BANK    = config->opamp1.opamp1_en_res_bank;
    Opamp->OPAMP_1_b.OPAMP1_INN_SEL        = config->opamp1.opamp1_sel_n_mux;
    Opamp->OPAMP_1_b.OPAMP1_INP_SEL        = config->opamp1.opamp1_sel_p_mux;
    Opamp->OPAMP_1_b.OPAMP1_LP_MODE        = config->opamp1.opamp1_lp_mode;
    Opamp->OPAMP_1_b.OPAMP1_R1_SEL         = config->opamp1.opamp1_r1_sel;
    Opamp->OPAMP_1_b.OPAMP1_R2_SEL         = config->opamp1.opamp1_r2_sel;
    Opamp->OPAMP_1_b.OPAMP1_RES_MUX_SEL    = config->opamp1.opamp1_res_mux_sel;
    Opamp->OPAMP_1_b.OPAMP1_RES_TO_OUT_VDD = config->opamp1.opamp1_res_to_out_vdd;
    Opamp->OPAMP_1_b.OPAMP1_OUT_MUX_EN     = config->opamp1.opamp1_out_mux_en;
    Opamp->OPAMP_1_b.OPAMP1_OUT_MUX_SEL    = config->opamp1.opamp1_out_mux_sel;
    Opamp->OPAMP_1_b.OPAMP1_ENABLE         = config->opamp1.opamp1_enable;
  }
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP2_Config(OPAMP_Type *Opamp, uint32_t channel, const OPAMP_CONFIG_T *config)
 * @brief       This API is used to configuration of an opamp2 
 * @param[in]   Opamp   : pointer to opamp
 * @param[in]   channel : channel no  to set in dynamic mode           
 * @param[in]   config  : pointer to opamp configuration                        
 * @return      none
 */
void RSI_OPAMP2_Config(OPAMP_Type *Opamp, uint32_t channel, const OPAMP_CONFIG_T *config)
{

  if (config->opamp2.opamp2_dyn_en) {
    Opamp->OPAMP_2_b.OPAMP2_DYN_EN = 1;
    AUX_ADC_DAC_COMP->ADC_CH_BIT_MAP_CONFIG[channel].ADC_CH_BIT_MAP_CONFIG_1_b.CHANNEL_BITMAP =
      (uint32_t)(config->opamp2.opamp2_lp_mode << 31 | config->opamp2.opamp2_enable << 30);
    AUX_ADC_DAC_COMP->ADC_CH_BIT_MAP_CONFIG[channel].ADC_CH_BIT_MAP_CONFIG_2_b.CHANNEL_BITMAP =
      (uint32_t)(config->opamp2.opamp2_sel_p_mux << 14 | config->opamp2.opamp2_sel_n_mux << 12
                 | config->opamp2.opamp2_out_mux_en << 11 | config->opamp2.opamp2_res_to_out_vdd << 9
                 | config->opamp2.opamp2_res_mux_sel << 6 | config->opamp2.opamp2_en_res_bank << 5
                 | config->opamp2.opamp2_r2_sel << 2 | config->opamp2.opamp2_r1_sel << 0);
  } else {
    Opamp->OPAMP_2_b.OPAMP2_EN_RES_BANK    = config->opamp2.opamp2_en_res_bank;
    Opamp->OPAMP_2_b.OPAMP2_INN_SEL        = config->opamp2.opamp2_sel_n_mux;
    Opamp->OPAMP_2_b.OPAMP2_INP_SEL        = config->opamp2.opamp2_sel_p_mux;
    Opamp->OPAMP_2_b.OPAMP2_LP_MODE        = config->opamp2.opamp2_lp_mode;
    Opamp->OPAMP_2_b.OPAMP2_R1_SEL         = config->opamp2.opamp2_r1_sel;
    Opamp->OPAMP_2_b.OPAMP2_R2_SEL         = config->opamp2.opamp2_r2_sel;
    Opamp->OPAMP_2_b.OPAMP2_RES_MUX_SEL    = config->opamp2.opamp2_res_mux_sel;
    Opamp->OPAMP_2_b.OPAMP2_RES_TO_OUT_VDD = config->opamp2.opamp2_res_to_out_vdd;
    Opamp->OPAMP_2_b.OPAMP2_OUT_MUX_EN     = config->opamp2.opamp2_out_mux_en;
    Opamp->OPAMP_2_b.OPAMP2_ENABLE         = config->opamp2.opamp2_enable;
  }
}

/*==============================================*/
/**   
 * @fn          void RSI_OPAMP3_Config(OPAMP_Type *Opamp, uint32_t channel, const OPAMP_CONFIG_T *config)
 * @brief       This API is used to configuration of an opamp3 
 * @param[in]   Opamp   : pointer to opamp
 * @param[in]   channel : channel no to set in dynamic mode           
 * @param[in]   config  : pointer to opamp configuration                        
 * @return      none
 */
void RSI_OPAMP3_Config(OPAMP_Type *Opamp, uint32_t channel, const OPAMP_CONFIG_T *config)
{
  if (config->opamp3.opamp3_dyn_en) {
    Opamp->OPAMP_3_b.OPAMP3_DYN_EN = 1;
    AUX_ADC_DAC_COMP->ADC_CH_BIT_MAP_CONFIG[channel].ADC_CH_BIT_MAP_CONFIG_3_b.CHANNEL_BITMAP =
      (unsigned int)((config->opamp3.opamp3_sel_p_mux) & 0x1F);
    AUX_ADC_DAC_COMP->ADC_CH_BIT_MAP_CONFIG[channel].ADC_CH_BIT_MAP_CONFIG_2_b.CHANNEL_BITMAP =
      (uint32_t)(config->opamp3.opamp3_sel_n_mux << 30 | config->opamp3.opamp3_out_mux_en << 29
                 | config->opamp3.opamp3_res_to_out_vdd << 28 | config->opamp3.opamp3_res_mux_sel << 25
                 | config->opamp3.opamp3_en_res_bank << 24 | config->opamp3.opamp3_r2_sel << 21
                 | config->opamp3.opamp3_r1_sel << 19 | config->opamp3.opamp3_lp_mode << 18
                 | config->opamp3.opamp3_enable << 17);

  } else {
    Opamp->OPAMP_3_b.OPAMP3_EN_RES_BANK    = config->opamp3.opamp3_en_res_bank;
    Opamp->OPAMP_3_b.OPAMP3_INN_SEL        = config->opamp3.opamp3_sel_n_mux;
    Opamp->OPAMP_3_b.OPAMP3_INP_SEL        = config->opamp3.opamp3_sel_p_mux;
    Opamp->OPAMP_3_b.OPAMP3_LP_MODE        = config->opamp3.opamp3_lp_mode;
    Opamp->OPAMP_3_b.OPAMP3_R1_SEL         = config->opamp3.opamp3_r1_sel;
    Opamp->OPAMP_3_b.OPAMP3_R2_SEL         = config->opamp3.opamp3_r2_sel;
    Opamp->OPAMP_3_b.OPAMP3_RES_MUX_SEL    = config->opamp3.opamp3_res_mux_sel;
    Opamp->OPAMP_3_b.OPAMP3_RES_TO_OUT_VDD = config->opamp3.opamp3_res_to_out_vdd;
    Opamp->OPAMP_3_b.OPAMP3_OUT_MUX_EN     = config->opamp3.opamp3_out_mux_en;
    Opamp->OPAMP_3_b.OPAMP3_ENABLE         = config->opamp3.opamp3_enable;
  }
}

/*==============================================*/
/**   
 * @fn          static void rsi_configure_OPAMP1(OPAMP_CONFIG_T *Config, uint8_t vin_p_sel, uint8_t vin_n_sel, uint8_t enable, uint8_t lp_mode, uint8_t out_mux_en, uint8_t out_mux_sel,uint8_t r1_sel, uint8_t r2_sel,uint8_t en_res_bank, uint8_t dyn_en)
 * @brief       This function consolidates repetitive configuration code for OPAMP1.
 * @param[in]   Config      : Pointer to OPAMP configuration structure.
 * @param[in]   vin_p_sel   : Input selection for positive (0 - GPIO_27,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 -ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8,6 - AUX_DAC_OUT, 7 - Resistor Tap)
 * @param[in]   vin_n_sel   : Input selection for negative (0 - GPIO_27,1 - ULP_GPIO_7,2 - AUX_DAC_OUT,3 - Resistor Tap,4 - Out )
 * @param[in]   vref_sel    : Input Resistor mux selection (0 - GPIO_27,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 -ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8,6 - AUX_DAC_OUT)
 * @param[in]   enable      : OPAMP enable (1 - enable ,0 - disable)
 * @param[in]   lp_mode     : Low power mode (1 - enable ,0 - disable)
 * @param[in]   r1_sel      : selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   r2_sel      : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm
 * @param[in]   out_mux_en  : Output mux enable (1 - enable ,0 - disable)
 * @param[in]   out_mux_sel : Output mux selection(0 - ULP_GPIO_4,1 - GPIO_30)
 * @param[in]   dyn_en      : dynamic mode enable(1 - enable ,0 - disable)
 * @param[in]   channel     : channel no to set in dynamic mode
 * @return      none
 */
static void rsi_configure_OPAMP1(OPAMP_CONFIG_T *Config,
                                 const Configure_OPAMP_t *config_opamp1,
                                 uint8_t en_res_bank,
                                 uint8_t dyn_en)
{
  Config->opamp1.opamp1_dyn_en         = dyn_en;
  Config->opamp1.opamp1_sel_p_mux      = config_opamp1->vin_p_sel;
  Config->opamp1.opamp1_sel_n_mux      = config_opamp1->vin_n_sel;
  Config->opamp1.opamp1_out_mux_en     = config_opamp1->out_mux_en;
  Config->opamp1.opamp1_out_mux_sel    = config_opamp1->out_mux_sel;
  Config->opamp1.opamp1_res_to_out_vdd = 0;
  Config->opamp1.opamp1_res_mux_sel    = config_opamp1->vref_sel;
  Config->opamp1.opamp1_en_res_bank    = en_res_bank;
  Config->opamp1.opamp1_r2_sel         = config_opamp1->r2_sel;
  Config->opamp1.opamp1_r1_sel         = config_opamp1->r1_sel;
  Config->opamp1.opamp1_lp_mode        = config_opamp1->lp_mode;
  Config->opamp1.opamp1_enable         = config_opamp1->enable;
}
/*==============================================*/
/**
 * @fn          static void rsi_configure_OPAMP2(OPAMP_CONFIG_T *Config, uint8_t vin_p_sel, uint8_t vin_n_sel, uint8_t enable, uint8_t lp_mode, uint8_t out_mux_en,uint8_t r1_sel, uint8_t r2_sel,uint8_t en_res_bank, uint8_t dyn_en)
 * @brief       This function consolidates repetitive configuration code for OPAMP2.
 * @param[in]   Config      : Pointer to OPAMP configuration structure.
 * @param[in]   vin_p_sel   : Input selection for positive (0 - ULP_GPIO_11,1 - ULP_GPIO_5, 3 - AUX_DAC_OUT, 4 - Resistor tap, 6 - OPAMP1_out).
 * @param[in]   vin_n_sel   : Input selection for negative (0 - ULP_GPIO_11,1 - AUX_DAC_OUT, 2 - Resistor tap, 3 - Out)
 * @param[in]   vref_sel    : Input Resistor mux selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5, 3 - AUX_DAC_OUT, 5 - OPAMP1_out )
 * @param[in]   enable      : OPAMP enable (1 - enable ,0 - disable)
 * @param[in]   lp_mode     : Low power mode (1 - enable ,0 - disable)
 * @param[in]   r1_sel      : selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   r2_sel      : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en  : Output mux enable(1 -opamp2 output on ULP_GPIO_9)
 * @param[in]   dyn_en      : dynamic mode enable(1 - enable ,0 - disable)
 * @param[in]   channel     : channel no to set in dynamic mode
 * @return      none
 */
static void rsi_configure_OPAMP2(OPAMP_CONFIG_T *Config,
                                 const Configure_OPAMP_t *config_opamp2,
                                 uint8_t en_res_bank,
                                 uint8_t dyn_en)
{
  Config->opamp2.opamp2_dyn_en         = dyn_en;
  Config->opamp2.opamp2_sel_p_mux      = config_opamp2->vin_p_sel;
  Config->opamp2.opamp2_sel_n_mux      = config_opamp2->vin_n_sel;
  Config->opamp2.opamp2_out_mux_en     = config_opamp2->out_mux_en;
  Config->opamp2.opamp2_res_to_out_vdd = 0;
  Config->opamp2.opamp2_res_mux_sel    = config_opamp2->vref_sel;
  Config->opamp2.opamp2_en_res_bank    = en_res_bank;
  Config->opamp2.opamp2_r2_sel         = config_opamp2->r2_sel;
  Config->opamp2.opamp2_r1_sel         = config_opamp2->r1_sel;
  Config->opamp2.opamp2_lp_mode        = config_opamp2->lp_mode;
  Config->opamp2.opamp2_enable         = config_opamp2->enable;
}
/*==============================================*/
/**
 * @fn          static void rsi_configure_OPAMP3(OPAMP_CONFIG_T *Config, uint8_t vin_p_sel, uint8_t vin_n_sel, uint8_t enable, uint8_t lp_mode, uint8_t out_mux_en, uint8_t out_mux_sel,uint8_t r1_sel, uint8_t r2_sel, uint8_t dyn_en)
 * @brief       This function consolidates repetitive configuration code for OPAMP3.
 * @param[in]   Config      : Pointer to OPAMP configuration structure.
 * @param[in]   vin_p_sel   : Input selection for positive (0 - ULP_GPIO_10,1 - GPIO_29,2 - AUX_DAC_OUT, 3 - Resistor tap,5 - OPAMP2_OUT)
 * @param[in]   vin_n_sel   : Input selection for negative (0 - ULP_GPIO_10,1 - AUX_DAC_OUT, 2 - Resistor tap, 3- Out)
 * @param[in]   vref_sel    : Input Resistor mux selection (0 - ULP_GPIO_10,1 - GPIO_29,2 - AUX_DAC_OUT, 4 - OPAMP2_out)
 * @param[in]   enable      : OPAMP enable (1 - enable ,0 - disable)
 * @param[in]   lp_mode     : Low power mode (1 - enable ,0 - disable)
 * @param[in]   r1_sel      : selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   r2_sel      : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm
 * @param[in]   out_mux_en  : Output mux enable (GPIO27)(1 - enable ,0 - disable)
 * @param[in]   dyn_en      : dynamic mode enable(1 - enable ,0 - disable)
 * @param[in]   channel     : channel no to set in dynamic mode
 * @return      none
 */
static void rsi_configure_OPAMP3(OPAMP_CONFIG_T *Config,
                                 const Configure_OPAMP_t *config_opamp3,
                                 uint8_t en_res_bank,
                                 uint8_t dyn_en)
{
  Config->opamp3.opamp3_dyn_en         = dyn_en;
  Config->opamp3.opamp3_sel_p_mux      = config_opamp3->vin_p_sel;
  Config->opamp3.opamp3_sel_n_mux      = config_opamp3->vin_n_sel;
  Config->opamp3.opamp3_out_mux_en     = config_opamp3->out_mux_en;
  Config->opamp3.opamp3_res_to_out_vdd = 0;
  Config->opamp3.opamp3_res_mux_sel    = config_opamp3->vref_sel;
  Config->opamp3.opamp3_en_res_bank    = en_res_bank;
  Config->opamp3.opamp3_r2_sel         = config_opamp3->r2_sel;
  Config->opamp3.opamp3_r1_sel         = config_opamp3->r1_sel;
  Config->opamp3.opamp3_lp_mode        = config_opamp3->lp_mode;
  Config->opamp3.opamp3_enable         = config_opamp3->enable;
}
/*==============================================*/
/**
 * @fn          void RSI_OPAMP_UGB_V2(uint8_t opamp_instance, const Configure_OPAMP_t *params, uint8_t dyn_en, uint8_t channel)
 * @brief       This API is used to configure the opamp1 in Unity gain buffer
 * @param[in]   out_mux_sel : opamp1 output mux selection(0 - ULP_GPIO_4,1 - GPIO_30)
 * @param[in]   dyn_en      : opamp1 dynamic mode enable(1 - enable ,0 - disable)
 * @param[in]   channel     : channel no to set in dynamic mode
 * @return      none
 */
void RSI_OPAMP_UGB_V2(uint8_t opamp_instance, const Configure_OPAMP_t *params, uint8_t dyn_en, uint8_t channel)
{

  OPAMP_CONFIG_T Config;
  if (opamp_instance == OPAMP1_INSTANCE) {
    rsi_configure_OPAMP1(&Config, params, 0, dyn_en);
    RSI_OPAMP1_Config(OPAMP, channel, &Config);
  } else if (opamp_instance == OPAMP2_INSTANCE) {
    rsi_configure_OPAMP2(&Config, params, 0, dyn_en);
    RSI_OPAMP2_Config(OPAMP, channel, &Config);
  } else if (opamp_instance == OPAMP3_INSTANCE) {
    rsi_configure_OPAMP3(&Config, params, 0, dyn_en);
    RSI_OPAMP3_Config(OPAMP, channel, &Config);
  } else {
    // Invalid opamp number
  }
}

/*==============================================*/
/**
 * @fn          void RSI_OPAMP_TIA_V2(uint8_t opamp_instance,const Configure_OPAMP_t *params, uint8_t dyn_en, uint8_t channel)
 * @brief       This API is used to configure the opamp in Trans Impedance Amplifier
 * @param[in]   dyn_en     : opamp1 dynamic mode enable(1 - enable ,0 - disable)
 * @param[in]   channel    : channel no to set in dynamic mode
 * @return      none
 */
void RSI_OPAMP_TIA_V2(uint8_t opamp_instance, const Configure_OPAMP_t *params, uint8_t dyn_en, uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  if (opamp_instance == OPAMP1_INSTANCE) {
    rsi_configure_OPAMP1(&Config, params, 1, dyn_en);
    RSI_OPAMP1_Config(OPAMP, channel, &Config);
  } else if (opamp_instance == OPAMP2_INSTANCE) {
    rsi_configure_OPAMP2(&Config, params, 1, dyn_en);
    RSI_OPAMP2_Config(OPAMP, channel, &Config);
  } else if (opamp_instance == OPAMP3_INSTANCE) {
    rsi_configure_OPAMP3(&Config, params, 1, dyn_en);
    RSI_OPAMP3_Config(OPAMP, channel, &Config);
  } else {
    // Invalid opamp number
  }
}

/*==============================================*/
/**
 * @fn          void RSI_OPAMP1_InvPGA_v2(const Configure_OPAMP1_t *params, uint8_t dyn_en, uint8_t channel)
 * @brief       This API is used to configure the opamp1 in Inverting PGA
 * @param[in]   dyn_en      : opamp1 dynamic mode enable(1 - enable ,0 - disable)
 * @param[in]   channel     : channel no to set in dynamic mode
 * @return      none
 */
void RSI_OPAMP_InvPGA_V2(uint8_t opamp_instance, const Configure_OPAMP_t *params, uint8_t dyn_en, uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  if (opamp_instance == OPAMP1_INSTANCE) {
    rsi_configure_OPAMP1(&Config, params, 1, dyn_en);
    RSI_OPAMP1_Config(OPAMP, channel, &Config);
  } else if (opamp_instance == OPAMP2_INSTANCE) {
    rsi_configure_OPAMP2(&Config, params, 1, dyn_en);
    RSI_OPAMP2_Config(OPAMP, channel, &Config);

  } else if (opamp_instance == OPAMP3_INSTANCE) {
    rsi_configure_OPAMP3(&Config, params, 1, dyn_en);
    RSI_OPAMP3_Config(OPAMP, channel, &Config);
  } else {
    // Invalid opamp number
  }
}

/*==============================================*/
/**
 * @fn          void RSI_OPAMP_NonInvPGA_V2(uint8_t opamp_instance,const Configure_OPAMP_t *params, uint8_t dyn_en, uint8_t channel)
 * @brief       This API is used to configure the opamp1 in Non inverting PGA
 * @param[in]   dyn_en     : opamp1 dynamic mode enable(1 - enable ,0 - disable)
 * @param[in]   channel    : channel no to set in dynamic mode
 * @return      none
 */
void RSI_OPAMP_NonInvPGA_V2(uint8_t opamp_instance, const Configure_OPAMP_t *params, uint8_t dyn_en, uint8_t channel)
{
  OPAMP_CONFIG_T Config;

  if (opamp_instance == OPAMP1_INSTANCE) {
    rsi_configure_OPAMP1(&Config, params, 1, dyn_en);
    RSI_OPAMP1_Config(OPAMP, channel, &Config);
  } else if (opamp_instance == OPAMP2_INSTANCE) {
    rsi_configure_OPAMP2(&Config, params, 1, dyn_en);
    RSI_OPAMP2_Config(OPAMP, channel, &Config);

  } else if (opamp_instance == OPAMP3_INSTANCE) {
    rsi_configure_OPAMP3(&Config, params, 1, dyn_en);
    RSI_OPAMP3_Config(OPAMP, channel, &Config);
  } else {
    // Invalid opamp number
  }
}

/*==============================================*/
/**
 * @fn          void RSI_OPAMP_InvCMP_V2(uint8_t opamp_instance, const Configure_OPAMP_t *params,  uint8_t dyn_en, uint8_t channel)
 * @brief       This API is used to configuration of an opamp1 for inverting comparator with programmable hysteresis
 * @param[in]   dyn_en     : opamp1 dynamic mode enable(1 - enable ,0 - disable)
 * @param[in]   channel    : channel no to set in dynamic mode
 * @return      none
 */
void RSI_OPAMP_InvCMP_V2(uint8_t opamp_instance, const Configure_OPAMP_t *params, uint8_t dyn_en, uint8_t channel)
{
  OPAMP_CONFIG_T Config;

  if (opamp_instance == OPAMP1_INSTANCE) {
    rsi_configure_OPAMP1(&Config, params, 1, dyn_en);
    RSI_OPAMP1_Config(OPAMP, channel, &Config);
  } else if (opamp_instance == OPAMP2_INSTANCE) {
    rsi_configure_OPAMP2(&Config, params, 1, dyn_en);
    RSI_OPAMP2_Config(OPAMP, channel, &Config);

  } else if (opamp_instance == OPAMP3_INSTANCE) {
    rsi_configure_OPAMP3(&Config, params, 1, dyn_en);
    RSI_OPAMP3_Config(OPAMP, channel, &Config);
  } else {
    // Invalid opamp number
  }
}

/*==============================================*/
/**
 * @fn          void RSI_OPAMP_NonInvCMP_V2(uint8_t opamp_instance,const Configure_OPAMP_t *params, uint8_t dyn_en, uint8_t channel)
 * @brief       This API is used to configuration of an opamp1 for inverting comparator with programmable hysteresis
 * @param[in]   dyn_en     : opamp1 dynamic mode enable(1 - enable ,0 - disable)
 * @param[in]   channel    : channel no to set in dynamic mode
 * @return      none
 */
void RSI_OPAMP_NonInvCMP_V2(uint8_t opamp_instance, const Configure_OPAMP_t *params, uint8_t dyn_en, uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  if (opamp_instance == OPAMP1_INSTANCE) {
    rsi_configure_OPAMP1(&Config, params, 1, dyn_en);
    RSI_OPAMP1_Config(OPAMP, channel, &Config);
  } else if (opamp_instance == OPAMP2_INSTANCE) {
    rsi_configure_OPAMP2(&Config, params, 1, dyn_en);
    RSI_OPAMP2_Config(OPAMP, channel, &Config);

  } else if (opamp_instance == OPAMP3_INSTANCE) {
    rsi_configure_OPAMP3(&Config, params, 1, dyn_en);
    RSI_OPAMP3_Config(OPAMP, channel, &Config);
  } else {
    // Invalid opamp number
  }
}

/*==============================================*/
/**
 * @fn          void RSI_OPAMP_InstrAMP_v2(const Configure_OPAMP1_t *params, uint8_t dyn_en, uint8_t channel)
 * @brief       This API is used to configuration of opamp as instrumentation amplifier
 * @param[in]   vin_p_sel   : input selection (0 - GPIO_27,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 -ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8)
 * @param[in]   vin_n_sel   : input selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5)
 * @param[in]   enable      : opamp1 enable (1 - enable ,0 - disable)
 * @param[in]   lp_mode     : opamp1 low power mode(1 - enable ,0 - disable)
 * @param[in]   r1_sel     : selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   r2_sel     : selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en  : opamp output mux enable(1 - enable ,0 - disable output on GPIO27)
 * @param[in]   dyn_en      : opamp dynamic mode enable(1 - enable ,0 - disable)
 * @param[in]   channel     : channel no to set in dynamic mode
 * @return      none
 */
void RSI_OPAMP_InstrAMP_v2(const Configure_OPAMP_t *params, uint8_t dyn_en, uint8_t channel)
{
  OPAMP_CONFIG_T Config;

  rsi_configure_OPAMP1(&Config, params, 0, dyn_en);

  rsi_configure_OPAMP2(&Config, params, 1, dyn_en);

  rsi_configure_OPAMP3(&Config, params, 1, dyn_en);

  RSI_OPAMP1_Config(OPAMP, channel, &Config);
  RSI_OPAMP2_Config(OPAMP, channel, &Config);
  RSI_OPAMP3_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**
 * @fn          void RSI_OPAMP1_OPAMP2_CascInvtPGAmp(const Configure_OPAMP1_OPAMP2_t *params, uint8_t dyn_en, uint8_t channel)
 * @brief       This API is used to configuration of opamp as cascaded inverting pga
 * @param[in]   vin1_p_sel   : input selection (0 - GPIO_27,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 -ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8)
 * @param[in]   vin1_n_sel   : input selection (0 - GPIO_27,1 - ULP_GPIO_7)
 * @param[in]   vref1_sel    : input selection (0 - GPIO_27,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 -ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8)
 * @param[in]   vin2_p_sel   : input selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5)
 * @param[in]   vin2_n_sel   : input selection (0 - ULP_GPIO_11)
 * @param[in]   vref2_sel    : input selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5)
 * @param[in]   enable       : opamp1 enable (1 - enable ,0 - disable)
 * @param[in]   lp_mode      : opamp1 low power mode(1 - enable ,0 - disable)
 * @param[in]   opamp1_r1_sel: selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   opamp1_r2_sel: selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   opamp2_r1_sel: selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   opamp2_r2_sel: selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en   : opamp output mux enable(1 - enable ,0 - disable opamp2 output on ULP_GPIO_9)
 * @param[in]   dyn_en       : opamp dynamic mode enable(1 - enable ,0 - disable)
 * @param[in]   channel      : channel no to set in dynamic mode
 * @return      none
 */
void RSI_OPAMP1_OPAMP2_CascInvtPGAmp(const Configure_OPAMP1_OPAMP2_t *params, uint8_t dyn_en, uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Configure_OPAMP_t params1 = {
    .vin_p_sel   = params->vin1_p_sel,
    .vin_n_sel   = params->vin1_n_sel,
    .vref_sel    = params->vref1_sel,
    .enable      = params->enable,
    .lp_mode     = params->lp_mode,
    .r1_sel      = params->opamp1_r1_sel,
    .r2_sel      = params->opamp1_r2_sel,
    .out_mux_en  = params->out_mux_en,
    .out_mux_sel = params->out_mux_sel,
  };

  rsi_configure_OPAMP1(&Config, &params1, 1, dyn_en);

  Configure_OPAMP_t params2 = {
    .vin_p_sel  = params->vin2_p_sel,
    .vin_n_sel  = params->vin2_n_sel,
    .vref_sel   = params->vref2_sel,
    .enable     = params->enable,
    .lp_mode    = params->lp_mode,
    .r1_sel     = params->opamp2_r1_sel,
    .r2_sel     = params->opamp2_r2_sel,
    .out_mux_en = params->out_mux_en,
  };
  rsi_configure_OPAMP2(&Config, &params2, 1, dyn_en);

  RSI_OPAMP1_Config(OPAMP, channel, &Config);
  RSI_OPAMP2_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**
 * @fn          void RSI_OPAMP2_OPAMP3_CascInvtPGAmp(const Configure_OPAMP2_OPAMP3_t *params, uint8_t dyn_en, uint8_t channel)
 * @brief       This API is used to configuration of opamp as cascaded inverting pga
 * @param[in]   vin2_p_sel   : input selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5)
 * @param[in]   vin2_n_sel   : input selection (0 - ULP_GPIO_11)
 * @param[in]   vref2_sel    : input selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5)
 * @param[in]   vin3_p_sel   : input selection (0 - ULP_GPIO_10,1 - GPIO_29)
 * @param[in]   vin3_n_sel   : input selection (0 - ULP_GPIO_10)
 * @param[in]   vref3_sel    : input selection (0 - ULP_GPIO_10,1 - GPIO_29)
 * @param[in]   enable       : opamp1 enable (1 - enable ,0 - disable)
 * @param[in]   lp_mode      : opamp1 low power mode(1 - enable ,0 - disable)
 * @param[in]   opamp2_r1_sel: selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   opamp2_r2_sel: selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   opamp3_r1_sel: selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   opamp3_r2_sel: selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en   : opamp output mux enable(1 - enable ,0 - disable opamp3 output on GPIO_27)
 * @param[in]   dyn_en       : opamp dynamic mode enable(1 - enable ,0 - disable)
 * @param[in]   channel      : channel no to set in dynamic mode
 * @return      none
 */
void RSI_OPAMP2_OPAMP3_CascInvtPGAmp(const Configure_OPAMP2_OPAMP3_t *params, uint8_t dyn_en, uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Configure_OPAMP_t params1 = {
    .vin_p_sel  = params->vin2_p_sel,
    .vin_n_sel  = params->vin2_n_sel,
    .vref_sel   = params->vref2_sel,
    .enable     = params->enable,
    .lp_mode    = params->lp_mode,
    .r1_sel     = params->opamp2_r1_sel,
    .r2_sel     = params->opamp2_r2_sel,
    .out_mux_en = params->out_mux_en,
  };

  rsi_configure_OPAMP2(&Config, &params1, 1, dyn_en);

  Configure_OPAMP_t params2 = {
    .vin_p_sel  = params->vin3_p_sel,
    .vin_n_sel  = params->vin3_n_sel,
    .vref_sel   = params->vref3_sel,
    .enable     = params->enable,
    .lp_mode    = params->lp_mode,
    .r1_sel     = params->opamp3_r1_sel,
    .r2_sel     = params->opamp3_r2_sel,
    .out_mux_en = params->out_mux_en,
  };
  rsi_configure_OPAMP3(&Config, &params2, 1, dyn_en);

  RSI_OPAMP2_Config(OPAMP, channel, &Config);
  RSI_OPAMP3_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**
 * @fn          void RSI_OPAMP1_OPAMP2_CascNonInvtPGAmp(const Configure_OPAMP1_OPAMP2_t *params, uint8_t dyn_en, uint8_t channel)
 * @brief       This API is used to configuration of opamp as cascaded inverting pga
 * @param[in]   vin1_p_sel   : input selection (0 - GPIO_27,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 -ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8)
 * @param[in]   vin1_n_sel   : input selection (0 - GPIO_27,1 - ULP_GPIO_7)
 * @param[in]   vref1_sel    : input selection (0 - GPIO_27,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 -ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8)
 * @param[in]   vin2_p_sel   : input selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5)
 * @param[in]   vin2_n_sel   : input selection (0 - ULP_GPIO_11)
 * @param[in]   vref2_sel    : input selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5)
 * @param[in]   enable       : opamp1 enable (1 - enable ,0 - disable)
 * @param[in]   lp_mode      : opamp1 low power mode(1 - enable ,0 - disable)
 * @param[in]   opamp1_r1_sel: selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   opamp1_r2_sel: selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   opamp2_r1_sel: selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   opamp2_r2_sel: selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en   : opamp output mux enable(1 - enable ,0 - disable opamp2 output on ULP_GPIO_9)
 * @param[in]   dyn_en       : opamp dynamic mode enable(1 - enable ,0 - disable)
 * @param[in]   channel      : channel no to set in dynamic mode
 * @return      none
 */
void RSI_OPAMP1_OPAMP2_CascNonInvtPGAmp(const Configure_OPAMP1_OPAMP2_t *params, uint8_t dyn_en, uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Configure_OPAMP_t params1 = {
    .vin_p_sel   = params->vin1_p_sel,
    .vin_n_sel   = params->vin1_n_sel,
    .vref_sel    = params->vref1_sel,
    .enable      = params->enable,
    .lp_mode     = params->lp_mode,
    .r1_sel      = params->opamp1_r1_sel,
    .r2_sel      = params->opamp1_r2_sel,
    .out_mux_en  = params->out_mux_en,
    .out_mux_sel = params->out_mux_sel,
  };

  rsi_configure_OPAMP1(&Config, &params1, 1, dyn_en);

  Configure_OPAMP_t params2 = {
    .vin_p_sel  = params->vin2_p_sel,
    .vin_n_sel  = params->vin2_n_sel,
    .vref_sel   = params->vref2_sel,
    .enable     = params->enable,
    .lp_mode    = params->lp_mode,
    .r1_sel     = params->opamp2_r1_sel,
    .r2_sel     = params->opamp2_r2_sel,
    .out_mux_en = params->out_mux_en,
  };
  rsi_configure_OPAMP2(&Config, &params2, 1, dyn_en);

  RSI_OPAMP1_Config(OPAMP, channel, &Config);
  RSI_OPAMP2_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**
 * @fn          void RSI_OPAMP2_OPAMP3_CascNonInvtPGAmp(const Configure_OPAMP2_OPAMP3_t *params, uint8_t dyn_en, uint8_t channel)
 * @brief       This API is used to configuration of opamp as cascaded inverting pga
 * @param[in]   vin2_p_sel   : input selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5)
 * @param[in]   vin2_n_sel   : input selection (0 - ULP_GPIO_11)
 * @param[in]   vref2_sel    : input selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5)
 * @param[in]   vin3_p_sel   : input selection (0 - ULP_GPIO_10,1 - GPIO_29)
 * @param[in]   vin3_n_sel   : input selection (0 - ULP_GPIO_10)
 * @param[in]   vref3_sel    : input selection (0 - ULP_GPIO_10,1 - GPIO_29)
 * @param[in]   enable       : opamp1 enable (1 - enable ,0 - disable)
 * @param[in]   lp_mode      : opamp1 low power mode(1 - enable ,0 - disable)
 * @param[in]   opamp2_r1_sel: selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   opamp2_r2_sel: selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   opamp3_r1_sel: selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   opamp3_r2_sel: selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en   : opamp output mux enable(1 - enable ,0 - disable opamp3 output on GPIO_27)
 * @param[in]   dyn_en       : opamp dynamic mode enable(1 - enable ,0 - disable)
 * @param[in]   channel      : channel no to set in dynamic mode
 * @return      none
 */
void RSI_OPAMP2_OPAMP3_CascNonInvtPGAmp(const Configure_OPAMP2_OPAMP3_t *params, uint8_t dyn_en, uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Configure_OPAMP_t params1 = {
    .vin_p_sel  = params->vin2_p_sel,
    .vin_n_sel  = params->vin2_n_sel,
    .vref_sel   = params->vref2_sel,
    .enable     = params->enable,
    .lp_mode    = params->lp_mode,
    .r1_sel     = params->opamp2_r1_sel,
    .r2_sel     = params->opamp2_r2_sel,
    .out_mux_en = params->out_mux_en,
  };

  rsi_configure_OPAMP2(&Config, &params1, 1, dyn_en);

  Configure_OPAMP_t params2 = {
    .vin_p_sel  = params->vin3_p_sel,
    .vin_n_sel  = params->vin3_n_sel,
    .vref_sel   = params->vref3_sel,
    .enable     = params->enable,
    .lp_mode    = params->lp_mode,
    .r1_sel     = params->opamp3_r1_sel,
    .r2_sel     = params->opamp3_r2_sel,
    .out_mux_en = params->out_mux_en,
  };
  rsi_configure_OPAMP3(&Config, &params2, 1, dyn_en);

  RSI_OPAMP2_Config(OPAMP, channel, &Config);
  RSI_OPAMP3_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**
 * @fn          void RSI_OPAMP1_OPAMP2_TwoOpampsDiffAmp(const Configure_OPAMP1_OPAMP2_t *params, uint8_t dyn_en, uint8_t channel)
 * @brief       This API is used to configuration of opamp as two opamps differential amp
 * @param[in]   vin1_p_sel   : input selection (0 - GPIO_27,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 -ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8)
 * @param[in]   vin1_n_sel   : input selection (0 - GPIO_27,1 - ULP_GPIO_7)
 * @param[in]   vref1_sel    : input selection (0 - GPIO_27,1 - ULP_GPIO_7,2 - ULP_GPIO_0,3 -ULP_GPIO_2,4 -ULP_GPIO_6,5 -ULP_GPIO_8)
 * @param[in]   vin2_p_sel   : input selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5)
 * @param[in]   vin2_n_sel   : input selection (0 - ULP_GPIO_11)
 * @param[in]   vref2_sel    : input selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5)
 * @param[in]   enable       : opamp1 enable (1 - enable ,0 - disable)
 * @param[in]   lp_mode      : opamp1 low power mode(1 - enable ,0 - disable)
 * @param[in]   opamp2_r1_sel: selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   opamp2_r2_sel: selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en   : opamp output mux enable(1 - enable ,0 - disable opamp2 output on ULP_GPIO_9)
 * @param[in]   dyn_en       : opamp dynamic mode enable(1 - enable ,0 - disable)
 * @param[in]   channel      : channel no to set in dynamic mode
 * @return      none
 */
void RSI_OPAMP1_OPAMP2_TwoOpampsDiffAmp(const Configure_OPAMP1_OPAMP2_t *params, uint8_t dyn_en, uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Configure_OPAMP_t params1 = {
    .vin_p_sel   = params->vin1_p_sel,
    .vin_n_sel   = params->vin1_n_sel,
    .vref_sel    = params->vref1_sel,
    .enable      = params->enable,
    .lp_mode     = params->lp_mode,
    .out_mux_en  = params->out_mux_en,
    .out_mux_sel = params->out_mux_sel,
  };
  rsi_configure_OPAMP1(&Config, &params1, 0, dyn_en);

  Configure_OPAMP_t params2 = {
    .vin_p_sel  = params->vin2_p_sel,
    .vin_n_sel  = params->vin2_n_sel,
    .vref_sel   = params->vref2_sel,
    .enable     = params->enable,
    .lp_mode    = params->lp_mode,
    .r1_sel     = params->opamp2_r1_sel,
    .r2_sel     = params->opamp2_r2_sel,
    .out_mux_en = params->out_mux_en,
  };

  rsi_configure_OPAMP2(&Config, &params2, 1, dyn_en);

  RSI_OPAMP1_Config(OPAMP, channel, &Config);
  RSI_OPAMP2_Config(OPAMP, channel, &Config);
}

/*==============================================*/
/**
 * @fn          void RSI_OPAMP2_OPAMP3_TwoOpampsDiffAmp(const Configure_OPAMP2_OPAMP3_t *params, uint8_t dyn_en, uint8_t channel)
 * @brief       This API is used to configuration of opamp as two opamps differential amp
 * @param[in]   vin2_p_sel   : input selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5)
 * @param[in]   vin2_n_sel   : input selection (0 - ULP_GPIO_11)
 * @param[in]   vref2_sel    : input selection (0 - ULP_GPIO_11,1 - ULP_GPIO_5)
 * @param[in]   vin3_p_sel   : input selection (0 - ULP_GPIO_10,1 - GPIO_29)
 * @param[in]   vin3_n_sel   : input selection (0 - ULP_GPIO_10)
 * @param[in]   vref3_sel    : input selection (0 - ULP_GPIO_10,1 - GPIO_29)
 * @param[in]   enable       : opamp1 enable (1 - enable ,0 - disable)
 * @param[in]   lp_mode      : opamp1 low power mode(1 - enable ,0 - disable)
 * @param[in]   opamp3_r1_sel: selection of resistor r1(0 - 0kohm,1 - 20kohm,2 - 60kohm,3- 140kohm)
 * @param[in]   opamp3_r2_sel: selection of resistor r2(0 - 20kohm,1 - 30kohm,2 - 40kohm,3- 60kohm,4 - 120kohm,5 - 250kohm,6 - 500kohm,7 - 1000kohm)
 * @param[in]   out_mux_en   : opamp output mux enable(1 - enable ,0 - disable opamp2 output on GPIO_27)
 * @param[in]   dyn_en       : opamp dynamic mode enable(1 - enable ,0 - disable)
 * @param[in]   channel      : channel no to set in dynamic mode
 * @return      none
 */
void RSI_OPAMP2_OPAMP3_TwoOpampsDiffAmp(const Configure_OPAMP2_OPAMP3_t *params, uint8_t dyn_en, uint8_t channel)
{
  OPAMP_CONFIG_T Config;
  Configure_OPAMP_t params1 = {
    .vin_p_sel  = params->vin2_p_sel,
    .vin_n_sel  = params->vin2_n_sel,
    .vref_sel   = params->vref2_sel,
    .enable     = params->enable,
    .lp_mode    = params->lp_mode,
    .out_mux_en = params->out_mux_en,
  };
  rsi_configure_OPAMP2(&Config, &params1, 0, dyn_en);

  Configure_OPAMP_t params2 = {
    .vin_p_sel  = params->vin3_p_sel,
    .vin_n_sel  = params->vin3_n_sel,
    .vref_sel   = params->vref3_sel,
    .enable     = params->enable,
    .lp_mode    = params->lp_mode,
    .r1_sel     = params->opamp3_r1_sel,
    .r2_sel     = params->opamp3_r2_sel,
    .out_mux_en = params->out_mux_en,
  };

  rsi_configure_OPAMP3(&Config, &params2, 1, dyn_en);

  RSI_OPAMP2_Config(OPAMP, channel, &Config);
  RSI_OPAMP3_Config(OPAMP, channel, &Config);
}

/** @} */
