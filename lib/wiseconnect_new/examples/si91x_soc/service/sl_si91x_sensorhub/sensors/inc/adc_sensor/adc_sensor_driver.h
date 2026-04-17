/***************************************************************************/ /**
 * @file adc_sensor_driver.h
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

#ifndef ADC_SENSOR_DRIVER_H_
#define ADC_SENSOR_DRIVER_H_
#include "sl_si91x_adc.h"

/***************************************************************************/ /**
 * Typedef for different pointer handles
 ******************************************************************************/
typedef void *sl_err_t;
typedef void *adc_bus_handle_t;

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Prototypes

/***************************************************************************/ /**
 * @fn sl_status_t sl_si91x_adc_channel_init(sl_adc_channel_config_t *adc_ch_cfg, sl_adc_config_t *adc_cfg)
 * @brief ADC channel initialization
 * This function should be called before everything else. And ADC peripheral
 * initialization should be done before calling this function. This function
 * will only initliaze respective ADC channel and does not start the ADC.
 *
 * @param[in] adc_ch_cfg : ADC channel configuration
 * @param[in] adc_cfg : ADC configuration
 * @return status 0 if successful, else error code
 *         \ref SL_STATUS_OK  - Success
 *         \ref SL_STATUS_NULL_POINTER  - The parameter is null pointer
 *         \ref SL_STATUS_INVALID_PARAMETER  - Parameters are invalid
 *         \ref SL_STATUS_FAIL  - The function is failed
 *         \ref SL_STATUS_NOT_INITIALIZED  - Clock is not initialized
 ******************************************************************************/
sl_status_t sl_si91x_adc_channel_init(sl_adc_channel_config_t *adc_ch_cfg, sl_adc_config_t *adc_cfg);

/***************************************************************************/ /**
 * @fn sl_status_t sl_si91x_adc_de_init(sl_adc_config_t *adc_cfg)
 * @brief ADC deinitialization
 * This function powers off ADC block and stops ADC operations
 *
 * @param[in] adc_cfg : ADC configuration
 * @return status 0 if successful, else error code
 *         \ref SL_STATUS_OK  - Success
 *         \ref SL_STATUS_NULL_POINTER  - The parameter is null pointer
 *         \ref SL_STATUS_INVALID_PARAMETER  - Parameters are invalid
 *         \ref SL_STATUS_FAIL  - The function is failed
 *         \ref SL_STATUS_NOT_INITIALIZED  - Clock is not initialized
 ******************************************************************************/
sl_status_t sl_si91x_adc_de_init(sl_adc_config_t *adc_cfg);

/***************************************************************************/ /**
 * @fn sl_status_t sl_si91x_adc_chnl_enable(uint8_t channel)
 * @brief ADC channel enable
 * This function enables respective ADC channel.
 *
 * @param[in] channel : ADC channel number
 * @return status 0 if successful, else error code
 *         \ref SL_STATUS_OK  - Success
 *         \ref SL_STATUS_NULL_POINTER  - The parameter is null pointer
 *         \ref SL_STATUS_INVALID_PARAMETER  - Parameters are invalid
 *         \ref SL_STATUS_FAIL  - The function is failed
 *         \ref SL_STATUS_NOT_INITIALIZED  - Clock is not initialized
 ******************************************************************************/
sl_status_t sl_si91x_adc_chnl_enable(uint8_t channel);

/***************************************************************************/ /**
 * @fn sl_status_t sl_si91x_adc_chnl_disable(uint8_t channel)
 * @brief ADC channel disable
 * This function disables respective ADC channel.
 *
 * @param[in] channel : ADC channel number
 * @return status 0 if successful, else error code
 *         \ref SL_STATUS_OK  - Success
 *         \ref SL_STATUS_NULL_POINTER  - The parameter is null pointer
 *         \ref SL_STATUS_INVALID_PARAMETER  - Parameters are invalid
 *         \ref SL_STATUS_FAIL  - The function is failed
 *         \ref SL_STATUS_NOT_INITIALIZED  - Clock is not initialized
 ******************************************************************************/
sl_status_t sl_si91x_adc_chnl_disable(uint8_t channel);

/***************************************************************************/ /**
 * @fn sl_status_t sl_si91x_adc_read_static_sample(sl_adc_channel_config_t *adc_ch_cfg,
                                            sl_adc_config_t *adc_cfg,
                                            uint16_t *adc_value)
 * @brief ADC read static data
 * This function reads static ADC data from the register.
 *
 * @param[in] adc_ch_cfg : ADC channel configuration
 * @param[in] adc_cfg : ADC configuration
 * @param[in] adc_value :  memory pointer to store ADC data
 * @return status 0 if successful, else error code
 *         \ref SL_STATUS_OK  - Success
 *         \ref SL_STATUS_NULL_POINTER  - The parameter is null pointer
 *         \ref SL_STATUS_INVALID_PARAMETER  - Parameters are invalid
 *         \ref SL_STATUS_FAIL  - The function is failed
 *         \ref SL_STATUS_NOT_INITIALIZED  - Clock is not initialized
 ******************************************************************************/
sl_status_t sl_si91x_adc_read_static_sample(sl_adc_channel_config_t *adc_ch_cfg,
                                            sl_adc_config_t *adc_cfg,
                                            uint16_t *adc_value);

/***************************************************************************/ /**
 * @fn sl_status_t sl_si91x_adc_channel_read_sample(sl_adc_channel_config_t *adc_ch_cfg, uint8_t channl_num)
 * @brief ADC read channle sample
 * This function reads ADC data for specific channel in FIFO mode
 *
 * @param[in] ADC channel configuration
 * @param[in] ADC channle number
 *
 * @return status 0 if successful, else error code
 *         \ref SL_STATUS_OK  - Success
 *         \ref SL_STATUS_NULL_POINTER  - The parameter is null pointer
 *         \ref SL_STATUS_INVALID_PARAMETER  - Parameters are invalid
 *         \ref SL_STATUS_FAIL  - The function is failed
 *         \ref SL_STATUS_NOT_INITIALIZED  - Clock is not initialized
 ******************************************************************************/
sl_status_t sl_si91x_adc_channel_read_sample(sl_adc_channel_config_t *adc_ch_cfg, uint8_t channl_num);

#ifdef __cplusplus
extern "C"
}
#endif

#endif /* ADC_SENSOR_DRIVER_H_ */
