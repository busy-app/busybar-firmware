/* -----------------------------------------------------------------------------
 * Copyright (c) 2013-2016 ARM Limited. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * $Date:        02. March 2016
 * $Revision:    V2.2
 *
 * Project:      SPI Driver Definitions for Silicon Labs MCU
 * -------------------------------------------------------------------------- */

#ifndef __SPI_H
#define __SPI_H

#include "rsi_pll.h"
#include "Driver_SPI.h"
#include "rsi_ccp_common.h"

#include "UDMA.h"     
#include "rsi_ulpss_clk.h"
#ifdef SSI_INSTANCE_CONFIG
#include "sl_ssi_common_config.h"
#elif SSI_CONFIG
#include "sl_si91x_ssi_common_config.h"
#endif

#define  SSI_DISABLE           			   				0x00 /*!< Disable the SSI Operation*/
#define  SSI_ENABLE           			   				0x01 /*!< Enable the SSI Operation*/
#define  TXEIM    			 			   							0x1
#define  TXOIM     			  			  						0x2
#define  RXUIM     			   			   						0x4
#define  RXOIM     			  			  						0x8
#define  RXFIM     			  			   						0x10
#define  TRANSMIT_AND_RECEIVE			 						0x00
#define  TRANSMIT_ONLY												0x01
#define  RECEIVE_ONLY													0x02
#define  STANDARD_SPI_FORMAT  		 						0x00
#define  DUAL_SPI_FORMAT                      0x01
#define  QUAD_SPI_FORMAT                      0x02
#define  MOTOROLA_SPI		   			   						0x00
#define  TEXAS_INSTRUMENTS_SSP 			   				0x01
#define  NATIONAL_SEMICONDUCTORS_MICROWIRE 		0x02
#define  SPI_MASTER_MODE											1U
#define  SPI_SLAVE_MODE												2U
#define  SPI_ULP_MASTER_MODE									3U
#define  SSI_INSTANCE_BIT 										30         // SSI Instance validation bits
#define  SSI_INSTANCE_MASK   									0x3FFFFFFF // Mask value for SSI instance
#define  SSI_MASTER_INSTANCE 									0          // SSI Master Instance
#define  SSI_SLAVE_INSTANCE 									1          // SSI Slave Instance
#define  SSI_ULP_MASTER_INSTANCE 							2          // SSI ULP Master Instance

#define  SPI_ISR_TX_FIFO_EMPTY								BIT(0)
#define  SPI_ISR_TX_FIFO_OVERFLOW							BIT(1)
#define  SPI_ISR_RX_FIFO_UNDERFLOW						BIT(2)
#define  SPI_ISR_RX_FIFO_OVERFLOW							BIT(3)
#define  SPI_ISR_RX_FIFO_FULL									BIT(4)

/*----- SPI Control Codes: Mode Parameters: Transfer mode ------*/
#define SPI_TRANSFER_MODE_Pos      24
#define SPI_TRANSFER_MODE_Msk      (3UL << SPI_TRANSFER_MODE_Pos)
#define SPI_TRANSFER_MODE_STANDARD (0UL << SPI_TRANSFER_MODE_Pos)   // Standard SPI Format
#define SPI_TRANSFER_MODE_DUAL     (1UL << SPI_TRANSFER_MODE_Pos)   // Dual SPI Format
#define SPI_TRANSFER_MODE_QUAD     (2UL << SPI_TRANSFER_MODE_Pos)   // Quad SPI Format
#define SPI_TRANSFER_MODE_RESERVED (3UL << SPI_TRANSFER_MODE_Pos)   // Reserved

ARM_DRIVER_VERSION SPI_GetVersion(void);
ARM_SPI_CAPABILITIES SPI_GetCapabilities(void);
void IRQ047_Handler(void) ;
void IRQ044_Handler(void);
void IRQ016_Handler(void);
void mySPI_callback(uint32_t event);

#if ((defined(RTE_Drivers_SSI_MASTER)     	  || \
		defined(RTE_Drivers_SSI_SLAVE)        		|| \
		defined(RTE_Drivers_SSI_ULP_MASTER))     		 \
		&& (RTE_SSI_MASTER == 0)                 		 \
		&& (RTE_SSI_SLAVE == 0)                  		 \
		&& (RTE_SSI_ULP_MASTER == 0))
#error "SPI not configured in RTE_Device_917.h!"
#endif


#if defined(RTE_SSI_MASTER) && (RTE_SSI_MASTER == 1)
#define  SSI_MASTER						 1U
// Configuring DMA thresholds based on SSI instances.
#ifdef SSI_INSTANCE_CONFIG
#if defined (SL_SSI_PRIMARY_DMA_CONFIG_ENABLE) && (SL_SSI_PRIMARY_DMA_CONFIG_ENABLE == ENABLE)
#define RTE_SSI_MASTER_RX_DMA 1
#define RTE_SSI_MASTER_TX_DMA 1
#endif

#if defined (SL_SSI_SECONDARY_DMA_CONFIG_ENABLE) && (SL_SSI_SECONDARY_DMA_CONFIG_ENABLE == ENABLE)
#define RTE_SSI_SLAVE_RX_DMA 1
#define RTE_SSI_SLAVE_TX_DMA 1
#endif

#if defined (SL_SSI_ULP_PRIMARY_DMA_CONFIG_ENABLE) && (SL_SSI_ULP_PRIMARY_DMA_CONFIG_ENABLE == ENABLE)
#define RTE_SSI_ULP_MASTER_RX_DMA 1
#define RTE_SSI_ULP_MASTER_TX_DMA 1
#endif
// DMA threshold configuration based on SSI unknown instance name.
#elif SSI_CONFIG
#if defined (SL_SSI_MASTER_DMA_CONFIG_ENABLE) && (SL_SSI_MASTER_DMA_CONFIG_ENABLE == ENABLE)
#define RTE_SSI_MASTER_RX_DMA 1
#define RTE_SSI_MASTER_TX_DMA 1
#endif

#if defined (SL_SSI_SLAVE_DMA_CONFIG_ENABLE) && (SL_SSI_SLAVE_DMA_CONFIG_ENABLE == ENABLE)
#define RTE_SSI_SLAVE_RX_DMA 1
#define RTE_SSI_SLAVE_TX_DMA 1
#endif

#if defined (SL_SSI_ULP_MASTER_DMA_CONFIG_ENABLE) && (SL_SSI_ULP_MASTER_DMA_CONFIG_ENABLE == ENABLE)
#define RTE_SSI_ULP_MASTER_RX_DMA 1
#define RTE_SSI_ULP_MASTER_TX_DMA 1
#endif
#endif

#if defined(RTE_SSI_MASTER_RX_DMA) && (RTE_SSI_MASTER_RX_DMA == 1)
#define SSI_MASTER_RX_DMA_Instance 1U
#define SSI_MASTER_RX_DMA_Channel  RTE_SSI_MASTER_UDMA_RX_CH
#endif

#if defined(RTE_SSI_MASTER_TX_DMA) && (RTE_SSI_MASTER_TX_DMA == 1)
#define SSI_MASTER_TX_DMA_Instance 	 1U
#define SSI_MASTER_TX_DMA_Channel  	 RTE_SSI_MASTER_UDMA_TX_CH
#endif

#if defined(RTE_SSI_MASTER_MISO) && (RTE_SSI_MASTER_MISO == 1)
#define  SSI_MASTER_MISO_SEL             1U
#define  SSI_MASTER_MISO_PORT            RTE_SSI_MASTER_MISO_PORT
#define  SSI_MASTER_MISO_PIN             RTE_SSI_MASTER_MISO_PIN
#define  SSI_MASTER_MISO_MODE            RTE_SSI_MASTER_MISO_MODE
#define  SSI_MASTER_MISO_PADSEL          RTE_SSI_MASTER_MISO_PADSEL
#endif

#if defined(RTE_SSI_MASTER_MOSI) && (RTE_SSI_MASTER_MOSI == 1)
#define  SSI_MASTER_MOSI_SEL             1U
#define  SSI_MASTER_MOSI_PORT            RTE_SSI_MASTER_MOSI_PORT
#define  SSI_MASTER_MOSI_PIN             RTE_SSI_MASTER_MOSI_PIN
#define  SSI_MASTER_MOSI_MODE            RTE_SSI_MASTER_MOSI_MODE
#define  SSI_MASTER_MOSI_PADSEL          RTE_SSI_MASTER_MOSI_PADSEL
#endif

#if (defined(SPI_QUAD_MODE) && (SPI_QUAD_MODE == 1))
#if defined(RTE_SSI_MASTER_DATA2) && (RTE_SSI_MASTER_DATA2 == 1)
#define  SSI_MASTER_DATA2_SEL             1U
#define  SSI_MASTER_DATA2__PORT            RTE_SSI_MASTER_DATA2_PORT
#define  SSI_MASTER_DATA2__PIN             RTE_SSI_MASTER_DATA2_PIN
#define  SSI_MASTER_DATA2_MODE            RTE_SSI_MASTER_DATA2_MODE
#define  SSI_MASTER_DATA2_PADSEL          RTE_SSI_MASTER_DATA2_PADSEL
#endif

#if defined(RTE_SSI_MASTER_DATA3) && (RTE_SSI_MASTER_DATA3 == 1)
#define  SSI_MASTER_DATA3_SEL             1U
#define  SSI_MASTER_DATA3__PORT            RTE_SSI_MASTER_DATA3_PORT
#define  SSI_MASTER_DATA3__PIN             RTE_SSI_MASTER_DATA3_PIN
#define  SSI_MASTER_DATA3_MODE            RTE_SSI_MASTER_DATA3_MODE
#define  SSI_MASTER_DATA3_PADSEL          RTE_SSI_MASTER_DATA3_PADSEL
#endif
#endif

#if defined(RTE_SSI_MASTER_SCK) && (RTE_SSI_MASTER_SCK == 1)
#define  SSI_MASTER_SCK_SEL              1U
#define  SSI_MASTER_SCK_PORT             RTE_SSI_MASTER_SCK_PORT
#define  SSI_MASTER_SCK_PIN              RTE_SSI_MASTER_SCK_PIN
#define  SSI_MASTER_SCK_MODE             RTE_SSI_MASTER_SCK_MODE
#define  SSI_MASTER_SCK_PADSEL           RTE_SSI_MASTER_SCK_PADSEL
#endif

#if defined(RTE_SSI_MASTER_CS0) && (RTE_SSI_MASTER_CS0 == 1)
#define  SSI_MASTER_CS0_SEL              1U
#define  SSI_MASTER_CS0_PORT             RTE_SSI_MASTER_CS0_PORT
#define  SSI_MASTER_CS0_PIN              RTE_SSI_MASTER_CS0_PIN
#define  SSI_MASTER_CS0_MODE             RTE_SSI_MASTER_CS0_MODE
#define  SSI_MASTER_CS0_PADSEL           RTE_SSI_MASTER_CS0_PADSEL
#endif

#if defined(RTE_SSI_MASTER_CS1) && (RTE_SSI_MASTER_CS1 == 1)
#define  SSI_MASTER_CS1_SEL              1U
#define  SSI_MASTER_CS1_PORT             RTE_SSI_MASTER_CS1_PORT
#define  SSI_MASTER_CS1_PIN              RTE_SSI_MASTER_CS1_PIN
#define  SSI_MASTER_CS1_MODE             RTE_SSI_MASTER_CS1_MODE
#define  SSI_MASTER_CS1_PADSEL           RTE_SSI_MASTER_CS1_PADSEL
#endif

#if defined(RTE_SSI_MASTER_CS2) && (RTE_SSI_MASTER_CS2 == 1)
#define  SSI_MASTER_CS2_SEL              1U
#define  SSI_MASTER_CS2_PORT             RTE_SSI_MASTER_CS2_PORT
#define  SSI_MASTER_CS2_PIN              RTE_SSI_MASTER_CS2_PIN
#define  SSI_MASTER_CS2_MODE             RTE_SSI_MASTER_CS2_MODE
#define  SSI_MASTER_CS2_PADSEL           RTE_SSI_MASTER_CS2_PADSEL
#endif

#if defined(RTE_SSI_MASTER_CS3) && (RTE_SSI_MASTER_CS3 == 1)
#define  SSI_MASTER_CS3_SEL              1U
#define  SSI_MASTER_CS3_PORT             RTE_SSI_MASTER_CS3_PORT
#define  SSI_MASTER_CS3_PIN              RTE_SSI_MASTER_CS3_PIN
#define  SSI_MASTER_CS3_MODE             RTE_SSI_MASTER_CS3_MODE
#define  SSI_MASTER_CS3_PADSEL           RTE_SSI_MASTER_CS3_PADSEL
#endif
#endif

#if defined(RTE_SSI_SLAVE) && (RTE_SSI_SLAVE == 1)
#define  SSI_SLAVE						 1U

#if defined(RTE_SSI_SLAVE_RX_DMA) && (RTE_SSI_SLAVE_RX_DMA == 1)
#define SSI_SLAVE_RX_DMA_Instance 	 1U
#define SSI_SLAVE_RX_DMA_Channel  	 RTE_SSI_SLAVE_UDMA_RX_CH
#endif

#if defined(RTE_SSI_SLAVE_TX_DMA) && (RTE_SSI_SLAVE_TX_DMA == 1)
#define SSI_SLAVE_TX_DMA_Instance 	 1U
#define SSI_SLAVE_TX_DMA_Channel  	 RTE_SSI_SLAVE_UDMA_TX_CH
#endif

#if defined(RTE_SSI_SLAVE_MISO) && (RTE_SSI_SLAVE_MISO == 1)
#define  SSI_SLAVE_MISO_SEL              1U
#define  SSI_SLAVE_MISO_PORT             RTE_SSI_SLAVE_MISO_PORT
#define  SSI_SLAVE_MISO_PIN              RTE_SSI_SLAVE_MISO_PIN
#define  SSI_SLAVE_MISO_MODE             RTE_SSI_SLAVE_MISO_MODE
#define  SSI_SLAVE_MISO_PADSEL           RTE_SSI_SLAVE_MISO_PADSEL
#endif

#if defined(RTE_SSI_SLAVE_MOSI) && (RTE_SSI_SLAVE_MOSI == 1)
#define  SSI_SLAVE_MOSI_SEL              1U
#define  SSI_SLAVE_MOSI_PORT             RTE_SSI_SLAVE_MOSI_PORT
#define  SSI_SLAVE_MOSI_PIN              RTE_SSI_SLAVE_MOSI_PIN
#define  SSI_SLAVE_MOSI_MODE             RTE_SSI_SLAVE_MOSI_MODE
#define  SSI_SLAVE_MOSI_PADSEL           RTE_SSI_SLAVE_MOSI_PADSEL
#endif

#if defined(RTE_SSI_SLAVE_SCK) && (RTE_SSI_SLAVE_SCK == 1)
#define  SSI_SLAVE_SCK_SEL               1U
#define  SSI_SLAVE_SCK_PORT              RTE_SSI_SLAVE_SCK_PORT
#define  SSI_SLAVE_SCK_PIN               RTE_SSI_SLAVE_SCK_PIN
#define  SSI_SLAVE_SCK_MODE              RTE_SSI_SLAVE_SCK_MODE
#define  SSI_SLAVE_SCK_PADSEL            RTE_SSI_SLAVE_SCK_PADSEL
#endif

#if defined(RTE_SSI_SLAVE_CS) && (RTE_SSI_SLAVE_CS == 1)
#define  SSI_SLAVE_CS0_SEL               1U
#define  SSI_SLAVE_CS0_PORT              RTE_SSI_SLAVE_CS_PORT
#define  SSI_SLAVE_CS0_PIN               RTE_SSI_SLAVE_CS_PIN
#define  SSI_SLAVE_CS0_MODE              RTE_SSI_SLAVE_CS_MODE
#define  SSI_SLAVE_CS0_PADSEL            RTE_SSI_SLAVE_CS_PADSEL
#endif
#endif

#if defined(RTE_SSI_ULP_MASTER) && (RTE_SSI_ULP_MASTER == 1)
#define  SSI_ULP_MASTER					 	 1U

#if defined(RTE_SSI_ULP_MASTER_RX_DMA) && (RTE_SSI_ULP_MASTER_RX_DMA == 1)
#define SSI_ULP_MASTER_RX_DMA_Instance 	 1U
#define SSI_ULP_MASTER_RX_DMA_Channel  	 RTE_SSI_ULP_MASTER_UDMA_RX_CH
#endif

#if defined(RTE_SSI_ULP_MASTER_TX_DMA) && (RTE_SSI_ULP_MASTER_TX_DMA == 1)
#define SSI_ULP_MASTER_TX_DMA_Instance 	 1U
#define SSI_ULP_MASTER_TX_DMA_Channel     RTE_SSI_ULP_MASTER_UDMA_TX_CH
#endif

#if defined(RTE_SSI_ULP_MASTER_MISO) && (RTE_SSI_ULP_MASTER_MISO == 1)
#define  SSI_ULP_MASTER_MISO_SEL         1U
#define  SSI_ULP_MASTER_MISO_PORT        RTE_SSI_ULP_MASTER_MISO_PORT
#define  SSI_ULP_MASTER_MISO_PIN         RTE_SSI_ULP_MASTER_MISO_PIN
#define  SSI_ULP_MASTER_MISO_MODE        RTE_SSI_ULP_MASTER_MISO_MODE
#define  SSI_ULP_MASTER_MISO_PADSEL      RTE_SSI_ULP_MASTER_MISO_PADSEL
#endif

#if defined(RTE_SSI_ULP_MASTER_MOSI) && (RTE_SSI_ULP_MASTER_MOSI == 1)
#define  SSI_ULP_MASTER_MOSI_SEL         1U
#define  SSI_ULP_MASTER_MOSI_PORT        RTE_SSI_ULP_MASTER_MOSI_PORT
#define  SSI_ULP_MASTER_MOSI_PIN         RTE_SSI_ULP_MASTER_MOSI_PIN
#define  SSI_ULP_MASTER_MOSI_MODE        RTE_SSI_ULP_MASTER_MOSI_MODE
#define  SSI_ULP_MASTER_MOSI_PADSEL      RTE_SSI_ULP_MASTER_MOSI_PADSEL
#endif

#if defined(RTE_SSI_ULP_MASTER_SCK) && (RTE_SSI_ULP_MASTER_SCK == 1)
#define  SSI_ULP_MASTER_SCK_SEL          1U
#define  SSI_ULP_MASTER_SCK_PORT         RTE_SSI_ULP_MASTER_SCK_PORT
#define  SSI_ULP_MASTER_SCK_PIN          RTE_SSI_ULP_MASTER_SCK_PIN
#define  SSI_ULP_MASTER_SCK_MODE         RTE_SSI_ULP_MASTER_SCK_MODE
#define  SSI_ULP_MASTER_SCK_PADSEL       RTE_SSI_ULP_MASTER_SCK_PADSEL
#endif

#if defined(RTE_SSI_ULP_MASTER_CS0) && (RTE_SSI_ULP_MASTER_CS0 == 1)
#define  SSI_ULP_MASTER_CS0_SEL          1U
#define  SSI_ULP_MASTER_CS0_PORT         RTE_SSI_ULP_MASTER_CS0_PORT
#define  SSI_ULP_MASTER_CS0_PIN          RTE_SSI_ULP_MASTER_CS0_PIN
#define  SSI_ULP_MASTER_CS0_MODE         RTE_SSI_ULP_MASTER_CS0_MODE
#define  SSI_ULP_MASTER_CS0_PADSEL       RTE_SSI_ULP_MASTER_CS0_PADSEL
#endif
#if defined(RTE_SSI_ULP_MASTER_CS1) && (RTE_SSI_ULP_MASTER_CS1 == 1)
#define  SSI_ULP_MASTER_CS1_SEL          1U
#define  SSI_ULP_MASTER_CS1_PORT         RTE_SSI_ULP_MASTER_CS1_PORT
#define  SSI_ULP_MASTER_CS1_PIN          RTE_SSI_ULP_MASTER_CS1_PIN
#define  SSI_ULP_MASTER_CS1_MODE         RTE_SSI_ULP_MASTER_CS1_MODE
#define  SSI_ULP_MASTER_CS1_PADSEL       RTE_SSI_ULP_MASTER_CS1_PADSEL
#endif
#if defined(RTE_SSI_ULP_MASTER_CS2) && (RTE_SSI_ULP_MASTER_CS2 == 1)
#define  SSI_ULP_MASTER_CS2_SEL          1U
#define  SSI_ULP_MASTER_CS2_PORT         RTE_SSI_ULP_MASTER_CS2_PORT
#define  SSI_ULP_MASTER_CS2_PIN          RTE_SSI_ULP_MASTER_CS2_PIN
#define  SSI_ULP_MASTER_CS2_MODE         RTE_SSI_ULP_MASTER_CS2_MODE
#define  SSI_ULP_MASTER_CS2_PADSEL       RTE_SSI_ULP_MASTER_CS2_PADSEL
#endif
#endif


/* SPI Register Interface Definitions */
#ifdef SSI_MASTER
#if (defined(SSI_MASTER_RX_DMA_Instance) || defined(SSI_MASTER_TX_DMA_Instance))
#ifndef SSI_MASTER_RX_DMA_Instance
#error "SSI_MASTER using DMA requires Rx and Tx DMA channel enabled in RTE_Device_917.h!"
#endif
#ifndef SSI_MASTER_TX_DMA_Instance
#error "SSI_MASTER using DMA requires Rx and Tx DMA channel enabled in RTE_Device_917.h!"
#endif
#endif
#endif

#ifdef SSI_SLAVE
#if (defined(SSI_SLAVE_RX_DMA_Instance) || defined(SSI_SLAVE_TX_DMA_Instance))
#ifndef SSI_SLAVE_RX_DMA_Instance
#error "SSI_SLAVE using DMA requires Rx and Tx DMA channel enabled in RTE_Device_917.h!"
#endif
#ifndef SSI_SLAVE_TX_DMA_Instance
#error "SSI_SLAVE using DMA requires Rx and Tx DMA channel enabled in RTE_Device_917.h!"
#endif
#endif
#endif

#ifdef SSI_ULP_MASTER
#if (defined(SSI_ULP_MASTER_RX_DMA_Instance) || defined(SSI_ULP_MASTER_TX_DMA_Instance))
#ifndef SSI_ULP_MASTER_RX_DMA_Instance
#error "SSI_ULP_MASTER using DMA requires Rx and Tx DMA channel enabled in RTE_Device_917.h!"
#endif
#ifndef SSI_ULP_MASTER_TX_DMA_Instance
#error "SSI_ULP_MASTER using DMA requires Rx and Tx DMA channel enabled in RTE_Device_917.h!"
#endif
#endif
#endif


#if ((defined(SSI_MASTER) && defined(SSI_MASTER_RX_DMA_Instance)) || \
		(defined(SSI_SLAVE) && defined(SSI_SLAVE_RX_DMA_Instance)) || \
		(defined(SSI_ULP_MASTER) && defined(SSI_ULP_MASTER_RX_DMA_Instance)))
#define __SPI_DMA_RX
#endif
#if ((defined(SSI_MASTER) && defined(SSI_MASTER_TX_DMA_Instance)) || \
		(defined(SSI_SLAVE) && defined(SSI_SLAVE_TX_DMA_Instance)) || \
		(defined(SSI_ULP_MASTER) && defined(SSI_ULP_MASTER_TX_DMA_Instance)))
#define __SPI_DMA_TX
#endif
#if (defined(__SPI_DMA_RX) && defined(__SPI_DMA_TX))
#define __SPI_DMA
#endif

/* Current driver status flag definition */
#define SPI_INITIALIZED                   (1    << 0)       // SPI initialized
#define SPI_POWERED                       (1    << 1)       // SPI powered on
#define SPI_CONFIGURED                    (1    << 2)       // SPI configured
#define SPI_DATA_LOST                     (1    << 3)       // SPI data lost occurred
#define SPI_MODE_FAULT                    (1    << 4)       // SPI mode fault occurred


#define SPI_CS0 0
#define SPI_CS1 1
#define SPI_CS2 2
#define SPI_CS3 3

/* SPI Pins Configuration */
typedef const struct _SPI_PIN {
	uint8_t port;                                        // SPI GPIO port
	uint8_t pin;                                         // SPI GPIO pin
	uint8_t mode;                                        // SPI GPIO mode
	uint8_t pad_sel;                                     // SPI GPIO pad selection
}SPI_PIN;

// SPI Input/Output Configuration
typedef const struct _SPI_IO {
	SPI_PIN              *mosi;          // Pointer to MOSI pin configuration
	SPI_PIN              *miso;          // Pointer to MISO pin configuration
#if (defined(SPI_QUAD_MODE) && (SPI_QUAD_MODE == 1))
    SPI_PIN              *data2;         // Pointer to DATA2 pin configuration
    SPI_PIN              *data3;         // Pointer to DATA3 pin configuration
#endif
	SPI_PIN              *sck;           // Pointer to SCK pin configuration
	SPI_PIN              *cs0;           // Pointer to CS(CHIP SELECT) pin configuration
#ifdef SPI_MULTI_SLAVE
    SPI_PIN              *cs1;           // Pointer to CS(CHIP SELECT) pin configuration
    SPI_PIN              *cs2;           // Pointer to CS(CHIP SELECT) pin configuration
    SPI_PIN              *cs3;           // Pointer to CS(CHIP SELECT) pin configuration
#endif
} SPI_IO;

// SPI DMA
typedef  struct SPI_DMA
{
	RSI_UDMA_CHA_CFG_T         chnl_cfg;
	uint8_t                    channel;       // DMA Channel number
	UDMA_SignalEvent_t         cb_event;      // DMA Event callback
} SPI_DMA;

/* SPI status */
typedef struct SPI_STATUS {
	uint8_t busy;                         // Transmitter/Receiver busy flag
	uint8_t data_lost;                    // Data lost: Receive overflow / Transmit underflow (cleared on start of transfer operation)
	uint8_t mode_fault;                   // Mode fault detected; optional (cleared on start of transfer operation)
} SPI_STATUS;

/* SPI Information (Run-time) */
typedef struct SPI_INFO {
	ARM_SPI_SignalEvent_t cb_event;       // Event Callback
	SPI_STATUS            status;         // Status flags
	uint8_t               state;          // Current SPI state
	uint32_t              mode;           // Current SPI mode
} SPI_INFO;

/* SPI Transfer Information (Run-Time) */
typedef struct SPI_TRANSFER_INFO {
	uint32_t              num;            // Total number of transfers
	uint8_t              *rx_buf;         // Pointer to in data buffer
	uint8_t              *tx_buf;         // Pointer to out data buffer
	uint32_t              rx_cnt;         // Number of data received
	uint32_t              tx_cnt;         // Number of data sent
	uint32_t              dump_val;       // Variable for dumping DMA data
	uint16_t              def_val;        // Default transfer value
} SPI_TRANSFER_INFO;

typedef struct _SPI_CLOCK
{
   SSI_MST_CLK_SRC_SEL_T  spi_clk_src;
	 ULP_SSI_CLK_SELECT_T   ulp_spi_clk_src;
	 uint32_t divfact; 
 }SPI_CLOCK;

/* SPI Resources */
typedef struct {
	SSI0_Type            *reg;                                   // SPI peripheral register interface
	SPI_IO               io;                                     // SPI pins configuration
	IRQn_Type            irq_num;                                // SPI IRQ number
	SPI_DMA              *rx_dma;                                // Receive stream register interface
	SPI_DMA              *tx_dma;                                // Transmit stream register interface
	SPI_INFO             *info;                                  // SPI Run-time information
	SPI_TRANSFER_INFO    *xfer;                                  // SPI transfer information
	uint8_t 		       	 instance_mode;
	SPI_CLOCK            clock;
} const SPI_RESOURCES;

void RSI_SPI_SetSlaveSelectNumber(uint8_t slavenumber);
void RSI_SPI_Slave_Disable(void);
void RSI_SPI_Slave_Set_CS_Init_State(void);
int32_t SSI_MASTER_Receive_Command_Data (void *data, uint32_t num, uint32_t instruction, uint32_t address, uint32_t wait_cycle);
int32_t SSI_MASTER_Send_Command_Data (void *data, uint32_t num, uint32_t instruction, uint32_t address);
#endif /* __SPI_H */
