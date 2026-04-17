/*
 * Copyright (c) 2013-2016 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "rsi_ccp_user_config.h"


#include "SAI.h"
#include "clock_update.h"
#include "rsi_i2s.h"
#include "sl_si91x_i2s_config.h"

#if defined(A11_ROM)
#include "rsi_rom_table_si91x.h"
#endif
extern RSI_UDMA_HANDLE_T udmaHandle0,udmaHandle1;    //check
extern uint32_t dma_rom_buff0[30], dma_rom_buff1[30];     //we can keep wrapeers

#define CONTROL_STRUCT0   (UDMA_NUMBER_OF_CHANNELS * 2)
#define CONTROL_STRUCT1   (ULP_UDMA_NUMBER_OF_CHANNELS * 2)
#define RESOLUTION_16_BIT 16
#define I2S0_CHANNEL1_CHANNEL_OFFSET 2

#define I2S0_CLK_SRC      ULP_I2S_PLL_CLK
#define I2S0_CLK_DIV_FACT 0
#define I2S1_CLK_SRC      ULP_I2S_REF_CLK
#define I2S1_CLK_DIV_FACT 0

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

/* UDMA0 Resources */
extern UDMA_RESOURCES UDMA0_Resources ;

/* UDMA1 Resources */
extern UDMA_RESOURCES UDMA1_Resources;


#if ((!defined(RTE_I2S0)) || (!defined(RTE_I2S1)))
#error "I2S missing in RTE_Device_917.h. Please update RTE_Device_917.h!"
#endif

#if ((defined(RTE_Drivers_SAI0) && !RTE_I2S0) && (defined(RTE_Drivers_SAI1) && !RTE_I2S1))
#error "I2S0/1 not configured in RTE_Device_917.h!"
#endif

// Definitions

// Frequency tolerance in percentage
#ifndef I2S_FREQ_TOLERANCE
#define I2S_FREQ_TOLERANCE   (1.)
#endif

#define D2F_DOMAIN           (255UL)

#if (RTE_I2S0)

#if ((I2S0_TX_FIFO_LEVEL < 0U) || (I2S0_TX_FIFO_LEVEL > 7U))
#error "Invalid FIFO Level value. FIFO Level can be 0 to 7"
#endif
#if ((I2S0_RX_FIFO_LEVEL < 0U) || (I2S0_RX_FIFO_LEVEL > 7U))
#error "Invalid FIFO Level value. FIFO Level can be 0 to 7"
#endif
#endif

#if (RTE_I2S1)

#if ((I2S1_TX_FIFO_LEVEL < 0U) || (I2S1_TX_FIFO_LEVEL > 7U))
#error "Invalid FIFO Level value. FIFO Level can be 0 to 7"
#endif
#if ((I2S1_RX_FIFO_LEVEL < 0U) || (I2S1_RX_FIFO_LEVEL > 7U))
#error "Invalid FIFO Level value. FIFO Level can be 0 to 7"
#endif
#endif


#define ARM_SAI_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,5)   // driver version
// Driver Version
static const ARM_DRIVER_VERSION DriverVersion = {
		ARM_SAI_API_VERSION,
		ARM_SAI_DRV_VERSION
};

// I2S0
#if (RTE_I2S0)

static I2S_INFO I2S0_Info = {0,{0},{0},{0}};
static I2S_CLK  I2S0_Clk = {I2S0_CLK_SRC, I2S0_CLK_DIV_FACT};

static I2S_PIN  i2s0_sclk = {RTE_I2S0_SCLK_PORT,RTE_I2S0_SCLK_PIN,RTE_I2S0_SCLK_MUX,RTE_I2S0_SCLK_PAD};
static I2S_PIN  i2s0_wsclk = {RTE_I2S0_WSCLK_PORT,RTE_I2S0_WSCLK_PIN,RTE_I2S0_WSCLK_MUX,RTE_I2S0_WSCLK_PAD};
static I2S_PIN  i2s0_din0 = {RTE_I2S0_DIN0_PORT,RTE_I2S0_DIN0_PIN,RTE_I2S0_DIN0_MUX,RTE_I2S0_DIN0_PAD};
static I2S_PIN  i2s0_dout0 = {RTE_I2S0_DOUT0_PORT,RTE_I2S0_DOUT0_PIN,RTE_I2S0_DOUT0_MUX,RTE_I2S0_DOUT0_PAD};
static I2S_PIN  i2s0_din1 = {RTE_I2S0_DIN1_PORT,RTE_I2S0_DIN1_PIN,RTE_I2S0_DIN1_MUX,RTE_I2S0_DIN1_PAD};
static I2S_PIN  i2s0_dout1 = {RTE_I2S0_DOUT1_PORT,RTE_I2S0_DOUT1_PIN,RTE_I2S0_DOUT1_MUX,RTE_I2S0_DOUT1_PAD};


#if (RTE_I2S0_CHNL_UDMA_TX_EN == 1U)

void I2S0_UDMA_Tx_Event (uint32_t event, uint32_t dmaCh);

static I2S_DMA I2S0_UDMA_TX_CHNL = {
		{
				UDMA_MODE_BASIC,
				0,
				(RTE_I2S0_DMA_TX_LEN_PER_DES -1),
				ARBSIZE_4,
				0x0,
				0x0,
#if (SL_I2S0_RESOLUTION <= RESOLUTION_16_BIT)
				SRC_SIZE_16,
				SRC_INC_16,
				DST_SIZE_16,
				DST_INC_NONE
#else
				SRC_SIZE_32,
				SRC_INC_32,
				DST_SIZE_32,
				DST_INC_NONE
#endif
		},  
#if (SL_I2S0_CHANNEL == 0)
		RTE_I2S0_CHNL_UDMA_TX_CH,
#else
		(RTE_I2S0_CHNL_UDMA_TX_CH + I2S0_CHANNEL1_CHANNEL_OFFSET),
#endif
		I2S0_UDMA_Tx_Event
};
#endif

#if (RTE_I2S0_CHNL_UDMA_RX_EN == 1U)

void I2S0_UDMA_Rx_Event (uint32_t event, uint32_t dmaCh);
static I2S_DMA I2S0_UDMA_RX_CHNL = {
		{  
				UDMA_MODE_BASIC,
				0,
				(RTE_I2S0_DMA_RX_LEN_PER_DES -1),
				ARBSIZE_4,
				0x0,
				0x0,
#if (SL_I2S0_RESOLUTION <= RESOLUTION_16_BIT)
				SRC_SIZE_16,
				SRC_INC_NONE,
				DST_SIZE_16,
				DST_INC_16
#else
				SRC_SIZE_32,
				SRC_INC_NONE,
				DST_SIZE_32,
				DST_INC_32
#endif
		},  
#if (SL_I2S0_CHANNEL == 0)
		RTE_I2S0_CHNL_UDMA_RX_CH,
#else
		(RTE_I2S0_CHNL_UDMA_RX_CH + I2S0_CHANNEL1_CHANNEL_OFFSET),
#endif
		I2S0_UDMA_Rx_Event
};
#endif


static I2S_RESOURCES I2S0_Resources = {
		{  // Capabilities
				1,   ///< supports asynchronous Transmit/Receive
				1,   ///< supports synchronous Transmit/Receive
				0,   ///< supports user defined Protocol
				1,   ///< supports I2S Protocol
				0,   ///< supports MSB/LSB justified Protocol
				1,   ///< supports PCM short/long frame Protocol
				0,   ///< supports AC'97 Protocol
				0,   ///< supports Mono mode
				0,   ///< supports Companding
				0,   ///< supports MCLK (Master Clock) pin
				0,   ///< supports Frame error event: \ref ARM_SAI_EVENT_FRAME_ERROR
		},
		I2S0,
		I2S0_IRQn,
#if (RTE_I2S0_CHNL_UDMA_TX_EN == 1U)
		&I2S0_UDMA_TX_CHNL,
#else
		NULL,
#endif
#if (RTE_I2S0_CHNL_UDMA_RX_EN == 1U)
		&I2S0_UDMA_RX_CHNL,
#else
		NULL,
#endif
		I2S0_TX_FIFO_LEVEL,
		I2S0_RX_FIFO_LEVEL,
#ifdef SL_I2S0_CHANNEL
		SL_I2S0_CHANNEL,
#else
		0,
#endif
		&I2S0_Info,
		0,
		I2S_PROTOCOL,  
		&I2S0_Clk,
		{
		//pins
		&i2s0_sclk,
		&i2s0_wsclk,
		&i2s0_din0,
		&i2s0_dout0,
    &i2s0_din1,
    &i2s0_dout1,
		},

};
#endif


// I2S1
#if (RTE_I2S1)
static I2S_INFO I2S1_Info = {0,{0},{0},{0}};
static I2S_CLK  I2S1_Clk = {I2S1_CLK_SRC, I2S1_CLK_DIV_FACT};

static I2S_PIN  i2s1_sclk = {RTE_I2S1_SCLK_PORT,RTE_I2S1_SCLK_PIN,RTE_I2S1_SCLK_MUX,RTE_I2S1_SCLK_PAD};
static I2S_PIN  i2s1_wsclk = {RTE_I2S1_WSCLK_PORT,RTE_I2S1_WSCLK_PIN,RTE_I2S1_WSCLK_MUX,RTE_I2S1_WSCLK_PAD};
static I2S_PIN  i2s1_din0 = {RTE_I2S1_DIN0_PORT,RTE_I2S1_DIN0_PIN,RTE_I2S1_DIN0_MUX,RTE_I2S1_DIN0_PAD};
static I2S_PIN  i2s1_dout0 = {RTE_I2S1_DOUT0_PORT,RTE_I2S1_DOUT0_PIN,RTE_I2S1_DOUT0_MUX,RTE_I2S1_DOUT0_PAD};


#if (RTE_I2S1_CHNL_UDMA_TX_EN == 1U)
void I2S1_UDMA_Tx_Event (uint32_t event, uint32_t dmaCh);
static I2S_DMA I2S1_UDMA_TX_CHNL = {
		{
				UDMA_MODE_BASIC,
				0,
				RTE_I2S1_DMA_TX_LEN_PER_DES-1,
				ARBSIZE_4,
				0x0,
				0x0,
#if (SL_ULP_I2S_RESOLUTION <= RESOLUTION_16_BIT)
				SRC_SIZE_16,
				SRC_INC_16,
				DST_SIZE_16,
				DST_INC_NONE
#else
				SRC_SIZE_32,
				SRC_INC_32,
				DST_SIZE_32,
				DST_INC_NONE
#endif
		},  
		RTE_I2S1_CHNL_UDMA_TX_CH,
		I2S1_UDMA_Tx_Event
};
#endif

#if (RTE_I2S1_CHNL_UDMA_RX_EN == 1U)
void I2S1_UDMA_Rx_Event (uint32_t event, uint32_t dmaCh);
static I2S_DMA I2S1_UDMA_RX_CHNL = {
		{  
				UDMA_MODE_BASIC,
				0,
				RTE_I2S1_DMA_RX_LEN_PER_DES-1,
				ARBSIZE_4,
				0x0,
				0x0,
#if (SL_ULP_I2S_RESOLUTION <= RESOLUTION_16_BIT)
				SRC_SIZE_16,
				SRC_INC_NONE,
				DST_SIZE_16,
				DST_INC_16
#else
				SRC_SIZE_32,
				SRC_INC_NONE,
				DST_SIZE_32,
				DST_INC_32
#endif
		},  
		RTE_I2S1_CHNL_UDMA_RX_CH,
		I2S1_UDMA_Rx_Event
};
#endif

static I2S_RESOURCES I2S1_Resources = {
		{  // Capabilities
				1,   ///< supports asynchronous Transmit/Receive
				1,   ///< supports synchronous Transmit/Receive
				0,   ///< supports user defined Protocol
				1,   ///< supports I2S Protocol
				0,   ///< supports MSB/LSB justified Protocol
				0,   ///< supports PCM short/long frame Protocol
				0,   ///< supports AC'97 Protocol
				0,   ///< supports Mono mode
				0,   ///< supports Companding
				0,   ///< supports MCLK (Master Clock) pin
				0,   ///< supports Frame error event: \ref ARM_SAI_EVENT_FRAME_ERROR
		},
		I2S1,
		I2S1_IRQn,
#if (RTE_I2S1_CHNL_UDMA_TX_EN == 1U)
		&I2S1_UDMA_TX_CHNL,
#else
		NULL,
#endif
#if (RTE_I2S1_CHNL_UDMA_RX_EN == 1U)
		&I2S1_UDMA_RX_CHNL,
#else
		NULL,
#endif
		I2S1_TX_FIFO_LEVEL,
		I2S1_RX_FIFO_LEVEL,  
		0,
		&I2S1_Info,  
		0,
		I2S_PROTOCOL,
		&I2S1_Clk ,
		{
		//pins
		&i2s1_sclk,
		&i2s1_wsclk,
		&i2s1_din0,
		&i2s1_dout0,
		NULL,
		NULL,
		},
};
#endif

// Extern Function

/**
  \fn          ARM_DRIVER_VERSION I2Sx_GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION                                */
static ARM_DRIVER_VERSION I2Sx_GetVersion (void) 
{
	return (DriverVersion);
}

/**
  \fn          ARM_SAI_CAPABILITIES I2Sx_GetCapabilities (void)
  \brief       Get driver capabilities.
  \param[in]   i2s       Pointer to I2S resources
  \return      \ref ARM_SAI_CAPABILITIES
 */
static ARM_SAI_CAPABILITIES I2S_GetCapabilities (I2S_RESOURCES *i2s) 
{
	return (i2s->capabilities);
}


#if (RTE_I2S0)
// I2S0 Driver Wrapper functions
static ARM_SAI_CAPABILITIES I2S0_GetCapabilities (void) 
{
	return I2S_GetCapabilities (&I2S0_Resources);
}

static int32_t I2S0_Initialize (ARM_SAI_SignalEvent_t cb_event) 
{
#if defined(A11_ROM)
	return ROMAPI_I2S_API->I2S_Initialize (cb_event, &I2S0_Resources,&UDMA0_Resources,UDMA0_Table,&udmaHandle0,dma_rom_buff0);
#else
	return I2S_Initialize (cb_event, &I2S0_Resources,&UDMA0_Resources,UDMA0_Table,&udmaHandle0,dma_rom_buff0); 	
#endif
}

static int32_t I2S0_Uninitialize (void) 
{
#if defined(A11_ROM) && defined(I2S_ROMDRIVER_PRESENT)
	return ROMAPI_I2S_API->I2S_Uninitialize (&I2S0_Resources,&UDMA0_Resources); 	 	
#else
	return I2S_Uninitialize (&I2S0_Resources,&UDMA0_Resources); 	
#endif
}

static int32_t I2S0_PowerControl (ARM_POWER_STATE state) 
{
#if defined(A11_ROM)
	return ROMAPI_I2S_API->I2S_PowerControl (state, &I2S0_Resources,&UDMA0_Resources,udmaHandle0);
#else
	return I2S_PowerControl (state, &I2S0_Resources,&UDMA0_Resources,udmaHandle0);
#endif	
}

static int32_t I2S0_Send (const void *data, uint32_t num) 
{
#if defined(A11_ROM) && defined(I2S_ROMDRIVER_PRESENT)
	return ROMAPI_I2S_API->I2S_Send (data, num, &I2S0_Resources,&UDMA0_Resources,udma0_chnl_info,udmaHandle0);	
#else
	return I2S_Send (data, num, &I2S0_Resources,&UDMA0_Resources,udma0_chnl_info,udmaHandle0); 	
#endif	
}

static int32_t I2S0_Receive (void *data, uint32_t num) 
{
#if defined(A11_ROM) && defined(I2S_ROMDRIVER_PRESENT)
	return ROMAPI_I2S_API->I2S_Receive (data, num, &I2S0_Resources,&UDMA0_Resources,udma0_chnl_info,udmaHandle0);
#else
	return I2S_Receive (data, num, &I2S0_Resources,&UDMA0_Resources,udma0_chnl_info,udmaHandle0); 	
#endif	
}

static uint32_t I2S0_GetTxCount (void) 
{
#if defined(A11_ROM)
	return ROMAPI_I2S_API->I2S_GetTxCount (&I2S0_Resources); 	 				
#else
	return I2S_GetTxCount (&I2S0_Resources); 	
#endif	
}

static uint32_t I2S0_GetRxCount (void) 
{
#if defined(A11_ROM)
	return ROMAPI_I2S_API->I2S_GetRxCount (&I2S0_Resources); 	 
#else
	return I2S_GetRxCount (&I2S0_Resources);
#endif	
}

static int32_t I2S0_Control (uint32_t control, uint32_t arg1, uint32_t arg2) 
{
#if defined(A11_ROM) && defined(I2S_ROMDRIVER_PRESENT)
    return ROMAPI_I2S_API->I2S_Control (control, arg1, arg2, &I2S0_Resources,&UDMA0_Resources,udmaHandle0);
#else
    return I2S_Control (control, arg1, arg2, &I2S0_Resources,&UDMA0_Resources,udmaHandle0);
#endif	
}

static ARM_SAI_STATUS I2S0_GetStatus (void) 
{
#if  defined(A11_ROM)
	return ROMAPI_I2S_API->I2S_GetStatus (&I2S0_Resources);	
#else
	return I2S_GetStatus (&I2S0_Resources); 	
#endif	
}

void IRQ064_Handler (void) 
{
#if defined(A11_ROM)
	ROMAPI_I2S_API-> I2S_IRQHandler (&I2S0_Resources);
#else
	I2S_IRQHandler (&I2S0_Resources); 	
#endif	
}

#if (RTE_I2S0_CHNL_UDMA_TX_EN == 1)
void I2S0_UDMA_Tx_Event (uint32_t event, uint32_t dmaCh)
{
#if defined(A11_ROM)
	ROMAPI_I2S_API->I2S_UDMA_Tx_Event(event,dmaCh,&I2S0_Resources);
#else
	I2S_UDMA_Tx_Event(event,(uint8_t)dmaCh,&I2S0_Resources);
#endif
}
#endif

#if (RTE_I2S0_CHNL_UDMA_RX_EN == 1)
void I2S0_UDMA_Rx_Event (uint32_t event,uint32_t dmaCh)
{
#if defined(A11_ROM)
	ROMAPI_I2S_API->I2S_UDMA_Rx_Event (event,dmaCh, &I2S0_Resources);
#else
	I2S_UDMA_Rx_Event (event,(uint8_t)dmaCh, &I2S0_Resources);
#endif	
}
#endif

// SAI0 Driver Control Block
ARM_DRIVER_SAI Driver_SAI0 = {
		I2Sx_GetVersion,
		I2S0_GetCapabilities,
		I2S0_Initialize,
		I2S0_Uninitialize,
		I2S0_PowerControl,
		I2S0_Send,
		I2S0_Receive,
		I2S0_GetTxCount,
		I2S0_GetRxCount,
		I2S0_Control,
		I2S0_GetStatus
};
#endif

#if (RTE_I2S1)
// I2S1 Driver Wrapper functions
static ARM_SAI_CAPABILITIES I2S1_GetCapabilities (void) 
{
	return I2S_GetCapabilities (&I2S1_Resources);
}

static int32_t I2S1_Initialize (ARM_SAI_SignalEvent_t cb_event) 
{
#if defined(A11_ROM) && defined(I2S_ROMDRIVER_PRESENT)
	return ROMAPI_I2S_API->I2S_Initialize (cb_event, &I2S1_Resources,&UDMA1_Resources,UDMA1_Table,&udmaHandle1,dma_rom_buff1);
#else
	return I2S_Initialize (cb_event, &I2S1_Resources,&UDMA1_Resources,UDMA1_Table,&udmaHandle1,dma_rom_buff1);
#endif
}

static int32_t I2S1_Uninitialize (void) 
{
	RSI_PS_UlpssPeriPowerDown(ULPSS_PWRGATE_ULP_I2S);
#if defined(A11_ROM) && defined(I2S_ROMDRIVER_PRESENT)
	return ROMAPI_I2S_API->I2S_Uninitialize (&I2S1_Resources,&UDMA1_Resources);
#else
	return I2S_Uninitialize (&I2S1_Resources,&UDMA1_Resources);
#endif	
}

static int32_t I2S1_PowerControl (ARM_POWER_STATE state) 
{
#if defined(A11_ROM)
	return ROMAPI_I2S_API->I2S_PowerControl (state, &I2S1_Resources,&UDMA1_Resources,udmaHandle1); 	 
#else
	return I2S_PowerControl (state, &I2S1_Resources,&UDMA1_Resources,udmaHandle1);
#endif	
}

static int32_t I2S1_Send (const void *data, uint32_t num) 
{
#if defined(A11_ROM) && defined(I2S_ROMDRIVER_PRESENT)
	return ROMAPI_I2S_API->I2S_Send (data, num, &I2S1_Resources,&UDMA1_Resources,udma1_chnl_info,udmaHandle1);
#else
	return I2S_Send (data, num, &I2S1_Resources,&UDMA1_Resources,udma1_chnl_info,udmaHandle1); 	
#endif
}

static int32_t I2S1_Receive (void *data, uint32_t num) 
{
#if defined(A11_ROM) && defined(I2S_ROMDRIVER_PRESENT)
	return ROMAPI_I2S_API->I2S_Receive (data, num, &I2S1_Resources,&UDMA1_Resources,udma1_chnl_info,udmaHandle1);
#else
	return I2S_Receive (data, num, &I2S1_Resources,&UDMA1_Resources,udma1_chnl_info,udmaHandle1);
#endif	
}

static uint32_t I2S1_GetTxCount (void) 
{
#if defined(A11_ROM)
	return ROMAPI_I2S_API->I2S_GetTxCount (&I2S1_Resources); 	 
#else
	return I2S_GetTxCount (&I2S1_Resources); 	
#endif	
}

static uint32_t I2S1_GetRxCount (void) 
{
#if defined(A11_ROM)
	return ROMAPI_I2S_API->I2S_GetRxCount (&I2S1_Resources); 	 	
#else
	return I2S_GetRxCount (&I2S1_Resources);
#endif	
}

static int32_t I2S1_Control (uint32_t control, uint32_t arg1, uint32_t arg2) 
{
#if defined(A11_ROM) && defined(I2S_ROMDRIVER_PRESENT)
	return ROMAPI_I2S_API->I2S_Control (control, arg1, arg2, &I2S1_Resources,&UDMA1_Resources,udmaHandle1);
#else
	return I2S_Control (control, arg1, arg2, &I2S1_Resources,&UDMA1_Resources,udmaHandle1);
#endif	
}

static ARM_SAI_STATUS I2S1_GetStatus (void) 
{
#if defined(A11_ROM)
	return ROMAPI_I2S_API->I2S_GetStatus (&I2S1_Resources);
#else
	return I2S_GetStatus (&I2S1_Resources);
#endif
}

void I2S1_IRQHandler (void) 
{
#if defined(A11_ROM)
	ROMAPI_I2S_API->I2S_IRQHandler (&I2S1_Resources);		
#else
	I2S_IRQHandler (&I2S1_Resources); 
#endif	
}

#if (RTE_I2S1_CHNL_UDMA_TX_EN == 1)
void I2S1_UDMA_Tx_Event (uint32_t event,uint32_t dmaCh)
{
#if defined(A11_ROM)
	ROMAPI_I2S_API->I2S_UDMA_Tx_Event (event, dmaCh,&I2S1_Resources);	
#else
	I2S_UDMA_Tx_Event (event,(uint8_t)dmaCh,&I2S1_Resources);
#endif	
}
#endif

#if (RTE_I2S1_CHNL_UDMA_RX_EN == 1)
void I2S1_UDMA_Rx_Event (uint32_t event, uint32_t dmaCh)
{
#if defined(A11_ROM)
	ROMAPI_I2S_API->I2S_UDMA_Rx_Event (event, dmaCh, &I2S1_Resources);
#else
	I2S_UDMA_Rx_Event (event,(uint8_t)dmaCh, &I2S1_Resources);
#endif	
}
#endif

uint8_t I2S_GetInitState(uint8_t i2s_instance)
{
  uint8_t status = 0;

  //Get the initialization status of I2S instance
  if(i2s_instance == 0) {
      status = (I2S0_Resources.flags && (0x01));
  }
  if(i2s_instance == 1) {
      status = (I2S1_Resources.flags && (0x01));
  }

  return status;
}
// SAI1 Driver Control Block
ARM_DRIVER_SAI Driver_SAI1 = {
		I2Sx_GetVersion,
		I2S1_GetCapabilities,
		I2S1_Initialize,
		I2S1_Uninitialize,
		I2S1_PowerControl,
		I2S1_Send,
		I2S1_Receive,
		I2S1_GetTxCount,
		I2S1_GetRxCount,
		I2S1_Control,
		I2S1_GetStatus
};
#endif

