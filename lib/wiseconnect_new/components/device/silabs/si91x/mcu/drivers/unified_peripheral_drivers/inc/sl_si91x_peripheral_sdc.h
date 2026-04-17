/******************************************************************************
* @file  sl_si91x_peripheral_sdc.h
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

#ifndef SL_SI91X_PERIPHERAL_SDC_H
#define SL_SI91X_PERIPHERAL_SDC_H

#include "rsi_ccp_common.h"
#include "base_types.h"
#include "sl_status.h"
#include "rsi_adc.h"
#include "rsi_rtc.h"
#include "rsi_rom_egpio.h"
#include "core_cm4.h"

/***************************************************************************/
/**
 * @addtogroup SDC Sensor Data Collector
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Structure to hold SDC general configuration parameters.
  @details
 * This structure contains the configuration parameters for the SDC peripheral.
 */
typedef struct {
  uint32_t sample_threshold;      ///<Sample threshold for SDC operation
  uint32_t number_of_channels;    ///<Number of channels to enable
  uint32_t clock_division_factor; ///<Clock division factor for SDC
  uint32_t trigger_ms_sec_select; ///< trigger in milliseconds or seconds
  uint32_t sampling_interval;     ///< Sampling interval in milliseconds or seconds
} sl_si91x_sdc_config_t;

/**
 * @brief Structure to hold SDC ADC pin configuration parameters.
 * @details
 * This structure contains the configuration parameters for the SDC ADC pins.
 */
typedef struct {
  uint32_t sdc_adc_mode;           ///< ADC mode: single-ended or differential
  uint32_t sdc_adc_p_input_pinsel; ///< Positive input pin selection
  uint32_t sdc_adc_n_input_pinsel; ///< Negative input pin selection
} sl_si91x_sdc_adc_config_t;

/**
 * @brief Initialize the SDC peripheral.
 *
 * @details This API initializes the SDC hardware and prepares it for operation.
          1.It Initializes the SDC clock
          2.Clears and Enables the SDC interrupt
          3.Reset the SDC FIFO pointer.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully initialized the SDC. 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ****************************************************************************************/
sl_status_t sl_si91x_sdc_init(void);

/***************************************************************************/

/**
 * @brief Deinitialize the SDC peripheral.
        1. It disables the SDC clock
        2.It clears and disables the SDC interrupt
        3.It resets the SDC FIFO pointer.
 *
 * @details This API deinitializes the SDC hardware and releases any resources.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully deinitialized the SDC.
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ****************************************************************************************/
sl_status_t sl_si91x_sdc_deinit(void);

/***************************************************************************/

/**
 * @brief Configure the SDC ADC pin settings.
 *
 * @details This API configures the SDC ADC for single-ended or differential mode and sets the input pins.
 *        1. It configures the SDC ADC pins based on the provided configuration.
 *
 * @param[in] sdc_adc_config Pointer to SDC ADC pin configuration structure.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully configured the SDC ADC pins.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ****************************************************************************************/
sl_status_t sl_si91x_sdc_adc_config(sl_si91x_sdc_adc_config_t *sdc_adc_config);

/***************************************************************************/

/**
 * @brief Configure the SDC general settings.
 *
 * @details This API configures the SDC general parameters such as sample threshold, channel count, and trigger settings.
 *          1. It sets the sample threshold, number of channels, clock division factor, trigger event selection, and trigger time selection.
 *
 * @param[in] sdc_general_config Pointer to SDC general configuration structure.
 *
 * @return sl_status_t Status code indicating the result:
 *         - SL_STATUS_OK                 - Successfully configured the SDC.
 *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ****************************************************************************************/
sl_status_t sl_si91x_sdc_config(sl_si91x_sdc_config_t *sdc_general_config);

/***************************************************************************/

/**
 * @brief Get the SDC interrupt status.
 * 
 * @details This API returns the current interrupt status for the SDC peripheral.
 *        1. It checks the interrupt status of the SDC peripheral.
 *
 * @return sl_status_t Status code indicating the result.
 * For more information on status codes, see [SL STATUS DOCUMENTATION](
 * https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ****************************************************************************************/
sl_status_t sl_si91x_sdc_interrupt_status(void);

/***************************************************************************/

/**
 * @brief Clear the SDC interrupt.
 *
 * @details This API clears the interrupt status for the SDC peripheral.
 *        1. It clears the interrupt status of the SDC peripheral.
 ****************************************************************************************/
void sl_si91x_sdc_interrupt_clear(void);

/***************************************************************************/

/**
 * @brief Reset the SDC FIFO.
 *
 * @details This API resets the FIFO buffer of the SDC peripheral.
 *        1. It resets the write pointer of the SDC FIFO.
 ****************************************************************************************/
void sl_si91x_sdc_fifo_reset(void);

/***************************************************************************/

/**
 * @brief Enable SDC sampling.
 *
 * @details This API enables the sampling process for the SDC peripheral.
 *        1. It enables the SDC sampling by setting the appropriate configuration.
 ****************************************************************************************/
void sl_si91x_sdc_sample_enable(void);

/***************************************************************************/

/**
 * @brief Configure the SDC ADC input pins.
 *
 * @details
 * This function configures the pin multiplexing for the SDC ADC input pins according to the specified configuration.
 * It sets up the appropriate pins for single-ended or differential ADC mode and supports configuration for multiple channels.
 *
 * @param[in] sdc_adc_config Pointer to the SDC ADC pin configuration structure containing mode and pin selections.
 * @param[in] number_of_channels Number of ADC channels to configure.
 *
 * @note This function must be called before starting SDC sampling to ensure correct pin setup.
 ***************************************************************************************************/
void sl_si91x_sdc_adc_pin_config(sl_si91x_sdc_adc_config_t *sdc_adc_config, uint32_t number_of_channels);

/***************************************************************************/

/**
 * @brief Enable the SDC LDO.
 *
 * @details This API enables the Low Dropout Regulator (LDO) for the SDC peripheral.
 *        1. It enables the LDO for the SDC peripheral to provide stable voltage.
 *********************************************************************************************/
void sl_si91x_sdc_ldo_enable(void);

/***************************************************************************/

/**
 * @brief Configure the SDC LDO voltage.
 *
 * @details This API configures the LDO voltage for the SDC peripheral.
 *        1. It sets the LDO voltage based on vref voltage and chip voltage.
 *
 * @param[in] vref_voltage Reference voltage for the LDO.
 * @param[in] chip_voltage Chip voltage.
 ************************************************************************************************/
void sl_si91x_sdc_ldo_config(float vref_voltage, uint32_t chip_voltage);

/***************************************************************************/

/**
 * @brief Disable the SDC LDO.
 *
 * @details This API disables the Low Dropout Regulator (LDO) for the SDC peripheral.
 *      1. It disables the LDO for the SDC peripheral to save power when not needed.
 *************************************************************************************************/
void sl_si91x_sdc_ldo_disable(void);

/**
 * @brief Bypass the SDC LDO.
 *
 * @details This API bypases the Low Dropout Regulator (LDO) for the SDC peripheral.
 *************************************************************************************************/
void sl_si91x_sdc_ldo_bypass(void);

/***************************************************************************/

/**
 * @brief Read data from the SDC data registers.
 *
 * @details This API reads data from the SDC peripheral and stores it in the receive buffer.
 *        1. It reads the data from the SDC FIFO returns it.
 *
 * @param[in] fifo_position Position in the FIFO from which to read data.
 * @param[in] data Pointer to variable where the read sample will be stored.
 * @param[in] channel_id Pointer to variable where channel id of the read sample is stored
 * @return sl_status_t Status code indicating the result:
    *         - SL_STATUS_OK                 - Successfully read data from the SDC FIFO.
    *         - SL_STATUS_INVALID_PARAMETER  - Parameters are invalid.
 ***************************************************************************************************/
sl_status_t sl_si91x_sdc_read_data(int16_t *data, uint32_t fifo_position, uint32_t *channel_id);

/***************************************************************************/

/**
 * @brief Stops the Sensor Data Collector (SDC) interface.
 *
 * This function halts all ongoing operations and disables the SDC peripheral,
 * ensuring that the interface is safely stopped. It should be called before
 * performing any hardware reconfiguration or when the SDC is no longer needed.
 *
 *************************************************************************************************/
void sl_si91x_sdc_stop(void);
/***************************************************************************/

#ifdef __cplusplus
}
#endif

/// @} end group SDC ********************************************************/

#endif // SL_SI91X_PERIPHERAL_SDC_H
