/***************************************************************************/ /**
* @file  ssi_slave_example.c
* @brief SSI Slave example.
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
// Include Files

#include "sl_si91x_ssi.h"
#include "rsi_debug.h"
#include "rsi_rom_clks.h"
#include "ssi_slave_example.h"
#include "sl_ulp_timer_instances.h"
#include "sl_si91x_ulp_timer_common_config.h"
#include "sl_si91x_clock_manager.h"
#include "rsi_rom_clks.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define SSI_SLAVE_BUFFER_SIZE      1024     // Length of data to be sent through SPI
#define SSI_SLAVE_INTF_PLL_REF_CLK 40000000 // PLL Ref Clock frequency
#define SSI_SLAVE_BIT_WIDTH        8        // SSI bit width
#define SSI_SLAVE_BAUDRATE         10000000 // SSI baudrate
#define SSI_SLAVE_MAX_BIT_WIDTH    16       // Maximum Bit width
#define TIMER_FREQUENCY            32000    // Timer frequency for delay
#define INITIAL_COUNT              7000     // Count configured at timer init
#define SYNC_TIME                  5000     // Delay to sync master and slave
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void ssi_slave_callback_event_handler(uint32_t event);
static void ssi_slave_compare_loopback_data(void);
static void init_timer_for_sync(void);
static void wait_for_sync(uint16_t time_ms);

/*******************************************************************************
 **********************  Local variables   *************************************
 ******************************************************************************/
static uint8_t ssi_slave_tx_buffer[SSI_SLAVE_BUFFER_SIZE] = { '\0' };
static uint8_t ssi_slave_rx_buffer[SSI_SLAVE_BUFFER_SIZE] = { '\0' };
static sl_ssi_handle_t ssi_driver_handle                  = NULL;
static boolean_t ssi_slave_transfer_complete              = false;
static uint16_t size_factor                               = 1;
static uint32_t ssi_slave_number                          = SSI_SLAVE_0;

/// @brief Enumeration for different transmission scenarios
typedef enum {
  SSI_SLAVE_TRANSFER_DATA,
  SSI_SLAVE_RECEIVE_DATA,
  SSI_SLAVE_SEND_DATA,
  SSI_SLAVE_TRANSMISSION_COMPLETED,
} ssi_mode_enum_t;
static ssi_mode_enum_t ssi_slave_current_mode = SSI_SLAVE_TRANSFER_DATA;

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * @fn         ssi_slave_example_init()
 * @brief      Main Application Function
 * @return     None
 ******************************************************************************/
void ssi_slave_example_init(void)
{
  uint16_t i            = 0;
  sl_status_t sl_status = 0;
  sl_ssi_version_t ssi_version;
  // Configuring the user configuration structure
  sl_ssi_control_config_t ssi_slave_config;
  ssi_slave_config.bit_width   = SSI_SLAVE_BIT_WIDTH;
  ssi_slave_config.device_mode = SL_SSI_SLAVE_ACTIVE;
  ssi_slave_config.clock_mode  = SL_SSI_PERIPHERAL_CPOL0_CPHA0;
  ssi_slave_config.baud_rate   = SSI_SLAVE_BAUDRATE;

  // Filled data into input buffer
  for (i = 0; i < SSI_SLAVE_BUFFER_SIZE; i++) {
    ssi_slave_tx_buffer[i] = (uint8_t)(i + 1);
  }
  do {
    // Set the clock for the SSI interface based on configuration
    // ssi_clk <= 4 × (maximum sclk_in frequency)
    // SSI secondary peripheral clock (ssi_clk)
    // sclk_in is the incoming clock from the master
    DEBUGOUT("Configuring clock for SSI interface...\n");
    DEBUGOUT("Setting SSI interface clock: ssi_clk <= 4 * (maximum sclk_in frequency)\n");
    sl_status = sl_si91x_clock_manager_set_pll_freq(INTF_PLL, SSI_SLAVE_BAUDRATE * 4, SSI_SLAVE_INTF_PLL_REF_CLK);
    if (sl_status != SL_STATUS_OK) {
      DEBUGOUT("Failed to set SSI interface clock, Error Code : %lu \n", sl_status);
      break;
    }
    DEBUGOUT("SSI interface clock set successfully\n");
    // Initialzing the timer
    init_timer_for_sync();
    // Version information of SSI driver
    ssi_version = sl_si91x_ssi_get_version();
    DEBUGOUT("SSI version is fetched successfully \n");
    DEBUGOUT("API version is %d.%d.%d\n", ssi_version.release, ssi_version.major, ssi_version.minor);
    // Initialize the SSI driver
    sl_status = sl_si91x_ssi_init(ssi_slave_config.device_mode, &ssi_driver_handle);
    if (sl_status != SL_STATUS_OK) {
      DEBUGOUT("SSI Initialization Failed, Error Code : %lu \n", sl_status);
      break;
    }
    DEBUGOUT("SSI Initialization Success \n");
    // Configure the SSI to Master, 16-bit mode @10000 kBits/sec
    sl_status = sl_si91x_ssi_set_configuration(ssi_driver_handle, &ssi_slave_config, ssi_slave_number);
    if (sl_status != SL_STATUS_OK) {
      DEBUGOUT("Failed to Set Configuration Parameters to SSI, Error Code : %lu \n", sl_status);
      break;
    }
    DEBUGOUT("Set Configuration Parameters to SSI\r\n");
    // Register the user callback
    sl_status = sl_si91x_ssi_register_event_callback(ssi_driver_handle, ssi_slave_callback_event_handler);
    if (sl_status != SL_STATUS_OK) {
      DEBUGOUT("SSI register event callback Failed, Error Code : %lu \n", sl_status);
      break;
    }
    DEBUGOUT("SSI register event callback Success \n");
    // Fetching and printing the current clock division factor
    DEBUGOUT("Current Clock division factor is %lu \n", sl_si91x_ssi_get_clock_division_factor(ssi_driver_handle));
    // Fetching and printing the current frame length
    DEBUGOUT("Current Frame Length is %lu \n", sl_si91x_ssi_get_frame_length(ssi_driver_handle));
    if (sl_si91x_ssi_get_frame_length(ssi_driver_handle) >= SSI_SLAVE_BIT_WIDTH) {
      size_factor = sizeof(ssi_slave_tx_buffer[0]);
    }
    // Syncing master and slave
    wait_for_sync(SYNC_TIME);
    // As per the macros enabled in the header file, it will configure the current mode.
    if (SSI_SLAVE_TRANSFER) {
      ssi_slave_current_mode = SSI_SLAVE_TRANSFER_DATA;
      break;
    }
    if (SSI_SLAVE_RECEIVE) {
      ssi_slave_current_mode = SSI_SLAVE_RECEIVE_DATA;
      break;
    }
    ssi_slave_current_mode = SSI_SLAVE_SEND_DATA;
  } while (false);
}
/*******************************************************************************
 * Implementation of state machine for Transfer data, receive data and
 * send data.
 ******************************************************************************/
void ssi_slave_example_process_action(void)
{
  static sl_status_t status;
  static boolean_t ssi_slave_begin_transmission = true;
  // In this switch case, according to the macros enabled in header file, it starts to execute the APIs
  // Assuming all the macros are enabled, after transfer, receive will be executed and after receive
  // send will be executed.
  switch (ssi_slave_current_mode) {
    case SSI_SLAVE_TRANSFER_DATA:
      if (ssi_slave_begin_transmission == true) {
        // Validation for executing the API only once
        status = sl_si91x_ssi_transfer_data(ssi_driver_handle,
                                            ssi_slave_tx_buffer,
                                            ssi_slave_rx_buffer,
                                            sizeof(ssi_slave_tx_buffer) / size_factor);
        if (status != SL_STATUS_OK) {
          // If it fails to execute the API, it will not execute rest of the things
          DEBUGOUT("sl_si91x_ssi_transfer_data: Error Code : %lu \n", status);
          ssi_slave_current_mode = SSI_SLAVE_TRANSMISSION_COMPLETED;
          break;
        }
        DEBUGOUT("SSI transfer begin successfully \n");
        ssi_slave_begin_transmission = false;
      }
      if (ssi_slave_transfer_complete) {
        ssi_slave_transfer_complete = false;
        DEBUGOUT("SSI transfer completed successfully \n");
        // After comparing the loopback transfer, it compares the data_out and
        // data_in.
        ssi_slave_compare_loopback_data();
        if (SSI_SLAVE_RECEIVE) {
          // If receive macro is enabled, current mode is set to receive
          ssi_slave_current_mode       = SSI_SLAVE_RECEIVE_DATA;
          ssi_slave_begin_transmission = true;
          break;
        }
        if (SSI_SLAVE_SEND) {
          // If send macro is enabled, current mode is set to send
          ssi_slave_current_mode       = SSI_SLAVE_SEND_DATA;
          ssi_slave_begin_transmission = true;
          break;
        }
        // If above macros are not enabled, current mode is set to complete
        ssi_slave_current_mode = SSI_SLAVE_TRANSMISSION_COMPLETED;
      }
      break;
    case SSI_SLAVE_RECEIVE_DATA:
      if (ssi_slave_begin_transmission == true) {
        // Validation for executing the API only once
        status =
          sl_si91x_ssi_receive_data(ssi_driver_handle, ssi_slave_rx_buffer, sizeof(ssi_slave_rx_buffer) / size_factor);
        if (status != SL_STATUS_OK) {
          // If it fails to execute the API, it will not execute rest of the things
          DEBUGOUT("sl_si91x_ssi_receive_data: Error Code : %lu \n", status);
          ssi_slave_current_mode = SSI_SLAVE_TRANSMISSION_COMPLETED;
          break;
        }
        DEBUGOUT("SSI receive begin successfully \n");
        ssi_slave_begin_transmission = false;
        //Waiting till the receive is completed
      }
      if (ssi_slave_transfer_complete) {
        // If DMA is enabled, it will wait until transfer_complete flag is set.
        ssi_slave_transfer_complete = false;
        if (SSI_SLAVE_SEND) {
          // If send macro is enabled, current mode is set to send
          ssi_slave_current_mode       = SSI_SLAVE_SEND_DATA;
          ssi_slave_begin_transmission = true;
          DEBUGOUT("SSI receive completed \n");
          break;
        }
        DEBUGOUT("SSI receive completed \n");
        // If send macro is not enabled, current mode is set to completed.
        ssi_slave_current_mode = SSI_SLAVE_TRANSMISSION_COMPLETED;
      }
      break;
    case SSI_SLAVE_SEND_DATA:
      if (ssi_slave_begin_transmission) {
        // Validation for executing the API only once
        status =
          sl_si91x_ssi_send_data(ssi_driver_handle, ssi_slave_tx_buffer, sizeof(ssi_slave_tx_buffer) / size_factor);
        if (status != SL_STATUS_OK) {
          // If it fails to execute the API, it will not execute rest of the things
          DEBUGOUT("sl_si91x_ssi_send_data: Error Code : %lu \n", status);
          ssi_slave_current_mode = SSI_SLAVE_TRANSMISSION_COMPLETED;
          break;
        }
        DEBUGOUT("SSI send begin successfully \n");
        ssi_slave_begin_transmission = false;
      }
      //Waiting till the send is completed
      if (ssi_slave_transfer_complete) {
        // If DMA is enabled, it will wait until transfer_complete flag is set.
        ssi_slave_transfer_complete = false;
        DEBUGOUT("SSI send completed \n");
        ssi_slave_compare_loopback_data();
        // At last current mode is set to completed.
        ssi_slave_current_mode = SSI_SLAVE_TRANSMISSION_COMPLETED;
      }
      break;
    case SSI_SLAVE_TRANSMISSION_COMPLETED:
      break;
  }
}

/*******************************************************************************
 * @fn         compare_loopback_data()
 * @brief      Internal private Function which validates input and output buffers
 *             and decides whether the test case passed/failed.
 * @return     None
*******************************************************************************/
static void ssi_slave_compare_loopback_data(void)
{
  // If the data width is not standard (8-bit) then the data should be masked.
  // The extra bits of the integer should be always zero.
  // For example, if bit width is 7, then from 8-15 all bits should be zero in a 16 bit integer.
  // So mask has value according to the data width and it is applied to the data.
  int ssi_data_index;
  uint32_t ssi_frame_length = 0;
  uint16_t ssi_mask         = (uint16_t)~0;
  ssi_frame_length          = sl_si91x_ssi_get_frame_length(ssi_driver_handle);
  ssi_mask                  = ssi_mask >> (SSI_SLAVE_MAX_BIT_WIDTH - ssi_frame_length);
  for (ssi_data_index = 0; ssi_data_index < SSI_SLAVE_BUFFER_SIZE; ssi_data_index++) {
    ssi_slave_rx_buffer[ssi_data_index] &= (uint8_t)ssi_mask;
    ssi_slave_tx_buffer[ssi_data_index] &= (uint8_t)ssi_mask;
    if (ssi_slave_tx_buffer[ssi_data_index] != ssi_slave_rx_buffer[ssi_data_index]) {
      break;
    }
  }
  if (ssi_data_index == SSI_SLAVE_BUFFER_SIZE) {
    DEBUGOUT("Data comparison successful, Test case Passed \n");
  } else {
    DEBUGOUT("Data comparison failed, Test case failed \n");
  }
}

/*******************************************************************************
 * @brief  SSI callback handler
 * @param[in]event SSI transmit and receive events
 * @return   None
*******************************************************************************/
static void ssi_slave_callback_event_handler(uint32_t event)
{
  switch (event) {
    case SSI_EVENT_TRANSFER_COMPLETE:
      ssi_slave_transfer_complete = true;
      break;

    case SSI_EVENT_DATA_LOST:
      // Occurs in slave mode when data is requested/sent by master
      // but send/receive/transfer operation has not been started
      // and indicates that data is lost. Occurs also in master mode
      // when driver cannot transfer data fast enough.
      break;

    case SSI_EVENT_MODE_FAULT:
      // Occurs in master mode when Slave Select is deactivated and
      // indicates Master Mode Fault.
      break;
  }
}

/*******************************************************************************
 * @brief  Initialization of timer for sync
 * @param[in] None
 * @return   None
*******************************************************************************/
static void init_timer_for_sync(void)
{
  sl_status_t status;
  status = sl_si91x_ulp_timer_configure_clock(&sl_timer_clk_handle);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("sl_si91x_ulp_timer_configure_clock failed, error code: %ld", status);
  }
  status = sl_si91x_ulp_timer_set_configuration(&sl_timer_handle_timer0);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("sl_si91x_ulp_timer_set_configuration failed, error code: %ld", status);
  }
  status = sl_si91x_ulp_timer_set_count(TIMER_0, TIMER_FREQUENCY * INITIAL_COUNT);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("sl_si91x_ulp_timer_set_count failed, error code: %ld", status);
  }
}

/*******************************************************************************
 * @brief  Waits till the master and slave application is synced by creating delay
 * @param[in] time_ms Sync time in milliseconds
 * @return   None
*******************************************************************************/
static void wait_for_sync(uint16_t time_ms)
{
  sl_status_t status;
  uint32_t start_time, current_time;
  uint32_t end_time = time_ms * TIMER_FREQUENCY;

  status = sl_si91x_ulp_timer_start(TIMER_0);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("sl_si91x_ulp_timer_start failed, error code: %ld", status);
  }
  status = sl_si91x_ulp_timer_get_count(TIMER_0, &start_time);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("sl_si91x_ulp_timer_get_count failed, error code: %ld", status);
  }
  do {
    status = sl_si91x_ulp_timer_get_count(TIMER_0, &current_time);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_ulp_timer_get_count failed, error code: %ld", status);
    }
  } while (!((current_time - start_time) > end_time));
  sl_si91x_ulp_timer_stop(TIMER_0);
}