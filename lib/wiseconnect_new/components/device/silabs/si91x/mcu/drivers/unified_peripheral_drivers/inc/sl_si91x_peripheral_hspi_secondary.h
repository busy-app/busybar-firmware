/******************************************************************************
* @file sl_si91x_peripheral_hspi_secondary.h
* @brief HSPI SLAVE API implementation
*******************************************************************************
* # License
* <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef __SL_SI91X_PERIPHERAL_HSPI_SECONDARY_H_
#define __SL_SI91X_PERIPHERAL_HSPI_SECONDARY_H_

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
 * @addtogroup HSPI
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 * 
 ******************************************************************************/

/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/
#define HSPI SPI_SLAVE ///< HSPI Base Address

#define SL_HSPI_MODE_SELECT         0x0605 ///< HSPI Mode Select.
#define SL_HSPI_MASK_HOST_INTERRUPT 0xF0   ///< MASK Host Interrupt.

#define SL_HSPI_M4_MISC_CONFIG_BASE 0x46008000 ///< HSPI MISC Configuration.

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
#define SL_HSPI_M4_HOST_INTR_MASK_REG   (*(volatile uint32_t *)(SL_HSPI_M4_MISC_CONFIG_BASE + 0x00))
#define SL_HSPI_M4_HOST_INTR_STATUS_REG (*(volatile uint32_t *)(SL_HSPI_M4_MISC_CONFIG_BASE + 0x04))
#define SL_HSPI_M4_HOST_INTR_CLEAR      (*(volatile uint32_t *)(SL_HSPI_M4_MISC_CONFIG_BASE + 0x08))
#ifndef SL_HSPI_MISC_CFG_HOST_CTRL
#define SL_HSPI_MISC_CFG_HOST_CTRL (*(volatile uint32_t *)(SL_HSPI_M4_MISC_CONFIG_BASE + 0x0C))
#endif

// SPI slave Interrupt bits
#define SL_HSPI_INT_WR          (1 << 0)
#define SL_HSPI_INT_RD          (1 << 1)
#define SL_HSPI_INT_CS_DEASSERT (1 << 2)

#define SL_HSPI_EVENT_WRITE       0x0001
#define SL_HSPI_EVENT_READ        0x0002
#define SL_HSPI_EVENT_CS_DEASSERT 0x0003

#define SL_HSPI_ALIGNMENT_VALUE 3
#define SL_HSPI_WORD_SIZE       4

// MISC host registers
#define SL_HSPI_MCSS_MISC_CFG_HOST_CTRL (*(volatile uint32_t *)(0x46008000 + 0x0C))

#define SL_HSPI_NWPAON_MEM_HOST_ACCESS_CTRL_CLEAR (0x41300000 + 0x004)
#define SL_HSPI_NWPAON_MEM_HOST_ACCESS_CTRL_SET   (0x41300000)

#define SL_HSPI_READY_FROM_RCORE          BIT(0)
#define SL_HSPI_LOAD_HOST_MODE            BIT(10)
#define SL_HSPI_HOST_SEL_AS_SPI           BIT(9)
#define SL_HSPI_SDIO_SPI_PROG_SEL         (BIT(15))
#define SL_HSPI_BOOT_MODE_EN              BIT(0)
#define SL_HSPI_HOST_SPI_BUS_ERR_OEN      BIT(2)
#define SL_HSPI_MISC_CFG_RST_LATCH_STATUS (*(volatile uint32_t *)(0x41050012))
#define SL_HSPI_HOST_CONTROL_TO_MCU       (BIT(5))
#define SL_HSPI_RECEIVE_EVENT             1
#if (SL_HSPI_DMA == ENABLE)
#define SL_HSPI_SEND_EVENT 0
#endif
#if (SL_HSPI_DMA == DISABLE)
#define SL_HSPI_SEND_EVENT 2
#endif

/*******************************************************************************
 * @brief Reads data from the HSPI FIFO.
 *
 * @pre Pre-condition:
 *      - The HSPI peripheral must be initialized.
 *
 * @param[in] data Pointer to the buffer where the read data will be stored.
 * @param[in] num  Number of bytes to read.
 * @return Number of bytes read from the HSPI FIFO.
 ******************************************************************************/
uint32_t hspi_secondary_read_fifo(void *data, uint32_t num);

/*******************************************************************************
 * @brief Writes data to the HSPI FIFO.
 *
 * @pre Pre-condition:
 *      - The HSPI peripheral must be initialized.
 *
 * @param[in] data Pointer to the buffer containing the data to be written.
 * @param[in] num  Number of bytes to write.
 * @return Number of bytes written to the HSPI FIFO.
 ******************************************************************************/
uint32_t hspi_secondary_write_fifo(const void *data, uint32_t num);

/***************************************************************************/
/**
 * @brief Initializes the HSPI Secondary at the peripheral layer.
 *
 * @details Initializes the HSPI secondary, grants pin access to M4, selects the HSPI mode, and enables HSPI primary interrupts.
 ******************************************************************************/
void hspi_secondary_peripheral_init(void);

/***************************************************************************/
/**
 * @brief De-Initializes the HSPI Secondary at the peripheral layer.
 *
 * @details De-Initializes the HSPI secondary
 ******************************************************************************/
void hspi_secondary_peripheral_deinit(void);

/***************************************************************************/
/**
 * @brief Enables the HSPI interrupts.
 *
 * @param[in] flags Interrupt flags to be enabled.
 ******************************************************************************/
__STATIC_INLINE void hspi_secondary_enable_interrupts(uint32_t flags)
{
  SPI_SLAVE->SPI_INTR_EN |= flags;
  SPI_SLAVE->SPI_INTR_UNMASK |= flags;
}

/***************************************************************************/
/**
 * @brief Disables the HSPI interrupts.
 *
 * @param[in] flags Interrupt flags to be disabled.
 ******************************************************************************/
__STATIC_INLINE void hspi_secondary_disable_interrupts(uint32_t flags)
{
  SPI_SLAVE->SPI_INTR_EN &= (~flags);
  SPI_SLAVE->SPI_INTR_MASK |= flags;
  NVIC_DisableIRQ(HIF0_IRQn);
}

/***************************************************************************/
/**
 * @brief Clears the HSPI interrupt status based on the provided flags.
 *
 * @param[in] flags Interrupt flags to be cleared.
 ******************************************************************************/
__INLINE void hspi_secondary_clear_interrupts_status(uint32_t flags)
{
  SPI_SLAVE->SPI_INTR_STATUS |= flags;
}

/***************************************************************************/
/**
 * @brief Retrieves the pending HSPI interrupts.
 *
 * @return Pending interrupt status.
 ******************************************************************************/
__INLINE uint32_t hspi_secondary_get_pending_interrupts(void)
{
  return SPI_SLAVE->SPI_INTR_STATUS;
}

/***************************************************************************/
/**
 * @brief Retrieves the enabled HSPI interrupts.
 *
 * @return Number of enabled interrupts.
 ******************************************************************************/
__INLINE uint32_t hspi_secondary_get_enabled_interrupts(void)
{
  return SPI_SLAVE->SPI_INTR_UNMASK;
}

/***************************************************************************/
/**
 * @brief Retrieves the enabled and pending HSPI interrupts.
 *
 * @return Number of enabled and pending interrupts.
 ******************************************************************************/
__INLINE uint32_t hspi_secondary_get_enabled_pending_interrupts(void)
{
  return (SPI_SLAVE->SPI_INTR_STATUS & SPI_SLAVE->SPI_INTR_UNMASK);
}

/***************************************************************************/
/**
 * @brief Retrieves the length of the received data.
 *
 * @return Length of the received data.
 ******************************************************************************/
__INLINE uint32_t hspi_secondary_get_data_len(void)
{
  return SPI_SLAVE->SPI_LENGTH;
}

/**************************************************************************/

/** @} (end addtogroup HSPI Secondary) */

#ifdef __cplusplus
}
#endif

#endif //__SL_SI91X_PERIPHERAL_HSPI_SECONDARY_H_
