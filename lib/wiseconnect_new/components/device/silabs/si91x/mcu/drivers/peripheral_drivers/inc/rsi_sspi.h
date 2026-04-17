/******************************************************************************
* @file  rsi_sspi.h
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

// Include Files

#include "rsi_ccp_common.h"
#include "base_types.h"
#include "rsi_error.h"

#ifndef RSI_SSPI_H
#define RSI_SSPI_H

#ifdef __cplusplus
extern "C" {
#endif

// SPI FIFO REGISTERS map

// Read FIFO data register
#define SPI_SLAVE_RFIFO_DATA *(volatile uint32_t *)(SPI_SLAVE_BASE + 0x3C0)
// Write FIFO data register
#define SPI_SLAVE_WFIFO_DATA *(volatile uint32_t *)(SPI_SLAVE_BASE + 0x380)
// SPI Slave wakeup register
#define SPI_SLAVE_WAKEUP_REG *(volatile uint16_t *)(SPI_SLAVE_BASE + 0x7E)

#define SPI_SLAVE_RFIFO_REG (SPI_SLAVE_BASE + 0x3C0)
// Write FIFO data register
#define SPI_SLAVE_WFIFO_REG (SPI_SLAVE_BASE + 0x380)

#define MAX_SSPI_CALL_BACKS 1

#define SSPI_HIGH_SPEED_MODE_EN 1

#define SSPI_FIFO_STAT_EMPTY (1 << 2)

// SPI slave Command Register
#define SSPI_CMD_WR               (1 << 5)
#define SSPI_CMD_AHB_BUS_ACCESS   (1 << 4)
#define SSPI_CMD_AHB_SLAVE_ACCESS (1 << 3)
#define SSPI_CMD_16BIT_LEN        (1 << 2)

// SPI slave Interrupt bits
#define SSPI_INT_WR          (1 << 0)
#define SSPI_INT_RD          (1 << 1)
#define SSPI_INT_CS_DEASSERT (1 << 2)

#define SSPI_EVENT_WRITE       0x0001
#define SSPI_EVENT_READ        0x0002
#define SSPI_EVENT_CS_DEASSERT 0x0003
#define SSPI_EVENT_TX_START    0x0004
#define SSPI_EVENT_RX_START    0x0005

// MISC host registers
#define MCSS_MISC_CFG_HOST_CTRL (*(volatile uint32_t *)(0x46008000 + 0x0C))

#define READY_FROM_RCORE          BIT(0)
#define LOAD_HOST_MODE            BIT(10)
#define HOST_SEL_AS_SPI           BIT(9)
#define SDIO_SPI_PROG_SEL         (BIT(15))
#define BOOT_MODE_EN              BIT(0)
#define MISC_CFG_RST_LATCH_STATUS (*(volatile uint32_t *)(0x41050012))
#define SDIO_HOST_CONTROL_TO_MCU  (BIT(5))

typedef void *RSI_SSPI_HANDLE_T;

typedef SPI_SLAVE_Type RSI_SSPI_REGS_T;

// SPI slave Initialisation structure
typedef struct {
  void *buf;

  uint32_t length;

  uint32_t count;

  uint8_t dwidth;

} RSI_SSPI_DATA_T;

typedef struct {

  RSI_SSPI_REGS_T *pREGS;

  RSI_SSPI_DATA_T tx;

  RSI_SSPI_DATA_T rx;

  void (*cbTable[MAX_SSPI_CALL_BACKS])(RSI_SSPI_HANDLE_T, uint32_t, void *);

} RSI_SSPI_DRIVER_T;

RSI_SSPI_HANDLE_T RSI_SSPI_Initialize(void *drv_mem, uint32_t base_address, uint8_t sspi_mode);

void RSI_SSPI_IntEnable(RSI_SSPI_HANDLE_T pSPIHandle, uint16_t intmask);

void RSI_SSPI_IntDisable(RSI_SSPI_HANDLE_T pSPIHandle, uint32_t intmask);

void RSI_SSPI_InterruptHandler(RSI_SSPI_HANDLE_T pSPIHandle);

uint32_t RSI_SSPI_Send(RSI_SSPI_HANDLE_T pSPIHandle, const void *data, uint32_t num);

uint32_t RSI_SSPI_SendBlocking(RSI_SSPI_HANDLE_T pSPIHandle, const void *data, uint32_t num);

uint32_t RSI_SSPI_Receive(RSI_SSPI_HANDLE_T pSPIHandle, void *data, uint32_t num);

uint32_t RSI_SSPI_ReceiveBlocking(RSI_SSPI_HANDLE_T pSPIHandle, void *data, uint32_t num);

int32_t RSI_SSPI_Transfer(RSI_SSPI_HANDLE_T *pSPIHandle, const void *data_out, void *data_in, uint32_t num);

rsi_error_t RSI_SSPI_RegisterCallBacks(RSI_SSPI_HANDLE_T pSPIHandle,
                                       void (*psspi_cb)(RSI_SSPI_HANDLE_T, uint32_t, void *));

void RSI_SSPI_IntClear(RSI_SSPI_HANDLE_T pSPIHandle, uint32_t intmask);

void RSI_Enable_Hostspi_Intf_to_M4(void);

#ifdef __cplusplus
}
#endif

#endif // RSI_SSPI_H
