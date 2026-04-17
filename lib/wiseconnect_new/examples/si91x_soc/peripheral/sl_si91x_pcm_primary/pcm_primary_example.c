/***************************************************************************/ /**
 * @file pcm_primary_example.c
 * @brief PCM example functions
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
#include "pcm_primary_example.h"
#include "rsi_debug.h"
#include "sl_si91x_clock_manager.h"
#include "sl_si91x_driver_gpio.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define PCM_PRIMARY_BUFFER_SIZE 1024                   // Transmit/Receive buffer size
#define PCM_INSTANCE            SL_SI91X_PCM0_INSTANCE // PCM instance
#define FRAME_SIZE_ALIGNMENT    4                      // Max frame offset
#define FRAME_OFFSET            2                      // Default frame offset

// PCM instance mapping
#if PCM_INSTANCE == PCM0
#define PCM_INSTANCE_CONFIG_(config) SL_PCM0##_##config
#else
#define PCM_INSTANCE_CONFIG_(config) SL_ULP_PCM##_##config
#endif

#define PRIMARY_SECONDARY_SYNC_PIN RTE_UULP_GPIO_2_PIN
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
typedef uint16_t pcm_data_size_t;
pcm_data_size_t pcm_primary_data_in[PCM_PRIMARY_BUFFER_SIZE + FRAME_SIZE_ALIGNMENT];
pcm_data_size_t pcm_primary_data_out[PCM_PRIMARY_BUFFER_SIZE];
static uint8_t pcm_primary_send_complete    = 0;
static uint8_t pcm_primary_receive_complete = 0;
typedef enum { SEND_DATA, RECEIVE_DATA, WAIT_STATE, INVALID_STATE } transfer_state_t;
static sl_i2s_handle_t pcm_handle = NULL;
static uint8_t pcm_resolution;
static int pcm_sampling_frequency;
static uint16_t mode;
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/

static void callback_event(uint32_t event);
static void compare_loop_back_data(void);
static void remove_pcm_frame_offset(pcm_data_size_t data_buffer[PCM_PRIMARY_BUFFER_SIZE + FRAME_OFFSET]);
static sl_status_t primary_sync_wait();
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * I2S example initialization function
 ******************************************************************************/
void pcm_primary_example_init(void)
{
  sl_status_t status;
  pcm_sampling_frequency = PCM_INSTANCE_CONFIG_(SAMPLING_RATE);
  pcm_resolution         = PCM_INSTANCE_CONFIG_(RESOLUTION);
  mode                   = PCM_INSTANCE_CONFIG_(MODE);

  // Filling the data out array with integer values
  for (uint16_t i = 0; i < PCM_PRIMARY_BUFFER_SIZE; i++) {
    pcm_primary_data_out[i] = i;
  }
  do {

    //Initialize I2S peripheral and store driver handle in i2s_driver_handle
    status = sl_si91x_pcm_init(PCM_INSTANCE, &pcm_handle);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("PCM Initialization fail\r\n");
      break;
    }
    DEBUGOUT("PCM Initialization success\r\n");
    //Register user callback handler
    status = sl_si91x_pcm_register_event_callback(pcm_handle, callback_event);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("PCM user callback register fail\r\n");
      break;
    } else {
      DEBUGOUT("PCM user callback register success\r\n");
    }

    status = sl_si91x_pcm_set_configuration(pcm_handle, pcm_sampling_frequency, pcm_resolution, mode);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("PCM configuration set fail\r\n");
      break;
    } else {
      DEBUGOUT("PCM configuration set success\r\n");
    }

  } while (false);

  // Synchronize primary device with secondary device using button press
  // This ensures both devices start PCM operations at the same time
  status = primary_sync_wait();
  if (status != SL_STATUS_OK) {
    DEBUGOUT("Primary device synchronization failed with error code: 0x%lx\r\n", status);
  } else {
    DEBUGOUT("Primary device synchronized successfully with secondary device\r\n");
  }
}
/*******************************************************************************
 * Function will run continuously in while loop
 ******************************************************************************/
void pcm_primary_example_process_action(void)
{
  static transfer_state_t state = RECEIVE_DATA;
  switch (state) {
    case SEND_DATA:
      do {
        //Configure PCM transmit DMA channel
        if (sl_si91x_pcm_transmit_data(pcm_handle,
                                       pcm_primary_data_out,
                                       (PCM_PRIMARY_BUFFER_SIZE + FRAME_SIZE_ALIGNMENT))) {
          DEBUGOUT("PCM transmit start fail\r\n");
          state = INVALID_STATE;
          break;
        } else {
          DEBUGOUT("PCM transmit start success\r\n");
        }
      } while (false);
      state = WAIT_STATE;
      break;

    case RECEIVE_DATA:
      do {
        //Configure PCM receive DMA channel
        if (sl_si91x_pcm_receive_data(pcm_handle,
                                      pcm_primary_data_in,
                                      (PCM_PRIMARY_BUFFER_SIZE + FRAME_SIZE_ALIGNMENT))) {
          DEBUGOUT("PCM receive start fail\r\n");
          state = INVALID_STATE;
          break;
        } else {
          DEBUGOUT("PCM receive start success\r\n");
        }
      } while (false);
      state = WAIT_STATE;
      break;

    case WAIT_STATE:
      if (pcm_primary_send_complete) {
        DEBUGOUT("Data send successfully\r\n");
        pcm_primary_send_complete = 0;
      }
      if (pcm_primary_receive_complete) {
        DEBUGOUT("Data received successfully\r\n");
        compare_loop_back_data();
        pcm_primary_receive_complete = 0;
        state                        = SEND_DATA;
      }
      break;

    default:
      break;
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
  remove_pcm_frame_offset(pcm_primary_data_in);

  for (data_index = 0; data_index < PCM_PRIMARY_BUFFER_SIZE; data_index++) {
    // If the data in and data out are same, it will be continued else, the for
    // loop will be break.
    if (pcm_primary_data_in[data_index] != pcm_primary_data_out[data_index]) {
      break;
    }
  }
  if (data_index == PCM_PRIMARY_BUFFER_SIZE) {
    DEBUGOUT("Data comparison successful\n");
  } else {
    DEBUGOUT("Data comparison failed\n");
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
      pcm_primary_send_complete = 1;
      break;
    case SL_I2S_RECEIVE_COMPLETE:
      pcm_primary_receive_complete = 1;
      break;
    case SL_I2S_TX_UNDERFLOW:
      break;
    case SL_I2S_RX_OVERFLOW:
      break;
    case SL_I2S_FRAME_ERROR:
      break;
  }
}

/*******************************************************************************
 * Function to remove the frame-sync offset
 *
 * @param data_buffer - input receive buffer to remove data offset.
 * @return none
 ******************************************************************************/
static void remove_pcm_frame_offset(pcm_data_size_t data_buffer[PCM_PRIMARY_BUFFER_SIZE + FRAME_OFFSET])
{
  for (int i = 0; i < PCM_PRIMARY_BUFFER_SIZE; i++) {
    data_buffer[i] = data_buffer[i + FRAME_OFFSET];
  }
}

/*******************************************************************************
 * Synchronizes primary and secondary devices using UULP NPSS GPIO 2 pin (button 0).
 * 
 * This function configures GPIO pin as input to detect button press for synchronization
 * between primary and secondary PCM devices. It waits for button 0 to be pressed on
 * the primary device before proceeding with PCM operations.
 *
 * @note Ensure secondary device is reset before pressing button 0 on primary device.
 * 
 * @return sl_status_t Status code indicating success or failure:
 *         - SL_STATUS_OK: Synchronization completed successfully
 *         - Error code: GPIO configuration or operation failed
 ******************************************************************************/
static sl_status_t primary_sync_wait()
{
  uint8_t pin_value  = 0;
  sl_status_t status = SL_STATUS_OK;

  // Enable GPIO ULP clock for UULP NPSS GPIO operation
  status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)ULPCLK_GPIO);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Enable receiver for UULP NPSS GPIO pin to read input state
  status = sl_si91x_gpio_driver_select_uulp_npss_receiver(PRIMARY_SECONDARY_SYNC_PIN, GPIO_RECEIVER_EN);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Configure pin mux mode for UULP NPSS GPIO functionality
  status = sl_si91x_gpio_driver_set_uulp_npss_pin_mux(PRIMARY_SECONDARY_SYNC_PIN, NPSS_GPIO_PIN_MUX_MODE0);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Set GPIO pin direction as input to detect button press
  status =
    sl_si91x_gpio_driver_set_uulp_npss_direction(PRIMARY_SECONDARY_SYNC_PIN, (sl_si91x_gpio_direction_t)GPIO_INPUT);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Wait for button 0 press to synchronize primary and secondary devices
  DEBUGOUT("Reset Secondary and Press button 0 on primary to sync.\r\n");
  while (1) {
    // Read the GPIO pin state
    pin_value = sl_si91x_gpio_driver_get_uulp_npss_pin(PRIMARY_SECONDARY_SYNC_PIN);

    // Check if button is pressed (active low)
    if (pin_value == 0) {
      DEBUGOUT("Button press detected, synchronization completed\r\n");
      break;
    }

    // Small delay to avoid excessive GPIO polling
    sl_si91x_delay_ms(10);
  }

  // Additional delay to allow secondary device to start sending data first
  sl_si91x_delay_ms(50);
  return status;
}
