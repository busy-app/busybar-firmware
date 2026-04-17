/***************************************************************************/ /**
 * @file i2s_example.c
 * @brief I2S examples functions
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
#include "sl_si91x_i2s.h"
#include "sl_si91x_i2s_config.h"
#include "i2s_loopback_example.h"
#include "rsi_debug.h"
#include "rsi_rom_table_si91x.h"
#include "rsi_rom_clks.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define I2S_BUFFER_SIZE 1024 // Transmit/Receive buffer size
#define I2S_INSTANCE    0    // I2S instance
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
uint8_t i2s_data_in[I2S_BUFFER_SIZE];
uint8_t i2s_data_out[I2S_BUFFER_SIZE];
static sl_i2s_handle_t i2s_driver_handle    = NULL;
static uint8_t i2s_send_complete            = 0;
static uint8_t i2s_receive_complete         = 0;
static sl_i2s_xfer_config_t i2s_xfer_config = { 0 };

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/

static void callback_event(uint32_t event);
static void compare_loop_back_data(void);
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * I2S example initialization function
 ******************************************************************************/
void i2s_example_init(void)
{
  sl_status_t status;
  sl_i2s_version_t i2s_version;
  sl_i2s_status_t i2s_status; //Initialize I2S transfer structure
  i2s_xfer_config.mode          = SL_I2S_MASTER;
  i2s_xfer_config.protocol      = SL_I2S_PROTOCOL;
  i2s_xfer_config.resolution    = SL_I2S0_RESOLUTION;
  i2s_xfer_config.sampling_rate = SL_I2S0_SAMPLING_RATE;
  i2s_xfer_config.sync          = SL_I2S_ASYNC;
  i2s_xfer_config.data_size     = SL_I2S_DATA_SIZE8;

  // Filling the data out array with integer values
  for (uint32_t i = 0; i < I2S_BUFFER_SIZE; i++) {
    i2s_data_out[i] = (uint16_t)i;
  }
  do {
    //Fetch I2S driver version
    i2s_version = sl_si91x_i2s_get_version();
    DEBUGOUT("I2S version is fetched successfully \n");
    DEBUGOUT("API version is %d.%d.%d\n", i2s_version.release, i2s_version.major, i2s_version.minor);
    //Initialize I2S peripheral and store driver handle in i2s_driver_handle
    status = sl_si91x_i2s_init(I2S_INSTANCE, &i2s_driver_handle);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("I2S Initialization fail\r\n");
      break;
    }
    DEBUGOUT("I2S Initialization success\r\n");
    //Get the status of I2S peripheral
    i2s_status = sl_si91x_i2s_get_status(i2s_driver_handle);
    DEBUGOUT("I2S status is fetched successfully \n");
    DEBUGOUT("Frame error: %d\n", i2s_status.frame_error);
    DEBUGOUT("Rx status: %d\n", i2s_status.rx_busy);
    DEBUGOUT("Rx overflow status: %d\n", i2s_status.rx_overflow);
    DEBUGOUT("Tx status: %d\n", i2s_status.tx_busy);
    DEBUGOUT("Tx underflow status: %d\n", i2s_status.tx_underflow);
    //Configure ARM full power mode
    status = sl_si91x_i2s_configure_power_mode(i2s_driver_handle, SL_I2S_FULL_POWER);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("I2S power mode config fail\r\n");
      break;
    }
    DEBUGOUT("I2S power mode config success\r\n");
    //Register user callback handler
    status = sl_si91x_i2s_register_event_callback(i2s_driver_handle, callback_event);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("I2S user callback register fail\r\n");
      break;
    }
    DEBUGOUT("I2S user callback register success\r\n");
    i2s_xfer_config.transfer_type = SL_I2S_TRANSMIT;
    //Configure transmitter parameters for i2s transfer
    if (sl_si91x_i2s_config_transmit_receive(i2s_driver_handle, &i2s_xfer_config)) {
      DEBUGOUT("I2S transmit config fail\r\n");
      break;
    }
    DEBUGOUT("I2S transmit config success\r\n");
    i2s_xfer_config.transfer_type = SL_I2S_RECEIVE;
    //Configure receiver parameters for i2s transfer
    if (sl_si91x_i2s_config_transmit_receive(i2s_driver_handle, &i2s_xfer_config)) {
      DEBUGOUT("I2S receive config fail\r\n");
      break;
    }
    DEBUGOUT("I2S receive config success\r\n");
    //Configure I2S receive DMA channel
    //Since 8-bit resolution is not supported in Si91x I2S module, configure receive data as 16-bit
    //chunks which contains two bytes of 8-bit data.
    if (sl_si91x_i2s_receive_data(i2s_driver_handle, (uint16_t *)i2s_data_in, I2S_BUFFER_SIZE / 2)) {
      DEBUGOUT("I2S receive start fail\r\n");
      break;
    }
    DEBUGOUT("I2S receive start success\r\n");
    //Configure I2S transmit DMA channel
    //Since 8-bit resolution is not supported in Si91x I2S module, configure transmit data as 16-bit
    //chunks which contains two bytes of 8-bit data.
    if (sl_si91x_i2s_transmit_data(i2s_driver_handle, (uint16_t *)i2s_data_out, I2S_BUFFER_SIZE / 2)) {
      DEBUGOUT("I2S transmit start fail\r\n");
      break;
    }
    DEBUGOUT("I2S transmit start success\r\n");
  } while (false);
}
/*******************************************************************************
 * Function will run continuously in while loop
 ******************************************************************************/
void i2s_example_process_action(void)
{
  if ((i2s_send_complete && i2s_receive_complete)) {
    //Data has been transferred and received successfully
    //Validate the transmit and receive data count
    if ((sl_si91x_i2s_get_transmit_data_count(i2s_driver_handle) == I2S_BUFFER_SIZE / 2)
        && (sl_si91x_i2s_get_receive_data_count(i2s_driver_handle) == I2S_BUFFER_SIZE / 2)) {
      //I2S transfer completed
      DEBUGOUT("I2S transfer complete\r\n");
      //Compare transmit data and receive data
      compare_loop_back_data();
      // Aborting the I2S send operation
      sl_si91x_i2s_end_transfer(i2s_driver_handle, SL_I2S_SEND_ABORT);
      // Aborting the I2S send operation
      sl_si91x_i2s_end_transfer(i2s_driver_handle, SL_I2S_RECEIVE_ABORT);
    }
    //reset send and receive complete status flags
    i2s_send_complete    = 0;
    i2s_receive_complete = 0;
  }
}

/*******************************************************************************
 * Function to compare the loop back data, i.e., after transfer it will compare
 * the send and receive data
 *
 * @param none
 * @return none
 ******************************************************************************/
static void compare_loop_back_data(void)
{
  uint16_t data_index = 0;
  for (data_index = 0; data_index < I2S_BUFFER_SIZE; data_index++) {
    // If the data in and data out are same, it will be continued else, the for
    // loop will be break.
    if (i2s_data_in[data_index] != i2s_data_out[data_index]) {
      break;
    }
  }
  if (data_index == I2S_BUFFER_SIZE) {
    DEBUGOUT("Data comparison successful, Loop Back Test Passed \n");
  } else {
    DEBUGOUT("Data comparison failed, Loop Back Test failed \n");
  }
}

/*******************************************************************************
 * Callback event function
 * It is responsible for the event which are triggered by GSPI interface
 * It updates the respective member of the structure as the event is triggered.
 * @param event - interrupt trigger event
 * @return none
 ******************************************************************************/
static void callback_event(uint32_t event)
{
  switch (event) {
    case SL_I2S_SEND_COMPLETE:
      i2s_send_complete = 1;
      break;
    case SL_I2S_RECEIVE_COMPLETE:
      i2s_receive_complete = 1;
      break;
    case SL_I2S_TX_UNDERFLOW:
      break;
    case SL_I2S_RX_OVERFLOW:
      break;
    case SL_I2S_FRAME_ERROR:
      break;
    default:
      break;
  }
}