/***************************************************************************/ /**
 * @file adc_sensor_driver.c
 * @brief adc sensor driver
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "adc_sensor_driver.h"
#include "rsi_debug.h"
#include "sl_si91x_adc.h"
#include "rsi_error.h"

/*******************************************************************************
 * @fn sl_status_t sl_si91x_adc_channel_init(sl_adc_channel_config_t *adc_ch_cfg, sl_adc_config_t *adc_cfg)
 * @brief ADC channel initialization
 * This function should be called before everything else. And ADC peripheral
 * initialization should be done before calling this function. This function
 * will only initliaze respective ADC channel and does not start the ADC.
 *
 * @param[in] ADC channel configuration
 * @param[in] ADC configuration
 *
 * @return respective sl error code
*******************************************************************************/
sl_status_t sl_si91x_adc_channel_init(sl_adc_channel_config_t *adc_ch_cfg, sl_adc_config_t *adc_cfg)
{
  sl_status_t sl_status;

  sl_status = sl_si91x_adc_set_channel_configuration(*adc_ch_cfg, *adc_cfg);

  return sl_status;
}

/*******************************************************************************
 * @fn sl_status_t sl_si91x_adc_de_init(sl_adc_config_t *adc_cfg)
 * @brief ADC deinitialization
 * This function powers off ADC block and stops ADC operations
 *
 * @param[in] ADC configuration
 *
 * @return respective sl error code
*******************************************************************************/
sl_status_t sl_si91x_adc_de_init(sl_adc_config_t *adc_cfg)
{
  sl_status_t sl_status;
  sl_status = sl_si91x_adc_stop(*adc_cfg);
  sl_status = sl_si91x_adc_deinit(*adc_cfg);

  return sl_status;
}

/*******************************************************************************
 * @fn sl_status_t sl_si91x_adc_chnl_enable(uint8_t channel)
 * @brief ADC channel enable
 * This function enables respective ADC channel.
 *
 * @param[in] ADC channel configuration
 * @param[in] ADC configuration
 *
 * @return respective sl error code
*******************************************************************************/
sl_status_t sl_si91x_adc_chnl_enable(uint8_t channel)
{
  sl_status_t sl_status;
  sl_status = sl_si91x_adc_channel_enable(channel);

  return sl_status;
}

/*******************************************************************************
 * @fn sl_status_t sl_si91x_adc_chnl_disable(uint8_t channel)
 * @brief ADC channel disable
 * This function disables respective ADC channel.
 *
 * @param[in] ADC channel configuration
 * @param[in] ADC configuration
 *
 * @return respective sl error code
*******************************************************************************/
sl_status_t sl_si91x_adc_chnl_disable(uint8_t channel)
{
  sl_status_t sl_status;
  sl_status = sl_si91x_adc_channel_disable(channel);

  return sl_status;
}

/*******************************************************************************
 * @fn sl_status_t sl_si91x_adc_read_static_sample(sl_adc_channel_config_t *adc_ch_cfg,
                                            sl_adc_config_t *adc_cfg,
                                            uint16_t *adc_value)
 * @brief ADC read static data
 * This function reads static ADC data from the register.
 *
 * @param[in] ADC channel configuration
 * @param[in] ADC configuration
 *
 * @return respective sl error code
*******************************************************************************/
sl_status_t sl_si91x_adc_read_static_sample(sl_adc_channel_config_t *adc_ch_cfg,
                                            sl_adc_config_t *adc_cfg,
                                            uint16_t *adc_value)
{
  sl_status_t sl_status;

  sl_status = sl_si91x_adc_read_data_static(*adc_ch_cfg, *adc_cfg, adc_value);
  if (sl_status != SL_STATUS_OK) {
    return sl_status;
  }

  if (*adc_value & BIT(11)) {
    *adc_value = (*adc_value & (ADC_MASK_VALUE));
  } else {
    *adc_value = *adc_value | BIT(11);
  }

  return sl_status;
}

/*******************************************************************************
 * @fn sl_status_t sl_si91x_adc_channel_read_sample(sl_adc_channel_config_t *adc_ch_cfg, uint8_t channl_num)
 * @brief ADC read channle sample
 * This function reads ADC data for specific channel in FIFO mode
 *
 * @param[in] ADC channel configuration
 * @param[in] ADC channle number
 *
 * @return respective sl error code
*******************************************************************************/
sl_status_t sl_si91x_adc_channel_read_sample(sl_adc_channel_config_t *adc_ch_cfg, uint8_t channl_num)
{
  sl_status_t sl_status;
  sl_status = sl_si91x_adc_read_data(*adc_ch_cfg, channl_num);
  if (sl_status != SL_STATUS_OK) {
    return sl_status;
  }

  return sl_status;
}
