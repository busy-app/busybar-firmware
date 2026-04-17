/******************************************************************************
* @file sl_si91x_peripheral_gpio.h
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
#ifndef SL_PERIPHERAL_GPIO_H
#define SL_PERIPHERAL_GPIO_H

#if !defined(GPIO_PRESENT)
#include "sl_si91x_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/ /**
 * @addtogroup GPIO-PERIPHERAL-DRIVER GPIO Peripheral Driver
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 * 
 ******************************************************************************/
/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
#define _GPIO_PORT_A_PIN_COUNT GPIO_PA_COUNT ///< GPIO port A maximum pins
#define _GPIO_PORT_B_PIN_COUNT GPIO_PB_COUNT ///< GPIO port B maximum pins
#define _GPIO_PORT_C_PIN_COUNT GPIO_PC_COUNT ///< GPIO port C maximum pins
#define _GPIO_PORT_D_PIN_COUNT GPIO_PD_COUNT ///< GPIO port D maximum pins
#define _GPIO_PORT_E_PIN_COUNT 0
#define _GPIO_PORT_F_PIN_COUNT 0
#define _GPIO_PORT_G_PIN_COUNT 0
#define _GPIO_PORT_H_PIN_COUNT 0
#define _GPIO_PORT_I_PIN_COUNT 0
#define _GPIO_PORT_J_PIN_COUNT 0
#define _GPIO_PORT_K_PIN_COUNT 0

#define _GPIO_PORT_A_PIN_MASK (GPIO_PA_MASK) ///< GPIO port A pin mask
#define _GPIO_PORT_B_PIN_MASK (GPIO_PB_MASK) ///< GPIO port B pin mask
#define _GPIO_PORT_C_PIN_MASK (GPIO_PC_MASK) ///< GPIO port C pin mask
#define _GPIO_PORT_D_PIN_MASK (GPIO_PD_MASK) ///< GPIO port D pin mask
#define _GPIO_PORT_E_PIN_MASK 0x0000UL
#define _GPIO_PORT_F_PIN_MASK 0x0000UL
#define _GPIO_PORT_G_PIN_MASK 0x0000UL
#define _GPIO_PORT_H_PIN_MASK 0x0000UL
#define _GPIO_PORT_I_PIN_MASK 0x0000UL
#define _GPIO_PORT_J_PIN_MASK 0x0000UL
#define _GPIO_PORT_K_PIN_MASK 0x0000UL

///< GPIO pins selection for selected port
#define _GPIO_PORT_SIZE(port)              \
  ((port) == 0    ? _GPIO_PORT_A_PIN_COUNT \
   : (port) == 1  ? _GPIO_PORT_B_PIN_COUNT \
   : (port) == 2  ? _GPIO_PORT_C_PIN_COUNT \
   : (port) == 3  ? _GPIO_PORT_D_PIN_COUNT \
   : (port) == 4  ? _GPIO_PORT_E_PIN_COUNT \
   : (port) == 5  ? _GPIO_PORT_F_PIN_COUNT \
   : (port) == 6  ? _GPIO_PORT_G_PIN_COUNT \
   : (port) == 7  ? _GPIO_PORT_H_PIN_COUNT \
   : (port) == 8  ? _GPIO_PORT_I_PIN_COUNT \
   : (port) == 9  ? _GPIO_PORT_J_PIN_COUNT \
   : (port) == 10 ? _GPIO_PORT_K_PIN_COUNT \
                  : 0)

///< GPIO pins mask for selected port
#define _GPIO_PORT_MASK(port)                  \
  (((int)port) == 0    ? _GPIO_PORT_A_PIN_MASK \
   : ((int)port) == 1  ? _GPIO_PORT_B_PIN_MASK \
   : ((int)port) == 2  ? _GPIO_PORT_C_PIN_MASK \
   : ((int)port) == 3  ? _GPIO_PORT_D_PIN_MASK \
   : ((int)port) == 4  ? _GPIO_PORT_E_PIN_MASK \
   : ((int)port) == 5  ? _GPIO_PORT_F_PIN_MASK \
   : ((int)port) == 6  ? _GPIO_PORT_G_PIN_MASK \
   : ((int)port) == 7  ? _GPIO_PORT_H_PIN_MASK \
   : ((int)port) == 8  ? _GPIO_PORT_I_PIN_MASK \
   : ((int)port) == 9  ? _GPIO_PORT_J_PIN_MASK \
   : ((int)port) == 10 ? _GPIO_PORT_K_PIN_MASK \
                       : 0UL)

#define SL_GPIO_PORT_VALID(port) (_GPIO_PORT_MASK(port) != 0x0UL) ///< Validation of GPIO port.

#define SL_GPIO_PORT_PIN_VALID(port, pin) \
  ((((_GPIO_PORT_MASK(port)) >> (pin)) & 0x1UL) == 0x1UL) ///< Validating GPIO port and pin

#define GPIO_PIN_MAX 15 ///< Highest GPIO pin number.

///@brief Highest GPIO port number.
#if (_GPIO_PORT_K_PIN_COUNT > 0)
#define GPIO_PORT_MAX 10
#elif (_GPIO_PORT_J_PIN_COUNT > 0)
#define GPIO_PORT_MAX 9
#elif (_GPIO_PORT_I_PIN_COUNT > 0)
#define GPIO_PORT_MAX 8
#elif (_GPIO_PORT_H_PIN_COUNT > 0)
#define GPIO_PORT_MAX 7
#elif (_GPIO_PORT_G_PIN_COUNT > 0)
#define GPIO_PORT_MAX 6
#elif (_GPIO_PORT_F_PIN_COUNT > 0)
#define GPIO_PORT_MAX 5
#elif (_GPIO_PORT_E_PIN_COUNT > 0)
#define GPIO_PORT_MAX 4
#elif (_GPIO_PORT_D_PIN_COUNT > 0)
#define GPIO_PORT_MAX 3
#else
#error "Max GPIO port number is undefined for this part."
#endif

#define GPIO_EXTINTNO_MAX 15 ///< Highest EXT GPIO interrupt number.

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

///@brief GPIO ports IDs.
typedef enum {
#if (_GPIO_PORT_A_PIN_COUNT > 0)
  SL_GPIO_PORT_A = 0, ///< GPIO Port A
#endif
#if (_GPIO_PORT_B_PIN_COUNT > 0)
  SL_GPIO_PORT_B = 1, ///< GPIO Port B
#endif
#if (_GPIO_PORT_C_PIN_COUNT > 0)
  SL_GPIO_PORT_C = 2, ///< GPIO Port C
#endif
#if (_GPIO_PORT_D_PIN_COUNT > 0)
  SL_GPIO_PORT_D = 3, ///< GPIO Port D
#endif
#if (_GPIO_PORT_E_PIN_COUNT > 0)
  SL_GPIO_PORT_E = 4,
#endif
#if (_GPIO_PORT_F_PIN_COUNT > 0)
  SL_GPIO_PORT_F = 5,
#endif
#if (_GPIO_PORT_G_PIN_COUNT > 0)
  SL_GPIO_PORT_G = 6,
#endif
#if (_GPIO_PORT_H_PIN_COUNT > 0)
  SL_GPIO_PORT_H = 7,
#endif
#if (_GPIO_PORT_I_PIN_COUNT > 0)
  SL_GPIO_PORT_I = 8,
#endif
#if (_GPIO_PORT_J_PIN_COUNT > 0)
  SL_GPIO_PORT_J = 9,
#endif
#if (_GPIO_PORT_K_PIN_COUNT > 0)
  SL_GPIO_PORT_K = 10,
#endif
} sl_gpio_port_t;

///@brief GPIO Pin Modes.
typedef enum {
  SL_GPIO_MODE_0          = _MODE0,                        ///< Pin MUX GPIO Mode 0.
  SL_GPIO_MODE_1          = _MODE1,                        ///< Pin MUX GPIO Mode 1.
  SL_GPIO_MODE_2          = _MODE2,                        ///< Pin MUX GPIO Mode 2.
  SL_GPIO_MODE_3          = _MODE3,                        ///< Pin MUX GPIO Mode 3.
  SL_GPIO_MODE_4          = _MODE4,                        ///< Pin MUX GPIO Mode 4.
  SL_GPIO_MODE_5          = _MODE5,                        ///< Pin MUX GPIO Mode 5.
  SL_GPIO_MODE_6          = _MODE6,                        ///< Pin MUX GPIO Mode 6.
  SL_GPIO_MODE_7          = _MODE7,                        ///< Pin MUX GPIO Mode 7.
  SL_GPIO_MODE_8          = _MODE8,                        ///< Pin MUX GPIO Mode 8.
  SL_GPIO_MODE_9          = _MODE9,                        ///< Pin MUX GPIO Mode 9.
  SL_GPIO_MODE_10         = _MODE10,                       ///< Pin MUX GPIO Mode 10.
  SL_GPIO_MODE_12         = _MODE12,                       ///< Pin MUX GPIO Mode 12.
  SL_GPIO_MODE_14         = _MODE14,                       ///< Pin MUX GPIO Mode 14.
  SL_GPIO_MODE_DISABLED   = _GPIO_P_MODEL_MODE0_DISABLED,  ///< Input disabled. Pull-up if DOUT is set.
  SL_GPIO_MODE_INPUT      = _GPIO_P_MODEL_MODE0_INPUT,     ///< Input enabled. Filter if DOUT is set.
  SL_GPIO_MODE_INPUT_PULL = _GPIO_P_MODEL_MODE0_INPUTPULL, ///< Input enabled. DOUT determines pull direction.
  SL_GPIO_MODE_INPUT_PULL_FILTER =
    _GPIO_P_MODEL_MODE0_INPUTPULLFILTER, ///< Input enabled with filter. DOUT determines pull direction.
  SL_GPIO_MODE_PUSH_PULL           = _GPIO_P_MODEL_MODE0_PUSHPULL,        ///< Push-pull output.
  SL_GPIO_MODE_PUSH_PULL_ALTERNATE = _GPIO_P_MODEL_MODE0_PUSHPULLALT,     ///< Push-pull using alternate control.
  SL_GPIO_MODE_WIRED_OR            = _GPIO_P_MODEL_MODE0_WIREDOR,         ///< Wired-or output.
  SL_GPIO_MODE_WIRED_OR_PULL_DOWN  = _GPIO_P_MODEL_MODE0_WIREDORPULLDOWN, ///< Wired-or output with pull-down.
  SL_GPIO_MODE_WIRED_AND           = _GPIO_P_MODEL_MODE0_WIREDAND,        ///< Open-drain output.
  SL_GPIO_MODE_WIRED_AND_FILTER    = _GPIO_P_MODEL_MODE0_WIREDANDFILTER,  ///< Open-drain output with filter.
  SL_GPIO_MODE_WIRED_AND_PULLUP    = _GPIO_P_MODEL_MODE0_WIREDANDPULLUP,  ///< Open-drain output with pull-up.
  SL_GPIO_MODE_WIRED_AND_PULLUP_FILTER =
    _GPIO_P_MODEL_MODE0_WIREDANDPULLUPFILTER,                         ///< Open-drain output with filter and pull-up.
  SL_GPIO_MODE_WIRED_AND_ALTERNATE = _GPIO_P_MODEL_MODE0_WIREDANDALT, ///< Open-drain output using alternate control.
  SL_GPIO_MODE_WIRED_AND_ALTERNATE_FILTER =
    _GPIO_P_MODEL_MODE0_WIREDANDALTFILTER, ///< Open-drain output using alternate control with filter.
  SL_GPIO_MODE_WIRED_AND_ALTERNATE_PULLUP =
    _GPIO_P_MODEL_MODE0_WIREDANDALTPULLUP, ///< Open-drain output using alternate control with pull-up.
  SL_GPIO_MODE_WIRED_AND_ALTERNATE_PULLUP_FILTER =
    _GPIO_P_MODEL_MODE0_WIREDANDALTPULLUPFILTER, ///< Open-drain output using alternate control with filter and pull-up.
} sl_gpio_mode_t;

///@brief GPIO Interrupt Configurations.
typedef enum {
  SL_GPIO_INTERRUPT_DISABLE      = 0,        ///< disable the interrupt
  SL_GPIO_INTERRUPT_ENABLE       = (1 << 0), ///< enable the interrupt
  SL_GPIO_INTERRUPT_HIGH         = (1 << 0), ///< interrupt when pin level is '1'
  SL_GPIO_INTERRUPT_LOW          = (1 << 1), ///< interrupt when pin level is '0'
  SL_GPIO_INTERRUPT_RISING_EDGE  = (1 << 2), ///< interrupt when rising edge is detected
  SL_GPIO_INTERRUPT_FALLING_EDGE = (1 << 3), ///< interrupt when falling edge is detected
  SL_GPIO_INTERRUPT_RISE_FALL_EDGE =
    (SL_GPIO_INTERRUPT_FALLING_EDGE
     | SL_GPIO_INTERRUPT_RISING_EDGE), ///< interrupt when rising and falling edge is detected
} sl_gpio_interrupt_flag_t;

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************/ /**
 * @brief     Configure the MCU HP instance pin interrupts with trigger type(level/edge)
 *            and register the callback function for interrupts.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_gpio_set_pin_mode() \n
 * @pre   \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  pin  - The pin number on the port.
 *                HP instance has total 57 GPIO pins. Port 0, 1, 2 has 16 pins each.
 *                Port 3 has 9 pins.
 *                ULP instance has total 12 pins.
 * @param[in] int_no - The interrupt number to trigger.
 * @param[in] flags  - Interrupt configuration flags
 * @return    None
 ******************************************************************************/
void sl_gpio_configure_interrupt(sl_gpio_port_t port, uint8_t pin, uint32_t int_no, sl_gpio_interrupt_flag_t flags);

/***************************************************************************/ /**
 * @brief      Set the pin mode (alternate function) of a GPIO for either HP instance (or) ULP instance as per the port number.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance.
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  pin  - The pin number on the port.
 *                HP instance has total 57 GPIO pins. Port 0, 1, 2 has 16 pins each.
 *                Port 3 has 9 pins.
 *                ULP instance has total 12 pins.
 * @param[in]  mode - The desired pin mode.
 * @param[in]  output_value - A value to set for the pin in the GPIO register.
 *                The GPIO setting is important for some input mode configurations.
 * @return     None
 ******************************************************************************/
void sl_gpio_set_pin_mode(sl_gpio_port_t port, uint8_t pin, sl_gpio_mode_t mode, uint32_t output_value);

/***************************************************************************/ /**
 * @brief      Get the pin mode (alternate function) of a GPIO for either HP instance or ULP instance as per the port number.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance.
 * @pre   \ref sl_gpio_set_pin_mode();
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  pin  - The pin number on the port.
 *                HP instance has total 57 GPIO pins. Port 0, 1, 2 has 16 pins each.
 *                Port 3 has 9 pins.
 *                ULP instance has total 12 pins.
 * @return     returns Pin status
 *                   '0' - Output
 *                   '1' - Input
 ******************************************************************************/
sl_gpio_mode_t sl_gpio_get_pin_mode(sl_gpio_port_t port, uint8_t pin);

/***************************************************************************/ /**
 * @brief Set a single GPIO pin of a GPIO port with 1.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 * @pre   \ref sl_gpio_set_pin_mode(); \n
 * @pre   \ref sl_si91x_gpio_set_pin_direction(); \n
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  pin  - The pin number on the port.
 *                HP instance has total 57 GPIO pins. Port 0, 1, 2 has 16 pins each.
 *                Port 3 has 9 pins.
 *                ULP instance has total 12 pins.
 * @return     None
 ******************************************************************************/
static __INLINE void sl_gpio_set_pin_output(sl_gpio_port_t port, uint8_t pin)
{
  SL_GPIO_ASSERT(SL_GPIO_VALIDATE_PORT(port));
  if ((port == SL_GPIO_PORT_A) || (port == SL_GPIO_PORT_B) || (port == SL_GPIO_PORT_C) || (port == SL_GPIO_PORT_D)) {
    SL_GPIO_ASSERT(SL_GPIO_NDEBUG_PORT_PIN(port, pin));
    GPIO->PIN_CONFIG[(port * MAX_GPIO_PORT_PIN) + pin].BIT_LOAD_REG = SET;
  } else if (port == SL_GPIO_ULP_PORT) {
    SL_GPIO_ASSERT(SL_GPIO_VALIDATE_ULP_PORT_PIN(port, pin));
    ULP_GPIO->PIN_CONFIG[pin].BIT_LOAD_REG = SET;
  } else {
    SL_GPIO_ASSERT(SL_GPIO_VALIDATE_UULP_PORT_PIN(port, pin));
    // Set pin in UULP GPIO instance by controlling pin value
    UULP_GPIO->NPSS_GPIO_CNTRL[pin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_OUT = SET;
  }
}

/***************************************************************************/ /**
 * @brief  Set the selected bits value of GPIO data out register to 1.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 * @pre   \ref sl_gpio_set_pin_mode(); \n
 * @pre   \ref sl_si91x_gpio_set_pin_direction(); \n
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in] pins - The GPIO pins in a port are set to 1's.
 * @return    None
 ******************************************************************************/
static __INLINE void sl_gpio_set_port_output(sl_gpio_port_t port, uint32_t pins)
{
  SL_GPIO_ASSERT(SL_GPIO_VALIDATE_PORT(port));
  if (port == SL_GPIO_ULP_PORT) {
    ULP_GPIO->PORT_CONFIG[SL_GPIO_PORT_A].PORT_SET_REG = (pins);
  } else {
    GPIO->PORT_CONFIG[port].PORT_SET_REG = (pins);
  }
}

/***************************************************************************/ /**
 * @brief  Set GPIO port value with the selected mask bits.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 * @pre   \ref sl_gpio_set_pin_mode(); \n
 * @pre   \ref sl_si91x_gpio_set_pin_direction(); \n
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  val -  Value to write to port configuration  register.
 * @param[in]  mask - Mask indicating which bits to modify.
 * @return     None
 ******************************************************************************/
static __INLINE void sl_gpio_set_port_output_value(sl_gpio_port_t port, uint32_t val, uint32_t mask)
{
  SL_GPIO_ASSERT(SL_GPIO_VALIDATE_PORT(port));
  if (port == SL_GPIO_ULP_PORT) {
    ULP_GPIO->PORT_CONFIG[SL_GPIO_PORT_A].PORT_SET_REG = (ULP_GPIO->PORT_CONFIG[port].PORT_SET_REG & ~mask)
                                                         | (val & mask);
  } else {
    GPIO->PORT_CONFIG[port].PORT_SET_REG = (GPIO->PORT_CONFIG[port].PORT_SET_REG & ~mask) | (val & mask);
  }
}

/***************************************************************************/ /**
 * @brief  Set slewrate for pins on a GPIO port.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @param[in]  port - The GPIO port to configure.
 * @param[in]  slewrate - The slewrate to configure for pins on this GPIO port.
 * @param[in]  slewrate_alt - The slewrate to configure for pins using alternate modes on this GPIO port.
 * @return     None
 ******************************************************************************/
static __INLINE void sl_gpio_set_slew_rate(sl_gpio_port_t port, uint32_t slewrate, uint32_t slewrate_alt)
{
  UNUSED_VAR(slewrate_alt);
  SL_GPIO_ASSERT(SL_GPIO_VALIDATE_PORT(port));
  for (uint8_t i = (port * MAX_GPIO_PORT_PIN); i < (MAX_GPIO_PORT_PIN * (port + 1)); i++) {
    PAD_REG(i)->GPIO_PAD_CONFIG_REG_b.PADCONFIG_SR = (sl_si91x_gpio_slew_rate_t)slewrate;
  }
}

/***************************************************************************/ /**
 * @brief  Clear a single pin in GPIO configuration register to 0.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_ulp_pad_receiver(); for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 * @pre   \ref sl_gpio_set_pin_mode(); \n
 * @pre   \ref sl_si91x_gpio_set_pin_direction(); \n
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  pin - The pin to set.
 * @return     None
 ******************************************************************************/
static __INLINE void sl_gpio_clear_pin_output(sl_gpio_port_t port, uint8_t pin)
{
  SL_GPIO_ASSERT(SL_GPIO_VALIDATE_PORT(port));
  if ((port == SL_GPIO_PORT_A) || (port == SL_GPIO_PORT_B) || (port == SL_GPIO_PORT_C) || (port == SL_GPIO_PORT_D)) {
    SL_GPIO_ASSERT(SL_GPIO_NDEBUG_PORT_PIN(port, pin));
    GPIO->PIN_CONFIG[(port * MAX_GPIO_PORT_PIN) + pin].BIT_LOAD_REG = CLR;
  } else if (port == SL_GPIO_ULP_PORT) {
    SL_GPIO_ASSERT(SL_GPIO_VALIDATE_ULP_PORT_PIN(port, pin));
    ULP_GPIO->PIN_CONFIG[pin].BIT_LOAD_REG = CLR;
  } else {
    SL_GPIO_ASSERT(SL_GPIO_VALIDATE_UULP_PORT_PIN(port, pin));
    // Set pin in UULP GPIO instance by controlling pin value
    UULP_GPIO->NPSS_GPIO_CNTRL[pin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_OUT = CLR;
  }
}

/***************************************************************************/ /**
 * @brief  Clear bits in configuration register for a port to 0.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 * @pre   \ref sl_gpio_set_pin_mode() \n
 * @pre   \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  pins - The GPIO pins in a port to clear.
 * @return     None
 ******************************************************************************/
static __INLINE void sl_gpio_clear_port_output(sl_gpio_port_t port, uint32_t pins)
{
  SL_GPIO_ASSERT(SL_GPIO_VALIDATE_PORT(port));
  if (port == SL_GPIO_ULP_PORT) {
    ULP_GPIO->PORT_CONFIG[SL_GPIO_PORT_A].PORT_CLEAR_REG = (pins);
  } else {
    GPIO->PORT_CONFIG[port].PORT_CLEAR_REG = (pins);
  }
}

/***************************************************************************/ /**
 * @brief  Read the pin value for a single pin in a GPIO port.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 * @pre   \ref sl_gpio_set_pin_mode() \n
 * @pre   \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  pin  - The pin number on the port.
 *                HP instance has total 57 GPIO pins. Port 0, 1, 2 has 16 pins each.
 *                Port 3 has 9 pins.
 *                ULP instance has total 12 pins.
 * @return The GPIO pin value
 *             '0' - Output\n
 *             '1' - Input\n
 ******************************************************************************/
static __INLINE uint8_t sl_gpio_get_pin_input(sl_gpio_port_t port, uint8_t pin)
{
  SL_GPIO_ASSERT(SL_GPIO_VALIDATE_PORT(port));
  if ((port == SL_GPIO_PORT_A) || (port == SL_GPIO_PORT_B) || (port == SL_GPIO_PORT_C) || (port == SL_GPIO_PORT_D)) {
    SL_GPIO_ASSERT(SL_GPIO_NDEBUG_PORT_PIN(port, pin));
    return (uint8_t)GPIO->PIN_CONFIG[(port * MAX_GPIO_PORT_PIN) + pin].BIT_LOAD_REG;
  } else if (port == SL_GPIO_ULP_PORT) {
    SL_GPIO_ASSERT(SL_GPIO_VALIDATE_ULP_PORT_PIN(port, pin));
    return (uint8_t)ULP_GPIO->PIN_CONFIG[pin].BIT_LOAD_REG;
  } else {
    SL_GPIO_ASSERT(SL_GPIO_VALIDATE_UULP_PORT_PIN(port, pin));
    // Read pin status in UULP GPIO instance
    return (UULP_GPIO_STATUS >> pin) & MASK_INTR;
  }
}

/***************************************************************************/ /**
 * @brief  Get the current pin value of selected pin in a GPIO port.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 * @pre   \ref sl_gpio_set_pin_mode() \n
 * @pre   \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in] pin  - The pin to get setting for.
 * @return The GPIO pin value
 *             '0' - Output\n
 *             '1' - Input\n
 ******************************************************************************/
static __INLINE uint8_t sl_gpio_get_pin_output(sl_gpio_port_t port, uint8_t pin)
{
  SL_GPIO_ASSERT(SL_GPIO_VALIDATE_PORT(port));
  if (port == SL_GPIO_ULP_PORT) {
    SL_GPIO_ASSERT(SL_GPIO_VALIDATE_ULP_PORT_PIN(port, pin));
    return (uint8_t)ULP_GPIO->PIN_CONFIG[pin].BIT_LOAD_REG;
  } else {
    SL_GPIO_ASSERT(SL_GPIO_NDEBUG_PORT_PIN(port, pin));
    return (uint8_t)GPIO->PIN_CONFIG[(port * MAX_GPIO_PORT_PIN) + pin].BIT_LOAD_REG;
  }
}

/***************************************************************************/ /**
 * @brief  Read the port value for GPIO port.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 * @pre   \ref sl_gpio_set_pin_mode() \n
 * @pre   \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @return The pin values of the GPIO port.
 ******************************************************************************/
static __INLINE uint32_t sl_gpio_get_port_input(sl_gpio_port_t port)
{
  SL_GPIO_ASSERT(SL_GPIO_VALIDATE_PORT(port));
  if (port == SL_GPIO_ULP_PORT) {
    return ULP_GPIO->PORT_CONFIG[SL_GPIO_PORT_A].PORT_READ_REG & PORT_MASK;
  } else {
    return GPIO->PORT_CONFIG[port].PORT_READ_REG & PORT_MASK;
  }
}

/***************************************************************************/ /**
 * @brief Get the current pin values of a selected GPIO Port register.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 * @pre   \ref sl_gpio_set_pin_mode() \n 
 * @pre   \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @return    The port value for the requested port.
 ******************************************************************************/
static __INLINE uint32_t sl_gpio_get_port_output(sl_gpio_port_t port)
{
  SL_GPIO_ASSERT(SL_GPIO_VALIDATE_PORT(port));
  if (port == SL_GPIO_ULP_PORT) {
    return (ULP_GPIO->PORT_CONFIG[SL_GPIO_PORT_A].PORT_READ_REG & PORT_MASK);
  } else {
    return (GPIO->PORT_CONFIG[port].PORT_READ_REG & PORT_MASK);
  }
}

/***************************************************************************/ /**
 * @brief  Toggle a single pin in selected GPIO port register.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 * @pre   \ref sl_gpio_set_pin_mode() \n
 * @pre   \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  pin  - The pin number on the port.
 *                HP instance has total 57 GPIO pins. Port 0, 1, 2 has 16 pins each.
 *                Port 3 has 9 pins.
 *                ULP instance has total 12 pins.
 * @return     None
 ******************************************************************************/
static __INLINE void sl_gpio_toggle_pin_output(sl_gpio_port_t port, uint8_t pin)
{
  SL_GPIO_ASSERT(SL_GPIO_VALIDATE_PORT(port));
  if ((port == SL_GPIO_PORT_A) || (port == SL_GPIO_PORT_B) || (port == SL_GPIO_PORT_C) || (port == SL_GPIO_PORT_D)) {
    SL_GPIO_ASSERT(SL_GPIO_NDEBUG_PORT_PIN(port, pin));
    GPIO->PIN_CONFIG[(port * MAX_GPIO_PORT_PIN) + pin].BIT_LOAD_REG ^= SET;
  } else if (port == SL_GPIO_ULP_PORT) {
    SL_GPIO_ASSERT(SL_GPIO_VALIDATE_ULP_PORT_PIN(port, pin));
    ULP_GPIO->PIN_CONFIG[pin].BIT_LOAD_REG ^= SET;
  } else {
    SL_GPIO_ASSERT(SL_GPIO_VALIDATE_UULP_PORT_PIN(port, pin));
    // Set or Clear pin in UULP GPIO instance by controlling pin value
    UULP_GPIO->NPSS_GPIO_CNTRL[pin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_OUT ^= SET;
  }
}

/***************************************************************************/ /**
 * @brief  Toggle selected pins in GPIO port register.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 * @pre   \ref sl_gpio_set_pin_mode() \n
 * @pre   \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  pins - Port pins to toggle.
 * @return     None
 ******************************************************************************/
static __INLINE void sl_gpio_toggle_port_output(sl_gpio_port_t port, uint32_t pins)
{
  SL_GPIO_ASSERT(SL_GPIO_VALIDATE_PORT(port));
  if (port == SL_GPIO_ULP_PORT) {
    ULP_GPIO->PORT_CONFIG[SL_GPIO_PORT_A].PORT_TOGGLE_REG = pins;
  } else {
    GPIO->PORT_CONFIG[port].PORT_TOGGLE_REG = pins;
  }
}

/***************************************************************************/ /**
 * @brief  Enable MCU HP instance GPIO pin interrupts.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_gpio_set_pin_mode() \n
 * @pre   \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  flags  - GPIO interrupt sources to enable.
 * @return     None
 ******************************************************************************/
static __INLINE void sl_gpio_enable_interrupts(uint32_t flags)
{
  uint16_t intch, int_flag;
  intch    = (uint16_t)((flags >> WORD_SHIFT) & LSB_WORD_MASK);
  int_flag = (uint16_t)(flags & INTERRUPT_MASK);
  if ((int_flag & SL_GPIO_INTERRUPT_RISE_EDGE) == SL_GPIO_INTERRUPT_RISE_EDGE) {
    GPIO->INTR[intch].GPIO_INTR_CTRL_b.RISE_EDGE_ENABLE = SET;
  }
  if ((int_flag & SL_GPIO_INTERRUPT_FALL_EDGE) == SL_GPIO_INTERRUPT_FALL_EDGE) {
    GPIO->INTR[intch].GPIO_INTR_CTRL_b.FALL_EDGE_ENABLE = SET;
  }
  if ((int_flag & SL_GPIO_INTERRUPT_LEVEL_HIGH) == SL_GPIO_INTERRUPT_LEVEL_HIGH) {
    GPIO->INTR[intch].GPIO_INTR_CTRL_b.LEVEL_HIGH_ENABLE = SET;
  }
  if ((int_flag & SL_GPIO_INTERRUPT_LEVEL_LOW) == SL_GPIO_INTERRUPT_LEVEL_LOW) {
    GPIO->INTR[intch].GPIO_INTR_CTRL_b.LEVEL_LOW_ENABLE = SET;
  }
}

/***************************************************************************/ /**
 * @brief  Disable MCU HP Instance GPIO pin interrupts.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_gpio_set_pin_mode() \n
 * @pre   \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  flags  -  GPIO interrupt sources to disable.
 * @return     None
 ******************************************************************************/
static __INLINE void sl_gpio_disable_interrupts(uint32_t flags)
{
  uint16_t intch, int_flag;
  intch    = (uint16_t)((flags >> WORD_SHIFT) & LSB_WORD_MASK);
  int_flag = (uint16_t)(flags & LSB_NIBBLE_MASK);
  if ((int_flag & SL_GPIO_INTERRUPT_RISE_EDGE) == SL_GPIO_INTERRUPT_RISE_EDGE) {
    GPIO->INTR[intch].GPIO_INTR_CTRL_b.RISE_EDGE_ENABLE = CLR;
  }
  if ((int_flag & SL_GPIO_INTERRUPT_FALL_EDGE) == SL_GPIO_INTERRUPT_FALL_EDGE) {
    GPIO->INTR[intch].GPIO_INTR_CTRL_b.FALL_EDGE_ENABLE = CLR;
  }
  if ((int_flag & SL_GPIO_INTERRUPT_LEVEL_HIGH) == SL_GPIO_INTERRUPT_LEVEL_HIGH) {
    GPIO->INTR[intch].GPIO_INTR_CTRL_b.LEVEL_HIGH_ENABLE = CLR;
  }
  if ((int_flag & SL_GPIO_INTERRUPT_LEVEL_LOW) == SL_GPIO_INTERRUPT_LEVEL_LOW) {
    GPIO->INTR[intch].GPIO_INTR_CTRL_b.LEVEL_LOW_ENABLE = CLR;
  }
}

/***************************************************************************/ /**
 * @brief   Clear MCU HP Instance GPIO pin interrupts.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_gpio_set_pin_mode() \n
 * @pre   \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  flags  -  Bitwise logic OR of GPIO interrupt sources to clear.
 * @return     None
 ******************************************************************************/
static __INLINE void sl_gpio_clear_interrupts(uint32_t flags)
{
  uint32_t reg_data = GPIO->INTR[flags].GPIO_INTR_STATUS;
  if (reg_data != 0)
    GPIO->INTR[flags].GPIO_INTR_STATUS = INTR_CLR;
}

/**************************************************************************/ /**
 * @brief  Set MCU HP Instance GPIO pin interrupts.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_gpio_set_pin_mode() \n
 * @pre   \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  flags  -  GPIO interrupt sources to set to pending.
 * @return     None
 *****************************************************************************/
static __INLINE void sl_gpio_set_interrupts(uint32_t flags)
{
  uint16_t intch, int_flag;
  intch                            = (uint16_t)((flags >> WORD_SHIFT) & LSB_WORD_MASK);
  int_flag                         = (uint16_t)(flags & INTERRUPT_MASK);
  GPIO->INTR[intch].GPIO_INTR_CTRL = (GPIO->INTR[intch].GPIO_INTR_CTRL | int_flag);
}

/***************************************************************************/ /**
 * @brief  Get pending MCU HP Instance GPIO pin interrupts.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_gpio_set_pin_mode() \n
 * @pre   \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  None
 * @return GPIO interrupt sources pending.
 ******************************************************************************/
static __INLINE uint32_t sl_gpio_get_pending_interrupts(void)
{
  uint32_t intflags = 0, tmp = 0;
  for (uint8_t intch = 0; intch < MAX_GPIO_PIN_INT; intch++) {
    tmp = (GPIO->INTR[intch].GPIO_INTR_STATUS & LSB_NIBBLE_MASK);
    intflags |= (tmp << (NIBBLE_SHIFT * intch));
  }
  return intflags;
}

/***************************************************************************/ /**
 * @brief  Get enabled MCU HP Instance GPIO pin interrupts.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_gpio_set_pin_mode() \n
 * @pre   \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  None
 * @return Returns enabled GPIO interrupt sources.
 ******************************************************************************/
static __INLINE uint32_t sl_gpio_get_enabled_interrupts(void)
{
  uint32_t intflags = 0, tmp = 0;
  for (uint8_t intch = 0; intch < MAX_GPIO_PIN_INT; intch++) {
    tmp = (GPIO->INTR[intch].GPIO_INTR_CTRL & LSB_NIBBLE_MASK);
    intflags |= (tmp << (NIBBLE_SHIFT * intch));
  }
  return intflags;
}

/***************************************************************************/ /**
 * @brief  Get enabled and pending GPIO pin interrupt of MCU HP Instance.
 *     Useful for handling more interrupt sources in the same interrupt handler.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @pre   \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @pre   \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 * @pre   \ref sl_gpio_set_pin_mode() \n
 * @pre   \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  None
 * @return     Returns pending GPIO interrupt sources
 ******************************************************************************/
static __INLINE uint32_t sl_gpio_get_enabled_pending_interrupts(void)
{
  uint32_t intflags = 0, tmp_ctrl = 0, tmp_sts = 0;
  uint8_t intch = 0;
  for (intch = 0; intch < MAX_GPIO_PIN_INT; intch++) {
    tmp_ctrl = (GPIO->INTR[intch].GPIO_INTR_CTRL & INTERRUPT_MASK);
    if (tmp_ctrl & MASK_CTRL) {
      tmp_ctrl = (tmp_ctrl >> 1) | MASK_INTR;
    }
    tmp_sts = ((GPIO->INTR[intch].GPIO_INTR_STATUS) & (tmp_ctrl));
    tmp_sts = (tmp_sts & LSB_NIBBLE_MASK);
    intflags |= (tmp_sts << (NIBBLE_SHIFT * intch));
  }
  return intflags;
}

/**************************************************************************/ /**
 * @brief        This API is used set ulp soc gpio mode
 *               \n(Gpio pin mode,ranges 000 -> Mode 0 to 111 -> Mode 7 Used for GPIO Pin Muxing )
 * @param[in]    ulp_gpio : ulp gpio number
 * @param[in]    mode     : GPIO mode
 *               \n possible values for this parameter are the following
 *               - \ref EGPIO_PIN_MUX_MODE0   : Select pin mode 0
 *               - \ref EGPIO_PIN_MUX_MODE1   : Select pin mode 1
 *               - \ref EGPIO_PIN_MUX_MODE2   : Select pin mode 2
 *               - \ref EGPIO_PIN_MUX_MODE3   : Select pin mode 3
 *               - \ref EGPIO_PIN_MUX_MODE4   : Select pin mode 4
 *               - \ref EGPIO_PIN_MUX_MODE5   : Select pin mode 5
 *               - \ref EGPIO_PIN_MUX_MODE6   : Select pin mode 6
 *               - \ref EGPIO_PIN_MUX_MODE7   : Select pin mode 7
 * @return       None
 ******************************************************************************/
static __INLINE void sl_si91x_gpio_ulp_soc_mode(uint8_t ulp_gpio, uint8_t mode)

{
  ULPCLK->ULP_SOC_GPIO_MODE_REG[ulp_gpio].ULP_SOC_GPIO_MODE_REG_b.ULP_SOC_GPIO_MODE_REG = (unsigned int)(mode & 0x07);
}

/** @} (end addtogroup GPIO) */

#ifdef __cplusplus
}
#endif

#endif ///< GPIO_PRESENT
#endif ///< SL_PERIPHERAL_GPIO_H
