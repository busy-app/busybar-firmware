/******************************************************************************
* @file sl_si91x_gpio.h
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
#ifndef SL_SI91X_GPIO_PERIPHERAL_H
#define SL_SI91X_GPIO_PERIPHERAL_H

#ifdef __cplusplus
extern "C" {
#endif

//// Includes
#include "sl_si91x_gpio_common.h"

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/
///@brief NPSS (or UULP) GPIO PIN MUX
typedef enum {
  NPSS_GPIO_PIN_MUX_MODE0,  ///< UULP Pin MUX Mode 0
  NPSS_GPIO_PIN_MUX_MODE1,  ///< UULP Pin MUX Mode 1
  NPSS_GPIO_PIN_MUX_MODE2,  ///< UULP Pin MUX Mode 2
  NPSS_GPIO_PIN_MUX_MODE3,  ///< UULP Pin MUX Mode 3
  NPSS_GPIO_PIN_MUX_MODE4,  ///< UULP Pin MUX Mode 4
  NPSS_GPIO_PIN_MUX_MODE5,  ///< UULP Pin MUX Mode 5
  NPSS_GPIO_PIN_MUX_MODE6,  ///< UULP Pin MUX Mode 6
  NPSS_GPIO_PIN_MUX_MODE7,  ///< UULP Pin MUX Mode 7
  NPSS_GPIO_PIN_MUX_MODE8,  ///< UULP Pin MUX Mode 8
  NPSS_GPIO_PIN_MUX_MODE9,  ///< UULP Pin MUX Mode 9
  NPSS_GPIO_PIN_MUX_MODE10, ///< UULP Pin MUX Mode 10
} sl_si91x_uulp_npss_mode_t;

///@brief GPIO group interrupts
typedef enum {
  GROUP_INT_1 = 0, ///< GPIO group interrupt 1
  GROUP_INT_2 = 1, ///< GPIO group interrupt 2
} sl_si91x_group_interrupt_t;

///@brief GPIO Interrupt Configurations.
typedef enum {
  SL_GPIO_INTERRUPT_NONE       = 0,        ///< interrupt disable
  SL_GPIO_INTERRUPT_LEVEL_HIGH = (1 << 0), ///< interrupt when pin level is '1'
  SL_GPIO_INTERRUPT_LEVEL_LOW  = (1 << 1), ///< interrupt when pin level is '0'
  SL_GPIO_INTERRUPT_RISE_EDGE  = (1 << 2), ///< interrupt when rising edge is detected
  SL_GPIO_INTERRUPT_FALL_EDGE  = (1 << 3), ///< interrupt when falling edge is detected
} sl_si91x_gpio_interrupt_config_flag_t;

///@brief GPIO driver disable state
typedef enum {
  GPIO_HZ = 0,   ///<  0 for Hi-impedance (P1=0,P2=0)
  GPIO_PULLUP,   ///<  1 for Pull up (P1=0,P2=1)
  GPIO_PULLDOWN, ///<  2 for Pull down(P1=1,P2=0)
  GPIO_REPEATER  ///<  3 for Repeater (P1=1,P2=1)
} sl_si91x_gpio_driver_disable_state_t;

///@brief GPIO driver strength select
typedef enum {
  GPIO_TWO_MILLI_AMPS = 0, ///<  0 for two_milli_amps   (E1=0,E2=0)
  GPIO_FOUR_MILLI_AMPS,    ///<  1 for four_milli_amps  (E1=0,E2=1)
  GPIO_EIGHT_MILLI_AMPS,   ///<  2 for eight_milli_amps (E1=1,E2=0)
  GPIO_TWELVE_MILLI_AMPS   ///<  3 for twelve_milli_amps(E1=1,E2=1)
} sl_si91x_gpio_driver_strength_select_t;

///@brief HP/ULP GPIO clock select
typedef enum {
  M4CLK_GPIO  = 0, ///<  0 for HP GPIO clock
  ULPCLK_GPIO = 1  ///<  1 for ULP GPIO clock
} sl_si91x_gpio_select_clock_t;

///@brief Direction of the GPIO pin enum
typedef enum {
  GPIO_OUTPUT = 0, ///< GPIO direction output
  GPIO_INPUT  = 1, ///< GPIO direction input
} sl_si91x_gpio_direction_t;

///@brief UULP GPIO PAD select enum
typedef enum {
  GPIO_PAD_M4 = 0, ///< GPIO M4 PAD selection
  GPIO_PAD_TA = 1, ///< GPIO NWP PAD selection
} sl_si91x_gpio_uulp_pad_t;

///@brief AND/OR of the GPIO group interrupt
typedef enum {
  GPIO_AND = 0, ///< GPIO AND group interrupt
  GPIO_OR  = 1, ///< GPIO OR group interrupt
} sl_si91x_gpio_and_or_t;

///@brief GPIO group interrupt wakeup flag
typedef enum {
  GPIO_FLAG_DS = 0, ///< wakeup flag disable
  GPIO_FLAG_EN = 1, ///< wakeup flag enable
} sl_si91x_gpio_wakeup_t;

///@brief GPIO polarity enum
typedef enum {
  GPIO_POLARITY_0 = 0, ///< GPIO polarity 0
  GPIO_POLARITY_1 = 1, ///< GPIO polarity 1
} sl_si91x_gpio_polarity_t;

///@brief GPIO level edge select
typedef enum {
  GPIO_LEVEL = 0, ///< GPIO level trigger
  GPIO_EDGE  = 1, ///< GPIO edge trigger
} sl_si91x_gpio_level_edge_t;

///@brief GPIO slew rate select
typedef enum {
  GPIO_SR_LOW  = 0, ///< GPIO slew rate low
  GPIO_SR_HIGH = 1, ///< GPIO slew rate high
} sl_si91x_gpio_slew_rate_t;

///@brief GPIO pin set/clear
typedef enum {
  GPIO_PIN_CLEAR = 0, ///< GPIO clear pin
  GPIO_PIN_SET   = 1, ///< GPIO set pin
} sl_si91x_gpio_pin_value_t;

///@brief UULP GPIO input buffer
typedef enum {
  GPIO_RECEIVER_DS = 0, ///< receiver disable
  GPIO_RECEIVER_EN = 1, ///< receiver enable
} sl_si91x_gpio_receiver_t;

///@brief UULP GPIO PAD configuration register fields
typedef struct {
  uint8_t gpio_padnum;                 ///< UULP GPIO pin number
  sl_si91x_uulp_npss_mode_t mode;      ///< UULP GPIO mode
  sl_si91x_gpio_receiver_t receiver;   ///< UULP GPIO PAD receiver
  sl_si91x_gpio_direction_t direction; ///< UULP GPIO direction of PAD
  sl_si91x_gpio_pin_value_t output;    ///< UULP GPIO value driven on PAD
  sl_si91x_gpio_uulp_pad_t pad_select; ///< UULP GPIO PAD selection
  sl_si91x_gpio_polarity_t polarity;   ///< UULP GPIO Polarity
} uulp_pad_config_t;

///@brief GPIO Group Interrupt Configuration. It selects random ports and pins.
typedef struct {
  sl_si91x_group_interrupt_t grp_interrupt;     ///< configure group interrupt
  uint8_t grp_interrupt_cnt;                    ///< Count of group interrupt pins
  uint8_t grp_interrupt_port[MAX_GPIO_PIN_INT]; ///< ports used for group interrupts
  uint8_t grp_interrupt_pin[MAX_GPIO_PIN_INT];  ///< pins used for group interrupts
  uint8_t grp_interrupt_pol[MAX_GPIO_PIN_INT];  ///< polarity used for interrupts
  sl_si91x_gpio_level_edge_t level_edge;        ///< configure level or edge trigger
  sl_si91x_gpio_and_or_t and_or;                ///< AND/OR ing of interrupts
} sl_si91x_gpio_group_interrupt_config_t;

///@brief GPIO pin numbers
typedef enum {
  GPIO_PIN_NUMBER0  = 0,  ///< GPIO pin number  0
  GPIO_PIN_NUMBER1  = 1,  ///< GPIO pin number  1
  GPIO_PIN_NUMBER2  = 2,  ///< GPIO pin number  2
  GPIO_PIN_NUMBER3  = 3,  ///< GPIO pin number  3
  GPIO_PIN_NUMBER4  = 4,  ///< GPIO pin number  4
  GPIO_PIN_NUMBER5  = 5,  ///< GPIO pin number  5
  GPIO_PIN_NUMBER6  = 6,  ///< GPIO pin number  6
  GPIO_PIN_NUMBER7  = 7,  ///< GPIO pin number  7
  GPIO_PIN_NUMBER8  = 8,  ///< GPIO pin number  8
  GPIO_PIN_NUMBER9  = 9,  ///< GPIO pin number  9
  GPIO_PIN_NUMBER10 = 10, ///< GPIO pin number  10
  GPIO_PIN_NUMBER11 = 11, ///< GPIO pin number  11
  GPIO_PIN_NUMBER12 = 12, ///< GPIO pin number  12
  GPIO_PIN_NUMBER13 = 13, ///< GPIO pin number  13
  GPIO_PIN_NUMBER14 = 14, ///< GPIO pin number  14
  GPIO_PIN_NUMBER15 = 15, ///< GPIO pin number  15
} sl_si91x_gpio_pin_t;

///@brief GPIO ULP pin numbers
typedef enum {
  ULP_GPIO_PIN_0  = 0,  ///< ULP GPIO pin number 0
  ULP_GPIO_PIN_1  = 1,  ///< ULP GPIO pin number 1
  ULP_GPIO_PIN_2  = 2,  ///< ULP GPIO pin number 2
  ULP_GPIO_PIN_3  = 3,  ///< ULP GPIO pin number 3
  ULP_GPIO_PIN_4  = 4,  ///< ULP GPIO pin number 4
  ULP_GPIO_PIN_5  = 5,  ///< ULP GPIO pin number 5
  ULP_GPIO_PIN_6  = 6,  ///< ULP GPIO pin number 6
  ULP_GPIO_PIN_7  = 7,  ///< ULP GPIO pin number 7
  ULP_GPIO_PIN_8  = 8,  ///< ULP GPIO pin number 8
  ULP_GPIO_PIN_9  = 9,  ///< ULP GPIO pin number 9
  ULP_GPIO_PIN_10 = 10, ///< ULP GPIO pin number 10
  ULP_GPIO_PIN_11 = 11, ///< ULP GPIO pin number 11
} sl_si91x_gpio_pin_ulp_t;

///@brief GPIO instances
typedef enum {
  M4_GPIO_INSTANCE   = 1, ///<  1 for HP GPIO
  ULP_GPIO_INSTANCE  = 2, ///<  2 for ULP GPIO
  UULP_GPIO_INSTANCE = 3, ///<  3 for UULP GPIO
  GPIO_INSTANCE_LAST,     ///< Last enum for validating
} sl_si91x_gpio_instances_t;

///@brief GPIO interrupt type
typedef enum {
  GPIO_PIN_INTERRUPT,   ///<  0 for GPIO pin interrupt
  GPIO_GROUP_INTERRUPT, ///<  1 for GPIO group interrupt
} sl_si91x_gpio_intr_t;

/// @brief Structure to hold the versions of peripheral API
typedef struct {
  uint8_t release; ///< Release version number
  uint8_t major;   ///< SQA version number
  uint8_t minor;   ///< Development version number
} sl_si91x_gpio_version_t;

/// @brief GPIO power-on start enable selection
typedef enum {
  GPIO_POS_DIS = 0, ///< power-on-start disable
  GPIO_POS_EN  = 1, ///< power-on-start enable
  GPIO_POS_LAST,    ///< Last enum for validating
} sl_si91x_gpio_pos_t;

/// @brief GPIO Schmitt trigger selection
typedef enum {
  GPIO_SCHMITT_TRIG_DIS = 0, ///< Schmitt trigger disable
  GPIO_SCHMITT_TRIG_EN  = 1, ///< Schmitt trigger enable
  GPIO_SCHMITT_TRIG_LAST,    ///< Last enum for validating
} sl_si91x_gpio_schmitt_trig_t;

// -----------------------------------------------------------------------------
// Prototypes
/***************************************************************************/ /**
 * @brief   Set the direction for a GPIO pin from the selected port.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. 
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  pin  - The pin number on the port.
 *                HP instance has total 57 GPIO pins. Port 0, 1, 2 has 16 pins each.
 *                Port 3 has 9 pins.
 *                ULP instance has total 12 pins.
 * @param[in]  direction  -  pin direction of type \ref sl_si91x_gpio_direction_t
 *                 '0' - Output\n
 *                 '1' - Input\n
 * @return     None
*******************************************************************************/
void sl_si91x_gpio_set_pin_direction(uint8_t port, uint8_t pin, sl_si91x_gpio_direction_t direction);

/***************************************************************************/ /**
 * @brief       Get the direction of a GPIO pin from selected port.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_pad_selection, for HP instance \n
 *        \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 *        \ref sl_si91x_gpio_set_pin_direction() \n
 *        \ref sl_si91x_gpio_get_pin_direction() \n
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  pin  - The pin number on the port.
 *                HP instance has total 57 GPIO pins. Port 0, 1, 2 has 16 pins each.
 *                Port 3 has 9 pins.
 *                ULP instance has total 12 pins.
 * @return      Returns the direction of the pin.
 *                  '0' - Output\n
 *                  '1' - Input\n
 ******************************************************************************/
uint8_t sl_si91x_gpio_get_pin_direction(uint8_t port, uint8_t pin);

/***************************************************************************/ /**
 * @brief      Enable the receiver enable bit in the PAD configuration register.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @param[in]  gpio_num - GPIO pin number to be use.
 * @return     None
*******************************************************************************/
void sl_si91x_gpio_enable_pad_receiver(uint8_t gpio_num);

/***************************************************************************/ /**
 * @brief      Disable the receiver enable bit in the PAD configuration register.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 * @param[in]  gpio_num - GPIO pin number to be use.
 * @return     None
*******************************************************************************/
void sl_si91x_gpio_disable_pad_receiver(uint8_t gpio_num);

/***************************************************************************/ /**
 * @brief   Select the pad enable for HP instance of GPIO pins
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @param[in]   gpio_padnum - PAD number to be use(0 to 21).
 * @return      None
*******************************************************************************/
void sl_si91x_gpio_enable_pad_selection(uint8_t gpio_padnum);

/***************************************************************************/ /**
 * @brief   Select the host pad enable for HP instance of GPIO pins
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @param[in]   gpio_num - GPIO pin number(25 to 30).
 * @return      None
*******************************************************************************/
void sl_si91x_gpio_enable_host_pad_selection(uint8_t gpio_num);

/***************************************************************************/ /**
 * @brief     Select drive strength of a GPIO pin for selected port.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_pad_selection() \n
 *        \ref sl_si91x_gpio_enable_pad_receiver() \n
 * @param[in]    gpio_num - GPIO pin number to be use
 * @param[in]    strength    - Drive strength selector(E1,E2) of type
 *                  \ref sl_si91x_gpio_driver_strength_select_t
 *                    possible values are
 *                      0, for two_milli_amps   (E1=0,E2=0)\n
 *                      1, for four_milli_amps  (E1=0,E2=1)\n
 *                      2, for eight_milli_amps (E1=1,E2=0)\n
 *                      3, for twelve_milli_amps(E1=1,E2=1)\n
 * @return       None
 ******************************************************************************/
void sl_si91x_gpio_select_pad_driver_strength(uint8_t gpio_num, sl_si91x_gpio_driver_strength_select_t strength);

/***************************************************************************/ /**
 * @brief     Configure the power-on-start selection for a GPIO pin.
 * @pre       \ref sl_si91x_gpio_enable_clock() \n
 *            \ref sl_si91x_gpio_enable_pad_selection() \n
 *            \ref sl_si91x_gpio_enable_pad_receiver() \n
 * @param[in] gpio_num - GPIO pin number to configure
 * @param[in] pos      - Power-on start level of type \ref sl_si91x_gpio_pos_t \n
 *                       Possible values are: \n
 *                         GPIO_POS_DIS  - power-on-start disable \n
 *                         GPIO_POS_EN - power-on-start enable \n
 * @return    None
 ******************************************************************************/
void sl_si91x_gpio_select_pad_power_on_start(uint8_t gpio_num, sl_si91x_gpio_pos_t pos);

/***************************************************************************/ /**
 * @brief     Configure the Schmitt trigger selection for a GPIO pin.
 * @pre       \ref sl_si91x_gpio_enable_clock() \n
 *            \ref sl_si91x_gpio_enable_pad_selection() \n
 *            \ref sl_si91x_gpio_enable_pad_receiver() \n
 * @param[in] gpio_num     - GPIO pin number to configure
 * @param[in] schmitt_trig - Schmitt trigger level of type \ref sl_si91x_gpio_schmitt_trig_t \n
 *                            Possible values are: \n
 *                              GPIO_SCHMITT_TRIG_DIS  - Schmitt trigger disable\n
 *                              GPIO_SCHMITT_TRIG_EN - Schmitt trigger enable \n
 * @return    None
 ******************************************************************************/
void sl_si91x_gpio_select_pad_active_high_schmitt_trigger(uint8_t gpio_num, sl_si91x_gpio_schmitt_trig_t schmitt_trig);

/***************************************************************************/ /**
 * @brief    Select the Driver disabled state control of a HP instance GPIO pin.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_pad_selection() \n
 *        \ref sl_si91x_gpio_enable_pad_receiver() \n
 * @param[in]    gpio_num     -  GPIO pin number to be use
 * @param[in]    disable_state    -  driver disable state of type
 *                  \ref sl_si91x_gpio_driver_disable_state_t
 *                 possible values are
 *                    0, for HiZ       (P1=0,P2=0)\n
 *                    1, for Pull-up   (P1=0,P2=1)\n
 *                    2, for Pull-down (P1=1,P2=0)\n
 *                    3, for Repeater  (P1=1,P2=1)\n
 * @return       None
 ******************************************************************************/
void sl_si91x_gpio_select_pad_driver_disable_state(uint8_t gpio_num,
                                                   sl_si91x_gpio_driver_disable_state_t disable_state);

/***************************************************************************/ /**
 * @brief Select AND/OR of the group interrupt. If multiple interrupts
 *    on same port (or) different are to be generated, then use this API.
 *   Example: Consider port 0: pin 2,3 and port 3: pin 1,2 for interrupt generation.
 *      Choose OR, any of the selected pin is fine for group interrupt generation
 *      Choose AND, all the selected pins are necessary for group interrupt generation
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance.
 *        \ref sl_gpio_set_pin_mode() \n
 *        \ref sl_si91x_gpio_set_pin_direction() \n
 *        \ref sl_si91x_gpio_configure_group_interrupt(), for HP instance \n
 *        \ref sl_si91x_gpio_configure_ulp_group_interrupt, for ULP instance \n
 *  Use corresponding group interrupt configuration API for corresponding GPIO instance.
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  group_interrupt - Group interrupt number of type
 *                            \ref sl_si91x_group_interrupt_t
 * @param[in]  and_or  - AND/OR of GPIO group interrupts of type
 *                    \ref sl_si91x_gpio_and_or_t
 *                          '0'  - AND\n
 *                          '1'  - OR\n
 * @return     None
*******************************************************************************/
void sl_si91x_gpio_select_group_interrupt_and_or(uint8_t port,
                                                 sl_si91x_group_interrupt_t group_interrupt,
                                                 sl_si91x_gpio_and_or_t and_or);

/***************************************************************************/ /**
 * @brief        Clear the selected group interrupt status for HP instance GPIO pins.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_pad_selection() \n
 *        \ref sl_si91x_gpio_enable_pad_receiver() \n
 *        \ref sl_gpio_set_pin_mode() \n
 *        \ref sl_si91x_gpio_set_pin_direction() \n
 *        \ref sl_si91x_gpio_configure_group_interrupt() \n
 * @param[in]    group_interrupt - Group interrupt number of type
 *                        \ref sl_si91x_group_interrupt_t
 * @return       None
*******************************************************************************/
void sl_si91x_gpio_clear_group_interrupt(sl_si91x_group_interrupt_t group_interrupt);

/***************************************************************************/ /**
 * @brief     Get the group interrupt status of selected instance.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 *        \ref sl_gpio_set_pin_mode(),
 *        \ref sl_si91x_gpio_set_pin_direction() \n
 *        \ref sl_si91x_gpio_configure_group_interrupt(), for HP instance \n
 *        \ref sl_si91x_gpio_configure_ulp_group_interrupt, for ULP instance \n
 *  Use corresponding group interrupt configuration API for corresponding GPIO instance.
 * @param[in]   port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]    group_interrupt - Group interrupt number of type
 *                        \ref sl_si91x_group_interrupt_t
 * @return       returns the group interrupt status register
 *                        1, when interrupt is enabled\n
 *                        0, when interrupt is disabled\n
 ******************************************************************************/
uint32_t sl_si91x_gpio_get_group_interrupt_status(uint8_t port, sl_si91x_group_interrupt_t group_interrupt);

/***************************************************************************/ /**
 * @brief     Configure the group interrupt as a wake up source across sleep wakeups.
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]    group_interrupt - Group interrupt number of type
 *                            \ref sl_si91x_group_interrupt_t
 * @param[in]    flags   - GPIO group interrupt wake up flag of type
 *                              \ref sl_si91x_gpio_wakeup_t
 *                        '1'  - enable\n
 *                        '0'  - disable\n
 * @return       None
 ******************************************************************************/
void sl_si91x_gpio_select_group_interrupt_wakeup(uint8_t port,
                                                 sl_si91x_group_interrupt_t group_interrupt,
                                                 sl_si91x_gpio_wakeup_t flags);

/***************************************************************************/ /**
 * @brief      Configure the MCU HP instance group interrupts with trigger type(level/edge), polarity(high/low),interrupt type(and/or)
 *             and register the callback function for interrupts.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_pad_selection() \n
 *        \ref sl_si91x_gpio_enable_pad_receiver() \n
 *        \ref sl_gpio_set_pin_mode() \n
 *        \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  configuration - configuration pointer to
 *                      \ref sl_si91x_gpio_group_interrupt_config_t structure
 * @return     None
*******************************************************************************/
void sl_si91x_gpio_configure_group_interrupt(sl_si91x_gpio_group_interrupt_config_t *configuration);

/***************************************************************************/ /**
 * @brief      Get the polarity of selected group interrupt of a HP instance GPIO.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 *        \ref sl_gpio_set_pin_mode() \n
 *        \ref sl_si91x_gpio_set_pin_direction() \n
 *        \ref sl_si91x_gpio_configure_group_interrupt(), for HP instance \n
 *        \ref sl_si91x_gpio_configure_ulp_group_interrupt, for ULP instance \n
 *  Use corresponding group interrupt configuration API for corresponding GPIO instance.
 * @param[in]  group_interrupt - GPIO group interrupt number of type
 *                  \ref  sl_si91x_group_interrupt_t
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  pin  - The pin number on the port.
 *                HP instance has total 57 GPIO pins. Port 0, 1, 2 has 16 pins each.
 *                Port 3 has 9 pins.
 *                ULP instance has total 12 pins.
 * @return     returns group interrupt polarity
 *                      1, when GPIO pin status is '1'\n
 *                      0, when GPIO pin status is '0'\n
 ******************************************************************************/
uint8_t sl_si91x_gpio_get_group_interrupt_polarity(sl_si91x_group_interrupt_t group_interrupt,
                                                   uint8_t port,
                                                   uint8_t pin);

/***************************************************************************/ /**
 * @brief      Configure the polarity of selected group interrupt for HP instance GPIO .
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 *        \ref sl_gpio_set_pin_mode() \n
 *        \ref sl_si91x_gpio_set_pin_direction() \n
 *        \ref sl_si91x_gpio_configure_group_interrupt(), for HP instance \n
 *        \ref sl_si91x_gpio_configure_ulp_group_interrupt, for ULP instance \n
 *  Use corresponding group interrupt configuration API for corresponding GPIO instance.
 * @param[in]  group_interrupt - GPIO group interrupt number of type
 *                  \ref  sl_si91x_group_interrupt_t
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  pin  - The pin number on the port.
 *                HP instance has total 57 GPIO pins. Port 0, 1, 2 has 16 pins each.
 *                Port 3 has 9 pins.
 *                ULP instance has total 12 pins.
 * @param[in]  polarity   -  polarity of GPIO group interrupt of type
 *                      \ref  sl_si91x_gpio_polarity_t
 *               1, group interrupt gets generated when GPIO pin status is '1'\n
 *               0, group interrupt gets generated when GPIO pin status is '0'\n
 * @return     None
 ******************************************************************************/
void sl_si91x_gpio_set_group_interrupt_polarity(sl_si91x_group_interrupt_t group_interrupt,
                                                uint8_t port,
                                                uint8_t pin,
                                                sl_si91x_gpio_polarity_t polarity);

/***************************************************************************/ /**
 * @brief      Get the level/edge event status of selected group interrupt.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 *        \ref sl_gpio_set_pin_mode(), \n
 *        \ref sl_si91x_gpio_set_pin_direction(), \n
 *        \ref sl_si91x_gpio_configure_group_interrupt(), for HP instance \n
 *        \ref sl_si91x_gpio_configure_ulp_group_interrupt, for ULP instance \n
 *  Use corresponding group interrupt configuration API for corresponding GPIO instance.
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  group_interrupt - GPIO group interrupt number of type
 *                    \ref  sl_si91x_group_interrupt_t
 * @return     returns group interrupt level_edge
 *                     1, for Edge\n
 *                     0, for Level\n
 ******************************************************************************/
uint8_t sl_si91x_gpio_get_group_interrupt_level_edge(uint8_t port, sl_si91x_group_interrupt_t group_interrupt);

/***************************************************************************/ /**
 * @brief      Set the level/edge event of group interrupt.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n 
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 *        \ref sl_gpio_set_pin_mode() \n
 *        \ref sl_si91x_gpio_set_pin_direction() \n
 *        \ref sl_si91x_gpio_configure_group_interrupt(), for HP instance \n
 *        \ref sl_si91x_gpio_configure_ulp_group_interrupt, for ULP instance \n
 *  Use corresponding group interrupt configuration API for corresponding GPIO instance.
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  group_interrupt    - GPIO group interrupt number of type
 *                  \ref  sl_si91x_group_interrupt_t
 * @param[in]  level_edge - GPIO level edge group interrupt of type
 *                  \ref  sl_si91x_gpio_level_edge_t
 *                     1, for Edge\n
 *                     0, for Level\n
 * @return     None
 ******************************************************************************/
void sl_si91x_gpio_set_group_interrupt_level_edge(uint8_t port,
                                                  sl_si91x_group_interrupt_t group_interrupt,
                                                  sl_si91x_gpio_level_edge_t level_edge);

/***************************************************************************/ /**
 * @brief      Unmask the selected group interrupt to enable interrupt clearing upon generation.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 *        \ref sl_gpio_set_pin_mode() \n
 *        \ref sl_si91x_gpio_set_pin_direction() \n
 *        \ref sl_si91x_gpio_configure_group_interrupt(), for HP instance \n
 *        \ref sl_si91x_gpio_configure_ulp_group_interrupt, for ULP instance \n
 *  Use corresponding group interrupt configuration API for corresponding GPIO instance.
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  group_interrupt - GPIO group interrupt number of type
 *                    \ref  sl_si91x_group_interrupt_t
 * @return     None
*******************************************************************************/
void sl_si91x_gpio_unmask_group_interrupt(uint8_t port, sl_si91x_group_interrupt_t group_interrupt);

/***************************************************************************/ /**
 * @brief      Mask the selected group interrupts.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 *        \ref sl_gpio_set_pin_mode() \n
 *        \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  group_interrupt - GPIO group interrupt number of type
 *                      \ref sl_si91x_group_interrupt_t
 * @return     None
 *******************************************************************************/
void sl_si91x_gpio_mask_group_interrupt(uint8_t port, sl_si91x_group_interrupt_t group_interrupt);

/***************************************************************************/ /**
 * @brief        Disable the clock for either HP or ULP instance of GPIO Peripheral.
 * @param[in]    clock  - Selects M4 clock or ULP clock of type
 *                    \ref  sl_si91x_gpio_select_clock_t
 *                         0, for M4 GPIO CLK\n
 *                         1, for ULP GPIO CLK\n
 * @return       None
 ******************************************************************************/
void sl_si91x_gpio_disable_clock(sl_si91x_gpio_select_clock_t clock);

/***************************************************************************/ /**
 * @brief    Enable the clock for either HP or ULP instance of GPIO peripheral.
 * @param[in]    clock  - Selects M4 clock or ULP clock of type
 *                    \ref  sl_si91x_gpio_select_clock_t
 *                         0, for M4 GPIO CLK\n
 *                         1, for ULP GPIO CLK\n
 * @return       None
 ******************************************************************************/
void sl_si91x_gpio_enable_clock(sl_si91x_gpio_select_clock_t clock);

/***************************************************************************/ /**
 * @brief     Enable the selected group interrupts for either HP or ULP instance of GPIO peripheral.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 *        \ref sl_gpio_set_pin_mode() \n
 *        \ref sl_si91x_gpio_set_pin_direction() \n
 *        \ref sl_si91x_gpio_configure_group_interrupt(), for HP instance \n
 *        \ref sl_si91x_gpio_configure_ulp_group_interrupt, for ULP instance \n
 *  Use corresponding group interrupt configuration API for corresponding GPIO instance.
 * @param[in]  group_interrupt  - GPIO group interrupt number of type
 *                  \ref sl_si91x_group_interrupt_t
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  pin  - The pin number on the port.
 *                HP instance has total 57 GPIO pins. Port 0, 1, 2 has 16 pins each.
 *                Port 3 has 9 pins.
 *                ULP instance has total 12 pins.
 * @return     None
*******************************************************************************/
void sl_si91x_gpio_enable_group_interrupt(sl_si91x_group_interrupt_t group_interrupt, uint8_t port, uint8_t pin);

/***************************************************************************/ /**
 * @brief      Disable the selected group interrupts for either HP or ULP instance of GPIO peripheral.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_pad_selection(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_pad_receiver(), for HP instance \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver(), for ULP instance \n
 *  Use corresponding pad receiver API for corresponding GPIO instance. \n
 *        \ref sl_gpio_set_pin_mode() \n
 *        \ref sl_si91x_gpio_set_pin_direction() \n
 *        \ref sl_si91x_gpio_configure_group_interrupt(), for HP instance \n
 *        \ref sl_si91x_gpio_configure_ulp_group_interrupt, for ULP instance \n
 *  Use corresponding group interrupt configuration API for corresponding GPIO instance.
 * @param[in]  group_interrupt  - GPIO group interrupt number of type
 *                    \ref  sl_si91x_group_interrupt_t
 * @param[in]  port - The port to associate with the pin.
 *                  HP instance - PORT 0,1,2,3
 *                  ULP instance - PORT 4
 * @param[in]  pin  - The pin number on the port.
 *                HP instance has total 57 GPIO pins. Port 0, 1, 2 has 16 pins each.
 *                Port 3 has 9 pins.
 *                ULP instance has total 12 pins.
 *
 * @return     None
*******************************************************************************/
void sl_si91x_gpio_disable_group_interrupt(sl_si91x_group_interrupt_t group_interrupt, uint8_t port, uint8_t pin);

/***************************************************************************/ /**
 * @brief      Select the slew rate for the ULP instance of GPIO peripheral.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver() \n
 * @param[in]  gpio_num - GPIO pin number to be use
 * @param[in]  slew_rate   -   slew rate of type \ref sl_si91x_gpio_slew_rate_t
 *                  '0' - Slow\n
 *                  '1' - Fast\n
 * @return    None
 ******************************************************************************/
void sl_si91x_gpio_select_ulp_pad_slew_rate(uint8_t gpio_num, sl_si91x_gpio_slew_rate_t slew_rate);

/***************************************************************************/ /**
 * @brief        Select the drive strength for the ULP instance of GPIO peripheral.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver() \n
 * @param[in]    gpio_num -  GPIO pin number to be use
 * @param[in]    strength    -  Drive strength selector(E1,E2) of type
 *                    \ref  sl_si91x_gpio_driver_strength_select_t
 *                    0, for two_milli_amps   (E1=0,E2=0)\n
 *                    1, for four_milli_amps  (E1=0,E2=1)\n
 *                    2, for eight_milli_amps (E1=1,E2=0)\n
 *                    3, for twelve_milli_amps(E1=1,E2=1)\n
 * @return       None
 ******************************************************************************/
void sl_si91x_gpio_select_ulp_pad_driver_strength(uint8_t gpio_num, sl_si91x_gpio_driver_strength_select_t strength);

/***************************************************************************/ /**
 * @brief     Configure the power-on-start selection for the ULP instance of GPIO peripheral.
 * @pre       \ref sl_si91x_gpio_enable_clock() \n
 *            \ref sl_si91x_gpio_enable_pad_selection() \n
 *            \ref sl_si91x_gpio_enable_pad_receiver() \n
 * @param[in] gpio_num - GPIO pin number to configure
 * @param[in] pos      - Power-on start level of type \ref sl_si91x_gpio_pos_t \n
 *                       Possible values are: \n
 *                         GPIO_POS_DIS  - power-on-start disable \n
 *                         GPIO_POS_EN - power-on-start enable \n
 * @return    None
 ******************************************************************************/
void sl_si91x_gpio_select_ulp_pad_power_on_start(uint8_t gpio_num, sl_si91x_gpio_pos_t pos);

/***************************************************************************/ /**
 * @brief     Configure the Schmitt trigger selection for the ULP instance of GPIO peripheral.
 * @pre       \ref sl_si91x_gpio_enable_clock() \n
 *            \ref sl_si91x_gpio_enable_pad_selection() \n
 *            \ref sl_si91x_gpio_enable_pad_receiver() \n
 * @param[in] gpio_num     - GPIO pin number to configure
 * @param[in] schmitt_trig - Schmitt trigger level of type \ref sl_si91x_gpio_schmitt_trig_t \n
 *                            Possible values are: \n
 *                              GPIO_SCHMITT_TRIG_DIS  - Schmitt trigger disable\n
 *                              GPIO_SCHMITT_TRIG_EN - Schmitt trigger enable \n
 * @return    None
 ******************************************************************************/
void sl_si91x_gpio_select_ulp_pad_active_high_schmitt_trigger(uint8_t gpio_num,
                                                              sl_si91x_gpio_schmitt_trig_t schmitt_trig);

/***************************************************************************/ /**
 * @brief       Select the driver-disabled state control for the ULP instance of GPIO peripheral.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver() \n
 * @param[in]    gpio_num    -  GPIO pin number to be use
 * @param[in]    disable_state  -  driver disable state of type
 *                    \ref sl_si91x_gpio_driver_disable_state_t
 *                  0, for HiZ       (P1=0,P2=0)\n
 *                  1, for Pull up   (P1=0,P2=1)\n
 *                  2, for Pull down (P1=1,P2=0)\n
 *                  3, for Repeater  (P1=1,P2=1)\n
 * @return       None
 ******************************************************************************/
void sl_si91x_gpio_select_ulp_pad_driver_disable_state(uint8_t gpio_num,
                                                       sl_si91x_gpio_driver_disable_state_t disable_state);

/***************************************************************************/ /**
 * @brief     Disable the receiver enable bit for the ULP instance of GPIO peripheral.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @param[in]    gpio_num  - GPIO pin number to be used
 * @return       None
*******************************************************************************/
void sl_si91x_gpio_disable_ulp_pad_receiver(uint32_t gpio_num);

/***************************************************************************/ /**
 * @brief        Enable the receiver enable bit for the ULP instance of GPIO peripheral.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @param[in]    gpio_num  - GPIO pin number to be used
 * @return       None
*******************************************************************************/
void sl_si91x_gpio_enable_ulp_pad_receiver(uint8_t gpio_num);

/***************************************************************************/ /**
 * @brief      Configure the MCU ULP instance pin interrupts with trigger type(level/edge)
 *             and register the callback function for interrupts.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver() \n
 *        \ref sl_gpio_set_pin_mode() \n
 *        \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  interrupt_no - The interrupt number to trigger.
 * @param[in]  flags  - Interrupt configuration flags of type
 *                   \ref  sl_si91x_gpio_interrupt_config_flag_t
 * @param[in]  pin - GPIO pin number
 * @return     None
*******************************************************************************/
void sl_si91x_gpio_configure_ulp_pin_interrupt(uint8_t interrupt_no,
                                               sl_si91x_gpio_interrupt_config_flag_t flags,
                                               sl_si91x_gpio_pin_ulp_t pin);

/***************************************************************************/ /**
 * @brief     Set the UULP GPIO pin MUX(mode) to selected mode.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_select_uulp_npss_receiver() \n
 * @param[in]  pin  -  UULP GPIO pin number(0 to 4) of type
 *                      \ref sl_si91x_uulp_npss_mode_t
 * @param[in]  mode  -  UULP GPIO  MUX value
 * @return     none
*******************************************************************************/
void sl_si91x_gpio_set_uulp_npss_pin_mux(uint8_t pin, sl_si91x_uulp_npss_mode_t mode);

/***************************************************************************/ /**
 * @brief     Enable/disable the UULP GPIO Input Buffer.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @param[in]  pin    - is UULP GPIO pin number (0 to 4)
 * @param[in]  receiver - is enable/disable UULP GPIO receiver of type
 *                  \ref  sl_si91x_gpio_receiver_t
 *                 '1' - Enable\n
 *                 '0' - Disable\n
 * @return     None
 ******************************************************************************/
void sl_si91x_gpio_select_uulp_npss_receiver(uint8_t pin, sl_si91x_gpio_receiver_t receiver);

/***************************************************************************/ /**
 * @brief    Set the direction for the selected UULP GPIO.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_select_uulp_npss_receiver() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_pin_mux() \n
 * @param[in]  pin  - is UULP GPIO pin number (0 to 4)
 * @param[in]  direction  - is direction value (Input / Output) of type
 *                \ref  sl_si91x_gpio_direction_t
 *                  '1' - Input Direction\n
 *                  '0' - Output Direction\n
 * @return     None
 ******************************************************************************/
void sl_si91x_gpio_set_uulp_npss_direction(uint8_t pin, sl_si91x_gpio_direction_t direction);

/***************************************************************************/ /**
 * @brief      Get the direction of the selected UULP GPIO.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_select_uulp_npss_receiver() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_pin_mux() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_direction() \n
 * @param[in]  pin - is UULP GPIO pin number(0...4)
 * @return     returns the GPIO pin direction
 *                - 1, Input Direction
 *                - 0, Output Direction
*******************************************************************************/
uint8_t sl_si91x_gpio_get_uulp_npss_direction(uint8_t pin);

/***************************************************************************/ /**
 * @brief       Control(set or clear) the UULP GPIO pin value.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_select_uulp_npss_receiver() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_pin_mux() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_direction() \n
 * @param[in]  pin - is UULP GPIO pin number (0...4) of type
 *                    \ref  sl_si91x_gpio_pin_value_t
 * @param[in]  pin_value - is UULP GPIO pin value
 *                      '1' - SET \n
 *                      '0' - CLEAR \n
 * @return     None
 ******************************************************************************/
void sl_si91x_gpio_set_uulp_npss_pin_value(uint8_t pin, sl_si91x_gpio_pin_value_t pin_value);

/***************************************************************************/ /**
 * @brief     Read the status of selected UULP GPIO pin value.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_select_uulp_npss_receiver() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_pin_mux() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_direction() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_pin_value() \n
 * @param[in]  pin  - is UULP GPIO pin number (0 to 4)
 * @return     returns the pin logical state of pin
 *                        '0' - LOW \n
 *                        '1' - HIGH \n
 ******************************************************************************/
uint8_t sl_si91x_gpio_get_uulp_npss_pin(uint8_t pin);

/***************************************************************************/ /**
 * @brief     Select the UULP GPIO polarity for generating interrupt.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @param[in] pin   - is UULP GPIO pin number (0 to 4)
 * @param[in] polarity - GPIO polarity
 *                 \ref sl_si91x_gpio_polarity_t
 *                 '1' - High\n
 *                 '0' - Low\n
 * @return    None
 ******************************************************************************/
void sl_si91x_gpio_select_uulp_npss_polarity(uint8_t pin, sl_si91x_gpio_polarity_t polarity);

/***************************************************************************/ /**
 * @brief       Set the UULP GPIO interrupt as a wake up source across sleep wakeups.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @param[in]   npssgpio_interrupt - UULP GPIO pin number (0 to 4)
 *                    \ref  sl_si91x_uulp_gpio_interrupt_t
 * @return      none
*******************************************************************************/
void sl_si91x_gpio_set_uulp_npss_wakeup_interrupt(uint8_t npssgpio_interrupt);

/***************************************************************************/ /**
 * @brief        Clear the UULP UULP GPIO Interrupt as wake up source.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @param[in]   npssgpio_interrupt - UULP GPIO pin number (0 to 4)
 *                    \ref  sl_si91x_uulp_gpio_interrupt_t
 * @return      none
*******************************************************************************/
void sl_si91x_gpio_clear_uulp_npss_wakeup_interrupt(uint8_t npssgpio_interrupt);

/***************************************************************************/ /**
 * @brief       Mask the selected UULP GPIO interrupt.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_set_uulp_pad_configuration() \n
 *        \ref sl_si91x_gpio_select_uulp_npss_receiver() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_pin_mux() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_direction() \n
 * @param[in]   npssgpio_interrupt - Bit position of the UULP GPIO interrupt to be masked
 *                    \ref  sl_si91x_uulp_gpio_interrupt_bit_t
 * 
 * @note This function is deprecated and should be replaced with `sl_si91x_gpio_mask_set_uulp_npss_interrupt`.
 * 
 * @return      none
*******************************************************************************/
void sl_si91x_gpio_mask_uulp_npss_interrupt(uint8_t npssgpio_interrupt);

/***************************************************************************/ /**
 * @brief        Unmask the selected UULP GPIO interrupt.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_set_uulp_pad_configuration() \n
 *        \ref sl_si91x_gpio_select_uulp_npss_receiver() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_pin_mux() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_direction() \n
 * @param[in]   npssgpio_interrupt - Bit position of the UULP GPIO interrupt to be unmasked
 *                    \ref  sl_si91x_uulp_gpio_interrupt_bit_t
 * 
 * @note This function is deprecated and should be replaced with `sl_si91x_gpio_mask_clear_uulp_npss_interrupt`.
 * 
 * @return      none
*******************************************************************************/
void sl_si91x_gpio_unmask_uulp_npss_interrupt(uint8_t npssgpio_interrupt);

/***************************************************************************/ /**
 * @brief    Clear the selected UULP GPIO interrupt.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_set_uulp_pad_configuration() \n
 *        \ref sl_si91x_gpio_select_uulp_npss_receiver() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_pin_mux() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_direction() \n
 *        \ref sl_si91x_gpio_configure_uulp_interrupt() \n
 * @param[in]   npssgpio_interrupt - Bit position of the UULP GPIO interrupt to be cleared
 *                    \ref  sl_si91x_uulp_gpio_interrupt_bit_t
 * 
 * @note This function is deprecated and should be replaced with `sl_si91x_gpio_clear_uulp_npss_interrupt`.
 * 
 * @return      none
*******************************************************************************/
void sl_si91x_gpio_clear_uulp_interrupt(uint8_t npssgpio_interrupt);

/***************************************************************************/ /**
 * @brief       Mask the selected UULP GPIO interrupt.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_set_uulp_pad_configuration() \n
 *        \ref sl_si91x_gpio_select_uulp_npss_receiver() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_pin_mux() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_direction() \n
 * @param[in]   npssgpio_interrupt - UULP GPIO pin number (0 to 4) to be masked \ref sl_si91x_uulp_gpio_interrupt_t
 * @return      none
*******************************************************************************/
void sl_si91x_gpio_mask_set_uulp_npss_interrupt(uint8_t npssgpio_interrupt);

/***************************************************************************/ /**
 * @brief        Unmask the selected UULP GPIO interrupt.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_set_uulp_pad_configuration() \n
 *        \ref sl_si91x_gpio_select_uulp_npss_receiver() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_pin_mux() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_direction() \n
 * @param[in]   npssgpio_interrupt - UULP GPIO pin number (0 to 4) to be unmasked \ref sl_si91x_uulp_gpio_interrupt_t
 * @return      none
*******************************************************************************/
void sl_si91x_gpio_mask_clear_uulp_npss_interrupt(uint8_t npssgpio_interrupt);

/***************************************************************************/ /**
 * @brief    Clear the selected UULP GPIO interrupt.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_set_uulp_pad_configuration() \n
 *        \ref sl_si91x_gpio_select_uulp_npss_receiver() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_pin_mux() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_direction() \n
 *        \ref sl_si91x_gpio_configure_uulp_interrupt() \n
 * @param[in]   npssgpio_interrupt - UULP GPIO pin number (0 to 4) to be cleared \ref sl_si91x_uulp_gpio_interrupt_t
 * @return      none
*******************************************************************************/
void sl_si91x_gpio_clear_uulp_npss_interrupt(uint8_t npssgpio_interrupt);

/***************************************************************************/ /**
 * @brief     Get the current status of all the UULP GPIO interrupt status.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_set_uulp_pad_configuration() \n
 *        \ref sl_si91x_gpio_select_uulp_npss_receiver() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_pin_mux() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_direction() \n
 *        \ref sl_si91x_gpio_configure_uulp_interrupt() \n
 * @param[in]  None
 * @return    returns the UULP INTR status.
 *                   1, interrupt has been raised\n
 *                   0, interrupt is masked or not raised\n
 ******************************************************************************/
uint8_t sl_si91x_gpio_get_uulp_interrupt_status(void);

/***************************************************************************/ /**
 * @brief     Get the selected ULP instance GPIO pin interrupt status.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver() \n
 *        \ref sl_gpio_set_pin_mode() \n
 *        \ref sl_si91x_gpio_set_pin_direction() \n
 *        \ref sl_si91x_gpio_configure_ulp_pin_interrupt() \n
 * @param[in]  flags : ULP GPIO interrupt sources.
 * @return    returns the ULP INTR status.
 *                   1, interrupt has been raised\n
 *                   0, interrupt is masked or not raised\n
 ******************************************************************************/
uint32_t sl_si91x_gpio_get_ulp_interrupt_status(uint32_t flags);

/***************************************************************************/ /**
 * @brief     Clear the selected ULP instance GPIO pin interrupts.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver() \n
 *        \ref sl_gpio_set_pin_mode() \n
 *        \ref sl_si91x_gpio_set_pin_direction() \n
 *        \ref sl_si91x_gpio_configure_ulp_pin_interrupt() \n
 * @param[in] flags : ULP GPIO interrupt sources to clear.
 * @return    None
*******************************************************************************/
void sl_si91x_gpio_clear_ulp_interrupt(uint32_t flags);

/***************************************************************************/ /**
 * @brief     Clear the selected ULP instance group interrupt.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver() \n
 *        \ref sl_gpio_set_pin_mode() \n
 *        \ref sl_si91x_gpio_set_pin_direction() \n
 *        \ref sl_si91x_gpio_configure_ulp_group_interrupt() \n
 *  Use corresponding group interrupt configuration API for corresponding GPIO instance.
 * @param[in]    group_interrupt - Group interrupt number of type
 *                     \ref   sl_si91x_group_interrupt_t
 * @return       None
*******************************************************************************/
void sl_si91x_gpio_clear_ulp_group_interrupt(sl_si91x_group_interrupt_t group_interrupt);

/***************************************************************************/ /**
* @brief     Configure the UULP GPIO pin interrupt with interrupt type level or edge and registers callback function for interrupts.
* @pre   \ref sl_si91x_gpio_enable_clock() \n
*        \ref sl_si91x_gpio_set_uulp_pad_configuration() \n
*        \ref sl_si91x_gpio_select_uulp_npss_receiver() \n
*        \ref sl_si91x_gpio_set_uulp_npss_pin_mux() \n
*        \ref sl_si91x_gpio_set_uulp_npss_direction() \n
* @param[in] flags  -  Interrupt configuration flags of type
*                     \ref  sl_si91x_gpio_interrupt_config_flag_t
* @param[in] npssgpio_interrupt - UULP GPIO pin number(0 to 4)
*                       \ref   sl_si91x_uulp_gpio_interrupt_t
* @return    None
*******************************************************************************/
void sl_si91x_gpio_configure_uulp_interrupt(sl_si91x_gpio_interrupt_config_flag_t flags, uint8_t npssgpio_interrupt);

/***************************************************************************/ /**
 * @brief      Configure the MCU ULP instance group interrupts with trigger type(level/edge), polarity(high/low),interrupt type(and/or)
 *             and register the callback function for interrupts.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_enable_ulp_pad_receiver() \n
 *        \ref sl_gpio_set_pin_mode() \n
 *        \ref sl_si91x_gpio_set_pin_direction() \n
 * @param[in]  configuration - configuration pointer to
 *                  \ref sl_si91x_gpio_group_interrupt_config_t structure
 * @return     None
*******************************************************************************/
void sl_si91x_gpio_configure_ulp_group_interrupt(sl_si91x_gpio_group_interrupt_config_t *configuration);

/***************************************************************************/ /**
 * @brief   Verify assumptions and print message if the assumption is false.
 * @param[in]  file - File name
 * @param[in]  line - Line number
 * @return     None
 ******************************************************************************/
void sl_assert_failed(uint8_t *file, unsigned int line);

/***************************************************************************/ /**
 * Toggle the selected UULP pin status.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 *        \ref sl_si91x_gpio_select_uulp_npss_receiver() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_pin_mux() \n
 *        \ref sl_si91x_gpio_set_uulp_npss_direction() \n
 * @param[in]   pin  -  UULP pin number to toggle
 * @return      None
 ******************************************************************************/
void sl_si91x_gpio_toggle_uulp_npss_pin(uint8_t pin);

/***************************************************************************/ /**
 * @brief        Indicate UULP GPIO PAD configuration.
 * @pre   \ref sl_si91x_gpio_enable_clock() \n
 * @param[in]   pad_config : PAD configuration pointer to \ref uulp_pad_config_t structure
 * @return       None
*******************************************************************************/
void sl_si91x_gpio_set_uulp_pad_configuration(uulp_pad_config_t *pad_config);

/***************************************************************************/ /**
 * @brief    Get the release, SQA, and development version numbers of the GPIO peripheral.
 * @param[in]    None
 * @return       returns structure of type \ref sl_si91x_gpio_version_t
*******************************************************************************/
sl_si91x_gpio_version_t sl_si91x_gpio_get_version(void);

/** @} (end addtogroup GPIO) */

#ifdef __cplusplus
}
#endif

#endif ///< SL_SI91X_GPIO_PERIPHERAL_H
/**************************************************************************************************/
