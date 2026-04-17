/***************************************************************************/ /**
 * @file
 * @brief SPI abstraction used by memory lcd display
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_memlcd_spi.h"
#include "SPI.h"
#include "rsi_debug.h"
#include "sl_status.h"
#include "sl_common.h"
#include "stdlib.h"

#define SPI_BIT_WIDTH 8

volatile uint8_t spi_done_ok = 0;
extern ARM_DRIVER_SPI Driver_SSI_ULP_MASTER;
int32_t status_spi = 0;

/*==============================================*/
/**
 * @fn       void mySPI_callback(uint32_t event)
 * @brief  SPI callback handler
 * @param[in]event SPI transmit and receive events
 * @return   None
 */
void mySPI_callback_spi(uint32_t event)
{
  // Clearing the instance number to evaluate the event
  event &= SSI_INSTANCE_MASK;
  switch (event) {
    case ARM_SPI_EVENT_TRANSFER_COMPLETE:
      spi_done_ok = 1;
      break;
    case ARM_SPI_EVENT_DATA_LOST:
      /*  Occurs in slave mode when data is requested/sent by master
            but send/receive/transfer operation has not been started
            and indicates that data is lost. Occurs also in master mode
            when driver cannot transfer data fast enough. */
      DEBUGOUT("\r\n ARM_SPI_EVENT_DATA_LOST \r\n");
      break;
    case ARM_SPI_EVENT_MODE_FAULT:
      /*  Occurs in master mode when Slave Select is deactivated and
            indicates Master Mode Fault. */
      DEBUGOUT("\r\n ARM_SPI_EVENT_MODE_FAULT \r\n");
      break;
  }
}

sl_status_t sli_memlcd_spi_init(uint32_t spi_baud)
{
  ARM_DRIVER_SPI *SPIdrv = &Driver_SSI_ULP_MASTER;

  // Uninitialize the SPI driver
  status_spi = SPIdrv->Uninitialize();
  if (status_spi != ARM_DRIVER_OK) {
    DEBUGOUT("\r\n SPI Uninitialization Failed, Error Code : %ld\r\n", status_spi);
    return SL_STATUS_FAIL;
  }

  // Initialize the SPI driver
  status_spi = SPIdrv->Initialize(mySPI_callback_spi);
  if (status_spi != ARM_DRIVER_OK) {
    DEBUGOUT("\r\n SPI Initialization Failed, Error Code : %ld\r\n", status_spi);
    return SL_STATUS_FAIL;
  }

  // Power up the SPI peripheral
  status_spi = SPIdrv->PowerControl(ARM_POWER_FULL);
  if (status_spi != ARM_DRIVER_OK) {
    DEBUGOUT("\r\n Failed to Set Power to SPI, Error Code : %ld\r\n", status_spi);
    return SL_STATUS_FAIL;
  }

#if (defined(SPI_MULTI_SLAVE))
  RSI_SPI_SetSlaveSelectNumber(0);
#endif
  // Configure the SPI to Master, 16-bit mode @10000 kBits/sec
  status_spi = SPIdrv->Control(
    ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA0 | ARM_SPI_SS_MASTER_HW_OUTPUT | ARM_SPI_DATA_BITS(SPI_BIT_WIDTH),
    spi_baud);
  if (status_spi != ARM_DRIVER_OK) {
    DEBUGOUT("\r\n Failed to Set Configuration Parameters to SPI, Error Code : %ld\r\n", status_spi);
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}
sl_status_t sli_memlcd_spi_shutdown()
{
  ARM_DRIVER_SPI *SPIdrv = &Driver_SSI_ULP_MASTER;
  status_spi             = SPIdrv->PowerControl(ARM_POWER_OFF);
  if (status_spi != ARM_DRIVER_OK) {
    DEBUGOUT("\r\n Failed to Set Power to SPI, Error Code : %ld\r\n", status_spi);
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t sli_memlcd_spi_tx(const void *data, unsigned len)
{
  const char *buffer = data;
  char *temp         = (char *)malloc(len);

  if (temp == NULL) {
    DEBUGOUT("\r\n Failed to allocate memory for temp buffer\r\n");
    return SL_STATUS_FAIL;
  }

  ARM_DRIVER_SPI *SPIdrv = &Driver_SSI_ULP_MASTER;

  status_spi = SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
  if (status_spi != ARM_DRIVER_OK) {
    DEBUGOUT("\r\n Failed to Active Slave Select Line for SPI Transfer, Error Code : %ld\r\n", status_spi);
    return SL_STATUS_FAIL;
  }

  for (uint32_t i = 0; i < len; i++) {
    temp[i] = SL_RBIT8(buffer[i]);
  }

  // Trigger the SPI data transfer
  status_spi = SPIdrv->Send(temp, len);

  // Free allocated memory
  free(temp);

  return SL_STATUS_OK;
}

void sli_memlcd_spi_wait()
{
  while (!spi_done_ok)
    ;

  spi_done_ok = 0;
}

void sli_memlcd_spi_rx_flush()
{
  // Do nothing, SPI Driver handles this.
}
