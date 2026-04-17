/***************************************************************************/ /**
 * @file gspi_example.c
 * @brief GSPI examples functions
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
#include "sl_si91x_gspi.h"
#include "sl_si91x_gspi_common_config.h"
#include "gspi_example.h"
#include "rsi_debug.h"
#include "sl_ulp_timer_instances.h"
#include "sl_si91x_ulp_timer_common_config.h"

#include "rsi_rom_clks.h"
/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define GSPI_BUFFER_SIZE             1024      // Size of buffer
#define GSPI_INTF_PLL_CLK            180000000 // Intf pll clock frequency
#define GSPI_INTF_PLL_REF_CLK        40000000  // Intf pll reference clock frequency
#define GSPI_SOC_PLL_CLK             20000000  // Soc pll clock frequency
#define GSPI_SOC_PLL_REF_CLK         40000000  // Soc pll reference clock frequency
#define GSPI_INTF_PLL_500_CTRL_VALUE 0xD900    // Intf pll control value
#define GSPI_SOC_PLL_MM_COUNT_LIMIT  0xA4      // Soc pll count limit
#define GSPI_DVISION_FACTOR          0         // Division factor
#define GSPI_SWAP_READ_DATA          1         // true to enable and false to disable swap read
#define GSPI_SWAP_WRITE_DATA         0         // true to enable and false to disable swap write
#define GSPI_BITRATE                 10000000  // Bitrate for setting the clock division factor
#define GSPI_BIT_WIDTH               8         // Default Bit width
#define GSPI_MAX_BIT_WIDTH           16        // Maximum Bit width
#define TIMER_FREQUENCY              32000     // Timer frequency for delay
#define INITIAL_COUNT                7000      // Count configured at timer init
#define SYNC_TIME                    5000      // Delay to sync master and slave
#define RECEIVE_SYNC_TIME            500       // Delay to settle the slave after send
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static uint8_t gspi_data_in[GSPI_BUFFER_SIZE];
static uint8_t gspi_data_out[GSPI_BUFFER_SIZE];
static uint16_t gspi_division_factor       = 1;
static sl_gspi_handle_t gspi_driver_handle = NULL;

//Enum for different transmission scenarios
typedef enum {
  SL_GSPI_TRANSFER_DATA,
  SL_GSPI_RECEIVE_DATA,
  SL_GSPI_SEND_DATA,
  SL_GSPI_TRANSMISSION_COMPLETED,
} gspi_mode_enum_t;
static gspi_mode_enum_t current_mode = SL_GSPI_TRANSFER_DATA;

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void compare_loop_back_data(void);
static void callback_event(uint32_t event);
static boolean_t transfer_complete  = false;
static boolean_t begin_transmission = true;
static void init_timer_for_sync(void);
static void wait_for_sync(uint16_t time_ms);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * GSPI example initialization function
 ******************************************************************************/
void gspi_example_init(void)
{
  sl_status_t status;
  sl_gspi_version_t version;
  sl_gspi_status_t gspi_status;
  sl_gspi_control_config_t config;
  config.bit_width         = GSPI_BIT_WIDTH;
  config.bitrate           = GSPI_BITRATE;
  config.clock_mode        = SL_GSPI_MODE_0;
  config.slave_select_mode = SL_GSPI_MASTER_HW_OUTPUT;
  config.swap_read         = GSPI_SWAP_READ_DATA;
  config.swap_write        = GSPI_SWAP_WRITE_DATA;

  // Filling the data out array with integer values
  for (uint16_t i = 0; i < GSPI_BUFFER_SIZE; i++) {
    gspi_data_out[i] = (uint8_t)(i + 1);
  }
  do {
    // Initialzing the timer
    init_timer_for_sync();
    // Version information of GSPI driver
    version = sl_si91x_gspi_get_version();
    DEBUGOUT("GSPI version is fetched successfully \n");
    DEBUGOUT("API version is %d.%d.%d\n", version.release, version.major, version.minor);
    // Pass the address of void pointer, it will be updated with the address
    // of GSPI instance which can be used in other APIs.
    status = sl_si91x_gspi_init(SL_GSPI_MASTER, &gspi_driver_handle);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gspi_init: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("GSPI initialization is successful \n");
    // Fetching the status of GSPI i.e., busy, data lost and mode fault
    gspi_status = sl_si91x_gspi_get_status(gspi_driver_handle);
    DEBUGOUT("GSPI status is fetched successfully \n");
    DEBUGOUT("Busy: %d\n", gspi_status.busy);
    DEBUGOUT("Data_Lost: %d\n", gspi_status.data_lost);
    DEBUGOUT("Mode_Fault: %d\n", gspi_status.mode_fault);
    //Configuration of all other parameters that are required by GSPI
    // gspi_configuration structure is from sl_si91x_gspi_init.h file.
    // The user can modify this structure with the configuration of
    // his choice by filling this structure.
    status = sl_si91x_gspi_set_configuration(gspi_driver_handle, &config);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gspi_control: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("GSPI configuration is successful \n");
    // Register user callback function
    status = sl_si91x_gspi_register_event_callback(gspi_driver_handle, callback_event);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gspi_register_event_callback: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("GSPI user event callback registered successfully \n");
    // Fetching and printing the current clock division factor
    DEBUGOUT("Current Clock division factor is %lu \n", sl_si91x_gspi_get_clock_division_factor(gspi_driver_handle));
    // Fetching and printing the current frame length
    DEBUGOUT("Current Frame Length is %lu \n", sl_si91x_gspi_get_frame_length());
    if (sl_si91x_gspi_get_frame_length() > GSPI_BIT_WIDTH) {
      gspi_division_factor = sizeof(gspi_data_out[0]);
    }
    // Syncing master and slave
    wait_for_sync(SYNC_TIME);
    // As per the macros enabled in the header file, it will configure the current mode.
    if (SL_USE_TRANSFER) {
      current_mode = SL_GSPI_TRANSFER_DATA;
      break;
    }
    if (SL_USE_SEND) {
      current_mode = SL_GSPI_SEND_DATA;
      break;
    }
    current_mode = SL_GSPI_RECEIVE_DATA;
  } while (false);
}
/*******************************************************************************
 * Function will run continuously in while loop
 ******************************************************************************/
void gspi_example_process_action(void)
{
  sl_status_t status;
  // In this switch case, according to the macros enabled in header file, it starts to execute the APIs
  // Assuming all the macros are enabled, after transfer, receive will be executed and after receive
  // send will be executed.
  switch (current_mode) {
    case SL_GSPI_TRANSFER_DATA:
      if (begin_transmission == true) {
        // Validation for executing the API only once
        sl_si91x_gspi_set_slave_number(GSPI_SLAVE_0);
        status = sl_si91x_gspi_transfer_data(gspi_driver_handle,
                                             gspi_data_out,
                                             gspi_data_in,
                                             sizeof(gspi_data_out) / gspi_division_factor);
        if (status != SL_STATUS_OK) {
          // If it fails to execute the API, it will not execute rest of the things
          DEBUGOUT("sl_si91x_gspi_transfer_data: Error Code : %lu \n", status);
          current_mode = SL_GSPI_TRANSMISSION_COMPLETED;
          break;
        }
        DEBUGOUT("GSPI transfer begin successfully \n");
        begin_transmission = false;
      }
      if (transfer_complete) {
        transfer_complete = false;
        DEBUGOUT("GSPI transfer completed successfully \n");
        // After comparing the loopback transfer, it compares the gspi_data_out and
        // gspi_data_in.
        compare_loop_back_data();
        if (SL_USE_SEND) {
          // If send macro is enabled, current mode is set to send
          current_mode       = SL_GSPI_SEND_DATA;
          begin_transmission = true;
          break;
        }
        if (SL_USE_RECEIVE) {
          // If receive macro is enabled, current mode is set to receive
          current_mode       = SL_GSPI_RECEIVE_DATA;
          begin_transmission = true;
          break;
        }
        // If above macros are not enabled, current mode is set to complete
        current_mode = SL_GSPI_TRANSMISSION_COMPLETED;
      }
      break;

    case SL_GSPI_SEND_DATA:
      if (begin_transmission) {
        // Validation for executing the API only once
        sl_si91x_gspi_set_slave_number(GSPI_SLAVE_0);
        status =
          sl_si91x_gspi_send_data(gspi_driver_handle, gspi_data_out, sizeof(gspi_data_out) / gspi_division_factor);
        if (status != SL_STATUS_OK) {
          // If it fails to execute the API, it will not execute rest of the things
          DEBUGOUT("sl_si91x_gspi_send_data: Error Code : %lu \n", status);
          current_mode = SL_GSPI_TRANSMISSION_COMPLETED;
          break;
        }
        DEBUGOUT("GSPI send begin successfully \n");
        begin_transmission = false;
      }
      if (transfer_complete) {
        // If DMA is enabled, it will wait until transfer_complete flag is set.
        transfer_complete = false;
        // At last current mode is set to completed.
        if (SL_USE_RECEIVE) {
          current_mode       = SL_GSPI_RECEIVE_DATA;
          begin_transmission = true;
          DEBUGOUT("GSPI send completed \n");
          break;
        }
        DEBUGOUT("GSPI send completed \n");
        // If receive macro is not enabled, current mode is set to completed.
        current_mode = SL_GSPI_TRANSMISSION_COMPLETED;
      }
      break;

    case SL_GSPI_RECEIVE_DATA:
      if (begin_transmission == true) {
        wait_for_sync(RECEIVE_SYNC_TIME);
        // Validation for executing the API only once
        sl_si91x_gspi_set_slave_number(GSPI_SLAVE_0);
        status =
          sl_si91x_gspi_receive_data(gspi_driver_handle, gspi_data_in, sizeof(gspi_data_in) / gspi_division_factor);
        if (status != SL_STATUS_OK) {
          // If it fails to execute the API, it will not execute rest of the things
          DEBUGOUT("sl_si91x_gspi_receive_data: Error Code : %lu \n", status);
          current_mode = SL_GSPI_TRANSMISSION_COMPLETED;
          break;
        }
        DEBUGOUT("GSPI receive begin successfully \n");
        begin_transmission = false;
        //Waiting till the receive is completed
      }
      if (transfer_complete) {
        // If DMA is enabled, it will wait until transfer_complete flag is set.
        transfer_complete = false;
        DEBUGOUT("GSPI receive completed \n");
        compare_loop_back_data();
        // At last, current mode is set to completed
        current_mode = SL_GSPI_TRANSMISSION_COMPLETED;
      }
      break;

    case SL_GSPI_TRANSMISSION_COMPLETED:
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
  // If the data width is not standard (8-bit) then the data should be masked.
  // The extra bits of the integer should be always zero.
  // For example, if bit width is 7, then from 8-15 all bits should be zero in a 16 bit integer.
  // So mask has value according to the data width and it is applied to the data.
  uint16_t data_index   = 0;
  uint32_t frame_length = 0;
  uint16_t mask         = (uint16_t)~0;
  frame_length          = sl_si91x_gspi_get_frame_length();
  mask                  = mask >> (GSPI_MAX_BIT_WIDTH - frame_length);
  for (data_index = 0; data_index < GSPI_BUFFER_SIZE; data_index++) {
    gspi_data_in[data_index] &= mask;
    gspi_data_out[data_index] &= mask;
    // If the data in and data out are same, it will be continued else, the for
    // loop will be break.
    if (gspi_data_in[data_index] != gspi_data_out[data_index]) {
      break;
    }
  }
  if (data_index == GSPI_BUFFER_SIZE) {
    DEBUGOUT("Data comparison successful, Loop Back Test Passed \n");
  } else {
    DEBUGOUT("Data comparison failed, Loop Back Test failed \n");
  }
}

/*******************************************************************************
 * Callback event function
 * It is responsible for the event which are triggered by GSPI interface
 * It updates the respective member of the structure as the event is triggered.
 ******************************************************************************/
static void callback_event(uint32_t event)
{
  switch (event) {
    case SL_GSPI_TRANSFER_COMPLETE:
      transfer_complete = true;
      break;
    case SL_GSPI_DATA_LOST:
      break;
    case SL_GSPI_MODE_FAULT:
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
