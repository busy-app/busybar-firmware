/******************************************************************************
* @file sl_si91x_peripheral_hspi_secondary.c
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
#include "sl_si91x_peripheral_hspi_secondary.h"
#include "rsi_power_save.h"
/*******************************************************************************
 * Initializes the HSPI secondary peripheral.
 * This function grants pin access to M4, selects the HSPI mode, and masks the HSPI primary interrupts.
 ******************************************************************************/
void hspi_secondary_peripheral_init(void)
{
  // enables power to the SDIO-SPI Slave
  RSI_PS_M4ssPeriPowerUp(M4SS_PWRGATE_ULP_SDIO_SPI);
  //  Enable SPI access to MCSS
  (*(volatile uint32_t *)(SL_HSPI_NWPAON_MEM_HOST_ACCESS_CTRL_CLEAR)) = SL_HSPI_HOST_CONTROL_TO_MCU;
  // Indication to the host that bootloading is done
  SL_HSPI_MCSS_MISC_CFG_HOST_CTRL |= SL_HSPI_READY_FROM_RCORE;
  // Overrides the Hardware detected Host with  S/W based Host on 1st interface
  SL_HSPI_MCSS_MISC_CFG_HOST_CTRL |= SL_HSPI_LOAD_HOST_MODE;
  // Selects the Host(SPI) as Interface
  SL_HSPI_MCSS_MISC_CFG_HOST_CTRL |= SL_HSPI_HOST_SEL_AS_SPI;
  // Host SPI Bus error output enable
  SL_HSPI_MCSS_MISC_CFG_HOST_CTRL |= SL_HSPI_HOST_SPI_BUS_ERR_OEN;
  // SDIO/SPI registers share the same based address. Based on this bit MCU can access either SDIO register or SPI registers
  SL_HSPI_MCSS_MISC_CFG_HOST_CTRL &= ~SL_HSPI_SDIO_SPI_PROG_SEL;
  // This bit is used to indicate if boot mode is enabled
  SL_HSPI_MISC_CFG_RST_LATCH_STATUS |= SL_HSPI_BOOT_MODE_EN;

  while ((SL_HSPI_MISC_CFG_HOST_CTRL & SL_HSPI_MODE_SELECT) != SL_HSPI_MODE_SELECT)
    ;
  // Mask the HSPI primary interrupts
  SL_HSPI_M4_HOST_INTR_MASK_REG = SL_HSPI_MASK_HOST_INTERRUPT;
}

/*******************************************************************************
 * De-Initializes the HSPI secondary peripheral.
 ******************************************************************************/
void hspi_secondary_peripheral_deinit(void)
{
  // Unmask the HSPI primary interrupts
  SL_HSPI_M4_HOST_INTR_MASK_REG &= ~SL_HSPI_MASK_HOST_INTERRUPT;

  // Clear boot mode
  SL_HSPI_MISC_CFG_RST_LATCH_STATUS &= ~SL_HSPI_BOOT_MODE_EN;

  // Reset SDIO/SPI register selection
  SL_HSPI_MCSS_MISC_CFG_HOST_CTRL |= SL_HSPI_SDIO_SPI_PROG_SEL;

  // Clear SPI bus error output
  SL_HSPI_MCSS_MISC_CFG_HOST_CTRL &= ~SL_HSPI_HOST_SPI_BUS_ERR_OEN;

  // Clear Host selection as SPI
  SL_HSPI_MCSS_MISC_CFG_HOST_CTRL &= ~SL_HSPI_HOST_SEL_AS_SPI;

  // Clear Host mode override
  SL_HSPI_MCSS_MISC_CFG_HOST_CTRL &= ~SL_HSPI_LOAD_HOST_MODE;

  // Clear ready from rcore indication
  SL_HSPI_MCSS_MISC_CFG_HOST_CTRL &= ~SL_HSPI_READY_FROM_RCORE;

  // Disable SPI access to MCSS
  (*(volatile uint32_t *)(SL_HSPI_NWPAON_MEM_HOST_ACCESS_CTRL_SET)) = SL_HSPI_HOST_CONTROL_TO_MCU;

  // Power down the SDIO-SPI Slave
  RSI_PS_M4ssPeriPowerDown(M4SS_PWRGATE_ULP_SDIO_SPI);
}

/*******************************************************************************
 * Writes a word to the SPI FIFO register.
 *
 * @param[in] byte Data word to be written to the SPI FIFO register.
 ******************************************************************************/
__STATIC_INLINE void hspi_secondary_write_word(int32_t byte)
{

  while (SPI_SLAVE->SPI_STATUS_b.SPI_RFIFO_FULL)
    ;

  // Write word to SPI Write FIFO register
  SPI_SLAVE->SPI_WFIFO_DATA = byte;
}

/*******************************************************************************
 * Reads a word from the SPI FIFO register.
 *
 * @return The word read from the SPI FIFO register.
 ******************************************************************************/

__STATIC_INLINE int32_t hspi_secondary_read_word(void)
{
  uint32_t byte;

  // Read word from SPI Read FIFO register
  byte = SPI_SLAVE->SPI_RFIFO_DATA;

  return byte;
}

/*******************************************************************************
 * Writes data to the HSPI FIFO.
 *
 * @param[in] data Pointer to the buffer containing the data to be written.
 * @param[in] num  Number of bytes to write.
 * @return Number of bytes written to the HSPI FIFO.
 ******************************************************************************/
uint32_t hspi_secondary_write_fifo(const void *data, uint32_t num)
{
  uint32_t sent_bytes = 0;

  int32_t *data32 = (int32_t *)data;

  // Send until the transmit FIFO is not full
  while (sent_bytes < num) {
    while (SPI_SLAVE->SPI_STATUS_b.SPI_RFIFO_FULL) {
    }
    // Write the word to HSPI FIFO register
    hspi_secondary_write_word(*data32);
    data32++;
    sent_bytes += SL_HSPI_WORD_SIZE;
  }

  // Return number of bytes sent
  return sent_bytes;
}

/*******************************************************************************
 * Reads data from the HSPI FIFO.
 *
 * @param[in] data Pointer to the buffer where the read data will be stored.
 * @param[in] num  Number of bytes to read.
 * @return Number of bytes read from the HSPI FIFO.
 ******************************************************************************/
uint32_t hspi_secondary_read_fifo(void *data, uint32_t num)
{
  uint32_t readBytes = 0;

  int32_t *data_32 = (int32_t *)data;

  // Check if at least one byte has arrived in the RBR register
  while ((readBytes < num) && (!SPI_SLAVE->SPI_STATUS_b.SPI_WFIFO_EMPTY)) {
    // Read the word
    *data_32 = hspi_secondary_read_word();

    data_32++;
    readBytes += SL_HSPI_WORD_SIZE;
  }

  return readBytes;
}
