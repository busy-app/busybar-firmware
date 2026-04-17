 /* --------------------------------------------------------------------------
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
 * $Date:        25 Sep 2019
 * $Revision:    V1.1
 *
 * Driver:       Driver_USART
 * Configured:   via RTE_Device_917.h configuration file
 * Project:      USART Driver for RS1xxxx
 */

/* History:
 * Version 1.0
 * Initial release
 */
#include "rsi_ccp_user_config.h"

#include "clock_update.h"
#include "GSPI.h"
#include "rsi_gspi.h"
#include "rsi_power_save.h"
#include "sl_si91x_clock_manager.h"
#ifdef GSPI_CONFIG
#include "sl_si91x_gspi_common_config.h"
#endif

#if defined(A11_ROM)
#include "rsi_rom_table_si91x.h"
#endif

extern RSI_UDMA_HANDLE_T udmaHandle0;    //check
extern uint32_t dma_rom_buff0[30];     //we can keep wrapeers

#define CONTROL_STRUCT0   (UDMA_NUMBER_OF_CHANNELS * 2)
#define MAX_FRAME_LENGTH  16

/* IAR support */
#if defined(__ICCARM__)
#pragma location = UDMA0_SRAM_BASE
extern RSI_UDMA_DESC_T UDMA0_Table[CONTROL_STRUCT0];
#pragma location = UDMA1_SRAM_BASE
extern RSI_UDMA_DESC_T UDMA1_Table[CONTROL_STRUCT1];
#endif

/* DMA descriptors must be aligned to 16 bytes */
#if defined(__CC_ARM)
extern RSI_UDMA_DESC_T UDMA0_Table[CONTROL_STRUCT0] ;
#endif /* defined (__CC_ARM) */

#if defined( __GNUC__ )
extern RSI_UDMA_DESC_T __attribute__ ((section(".udma_addr0"))) UDMA0_Table[CONTROL_STRUCT0];
#endif /* defined (__GNUC__) */

extern UDMA_Channel_Info udma0_chnl_info[32] ;

/* UDMA0 Resources */
extern UDMA_RESOURCES UDMA0_Resources ;

#ifdef GSPI_CONFIG
// GSPI DMA configuration
#if (SL_GSPI_DMA_CONFIG_ENABLE == ENABLE)
#undef RTE_GSPI_MASTER_CHNL_UDMA_TX_EN
#undef RTE_GSPI_MASTER_CHNL_UDMA_RX_EN
#define RTE_GSPI_MASTER_CHNL_UDMA_TX_EN 1
#define RTE_GSPI_MASTER_CHNL_UDMA_RX_EN 1

#if defined(SL_GSPI_AFULL_THRESHOLD)
#undef RTE_FIFO_AFULL_THRLD
#define RTE_FIFO_AFULL_THRLD SL_GSPI_AFULL_THRESHOLD
#endif

#if defined(SL_GSPI_AEMPTY_THRESHOLD)
#undef RTE_FIFO_AEMPTY_THRLD
#define RTE_FIFO_AEMPTY_THRLD SL_GSPI_AEMPTY_THRESHOLD
#endif

#undef TX_DMA_ARB_SIZE
#undef RX_DMA_ARB_SIZE
#define TX_DMA_ARB_SIZE ARBSIZE_4
#define RX_DMA_ARB_SIZE ARBSIZE_4
#endif
#endif

#define SYSTEM_CLOCK_180MHZ      180000000 // System frequency value at 180 MHz
#define SYSTEM_CLOCK_90MHZ       90000000  // System frequency value at 90 MHz
#define DIVISION_FACTOR_180MHZ   4         // Division factor to get accurate ticks at 180 MHz
#define DIVISION_FACTOR_90MHZ    3         // Division factor to get accurate ticks at 90 MHz
#define DIVISION_FACTOR_32MHZ    2         // Division factor to get accurate ticks at 32 MHz
#define CONVERT_TO_MS            10000     // To convert the system frequency into milliseconds, divide it by this value

#define ARM_SPI_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(2, 0) /* driver version */
extern RSI_UDMA_HANDLE_T udmaHandle0;
/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
		ARM_SPI_API_VERSION,
		ARM_SPI_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_SPI_CAPABILITIES DriverCapabilities = {
		1, /* Simplex Mode (Master and Slave) */
		0, /* TI Synchronous Serial Interface */
		0, /* Microwire Interface */
		1  /* Signal Mode Fault event: \ref SPI_EVENT_MODE_FAULT */
};
#if (RTE_GSPI_MASTER)

static  GSPI_PIN gspi_clock    = { RTE_GSPI_MASTER_CLK_PORT ,RTE_GSPI_MASTER_CLK_PIN ,RTE_GSPI_MASTER_CLK_MUX ,RTE_GSPI_MASTER_CLK_PAD };
#ifdef SL_GSPI_MASTER_CS0__PORT
static  GSPI_PIN gspi_cs0       = { RTE_GSPI_MASTER_CS0_PORT ,RTE_GSPI_MASTER_CS0_PIN ,RTE_GSPI_MASTER_CS0_MUX ,RTE_GSPI_MASTER_CS0_PAD };
#endif
#ifdef SL_GSPI_MASTER_CS1__PORT
static  GSPI_PIN gspi_cs1       = { RTE_GSPI_MASTER_CS1_PORT ,RTE_GSPI_MASTER_CS1_PIN ,RTE_GSPI_MASTER_CS1_MUX ,RTE_GSPI_MASTER_CS1_PAD };
#endif
#ifdef SL_GSPI_MASTER_CS2__PORT
static  GSPI_PIN gspi_cs2       = { RTE_GSPI_MASTER_CS2_PORT ,RTE_GSPI_MASTER_CS2_PIN ,RTE_GSPI_MASTER_CS2_MUX ,RTE_GSPI_MASTER_CS2_PAD };
#endif
static  GSPI_PIN gspi_mosi     = { RTE_GSPI_MASTER_MOSI_PORT ,RTE_GSPI_MASTER_MOSI_PIN ,RTE_GSPI_MASTER_MOSI_MUX ,RTE_GSPI_MASTER_MOSI_PAD };
static  GSPI_PIN gspi_miso     = { RTE_GSPI_MASTER_MISO_PORT ,RTE_GSPI_MASTER_MISO_PIN ,RTE_GSPI_MASTER_MISO_MUX ,RTE_GSPI_MASTER_MISO_PAD };

static GSPI_FIFO_THRESHOLDS gspi_threshold = {TX_DMA_ARB_SIZE,RX_DMA_ARB_SIZE,RTE_FIFO_AFULL_THRLD,RTE_FIFO_AEMPTY_THRLD};

#define  GSPI_MASTER_IRQHandler     IRQ046_Handler
// GSPI_MASTER Run-Time Information
static GSPI_INFO          GSPI_MASTER_Info         = { 0U };
static GSPI_TRANSFER_INFO GSPI_MASTER_TransferInfo = { 0U };

#if (RTE_GSPI_MASTER_CHNL_UDMA_TX_EN == 1)
void GSPI_MASTER_UDMA_Tx_Event (uint32_t event ,uint32_t dmaCh);
static GSPI_DMA GSPI_MASTER_UDMA_TX_CHNL = {
		{
				0,  //channelPrioHigh
				0,  //altStruct
				0,   //burstReq
				0,   //reqMask
				1,    //periphReq
				0,//DMA PERI ACK
				RTE_GSPI_MASTER_CHNL_UDMA_TX_CH    //dmaCh
		},
		RTE_GSPI_MASTER_CHNL_UDMA_TX_CH,
		GSPI_MASTER_UDMA_Tx_Event
};
#endif
#if (RTE_GSPI_MASTER_CHNL_UDMA_RX_EN == 1)
void GSPI_MASTER_UDMA_Rx_Event (uint32_t event ,uint32_t dmaCh);
static GSPI_DMA GSPI_MASTER_UDMA_RX_CHNL = {
		{
				0,
				0,
				0,
				0,
				1,
				0,  //DMA ACK
				RTE_GSPI_MASTER_CHNL_UDMA_RX_CH
		},
		RTE_GSPI_MASTER_CHNL_UDMA_RX_CH,
		GSPI_MASTER_UDMA_Rx_Event
};
#endif

// GSPI_MASTER Resources
static const GSPI_RESOURCES GSPI_MASTER_Resources = {
		GSPI0,
		GSPI0_IRQn,

#if(RTE_GSPI_MASTER_CHNL_UDMA_RX_EN)
		&GSPI_MASTER_UDMA_RX_CHNL,
#else
		NULL,
#endif

#if (RTE_GSPI_MASTER_CHNL_UDMA_TX_EN)
		&GSPI_MASTER_UDMA_TX_CHNL,
#else
		NULL,
#endif
		&GSPI_MASTER_Info,
		&GSPI_MASTER_TransferInfo,
		//pins 
    {
		&gspi_clock,
#ifdef SL_GSPI_MASTER_CS0__PORT
		&gspi_cs0,
#else
		NULL,
#endif
#ifdef SL_GSPI_MASTER_CS1__PORT
		&gspi_cs1,
#else
		NULL,
#endif
#ifdef SL_GSPI_MASTER_CS2__PORT
		&gspi_cs2,
#else
		NULL,
#endif
		&gspi_mosi,
		&gspi_miso, //pins end 
    }, //pins end
		GSPI_INTF_PLL_CLK,//clock  
		//thresholds    
		&gspi_threshold,    
0 };
#endif /* GSPI_Master */

static uint32_t get_gspi_tick_count(uint32_t time_ms);

ARM_DRIVER_VERSION GSPI_MASTER_GetVersion(void)
{
	return DriverVersion;
}

ARM_SPI_CAPABILITIES GSPI_MASTER_GetCapabilities(void)
{
	return DriverCapabilities;
}

uint8_t gspi_slavenumber=0xA5;

void RSI_GSPI_SetSlaveSelectNumber(uint8_t slavenumber)
{
	gspi_slavenumber=slavenumber;
}
uint8_t RSI_GSPI_GetSlaveSelectNumber(void)
{
	return gspi_slavenumber;
}

#if (RTE_GSPI_MASTER_CHNL_UDMA_TX_EN == 1)
void GSPI_MASTER_UDMA_Tx_Event (uint32_t event, uint32_t dmaCh)
{
#if defined(A11_ROM) && defined(GSPI_ROMDRIVER_PRESENT)
	ROMAPI_GSPI_API->GSPI_UDMA_Tx_Event (event, (uint8_t)dmaCh, &GSPI_MASTER_Resources);	
#else
	GSPI_UDMA_Tx_Event (event,(uint8_t)dmaCh, &GSPI_MASTER_Resources);
#endif
}
#endif

#if (RTE_GSPI_MASTER_CHNL_UDMA_RX_EN == 1)
void GSPI_MASTER_UDMA_Rx_Event (uint32_t event,uint32_t dmaCh)
{
#if defined(A11_ROM) && defined(GSPI_ROMDRIVER_PRESENT)
	ROMAPI_GSPI_API->GSPI_UDMA_Rx_Event (event,(uint8_t)dmaCh, &GSPI_MASTER_Resources);
#else
	GSPI_UDMA_Rx_Event (event,(uint8_t)dmaCh, &GSPI_MASTER_Resources);
#endif
}
#endif

#if RTE_GSPI_MASTER
static int32_t GSPI_MASTER_Initialize (ARM_SPI_SignalEvent_t pSignalEvent)                
{ 
#if defined(A11_ROM) && defined(GSPI_ROMDRIVER_PRESENT)
	return ROMAPI_GSPI_API->GSPI_Initialize (pSignalEvent, &GSPI_MASTER_Resources,&UDMA0_Resources,UDMA0_Table,&udmaHandle0,dma_rom_buff0);
#else
	return GSPI_Initialize (pSignalEvent, &GSPI_MASTER_Resources,&UDMA0_Resources,UDMA0_Table,&udmaHandle0,dma_rom_buff0);
#endif	
}

static int32_t GSPI_MASTER_Uninitialize(void) 
{
#if defined(A11_ROM) && defined(GSPI_ROMDRIVER_PRESENT)
	return ROMAPI_GSPI_API->GSPI_Uninitialize (&GSPI_MASTER_Resources,&UDMA0_Resources);  		
#else
	return GSPI_Uninitialize (&GSPI_MASTER_Resources,&UDMA0_Resources);
#endif
}

static int32_t GSPI_MASTER_PowerControl(ARM_POWER_STATE state)
{
#if defined(A11_ROM) && defined(GSPI_ROMDRIVER_PRESENT)
	return ROMAPI_GSPI_API->GSPI_PowerControl (state, &GSPI_MASTER_Resources);   	
#else
	return GSPI_PowerControl (state, &GSPI_MASTER_Resources); 
#endif	
}

static int32_t GSPI_MASTER_Send(const void *data, uint32_t num)
{
#if defined(A11_ROM) && defined(GSPI_ROMDRIVER_PRESENT)
	return ROMAPI_GSPI_API->GSPI_Send (data, num, &GSPI_MASTER_Resources,&UDMA0_Resources,udma0_chnl_info,udmaHandle0); 
#else
	return GSPI_Send (data, num, &GSPI_MASTER_Resources,&UDMA0_Resources,udma0_chnl_info,udmaHandle0); 
#endif
}

int32_t GSPI_MASTER_Send_Blocking(const void *data, uint32_t num, uint32_t timeout)
{
	GSPI_RESOURCES *gspi = &GSPI_MASTER_Resources;
	uint16_t data_bits = 0;
	uint8_t data_8bit = 0;
	uint16_t data_16bit = 0;
	uint8_t data_width_in_bytes = 0;
	uint32_t gspi_tick_count = get_gspi_tick_count(timeout);

	// Validate GSPI configuration state
	if (!(gspi->info->state & SPI_CONFIGURED)) {
		return ARM_DRIVER_ERROR;
	}

	// Check if GSPI is currently busy
	if (gspi->info->status.busy) {
		return ARM_DRIVER_ERROR_BUSY;
	}

	// Initialize transfer status and buffer pointers
	gspi->info->status.data_lost  = 0;
	gspi->info->status.mode_fault = 0;
	gspi->xfer->tx_cnt            = 0;
	gspi->xfer->tx_buf            = (uint8_t *)data;

	// Get configured data frame width from register
	data_bits = gspi->reg->GSPI_WRITE_DATA2_b.GSPI_MANUAL_WRITE_DATA2;
	if (data_bits == 0) {
		data_bits = 16; // 0 indicates all 16 bits are valid
	}

	// Configure GSPI for optimized send-only operation
	gspi->reg->GSPI_CONFIG1_b.SPI_FULL_DUPLEX_EN = DISABLE;  // Disable full duplex for send-only
	gspi->reg->GSPI_WRITE_DATA2_b.USE_PREV_LENGTH = true;    // Use previous configured length
	gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_WR = ENABLE;       // Enable manual write mode

	// Calculate bytes per transfer based on frame width
	data_width_in_bytes = (data_bits <= 8) ? 1 : 2;
	gspi->xfer->num = num * data_width_in_bytes;

	// Enable interrupt mask for transfer monitoring
	gspi->reg->GSPI_INTR_MASK |= (GSPI_INTR_MASK_BIT);

	// Perform blocking data transfer with timeout
	while (gspi->xfer->tx_cnt < gspi->xfer->num) {
		// Wait until write FIFO is not full or timeout
		uint32_t local_tick = gspi_tick_count;
		while ((gspi->reg->GSPI_STATUS_b.FIFO_FULL_WFIFO_S == 1) && (local_tick-- > 0));
		if (local_tick == 0) {
			return ARM_DRIVER_ERROR_TIMEOUT;
		}

		// Transfer data based on configured width
		if (data_width_in_bytes == 1) {
			// 8-bit data transfer
			data_8bit = *(gspi->xfer->tx_buf + gspi->xfer->tx_cnt++);
			gspi->reg->GSPI_WRITE_FIFO[0] = (uint32_t)data_8bit;
		} else if (data_width_in_bytes == 2) {
			// 16-bit data transfer
			data_16bit = *(gspi->xfer->tx_buf + gspi->xfer->tx_cnt++);
			data_16bit |= (uint16_t)(*(gspi->xfer->tx_buf + gspi->xfer->tx_cnt++) << 8);
			gspi->reg->GSPI_WRITE_FIFO[0] = (uint32_t)data_16bit;
		}
	}

	// Wait for GSPI to complete all pending operations or timeout
	uint32_t busy_tick = gspi_tick_count;
	while ((gspi->reg->GSPI_STATUS_b.GSPI_BUSY != 0) && (busy_tick-- > 0));
	if (busy_tick == 0) {
		return ARM_DRIVER_ERROR_TIMEOUT;
	}

	return ARM_DRIVER_OK;
}

static int32_t GSPI_MASTER_Receive (void *data, uint32_t num)
{
#if  defined(A11_ROM) && defined(GSPI_ROMDRIVER_PRESENT)
	return ROMAPI_GSPI_API->GSPI_Receive (data, num, &GSPI_MASTER_Resources,&UDMA0_Resources,udma0_chnl_info,udmaHandle0);  	
#else
	return GSPI_Receive (data, num, &GSPI_MASTER_Resources,&UDMA0_Resources,udma0_chnl_info,udmaHandle0); 
#endif
}

int32_t GSPI_MASTER_Receive_Blocking(void *data, uint32_t num, uint32_t timeout)
{
	GSPI_RESOURCES *gspi = &GSPI_MASTER_Resources;
	uint16_t data_bits = 0;
	uint8_t data_width_in_bytes = 0;
	uint8_t data_8bit = 0;
	uint16_t data_16bit = 0;
	uint32_t gspi_tick_count = get_gspi_tick_count(timeout);

	// Validate input parameters
	if ((data == NULL) || (num == 0)) {
		return ARM_DRIVER_ERROR_PARAMETER;
	}
	// Check GSPI configuration state
	if (!(gspi->info->state & SPI_CONFIGURED)) {
		return ARM_DRIVER_ERROR;
	}
	// Check if GSPI is currently busy
	if (gspi->info->status.busy) {
		return ARM_DRIVER_ERROR_BUSY;
	}

	// Initialize transfer status and buffer pointers
	gspi->info->status.data_lost  = 0;
	gspi->info->status.mode_fault = 0;
	gspi->xfer->rx_buf            = (uint8_t *)data;
	gspi->xfer->rx_cnt            = 0;
	gspi->xfer->tx_cnt            = 0;

	// Get configured data frame width from register
	data_bits = gspi->reg->GSPI_WRITE_DATA2_b.GSPI_MANUAL_WRITE_DATA2;
	if (data_bits == 0) {
		data_bits = 16; // 0 indicates all 16 bits are valid
	}
	data_width_in_bytes = (data_bits <= 8) ? 1 : 2;

	// Configure GSPI for optimized receive-only operation
	gspi->reg->GSPI_WRITE_DATA2_b.USE_PREV_LENGTH = true;
	gspi->reg->GSPI_CONFIG1_b.SPI_FULL_DUPLEX_EN  = ENABLE;
	gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_WR      = ENABLE; // Write enable

	// Enable interrupt mask for transfer monitoring
	gspi->reg->GSPI_INTR_MASK |= (GSPI_INTR_MASK_BIT);

	// Set manual read count and enable manual read
	gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_RD_CNT  = num - 1;
	gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_RD      = ENABLE; // Read enable

	// Set number of bytes to receive
	gspi->xfer->num = num * data_width_in_bytes;

	// Perform blocking data transfer with timeout
	while (gspi->xfer->rx_cnt < gspi->xfer->num) {
		uint32_t gspi_tick = gspi_tick_count;
		gspi->reg->GSPI_WRITE_FIFO[0] = 0; // Dummy write to generate clock

		// Wait until read FIFO is not empty or timeout
		while ((gspi->reg->GSPI_STATUS_b.FIFO_EMPTY_RFIFO_S == 1) && (gspi_tick-- > 0));
		if (gspi_tick == 0) {
			return ARM_DRIVER_ERROR_TIMEOUT;
		}

		// Read the next data available in the FIFO into rx_buf
		if (1 == data_width_in_bytes) {
			// 8-bit data transfer
			data_8bit = gspi->reg->GSPI_READ_FIFO[0];
			*(gspi->xfer->rx_buf + gspi->xfer->rx_cnt++) = data_8bit;
		} else if (2 == data_width_in_bytes) {
			// 16-bit data transfer
			data_16bit = gspi->reg->GSPI_READ_FIFO[0];
			*(gspi->xfer->rx_buf + gspi->xfer->rx_cnt++) = (uint8_t)data_16bit;
			*(gspi->xfer->rx_buf + gspi->xfer->rx_cnt++) = (uint8_t)(data_16bit >> 8);
		}
	}

	return ARM_DRIVER_OK;
}

static int32_t GSPI_MASTER_Transfer (const void *data_out, void *data_in, uint32_t num) 
{
#if defined(A11_ROM) && defined(GSPI_ROMDRIVER_PRESENT)
	return ROMAPI_GSPI_API->GSPI_Transfer (data_out, data_in, num, &GSPI_MASTER_Resources,&UDMA0_Resources,udma0_chnl_info,udmaHandle0); 
#else
	return GSPI_Transfer (data_out, data_in, num, &GSPI_MASTER_Resources,&UDMA0_Resources,udma0_chnl_info,udmaHandle0); 
#endif
}

int32_t GSPI_MASTER_Transfer_Blocking(const void *data_out, void *data_in, uint32_t num, uint32_t timeout)
{
	GSPI_RESOURCES *gspi = &GSPI_MASTER_Resources;
	uint16_t data_bits = 0;
	uint8_t data_width_in_bytes = 0;
	uint8_t data_8bit;
	uint16_t data_16bit;
	uint32_t gspi_tick_count = get_gspi_tick_count(timeout);

	// Validate input parameters
	if ((data_out == NULL) || (data_in == NULL) || (num == 0U)) {
		return ARM_DRIVER_ERROR_PARAMETER;
	}
	// Check GSPI configuration state
	if (!(gspi->info->state & SPI_CONFIGURED)) {
		return ARM_DRIVER_ERROR;
	}
	// Check if GSPI is currently busy
	if (gspi->info->status.busy) {
		return ARM_DRIVER_ERROR_BUSY;
	}

	// Initialize transfer status and buffer pointers
	gspi->info->status.data_lost  = 0U;
	gspi->info->status.mode_fault = 0U;
	gspi->xfer->rx_buf            = (uint8_t *)data_in;
	gspi->xfer->tx_buf            = (uint8_t *)data_out;
	gspi->xfer->rx_cnt            = 0U;
	gspi->xfer->tx_cnt            = 0U;

	// Get configured data frame width from register
	data_bits = gspi->reg->GSPI_WRITE_DATA2_b.GSPI_MANUAL_WRITE_DATA2;
	if (data_bits == 0) {
		data_bits = 16; // 0 indicates all 16 bits are valid
	}
	data_width_in_bytes = (data_bits <= 8) ? 1 : 2;

	// Configure GSPI for full-duplex transfer
	gspi->reg->GSPI_WRITE_DATA2_b.USE_PREV_LENGTH = true;
	gspi->reg->GSPI_CONFIG1_b.SPI_FULL_DUPLEX_EN  = DISABLE;
	gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_WR      = ENABLE; // Write enable

	// Enable full duplex mode
	gspi->reg->GSPI_CONFIG1_b.SPI_FULL_DUPLEX_EN  = ENABLE;
	gspi->xfer->num                               = num * data_width_in_bytes;

	// Enable interrupt mask for transfer monitoring
	gspi->reg->GSPI_INTR_MASK |= (GSPI_INTR_MASK_BIT);

	// Set manual read count and enable manual read
	gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_RD_CNT  = num - 1;
	gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_RD      = ENABLE; // Read enable

	// Perform blocking full-duplex data transfer with timeout
	while (gspi->xfer->rx_cnt < gspi->xfer->num) {
		uint32_t local_gspi_tick = gspi_tick_count;
		// Wait until write FIFO is not full or timeout
		while ((gspi->reg->GSPI_STATUS_b.FIFO_FULL_WFIFO_S == 1) && (local_gspi_tick-- > 0));
		if (local_gspi_tick == 0) {
			// Timeout occurred while waiting to write data
			return ARM_DRIVER_ERROR_TIMEOUT;
		}

		// Write next data to FIFO
		if (data_width_in_bytes == 1) {
			// 8-bit data transfer
			data_8bit = *(gspi->xfer->tx_buf + gspi->xfer->tx_cnt++);
			gspi->reg->GSPI_WRITE_FIFO[0] = (uint32_t)data_8bit;
		} else if (data_width_in_bytes == 2) {
			// 16-bit data transfer
			data_16bit = *(gspi->xfer->tx_buf + gspi->xfer->tx_cnt++);
			data_16bit |= (uint16_t)(*(gspi->xfer->tx_buf + gspi->xfer->tx_cnt++) << 8);
			gspi->reg->GSPI_WRITE_FIFO[0] = (uint32_t)data_16bit;
		}

		local_gspi_tick = gspi_tick_count;
		// Wait until read FIFO is not empty or timeout
		while ((gspi->reg->GSPI_STATUS_b.FIFO_EMPTY_RFIFO_S == 1) && (local_gspi_tick-- > 0));
		if (local_gspi_tick == 0) {
			// Timeout occurred while waiting for data in RX FIFO
			return ARM_DRIVER_ERROR_TIMEOUT;
		}

		// Read the next data available in the FIFO into rx_buf
		if (data_width_in_bytes == 1) {
			// 8-bit data transfer
			data_8bit = gspi->reg->GSPI_READ_FIFO[0];
			*(gspi->xfer->rx_buf + gspi->xfer->rx_cnt++) = data_8bit;
		} else if (data_width_in_bytes == 2) {
			// 16-bit data transfer
			data_16bit = gspi->reg->GSPI_READ_FIFO[0];
			*(gspi->xfer->rx_buf + gspi->xfer->rx_cnt++) = (uint8_t)data_16bit;
			*(gspi->xfer->rx_buf + gspi->xfer->rx_cnt++) = (uint8_t)(data_16bit >> 8U);
		}
	}

	return ARM_DRIVER_OK;
}

int32_t GSPI_MASTER_SetSlaveSelectGPIOState(boolean_t value)
{
	GSPI_RESOURCES *gspi = &GSPI_MASTER_Resources;

	gspi_slavenumber = RSI_GSPI_GetSlaveSelectNumber();
	/* check slave number is valied or not */
	if (gspi_slavenumber == 0xA5) {
		/* Update the slave number by using RSI_GSPI_SetSlaveSelectNumber API */
		return RSI_MUTLI_SLAVE_SELECT_ERROR;
	}
	gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_CSN_SELECT = (uint32_t)(gspi_slavenumber & 0x03);

	gspi->reg->GSPI_CONFIG1_b.GSPI_MANUAL_CSN        = (value == ENABLE) ? 0 : 0x1;

	while ((gspi->reg->GSPI_STATUS & GSPI_MAN_CSN))
		;
	return ARM_DRIVER_OK;
}

static uint32_t get_gspi_tick_count(uint32_t time_ms)
{
	uint32_t tick_count      = 0;
	uint8_t division_factor  = 0;
	uint32_t clock_frequency = 0;
	sl_si91x_clock_manager_m4_get_core_clk_src_freq(&clock_frequency);
	if (clock_frequency >= SYSTEM_CLOCK_180MHZ) {
		division_factor = DIVISION_FACTOR_180MHZ;
	} else if (clock_frequency >= SYSTEM_CLOCK_90MHZ) {
		division_factor = DIVISION_FACTOR_90MHZ;
	} else {
		division_factor = DIVISION_FACTOR_32MHZ;
	}
	// gspi_tick_count value is updated with the ticks according to the input value and clock freq.
	tick_count = ((clock_frequency / CONVERT_TO_MS) * time_ms) / division_factor;
	return tick_count;
}

static uint32_t GSPI_MASTER_GetDataCount (void)                                              
{
#if defined(A11_ROM) && defined(GSPI_ROMDRIVER_PRESENT)
	return ROMAPI_GSPI_API->GSPI_GetDataCount (&GSPI_MASTER_Resources);	
#else
	return GSPI_GetDataCount (&GSPI_MASTER_Resources);
#endif	
}

static int32_t GSPI_MASTER_Control(uint32_t control, uint32_t arg)                    
{
  int32_t status          = 0;
  uint32_t gspi_get_clock = 0;
  gspi_get_clock          = RSI_CLK_GetBaseClock(M4_GSPI);
  if ((control & ARM_SPI_CONTROL_Msk) == ARM_SPI_CONTROL_SS) {
    gspi_slavenumber = RSI_GSPI_GetSlaveSelectNumber();
    /* check slave number is valied or not */
    if (gspi_slavenumber == 0xA5) {
      /* Update the slave number by using RSI_GSPI_SetSlaveSelectNumber API */
      return RSI_MUTLI_SLAVE_SELECT_ERROR;
    }
  }
#if (defined(GSPI_ROMDRIVER_PRESENT))
#if ((defined(A11_ROM)) && (defined(SLI_SI917)) && (defined(SLI_SI917B0)))
  return ROMAPI_GSPI_API->GSPI_Control(control, arg, &GSPI_MASTER_Resources, gspi_get_clock, gspi_slavenumber);
#else
  return ROMAPI_GSPI_API->GSPI_Control(control, arg, &GSPI_MASTER_Resources, gspi_get_clock);
#endif
#else
  status = GSPI_Control(control, arg, &GSPI_MASTER_Resources, gspi_get_clock, gspi_slavenumber);
#endif
  return status;
}

static ARM_SPI_STATUS GSPI_MASTER_GetStatus (void)
{
	return GSPI_GetStatus (&GSPI_MASTER_Resources); 	
}

void GSPI_MASTER_IRQHandler(void)
{
#if defined(A11_ROM) && defined(GSPI_ROMDRIVER_PRESENT)
	ROMAPI_GSPI_API->GSPI_IRQHandler (&GSPI_MASTER_Resources); 	
#else
	GSPI_IRQHandler (&GSPI_MASTER_Resources); 
#endif	
}

ARM_DRIVER_SPI Driver_GSPI_MASTER = {
		GSPI_MASTER_GetVersion,
		GSPI_MASTER_GetCapabilities,
		GSPI_MASTER_Initialize,
		GSPI_MASTER_Uninitialize,
		GSPI_MASTER_PowerControl,
		GSPI_MASTER_Send,
		GSPI_MASTER_Receive,
		GSPI_MASTER_Transfer,
		GSPI_MASTER_GetDataCount,
		GSPI_MASTER_Control,
		GSPI_MASTER_GetStatus
};
// To get the Frame length
uint32_t GSPI_GetFrameLength(void)
{
	uint32_t frame_length = GSPI_MASTER_Resources.reg->GSPI_WRITE_DATA2_b.GSPI_MANUAL_WRITE_DATA2;
	if (!frame_length) {
		frame_length = MAX_FRAME_LENGTH;
	}
  return frame_length;
}

// To enable/disable the swapping of byte for read and write operation
int32_t GSPI_SwapReadWriteByte(boolean_t read, boolean_t write)
{
  if (!(GSPI_MASTER_Resources.info->state & SPI_POWERED)) {
    return ARM_DRIVER_ERROR;
  }
  if (GSPI_MASTER_Resources.info->status.busy) {
    return ARM_DRIVER_ERROR_BUSY;
  }
  if (read) {
    GSPI_MASTER_Resources.reg->GSPI_CONFIG2_b.GSPI_RD_DATA_SWAP_MNL_CSN0 = ENABLE;
  } else {
    GSPI_MASTER_Resources.reg->GSPI_CONFIG2_b.GSPI_RD_DATA_SWAP_MNL_CSN0 = DISABLE;
  }
  if (write) {
    GSPI_MASTER_Resources.reg->GSPI_CONFIG2_b.GSPI_WR_DATA_SWAP_MNL_CSN0 = ENABLE;
  } else {
    GSPI_MASTER_Resources.reg->GSPI_CONFIG2_b.GSPI_WR_DATA_SWAP_MNL_CSN0 = DISABLE;
  }
  return ARM_DRIVER_OK;
}

//To get the transfer count
uint32_t GSPI_GetTxCount(void)
{
  if (!(GSPI_MASTER_Resources.info->state & SPI_CONFIGURED)) {
    return 0U;
  }
  return GSPI_MASTER_Resources.xfer->tx_cnt;
}

//To get the GSPI state, initialized or not
uint8_t GSPI_GetInitState(void)
{
  return (GSPI_MASTER_Resources.info->state & SPI_INITIALIZED);
}

#ifdef SL_SI91X_GSPI_DMA
/*******************************************************************************
 * Transfer complete callback function which is registered by SL_DMA driver
 * for GSPI peripheral-memory and memory-peripheral transfers. This function calls
 * GSPI Rx and Tx transfer complete event for respective channels.
 * *****************************************************************************/
void gspi_transfer_complete_callback(uint32_t channel, void *data)
{
  (void)(&data);
  if (channel == RTE_GSPI_MASTER_CHNL_UDMA_TX_CH) {
    GSPI_UDMA_Tx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel, &GSPI_MASTER_Resources);
  }
  if (channel == RTE_GSPI_MASTER_CHNL_UDMA_RX_CH) {
    GSPI_UDMA_Rx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel, &GSPI_MASTER_Resources);
  }
}

/*******************************************************************************
 * Error callback function which is registered by SL_DMA driver for GSPI
 * peripheral-memory and memory-peripheral transfers. This function calls
 * GSPI Rx and Tx error event for respective channels.
 * *****************************************************************************/
void gspi_error_callback(uint32_t channel, void *data)
{
  (void)(&data);
  if (channel == RTE_GSPI_MASTER_CHNL_UDMA_TX_CH) {
    GSPI_UDMA_Tx_Event(UDMA_EVENT_ERROR, (uint8_t)channel, &GSPI_MASTER_Resources);
  }
  if (channel == RTE_GSPI_MASTER_CHNL_UDMA_RX_CH) {
    GSPI_UDMA_Rx_Event(UDMA_EVENT_ERROR, (uint8_t)channel, &GSPI_MASTER_Resources);
  }
}

/*******************************************************************************
 * The GSPI_WriteDummyByte function is used to perform a dummy write operation on
 * the GSPI. In the first SPI transaction after out of reset, two sampling clock
 * cycles are taken to synchronize the reset.
 * This dummy write is particularly useful to ensure that the sampling
 * FIFOs are correctly synchronized after a reset.
 * *****************************************************************************/
void GSPI_WriteDummyByte(void)
{
  GSPI_Write_Dummy_Byte(&GSPI_MASTER_Resources);
}
#endif /* SL_SI91X_GSPI_DMA */
#endif

