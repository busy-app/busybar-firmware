/******************************************************************************
* @file sl_si91x_driver_gpio.c
*******************************************************************************
* # License
* <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* SPDX-License-Identifier: Zlib
*
* The licensor of this software is Silicon Laboratories Inc.
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
******************************************************************************/
#include "sl_si91x_driver_gpio.h"
#include <stdio.h>
#include "sl_status.h"
/*******************************************************************************
 ***************************  DEFINES / MACROS ********************************
 ******************************************************************************/
#define GPIO_RELEASE_VERSION                0  // GPIO Release version
#define GPIO_MAJOR_VERSION                  0  // GPIO SQA version
#define GPIO_MINOR_VERSION                  2  // GPIO Developer version
#define GPIO_DIRECTION_MAX_VALUE            1  // GPIO maximum direction set value
#define GPIO_CLOCK_MAX_VAL                  1  // Validating clock for GPIO instance
#define GPIO_GROUP_INTERRUPT_MAX_VALUE      1  // Maximum number of GPIO group interrupts
#define GPIO_LEVEL_EDGE_MAX_VALUE           1  // GPIO maximum level edge value
#define GPIO_POLARITY_MAX_VALUE             1  // GPIO maximum polarity value
#define GPIO_AND_OR_MAX_VALUE               1  // GPIO maximum AND_OR value
#define GPIO_SLEW_RATE_MAX_VALUE            1  // GPIO maximum SLEW RATE value
#define GPIO_RECEIVER_MAX_VALUE             1  // GPIO maximum receiver value
#define GPIO_PIN_VALUE_MAX_VALUE            1  // GPIO maximum pin value
#define GPIO_STRENGTH_MAX_VAL               3  // GPIO maximum strength value
#define GPIO_DISABLE_STATE_MAX_VAL          3  // GPIO maximum disable state value
#define GPIO_UULP_MAX_PIN_NUM               5  // GPIO UULP maximum pin number
#define GPIO_MODE_MAX_VALUE                 7  // GPIO maximum MODE value
#define GPIO_INTERRUPT_MAX_VALUE            7  // GPIO interrupt maximum value
#define GPIO_ULP_INTERRUPT_MAX_VALUE        7  // GPIO interrupt maximum value
#define GPIO_ULP_MAX_PIN_NUM                11 // GPIO ULP maximum pin number
#define GPIO_UULP_INTERRUPT_VALUE_MAX_VALUE 16 // UULP GPIO maximum value
#define GPIO_UULP_WAKEUP_MAX_VALUE          4  // UULP GPIO maximum value
#define GPIO_UULP_PIN_MAX_VALUE             4  // UULP GPIO pin maximum value
#define GPIO_MAX_PAD_NUM                    34 // GPIO maximum pad number
#define GPIO_MAX_PIN_NUM                    57 // Maximum number of GPIO pins in m4 instance

#define ULP_GPIO_INTERRUPT_PRIORITY 18 // Priority 18 for ulp pin interrupt
#define GPIO_INTERRUPT_PRIORITY0    52 // Priority 52 for m4 pin interrupt 0
#define GPIO_INTERRUPT_PRIORITY1    53 // Priority 53 for m4 pin interrupt 1
#define GPIO_INTERRUPT_PRIORITY2    54 // Priority 54 for m4 pin interrupt 2
#define GPIO_INTERRUPT_PRIORITY3    55 // Priority 55 for m4 pin interrupt 3
#define GPIO_INTERRUPT_PRIORITY4    56 // Priority 56 for m4 pin interrupt 4
#define GPIO_INTERRUPT_PRIORITY5    57 // Priority 57 for m4 pin interrupt 5
#define GPIO_INTERRUPT_PRIORITY6    58 // Priority 58 for m4 pin interrupt 6
#define GPIO_INTERRUPT_PRIORITY7    59 // Priority 59 for m4 pin interrupt 7

#define GPIO_PAD_SELECT_NO_PAD 0  // GPIO PAD selection number 0
#define GPIO_PAD_SELECT_1      1  // GPIO PAD selection number 1
#define GPIO_PAD_SELECT_2      2  // GPIO PAD selection number 2
#define GPIO_PAD_SELECT_3      3  // GPIO PAD selection number 3
#define GPIO_PAD_SELECT_4      4  // GPIO PAD selection number 4
#define GPIO_PAD_SELECT_5      5  // GPIO PAD selection number 5
#define GPIO_PAD_SELECT_6      6  // GPIO PAD selection number 6
#define GPIO_PAD_SELECT_7      7  // GPIO PAD selection number 7
#define GPIO_PAD_SELECT_8      8  // GPIO PAD selection number 8
#define GPIO_PAD_SELECT_9      9  // GPIO PAD selection number 9
#define GPIO_PAD_SELECT_10     10 // GPIO PAD selection number 10
#define GPIO_PAD_SELECT_11     11 // GPIO PAD selection number 11
#define GPIO_PAD_SELECT_12     12 // GPIO PAD selection number 12
#define GPIO_PAD_SELECT_13     13 // GPIO PAD selection number 13
#define GPIO_PAD_SELECT_14     14 // GPIO PAD selection number 14
#define GPIO_PAD_SELECT_15     15 // GPIO PAD selection number 15
#define GPIO_PAD_SELECT_16     16 // GPIO PAD selection number 16
#define GPIO_PAD_SELECT_17     17 // GPIO PAD selection number 17
#define GPIO_PAD_SELECT_18     18 // GPIO PAD selection number 18
#define GPIO_PAD_SELECT_19     19 // GPIO PAD selection number 19
#define GPIO_PAD_SELECT_20     20 // GPIO PAD selection number 20
#define GPIO_PAD_SELECT_21     21 // GPIO PAD selection number 21
#define GPIO_PAD_SELECT_22     22 // GPIO PAD selection number 22
#define GPIO_PAD_SELECT_23     23 // GPIO PAD selection number 23
#define GPIO_PAD_SELECT_24     24 // GPIO PAD selection number 24
#define GPIO_PAD_SELECT_25     25 // GPIO PAD selection number 25
#define GPIO_PAD_SELECT_26     26 // GPIO PAD selection number 26
#define GPIO_PAD_SELECT_27     27 // GPIO PAD selection number 27
#define GPIO_PAD_SELECT_28     28 // GPIO PAD selection number 28
#define GPIO_PAD_SELECT_29     29 // GPIO PAD selection number 29
#define GPIO_PAD_SELECT_30     30 // GPIO PAD selection number 30
#define GPIO_PAD_SELECT_31     31 // GPIO PAD selection number 31
#define GPIO_PAD_SELECT_32     32 // GPIO PAD selection number 32
#define GPIO_PAD_SELECT_33     33 // GPIO PAD selection number 33
#define OUTPUT_VALUE           0  // GPIO output value

#define ULP_PERI_ON_SOC_GPIO_HOST_PAD_PIN_MODE 11 // Pin mode for HP pin to act as ULP pin within the host pad range
#define ULP_PERI_ON_SOC_GPIO_PORT_B_OFFSET     3  // Offset for PORT_B HP GPIO to act as ULP pin
#define ULP_PERI_ON_SOC_GPIO_PORT_C_OFFSET     6  // Offset for PORT_C HP GPIO to act as ULP pin
#define ULP_PERI_ON_SOC_GPIO_PORT_D_OFFSET     10 // Offset for PORT_D HP GPIO to act as ULP pin
#define SOC_PERI_ON_ULP_GPIO_OFFSET            64 // Offset for ULP GPIO to act as HP pin
#define ULP_PERI_ON_SOC_GPIO_HOST_PAD_OFFSET   19 // Offset for HP GPIO to act as ULP pin within the host pad range
#define ULP_PERI_ON_SOC_GPIO_OFFSET            6  // Offset for HP GPIO to act as ULP pin
#define ULP_PERI_ON_SOC_GPIO_PIN15_OFFSET      8  // Offset for HP pin to act as ULP pin for pin 15
#define ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MIN \
  46 // Minimum pin number for specific range of HP pins to act as ULP pins
#define ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MAX \
  49 // Maximum pin number for specific range of HP pins to act as ULP pins
#define ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_OFFSET 38 // Offset for specific range of HP pins to act as ULP pins
/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

///@brief structure to hold GPIO interrupt configurations
typedef struct {
  uint8_t gpio_priority[8];
  uint8_t gpio_nvic[8];
} sl_gpio_intr_configure_t;

/*******************************************************************************
 *************************** LOCAL VARIABLES   *********************************
 ******************************************************************************/

const sl_gpio_intr_configure_t intr_config = { { GPIO_INTERRUPT_PRIORITY0,
                                                 GPIO_INTERRUPT_PRIORITY1,
                                                 GPIO_INTERRUPT_PRIORITY2,
                                                 GPIO_INTERRUPT_PRIORITY3,
                                                 GPIO_INTERRUPT_PRIORITY4,
                                                 GPIO_INTERRUPT_PRIORITY5,
                                                 GPIO_INTERRUPT_PRIORITY6,
                                                 GPIO_INTERRUPT_PRIORITY7 },
                                               { PININT0_NVIC_NAME,
                                                 PININT1_NVIC_NAME,
                                                 PININT2_NVIC_NAME,
                                                 PININT3_NVIC_NAME,
                                                 PININT4_NVIC_NAME,
                                                 PININT5_NVIC_NAME,
                                                 PININT6_NVIC_NAME,
                                                 PININT7_NVIC_NAME } };
// The m4_gpio_pad array refers to PAD selected for particular GPIO pin. For example GPIO_0 selects GPIO_PAD_SELECT_NO_PAD.
// GPIO_6 selects GPIO_PAD_SELECT_1, GPIO_7 selects GPIO_PAD_SELECT_2 and so on.
static const uint8_t m4_gpio_pad[76] = {
  GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD,
  GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_1,      GPIO_PAD_SELECT_2,
  GPIO_PAD_SELECT_3,      GPIO_PAD_SELECT_4,      GPIO_PAD_SELECT_5,      GPIO_PAD_SELECT_6,
  GPIO_PAD_SELECT_7,      GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_8,
  GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD,
  GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD,
  GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_25,     GPIO_PAD_SELECT_26,     GPIO_PAD_SELECT_27,
  GPIO_PAD_SELECT_28,     GPIO_PAD_SELECT_29,     GPIO_PAD_SELECT_30,     GPIO_PAD_SELECT_9,
  GPIO_PAD_SELECT_9,      GPIO_PAD_SELECT_9,      GPIO_PAD_SELECT_9,      GPIO_PAD_SELECT_NO_PAD,
  GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD,
  GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD,
  GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_10,     GPIO_PAD_SELECT_11,
  GPIO_PAD_SELECT_12,     GPIO_PAD_SELECT_13,     GPIO_PAD_SELECT_14,     GPIO_PAD_SELECT_15,
  GPIO_PAD_SELECT_16,     GPIO_PAD_SELECT_17,     GPIO_PAD_SELECT_18,     GPIO_PAD_SELECT_19,
  GPIO_PAD_SELECT_20,     GPIO_PAD_SELECT_21,     GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD,
  GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD, GPIO_PAD_SELECT_NO_PAD,
  GPIO_PAD_SELECT_22,     GPIO_PAD_SELECT_23,     GPIO_PAD_SELECT_24,     GPIO_PAD_SELECT_25,
  GPIO_PAD_SELECT_26,     GPIO_PAD_SELECT_27,     GPIO_PAD_SELECT_28,     GPIO_PAD_SELECT_29,
  GPIO_PAD_SELECT_30,     GPIO_PAD_SELECT_31,     GPIO_PAD_SELECT_32,     GPIO_PAD_SELECT_33
};

static uint8_t ulp_gpio_pad[12] = { GPIO_PAD_SELECT_22, GPIO_PAD_SELECT_23, GPIO_PAD_SELECT_24, GPIO_PAD_SELECT_25,
                                    GPIO_PAD_SELECT_26, GPIO_PAD_SELECT_27, GPIO_PAD_SELECT_28, GPIO_PAD_SELECT_29,
                                    GPIO_PAD_SELECT_30, GPIO_PAD_SELECT_31, GPIO_PAD_SELECT_32, GPIO_PAD_SELECT_33 };

static uint32_t soc_peri_on_ulp_gpio_status = 0; // Holds the status of SOC peripheral on ulp gpio
/*******************************************************************************
 ***********************  Global function Prototypes ***************************
 ******************************************************************************/
void PIN_IRQ0_Handler(void);
void PIN_IRQ1_Handler(void);
void PIN_IRQ2_Handler(void);
void PIN_IRQ3_Handler(void);
void PIN_IRQ4_Handler(void);
void PIN_IRQ5_Handler(void);
void PIN_IRQ6_Handler(void);
void PIN_IRQ7_Handler(void);
void GRP_IRQ0_Handler(void);
void GRP_IRQ1_Handler(void);
void UULP_PIN_IRQ_Handler(void);
void ULP_PIN_IRQ_Handler(void);
void ULP_GROUP_IRQ_Handler(void);

/*******************************************************************************
 ************************       GLOBAL FUNCTIONS      **************************
 ******************************************************************************/
sl_gpio_irq_callback_t gpio_callback_function_pointer[GPIO_MAX_INTR_VALUE];
sl_gpio_irq_callback_t gpio_group_int_callback_fptr[MAX_GROUP_INT];
sl_gpio_irq_callback_t gpio_ulp_pin_int_callback_fptr[GPIO_MAX_INTR_VALUE];
sl_gpio_irq_callback_t gpio_ulp_group_int_callback_fptr[MAX_GROUP_INT];
sl_gpio_irq_callback_t gpio_uulp_pin_int_callback_fptr[MAX_UULP_INT];

/*******************************************************************************
 * @brief This API is used for GPIO pin configuration to set the direction, mode,
 *  pin and port, clock.
 ******************************************************************************/
sl_status_t sl_gpio_set_configuration(sl_si91x_gpio_pin_config_t pin_config)
{
  sl_status_t status;
  switch (pin_config.port_pin.port) {
    case SL_GPIO_PORT_A:
    case SL_GPIO_PORT_B:
    case SL_GPIO_PORT_C:
    case SL_GPIO_PORT_D:
      status = sl_gpio_validation(&pin_config.port_pin);
      if (status != SL_STATUS_OK) {
        return status;
      }
      // Check if the GPIO pad is selected and it's not NO PAD.
      if (m4_gpio_pad[(pin_config.port_pin.port * MAX_GPIO_PORT_PIN) + pin_config.port_pin.pin]
          != GPIO_PAD_SELECT_NO_PAD) {
        // Check if the GPIO pad is selected and not PAD_SELECT_9.
        if (m4_gpio_pad[(pin_config.port_pin.port * MAX_GPIO_PORT_PIN) + pin_config.port_pin.pin] != PAD_SELECT_9) {
          if (SL_GPIO_VALIDATE_HOST_PIN(pin_config.port_pin.port, pin_config.port_pin.pin)) {
            status = sl_si91x_gpio_driver_enable_host_pad_selection(
              m4_gpio_pad[(pin_config.port_pin.port * MAX_GPIO_PORT_PIN) + pin_config.port_pin.pin]);
            if (status != SL_STATUS_OK) {
              return status;
            }
          } else {
            status = sl_si91x_gpio_driver_enable_pad_selection(
              m4_gpio_pad[(pin_config.port_pin.port * MAX_GPIO_PORT_PIN) + pin_config.port_pin.pin]);
            if (status != SL_STATUS_OK) {
              return status;
            }
          }
        }
      }
      // Enable the pad receiver for the pin. It will enable the pad for respective GPIO pin
      if (pin_config.port_pin.port == SL_GPIO_PORT_A) {
        status = sl_si91x_gpio_driver_enable_pad_receiver(pin_config.port_pin.pin);
        if (status != SL_STATUS_OK) {
          return status;
        }
      } else {
        status = sl_si91x_gpio_driver_enable_pad_receiver((pin_config.port_pin.port * MAX_GPIO_PORT_PIN)
                                                          + pin_config.port_pin.pin);
        if (status != SL_STATUS_OK) {
          return status;
        }
      }
      // Set pin mode for the pin. By default mode 0 is set, which is a normal GPIO
      status = sl_gpio_driver_set_pin_mode(&pin_config.port_pin, (sl_gpio_mode_t)SL_GPIO_MODE_0, OUTPUT_VALUE);
      if (status != SL_STATUS_OK) {
        return status;
      }
      sl_si91x_gpio_set_pin_direction(pin_config.port_pin.port,
                                      pin_config.port_pin.pin,
                                      (sl_si91x_gpio_direction_t)pin_config.direction);
      break;
    default:
      break;
  }
  // Check if the GPIO port is the Ultra-Low Power GPIO port.
  if (pin_config.port_pin.port == SL_GPIO_ULP_PORT) {
    // Enable pad selection for the pin.
    status = sl_si91x_gpio_driver_enable_pad_selection(ulp_gpio_pad[pin_config.port_pin.pin]);
    if (status != SL_STATUS_OK) {
      return status;
    }
    // Enable ulp pad receiver for the pin. It will enable the pad for respective GPIO pin
    status = sl_si91x_gpio_driver_enable_ulp_pad_receiver(pin_config.port_pin.pin);
    if (status != SL_STATUS_OK) {
      return status;
    }
    // Set ulp pin mode for the pin. By default mode 0 is set, which is a normal GPIO
    status = sl_gpio_driver_set_pin_mode(&pin_config.port_pin, (sl_gpio_mode_t)SL_GPIO_MODE_0, OUTPUT_VALUE);
    if (status != SL_STATUS_OK) {
      return status;
    }
    sl_si91x_gpio_set_pin_direction(pin_config.port_pin.port,
                                    pin_config.port_pin.pin,
                                    (sl_si91x_gpio_direction_t)pin_config.direction);
  }
  // Check if the GPIO port is the Ultra-Ultra Low Power GPIO port.
  if (pin_config.port_pin.port == SL_GPIO_UULP_PORT) {
    // Enable uulp pad receiver for the pin.
    status = sl_si91x_gpio_driver_select_uulp_npss_receiver(pin_config.port_pin.pin, SET);
    if (status != SL_STATUS_OK) {
      return status;
    }
    // Set uulp pin mode for the pin. By default mode 0 is set, which is a normal GPIO
    status = sl_si91x_gpio_driver_set_uulp_npss_pin_mux(pin_config.port_pin.pin,
                                                        (sl_si91x_uulp_npss_mode_t)NPSS_GPIO_PIN_MUX_MODE0);
    if (status != SL_STATUS_OK) {
      return status;
    }
    sl_si91x_gpio_set_uulp_npss_direction(pin_config.port_pin.pin, (sl_si91x_gpio_direction_t)pin_config.direction);
  }
  return SL_STATUS_OK;
}

/*******************************************************************************
 * @brief This API is used to configure the pin interrupt in 3 instance.
 * To configure the interrupt, first GPIO initialization must be done.
 * The actions to be performed in GPIO initialization are:
 *  - Enable the M4/ULP clock based on GPIO instance.
 *  - Select PAD selection for GPIO instance.
 *  - Enable PAD receiver for GPIO pin number, whether GPIO pin is selected as
 *    output/input.
 *  - Set pin mode and direction of the GPIO pin.
 ******************************************************************************/
sl_status_t sl_gpio_driver_configure_interrupt(sl_gpio_t *gpio,
                                               uint32_t int_no,
                                               sl_gpio_interrupt_flag_t flags,
                                               sl_gpio_irq_callback_t gpio_callback,
                                               uint32_t *avl_intr_no)
{
  sl_status_t status;
  (void)avl_intr_no;
  // Check if gpio pointer and gpio_callback pointer is NULL
  if ((gpio == NULL) || (gpio_callback == NULL)) {
    // Return error code for NULL pointer
    return SL_STATUS_NULL_POINTER;
  }
  // Check if flags exceed maximum allowed value
  if (flags > GPIO_FLAGS_MAX_VALUE) {
    // Return error code for invalid parameter
    return SL_STATUS_INVALID_PARAMETER;
  }
  status = sl_gpio_validation(gpio);
  if (status != SL_STATUS_OK) {
    return status;
  }
  switch (gpio->port) {
    case SL_GPIO_PORT_A:
    case SL_GPIO_PORT_B:
    case SL_GPIO_PORT_C:
    case SL_GPIO_PORT_D:
      // Check if the interrupt number exceeds the maximum allowed value.
      if (int_no > GPIO_MAX_INTR_VALUE) {
        return SL_STATUS_INVALID_PARAMETER;
      }
      // Check if a callback function is already registered for the given interrupt number.
      if (gpio_callback_function_pointer[int_no] != NULL) {
        return SL_STATUS_BUSY;
      }
      // Enable the NVIC for the GPIO interrupt and set its priority.
      NVIC_EnableIRQ(intr_config.gpio_nvic[int_no]);
      NVIC_SetPriority(intr_config.gpio_nvic[int_no], intr_config.gpio_priority[int_no]);
      // Assign the callback function pointer for the specified interrupt number.
      gpio_callback_function_pointer[int_no] = gpio_callback;
      // Configure the GPIO interrupt.
      sl_gpio_configure_interrupt(gpio->port, gpio->pin, int_no, flags);
      break;
    default:
      break;
  }
  // Check if the GPIO port is the Ultra-Low Power GPIO port.
  if (gpio->port == SL_GPIO_ULP_PORT) {
    // Check if the GPIO pin or interrupt number exceeds the maximum allowed values.
    if (int_no > GPIO_ULP_INTERRUPT_MAX_VALUE) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    // Check if a callback function is already registered for the given Ultra-Low Power GPIO interrupt number.
    if (gpio_ulp_pin_int_callback_fptr[int_no] != NULL) {
      return SL_STATUS_BUSY;
    }
    // Enable the NVIC for the Ultra-Low Power GPIO pin interrupt and set its priority.
    NVIC_EnableIRQ(ULP_PININT0_NVIC_NAME);
    NVIC_SetPriority(ULP_PININT0_NVIC_NAME, ULP_GPIO_INTERRUPT_PRIORITY);
    // Assign the callback function pointer for the specified Ultra-Low Power GPIO interrupt number.
    gpio_ulp_pin_int_callback_fptr[int_no] = gpio_callback;
    // Configure the Ultra-Low Power GPIO pin interrupt.
    sl_si91x_gpio_configure_ulp_pin_interrupt((uint8_t)int_no, (sl_si91x_gpio_interrupt_config_flag_t)flags, gpio->pin);
  }
  // Check if the GPIO port is the Ultra-Ultra Low Power GPIO port.
  if (gpio->port == SL_GPIO_UULP_PORT) {
    // Check if the GPIO pin or interrupt number exceeds the maximum allowed values.
    if (int_no > GPIO_UULP_PIN_MAX_VALUE) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    // Check if a callback function is already registered for the given Ultra-Ultra Low Power GPIO interrupt number.
    if (gpio_uulp_pin_int_callback_fptr[int_no] != NULL) {
      return SL_STATUS_BUSY;
    }
    // Assign the callback function pointer for the specified Ultra-Ultra Low Power GPIO interrupt number.
    gpio_uulp_pin_int_callback_fptr[int_no] = gpio_callback;
    // Configure the Ultra-Ultra Low Power GPIO interrupt.
    sl_si91x_gpio_configure_uulp_interrupt((sl_si91x_gpio_interrupt_config_flag_t)flags, (uint8_t)int_no);
  }
  return SL_STATUS_OK;
}

/*******************************************************************************
 * @brief This API is used to configure the group interrupts in 3 instance.
 * To configure the interrupt, first GPIO initialization must be done.
 * The actions to be performed in GPIO initialization are:
 *  - Enable the M4/ULP clock based on GPIO instance.
 *  - Select PAD selection for GPIO instance.
 *  - Enable PAD receiver for GPIO pin number, whether GPIO pin is selected as
 *    output/input.
 *  - Set pin mode and direction of the GPIO pin.
 ******************************************************************************/
sl_status_t sl_gpio_configure_group_interrupt(sl_si91x_gpio_group_interrupt_config_t *configuration,
                                              sl_gpio_irq_callback_t gpio_callback)
{
  // Check if configuration pointer and gpio_callback pointer is NULL.
  if ((configuration == NULL) || (gpio_callback == NULL)) {
    // Return error code for NULL pointer
    return SL_STATUS_NULL_POINTER;
  }
  for (uint8_t i = 0; i < configuration->grp_interrupt_cnt; i++) {
    switch (configuration->grp_interrupt_port[i]) {
      case SL_GPIO_PORT_A:
      case SL_GPIO_PORT_B:
      case SL_GPIO_PORT_C:
      case SL_GPIO_PORT_D:
        // Check if a callback function is already registered for the given GPIO interrupt number.
        if (gpio_group_int_callback_fptr[configuration->grp_interrupt] != NULL) {
          return SL_STATUS_BUSY;
        }
        // Assign the callback function pointer for the specified GPIO interrupt number.
        gpio_group_int_callback_fptr[configuration->grp_interrupt] = gpio_callback;
        // Configure the GPIO group interrupt.
        sl_si91x_gpio_configure_group_interrupt(configuration);
        break;
      default:
        break;
    }
    if (configuration->grp_interrupt_port[i] == SL_GPIO_ULP_PORT) {
      // Check if a callback function is already registered for the given GPIO interrupt number.
      if (gpio_ulp_group_int_callback_fptr[configuration->grp_interrupt] != NULL) {
        return SL_STATUS_BUSY;
      }
      // Assign the callback function pointer for the specified ULP GPIO interrupt number.
      gpio_ulp_group_int_callback_fptr[configuration->grp_interrupt] = gpio_callback;
      // Configure the GPIO ULP group interrupt.
      sl_si91x_gpio_configure_ulp_group_interrupt(configuration);
    }
  }
  return SL_STATUS_OK;
}

/*******************************************************************************
 * @brief  This API is used for GPIO HP, ULP instances to set pin mode.
 * - If GPIO HP instance is considered, the following actions are performed:
 *   - To set the pin mode in GPIO HP instance, GPIO initialization needs to
 *     be done first.
 *   - The actions to be performed in GPIO initialization are:
 *      - Enable the M4 clock of GPIO HP instance.
 *      - Select PAD selection of the GPIO HP instance.
 *      - Enable PAD receiver for GPIO pin number, whether GPIO pin is
 *        selected as output/input.
 *  @note: Select HP GPIO pins for HP instances(6 to 57). Do not use
 *      GPIO pin number(0 to 5) in HP instance as these are used for other
 *      functionality.
 * - If GPIO ULP instance is considered, the following actions are performed:
 *   - To set the pin mode in GPIO ULP instance, GPIO initialization needs to
 *     be done first.
 *   - The actions to be performed in GPIO initialization are:
 *      - Enable the ULP clock of GPIO ULP instance.
 *      - Enable ULP PAD receiver for GPIO pin number, whether GPIO pin is
 *        selected as output/input.
 *  @note: Select ULP GPIO pins for ULP instances(0 to 11).
 ******************************************************************************/
sl_status_t sl_gpio_driver_set_pin_mode(sl_gpio_t *gpio, sl_gpio_mode_t mode, uint32_t output_value)
{
  sl_status_t status;
  // Check if the GPIO pointer is NULL.
  if (gpio == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  status = sl_gpio_validation(gpio);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Check if the mode or output value exceeds the maximum allowed values.
  if ((mode > MAX_MODE) || (output_value > GPIO_MAX_OUTPUT_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Set the mode for the GPIO pin.
  sl_gpio_set_pin_mode(gpio->port, gpio->pin, mode, output_value);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * @brief  This API is used for GPIO HP, ULP instances to get pin mode.
 * - If GPIO HP instance is considered, the following actions are performed:
 *   - To get the pin status in GPIO HP instance, GPIO initialization needs
 *     to be done first.
 *   - The actions to be performed in GPIO initialization are:
 *      - Enable the M4 clock of GPIO HP instance.
 *      - Select PAD selection of the GPIO HP instance.
 *      - Enable PAD receiver for GPIO pin number, whether GPIO pin is
 *        selected as output/input.
 *      - Set pin mode and direction of the GPIO pin.
 *      - Get the pin mode of GPIO pin.
 *  @note: Select HP GPIO pins for HP instances(6 to 57). Do not
 *         use GPIO pin number(0 to 5) in HP instance as these are used for other
 *         functionality.
 * - If GPIO ULP instance is considered, the following actions are
 *   performed:
 *   - To get the pin mode in GPIO ULP instance, GPIO initialization needs
 *     to be done first.
 *   - The actions to be performed in GPIO initialization are:
 *      - Enable the ULP clock of GPIO ULP instance.
 *      - Enable ULP PAD receiver for GPIO pin number, whether GPIO pin is
 *        selected as output/input.
 *      - Set pin mode and direction of the GPIO pin.
 *      - Get the pin mode of GPIO pin.
 *  @note: Select ULP GPIO pins for ULP instances(ULP_GPIO_0 to
 *     ULP_GPIO_11).
 ******************************************************************************/
sl_status_t sl_gpio_driver_get_pin_mode(sl_gpio_t *gpio, sl_gpio_mode_t *mode)
{
  sl_status_t status;
  // Check if the GPIO pointer and mode pointer is NULL.
  if ((gpio == NULL) || (mode == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }
  status = sl_gpio_validation(gpio);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Get the mode for the GPIO pin.
  *mode = sl_gpio_get_pin_mode(gpio->port, gpio->pin);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * Initialization of GPIO driver.
 ******************************************************************************/
sl_status_t sl_gpio_driver_init(void)
{
  uint32_t flag;
  sl_status_t status;
  GPIO_NPSS_GPIO_CONFIG_REG = CLR; //By default making all the interrupts zero.
  for (flag = 0; flag < GPIO_MAX_INTR_VALUE; flag++) {
    sl_gpio_driver_clear_interrupts(flag);
    gpio_callback_function_pointer[flag] = NULL;
  }
  for (flag = 0; flag < MAX_GROUP_INT; flag++) {
    sl_si91x_gpio_driver_clear_group_interrupt(flag);
    gpio_group_int_callback_fptr[flag] = NULL;
  }
  for (flag = 0; flag < MAX_UULP_INT; flag++) {
    sl_si91x_gpio_driver_clear_uulp_npss_interrupt((uint8_t)flag);
    gpio_uulp_pin_int_callback_fptr[flag] = NULL;
  }
  for (flag = 0; flag < GPIO_MAX_INTR_VALUE; flag++) {
    sl_si91x_gpio_driver_clear_ulp_interrupt((uint8_t)flag);
    gpio_ulp_pin_int_callback_fptr[flag] = NULL;
  }
  for (flag = 0; flag < MAX_GROUP_INT; flag++) {
    sl_si91x_gpio_driver_clear_ulp_group_interrupt(flag);
    gpio_ulp_group_int_callback_fptr[flag] = NULL;
  }
  status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)M4CLK_GPIO); // Enable GPIO M4_CLK
  if (status != SL_STATUS_OK) {
    return status;
  }
  status = sl_si91x_gpio_driver_enable_clock((sl_si91x_gpio_select_clock_t)ULPCLK_GPIO); // Enable GPIO ULP_CLK
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Clearing the soc peripheral on ulp gpio status flag
  soc_peri_on_ulp_gpio_status = 0;
  return SL_STATUS_OK;
}

/*******************************************************************************
* Get the release, SQA, and development version numbers of the GPIO peripheral.
*******************************************************************************/
sl_si91x_gpio_version_t sl_si91x_gpio_driver_get_version(void)
{
  sl_si91x_gpio_version_t version;
  version = sl_si91x_gpio_get_version();
  return version;
}

/*******************************************************************************
 * Indicate UULP GPIO PAD configuration.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_set_uulp_pad_configuration(uulp_pad_config_t *pad_config)
{
  // Check if the pad configuration pointer is NULL.
  if (pad_config == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Set the UULP pad configuration.
  sl_si91x_gpio_set_uulp_pad_configuration(pad_config);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used for GPIO HP, ULP instances to set pin direction.
 * - If GPIO HP instance is considered, the following actions are performed:
 *   - To set the pin direction in GPIO HP instance, GPIO initialization
 *    needs to be done first.
 *   - The actions to be performed in GPIO initialization are:
 *      - Enable the M4 clock of GPIO HP instance.
 *      - Select PAD selection of the GPIO HP instance.
 *      - Enable PAD receiver for GPIO pin number, whether GPIO pin is
 *      selected as output/input.
 *      - Set pin mode and direction of the GPIO pin.
 *  @note: Select HP GPIO pins for HP instances(6 to 57). Do not
 *  use GPIO pin number(0 to 5) in HP instance as these are used for other
 *  functionality.
 * - If GPIO ULP instance is considered, the following actions are
 *    performed:
 *   - To set the pin direction in GPIO ULP instance, GPIO initialization
 *    needs to be done first.
 *   - The actions to be performed in GPIO initialization are:
 *      - Enable the ULP clock of GPIO ULP instance.
 *      - Enable ULP PAD receiver for GPIO pin number, whether GPIO pin is
 *      selected as output/input.
 *      - Set pin mode and direction of the GPIO pin.
 *  @note: Select ULP GPIO pins for ULP instances(0 to 11).
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_set_pin_direction(uint8_t port, uint8_t pin, sl_si91x_gpio_direction_t direction)
{
  sl_status_t status;
  sl_gpio_t gpio = { (sl_gpio_port_t)port, pin };
  status         = sl_gpio_validation(&gpio);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Check if the GPIO port and direction exceeds the maximum allowed value. Returns an invalid parameter if true
  if ((port > GPIO_PORT_MAX_VALUE) || (direction > GPIO_DIRECTION_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Set GPIO pin direction
  sl_si91x_gpio_set_pin_direction(port, pin, direction);
  status = SL_STATUS_OK;
  return status;
}

/*******************************************************************************
 * This API is used for GPIO HP, ULP instances to get pin direction.
 * - If GPIO HP instance is considered, the following actions are performed:
 *   - To get the pin direction in GPIO HP instance, GPIO initialization
 *      needs to be done first.
 *   - The actions to be performed in GPIO initialization are:
 *      - Enable the M4 clock of GPIO HP instance.
 *      - Select PAD selection of the GPIO HP instance.
 *      - Enable PAD receiver for GPIO pin number, whether GPIO pin is
 *      selected as output/input.
 *      - Set pin mode and direction of the GPIO pin.
 *      - Get the pin direction of the GPIO pin.
 *  @note: Select HP GPIO pins for HP instances(6 to 57). Do not
 *    use GPIO pin number(0 to 5) in HP instance as these are used for other
 *    functionality.
 * - If GPIO ULP instance is considered, the following actions are
 *    performed:
 *   - To get the pin direction in GPIO ULP instance, GPIO initialization
 *    needs to be done first.
 *   - The actions to be performed in GPIO initialization are:
 *      - Enable the ULP clock of GPIO ULP instance.
 *      - Enable ULP PAD receiver for GPIO pin number, whether GPIO pin is
 *      selected as output/input.
 *      - Set pin mode and direction of the GPIO pin.
 *      - Get the pin direction of the GPIO pin.
 *  @note: Select ULP GPIO pins for ULP instances(0 to 11).
 ******************************************************************************/
uint8_t sl_si91x_gpio_driver_get_pin_direction(uint8_t port, uint8_t pin)
{
  uint8_t direction;
  sl_status_t status;
  sl_gpio_t gpio = { (sl_gpio_port_t)port, pin };
  status         = sl_gpio_validation(&gpio);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Check if the GPIO port exceeds the maximum allowed value. Returns an invalid parameter if true
  if (port > GPIO_PORT_MAX_VALUE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Get pin direction for GPIO pin
  direction = sl_si91x_gpio_get_pin_direction(port, pin);
  return direction;
}

/*******************************************************************************
 * This API is used to enable PAD receiver in GPIO HP instance.
 * The actions to be performed for enabling PAD are:
 *   - Enable the M4 clock of GPIO HP instance.
 *   - Select PAD selection of the GPIO HP instance.
 *   - Enable PAD receiver for GPIO pin number, whether GPIO pin is
 *    selected as output/input.
 *  @note: Select HP GPIO pins for HP instances(6 to 57). Do not
 *  use GPIO pin number(0 to 5) in HP instance as these are used for other
 *  functionality.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_enable_pad_receiver(uint8_t gpio_num)
{
  // Check if the GPIO pin number exceeds the maximum allowed value.
  if ((gpio_num < GPIO_PA_PIN_0_5_VALIDATE) || (gpio_num > GPIO_MAX_PIN_NUM)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Enable the pad receiver for the GPIO pin.
  sl_si91x_gpio_enable_pad_receiver(gpio_num);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to disable PAD receiver in GPIO HP instance.
 * The actions to be performed for disabling PAD are:
 *   - Enable the M4 clock of GPIO HP instance.
 *   - Select PAD selection of the GPIO HP instance.
 *   - Disable PAD receiver for GPIO pin number, whether GPIO pin is
 *    selected as output/input.
 *  @note: Select HP GPIO pins for HP instances(6 to 57). Do not
 *  use GPIO pin number(0 to 5) in HP instance as these are used for other
 *  functionality.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_disable_pad_receiver(uint8_t gpio_num)
{
  // Check if the GPIO pin number exceeds the maximum allowed value.
  if ((gpio_num < GPIO_PA_PIN_0_5_VALIDATE) || (gpio_num > GPIO_MAX_PIN_NUM)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Disable the pad receiver for the GPIO pin.
  sl_si91x_gpio_disable_pad_receiver(gpio_num);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to enable PAD selection in GPIO HP instance.
 * The actions to be performed for enable PAD selection are:
 *   - Enable the M4 clock of GPIO HP instance.
 *   - Select PAD selection of the GPIO HP instance.
 *  @note: PAD number(25 to 30) are used for HOST PAD selection.
 *      Do not use PAD number-9 as it is used for other functionality.
 *  @note: Select HP GPIO pins for HP instances(GPIO_6 to GPIO_57). Do not
 *  use GPIO pin number(0 to 5) in HP instance as these are used for other
 *  functionality.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_enable_pad_selection(uint8_t gpio_padnum)
{
  // Check if the GPIO pad number exceeds the maximum allowed value.
  if (gpio_padnum > GPIO_MAX_PAD_NUM) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Enable pad selection for the GPIO pad.
  sl_si91x_gpio_enable_pad_selection(gpio_padnum);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to enable HOST PAD selection in GPIO HP instance.
 *  @note: GPIO pin number(25 to 30) are used for HOST PAD selection.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_enable_host_pad_selection(uint8_t gpio_num)
{
  // Check if the GPIO pin number exceeds the maximum allowed value.
  if (!(gpio_num >= HOST_PAD_MIN && gpio_num <= HOST_PAD_MAX)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Enable host pad selection for the GPIO pin.
  sl_si91x_gpio_enable_host_pad_selection(gpio_num);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * To select the PAD driver strength in GPIO HP instance, GPIO
 *initialization needs to be done first.
 * - The actions to be performed in GPIO initialization are:
 *   - Enable the M4 clock of GPIO HP instance.
 *   - Select PAD selection of the GPIO HP instance.
 *   - Enable PAD receiver for GPIO pin number, whether GPIO pin is
 *    selected as output/input.
 *   - Set pin mode and direction of the GPIO pin.
 *   - Select the PAD driver strength of type @ref
 *    sl_si91x_gpio_driver_strength_select_t.
 *  @note: Select HP GPIO pins for HP instances(6 to 57). Do not
 *  use GPIO pin number(0 to 5) in HP instance as these are used for other
 *  functionality.
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_select_pad_driver_strength(uint8_t gpio_num,
                                                            sl_si91x_gpio_driver_strength_select_t strength)
{
  // Check if GPIO pin number or strength value exceeds the maximum allowed.
  if ((gpio_num < GPIO_PA_PIN_0_5_VALIDATE) || (gpio_num > GPIO_MAX_PIN_NUM) || (strength > GPIO_STRENGTH_MAX_VAL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Select the pad driver strength for the GPIO pin.
  sl_si91x_gpio_select_pad_driver_strength(gpio_num, strength);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * To enable power-on start for PAD in GPIO HP instance, GPIO
 * initialization needs to be done first.
 * - The actions to be performed in GPIO initialization are:
 *   - Enable the M4 clock of GPIO HP instance.
 *   - Select PAD selection of the GPIO HP instance.
 *   - Enable PAD receiver for GPIO pin number, whether GPIO pin is
 *     selected as output/input.
 *   - Set pin mode and direction of the GPIO pin.
 *   - Enable power-on start using @ref sl_si91x_gpio_pos_t.
 * @note: Select HP GPIO pins for HP instances (6 to 57). Do not use
 * GPIO pin numbers (0 to 5) in HP instance as these are reserved.
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_enable_pad_power_on_start(uint8_t gpio_num, sl_si91x_gpio_pos_t pos)
{
  // Validate GPIO number and position parameter
  if ((gpio_num < GPIO_PA_PIN_0_5_VALIDATE) || (gpio_num > GPIO_MAX_PIN_NUM) || (pos >= GPIO_POS_LAST)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Enable power-on start for HP GPIO
  sl_si91x_gpio_select_pad_power_on_start(gpio_num, pos);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * To select the PAD schmitt trigger in GPIO HP instance, GPIO
 * initialization needs to be done first.
 * - The actions to be performed in GPIO initialization are:
 *   - Enable the M4 clock of GPIO HP instance.
 *   - Select PAD selection of the GPIO HP instance.
 *   - Enable PAD receiver for GPIO pin number, whether GPIO pin is
 *     selected as output/input.
 *   - Set pin mode and direction of the GPIO pin.
 *   - Select the PAD schmitt trigger of type @ref sl_si91x_gpio_schmitt_trig_t.
 * @note: Select HP GPIO pins for HP instances (6 to 57). Do not use
 * GPIO pin numbers (0 to 5) in HP instance as these are reserved.
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_select_pad_schmitt_trigger(uint8_t gpio_num, sl_si91x_gpio_schmitt_trig_t schmitt_trig)
{
  // Validate GPIO number and schmitt trigger parameter
  if ((gpio_num < GPIO_PA_PIN_0_5_VALIDATE) || (gpio_num > GPIO_MAX_PIN_NUM)
      || (schmitt_trig >= GPIO_SCHMITT_TRIG_LAST)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Select schmitt trigger for HP GPIO
  sl_si91x_gpio_select_pad_active_high_schmitt_trigger(gpio_num, schmitt_trig);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * To select the PAD driver disable state in GPIO HP instance, GPIO
 *initialization needs to be done first.
 * - The actions to be performed in GPIO initialization are:
 *   - Enable the M4 clock of GPIO HP instance.
 *   - Select PAD selection of the GPIO HP instance.
 *   - Enable PAD receiver for GPIO pin number, whether GPIO pin is
 *    selected as output/input.
 *   - Set pin mode and direction of the GPIO pin.
 *   - Select the PAD driver disable state of type @ref
 *    sl_si91x_gpio_driver_disable_state_t.
 *  @note: Select HP GPIO pins for HP instances(6 to 57). Do not
 *  use GPIO pin number(0 to 5) in HP instance as these are used for other
 *  functionality.
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_select_pad_driver_disable_state(uint8_t gpio_num,
                                                                 sl_si91x_gpio_driver_disable_state_t disable_state)
{
  // Check if GPIO pin number or disable state value exceeds the maximum allowed.
  if ((gpio_num < GPIO_PA_PIN_0_5_VALIDATE) || (gpio_num > GPIO_MAX_PIN_NUM)
      || (disable_state > GPIO_DISABLE_STATE_MAX_VAL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Select the pad driver disable state for the GPIO pin.
  sl_si91x_gpio_select_pad_driver_disable_state(gpio_num, disable_state);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * The GPIO pins to work in different instances, requires this clock.
 * For GPIO HP instance, enable M4 clock of type @ref
 * sl_si91x_gpio_select_clock_t. For GPIO ULP/UULP instances, enable ULP
 * clock of type
 *  @ref sl_si91x_gpio_select_clock_t
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_enable_clock(sl_si91x_gpio_select_clock_t clock)
{
  // Check if clock value exceeds the maximum allowed.
  if (clock > GPIO_CLOCK_MAX_VAL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Enable the clock.
  sl_si91x_gpio_enable_clock(clock);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API disables the M4/ ULP clock of GPIO instances.
 * For GPIO HP instance, disable M4 clock of type @ref
 * sl_si91x_gpio_select_clock_t. For GPIO ULP/UULP instances, disable ULP
 * clock of type
 *  @ref sl_si91x_gpio_select_clock_t
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_disable_clock(sl_si91x_gpio_select_clock_t clock)
{
  // Check if clock value exceeds the maximum allowed.
  if (clock > GPIO_CLOCK_MAX_VAL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Disable the clock.
  sl_si91x_gpio_disable_clock(clock);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used for GPIO HP, ULP instances.
 * It is used to enable the group interrupts.
 * @note: We are calling this API, inside the group interrupt
 * configuration API's
 *      @ref sl_si91x_gpio_driver_configure_group_interrupt(), used for HP
 *    instance,
 *      @ref sl_si91x_gpio_driver_configure_ulp_group_interrupt(), used for ULP
 *    instance.
 * @note: Select HP GPIO pins for HP instances(6 to 57). Do not
 * use GPIO pin number(0 to 5) in HP instance as these are used for other
 * functionality.
 * @note: Select ULP GPIO pins for ULP instances(0 to 11).
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_enable_group_interrupt(sl_si91x_group_interrupt_t group_interrupt,
                                                        uint8_t port,
                                                        uint8_t pin)
{
  sl_status_t status;
  sl_gpio_t gpio = { (sl_gpio_port_t)port, pin };
  status         = sl_gpio_validation(&gpio);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Check if group interrupt or port value exceeds the maximum allowed.
  if ((group_interrupt > GPIO_GROUP_INTERRUPT_MAX_VALUE) || (port > GPIO_PORT_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Enable GPIO group interrupt for selected GPIO port pin
  sl_si91x_gpio_enable_group_interrupt(group_interrupt, port, pin);
  status = SL_STATUS_OK;
  return status;
}

/*******************************************************************************
 * This API is used for GPIO HP, ULP instances.
 * It is used to disable the group interrupts.
 * @note: Select HP GPIO pins for HP instances(6 to 57). Do not
 * use GPIO pin number(0 to 5) in HP instance as these are used for other
 * functionality.
 * @note: Select ULP GPIO pins for ULP instances(0 to 11).
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_disable_group_interrupt(sl_si91x_group_interrupt_t group_interrupt,
                                                         uint8_t port,
                                                         uint8_t pin)
{
  sl_status_t status;
  sl_gpio_t gpio = { (sl_gpio_port_t)port, pin };
  status         = sl_gpio_validation(&gpio);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Check if group interrupt or port value exceeds the maximum allowed.
  if ((group_interrupt > GPIO_GROUP_INTERRUPT_MAX_VALUE) || (port > GPIO_PORT_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Disable GPIO group interrupt for selected GPIO port pin
  sl_si91x_gpio_disable_group_interrupt(group_interrupt, port, pin);
  status = SL_STATUS_OK;
  return status;
}

/*******************************************************************************
 * This API is used for GPIO HP, ULP instances.
 * It is used to mask the group interrupts.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_mask_group_interrupt(uint8_t port, sl_si91x_group_interrupt_t group_interrupt)
{
  // Check if group interrupt or port value exceeds the maximum allowed.
  if ((port > GPIO_PORT_MAX_VALUE) || (group_interrupt > GPIO_GROUP_INTERRUPT_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Mask group interrupt for port
  sl_si91x_gpio_mask_group_interrupt(port, group_interrupt);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used for GPIO HP, ULP instances.
 * It is used to unmask the group interrupts.
 * @note: We are calling this API, inside the group interrupt
 * configuration API's
 *      @ref sl_si91x_gpio_driver_configure_group_interrupt(), used for HP
 *    instance,
 *      @ref sl_si91x_gpio_driver_configure_ulp_group_interrupt(), used for ULP
 *    instance.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_unmask_group_interrupt(uint8_t port, sl_si91x_group_interrupt_t group_interrupt)
{
  // Check if group interrupt or port value exceeds the maximum allowed.
  if ((port > GPIO_PORT_MAX_VALUE) || (group_interrupt > GPIO_GROUP_INTERRUPT_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Unmask group interrupt for port
  sl_si91x_gpio_unmask_group_interrupt(port, group_interrupt);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used for GPIO HP, ULP instances.
 * It is used to set level/edge event of group interrupt.
 * @note: We are calling this API, inside the group interrupt
 * configuration API's
 *      @ref sl_si91x_gpio_driver_configure_group_interrupt(), used for HP
 *    instance,
 *      @ref sl_si91x_gpio_driver_configure_ulp_group_interrupt(), used for ULP
 *    instance.
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_set_group_interrupt_level_edge(uint8_t port,
                                                                sl_si91x_group_interrupt_t group_interrupt,
                                                                sl_si91x_gpio_level_edge_t level_edge)
{
  // Check if group interrupt or port value or level edge exceeds the maximum allowed.
  if ((port > GPIO_PORT_MAX_VALUE) || (group_interrupt > GPIO_GROUP_INTERRUPT_MAX_VALUE)
      || (level_edge > GPIO_LEVEL_EDGE_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Set group interrupt level/edge event
  sl_si91x_gpio_set_group_interrupt_level_edge(port, group_interrupt, level_edge);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used for GPIO HP, ULP instances.
 * It is used to get level/edge event of group interrupt.
 ******************************************************************************/
uint8_t sl_si91x_gpio_driver_get_group_interrupt_level_edge(uint8_t port, sl_si91x_group_interrupt_t group_interrupt)
{
  // Check if group interrupt or port value exceeds the maximum allowed.
  uint8_t level_edge;
  if ((port > GPIO_PORT_MAX_VALUE) || (group_interrupt > GPIO_GROUP_INTERRUPT_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Get group interrupt level/edge event
  level_edge = sl_si91x_gpio_get_group_interrupt_level_edge(port, group_interrupt);
  return level_edge;
}

/*******************************************************************************
 * This API is used for GPIO HP, ULP instances.
 * It is used to set polarity of group interrupt.
 * @note: We are calling this API, inside the group interrupt
 * configuration API's
 *      @ref sl_si91x_gpio_driver_configure_group_interrupt(), used for HP
 *    instance,
 *      @ref sl_si91x_gpio_driver_configure_ulp_group_interrupt(), used for ULP
 *    instance.
 * @note: Select HP GPIO pins for HP instances(6 to 57). Do not
 * use GPIO pin number(0 to 5) in HP instance as these are used for other
 * functionality.
 * @note: Select ULP GPIO pins for ULP instances(0 to 11).
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_set_group_interrupt_polarity(sl_si91x_group_interrupt_t group_interrupt,
                                                              uint8_t port,
                                                              uint8_t pin,
                                                              sl_si91x_gpio_polarity_t polarity)
{
  sl_status_t status;
  sl_gpio_t gpio = { (sl_gpio_port_t)port, pin };
  status         = sl_gpio_validation(&gpio);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Check if group interrupt or port value or polarity exceeds the maximum allowed.
  if ((group_interrupt > GPIO_GROUP_INTERRUPT_MAX_VALUE) || (polarity > GPIO_POLARITY_MAX_VALUE)
      || (port > GPIO_PORT_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Set group interrupt polarity for GPIO port pin
  sl_si91x_gpio_set_group_interrupt_polarity(group_interrupt, port, pin, polarity);
  status = SL_STATUS_OK;
  return status;
}

/*******************************************************************************
 * This API is used for GPIO HP, ULP instances.
 * It is used to get polarity of group interrupt .
 * @note: Select HP GPIO pins for HP instances(6 to 57). Do not use
 *      GPIO pin number(0 to 5) in HP instance as these are used for other functionality.
 * @note: Select ULP GPIO pins for ULP instances(0 to 11).
 ******************************************************************************/
uint8_t sl_si91x_gpio_driver_get_group_interrupt_polarity(sl_si91x_group_interrupt_t group_interrupt,
                                                          uint8_t port,
                                                          uint8_t pin)
{
  uint8_t polarity = 0;
  sl_status_t status;
  sl_gpio_t gpio = { (sl_gpio_port_t)port, pin };
  status         = sl_gpio_validation(&gpio);
  if (status != SL_STATUS_OK) {
    return status;
  }
  // Check if group interrupt or port value exceeds the maximum allowed.
  if ((group_interrupt > GPIO_GROUP_INTERRUPT_MAX_VALUE) || (port > GPIO_PORT_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Get group interrupt polarity for GPIO port pin
  polarity = sl_si91x_gpio_get_group_interrupt_polarity(group_interrupt, port, pin);
  return polarity;
}

/*******************************************************************************
 * This API is used for GPIO HP, ULP instances.
 * It is used to select and/or event of group interrupt.
 * @example: Consider two GPIO pins for group interrupts.
 *    - If AND event is selected then both GPIO pins, interrupt should be
 *        generated to do specific task.
 *    - If OR event is selected then any one GPIO pin, interrupt
 *      generation should be enough to do specific task.
 * @note: We are calling this API, inside the group interrupt
 * configuration API's
 *      @ref sl_si91x_gpio_driver_configure_group_interrupt(), used for HP
 *    instance,
 *      @ref sl_si91x_gpio_driver_configure_ulp_group_interrupt(), used for ULP
 *    instance.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_select_group_interrupt_and_or(uint8_t port,
                                                               sl_si91x_group_interrupt_t group_interrupt,
                                                               sl_si91x_gpio_and_or_t and_or)
{
  // Check if group interrupt or port value or and_or exceeds the maximum allowed.
  if ((port > GPIO_PORT_MAX_VALUE) || (group_interrupt > GPIO_GROUP_INTERRUPT_MAX_VALUE)
      || (and_or > GPIO_AND_OR_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Select group interrupt AND/OR event
  sl_si91x_gpio_select_group_interrupt_and_or(port, group_interrupt, and_or);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used in GPIO HP instance to configure group interrupts.
 * It has configuration pointer of type @ref
 * sl_si91x_gpio_group_interrupt_config_t structure. GPIO HP instance has
 * total 4 ports. Port-A, B, C has 16 pins each. Port-D has 9 pins. While
 * configuring group interrupts, one can select random ports and pins for
 *  group interrupt.
 * @example 1:
 * - If port 1 is selected, any group of pins(0 to 16) can be selected for
 * group interrupt.
 * - Same applied for other ports also.
 * @example 2:
 * - Once can select port B, pin 7 and port C, pin 3 as a group for
 * interrupt generation.
 * - One should assign group count of how many pins are passed.
 * For more clarification look into group interrupt configuration
 * structure
 *       @ref sl_si91x_gpio_driver_group_interrupt_config_t.
 *  @note: Do not use Port A, GPIO pin number(0 to 5) in HP instance
 *     as these are used for other functionality.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_configure_group_interrupt(sl_si91x_gpio_group_interrupt_config_t *configuration,
                                                           sl_gpio_irq_callback_t gpio_callback)
{
  // Check if configuration pointer and gpio_callback pointer is NULL.
  if ((configuration == NULL) || (gpio_callback == NULL)) {
    // Return error code for NULL pointer
    return SL_STATUS_NULL_POINTER;
  }
  // Check if a callback function is already registered for the given GPIO interrupt number.
  if (gpio_group_int_callback_fptr[configuration->grp_interrupt] != NULL) {
    return SL_STATUS_BUSY;
  }
  // Assign the callback function pointer for the specified GPIO interrupt number.
  gpio_group_int_callback_fptr[configuration->grp_interrupt] = gpio_callback;
  // Configure the GPIO group interrupt.
  sl_si91x_gpio_configure_group_interrupt(configuration);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to configure the pin interrupt in GPIO ULP instance.
 * There are total 12 pin interrupts in this instance.
 * To configure the interrupt, first ULP GPIO initialization must be done.
 * The actions to be performed in ULP GPIO initialization are:
 *  - Enable the ULP clock of GPIO ULP instance.
 *  - Enable ULP PAD receiver for GPIO pin number, whether GPIO pin is
 *   selected as output/input.
 *  - Set pin mode and direction of the GPIO pin.
 * @note: Select ULP GPIO pins for ULP instances(0 to 11).
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_configure_ulp_pin_interrupt(uint8_t int_no,
                                                             sl_si91x_gpio_interrupt_config_flag_t flags,
                                                             sl_si91x_gpio_pin_ulp_t pin,
                                                             sl_gpio_irq_callback_t gpio_callback)
{
  // Check if gpio_callback pointer is NULL.
  if (gpio_callback == NULL) {
    // Return error code for NULL pointer
    return SL_STATUS_NULL_POINTER;
  }
  // Check if interrupt number or flags or pin exceeds the maximum allowed.
  if ((int_no > GPIO_ULP_INTERRUPT_MAX_VALUE) || (flags > GPIO_FLAGS_MAX_VALUE) || (pin > ULP_PIN_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Check if a callback function is already registered for the given GPIO interrupt number.
  if (gpio_ulp_pin_int_callback_fptr[int_no] != NULL) {
    return SL_STATUS_BUSY;
  }
  // Enable the NVIC for the GPIO interrupt and set its priority.
  NVIC_EnableIRQ(ULP_PININT0_NVIC_NAME);
  NVIC_SetPriority(ULP_PININT0_NVIC_NAME, ULP_GPIO_INTERRUPT_PRIORITY);
  // Assign the callback function pointer for the specified ULP GPIO interrupt number.
  gpio_ulp_pin_int_callback_fptr[int_no] = gpio_callback;
  // Configure the GPIO ULP pin interrupt.
  sl_si91x_gpio_configure_ulp_pin_interrupt(int_no, flags, pin);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used in GPIO ULP instance to configure group interrupts.
 * It has configuration pointer of type @ref
 * sl_si91x_gpio_group_interrupt_config_t structure. ULP GPIO domain has
 * only one port and calling as Port 4 in program which has maximum of 12
 * pins. While configuring group interrupts, one can select random pins
 * which are allocated for ULP port.
 * - One should assign group count of how many pins are passed.
 * For more clarification look into group interrupt configuration
 * structure
 *       @ref sl_si91x_gpio_group_interrupt_config_t.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_configure_ulp_group_interrupt(sl_si91x_gpio_group_interrupt_config_t *configuration,
                                                               sl_gpio_irq_callback_t gpio_callback)
{
  // Check if configuration pointer and gpio_callback pointer is NULL.
  if ((configuration == NULL) || (gpio_callback == NULL)) {
    // Return error code for NULL pointer
    return SL_STATUS_NULL_POINTER;
  }
  // Check if a callback function is already registered for the given GPIO interrupt number.
  if (gpio_ulp_group_int_callback_fptr[configuration->grp_interrupt] != NULL) {
    return SL_STATUS_BUSY;
  }
  // Assign the callback function pointer for the specified ULP GPIO interrupt number.
  gpio_ulp_group_int_callback_fptr[configuration->grp_interrupt] = gpio_callback;
  // Configure the GPIO ULP group interrupt.
  sl_si91x_gpio_configure_ulp_group_interrupt(configuration);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used for GPIO HP instance.
 * It is used to clear group interrupt.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_clear_group_interrupt(sl_si91x_group_interrupt_t group_interrupt)
{
  // Check if group_interrupt exceeds the maximum allowed.
  if (group_interrupt > GPIO_GROUP_INTERRUPT_MAX_VALUE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Clear GPIO group interrupt
  sl_si91x_gpio_clear_group_interrupt(group_interrupt);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used for GPIO HP, ULP instance.
 * It is used to get status of group interrupt generated.
 ******************************************************************************/
uint32_t sl_si91x_gpio_driver_get_group_interrupt_status(uint8_t port, sl_si91x_group_interrupt_t group_interrupt)
{
  uint32_t status;
  // Check if group_interrupt or port exceeds the maximum allowed.
  if ((port > GPIO_PORT_MAX_VALUE) || (group_interrupt > GPIO_GROUP_INTERRUPT_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Get GPIO group interrupt status
  status = sl_si91x_gpio_get_group_interrupt_status(port, group_interrupt);
  return status;
}

/*******************************************************************************
 * This API is used for GPIO HP, ULP instance.
 * It is used to select group interrupt wakeup.
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_select_group_interrupt_wakeup(uint8_t port,
                                                               sl_si91x_group_interrupt_t group_interrupt,
                                                               sl_si91x_gpio_wakeup_t flags)
{
  // Check if group_interrupt or port exceeds the maximum allowed.
  if ((port > GPIO_PORT_MAX_VALUE) || (group_interrupt > GPIO_GROUP_INTERRUPT_MAX_VALUE)
      || (flags > GPIO_FLAGS_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Select GPIO group interrupt wakeup
  sl_si91x_gpio_select_group_interrupt_wakeup(port, group_interrupt, flags);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * To enable ULP PAD receiver in GPIO ULP instance, ULP GPIO
 * initialization needs to be done first.
 *   - The actions to be performed in ULP GPIO initialization are:
 *      - Enable the ULP clock of GPIO ULP instance.
 *      - Enable ULP PAD receiver for GPIO pin number, whether GPIO pin is
 *  selected as output/input.
 *  @note: Select ULP GPIO pins for ULP instances(ULP_GPIO_0 to
 *  ULP_GPIO_11).
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_enable_ulp_pad_receiver(uint8_t gpio_num)
{
  // Check if gpio_num exceeds the maximum allowed
  if (gpio_num > GPIO_ULP_MAX_PIN_NUM) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Enable ULP GPIO pad receiver
  sl_si91x_gpio_enable_ulp_pad_receiver(gpio_num);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to disable the ULP PAD receiver.
 * @note: Select ULP GPIO pins for ULP instances(ULP_GPIO_0 to
 * ULP_GPIO_11).
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_disable_ulp_pad_receiver(uint8_t gpio_num)
{
  // Check if gpio_num exceeds the maximum allowed
  if (gpio_num > GPIO_ULP_MAX_PIN_NUM) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Disable ULP GPIO pad receiver
  sl_si91x_gpio_disable_ulp_pad_receiver(gpio_num);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * To select the ULP PAD driver disable state in GPIO ULP instance, ULP
 * GPIO initialization needs to be done first.
 * - The actions to be performed in ULP GPIO initialization are:
 *   - Enable the ULP clock of GPIO ULP instance.
 *   - Enable ULP PAD receiver for GPIO pin number, whether GPIO pin is
 *    selected as output/input.
 *   - Set pin mode and direction of the GPIO pin.
 *   - Select the PAD driver disable state of type @ref
 *    sl_si91x_gpio_driver_disable_state_t.
 *  @note: Select ULP GPIO pins for ULP instances(0 to 11).
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_select_ulp_pad_driver_disable_state(uint8_t gpio_num,
                                                                     sl_si91x_gpio_driver_disable_state_t disable_state)
{
  // Check if gpio_num or disable_state exceeds the maximum allowed
  if ((gpio_num > GPIO_ULP_MAX_PIN_NUM) || (disable_state > GPIO_DISABLE_STATE_MAX_VAL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Select ULP GPIO pad driver disable state
  sl_si91x_gpio_select_ulp_pad_driver_disable_state(gpio_num, disable_state);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * To select the ULP PAD driver strength in GPIO ULP instance, ULP GPIO
 *initialization needs to be done first.
 * - The actions to be performed in ULP GPIO initialization are:
 *   - Enable the ULP clock of GPIO ULP instance.
 *   - Enable ULP PAD receiver for GPIO pin number, whether GPIO pin is
 *    selected as output/input.
 *   - Set pin mode and direction of the GPIO pin.
 *   - Select the PAD driver strength of type @ref
 *    sl_si91x_gpio_driver_strength_select_t.
 *  @note: Select ULP GPIO pins for ULP instances(0 to 11).
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_select_ulp_pad_driver_strength(uint8_t gpio_num,
                                                                sl_si91x_gpio_driver_strength_select_t strength)
{
  // Check if gpio_num or strength exceeds the maximum allowed
  if ((gpio_num > GPIO_ULP_MAX_PIN_NUM) || (strength > GPIO_STRENGTH_MAX_VAL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Select ULP GPIO pad driver strength
  sl_si91x_gpio_select_ulp_pad_driver_strength(gpio_num, strength);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * To enable power-on-start for ULP PAD in GPIO ULP instance, ULP GPIO
 * initialization needs to be done first.
 * - The actions to be performed in ULP GPIO initialization are:
 *   - Enable the ULP clock of GPIO ULP instance.
 *   - Enable ULP PAD receiver for GPIO pin number, whether GPIO pin is
 *     selected as output/input.
 *   - Set pin mode and direction of the GPIO pin.
 *   - Enable power-on start using @ref sl_si91x_gpio_pos_t.
 * @note: Select ULP GPIO pins for ULP instances (ULP_GPIO_0 to ULP_GPIO_11).
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_enable_ulp_pad_power_on_start(uint8_t gpio_num, sl_si91x_gpio_pos_t pos)
{
  // Validate GPIO number and position parameter
  if ((gpio_num > GPIO_ULP_MAX_PIN_NUM) || (pos >= GPIO_POS_LAST)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Enable power-on start for ULP GPIO pad
  sl_si91x_gpio_select_ulp_pad_power_on_start(gpio_num, pos);

  return SL_STATUS_OK;
}

/*******************************************************************************
 * To select the ULP PAD schmitt trigger in GPIO ULP instance, ULP GPIO
 * initialization needs to be done first.
 * - The actions to be performed in ULP GPIO initialization are:
 *   - Enable the ULP clock of GPIO ULP instance.
 *   - Enable ULP PAD receiver for GPIO pin number, whether GPIO pin is
 *     selected as output/input.
 *   - Set pin mode and direction of the GPIO pin.
 *   - Select the PAD schmitt trigger of type @ref sl_si91x_gpio_schmitt_trig_t.
 * @note: Select ULP GPIO pins for ULP instances (ULP_GPIO_0 to ULP_GPIO_11).
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_select_ulp_pad_schmitt_trigger(uint8_t gpio_num,
                                                                sl_si91x_gpio_schmitt_trig_t schmitt_trig)
{
  // Validate GPIO number and schmitt trigger parameter
  if ((gpio_num > GPIO_ULP_MAX_PIN_NUM) || (schmitt_trig >= GPIO_SCHMITT_TRIG_LAST)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Select ULP GPIO pad schmitt trigger
  sl_si91x_gpio_select_ulp_pad_active_high_schmitt_trigger(gpio_num, schmitt_trig);

  return SL_STATUS_OK;
}

/*******************************************************************************
 * To select the ULP PAD slew rate in GPIO ULP instance, ULP GPIO
 * initialization needs to be done first.
 * - The actions to be performed in ULP GPIO initialization are:
 *   - Enable the ULP clock of GPIO ULP instance.
 *   - Enable ULP PAD receiver for GPIO pin number, whether GPIO pin is
 *    selected as output/input.
 *   - Set pin mode and direction of the GPIO pin.
 *   - Select the PAD slew rate of type @ref sl_si91x_gpio_slew_rate_t.
 *  @note: Select ULP GPIO pins for ULP instances(0 to 11).
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_select_ulp_pad_slew_rate(uint8_t gpio_num, sl_si91x_gpio_slew_rate_t slew_rate)
{
  // Check if gpio_num or slew_rate exceeds the maximum allowed
  if ((gpio_num > GPIO_ULP_MAX_PIN_NUM) || (slew_rate > GPIO_SLEW_RATE_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Select ULP GPIO pad slew rate
  sl_si91x_gpio_select_ulp_pad_slew_rate(gpio_num, slew_rate);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to select the UULP mode in UULP GPIO control register.
 * Few actions are required to be performed before setting the mode,
 *  - Enable the ULP clock using @ref sl_si91x_gpio_driver_enable_clock() API.
 *  - Select UULP receiver for UULP GPIO pin.
 *  @note: Select UULP GPIO pins for UULP instances(0 to 4).
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_set_uulp_npss_pin_mux(uint8_t pin, sl_si91x_uulp_npss_mode_t mode)
{
  // Check if pin or mode exceeds the maximum allowed
  if ((pin > GPIO_UULP_MAX_PIN_NUM) || (mode > GPIO_MODE_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Set UULP GPIO pin mode
  sl_si91x_gpio_set_uulp_npss_pin_mux(pin, mode);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to enable receiver bit in UULP GPIO control register.
 * Enable the ULP clock using @ref sl_si91x_gpio_driver_enable_clock() API,
 * before using this API.
 * @note: Select UULP GPIO pins for UULP instances(0 to 4).
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_select_uulp_npss_receiver(uint8_t pin, sl_si91x_gpio_receiver_t receiver)
{
  // Check if pin or receiver exceeds the maximum allowed
  if ((pin > GPIO_UULP_MAX_PIN_NUM) || (receiver > GPIO_RECEIVER_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Select UULP GPIO receiver
  sl_si91x_gpio_select_uulp_npss_receiver(pin, receiver);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to select the UULP direction in UULP GPIO control
 * register. Few actions are required to be performed before setting the
 * direction,
 *  - Enable the ULP clock using @ref sl_si91x_gpio_driver_enable_clock() API.
 *  - Select UULP receiver for UULP GPIO pin.
 *  - Select UULP direction for UULP GPIO pin.
 *  - Set the mode of the GPIO pin.
 * @note: Select UULP GPIO pins for UULP instances(0 to 4).
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_set_uulp_npss_direction(uint8_t pin, sl_si91x_gpio_direction_t direction)
{
  // Check if pin or direction exceeds the maximum allowed
  if ((pin > GPIO_UULP_MAX_PIN_NUM) || (direction > GPIO_DIRECTION_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Set UULP GPIO direction
  sl_si91x_gpio_set_uulp_npss_direction(pin, direction);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to get the UULP direction in UULP GPIO control
 * register. Few actions are required to be performed before setting the
 * direction,
 *  - Enable the ULP clock using @ref sl_si91x_gpio_driver_enable_clock() API.
 *  - Select UULP receiver for UULP GPIO pin.
 *  - Set the mode of the GPIO pin.
 *  - Set the direction of the GPIO pin.
 *  - Get the direction of the GPIO pin.
 * @note: Select UULP GPIO pins for UULP instances(0 to 4).
 *******************************************************************************/
uint8_t sl_si91x_gpio_driver_get_uulp_npss_direction(uint8_t pin)
{
  uint8_t direction;
  // Check if pin exceeds the maximum allowed
  if (pin > GPIO_UULP_MAX_PIN_NUM) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Get UULP GPIO direction
  direction = sl_si91x_gpio_get_uulp_npss_direction(pin);
  return direction;
}

/*******************************************************************************
 * This API is used to select the UULP pin value in UULP GPIO control
 * register. Few actions are required to be performed before setting the
 * direction,
 *  - Enable the ULP clock using @ref sl_si91x_gpio_driver_enable_clock() API.
 *  - Select UULP receiver for UULP GPIO pin.
 *  - Set the mode of the GPIO pin.
 *  - Set the direction of the GPIO pin.
 *  - Select the GPIO pin value.
 * @note: Select UULP GPIO pins for UULP instances(0 to 4).
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_set_uulp_npss_pin_value(uint8_t pin, sl_si91x_gpio_pin_value_t pin_value)
{
  // Check if pin or pin_value exceeds the maximum allowed
  if ((pin > GPIO_UULP_MAX_PIN_NUM) || (pin_value > GPIO_PIN_VALUE_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Set UULP GPIO pin value
  sl_si91x_gpio_set_uulp_npss_pin_value(pin, pin_value);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to toggle the UULP pin.
 * Few actions are required to be performed before setting the direction,
 *  - Enable the ULP clock using @ref sl_si91x_gpio_driver_enable_clock() API.
 *  - Select UULP receiver for UULP GPIO pin.
 *  - Set the mode of the GPIO pin.
 *  - Set the direction of the GPIO pin.
 *  - Toggle the UULP GPIO pin.
 * @note: Select UULP GPIO pins for UULP instances(0 to 4).
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_toggle_uulp_npss_pin(uint8_t pin)
{
  // Check if pin exceeds the maximum allowed
  if (pin > GPIO_UULP_MAX_PIN_NUM) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Toggle UULP GPIO pin
  sl_si91x_gpio_toggle_uulp_npss_pin(pin);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to get the UULP pin value in UULP GPIO control
 * register. Few actions are required to be performed before setting the
 * direction,
 *  - Enable the ULP clock using @ref sl_si91x_gpio_driver_enable_clock() API.
 *  - Select UULP receiver for UULP GPIO pin.
 *  - Set the mode of the GPIO pin.
 *  - Set the direction of the GPIO pin.
 *  - Select the GPIO pin value.
 *  - Get the GPIO pin value.
 * @note: Select UULP GPIO pins for UULP instances(0 to 4).
 ******************************************************************************/
uint8_t sl_si91x_gpio_driver_get_uulp_npss_pin(uint8_t pin)
{
  uint8_t uulp_pin;
  // Check if pin exceeds the maximum allowed
  if (pin > GPIO_UULP_MAX_PIN_NUM) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Get UULP GPIO pin
  uulp_pin = sl_si91x_gpio_get_uulp_npss_pin(pin);
  return uulp_pin;
}

/*******************************************************************************
 * This API is used to select polarity of the UULP GPIO to be considered
      when used as a wakeup source from any of the Sleep States.
 ******************************************************************************/
sl_status_t sl_si91x_gpio_driver_select_uulp_npss_polarity(uint8_t pin, sl_si91x_gpio_polarity_t polarity)
{
  // Check if pin or polarity exceeds the maximum allowed
  if ((pin > GPIO_UULP_MAX_PIN_NUM) || (polarity > GPIO_POLARITY_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Select UULP GPIO polarity
  sl_si91x_gpio_select_uulp_npss_polarity(pin, polarity);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to set the UULP GPIO to wakeup interrupt
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_set_uulp_npss_wakeup_interrupt(uint8_t npssgpio_interrupt)
{
  // Check if npssgpio_interrupt exceeds the maximum allowed
  if (npssgpio_interrupt > GPIO_UULP_WAKEUP_MAX_VALUE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Set UULP GPIO wakeup interrupt
  sl_si91x_gpio_set_uulp_npss_wakeup_interrupt(npssgpio_interrupt);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to clear the UULP GPIO to wakeup interrupt
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_clear_uulp_npss_wakeup_interrupt(uint8_t npssgpio_interrupt)
{
  // Check if npssgpio_interrupt exceeds the maximum allowed
  if (npssgpio_interrupt > GPIO_UULP_WAKEUP_MAX_VALUE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Clear UULP GPIO wakeup interrupt
  sl_si91x_gpio_clear_uulp_npss_wakeup_interrupt(npssgpio_interrupt);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to mask the UULP GPIO interrupt.
 * Few actions are required to be performed before interrupt mask is
 * performed,
 *  - Enable the ULP clock using @ref sl_si91x_gpio_driver_enable_clock() API.
 * 
 *  @note: All the UULP interrupts are masked by default.
 *  @note This function is deprecated and should be replaced with `sl_si91x_gpio_driver_mask_set_uulp_npss_interrupt`.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_mask_uulp_npss_interrupt(uint8_t npssgpio_interrupt)
{
  // Check if npssgpio_interrupt exceeds the maximum allowed
  if (npssgpio_interrupt > GPIO_UULP_INTERRUPT_VALUE_MAX_VALUE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Mask UULP GPIO interrupt
  sl_si91x_gpio_mask_uulp_npss_interrupt(npssgpio_interrupt);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * Get the UULP GPIO interrupt status.
 ******************************************************************************/
uint8_t sl_si91x_gpio_driver_get_uulp_interrupt_status(void)
{
  uint8_t status;
  status = sl_si91x_gpio_get_uulp_interrupt_status();
  return status;
}

/*******************************************************************************
 * Get the ULP GPIO interrupt status.
 ******************************************************************************/
uint32_t sl_si91x_gpio_driver_get_ulp_interrupt_status(uint32_t flags)
{
  uint32_t status;
  status = sl_si91x_gpio_get_ulp_interrupt_status(flags);
  return status;
}

/*******************************************************************************
 * This API is used to un-mask the UULP GPIO interrupt.
 * Few actions are required to be performed before interrupt un-mask is
 * performed,
 *  - Enable the ULP clock using @ref sl_si91x_gpio_driver_enable_clock() API.
 *  - Set UULP PAD configuration register.
 *  - Select UULP receiver for UULP GPIO pin.
 *  - Set the mode of the GPIO pin.
 *  - Set the direction of the GPIO pin.
 *  - Un-mask interrupt by setting corresponding bit in register.
 *  @note: All the UULP interrupts are masked by default.
 *  @note This function is deprecated and should be replaced with `sl_si91x_gpio_driver_mask_clear_uulp_npss_interrupt`.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_unmask_uulp_npss_interrupt(uint8_t npssgpio_interrupt)
{
  // Check if npssgpio_interrupt exceeds the maximum allowed
  if (npssgpio_interrupt > GPIO_UULP_INTERRUPT_VALUE_MAX_VALUE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Unmask UULP GPIO interrupt
  sl_si91x_gpio_unmask_uulp_npss_interrupt(npssgpio_interrupt);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to clear the UULP interrupt.
 * @note This function is deprecated and should be replaced with `sl_si91x_gpio_driver_clear_uulp_npss_interrupt`.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_clear_uulp_interrupt(uint8_t npssgpio_interrupt)
{
  // Check if npssgpio_interrupt exceeds the maximum allowed
  if (npssgpio_interrupt > GPIO_UULP_INTERRUPT_VALUE_MAX_VALUE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Clear UULP GPIO interrupt
  sl_si91x_gpio_clear_uulp_interrupt(npssgpio_interrupt);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to mask the UULP GPIO interrupt.
 * Few actions are required to be performed before interrupt mask is
 * performed,
 *  - Enable the ULP clock using @ref sl_si91x_gpio_driver_enable_clock() API.
 *
 *  @note: All the UULP interrupts are masked by default.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_mask_set_uulp_npss_interrupt(uint8_t npssgpio_interrupt)
{
  // Check if npssgpio_interrupt exceeds the maximum allowed
  if (npssgpio_interrupt > GPIO_UULP_PIN_MAX_VALUE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Mask UULP GPIO interrupt
  sl_si91x_gpio_mask_set_uulp_npss_interrupt(npssgpio_interrupt);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to un-mask the UULP GPIO interrupt.
 * Few actions are required to be performed before interrupt un-mask is
 * performed,
 *  - Enable the ULP clock using @ref sl_si91x_gpio_driver_enable_clock() API.
 *  - Set UULP PAD configuration register.
 *  - Select UULP receiver for UULP GPIO pin.
 *  - Set the mode of the GPIO pin.
 *  - Set the direction of the GPIO pin.
 *  - Un-mask interrupt by setting corresponding bit in register.
 *  @note: All the UULP interrupts are masked by default.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_mask_clear_uulp_npss_interrupt(uint8_t npssgpio_interrupt)
{
  // Check if npssgpio_interrupt exceeds the maximum allowed
  if (npssgpio_interrupt > GPIO_UULP_PIN_MAX_VALUE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Unmask UULP GPIO interrupt
  sl_si91x_gpio_mask_clear_uulp_npss_interrupt(npssgpio_interrupt);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to clear the UULP interrupt.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_clear_uulp_npss_interrupt(uint8_t npssgpio_interrupt)
{
  // Check if npssgpio_interrupt exceeds the maximum allowed
  if (npssgpio_interrupt > GPIO_UULP_PIN_MAX_VALUE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Clear UULP GPIO interrupt
  sl_si91x_gpio_clear_uulp_npss_interrupt(npssgpio_interrupt);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to configure the pin interrupt in GPIO UULP instance.
 * There are total 5 pin interrupts in this instance.
 * To configure the interrupt, first UULP GPIO initialization must be
 * done. The actions to be performed in UULP GPIO initialization are:
 *  - Enable the ULP clock using @ref sl_si91x_gpio_driver_enable_clock() API.
 *  - Set UULP PAD configuration register.
 *  - Select UULP receiver for UULP GPIO pin.
 *  - Set the mode of the GPIO pin.
 *  - Set the direction of the GPIO pin.
 *  - Configure the UULP pin interrupt.
 * Enable the IRQ handler.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_configure_uulp_interrupt(sl_si91x_gpio_interrupt_config_flag_t flags,
                                                          uint8_t npssgpio_interrupt,
                                                          sl_gpio_irq_callback_t gpio_callback)
{
  // Check if gpio_callback pointer is NULL
  if (gpio_callback == NULL) {
    return SL_STATUS_NULL_POINTER;
  }
  // Check if a callback function is already registered for the given interrupt number.
  if (gpio_uulp_pin_int_callback_fptr[npssgpio_interrupt] != NULL) {
    return SL_STATUS_BUSY;
  }
  // Check if npssgpio_interrupt or flags exceeds the maximum allowed
  if ((flags > GPIO_FLAGS_MAX_VALUE) || (npssgpio_interrupt > GPIO_UULP_PIN_MAX_VALUE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Assign the callback function pointer for the specified interrupt number.
  gpio_uulp_pin_int_callback_fptr[npssgpio_interrupt] = gpio_callback;
  // Configure the UULP GPIO interrupt.
  sl_si91x_gpio_configure_uulp_interrupt(flags, npssgpio_interrupt);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to clear one (or) more pending ULP GPIO interrupts.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_clear_ulp_interrupt(uint32_t flags)
{
  // Check if flags exceeds the maximum allowed.
  if (flags > GPIO_FLAGS_MAX_VALUE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Clears ULP GPIO pin interrupt
  sl_si91x_gpio_clear_ulp_interrupt(flags);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * This API is used to clear the ULP group interrupts.
 *******************************************************************************/
sl_status_t sl_si91x_gpio_driver_clear_ulp_group_interrupt(sl_si91x_group_interrupt_t group_interrupt)
{
  // Check if group interrupt exceeds the maximum allowed.
  if (group_interrupt > GPIO_GROUP_INTERRUPT_MAX_VALUE) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Clears ULP GPIO group interrupt
  sl_si91x_gpio_clear_ulp_group_interrupt(group_interrupt);
  return SL_STATUS_OK;
}

/*******************************************************************************
 *   This API handles GPIO pin interrupt 0 request
 ******************************************************************************/
void PIN_IRQ0_Handler(void)
{
  sl_gpio_driver_clear_interrupts(PIN_INTR_0);
  gpio_callback_function_pointer[PIN_INTR_0](PIN_INTR_0);
}

/*******************************************************************************
 *  This API handles GPIO pin interrupt 1 request
 ******************************************************************************/
void PIN_IRQ1_Handler(void)
{
  sl_gpio_driver_clear_interrupts(PIN_INTR_1);
  gpio_callback_function_pointer[PIN_INTR_1](PIN_INTR_1);
}

/*******************************************************************************
 *  This API handles GPIO pin interrupt 2 request
 ******************************************************************************/
void PIN_IRQ2_Handler(void)
{
  sl_gpio_driver_clear_interrupts(PIN_INTR_2);
  gpio_callback_function_pointer[PIN_INTR_2](PIN_INTR_2);
}

/*******************************************************************************
 *  This API handles GPIO pin interrupt 3 request
 ******************************************************************************/
void PIN_IRQ3_Handler(void)
{
  sl_gpio_driver_clear_interrupts(PIN_INTR_3);
  gpio_callback_function_pointer[PIN_INTR_3](PIN_INTR_3);
}

/*******************************************************************************
 *  This API handles GPIO pin interrupt 4 request
 ******************************************************************************/
void PIN_IRQ4_Handler(void)
{
  sl_gpio_driver_clear_interrupts(PIN_INTR_4);
  gpio_callback_function_pointer[PIN_INTR_4](PIN_INTR_4);
}

/*******************************************************************************
 *  This API handles GPIO pin interrupt 5 request
 ******************************************************************************/
void PIN_IRQ5_Handler(void)
{
  sl_gpio_driver_clear_interrupts(PIN_INTR_5);
  gpio_callback_function_pointer[PIN_INTR_5](PIN_INTR_5);
}

/*******************************************************************************
 *  This API handles GPIO pin interrupt 6 request
 ******************************************************************************/
void PIN_IRQ6_Handler(void)
{
  sl_gpio_driver_clear_interrupts(PIN_INTR_6);
  gpio_callback_function_pointer[PIN_INTR_6](PIN_INTR_6);
}

/*******************************************************************************
 *  This API handles GPIO pin interrupt 7 request
 ******************************************************************************/
void PIN_IRQ7_Handler(void)
{
  sl_gpio_driver_clear_interrupts(PIN_INTR_7);
  gpio_callback_function_pointer[PIN_INTR_7](PIN_INTR_7);
}
/*******************************************************************************
 *  This API handles GPIO Group interrupt 0 request
 ******************************************************************************/
void GRP_IRQ0_Handler(void)
{
  sl_si91x_gpio_driver_clear_group_interrupt(GROUP_INT_1);
  gpio_group_int_callback_fptr[GROUP_INT_1](GROUP_INT_1);
}

/*******************************************************************************
 *  This API handles GPIO Group interrupt 1 request
 ******************************************************************************/
void GRP_IRQ1_Handler(void)
{
  sl_si91x_gpio_driver_clear_group_interrupt(GROUP_INT_2);
  gpio_group_int_callback_fptr[GROUP_INT_2](GROUP_INT_2);
}

/*******************************************************************************
 *  This API handles UULP GPIO pin interrupt 0 request
 ******************************************************************************/
void UULP_PIN_IRQ_Handler(void)
{
  uint32_t flag = 0;
  if ((sl_si91x_gpio_driver_get_uulp_interrupt_status() & UULP_INTR_0_STATUS_BIT) != UULP_MASK) {
    sl_si91x_gpio_clear_uulp_npss_interrupt(UULP_GPIO_INTERRUPT_0);
    flag = UULP_GPIO_INTERRUPT_0;
  }
  if ((sl_si91x_gpio_driver_get_uulp_interrupt_status() & UULP_INTR_1_STATUS_BIT) != UULP_MASK) {
    sl_si91x_gpio_clear_uulp_npss_interrupt(UULP_GPIO_INTERRUPT_1);
    flag = UULP_GPIO_INTERRUPT_1;
  }
  if ((sl_si91x_gpio_driver_get_uulp_interrupt_status() & UULP_INTR_2_STATUS_BIT) != UULP_MASK) {
    sl_si91x_gpio_clear_uulp_npss_interrupt(UULP_GPIO_INTERRUPT_2);
    flag = UULP_GPIO_INTERRUPT_2;
  }
  if ((sl_si91x_gpio_driver_get_uulp_interrupt_status() & UULP_INTR_3_STATUS_BIT) != UULP_MASK) {
    sl_si91x_gpio_clear_uulp_npss_interrupt(UULP_GPIO_INTERRUPT_3);
    flag = UULP_GPIO_INTERRUPT_3;
  }
  if ((sl_si91x_gpio_driver_get_uulp_interrupt_status() & UULP_INTR_4_STATUS_BIT) != UULP_MASK) {
    sl_si91x_gpio_clear_uulp_npss_interrupt(UULP_GPIO_INTERRUPT_4);
    flag = UULP_GPIO_INTERRUPT_4;
  }
  if (gpio_uulp_pin_int_callback_fptr[flag] != NULL) {
    gpio_uulp_pin_int_callback_fptr[flag](flag);
  }
}

/*******************************************************************************
 *  This API handles ULP GPIO OR'ed pin interrupt request
 ******************************************************************************/
void ULP_PIN_IRQ_Handler(void)
{
  uint32_t flag = 0;
  if ((sl_si91x_gpio_driver_get_ulp_interrupt_status(ULP_PIN_INTR_0)) == ULP_STATUS) {
    sl_si91x_gpio_driver_clear_ulp_interrupt(ULP_PIN_INTR_0);
    flag = ULP_PIN_INTR_0;
  }
  if ((sl_si91x_gpio_driver_get_ulp_interrupt_status(ULP_PIN_INTR_1)) == ULP_STATUS) {
    sl_si91x_gpio_driver_clear_ulp_interrupt(ULP_PIN_INTR_1);
    flag = ULP_PIN_INTR_1;
  }
  if ((sl_si91x_gpio_driver_get_ulp_interrupt_status(ULP_PIN_INTR_2)) == ULP_STATUS) {
    sl_si91x_gpio_driver_clear_ulp_interrupt(ULP_PIN_INTR_2);
    flag = ULP_PIN_INTR_2;
  }
  if ((sl_si91x_gpio_driver_get_ulp_interrupt_status(ULP_PIN_INTR_3)) == ULP_STATUS) {
    sl_si91x_gpio_driver_clear_ulp_interrupt(ULP_PIN_INTR_3);
    flag = ULP_PIN_INTR_3;
  }
  if ((sl_si91x_gpio_driver_get_ulp_interrupt_status(ULP_PIN_INTR_4)) == ULP_STATUS) {
    sl_si91x_gpio_driver_clear_ulp_interrupt(ULP_PIN_INTR_4);
    flag = ULP_PIN_INTR_4;
  }
  if ((sl_si91x_gpio_driver_get_ulp_interrupt_status(ULP_PIN_INTR_5)) == ULP_STATUS) {
    sl_si91x_gpio_driver_clear_ulp_interrupt(ULP_PIN_INTR_5);
    flag = ULP_PIN_INTR_5;
  }
  if ((sl_si91x_gpio_driver_get_ulp_interrupt_status(ULP_PIN_INTR_6)) == ULP_STATUS) {
    sl_si91x_gpio_driver_clear_ulp_interrupt(ULP_PIN_INTR_6);
    flag = ULP_PIN_INTR_6;
  }
  if ((sl_si91x_gpio_driver_get_ulp_interrupt_status(ULP_PIN_INTR_7)) == ULP_STATUS) {
    sl_si91x_gpio_driver_clear_ulp_interrupt(ULP_PIN_INTR_7);
    flag = ULP_PIN_INTR_7;
  }
  gpio_ulp_pin_int_callback_fptr[flag](flag);
}

/*******************************************************************************
 *  This API handles ULP GPIO group interrupt request
 ******************************************************************************/
void ULP_GROUP_IRQ_Handler(void)
{
  uint32_t flag = 0;
  if (sl_si91x_gpio_get_group_interrupt_status(SL_GPIO_ULP_PORT, GROUP_INT_1) == ULP_STATUS) {
    sl_si91x_gpio_driver_clear_ulp_group_interrupt(GROUP_INT_1);
    flag = GROUP_INT_1;
  }
  if (sl_si91x_gpio_get_group_interrupt_status(SL_GPIO_ULP_PORT, GROUP_INT_2) == ULP_STATUS) {
    sl_si91x_gpio_driver_clear_ulp_group_interrupt(GROUP_INT_2);
    flag = GROUP_INT_2;
  }

  gpio_ulp_group_int_callback_fptr[flag](flag);
}

/*******************************************************************************
 * De-Initialization of GPIO driver.
 ******************************************************************************/
sl_status_t sl_gpio_driver_deinit(void)
{
  sl_status_t status;
  uint8_t flag;
  do {
    for (flag = 0; flag < GPIO_MAX_INTR_VALUE; flag++) {
      gpio_callback_function_pointer[flag] = NULL;
    }
    for (flag = 0; flag < MAX_GROUP_INT; flag++) {
      gpio_group_int_callback_fptr[flag] = NULL;
    }
    for (flag = 0; flag < MAX_UULP_INT; flag++) {
      gpio_uulp_pin_int_callback_fptr[flag] = NULL;
    }
    for (flag = 0; flag < GPIO_MAX_INTR_VALUE; flag++) {
      gpio_ulp_pin_int_callback_fptr[flag] = NULL;
    }
    for (flag = 0; flag < MAX_GROUP_INT; flag++) {
      gpio_ulp_group_int_callback_fptr[flag] = NULL;
    }
    status = sl_si91x_gpio_driver_disable_clock(M4CLK_GPIO); // Disables M4 GPIO clock
    if (status != SL_STATUS_OK) {
      return status;
    }
    status = sl_si91x_gpio_driver_disable_clock(ULPCLK_GPIO); // Disables ULP GPIO clock
    if (status != SL_STATUS_OK) {
      return status;
    }
    // Clearing the soc peripheral on ulp gpio status flag
    soc_peri_on_ulp_gpio_status = 0;
  } while (false);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * Unregister GPIO interrupts
 ******************************************************************************/
sl_status_t sl_gpio_driver_unregister(sl_si91x_gpio_instances_t gpio_instance,
                                      sl_si91x_gpio_intr_t gpio_intr,
                                      uint8_t flag)
{
  if (gpio_instance >= GPIO_INSTANCE_LAST) {
    // Returns invalid parameter status code if gpio_instance > GPIO_INSTANCE_LAST
    return SL_STATUS_INVALID_PARAMETER;
  }
  switch (gpio_instance) {
    case M4_GPIO_INSTANCE:
      switch (gpio_intr) {
        case GPIO_PIN_INTERRUPT:
          if (flag > GPIO_MAX_INTR_VALUE) {
            // Returns invalid parameter status code if flag > GPIO_MAX_INTR_VALUE
            return SL_STATUS_INVALID_PARAMETER;
          }
          gpio_callback_function_pointer[flag] = NULL;
          break;
        case GPIO_GROUP_INTERRUPT:
          if (flag > MAX_GROUP_INT) {
            // Returns invalid parameter status code if flag > MAX_GROUP_INT
            return SL_STATUS_INVALID_PARAMETER;
          }
          gpio_group_int_callback_fptr[flag] = NULL;
          break;
      }
      break;
    case ULP_GPIO_INSTANCE:
      switch (gpio_intr) {
        case GPIO_PIN_INTERRUPT:
          if (flag > GPIO_MAX_INTR_VALUE) {
            // Returns invalid parameter status code if flag > GPIO_MAX_INTR_VALUE
            return SL_STATUS_INVALID_PARAMETER;
          }
          gpio_ulp_pin_int_callback_fptr[flag] = NULL;
          break;
        case GPIO_GROUP_INTERRUPT:
          if (flag > MAX_GROUP_INT) {
            // Returns invalid parameter status code if flag > MAX_GROUP_INT
            return SL_STATUS_INVALID_PARAMETER;
          }
          gpio_ulp_group_int_callback_fptr[flag] = NULL;
          break;
      }
      break;
    case UULP_GPIO_INSTANCE:
      if (flag > MAX_UULP_INT) {
        // Returns invalid parameter status code if flag > GPIO_MAX_INTR_VALUE
        return SL_STATUS_INVALID_PARAMETER;
      }
      gpio_uulp_pin_int_callback_fptr[flag] = NULL;
      break;
    case GPIO_INSTANCE_LAST:
      break;
    default:
      break;
  }
  return SL_STATUS_OK;
}

/******************************************************************************* 
 * This function checks if the given ULP pin is configured as a SOC pin by
 * examining the `soc_peri_on_ulp_gpio_status` bit corresponding to the pin number.
 * The function performs the following steps:
 *   1. Shifts the `soc_peri_on_ulp_gpio_status` by the pin number minus the offset for ULP pins.
 *   2. Checks if the resulting bit is set.
 *      - If the bit is set, the pin is valid and SOC peripheral is configured on that ULP pin.
 *      - If the bit is not set, the pin is invalid and SOC peripheral is not configured on that ULP pin.
******************************************************************************/
sl_status_t sl_si91x_gpio_validate_soc_peri_on_ulp_gpio(uint8_t pin)
{
  if ((soc_peri_on_ulp_gpio_status >> (pin - SOC_PERI_ON_ULP_GPIO_OFFSET)) & 1) {
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_INVALID_PARAMETER;
  }
}

/******************************************************************************* 
 * This function configures the mode for a specified GPIO pin. It converts
 * the ULP GPIO in to SOC (HP) Mode. The function performs the following steps:
 * 1. Validates the input GPIO pointer.
 * 2. Validates the mode value parameters.
 * 3. The pin is in ULP mode and should be configured as HP, it sets the pin mode to 6 and configures
 *    the pin mode for the corresponding HP pin.
 * @note This function uses helper functions to set the pin mode and validate the GPIO structure.
******************************************************************************/
sl_status_t sl_si91x_gpio_driver_set_soc_peri_on_ulp_pin_mode(sl_gpio_t *gpio, sl_gpio_mode_t mode)
{
  sl_status_t status = SL_STATUS_OK;
  // Check if the GPIO pointer is NULL
  if (gpio == NULL) {
    // Return error if GPIO pointer is NULL
    status = SL_STATUS_NULL_POINTER;
    return status;
  }
  // Validate the GPIO structure
  status = sl_gpio_validation(gpio);
  if (status != SL_STATUS_OK) {
    // Return validation status if not OK
    return status;
  }
  // Check if the mode value exceeds the maximum allowed values
  if (mode > MAX_MODE) {
    // Return error if mode value is invalid
    status = SL_STATUS_INVALID_PARAMETER;
    return status;
  }
  // If the ULP pin should be configured as a high power pin
  // Set pin mode (mode 6) for ULP pin to act as HP pin
  sl_gpio_set_pin_mode(gpio->port, gpio->pin, SL_GPIO_MODE_6, OUTPUT_VALUE);
  // Sets the bit position of the ULP pin converted to the SOC gpio for further validation
  soc_peri_on_ulp_gpio_status |= BIT(gpio->pin);
  // Set pin mode for corresponding HP pin.
  sl_gpio_set_pin_mode(SL_GPIO_PORT_A, gpio->pin + SOC_PERI_ON_ULP_GPIO_OFFSET, mode, OUTPUT_VALUE);
  return status;
}

/******************************************************************************* 
 * This function configures the mode value for a specified GPIO pin. It converts
 * the SOC (HP) GPIO in to ULP Mode. The function performs the following steps:
 * 1. Validates the input GPIO pointer.
 * 2. Validates the mode value parameters.
 * 3. Configures the pin mode based on the input pin number:
 *      - For pins 25 to 30, it sets the pin mode to 11 and configures the pin mode for the corresponding HP pin.
 *      - For pin 15, it sets the pin mode to 9 and configures the pin mode for the corresponding HP pin.
 *      - For other pins, it sets the pin mode to 9 and configures the pin mode for the corresponding HP pin.
 * @note This function uses helper functions to set the pin mode and validate the GPIO structure.
******************************************************************************/
sl_status_t sl_si91x_gpio_driver_set_ulp_peri_on_soc_pin_mode(sl_gpio_t *gpio, sl_gpio_mode_t mode)
{
  sl_status_t status = SL_STATUS_OK;
  // Check if the GPIO pointer is NULL
  if (gpio == NULL) {
    // Return error if GPIO pointer is NULL
    status = SL_STATUS_NULL_POINTER;
    return status;
  }
  // Validate the GPIO structure
  status = sl_gpio_validation(gpio);
  if (status != SL_STATUS_OK) {
    // Return validation status if not OK
    return status;
  }
  // Check if the mode exceeds the maximum allowed values
  if (mode > MAX_MODE) {
    // Return error if mode value is invalid
    status = SL_STATUS_INVALID_PARAMETER;
    return status;
  }
  //Setting offset value for PORT_A HP GPIO to act as ULP pin
  if (gpio->port == SL_GPIO_PORT_A) {
    // If the high power pin should be configured as a ULP pin
    // Check if the pin is within the host pad range
    if (gpio->pin >= HOST_PAD_MIN && gpio->pin <= HOST_PAD_MAX) {
      // Set pin mode(mode 11) for HP pin to act as ULP pin
      sl_gpio_set_pin_mode(gpio->port, gpio->pin, ULP_PERI_ON_SOC_GPIO_HOST_PAD_PIN_MODE, OUTPUT_VALUE);
      // Set pin mode for corresponding ULP pin
      sl_gpio_set_pin_mode(SL_GPIO_ULP_PORT, gpio->pin - ULP_PERI_ON_SOC_GPIO_HOST_PAD_OFFSET, 0, OUTPUT_VALUE);
      sl_si91x_gpio_ulp_soc_mode(gpio->pin - ULP_PERI_ON_SOC_GPIO_HOST_PAD_OFFSET, mode);
    } else {
      // If not host pad pin then set pin mode (mode 9) for HP pin to act as ULP pin
      sl_gpio_set_pin_mode(gpio->port, gpio->pin, SL_GPIO_MODE_9, OUTPUT_VALUE);
      if (gpio->pin == 15) {
        // Set pin mode for pin 15, exception because the offset is different from others
        sl_gpio_set_pin_mode(SL_GPIO_ULP_PORT, gpio->pin - ULP_PERI_ON_SOC_GPIO_PIN15_OFFSET, 0, OUTPUT_VALUE);
        sl_si91x_gpio_ulp_soc_mode(gpio->pin - ULP_PERI_ON_SOC_GPIO_PIN15_OFFSET, mode);
      } else if (gpio->pin >= ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MIN
                 && gpio->pin <= ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_MAX) {
        // Set pin mode for specific range of pins
        sl_gpio_set_pin_mode(SL_GPIO_ULP_PORT, gpio->pin - ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_OFFSET, 0, OUTPUT_VALUE);
        sl_si91x_gpio_ulp_soc_mode(gpio->pin - ULP_PERI_ON_SOC_GPIO_SPECIFIC_RANGE_OFFSET, mode);
      } else {
        // Set pin mode for other pins
        sl_gpio_set_pin_mode(SL_GPIO_ULP_PORT, gpio->pin - ULP_PERI_ON_SOC_GPIO_OFFSET, 0, OUTPUT_VALUE);
        sl_si91x_gpio_ulp_soc_mode(gpio->pin - ULP_PERI_ON_SOC_GPIO_OFFSET, mode);
      }
    }
  } else if (gpio->port == SL_GPIO_PORT_B) { //Setting offset value for PORT_B HP GPIO to act as ULP pin
    // If the high power pin should be configured as a ULP pin
    // Set pin mode(mode 11) for HP pin to act as ULP pin
    sl_gpio_set_pin_mode(gpio->port, gpio->pin, ULP_PERI_ON_SOC_GPIO_HOST_PAD_PIN_MODE, OUTPUT_VALUE);
    // Set pin mode for corresponding ULP pin
    sl_gpio_set_pin_mode(SL_GPIO_ULP_PORT, gpio->pin - ULP_PERI_ON_SOC_GPIO_PORT_B_OFFSET, 0, OUTPUT_VALUE);
    sl_si91x_gpio_ulp_soc_mode(gpio->pin - ULP_PERI_ON_SOC_GPIO_PORT_B_OFFSET, mode);
  } else {
    if (gpio->port == SL_GPIO_PORT_C) { //Setting offset value for PORT_C HP GPIO to act as ULP pin
      if ((gpio->pin == 14) || (gpio->pin == 15)) {
        // Set pin mode for specific range of pins
        sl_gpio_set_pin_mode(SL_GPIO_ULP_PORT, gpio->pin - ULP_PERI_ON_SOC_GPIO_PORT_C_OFFSET, 0, OUTPUT_VALUE);
        sl_si91x_gpio_ulp_soc_mode(gpio->pin - ULP_PERI_ON_SOC_GPIO_PORT_C_OFFSET, mode);
      }
    }
    if (gpio->port == SL_GPIO_PORT_D) { //Setting offset value for PORT_D HP GPIO to act as ULP pin
      if ((gpio->pin == 0) || (gpio->pin == 1)) {
        // Set pin mode for specific range of pins
        sl_gpio_set_pin_mode(SL_GPIO_ULP_PORT, gpio->pin + ULP_PERI_ON_SOC_GPIO_PORT_D_OFFSET, 0, OUTPUT_VALUE);
        sl_si91x_gpio_ulp_soc_mode(gpio->pin + ULP_PERI_ON_SOC_GPIO_PORT_D_OFFSET, mode);
      }
    }
  }
  return status;
}
