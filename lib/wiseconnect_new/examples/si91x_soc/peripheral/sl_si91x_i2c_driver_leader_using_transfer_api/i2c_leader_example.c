/***************************************************************************/ /**
* @file i2c_leader_example.c
* @brief I2C DRIVER examples functions
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
#include "i2c_leader_example.h"
#include "rsi_debug.h"
#include "sl_i2c_instances.h"
#include "sl_si91x_i2c.h"
#include "sl_si91x_peripheral_i2c.h"
#include "sl_component_catalog.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define FOLLOWER_I2C_ADDR        0x50  // I2C follower address
#define MAX_BUFFER_SIZE_BLOCKING 80000 // Maximum buffer size for RX and TX length when transferring without DMA
#define I2C_BUFFER_SIZE          1024  // Size of data buffer
#define I2C_TX_FIFO_THRESHOLD    0     // FIFO threshold
#define I2C_RX_FIFO_THRESHOLD    0     // FIFO threshold
#define INITIAL_VALUE            0     // Initial value of buffer
#define BUFFER_OFFSET            0x1   // Buffer offset
/*******************************************************************************
 ******************************  Data Types  ***********************************
 ******************************************************************************/
// Enum for different transmission scenarios
typedef enum {
  I2C_TRANSFER_DATA,
  I2C_TRANSMISSION_COMPLETED, // Transmission completed mode
} i2c_action_enum_t;

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
sl_i2c_instance_t i2c_instance;
static uint8_t i2c_read_buffer[I2C_BUFFER_SIZE];
static uint8_t i2c_write_buffer[I2C_BUFFER_SIZE];
static i2c_action_enum_t current_mode   = I2C_TRANSFER_DATA;
static boolean_t i2c_transfer_data_flag = false;
static sl_i2c_config_t sl_i2c_config;

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void compare_data(void);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/*******************************************************************************
 * I2C example initialization function
 ******************************************************************************/
void i2c_leader_example_init(void)
{
  sl_i2c_status_t i2c_status;

  // Update structure name as per instance used, to register I2C callback
#if defined(SL_CATALOG_I2C_I2C0_PRESENT)
  sl_i2c_config = sl_i2c_i2c0_config;
  i2c_instance  = SL_I2C0;
#elif defined(SL_CATALOG_I2C_I2C1_PRESENT)
  sl_i2c_config = sl_i2c_i2c1_config;
  i2c_instance  = SL_I2C1;
#elif defined(SL_CATALOG_I2C_I2C2_PRESENT)
  sl_i2c_config = sl_i2c_i2c2_config;
  i2c_instance  = SL_ULP_I2C;
#endif

  // Initializing I2C instance (update i2c config-strucure name as per instance used)
  i2c_status = sl_i2c_driver_init(i2c_instance, &sl_i2c_config);
  if (i2c_status != SL_I2C_SUCCESS) {
    DEBUGOUT("sl_i2c_driver_init : Invalid Parameters, Error Code : %u \n", i2c_status);
  } else {
    DEBUGOUT("Successfully initialized and configured i2c leader\n");
  }
  // Configuring RX and TX FIFO thresholds
  i2c_status = sl_i2c_driver_configure_fifo_threshold(i2c_instance, I2C_TX_FIFO_THRESHOLD, I2C_RX_FIFO_THRESHOLD);
  if (i2c_status != SL_I2C_SUCCESS) {
    DEBUGOUT("sl_i2c_driver_configure_fifo_threshold : Invalid Parameters, Error Code : %u \n", i2c_status);
  } else {
    DEBUGOUT("Successfully configured i2c TX & RX FIFO thresholds\n");
  }
  // Enabling combined format transfer, by enabling repeated start
  i2c_status = sl_i2c_driver_enable_repeated_start(i2c_instance, true);
  if (i2c_status != SL_I2C_SUCCESS) {
    DEBUGOUT("sl_i2c_driver_enable_repeated_start : Invalid Parameters, Error Code : %u \n", i2c_status);
  } else {
    DEBUGOUT("Successfully enabled repeated start\n");
  }
  // Generating a buffer with values that needs to be sent.
  for (uint32_t loop = INITIAL_VALUE; loop < I2C_BUFFER_SIZE; loop++) {
    i2c_write_buffer[loop] = (uint8_t)(loop + BUFFER_OFFSET);
  }
  i2c_transfer_data_flag = true;
}

/*******************************************************************************
 * Function will run continuously in while loop
 ******************************************************************************/
void i2c_leader_example_process_action(void)
{
  sl_i2c_status_t i2c_status;
  // In switch case, according to the current mode, the transmission is
  // executed.
  // First leader sends data to follower, then leader receives same data from follower, using I2C transfer API.
  switch (current_mode) {
    case I2C_TRANSFER_DATA:
      if (i2c_transfer_data_flag) {
        sl_i2c_transfer_config_t i2c_transfer_config;
        i2c_transfer_config.rx_buffer = i2c_read_buffer;
        i2c_transfer_config.tx_buffer = i2c_write_buffer;
        i2c_transfer_config.rx_len    = I2C_BUFFER_SIZE;
        i2c_transfer_config.tx_len    = I2C_BUFFER_SIZE;
        //  Validation for executing the API only once.
        i2c_status = sl_i2c_driver_transfer_data(i2c_instance, &i2c_transfer_config, FOLLOWER_I2C_ADDR);
        if (i2c_status != SL_I2C_SUCCESS) {
          DEBUGOUT("sl_i2c_driver_transfer_data : Invalid Parameters, "
                   "Error Code : %u \n",
                   i2c_status);
          // If error occurs due to timeout then application will retry sending data else not
          if (i2c_status != SL_I2C_TIMEOUT) {
            i2c_transfer_data_flag = false;
          }
          break;
        }
        i2c_transfer_data_flag = false;
        compare_data();
        current_mode = I2C_TRANSMISSION_COMPLETED;
      }
      break;
    case I2C_TRANSMISSION_COMPLETED:
      // I2C will be Idle in this mode
    default:
      break;
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
  for (data_index = 0; data_index < I2C_BUFFER_SIZE; data_index++) {
    // If the i2c_read_buffer and buffer are same, it will be continued else, the
    // for loop will be break.
    if (i2c_write_buffer[data_index] != i2c_read_buffer[data_index]) {
      break;
    }
  }
  if (data_index == I2C_BUFFER_SIZE) {
    DEBUGOUT("Leader-Follower read-write Data comparison is successful, Test "
             "Case Passed \n");
  } else {
    DEBUGOUT("Leader-Follower read-write Data comparison is not successful, "
             "Test Case Failed \n");
  }
}
