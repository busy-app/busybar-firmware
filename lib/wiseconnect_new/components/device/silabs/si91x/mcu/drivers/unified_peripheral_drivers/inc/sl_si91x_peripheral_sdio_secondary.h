/******************************************************************************
* @file  sl_si91x_peripheral_sdio_secondary.h
* @brief sdio secondary peripheral API's
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
#ifndef __SL_SI91X_PERIPHERAL_SDIO_SECONDARY_H_
#define __SL_SI91X_PERIPHERAL_SDIO_SECONDARY_H_

#include "si91x_device.h"
#include "sl_component_catalog.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SL_CATALOG_KERNEL_PRESENT
#include "rsi_m4.h"
#endif

/***************************************************************************/
/**
 * @addtogroup SDIO
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 * 
 ******************************************************************************/

/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/
#define SDIO SDIO0 ///< SDIO Base Address

#define SDIO_Handler  HIF1_IRQHandler ///< HIF1 interrrupt handler.
#define GPDMA_Handler IRQ031_Handler  ///< GPDMA interrupt handler.

/// SDIO Write FIFO Data Register.
#define RX_SOURCE_ADDR 0x20200080 ///< SDIO write FIFO Data Register.

#define TX_SOURCE_ADDR 0x20200040 ///< SDIO Read FIFO Data Register.

#define SDIO_MODE_SELECT    0x0705 ///< SDIO Mode Select.
#define MASK_HOST_INTERRUPT 0xF0   ///< MASK Host Interrupt.

#define M4_MISC_CONFIG_BASE 0x46008000 ///< SDIO MISC Configuration.
#define SDIO_BASE           0x20200000 ///< SDIO BASE Address

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
#define RX_NUM_CHUNKS           (*(volatile uint32_t *)(SDIO_BASE + 0x242))
#define M4_HOST_INTR_MASK_REG   (*(volatile uint32_t *)(M4_MISC_CONFIG_BASE + 0x00))
#define M4_HOST_INTR_STATUS_REG (*(volatile uint32_t *)(M4_MISC_CONFIG_BASE + 0x04))
#define M4_HOST_INTR_CLEAR      (*(volatile uint32_t *)(M4_MISC_CONFIG_BASE + 0x08))
#ifndef MISC_CFG_HOST_CTRL
#define MISC_CFG_HOST_CTRL (*(volatile uint32_t *)(M4_MISC_CONFIG_BASE + 0x0C))
#endif

/** @endcond */

// @brief SDIO Function1 interrupt enable bits.
#define SL_SDIO_WR_INT_EN      BIT(0) ///< This bit is used to enable CMD53 write interrupt.
#define SL_SDIO_RD_INT_EN      BIT(1) ///< This bit is used to enable CMD53 read interrupt.
#define SL_SDIO_CSA_INT_EN     BIT(2) ///< This bit is used to enable CMD53 CSA interrupt.
#define SL_SDIO_CMD52_INT_EN   BIT(3) ///< This bit is used to enable CMD52 interrupt.
#define SL_SDIO_PWR_LEV_INT_EN BIT(4) ///< This bit is used to enable power level change interrupt.
#define SL_SDIO_CRC_ERR_INT_EN BIT(5) ///< This bit is used to enable CRC error interrupt.
#define SL_SDIO_ABORT_INT_EN   BIT(6) ///< This bit is used to enable abort interrupt.
#define SL_SDIO_TOUT_INT_EN    BIT(7) ///< This bit is used to enable "read FIFO wait time over" interrupt.

// @brief SDIO Function1 interrupt mask bits.
#define SL_SDIO_WR_INT_MSK      BIT(0) ///< This bit is used to mask CMD53 write interrupt.
#define SL_SDIO_RD_INT_MSK      BIT(1) ///< This bit is used to mask CMD53 read interrupt.
#define SL_SDIO_CSA_INT_MSK     BIT(2) ///< This bit is used to mask CMD53 CSA interrupt.
#define SL_SDIO_CMD52_INT_MSK   BIT(3) ///< This bit is used to mask CMD52 interrupt.
#define SL_SDIO_PWR_LEV_INT_MSK BIT(4) ///< This bit is used to mask power level change interrupt.
#define SL_SDIO_CRC_ERR_INT_MSK BIT(5) ///< This bit is used to mask CRC error interrupt.
#define SL_SDIO_ABORT_INT_MSK   BIT(6) ///< This bit is used to mask abort interrupt.
#define SL_SDIO_TOUT_INT_MSK    BIT(7) ///< This bit is used to mask "read FIFO wait time over" interrupt.
#define SL_SDIO_ERROR_INT_MSK \
  (SL_SDIO_CRC_ERR_INT_MSK | SL_SDIO_ABORT_INT_MSK | SL_SDIO_TOUT_INT_MSK) ///< Combined error interrupt mask.

// @brief SDIO Function1 interrupt unmask bits.
#define SL_SDIO_WR_INT_UNMSK      BIT(0) ///< This bit is used to unmask CMD53 write interrupt.
#define SL_SDIO_RD_INT_UNMSK      BIT(1) ///< This bit is used to unmask CMD53 read interrupt.
#define SL_SDIO_CSA_INT_UNMSK     BIT(2) ///< This bit is used to unmask CMD53 CSA interrupt.
#define SL_SDIO_CMD52_INT_UNMSK   BIT(3) ///< This bit is used to unmask CMD52 interrupt.
#define SL_SDIO_PWR_LEV_INT_UNMSK BIT(4) ///< This bit is used to unmask power level change interrupt.
#define SL_SDIO_CRC_ERR_INT_UNMSK BIT(5) ///< This bit is used to unmask CRC error interrupt.
#define SL_SDIO_ABORT_INT_UNMSK   BIT(6) ///< This bit is used to unmask abort interrupt.
#define SL_SDIO_TOUT_INT_UNMSK    BIT(7) ///< This bit is used to unmask "read FIFO wait time over" interrupt.
#define SL_SDIO_ERROR_INT_UNMSK \
  (SL_SDIO_CRC_ERR_INT_UNMSK | SL_SDIO_ABORT_INT_UNMSK | SL_SDIO_TOUT_INT_UNMSK) ///< Combined error interrupt unmask.

/***************************************************************************/
/**
 * To initialize the SDIO Secondary at peripheral layer.
 * This API initializes the SDIO secondary, by default SDIO pin access with NWP
 * this API gives the pin access to M4, selects the SDIO mode and enable the sdio
 * primary interrupts
 ******************************************************************************/
void sl_si91x_sdio_secondary_peripheral_init(void);

/***************************************************************************/
/**
 * To enable the SDIO interrupts.
 *
 * @param[in] flags Interrupt Flag which needs to be enabled.
 *          Different interrupts flag which can be enabled are:
 *          @ref SL_SDIO_WR_INT_EN      
 * 
 *          @ref SL_SDIO_RD_INT_EN      
 * 
 *          @ref SL_SDIO_CSA_INT_EN     
 * 
 *          @ref SL_SDIO_CMD52_INT_EN   
 * 
 *          @ref SL_SDIO_PWR_LEV_INT_EN 
 * 
 *          @ref SL_SDIO_CRC_ERR_INT_EN 
 * 
 *          @ref SL_SDIO_ABORT_INT_EN   
 * 
 *          @ref SL_SDIO_TOUT_INT_EN    
 * 
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_sdio_secondary_enable_interrupts(uint32_t flags)
{
  SDIO->SDIO_INTR_FN1_ENABLE_REG |= flags;
  NVIC_EnableIRQ(HIF0_IRQn);
}

/***************************************************************************/
/**
 * To disable the SDIO interrupts.
 *
 * @param[in] flags Interrupt Flag which needs to be disabled.
 *          Different interrupts flags which can be disabled are:
 *          @ref SL_SDIO_WR_INT_EN      
 * 
 *          @ref SL_SDIO_RD_INT_EN      
 * 
 *          @ref SL_SDIO_CSA_INT_EN     
 * 
 *          @ref SL_SDIO_CMD52_INT_EN   
 * 
 *          @ref SL_SDIO_PWR_LEV_INT_EN 
 * 
 *          @ref SL_SDIO_CRC_ERR_INT_EN 
 * 
 *          @ref SL_SDIO_ABORT_INT_EN   
 * 
 *          @ref SL_SDIO_TOUT_INT_EN    
 * 
 ******************************************************************************/
__STATIC_INLINE void sl_si91x_sdio_secondary_disable_interrupts(uint32_t flags)
{
  SDIO->SDIO_INTR_FN1_ENABLE_REG &= (~flags);
  NVIC_DisableIRQ(HIF0_IRQn);
}

/***************************************************************************/
/**
 * To set the interrupts i.e., unmask according to the flags passed in the parameter.
 * @pre \ref sl_si91x_sdio_secondary_enable_interrupts();
 *
 * @param[in] flags Interrupt flags which needs to be set.
 *          Different interrupts flags which needs to be set:
 *          @ref SL_SDIO_WR_INT_UNMSK      
 * 
 *          @ref SL_SDIO_RD_INT_UNMSK      
 * 
 *          @ref SL_SDIO_CSA_INT_UNMSK     
 * 
 *          @ref SL_SDIO_CMD52_INT_UNMSK   
 * 
 *          @ref SL_SDIO_PWR_LEV_INT_UNMSK 
 * 
 *          @ref SL_SDIO_CRC_ERR_INT_UNMSK 
 * 
 *          @ref SL_SDIO_ABORT_INT_UNMSK   
 * 
 *          @ref SL_SDIO_TOUT_INT_UNMSK    
 * 
 ******************************************************************************/
__INLINE void sl_si91x_sdio_secondary_set_interrupts(uint32_t flags)
{
  SDIO->SDIO_INTR_FN1_UNMASK_REG |= flags;
}

/***************************************************************************/
/**
 * To clear the interrupts i.e., mask according to the flags passed in the parameter.
 * @pre \ref sl_si91x_sdio_secondary_disable_interrupts();
 *
 * @param[in] flags Interrupt flags which needs to be masked.
 *          @ref SL_SDIO_WR_INT_MSK      
 * 
 *          @ref SL_SDIO_RD_INT_MSK      
 * 
 *          @ref SL_SDIO_CSA_INT_MSK     
 * 
 *          @ref SL_SDIO_CMD52_INT_MSK   
 * 
 *          @ref SL_SDIO_PWR_LEV_INT_MSK 
 * 
 *          @ref SL_SDIO_CRC_ERR_INT_MSK 
 * 
 *          @ref SL_SDIO_ABORT_INT_MSK   
 * 
 *          @ref SL_SDIO_TOUT_INT_MSK    
 * 
 ******************************************************************************/
__INLINE void sl_si91x_sdio_secondary_clear_interrupts(uint32_t flags)
{
  SDIO->SDIO_INTR_FN1_MASK_REG |= flags;
  SDIO->SDIO_INTR_FN1_STATUS_CLEAR_REG |= flags;
}

/***************************************************************************/
/**
 * To get the pending function interrupt.
 *
 * @return Return the pending interrupt status.
 ******************************************************************************/
__INLINE uint32_t sl_si91x_sdio_secondary_get_pending_interrupts(void)
{
  return SDIO->SDIO_INTR_STATUS_REG;
}

/***************************************************************************/
/**
 * To get all the enabled interrupts in function1.
 *
 * @return No of interrupts enabled.
 ******************************************************************************/
__INLINE uint32_t sl_si91x_sdio_secondary_get_enabled_interrupts(void)
{
  return SDIO->SDIO_INTR_FN1_UNMASK_REG;
}

/***************************************************************************/
/**
 * To get all the enabled interrupts in function1 and pending interrupts.
 *
 * @return No of interrupts enabled.
 ******************************************************************************/
__INLINE uint32_t sl_si91x_sdio_secondary_get_enabled_pending_interrupts(void)
{
  return (SDIO->SDIO_INTR_STATUS_REG & SDIO->SDIO_INTR_FN1_UNMASK_REG);
}

/**************************************************************************/
/**
 * To get block count for the last received CMD53.
 *
 * @return No of block counts.
 ******************************************************************************/
__INLINE uint32_t sl_si91x_sdio_secondary_get_block_cnt(void)
{
  // return the block count for last received command 53.
  return SDIO->SDIO_BLK_CNT_REG;
}

/**************************************************************************/
/**
 * To get the length of each for the last received CMD53.
 *
 * @return Length of each block.
 ******************************************************************************/
__INLINE uint32_t sl_si91x_sdio_secondary_get_block_len(void)
{
  // return the length of each block for last received command 53.
  return SDIO->SDIO_BLK_LEN_REG;
}

/**************************************************************************/
/**
 * To set no of blocks to be transferred.
 * This API is used when transferring the data from secondary to primary in
 * block mode.
 *
 * @param[in] no_of_blocks  no of blocks to be transfered.
 ******************************************************************************/
__INLINE void sl_si91x_sdio_secondary_set_tx_blocks(uint8_t no_of_blocks)
{
  RX_NUM_CHUNKS = no_of_blocks;
}

/** @} (end addtogroup SDIO Secondary) */

#ifdef __cplusplus
}
#endif

#endif //__SL_SI91X_PERIPHERAL_SDIO_SECONDARY_H_
