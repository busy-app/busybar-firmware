/*******************************************************************************
* @file  rsi_rom_egpio.h
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

#ifndef __RSI_ROM_EGPIO_H__
#define __RSI_ROM_EGPIO_H__

/**
 * \ingroup   RSI_SPECIFIC_DRIVERS
 * \defgroup EGPIO_DRIVER
 *  @{
 *
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "rsi_ccp_user_config.h"
#include "rsi_packing.h"
#include "rsi_egpio.h"
#if defined(A11_ROM)
#include "rsi_rom_table_si91x.h"
#else
#include "rsi_rom_table_RS1xxxx.h"
#endif
/** @addtogroup SOC11
* @{
*/

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_SetDir(EGPIO_Type *pEGPIO ,uint8_t port,uint8_t pin, boolean_t dir)
 * @brief        This API is used to set the EGPIO direction(Direction of the GPIO pin. '1' for INPUT, '0' for OUTPUT)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @param[in]    dir     : boolean type pin direction
 *                \n '0' : Output
 *                \n '1' : Input
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_SetDir(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, boolean_t dir)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_set_dir(pEGPIO, port, pin, dir);
#else
  egpio_set_dir(pEGPIO, port, pin, dir);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_SetPin(EGPIO_Type *pEGPIO ,uint8_t port,uint8_t pin , uint8_t val)
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
STATIC INLINE void RSI_EGPIO_SetPin(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t val)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_set_pin(pEGPIO, port, pin, val);
#else
  egpio_set_pin(pEGPIO, port, pin, val);
#endif
}

/**
 * @fn           STATIC INLINE boolean_t RSI_EGPIO_GetPin(const EGPIO_Type *pEGPIO ,uint8_t port,uint8_t pin)
 * @brief        This API is used get the GPIO pin status.
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @return       returns Pin status
 */
STATIC INLINE boolean_t RSI_EGPIO_GetPin(const EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
{
#if defined(ROMDRIVER_PRESENT)
  return ROMAPI_EGPIO_API->egpio_get_pin(pEGPIO, port, pin);
#else
  return egpio_get_pin(pEGPIO, port, pin);
#endif
}

/**
 * @fn           STATIC INLINE boolean_t RSI_EGPIO_GetDir(const EGPIO_Type *pEGPIO,uint8_t port ,uint8_t pin)
 * @brief        This API is used to Get the Direction GPIO(Direction of the GPIO pin. '1' for INPUT,and '0'for OUTPUT)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @return       returns the GPIO direction value
 */
STATIC INLINE boolean_t RSI_EGPIO_GetDir(const EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
{
#if defined(ROMDRIVER_PRESENT)
  return ROMAPI_EGPIO_API->egpio_get_dir(pEGPIO, port, pin);
#else
  return egpio_get_dir(pEGPIO, port, pin);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_PinIntSel(EGPIO_Type *pEGPIO ,uint8_t intCh ,uint8_t port , uint8_t pin)
 * @brief        This API is used to select the pin for interrupt generation
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_PinIntSel(EGPIO_Type *pEGPIO, uint8_t intCh, uint8_t port, uint8_t pin)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_pin_int_sel(pEGPIO, intCh, port, pin);
#else
  egpio_pin_int_sel(pEGPIO, intCh, port, pin);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_SetIntFallEdgeEnable(EGPIO_Type *pEGPIO ,uint8_t intCh)
 * @brief        This API is used to set the pin interrupt mode configuration
 *               \n(enables interrupt generation when falling edge is detected on pin '1' for intr enabled and '0' for disabled)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_SetIntFallEdgeEnable(EGPIO_Type *pEGPIO, uint8_t intCh)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_set_int_fall_edge_enable(pEGPIO, intCh);
#else
  egpio_set_int_fall_edge_enable(pEGPIO, intCh);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_SetIntFallEdgeDisable(EGPIO_Type *pEGPIO ,uint8_t intCh)
 * @brief        This API to used to set the pin interrupt mode configuration
 *               \n(enables interrupt generation when falling edge is detected on pin '1' for intr enabled and '0' for disabled)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_SetIntFallEdgeDisable(EGPIO_Type *pEGPIO, uint8_t intCh)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_set_int_fall_edge_disable(pEGPIO, intCh);
#else
  egpio_set_int_fall_edge_disable(pEGPIO, intCh);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_SetIntRiseEdgeEnable(EGPIO_Type *pEGPIO ,uint8_t intCh)
 * @brief        This API to used to set the pin interrupt mode configuration
 *               \n(enables interrupt generation when rising edge is detected on pin '1' for intr enabled and '0' for disabled)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_SetIntRiseEdgeEnable(EGPIO_Type *pEGPIO, uint8_t intCh)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_set_int_rise_edge_enable(pEGPIO, intCh);
#else
  egpio_set_int_rise_edge_enable(pEGPIO, intCh);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_SetIntRiseEdgeDisable(EGPIO_Type *pEGPIO ,uint8_t intCh)
 * @brief        This API to used to set the pin interrupt mode configuration
 *               \n(enables interrupt generation when rising edge is detected on pin '1' for intr enabled '0' for disabled)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_SetIntRiseEdgeDisable(EGPIO_Type *pEGPIO, uint8_t intCh)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_set_int_rise_edge_disable(pEGPIO, intCh);
#else
  egpio_set_int_rise_edge_disable(pEGPIO, intCh);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_SetIntLowLevelEnable(EGPIO_Type *pEGPIO ,uint8_t intCh)
 * @brief        This API is used to set the pin interrupt mode configuration
 *               \n (enables interrupt generation when pin level is 0, '1' for intr enabled, '0' for disabled)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_SetIntLowLevelEnable(EGPIO_Type *pEGPIO, uint8_t intCh)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_set_int_low_level_enable(pEGPIO, intCh);
#else
  egpio_set_int_low_level_enable(pEGPIO, intCh);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_IntMask(EGPIO_Type *pEGPIO ,uint8_t intCh)
 * @brief        This API is used to set the pin interrupt mode configuration
 *               \n(Masks the interrupt. Interrupt will still be seen in status register when enabled
 *							 '1' for intr masked '0' for intr unmasked)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_IntMask(EGPIO_Type *pEGPIO, uint8_t intCh)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_int_mask(pEGPIO, intCh);
#else
  egpio_int_mask(pEGPIO, intCh);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_IntUnMask(EGPIO_Type *pEGPIO ,uint8_t intCh)
 * @brief        This API is used to used to set the pin interrupt mode configuration
 *               \n(UnMasks the interrupt.						
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_IntUnMask(EGPIO_Type *pEGPIO, uint8_t intCh)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_int_un_mask(pEGPIO, intCh);
#else
  egpio_int_un_mask(pEGPIO, intCh);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_SetIntLowLevelDisable(EGPIO_Type *pEGPIO ,uint8_t intCh)
 * @brief        This API is used to set the pin interrupt mode configuration
 *               \n(enables interrupt generation when pin level is 0 ,'1' for intr enabled '0' for disabled)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_SetIntLowLevelDisable(EGPIO_Type *pEGPIO, uint8_t intCh)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_set_int_low_level_disable(pEGPIO, intCh);
#else
  egpio_set_int_low_level_disable(pEGPIO, intCh);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_SetIntHighLevelEnable(EGPIO_Type *pEGPIO ,uint8_t intCh)
 * @brief        This API used to set the pin interrupt mode configuration
 *               \n(enables interrupt generation when pin level is 1, '1' for intr enabled '0' for disabled)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_SetIntHighLevelEnable(EGPIO_Type *pEGPIO, uint8_t intCh)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_set_int_high_level_enable(pEGPIO, intCh);
#else
  egpio_set_int_high_level_enable(pEGPIO, intCh);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_SetIntHighLevelDisable(EGPIO_Type *pEGPIO ,uint8_t intCh)
 * @brief        This API is used to used to set the pin interrupt mode configuration
                 \n(disables interrupt generation when pin level is 1 ,'1' for intr enabled '0' for disabled)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_SetIntHighLevelDisable(EGPIO_Type *pEGPIO, uint8_t intCh)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_set_int_high_level_disable(pEGPIO, intCh);
#else
  egpio_set_int_high_level_disable(pEGPIO, intCh);
#endif
}

/**
 * @fn           uint8_t RSI_EGPIO_GetIntStat(const EGPIO_Type *pEGPIO ,uint8_t intCh)
 * @brief        This API is used to get the pin interrupt status register
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @return       returns the interrupt status register
 */
STATIC INLINE uint8_t RSI_EGPIO_GetIntStat(const EGPIO_Type *pEGPIO, uint8_t intCh)
{
#if defined(ROMDRIVER_PRESENT)
  return ROMAPI_EGPIO_API->egpio_get_int_stat(pEGPIO, intCh);
#else
  return egpio_get_int_stat(pEGPIO, intCh);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_IntClr(EGPIO_Type *pEGPIO ,uint8_t intCh , uint8_t flags)
 * @brief        This API is used to clear the pin interrupt in status register
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    intCh   : GPIO pin interrupt channel number (0 to 7)
 * @param[in]    flags   : GPIO pin interrupt channel number (0 to 7)
													\n 2- \ref EGPIO_PIN_INT_CLR_FALLING
													\n 1- \ref EGPIO_PIN_INT_CLR_RISING
													\n 0- \ref INTERRUPT_STATUS_CLR
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_IntClr(EGPIO_Type *pEGPIO, uint8_t intCh, uint8_t flags)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_int_clr(pEGPIO, intCh, flags);
#else
  egpio_int_clr(pEGPIO, intCh, flags);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_SetPinMux(EGPIO_Type *pEGPIO ,uint8_t port , uint8_t pin , uint8_t mux)
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
STATIC INLINE void RSI_EGPIO_SetPinMux(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t mux)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_set_pin_mux(pEGPIO, port, pin, mux);
#else
  egpio_set_pin_mux(pEGPIO, port, pin, mux);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_UlpSocGpioMode(ULPCLK_Type *pULPCLK,uint8_t gpio,uint8_t mode)
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
STATIC INLINE void RSI_EGPIO_UlpSocGpioMode(ULPCLK_Type *pULPCLK, uint8_t gpio, uint8_t mode)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_ulp_soc_gpio_mode(pULPCLK, gpio, mode);
#else
  egpio_ulp_soc_gpio_mode(pULPCLK, gpio, mode);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_SetPortMask(EGPIO_Type *pEGPIO ,uint8_t port,uint8_t pin)
 * @brief        This API is used to set the EGPIO port mask. When set, pin is masked when written/read through PORT MASK REG.
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_SetPortMask(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_set_port_mask(pEGPIO, port, pin);
#else
  egpio_set_port_mask(pEGPIO, port, pin);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_SetPortUnMask(EGPIO_Type *pEGPIO ,uint8_t port,uint8_t pin)
 * @brief        This API is used to set the EGPIO port unmask. When set, pin is masked when written/read through PORT MASK REG.
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_SetPortUnMask(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_set_port_un_mask(pEGPIO, port, pin);
#else
  egpio_set_port_un_mask(pEGPIO, port, pin);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_PortMaskedLoad(EGPIO_Type *pEGPIO ,uint8_t port,  uint16_t val)
 * @brief        This API is used to set the EGPIO port mask load. When set, pin is masked when written/read through PORT MASK REG.
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    val     : Port value to be set
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_PortMaskedLoad(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_port_masked_load(pEGPIO, port, val);
#else
  egpio_port_masked_load(pEGPIO, port, val);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_SetPort(EGPIO_Type *pEGPIO ,uint8_t port , uint16_t val)
 * @brief        This API is used to set the port value.
 *               Sets the pin when corresponding bit is high. Writing zero has no effect
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : Port number to the EGPIO register instance
 * @param[in]    val     : Port value to be set
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_SetPort(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_set_port(pEGPIO, port, val);
#else
  egpio_set_port(pEGPIO, port, val);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_PortLoad(EGPIO_Type *pEGPIO ,uint8_t port , uint16_t val)
 * @brief        This API is used to load the port value.
 *               Loads the value on to pin on write. And reads the value of load register on read
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : Port number to the EGPIO register instance
 * @param[in]    val     : Port value to be set
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_PortLoad(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_port_load(pEGPIO, port, val);
#else
  egpio_port_load(pEGPIO, port, val);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_WordLoad(EGPIO_Type *pEGPIO ,uint8_t pin , uint16_t val)
 * @brief        This API is used to load the port value.
 *               Loads 1 on the pin when any of the bit in load value is 1. On read pass the bit status into all bits.                       
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    pin     : pin number to the EGPIO register instance
 * @param[in]    val     : Port value to be set
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_WordLoad(EGPIO_Type *pEGPIO, uint8_t pin, uint16_t val)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_word_load(pEGPIO, pin, val);
#else
  egpio_word_load(pEGPIO, pin, val);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_ClrPort(EGPIO_Type *pEGPIO ,uint8_t port , uint16_t val)
 * @brief        This API is used to clear the port value.
 *               Clears the pin when corresponding bit is high. Writing zero has no effect.
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : Port number
 * @param[in]    val     : Port value to be clear
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_ClrPort(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_clr_port(pEGPIO, port, val);
#else
  egpio_clr_port(pEGPIO, port, val);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_TogglePort(EGPIO_Type *pEGPIO ,uint8_t port , uint16_t val)
 * @brief        This API is used to toggle the port.
 *               Toggles the pin when corresponding bit is high. Writing zero has not effect.
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : Port number
 * @param[in]    val     : Port value to be toggle
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_TogglePort(EGPIO_Type *pEGPIO, uint8_t port, uint16_t val)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_toggle_port(pEGPIO, port, val);
#else
  egpio_toggle_port(pEGPIO, port, val);
#endif
}

/**
 * @fn           STATIC INLINE uint16_t RSI_EGPIO_GetPort(const EGPIO_Type *pEGPIO ,uint8_t port)
 * @brief        This API is used to used to get the EGPIO port value.
 *               Reads the value on GPIO pins irrespective of the pin mode.
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : Port number to be read
 * @return       port value
 */
STATIC INLINE uint16_t RSI_EGPIO_GetPort(const EGPIO_Type *pEGPIO, uint8_t port)
{
#if defined(ROMDRIVER_PRESENT)
  return ROMAPI_EGPIO_API->egpio_get_port(pEGPIO, port);
#else
  return egpio_get_port(pEGPIO, port);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_GroupIntOneEnable(EGPIO_Type *pEGPIO,uint8_t port,uint8_t pin)
 * @brief        This API is used to enable the group interrupt one ,  When set,
 *               the corresponding GPIO pin is selected for group interrupt 1 generation
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_GroupIntOneEnable(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_group_int_one_enable(pEGPIO, port, pin);
#else
  egpio_group_int_one_enable(pEGPIO, port, pin);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_GroupIntOneDisable(EGPIO_Type *pEGPIO,uint8_t port,uint8_t pin)
 * @brief        This API is used to disable the group interrupt one
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_GroupIntOneDisable(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_group_int_one_disable(pEGPIO, port, pin);
#else
  egpio_group_int_one_disable(pEGPIO, port, pin);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_GroupIntTwoEnable(EGPIO_Type *pEGPIO,uint8_t port,uint8_t pin)
 * @brief        This API is used to enable the group interrupt Two ,  When set,
 *               the corresponding GPIO pin is selected for group interrupt 2 generation
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : GPIO port number
 * @param[in]    pin     : GPIO pin number
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_GroupIntTwoEnable(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_group_int_two_enable(pEGPIO, port, pin);
#else
  egpio_group_int_two_enable(pEGPIO, port, pin);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_GroupIntMask(EGPIO_Type *pEGPIO ,uint8_t grpInt)
 * @brief        This API is used to configure the group interrupts(1-mask,0-unmask)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    grpInt  : Group interrupt number
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_GroupIntMask(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_group_int_mask(pEGPIO, grpInt);
#else
  egpio_group_int_mask(pEGPIO, grpInt);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_GroupIntUnMask(EGPIO_Type *pEGPIO ,uint8_t grpInt)
 * @brief        This API is used to configure the group interrupts(1-mask,0-unmask)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    grpInt  : Group interrupt number
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_GroupIntUnMask(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_group_int_un_Mask(pEGPIO, grpInt);
#else
  egpio_group_int_un_Mask(pEGPIO, grpInt);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_GroupIntEnable(EGPIO_Type *pEGPIO ,uint8_t grpInt)
 * @brief        This API is used to configure the group interrupts(1-enable, 0-disable)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    grpInt  : Group interrupt number
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_GroupIntEnable(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_group_int_enable(pEGPIO, grpInt);
#else
  egpio_group_int_enable(pEGPIO, grpInt);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_GroupIntDisable(EGPIO_Type *pEGPIO ,uint8_t grpInt)
 * @brief        This API is used to configure the group interrupts(1-enable, 0-disable)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    grpInt  : Group interrupt number
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_GroupIntDisable(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_group_int_disable(pEGPIO, grpInt);
#else
  egpio_group_int_disable(pEGPIO, grpInt);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_GroupIntLevel(EGPIO_Type *pEGPIO ,uint8_t grpInt)
 * @brief        This API is used to configure the group interrupts(0-level,1-edge)
 * @param[in]    pEGPIO   : Pointer to the EGPIO register instance
 * @param[in]    grpInt   : Group interrupt number
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_GroupIntLevel(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_group_int_level(pEGPIO, grpInt);
#else
  egpio_group_int_level(pEGPIO, grpInt);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_GroupIntEdge(EGPIO_Type *pEGPIO ,uint8_t grpInt)
 * @brief        This API is used to configure the group interrupts(0-level,1-edge)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    grpInt  : Group interrupt number
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_GroupIntEdge(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_group_int_edge(pEGPIO, grpInt);
#else
  egpio_group_int_edge(pEGPIO, grpInt);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_GroupIntAnd(EGPIO_Type *pEGPIO ,uint8_t grpInt)
 * @brief        This API is used to configure the group interrupts(0-AND ,1-Or)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    grpInt  : Group interrupt number
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_GroupIntAnd(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_group_int_and(pEGPIO, grpInt);
#else
  egpio_group_int_and(pEGPIO, grpInt);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_GroupIntOr(EGPIO_Type *pEGPIO ,uint8_t grpInt)
 * @brief        This API is used to configure the group interrupts(0- AND , 1-Or)
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    grpInt  : Group interrupt number
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_GroupIntOr(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_group_int_or(pEGPIO, grpInt);
#else
  egpio_group_int_or(pEGPIO, grpInt);
#endif
}

/**
 * @fn           STATIC INLINE uint32_t RSI_EGPIO_GroupIntStat(const EGPIO_Type *pEGPIO ,uint8_t grpInt)
 * @brief        This API to used to get the group interrupt status
 * @param[in]    pEGPIO   : Pointer to the EGPIO register instance
 * @param[in]    grpInt   : Group interrupt number
 * @return       returns the group interrupt status register
 */
STATIC INLINE uint32_t RSI_EGPIO_GroupIntStat(const EGPIO_Type *pEGPIO, uint8_t grpInt)
{
#if defined(ROMDRIVER_PRESENT)
  return ROMAPI_EGPIO_API->egpio_group_int_stat(pEGPIO, grpInt);
#else
  return egpio_group_int_stat(pEGPIO, grpInt);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_GroupIntWkeUpEnable(EGPIO_Type *pEGPIO ,uint8_t grpInt)
 * @brief        This API to used to Enable the group interrupt wakeup interrupt
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    grpInt  : Group interrupt number
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_GroupIntWkeUpEnable(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_group_int_wkeup_Enable(pEGPIO, grpInt);
#else
  egpio_group_int_wkeup_Enable(pEGPIO, grpInt);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_GroupIntWkeUpDisable(EGPIO_Type *pEGPIO ,uint8_t grpInt)
 * @brief        This API to used to Disable the group interrupt wakeup interrupt
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    grpInt  : Group interrupt number
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_GroupIntWkeUpDisable(EGPIO_Type *pEGPIO, uint8_t grpInt)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_group_int_wkeup_disable(pEGPIO, grpInt);
#else
  egpio_group_int_wkeup_disable(pEGPIO, grpInt);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_GroupIntClr(EGPIO_Type *pEGPIO ,uint8_t grpInt , uint8_t flags)
 * @brief        This API is used to used to clear the group interrupt status
 * @param[in]    pEGPIO     : Pointer to the EGPIO register instance
 * @param[in]    grpInt     : Group interrupt number
 * @param[in]    flags : clear flags
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_GroupIntClr(EGPIO_Type *pEGPIO, uint8_t grpInt, uint8_t flags)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_group_int_clr(pEGPIO, grpInt, flags);
#else
  egpio_group_int_clr(pEGPIO, grpInt, flags);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_GroupIntTwoDisable(EGPIO_Type *pEGPIO ,uint8_t port ,uint8_t pin)
 * @brief        This API is used to used to disable the group interrupt two
 * @param[in]    pEGPIO  : Pointer to the EGPIO register instance
 * @param[in]    port    : PORT number
 * @param[in]    pin     : PIN number
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_GroupIntTwoDisable(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_group_int_two_disable(pEGPIO, port, pin);
#else
  egpio_group_int_two_disable(pEGPIO, port, pin);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_SetGroupIntOnePol(EGPIO_Type *pEGPIO ,uint8_t port , uint8_t pin , uint8_t pol)
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
STATIC INLINE void RSI_EGPIO_SetGroupIntOnePol(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t pol)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_set_group_int_one_pol(pEGPIO, port, pin, pol);
#else
  egpio_set_group_int_one_pol(pEGPIO, port, pin, pol);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_SetGroupIntTwoPol(EGPIO_Type *pEGPIO ,uint8_t port,uint8_t pin , uint8_t pol)
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
STATIC INLINE void RSI_EGPIO_SetGroupIntTwoPol(EGPIO_Type *pEGPIO, uint8_t port, uint8_t pin, uint8_t pol)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_set_group_int_two_pol(pEGPIO, port, pin, pol);
#else
  egpio_set_group_int_two_pol(pEGPIO, port, pin, pol);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_HostPadsGpioModeEnable(uint8_t u8GpioNum)
 * @brief        This API is used to select the host pad gpios(25 to 30)
 * @param[in]    u8GpioNum  :  PAD number to be use
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_HostPadsGpioModeEnable(uint8_t u8GpioNum)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_host_pads_gpio_mode_enable(u8GpioNum);
#else
  egpio_host_pads_gpio_mode_enable(u8GpioNum);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_HostPadsGpioModeDisable(uint8_t u8GpioNum)
 * @brief        This API is used to deselect the host pad gpios(25 to 30)
 * @param[in]    u8GpioNum  :  PAD number to be use
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_HostPadsGpioModeDisable(uint8_t u8GpioNum)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_host_pads_gpio_mode_disable(u8GpioNum);
#else
  egpio_host_pads_gpio_mode_disable(u8GpioNum);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_PadSelectionEnable(uint8_t padNum)
 * @brief        This API is used to select the pad(0 to 21)
 * @param[in]    padNum  :  PAD number to be use
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_PadSelectionEnable(uint8_t padNum)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_pad_selection_enable(padNum);
#else
  egpio_pad_selection_enable(padNum);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_PadSelectionDisable(uint8_t padNum)
 * @brief        This API is used to deselect the pad(0 to 21)
 * @param[in]    padNum  :  PAD number to be use
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_PadSelectionDisable(uint8_t padNum)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_pad_selection_disable(padNum);
#else
  egpio_pad_selection_disable(padNum);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_PadReceiverEnable(uint8_t u8GpioNum)
 * @brief        This API is used to enable the receiver enable bit(REN)
 * @param[in]    u8GpioNum  :  GPIO num to be use
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_PadReceiverEnable(uint8_t u8GpioNum)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_pad_receiver_enable(u8GpioNum);
#else
  egpio_pad_receiver_enable(u8GpioNum);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_PadReceiverDisable(uint8_t u8GpioNum)
 * @brief        This API is used to Disable the receiver enable bit(REN)
 * @param[in]    u8GpioNum  :  GPIO num to be use
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_PadReceiverDisable(uint8_t u8GpioNum)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_pad_receiver_disable(u8GpioNum);
#else
  egpio_pad_receiver_disable(u8GpioNum);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_PadSdioConnected(void)
 * @brief        This API is used to use the SDIO pins(25 to 30) in M4 or NWP (0 for M4SS and 1 for TASS)
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_PadSdioConnected(void)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_pad_sdio_connected();
#else
  egpio_pad_sdio_connected();
#endif
}

/**
 * @fn           void RSI_EGPIO_PadDriverDisableState(uint8_t u8GpioNum , en_driver_state_t  endstate)
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
STATIC INLINE void RSI_EGPIO_PadDriverDisableState(uint8_t u8GpioNum, en_driver_state_t endstate)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_pad_driver_disable_state(u8GpioNum, endstate);
#else
  egpio_pad_driver_disable_state(u8GpioNum, endstate);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_PadDriverStrengthSelect(uint8_t u8GpioNum , en_driver_strength_select_t strength)
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
STATIC INLINE void RSI_EGPIO_PadDriverStrengthSelect(uint8_t u8GpioNum, en_driver_strength_select_t strength)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_pad_driver_strength_select(u8GpioNum, strength);
#else
  egpio_pad_driver_strength_select(u8GpioNum, strength);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_PadPowerOnStartEnable(uint8_t u8GpioNum ,uint8_t val)
 * @brief        This API is used to select Power on Start enable
 * @param[in]    u8GpioNum  :  GPIO number to be use
 * @param[in]    val        :  POS = 1 : Enables active pull down for invalid power;
 *               \n         :  POS = 0 : Active pull down capability disabled .
 *               \n When one of the power supplies is invalid and active high POS is set to 1,
 *               \n AD is pulled to weak 0. When POS is set to 0, PAD remains in a high-Z state. : Default 0
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_PadPowerOnStartEnable(uint8_t u8GpioNum, uint8_t val)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_pad_power_on_start_enable(u8GpioNum, val);
#else
  egpio_pad_power_on_start_enable(u8GpioNum, val);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_PadActiveHighSchmittTrigger(uint8_t u8GpioNum ,uint8_t val)
 * @brief        Active high Schmitt trigger (Hysteresis) select;
 *               \n SMT=0 for No hysteresis; Default value for reset is 1'b1 and others is 1'b0
 * @param[in]    u8GpioNum  : GPIO number to be use
 * @param[in]    val        : SMT=0 : No hysteresis; Default value for reset is 1'b1 and others is 1'b0
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_PadActiveHighSchmittTrigger(uint8_t u8GpioNum, uint8_t val)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_pad_active_high_schmitt_trigger(u8GpioNum, val);
#else
  egpio_pad_active_high_schmitt_trigger(u8GpioNum, val);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_PadSlewRateControll(uint8_t u8GpioNum ,uint8_t val)
 * @brief        this API is used to control the slew rate
 * @param[in]    u8GpioNum  :  GPIO number to be use
 * @param[in]    val        :  slew rate
 *               \n         -   SR = 0 : Slow (half frequency)
                 \n         -  SR = 1 : Fast  ,Default 1
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_PadSlewRateControll(uint8_t u8GpioNum, uint8_t val)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_pad_slew_rate_controll(u8GpioNum, val);
#else
  egpio_pad_slew_rate_controll(u8GpioNum, val);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_UlpPadReceiverEnable(uint8_t u8GpioNum)
 * @brief        This API is used to enable the REN for ULP
 * @param[in]    u8GpioNum   : GPIO number to be used
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_UlpPadReceiverEnable(uint8_t u8GpioNum)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_ulp_pad_receiver_enable(u8GpioNum);
#else
  egpio_ulp_pad_receiver_enable(u8GpioNum);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_UlpPadReceiverDisable(uint8_t u8GpioNum)
 * @brief        This API is used to enable the REN for ULP
 * @param[in]    u8GpioNum   : GPIO number to be used
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_UlpPadReceiverDisable(uint8_t u8GpioNum)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_ulp_pad_receiver_disable(u8GpioNum);
#else
  egpio_ulp_pad_receiver_disable(u8GpioNum);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_UlpPadDriverDisableState(uint8_t u8GpioNum , en_ulp_driver_disable_state_t  disablestate)
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
STATIC INLINE void RSI_EGPIO_UlpPadDriverDisableState(uint8_t u8GpioNum, en_ulp_driver_disable_state_t disablestate)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_ulp_pad_driver_disable_state(u8GpioNum, disablestate);
#else
  egpio_ulp_pad_driver_disable_state(u8GpioNum, disablestate);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_UlpPadDriverStrengthSelect(uint8_t u8GpioNum , en_ulp_driver_strength_select_t strength)
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
STATIC INLINE void RSI_EGPIO_UlpPadDriverStrengthSelect(uint8_t u8GpioNum, en_ulp_driver_strength_select_t strength)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_ulp_pad_driver_strength_select(u8GpioNum, strength);
#else
  egpio_ulp_pad_driver_strength_select(u8GpioNum, strength);
#endif
}

/**
 * @fn            STATIC INLINE void  RSI_EGPIO_UlpPadPowerOnStartEnable(uint8_t u8GpioNum ,uint8_t val )
 * @brief         Power-on-Start enable;
 * @param[in]     u8GpioNum  :   GPIO number to be use
 * @param[in]     val        :   POS = 1 : Enables active pull down for invalid power;
 *               \n          :    POS = 0 : Active pull down capability disabled .
 *               \n When one of the power supplies is invalid and active high POS is set to 1,
 *PAD is pulled to weak 0. When POS is set to 0, PAD remains in a high Z state. : Default 0
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_UlpPadPowerOnStartEnable(uint8_t u8GpioNum, uint8_t val)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_ulp_pad_power_on_start_enable(u8GpioNum, val);
#else
  egpio_ulp_pad_power_on_start_enable(u8GpioNum, val);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_UlpPadActiveHighSchmittTrigger(uint8_t u8GpioNum ,uint8_t val )
 * @brief        Active high Schmitt trigger (Hysteresis) select;
 * @param[in]    u8GpioNum  :  GPIO number to be use
 * @param[in]    val        :  SMT=0 : No hysteresis; Default value for reset is 1'b1 and others is 1'b0
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_UlpPadActiveHighSchmittTrigger(uint8_t u8GpioNum, uint8_t val)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_ulp_pad_active_high_schmitt_trigger(u8GpioNum, val);
#else
  egpio_ulp_pad_active_high_schmitt_trigger(u8GpioNum, val);
#endif
}

/**
 * @fn           STATIC INLINE void  RSI_EGPIO_UlpPadSlewRateControll(uint8_t u8GpioNum ,uint8_t val )
 * @brief        Slew Rate Control
 * @param[in]    u8GpioNum  :   GPIO number to be use
 * @param[in]    val        :   slew rate
 *               \n         -   SR = 0 : Slow (half frequency); SR = 1 for Fast , Default 1
 * @return       None
 */
STATIC INLINE void RSI_EGPIO_UlpPadSlewRateControll(uint8_t u8GpioNum, uint8_t val)
{
#if defined(ROMDRIVER_PRESENT)
  ROMAPI_EGPIO_API->egpio_ulp_pad_slew_rate_controll(u8GpioNum, val);
#else
  egpio_ulp_pad_slew_rate_controll(u8GpioNum, val);
#endif
}

#ifdef __cplusplus
}
#endif

#endif /*__RSI_ROM_EGPIO_H__*/
/** @} */
/* @}end of  RSI_EGPIO_DRIVERS */
