/***************************************************************************/ /**
 * @file i2c_follower_example.c
 * @brief I2C examples functions
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
#include "sl_si91x_peripheral_i2c.h"
#include "i2c_follower_app.h"
#include "rsi_debug.h"
#include "cmsis_os2.h"
#include "sl_si91x_i2c.h"
#include "sl_i2c_instances.h"
#include "sl_component_catalog.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define OWN_I2C_ADDR           0x50    // Own I2C address
#define DUMMY_FOLLOWER_ADDRESS 0x00    // In Follower mode, the I2C follower address is ignored, so kept the value as 0
#define FIFO_THRESHOLD         0x0     // FIFO threshold
#define ZERO_FLAG              0       // Zero flag, No argument
#define I2C                    ULP_I2C // I2C Instance
#define MAX_7BIT_ADDRESS       127     // Maximum 7-bit address
#define I2C_TX_FIFO_THRESHOLD  0       // FIFO threshold
#define I2C_RX_FIFO_THRESHOLD  0       // FIFO threshold
/*******************************************************************************
 ******************************  Data Types  ***********************************
 ******************************************************************************/
// Structure to hold the pin configuration
typedef const struct {
  uint8_t port;    // GPIO port
  uint8_t pin;     // GPIO pin
  uint8_t mode;    // GPIO mode
  uint8_t pad_sel; // GPIO pad selection
} I2C_PIN_;

// Enum for different transmission scenarios
typedef enum {
  SEND_DATA,              // Send mode
  RECEIVE_DATA,           // Receive mode
  TRANSMISSION_COMPLETED, // Transmission completed mode
} i2c_mode_enum_t;

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static sl_i2c_instance_t i2c_instance;
static i2c_mode_enum_t current_mode = RECEIVE_DATA;
boolean_t i2c_send_complete         = false;
boolean_t i2c_receive_complete      = false;
boolean_t send_data_flag            = false;
boolean_t receive_data_flag         = false;
static uint8_t i2c_write_buffer[BUFFER_SIZE];
static uint8_t i2c_read_buffer[BUFFER_SIZE];
static sl_i2c_config_t sl_i2c_config;
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void compare_data(void);

/*******************************************************************************
 *************************** GLOBAL VARIABLES   *******************************
 ******************************************************************************/
osMessageQueueId_t mid_i2c_msg_queue;          // message queue id
extern osMessageQueueId_t mid_usart_msg_queue; // message queue id
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * I2C example initialization function
 ******************************************************************************/
void i2c_follower_example_init(void)
{
  sl_i2c_status_t i2c_status;

#if defined(SL_CATALOG_I2C_I2C0_PRESENT)
  i2c_instance  = SL_I2C0;
  sl_i2c_config = sl_i2c_i2c0_config;
#elif defined(SL_CATALOG_I2C_I2C1_PRESENT)
  i2c_instance  = SL_I2C1;
  sl_i2c_config = sl_i2c_i2c1_config;
#elif defined(SL_CATALOG_I2C_I2C2_PRESENT)
  i2c_instance  = SL_ULP_I2C;
  sl_i2c_config = sl_i2c_i2c2_config;
#endif

  // Initializing I2C instance
  i2c_status = sl_i2c_driver_init(i2c_instance, &sl_i2c_config);
  DEBUGINIT();
  if (i2c_status != SL_I2C_SUCCESS) {
    DEBUGOUT("sl_i2c_driver_init : Invalid Parameters, Error Code : %u \n", i2c_status);
  } else {
    DEBUGOUT("Successfully initialized i2c follower\n");
  }
  // Configuring follower mask address
  i2c_status = sl_i2c_driver_set_follower_address(i2c_instance, OWN_I2C_ADDR);
  if (i2c_status != SL_I2C_SUCCESS) {
    DEBUGOUT("sl_i2c_driver_init : Invalid Parameters, Error Code : %u \n", i2c_status);
  } else {
    DEBUGOUT("Successfully configured i2c follower address\n");
  }
  // Configuring RX and TX FIFO thresholds
  i2c_status = sl_i2c_driver_configure_fifo_threshold(i2c_instance, I2C_TX_FIFO_THRESHOLD, I2C_RX_FIFO_THRESHOLD);
  if (i2c_status != SL_I2C_SUCCESS) {
    DEBUGOUT("sl_i2c_driver_configure_fifo_threshold : Invalid Parameters, Error Code : %u \n", i2c_status);
  } else {
    DEBUGOUT("Successfully configured i2c TX & RX FIFO thresholds\n");
  }

  // Generating a buffer with values that needs to be sent.
  for (uint32_t loop = 0; loop < BUFFER_SIZE; loop++) {
    //    i2c_write_buffer[loop] = (loop + 0x1);
    i2c_read_buffer[loop] = 0;
  }
  receive_data_flag = true;

  // create and initialize message queue object for I2C Rx msgs
  mid_i2c_msg_queue = osMessageQueueNew(MSGQUEUE_OBJECTS, BUFFER_SIZE, NULL);
  if (mid_i2c_msg_queue == NULL) {
    ; // Message Queue object not created, handle failure
  }
}

/*******************************************************************************
 * Function will run continuously in while loop
 ******************************************************************************/
void i2c_follower_example_process_action(void)
{
  sl_i2c_status_t i2c_status;
  osStatus_t os_status;

  while (1) {
    // In switch case, according to the current mode, the transmission is executed.
    // First EEPROM write is executes i.e., I2C send.
    // After that offset address is sent to the follower device.
    // Now data is read from the EEPROM using I2C receive.
    switch (current_mode) {
      case RECEIVE_DATA:
        if (receive_data_flag) {
          // Validation for executing the API only once.
          i2c_status =
            sl_i2c_driver_receive_data_blocking(i2c_instance, DUMMY_FOLLOWER_ADDRESS, i2c_read_buffer, BUFFER_SIZE);
          if (i2c_status != SL_I2C_SUCCESS) {
            DEBUGOUT("sl_i2c_driver_receive_data_blocking : Invalid Parameters, Error Code : %u \n", i2c_status);
            if (i2c_status != SL_I2C_TIMEOUT) {
              receive_data_flag = false;
            }
            break;
          }
          receive_data_flag = false;

          // It waits till i2c_receive_complete is true in IRQ handler.
          current_mode         = SEND_DATA;
          i2c_receive_complete = false;
          send_data_flag       = true;

          DEBUGOUT("I2C_Task: Data read from Leader successfully and writing the same into I2C_Message_Queue\n");

          // put the received msg into MessageQueue which will be taken by USART
          os_status = osMessageQueuePut(mid_i2c_msg_queue, i2c_read_buffer, 0U, 0U);
          if (os_status != osOK) {
            DEBUGOUT("I2C Message Queue write failed\n");
            break;
          }
        }
        break;
      case SEND_DATA:
        if (send_data_flag) {
          // wait for the i2c_write_buffer buffer to be filled from uart reception
          os_status = osMessageQueueGet(mid_usart_msg_queue, i2c_write_buffer, NULL, osWaitForever); // wait for message
          if (os_status != osOK) {
            DEBUGOUT("USART Message Queue read failed\n");
            break;
          }
          DEBUGOUT("I2C_Task: Data read from USART_Message_Queue successfully and forwarding to Leader\n");

          // Validation for executing the API only once.
          i2c_status =
            sl_i2c_driver_send_data_blocking(i2c_instance, DUMMY_FOLLOWER_ADDRESS, i2c_write_buffer, BUFFER_SIZE);
          if (i2c_status != SL_I2C_SUCCESS) {
            DEBUGOUT("sl_i2c_driver_send_data_blocking : Invalid Parameters, Error Code : %u \n", i2c_status);
            if (i2c_status != SL_I2C_TIMEOUT) {
              send_data_flag = false;
            }
            break;
          }
          send_data_flag = false;
        }
        // It waits till i2c_send_complete is true in IRQ handler.
        current_mode = TRANSMISSION_COMPLETED;
        // After the receive is completed, input and output data is compared and
        // output is printed on console.
        compare_data();
        break;
      case TRANSMISSION_COMPLETED:
        break;
      default:
        break;
    }

    osDelay(10);
  }
}

/*******************************************************************************
 * Function to compare the input and output data
 * 
 * @param none
 * @return none
 ******************************************************************************/
static void compare_data(void)
{
  uint32_t data_index = 0;
  for (data_index = 0; data_index < BUFFER_SIZE; data_index++) {
    // If the i2c_read_buffer and buffer are same, it will be continued else, the for
    // loop will be break.
    if (i2c_write_buffer[data_index] != i2c_read_buffer[data_index]) {
      break;
    }
  }
  if (data_index == BUFFER_SIZE) {
    DEBUGOUT("Leader-follower read-write data comparison is successful, Test Case Passed \n");
  } else {
    DEBUGOUT("Leader-follower read-write data comparison is not successful, Test Case Failed \n");
  }
}
