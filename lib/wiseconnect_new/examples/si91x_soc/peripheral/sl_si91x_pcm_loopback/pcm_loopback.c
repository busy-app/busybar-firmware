/***************************************************************************/ /**
 * @file pcm_example.c
 * @brief PCM examples functions
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
#include "sl_si91x_pcm.h"
#include "sl_si91x_pcm_config.h"
#include "pcm_loopback.h"
#include "rsi_debug.h"
#include "rsi_rom_table_si91x.h"
#include "rsi_rom_clks.h"
#include <math.h>

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define PCM_BUFFER_SIZE      1024                   // Transmit/Receive buffer size
#define PCM_INSTANCE         SL_SI91X_PCM0_INSTANCE // PCM instance
#define FRAME_SIZE_ALIGNMENT 4                      // Max frame offset
#define FRAME_OFFSET         2                      // Default frame offset

// PCM instance mapping
#if PCM_INSTANCE == PCM0
#define PCM_INSTANCE_CONFIG_(config) SL_PCM0##_##config
#else
#define PCM_INSTANCE_CONFIG_(config) SL_ULP_PCM##_##config
#endif
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static uint8_t pcm_send_complete    = 0;
static uint8_t pcm_receive_complete = 0;
typedef uint16_t pcm_data_size_t;
pcm_data_size_t pcm_data_in[PCM_BUFFER_SIZE + FRAME_SIZE_ALIGNMENT];
pcm_data_size_t pcm_data_out[PCM_BUFFER_SIZE];
static uint8_t pcm_resolution;
static int pcm_sampling_frequency;
static int mode;
static sl_i2s_handle_t pcm_handle = NULL;
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void callback_event(uint32_t event);
static void compare_loop_back_data(void);
static void remove_pcm_frame_offset(pcm_data_size_t data_buffer[PCM_BUFFER_SIZE + FRAME_OFFSET]);
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * PCM example initialization function
 ******************************************************************************/
void pcm_example_init(void)
{
  sl_status_t status;
  sl_i2s_status_t pcm_status; //Initialize I2S transfer structure
  pcm_sampling_frequency = PCM_INSTANCE_CONFIG_(SAMPLING_RATE);
  pcm_resolution         = PCM_INSTANCE_CONFIG_(RESOLUTION);
  mode                   = PCM_INSTANCE_CONFIG_(MODE);

  // Filling the data out array with integer values
  for (uint16_t i = 0; i < PCM_BUFFER_SIZE; i++) {
    pcm_data_out[i] = (uint16_t)i;
  }
  do {

    //Initialize PCM peripheral and store driver handle in pcm_driver_handle
    status = sl_si91x_pcm_init(PCM_INSTANCE, &pcm_handle);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("PCM Initialization fail\r\n");
      break;
    }
    DEBUGOUT("PCM Initialization success\r\n");
    //Get the status of PCM peripheral
    pcm_status = sl_si91x_pcm_get_status(pcm_handle);
    DEBUGOUT("PCM status is fetched successfully \n");
    DEBUGOUT("Frame error: %d\n", pcm_status.frame_error);
    DEBUGOUT("Rx status: %d\n", pcm_status.rx_busy);
    DEBUGOUT("Rx overflow status: %d\n", pcm_status.rx_overflow);
    DEBUGOUT("Tx status: %d\n", pcm_status.tx_busy);
    DEBUGOUT("Tx underflow status: %d\n", pcm_status.tx_underflow);

    //Set the required PCM configuration
    status = sl_si91x_pcm_set_configuration(pcm_handle, pcm_sampling_frequency, pcm_resolution, mode);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("PCM configuration set fail\r\n");
      break;
    }
    DEBUGOUT("PCM configuration set success\r\n");
    //Register user callback handler
    status = sl_si91x_pcm_register_event_callback(pcm_handle, callback_event);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("PCM user callback register fail\r\n");
      break;
    }
    DEBUGOUT("PCM user callback register success\r\n");
    //Configure PCM receive DMA channel
    if (sl_si91x_pcm_receive_data(pcm_handle, (uint16_t *)pcm_data_in, (PCM_BUFFER_SIZE + FRAME_SIZE_ALIGNMENT))) {
      DEBUGOUT("PCM receive start fail\r\n");
      break;
    }
    DEBUGOUT("PCM receive start success\r\n");
    //Configure PCM transmit DMA channel
    if (sl_si91x_pcm_transmit_data(pcm_handle, (uint16_t *)pcm_data_out, PCM_BUFFER_SIZE)) {
      DEBUGOUT("PCM transmit start fail\r\n");
      break;
    }
    DEBUGOUT("PCM transmit start success\r\n");
  } while (false);
}
/*******************************************************************************
 * Function will run continuously in while loop
 ******************************************************************************/
void pcm_example_process_action(void)
{
  if ((pcm_send_complete && pcm_receive_complete)) {
    //Data has been transferred and received successfully
    //Validate the transmit and receive data count
    if ((sl_si91x_pcm_get_transmit_data_count(pcm_handle) == PCM_BUFFER_SIZE)
        && (sl_si91x_pcm_get_receive_data_count(pcm_handle) == (PCM_BUFFER_SIZE + FRAME_SIZE_ALIGNMENT))) {
      //PCM transfer completed
      DEBUGOUT("PCM transfer complete\r\n");
      //Compare transmit data and receive data
      compare_loop_back_data();
      // Aborting the PCM send operation
      sl_si91x_pcm_end_transfer(pcm_handle, SL_I2S_SEND_ABORT);
      // Aborting the PCM send operation
      sl_si91x_pcm_end_transfer(pcm_handle, SL_I2S_RECEIVE_ABORT);
    }
    //reset send and receive complete status flags
    pcm_send_complete    = 0;
    pcm_receive_complete = 0;
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

  //Remove the frame-sync offset
  remove_pcm_frame_offset(pcm_data_in);

  for (data_index = 0; data_index < PCM_BUFFER_SIZE; data_index++) {
    // If the data in and data out are same, it will be continued else, the for
    // loop will be break.
    if (pcm_data_in[data_index] != pcm_data_out[data_index]) {
      break;
    }
  }

  if (data_index == PCM_BUFFER_SIZE) {
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
      pcm_send_complete = 1;
      break;
    case SL_I2S_RECEIVE_COMPLETE:
      pcm_receive_complete = 1;
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

/*******************************************************************************
 * Function to remove the frame-sync offset
 *
 * @param data_buffer - input receive buffer to remove data offset.
 * @return none
 ******************************************************************************/
static void remove_pcm_frame_offset(pcm_data_size_t data_buffer[PCM_BUFFER_SIZE + FRAME_OFFSET])
{
  for (int i = 0; i < PCM_BUFFER_SIZE; i++) {
    data_buffer[i] = data_buffer[i + FRAME_OFFSET];
  }
}
