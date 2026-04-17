/***************************************************************************/ /**
 * @file usart_example.c
 * @brief USART examples functions
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

#include "app.h"
#include "sl_si91x_usart.h"
#include "rsi_debug.h"
#include "usart_app.h"
#include "cmsis_os2.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define USART_BAUDRATE 115200 // Baud rate <9600-7372800>
#define NON_UC_DEFAULT_CONFIG \
  0 //  Enable this macro to set the default configurations in non_uc case, this is useful when someone don't want to use UC configuration

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static uint8_t usart_data_in[BUFFER_SIZE];
static uint8_t usart_data_out[BUFFER_SIZE];

volatile boolean_t send_complete = false, transfer_complete = false, receive_complete = false;
static boolean_t begin_transmission = true;

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
void callback_event(uint32_t event);
/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/
sl_usart_handle_t usart_handle;
usart_mode_enum_t current_mode = SL_SEND_DATA;

osMessageQueueId_t mid_usart_msg_queue;      // message queue id
extern osMessageQueueId_t mid_i2c_msg_queue; // message queue id
/*******************************************************************************
 * USART Example Initialization function
 ******************************************************************************/
void usart_example_init(void)
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
    // Initialize the USART
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
    status = sl_si91x_usart_register_event_callback(callback_event);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_usart_register_event_callback: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("USART user event callback registered successfully \n");
    sl_si91x_usart_get_configurations(USART_0, &get_config);
#if SL_USART_SYNCH_MODE
    DEBUGOUT("Baud Rate = %ld \n", (get_config.baudrate << 3));
#else
    DEBUGOUT("Baud Rate = %ld \n", get_config.baudrate);
#endif

    // create and initialize message queue object for USART Rx msgs
    mid_usart_msg_queue = osMessageQueueNew(MSGQUEUE_OBJECTS, BUFFER_SIZE, NULL);
    if (mid_usart_msg_queue == NULL) {
      ; // Message Queue object not created, handle failure
    }

  } while (false);
}

/*******************************************************************************
 * Example ticking function
 ******************************************************************************/
void usart_example_process_action(void)
{
  sl_status_t status;
  osStatus_t os_status;

  while (1) {
    // In this switch case, according to the macros enabled in header file, it starts to execute the APIs
    // Assuming all the macros are enabled, after transfer, receive will be executed and after receive
    // send will be executed.
    switch (current_mode) {
      case SL_SEND_DATA:
#if (SL_USART_SYNCH_MODE != ENABLE)
        // wait for the usart_data_out buffer to be filled from i2c reception
        os_status = osMessageQueueGet(mid_i2c_msg_queue, usart_data_out, NULL, osWaitForever); // wait for message
        if (os_status != osOK) {
          DEBUGOUT("I2C Message Queue read failed\n");
          break;
        }

        DEBUGOUT("USART_Task: Data read from I2C_Message_Queue successfully and performing loopback\n");

        // Validation for executing the API only once
        if (begin_transmission == true) {
          status = sl_si91x_usart_send_data(usart_handle, usart_data_out, sizeof(usart_data_out));
          if (status != SL_STATUS_OK) {
            // If it fails to execute the API, it will not execute rest of the things
            DEBUGOUT("sl_si91x_usart_send_data: Error Code : %lu \n", status);
            current_mode = SL_TRANSMISSION_COMPLETED;
            break;
          }
          begin_transmission = false;
        }

        send_complete = false;

        DEBUGOUT("USART send completed successfully \n");

        if (USE_RECEIVE) {
          // If receive macro is enabled, current mode is set to receive
          current_mode       = SL_RECEIVE_DATA;
          begin_transmission = true;
          break;
        }

        // Current mode is set to complete
        current_mode = SL_TRANSMISSION_COMPLETED;
#endif
        break;

      case SL_RECEIVE_DATA:
#if (SL_USART_SYNCH_MODE != ENABLE)
        if (begin_transmission == true) {
          // Validation for executing the API only once
          status = sl_si91x_usart_receive_data(usart_handle, usart_data_in, sizeof(usart_data_in));
          if (status != SL_STATUS_OK) {
            // If it fails to execute the API, it will not execute rest of the things
            DEBUGOUT("sl_si91x_usart_receive_data: Error Code : %lu \n", status);
            current_mode = SL_TRANSMISSION_COMPLETED;
            break;
          }
          DEBUGOUT("USART receive begin successfully \n");
          begin_transmission = false;
        }
        //Waiting till the receive is completed
        if (receive_complete) {
          // Update the receive compelete flag with 0.
          receive_complete = false;

          if (USE_SEND) {
            // If send macro is enabled, current mode is set to send
            current_mode       = SL_SEND_DATA;
            begin_transmission = true;
            DEBUGOUT("USART receive completed \n");
            break;
          }

          DEBUGOUT("USART receive completed \n");

          // put the received msg into MessageQueue which will be taken by I2C
          os_status = osMessageQueuePut(mid_usart_msg_queue, usart_data_in, 0U, 0U);
          if (os_status != osOK) {
            DEBUGOUT("USART Message Queue write failed\n");
            break;
          }
          DEBUGOUT("USART_Task: USART Loopback data read successfully and written into USART_Message_Queue\n");

          // If send macro is not enabled, current mode is set to completed.
          current_mode = SL_TRANSMISSION_COMPLETED;
        }
#endif
        break;
      case SL_TRANSFER_DATA:
#if (SL_USART_SYNCH_MODE == ENABLE)
        // Validation for executing the API only once
        if (begin_transmission) {
          // Fill the data buffer to be send
          for (int i = 0; i < BUFFER_SIZE; i++) {
            usart_data_out[i] = (uint8_t)i + 1;
          }

          status = sl_si91x_usart_transfer_data(usart_handle, usart_data_out, usart_data_in, sizeof(usart_data_out));
          if (status != SL_STATUS_OK) {
            // If it fails to execute the API, it will not execute rest of the things
            DEBUGOUT("sl_si91x_usart_transfer_data: Error Code : %lu \n", status);
            current_mode = SL_TRANSMISSION_COMPLETED;
            break;
          }
          DEBUGOUT("USART transfer begin successfully \n");
          begin_transmission = false;
        }

        //Waiting till the transfer is completed
        if (transfer_complete) {
          // Update transfer_complete flag with 0.
          transfer_complete = false;

          // At last current mode is set to completed.
          current_mode = SL_TRANSMISSION_COMPLETED;
          DEBUGOUT("USART transfer completed \n");
        }
#endif
        break;

      case SL_TRANSMISSION_COMPLETED:
        osDelay(100);
        break;
    }
  }
}

/*******************************************************************************
 * Callback function triggered on data Transfer and reception
 ******************************************************************************/
void callback_event(uint32_t event)
{
  switch (event) {
    case SL_USART_EVENT_SEND_COMPLETE:
      send_complete = true;
      break;
    case SL_USART_EVENT_RECEIVE_COMPLETE:
      receive_complete = true;
      break;
    case SL_USART_EVENT_TRANSFER_COMPLETE:
      transfer_complete = true;
      break;
  }
}
