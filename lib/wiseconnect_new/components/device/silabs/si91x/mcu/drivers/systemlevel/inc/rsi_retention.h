/*******************************************************************************
* @file  rsi_retention.h
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

/**
 * Includes
 */

#ifndef __RSI_RETENTION_H__
#define __RSI_RETENTION_H__
#include "rsi_ccp_common.h"
#include "rsi_power_save.h"

#ifdef __cplusplus
extern "C" {
#endif

/*NPSS GPIO PIN MUX VALEUS*/
#define NPSS_GPIO_STATUS         (*(volatile uint32_t *)(NPSS_INTR_BASE + 0x14))
#define NPSS_GPIO_CONFIG_REG     (*(volatile uint32_t *)(NPSS_INTR_BASE + 0x10))
#define NPSS_GPIO_CONFIG_CLR_REG (*(volatile uint32_t *)(NPSS_INTR_BASE + 0x8))

/*NPSS GPIO PIN MUX VALEUS*/
#define NPSSGPIO_PIN_MUX_MODE0 0
#define NPSSGPIO_PIN_MUX_MODE1 1
#define NPSSGPIO_PIN_MUX_MODE2 2
#define NPSSGPIO_PIN_MUX_MODE3 3
#define NPSSGPIO_PIN_MUX_MODE4 4
#define NPSSGPIO_PIN_MUX_MODE5 5
#define NPSSGPIO_PIN_MUX_MODE6 6
#define NPSSGPIO_PIN_MUX_MODE7 7
/*@note : Please refer to pin MUX excel to configure in desired mode
 * */
/*EDGE INTERRUPT MODE */
#define RISING_EDGE         0
#define FALLING_EDGE        1
#define BOTH_FALL_RISE_EDGE 3

/*NPSS GPIO pin interrupt edge configuration */
#define NPSS_INTR_RISING_EDGE  1
#define NPSS_INTR_FALLING_EDGE 0

/*NPSS GPIO pin direction */
#define NPSS_GPIO_DIR_INPUT  1
#define NPSS_GPIO_DIR_OUTPUT 0

/*NPSS GPIO pin interrupt levels*/
#define NPSS_GPIO_INTR_HIGH 1
#define NPSS_GPIO_INTR_LOW  0

/*NPSS GPIO pin defines */
#define NPSS_GPIO_0 0
#define NPSS_GPIO_1 1
#define NPSS_GPIO_2 2
#define NPSS_GPIO_3 3
#define NPSS_GPIO_4 4

/*NPSS GPIO Interrupt defines */
#define NPSS_GPIO_0_INTR BIT(0)
#define NPSS_GPIO_1_INTR BIT(1)
#define NPSS_GPIO_2_INTR BIT(2)
#define NPSS_GPIO_3_INTR BIT(3)
#define NPSS_GPIO_4_INTR BIT(4)

/**
 * \ingroup   RSI_SPECIFIC_DRIVERS
 * \defgroup RSI_NPSSGPIO RSI:RS1xxxx NPSSGPIO 
 *  @{
 *
 */

/**
* @brief         This API is used to set the NPSS GPIO pin MUX (mode)
 *@param[in]  pin: is NPSS GPIO pin number (0...4)
 *@param[in]  mux : is NPSS GPIO  MUX value
 *@return  : none
 * */
STATIC INLINE void RSI_NPSSGPIO_SetPinMux(uint8_t pin, uint8_t mux)
{
  MCU_RET->NPSS_GPIO_CNTRL[pin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_MODE = (unsigned int)(mux & 0x07);
}

/**
 * @fn         void RSI_NPSSGPIO_InputBufferEn(uint8_t pin , boolean_t enable)
* @brief        This API is used to enable/disable NPSS GPIO input buffer
 *@param[in] pin: is NPSS GPIO pin number (0...4)
 *@param[in]  enable: is enable / disable NPSS GPIO input buffer
 *           1- Enable
 *           0- Disable
 *@return  : none
 * */
STATIC INLINE void RSI_NPSSGPIO_InputBufferEn(uint8_t pin, boolean_t enable)
{
  MCU_RET->NPSS_GPIO_CNTRL[pin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_REN = (unsigned int)(enable & 0x01);
}

/**
 * @brief    This API is used to set the direction of the NPSS GPIO
 *@param[in]  pin: is NPSS GPIO pin number (0...4)
 *@param[in]  dir: is direction value (Input / Output)
 *           1- Input Direction
 *           0- Output Direction
 *@return  : none
 * */
STATIC INLINE void RSI_NPSSGPIO_SetDir(uint8_t pin, boolean_t dir)
{
  MCU_RET->NPSS_GPIO_CNTRL[pin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_OEN = (unsigned int)(dir & 0x01);
}

/**
 * @brief    This API is used to Get the direction of the NPSS GPIO
 *@param[in]  pin: is NPSS GPIO pin number (0...4)
 * @return  : returns the GPIO pin direction
 * */
STATIC INLINE boolean_t RSI_NPSSGPIO_GetDir(uint8_t pin)
{
  return MCU_RET->NPSS_GPIO_CNTRL[pin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_OEN;
}

/**
 * @brief         This API is used to set the NPSS GPIO pin value
 *@param[in]  pin: is NPSS GPIO pin number (0...4)
 *@param[in]  val: is NPSS GPIO pin value 0 or 1
 * @return    none
 */
STATIC INLINE void RSI_NPSSGPIO_SetPin(uint8_t pin, boolean_t val)
{
  MCU_RET->NPSS_GPIO_CNTRL[pin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_OUT = (unsigned int)(val & 0x01);
}

/**
 * @brief         This API is used to Get the NPSS GPIO pin value
 *@param[in]  pin: is NPSS GPIO pin number (0...4)
 * @return    returns the pin logical state of pin
 */
STATIC INLINE boolean_t RSI_NPSSGPIO_GetPin(uint8_t pin)
{
  return (NPSS_GPIO_STATUS >> pin) & 0x01;
}

/**
* @brief        This API is used to select  NPSS GPIO wake up detection when in sleep
 *@param[in] pin: is NPSS GPIO pin number (0...4)
 *@param[in] level :Gpio polarity to wake up from sleep
 *           1 - When signal is High
 *           0 - When signal is Low
 *@return  : none
 * */
STATIC INLINE void RSI_NPSSGPIO_SetPolarity(uint8_t pin, boolean_t level)
{
  MCU_RET->NPSS_GPIO_CNTRL[pin].NPSS_GPIO_CTRLS_b.NPSS_GPIO_POLARITY = (unsigned int)(level & 0x01);
}

/**
 * @brief         This API is used to set the GPIO to wake from deep sleep
 * @param[in]   npssGpioPinIntr: OR'ed values of the NPSS GPIO interrupts
 * @return    none
 */
STATIC INLINE void RSI_NPSSGPIO_SetWkpGpio(uint8_t npssGpioPinIntr)
{
  MCU_FSM->GPIO_WAKEUP_REGISTER |= npssGpioPinIntr;
}

/**
 * @brief         This API is used to clear the GPIO to wake from deep sleep
 * @param[in]   npssGpioPinIntr: OR'ed values of the NPSS GPIO interrupts
 * @return    none
 */
STATIC INLINE void RSI_NPSSGPIO_ClrWkpGpio(uint8_t npssGpioPinIntr)
{
  MCU_FSM->GPIO_WAKEUP_REGISTER &= ~npssGpioPinIntr;
}

/**
 * @brief         This API is used to mask the NPSS GPIO interrupt
 * @param[in]   npssGpioPinIntr: OR'ed values of the NPSS GPIO interrupts
 * @return    none
 */
STATIC INLINE void RSI_NPSSGPIO_IntrMask(uint8_t npssGpioPinIntr)
{
  NPSS_INTR_MASK_SET_REG = (npssGpioPinIntr << 1);
}

/**
 * @brief         This API is used to un mask the NPSS GPIO interrupt
 * @param[in]   npssGpioPinIntr: OR'ed values of the NPSS GPIO interrupts
 * @return    none
 */
STATIC INLINE void RSI_NPSSGPIO_IntrUnMask(uint8_t npssGpioPinIntr)
{
  NPSS_INTR_MASK_CLR_REG = (npssGpioPinIntr << 1);
}

/**
 * @brief         This API is used to un mask the NPSS GPIO interrupt
 * @param[in]   npssGpioPinIntr: OR'ed values of the NPSS GPIO interrupts
 * @return    none
 */
STATIC INLINE void RSI_NPSSGPIO_SetIntFallEdgeEnable(uint8_t npssGpioPinIntr)
{
  NPSS_GPIO_CONFIG_REG |= (npssGpioPinIntr << 8);
}

/**
 * @brief         This API is used to un mask the NPSS GPIO interrupt
 * @param[in]   npssGpioPinIntr: OR'ed values of the NPSS GPIO interrupts
 * @return    none
 */
STATIC INLINE void RSI_NPSSGPIO_ClrIntFallEdgeEnable(uint8_t npssGpioPinIntr)
{
  NPSS_GPIO_CONFIG_REG &= (uint32_t)(~(npssGpioPinIntr << 8));
}

/**
 * @brief         This API is used to Set the rise edge interrupt detection for NPSS GPIO
 * @param[in]   npssGpioPinIntr: OR'ed values of the NPSS GPIO interrupts
 * @return    none
 */
STATIC INLINE void RSI_NPSSGPIO_SetIntRiseEdgeEnable(uint8_t npssGpioPinIntr)
{
  NPSS_GPIO_CONFIG_REG |= (npssGpioPinIntr << 0);
}

/**
 * @brief         This API is used to  Enable rise edge interrupt detection for NPSS GPIO
 * @param[in]   npssGpioPinIntr: OR'ed values of the NPSS GPIO interrupts
 * @return    none
 */
STATIC INLINE void RSI_NPSSGPIO_ClrIntRiseEdgeEnable(uint8_t npssGpioPinIntr)
{
  NPSS_GPIO_CONFIG_REG &= (uint32_t)(~(npssGpioPinIntr << 0));
}

/**
 * @brief     This API is used to un mask the NPSS GPIO interrupt
 * @param[in]   npssGpioPinIntr: OR'ed values of the NPSS GPIO interrupts
 * @return    none
 */
STATIC INLINE void RSI_NPSSGPIO_SetIntLevelHighEnable(uint8_t npssGpioPinIntr)
{
  NPSS_GPIO_CONFIG_REG |= (npssGpioPinIntr << 24);
}

/**
 * @brief         This API is used to un mask the NPSS GPIO interrupt
 * @param[in]   npssGpioPinIntr: OR'ed values of the NPSS GPIO interrupts
 * @return    none
 */
STATIC INLINE void RSI_NPSSGPIO_ClrIntLevelHighEnable(uint8_t npssGpioPinIntr)
{
  NPSS_GPIO_CONFIG_REG &= (uint32_t)(~(npssGpioPinIntr << 24));
}

/**
 * @brief         This API is used to un mask the NPSS GPIO interrupt
 * @param[in]   npssGpioPinIntr: OR'ed values of the NPSS GPIO interrupts
 * @return    none
 */
STATIC INLINE void RSI_NPSSGPIO_SetIntLevelLowEnable(uint8_t npssGpioPinIntr)
{
  NPSS_GPIO_CONFIG_REG |= (npssGpioPinIntr << 16);
}

/**
 * @brief         This API is used clear the interrupt low level enable
 * @param[in]   npssGpioPinIntr: OR'ed values of the NPSS GPIO interrupts
 * @return    none
 */
STATIC INLINE void RSI_NPSSGPIO_ClrIntLevelLowEnable(uint8_t npssGpioPinIntr)
{
  NPSS_GPIO_CONFIG_REG &= (uint32_t)(~(npssGpioPinIntr << 16));
}

/**
 * @brief         This API is used to clear NPSS GPIO interrupt
 * @param[in]   npssGpioPinIntr: OR'ed values of the NPSS GPIO interrupts
 * @return    none
 */
STATIC INLINE void RSI_NPSSGPIO_ClrIntr(uint8_t npssGpioPinIntr)
{
  NPSS_INTR_CLEAR_REG = (npssGpioPinIntr << 1);
}

/**
 * @brief         This API is used to get the NPSS GPIO interrupt status
 * @return    returns the GPIO status
 */
STATIC INLINE uint8_t RSI_NPSSGPIO_GetIntrStatus(void)
{
  return (NPSS_INTR_STATUS_REG >> 1) & 0x1F;
}
/*
 *@}
 */

#ifdef __cplusplus
}
#endif

/*End of file not truncated*/
#endif /*__RSI_RETENTION_H__*/
