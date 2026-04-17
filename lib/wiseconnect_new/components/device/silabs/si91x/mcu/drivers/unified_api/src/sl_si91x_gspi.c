/******************************************************************************
* @file sl_si91x_gspi.c
* @brief GSPI API implementation
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

#include "rsi_gspi.h"
#include "rsi_debug.h"
#include "rsi_rom_clks.h"
#include "clock_update.h"
#include "sl_si91x_gspi.h"
#include "sl_si91x_gspi_config.h"
#include "rsi_rom_clks.h"

/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/
#define MAX_BITRATE               116000000 // Maximum bitrate that GSPI supports
#define SOC_PLL_REF_FREQUENCY     40000000  // SOC PLL reference frequency (same for all peripherals)
#define MINIMUM_BITRATE           500000    // Minimum bitrate for soc pll configuration
#define MINIMUM_SOC_PLL_FREQUENCY 1000000   // Minimum soc pll frequency
#define MANUAL_LOCK               1         // Manual lock enable
#define BYPASS_MANUAL_LOCK        1         // Bypass manual lock enable
#define NO_VALUE                  0         // No value for function argument
#define MAX_READ_SIZE             32000     // Maximum size for receiving data
#define MIN_DATA_LENGTH           0         // Minimum data length
#define MIN_BIT_WIDTH             0         // Minimum bit width
#define MAX_BIT_WIDTH             16        // Maximum bit width
#define GSPI_RELEASE_VERSION      0         // GSPI Release version
#define GSPI_SQA_VERSION          0         // GSPI SQA version
#define GSPI_DEV_VERSION          2         // GSPI Developer version
#define ODD_DIV_ENABLE            0         // Odd division enable for QSPI clock
#define SWALLO_ENABLE             0         // Swallo enable for QSPI clock

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static sl_gspi_signal_event_t user_callback = NULL;
static const void *local_gspi_handle        = NULL;
extern sl_gspi_driver_t Driver_GSPI_MASTER;
static volatile boolean_t dummy_flag = false;
/*******************************************************************************
 ***************************  Local Types  ********************************
 ******************************************************************************/

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
static sl_status_t convert_arm_to_sl_error_code(int32_t error);
static sl_status_t validate_control_parameters(sl_gspi_control_config_t *control_configuration);
static sl_status_t set_slave_gpio_state(sl_gspi_handle_t gspi_handle, boolean_t value);
static sl_status_t get_gspi_handle(sl_gspi_instance_t instance, sl_gspi_handle_t *gspi_handle);
static boolean_t validate_gspi_handle(sl_gspi_handle_t gspi_handle);
static sl_status_t sli_si91x_gspi_configure_power_mode(sl_gspi_handle_t gspi_handle, sl_gspi_power_state_t state);
static void callback_event_handler(uint32_t event);

/*******************************************************************************
 **********************  Local Function Definition****************************
 ******************************************************************************/

/*******************************************************************************
***********************  Global function Definitions *************************
 ******************************************************************************/
/*******************************************************************************
 * To set the clock for GSPI peripheral
 * Validates the clock frequency via validate_clock_parameters function
 * If the clock frequency is in range, it activates the clock for GSPI peripheral
 * RSI errors are converted to the SL errors via convert_rsi_to_sl_error_code
 * function.
 ******************************************************************************/
sl_status_t sl_si91x_gspi_configure_clock(sl_gspi_clock_config_t *clock_configuration)
{
  sl_status_t status = SL_STATUS_OK;
  do {
    // To validate the structure pointer, if the parameters is NULL, it
    // returns an error code
    if (clock_configuration == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * Initialization of GSPI
 * It should be called after the configure clock API.
 * It enables the GSPI peripheral.
 * If DMA is enabled then it enables the DMA also.
 * Call this function for fetching the GSPI handle according to the instance provided.
 * It expects address of void pointer and GSPI instance.
 * If GSPI is already enabled, it will return busy until it is disabled using deinit().
 ******************************************************************************/
sl_status_t sl_si91x_gspi_init(sl_gspi_instance_t instance, sl_gspi_handle_t *gspi_handle)
{
  sl_status_t status;
  int32_t error_status;
  sl_gspi_handle_t gspi_temp_handle;
  do {
    // To validate status of GSPI, if already enabled, returns error code.
    if (GSPI_GetInitState() == ENABLE) {
      status = SL_STATUS_BUSY;
      break;
    }
    if (gspi_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // To validate the structure pointer and gspi handle, if the parameters is NULL, it
    // returns an error code
    if (*gspi_handle != NULL) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Static function to get the GSPI handle according to the GSPI instance provided.
    status = get_gspi_handle(instance, &gspi_temp_handle);
    // If the status is not equal to SL_STATUS_OK, returns error code.
    if (status != SL_STATUS_OK) {
      break;
    }
    *gspi_handle = gspi_temp_handle;
    // Validate the GSPI handle address, if incorrect returns error code
    if (*gspi_handle != gspi_temp_handle) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // CMSIS API for initialization is called with the function address of
    // callback function, and the arm error code returned from the API is
    // converted to SL error code via convert_arm_to_sl_error_code function.
    error_status = ((sl_gspi_driver_t *)gspi_temp_handle)->Initialize(callback_event_handler);
    status       = convert_arm_to_sl_error_code(error_status);
    if (status != SL_STATUS_OK) {
      return status;
    }
    // Configuration of power mode
    status = sli_si91x_gspi_configure_power_mode(gspi_temp_handle, SL_GSPI_FULL_POWER);
  } while (false);
  return status;
}

/*******************************************************************************
 * Un-initialization of GSPI
 * It un-initialize GSPI as well as efuse peripheral.
 * To enable debug, DEBUGINIT() needs to be called after using this API.
 ******************************************************************************/
sl_status_t sl_si91x_gspi_deinit(sl_gspi_handle_t gspi_handle)
{
  sl_status_t status;
  int32_t error_status;
  do {
    // To validate the gspi handle, if the parameters is NULL, it
    // returns an error code
    if (gspi_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validate the GSPI handle address, if incorrect returns error code
    if (!validate_gspi_handle(gspi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Unregistering the GSPI event callback.
    sl_si91x_gspi_unregister_event_callback();
    // CMSIS API for un-initialization is called and the arm error code returned from
    // the API is converted to SL error code via convert_arm_to_sl_error_code function.
    error_status = ((sl_gspi_driver_t *)gspi_handle)->Uninitialize();
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * To change the power mode of GSPI
 * The following two modes are supported
 * GSPI_POWER_OFF
 * GSPI_FULL_POWER
 * In power off mode, debug is disabled, to enable the debug DEBUGINIT() needs
 * to be called.
 ******************************************************************************/
static sl_status_t sli_si91x_gspi_configure_power_mode(sl_gspi_handle_t gspi_handle, sl_gspi_power_state_t state)
{
  sl_status_t status;
  int32_t error_status;
  do {
    // Validates the null pointer, if results true,
    // returns error code.
    if (gspi_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validates the power mode, if it is greater than POWER_MODE_LAST, returns error code.
    if (state >= SL_GSPI_POWER_MODE_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validate the GSPI handle address, if incorrect returns error code
    if (!validate_gspi_handle(gspi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // CMSIS API for power mode config is called and the arm error code returned from
    // the API is converted to SL error code via convert_arm_to_sl_error_code function.
    error_status = ((sl_gspi_driver_t *)gspi_handle)->PowerControl((ARM_POWER_STATE)state);
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * To control and configure the GSPI
 * It configures GSPI mode, Bit width (frame length), master mode, bitrate, 
 * swap read, swap write and slave select mode.
 * - GSPI Mode, Value: GSPI_MODE_0 / GSPI_MODE_3
 * - Bit width (frame length), Value: 1 to 16
 * - Master Mode, Value: GSPI_MASTER_ACTIVE / GSPI_MASTER_INACTIVE
 * - Bitrate, Value: Less than 40000000
 * - Swap Read, Value: ENABLE / DISABLE
 * - Swap Write, Value: ENABLE / DISABLE
 * - Slave Select Mode, Value: GSPI_MASTER_HW_OUTPUT / GSPI_MASTER_SW
 ******************************************************************************/
sl_status_t sl_si91x_gspi_set_configuration(sl_gspi_handle_t gspi_handle,
                                            sl_gspi_control_config_t *control_configuration)
{
  sl_status_t status;
  int32_t error_status;
  uint32_t input_mode;
  /* GSPI_UC is defined by default. when this macro (GSPI_UC) is defined, peripheral
   * configuration is directly taken from the configuration set in the universal configuration (UC).
   * if the application requires the configuration to be changed in run-time, undefined this macro
   * and change the peripheral configuration through the sl_si91x_gspi_set_configuration API.
   */
#if (GSPI_UC == 1)
  control_configuration = &gspi_configuration;
#endif
  do {
    // To validate the structure pointer, if the parameters is NULL, it
    // returns an error code
    if ((control_configuration == NULL) || (gspi_handle == NULL)) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validate the GSPI handle address, if incorrect returns error code
    if (!validate_gspi_handle(gspi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // By doing 'OR' operations and validations, it builds one uint32_t integer which is
    // passed inside the contol API.
    status = validate_control_parameters(control_configuration);
    // If the status is not equal to SL_STATUS_OK, returns error code.
    if (status != SL_STATUS_OK) {
      break;
    }
    if (control_configuration->bit_width == MAX_BIT_WIDTH) {
      // For 16 data width, it is required to set the data width as 0.
      control_configuration->bit_width = 0;
    }
    input_mode = (control_configuration->clock_mode | SL_GSPI_MASTER_ACTIVE | control_configuration->slave_select_mode
                  | ARM_SPI_DATA_BITS(control_configuration->bit_width));
    // CMSIS API for GSPI control is called and the arm error code returned from
    // the API is converted to SL error code via convert_arm_to_sl_error_code function.
    error_status = ((sl_gspi_driver_t *)gspi_handle)->Control(input_mode, control_configuration->bitrate);
    status       = convert_arm_to_sl_error_code(error_status);
    if (status != SL_STATUS_OK) {
      break;
    }
    // Updating the registers according to the value of swap read and write.
    error_status = GSPI_SwapReadWriteByte(control_configuration->swap_read, control_configuration->swap_write);
    status       = convert_arm_to_sl_error_code(error_status);
    // This is used to perform a dummy write operation on the GSPI.
    // In the first SPI transaction after out of reset, two sampling clock cycles are taken to synchronize the reset.
    // This dummy write is particularly useful to ensure that the sampling FIFOs are correctly synchronized after a reset.
    if (!dummy_flag) {
      dummy_flag = true;
      GSPI_WriteDummyByte();
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * Receives the data from the slave device
 * It stores the received data in the data buffer
 * It takes two arguments,
 *   - data: pointer to the data buffer which stores the data that is received
 *   - data_legth: The number of data items to be received
 * It stores the received data in the data_in buffer
 ******************************************************************************/
sl_status_t sl_si91x_gspi_receive_data(sl_gspi_handle_t gspi_handle, void *data, uint32_t data_length)
{
  sl_status_t status;
  int32_t error_status;
  do {
    // To validate pointers, if the parameters is NULL, it returns an error code
    if ((data == NULL) || (gspi_handle == NULL)) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validate the GSPI handle address, if incorrect returns error code
    if (!validate_gspi_handle(gspi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // If data_length is not in range, it returns an error code
    if ((data_length == MIN_DATA_LENGTH) || (data_length > MAX_READ_SIZE)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // It calls the API to enable to slave gpio.
    status = set_slave_gpio_state(gspi_handle, ENABLE);
    if (status != SL_STATUS_OK) {
      break;
    }
    // CMSIS API for receiving data is called and the arm error code returned from
    // the API is converted to SL error code via convert_arm_to_sl_error_code function.
    // It updates the data variable with the data received.
    error_status = ((sl_gspi_driver_t *)gspi_handle)->Receive(data, data_length);
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * Receives data from the slave device in blocking mode
 * It stores the received data in the data buffer.
 * Arguments:
 *   - gspi_handle: GSPI handle
 *   - data: pointer to the data buffer which stores the data that is received
 *   - data_length: The number of data items to be received
 *   - timeout: timeout value for the receive operation
 * Returns SL_STATUS_OK on success or appropriate error code.
 ******************************************************************************/
sl_status_t sl_si91x_gspi_receive_data_blocking(sl_gspi_handle_t gspi_handle,
                                                void *data,
                                                uint32_t data_length,
                                                uint32_t timeout)
{
  sl_status_t status;
  int32_t error_status;
  do {
    // Validate pointers, if any parameter is NULL, return error code
    if ((data == NULL) || (gspi_handle == NULL)) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validate the GSPI handle address, if incorrect return error code
    if (!validate_gspi_handle(gspi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // If data_length is not in range, return error code
    if ((data_length == MIN_DATA_LENGTH) || (data_length > MAX_READ_SIZE)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Enable slave select GPIO
    status = GSPI_MASTER_SetSlaveSelectGPIOState(ENABLE);
    if (status != SL_STATUS_OK) {
      break;
    }
    // Receive data in blocking mode and convert ARM error code to SL error code
    error_status = GSPI_MASTER_Receive_Blocking(data, data_length, timeout);
    status       = convert_arm_to_sl_error_code(error_status);
    // Disable slave select GPIO
    if (GSPI_MASTER_SetSlaveSelectGPIOState(DISABLE) != SL_STATUS_OK) {
      status = SL_STATUS_FAIL;
      break;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * Sends the data to the slave device
 * It takes two arguments,
 *   - data: pointer to the data buffer which stores the data that needs to
 *           be sent
 *   - data_legth: The number of data items to be send
 ******************************************************************************/
sl_status_t sl_si91x_gspi_send_data(sl_gspi_handle_t gspi_handle, const void *data, uint32_t data_length)
{
  sl_status_t status;
  int32_t error_status;
  do {
    // To validate pointers, if the parameters is NULL, it returns an error code
    if ((data == NULL) || (gspi_handle == NULL)) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validate the GSPI handle address, if incorrect returns error code
    if (!validate_gspi_handle(gspi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validates the data length parameter, if zero then returns error code
    if (data_length == MIN_DATA_LENGTH) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // It calls the API to enable to slave gpio.
    status = set_slave_gpio_state(gspi_handle, ENABLE);
    if (status != SL_STATUS_OK) {
      break;
    }
    // CMSIS API for sending data is called and the arm error code returned from
    // the API is converted to SL error code via convert_arm_to_sl_error_code function.
    error_status = ((sl_gspi_driver_t *)gspi_handle)->Send(data, data_length);
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * Sends data to the slave device in blocking mode.
 * Arguments:
 *   - gspi_handle: GSPI handle
 *   - data: pointer to the data buffer to be sent
 *   - data_length: number of data items to send
 *   - timeout: timeout value for the send operation
 * Returns SL_STATUS_OK on success or appropriate error code.
 ******************************************************************************/
sl_status_t sl_si91x_gspi_send_data_blocking(sl_gspi_handle_t gspi_handle,
                                             const void *data,
                                             uint32_t data_length,
                                             uint32_t timeout)
{
  sl_status_t status;
  int32_t error_status;
  do {
    // Validate pointers, if any parameter is NULL, return error code
    if ((data == NULL) || (gspi_handle == NULL)) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validate the GSPI handle address, if incorrect return error code
    if (!validate_gspi_handle(gspi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validate data_length, if zero return error code
    if (data_length == MIN_DATA_LENGTH) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Enable slave select GPIO
    status = GSPI_MASTER_SetSlaveSelectGPIOState(ENABLE);
    if (status != SL_STATUS_OK) {
      break;
    }
    // Send data in blocking mode and convert ARM error code to SL error code
    error_status = GSPI_MASTER_Send_Blocking(data, data_length, timeout);
    status       = convert_arm_to_sl_error_code(error_status);
    // Disable slave select GPIO
    if (GSPI_MASTER_SetSlaveSelectGPIOState(DISABLE) != SL_STATUS_OK) {
      status = SL_STATUS_FAIL;
      break;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * Sends and recieves the data to the slave device simultaneously
 * It takes three arguments, 
 *   - data_in: pointer to the data buffer which stores the received data
 *   - data_out: pointer to the data buffer which stores the data that needs to 
 *               be sent
 *   - data_legth: The number of data items to be transferred 
 * It stores the received data in the data_in buffer
 ******************************************************************************/
sl_status_t sl_si91x_gspi_transfer_data(sl_gspi_handle_t gspi_handle,
                                        const void *data_out,
                                        void *data_in,
                                        uint32_t data_length)
{
  sl_status_t status;
  int32_t error_status;
  do {
    // To validate pointers, if the parameters is NULL, it returns an error code
    if ((data_in == NULL) || (data_out == NULL) || (gspi_handle == NULL)) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validate the GSPI handle address, if incorrect returns error code
    if (!validate_gspi_handle(gspi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validates the data length parameter, if zero returns error code
    if (data_length == MIN_DATA_LENGTH) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // It calls the API to enable to slave gpio.
    status = set_slave_gpio_state(gspi_handle, ENABLE);
    if (status != SL_STATUS_OK) {
      break;
    }
    // CMSIS API for transfering data is called and the arm error code returned from
    // the API is converted to SL error code via convert_arm_to_sl_error_code function.
    // It updates the data_in variable with the data received.
    error_status = ((sl_gspi_driver_t *)gspi_handle)->Transfer(data_out, data_in, data_length);
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * Sends and receives data to/from the slave device simultaneously in blocking mode.
 * Arguments:
 *   - gspi_handle: GSPI handle
 *   - data_out: pointer to the data buffer to be sent
 *   - data_in: pointer to the data buffer to store received data
 *   - data_length: number of data items to transfer
 *   - timeout: timeout value for the transfer
 * Returns SL_STATUS_OK on success or appropriate error code.
 ******************************************************************************/
sl_status_t sl_si91x_gspi_transfer_data_blocking(sl_gspi_handle_t gspi_handle,
                                                 const void *data_out,
                                                 void *data_in,
                                                 uint32_t data_length,
                                                 uint32_t timeout)
{
  sl_status_t status;
  int32_t error_status;
  do {
    // Validate pointers, if any parameter is NULL, return error code
    if ((data_in == NULL) || (data_out == NULL) || (gspi_handle == NULL)) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validate the GSPI handle address, if incorrect return error code
    if (!validate_gspi_handle(gspi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validate data_length, if zero return error code
    if (data_length == MIN_DATA_LENGTH) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Enable slave select GPIO
    status = GSPI_MASTER_SetSlaveSelectGPIOState(ENABLE);
    if (status != SL_STATUS_OK) {
      break;
    }
    // Transfer data in blocking mode and convert ARM error code to SL error code
    error_status = GSPI_MASTER_Transfer_Blocking(data_out, data_in, data_length, timeout);
    status       = convert_arm_to_sl_error_code(error_status);
    // Disable slave select GPIO
    if (GSPI_MASTER_SetSlaveSelectGPIOState(DISABLE) != SL_STATUS_OK) {
      status = SL_STATUS_FAIL;
      break;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * To set the master state i.e., activate/de-activate the master
 * It takes argument of boolean type either enable or disable
 * It sets the master mode and returns the status of operation.
 ******************************************************************************/
sl_status_t sl_si91x_gspi_set_master_state(sl_gspi_handle_t gspi_handle, boolean_t value)
{
  sl_status_t status;
  int32_t error_status;
  do {
    // Validates the null pointer, if true returns error code
    if (gspi_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validate the GSPI handle address, if incorrect returns error code
    if (!validate_gspi_handle(gspi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (value == ENABLE) {
      // If the value is enable, it calls the CMSIS API control to set the
      // master in enable mode.
      error_status = ((sl_gspi_driver_t *)gspi_handle)
                       ->Control(ARM_SPI_MODE_MASTER | ARM_SPI_DATA_BITS(GSPI_GetFrameLength()), NO_VALUE);
      // The arm error code returned from  the API is converted to SL error code
      // via convert_arm_to_sl_error_code function.
      status = convert_arm_to_sl_error_code(error_status);
      break;
    }
    // If the value is disable, it calls the CMSIS API control to set the
    // master in disable mode.
    error_status = ((sl_gspi_driver_t *)gspi_handle)
                     ->Control(ARM_SPI_MODE_INACTIVE | ARM_SPI_DATA_BITS(GSPI_GetFrameLength()), NO_VALUE);
    // The arm error code returned from  the API is converted to SL error code
    // via convert_arm_to_sl_error_code function.
    status = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
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
sl_status_t sl_si91x_gspi_register_event_callback(sl_gspi_handle_t gspi_handle, sl_gspi_signal_event_t callback_event)
{
  sl_status_t status;
  do {
    // Validates the null pointer, if true returns error code
    if ((gspi_handle == NULL) || (callback_event == NULL)) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validate the GSPI handle address, if incorrect returns error code
    if (!validate_gspi_handle(gspi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // To validate the function pointer if the parameters is not NULL then, it
    // returns an error code
    if ((user_callback != NULL) || (local_gspi_handle != NULL)) {
      status = SL_STATUS_BUSY;
      break;
    }
    // User callback address is passed to the static variable which is called at the time of
    // interrupt
    user_callback     = callback_event;
    local_gspi_handle = gspi_handle;
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
void sl_si91x_gspi_unregister_event_callback(void)
{
  // Pass the NULL value to the static variable which is called at the time of
  // interrupt.
  // It is further validated in register callback API.
  user_callback     = NULL;
  local_gspi_handle = NULL;
}

/*******************************************************************************
 * To get the release, sqa and dev version of GSPI
 * It returns the structure for gspi version.
 * Structure includes three members:
 * - Release version
 * - Major version (SQA version)
 * - Minor version (Dev version)
 ******************************************************************************/
sl_gspi_version_t sl_si91x_gspi_get_version(void)
{
  sl_gspi_version_t version;
  version.release = GSPI_RELEASE_VERSION;
  version.major   = GSPI_SQA_VERSION;
  version.minor   = GSPI_DEV_VERSION;
  return version;
}

/*******************************************************************************
 * To get the transfer status GSPI
 * It returns the result (sl_gspi_status_t) type structure fetched from 
 * GetStatus() API.
 * It updates three members of the structure i.e., busy, mode_fault, data_lost
 ******************************************************************************/
sl_gspi_status_t sl_si91x_gspi_get_status(sl_gspi_handle_t gspi_handle)
{
  return ((sl_gspi_driver_t *)gspi_handle)->GetStatus();
}

/*******************************************************************************
 * To get the receive data count of GSPI
 * It returns the rx count (uint32_t) fetched from GetDataCount() API.
 ******************************************************************************/
uint32_t sl_si91x_gspi_get_rx_data_count(sl_gspi_handle_t gspi_handle)
{
  return ((sl_gspi_driver_t *)gspi_handle)->GetDataCount();
}

/*******************************************************************************
 * To get the transmit data count of GSPI
 * It returns the tx count (uint32_t) fetched from GSPI_GetTxCount() API.
 ******************************************************************************/
uint32_t sl_si91x_gspi_get_tx_data_count(void)
{
  return GSPI_GetTxCount();
}

/*******************************************************************************
 * To fetch the clock division factor 
 * It reads the register which holds the clock division factor
 * and return the value of in 32 bit unsigned integer.
 ******************************************************************************/
int32_t sl_si91x_gspi_get_clock_division_factor(sl_gspi_handle_t gspi_handle)
{
  return ((sl_gspi_driver_t *)gspi_handle)->Control(ARM_SPI_GET_BUS_SPEED, NO_VALUE);
}

/*******************************************************************************
 * To fetch the frame length i.e., bit width
 * It reads the register which holds the frame length
 * and return the value of in 32 bit unsigned integer.
 ******************************************************************************/
uint32_t sl_si91x_gspi_get_frame_length(void)
{
  return GSPI_GetFrameLength();
}

/*******************************************************************************
 * To set the slave state i.e., activate/de-activate the slave
 * It takes argument of boolean type either enable or disable
 * It sets the slave mode and returns the status of operation.
 ******************************************************************************/
static sl_status_t set_slave_gpio_state(sl_gspi_handle_t gspi_handle, boolean_t value)
{
  sl_status_t status;
  int32_t error_status;
  do {
    if (gspi_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validate the GSPI handle address, if incorrect returns error code
    if (!validate_gspi_handle(gspi_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (value) {
      // If the value is enable, it calls the CMSIS API control to set the
      // slave in enable mode.
      error_status = ((sl_gspi_driver_t *)gspi_handle)->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
      // The arm error code returned from  the API is converted to SL error code
      // via convert_arm_to_sl_error_code function.
      status = convert_arm_to_sl_error_code(error_status);
      break;
    }
    // If the value is disable, it calls the CMSIS API control to set the
    // slave in disable mode.
    error_status = ((sl_gspi_driver_t *)gspi_handle)->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
    // The arm error code returned from  the API is converted to SL error code
    // via convert_arm_to_sl_error_code function.
    status = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * To validate the ARM error code
 * While calling the CMSIS APIs, it returns the ARM Error codes.
 * This function converts the ARM error codes into SL error codes.
 * It takes argument as 32 bit signed integer and returns the SL error type.
 * It has a single switch statement which is mapped with the SL error code and
 * after successful conversion it breaks the switch statement.
 * If the error code is not listed, by default is SL_STATUS_FAIL.
 ******************************************************************************/
static sl_status_t convert_arm_to_sl_error_code(int32_t error)
{
  sl_status_t status;
  switch (error) {
    // Driver Errors
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
    // SPI Errors
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
 * To validate the control parameters and build a 32 bit integer
 * It takes pointer to control configuration structure and pointer to the uint32_t
 * as argument and builds the integer using the validations and 'OR' operation.
 * According to the values in control configuration structure, it performs the 
 * 'OR' operation of the values.
 ******************************************************************************/
static sl_status_t validate_control_parameters(sl_gspi_control_config_t *control_configuration)
{
  sl_status_t status;
  do {
    // Validates the bitrate, if it is greater than maximum bitrate, returns error code.
    if (control_configuration->bitrate > MAX_BITRATE) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // If the mode is neither 0 nor 3, returns the error code.
    if (control_configuration->clock_mode != SL_GSPI_MODE_0 && control_configuration->clock_mode != SL_GSPI_MODE_3) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // If the bit width is not in range i.e., between 0 and 16,
    // returns the error code.
    if ((control_configuration->bit_width == MIN_BIT_WIDTH) || (control_configuration->bit_width > MAX_BIT_WIDTH)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // If the slave select mode is greater than the slave select last enum, it
    // returns the error code.
    if (control_configuration->slave_select_mode >= SL_GSPI_SLAVE_SELECT_MODE_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Returns SL_STATUS_OK if the parameter are appropriate
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * To get the handle of GSPI structure.
 * It should be called before calling any GSPI function to get the appropriate handle.
 * It updates the address of the gspi_handle passed by the user.
 ******************************************************************************/
static sl_status_t get_gspi_handle(sl_gspi_instance_t instance, sl_gspi_handle_t *gspi_handle)
{
  sl_status_t status;
  do {
    if (instance >= SL_GSPI_INSTANCE_LAST_ENUM) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    if (gspi_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    *gspi_handle = &Driver_GSPI_MASTER;
    status       = SL_STATUS_OK;
  } while (false);
  return status;
}

static boolean_t validate_gspi_handle(sl_gspi_handle_t gspi_handle)
{
  boolean_t status;
  if (gspi_handle == &Driver_GSPI_MASTER) {
    status = true;
  } else {
    status = false;
  }
  return status;
}

/*******************************************************************************
 * Static callback function for handling the callbacks
 * At the time of send/receive or transfer, the chip select is enabled
 * Here after transfer completes, the chip select disabled.
 ******************************************************************************/
static void callback_event_handler(uint32_t event)
{
  switch (event) {
    case SL_GSPI_TRANSFER_COMPLETE:
      set_slave_gpio_state(local_gspi_handle, DISABLE);
      break;
    case SL_GSPI_DATA_LOST:
      break;
    case SL_GSPI_MODE_FAULT:
      break;
  }
  user_callback(event);
}
