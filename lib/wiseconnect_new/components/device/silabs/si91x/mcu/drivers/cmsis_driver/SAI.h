/* -----------------------------------------------------------------------------
 * SPDX-License-Identifier: Zlib
 * Copyright (c) 2013-2014 ARM Ltd.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * $Date:        9. Dec 2014
 * $Revision:    V1.00
 *
 * Project:      SAI (Serial Audio Interface) Driver definitions
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 1.00
 *    Initial release
 */

#ifndef __SAI_H__
#define __SAI_H__   

#include "rsi_pll.h"
#include "rsi_ulpss_clk.h"
#include "Driver_SAI.h"
#include "rsi_ccp_common.h"

#include "UDMA.h"


/****** I2S Events *****/
#define 	RSI_I2S_EVENT_RXDA				        0x0  				                  /*!< Receive Data available event */ 
#define 	RSI_I2S_EVENT_RXDATA_OVERRUN			0x1  				                  /*!< RX data overrun event        */ 
#define 	RSI_I2S_EVENT_TRANSMIT_EMPTY      0x2  				                  /*!< TX FIFO empty event          */ 
#define 	RSI_I2S_EVENT_TXDATA_OVERRUN     	0x3  				                  /*!< TX data overrun event        */
#define   RSI_I2S_EVENT_DMA_ENABLE          0x4  				                  /*!< DMA enable event             */
 
/****** I2S Status Flags *****/
#define 	I2S_STAT_RXDA				        			(1UL << 0)                    /*!< RXDA interrupt flag          */
#define 	I2S_STAT_RXDATA_OVERRUN						(1UL << 1)                    /*!< RXDO interrupt flag          */
#define 	I2S_STAT_TRANSMIT_EMPTY     			(1UL << 4)                    /*!< TXFE interrupt flag          */ 
#define 	I2S_STAT_TXDATA_OVERRUN     			(1UL << 5)                    /*!< TXFO interrupt flag          */ 
 
#define  	F_RXDA														0x01
#define  	F_RXFO     												0x02
#define  	F_TXFE														0x03
#define  	F_TXFO														0x04

#define   F_RXDAM	                          (1UL << 0)
#define   F_RXFOM                           (1UL << 1) 
#define   F_TXFEM	                          (1UL << 4)
#define   F_TXFOM	                          (1UL << 5) 

#define   MASTER														0x1
#define   SLAVE							  							0x0
#define   RES_12														0x1
#define   RES_16														0x2
#define   RES_20														0x3
#define   RES_24														0x4

#define   MISC_SOFT_SET_REG_2              	(*((uint32_t volatile *)(0x46008000UL + 0x34)))
#define   CHNL_0   0
#define   CHNL_1   1

/* Number of sclk cycles for which the word select
   line (ws_out) stays in the left or right sample mode */
#define    WSS_24_CYCLES			0x1
#define    WSS_16_CYCLES			0x0
#define    WSS_32_CYCLES			0x2

#define		 SCLKG_12_CYCLES		0x1
#define    SCLKG_16_CYCLES		0x2
#define    SCLKG_20_CYCLES		0x3
#define    SCLKG_24_CYCLES		0x4
#define    SCLKG_NO_GATE			0x0


#define    RX_BLOCK_EN				0x1
#define    TX_BLOCK_EN				0x1

#define    RES_12_BIT					0x1
#define    RES_16_BIT					0x2
#define    RES_20_BIT					0x3
#define    RES_24_BIT					0x4
#define    RES_32_BIT					0x5

#define    PCM_RES_8_BIT          0x0
#define    PCM_RES_12_BIT         0x1
#define    PCM_RES_16_BIT         0x2
#define    PCM_RES_24_BIT         0x3
#define    PCM_RES_32_BIT         0x4

#define    I2S_FIFO_DEPTH			8
#define    I2S_TX_TL					7
#define    I2S_RX_TL					7

#define    MASTER							0x1
#define    SLAVE							0x0

#define    PCM_EN								1
#define    PCM_DIS							0
#define    FSYNC_EN							1
#define 	 FSYNC_DIS						0

#define    ULP_I2S_CLK_SEL			8
#define    ULP_I2S_CLK_DIV_FACT	1
#define    M4_I2S_CLK_SEL			  0
#define    M4_I2S_CLK_DIV_FACT	0

/**
 \brief Macro defines for MISC_SOFT_SET3_REG  
 */ 
#define   PCM_ENA	   				    (1UL << 0)
#define   PCM_FSYNC_START_M			(1UL << 1)
#define   PCM_BIT_RES_8_SET			(0   << 2)
#define   PCM_BIT_RES_12_SET		(1   << 2)
#define   PCM_BIT_RES_16_SET		(2   << 2)
#define   PCM_BIT_RES_24_SET		(3   << 2)				

/**
 \brief Macro defines for MISC_SOFT_CLR3_REG  
 */
#define   PCMEN     				    (1UL << 0)
#define   PCM_FSYNCSTART 			  (1UL << 1)
#define   PCM_BIT_RES_8_CLR			(0   << 2)
#define   PCM_BIT_RES_12_CLR		(1   << 2)
#define   PCM_BIT_RES_16_CLR		(2   << 2)
#define   PCM_BIT_RES_24_CLR		(3   << 2)		

/**
 \brief Macro defines for MISC_CFG_MISC_CTRL1  
 */
#define   I2S_MASTER_SLAVE_MODE	(1 << 23)				/*!< Sets I2S/ PCM master mode */
#define   ARM_SOFT_RESET				(1 << 19)


/**
 \brief Macro defines for ULP_MISC_SOFT_SET_REG
 */
#define   ULP_I2S_CLK_ENABLE			    (1UL << 6)
#define   ULP_PCM_FSYNC_START					(1UL << 1)
#define   ULP_PCM_ENABLE							(1UL << 0)
#define   ULP_PCM_BIT_RES_8_SET				(0   << 2)
#define   ULP_PCM_BIT_RES_12_SET			(1   << 2)
#define   ULP_PCM_BIT_RES_16_SET			(2   << 2)
#define   ULP_PCM_BIT_RES_24_SET			(3   << 2)	    

#define   I2S_BLOCKING_XFER            1
#define   I2S_NONBLOCKING_XFER         0

// I2S flags
#define I2S_FLAG_INITIALIZED            (1U)
#define I2S_FLAG_POWERED                (1U << 1)
#define I2S_FLAG_CONFIGURED             (1U << 2)

#define INTR_MASK                      1
#define INTR_UNMASK                    0

#define I2S_PROTOCOL                   0
#define PCM_PROTOCOL                   1



// I2S Stream Information (Run-Time)
typedef struct _I2S_STREAM_INFO 
{
  uint32_t                num;           // Total number of data to be transmited/received
  uint8_t                *buf;           // Pointer to data buffer
  uint32_t                cnt;           // Number of data transmited/receive
  uint8_t                 data_bits;     // Number of data bits
  uint8_t                 master;        // Master flag
  uint8_t                 residue_num;
  uint8_t                 residue_buf[4];
  uint8_t                 residue_cnt;
} I2S_STREAM_INFO;

typedef struct _I2S_STATUS {
  uint8_t tx_busy;                       // Transmitter busy flag
  uint8_t rx_busy;                       // Receiver busy flag
  uint8_t tx_underflow;                  // Transmit data underflow detected (cleared on start of next send operation)
  uint8_t rx_overflow;                   // Receive data overflow detected (cleared on start of next receive operation)
  uint8_t frame_error;                   // Sync Frame error detected (cleared on start of next send/receive operation)
} I2S_STATUS;

// I2S Information Run Time
typedef struct _I2S_INFO 
{
  ARM_SAI_SignalEvent_t   cb_event;      // Event callback
  I2S_STATUS              status;        // Status flags
  I2S_STREAM_INFO         tx;            // Transmit information
  I2S_STREAM_INFO         rx;            // Receive information
} I2S_INFO;

// I2S DMA
typedef  struct _I2S_DMA 
{
  RSI_UDMA_CHA_CONFIG_DATA_T control;
  uint8_t                 channel;       // DMA Channel number
  UDMA_SignalEvent_t      cb_event;      // DMA Event callback
} I2S_DMA;

// I2S Clock Config
typedef  struct _I2S_CLK 
{
  uint8_t                 clk_src;       // I2S Clock Source
  uint8_t                 div_fact;      // Clock Division factor
} I2S_CLK;
 
/*  Pins Configuration */
typedef const struct _I2S_PIN {
	uint8_t port;                                        ///< SPI GPIO port
	uint8_t pin;                                         ///< SPI GPIO pin
	uint8_t mode;                                        ///< SPI GPIO mode
	uint8_t pad_sel;                                     ///< SPI GPIO pad selection
}I2S_PIN;

// Input/Output Configuration
typedef struct I2S_IO_PINS {
	I2S_PIN  *sclk;            
	I2S_PIN  *wsclk;          
	I2S_PIN  *din0;             
	I2S_PIN  *dout0;
	I2S_PIN  *din1;
	I2S_PIN  *dout1;
}I2S_IO;

// I2S Resources definitions
typedef struct 
{
  ARM_SAI_CAPABILITIES    capabilities;  // Capabilities
  I2S0_Type              *reg;           // Pointer to I2S peripheral
  IRQn_Type              irq_num;        // I2S IRQ Number
  I2S_DMA                *dma_tx;  // I2S TX DMA configuration
  I2S_DMA                *dma_rx;  // I2S RX DMA configuration
  uint8_t                tx_fifo_level;  // I2S transmit fifo level
  uint8_t                rx_fifo_level;  // I2S receive fifo level
  uint8_t                xfer_chnl;
  I2S_INFO               *info;    // Run-Time information
  uint32_t               flags;
  uint8_t                protocol; 
  I2S_CLK                *clk;
  I2S_IO                 io;    
} I2S_RESOURCES;
void IRQ064_Handler (void);
void IRQ014_Handler (void);


#endif /* __DRIVER_SAI_H */
