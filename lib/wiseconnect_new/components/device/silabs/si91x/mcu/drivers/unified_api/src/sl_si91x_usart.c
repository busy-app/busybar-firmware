/******************************************************************************
* @file sl_si91x_usart.c
* @brief USART API implementation
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

#include "sl_si91x_usart.h"
#include "sl_si91x_clock_manager.h"
#if (SLI_SI91X_MCU_RS485_MODE == 1)
#include "sl_gpio_board.h"
#include "sl_si91x_driver_gpio.h"
#ifdef UART1_RS485_MODE
#include "sl_si91x_uart1_rs485_config.h"
#endif
#ifdef UART0_RS485_MODE
#include "sl_si91x_uart0_rs485_config.h"
#endif
#endif
#ifdef USART_MODULE
#include "sl_si91x_usart_config.h"
#endif
#ifdef UART_MODULE
#include "sl_si91x_uart_config.h"
#endif
#ifdef ULP_UART_MODULE
#include "sl_si91x_ulp_uart_config.h"
#endif
/*******************************************************************************
 ***************************  LOCAL MACROS   ********************************
 ******************************************************************************/
#define MAX_BAUDRATE           7372800   // Max baud rate supported
#define CONFIG_ENABLE          1         // Used to configure USART/UART enable should be one
#define USART_RELEASE_VERSION  0         // USART Release version
#define USART_SQA_VERSION      0         // USART SQA version
#define USART_DEV_VERSION      2         // USART Developer version
#define NO_OF_UART_INSTANCES   3         // No of usart instances, 3 uart instances in 91x
#define CONVERT_TO_MS          10000     // To convert the system frequency into milliseconds, divide it by this value
#define TIMEOUT                50000     // Timeout value for blocking calls (in ms)
#define TIMEOUT_EXPIRY         1         // At this value the timeout is expired and exits the loop
#define SYSTEM_CLOCK_180MHZ    180000000 // System frequency value at 180 MHz
#define SYSTEM_CLOCK_90MHZ     90000000  // System frequency value at 90 MHz
#define DIVISION_FACTOR_180MHZ 4         // Division factor to get accurate ticks at 180 MHz
#define DIVISION_FACTOR_90MHZ  3         // Division factor to get accurate ticks at 90 MHz
#define DIVISION_FACTOR_32MHZ  2         // Division factor to get accurate ticks at 32 MHz
#define OUTPUT                 0         // Output value
#define SL_GPIO_MODE_11        11        // GPIO MODE 11
/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/
/// @brief USART configuration structure
typedef struct {
  sl_usart_handle_t usart_handle;                  ///< USART/UART Handle
  sl_si91x_usart_control_config_t *control_config; ///< UART/USART control configuration structure
} sl_si91x_usart_config;

/*******************************************************************************
 ***************************  Global  VARIABLES   ********************************
 ******************************************************************************/
extern ARM_DRIVER_USART Driver_USART0;
extern ARM_DRIVER_USART Driver_UART1;
extern ARM_DRIVER_USART Driver_ULP_UART;
boolean_t sl_usart_receive_complete = false, sl_usart_send_complete = false;

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/
static sl_usart_signal_event_t user_callback[] = { NULL, NULL, NULL }; ///< Callback for USART user callback

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
static uint32_t usart_get_baud_rate(uint8_t usart_module);
static sl_status_t usart_get_parity(uint8_t usart_module, usart_parity_typedef_t *parity);
static sl_status_t usart_get_stop_bit(uint8_t usart_module, usart_stopbit_typedef_t *stopbits);
static uint8_t sli_si91x_usart_get_mode(uint8_t usart_module);
static sl_status_t convert_arm_to_sl_error_code(int32_t error);
static sl_status_t validate_control_parameters(sl_si91x_usart_control_config_t *control_configuration,
                                               uint32_t *input_mode);
static sl_status_t usart_set_tx_rx_configuration(const void *usart_handle);
static void callback_event_handler(uint32_t event);
static sl_status_t usart_get_handle(usart_peripheral_t usart_instance, sl_usart_handle_t *usart_handle);
static boolean_t validate_usart_handle(sl_usart_handle_t usart_handle);
static sl_status_t sli_si91x_usart_set_power_mode(sl_usart_handle_t usart_handle, power_mode_typedef_t state);
static usart_peripheral_t get_usart_instance(sl_usart_handle_t usart_handle);
#if (SLI_SI91X_MCU_RS485_MODE == 1)
static sl_status_t configure_rs485_gpio_pin(sl_rs485_gpio_pin_config_t *pin_config);
static sl_status_t sl_si91x_uart_rs485_gpio_init(sl_uart_rs485_pin_init_t *pin_config);
static uint32_t get_usart_tick_count(uint32_t time_ms);
#endif
/*******************************************************************************
 **************************   Global function Definitions   *******************************
 ******************************************************************************/
/*******************************************************************************
 * @brief
 * Get the USART handle
 *
 * @details
 * This function update the usart handle based on usart instance if not valid usart
 * instance then returns the invalid parameter error code
 ******************************************************************************/
static sl_status_t usart_get_handle(usart_peripheral_t usart_instance, sl_usart_handle_t *usart_handle)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    // Check for USART instance valid
    if (usart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // If instance is USART0 then update the USART handle USART0
    if (usart_instance == USART_0) {
      *usart_handle = &Driver_USART0;
      status        = SL_STATUS_OK;
      break;
    }
    // If instance is UART1 then update the USART handle UART1
    if (usart_instance == UART_1) {
      *usart_handle = &Driver_UART1;
      status        = SL_STATUS_OK;
      break;
    }
    // If instance is ULP_UART then update the USART handle with ULP UART
    if (usart_instance == ULPUART) {
      *usart_handle = &Driver_ULP_UART;
      status        = SL_STATUS_OK;
      break;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * Return the usart instance
 *
 * @details
 * This function checks the usart handle is valid or not if it's valid handle then
 * returns the USART instance
 ******************************************************************************/
static usart_peripheral_t get_usart_instance(sl_usart_handle_t usart_handle)
{
  usart_peripheral_t usart_instance = USART_0;
  // Return the USART instance
  if (usart_handle == &Driver_USART0) {
    usart_instance = USART_0;
  }
  if (usart_handle == &Driver_UART1) {
    usart_instance = UART_1;
  }
  if (usart_handle == &Driver_ULP_UART) {
    usart_instance = ULPUART;
  }
  return usart_instance;
}

/*******************************************************************************
 * @brief
 * Validate the USART handle is valid
 *
 * @details
 * This function checks the usart handle is valid or not if it's valid handle then
 * returns the SL_STATUS_OK else returns the error code
 ******************************************************************************/
static boolean_t validate_usart_handle(sl_usart_handle_t usart_handle)
{
  boolean_t status;
  // Check for USART instance valid
  if ((usart_handle == &Driver_USART0) || (usart_handle == &Driver_UART1) || (usart_handle == &Driver_ULP_UART)) {
    status = true;
  }
  // if usart instance is ULP_UART then check usart_handle is ULP_UART
  else {
    status = false;
  }
  return status;
}

/*******************************************************************************
 * @brief
 * Initialization of USART/UART interface
 *
 * @details
 * Initializes and Configures the USART base clock
 * If DMA is enabled then Initializes the DMA .
 * It will take callback event as input argument and call the callback function
 * on usart data transfer abd reception
 ******************************************************************************/
sl_status_t sl_si91x_usart_init(usart_peripheral_t usart_instance, sl_usart_handle_t *usart_handle)
{
  sl_status_t status = SL_STATUS_FAIL;
  int32_t error_status;
  sl_usart_handle_t usart_temp_handle = NULL;

  do {
    // Check the USART Handle for NUll
    if (usart_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }

    if (*usart_handle != NULL) {
      // Get the USART Handle
      usart_get_handle(usart_instance, &usart_temp_handle);

      *usart_handle = usart_temp_handle;
      // Uninitialize USART/UART Driver
      error_status = ((sl_usart_driver_t *)usart_temp_handle)->Uninitialize();
      // Assign callback handler with null
      *usart_handle = NULL;
    }

#ifdef DEBUG_UART_UC
    // Check the USART instance is already used for Debug output
    if (SL_DEBUG_INSTANCE == usart_instance) {
      status = SL_STATUS_NOT_AVAILABLE;
      break;
    }
#endif

    // Get the USART Init state
    if (USART_GetInitState(usart_instance)) {
      status = SL_STATUS_BUSY;
      break;
    }

    // To validate the structure pointer and usart handle, if the parameters is NULL, it
    // returns an error code
    if (*usart_handle != NULL) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Get the USART Handle
    usart_get_handle(usart_instance, &usart_temp_handle);
    *usart_handle = usart_temp_handle;

    // CMSIS API for initialization is called with the function address of
    // callback function, and the arm error code returned from the API is
    // converted to SL error code via convert_arm_to_sl_error_code function.
    error_status = ((sl_usart_driver_t *)usart_temp_handle)->Initialize(callback_event_handler);
    status       = convert_arm_to_sl_error_code(error_status);
    if (status != SL_STATUS_OK) {
      return status;
    }
    // If USART Initializes success set the power mode
    status = sli_si91x_usart_set_power_mode(usart_temp_handle, SL_POWER_FULL);
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * De-init the USART
 *
 * @details
 * This function will disable the clocks for USART/UART module and also
 * Deinit the DMA for UART/USART if DMA is enabled for data transfers.
 * Validates the error code returns return the status.
 *
 * @note
 *  To enable debug, DEBUGINIT() needs to be called after using this API.
 ******************************************************************************/
sl_status_t sl_si91x_usart_deinit(sl_usart_handle_t usart_handle)
{
  sl_status_t status = SL_STATUS_FAIL;
  int32_t error_status;
  usart_peripheral_t usart_instance = USART_0;
  do {
    // Check for USART handle parameter, if NULL return from here
    if (usart_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validate USART handle
    if (!validate_usart_handle(usart_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Get the USART Insatnce
    usart_instance = get_usart_instance(usart_handle);
    // Unregister user callback for multiple instance
    sl_si91x_usart_multiple_instance_unregister_event_callback(usart_instance);
    //Deinit the USART/UART
    error_status = ((sl_usart_driver_t *)usart_handle)->Uninitialize();
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * To set the power mode of USART
 *
 * @details
 * It will set the power mode of usart
 *          - powerOff
 *          - powerLow
 *          - powerFull
 *
 * In poweroff power state will disable the interrupt , disable DMA channel if DMA mode
 * disable the power to the USART block.So no usart operations are possible when set
 * this mode
 *
 * powerLow power state is not supported
 *
 * In powerFull power state will enable the intterupt and set the flags initialization
 * and power on flag so that USART data teransfer can takes place
 *******************************************************************************/
static sl_status_t sli_si91x_usart_set_power_mode(sl_usart_handle_t usart_handle, power_mode_typedef_t state)
{
  sl_status_t status = SL_STATUS_FAIL;
  int32_t error_status;
  do {
    // Check for USART handle parameter, if NULL return from here
    if (usart_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Check for power state
    if (state >= SL_POWER_MODE_LAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validate USART handle
    if (!validate_usart_handle(usart_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Set the USART power state
    error_status = ((sl_usart_driver_t *)usart_handle)->PowerControl((ARM_POWER_STATE)state);
    status       = convert_arm_to_sl_error_code(error_status);

  } while (false);
  return status;
}

/*******************************************************************************
 * To register the event callback
 * It registers the callback, i.e., stores the callback function address
 * and pass to the variable that is called in Interrupt Handler.
 * If another callback is registered without unregistering previous callback then, it
 * returns an error code, so it is mandatory to unregister the callback before registering
 * another callback.
 * It will returns error if any callback is already registered.
 ******************************************************************************/
sl_status_t sl_si91x_usart_register_event_callback(sl_usart_signal_event_t callback_event)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    // Validates the null pointer, if true returns error code
    if (callback_event == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // To validate the function pointer if the parameters is not NULL then, it
    // returns an error code
    if (user_callback[0] != NULL) {
      status = SL_STATUS_BUSY;
      break;
    }
    // User callback address is passed to the static variable which is called at the time of
    // interrupt
    user_callback[0] = callback_event;
    // Returns SL_STATUS_OK if callback is successfully registered
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * It unregisters the callback, i.e., clear the callback function address
 * and pass NULL value to the variable
 ******************************************************************************/
void sl_si91x_usart_unregister_event_callback(void)
{
  // Pass the NULL value to the static variable which is called at the time of
  // interrupt.
  // It is further validated in register callback API.
  // Hadcoded to 0 because this API is used to unregiser, only one usart callback handler
  user_callback[0] = NULL;
}

/*******************************************************************************
 * To register the event callback in case of multiple usart instances
 * It registers the callback, i.e., stores the callback function address
 * and pass to the variable that is called in Interrupt Handler.
 * If another callback is registered without unregistering previous callback then, it
 * returns an error code, so it is mandatory to unregister the callback before registering
 * another callback.
 * It will returns error if any callback is already registered.
 ******************************************************************************/
sl_status_t sl_si91x_usart_multiple_instance_register_event_callback(usart_peripheral_t usart_instance,
                                                                     sl_usart_signal_event_t callback_event)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    // Validates the null pointer, if true returns error code
    if (callback_event == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Check for USART instance valid
    if (usart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // To validate the function pointer if the parameters is not NULL then, it
    // returns an error codex
    if (user_callback[usart_instance] != NULL) {
      status = SL_STATUS_BUSY;
      break;
    }
    // User callback address is passed to the static variable which is called at the time of
    // interrupt
    user_callback[usart_instance] = callback_event;
    // Returns SL_STATUS_OK if callback is successfully registered
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * It unregisters the callback, i.e., clear the callback function address
 * and pass NULL value to the variable
 ******************************************************************************/
void sl_si91x_usart_multiple_instance_unregister_event_callback(usart_peripheral_t usart_instance)
{
  // Pass the NULL value to the static variable which is called at the time of
  // interrupt.
  // It is further validated in register callback API.
  // Check for USART instance valid
  if (usart_instance < UARTLAST) {
    user_callback[usart_instance] = NULL;
  }
}

/*******************************************************************************
 * @brief
 * To send the data when USART/UART is configured
 *
 * @details
 * This function sends the data on USART transmitter
 * It takes two arguments,
 *   - data: pointer to the data buffer which will store the data that needs to
 *           be sent
 *   - data_legth: The number of data items to be send
 ******************************************************************************/
sl_status_t sl_si91x_usart_send_data(sl_usart_handle_t usart_handle, const void *data, uint32_t data_length)
{
  sl_status_t status = SL_STATUS_FAIL;
  int32_t error_status;
  do {
    // Check USART handle and data parameters, if NULL return from here
    if ((usart_handle == NULL) || (data == NULL)) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Check for data length
    if (data_length == 0) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validate USART handle
    if (!validate_usart_handle(usart_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Send the data available in data pointer parameter
    error_status = ((sl_usart_driver_t *)usart_handle)->Send(data, data_length);
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * Internal function to send data in blocking mode byte-by-byte
 *
 * @details
 * This internal function sends data byte-by-byte via USART/UART and waits (blocks)
 * until each byte transmission is complete before sending the next byte. This ensures
 * true byte-by-byte transmission with hardware verification for each byte.
 * The function accesses UART hardware registers (THR) directly from the handle and
 * checks the transmitter empty flag (TEMT) for each byte.
 *
 * Supports all UART instances:
 *   - USART_0 (USART0)
 *   - UART_1  (UART1)
 *   - ULPUART (ULP_UART)
 *
 * It takes three arguments:
 *   - usart_handle: pointer to the USART/UART driver handle (supports USART0, UART1, ULP_UART)
 *   - data: pointer to the data buffer which contains the data to be sent
 *   - data_length: The number of data bytes to send
 *
 * @return sl_status_t - Returns SL_STATUS_OK on successful transmission,
 *                       SL_STATUS_NULL_POINTER if parameters are NULL,
 *                       SL_STATUS_INVALID_PARAMETER if data_length is 0
 *
 * @note
 *   - Works with all UART instances by extracting register base from handle
 *   - Each byte is transmitted and verified before sending the next
 *   - Do not call from interrupt context
 ******************************************************************************/
sl_status_t sli_si91x_usart_send_data_blocking(sl_usart_handle_t usart_handle, const void *data, uint32_t data_length)
{
  const uint8_t *buffer = (const uint8_t *)data;
  USART0_Type *uart_reg;
  usart_peripheral_t uart_instance;

  // Check for NULL pointers
  if ((usart_handle == NULL) || (data == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }
  // Check for data length
  if (data_length == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  uart_instance = get_usart_instance(usart_handle);
  // Get the register base address from resources
  // if uart_instance is  ULPUART, send data to ulp_uart
  if (uart_instance == ULPUART) {
    uart_reg = ULP_UART;
  }
  // ifuart_instance is  USART_0, send data to uart0
  else if (uart_instance == USART_0) {
    uart_reg = UART0;

  }
  // if uart_instance is  UART_1, send data to uart1
  else if (uart_instance == UART_1) {
    uart_reg = UART1;
  }
  // Send data byte-by-byte
  for (uint32_t index = 0; index < data_length; index++) {
    uart_reg->THR = buffer[index];    // Write byte to Transmit Holding Register
    while (uart_reg->LSR_b.TEMT == 0) // Wait for Transmitter Empty
      ;
  }
  return SL_STATUS_OK;
}
/*******************************************************************************
 * @brief
 * To Send the data in async mode when USART/UART is configured
 *
 * @details
 * This function returns immediately and data transfer happens asyncronously. Once the data transfer compeletes ,
 * registered user callback get invoked
 * It takes two arguments,
 *   - data: pointer to the data buffer which stores the data received
 *   - data_legth: The number of data length to receive
 *
 * Stores the data received in data pointer
 ******************************************************************************/
sl_status_t sl_si91x_usart_async_send_data(sl_usart_handle_t usart_handle, const void *data, uint32_t data_length)
{
  sl_status_t status = SL_STATUS_FAIL;
  sl_si91x_usart_control_config_t get_config;

  // Validate input parameters
  if (usart_handle == NULL || data == NULL || data_length == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  status = sl_si91x_usart_get_configurations(get_usart_instance(usart_handle), &get_config);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Only allow async receive in async mode
  if (get_config.mode != ARM_USART_MODE_ASYNCHRONOUS) {
    return SL_STATUS_INVALID_STATE;
  }

  return sl_si91x_usart_send_data(usart_handle, data, data_length);
}

/*******************************************************************************
 * @brief
 * To receive the data when USART/UART is configured
 *
 * @details
 * This function receives the data received on USART receiver
 * It takes two arguments,
 *   - data: pointer to the data buffer which stores the data received
 *   - data_legth: The number of data length to receive
 *
 * Stores the data received in data pointer
 ******************************************************************************/
sl_status_t sl_si91x_usart_receive_data(sl_usart_handle_t usart_handle, void *data, uint32_t data_length)
{
  sl_status_t status = SL_STATUS_FAIL;
  int32_t error_status;
  do {
    // Check USART handle and data parameters, if NULL return from here
    if ((usart_handle == NULL) || (data == NULL)) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Check for data length
    if (data_length == 0) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validate USART handle
    if (!validate_usart_handle(usart_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Receive the data from USART receiver and point it the data pointer received as argument
    error_status = ((sl_usart_driver_t *)usart_handle)->Receive(data, data_length);
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * To receive the data in async mode when USART/UART is configured
 *
 * @details
 * This function returns immediately and data reception happens asynchronously. Once the data reception completes ,
 * registered user callback get invoked
 * It takes two arguments,
 *   - data: pointer to the data buffer which stores the data received
 *   - data_length: The number of data length to receive
 *
 * Stores the data received in data pointer
 ******************************************************************************/
sl_status_t sl_si91x_usart_async_receive_data(sl_usart_handle_t usart_handle, void *data, uint32_t data_length)
{
  sl_status_t status = SL_STATUS_FAIL;
  sl_si91x_usart_control_config_t get_config;

  // Validate input parameters
  if (usart_handle == NULL || data == NULL || data_length == 0) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  status = sl_si91x_usart_get_configurations(get_usart_instance(usart_handle), &get_config);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Only allow async receive in async mode
  if (get_config.mode != ARM_USART_MODE_ASYNCHRONOUS) {
    return SL_STATUS_INVALID_STATE;
  }

  return sl_si91x_usart_receive_data(usart_handle, data, data_length);
}

/*******************************************************************************
 * @brief
 * To send and receive the data when USART in Synchronous mode is enabled
 *
 * @details
 * This function sends and receives the data to and from usart transmitter and
 * receiver in synchronous mode of operation
 * It takes three arguments,
 *   - data_in: pointer to the data buffer which will store the received data
 *   - data_out: pointer to the data buffer which will store the data that needs to
 *               be sent
 *   - data_length: The number of data items to be transferred
 *
 * It will store the received data in the data_in buffer
 *
 * @note
 * This function use only in synchronous mode
 ******************************************************************************/
sl_status_t sl_si91x_usart_transfer_data(sl_usart_handle_t usart_handle,
                                         const void *data_out,
                                         void *data_in,
                                         uint32_t data_length)
{
  sl_status_t status = SL_STATUS_FAIL;
  int32_t error_status;
  do {
    // Check USART handle data in and data out parameters, if NULL return from here
    if ((usart_handle == NULL) || (data_in == NULL) || (data_out == NULL)) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Check for Data length
    if (data_length == 0) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validate USART handle
    if (!validate_usart_handle(usart_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Transfers and Receives the data in USART synch mode
    error_status = ((sl_usart_driver_t *)usart_handle)->Transfer(data_out, data_in, data_length);
    status       = convert_arm_to_sl_error_code(error_status);

  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * To get the TX data count of USART
 *
 * @details
 * It will return Transferred data bytes by USART/UART transmitter
 ******************************************************************************/
uint32_t sl_si91x_usart_get_tx_data_count(sl_usart_handle_t usart_handle)
{
  return ((sl_usart_driver_t *)usart_handle)->GetTxCount();
}

/*******************************************************************************
 * @brief
 * To get the RX data count of USART
 *
 * @details
 * It will return RX data bytes by USART/UART receiver
 ******************************************************************************/
uint32_t sl_si91x_usart_get_rx_data_count(sl_usart_handle_t usart_handle)
{
  return ((sl_usart_driver_t *)usart_handle)->GetRxCount();
}

/*******************************************************************************
 * @brief
 * To control and configure the USART
 *
 * @details
 * It takes argument of pointer to the structure of control configuration.
 * It will configure the different configurations such as  USART mode, Data Bits,
 * Parity , stop bits, flow control ,baud rate,
 * - USART mode - mode of operation
 *              - usartAsynchMode
 *              - usartSynchModeMaster
 *              - usartSynchModeSlave
 *              - usartModeSingleWire
 *              - usartModeIrda
 * - Data Bits - Set the no of data bits in frame format
 *             - usartDatabits5
 *             - usartDatabits6
 *             - usartDatabits7
 *             - usartDatabits8
 *             - usartDatabits9
 * - Parity   - Set the parity bit
 *             - usartNoParity
 *             - usartEvenParity
 *             - usartOddParity
 * - Stop bits - Set the stop bits
 *             - ARM_USART_STOP_BITS_1
 *             - ARM_USART_STOP_BITS_2
 *  @note
 *  ARM_USART_STOP_BITS_0_5 is not supported in driver
 *  Even we set ARM_USART_STOP_BITS_1_5 driver will consider the
 *          ARM_USART_STOP_BITS_2
 *
 * - Flow control -Set the usart flow control in synchronous mode
 *                - usartHwFlowControlNone
 *                - usartHwFlowControlCts
 *                - usartHwFlowControlRts
 *                - usartHwFlowControlCtsAndRts
 * - Baud rate -Set the usart baud rate
 ********************************************************************************/
sl_status_t sl_si91x_usart_set_configuration(sl_usart_handle_t usart_handle,
                                             sl_si91x_usart_control_config_t *control_configuration)
{
  sl_status_t status = SL_STATUS_FAIL;
  int32_t error_status;
  uint32_t input_mode = false;
  sl_si91x_usart_control_config_t control_config[NO_OF_UART_INSTANCES];
  usart_peripheral_t uart_instance;
  (void)control_config;
  (void)uart_instance;

  /* USART_UC is defined by default. when this macro (USART_UC) is defined, peripheral
   * configuration is directly taken from the configuration set in the universal configuration (UC).
   * if the application requires the configuration to be changed in run-time, undefined this macro
   * and change the peripheral configuration through the sl_si91x_usart_set_configuration API.
   */
  do {

#if (USART_UC == 1)
    // Get the USART Instance
    uart_instance = get_usart_instance(usart_handle);
    // if usart intance is USART_0 , update usart config  from UC
    if (uart_instance == USART_0) {
      control_config[uart_instance] = usart_configuration;
    }
    *control_configuration = control_config[uart_instance];
#endif
#if (UART_UC == 1)
    // Get the USART Instance
    uart_instance = get_usart_instance(usart_handle);
    // if usart intance is UART_1 , update uart1 config  from UC
    if (uart_instance == UART_1) {
      control_config[uart_instance] = uart1_configuration;
    }
    *control_configuration = control_config[uart_instance];
#endif
#if (ULP_UART_UC == 1)
    // Get the USART Instance
    uart_instance = get_usart_instance(usart_handle);
    // if usart intance is  ULPUART, update ulp uart config  from UC
    if (uart_instance == ULPUART) {
      control_config[uart_instance] = ulp_uart_configuration;
    }
    *control_configuration = control_config[uart_instance];
#endif

    // Check USART handle and control_configuration parameter, if NULL return from here
    if ((usart_handle == NULL) || (control_configuration == NULL)) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Check for Max Baud rate supported
    if (control_configuration->baudrate > MAX_BAUDRATE) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validate USART handle
    if (!validate_usart_handle(usart_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Configure the USART Transfer and receive line
    status = usart_set_tx_rx_configuration(usart_handle);
    if (status != SL_STATUS_OK) {
      break;
    }
    // Validate the control parameters and combine in single variable
    status = validate_control_parameters(control_configuration, &input_mode);
    if (status != SL_STATUS_OK) {
      break;
    }
    // Configure the USART parameters
    error_status = ((sl_usart_driver_t *)usart_handle)->Control(input_mode, control_configuration->baudrate);
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * To control and configure the USART
 *
 * @details
 * It takes argument of pointer to the structure of control configuration and
 * baud rate.
 * It will configure the different configurations such as  USART mode, Data Bits,
 * Parity , stop bits, flow control ,baud rate,
 * - USART mode - mode of operation
 *              - usartAsynchMode
 *              - usartSynchModeMaster
 *              - usartSynchModeSlave
 *              - usartModeSingleWire
 *              - usartModeIrda
 * - Data Bits - Set the no of data bits in frame format
 *             - usartDatabits5
 *             - usartDatabits6
 *             - usartDatabits7
 *             - usartDatabits8
 *             - usartDatabits9
 * - Parity   - Set the parity bit
 *             - usartNoParity
 *             - usartEvenParity
 *             - usartOddParity
 * - Stop bits - Set the stop bits
 *             - ARM_USART_STOP_BITS_1
 *             - ARM_USART_STOP_BITS_2
 *  @note
 *  ARM_USART_STOP_BITS_0_5 is not supported in driver
 *  Even we set ARM_USART_STOP_BITS_1_5 driver will consider the
 *          ARM_USART_STOP_BITS_2
 *
 * - Flow control -Set the usart flow control in synchronous mode
 *                - usartHwFlowControlNone
 *                - usartHwFlowControlCts
 *                - usartHwFlowControlRts
 *                - usartHwFlowControlCtsAndRts
 * - Baud rate -Set the usart baud rate
 *
 * @note
 * Call the sl_si91x_usart_set_tx_rx_configuration() to set the tx and rx
 * lines before this API
 ********************************************************************************/
sl_status_t sli_si91x_usart_set_non_uc_configuration(sl_usart_handle_t usart_handle,
                                                     sl_si91x_usart_control_config_t *control_configuration)
{
  sl_status_t status = SL_STATUS_FAIL;
  int32_t error_status;
  uint32_t input_mode = false;

  do {
    // Check USART handle and control_configuration parameter, if NULL return from here
    if ((usart_handle == NULL) || (control_configuration == NULL)) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Check for Max Baud rate supported
    if (control_configuration->baudrate > MAX_BAUDRATE) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Validate USART handle
    if (!validate_usart_handle(usart_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Configure the USART Transfer and receive line
    status = usart_set_tx_rx_configuration(usart_handle);
    if (status != SL_STATUS_OK) {
      break;
    }
    // Validate the control parameters and combine in single variable
    status = validate_control_parameters(control_configuration, &input_mode);
    if (status != SL_STATUS_OK) {
      break;
    }
    // Configure the USART parameters
    error_status = ((sl_usart_driver_t *)usart_handle)->Control(input_mode, control_configuration->baudrate);
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * Configures the USART Tx and Rx lines
 *
 * @details
 * This functions configures the usart tx and rx lines for data transfer
 * Takes the argument config enable to decide Tx and Rx lines configure or not
 * Validate the error code return from drives and return SL_STATUS
 ********************************************************************************/
static sl_status_t usart_set_tx_rx_configuration(sl_usart_handle_t usart_handle)
{
  sl_status_t status = SL_STATUS_FAIL;
  int32_t error_status;
  do {
    // Check USART handle parameter, if NULL return from here
    if (usart_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Set the USART Tx control line
    error_status = ((sl_usart_driver_t *)usart_handle)->Control(SL_USART_CONTROL_TX, CONFIG_ENABLE);
    status       = convert_arm_to_sl_error_code(error_status);
    if (status != SL_STATUS_OK) {
      break;
    }
    // Set the USART Rx control line
    error_status = ((sl_usart_driver_t *)usart_handle)->Control(SL_USART_CONTROL_RX, CONFIG_ENABLE);
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * To get the USART status
 *
 * @details
 * This function returns the sl_usart_status_t structure with usart status info about
 * tx_busy ,rx_busy, tx_inderfloe ,rx_overflow ,rx_break, rx_framing_error and
 * rx_parity_error
 ******************************************************************************/
sl_usart_status_t sl_si91x_usart_get_status(sl_usart_handle_t usart_handle)
{
  return ((sl_usart_driver_t *)usart_handle)->GetStatus();
}

/*******************************************************************************
 * @brief
 * To set the modem control line in USART/UART
 *
 * @details
 * It takes argument as structure of sl_usart_modem_control_t type
 * It validates the error returned from driver and return the status of operation.
 ******************************************************************************/
sl_status_t sl_si91x_usart_set_modem_control(sl_usart_handle_t usart_handle, sl_usart_modem_control_t control)
{
  sl_status_t status = SL_STATUS_FAIL;
  int32_t error_status;
  do {
    // Check USART handle parameter, if NULL return from here
    if (usart_handle == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Validate USART handle
    if (!validate_usart_handle(usart_handle)) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Set the USART Modem control state received as argument
    error_status = ((sl_usart_driver_t *)usart_handle)->SetModemControl(control);
    status       = convert_arm_to_sl_error_code(error_status);
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * To get the modem control status of USART/UART
 *
 * @details
 * This function gets the usart modem control status such as modem control lines
 * are active or not.
 * It return the structure with information about status of modem lines
 ******************************************************************************/
sl_usart_modem_status_t sl_si91x_usart_get_modem_status(sl_usart_handle_t usart_handle)
{
  return ((sl_usart_driver_t *)usart_handle)->GetModemStatus();
}

/*******************************************************************************
 * @brief
 * To get the release, sqa and dev version of USART
 * It returns the structure for USART version.
 * Structure includes three members:
 * - Release version
 * - SQA version
 * - Dev version
 ******************************************************************************/
sl_usart_version_t sl_si91x_usart_get_version(void)
{
  sl_usart_version_t version;
  version.minor   = USART_DEV_VERSION;
  version.release = USART_RELEASE_VERSION;
  version.major   = USART_SQA_VERSION;
  return version;
}

/*******************************************************************************
 * @brief
 * Get the usart configurations set
 *
 * @details
 * Get the USART configurations set in the module such as mode, baudrate, parity bit
 * stop bits
 *
 * @note
 * This function should call after USART Initliazation ,configuration
 *******************************************************************************/
sl_status_t sl_si91x_usart_get_configurations(uint8_t usart_module, sl_si91x_usart_control_config_t *usart_config)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    if (usart_config == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    //Get the usart operating mode
    usart_config->mode = sli_si91x_usart_get_mode(usart_module);
    // Get the usart buadate
    usart_config->baudrate = usart_get_baud_rate(usart_module);
    // Get the USART parity bit set
    status = usart_get_parity(usart_module, &usart_config->parity);
    if (status != SL_STATUS_OK) {
      break;
    }
    //Get the usart stop bit set
    status = usart_get_stop_bit(usart_module, &usart_config->stopbits);
    if (status != SL_STATUS_OK) {
      break;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * Get the UART/USART baud rate programmed
 *
 * @details
 * This function return the buadrate set in the module
 *
 * @note
 * This function should call after USART Initliazation ,configuration
 ******************************************************************************/
static uint32_t usart_get_baud_rate(uint8_t usart_module)
{
  return USART_GetBaudrate(usart_module);
}

/*******************************************************************************
 * @brief
 * Get the UART/USART parity bit set in the module
 *
 * @details
 * It takes argument as usart peripheral i.e USART0,UART1 or ULP_UART and
 * enum address to store the parity bit set.
 * if BIT(4) and BIT(3) are set
 *      00 - No parity
 *      01 - Odd Parity
 *      11 - Even Parity
 *
 * @note
 * This function should call after USART Initliazation ,configuration
 ******************************************************************************/
static sl_status_t usart_get_parity(uint8_t usart_module, usart_parity_typedef_t *parity)
{
  uint32_t parity_bit;
  sl_status_t status = SL_STATUS_FAIL;
  do {
    if (usart_module >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Get the parity bit data
    parity_bit = USART_GetParity_StopBit(usart_module);
    // Check the BIT(4) AND bit(3) to decide which parity is set
    if (parity_bit & BIT(3)) {
      ((parity_bit & BIT(4)) ? (*parity = SL_USART_EVEN_PARITY) : (*parity = SL_USART_ODD_PARITY));
    } else {
      // BIT(3) and BIT(4) both are 0 then No parity
      (*parity = SL_USART_NO_PARITY);
    }
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * Get USART stop bit set
 *
 * @details
 * This function is used to get the usart stop bit set ,It takes argument as usart
 * peipheral i.e USART0,UART1 or ULP_UART and enum address to store the stopbits
 *
 * @note
 * This function should call after USART Initialization ,configuration
 ******************************************************************************/
static sl_status_t usart_get_stop_bit(uint8_t usart_module, usart_stopbit_typedef_t *stopbits)
{
  uint32_t stopbit;
  sl_status_t status = SL_STATUS_FAIL;
  do {
    if (usart_module >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Get the stop bit data from h/w
    stopbit = USART_GetParity_StopBit(usart_module);
    // if bit(2) is set then usartStopbits2 and bit(2) is 0 then usartStopbits1
    (stopbit & BIT(2)) ? (*stopbits = SL_USART_STOP_BITS_2) : (*stopbits = SL_USART_STOP_BITS_1);
    status = SL_STATUS_OK;
  } while (false);
  // Return status
  return status;
}

/*******************************************************************************
 * @brief
 * Get the UART/USART mode programmed
 *
 * @details
 * This function return the mode set in the module
 *
 * @note
 * This function should call after USART Initliazation ,configuration
 ******************************************************************************/
static uint8_t sli_si91x_usart_get_mode(uint8_t usart_module)
{
  return USART_GetMode(usart_module);
}

/*******************************************************************************
 * @brief
 * To validate the ARM error code
 *
 * @details
 * While calling the CMSIS APIs, it will return the ARM Error codes.
 * This function will convert the ARM error codes into SL error codes.
 * It takes argument as 32 bit signed integer and returns the SL error type.
 * It has a single switch statement which is mapped with the SL error code and
 * after successful conversion it will break the switch statement.
 * If the error code is not listed, by default it will be SL_STATUS_FAIL.
 ******************************************************************************/
static sl_status_t convert_arm_to_sl_error_code(int32_t error)
{
  sl_status_t status = SL_STATUS_FAIL;
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

    case ARM_USART_ERROR_MODE:
      status = SL_STATUS_INVALID_MODE;
      break;
    case ARM_USART_ERROR_BAUDRATE:
      status = SL_STATUS_INVALID_RANGE;
      break;
    case ARM_USART_ERROR_DATA_BITS:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ARM_USART_ERROR_PARITY:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ARM_USART_ERROR_STOP_BITS:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ARM_USART_ERROR_FLOW_CONTROL:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ARM_USART_ERROR_CPOL:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ARM_USART_ERROR_CPHA:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    default:
      status = SL_STATUS_FAIL;
      break;
  }
  return status;
}

/*******************************************************************************
 * @brief
 * To validate the control parameters and build a 32 bit integer
 *
 * @details
 * It takes pointer to control configuration structure and pointer to the uint32_t
 * as argument and builds the integer using the validations and 'OR' operation.
 * According to the values in control configuration structure, it will 'OR' the values.
 ******************************************************************************/
static sl_status_t validate_control_parameters(sl_si91x_usart_control_config_t *control_configuration,
                                               uint32_t *input_mode)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    if ((control_configuration == NULL) || (input_mode == NULL)) {
      status = SL_STATUS_NULL_POINTER;
    }
    // Append all the input control parameters
    *input_mode =
      (*input_mode | control_configuration->mode | control_configuration->parity | control_configuration->stopbits
       | control_configuration->hwflowcontrol | control_configuration->databits | control_configuration->misc_control);
  } while (false);
  status = SL_STATUS_OK;
  return status;
}

/*******************************************************************************
 * Static callback function for handling the callbacks
 ******************************************************************************/
static void callback_event_handler(uint32_t event)
{
  uint8_t usart_instance = 0;
  // To get the usart instance
  usart_instance = (uint8_t)(event >> USART_INSTANCE_BIT);
  // Remove USART instance by anding with USART instance mask bit
  event &= (USART_EVENT_MASK);

  switch (event) {
    case SL_USART_EVENT_SEND_COMPLETE:
      sl_usart_send_complete = true;
      break;
    case SL_USART_EVENT_RECEIVE_COMPLETE:
      sl_usart_receive_complete = true;
      break;
    case SL_USART_EVENT_TRANSFER_COMPLETE:
      break;
  }
  // Call user callback if registered for this USART instance
  if (user_callback[usart_instance] != NULL) {
    user_callback[usart_instance](event);
  }
}

#if (SLI_SI91X_MCU_RS485_MODE == 1)
/*******************************************************************************
 * This API is used to configure GPIO pins for RS485
 ******************************************************************************/
static sl_status_t configure_rs485_gpio_pin(sl_rs485_gpio_pin_config_t *pin_config)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    // Validates the null pointer, if true returns error code
    if (pin_config == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (pin_config->pin >= GPIO_MAX_PIN) {
      sl_si91x_gpio_pin_config_t gpio_config = { .port_pin = { .port = SL_GPIO_ULP_PORT,
                                                               .pin  = (pin_config->pin - GPIO_MAX_PIN) },
                                                 GPIO_OUTPUT };
      sl_gpio_t sl_gpio_pin_config           = { .port = SL_GPIO_ULP_PORT, .pin = (pin_config->pin - GPIO_MAX_PIN) };
      sl_si91x_gpio_pin_config_t sl_si91x_hp_gpio_pin_config = { .port_pin = { .port = SL_GPIO_PORT_A,
                                                                               .pin  = (pin_config->pin) },
                                                                 GPIO_OUTPUT };
      sl_gpio_set_configuration(gpio_config);
      sl_gpio_driver_set_pin_mode(&gpio_config.port_pin, (sl_gpio_mode_t)SL_GPIO_MODE_6, OUTPUT);
#if defined(UART1_RS485_MODE)
      sl_si91x_gpio_driver_set_soc_peri_on_ulp_pin_mode(&sl_gpio_pin_config, SL_GPIO_MODE_6);
#endif
#if defined(UART0_RS485_MODE)
      // Check GPIO pin for UART0
      if (gpio_config.port_pin.pin == SL_SI91X_ULP_GPIO_5_PIN || gpio_config.port_pin.pin == SL_SI91X_ULP_GPIO_6_PIN
          || gpio_config.port_pin.pin == SL_SI91X_ULP_GPIO_7_PIN) {
        // Set mode to 11 for USART0 pins ULP GPIO's 5, 6, or 7
        sl_si91x_gpio_driver_set_soc_peri_on_ulp_pin_mode(&sl_gpio_pin_config, SL_GPIO_MODE_11);
      } else if (gpio_config.port_pin.pin == SL_SI91X_ULP_GPIO_9_PIN
#ifndef SL_SI91X_ACX_MODULE
                 || gpio_config.port_pin.pin == SL_SI91X_ULP_GPIO_10_PIN
#endif
                 || gpio_config.port_pin.pin == SL_SI91X_ULP_GPIO_11_PIN) {
        // Set mode to 2 for USART0 pins ULP GPIO's 9, 10, or 11
        sl_si91x_gpio_driver_set_soc_peri_on_ulp_pin_mode(&sl_gpio_pin_config, SL_GPIO_MODE_2);
      }
#endif
      sl_si91x_gpio_set_pin_direction(sl_si91x_hp_gpio_pin_config.port_pin.port,
                                      sl_si91x_hp_gpio_pin_config.port_pin.pin,
                                      (sl_si91x_gpio_direction_t)sl_si91x_hp_gpio_pin_config.direction);

    } else {
      sl_si91x_gpio_pin_config_t gpio_config = {
        .port_pin = { .port = pin_config->port, .pin = pin_config->pin },
      };
      sl_gpio_set_configuration(gpio_config);
      sl_gpio_set_pin_mode(pin_config->port, pin_config->pin, pin_config->mux, OUTPUT);
    }
    status = SL_STATUS_OK;
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * To Configure the RS485 lines
 *
 * @details
 * This API is used to configure the RS485 DE, RE and EN lines.
 ******************************************************************************/
static sl_status_t sl_si91x_uart_rs485_gpio_init(sl_uart_rs485_pin_init_t *rs485_init)
{
  sl_status_t status = SL_STATUS_FAIL;
  // Validates the null pointer, if true returns error code
  if (rs485_init == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    // Configure RS485 pins
    status = configure_rs485_gpio_pin(&rs485_init->de); // RS485 DE pin mux
    if (status != SL_STATUS_OK) {
      return status;
    }
    status = configure_rs485_gpio_pin(&rs485_init->re); // RS485 RE pin mux
    if (status != SL_STATUS_OK) {
      return status;
    }
    status = configure_rs485_gpio_pin(&rs485_init->rs485_en); // RS485 EN pin mux
    if (status != SL_STATUS_OK) {
      return status;
    }
  }
  return status;
}

/*****************************************************************************
 * @brief 
 * Initialize the RS485 Module.
 *
 * @details
 * This API is used to initialize the RS485 Instance.
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_init(usart_peripheral_t uart_instance)
{
  sl_status_t status = SL_STATUS_FAIL;

  sl_uart_rs485_pin_init_t pin_init;
  do {
    // Check for USART instance valid
    if (uart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    if (uart_instance == USART_0) {
#if (RS485_USART_UC == 1)
      pin_init = uart0_pin_init;
#endif
      status = sl_si91x_uart_rs485_gpio_init(&pin_init);
      if (status != SL_STATUS_OK) {
        return status;
      }
      rsi_usart_rs485_enable(UART0);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == UART_1) {
#if (RS485_UART_UC == 1)
      pin_init = uart1_pin_init;
#endif
      status = sl_si91x_uart_rs485_gpio_init(&pin_init);
      if (status != SL_STATUS_OK) {
        return status;
      }
      rsi_usart_rs485_enable(UART1);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == ULPUART) {
      status = SL_STATUS_NOT_SUPPORTED;
    }
  } while (false);
  return status;
}

/*****************************************************************************
 * Configures the RS485 configurations.
 ******************************************************************************/
void sl_si91x_get_uart_rs485_configure(usart_rs485_config_t *uart_rs485_configs)
{
#if (RS485_USART_UC == 1)
  *uart_rs485_configs = uart0_rs485_configuration;
#endif
#if (RS485_UART_UC == 1)
  *uart_rs485_configs = uart1_rs485_configuration;
#endif
}

/*****************************************************************************
 * @brief
 * Configures the RS485 configurations.
 *
 * @details
 * This API configures the RS485 configurations such as transfer mode, Driver enable Assertion, Driver enable De-assertion time, DE pol, RE pol 
 * and Multidrop configurations such as data bit 9, addr match, send addr etc. 
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_set_configuration(usart_peripheral_t uart_instance)
{
  sl_status_t status = SL_STATUS_FAIL;
  usart_rs485_config_t rs485_configs;
  do {
    // Check for USART instance valid
    if (uart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    if (uart_instance == USART_0) {
      sl_si91x_get_uart_rs485_configure(&rs485_configs);
      rsi_usart_rs485_configure(UART0, rs485_configs);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == UART_1) {
      sl_si91x_get_uart_rs485_configure(&rs485_configs);
      rsi_usart_rs485_configure(UART1, rs485_configs);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == ULPUART) {
      status = SL_STATUS_NOT_SUPPORTED;
    }
  } while (false);
  return status;
}

/*****************************************************************************
 * @details
 * This API enable or disable the DE(Driver Enable) signal 
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_de_enable(usart_peripheral_t uart_instance, bool enable)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    // Check for USART instance valid
    if (uart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    if (uart_instance == USART_0) {
      rsi_usart_rs485_de_enable(UART0, enable);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == UART_1) {
      rsi_usart_rs485_de_enable(UART1, enable);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == ULPUART) {
      status = SL_STATUS_NOT_SUPPORTED;
    }
  } while (false);
  return status;
}

/*****************************************************************************
 * @details
 * This API enable or disable the RE(Receiver Enable) signal 
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_re_enable(usart_peripheral_t uart_instance, bool enable)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    // Check for USART instance valid
    if (uart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    if (uart_instance == USART_0) {
      rsi_usart_rs485_re_enable(UART0, enable);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == UART_1) {
      rsi_usart_rs485_re_enable(UART1, enable);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == ULPUART) {
      status = SL_STATUS_NOT_SUPPORTED;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * To set the receive address in receive mode.
 *
 * @details
 * This API used to set the address to the module in receive mode .
 * @note
 * This API is useful only in Hardware address match receive mode i.e ADDR_MATCH is set to 1.
 * if ADDR_MATCH is set to 0, t always receives the 9-bit  data and pushes in to RxFIFO in 
 * FIFO mode or to the RBR register in non-FIFO mode. The user must compare the address 
 * whenever address byte is received and indicated through ADDR_RCVD bit in the Line Status 
 * register. .
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_rx_hardware_address_set(usart_peripheral_t uart_instance, uint8_t *rx_addr)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    // Check for USART instance valid
    if (uart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Check for rx_addr valid
    if (rx_addr == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    if (uart_instance == USART_0) {
      rsi_usart_rs485_rx_address_set(UART0, *rx_addr);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == UART_1) {
      rsi_usart_rs485_rx_address_set(UART1, *rx_addr);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == ULPUART) {
      status = SL_STATUS_NOT_SUPPORTED;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * This is used wait till address received in case of receive mode.
 *
 * @details
 * This API wait till the address received in receive mode  
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_address_received(usart_peripheral_t uart_instance)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    // Check for USART instance valid
    if (uart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    if (uart_instance == USART_0) {
      uint32_t usart_tick_count = get_usart_tick_count(TIMEOUT);
      // Waiting for read request or timeout, any one event can terminate the loop. Here the timeout is of 2 sec.
      while (!(UART0->LSR_b.ADDR_RCVD || (usart_tick_count-- <= TIMEOUT_EXPIRY)))
        ;
      if (usart_tick_count <= TIMEOUT_EXPIRY) {
        // The loop terminated due to a timeout. Exiting the function with an error code.
        status = SL_STATUS_TIMEOUT;

      } else {
        status = SL_STATUS_OK;
      }
      break;
    } else if (uart_instance == UART_1) {
      uint32_t usart_tick_count = get_usart_tick_count(TIMEOUT);
      // Waiting for read request or timeout, any one event can terminate the loop. Here the timeout is of 2 sec.
      while (!(UART1->LSR_b.ADDR_RCVD || (usart_tick_count-- <= TIMEOUT_EXPIRY)))
        ;
      if (usart_tick_count <= TIMEOUT_EXPIRY) {
        // The loop terminated due to a timeout. Exiting the function with an error code.
        status = SL_STATUS_TIMEOUT;

      } else {
        status = SL_STATUS_OK;
      }
      break;
    } else if (uart_instance == ULPUART) {
      status = SL_STATUS_NOT_SUPPORTED;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * To transfer the address in transmit mode.
 *
 * @details
 * This API used to transfer the address during transmit mode
 * This is an address matching api during transmit mode 0. When 9th bit in character 
 * is set to 1 and remaining 8-bit address will be sent using this API.
 * @note
 * In transmit mode 1, uart Transmit holding lines are 9-bit wide and both address and data are programmed 
 * through this. The uart does not differentiate between address and data both are taken from the TxFIFO. 
 * The software must pack the 9th bit with 1/0 depending on whether address/data has to be sent.  
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_transfer_hardware_address(usart_peripheral_t uart_instance, uint8_t *tx_address)
{
  sl_status_t status = SL_STATUS_FAIL;
  usart_rs485_multidrop_t uart_rs485_multidrop;
  do {
    // Check for USART instance valid
    if (uart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }
    // Check for tx_address valid
    if (tx_address == NULL) {
      status = SL_STATUS_NULL_POINTER;
      break;
    }
    // Set the SLave address to which data wants to transmit from UC
#if (RS485_USART_UC == 1)
    uart_rs485_multidrop.send_addr = uart0_rs485_configuration.multidrop.send_addr;
#endif
#if (RS485_UART_UC == 1)
    uart_rs485_multidrop.send_addr = uart1_rs485_configuration.multidrop.send_addr;
#endif
    if (uart_instance == USART_0) {
      rsi_usart_rs485_transfer_address(UART0, uart_rs485_multidrop, *tx_address);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == UART_1) {
      rsi_usart_rs485_transfer_address(UART1, uart_rs485_multidrop, *tx_address);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == ULPUART) {
      status = SL_STATUS_NOT_SUPPORTED;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * To halt or resume the transfer
 *
 * @details
 * This api is used when application is required to fill the data bytes in the TxFIFO before sending the address on 
 * the uart line.Then this api need to call with enable bit set to 1 so that uart does not start sending out the data in the 
 * TxFIFO as data byte, Once the TxFIFO is filled, then call the rsi_rs485_transfer_address() api  and call thsi api with 
 * enable bit set as 0.  
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_resume_tx(usart_peripheral_t uart_instance, bool enable)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    // Check for USART instance valid
    if (uart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    if (uart_instance == USART_0) {
      rsi_usart_rs485_resume_tx(UART0, enable);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == UART_1) {
      rsi_usart_rs485_resume_tx(UART1, enable);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == ULPUART) {
      status = SL_STATUS_NOT_SUPPORTED;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * Deinit the RS485 module
 *
 * @details
 * This function deinit the RS485 and return the SL_STATUS_OK on success and 
 * error code on invalid usart instance  
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_deinit(usart_peripheral_t uart_instance)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    // Check for USART instance valid
    if (uart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    if (uart_instance == USART_0) {
      rsi_usart_rs485_disable(UART0);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == UART_1) {
      rsi_usart_rs485_disable(UART1);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == ULPUART) {
      status = SL_STATUS_NOT_SUPPORTED;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief
 * Enable/Disable send address bit while transferring address
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_control_send_address(usart_peripheral_t uart_instance, bool send_addr_ctrl)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    // Check for USART instance valid
    if (uart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    if (uart_instance == USART_0) {
      rsi_usart_rs485_control_send_address(UART0, send_addr_ctrl);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == UART_1) {
      rsi_usart_rs485_control_send_address(UART1, send_addr_ctrl);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == ULPUART) {
      status = SL_STATUS_NOT_SUPPORTED;
    }
  } while (false);
  return status;
}

/*******************************************************************************
* @brief
* Enable/Disable address match bit while receiving.
******************************************************************************/
sl_status_t sl_si91x_uart_rs485_control_address_match(usart_peripheral_t uart_instance, bool addr_match_ctrl)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    // Check for USART instance valid
    if (uart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    if (uart_instance == USART_0) {
      rsi_usart_rs485_control_address_match(UART0, addr_match_ctrl);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == UART_1) {
      rsi_usart_rs485_control_address_match(UART1, addr_match_ctrl);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == ULPUART) {
      status = SL_STATUS_NOT_SUPPORTED;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief 
 * Enables or disables the Driver Enable assertion time for RS-485.
 *
 * This function configures the duration for which the Driver Enable (DE) signal
 * is asserted before the start of data transmission for the specified UART instance.
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_set_de_assert_time(usart_peripheral_t uart_instance, uint8_t assert_time)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    // Check for USART instance valid
    if (uart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    if (uart_instance == USART_0) {
      rsi_usart_rs485_set_de_assert_time(UART0, assert_time);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == UART_1) {
      rsi_usart_rs485_set_de_assert_time(UART1, assert_time);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == ULPUART) {
      status = SL_STATUS_NOT_SUPPORTED;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief 
 * Enables or disables the Driver Enable de-assertion time for RS-485.
 *
 * This function configures the duration for which the Driver Enable (DE) signal
 * remains asserted after the completion of data transmission for the specified
 * UART instance.
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_set_de_deassert_time(usart_peripheral_t uart_instance, uint8_t deassert_time)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    // Check for USART instance valid
    if (uart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    if (uart_instance == USART_0) {
      rsi_usart_rs485_set_de_deassert_time(UART0, deassert_time);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == UART_1) {
      rsi_usart_rs485_set_de_deassert_time(UART1, deassert_time);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == ULPUART) {
      status = SL_STATUS_NOT_SUPPORTED;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief 
 * Sets the Driver Enable to Receiver Enable turnaround time for RS-485.
 *
 * This function configures the delay between de-asserting the Driver Enable (DE)
 * signal and asserting the Receiver Enable (RE) signal when switching from
 * transmit to receive mode for the specified UART instance.
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_set_de_re_turnaround_time(usart_peripheral_t uart_instance, uint16_t de_re_tat)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    // Check for USART instance valid
    if (uart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    if (uart_instance == USART_0) {
      rsi_usart_rs485_set_de_re_turnaround_time(UART0, de_re_tat);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == UART_1) {
      rsi_usart_rs485_set_de_re_turnaround_time(UART1, de_re_tat);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == ULPUART) {
      status = SL_STATUS_NOT_SUPPORTED;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief 
 * Sets the Receiver Enable to Driver Enable turnaround time for RS-485.
 *
 * This function configures the delay between de-asserting the Receiver Enable (RE)
 * signal and asserting the Driver Enable (DE) signal when switching from
 * receive to transmit mode for the specified UART instance. 
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_set_re_de_turnaround_time(usart_peripheral_t uart_instance, uint16_t re_de_tat)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    // Check for USART instance valid
    if (uart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    if (uart_instance == USART_0) {
      rsi_usart_rs485_set_re_de_turnaround_time(UART0, re_de_tat);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == UART_1) {
      rsi_usart_rs485_set_re_de_turnaround_time(UART1, re_de_tat);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == ULPUART) {
      status = SL_STATUS_NOT_SUPPORTED;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief 
 * Sets the Driver Enable polarity for RS-485 for the specified UART instance.
 *
 * This function configures the active level of the Driver Enable (DE) signal.
  ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_set_de_polarity(usart_peripheral_t uart_instance, uint8_t de_polarity)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    // Check for USART instance valid
    if (uart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    if (uart_instance == USART_0) {
      rsi_usart_rs485_set_de_polarity(UART0, de_polarity);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == UART_1) {
      rsi_usart_rs485_set_de_polarity(UART1, de_polarity);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == ULPUART) {
      status = SL_STATUS_NOT_SUPPORTED;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * @brief 
 * Sets the Receiver Enable polarity for RS-485 for the specified UART instance.
 *
 * This function configures the active level of the Receiver Enable (RE) signal.
 ******************************************************************************/
sl_status_t sl_si91x_uart_rs485_set_re_polarity(usart_peripheral_t uart_instance, uint8_t re_polarity)
{
  sl_status_t status = SL_STATUS_FAIL;
  do {
    // Check for USART instance valid
    if (uart_instance >= UARTLAST) {
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    }

    if (uart_instance == USART_0) {
      rsi_usart_rs485_set_re_polarity(UART0, re_polarity);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == UART_1) {
      rsi_usart_rs485_set_re_polarity(UART1, re_polarity);
      status = SL_STATUS_OK;
      break;
    } else if (uart_instance == ULPUART) {
      status = SL_STATUS_NOT_SUPPORTED;
    }
  } while (false);
  return status;
}

/*******************************************************************************
 * Function to get the tick_count value based on the current system clock frequency
 * clock_frequency has the current system clock frequency.
 *
 * Here the clock is divided by CONVERT_TO_MS to get the value in milliseconds, and then
 * it is multiplied by the input value which is in ms.
 * To match the expected delay, the final value is divided by division factor based on current system frequency.
 * @param time_ms (uint32_t) Timeout value in ms.
 * @return tick_count Tick values as per the clock frequency and input ms value.
 ******************************************************************************/
static uint32_t get_usart_tick_count(uint32_t time_ms)
{
  uint32_t tick_count      = 0;
  uint8_t division_factor  = 0;
  uint32_t clock_frequency = 0;
  sl_si91x_clock_manager_m4_get_core_clk_src_freq(&clock_frequency);
  if (clock_frequency >= SYSTEM_CLOCK_180MHZ) {
    division_factor = DIVISION_FACTOR_180MHZ;
  } else if (clock_frequency >= SYSTEM_CLOCK_90MHZ) {
    division_factor = DIVISION_FACTOR_90MHZ;
  } else {
    division_factor = DIVISION_FACTOR_32MHZ;
  }
  // tick_count value is updated with the ticks according to the input value and clock freq.
  tick_count = ((clock_frequency / CONVERT_TO_MS) * time_ms) / division_factor;
  return tick_count;
}

#endif