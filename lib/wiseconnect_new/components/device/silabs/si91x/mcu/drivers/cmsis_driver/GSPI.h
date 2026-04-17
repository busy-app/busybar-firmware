/******************************************************************************
* @file  GSPI.h
*******************************************************************************
* # License
* <b>Copyright 2023,2024 Silicon Laboratories Inc. www.silabs.com</b>
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


#ifndef __GSPI_H__
#define __GSPI_H__

#ifdef __cplusplus
extern "C" {
#endif
  
#include "rsi_pll.h"
#include "Driver_SPI.h"
#include "rsi_ccp_common.h"

#include "UDMA.h" 



/* Current driver status flag definition */
#define SPI_INITIALIZED                   (1    << 0)       // SPI initialized
#define SPI_POWERED                       (1    << 1)       // SPI powered on
#define SPI_CONFIGURED                    (1    << 2)       // SPI configured
#define SPI_DATA_LOST                     (1    << 3)       // SPI data lost occurred
#define SPI_MODE_FAULT                    (1    << 4)       // SPI mode fault occurred
#define RSI_MUTLI_SLAVE_SELECT_ERROR -7 

/**
 * Macro defines for GSPI MANUAL STATUS Register
 */

#define GSPI_MANUAL_STATUS_BITMASK   0x07AF   /*!< GSPI Manual Status reg bitmask*/
#define GSPI_BUSY_F                  0x01     /*!< A read,write or dummy cycle operation is in process in manual mode*/
#define GSPI_FIFO_FULL_WFIFO_S       0x02     /*!< Full status indication for Wfifo in manual mode*/
#define GSPI_FIFO_AFULL_WFIFO_S      0x04     /*!< AFull status indication for Wfifo in manual mode*/
#define GSPI_FIFO_EMPTY_WFIFO_S      0x08     /*!< Empty status indication for Wfifo in manual mode*/
#define GSPI_FIFO_EMPTY_RFIFO_S      0x80     /*!< Empty status indication for Rfifo in manual mode*/
#define GSPI_FIFO_AEMPTY_WFIFO_S     0x100    /*!< AEmpty status indication for Rfifo in manual mode*/
#define GSPI_MAN_CSN                 0x400    /*!< Status of chip select signal*/

/**
 * Macro defines for GSPI INTR MASK Register
 */
#define GSPI_INTR_MASK_BITMASK         0x7F 	/*!< GSPI Interrupt mask reg bitmask*/
#define GSPI_INTR_MASK_BIT             0x01 	/*!< Mask the GSPI intr*/
#define GSPI_FIFO_AEMPTY_RFIFO_MASK    0x02 	/*!< read fifo almost empty intr mask */
#define GSPI_FIFO_AFULL_RFIFO_MASK     0x04		/*!< read fifo almost full intr mask*/
#define GSPI_FIFO_AEMPTY_WFIFO_MASK    0x08	  /*!< write fifo almost empty intr mask*/
#define GSPI_FIFO_AFULL_WFIFO_MASK     0x10 	/*!< Write Fifo almost full intr mask*/
#define GSPI_FIFO_FULL_WFIFO_MASK      0x20   /*!< write fifo is full intr mask*/
#define GSPI_FIFO_EMPTY_RFIFO_MASK     0x40   /*!< read fifo is empty intr mask*/
/**
 * Macro defines for GSPI INTR UNMASK Register
 */
#define GSPI_INTR_UNMASK_BITMASK         0x7F   /*!< GSPI Interrupt Unmask reg bitmask*/
#define GSPI_INTR_UNMASK_BIT             0x01   /*!< Unmask the GSPI intr*/
#define GSPI_FIFO_AEMPTY_RFIFO_UNMASK    0x02   /*!< read fifo almost empty intr unmask */
#define GSPI_FIFO_AFULL_RFIFO_UNMASK     0x04   /*!< read fifo almost full intr unmask*/
#define GSPI_FIFO_AEMPTY_WFIFO_UNMASK    0x08   /*!< write fifo almost empty intr unmask*/
#define GSPI_FIFO_AFULL_WFIFO_UNMASK     0x10   /*!< Write Fifo almost full intr unmask*/
#define GSPI_FIFO_FULL_WFIFO_UNMASK      0x20   /*!< write fifo is full intr unmask*/
#define GSPI_FIFO_EMPTY_RFIFO_UNMASK     0x40   /*!< read fifo is empty intr unmask*/

#define GSPI_CS0 0
#define GSPI_CS1 1
#define GSPI_CS2 2

// SPI DMA
typedef  struct _SPI_DMA
{
	RSI_UDMA_CHA_CFG_T         chnl_cfg;
	uint8_t                    channel;       // DMA Channel number
	UDMA_SignalEvent_t         cb_event;      // DMA Event callback
} GSPI_DMA;
/* SPI status */
typedef struct _SPI_STATUS {
	uint8_t busy;                         // Transmitter/Receiver busy flag
	uint8_t data_lost;                    // Data lost: Receive overflow / Transmit underflow (cleared on start of transfer operation)
	uint8_t mode_fault;                   // Mode fault detected; optional (cleared on start of transfer operation)
} GSPI_STATUS;

/* SPI Information (Run-time) */
typedef struct _SPI_INFO {
	ARM_SPI_SignalEvent_t cb_event;       // Event Callback
	GSPI_STATUS            status;        // Status flags
	uint8_t               state;          // Current SPI state
	uint32_t              mode;           // Current SPI mode
} GSPI_INFO;

/* SPI Transfer Information (Run-Time) */
typedef struct _SPI_TRANSFER_INFO {
	uint32_t              num;             // Total number of transfers
	uint8_t               *rx_buf;         // Pointer to in data buffer
	uint8_t               *tx_buf;         // Pointer to out data buffer
	uint32_t              rx_cnt;          // Number of data received
	uint32_t              tx_cnt;          // Number of data sent
	uint32_t              dump_val;        // Variable for dumping DMA data
	uint16_t              def_val;         // Default transfer value
} GSPI_TRANSFER_INFO;

/* SPI Pins Configuration */
typedef const struct _GSPI_PIN {
	uint8_t port;                                        ///< SPI GPIO port
	uint8_t pin;                                         ///< SPI GPIO pin
	uint8_t mode;                                        ///< SPI GPIO mode
	uint8_t pad_sel;                                     ///< SPI GPIO pad selection
}GSPI_PIN;

// SPI Input/Output Configuration
typedef  struct GSPI_IO_PINS {
	GSPI_PIN      *clock;            
	GSPI_PIN      *cs0;
	GSPI_PIN      *cs1;  
	GSPI_PIN      *cs2; 
	GSPI_PIN      *mosi;             
	GSPI_PIN      *miso;            
} GSPI_IO;

typedef  struct __GSPI_FIFO_THRESHOLDS {
 uint8_t txdma_arb_size;
 uint8_t rxdma_arb_size;
 uint8_t gspi_afull_threshold ;
 uint8_t gspi_aempty_threshold;
}GSPI_FIFO_THRESHOLDS;

/* SPI Resources */
typedef const struct {
	GSPI0_Type             *reg;                                   // SPI peripheral register interface
	IRQn_Type              irq_num;                                // SPI IRQ number
	GSPI_DMA               *rx_dma;                                // Receive stream register interface
	GSPI_DMA               *tx_dma;                                // Transmit stream register interface
	GSPI_INFO              *info;                                  // SPI Run-time information
	GSPI_TRANSFER_INFO     *xfer;                                  // SPI transfer information  
  GSPI_IO                 io;
  GSPI_CLK_SRC_SEL_T     clock_source;
  GSPI_FIFO_THRESHOLDS   *threshold;
	uint32_t cs_en;
} GSPI_RESOURCES;

void RSI_GSPI_SetSlaveSelectNumber(uint8_t slavenumber);
uint8_t RSI_GSPI_GetSlaveSelectNumber(void);
ARM_DRIVER_VERSION GSPI_MASTER_GetVersion(void);
uint8_t RSI_GSPI_GetSlaveSelectNumber(void);
ARM_SPI_CAPABILITIES GSPI_MASTER_GetCapabilities(void);
void GSPI_WriteDummyByte(void);
int32_t GSPI_MASTER_Send_Blocking(const void *data, uint32_t num, uint32_t timeout);
int32_t GSPI_MASTER_Receive_Blocking(void *data, uint32_t num, uint32_t timeout);
int32_t GSPI_MASTER_Transfer_Blocking(const void *data_out, void *data_in, uint32_t num, uint32_t timeout);
int32_t GSPI_MASTER_SetSlaveSelectGPIOState(boolean_t value);
#ifdef __cplusplus
}
#endif
#endif // __GSPI_H__
