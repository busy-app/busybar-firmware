/***************************************************************************/
/**
 * @file ulp_i2c_leader_example.c
 * @brief ULP_I2C Leader Blocking example functions
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
#include "ulp_i2c_leader_example.h"
#include "rsi_debug.h"
#include "sl_i2c_instances.h"
#include "sl_si91x_i2c.h"
#include "sl_si91x_peripheral_i2c.h"
#include "sl_si91x_power_manager.h"
#include "sl_si91x_wireless_shutdown.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define I2C_INSTANCE_USED 2    // For ULP application instance used should be 2 only
#define FOLLOWER_I2C_ADDR 0x50 // I2C follower address
#define MAX_BUFFER_SIZE_ULP_NON_BLOCKING \
  10240 // Maximum buffer size for RX and TX length when transferring with DMA in ULP mode
#define I2C_BUFFER_SIZE           1024  // Size of data buffer
#define I2C_TX_FIFO_THRESHOLD     0     // FIFO threshold
#define I2C_RX_FIFO_THRESHOLD     0     // FIFO threshold
#define INITIAL_VALUE             0     // Initial value of buffer
#define BUFFER_OFFSET             0x1   // Buffer offset
#define MS_DELAY_COUNTER          4600  // Delay count
#define TEN_SECOND_DELAY_HP_MODE  20000 // giving the 10 second delay in HP mode
#define TEN_SECOND_DELAY_ULP_MODE 10000 // giving the 10 second delay in ULP mode

/*******************************************************************************
 ******************************  Data Types  ***********************************
 ******************************************************************************/
// Enum for different transmission scenarios
typedef enum {
  SL_ULP_I2C_SEND_DATA,              // Send mode
  SL_ULP_I2C_RECEIVE_DATA,           // receive mode
  SL_ULP_I2C_POWER_STATE_TRANSITION, // power state transition for i2c
  SL_ULP_I2C_TRANSMISSION_COMPLETED, // Transmission completed mode
} i2c_action_enum_t;

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/

static sl_i2c_instance_t i2c_instance = I2C_INSTANCE_USED;
static uint8_t i2c_read_buffer[I2C_BUFFER_SIZE];
static uint8_t i2c_write_buffer[I2C_BUFFER_SIZE];
static i2c_action_enum_t current_mode       = SL_ULP_I2C_SEND_DATA;
static sl_power_state_t current_power_state = SL_SI91X_POWER_MANAGER_PS4;
static boolean_t i2c_send_data_flag         = false;
static boolean_t i2c_receive_data_flag      = false;
static sl_i2c_config_t sl_i2c_config;
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void delay(uint32_t idelay);
static void compare_data(void);
static void configuring_ps2_power_state(void);
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * I2C example initialization function
 ******************************************************************************/
void ulp_i2c_leader_example_init(void)
{
  sl_i2c_status_t i2c_status;
  sl_i2c_config = sl_i2c_i2c2_config;

  // Initializing I2C instance (update i2c config-strucure name as per instance used)
  i2c_status = sl_i2c_driver_init(i2c_instance, &sl_i2c_config);
  if (i2c_status != SL_I2C_SUCCESS) {
    DEBUGOUT("sl_i2c_driver_init : Invalid Parameters, Error Code : %u \n", i2c_status);
  } else {
    DEBUGOUT("Successfully initialized & configured i2c leader in PS4 mode\n");
  }
  // Configuring RX and TX FIFO thresholds
  i2c_status = sl_i2c_driver_configure_fifo_threshold(i2c_instance, I2C_TX_FIFO_THRESHOLD, I2C_RX_FIFO_THRESHOLD);
  if (i2c_status != SL_I2C_SUCCESS) {
    DEBUGOUT("sl_i2c_driver_configure_fifo_threshold : Invalid Parameters, Error Code : %u \n", i2c_status);
  } else {
    DEBUGOUT("Successfully configured i2c TX & RX FIFO thresholds\n");
  }
  // Generating a buffer with values that needs to be sent.
  for (uint32_t loop = INITIAL_VALUE; loop < I2C_BUFFER_SIZE; loop++) {
    i2c_write_buffer[loop] = (uint8_t)(loop + BUFFER_OFFSET);
  }
  i2c_send_data_flag = true;
}

/*******************************************************************************
 * Function will run continuously in while loop
 ******************************************************************************/
void ulp_i2c_leader_example_process_action(void)
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
  sl_i2c_status_t i2c_status;
  // In switch case, according to the current mode, the transmission is
  // executed.
  // First application runs in High power(PS4) mode leader sends data to
  // follower, using I2C send Then leader receives same data from follower,
  // using I2C receive. Then application moves to Ultra Low Power(PS2) state and
  // reconfig I2C for PS2 mode and data transfer occurs. Then again application
  // moves to PS4 state and reconfig I2C for PS4 mode and data transfer occurs.
  // data from follower, using I2C receive. Then de-initializes I2C
  switch (current_mode) {
    case SL_ULP_I2C_SEND_DATA:
      if (i2c_send_data_flag) {
        //  Validation for executing the API only once.
        i2c_status =
          sl_i2c_driver_send_data_blocking(i2c_instance, FOLLOWER_I2C_ADDR, i2c_write_buffer, I2C_BUFFER_SIZE);
        if (i2c_status != SL_I2C_SUCCESS) {
          DEBUGOUT("sl_i2c_driver_send_data_blocking : Invalid Parameters,  Error Code : %u \n", i2c_status);
          if (i2c_status != SL_I2C_TIMEOUT) {
            i2c_send_data_flag = false;
          }
          break;
        }
        i2c_send_data_flag = false;
      }
      i2c_receive_data_flag = true;
      current_mode          = SL_ULP_I2C_RECEIVE_DATA;
      break;
    case SL_ULP_I2C_RECEIVE_DATA:
      if (i2c_receive_data_flag) {
        // Validation for executing the API only once.
        i2c_status =
          sl_i2c_driver_receive_data_blocking(i2c_instance, FOLLOWER_I2C_ADDR, i2c_read_buffer, I2C_BUFFER_SIZE);
        if (i2c_status != SL_I2C_SUCCESS) {
          DEBUGOUT("sl_i2c_driver_receive_data_blocking : Invalid Parameters, Error Code : %u \n", i2c_status);
          if (i2c_status != SL_I2C_TIMEOUT) {
            i2c_receive_data_flag = false;
          }
          break;
        }
        i2c_receive_data_flag = false;
      }
      current_mode = SL_ULP_I2C_POWER_STATE_TRANSITION;
      // After the receive is completed, input and output data is compared and
      // output is printed on console.
      compare_data();
      break;
    case SL_ULP_I2C_POWER_STATE_TRANSITION:
      // After first cycle of data transfer changing to PS2 mode
      if (current_power_state == SL_SI91X_POWER_MANAGER_PS4) {
        DEBUGOUT("Switching PS4->PS2 state, reset follower within 10 seconds\n");
        // Control power management by adjusting clock references and shutting down
        // the power supply
        // This function is for demonstration purpose only. For more details, refer to the README file.
        sl_si91x_wireless_shutdown();
        // Adding 10 seconds delay before 2nd cycle of transfer
        delay(TEN_SECOND_DELAY_HP_MODE);
        // switching the power state PS4 to PS2 mode.
        sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS2);
        /* Due to calling trim_efuse API om power manager it will change the clock
    frequency, if we are not initialize the debug again it will print the
    garbage data or no data in console output. */
        DEBUGINIT();
        // Configuring the ps2 power state by configuring
        // the ram retention and removing the unused peripherals
        configuring_ps2_power_state();
        // reconfiguring I2C leader as per new power mode
        i2c_status = sl_i2c_driver_leader_reconfig_on_power_mode_change(SL_I2C_ULP_MODE);
        if (i2c_status != SL_I2C_SUCCESS) {
          DEBUGOUT("sl_i2c_driver_leader_reconfig_on_power_mode_change : Invalid Parameters, Error Code : %u \n",
                   i2c_status);
        }
        DEBUGOUT("Successfully re-configured I2C leader for PS2 mode\n");
        // current power state is updated to PS2
        current_power_state = SL_SI91X_POWER_MANAGER_PS2;
        // Changing send data flag to true for next cycle of transfer
        i2c_send_data_flag = true;
        // Changing mode to send data
        current_mode = SL_ULP_I2C_SEND_DATA;
      }
      // After second cycle of data transfer changing mode back to PS4.
      else if (current_power_state == SL_SI91X_POWER_MANAGER_PS2) {
        DEBUGOUT("Switching PS2->PS4 state, reset follower within 10 seconds\n");
        // Adding 10 seconds delay before 2nd cycle of transfer
        delay(TEN_SECOND_DELAY_ULP_MODE);
        // switching the power state from PS2 to PS4 mode
        sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS4);
        /* Due to calling trim_efuse API om power manager it will change the clock
    frequency, if we are not initialize the debug again it will print the
    garbage data or no data in console output. */
        DEBUGINIT();
        // reconfiguring I2C leader as per new power mode
        i2c_status = sl_i2c_driver_leader_reconfig_on_power_mode_change(SL_I2C_HP_MODE);
        if (i2c_status != SL_I2C_SUCCESS) {
          DEBUGOUT("sl_i2c_driver_leader_reconfig_on_power_mode_change : Invalid Parameters, Error Code : %u \n",
                   i2c_status);
        }
        DEBUGOUT("Successfully re-configured I2C leader for PS4 mode\n");
        // current power state is updated to last enum after the power state cycle
        // is completed
        current_power_state = LAST_ENUM_POWER_STATE;
        // Changing send data flag to true for next cycle of transfer
        i2c_send_data_flag = true;
        // Changing mode to send data
        current_mode = SL_ULP_I2C_SEND_DATA;
      } else {
        // After third cycle of data transfer De-initializing i2c instance and
        // unregistering callback
        i2c_status = sl_i2c_driver_deinit(i2c_instance);
        if (i2c_status != SL_I2C_SUCCESS) {
          DEBUGOUT("sl_i2c_driver_deinit : Invalid Parameters, Error Code : %u \n", i2c_status);
          break;
        }
        current_mode = SL_ULP_I2C_TRANSMISSION_COMPLETED;
      }
      break;
    case SL_ULP_I2C_TRANSMISSION_COMPLETED:
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
    DEBUGOUT("Leader-Follower read-write Data comparison is successful, Test Case Passed \n");
  } else {
    DEBUGOUT("Leader-Follower read-write Data comparison is not successful, Test Case Failed \n");
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
  // Configure RAM banks for retention during power management
  config.configure_ram_banks = true; // Enable RAM bank configuration
  config.m4ss_ram_banks      = SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_8 | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_9
                          | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_10; // Specify the RAM banks to be
                                                                     // retained during power
                                                                     // management
  config.ulpss_ram_banks = 0;
  // Clear the peripheral configuration
  peri.m4ss_peripheral = SL_SI91X_POWER_MANAGER_M4SS_PG_QSPI | SL_SI91X_POWER_MANAGER_M4SS_PG_EFUSE
                         | SL_SI91X_POWER_MANAGER_M4SS_PG_SDIO_SPI;
  // Ored value for ulpss peripheral.
  peri.ulpss_peripheral = SL_SI91X_POWER_MANAGER_ULPSS_PG_MISC | SL_SI91X_POWER_MANAGER_ULPSS_PG_SSI
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_I2S | SL_SI91X_POWER_MANAGER_ULPSS_PG_IR
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_FIM | SL_SI91X_POWER_MANAGER_ULPSS_PG_AUX;
  // Ored value for npss peripheral.
  peri.npss_peripheral = SL_SI91X_POWER_MANAGER_NPSS_PG_MCUWDT | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUPS
                         | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUTS | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE2
                         | SL_SI91X_POWER_MANAGER_NPSS_PG_MCUSTORE3;
  do {
    // Peripherals passed in this API are powered off.
    sl_si91x_power_manager_remove_peripheral_requirement(&peri);
    // RAM retention modes are configured and passed into this API.
    sl_si91x_power_manager_configure_ram_retention(&config);
  } while (false);
}
