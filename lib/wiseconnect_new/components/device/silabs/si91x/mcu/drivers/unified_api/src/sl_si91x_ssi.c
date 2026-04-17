/******************************************************************************
* @file sl_si91x_ssi.c
* @brief SSI Peripheral API implementation
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

#include "sl_si91x_ssi.h"
#include "rsi_rom_clks.h"
#include "rsi_spi.h"
#ifdef SSI_INSTANCE_CONFIG
#include "sl_ssi_instances.h"
#elif SSI_CONFIG
#include "sl_si91x_ssi_config.h"
#endif

/*******************************************************************************
****************************  DEFINES / MACROS   ********************************
*******************************************************************************/
#define MAX_BAUDRATE         40000000 // Maximum bitrate
#define ULP_SSI_MAX_BAUDRATE 5000000  // Maximum bit rate that ULP_SSI can support
#define MANUAL_LOCK          1        // Manual Lock Enable
#define BYPASS_MANUAL_LOCK   1        // Bypass Manual Lock Enable
#define SSI_RELEASE_VERSION  0        // SSI Release version
#define SSI_SQA_VERSION      0        // SSI SQA version
#define SSI_DEV_VERSION      2        // SSI Developer version
#define ODD_DIV_ENABLE       0        // Odd division enable for QSPI clock
#define SWALLO_ENABLE        0        // Swallo enable for QSPI clock
#define MIN_BIT_WIDTH        4        // Minimum Bit width
#define MAX_SLAVE_BIT_WIDTH  16       // Maximum Slave Bit width
#define MAX_BIT_WIDTH        16       // Maximum Bit width
#define MAX_INSTANCE         3        // Maximum SSI Instance

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static sl_ssi_signal_event_t user_callback[] = { NULL, NULL, NULL };
extern sl_ssi_driver_t Driver_SSI_MASTER;
extern sl_ssi_driver_t Driver_SSI_SLAVE;
extern sl_ssi_driver_t Driver_SSI_ULP_MASTER;

/*******************************************************************************
 ***************************  Local Types  ********************************
 ******************************************************************************/

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
static sl_status_t convert_arm_to_sl_error_code(int32_t error);
static sl_status_t get_ssi_handle(sl_ssi_instance_t instance, sl_ssi_handle_t *ssi_handle);
static sl_status_t validate_control_parameters(sl_ssi_control_config_t *control_configuration);
static void sl_si91x_ssi_set_fifo_threshold(sl_ssi_handle_t ssi_handle);
static void callback_event_handler(uint32_t event);
static void sl_ssi_set_receive_sample_delay(sl_ssi_handle_t ssi_handle, uint32_t sample_delay);
static boolean_t validate_ssi_handle(sl_ssi_handle_t ssi_handle);
static sl_status_t sli_si91x_ssi_configure_power_mode(sl_ssi_handle_t ssi_handle, sl_ssi_power_state_t state);
static uint8_t convert_handle_to_instance(sl_ssi_handle_t ssi_handle);

/*******************************************************************************
 **********************  Local Function Definition****************************
 ******************************************************************************/

/*******************************************************************************
***********************  Global function Definitions *************************
 ******************************************************************************/
/*******************************************************************************
 * To configure clock with input clock config structure for SSI. It validates
 * the clock frequency validate_clock_paramters function. If the clock frequency
 * is in range, it activates the clock for SSI peripheral. RSI errors are converted
 * to SL errors via convert_rsi_to_sl_error_code function.
 *
 * @param[in] clock config structure
 * @return
 * *         returns status - SL_STATUS_OK if data is set successfully,
 *           error return code such as SL_STATUS_INVALID_PARAMETER otherwise.
 * @note
 *   Moving forward, this API will be deprecated. This is retained for backward compatibility.
*******************************************************************************/
sl_status_t sl_si91x_ssi_configure_clock(sl_ssi_clock_config_t *clock_config)
{
  sl_status_t status = SL_STATUS_OK;

  do {
    if (clock_config == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * To initialize ssi peripheral interface. Call this function for fetching SSI
 * handle according to the instance provided.  ARM errors are converted to SL errors via
 * convert_arm_to_sl_error_code function.
 *
 * @param[in] instance of SSI
 * @param[in] ssi_handle.
 * @return    status 0 if successful, else error code.
 *
 *         SL_STATUS_OK (0x0000) - Success, otherwise fail error code
*******************************************************************************/
sl_status_t sl_si91x_ssi_init(sl_ssi_instance_t instance, sl_ssi_handle_t *ssi_handle)
{
  sl_status_t status              = 0;
  int32_t error_status            = 0;
  sl_ssi_handle_t ssi_temp_handle = NULL;
  do {
#ifdef SSI_INSTANCE_CONFIG
    if (instance == SL_SSI_MASTER_ACTIVE) {
#if (SSI_PRIMARY_UC == ENABLE)
      instance = ssi_primary_configuration.device_mode;
#endif
    } else if (instance == SL_SSI_SLAVE_ACTIVE) {
#if (SSI_SECONDARY_UC == ENABLE)
      instance = ssi_secondary_configuration.device_mode;
#endif
    } else if (instance == SL_SSI_ULP_MASTER_ACTIVE) {
#if (SSI_ULP_PRIMARY_UC == ENABLE)
      instance = ssi_ulp_primary_configuration.device_mode;
#endif
    }
#elif SSI_CONFIG
#ifdef SSI_UC
    instance              = ssi_configuration.device_mode;
#endif
#endif
    if (SSI_GetInitState(instance) == ENABLE) {
      status = SL_STATUS_BUSY;
      break;
    }
    status = get_ssi_handle(instance, &ssi_temp_handle);
    if (status != SL_STATUS_OK) {
      status = SL_STATUS_FAIL;
      break;
    }
    if (ssi_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    *ssi_handle  = ssi_temp_handle;
    error_status = ((sl_ssi_driver_t *)ssi_temp_handle)->Initialize(callback_event_handler);
    status       = convert_arm_to_sl_error_code(error_status);
    if (status != SL_STATUS_OK) {
      return status;
    }
    status = sli_si91x_ssi_configure_power_mode(ssi_temp_handle, ARM_POWER_FULL);
  } while (false);
  return status;
}

/*******************************************************************************
 * To uninitialize SSI peripheral interface. ARM errors are converted
 * to SL errors via convert_arm_to_sl_error_code function.
 *
 * @param[in] ssi_handle
 * @return    status 0 if successful, else error code.
 *
 *         SL_STATUS_OK (0x0000) - Success, otherwise fail error code
*******************************************************************************/
sl_status_t sl_si91x_ssi_deinit(sl_ssi_handle_t ssi_handle)
{
  sl_status_t status;
  int32_t error_status;
  do {
    // validate passed in parameter
    if (ssi_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (!validate_ssi_handle(ssi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    //unregister the callback event on a per-instance basis.
    sl_si91x_ssi_per_instance_unregister_event_callback(ssi_handle);

    error_status = ((sl_ssi_driver_t *)ssi_handle)->Uninitialize();
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * To configure SSI interface power.
 *
 * @param[in] ARM power state. The following 2 modes are supported:
 * ARM_POWER_OFF
 * ARM_POWER_FULL
 * @return
 * *         returns status - SL_STAUS_OK if power was set successfully,
 *  error return code such as SL_STATUS_FAIL otherwise.
 *  ARM errors are converted to SL errors via convert_arm_to_sl_error_code function.
*******************************************************************************/
static sl_status_t sli_si91x_ssi_configure_power_mode(sl_ssi_handle_t ssi_handle, sl_ssi_power_state_t state)
{
  int32_t error_status;
  sl_status_t status;
  do {
    // validates the power mode, if invalid state or invalid handle passed in
    // then returns error status
    if (state > ARM_POWER_FULL || !validate_ssi_handle(ssi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (ssi_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    error_status = ((sl_ssi_driver_t *)ssi_handle)->PowerControl(state);
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * SSI peripheral control and config.
 *
 * @param[in] ssi handle
 * @param[in] SSI Control config struct with fields for various params,
 * @return
 * *         returns status - SL_STATUS_OK if config operation was
 *           fine, error return code such as SL_STATUS_FAIL otherwise.
 *           ARM errors are converted to SL errors via convert_arm_to_sl_error_code
 *           function.
*******************************************************************************/
sl_status_t sl_si91x_ssi_set_configuration(sl_ssi_handle_t ssi_handle,
                                           sl_ssi_control_config_t *control_configuration,
                                           sl_ssi_slave_number_t slave_number)
{
  UNUSED_PARAMETER(slave_number);
  sl_status_t status;
  int32_t error_status;
  uint32_t input_mode = 0;
  do {
#ifdef SSI_INSTANCE_CONFIG
    if (ssi_handle == &Driver_SSI_MASTER) {
#if (SSI_PRIMARY_UC == ENABLE)
      control_configuration = &ssi_primary_configuration;
#endif
    } else if (ssi_handle == &Driver_SSI_SLAVE) {
#if (SSI_SECONDARY_UC == ENABLE)
      control_configuration = &ssi_secondary_configuration;
#endif
    } else if (ssi_handle == &Driver_SSI_ULP_MASTER) {
#if (SSI_ULP_PRIMARY_UC == ENABLE)
      control_configuration = &ssi_ulp_primary_configuration;
#endif
    }
#elif SSI_CONFIG
#ifdef SSI_UC
    control_configuration = &ssi_configuration;
#endif
#endif

    // validate parameters passed in
    if (control_configuration == NULL || ssi_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // validate ssi handle and baud rate
    if (!validate_ssi_handle(ssi_handle) || control_configuration->baud_rate > MAX_BAUDRATE) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // validate control parameters
    status = validate_control_parameters(control_configuration);
    if (status != SL_STATUS_OK) {
      break;
    }

    // bitwise or various parameters such as clock mode, master mode, bit width and slave select mode
    // and pass it(input_mode) into control API.
    input_mode = (control_configuration->clock_mode | ARM_SPI_DATA_BITS(control_configuration->bit_width));
    if (control_configuration->device_mode == SL_SSI_SLAVE_ACTIVE) {
      input_mode |= ARM_SPI_SS_SLAVE_HW;
      input_mode |= control_configuration->device_mode;
    } else if ((control_configuration->transfer_mode != SPI_TRANSFER_MODE_STANDARD)
               && (control_configuration->device_mode == SL_SSI_MASTER_ACTIVE)) {
      input_mode |= ARM_SPI_SS_MASTER_SW;
      input_mode |= SL_SSI_MASTER_ACTIVE;
    } else {
      input_mode |= ARM_SPI_SS_MASTER_HW_OUTPUT;
      input_mode |= SL_SSI_MASTER_ACTIVE;
    }

    // Set transfer mode for master, standard for others
    if (ssi_handle == &Driver_SSI_MASTER) {
      input_mode |= control_configuration->transfer_mode;
    } else {
      input_mode |= SPI_TRANSFER_MODE_STANDARD;
    }

    sl_ssi_set_receive_sample_delay(ssi_handle, control_configuration->receive_sample_delay);
    error_status = ((sl_ssi_driver_t *)ssi_handle)->Control(input_mode, control_configuration->baud_rate);
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/**
 * @brief       Configure the SPI command phase for dual or quad mode operations.
 *
 * @param[in]   ssi_handle  Pointer to the SSI driver handle (only master mode supported)
 * @param[in]   inst_len    Instruction length
 * @param[in]   addr_len    Address length
 * @param[in]   spi_frf     SPI frame format (SSI_FRF_STANDARD, SSI_FRF_DUAL, SSI_FRF_QUAD)
 * @param[in]   xfer_type   Transfer type (instruction and address phase configuration)
 * @return      sl_status_t SL_STATUS_OK if configuration was successful, 
 *                          error code otherwise (SL_STATUS_INVALID_PARAMETER, etc.)
 */
sl_status_t sl_si91x_ssi_command_config(sl_ssi_handle_t ssi_handle,
                                        sl_ssi_inst_len_t inst_len,
                                        sl_ssi_addr_len_t addr_len,
                                        sl_ssi_frf_t spi_frf,
                                        sl_ssi_xfer_type_t xfer_type)
{
  sl_status_t status   = SL_STATUS_OK;
  int32_t error_status = 0;
  do {
    // validate input arguments
    if (ssi_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // validate ssi handle
    if (!validate_ssi_handle(ssi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Only support master mode for command API
    if (ssi_handle != &Driver_SSI_MASTER) {
      status = SL_STATUS_INVALID_HANDLE;
      break;
    }
    // Validate command parameters
    if (spi_frf >= SSI_FRF_LAST || inst_len > SSI_INST_LEN_16_BITS || addr_len > SSI_ADDR_LEN_60_BITS
        || inst_len == SSI_INST_LEN_0_BITS || xfer_type > SSI_XFER_TYPE_BOTH_ENH) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    // Call the SSI command API to configure and send the command phase
    error_status = SSI_Command_Configure(inst_len, addr_len, spi_frf, xfer_type);
    status       = convert_arm_to_sl_error_code(error_status);

  } while (false);
  return status;
}

/*******************************************************************************
 * SSI peripheral receive with connected slave. This makes use of half duplex
 *            reception.
 *
 * @param[in] ssi handle
 * @param[in] pointer to Rx data buffer,
 * @param[in] length of input data
 * @return
 * *         returns status - SL_STATUS_OK if received data was
 *           fine, error return code such as SL_STATUS_FAIL otherwise.
 *           ARM errors are converted to SL errors via convert_arm_to_sl_error_code
 *           function.
*******************************************************************************/
sl_status_t sl_si91x_ssi_receive_data(sl_ssi_handle_t ssi_handle, void *data, uint32_t data_length)
{
  sl_status_t status;
  int32_t error_status;
  do {
    // validate input arguments
    if (data == NULL || ssi_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // validate data length and ssi handle
    if (!data_length || !validate_ssi_handle(ssi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (ssi_handle != &Driver_SSI_SLAVE) {
      // Assert the slave select line for the connected slave
      error_status = ((sl_ssi_driver_t *)ssi_handle)->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
      status       = convert_arm_to_sl_error_code(error_status);
      if (status != SL_STATUS_OK) {
        break;
      }
    }
    sl_si91x_ssi_set_fifo_threshold(ssi_handle);
    error_status = ((sl_ssi_driver_t *)ssi_handle)->Receive(data, data_length);
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/**
 * @brief       Receive data over SSI in dual or quad mode from connected slave.
 *
 * @details     This function configures SSI Primary in dual/quad mode, sends the instruction
 *              and address phase, then receives the specified amount of data.
 *              It supports standard, dual and quad SPI modes depending on the
 *              previously configured transfer mode.
 *
 * @param[in]   ssi_handle   Pointer to the SSI driver handle (must be master)
 * @param[out]  data         Pointer to buffer for received data
 * @param[in]   data_length  Number of bytes to receive
 * @param[in]   instruction  Command/instruction byte
 * @param[in]   address      Memory address to read from
 * @param[in]   wait_cycles  Number of dummy/wait cycles between address and data phases 
 * @note        The SPI frame format (standard/dual/quad) must be configured first
 *              by calling sl_si91x_ssi_command_config()
 *
 * @return      sl_status_t  SL_STATUS_OK if successful, error code otherwise.
 */
sl_status_t sl_si91x_ssi_receive_command_data(sl_ssi_handle_t ssi_handle,
                                              void *data,
                                              uint32_t data_length,
                                              uint8_t instruction,
                                              uint32_t address,
                                              uint8_t wait_cycles)
{
  sl_status_t status = SL_STATUS_OK;
  int32_t error_status;

  // Validate input arguments
  if ((data == NULL) || (ssi_handle == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }

  // Validate data length and ssi handle
  if ((!validate_ssi_handle(ssi_handle)) || (!data_length)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Only master mode supports advanced commands
  if (&Driver_SSI_MASTER != ssi_handle) {
    return SL_STATUS_INVALID_HANDLE;
  }

  // Assert the slave select line for the connected slave
  error_status = ((sl_ssi_driver_t *)ssi_handle)->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
  if ((status = convert_arm_to_sl_error_code(error_status)) != SL_STATUS_OK) {
    return status;
  }

  sl_si91x_ssi_set_fifo_threshold(ssi_handle);
  error_status = SSI_MASTER_Receive_Command_Data(data, data_length, instruction, address, wait_cycles);
  status       = convert_arm_to_sl_error_code(error_status);
  return status;
}

/*******************************************************************************
 * SSI peripheral send with connected device. This makes use of half duplex
 *          transmission.
 *
 * @param[in] ssi handle
 * @param[in] pointer to Tx data buffer
 * @param[in] length of data
 * @return   returns status - SL_STATUS_OK if data sent was
 *           fine, error return code such as SL_STATUS_FAIL otherwise.
 *           ARM errors are converted to SL errors via convert_arm_to_sl_error_code
 *           function.
*******************************************************************************/
sl_status_t sl_si91x_ssi_send_data(sl_ssi_handle_t ssi_handle, const void *data, uint32_t data_length)
{
  sl_status_t status;
  int32_t error_status;
  do {
    // validate input arguments
    if (data == NULL || ssi_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (!data_length || !validate_ssi_handle(ssi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    if (ssi_handle != &Driver_SSI_SLAVE) {
      // Assert the slave select line for the connected slave
      error_status = ((sl_ssi_driver_t *)ssi_handle)->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
      status       = convert_arm_to_sl_error_code(error_status);
      if (status != SL_STATUS_OK) {
        break;
      }
    }
    sl_si91x_ssi_set_fifo_threshold(ssi_handle);
    error_status = ((sl_ssi_driver_t *)ssi_handle)->Send(data, data_length);
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/**
 * @brief       Send data using dual or quad SPI mode with command and address phases.
 *
 * @details     This function configures SSI in dual/quad mode, sends the instruction
 *              and address phase, then Sends the data using the configured frame format.
 *              It supports standard, dual, or quad SPI modes depending on the
 *              previously configured transfer mode.
 *
 * @param[in]   ssi_handle   Pointer to the SSI driver handle (must be master mode)
 * @param[in]   data         Pointer to buffer containing data to send
 * @param[in]   data_length  Number of bytes to send
 * @param[in]   instruction  Command/instruction byte to send
 * @param[in]   address      Memory address to write
 *
 * @return      sl_status_t  SL_STATUS_OK if successful, error code otherwise
 * 
 * @note        The SPI frame format (standard/dual/quad) must be configured first
 *              by calling sl_si91x_ssi_command_config()
 */
sl_status_t sl_si91x_ssi_send_command_data(sl_ssi_handle_t ssi_handle,
                                           void *data,
                                           uint32_t data_length,
                                           uint8_t instruction,
                                           uint32_t address)
{
  sl_status_t status;
  int32_t error_status;
  do {
    // Validate input arguments
    if (data == NULL || ssi_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }

    // Validate data length and ssi handle
    if (!data_length || !validate_ssi_handle(ssi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    // Only master mode supports advanced commands
    if (ssi_handle != &Driver_SSI_MASTER) {
      status = SL_STATUS_INVALID_HANDLE;
      break;
    }

    // Assert the slave select line for the connected slave
    error_status = ((sl_ssi_driver_t *)ssi_handle)->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
    status       = convert_arm_to_sl_error_code(error_status);
    if (status != SL_STATUS_OK) {
      break;
    }

    sl_si91x_ssi_set_fifo_threshold(ssi_handle);
    error_status = SSI_MASTER_Send_Command_Data(data, data_length, instruction, address);
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * To run SSI peripheral data transfer (bi-directional full duplex) with connected device.
 *
 * @param[in] ssi handle
 * @param[in] pointer to tx buffer
 * @param[in] pointer to rx buffer
 * @param[in] data length
 * @return
 * *         returns status - SL_STATUS_OK if data was
 *           sent and received OK, error return code such as SL_STATUS_FAIL otherwise.
 *           Data is sent out from Master on the MOSI line, the slave can simultaneously
 *           send data on MISO line(full duplex). ARM errors are converted to SL errors
 *           via convert_arm_to_sl_error_code function.
*******************************************************************************/
sl_status_t sl_si91x_ssi_transfer_data(sl_ssi_handle_t ssi_handle,
                                       const void *data_out,
                                       void *data_in,
                                       uint32_t data_length)
{
  sl_status_t status;
  int32_t error_status;
  do {
    // validate input arguments
    if ((data_out == NULL) || (data_in == NULL) || ssi_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (!validate_ssi_handle(ssi_handle) || !data_length) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (ssi_handle != &Driver_SSI_SLAVE) {
      // Assert the slave select line for the connected slave
      error_status = ((sl_ssi_driver_t *)ssi_handle)->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
      status       = convert_arm_to_sl_error_code(error_status);
      if (status != SL_STATUS_OK) {
        break;
      }
    }
    sl_si91x_ssi_set_fifo_threshold(ssi_handle);
    error_status = ((sl_ssi_driver_t *)ssi_handle)->Transfer(data_out, data_in, data_length);
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * To set the Receive Data (rxd) Sample Delay. This is used to delay the sample
 * of the rxd input signal.
 * NOTE: If this register is programmed with a value that exceeds the depth of
 *       the internal shift registers (63), a zero delay will be applied to the
 *       rxd sample.
 *
 * @param[in] ssi handle
 * @param[in] sample_delay
 * @return    none
*******************************************************************************/
static void sl_ssi_set_receive_sample_delay(sl_ssi_handle_t ssi_handle, uint32_t sample_delay)
{
  do {
    if (ssi_handle == &Driver_SSI_MASTER) {
      SSI_SetRxSamplingDelay(SPI_MASTER_MODE, sample_delay);
      break;
    }
    if (ssi_handle == &Driver_SSI_SLAVE) {
      SSI_SetRxSamplingDelay(SPI_SLAVE_MODE, sample_delay);
      break;
    }
    if (ssi_handle == &Driver_SSI_ULP_MASTER) {
      SSI_SetRxSamplingDelay(SPI_ULP_MASTER_MODE, sample_delay);
      break;
    }
  } while (false);
}

/*******************************************************************************
 * This API to set the Transmit FIFO Threshold and Receive FIFO Threshold values
 * Controls the level of entries at which the transmit/receive FIFO controller
 * triggers an interrupt.
 * NOTE: If this field is set to a value greater than or equal  to the depth of
 *       the FIFO, this field is not written and retains its current value.
 * @param[in] ssi handle
 * @return    none
*******************************************************************************/
static void sl_si91x_ssi_set_fifo_threshold(sl_ssi_handle_t ssi_handle)
{
  do {
    if (ssi_handle == &Driver_SSI_MASTER) {
      SSI_SetFifoThreshold(SPI_MASTER_MODE);
      break;
    }
    if (ssi_handle == &Driver_SSI_SLAVE) {
      SSI_SetFifoThreshold(SPI_SLAVE_MODE);
      break;
    }
    if (ssi_handle == &Driver_SSI_ULP_MASTER) {
      SSI_SetFifoThreshold(SPI_ULP_MASTER_MODE);
      break;
    }
  } while (false);
}

/*******************************************************************************
 * Get SSI peripheral Rx transferred data count with connected device.
 *
 * @param[in] ssi handle
 * @return
 * *     returns Rx transferred data count
*******************************************************************************/
uint32_t sl_si91x_ssi_get_rx_data_count(sl_ssi_handle_t ssi_handle)
{
  return ((sl_ssi_driver_t *)ssi_handle)->GetDataCount();
}

/*******************************************************************************
 * Get SSI peripheral Tx transferred data count with connected device.
 *
 * @param[in] ssi handle
 * @return
 * *     returns Rx transferred data count
*******************************************************************************/
uint32_t sl_si91x_ssi_get_tx_data_count(sl_ssi_handle_t ssi_handle)
{
  uint32_t tx_count = 0;
  do {
    if (ssi_handle == &Driver_SSI_MASTER) {
      tx_count = SSI_GetTxCount(SPI_MASTER_MODE);
      break;
    }
    if (ssi_handle == &Driver_SSI_SLAVE) {
      tx_count = SSI_GetTxCount(SPI_SLAVE_MODE);
      break;
    }
    if (ssi_handle == &Driver_SSI_ULP_MASTER) {
      tx_count = SSI_GetTxCount(SPI_ULP_MASTER_MODE);
      break;
    }
  } while (false);
  return tx_count;
}

/*******************************************************************************
 * To get SSI status
 *
 * @param[in] SSI handle
 * @return
 * *         returns status as a bit field - busy, data lost or mode fault
*******************************************************************************/
sl_ssi_status_t sl_si91x_ssi_get_status(sl_ssi_handle_t ssi_handle)
{
  return ((sl_ssi_driver_t *)ssi_handle)->GetStatus();
}

/*******************************************************************************
 * To get the driver version
 *
 * @param[in] None
 * @return
 * *         returns driver version
*******************************************************************************/
sl_ssi_version_t sl_si91x_ssi_get_version(void)
{
  sl_ssi_version_t version_struct;
  version_struct.release = SSI_RELEASE_VERSION;
  version_struct.major   = SSI_SQA_VERSION;
  version_struct.minor   = SSI_DEV_VERSION;
  return version_struct;
}

/*******************************************************************************
 * To register the user event callback
 * It registers the callback, i.e., stores the callback function address
 * and pass to the variable that is called in Interrupt Handler.
 * If another callback is registered without unregistering previous callback then, it
 * returns an error code, so it is mandatory to unregister the callback before registering
 * another callback.
 * It will returns error if any callback is already registered.
 ******************************************************************************/
sl_status_t sl_si91x_ssi_register_event_callback(sl_ssi_handle_t ssi_handle, sl_ssi_signal_event_t callback_event)
{
  sl_status_t status;
  do {
    // Validates the null pointer, if true returns error code
    if ((ssi_handle == NULL) || (callback_event == NULL)) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validate the SSI handle address, if incorrect returns error code
    if (!validate_ssi_handle(ssi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Getting the instance number from the input handle
    uint8_t instance = convert_handle_to_instance(ssi_handle);
    // To validate the function pointer if the parameters is not NULL then, it
    // returns an error code
    if (user_callback[instance] != NULL) {
      status = SL_STATUS_BUSY;
      break;
    }
    // User callback address is passed to the static variable which is called at the time of
    // interrupt
    user_callback[instance] = callback_event;
    // Returns SL_STATUS_OK if callback is successfully registered
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * To unregister the user event callback
 * It unregisters the callback, i.e., clear the callback function address
 * and pass NULL value to the variable
 ******************************************************************************/
void sl_si91x_ssi_unregister_event_callback(void)
{
  // Pass the NULL value to the static variable which is called at the time of
  // interrupt.
  // It is further validated in register callback API.
  for (int i = 0; i < MAX_INSTANCE; i++) {
    // Clearing all the three instance callbacks
    user_callback[i] = NULL;
  }
}

/*******************************************************************************
 * To unregister the user event callback on a per-instance basis.
 * Get the instance, 0,1,2 for master, slave and ulp master respectively
 * and NULL is passed to the callback function pointer of instance.
*******************************************************************************/
void sl_si91x_ssi_per_instance_unregister_event_callback(sl_ssi_handle_t ssi_handle)
{
  // Getting the instance number from the input handle
  uint8_t instance = convert_handle_to_instance(ssi_handle);
  // NULL is passed to the callback function pointer of instance.
  user_callback[instance] = NULL;
}

/*******************************************************************************
 * To fetch the clock division factor
 * @param[in] ssi_handle
 * @return
 * *      It reads the register which holds the clock division factor
 * *      and return the value of in 32 bit unsigned integer.
 ******************************************************************************/
uint32_t sl_si91x_ssi_get_clock_division_factor(sl_ssi_handle_t ssi_handle)
{
  uint32_t division_factor = 0;
  do {
    if (ssi_handle == &Driver_SSI_MASTER) {
      division_factor = SSI_GetClockDivisionFactor(SPI_MASTER_MODE);
      break;
    }
    if (ssi_handle == &Driver_SSI_SLAVE) {
      division_factor = SSI_GetClockDivisionFactor(SPI_SLAVE_MODE);
      break;
    }
    if (ssi_handle == &Driver_SSI_ULP_MASTER) {
      division_factor = SSI_GetClockDivisionFactor(SPI_ULP_MASTER_MODE);
      break;
    }
  } while (false);
  return division_factor;
}

/*******************************************************************************
 * To fetch the frame length i.e., bit width
 * @param[in] ssi_handle
 * @return
 * *      It reads the register which holds the frame length
 * *      and return the value of in 32 bit unsigned integer.
 ******************************************************************************/
uint32_t sl_si91x_ssi_get_frame_length(sl_ssi_handle_t ssi_handle)
{
  uint32_t frame_length = 0;
  do {
    if (ssi_handle == &Driver_SSI_MASTER) {
      frame_length = SSI_GetFrameLength(SPI_MASTER_MODE);
      break;
    }
    if (ssi_handle == &Driver_SSI_SLAVE) {
      frame_length = SSI_GetFrameLength(SPI_SLAVE_MODE);
      break;
    }
    if (ssi_handle == &Driver_SSI_ULP_MASTER) {
      frame_length = SSI_GetFrameLength(SPI_ULP_MASTER_MODE);
      break;
    }
  } while (false);
  return frame_length;
}

/*******************************************************************************
 * To set the frame length i.e., bit width
 * @param[in] ssi_handle  Handle to the SSI instance
 * @param[in] frame_length Frame length (bit width) to set (4-32 bits)
 * @return
 * *      SL_STATUS_OK if successful, error code otherwise
 ******************************************************************************/
sl_status_t sl_si91x_ssi_set_frame_length(sl_ssi_handle_t ssi_handle, uint8_t frame_length)
{
  sl_status_t status = SL_STATUS_OK;
  int32_t result     = 0;
  do {
    // Validate input parameters
    if (ssi_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }

    if (!validate_ssi_handle(ssi_handle)) {
      status = SL_STATUS_INVALID_HANDLE;
      break;
    }

    // Validate frame length range (4-32 bits)
    if (frame_length < 4 || frame_length > 32) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    // Set frame length based on SSI instance
    if (ssi_handle == &Driver_SSI_MASTER) {
      result = SSI_SetFrameLength(SPI_MASTER_MODE, frame_length);
      break;
    }
    if (ssi_handle == &Driver_SSI_SLAVE) {
      result = SSI_SetFrameLength(SPI_SLAVE_MODE, frame_length);
      break;
    }
    if (ssi_handle == &Driver_SSI_ULP_MASTER) {
      result = SSI_SetFrameLength(SPI_ULP_MASTER_MODE, frame_length);
      break;
    }
  } while (false);

  // Convert any error codes from the lower-level API
  if (result != 0 && status == SL_STATUS_OK) {
    status = convert_arm_to_sl_error_code(result);
  }

  return status;
}

/*******************************************************************************
 * To fetch the transfer fifo threshold
 * @param[in] ssi_handle
 * @return
 * *      It reads the register which holds the transfer fifo threshold
 * *      and return the value of in 32 bit unsigned integer.
 ******************************************************************************/
uint32_t sl_si91x_ssi_get_tx_fifo_threshold(sl_ssi_handle_t ssi_handle)
{
  uint32_t tx_fifo_threshold = 0;
  do {
    if (ssi_handle == &Driver_SSI_MASTER) {
      tx_fifo_threshold = SSI_GetTxFifoThreshold(SPI_MASTER_MODE);
      break;
    }
    if (ssi_handle == &Driver_SSI_SLAVE) {
      tx_fifo_threshold = SSI_GetTxFifoThreshold(SPI_SLAVE_MODE);
      break;
    }
    if (ssi_handle == &Driver_SSI_ULP_MASTER) {
      tx_fifo_threshold = SSI_GetTxFifoThreshold(SPI_ULP_MASTER_MODE);
      break;
    }
  } while (false);
  return tx_fifo_threshold;
}

/*******************************************************************************
 * To fetch the receiver fifo threshold
 * @param[in] ssi_handle
 * @return
 * *      It reads the register which holds the receiver fifo threshold
 * *      and return the value of in 32 bit unsigned integer.
 ******************************************************************************/
uint32_t sl_si91x_ssi_get_rx_fifo_threshold(sl_ssi_handle_t ssi_handle)
{
  uint32_t rx_fifo_threshold = 0;
  do {
    if (ssi_handle == &Driver_SSI_MASTER) {
      rx_fifo_threshold = SSI_GetRxFifoThreshold(SPI_MASTER_MODE);
      break;
    }
    if (ssi_handle == &Driver_SSI_SLAVE) {
      rx_fifo_threshold = SSI_GetRxFifoThreshold(SPI_SLAVE_MODE);
      break;
    }
    if (ssi_handle == &Driver_SSI_ULP_MASTER) {
      rx_fifo_threshold = SSI_GetRxFifoThreshold(SPI_ULP_MASTER_MODE);
      break;
    }
  } while (false);
  return rx_fifo_threshold;
}

/*******************************************************************************
 * To fetch the receiver sample delay
 * @param[in] ssi_handle
 * @return
 * *      It reads the register which holds the receiver sample delay
 * *      and return the value of in 32 bit unsigned integer.
 ******************************************************************************/
uint32_t sl_si91x_ssi_get_receiver_sample_delay(sl_ssi_handle_t ssi_handle)
{
  uint32_t rx_sample_delay = 0;
  do {
    if (ssi_handle == &Driver_SSI_MASTER) {
      rx_sample_delay = SSI_GetReceiveSampleDelay(SPI_MASTER_MODE);
      break;
    }
    if (ssi_handle == &Driver_SSI_SLAVE) {
      rx_sample_delay = SSI_GetReceiveSampleDelay(SPI_SLAVE_MODE);
      break;
    }
    if (ssi_handle == &Driver_SSI_ULP_MASTER) {
      rx_sample_delay = SSI_GetReceiveSampleDelay(SPI_ULP_MASTER_MODE);
      break;
    }
  } while (false);
  return rx_sample_delay;
}

/*==============================================*/
/**
 * @fn     void callback_event_handler(uint32_t event)
 * @brief  SSI callback handler
 * @param[in] event SSI transmit and receive events
 * @return   None
 * On event transfer complete, the slave select line
 * for the connected slave is de-asserted
 */

static void callback_event_handler(uint32_t event)
{
  uint8_t ssi_instance   = 0;
  ARM_DRIVER_SPI *handle = NULL;
  // Extracting the instance number from the event variable
  ssi_instance = (uint8_t)(event >> SSI_INSTANCE_BIT);
  if (ssi_instance == SSI_MASTER_INSTANCE) {
    // assigning local handle to the master mode address
    handle = &Driver_SSI_MASTER;
  } else if (ssi_instance == SSI_SLAVE_INSTANCE) {
    // assigning local handle to the slave mode address
    handle = &Driver_SSI_SLAVE;
  } else if (ssi_instance == SSI_ULP_MASTER_INSTANCE) {
    // assigning local handle to the ulp master mode address
    handle = &Driver_SSI_ULP_MASTER;
  }
  // Clearing the instance number to evaluate the event
  event &= SSI_INSTANCE_MASK;
  switch (event) {
    case SSI_EVENT_TRANSFER_COMPLETE:
      // Disabling the slave select line after the transfer complete
      handle->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
      break;
    case SSI_EVENT_DATA_LOST:
      //      Occurs in slave mode when data is requested/sent by master
      //      but send/receive/transfer operation has not been started
      //      and indicates that data is lost. Occurs also in master mode
      //      when driver cannot transfer data fast enough.
      break;
    case SSI_EVENT_MODE_FAULT:
      //      Occurs in master mode when Slave Select is deactivated and
      //      indicates Master Mode Fault.
      break;
    default:
      break;
  }
  // Calling the callback as per the instance
  if (user_callback[ssi_instance] != NULL) {
    user_callback[ssi_instance](event);
  }
}

/*******************************************************************************
 * To validate ARM error code and map to sl_status_t type errors.
 *
 * ARM errors are converted to the SL errors via this function.
 * @param[in] int32_t error code
 * @return
 * *         returns SL_STATUS type errors.
*******************************************************************************/
static sl_status_t convert_arm_to_sl_error_code(int32_t error)
{
  sl_status_t status;
  switch (error) {
    //Driver Errors
    case ARM_DRIVER_OK:
      status = SL_STATUS_OK;
      break;
    case ARM_DRIVER_ERROR:
      status = SL_STATUS_FAIL;
      break;
    case ARM_DRIVER_ERROR_BUSY:
      status = SL_STATUS_BUSY;
      break;
    case ARM_DRIVER_ERROR_TIMEOUT:
      status = SL_STATUS_TIMEOUT;
      break;
    case ARM_DRIVER_ERROR_UNSUPPORTED:
      status = SL_STATUS_NOT_SUPPORTED;
      break;
    case ARM_DRIVER_ERROR_PARAMETER:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ARM_DRIVER_ERROR_SPECIFIC:
      status = SL_STATUS_FAIL;
      break;
    //SPI Errors
    case ARM_SPI_ERROR_MODE:
      status = SL_STATUS_INVALID_MODE;
      break;
    case ARM_SPI_ERROR_FRAME_FORMAT:
      status = SL_STATUS_INVALID_TYPE;
      break;
    case ARM_SPI_ERROR_DATA_BITS:
      status = SL_STATUS_INVALID_RANGE;
      break;
    case ARM_SPI_ERROR_BIT_ORDER:
      status = SL_STATUS_INVALID_RANGE;
      break;
    case ARM_SPI_ERROR_SS_MODE:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    default:
      status = SL_STATUS_FAIL;
      break;
  }
  return status;
}

/*******************************************************************************
 * internal function which validates the control structure passed into master
 * control function below
 *
 * @param[in] pointer to control config struct.
 * @return    status 0 if successful, else error code.
 *
 *         SL_STATUS_OK (0x0000) - Success, otherwise SL_STATUS_INVALID_PARAMETER,
 *         SL_STATUS_NULL_POINTER fail error code
*******************************************************************************/
static sl_status_t validate_control_parameters(sl_ssi_control_config_t *control_configuration)
{
  sl_status_t status = SL_STATUS_OK;
  do {
    if (control_configuration == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }

    if (control_configuration->clock_mode > SL_SSI_PERIPHERAL_MICROWIRE) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if ((control_configuration->bit_width < MIN_BIT_WIDTH) || (control_configuration->bit_width > MAX_BIT_WIDTH)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Maximum bit rate that ULP_SSI can support.
    if (control_configuration->device_mode == SL_SSI_ULP_MASTER_ACTIVE) {
      if (control_configuration->baud_rate > ULP_SSI_MAX_BAUDRATE) {
        status = SL_STATUS_INVALID_PARAMETER;
        break;
      }
    } else {
      if (control_configuration->baud_rate > MAX_BAUDRATE) {
        status = SL_STATUS_INVALID_PARAMETER;
        break;
      }
    }
    if (control_configuration->device_mode >= SL_SSI_INSTANCE_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * To get the handle of SSI structure.
 * It should be called before calling any SSI function to get the appropriate handle.
 * It updates the address of the ssi_handle passed by the user.
 ******************************************************************************/
static sl_status_t get_ssi_handle(sl_ssi_instance_t instance, sl_ssi_handle_t *ssi_handle)
{
  sl_status_t status = SL_STATUS_OK;
  do {
    if (instance >= SL_SSI_INSTANCE_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (ssi_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (instance == SL_SSI_MASTER_ACTIVE) {
      *ssi_handle = &Driver_SSI_MASTER;
      status      = SL_STATUS_OK;
      break;
    }
    if (instance == SL_SSI_SLAVE_ACTIVE) {
      *ssi_handle = &Driver_SSI_SLAVE;
      status      = SL_STATUS_OK;
      break;
    }
    if (instance == SL_SSI_ULP_MASTER_ACTIVE) {
      *ssi_handle = &Driver_SSI_ULP_MASTER;
      status      = SL_STATUS_OK;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * internal function which validates the ssi handle passed in
 * *
 * @param[in] ssi handle.
 * @return    status true if successful, else false.
 *
*******************************************************************************/
static boolean_t validate_ssi_handle(sl_ssi_handle_t ssi_handle)
{
  boolean_t status;
  if (ssi_handle == &Driver_SSI_MASTER || ssi_handle == &Driver_SSI_SLAVE || ssi_handle == &Driver_SSI_ULP_MASTER) {
    status = true;
  } else {
    status = false;
  }
  return status;
}

/*******************************************************************************
 * Internal function to convert ssi handle to the instance number.
 *
 * @param[in] ssi handle.
 * @return    instance, 0,1,2 for master, slave and ulp master respectively
 *
*******************************************************************************/
static uint8_t convert_handle_to_instance(sl_ssi_handle_t ssi_handle)
{
  uint8_t instance = 0;
  do {
    if (ssi_handle == &Driver_SSI_MASTER) {
      instance = SSI_MASTER_INSTANCE;
      break;
    }
    if (ssi_handle == &Driver_SSI_SLAVE) {
      instance = SSI_SLAVE_INSTANCE;
      break;
    }
    if (ssi_handle == &Driver_SSI_ULP_MASTER) {
      instance = SSI_ULP_MASTER_INSTANCE;
      break;
    }
  } while (false);
  return instance;
}
