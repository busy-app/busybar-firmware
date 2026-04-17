/*******************************************************************************
 * @file  gpio_driver_example.c
 * @brief  gpio driver example
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
/**============================================================================
 * @brief : This file contains example application for GPIO example
 * @section Description :
 * This application demonstrates toggling, pin, group interrupts of GPIO pin
  for HP, ULP and UULP instances.
 ============================================================================**/
#include "ulp_gpio_example.h"
#include "rsi_debug.h"
#include "sl_driver_gpio.h"
#include "sl_gpio_board.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_si91x_power_manager.h"
#include "sl_si91x_wireless_shutdown.h"
#include "sl_si91x_clock_manager.h"
/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
#define PORT0             0      // PORT 0
#define AVL_INTR_NO       0      // available interrupt number
#define POLARITY          0      // Polarity for GPIO pin
#define AND_EVENT         0      // AND for group interrupt
#define LEVEL_EVENT       0      // level for group interrupt
#define INT_CH            0      // GPIO Pin interrupt 0
#define ULP_INT_CH        0      // ULP GPIO Pin interrupt 0
#define MODE_0            0      // Initializing GPIO MODE_0 value
#define PORT1             1      // PORT 1
#define EDGE_EVENT        1      // Edge for group interrupt
#define OR_EVENT          1      // OR for group interrupt
#define OUTPUT_VALUE      1      // GPIO output value
#define PIN_COUNT         2      // Number of interrupts needed
#define GRP_COUNT         2      // Count of group interrupt pins
#define FALL_EDGE         8      // Fall edge event
#define PAD_SELECT_9      9      // GPIO PAD selection number
#define MAX_PAD_SELECT    34     // Maximum GPIO PAD selection number
#define PINS              0x0c40 // Pins in a port to mask
#define FIVE_SECOND_DELAY 5000   // Delay for 5 sec
#define UULP_GPIO_INTR_2  2      // UULP GPIO pin interrupt 2
#define TOGGLE_COUNT      10     // Count for number of times to repeat
/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/
// Enum for enabling GPIO instances functionality
typedef enum {
  ULP_GPIO_PIN        = 1, // Initialize GPIO ULP instance
  UULP_GPIO_PIN       = 0, // Initialize GPIO UULP instance
  ULP_GPIO_PIN_INTR   = 0, // Configure GPIO ULP instance pin interrupt
  ULP_GPIO_GROUP_INTR = 0, // Configure GPIO ULP instance group interrupt
  UULP_GPIO_PIN_INTR  = 0, // Configure GPIO UULP instance pin interrupt check
} gpio_instance_type_t;

typedef enum {
  SL_ULP_GPIO_PROCESS_ACTION,    // gpio process action mode
  SL_ULP_POWER_STATE_TRANSITION, // gpio power state transition mode
  SL_ULP_GPIO_TERMINATED,        // gpio termination mode
} ulp_gpio_current_mode_t;

ulp_gpio_current_mode_t ulp_gpio_current_state = SL_ULP_GPIO_PROCESS_ACTION;
static sl_power_state_t current_power_state    = SL_SI91X_POWER_MANAGER_PS4;
/*******************************************************************************
 *************************** LOCAL VARIABLES   *********************************
 ******************************************************************************/
static sl_si91x_gpio_group_interrupt_config_t config_grp_int;
uint8_t pad_sel = 1;
uint32_t i;
static uulp_pad_config_t uulp_pad;
typedef sl_gpio_t sl_si91x_gpio_t;
typedef sl_gpio_mode_t sl_si91x_gpio_mode_t;
uint8_t gpio_toggle_count;
/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void gpio_driver_ulp_initialization(void);
static void gpio_driver_uulp_initialization(void);
static void gpio_uulp_pin_interrupt_callback(uint32_t pin_intr);
static void gpio_ulp_pin_interrupt_callback(uint32_t pin_intr);
static void gpio_ulp_group_interrupt_callback(uint32_t pin_intr);
static void configuring_ps2_power_state(void);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * GPIO example initialization function. It initializes HP/ULP clock, pin mode,
 * direction and configure pin and group interrupts
 ******************************************************************************/
void gpio_example_init(void)
{
  sl_status_t status;
  uint8_t direction;
  sl_si91x_gpio_version_t version;

  // Version information of gpio
  version = sl_si91x_gpio_driver_get_version();
  DEBUGOUT("gpio version is fetched successfully \n");
  DEBUGOUT("API version is %d.%d.%d\n", version.release, version.major, version.minor);
  do {
    // Initialize GPIO ULP instance
    if (ULP_GPIO_PIN == SET) {
      sl_si91x_gpio_t gpio_port_pin = { SL_SI91X_ULP_GPIO_8_PORT, SL_SI91X_ULP_GPIO_8_PIN };
#ifndef SL_SI91X_ACX_MODULE
      sl_si91x_gpio_t port_pin = { SL_SI91X_ULP_GPIO_10_PORT, SL_SI91X_ULP_GPIO_10_PIN };
#else
      sl_si91x_gpio_t port_pin          = { SL_SI91X_ULP_GPIO_4_PORT, SL_SI91X_ULP_GPIO_4_PIN };
#endif
      sl_si91x_gpio_mode_t mode = MODE_0;
      // GPIO initialization function for ULP instance
      gpio_driver_ulp_initialization();
      // Get the pin direction for ULP GPIO pin
      direction = sl_si91x_gpio_driver_get_pin_direction(gpio_port_pin.port, gpio_port_pin.pin);
      DEBUGOUT("get_pin_direction = %d\n", direction);
      direction = sl_si91x_gpio_driver_get_pin_direction(port_pin.port, port_pin.pin);
      DEBUGOUT("get_pin_direction = %d\n", direction);

      // Get the pin mode for ULP GPIO pin
      status = sl_gpio_driver_get_pin_mode(&gpio_port_pin, &mode);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_gpio_driver_get_pin_mode, Error code: %lu", status);
        break;
      }
      DEBUGOUT("GPIO driver get pin mode is successful \n");
      DEBUGOUT("get_pin_mode = %d\n", mode);
      status = sl_gpio_driver_get_pin_mode(&port_pin, &mode);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_gpio_driver_get_pin_mode, Error code: %lu", status);
        break;
      }
      DEBUGOUT("GPIO driver get pin mode is successful \n");
      DEBUGOUT("get_pin_mode = %d\n", mode);
    }

    // Configure GPIO ULP instance pin interrupt
    if (ULP_GPIO_PIN_INTR == SET) {
      // GPIO initialization function for ULP instance
      gpio_driver_ulp_initialization();
      // Configure ULP GPIO pin interrupts
      status = sl_si91x_gpio_driver_configure_ulp_pin_interrupt(
        ULP_INT_CH,
        (sl_si91x_gpio_interrupt_config_flag_t)SL_GPIO_INTERRUPT_FALL_EDGE,
        SL_SI91X_ULP_GPIO_8_PIN,
        (sl_gpio_irq_callback_t)&gpio_ulp_pin_interrupt_callback);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_gpio_driver_configure_ulp_pin_interrupt, Error code: %lu", status);
        break;
      }
      DEBUGOUT("GPIO driver configure ulp pin interrupt is successful \n");
    }

    // Configure GPIO ULP instance group interrupt
    if (ULP_GPIO_GROUP_INTR == SET) {
#ifndef SL_SI91X_ACX_MODULE
      uint8_t ulp_group_pins[PIN_COUNT] = { SL_SI91X_ULP_GPIO_8_PIN,
                                            SL_SI91X_ULP_GPIO_10_PIN }; // pins for group interrupt in IC boards
#else
      uint8_t ulp_group_pins[PIN_COUNT] = { SL_SI91X_ULP_GPIO_8_PIN,
                                            SL_SI91X_ULP_GPIO_4_PIN }; // pins for group interrupt in ACX Module boards
#endif
      uint8_t ulp_group_pol[PIN_COUNT] = { POLARITY, POLARITY }; // polarity selected for group interrupt

      // Configure ULP GPIO group parameters
      config_grp_int.grp_interrupt     = ULP_GROUP_INTR_0; // Set ULP group interrupt
      config_grp_int.grp_interrupt_cnt = GRP_COUNT;        // Count of group interrupt pins
      config_grp_int.and_or            = AND_EVENT;        // AND/OR of group interrupt
      config_grp_int.level_edge        = LEVEL_EVENT;      // Level/Edge of group interrupt
      memcpy(config_grp_int.grp_interrupt_pin, ulp_group_pins, PIN_COUNT);
      memcpy(config_grp_int.grp_interrupt_pol, ulp_group_pol, PIN_COUNT);

      // GPIO initialization function for ULP instance
      gpio_driver_ulp_initialization();
      // Configure group interrupt for grp_config_int structure
      status =
        sl_si91x_gpio_driver_configure_ulp_group_interrupt(&config_grp_int,
                                                           (sl_gpio_irq_callback_t)&gpio_ulp_group_interrupt_callback);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_gpio_driver_configure_ulp_group_interrupt, Error "
                 "code: %lu",
                 status);
        break;
      }
      DEBUGOUT("GPIO driver configure ulp group interrupt is successful \n");
    }

    // Initialize GPIO UULP instance
    if (UULP_GPIO_PIN == SET) {
      // GPIO initialization function for UULP instance
      gpio_driver_uulp_initialization();
      // Get NPSS GPIO pin direction
      direction = sl_si91x_gpio_driver_get_uulp_npss_direction(SL_SI91X_UULP_GPIO_0_PIN);
      DEBUGOUT("get_direction = %d\n", direction);
    }

    // Configure GPIO UULP instance pin interrupt
    if (UULP_GPIO_PIN_INTR == SET) {
      uulp_pad.gpio_padnum = SL_SI91X_UULP_GPIO_2_PIN; // UULP GPIO pin number 2 is selected
      uulp_pad.pad_select  = SET;                      // UULP GPIO PAD is selected
      uulp_pad.mode        = CLR;                      // UULP GPIO mode 0 is selected
      uulp_pad.direction   = SET;                      // UULP GPIO direction is selected
      uulp_pad.receiver    = SET;                      // UULP GPIO receiver is enabled

      // GPIO initialization function for UULP instance
      gpio_driver_uulp_initialization();
      status = sl_si91x_gpio_driver_set_uulp_pad_configuration(&uulp_pad);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_gpio_driver_set_uulp_pad_configuration, Error code: %lu", status);
        break;
      }
      DEBUGOUT("GPIO driver set uulp pad configuration is successful \n");
      // Configure pin interrupt for sl_si91x_gpio_interrupt_config_flag_t
      // structure
      status = sl_si91x_gpio_driver_configure_uulp_interrupt(
        (sl_si91x_gpio_interrupt_config_flag_t)SL_GPIO_INTERRUPT_RISE_EDGE,
        UULP_GPIO_INTR_2,
        (sl_gpio_irq_callback_t)&gpio_uulp_pin_interrupt_callback);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("sl_si91x_gpio_driver_configure_uulp_interrupt, Error code: %lu", status);
        break;
      }
      DEBUGOUT("GPIO driver configure uulp interrupt is successful \n");
    }
  } while (false);
}
/*******************************************************************************
 * Function will run continuously in while loop and led toggles for every 1sec
 ******************************************************************************/
void gpio_example_process_action(void)
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
  switch (ulp_gpio_current_state) {
    case SL_ULP_GPIO_PROCESS_ACTION:
      gpio_toggle_count = 0;
      do {
        // Toggle GPIO ULP Pin
        if (ULP_GPIO_PIN == SET) {
          sl_si91x_gpio_t port_pin = { SL_SI91X_ULP_GPIO_2_PORT, SL_SI91X_ULP_GPIO_2_PIN };
          status                   = sl_gpio_driver_toggle_pin(&port_pin); // Toggle ULP GPIO pin
          if (status != SL_STATUS_OK) {
            DEBUGOUT("sl_gpio_driver_toggle_pin, Error code: %lu", status);
            break;
          }
        }
        // Toggle GPIO UULP Pin
        if (UULP_GPIO_PIN == SET) {
          // Set UULP GPIO pin
          status = sl_si91x_gpio_driver_set_uulp_npss_pin_value(SL_SI91X_UULP_GPIO_0_PIN, SET);
          if (status != SL_STATUS_OK) {
            DEBUGOUT("sl_si91x_gpio_driver_set_uulp_npss_pin_value, Error code: %lu", status);
            break;
          }
          // Clear UULP GPIO pin
          status = sl_si91x_gpio_driver_set_uulp_npss_pin_value(SL_SI91X_UULP_GPIO_0_PIN, CLR);
          if (status != SL_STATUS_OK) {
            DEBUGOUT("sl_si91x_gpio_driver_set_uulp_npss_pin_value, Error code: %lu", status);
            break;
          }
        }
      } while (gpio_toggle_count++ < TOGGLE_COUNT);
      // current mode being updated with power state transition to change the
      // power state mode
      ulp_gpio_current_state = SL_ULP_POWER_STATE_TRANSITION;
      break;
    case SL_ULP_POWER_STATE_TRANSITION:
      if (current_power_state == SL_SI91X_POWER_MANAGER_PS4) {
        DEBUGOUT("Switching GPIO from PS4->PS2 state \n");
        // Control power management by adjusting clock references and shutting down
        // the power supply
        // This function is for demonstration purpose only. For more details, refer to the README file.
        sl_si91x_wireless_shutdown();
        // switching the power state PS4 to PS2 mode.
        sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS2);
        /* Due to calling trim_efuse API om power manager it will change the clock
    frequency, if we are not initialize the debug again it will print the
    garbage data or no data in console output. */
        DEBUGINIT();
        // Configuring the ps2 power state by adjusting the clocks, configuring
        // the ram retention and removing the unused peripherals
        configuring_ps2_power_state();
        // giving 5 sec delay for current consumption verification
        sl_si91x_delay_ms(FIVE_SECOND_DELAY);
        // current power state is updated to PS2
        current_power_state = SL_SI91X_POWER_MANAGER_PS2;
        // current mode is updated with process action for verifying the
        // triggering and processing
        ulp_gpio_current_state = SL_ULP_GPIO_PROCESS_ACTION;
      } else if (current_power_state == SL_SI91X_POWER_MANAGER_PS2) {
        DEBUGOUT("Switching GPIO from PS2 -> PS4  state \n");
        // switching the power state from PS2 to PS4 mode
        sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS4);
        DEBUGINIT();
        // giving 5 sec delay for current consumption verification
        sl_si91x_delay_ms(FIVE_SECOND_DELAY);
        // current power state is updated to last enum after the power state cycle
        // is completed
        current_power_state = LAST_ENUM_POWER_STATE;
        // current mode is updated with process action for verifying the
        // triggering and processing
        ulp_gpio_current_state = SL_ULP_GPIO_PROCESS_ACTION;
      } else {
        //  de initializing the gpio
        status = sl_gpio_driver_deinit();
        if (status != SL_STATUS_OK) {
          DEBUGOUT("sl_gpio_driver_deinit, Error code: %lu", status);
          break;
        }
        DEBUGOUT("GPIO driver de-initialization is successful \n");
        ulp_gpio_current_state = SL_ULP_GPIO_TERMINATED;
      }
      break;
    case SL_ULP_GPIO_TERMINATED:
      break;
    default:
      break;
  }
}

/*******************************************************************************
 * ULP GPIO initialization function
 ******************************************************************************/
static void gpio_driver_ulp_initialization(void)
{
  sl_status_t status;
  sl_gpio_driver_init();
  sl_si91x_gpio_t gpio_port_pin1 = { SL_SI91X_ULP_GPIO_1_PORT, SL_SI91X_ULP_GPIO_1_PIN };
  sl_si91x_gpio_t gpio_port_pin2 = { SL_SI91X_ULP_GPIO_2_PORT, SL_SI91X_ULP_GPIO_2_PIN };
  sl_si91x_gpio_t gpio_port_pin8 = { SL_SI91X_ULP_GPIO_8_PORT, SL_SI91X_ULP_GPIO_8_PIN };
  sl_gpio_mode_t mode            = MODE_0;

  do {
    DEBUGOUT("\r\n ULP_GPIO_PIN test starts \r\n");
    status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)ULPCLK_GPIO); // Enable GPIO ULP_CLK
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_enable_clock, Error code: %lu", status);
      break;
    }
    DEBUGOUT("GPIO driver clock enable is successful \n");
    // Enable pad receiver for ULP GPIO pins
    status = sl_si91x_gpio_driver_enable_ulp_pad_receiver(SL_SI91X_ULP_GPIO_1_PIN);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_enable_ulp_pad_receiver, Error code: %lu", status);
      break;
    }
    DEBUGOUT("GPIO driver ulp pad receiver enable is successful \n");
    status = sl_si91x_gpio_driver_enable_ulp_pad_receiver(SL_SI91X_ULP_GPIO_2_PIN);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_enable_ulp_pad_receiver, Error code: %lu", status);
      break;
    }
    DEBUGOUT("GPIO driver ulp pad receiver enable is successful \n");
    status = sl_si91x_gpio_driver_enable_ulp_pad_receiver(SL_SI91X_ULP_GPIO_8_PIN);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_enable_ulp_pad_receiver, Error code: %lu", status);
      break;
    }
    DEBUGOUT("GPIO driver ulp pad receiver enable is successful \n");

    // Select pad driver strength for ULP GPIO pins
    status =
      sl_si91x_gpio_driver_select_ulp_pad_driver_strength(SL_SI91X_ULP_GPIO_1_PIN,
                                                          (sl_si91x_gpio_driver_strength_select_t)GPIO_TWO_MILLI_AMPS);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_select_ulp_pad_driver_strength, Error "
               "code: %lu",
               status);
      break;
    }
    DEBUGOUT("GPIO driver ulp pad driver strength selection is successful \n");
    // Select pad driver disable state for ULP GPIO pins
    status = sl_si91x_gpio_driver_select_ulp_pad_driver_disable_state(SL_SI91X_ULP_GPIO_1_PIN,
                                                                      (sl_si91x_gpio_driver_disable_state_t)GPIO_HZ);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_select_ulp_pad_driver_disable_state, "
               "Error code: %lu",
               status);
      break;
    }
    DEBUGOUT("GPIO driver ulp pad driver disable state selection is successful \n");
    // Set the pin mode for ULP GPIO pins.
    status = sl_gpio_driver_set_pin_mode(&gpio_port_pin1, mode, OUTPUT_VALUE);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_gpio_driver_set_pin_mode, Error code: %lu", status);
      break;
    }
    DEBUGOUT("GPIO driver set pin mode is successful \n");
    status = sl_gpio_driver_set_pin_mode(&gpio_port_pin2, mode, OUTPUT_VALUE);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_gpio_driver_set_pin_mode, Error code: %lu", status);
      break;
    }
    DEBUGOUT("GPIO driver set pin mode is successful \n");
    status = sl_gpio_driver_set_pin_mode(&gpio_port_pin8, mode, OUTPUT_VALUE);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_gpio_driver_set_pin_mode, Error code: %lu", status);
      break;
    }
    DEBUGOUT("GPIO driver set pin mode is successful \n");

    // Set the pin direction for ULP GPIO pins.
    status = sl_si91x_gpio_driver_set_pin_direction(SL_SI91X_ULP_GPIO_1_PORT,
                                                    SL_SI91X_ULP_GPIO_1_PIN,
                                                    (sl_si91x_gpio_direction_t)GPIO_INPUT);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_set_pin_direction, Error code: %lu", status);
      break;
    }
    DEBUGOUT("GPIO driver set pin direction is successful \n");
    status = sl_si91x_gpio_driver_set_pin_direction(SL_SI91X_ULP_GPIO_2_PORT,
                                                    SL_SI91X_ULP_GPIO_2_PIN,
                                                    (sl_si91x_gpio_direction_t)GPIO_OUTPUT);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_set_pin_direction, Error code: %lu", status);
      break;
    }
    DEBUGOUT("GPIO driver set pin direction is successful \n");
    status = sl_si91x_gpio_driver_set_pin_direction(SL_SI91X_ULP_GPIO_8_PORT,
                                                    SL_SI91X_ULP_GPIO_8_PIN,
                                                    (sl_si91x_gpio_direction_t)GPIO_INPUT);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_set_pin_direction, Error code: %lu", status);
      break;
    }
    DEBUGOUT("GPIO driver set pin direction is successful \n");
  } while (false);
}

/*******************************************************************************
 * UULP GPIO initialization function
 ******************************************************************************/
static void gpio_driver_uulp_initialization(void)
{
  sl_status_t status;
  sl_gpio_driver_init();
  do {
    DEBUGOUT("\r\n UULP_GPIO_PIN test starts \r\n");
    status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)ULPCLK_GPIO); // Enable GPIO ULP_CLK
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_enable_clock, Error code: %lu", status);
      break;
    }
    DEBUGOUT("GPIO driver clock enable is successful \n");
    // By default making all the interrupts zero.
    GPIO_NPSS_GPIO_CONFIG_REG = CLR;
    // Set NPSS GPIO input buffer
    status = sl_si91x_gpio_driver_select_uulp_npss_receiver(SL_SI91X_UULP_GPIO_0_PIN, SET);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_select_uulp_npss_receiver, Error code: %lu", status);
      break;
    }
    DEBUGOUT("GPIO driver uulp receiver selection is successful \n");
    status = sl_si91x_gpio_driver_select_uulp_npss_receiver(SL_SI91X_UULP_GPIO_2_PIN, SET);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_select_uulp_npss_receiver, Error code: %lu", status);
      break;
    }
    DEBUGOUT("GPIO driver uulp receiver selection is successful \n");
    // Set NPSS GPIO pin MUX
    status = sl_si91x_gpio_driver_set_uulp_npss_pin_mux(SL_SI91X_UULP_GPIO_0_PIN, NPSS_GPIO_PIN_MUX_MODE0);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_set_uulp_npss_pin_mux, Error code: %lu", status);
      break;
    }
    DEBUGOUT("GPIO driver uulp pin mux selection is successful \n");
    status = sl_si91x_gpio_driver_set_uulp_npss_pin_mux(SL_SI91X_UULP_GPIO_2_PIN, NPSS_GPIO_PIN_MUX_MODE0);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_set_uulp_npss_pin_mux, Error code: %lu", status);
      break;
    }
    DEBUGOUT("GPIO driver uulp pin mux selection is successful \n");
    // Set NPSS GPIO pin direction
    status =
      sl_si91x_gpio_driver_set_uulp_npss_direction(SL_SI91X_UULP_GPIO_0_PIN, (sl_si91x_gpio_direction_t)GPIO_OUTPUT);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_set_uulp_npss_direction, Error code: %lu", status);
      break;
    }
    DEBUGOUT("GPIO driver uulp pin direction selection is successful \n");
    status =
      sl_si91x_gpio_driver_set_uulp_npss_direction(SL_SI91X_UULP_GPIO_2_PIN, (sl_si91x_gpio_direction_t)GPIO_INPUT);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_set_uulp_npss_direction, Error code: %lu", status);
      break;
    }
    DEBUGOUT("GPIO uulp driver clear pin direction selection is successful \n");
  } while (false);
}

/*******************************************************************************
 *  This API handles UULP GPIO pin interrupt 0 request
 ******************************************************************************/
static void gpio_uulp_pin_interrupt_callback(uint32_t pin_intr)
{
  if (pin_intr == UULP_GPIO_INTR_2) {
    // This is with respect to ISR context. Debugout might cause issues
    // sometimes.
    DEBUGOUT("gpio uulp pin interrupt2\n");
  }
}

/*******************************************************************************
 *  This API handles ULP GPIO OR'ed pin interrupt request
 ******************************************************************************/
static void gpio_ulp_pin_interrupt_callback(uint32_t pin_intr)
{
  if (pin_intr == ULP_PIN_INTR_0) {
    // This is with respect to ISR context. Debugout might cause issues
    // sometimes.
    DEBUGOUT("gpio ulp pin interrupt0\n");
  }
}

/*******************************************************************************
 *  This API handles ULP GPIO group interrupt request
 ******************************************************************************/
static void gpio_ulp_group_interrupt_callback(uint32_t pin_intr)
{
  if (pin_intr == ULP_GROUP_INTR_0) {
    // This is with respect to ISR context. Debugout might cause issues
    // sometimes.
    DEBUGOUT("gpio ulp group interrupt0\n");
  }
}
/*******************************************************************************
 * powering off the peripherals not in use,
 * Configuring power manager ram-retention
 ******************************************************************************/
static void configuring_ps2_power_state(void)
{
  sl_status_t status;
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
  config.ulpss_ram_banks = SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_2 | SL_SI91X_POWER_MANAGER_ULPSS_RAM_BANK_3;
  // Ored value for ulpss peripheral.
  peri.ulpss_peripheral = SL_SI91X_POWER_MANAGER_ULPSS_PG_MISC | SL_SI91X_POWER_MANAGER_ULPSS_PG_SSI
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_I2S | SL_SI91X_POWER_MANAGER_ULPSS_PG_I2C
                          | SL_SI91X_POWER_MANAGER_ULPSS_PG_IR | SL_SI91X_POWER_MANAGER_ULPSS_PG_UDMA
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
