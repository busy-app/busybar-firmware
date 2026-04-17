/***************************************************************************/ /**
 * @file usart_async_example.c
 * @brief USART Asynchronous examples functions
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "sl_si91x_usart.h"
#include "rsi_debug.h"
#include "usart_async_example.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define USART_BUFFER_SIZE 1024   // Data send and receive length
#define USART_BAUDRATE    115200 // Baud rate <9600-7372800>
#define NON_UC_DEFAULT_CONFIG \
  0 //  Enable this macro to set the default configurations in non_uc case, this is useful when someone don't want to use UC configuration

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static uint8_t usart_data_in[USART_BUFFER_SIZE];
static uint8_t usart_data_out[USART_BUFFER_SIZE];

volatile boolean_t usart_send_complete = false, usart_transfer_complete = false, usart_receive_complete = false;
static boolean_t usart_begin_transmission = true;

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
void usart_callback_event(uint32_t event);
static void compare_loop_back_data(void);
/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/
sl_usart_handle_t usart_handle;
usart_mode_enum_t current_mode = SL_USART_SEND_DATA;
/*******************************************************************************
 * USART Example Initialization function
 ******************************************************************************/
void usart_async_example_init(void)
{
  sl_status_t status;
  sl_si91x_usart_control_config_t usart_config;

#if NON_UC_DEFAULT_CONFIG
  usart_config.baudrate      = USART_BAUDRATE;
  usart_config.mode          = SL_USART_MODE_ASYNCHRONOUS;
  usart_config.parity        = SL_USART_NO_PARITY;
  usart_config.stopbits      = SL_USART_STOP_BITS_1;
  usart_config.hwflowcontrol = SL_USART_FLOW_CONTROL_NONE;
  usart_config.databits      = SL_USART_DATA_BITS_8;
  usart_config.misc_control  = SL_USART_MISC_CONTROL_NONE;
  usart_config.usart_module  = USART_0;
  usart_config.config_enable = ENABLE;
  usart_config.synch_mode    = DISABLE;
#endif
  sl_si91x_usart_control_config_t get_config;

  do {
    // Initialize the UART
    status = sl_si91x_usart_init(USART_0, &usart_handle);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_usart_initialize: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("USART initialization is successful \n");
    // Configure the USART configurations
    status = sl_si91x_usart_set_configuration(usart_handle, &usart_config);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_usart_set_configuration: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("USART configuration is successful \n");
    // Register user callback function
    status = sl_si91x_usart_multiple_instance_register_event_callback(USART_0, usart_callback_event);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_usart_multiple_instance_register_event_callback: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("USART user event callback registered successfully \n");
    sl_si91x_usart_get_configurations(USART_0, &get_config);
    DEBUGOUT("Baud Rate = %ld \n", get_config.baudrate);
  } while (false);
}

/*******************************************************************************
 * Example ticking function
 ******************************************************************************/
void usart_async_example_process_action(void)
{
  sl_status_t status;
  // In this switch case, according to the macros enabled in header file, it starts to execute the APIs
  // Assuming all the macros are enabled, after transfer, receive will be executed and after receive
  // send will be executed.
  switch (current_mode) {
    case SL_USART_SEND_DATA:
      // Validation for executing the API only once
      if (usart_begin_transmission == true) {
        // Fill the data buffer to be send
        for (uint16_t i = 0; i < USART_BUFFER_SIZE; i++) {
          usart_data_out[i] = (uint8_t)(i + 1);
        }
        status = sl_si91x_usart_async_send_data(usart_handle, usart_data_out, sizeof(usart_data_out));
        if (status != SL_STATUS_OK) {
          // If it fails to execute the API, it will not execute rest of the things
          DEBUGOUT("sl_si91x_usart_async_send_data: Error Code : %lu \n", status);
          current_mode = SL_USART_TRANSMISSION_COMPLETED;
          break;
        }
        usart_begin_transmission = false;
      }

      usart_send_complete = false;

      if (USE_RECEIVE) {
        // If receive macro is enabled, current mode is set to receive
        current_mode             = SL_USART_RECEIVE_DATA;
        usart_begin_transmission = true;
        break;
      }
      DEBUGOUT("USART send completed successfully \n");
      // Current mode is set to complete
      current_mode = SL_USART_TRANSMISSION_COMPLETED;
      break;

    case SL_USART_RECEIVE_DATA:
      if (usart_begin_transmission == true) {
        // Validation for executing the API only once
        status = sl_si91x_usart_async_receive_data(usart_handle, usart_data_in, sizeof(usart_data_in));
        if (status != SL_STATUS_OK) {
          // If it fails to execute the API, it will not execute rest of the things
          DEBUGOUT("sl_si91x_usart_async_receive_data: Error Code : %lu \n", status);
          current_mode = SL_USART_TRANSMISSION_COMPLETED;
          break;
        }
        DEBUGOUT("USART receive begin successfully \n");
        usart_begin_transmission = false;
      }
      //Waiting till the receive is completed
      if (usart_receive_complete) {
        // Update the receive compelete flag with 0.
        usart_receive_complete = false;
        if (USE_SEND) {
          // If send macro is enabled, current mode is set to send
          current_mode             = SL_USART_SEND_DATA;
          usart_begin_transmission = true;
          DEBUGOUT("USART receive completed \n");
          break;
        }

        DEBUGOUT("USART send completed successfully \n");
        DEBUGOUT("USART receive completed \n");
        compare_loop_back_data();
        // If send macro is not enabled, current mode is set to completed.
        current_mode = SL_USART_TRANSMISSION_COMPLETED;
      }
      break;

    case SL_USART_TRANSMISSION_COMPLETED:
      break;
  }
}

/*******************************************************************************
 * Compares data received buffer via USART with data transfer buffer and print
 * the loopback test passed or failed
 ******************************************************************************/
static void compare_loop_back_data(void)
{
  uint16_t data_index = 0;
  // Check for data in and data out are same, if same then comparision
  // will continue till end of the buffer
  for (data_index = 0; data_index < USART_BUFFER_SIZE; data_index++) {
    if (usart_data_in[data_index] != usart_data_out[data_index]) {
      break;
    }
  }
  if (data_index == USART_BUFFER_SIZE) {
    DEBUGOUT("Data comparison successful, Loop Back Test Passed \n");
  } else {
    DEBUGOUT("Data comparison failed, Loop Back Test failed \n");
  }
}

/*******************************************************************************
 * Callback function triggered on data Transfer and reception
 ******************************************************************************/
void usart_callback_event(uint32_t event)
{
  switch (event) {
    case SL_USART_EVENT_SEND_COMPLETE:
      usart_send_complete = true;
      break;
    case SL_USART_EVENT_RECEIVE_COMPLETE:
      usart_receive_complete = true;
      break;
    case SL_USART_EVENT_TRANSFER_COMPLETE:
      usart_transfer_complete = true;
      break;
  }
}
