/***************************************************************************/
/**
 * @file ulp_i2s_example.c
 * @brief Ulp I2S example functions
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
#include "ulp_i2s_example.h"

#include "rsi_debug.h"
#include "sl_si91x_i2s.h"
#include "sl_si91x_i2s_config.h"
#include "sl_si91x_power_manager.h"
#include "sl_si91x_wireless_shutdown.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define I2S_SOC_PLL_REF_FREQUENCY 40000000  // PLL input REFERENCE clock 40MHZ
#define I2S_PS4_SOC_FREQ          180000000 // PLL out clock 180MHz
#define I2S_LOWPOWER_BUFFER_SIZE  1024      // Transmit/Receive buffer size
#define ULP_I2S_INSTANCE          1         // I2S instance
#define I2S_ULP_BANK_OFFSET       0x800
#define I2S_TX_BUF_MEMORY         (ULP_SRAM_START_ADDR + (1 * I2S_ULP_BANK_OFFSET))
#define I2S_RX_BUF_MEMORY         (ULP_SRAM_START_ADDR + (2 * I2S_ULP_BANK_OFFSET))
#define MS_DELAY_COUNTER          4600 // Delay count
#define FIVE_SECOND_DELAY         5000 // giving the 5 second delay in between state changes

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static uint16_t i2s_lowpower_data_in[I2S_LOWPOWER_BUFFER_SIZE];
static uint16_t i2s_lowpower_data_out[I2S_LOWPOWER_BUFFER_SIZE];
static sl_i2s_handle_t i2s_driver_handle     = NULL;
static uint8_t i2s_lowpower_send_complete    = 0;
static uint8_t i2s_lowpower_receive_complete = 0;
static sl_i2s_xfer_config_t i2s_xfer_config  = { 0 };
sl_status_t status;

/*******************************************************************************
 ******************************  Data Types  ***********************************
 ******************************************************************************/
// Enum for different transmission scenarios
typedef enum {
  SL_ULP_I2S_PROCESS_ACTION,         // i2s event triggering and operational
                                     // transforming
  SL_ULP_I2S_POWER_STATE_TRANSITION, // i2s power state transition mode
  SL_ULP_I2S_TRANSMISSION_COMPLETED, // i2s transmission completed mode
} ulp_i2s_enum_t;

static ulp_i2s_enum_t ulp_i2s_current_mode  = SL_ULP_I2S_PROCESS_ACTION;
static sl_power_state_t current_power_state = SL_SI91X_POWER_MANAGER_PS4;
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void callback_event(uint32_t event);
static void compare_loop_back_data(void);
static void delay(uint32_t idelay);
static void configuring_ps2_power_state(void);
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * I2S example initialization function
 ******************************************************************************/
void ulp_i2s_example_init(void)
{
  i2s_xfer_config.mode          = SL_I2S_MASTER;
  i2s_xfer_config.protocol      = SL_I2S_PROTOCOL;
  i2s_xfer_config.resolution    = SL_ULP_I2S_RESOLUTION;
  i2s_xfer_config.sampling_rate = SL_ULP_I2S_SAMPLING_RATE;
  i2s_xfer_config.sync          = SL_I2S_ASYNC;
  i2s_xfer_config.data_size     = SL_I2S_DATA_SIZE16;

  // Filling the data out array with integer values
  for (uint32_t i = 0; i < I2S_LOWPOWER_BUFFER_SIZE; i++) {
    i2s_lowpower_data_out[i] = (uint16_t)i;
  }
  memcpy((uint16_t *)I2S_TX_BUF_MEMORY, i2s_lowpower_data_out, sizeof(i2s_lowpower_data_out));
  do {
    // Initialize I2S peripheral and store driver handle in i2s_driver_handle
    status = sl_si91x_i2s_init(ULP_I2S_INSTANCE, &i2s_driver_handle);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("I2S Initialization fail\r\n");
      break;
    }
    DEBUGOUT("I2S Initialization success\r\n");
    // Configure ARM full power mode
    status = sl_si91x_i2s_configure_power_mode(i2s_driver_handle, SL_I2S_FULL_POWER);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("I2S power mode config fail\r\n");
      break;
    }
    DEBUGOUT("I2S power mode config success\r\n");
    // Register user callback handler
    status = sl_si91x_i2s_register_event_callback(i2s_driver_handle, callback_event);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("I2S user callback register fail\r\n");
      break;
    }
    DEBUGOUT("I2S user callback register success\r\n");
    i2s_xfer_config.transfer_type = SL_I2S_TRANSMIT;
    // Configure transmitter parameters for i2s transfer
    if (sl_si91x_i2s_config_transmit_receive(i2s_driver_handle, &i2s_xfer_config)) {
      DEBUGOUT("I2S transmit config fail\r\n");
      break;
    }
    DEBUGOUT("I2S transmit config success\r\n");
    i2s_xfer_config.transfer_type = SL_I2S_RECEIVE;
    // Configure receiver parameters for i2s transfer
    if (sl_si91x_i2s_config_transmit_receive(i2s_driver_handle, &i2s_xfer_config)) {
      DEBUGOUT("I2S receive config fail\r\n");
      break;
    }
    DEBUGOUT("I2S receive config success\r\n");
    // Configure I2S receive DMA channel
    if (sl_si91x_i2s_receive_data(i2s_driver_handle, (uint16_t *)I2S_RX_BUF_MEMORY, I2S_LOWPOWER_BUFFER_SIZE)) {
      DEBUGOUT("I2S receive start fail\r\n");
      break;
    }
    DEBUGOUT("I2S receive start success\r\n");
    // Configure I2S transmit DMA channel
    if (sl_si91x_i2s_transmit_data(i2s_driver_handle, (uint16_t *)I2S_TX_BUF_MEMORY, I2S_LOWPOWER_BUFFER_SIZE)) {
      DEBUGOUT("I2S transmit start fail\r\n");
      break;
    }
    DEBUGOUT("I2S transmit start success\r\n");
  } while (false);
}
/*******************************************************************************
 * Function will run continuously in while loop
 ******************************************************************************/
void ulp_i2s_example_process_action(void)
{
  /*************************************************************************************************
   * This section manages power state transitions within the system, optimizing
   *power consumption while maintaining essential functionality. It transitions
   *the system from a higher power state (PS4) to a lower one (PS2) during
   *specific operations to conserve power. This involves adjusting clock
   * references and shutting down unnecessary power supplies. After completing
   *the operation, the code transitions back to the higher power state (PS4) to
   *ensure adequate resources for subsequent tasks. This approach balances power
   *efficiency with operational requirements across various system functions.
   ***************************************************************************************************/
  switch (ulp_i2s_current_mode) {
    case SL_ULP_I2S_PROCESS_ACTION:
      if ((i2s_lowpower_send_complete && i2s_lowpower_receive_complete)) {
        memcpy(i2s_lowpower_data_in, (uint16_t *)I2S_RX_BUF_MEMORY, sizeof(i2s_lowpower_data_in));
        // Data has been transferred and received successfully
        // Validate the transmit and receive data count
        if ((sl_si91x_i2s_get_transmit_data_count(i2s_driver_handle) == I2S_LOWPOWER_BUFFER_SIZE)
            && (sl_si91x_i2s_get_receive_data_count(i2s_driver_handle) == I2S_LOWPOWER_BUFFER_SIZE)) {
          // I2S transfer completed
          DEBUGOUT("I2S transfer complete\r\n");
          // Compare transmit data and receive data
          compare_loop_back_data();
        }
        // reset send and receive complete status flags
        i2s_lowpower_send_complete    = 0;
        i2s_lowpower_receive_complete = 0;
        //  de initializing the i2s
        if (!(sl_si91x_i2s_deinit((sl_i2s_handle_t *)i2s_driver_handle))) {
          DEBUGOUT("DEINIT SUCCESS\n");
        }
        // current mode being updated with power state transition to change the
        // power state mode
        ulp_i2s_current_mode = SL_ULP_I2S_POWER_STATE_TRANSITION;
      }
      break;
    case SL_ULP_I2S_POWER_STATE_TRANSITION:
      if (current_power_state == SL_SI91X_POWER_MANAGER_PS4) {
        DEBUGOUT("Switching i2s from PS4->PS2 state \n");
        // Control power management by adjusting clock references and shutting down
        // the power supply
        // This function is for demonstration purpose only. For more details, refer to the README file.
        sl_si91x_wireless_shutdown();
        // switching the power state PS4 to PS2 mode.
        status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS2);
        if (status != SL_STATUS_OK) {
          DEBUGOUT("sl_si91x_power_manager_add_ps_requirement: Error Code : %lu \n", status);
          break;
        }
        /* Due to calling trim_efuse API om power manager it will change the clock
    frequency, if we are not initialize the debug again it will print the
    garbage data or no data in console output. */
        DEBUGINIT();
        // Configuring the ps2 power state by configuring
        // the ram retention and removing the unused peripherals
        configuring_ps2_power_state();
        // giving 5 sec delay for current consumption verification
        delay(FIVE_SECOND_DELAY);
        // Initializing the i2s
        ulp_i2s_example_init();
        // current mode is updated with process action for verifying the
        // triggering and processing
        ulp_i2s_current_mode = SL_ULP_I2S_PROCESS_ACTION;
        // current power state is updated to PS2
        current_power_state = SL_SI91X_POWER_MANAGER_PS2;
      } else if (current_power_state == SL_SI91X_POWER_MANAGER_PS2) {
        DEBUGOUT("Switching the i2s from PS2->PS4 state\n");
        // switching the power state from PS2 to PS4 mode
        status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS4);
        if (status != SL_STATUS_OK) {
          DEBUGOUT("sl_si91x_power_manager_add_ps_requirement: Error Code : %lu \n", status);
          break;
        }
        /* Due to calling trim_efuse API om power manager it will change the clock
    frequency, if we are not initialize the debug again it will print the
    garbage data or no data in console output. */
        DEBUGINIT();
        // giving 5 sec delay for current consumption verification
        delay(FIVE_SECOND_DELAY);
        // Initializing the i2s
        ulp_i2s_example_init();
        // current mode is updated with process action for verifying the
        // triggering and processing
        ulp_i2s_current_mode = SL_ULP_I2S_PROCESS_ACTION;
        // current power state is updated to last enum after the power state cycle
        // is completed
        current_power_state = LAST_ENUM_POWER_STATE;
      } else {
        ulp_i2s_current_mode = SL_ULP_I2S_TRANSMISSION_COMPLETED;
      }
      break;
    case SL_ULP_I2S_TRANSMISSION_COMPLETED:
      break;
    default:
      break;
  }
}

/*******************************************************************************
 * Delay function for 1ms
 ******************************************************************************/
static void delay(uint32_t idelay)
{
  for (uint32_t x = 0; x < MS_DELAY_COUNTER * idelay; x++) {
    __NOP();
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
  for (data_index = 0; data_index < I2S_LOWPOWER_BUFFER_SIZE; data_index++) {
    // If the data in and data out are same, it will be continued else, the for
    // loop will be break.
    if (i2s_lowpower_data_in[data_index] != i2s_lowpower_data_out[data_index]) {
      break;
    }
  }
  if (data_index == I2S_LOWPOWER_BUFFER_SIZE) {
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
      i2s_lowpower_send_complete = 1;
      break;
    case SL_I2S_RECEIVE_COMPLETE:
      i2s_lowpower_receive_complete = 1;
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
 * powering off the peripherals not in use,
 * Configuring power manager ram-retention
 ******************************************************************************/
static void configuring_ps2_power_state(void)
{
  sl_power_peripheral_t peri;
  sl_power_ram_retention_config_t config;
  // Clear the peripheral configuration
  peri.m4ss_peripheral = 0;
  // Configure RAM banks for retention during power management
  config.configure_ram_banks = true; // Enable RAM bank configuration
  config.m4ss_ram_banks      = SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_8 | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_9
                          | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_10; // Specify the RAM banks to be
                                                                     // retained during power
                                                                     // management
  config.ulpss_ram_banks = 0;
  // Ored value for ulpss peripheral.
  peri.ulpss_peripheral = SL_SI91X_POWER_MANAGER_ULPSS_PG_MISC | SL_SI91X_POWER_MANAGER_ULPSS_PG_SSI
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_I2C | SL_SI91X_POWER_MANAGER_ULPSS_PG_IR
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_FIM | SL_SI91X_POWER_MANAGER_ULPSS_PG_AUX;
  // Ored value for npss peripheral.
  peri.npss_peripheral = SL_SI91X_POWER_MANAGER_NPSS_PG_MCURTC | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUWDT
                         | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUPS | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUTS
                         | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE2 | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE3
                         | SL_SI91X_POWER_MANAGER_NPSS_PG_TIMEPERIOD;
  do {
    // Peripherals passed in this API are powered off.
    status = sl_si91x_power_manager_remove_peripheral_requirement(&peri);
    if (status != SL_STATUS_OK) {
      // If status is not OK, return with the error code.
      DEBUGOUT("sl_si91x_power_manager_remove_peripheral_requirement failed, "
               "Error Code: 0x%lX",
               status);
      break;
    }
    // RAM retention modes are configured and passed into this API.
    status = sl_si91x_power_manager_configure_ram_retention(&config);
    if (status != SL_STATUS_OK) {
      // If status is not OK, return with the error code.
      DEBUGOUT("sl_si91x_power_manager_configure_ram_retention failed, Error "
               "Code: 0x%lX",
               status);
      break;
    }
  } while (false);
}
