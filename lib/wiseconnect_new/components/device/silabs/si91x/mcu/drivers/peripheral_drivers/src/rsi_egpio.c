/******************************************************************************
* @file  rsi_egpio.c
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

//Includes

#include "rsi_ccp_user_config.h"
#include "rsi_ccp_common.h"
#ifndef ROMDRIVER_PRESENT
#include "rsi_rom_egpio.h"
/** @addtogroup SOC11
* @{
*/
/*==============================================*/
/**
 * @fn           void egpio_set_dir(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, boolean_t dir)
 * @brief        This API is used to set the EGPIO direction(Direction of the GPIO pin. 
 *               '1' for INPUT, '0' for OUTPUT)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @param[in]    dir     : boolean type pin direction
 *                \n '0' : Output
 *                \n '1' : Input
 * @return       None
 */
void egpio_set_dir(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, boolean_t dir)
{
  pEGPIO->PIN_CONFIG[(port * 16) + pin].GPIO_CONFIG_REG_b.DIRECTION = (unsigned int)(dir & 0x01);
}

/*==============================================*/
/**
 * @fn           void egpio_set_pin(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t val)
 * @brief        This API is used to set the GPIO pin value.It Loads 0th bit on to the pin on write &
 *                reads the value on pin on read into 0th bit
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @param[in]    val     : value to be set for the pin
 *                \n '0' : Logic on Pin
 *                \n '1' : Logic on Pin
 * @return       None
 */
void egpio_set_pin(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t val)
{
  pEGPIO->PIN_CONFIG[(port * 16) + pin].BIT_LOAD_REG = val;
}

/*==============================================*/
/**
 * @fn           boolean_t egpio_get_pin(const EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
 * @brief        This API is used get the GPIO pin status.
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @return       returns Pin status
 */
boolean_t egpio_get_pin(const EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
{
  return ((boolean_t)(pEGPIO->PIN_CONFIG[(port * 16) + pin].BIT_LOAD_REG));
}

/*==============================================*/
/**
 * @fn           boolean_t egpio_get_dir(const EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
 * @brief        This API is used to Get the Direction GPIO(Direction of the GPIO pin.
 *               '1' for INPUT,and '0'for OUTPUT)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @return       returns the GPIO direction value
 */
boolean_t egpio_get_dir(const EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
{
  return pEGPIO->PIN_CONFIG[(port * 16) + pin].GPIO_CONFIG_REG_b.DIRECTION;
}

/*==============================================*/
/**
 * @fn           void egpio_pin_int_sel(EGPIO_Type *pEGPIO, uint8_t intCh, uint8_t port, uint8_t pin)
 * @brief        This API is used to select the pin for interrupt generation
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @return       None
 */
void egpio_pin_int_sel(EGPIO_Type *pEGPIO, uint8_t intCh, uint8_t port, uint8_t pin)
{
  pEGPIO->INTR[intCh].GPIO_INTR_CTRL_b.PIN_NUMBER  = (unsigned int)(pin & 0x0F);
  pEGPIO->INTR[intCh].GPIO_INTR_CTRL_b.PORT_NUMBER = (unsigned int)(port & 0x03);
}

/*==============================================*/
/**
 * @fn           void egpio_set_int_fall_edge_enable(EGPIO_Type *pEGPIO, uint8_t intCh)
 * @brief        This API is used to set the pin interrupt mode configuration
 *               \n(enables interrupt generation when falling edge is detected on pin 
 *               '1' for intr enabled and '0' for disabled)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
void egpio_set_int_fall_edge_enable(EGPIO_Type *pEGPIO, uint8_t intCh)
{
  pEGPIO->INTR[intCh].GPIO_INTR_CTRL_b.FALL_EDGE_ENABLE = 1U;
}

/*==============================================*/
/**
 * @fn           void egpio_set_int_fall_edge_disable(EGPIO_Type *pEGPIO, uint8_t intCh)
 * @brief        This API to used to set the pin interrupt mode configuration
 *               \n(enables interrupt generation when falling edge is detected on pin
 *               '1' for intr enabled and '0' for disabled)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
void egpio_set_int_fall_edge_disable(EGPIO_Type *pEGPIO, uint8_t intCh)
{
  pEGPIO->INTR[intCh].GPIO_INTR_CTRL_b.FALL_EDGE_ENABLE = 0U;
}

/*==============================================*/
/**
 * @fn           void egpio_set_int_rise_edge_enable(EGPIO_Type *pEGPIO, uint8_t intCh)
 * @brief        This API to used to set the pin interrupt mode configuration
 *               \n(enables interrupt generation when rising edge is detected on pin 
 *               '1' for intr enabled and '0' for disabled)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
void egpio_set_int_rise_edge_enable(EGPIO_Type *pEGPIO, uint8_t intCh)
{
  pEGPIO->INTR[intCh].GPIO_INTR_CTRL_b.RISE_EDGE_ENABLE = 1U;
}

/*==============================================*/
/**
 * @fn           void egpio_set_int_rise_edge_disable(EGPIO_Type *pEGPIO, uint8_t intCh)
 * @brief        This API to used to set the pin interrupt mode configuration
 *               \n(enables interrupt generation when rising edge is detected on pin 
 *               '1' for intr enabled '0' for disabled)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
void egpio_set_int_rise_edge_disable(EGPIO_Type *pEGPIO, uint8_t intCh)
{
  pEGPIO->INTR[intCh].GPIO_INTR_CTRL_b.RISE_EDGE_ENABLE = 0U;
}

/*==============================================*/
/**
 * @fn           void egpio_set_int_low_level_enable(EGPIO_Type *pEGPIO, uint8_t intCh)
 * @brief        This API is used to set the pin interrupt mode configuration
 *               \n (enables interrupt generation when pin level is 0, 
 *               '1' for intr enabled, '0' for disabled)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
void egpio_set_int_low_level_enable(EGPIO_Type *pEGPIO, uint8_t intCh)
{
  pEGPIO->INTR[intCh].GPIO_INTR_CTRL_b.LEVEL_LOW_ENABLE = 1U;
}

/*==============================================*/
/**
 * @fn           void egpio_int_mask(EGPIO_Type *pEGPIO, uint8_t intCh)
 * @brief        This API is used to set the pin interrupt mode configuration
 *               \n(Masks the interrupt. Interrupt will still be seen in status register when enabled
 *							 '1' for intr masked '0' for intr unmasked)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
void egpio_int_mask(EGPIO_Type *pEGPIO, uint8_t intCh)
{
  pEGPIO->INTR[intCh].GPIO_INTR_CTRL_b.MASK = 1U;
}

/*==============================================*/
/**
 * @fn           void egpio_int_un_mask(EGPIO_Type *pEGPIO, uint8_t intCh)
 * @brief        This API is used to used to set the pin interrupt mode configuration
 *               \n(UnMasks the interrupt.						
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
void egpio_int_un_mask(EGPIO_Type *pEGPIO, uint8_t intCh)
{
  pEGPIO->INTR[intCh].GPIO_INTR_CTRL_b.MASK = 0U;
}

/*==============================================*/
/**
 * @fn           void egpio_set_int_low_level_disable(EGPIO_Type *pEGPIO, uint8_t intCh)
 * @brief        This API is used to set the pin interrupt mode configuration
 *               \n(enables interrupt generation when pin level is 0 ,'1' for intr enabled '0' for disabled)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
void egpio_set_int_low_level_disable(EGPIO_Type *pEGPIO, uint8_t intCh)
{
  pEGPIO->INTR[intCh].GPIO_INTR_CTRL_b.LEVEL_LOW_ENABLE = 0U;
}

/*==============================================*/
/**
 * @fn           void egpio_set_int_high_level_enable(EGPIO_Type *pEGPIO, uint8_t intCh)
 * @brief        This API used to set the pin interrupt mode configuration
 *               \n(enables interrupt generation when pin level is 1, '1' for intr enabled '0' for disabled)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
void egpio_set_int_high_level_enable(EGPIO_Type *pEGPIO, uint8_t intCh)
{
  pEGPIO->INTR[intCh].GPIO_INTR_CTRL_b.LEVEL_HIGH_ENABLE = 1U;
}

/*==============================================*/
/**
 * @fn           void egpio_set_int_high_level_disable(EGPIO_Type *pEGPIO, uint8_t intCh)
 * @brief        This API is used to used to set the pin interrupt mode configuration
                 \n(disables interrupt generation when pin level is 1 ,'1' for intr enabled '0' for disabled)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
void egpio_set_int_high_level_disable(EGPIO_Type *pEGPIO, uint8_t intCh)
{
  pEGPIO->INTR[intCh].GPIO_INTR_CTRL_b.LEVEL_HIGH_ENABLE = 0U;
}

/*==============================================*/
/**
 * @fn           uint8_t egpio_get_int_stat(const EGPIO_Type *pEGPIO, uint8_t intCh)
 * @brief        This API is used to get the pin interrupt status register
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       returns the interrupt status register
 */
uint8_t egpio_get_int_stat(const EGPIO_Type *pEGPIO, uint8_t intCh)
{
  return (uint8_t)(pEGPIO->INTR[intCh].GPIO_INTR_STATUS);
}

/*==============================================*/
/**
 * @fn           void egpio_int_clr(EGPIO_Type *pEGPIO, uint8_t intCh, uint8_t flags)
 * @brief        This API is used to clear the pin interrupt in status register
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @param[in]    flags   : GPIO pin interrupt channel number (0 to 7)
													\n 2- \ref EGPIO_PIN_INT_CLR_FALLING
													\n 1- \ref EGPIO_PIN_INT_CLR_RISING
													\n 0- \ref INTERRUPT_STATUS_CLR
 * @return       None
 */
void egpio_int_clr(EGPIO_Type *pEGPIO, uint8_t intCh, uint8_t flags)
{
  pEGPIO->INTR[intCh].GPIO_INTR_STATUS = flags;
}

/*==============================================*/
/**
 * @fn           void egpio_set_pin_mux(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t mux)
 * @brief        This API to used to set pin multiplexing
 *               \n(GPIO Pin Mode. Ranges 000 -> Mode 0 to 111 -> Mode 7 Used for GPIO Pin Muxing)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @param[in]    mux     : pin function value
 *               \n possible values for this parameter are the following
 *               - \ref EGPIO_PIN_MUX_MODE0   : Select pin mode 0
 *               - \ref EGPIO_PIN_MUX_MODE1   : Select pin mode 1
 *               - \ref EGPIO_PIN_MUX_MODE2   : Select pin mode 2
 *               - \ref EGPIO_PIN_MUX_MODE3   : Select pin mode 3
 *               - \ref EGPIO_PIN_MUX_MODE4   : Select pin mode 4
 *               - \ref EGPIO_PIN_MUX_MODE5   : Select pin mode 5
 *               - \ref EGPIO_PIN_MUX_MODE6   : Select pin mode 6
 *               - \ref EGPIO_PIN_MUX_MODE7   : Select pin mode 7
 *               - \ref EGPIO_PIN_MUX_MODE8   : Select pin mode 8
 *               - \ref EGPIO_PIN_MUX_MODE9   : Select pin mode 9
 *               - \ref EGPIO_PIN_MUX_MODE10  : Select pin mode 10
 *               - \ref EGPIO_PIN_MUX_MODE11  : Select pin mode 11
 *               - \ref EGPIO_PIN_MUX_MODE12  : Select pin mode 12
 *               - \ref EGPIO_PIN_MUX_MODE13  : Select pin mode 13
 *               - \ref EGPIO_PIN_MUX_MODE14  : Select pin mode 14
 *               - \ref EGPIO_PIN_MUX_MODE15  : Select pin mode 15
 * @return       None
 */
void egpio_set_pin_mux(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t mux)
{
  pEGPIO->PIN_CONFIG[(port * 16) + pin].GPIO_CONFIG_REG_b.MODE = (unsigned int)(mux & 0x0F);
}

/*==============================================*/
/**
 * @fn           void egpio_ulp_soc_gpio_mode(ULPCLK_Type *pULPCLK, uint8_t gpio, uint8_t mode)
 * @brief        This API is used set ulp soc gpio mode
 *               \n(Gpio pin mode,ranges 000 -> Mode 0 to 111 -> Mode 7 Used for GPIO Pin Muxing )
 * @param[in]    pULPCLK  : Pointer to the ULP register instance
 * @param[in]    gpio     : Gpio number
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
 */
void egpio_ulp_soc_gpio_mode(ULPCLK_Type *pULPCLK, uint8_t gpio, uint8_t mode)
{
#if defined(SLI_SI917)
  pULPCLK->ULP_SOC_GPIO_MODE_REG[gpio].ULP_SOC_GPIO_MODE_REG_b.ULP_SOC_GPIO_MODE_REG = (unsigned int)(mode & 0x07);
#else
  pULPCLK->ULP_SOC_GPIO_MODE_REG[gpio].ULP_SOC_GPIO_0_MODE_REG_b.ULP_SOCGPIO_N_MODE = (mode);
#endif
}

/*==============================================*/
/**
 * @fn           void egpio_set_port_mask(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
 * @brief        This API is used to set the EGPIO port mask. When set, pin is masked when written/read through PORT MASK REG.
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @return       None
 */
void egpio_set_port_mask(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
{
  pEGPIO->PIN_CONFIG[(port * 16) + pin].GPIO_CONFIG_REG_b.PORTMASK = 1U;
}

/*==============================================*/
/**
 * @fn           void egpio_set_port_un_mask(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
 * @brief        This API is used to set the EGPIO port unmask. 
 *               When set, pin is masked when written/read through PORT MASK REG.
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @return       None
 */
void egpio_set_port_un_mask(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
{
  pEGPIO->PIN_CONFIG[(port * 16) + pin].GPIO_CONFIG_REG_b.PORTMASK = 0U;
}

/*==============================================*/
/**
 * @fn           void egpio_port_masked_load(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val)
 * @brief        This API is used to set the EGPIO port mask load.
 *               When set, pin is masked when written/read through PORT MASK REG.
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    val     : Port value to be set
 * @return       None
 */
void egpio_port_masked_load(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val)
{
  pEGPIO->PORT_CONFIG[port].PORT_MASKED_LOAD_REG_b.PORT_MASKED_LOAD = val;
}

/*==============================================*/
/**
 * @fn           void egpio_set_port(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val)
 * @brief        This API is used to set the port value.
 *               Sets the pin when corresponding bit is high. Writing zero has no effect
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : Port number to the EGPIO register instance
 * @param[in]    val     : Port value to be set
 * @return       None
 */
void egpio_set_port(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val)
{
  pEGPIO->PORT_CONFIG[port].PORT_SET_REG = val;
}

/*==============================================*/
/**
 * @fn           void egpio_port_load(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val)
 * @brief        This API is used to load the port value.
 *               Loads the value on to pin on write. And reads the value of load register on read
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : Port number to the EGPIO register instance
 * @param[in]    val     : Port value to be set
 * @return       None
 */
void egpio_port_load(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val)
{
  pEGPIO->PORT_CONFIG[port].PORT_LOAD_REG = val;
}

/*==============================================*/
/**
 * @fn           void egpio_word_load(EGPIO_Type *pEGPIO, uint8_t pin, uint16_t val)
 * @brief        This API is used to load the port value.
 *               Loads 1 on the pin when any of the bit in load value is 1. On read pass the bit status into all bits.                       
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    pin     : pin number to the EGPIO register instance
 * @param[in]    val     : Port value to be set
 * @return       None
 */
void egpio_word_load(EGPIO_Type *pEGPIO, uint8_t pin, uint16_t val)
{
  pEGPIO->PIN_CONFIG[pin].WORD_LOAD_REG_b.WORD_LOAD = val;
}

/*==============================================*/
/**
 * @fn           void egpio_clr_port(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val)
 * @brief        This API is used to clear the port value.
 *               Clears the pin when corresponding bit is high. Writing zero has no effect.
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : Port number
 * @param[in]    val     : Port value to be clear
 * @return       None
 */
void egpio_clr_port(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val)
{
  pEGPIO->PORT_CONFIG[port].PORT_CLEAR_REG = val;
}

/*==============================================*/
/**
 * @fn           void egpio_toggle_port(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val)
 * @brief        This API is used to toggle the port.
 *               Toggles the pin when corresponding bit is high. Writing zero has not effect.
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : Port number
 * @param[in]    val     : Port value to be toggle
 * @return       None
 */
void egpio_toggle_port(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val)
{
  pEGPIO->PORT_CONFIG[port].PORT_TOGGLE_REG = val;
}

/*==============================================*/
/**
 * @fn           uint16_t egpio_get_port(const EGPIO_Type *pEGPIO, uint8_t port)
 * @brief        This API is used to used to get the EGPIO port value.
 *               Reads the value on GPIO pins irrespective of the pin mode.
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : Port number to be read
 * @return       port value
 */
uint16_t egpio_get_port(const EGPIO_Type *pEGPIO, uint8_t port)
{
  return (pEGPIO->PORT_CONFIG[port].PORT_READ_REG & 0XFFFF);
}

/*==============================================*/
/**
 * @fn           void egpio_group_int_one_enable(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
 * @brief        This API is used to enable the group interrupt one ,  When set,
 *               the corresponding GPIO pin is selected for group interrupt 1 generation
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @return       None
 */
void egpio_group_int_one_enable(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
{
  pEGPIO->PIN_CONFIG[(port * 16) + pin].GPIO_CONFIG_REG_b.GROUP_INTERRUPT1_ENABLE = 1U;
}

/*==============================================*/
/**
 * @fn           void egpio_group_int_one_disable(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
 * @brief        This API is used to disable the group interrupt one
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @return       None
 */
void egpio_group_int_one_disable(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
{
  pEGPIO->PIN_CONFIG[(port * 16) + pin].GPIO_CONFIG_REG_b.GROUP_INTERRUPT1_ENABLE = 0U;
}

/*==============================================*/
/**
 * @fn           void egpio_group_int_two_enable(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
 * @brief        This API is used to enable the group interrupt Two ,  When set,
 *               the corresponding GPIO pin is selected for group interrupt 2 generation
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @return       None
 */
void egpio_group_int_two_enable(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
{
  pEGPIO->PIN_CONFIG[(port * 16) + pin].GPIO_CONFIG_REG_b.GROUP_INTERRUPT2_ENABLE = 1U;
}

/*==============================================*/
/**
 * @fn           void egpio_group_int_mask(EGPIO_Type *pEGPIO, uint8_t grpInt)
 * @brief        This API is used to configure the group interrupts(1-mask,0-unmask)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    grpInt  : Group interrupt number
 * @return       None
 */
void egpio_group_int_mask(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
  pEGPIO->GPIO_GRP_INTR[grpInt].GPIO_GRP_INTR_CTRL_REG_b.MASK = 1u;
}

/*==============================================*/
/**
 * @fn           void egpio_group_int_un_Mask(EGPIO_Type *pEGPIO, uint8_t grpInt)
 * @brief        This API is used to configure the group interrupts(1-mask,0-unmask)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    grpInt  : Group interrupt number
 * @return       None
 */
void egpio_group_int_un_Mask(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
  pEGPIO->GPIO_GRP_INTR[grpInt].GPIO_GRP_INTR_CTRL_REG_b.MASK = 0u;
}

/*==============================================*/
/**
 * @fn           void egpio_group_int_enable(EGPIO_Type *pEGPIO, uint8_t grpInt)
 * @brief        This API is used to configure the group interrupts(1-enable, 0-disable)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    grpInt  : Group interrupt number
 * @return       None
 */
void egpio_group_int_enable(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
  pEGPIO->GPIO_GRP_INTR[grpInt].GPIO_GRP_INTR_CTRL_REG_b.ENABLE_INTERRUPT = 1u;
}

/*==============================================*/
/**
 * @fn           void egpio_group_int_disable(EGPIO_Type *pEGPIO, uint8_t grpInt)
 * @brief        This API is used to configure the group interrupts(1-enable, 0-disable)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    grpInt  : Group interrupt number
 * @return       None
 */
void egpio_group_int_disable(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
  pEGPIO->GPIO_GRP_INTR[grpInt].GPIO_GRP_INTR_CTRL_REG_b.ENABLE_INTERRUPT = 0u;
}

/*==============================================*/
/**
 * @fn           void egpio_group_int_level(EGPIO_Type *pEGPIO, uint8_t grpInt)
 * @brief        This API is used to configure the group interrupts(0-level,1-edge)
 * @param[in]    pEGPIO   : Pointer to the EGPIO register instance
 * @param[in]    grpInt   : Group interrupt number
 * @return       None
 */
void egpio_group_int_level(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
  pEGPIO->GPIO_GRP_INTR[grpInt].GPIO_GRP_INTR_CTRL_REG_b.LEVEL_EDGE = 0u;
}

/*==============================================*/
/**
 * @fn           void egpio_group_int_edge(EGPIO_Type *pEGPIO, uint8_t grpInt)
 * @brief        This API is used to configure the group interrupts(0-level,1-edge)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    grpInt  : Group interrupt number
 * @return       None
 */
void egpio_group_int_edge(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
  pEGPIO->GPIO_GRP_INTR[grpInt].GPIO_GRP_INTR_CTRL_REG_b.LEVEL_EDGE = 1u;
}

/*==============================================*/
/**
 * @fn           void egpio_group_int_and(EGPIO_Type *pEGPIO, uint8_t grpInt)
 * @brief        This API is used to configure the group interrupts(0-AND ,1-Or)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    grpInt  : Group interrupt number
 * @return       None
 */
void egpio_group_int_and(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
  pEGPIO->GPIO_GRP_INTR[grpInt].GPIO_GRP_INTR_CTRL_REG_b.AND_OR = 0u;
}

/*==============================================*/
/**
 * @fn           void egpio_group_int_or(EGPIO_Type *pEGPIO, uint8_t grpInt)
 * @brief        This API is used to configure the group interrupts(0- AND , 1-Or)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    grpInt  : Group interrupt number
 * @return       None
 */
void egpio_group_int_or(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
  pEGPIO->GPIO_GRP_INTR[grpInt].GPIO_GRP_INTR_CTRL_REG_b.AND_OR = 1u;
}

/*==============================================*/
/**
 * @fn           uint32_t egpio_group_int_stat(const EGPIO_Type *pEGPIO, uint8_t grpInt)
 * @brief        This API to used to get the group interrupt status
 * @param[in]    pEGPIO   : Pointer to the EGPIO register instance
 * @param[in]    grpInt   : Group interrupt number
 * @return       returns the group interrupt status register
 */
uint32_t egpio_group_int_stat(const EGPIO_Type *pEGPIO, uint8_t grpInt)
{
  return pEGPIO->GPIO_GRP_INTR[grpInt].GPIO_GRP_INTR_STS;
}

/*==============================================*/
/**
 * @fn           void egpio_group_int_wkeup_Enable(EGPIO_Type *pEGPIO, uint8_t grpInt)
 * @brief        This API to used to Enable the group interrupt wakeup interrupt
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    grpInt  : Group interrupt number
 * @return       None
 */
void egpio_group_int_wkeup_Enable(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
  pEGPIO->GPIO_GRP_INTR[grpInt].GPIO_GRP_INTR_CTRL_REG_b.ENABLE_WAKEUP = 1;
}

/*==============================================*/
/**
 * @fn           void egpio_group_int_wkeup_disable(EGPIO_Type *pEGPIO, uint8_t grpInt)
 * @brief        This API to used to Disable the group interrupt wakeup interrupt
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    grpInt  : Group interrupt number
 * @return       None
 */
void egpio_group_int_wkeup_disable(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
  pEGPIO->GPIO_GRP_INTR[grpInt].GPIO_GRP_INTR_CTRL_REG_b.ENABLE_WAKEUP = 0;
}

/*==============================================*/
/**
 * @fn           void egpio_group_int_clr(EGPIO_Type *pEGPIO, uint8_t grpInt, uint8_t u8ClrFlags)
 * @brief        This API is used to used to clear the group interrupt status
 * @param[in]    pEGPIO     : Pointer to the EGPIO register instance
 * @param[in]    grpInt     : Group interrupt number
 * @param[in]    flags : clear flags
 * @return       None
 */
void egpio_group_int_clr(EGPIO_Type *pEGPIO, uint8_t grpInt, uint8_t u8ClrFlags)
{
  pEGPIO->GPIO_GRP_INTR[grpInt].GPIO_GRP_INTR_STS = u8ClrFlags;
}

/*==============================================*/
/**
 * @fn           void egpio_group_int_two_disable(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
 * @brief        This API is used to used to disable the group interrupt two
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : PORT number
 * @param[in]    pin     : PIN number
 * @return       None
 */
void egpio_group_int_two_disable(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
{
  pEGPIO->PIN_CONFIG[(port * 16) + pin].GPIO_CONFIG_REG_b.GROUP_INTERRUPT2_ENABLE = 0U;
}

/*==============================================*/
/**
 * @fn           void egpio_set_group_int_one_pol(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t pol)
 * @brief        This API is used to set the group polarity of interrupt one.
 *               Decides the active value of the pin to be considered for group interrupt 1 generation when enabled
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : PORT number
 * @param[in]    pin     : PIN number
 * @param[in]    pol     : Polarity of interrupt
 *               \n '0'  : group interrupt gets generated when GPIO input pin status is '0'.
 *               \n '1'  : group interrupt gets generated when GPIO input pin status is '1'
 * @return       None
 */
void egpio_set_group_int_one_pol(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t pol)
{
  pEGPIO->PIN_CONFIG[(port * 16) + pin].GPIO_CONFIG_REG_b.GROUP_INTERRUPT1_POLARITY = (unsigned int)(pol & 0x01);
}

/*==============================================*/
/**
 * @fn           void egpio_set_group_int_two_pol(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t pol)
 * @brief        This API is used to set the group polarity of interrupt two.
 *               Decides the active value of the pin to be considered for group interrupt 2 generation when enabled
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : PORT number
 * @param[in]    pin     : PIN number
 * @param[in]    pol     : Polarity of interrupt
 *               \n '0'  : group interrupt gets generated when GPIO input pin status is '0'.
 *               \n '1'  : group interrupt gets generated when GPIO input pin status is '1'.
 * @return       None
 */
void egpio_set_group_int_two_pol(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t pol)
{
  pEGPIO->PIN_CONFIG[(port * 16) + pin].GPIO_CONFIG_REG_b.GROUP_INTERRUPT2_POLARITY = (unsigned int)(pol & 0x01);
}

/*==============================================*/
/**
 * @fn           void egpio_host_pads_gpio_mode_enable(uint8_t u8GpioNum)
 * @brief        This API is used to select the host pad gpios(25 to 30)
 * @param[in]    u8GpioNum  :  PAD number to be use
 * @return       None
 */
void egpio_host_pads_gpio_mode_enable(uint8_t u8GpioNum)
{
  if (u8GpioNum == 25) {
    HOST_PADS_GPIO_MODE |= (0x1 << 13);
  } else if (u8GpioNum == 26) {
    HOST_PADS_GPIO_MODE |= (0x1 << 14);
  } else if (u8GpioNum == 27) {
    HOST_PADS_GPIO_MODE |= (0x1 << 15);
  } else if (u8GpioNum == 28) {
    HOST_PADS_GPIO_MODE |= (0x1 << 16);
  } else if (u8GpioNum == 29) {
    HOST_PADS_GPIO_MODE |= (0x1 << 17);
  } else if (u8GpioNum == 30) {
    HOST_PADS_GPIO_MODE |= (0x1 << 18);
  }
}

/*==============================================*/
/**
 * @fn           void egpio_host_pads_gpio_mode_disable(uint8_t u8GpioNum)
 * @brief        This API is used to deselect the host pad gpios(25 to 30)
 * @param[in]    u8GpioNum  :  PAD number to be use
 * @return       None
 */
void egpio_host_pads_gpio_mode_disable(uint8_t u8GpioNum)
{
  if (u8GpioNum == 25) {
    HOST_PADS_GPIO_MODE &= (unsigned int)(~(0x1 << 13));
  } else if (u8GpioNum == 26) {
    HOST_PADS_GPIO_MODE &= (unsigned int)(~(0x1 << 14));
  } else if (u8GpioNum == 27) {
    HOST_PADS_GPIO_MODE &= (unsigned int)(~(0x1 << 15));
  } else if (u8GpioNum == 28) {
    HOST_PADS_GPIO_MODE &= (unsigned int)(~(0x1 << 16));
  } else if (u8GpioNum == 29) {
    HOST_PADS_GPIO_MODE &= (unsigned int)(~(0x1 << 17));
  } else if (u8GpioNum == 30) {
    HOST_PADS_GPIO_MODE &= (unsigned int)(~(0x1 << 18));
  }
}

/*==============================================*/
/**
 * @fn           void egpio_pad_selection_enable(uint8_t padNum)
 * @brief        This API is used to select the pad(0 to 21)
 * @param[in]    padNum  :  PAD number to be use
 * @return       None
 */
void egpio_pad_selection_enable(uint8_t padNum)
{
  if (padNum < 22) {
    /*(tass_m4ss_gpio_sel)PAD selection (0 t0 21) 
	A value of 1 on this gives control to M4SS(by default it is 0 means NWP control) */
    PADSELECTION |= BIT(padNum);
  }
#if defined(SLI_SI917)
  else {
    /*(tass_m4ss_gpio_sel)PAD selection (22 t0 33) 
	A value of 1 on this gives control to M4SS(by default it is 0 means NWP control) */
    PADSELECTION_1 |= BIT(padNum - 22);
  }
#endif
}

/*==============================================*/
/**
 * @fn           void egpio_pad_selection_disable(uint8_t padNum)
 * @brief        This API is used to deselect the pad(0 to 21)
 * @param[in]    padNum  :  PAD number to be use
 * @return       None
 */
void egpio_pad_selection_disable(uint8_t padNum)
{
  if (padNum < 22) {
    /*(tass_m4ss_gpio_sel)PAD selection (0 t0 21) 
	A value of 0 on this gives control to TASS(by default it is 0 means NWP control) */
    PADSELECTION &= ~BIT(padNum);
  }
#if defined(SLI_SI917)
  else {
    /*(tass_m4ss_gpio_sel)PAD selection (22 t0 33) 
	A value of 0 on this gives control to TASS(by default it is 0 means NWP control) */
    PADSELECTION_1 &= ~BIT(padNum - 22);
  }
#endif
}

/*==============================================*/
/**
 * @fn           void egpio_pad_receiver_enable(uint8_t u8GpioNum)
 * @brief        This API is used to enable the receiver enable bit(REN)
 * @param[in]    u8GpioNum  :  GPIO num to be use
 * @return       None
 */
void egpio_pad_receiver_enable(uint8_t u8GpioNum)
{
  // REN enable bit(this should be enable)
  PAD_CONFIG_REG(u8GpioNum) |= (uint32_t)(0x1 << 4);
}

/*==============================================*/
/**
 * @fn           void egpio_pad_receiver_disable(uint8_t u8GpioNum)
 * @brief        This API is used to Disable the receiver enable bit(REN)
 * @param[in]    u8GpioNum  :  GPIO num to be use
 * @return       None
 */
void egpio_pad_receiver_disable(uint8_t u8GpioNum)
{
  // REN enable bit(this should be enable)
  PAD_CONFIG_REG(u8GpioNum) &= (uint32_t)(~(0x1 << 4));
}

/*==============================================*/
/**
 * @fn           void egpio_pad_sdio_connected()
 * @brief        This API is used to use the SDIO pins(25 to 30) in M4 or NWP (0 for M4SS and 1 for TASS)
 * @return       None
 */
void egpio_pad_sdio_connected()
{
  // sdio connected to tass  (0 for M4SS and 1 for TASS) for SDIO pads(25 to 30)
  SDIO_CNTD_TO_TASS = (0x1 << 5);
}

/*==============================================*/
/**
 * @fn           void egpio_pad_driver_disable_state(uint8_t u8GpioNum, en_driver_state_t endstate)
 * @brief        This API is used to control the Driver disabled state control
 * @param[in]    u8GpioNum :  GPIO number to be use
 * @param[in]    endstate  :  the value to be passed
 *               \n            possible values are
 *               \n         -   0 for  \ref HiZ      (P1=0,P2=0)
 *	             \n         -   1 for  \ref Pullup   (P1=0,P2=1)
 *	             \n         -   2 for  \ref Pulldown (P1=1,P2=0)
 *	             \n         -   3 for  \ref Repeater (P1=1,P2=1)
 * @return       None
 */
void egpio_pad_driver_disable_state(uint8_t u8GpioNum, en_driver_state_t endstate)
{
  uint32_t reg = 0;
  reg          = PAD_CONFIG_REG(u8GpioNum);
  reg &= (uint32_t)(~DDS_MASK);
  reg |= (uint32_t)(endstate << 6);
  PAD_CONFIG_REG(u8GpioNum) = reg;
}

/*==============================================*/
/**
 * @fn           void egpio_pad_driver_strength_select(uint8_t u8GpioNum, en_driver_strength_select_t strength)
 * @brief        This API is used to select Drive strength
 * @param[in]    u8GpioNum :  GPIO number to be use
 * @param[in]    strength  :  Drive strength selector(E1,E2)
 *               \n            possible values are
 *               \n          -  0 for \ref two_milli_amps   (E1=0,E2=0)
 *	             \n          -  1 for \ref four_milli_amps  (E1=0,E2=1)
 *               \n          -  2 for \ref eight_milli_amps (E1=1,E2=0)
 *               \n          -  3 for \ref twelve_milli_amps(E1=1,E2=1)
 * @return       None
 */
void egpio_pad_driver_strength_select(uint8_t u8GpioNum, en_driver_strength_select_t strength)
{
  uint32_t reg = 0;
  reg          = PAD_CONFIG_REG(u8GpioNum);
  reg &= (uint32_t)(~DSS_MASK);
  reg |= (strength << 0);
  PAD_CONFIG_REG(u8GpioNum) = reg;
}

/*==============================================*/
/**
 * @fn           void egpio_pad_power_on_start_enable(uint8_t u8GpioNum, uint8_t val)
 * @brief        This API is used to select Power on Start enable
 * @param[in]    u8GpioNum  :  GPIO number to be use
 * @param[in]    val        :  POS = 1 : Enables active pull down for invalid power;
 *               \n         :  POS = 0 : Active pull down capability disabled .
 *               \n When one of the power supplies is invalid and active high POS is set to 1,
 *               \n AD is pulled to weak 0. When POS is set to 0, PAD remains in a high-Z state. : Default 0
 * @return       None
 */
void egpio_pad_power_on_start_enable(uint8_t u8GpioNum, uint8_t val)
{
  uint32_t reg = 0;
  reg          = PAD_CONFIG_REG(u8GpioNum);
  reg &= (uint32_t)(~POS_MASK);
  reg |= (uint32_t)(val << 2);
  PAD_CONFIG_REG(u8GpioNum) = reg;
}

/*==============================================*/
/**
 * @fn           void egpio_pad_active_high_schmitt_trigger(uint8_t u8GpioNum, uint8_t val)
 * @brief        Active high Schmitt trigger (Hysteresis) select;
 *               \n SMT=0 for No hysteresis; Default value for reset is 1'b1 and others is 1'b0
 * @param[in]    u8GpioNum  : GPIO number to be use
 * @param[in]    val        : SMT=0 : No hysteresis; Default value for reset is 1'b1 and others is 1'b0
 * @return       None
 */
void egpio_pad_active_high_schmitt_trigger(uint8_t u8GpioNum, uint8_t val)
{
  uint32_t reg = 0;
  reg          = PAD_CONFIG_REG(u8GpioNum);
  reg &= (uint32_t)(~ST_MASK);
  reg |= (uint32_t)(val << 3);
  PAD_CONFIG_REG(u8GpioNum) = reg;
}

/*==============================================*/
/**
 * @fn           void egpio_pad_slew_rate_controll(uint8_t u8GpioNum, uint8_t val)
 * @brief        this API is used to control the slew rate
 * @param[in]    u8GpioNum  :  GPIO number to be use
 * @param[in]    val        :  slew rate
 *               \n         -   SR = 0 : Slow (half frequency)
                 \n         -  SR = 1 : Fast  ,Default 1
 * @return       None
 */
void egpio_pad_slew_rate_controll(uint8_t u8GpioNum, uint8_t val)
{
  uint32_t reg = 0;
  reg          = PAD_CONFIG_REG(u8GpioNum);
  reg &= (uint32_t)(~SR_MASK);
  reg |= (uint32_t)(val << 5);
  PAD_CONFIG_REG(u8GpioNum) = reg;
}

/*==============================================*/
/**
 * @fn           void egpio_ulp_pad_receiver_enable(uint8_t u8GpioNum)
 * @brief        This API is used to enable the REN for ULP
 * @param[in]    u8GpioNum   : GPIO number to be used
 * @return       None
 */
void egpio_ulp_pad_receiver_enable(uint8_t u8GpioNum)
{
  ULP_PAD_CONFIG_REG_2 |= (0x1 << u8GpioNum);
}

/*==============================================*/
/**
 * @fn           void egpio_ulp_pad_receiver_disable(uint8_t u8GpioNum)
 * @brief        This API is used to enable the REN for ULP
 * @param[in]    u8GpioNum   : GPIO number to be used
 * @return       None
 */
void egpio_ulp_pad_receiver_disable(uint8_t u8GpioNum)
{
  ULP_PAD_CONFIG_REG_2 &= (uint32_t)(~(0x1 << u8GpioNum));
}

/*==============================================*/
/**
 * @fn           void egpio_ulp_pad_driver_disable_state(uint8_t u8GpioNum, en_ulp_driver_disable_state_t disablestate)
 * @brief        This API is used to control the Driver disabled state control
 * @param[in]    u8GpioNum 		 :  GPIO number to be use
 * @param[in]    disablestate  :  the value to be passed
 *               \n            possible values are
 *               \n           - 0 for  \ref HiZ      (P1=0,P2=0)
 *	             \n           - 1 for  \ref Pullup   (P1=0,P2=1)
 *	             \n           - 2 for  \ref Pulldown (P1=1,P2=0)
 *	             \n           - 3 for  \ref Repeater (P1=1,P2=1)
 * @return       None
 */
void egpio_ulp_pad_driver_disable_state(uint8_t u8GpioNum, en_ulp_driver_disable_state_t disablestate)
{
  uint32_t reg = 0;
  switch (u8GpioNum) {
    case 0:
    case 1:
    case 2:
    case 3:
      reg = ULP_PAD_CONFIG_REG_0;
      reg &= (uint32_t)(~ULP_DDS_MASK_SET_1_AND_3);
      reg |= (uint32_t)(disablestate << 6);
      ULP_PAD_CONFIG_REG_0 = reg;
      break;
    case 4:
    case 5:
    case 6:
    case 7:
      reg = ULP_PAD_CONFIG_REG_0;
      reg &= (uint32_t)(~ULP_DDS_MASK_SET_2_AND_4);
      reg |= (uint32_t)(disablestate << 14);
      ULP_PAD_CONFIG_REG_0 = reg;
      break;
    case 8:
    case 9:
    case 10:
    case 11:
      reg = ULP_PAD_CONFIG_REG_1;
      reg &= (uint32_t)(~ULP_DDS_MASK_SET_1_AND_3);
      reg |= (uint32_t)(disablestate << 6);
      ULP_PAD_CONFIG_REG_1 = reg;
      break;
    case 12:
    case 13:
    case 14:
    case 15:
      reg = ULP_PAD_CONFIG_REG_1;
      reg &= (uint32_t)(~ULP_DDS_MASK_SET_2_AND_4);
      reg |= (uint32_t)(disablestate << 14);
      ULP_PAD_CONFIG_REG_1 = reg;
      break;
    default:
      break;
  }
}

/*==============================================*/
/**
 * @fn           void egpio_ulp_pad_driver_strength_select(uint8_t u8GpioNum, en_ulp_driver_strength_select_t strength)
 * @brief        this API is used to select Drive strength
 * @param[in]    u8GpioNum :  GPIO number to be use
 * @param[in]    strength  :  Drive strength selector(E1,E2)
 *               \n            possible values are
 *               \n          -  0 for \ref two_milli_amps   (E1=0,E2=0)
 *	             \n      -      1 for \ref four_milli_amps  (E1=0,E2=1)
 *               \n          -  2 for \ref eight_milli_amps (E1=1,E2=0)
 *               \n          -  3 for \ref twelve_milli_amps(E1=1,E2=1)
 * @return       None
 */
void egpio_ulp_pad_driver_strength_select(uint8_t u8GpioNum, en_ulp_driver_strength_select_t strength)
{
  uint32_t reg = 0;
  switch (u8GpioNum) {
    case 0:
    case 1:
    case 2:
    case 3:
      reg = ULP_PAD_CONFIG_REG_0;
      reg &= (uint32_t)(~ULP_DSS_MASK_SET_1_AND_3);
      reg |= (strength << 0);
      ULP_PAD_CONFIG_REG_0 = reg;
      break;
    case 4:
    case 5:
    case 6:
    case 7:
      reg = ULP_PAD_CONFIG_REG_0;
      reg &= (uint32_t)(~ULP_DSS_MASK_SET_2_AND_4);
      reg |= (uint32_t)(strength << 8);
      ULP_PAD_CONFIG_REG_0 = reg;
      break;
    case 8:
    case 9:
    case 10:
    case 11:
      reg = ULP_PAD_CONFIG_REG_1;
      reg &= (uint32_t)(~ULP_DSS_MASK_SET_1_AND_3);
      reg |= (strength << 0);
      ULP_PAD_CONFIG_REG_1 = reg;
      break;
    case 12:
    case 13:
    case 14:
    case 15:
      reg = ULP_PAD_CONFIG_REG_1;
      reg &= (uint32_t)(~ULP_DSS_MASK_SET_2_AND_4);
      reg |= (uint32_t)(strength << 8);
      ULP_PAD_CONFIG_REG_1 = reg;
      break;
    default:
      break;
  }
}

/*==============================================*/
/**
 * @fn            void egpio_ulp_pad_power_on_start_enable(uint8_t u8GpioNum, uint8_t val)
 * @brief         Power-on-Start enable;
 * @param[in]     u8GpioNum  :   GPIO number to be use
 * @param[in]     val        :   POS = 1 : Enables active pull down for invalid power;
 *               \n          :    POS = 0 : Active pull down capability disabled .
 *               \n When one of the power supplies is invalid and active high POS is set to 1,
 *               PAD is pulled to weak 0. When POS is set to 0, PAD remains in a high Z state. : Default 0
 * @return       None
 */
void egpio_ulp_pad_power_on_start_enable(uint8_t u8GpioNum, uint8_t val)
{
  uint32_t reg = 0;
  switch (u8GpioNum) {
    case 0:
    case 1:
    case 2:
    case 3:
      reg = ULP_PAD_CONFIG_REG_0;
      reg &= (uint32_t)(~ULP_POS_MASK_SET_1_AND_3);
      reg |= (uint32_t)(val << 2);
      ULP_PAD_CONFIG_REG_0 = reg;
      break;
    case 4:
    case 5:
    case 6:
    case 7:
      reg = ULP_PAD_CONFIG_REG_0;
      reg &= (uint32_t)(~ULP_POS_MASK_SET_2_AND_4);
      reg |= (uint32_t)(val << 10);
      ULP_PAD_CONFIG_REG_0 = reg;
      break;
    case 8:
    case 9:
    case 10:
    case 11:
      reg = ULP_PAD_CONFIG_REG_1;
      reg &= (uint32_t)(~ULP_POS_MASK_SET_1_AND_3);
      reg |= (uint32_t)(val << 2);
      ULP_PAD_CONFIG_REG_1 = reg;
      break;
    case 12:
    case 13:
    case 14:
    case 15:
      reg = ULP_PAD_CONFIG_REG_1;
      reg &= (uint32_t)(~ULP_POS_MASK_SET_2_AND_4);
      reg |= (uint32_t)(val << 10);
      ULP_PAD_CONFIG_REG_1 = reg;
      break;
    default:
      break;
  }
}

/*==============================================*/
/**
 * @fn           void egpio_ulp_pad_active_high_schmitt_trigger(uint8_t u8GpioNum, uint8_t val)
 * @brief        Active high Schmitt trigger (Hysteresis) select;
 * @param[in]    u8GpioNum  :  GPIO number to be use
 * @param[in]    val        :  SMT=0 : No hysteresis; Default value for reset is 1'b1 and others is 1'b0
 * @return       None
 */
void egpio_ulp_pad_active_high_schmitt_trigger(uint8_t u8GpioNum, uint8_t val)
{
  uint32_t reg = 0;
  switch (u8GpioNum) {
    case 0:
    case 1:
    case 2:
    case 3:
      reg = ULP_PAD_CONFIG_REG_0;
      reg &= (uint32_t)(~ULP_ST_MASK_SET_1_AND_3);
      reg |= (uint32_t)(val << 3);
      ULP_PAD_CONFIG_REG_0 = reg;
      break;
    case 4:
    case 5:
    case 6:
    case 7:
      reg = ULP_PAD_CONFIG_REG_0;
      reg &= (uint32_t)(~ULP_ST_MASK_SET_2_AND_4);
      reg |= (uint32_t)(val << 11);
      ULP_PAD_CONFIG_REG_0 = reg;
      break;
    case 8:
    case 9:
    case 10:
    case 11:
      reg = ULP_PAD_CONFIG_REG_1;
      reg &= (uint32_t)(~ULP_ST_MASK_SET_1_AND_3);
      reg |= (uint32_t)(val << 3);
      ULP_PAD_CONFIG_REG_1 = reg;
      break;
    case 12:
    case 13:
    case 14:
    case 15:
      reg = ULP_PAD_CONFIG_REG_1;
      reg &= (uint32_t)(~ULP_ST_MASK_SET_2_AND_4);
      reg |= (uint32_t)(val << 11);
      ULP_PAD_CONFIG_REG_1 = reg;
      break;
    default:
      break;
  }
}

/*==============================================*/
/**
 * @fn           void egpio_ulp_pad_slew_rate_controll(uint8_t u8GpioNum ,uint8_t val )
 * @brief        Slew Rate Control
 * @param[in]    u8GpioNum  :   GPIO number to be use
 * @param[in]    val        :   slew rate
 *               \n         -   SR = 0 : Slow (half frequency); SR = 1 for Fast , Default 1
 * @return       None
 */
void egpio_ulp_pad_slew_rate_controll(uint8_t u8GpioNum, uint8_t val)
{
  uint32_t reg = 0;
  switch (u8GpioNum) {
    case 0:
    case 1:
    case 2:
    case 3:
      reg = ULP_PAD_CONFIG_REG_0;
      reg &= (uint32_t)(~ULP_SR_MASK_SET_1_AND_3);
      reg |= (uint32_t)(val << 5);
      ULP_PAD_CONFIG_REG_0 = reg;
      break;
    case 4:
    case 5:
    case 6:
    case 7:
      reg = ULP_PAD_CONFIG_REG_0;
      reg &= (uint32_t)(~ULP_SR_MASK_SET_2_AND_4);
      reg |= (uint32_t)(val << 13);
      ULP_PAD_CONFIG_REG_0 = reg;
      break;
    case 8:
    case 9:
    case 10:
    case 11:
      reg = ULP_PAD_CONFIG_REG_1;
      reg &= (uint32_t)(~ULP_SR_MASK_SET_1_AND_3);
      reg |= (uint32_t)(val << 5);
      ULP_PAD_CONFIG_REG_1 = reg;
      break;
    case 12:
    case 13:
    case 14:
    case 15:
      reg = ULP_PAD_CONFIG_REG_1;
      reg &= (uint32_t)(~ULP_SR_MASK_SET_2_AND_4);
      reg |= (uint32_t)(val << 13);
      ULP_PAD_CONFIG_REG_1 = reg;
      break;
    default:
      break;
  }
}
/** @} */
#endif //ROMDRIVER_PRESENT
