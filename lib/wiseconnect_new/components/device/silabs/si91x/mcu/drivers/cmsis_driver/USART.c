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

#include "UDMA.h"
#include "USART.h"
#include "clock_update.h"
#include "rsi_usart.h"
#include "rsi_ulpss_clk.h"
#if (SLI_SI91X_MCU_RS485_MODE== 1)
#include "sl_si91x_driver_gpio.h"
#ifdef UART1_RS485_MODE
#include "sl_si91x_uart1_rs485_common_config.h"
#endif
#ifdef UART0_RS485_MODE
#include "sl_si91x_uart0_rs485_common_config.h"
#endif
#endif
#ifdef USART_MODULE
#include "sl_si91x_usart_common_config.h"
#endif
#ifdef UART_MODULE
#include "sl_si91x_uart_common_config.h"
#endif
#ifdef ULP_UART_MODULE
#include "sl_si91x_ulp_uart_common_config.h"
#endif
#if defined(A11_ROM)
#include "rsi_rom_table_si91x.h"
#endif

#define ARM_USART_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(2,10) /* driver version */
extern RSI_UDMA_HANDLE_T udmaHandle0;
extern RSI_UDMA_HANDLE_T udmaHandle1;    //check
extern uint32_t dma_rom_buff0[30];
extern uint32_t dma_rom_buff1[30];     //we can keep wrapeers

#define CONTROL_STRUCT0   (UDMA_NUMBER_OF_CHANNELS * 2)
#define CONTROL_STRUCT1   (ULP_UDMA_NUMBER_OF_CHANNELS * 2)

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
extern RSI_UDMA_DESC_T UDMA1_Table[CONTROL_STRUCT1] ;
#endif /* defined (__CC_ARM) */

#if defined( __GNUC__ )
extern RSI_UDMA_DESC_T __attribute__ ((section(".udma_addr0"))) UDMA0_Table[CONTROL_STRUCT0];
extern RSI_UDMA_DESC_T __attribute__ ((section(".udma_addr1"))) UDMA1_Table[CONTROL_STRUCT1];
#endif /* defined (__GNUC__) */

extern UDMA_Channel_Info udma0_chnl_info[32] ;
extern UDMA_Channel_Info udma1_chnl_info[12] ;

#if defined(SL_USART0_DMA_CONFIG_ENABLE) && (SL_USART0_DMA_CONFIG_ENABLE == 1)
#define RTE_USART0_CHNL_UDMA_TX_EN 1
#define RTE_USART0_CHNL_UDMA_RX_EN 1
#endif
#if defined(SL_UART1_DMA_CONFIG_ENABLE) && (SL_UART1_DMA_CONFIG_ENABLE == 1)
#define RTE_UART1_CHNL_UDMA_TX_EN 1
#define RTE_UART1_CHNL_UDMA_RX_EN 1
#endif
#if defined(SL_ULPUART_DMA_CONFIG_ENABLE) && (SL_ULPUART_DMA_CONFIG_ENABLE == 1)
#define RTE_ULPUART_CHNL_UDMA_TX_EN 1
#define RTE_ULPUART_CHNL_UDMA_RX_EN 1
#endif
/* UDMA0 Resources */
extern UDMA_RESOURCES UDMA0_Resources ;

/* UDMA1 Resources */
extern UDMA_RESOURCES UDMA1_Resources;

// Driver Version
static const ARM_DRIVER_VERSION UsartDriverVersion =
{
		ARM_USART_API_VERSION,
		ARM_USART_DRV_VERSION
};
// USART AND UART1
#if (RTE_USART0)
#define  USART0_IRQ_HANDLER     IRQ038_Handler
static USART_INFO USART0_Info = {0};

/* USART PIN configuration structure*/
static  USART_PIN usart0_clock = { RTE_USART0_CLK_PORT ,RTE_USART0_CLK_PIN ,RTE_USART0_CLK_MUX ,RTE_USART0_CLK_PAD };
static  USART_PIN usart0_tx     = { RTE_USART0_TX_PORT ,RTE_USART0_TX_PIN ,RTE_USART0_TX_MUX ,RTE_USART0_TX_PAD };
static  USART_PIN usart0_rx     = { RTE_USART0_RX_PORT ,RTE_USART0_RX_PIN ,RTE_USART0_RX_MUX ,RTE_USART0_RX_PAD };
static  USART_PIN usart0_cts    = { RTE_USART0_CTS_PORT ,RTE_USART0_CTS_PIN ,RTE_USART0_CTS_MUX ,RTE_USART0_CTS_PAD };
static  USART_PIN usart0_rts    = { RTE_USART0_RTS_PORT ,RTE_USART0_RTS_PIN ,RTE_USART0_RTS_MUX ,RTE_USART0_RTS_PAD };
static  USART_PIN usart0_ir_tx  = { RTE_USART0_IR_TX_PORT ,RTE_USART0_IR_TX_PIN ,RTE_USART0_IR_TX_MUX ,RTE_USART0_IR_TX_PAD };
static  USART_PIN usart0_ir_rx  = { RTE_USART0_IR_RX_PORT ,RTE_USART0_IR_RX_PIN ,RTE_USART0_IR_RX_MUX ,RTE_USART0_IR_RX_PAD };
#if defined(RTE_USART0_CHNL_UDMA_TX_EN) && (RTE_USART0_CHNL_UDMA_TX_EN == 1)
void USART0_UDMA_Tx_Event (uint32_t event ,uint32_t dmaCh);
static USART_DMA USART0_UDMA_TX_CHNL = {
		{
				UDMA_MODE_BASIC,
				0,
				(RTE_USART0_DMA_TX_LEN_PER_DES-1),
				ARBSIZE_1,
				0x0,
				0x0,
				SRC_SIZE_8,
				SRC_INC_8,
				DST_SIZE_8,
				DST_INC_NONE
		},
		RTE_USART0_CHNL_UDMA_TX_CH,
		USART0_UDMA_Tx_Event
};
#endif
#if defined(RTE_USART0_CHNL_UDMA_RX_EN) && (RTE_USART0_CHNL_UDMA_RX_EN == 1)
void USART0_UDMA_Rx_Event (uint32_t event ,uint32_t dmaCh);
static USART_DMA USART0_UDMA_RX_CHNL = {
		{
				UDMA_MODE_BASIC,
				0,
				(RTE_USART0_DMA_RX_LEN_PER_DES-1),
				ARBSIZE_1,
				0x0,
				0x0,
				SRC_SIZE_8,
				SRC_INC_NONE,
				DST_SIZE_8,
				DST_INC_8
		},
		RTE_USART0_CHNL_UDMA_RX_CH,
		USART0_UDMA_Rx_Event
};
#endif
// Resources structure
static USART_RESOURCES USART0_Resources = {
  {             // Capabilities
    1,          // supports UART(Asynchronous) mode
    1,          // supports Synchronous Master mode
    1,          // supports Synchronous Slave mode
    1,          // supports UART Single-wire mode
    1,          // supports UART IrDA mode(SIR_MODE)
    0,          // supports UART Smart Card mode
    0,          // Smart Card Clock generator
    1,          // RTS Flow Control available
    1,          // CTS Flow Control available
    1,          // Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE
    1,          // Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
    1,          // RTS Line: 0=not available, 1=available.
    1,          // CTS Line: 0=not available, 1=available.
    1,          // DTR Line: 0=not available, 1=available.
    1,          // DSR Line: 0=not available, 1=available.
    1,          // DCD Line: 0=not available, 1=available.
    1,          // RI Line: 0=not available, 1=available.
    1,          // Signal CTS change event(optional)
    1,          // Signal DSR change event(optional)
    1,          // event_dcd(Signal DCD change event)
    1,          // Signal RI change event
  },            // Capabilities end
#if(SL_USART0_SYNC_MODE_EN)
  USART0,       // USART ADDRESS
#else
  UART0,        // UART ADDRESS
#endif
  USART0_IRQn,  // IRQn
#if defined(RTE_USART0_CHNL_UDMA_TX_EN) && (RTE_USART0_CHNL_UDMA_TX_EN == 1)
  &USART0_UDMA_TX_CHNL,
#else
  NULL,
#endif
#if defined(RTE_USART0_CHNL_UDMA_RX_EN) && (RTE_USART0_CHNL_UDMA_RX_EN == 1)
  &USART0_UDMA_RX_CHNL,
#else
  NULL,
#endif
  &USART0_Info ,
  {             // Pins
    &usart0_clock,
    &usart0_tx,
    &usart0_rx,
    &usart0_cts,
    &usart0_rts,
    &usart0_ir_tx,
    &usart0_ir_rx,
  },
  {             // Clocks
#ifdef USART_MODULE
    SL_USART0_CLOCK_SRC,
#else
    USART_ULPREFCLK,
#endif
    ULP_UART_REF_CLK,
    RTE_USART0_CLK_DIV_FACT,
    RTE_USART0_FRAC_DIV_SEL,
  },            // Clocks end
  {             // Sync mode
#ifdef USART_MODULE
  SL_USART0_SYNC_MODE_EN,
#else
  0,
#endif
  RTE_CONTINUOUS_CLOCK_MODE,
  },            // Sync mode end
};

#endif

// UART1
#if (RTE_UART1)
#define  UART1_IRQ_HANDLER     IRQ039_Handler
static USART_INFO UART1_Info = {0};

static  USART_PIN uart1_tx     = { RTE_UART1_TX_PORT ,RTE_UART1_TX_PIN ,RTE_UART1_TX_MUX ,RTE_UART1_TX_PAD };
static  USART_PIN uart1_rx     = { RTE_UART1_RX_PORT ,RTE_UART1_RX_PIN ,RTE_UART1_RX_MUX ,RTE_UART1_RX_PAD };
static  USART_PIN uart1_cts    = { RTE_UART1_CTS_PORT ,RTE_UART1_CTS_PIN ,RTE_UART1_CTS_MUX ,RTE_UART1_CTS_PAD };
static  USART_PIN uart1_rts    = { RTE_UART1_RTS_PORT ,RTE_UART1_RTS_PIN ,RTE_UART1_RTS_MUX ,RTE_UART1_RTS_PAD };
#if defined(RTE_UART1_CHNL_UDMA_TX_EN) && (RTE_UART1_CHNL_UDMA_TX_EN == 1)
void UART1_UDMA_Tx_Event (uint32_t event ,uint32_t dmaCh);

static USART_DMA UART1_UDMA_TX_CHNL = {
  {
    UDMA_MODE_BASIC,
    0,
    (RTE_UART1_DMA_TX_LEN_PER_DES-1),
    ARBSIZE_1,
    0x0,
    0x0,
    SRC_SIZE_8,
    SRC_INC_8,
    DST_SIZE_8,
    DST_INC_NONE
  },
  RTE_UART1_CHNL_UDMA_TX_CH,
  UART1_UDMA_Tx_Event
};
#endif
#if defined(RTE_UART1_CHNL_UDMA_RX_EN) && (RTE_UART1_CHNL_UDMA_RX_EN == 1)
void UART1_UDMA_Rx_Event (uint32_t event ,uint32_t dmaCh);
static USART_DMA UART1_UDMA_RX_CHNL = {
  {
    UDMA_MODE_BASIC,
    0,
    (RTE_UART1_DMA_RX_LEN_PER_DES-1),
    ARBSIZE_1,
    0x0,
    0x0,
    SRC_SIZE_8,
    SRC_INC_NONE,
    DST_SIZE_8,
    DST_INC_8
  },
  RTE_UART1_CHNL_UDMA_RX_CH,
  UART1_UDMA_Rx_Event
};
#endif

// Resources structure
static USART_RESOURCES UART1_Resources = {
  {             // Capabilities
    1,          // supports UART(Asynchronous) mode
    0,          // synchronous_master (not supported)
    0,          // synchronous_slave (not supported)
    1,          // supports UART Single-wire mode
    0,          // IRDA(SIR_MODE) mode (not supported)
    0,          // smart_card (not supported)
    0,          // smart_card_clock (not supported)
    1,          // CTS Flow Control available
    1,          // RTS Flow Control available
    1,          // Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE
    1,          // Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
    1,          // RTS Line: 0=not available, 1=available.
    1,          // CTS Line: 0=not available, 1=available.
    0,          // DTR Line: 0=not available, 1=available.
    0,          // DSR Line: 0=not available, 1=available.
    0,          // DCD Line: 0=not available, 1=available.
    0,          // RI Line: 0=not available, 1=available.
    0,          // Signal CTS change event(optional)
    0,          // Signal DSR change event(optional)
    0,          // event_dcd(Signal DCD change event)
    0,          // Signal RI change event
  },            // Capabilities end
  UART1,        // ADDRESS
  UART1_IRQn,   // IRQn
#if defined(RTE_UART1_CHNL_UDMA_TX_EN) && (RTE_UART1_CHNL_UDMA_TX_EN == 1)
  &UART1_UDMA_TX_CHNL,
#else
  NULL,
#endif
#if defined(RTE_UART1_CHNL_UDMA_RX_EN) && (RTE_UART1_CHNL_UDMA_RX_EN == 1)
  &UART1_UDMA_RX_CHNL,
#else
  NULL,
#endif
  &UART1_Info,
  {             // Pins
    NULL,
    &uart1_tx,
    &uart1_rx,
    &uart1_cts,
    &uart1_rts,
    NULL,
    NULL,
  },
  {             // Clocks
#ifdef UART_MODULE
    SL_UART1_CLOCK_SRC,
#else
    USART_ULPREFCLK,
#endif
    ULP_UART_REF_CLK,
    RTE_UART1_CLK_DIV_FACT,
    RTE_UART1_FRAC_DIV_SEL, 
  },
  {  // Sync mode
    0,
    0,
  },     
};

#endif

// ULPSS UART
#if (RTE_ULP_UART)
#define  ULP_UART_IRQ_HANDLER     IRQ012_Handler
static USART_INFO ULP_UART_Info = {0};

static  USART_PIN ulp_uart_tx     = { RTE_ULP_UART_TX_PORT ,RTE_ULP_UART_TX_PIN ,RTE_ULP_UART_TX_MUX ,RTE_ULP_UART_TX_PAD };
static  USART_PIN ulp_uart_rx     = { RTE_ULP_UART_RX_PORT ,RTE_ULP_UART_RX_PIN ,RTE_ULP_UART_RX_MUX ,RTE_ULP_UART_RX_PAD };
static  USART_PIN ulp_uart_cts    = { RTE_ULP_UART_CTS_PORT ,RTE_ULP_UART_CTS_PIN ,RTE_ULP_UART_CTS_MUX ,RTE_ULP_UART_CTS_PAD };
static  USART_PIN ulp_uart_rts    = { RTE_ULP_UART_RTS_PORT ,RTE_ULP_UART_RTS_PIN ,RTE_ULP_UART_RTS_MUX ,RTE_ULP_UART_RTS_PAD };

#if defined(RTE_ULPUART_CHNL_UDMA_TX_EN) && (RTE_ULPUART_CHNL_UDMA_TX_EN == 1)
void ULPUART_UDMA_Tx_Event (uint32_t event ,uint32_t dmaCh);
static USART_DMA ULPUART_UDMA_TX_CHNL = {
  {
    UDMA_MODE_BASIC,
    0,
    (RTE_ULP_UART_DMA_TX_LEN_PER_DES-1),
    ARBSIZE_1,
    0x0,
    0x0,
    SRC_SIZE_8,
    SRC_INC_8,
    DST_SIZE_8,
    DST_INC_NONE
  },
  RTE_ULPUART_CHNL_UDMA_TX_CH,
  ULPUART_UDMA_Tx_Event
};
#endif
#if defined(RTE_ULPUART_CHNL_UDMA_RX_EN) && (RTE_ULPUART_CHNL_UDMA_RX_EN == 1)
void ULPUART_UDMA_Rx_Event (uint32_t event ,uint32_t dmaCh);
static USART_DMA ULPUART_UDMA_RX_CHNL = {
  {
    UDMA_MODE_BASIC,
    0,
    (RTE_ULP_UART_DMA_RX_LEN_PER_DES-1),
    ARBSIZE_1,
    0x0,
    0x0,
    SRC_SIZE_8,
    SRC_INC_NONE,
    DST_SIZE_8,
    DST_INC_8
  },
  RTE_ULPUART_CHNL_UDMA_RX_CH,
  ULPUART_UDMA_Rx_Event
};
#endif

// Resources structure
static USART_RESOURCES ULP_UART_Resources = {
  {                 // Capabilities
    1,              // supports UART(Asynchronous) mode
    0,              // synchronous_master (Not supported)
    0,              // synchronous_slave  (Not supported)
    1,              // single_wire
    0,              // IRDA               (Not supported)
    0,              // smart_card         (Not supported)
    0,              // smart_card_clock   (Not supported)
#ifdef  USART_ROMDRIVER_PRESENT		
    0,              // RTS Flow Control available
    0,              // CTS Flow Control available
#else	
    1,              // RTS Flow Control available
    1,              // CTS Flow Control available
#endif			
    1,              // Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE
    1,              // Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT
#ifdef  USART_ROMDRIVER_PRESENT		
    0,              // RTS Line: 0=not available, 1=available.
    0,              // CTS Line: 0=not available, 1=available.
#else
    1,              // RTS Line: 0=not available, 1=available.
    1,              // CTS Line: 0=not available, 1=available.		 
#endif
    0,              // DTR Line: 0=not available, 1=available.
    0,              // DSR Line: 0=not available, 1=available.
    0,              // DCD Line: 0=not available, 1=available.
    0,              // RI Line: 0=not available, 1=available.
    0,              // Signal CTS change event(optional)
    0,              // Signal DSR change event(optional)
    0,              // event_dcd(Signal DCD change event)
    0,              // Signal RI change event
  },                // Capabilities end
  ULP_UART,         // ADDRESS
  ULPSS_UART_IRQn,  // IRQn
#if defined(RTE_ULPUART_CHNL_UDMA_TX_EN) && (RTE_ULPUART_CHNL_UDMA_TX_EN == 1)
  &ULPUART_UDMA_TX_CHNL,
#else
  NULL,
#endif
#if defined(RTE_ULPUART_CHNL_UDMA_RX_EN) && (RTE_ULPUART_CHNL_UDMA_RX_EN == 1)
  &ULPUART_UDMA_RX_CHNL,
#else
  NULL,
#endif
  &ULP_UART_Info,	
  {                 // Pins
    NULL,
    &ulp_uart_tx,
    &ulp_uart_rx,
    &ulp_uart_cts,
    &ulp_uart_rts,
    NULL,
    NULL,
  },
  {                 // Clocks
    USART_ULPREFCLK ,
#ifdef ULP_UART_MODULE
    SL_ULPUART_CLOCK_SRC,
#else
    ULP_UART_REF_CLK,
#endif
    RTE_ULP_UART_CLK_DIV_FACT,
    RTE_ULP_UART_FRAC_SEL , 
  },                 // Clocks end
  {                  // Sync mode
    0,
    0,
  },     
};

#endif

ARM_DRIVER_VERSION ARM_USARTx_GetVersion(void)
{
	return UsartDriverVersion;
}
/**
  @fn           ARM_USART_CAPABILITIES USART_GetCapabilities (const USART_RESOURCES *usart)
  @brief        Gets driver capabilities
  @param[in]    usart     Pointer to USART resources
  @return       \ref ARM_USART_CAPABILITIES
 */
ARM_USART_CAPABILITIES USART_GetCapabilities (const USART_RESOURCES *usart)
{
	return usart->capabilities;
}
//USART0
#if(RTE_USART0)
static ARM_USART_CAPABILITIES ARM_USART0_GetCapabilities (void)
{
	return USART_GetCapabilities (&USART0_Resources);
}
static int32_t ARM_USART0_Initialize (ARM_USART_SignalEvent_t cb_event)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_Initialize (cb_event, &USART0_Resources,&UDMA0_Resources,UDMA0_Table,&udmaHandle0,dma_rom_buff0);
#else
	return USART_Initialize (cb_event, &USART0_Resources,&UDMA0_Resources,UDMA0_Table,&udmaHandle0,dma_rom_buff0);	    
#endif
}

static int32_t ARM_USART0_Uninitialize (void)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_Uninitialize(&USART0_Resources,&UDMA0_Resources);
#else
	return USART_Uninitialize(&USART0_Resources,&UDMA0_Resources);
#endif
}

static int32_t ARM_USART0_PowerControl (ARM_POWER_STATE state)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_PowerControl (state, &USART0_Resources,&UDMA0_Resources,udmaHandle0);
#else
	return USART_PowerControl (state, &USART0_Resources,&UDMA0_Resources,udmaHandle0);
#endif
}

static int32_t ARM_USART0_Send (const void *data, uint32_t num)
{
  if(num < RTE_USART0_DMA_TX_LEN_PER_DES) {
      USART0_Resources.dma_tx->control.totalNumOfDMATrans=(unsigned int)((num-1) & 0x03FF);
  }
  else
    {
      USART0_Resources.dma_tx->control.totalNumOfDMATrans=RTE_USART0_DMA_TX_LEN_PER_DES-1;
    }
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_Send_Data (data, num, &USART0_Resources ,&UDMA0_Resources,udma0_chnl_info,udmaHandle0);
#else
	return USART_Send_Data (data, num, &USART0_Resources ,&UDMA0_Resources,udma0_chnl_info,udmaHandle0);
#endif	

}
static int32_t ARM_USART0_Receive (const void *data, uint32_t num)
{
  if(num < RTE_USART0_DMA_RX_LEN_PER_DES) {
      USART0_Resources.dma_rx->control.totalNumOfDMATrans=(unsigned int)((num-1) & 0x03FF);
  }
  else
    {
      USART0_Resources.dma_rx->control.totalNumOfDMATrans=RTE_USART0_DMA_RX_LEN_PER_DES-1;
    }
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_Receive_Data ( data, num,&USART0_Resources,&UDMA0_Resources,udma0_chnl_info,udmaHandle0);
#else
	return USART_Receive_Data ( data, num,&USART0_Resources,&UDMA0_Resources,udma0_chnl_info,udmaHandle0);
#endif	
}

static int32_t ARM_USART0_Transfer (const void *data_out, const void *data_in, uint32_t num)
{
  if((num < RTE_USART0_DMA_TX_LEN_PER_DES) && (num < RTE_USART0_DMA_RX_LEN_PER_DES)) {

      USART0_Resources.dma_tx->control.totalNumOfDMATrans=(unsigned int)((num-1) & 0x03FF);
      USART0_Resources.dma_rx->control.totalNumOfDMATrans=(unsigned int)((num-1) & 0x03FF);
  }
  else
    {
      USART0_Resources.dma_tx->control.totalNumOfDMATrans=RTE_USART0_DMA_TX_LEN_PER_DES-1;
      USART0_Resources.dma_rx->control.totalNumOfDMATrans=RTE_USART0_DMA_RX_LEN_PER_DES-1;
    }
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_Transfer (data_out, data_in, num, &USART0_Resources,&UDMA0_Resources,udma0_chnl_info,udmaHandle0);
#else
	return USART_Transfer (data_out, data_in, num, &USART0_Resources,&UDMA0_Resources,udma0_chnl_info,udmaHandle0);
#endif	
}

static uint32_t ARM_USART0_GetTxCount (void)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_GetTxCount (&USART0_Resources);
#else
	return USART_GetTxCount(&USART0_Resources);
#endif	
}

static uint32_t ARM_USART0_GetRxCount (void)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_GetRxCount(&USART0_Resources);
#else
	return USART_GetRxCount(&USART0_Resources);
#endif	
}

static int32_t ARM_USART0_Control (uint32_t control, uint32_t arg)
{
#if SL_USART0_SYNC_MODE_EN
   if(arg!=1)
    arg= (arg >> 3);
#endif
	uint32_t usart0_get_clock=0;
	usart0_get_clock = RSI_CLK_GetBaseClock(M4_USART0);
#if  defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_Control (control, arg, usart0_get_clock, &USART0_Resources,&UDMA0_Resources,udmaHandle0);	
#else
	return USART_Control (control, arg, usart0_get_clock, &USART0_Resources,&UDMA0_Resources,udmaHandle0);
#endif
}

static ARM_USART_STATUS ARM_USART0_GetStatus (void)
{
#if  defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_GetStatus(&USART0_Resources);
#else
	return USART_GetStatus(&USART0_Resources);
#endif	
}

static int32_t ARM_USART0_SetModemControl (ARM_USART_MODEM_CONTROL control)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_SetModemControl (control, &USART0_Resources);
#else
	return USART_SetModemControl(control, &USART0_Resources);
#endif
}

static ARM_USART_MODEM_STATUS ARM_USART0_GetModemStatus (void)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_GetModemStatus (&USART0_Resources);
#else
	return USART_GetModemStatus(&USART0_Resources);
#endif
}

void RSI_M4SSUsart0Handler(void)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	ROMAPI_USART_API->UartIrqHandler(&USART0_Resources);
#else
	UartIrqHandler(&USART0_Resources);
#endif
}
#if defined(RTE_USART0_CHNL_UDMA_TX_EN) && (RTE_USART0_CHNL_UDMA_TX_EN == 1)
void USART0_UDMA_Tx_Event (uint32_t event, uint32_t dmaCh)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	ROMAPI_USART_API->USART_UDMA_Tx_Event (event,dmaCh, &USART0_Resources);
#else
	USART_UDMA_Tx_Event (event,(uint8_t)dmaCh, &USART0_Resources);
#endif
}
#endif
#if defined(RTE_USART0_CHNL_UDMA_RX_EN) && (RTE_USART0_CHNL_UDMA_RX_EN == 1)
void USART0_UDMA_Rx_Event (uint32_t event,uint32_t dmaCh)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	ROMAPI_USART_API->USART_UDMA_Rx_Event (event,dmaCh, &USART0_Resources);	
#else
	USART_UDMA_Rx_Event (event,(uint8_t)dmaCh, &USART0_Resources);
#endif
}
#endif

// USART driver functions structure
ARM_DRIVER_USART Driver_USART0 =
{
		ARM_USARTx_GetVersion,
		ARM_USART0_GetCapabilities,
		ARM_USART0_Initialize,
		ARM_USART0_Uninitialize,
		ARM_USART0_PowerControl,
		ARM_USART0_Send,
		ARM_USART0_Receive,
		ARM_USART0_Transfer,
		ARM_USART0_GetTxCount,
		ARM_USART0_GetRxCount,
		ARM_USART0_Control,
		ARM_USART0_GetStatus,
		ARM_USART0_SetModemControl,
		ARM_USART0_GetModemStatus
};
// USART_IRQ_HANDLER
void USART0_IRQ_HANDLER (void)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	ROMAPI_USART_API->UartIrqHandler(&USART0_Resources);
#else
	UartIrqHandler(&USART0_Resources);
#endif	
}

#endif

//UART1
#if(RTE_UART1)

static ARM_USART_CAPABILITIES ARM_UART1_GetCapabilities (void)
{
	return USART_GetCapabilities (&UART1_Resources);
}

static int32_t ARM_UART1_Initialize (ARM_USART_SignalEvent_t cb_event)
{
#if  defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return  ROMAPI_USART_API->USART_Initialize (cb_event, &UART1_Resources,&UDMA0_Resources,UDMA0_Table,&udmaHandle0,dma_rom_buff0);
#else
	return  USART_Initialize (cb_event, &UART1_Resources,&UDMA0_Resources,UDMA0_Table,&udmaHandle0,dma_rom_buff0);
#endif
}

static int32_t ARM_UART1_Uninitialize (void)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_Uninitialize(&UART1_Resources,&UDMA0_Resources);
#else
	return USART_Uninitialize(&UART1_Resources,&UDMA0_Resources);
#endif	
}

static int32_t ARM_UART1_PowerControl (ARM_POWER_STATE state)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_PowerControl (state, &UART1_Resources,&UDMA0_Resources,udmaHandle0);	
#else
	return USART_PowerControl (state, &UART1_Resources,&UDMA0_Resources,udmaHandle0);
#endif
}

static int32_t ARM_UART1_Send (const void *data, uint32_t num)
{
  if(num < RTE_UART1_DMA_TX_LEN_PER_DES) {
      UART1_Resources.dma_tx->control.totalNumOfDMATrans = (unsigned int)((num-1) & 0x03FF);
  }
  else
    {
      UART1_Resources.dma_tx->control.totalNumOfDMATrans = RTE_UART1_DMA_TX_LEN_PER_DES-1;
    }
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_Send_Data (data, num, &UART1_Resources ,&UDMA0_Resources,udma0_chnl_info,udmaHandle0);	
#else
	return USART_Send_Data (data, num, &UART1_Resources ,&UDMA0_Resources,udma0_chnl_info,udmaHandle0);
#endif
}

static int32_t ARM_UART1_Receive (const void *data, uint32_t num)
{
  if(num < RTE_UART1_DMA_RX_LEN_PER_DES) {
      UART1_Resources.dma_rx->control.totalNumOfDMATrans = (unsigned int)((num-1) & 0x03FF);
  }
  else
    {
      UART1_Resources.dma_rx->control.totalNumOfDMATrans = RTE_UART1_DMA_RX_LEN_PER_DES-1;
    }
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_Receive_Data ( data, num,&UART1_Resources,&UDMA0_Resources,udma0_chnl_info,udmaHandle0);
#else
	return USART_Receive_Data ( data, num,&UART1_Resources,&UDMA0_Resources,udma0_chnl_info,udmaHandle0);
#endif	
}

static int32_t ARM_UART1_Transfer (const void *data_out, const void *data_in, uint32_t num)
{
  if((num < RTE_UART1_DMA_TX_LEN_PER_DES) && (num < RTE_UART1_DMA_RX_LEN_PER_DES)) {
      UART1_Resources.dma_tx->control.totalNumOfDMATrans = (unsigned int)((num-1) & 0x03FF);
      UART1_Resources.dma_rx->control.totalNumOfDMATrans = (unsigned int)((num-1) & 0x03FF);
  }
  else
    {
      UART1_Resources.dma_tx->control.totalNumOfDMATrans = RTE_UART1_DMA_TX_LEN_PER_DES-1;
      UART1_Resources.dma_rx->control.totalNumOfDMATrans = RTE_UART1_DMA_RX_LEN_PER_DES-1;
    }
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_Transfer (data_out, data_in, num, &UART1_Resources,&UDMA0_Resources,udma0_chnl_info,udmaHandle0);	
#else
	return USART_Transfer (data_out, data_in, num, &UART1_Resources,&UDMA0_Resources,udma0_chnl_info,udmaHandle0);
#endif
}

static uint32_t ARM_UART1_GetTxCount (void)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_GetTxCount (&UART1_Resources);	
#else
	return USART_GetTxCount(&UART1_Resources);
#endif
}

static uint32_t ARM_UART1_GetRxCount (void)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_GetRxCount(&UART1_Resources);	
#else
	return USART_GetRxCount(&UART1_Resources);
#endif
}

static int32_t ARM_UART1_Control (uint32_t control, uint32_t arg)
{
	uint32_t uart1_get_clock;
	uart1_get_clock = RSI_CLK_GetBaseClock(M4_UART1);
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_Control (control, arg, uart1_get_clock, &UART1_Resources,&UDMA0_Resources,udmaHandle0);
#else
	return USART_Control (control, arg, uart1_get_clock, &UART1_Resources,&UDMA0_Resources,udmaHandle0);
#endif
}

static ARM_USART_STATUS ARM_UART1_GetStatus (void)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_GetStatus(&UART1_Resources);
#else
	return USART_GetStatus(&UART1_Resources);
#endif
}

static int32_t ARM_UART1_SetModemControl (ARM_USART_MODEM_CONTROL control)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_SetModemControl (control, &UART1_Resources);
#else
	return USART_SetModemControl(control, &UART1_Resources);
#endif
}

static ARM_USART_MODEM_STATUS ARM_UART1_GetModemStatus (void)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_GetModemStatus (&UART1_Resources);
#else
	return USART_GetModemStatus(&UART1_Resources);
#endif
}

void RSI_M4SSUart1Handler(void)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	ROMAPI_USART_API->UartIrqHandler(&UART1_Resources);	
#else
	UartIrqHandler(&UART1_Resources);
#endif
}

#if defined(RTE_UART1_CHNL_UDMA_TX_EN) && (RTE_UART1_CHNL_UDMA_TX_EN == 1)
void UART1_UDMA_Tx_Event (uint32_t event,uint32_t dmaCh)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	ROMAPI_USART_API->USART_UDMA_Tx_Event (event,dmaCh, &UART1_Resources);	
#else
	USART_UDMA_Tx_Event (event,(uint8_t)dmaCh, &UART1_Resources);
#endif
}
#endif

#if defined(RTE_UART1_CHNL_UDMA_RX_EN) && (RTE_UART1_CHNL_UDMA_RX_EN == 1)
void UART1_UDMA_Rx_Event (uint32_t event,uint32_t dmaCh)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	ROMAPI_USART_API->USART_UDMA_Rx_Event (event,dmaCh, &UART1_Resources);
#else
	USART_UDMA_Rx_Event (event,(uint8_t)dmaCh, &UART1_Resources);
#endif
}
#endif
// USART driver functions structure
ARM_DRIVER_USART Driver_UART1 = {
		ARM_USARTx_GetVersion,
		ARM_UART1_GetCapabilities,
		ARM_UART1_Initialize,
		ARM_UART1_Uninitialize,
		ARM_UART1_PowerControl,
		ARM_UART1_Send,
		ARM_UART1_Receive,
		ARM_UART1_Transfer,
		ARM_UART1_GetTxCount,
		ARM_UART1_GetRxCount,
		ARM_UART1_Control,
		ARM_UART1_GetStatus,
		ARM_UART1_SetModemControl,
		ARM_UART1_GetModemStatus
};

// USART_IRQ_HANDLER
void UART1_IRQ_HANDLER (void)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	ROMAPI_USART_API->UartIrqHandler(&UART1_Resources);
#else
	UartIrqHandler(&UART1_Resources);
#endif	
}
#endif

//ULP_UART
#if(RTE_ULP_UART)
static ARM_USART_CAPABILITIES ARM_ULP_UART_GetCapabilities (void)
{
	return USART_GetCapabilities (&ULP_UART_Resources);
}
static int32_t ARM_ULP_UART_Initialize (ARM_USART_SignalEvent_t cb_event)
{
#if  defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return  ROMAPI_USART_API->USART_Initialize (cb_event, &ULP_UART_Resources,&UDMA1_Resources,UDMA1_Table,&udmaHandle1,dma_rom_buff1);	
#else
	return  USART_Initialize (cb_event, &ULP_UART_Resources,&UDMA1_Resources,UDMA1_Table,&udmaHandle1,dma_rom_buff1);
#endif	
}

static int32_t ARM_ULP_UART_Uninitialize (void)
{
	RSI_PS_UlpssPeriPowerDown(ULPSS_PWRGATE_ULP_UART);
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_Uninitialize(&ULP_UART_Resources ,&UDMA1_Resources);
#else
	return USART_Uninitialize(&ULP_UART_Resources ,&UDMA1_Resources);
#endif
}

static int32_t ARM_ULP_UART_PowerControl (ARM_POWER_STATE state)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_PowerControl (state, &ULP_UART_Resources ,&UDMA1_Resources,udmaHandle1);
#else
	return USART_PowerControl (state, &ULP_UART_Resources ,&UDMA1_Resources,udmaHandle1);
#endif
}

static int32_t ARM_ULP_UART_Send (const void *data, uint32_t num)
{
  if(num < RTE_ULP_UART_DMA_TX_LEN_PER_DES) {
      ULP_UART_Resources.dma_tx->control.totalNumOfDMATrans = (unsigned int)((num-1) & 0x03FF);
  }
  else
    {
      ULP_UART_Resources.dma_tx->control.totalNumOfDMATrans = RTE_ULP_UART_DMA_TX_LEN_PER_DES-1;
    }
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_Send_Data (data, num, &ULP_UART_Resources ,&UDMA1_Resources,udma1_chnl_info,udmaHandle1);	
#else
	return USART_Send_Data (data, num, &ULP_UART_Resources ,&UDMA1_Resources,udma1_chnl_info,udmaHandle1);
#endif
}

static int32_t ARM_ULP_UART_Receive (const void *data, uint32_t num)
{
  if(num < RTE_ULP_UART_DMA_RX_LEN_PER_DES) {
      ULP_UART_Resources.dma_rx->control.totalNumOfDMATrans = (unsigned int)((num-1) & 0x03FF);
  }
  else
    {
      ULP_UART_Resources.dma_rx->control.totalNumOfDMATrans = RTE_ULP_UART_DMA_RX_LEN_PER_DES-1;
    }
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_Receive_Data ( data, num,&ULP_UART_Resources,&UDMA1_Resources,udma1_chnl_info,udmaHandle1);
#else
	return USART_Receive_Data ( data, num,&ULP_UART_Resources,&UDMA1_Resources,udma1_chnl_info,udmaHandle1);
#endif	
}

static int32_t ARM_ULP_UART_Transfer (const void *data_out, const void *data_in, uint32_t num)
{
  if((num < RTE_ULP_UART_DMA_TX_LEN_PER_DES) && (num < RTE_ULP_UART_DMA_RX_LEN_PER_DES)) {
      ULP_UART_Resources.dma_tx->control.totalNumOfDMATrans = (unsigned int)((num-1) & 0x03FF);
      ULP_UART_Resources.dma_rx->control.totalNumOfDMATrans = (unsigned int)((num-1) & 0x03FF);
  }
  else
    {
      ULP_UART_Resources.dma_tx->control.totalNumOfDMATrans = RTE_ULP_UART_DMA_TX_LEN_PER_DES-1;
      ULP_UART_Resources.dma_rx->control.totalNumOfDMATrans = RTE_ULP_UART_DMA_RX_LEN_PER_DES-1;
    }
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_Transfer (data_out, data_in, num, &ULP_UART_Resources ,&UDMA1_Resources,udma1_chnl_info,udmaHandle1);	
#else
	return USART_Transfer (data_out, data_in, num, &ULP_UART_Resources ,&UDMA1_Resources,udma1_chnl_info,udmaHandle1);
#endif	
}

static uint32_t ARM_ULP_UART_GetTxCount (void)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_GetTxCount (&ULP_UART_Resources);
#else
	return USART_GetTxCount(&ULP_UART_Resources);
#endif
}

static uint32_t ARM_ULP_UART_GetRxCount (void)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_GetRxCount(&ULP_UART_Resources);	
#else
	return USART_GetRxCount(&ULP_UART_Resources);
#endif
}

static int32_t ARM_ULP_UART_Control (uint32_t control, uint32_t arg)
{
	uint32_t ulp_uart_get_clock;
	ulp_uart_get_clock = RSI_CLK_GetBaseClock(ULPSS_UART);
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_Control (control, arg, ulp_uart_get_clock, &ULP_UART_Resources ,&UDMA1_Resources,udmaHandle1);
#else
	return USART_Control (control, arg, ulp_uart_get_clock, &ULP_UART_Resources ,&UDMA1_Resources,udmaHandle1);
#endif
}

static ARM_USART_STATUS ARM_ULP_UART_GetStatus (void)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_GetStatus(&ULP_UART_Resources);
#else
	return USART_GetStatus(&ULP_UART_Resources);
#endif	
}

static int32_t ARM_ULP_UART_SetModemControl (ARM_USART_MODEM_CONTROL control)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_SetModemControl (control, &ULP_UART_Resources);	
#else
	return USART_SetModemControl(control, &ULP_UART_Resources);
#endif	
}

static ARM_USART_MODEM_STATUS ARM_ULP_UART_GetModemStatus (void)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	return ROMAPI_USART_API->USART_GetModemStatus (&ULP_UART_Resources);
#else
	return USART_GetModemStatus(&ULP_UART_Resources);
#endif	
}

void RSI_ULPUartHandler(void)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	ROMAPI_USART_API->UartIrqHandler(&ULP_UART_Resources);
#else
	UartIrqHandler(&ULP_UART_Resources);
#endif	
}

#if defined(RTE_ULPUART_CHNL_UDMA_TX_EN) && (RTE_ULPUART_CHNL_UDMA_TX_EN == 1)
void ULPUART_UDMA_Tx_Event (uint32_t event,uint32_t dmaCh)
{
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	ROMAPI_USART_API->USART_UDMA_Tx_Event (event,dmaCh, &ULP_UART_Resources);
#else
	USART_UDMA_Tx_Event (event,(uint8_t)dmaCh, &ULP_UART_Resources);
#endif
}
#endif

#if defined(RTE_ULPUART_CHNL_UDMA_RX_EN) && (RTE_ULPUART_CHNL_UDMA_RX_EN == 1)
void ULPUART_UDMA_Rx_Event (uint32_t event,uint32_t dmaCh)
{	
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	ROMAPI_USART_API->USART_UDMA_Rx_Event (event,dmaCh, &ULP_UART_Resources);
#else
	USART_UDMA_Rx_Event (event,(uint8_t)dmaCh, &ULP_UART_Resources);
#endif
}
#endif
// USART driver functions structure
ARM_DRIVER_USART Driver_ULP_UART = {
		ARM_USARTx_GetVersion,
		ARM_ULP_UART_GetCapabilities,
		ARM_ULP_UART_Initialize,
		ARM_ULP_UART_Uninitialize,
		ARM_ULP_UART_PowerControl,
		ARM_ULP_UART_Send,
		ARM_ULP_UART_Receive,
		ARM_ULP_UART_Transfer,
		ARM_ULP_UART_GetTxCount,
		ARM_ULP_UART_GetRxCount,
		ARM_ULP_UART_Control,
		ARM_ULP_UART_GetStatus,
		ARM_ULP_UART_SetModemControl,
		ARM_ULP_UART_GetModemStatus
};

// USART_IRQ_HANDLER
void ULP_UART_IRQ_HANDLER (void)
{ 
#if defined(A11_ROM) && defined(USART_ROMDRIVER_PRESENT)
	ROMAPI_USART_API->UartIrqHandler(&ULP_UART_Resources);
#else
	UartIrqHandler(&ULP_UART_Resources);
#endif
}
uint32_t USART_GetParity_StopBit(uint8_t usart_peripheral)
{
  uint32_t reg_value = false;

	switch (usart_peripheral) {
		case USART_0:
			reg_value = USART0_Resources.pREGS->LCR;
			break;

		case UART_1:
			reg_value = UART1_Resources.pREGS->LCR;
			break;

		case ULPUART:
			reg_value = ULP_UART_Resources.pREGS->LCR;
			break;

		default:
			break;
	}
  return reg_value;
}
uint32_t USART_GetBaudrate(uint8_t usart_peripheral)
{
  uint32_t baud_rate = 0;

	switch (usart_peripheral) {
		case USART_0:
			baud_rate = USART0_Resources.info->baudrate;
			break;

		case UART_1:
			baud_rate = UART1_Resources.info->baudrate;
			break;

		case ULPUART:
			baud_rate = ULP_UART_Resources.info->baudrate;
			break;

		default:
			break;
	}
  return baud_rate;
}
uint8_t USART_GetInitState(uint8_t usart_peripheral)
{
  uint8_t init_state = false;

	switch (usart_peripheral) {
		case USART_0:
			init_state = (USART0_Resources.info->flags & USART_FLAG_INITIALIZED);
			break;

		case UART_1:
			init_state = (UART1_Resources.info->flags & USART_FLAG_INITIALIZED);
			break;

		case ULPUART:
			init_state = (ULP_UART_Resources.info->flags & USART_FLAG_INITIALIZED);
			break;

		default:
			break;
	}
	
  return init_state;
}

uint8_t USART_GetMode(uint8_t usart_peripheral)
{
  uint8_t mode = 0;

  switch (usart_peripheral) {
    case USART_0:
      mode = USART0_Resources.info->mode;
      break;

    case UART_1:
      mode = UART1_Resources.info->mode;
      break;

    case ULPUART:
      mode = ULP_UART_Resources.info->mode;
      break;

    default:
      break;
  }
  return mode;
}

//Below set of functions are only used by SL_DMA added as part of Dx improvements
#ifdef SL_SI91X_USART_DMA
/*******************************************************************************
 * Transfer complete callback function which is registered by SL_DMA driver
 * for USART peripheral-memory and memory-peripheral transfers. This function calls
 * USART Rx and Tx transfer complete event for respective channels.
 * *****************************************************************************/
void usart_transfer_complete_callback(uint32_t channel, void *data) {
  (void)(&data);
  (void)channel;
#if (SL_USART0_DMA_CONFIG_ENABLE == 1)
  if(channel == RTE_USART0_CHNL_UDMA_TX_CH) {
    USART_UDMA_Tx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel, &USART0_Resources);
  }
  if(channel == RTE_USART0_CHNL_UDMA_RX_CH) {
    USART_UDMA_Rx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel, &USART0_Resources);
  }
#endif
#if (SL_UART1_DMA_CONFIG_ENABLE == 1)
  if(channel == RTE_UART1_CHNL_UDMA_TX_CH) {
    USART_UDMA_Tx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel, &UART1_Resources);
  }
  if(channel == RTE_UART1_CHNL_UDMA_RX_CH) {
    USART_UDMA_Rx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel, &UART1_Resources);
  }
#endif
#if (SL_ULPUART_DMA_CONFIG_ENABLE == 1)
  if(channel == RTE_ULPUART_CHNL_UDMA_TX_CH) {
    USART_UDMA_Tx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel, &ULP_UART_Resources);
  }
  if(channel == RTE_ULPUART_CHNL_UDMA_RX_CH) {
    USART_UDMA_Rx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel, &ULP_UART_Resources);
  }
#endif
}

/*******************************************************************************
 * Error callback function which is registered by SL_DMA driver for USART
 * peripheral-memory and memory-peripheral transfers. This function calls
 * USART Rx and Tx error event for respective channels.
 * *****************************************************************************/
void usart_error_callback(uint32_t channel, void *data) {
  (void)(&data);
  (void)channel;
#if (SL_USART0_DMA_CONFIG_ENABLE == 1)
  if(channel == RTE_USART0_CHNL_UDMA_TX_CH) {
    USART_UDMA_Tx_Event(UDMA_EVENT_ERROR, (uint8_t)channel, &USART0_Resources);
  }
  if(channel == RTE_USART0_CHNL_UDMA_RX_CH) {
    USART_UDMA_Rx_Event(UDMA_EVENT_ERROR, (uint8_t)channel, &USART0_Resources);
  }
#endif
#if (SL_UART1_DMA_CONFIG_ENABLE == 1)
  if(channel == RTE_UART1_CHNL_UDMA_TX_CH) {
    USART_UDMA_Tx_Event(UDMA_EVENT_ERROR, (uint8_t)channel, &UART1_Resources);
  }
  if(channel == RTE_UART1_CHNL_UDMA_RX_CH) {
    USART_UDMA_Rx_Event(UDMA_EVENT_ERROR, (uint8_t)channel, &UART1_Resources);
  }
#endif
#if (SL_ULPUART_DMA_CONFIG_ENABLE == 1)
  if(channel == RTE_ULPUART_CHNL_UDMA_TX_CH) {
    USART_UDMA_Tx_Event(UDMA_EVENT_ERROR, (uint8_t)channel, &ULP_UART_Resources);
  }
  if(channel == RTE_ULPUART_CHNL_UDMA_RX_CH) {
    USART_UDMA_Rx_Event(UDMA_EVENT_ERROR, (uint8_t)channel, &ULP_UART_Resources);
  }
#endif
}
#endif
#endif
