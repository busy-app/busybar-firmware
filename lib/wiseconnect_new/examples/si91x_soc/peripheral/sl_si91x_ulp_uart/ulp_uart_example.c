/***************************************************************************/
/**
 * @file ulp_uart_example.c
 * @brief Ulp UART examples functions
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
#include "ulp_uart_example.h"
#include "rsi_debug.h"
#include "rsi_egpio.h"
#include "rsi_rom_egpio.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_si91x_power_manager.h"
#include "sl_si91x_usart.h"
#include "sl_si91x_wireless_shutdown.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/

#define ULP_UART_BUFFER_SIZE 1024   // Data send and receive length
#define USART_BAUDRATE       115200 // Baud rate <9600-7372800>
#define ULP_GPIO_PIN         2      // ULP GPIO to receive
#define ULP_GPIO_TOGGLE      8      // ULP GPIO to toggle
#define OUTPUT_VALUE         1      // GPIO output value
#define ULP_GPIO_PORT        4      // GPIO Port no
#define SET                  1      // Macro to set
#define CLR                  0      // Macro to clear
#define NON_UC_DEFAULT_CONFIG \
  0 //  Enable this macro to set the default configurations in non_uc case, this is useful when someone don't want to use UC configuration
#define ULP_BANK_OFFSET     0x800                                         // ULP Memory bank offset value.
#define TX_BUF_MEMORY       (ULP_SRAM_START_ADDR + (1 * ULP_BANK_OFFSET)) // Transfer buffer start address at 2K
#define RX_BUF_MEMORY       (ULP_SRAM_START_ADDR + (2 * ULP_BANK_OFFSET)) // Rx buffer start address at 4k
#define FIRST_ITERATION     1    // first iteration from High Power to Ultra Low Power state
#define SECOND_ITERATION    2    // second iteration from Ultra Low Power state to High Power
#define THIRD_ITERATION     3    // third iteration deinitialization of uart.
#define MS_DELAY_COUNTER    4600 // Delay count
#define FIVE_SECOND_DELAY   5000 // giving the 5 second delay in between state changes
#define MINIMUM_COUNT_VALUE 0    // initiating the minimum count value as zero
#define MAXIMUM_COUNT_VALUE 10   // maximum comparison pin triggering for referal

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static uint8_t ulp_uart_data_in[ULP_UART_BUFFER_SIZE];
static uint8_t ulp_uart_data_out[ULP_UART_BUFFER_SIZE];

volatile boolean_t ulp_uart_send_complete = false, ulp_uart_transfer_complete = false,
                   ulp_uart_receive_complete = false;
static boolean_t ulp_uart_begin_transmission = true;
static uint8_t ulp_uart_count                = MINIMUM_COUNT_VALUE;
static void delay(uint32_t idelay);
static sl_power_state_t current_power_state = SL_SI91X_POWER_MANAGER_PS4;
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
void ulp_uart_callback_event(uint32_t event);
static void compare_loop_back_data(void);
static void configure_ps2_power_state(void);
/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/
sl_usart_handle_t usart_handle;
usart_mode_enum_t current_mode = SL_ULP_UART_SEND_DATA;
sl_gpio_t ulp_gpio_rx          = { ULP_GPIO_PORT, ULP_GPIO_PIN };
sl_gpio_t ulp_gpio_toggle      = { ULP_GPIO_PORT, ULP_GPIO_TOGGLE };

/*******************************************************************************
 * USART Example Initialization function
 ******************************************************************************/
void usart_example_init(void)
{
  sl_status_t status;
  sl_si91x_usart_control_config_t usart_config;
  sl_si91x_usart_control_config_t get_config;
#if NON_UC_DEFAULT_CONFIG
  usart_config.baudrate      = USART_BAUDRATE;
  usart_config.mode          = SL_USART_MODE_ASYNCHRONOUS;
  usart_config.parity        = SL_USART_NO_PARITY;
  usart_config.stopbits      = SL_USART_STOP_BITS_1;
  usart_config.hwflowcontrol = SL_USART_FLOW_CONTROL_NONE;
  usart_config.databits      = SL_USART_DATA_BITS_8;
  usart_config.misc_control  = SL_USART_MISC_CONTROL_NONE;
  usart_config.usart_module  = ULPUART;
  usart_config.config_enable = ENABLE;
  usart_config.synch_mode    = DISABLE;
#endif

  // Set ulp pin 10 in GPIO mode
  sl_gpio_driver_set_pin_mode(&ulp_gpio_toggle, EGPIO_PIN_MUX_MODE0, OUTPUT_VALUE);
  // Set output direction
  sl_si91x_gpio_driver_set_pin_direction(ULP_GPIO_PORT, ULP_GPIO_TOGGLE, (sl_si91x_gpio_direction_t)GPIO_OUTPUT);

  do {
    // Initialize the UART
    status = sl_si91x_usart_init(ULPUART, &usart_handle);
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
    status = sl_si91x_usart_multiple_instance_register_event_callback(ULPUART, ulp_uart_callback_event);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_usart_multiple_instance_register_event_callback: Error Code : %lu \n", status);
      break;
    }
    DEBUGOUT("USART user event callback registered successfully \n");
    sl_si91x_usart_get_configurations(ULPUART, &get_config);
    DEBUGOUT("Baud Rate = %ld \n", get_config.baudrate);
  } while (false);
}

/*******************************************************************************
 * Example ticking function
 ******************************************************************************/
void usart_example_process_action(void)
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
  sl_status_t status;
  sl_si91x_usart_control_config_t usart_config;
  // In this switch case, according to the macros enabled in header file, it
  // starts to execute the APIs
  switch (current_mode) {
    case SL_ULP_UART_SEND_DATA:
      // Validation for executing the API only once
      if (ulp_uart_begin_transmission == true) {
        // Fill the data buffer to be send
        for (uint16_t i = 0; i < ULP_UART_BUFFER_SIZE; i++) {
          ulp_uart_data_out[i] = (uint8_t)(i + 1);
        }

        memcpy((uint8_t *)TX_BUF_MEMORY, &ulp_uart_data_out, sizeof(ulp_uart_data_out[0]) * ULP_UART_BUFFER_SIZE);

        status = sl_si91x_usart_send_data(usart_handle, (uint8_t *)TX_BUF_MEMORY, sizeof(ulp_uart_data_out));

        if (status != SL_STATUS_OK) {
          // If it fails to execute the API, it will not execute rest of the
          // things
          DEBUGOUT("sl_si91x_usart_send_data: Error Code : %lu \n", status);
          current_mode = SL_ULP_UART_POWER_STATE_TRANSITION;
          break;
        }
        ulp_uart_begin_transmission = false;
      }

      ulp_uart_send_complete = false;

      if (USE_RECEIVE) {
        // If receive macro is enabled, current mode is set to receive
        current_mode                = SL_ULP_UART_RECEIVE_DATA;
        ulp_uart_begin_transmission = true;
        break;
      }
      DEBUGOUT("USART send completed successfully \n");
      // Current mode is set to complete
      current_mode = SL_ULP_UART_POWER_STATE_TRANSITION;
      break;

    case SL_ULP_UART_RECEIVE_DATA:
      if (ulp_uart_begin_transmission == true) {
        // Validation for executing the API only once
        status = sl_si91x_usart_receive_data(usart_handle, (uint8_t *)RX_BUF_MEMORY, sizeof(ulp_uart_data_in));
        if (status != SL_STATUS_OK) {
          // If it fails to execute the API, it will not execute rest of the
          // things
          DEBUGOUT("sl_si91x_usart_receive_data: Error Code : %lu \n", status);
          current_mode = SL_ULP_UART_POWER_STATE_TRANSITION;
          break;
        }
        DEBUGOUT("USART receive begin successfully \n");
        ulp_uart_begin_transmission = false;
      }

      if (ulp_uart_receive_complete) {

        // Waiting till the receive is completed
        memcpy(&ulp_uart_data_in, (uint8_t *)RX_BUF_MEMORY, sizeof(ulp_uart_data_in[0]) * ULP_UART_BUFFER_SIZE);
        // Update the receive compelete flag with 0.
        ulp_uart_receive_complete = false;
        if ((USE_SEND) && (ulp_uart_count <= MAXIMUM_COUNT_VALUE)) {
          // If send macro is enabled, current mode is set to send
          current_mode                = SL_ULP_UART_SEND_DATA;
          ulp_uart_begin_transmission = true;
          compare_loop_back_data();
          DEBUGOUT("USART receive completed \n");
          ulp_uart_count++;
          break;
        }

        DEBUGOUT("USART send completed successfully \n");
        DEBUGOUT("USART receive completed \n");
        compare_loop_back_data();
        // If send macro is not enabled, current mode is set to completed.
        current_mode = SL_ULP_UART_POWER_STATE_TRANSITION;
      }
      break;

    case SL_ULP_UART_POWER_STATE_TRANSITION:
      if (current_power_state == SL_SI91X_POWER_MANAGER_PS4) {
        // Control power management by adjusting clock references and shutting down
        // the power supply
        // This function is for demonstration purpose only. For more details, refer to the README file.
        sl_si91x_wireless_shutdown();
        // switching the power state PS4 to PS2 mode.
        sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS2);
        // Configuring the ps2 power state by configuring
        // the ram retention and removing the unused peripherals
        configure_ps2_power_state();
        // Reconfigure the uart configs once power state changes. In case of UC macro is not set, user has to pass the usart_configs
        sl_si91x_usart_set_configuration(usart_handle, &usart_config);
        // giving 5 sec delay for current consumption verification
        delay(FIVE_SECOND_DELAY);
        // count value re initiating to 0
        ulp_uart_count = MINIMUM_COUNT_VALUE;
        // Changing send data flag to true for next cycle of transfer
        ulp_uart_begin_transmission = true;
        // Changing mode to send data
        current_mode = SL_ULP_UART_SEND_DATA;
        // current power state is updated to PS2
        current_power_state = SL_SI91X_POWER_MANAGER_PS2;
      } else if (current_power_state == SL_SI91X_POWER_MANAGER_PS2) {
        // giving 5 sec delay for current consumption verification
        delay(FIVE_SECOND_DELAY);
        // switching the power state from PS2 to PS4 mode
        sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS4);
        // Reconfigure the uart configs once power state changes, In case of UC macro is not set, user has to pass the usart_configs
        sl_si91x_usart_set_configuration(usart_handle, &usart_config);
        // count value re initiating to 0
        ulp_uart_count = MINIMUM_COUNT_VALUE;
        // Changing send data flag to true for next cycle of transfer
        ulp_uart_begin_transmission = true;
        // Changing mode to send data
        current_mode = SL_ULP_UART_SEND_DATA;
        // current power state is updated to last enum after the power state cycle
        // is completed
        current_power_state = LAST_ENUM_POWER_STATE;
      } else {
        // After third cycle of data transfer De-initializing i2c instance and
        // unregistering callback
        status = sl_si91x_usart_deinit(usart_handle);
        if (status != SL_STATUS_OK) {
          DEBUGOUT("sl_si91x_usart_deinit : Invalid Parameters, "
                   "Error Code : %lx \n",
                   status);
          break;
        }
        current_mode = SL_ULP_UART_TRANSMISSION_COMPLETED;
      }
      break;
    case SL_ULP_UART_TRANSMISSION_COMPLETED:
      break;
    default:
      break;
  }
}

/*******************************************************************************
 * Compares data received buffer via USART with data transfer buffer and print
 * the loopback test passed or failed
 ******************************************************************************/
static void compare_loop_back_data(void)
{
  uint16_t data_index = 0;
  // Check for data in and data out are same, if same then comparision
  // will continue till end of the buffer
  for (data_index = 0; data_index < ULP_UART_BUFFER_SIZE; data_index++) {
    if (ulp_uart_data_in[data_index] != ulp_uart_data_out[data_index]) {
      break;
    }
  }
  if (data_index == ULP_UART_BUFFER_SIZE) {
    // Set the pin
    sl_gpio_driver_set_pin(&ulp_gpio_toggle);
    // Clear the pin
    sl_gpio_driver_clear_pin(&ulp_gpio_toggle);
    DEBUGOUT("Data comparison successful, Loop Back Test Passed \n");
  } else {
    DEBUGOUT("Data comparison failed, Loop Back Test failed \n");
  }
}

/*******************************************************************************
 * Callback function triggered on data Transfer and reception
 ******************************************************************************/
void ulp_uart_callback_event(uint32_t event)
{
  switch (event) {
    case SL_USART_EVENT_SEND_COMPLETE:
      ulp_uart_send_complete = true;
      break;
    case SL_USART_EVENT_RECEIVE_COMPLETE:
      ulp_uart_receive_complete = true;
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
 * shutting down the power supply, adjusting the clocks,
 * powering off the peripherals not in use,
 * Configuring power manager ram-retention
 ******************************************************************************/
static void configure_ps2_power_state(void)
{
  sl_status_t status;
  sl_power_peripheral_t peri;
  sl_power_ram_retention_config_t config;
  // Clear the peripheral configuration
  peri.m4ss_peripheral = 0;
  // Configure RAM banks for retention during power management
  config.configure_ram_banks = true; // Enable RAM bank configuration
  config.m4ss_ram_banks      = SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_9
                          | SL_SI91X_POWER_MANAGER_M4SS_RAM_BANK_10; // Specify the RAM banks to be
                                                                     // retained during power
                                                                     // management
  config.ulpss_ram_banks = SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_2 | SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_3;
  // Ored value for ulpss peripheral.
  // Ored value for ulpss peripheral.
  peri.ulpss_peripheral = SL_SI91X_POWER_MANAGER_ULPSS_PG_MISC | SL_SI91X_POWER_MANAGER_ULPSS_PG_SSI
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_I2S | SL_SI91X_POWER_MANAGER_ULPSS_PG_I2C
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_IR | SL_SI91X_POWER_MANAGER_ULPSS_PG_FIM
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_AUX;
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
    sl_si91x_power_manager_configure_ram_retention(&config);
  } while (false);
}
