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
 * Driver:       Driver_SPI
 * Configured:   via RTE_Device_917.h configuration file
 * Project:      SPI Driver for RS1xxxx
 */

/* History:
 * Version 1.0
 * Initial release
 */
#include "SPI.h"
#include "UDMA.h"
#include "rsi_rom_udma_wrapper.h" 
#include "rsi_ccp_user_config.h"
#include "clock_update.h"
#include "rsi_spi.h"
#include "rsi_rom_table_si91x.h"

extern RSI_UDMA_HANDLE_T udmaHandle0;    //check
extern uint32_t dma_rom_buff0[30],dma_rom_buff1[30];     //we can keep wrapeers

#define CONTROL_STRUCT0   (UDMA_NUMBER_OF_CHANNELS * 2)
#define CONTROL_STRUCT1   (ULP_UDMA_NUMBER_OF_CHANNELS * 2)

#define LENGTH_OFFSET 1 // Offset used while returning frame length

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

#define ARM_SPI_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(2, 0) /* driver version */

extern RSI_UDMA_HANDLE_T udmaHandle0,udmaHandle1;

#define ARM_SPI_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(2, 0) /* driver version */

#define RSI_MUTLI_SLAVE_SELECT_ERROR -7 
/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
		ARM_SPI_API_VERSION,
		ARM_SPI_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_SPI_CAPABILITIES DriverCapabilities = {
		0, /* Simplex Mode (Master and Slave) */
		1, /* TI Synchronous Serial Interface */
		1, /* Microwire Interface */
		1  /* Signal Mode Fault event: \ref SPI_EVENT_MODE_FAULT */
};

#ifdef SSI_MASTER

#define  SSI_MASTER_IRQHandler     IRQ047_Handler
// SSI_MASTER Run-Time Information
static SPI_INFO          SSI_MASTER_Info         = { 0U };
static SPI_TRANSFER_INFO SSI_MASTER_TransferInfo = { 0U };

#ifdef SSI_MASTER_MOSI_SEL
static SPI_PIN SSI_MASTER_mosi = {SSI_MASTER_MOSI_PORT, SSI_MASTER_MOSI_PIN, SSI_MASTER_MOSI_MODE, SSI_MASTER_MOSI_PADSEL};
#endif
#ifdef SSI_MASTER_MISO_SEL
static SPI_PIN SSI_MASTER_miso = {SSI_MASTER_MISO_PORT, SSI_MASTER_MISO_PIN, SSI_MASTER_MISO_MODE, SSI_MASTER_MISO_PADSEL};
#endif
#if (defined(SPI_QUAD_MODE) && (SPI_QUAD_MODE == 1))
#ifdef SSI_MASTER_DATA2_SEL
static SPI_PIN SSI_MASTER_data2 = {SSI_MASTER_DATA2__PORT, SSI_MASTER_DATA2__PIN, SSI_MASTER_DATA2_MODE, SSI_MASTER_DATA2_PADSEL};
#endif
#ifdef SSI_MASTER_DATA3_SEL
static SPI_PIN SSI_MASTER_data3 = {SSI_MASTER_DATA3__PORT, SSI_MASTER_DATA3__PIN, SSI_MASTER_DATA3_MODE, SSI_MASTER_DATA3_PADSEL};
#endif
#endif
#ifdef SSI_MASTER_SCK_SEL
static SPI_PIN SSI_MASTER_sck  = {SSI_MASTER_SCK_PORT,  SSI_MASTER_SCK_PIN, SSI_MASTER_SCK_MODE, SSI_MASTER_SCK_PADSEL};
#endif
#ifdef SSI_MASTER_CS0_SEL
static SPI_PIN SSI_MASTER_cs0  =  {SSI_MASTER_CS0_PORT,  SSI_MASTER_CS0_PIN, SSI_MASTER_CS0_MODE, SSI_MASTER_CS0_PADSEL};
#endif
#ifdef SPI_MULTI_SLAVE
#ifdef SSI_MASTER_CS1_SEL
static SPI_PIN SSI_MASTER_cs1  =  {SSI_MASTER_CS1_PORT,  SSI_MASTER_CS1_PIN, SSI_MASTER_CS1_MODE, SSI_MASTER_CS1_PADSEL};
#endif
#ifdef SSI_MASTER_CS2_SEL
static SPI_PIN SSI_MASTER_cs2  =  {SSI_MASTER_CS2_PORT,  SSI_MASTER_CS2_PIN, SSI_MASTER_CS2_MODE, SSI_MASTER_CS2_PADSEL};
#endif
#ifdef SSI_MASTER_CS3_SEL
static SPI_PIN SSI_MASTER_cs3  =  {SSI_MASTER_CS3_PORT,  SSI_MASTER_CS3_PIN, SSI_MASTER_CS3_MODE, SSI_MASTER_CS3_PADSEL};
#endif
#endif
#if defined(SSI_MASTER_TX_DMA_Instance) && (SSI_MASTER_TX_DMA_Instance == 1)
void SSI_MASTER_UDMA_Tx_Event (uint32_t event ,uint32_t dmaCh);
static SPI_DMA SSI_MASTER_UDMA_TX_CHNL = {
		{
				0,
				0,
				0,
				0,
				1,
				SSIMASTER_ACK,
				SSI_MASTER_TX_DMA_Channel
		},
		SSI_MASTER_TX_DMA_Channel,
		SSI_MASTER_UDMA_Tx_Event
};
#endif
#if defined(SSI_MASTER_RX_DMA_Instance) && (SSI_MASTER_RX_DMA_Instance == 1)
void SSI_MASTER_UDMA_Rx_Event (uint32_t event ,uint32_t dmaCh);
static SPI_DMA SSI_MASTER_UDMA_RX_CHNL = {
		{
				0,
				0,
				0,
				0,
				1,
				SSIMASTER_ACK,
				SSI_MASTER_RX_DMA_Channel
		},
		SSI_MASTER_RX_DMA_Channel,
		SSI_MASTER_UDMA_Rx_Event
};
#endif

// SSI_MASTER Resources
static const SPI_RESOURCES SSI_MASTER_Resources = {
		SSI0,
		// PINS
		{
#ifdef SSI_MASTER_MOSI_SEL
				&SSI_MASTER_mosi,
#else
				NULL,
#endif
#ifdef SSI_MASTER_MISO_SEL
				&SSI_MASTER_miso,
#else
				NULL,
#endif
#if (defined(SPI_QUAD_MODE) && (SPI_QUAD_MODE == 1))
#ifdef SSI_MASTER_DATA2_SEL
        &SSI_MASTER_data2,
#else
        NULL,
#endif
#ifdef SSI_MASTER_DATA3_SEL
        &SSI_MASTER_data3,
#else
        NULL,
#endif
#endif
#ifdef SSI_MASTER_SCK_SEL
				&SSI_MASTER_sck,
#else
				NULL,
#endif
#ifdef SSI_MASTER_CS0_SEL
				&SSI_MASTER_cs0,
#else
				NULL,
#endif
#ifdef SPI_MULTI_SLAVE
#ifdef SSI_MASTER_CS1_SEL
				&SSI_MASTER_cs1,
#else
				NULL,
#endif
#ifdef SSI_MASTER_CS2_SEL
				&SSI_MASTER_cs2,
#else
				NULL,
#endif
#ifdef SSI_MASTER_CS3_SEL
				&SSI_MASTER_cs3,
#else
				NULL,
#endif
#endif
		},

		SSI0_IRQn,

#ifdef SSI_MASTER_RX_DMA_Instance
		&SSI_MASTER_UDMA_RX_CHNL,
#else
		NULL,
#endif

#ifdef SSI_MASTER_TX_DMA_Instance
		&SSI_MASTER_UDMA_TX_CHNL,
#else
		NULL,
#endif
    &SSI_MASTER_Info,
    &SSI_MASTER_TransferInfo,
    SPI_MASTER_MODE,
    {
    SSI_INTFPLLCLK,
    ULP_SSI_ULP_MHZ_RC_CLK,
    0,
    }
};
#endif /* SSI_Master */
#ifdef SSI_SLAVE

#define  SSI_SLAVE_IRQHandler     IRQ044_Handler
// SSI_SLAVE Run-Time Information
static SPI_INFO          SSI_SLAVE_Info         = { 0U };
static SPI_TRANSFER_INFO SSI_SLAVE_TransferInfo = { 0U };

#ifdef SSI_SLAVE_MOSI_SEL
static SPI_PIN SSI_SLAVE_mosi = {SSI_SLAVE_MOSI_PORT, SSI_SLAVE_MOSI_PIN, SSI_SLAVE_MOSI_MODE, SSI_SLAVE_MOSI_PADSEL};
#endif
#ifdef SSI_SLAVE_MISO_SEL
static SPI_PIN SSI_SLAVE_miso = {SSI_SLAVE_MISO_PORT, SSI_SLAVE_MISO_PIN, SSI_SLAVE_MISO_MODE, SSI_SLAVE_MISO_PADSEL};
#endif
#ifdef SSI_SLAVE_SCK_SEL
static SPI_PIN SSI_SLAVE_sck  = {SSI_SLAVE_SCK_PORT,  SSI_SLAVE_SCK_PIN, SSI_SLAVE_SCK_MODE, SSI_SLAVE_SCK_PADSEL};
#endif
#ifdef SSI_SLAVE_CS0_SEL
static SPI_PIN SSI_SLAVE_cs  = {SSI_SLAVE_CS0_PORT,  SSI_SLAVE_CS0_PIN, SSI_SLAVE_CS0_MODE, SSI_SLAVE_CS0_PADSEL};
#endif

#if defined(SSI_SLAVE_TX_DMA_Instance) && (SSI_SLAVE_TX_DMA_Instance == 1)
void SSI_SLAVE_UDMA_Tx_Event (uint32_t event ,uint32_t dmaCh);
static SPI_DMA SSI_SLAVE_UDMA_TX_CHNL = {
		{
				0,
				0,
				0,
				0,
				1,
				SSISLAVE_ACK,
				SSI_SLAVE_TX_DMA_Channel
		},
		SSI_SLAVE_TX_DMA_Channel,
		SSI_SLAVE_UDMA_Tx_Event
};
#endif
#if defined(SSI_SLAVE_RX_DMA_Instance) && (SSI_SLAVE_RX_DMA_Instance == 1)
void SSI_SLAVE_UDMA_Rx_Event (uint32_t event ,uint32_t dmaCh);
static SPI_DMA SSI_SLAVE_UDMA_RX_CHNL = {
		{
				0,
				0,
				0,
				0,
				1,
				SSISLAVE_ACK,
				SSI_SLAVE_RX_DMA_Channel
		},
		SSI_SLAVE_RX_DMA_Channel,
		SSI_SLAVE_UDMA_Rx_Event
};
#endif

// SSI_SLAVE Resources
static const SPI_RESOURCES SSI_SLAVE_Resources = {
		SSISlave,
		// PINS
		{
#ifdef SSI_SLAVE_MOSI_SEL
				&SSI_SLAVE_mosi,
#else
				NULL,
#endif
#ifdef SSI_SLAVE_MISO_SEL
				&SSI_SLAVE_miso,
#else
				NULL,
#endif
#if (defined(SPI_QUAD_MODE) && (SPI_QUAD_MODE == 1))
        NULL,
        NULL,
#endif
#ifdef SSI_SLAVE_SCK_SEL
				&SSI_SLAVE_sck,
#else
				NULL,
#endif
#ifdef SSI_SLAVE_CS0_SEL
				&SSI_SLAVE_cs,
#else
				NULL,
#endif
#ifdef SPI_MULTI_SLAVE
#ifdef SSI_SLAVE_CS1_SEL
                                                     &SSI_SLAVE_cs,
#else
                                                     NULL,
#endif
#ifdef SSI_SLAVE_CS2_SEL
                                                     &SSI_SLAVE_cs,
#else
                                                     NULL,
#endif
#ifdef SSI_SLAVE_CS3_SEL
                                                     &SSI_SLAVE_cs,
#else
				NULL,
#endif
#endif
                                                   },
#ifdef SLI_SI917
                                                   SSISlave_IRQn,
#endif
#ifdef	CHIP_9118
		SSI1_IRQn,
#endif
		
#ifdef SSI_SLAVE_RX_DMA_Instance
		&SSI_SLAVE_UDMA_RX_CHNL,
#else
		NULL,
#endif

#ifdef SSI_SLAVE_TX_DMA_Instance
		&SSI_SLAVE_UDMA_TX_CHNL,
#else
		NULL,
#endif

    &SSI_SLAVE_Info,
    &SSI_SLAVE_TransferInfo,
    SPI_SLAVE_MODE,
    {
    SSI_INTFPLLCLK,
    ULP_SSI_ULP_MHZ_RC_CLK,
    0,
    }
};
#endif /* SSI_SLAVE */

#ifdef SSI_ULP_MASTER

#define  SSI_ULP_MASTER_IRQHandler     IRQ016_Handler
// SSI_ULP_MASTER Run-Time Information
static SPI_INFO          SSI_ULP_MASTER_Info         = { 0U };
static SPI_TRANSFER_INFO SSI_ULP_MASTER_TransferInfo = { 0U };

#ifdef SSI_ULP_MASTER_MOSI_SEL
static SPI_PIN SSI_ULP_MASTER_mosi = {SSI_ULP_MASTER_MOSI_PORT, SSI_ULP_MASTER_MOSI_PIN, SSI_ULP_MASTER_MOSI_MODE, SSI_ULP_MASTER_MOSI_PADSEL};
#endif
#ifdef SSI_ULP_MASTER_MISO_SEL
static SPI_PIN SSI_ULP_MASTER_miso = {SSI_ULP_MASTER_MISO_PORT, SSI_ULP_MASTER_MISO_PIN, SSI_ULP_MASTER_MISO_MODE, SSI_ULP_MASTER_MISO_PADSEL};
#endif
#ifdef SSI_ULP_MASTER_SCK_SEL
static SPI_PIN SSI_ULP_MASTER_sck  = {SSI_ULP_MASTER_SCK_PORT,  SSI_ULP_MASTER_SCK_PIN, SSI_ULP_MASTER_SCK_MODE, SSI_ULP_MASTER_SCK_PADSEL};
#endif
#ifdef SSI_ULP_MASTER_CS0_SEL
static SPI_PIN SSI_ULP_MASTER_cs0  = {SSI_ULP_MASTER_CS0_PORT,  SSI_ULP_MASTER_CS0_PIN, SSI_ULP_MASTER_CS0_MODE, SSI_ULP_MASTER_CS0_PADSEL};
#endif
#ifdef SPI_MULTI_SLAVE
#ifdef SSI_ULP_MASTER_CS1_SEL
static SPI_PIN SSI_ULP_MASTER_cs1  =  {SSI_ULP_MASTER_CS1_PORT,  SSI_ULP_MASTER_CS1_PIN, SSI_ULP_MASTER_CS1_MODE, SSI_ULP_MASTER_CS1_PADSEL};
#endif
#ifdef SSI_ULP_MASTER_CS2_SEL
static SPI_PIN SSI_ULP_MASTER_cs2  =  {SSI_ULP_MASTER_CS2_PORT,  SSI_ULP_MASTER_CS2_PIN, SSI_ULP_MASTER_CS2_MODE, SSI_ULP_MASTER_CS2_PADSEL};
#endif
#endif

#if defined(SSI_ULP_MASTER_TX_DMA_Instance) && (SSI_ULP_MASTER_TX_DMA_Instance == 1)
void SSI_ULP_MASTER_UDMA_Tx_Event (uint32_t event ,uint32_t dmaCh);
static SPI_DMA SSI_ULP_MASTER_UDMA_TX_CHNL = {
		{
				0,
				0,
				0,
				0,
				1,
				0,
				SSI_ULP_MASTER_TX_DMA_Channel
		},
		SSI_ULP_MASTER_TX_DMA_Channel,
		SSI_ULP_MASTER_UDMA_Tx_Event
};
#endif
#if defined(SSI_ULP_MASTER_RX_DMA_Instance) && (SSI_ULP_MASTER_RX_DMA_Instance == 1)
void SSI_ULP_MASTER_UDMA_Rx_Event (uint32_t event ,uint32_t dmaCh);
static SPI_DMA SSI_ULP_MASTER_UDMA_RX_CHNL = {
		{
				0,
				0,
				0,
				0,
				1,
				0,
				SSI_ULP_MASTER_RX_DMA_Channel
		},
		SSI_ULP_MASTER_RX_DMA_Channel,
		SSI_ULP_MASTER_UDMA_Rx_Event
};
#endif

// SSI_ULP_MASTER Resources
static const SPI_RESOURCES SSI_ULP_MASTER_Resources = {
		SSI2,
		// PINS
		{
#ifdef SSI_ULP_MASTER_MOSI_SEL
				&SSI_ULP_MASTER_mosi,
#else
				NULL,
#endif
#ifdef SSI_ULP_MASTER_MISO_SEL
				&SSI_ULP_MASTER_miso,
#else
				NULL,
#endif
#if (defined(SPI_QUAD_MODE) && (SPI_QUAD_MODE == 1))
        NULL,
        NULL,
#endif
#ifdef SSI_ULP_MASTER_SCK_SEL
				&SSI_ULP_MASTER_sck,
#else
				NULL,
#endif
#ifdef SSI_ULP_MASTER_CS0_SEL
				&SSI_ULP_MASTER_cs0,
#else
				NULL,
#endif
#ifdef SPI_MULTI_SLAVE
#ifdef SSI_ULP_MASTER_CS1_SEL
                                                          &SSI_ULP_MASTER_cs1,
#else
				NULL,
#endif
#ifdef SSI_ULP_MASTER_CS2_SEL
                                                          &SSI_ULP_MASTER_cs2,
#else
                                                          NULL,
#endif
#ifdef SSI_SLAVE_CS3_SEL
                                                          &SSI_SLAVE_cs,
#else
                                                          NULL,
#endif
#endif
                                                        },

		SSI2_IRQn,

#ifdef SSI_ULP_MASTER_RX_DMA_Instance
		&SSI_ULP_MASTER_UDMA_RX_CHNL,
#else
		NULL,
#endif

#ifdef SSI_ULP_MASTER_TX_DMA_Instance
		&SSI_ULP_MASTER_UDMA_TX_CHNL,
#else
		NULL,
#endif

    &SSI_ULP_MASTER_Info,
    &SSI_ULP_MASTER_TransferInfo,
    SPI_ULP_MASTER_MODE,
    {
    SSI_INTFPLLCLK,
    ULP_SSI_REF_CLK,
    0,
    }
};
#endif /* SSI_ULP_MASTER */

/**
 * @fn           ARM_DRIVER_VERSION SPI_GetVersion(void)
 * @brief        Get Driver Version.
 * @param[in]    none
 * @return 		   ARM DRIVER VERSION
 */
ARM_DRIVER_VERSION SPI_GetVersion(void)
{
	return DriverVersion;
}

/**
 * @fn           ARM_SPI_CAPABILITIES SPI_GetCapabilities(void)
 * @brief        Get Driver capabilities.
 * @param[in]    none
 * @return 		   ARM SPI VERSION
 */
ARM_SPI_CAPABILITIES SPI_GetCapabilities(void)
{
	return DriverCapabilities;
}

// SSI_MASTER
#ifdef SSI_MASTER
static int32_t SSI_MASTER_Initialize(ARM_SPI_SignalEvent_t pSignalEvent)             
   { 		 
		 /*Power up the ssi*/
		#ifdef SLI_SI917
		RSI_PS_M4ssPeriPowerUp(M4SS_PWRGATE_ULP_EFUSE_PERI);
		#else
		RSI_PS_M4ssPeriPowerUp(M4SS_PWRGATE_ULP_PERI1);
		#endif
     #if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_Initialize(pSignalEvent, &SSI_MASTER_Resources,
		   &UDMA0_Resources,UDMA0_Table,&udmaHandle0,dma_rom_buff0);
		 #else
	 return SPI_Initialize (pSignalEvent, &SSI_MASTER_Resources,
		   &UDMA0_Resources,UDMA0_Table,&udmaHandle0,dma_rom_buff0);
		 #endif
	 }
static int32_t SSI_MASTER_Uninitialize (void)
{ 
			 #if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_Uninitialize(&SSI_MASTER_Resources,&UDMA0_Resources);
		 #else
return SPI_Uninitialize (&SSI_MASTER_Resources,&UDMA0_Resources); 
	#endif
}
static int32_t SSI_MASTER_PowerControl(ARM_POWER_STATE state)
{
	 #if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_PowerControl(state, &SSI_MASTER_Resources);
	#else
return SPI_PowerControl (state, &SSI_MASTER_Resources); 
	#endif
}
static int32_t SSI_MASTER_Send (const void *data, uint32_t num)
{ 
			 #if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_Send(data, num, &SSI_MASTER_Resources,&UDMA0_Resources,
                        udma0_chnl_info,
                        udmaHandle0);
		 #else
return SPI_Send (data, num, &SSI_MASTER_Resources,&UDMA0_Resources,
                        udma0_chnl_info,
                        udmaHandle0); 
	#endif
}

int32_t SSI_MASTER_Send_Command_Data (void *data, uint32_t num, uint32_t instruction, uint32_t address)
{
  return SPI_Send_Command_Data (data, num, instruction, address, &SSI_MASTER_Resources,&UDMA0_Resources,
                      udma0_chnl_info,
                      udmaHandle0);
}

static int32_t SSI_MASTER_Receive (void *data, uint32_t num)
{ 
			 #if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_Receive(data, num, &SSI_MASTER_Resources,&UDMA0_Resources,
                        udma0_chnl_info,
                        udmaHandle0);
		 #else
  return SPI_Receive (data, num, &SSI_MASTER_Resources,&UDMA0_Resources,
                        udma0_chnl_info,
                        udmaHandle0); 
	#endif
}

int32_t SSI_MASTER_Receive_Command_Data (void *data, uint32_t num, uint32_t instruction, uint32_t address, uint32_t wait_cycle)
{
  return SPI_Receive_Command_Data (data, num, instruction, address, wait_cycle, &SSI_MASTER_Resources,&UDMA0_Resources,
                      udma0_chnl_info,
                      udmaHandle0);
}

static int32_t SSI_MASTER_Transfer(const void *data_out, void *data_in, uint32_t num)
 {
	 		 #if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_Transfer(data_out, data_in, num, &SSI_MASTER_Resources,&UDMA0_Resources,
                        udma0_chnl_info,
                        udmaHandle0);
		 #else
 return SPI_Transfer (data_out, data_in, num, &SSI_MASTER_Resources,&UDMA0_Resources,
                        udma0_chnl_info,
                        udmaHandle0) ;
	 #endif
 }
static uint32_t SSI_MASTER_GetDataCount(void)
{
		 #if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_GetDataCount(&SSI_MASTER_Resources);
		 #else	
return SPI_GetDataCount (&SSI_MASTER_Resources); 
	#endif
}
	
uint8_t spi_slavenumber=0xA5;

void RSI_SPI_SetSlaveSelectNumber(uint8_t slavenumber)
{
	spi_slavenumber=slavenumber;
}

void RSI_SPI_Slave_Disable(void)
{
  SPI_Clear_SSI_Enable_State(&SSI_SLAVE_Resources);
}
void RSI_SPI_Slave_Set_CS_Init_State(void)
{
  SPI_Slave_Set_CS_Init_State(&SSI_SLAVE_Resources);
}
uint8_t RSI_SPI_GetSlaveSelectNumber(void)
{
	return spi_slavenumber;
}
 
static int32_t SSI_MASTER_Control(uint32_t control, uint32_t arg)   
{ 
//!ROM bypass
  int32_t status         = 0;
 uint32_t ssi_get_clock=0;
 ssi_get_clock = RSI_CLK_GetBaseClock(M4_SSI_MST);
	#ifdef SPI_MULTI_SLAVE
  if((control & ARM_SPI_CONTROL_Msk )== ARM_SPI_CONTROL_SS )
	 {
	  spi_slavenumber =RSI_SPI_GetSlaveSelectNumber();
	/* check slave number is valied or not */
		if(spi_slavenumber == 0xA5)
		{
			/* Update the slave number by using RSI_SPI_SetSlaveSelectNumber API */
		return RSI_MUTLI_SLAVE_SELECT_ERROR; 
		}
	}
	 #endif
	#if (defined(SSI_ROMDRIVER_PRESENT))
  #if (defined(A11_ROM) && (defined(SLI_SI917)) && defined(SLI_SI917B0))
  return ROMAPI_SSI_API->SPI_Control(control, arg, &SSI_MASTER_Resources,ssi_get_clock,spi_slavenumber);
	#else
	return ROMAPI_SSI_API->SPI_Control(control, arg, &SSI_MASTER_Resources,ssi_get_clock);
	#endif
	#else
  status = SPI_Control(control, arg, &SSI_MASTER_Resources, ssi_get_clock, spi_slavenumber);
	#endif
  return status;
	}
static ARM_SPI_STATUS SSI_MASTER_GetStatus (void)
	{
				 #if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_GetStatus(&SSI_MASTER_Resources);
		 #else
   return SPI_GetStatus (&SSI_MASTER_Resources); 
		#endif
	}
void SSI_MASTER_IRQHandler (void) 
{		 
	#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
        ROMAPI_SSI_API->SPI_IRQHandler(&SSI_MASTER_Resources);
		 #else 
SPI_IRQHandler (&SSI_MASTER_Resources);
	#endif
}
#if defined(SSI_MASTER_TX_DMA_Instance) && (SSI_MASTER_TX_DMA_Instance == 1)
void SSI_MASTER_UDMA_Tx_Event (uint32_t event, uint32_t dmaCh) 
{
		 #if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
        ROMAPI_SSI_API->SPI_UDMA_Tx_Event(event,(uint8_t)dmaCh, &SSI_MASTER_Resources);
		 #else
SPI_UDMA_Tx_Event (event,(uint8_t)dmaCh, &SSI_MASTER_Resources);	
#endif
}
#endif
#if defined(SSI_MASTER_RX_DMA_Instance) && (SSI_MASTER_RX_DMA_Instance == 1)
void SSI_MASTER_UDMA_Rx_Event (uint32_t event, uint32_t dmaCh) 
{		 
#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
        ROMAPI_SSI_API->SPI_UDMA_Rx_Event(event,(uint8_t)dmaCh, &SSI_MASTER_Resources);
		 #else
SPI_UDMA_Rx_Event (event,(uint8_t)dmaCh, &SSI_MASTER_Resources);	
#endif
}
#endif


// End SPI Interface

ARM_DRIVER_SPI Driver_SSI_MASTER = {
		SPI_GetVersion,
		SPI_GetCapabilities,
		SSI_MASTER_Initialize,
		SSI_MASTER_Uninitialize,
		SSI_MASTER_PowerControl,
		SSI_MASTER_Send,
		SSI_MASTER_Receive,
		SSI_MASTER_Transfer,
		SSI_MASTER_GetDataCount,
		SSI_MASTER_Control,
		SSI_MASTER_GetStatus
};
#endif

// SSI_SLAVE
#ifdef SSI_SLAVE
static int32_t SSI_SLAVE_Initialize (ARM_SPI_SignalEvent_t pSignalEvent)
{ 
		 /*Power up the ssi*/
		#ifdef SLI_SI917
		RSI_PS_M4ssPeriPowerUp(M4SS_PWRGATE_ULP_EFUSE_PERI);
		#else
		RSI_PS_M4ssPeriPowerUp(M4SS_PWRGATE_ULP_PERI1);
		#endif
	#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_Initialize(pSignalEvent, &SSI_SLAVE_Resources,
	&UDMA0_Resources,UDMA0_Table,&udmaHandle0,dma_rom_buff0);
		 #else
return SPI_Initialize (pSignalEvent, &SSI_SLAVE_Resources,
	&UDMA0_Resources,UDMA0_Table,&udmaHandle0,dma_rom_buff0);
	#endif
}
static int32_t SSI_SLAVE_Uninitialize (void)
{
		#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_Uninitialize(&SSI_SLAVE_Resources,&UDMA0_Resources);
	#else
return SPI_Uninitialize (&SSI_SLAVE_Resources,&UDMA0_Resources); 
	#endif
}
static int32_t SSI_SLAVE_PowerControl (ARM_POWER_STATE state)
{
			#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_PowerControl(state, &SSI_SLAVE_Resources);
	#else
return SPI_PowerControl (state, &SSI_SLAVE_Resources);
	#endif
}
static int32_t SSI_SLAVE_Send (const void *data, uint32_t num) 
{ 
		#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_Send(data, num, &SSI_SLAVE_Resources,&UDMA0_Resources,
                        udma0_chnl_info,
                        udmaHandle0);
	#else
 return SPI_Send (data, num, &SSI_SLAVE_Resources,&UDMA0_Resources,
                        udma0_chnl_info,
                        udmaHandle0);
	#endif
}
static int32_t SSI_SLAVE_Receive(void *data, uint32_t num)
{
		#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_Receive(data, num, &SSI_SLAVE_Resources,&UDMA0_Resources,
                        udma0_chnl_info,
                        udmaHandle0);
	#else
 return SPI_Receive (data, num, &SSI_SLAVE_Resources,&UDMA0_Resources,
                        udma0_chnl_info,
                        udmaHandle0);
	#endif
}
static int32_t SSI_SLAVE_Transfer(const void *data_out, void *data_in, uint32_t num) 
{ 
			#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_Transfer(data_out, data_in, num, &SSI_SLAVE_Resources,&UDMA0_Resources,
                        udma0_chnl_info,
                        udmaHandle0);
	#else
	return SPI_Transfer (data_out, data_in, num, &SSI_SLAVE_Resources,&UDMA0_Resources,
                        udma0_chnl_info,
                        udmaHandle0); 
	#endif
}
static uint32_t       SSI_SLAVE_GetDataCount(void)
{
	#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_GetDataCount(&SSI_SLAVE_Resources);
	#else 
return SPI_GetDataCount (&SSI_SLAVE_Resources);
	#endif
}
static int32_t  SSI_SLAVE_Control (uint32_t control, uint32_t arg) 
{ 
  //!ROM bypass  
   uint32_t ssi_get_clock=0;
  int32_t status         = 0;
	#if (defined(SSI_ROMDRIVER_PRESENT))
  #if (defined(A11_ROM) && (defined(SLI_SI917)) && defined(SLI_SI917B0))
  return ROMAPI_SSI_API->SPI_Control(control, arg, &SSI_SLAVE_Resources,ssi_get_clock,0);
	#else
	return ROMAPI_SSI_API->SPI_Control(control, arg, &SSI_SLAVE_Resources,ssi_get_clock);
	#endif
	#else
	#ifdef SPI_MULTI_SLAVE
	return SPI_Control(control, arg, &SSI_SLAVE_Resources,ssi_get_clock,0);
	#else
	return SPI_Cotrol(control, arg, &SSI_SLAVE_Resources,ssi_get_clock);
	#endif
	#endif
  return status;
}
static ARM_SPI_STATUS SSI_SLAVE_GetStatus(void)
{
#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_GetStatus(&SSI_SLAVE_Resources);
	#else
return SPI_GetStatus (&SSI_SLAVE_Resources);
	#endif
}
void SSI_SLAVE_IRQHandler (void)
{     
	#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
        ROMAPI_SSI_API->SPI_IRQHandler(&SSI_SLAVE_Resources);
	#else
SPI_IRQHandler (&SSI_SLAVE_Resources); 
	#endif
}
#if defined(SSI_SLAVE_TX_DMA_Instance) && (SSI_SLAVE_TX_DMA_Instance == 1)
void SSI_SLAVE_UDMA_Tx_Event  (uint32_t event, uint32_t dmaCh) 
{	
	#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
        ROMAPI_SSI_API->SPI_UDMA_Tx_Event (event,(uint8_t)dmaCh, &SSI_SLAVE_Resources);	
#else
SPI_UDMA_Tx_Event (event,(uint8_t)dmaCh, &SSI_SLAVE_Resources);	
#endif
}
#endif

#if defined(SSI_SLAVE_RX_DMA_Instance) && (SSI_SLAVE_RX_DMA_Instance == 1)
void  SSI_SLAVE_UDMA_Rx_Event (uint32_t event, uint32_t dmaCh)
{
	#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
        ROMAPI_SSI_API->SPI_UDMA_Rx_Event (event,(uint8_t)dmaCh, &SSI_SLAVE_Resources);	
#else
SPI_UDMA_Rx_Event (event,(uint8_t)dmaCh, &SSI_SLAVE_Resources);	
#endif
}
#endif


// End SPI Interface

ARM_DRIVER_SPI Driver_SSI_SLAVE = {
		SPI_GetVersion,
		SPI_GetCapabilities,
		SSI_SLAVE_Initialize,
		SSI_SLAVE_Uninitialize,
		SSI_SLAVE_PowerControl,
		SSI_SLAVE_Send,
		SSI_SLAVE_Receive,
		SSI_SLAVE_Transfer,
		SSI_SLAVE_GetDataCount,
		SSI_SLAVE_Control,
		SSI_SLAVE_GetStatus
};
#endif

// SSI_ULP_MASTER
#ifdef SSI_ULP_MASTER
static int32_t SSI_ULP_MASTER_Initialize (ARM_SPI_SignalEvent_t pSignalEvent)
{
		/*power up the ssi peripheral*/
		RSI_PS_UlpssPeriPowerUp(ULPSS_PWRGATE_ULP_SSI);
#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_Initialize (pSignalEvent, &SSI_ULP_MASTER_Resources,
	                &UDMA1_Resources,UDMA1_Table,&udmaHandle1,dma_rom_buff1);
#else
return SPI_Initialize (pSignalEvent, &SSI_ULP_MASTER_Resources,
	                &UDMA1_Resources,UDMA1_Table,&udmaHandle1,dma_rom_buff1);
#endif
}
static int32_t SSI_ULP_MASTER_Uninitialize(void)
{ 
	/*power down the ulp ssi peripheral*/
	RSI_PS_UlpssPeriPowerDown(ULPSS_PWRGATE_ULP_SSI);
#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_Uninitialize (&SSI_ULP_MASTER_Resources,&UDMA1_Resources);
#else
	return SPI_Uninitialize (&SSI_ULP_MASTER_Resources,&UDMA1_Resources);
	#endif
}
static int32_t        SSI_ULP_MASTER_PowerControl (ARM_POWER_STATE state)
{
#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_PowerControl (state, &SSI_ULP_MASTER_Resources);
#else
return SPI_PowerControl (state, &SSI_ULP_MASTER_Resources);
	#endif
}
static int32_t SSI_ULP_MASTER_Send (const void *data, uint32_t num)
 { 
#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_Send (data, num, &SSI_ULP_MASTER_Resources,
		              &UDMA1_Resources,udma1_chnl_info,udmaHandle1); 
#else
	 return SPI_Send (data, num, &SSI_ULP_MASTER_Resources,
		              &UDMA1_Resources,udma1_chnl_info,udmaHandle1); 
	#endif 
 }
static int32_t SSI_ULP_MASTER_Receive (void *data, uint32_t num)
{
#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_Receive (data, num, &SSI_ULP_MASTER_Resources,
	                &UDMA1_Resources,udma1_chnl_info,udmaHandle1); 
#else
 return SPI_Receive (data, num, &SSI_ULP_MASTER_Resources,
	                &UDMA1_Resources,udma1_chnl_info,udmaHandle1); 
	#endif
}
static int32_t SSI_ULP_MASTER_Transfer (const void *data_out, void *data_in, uint32_t num)
 {
 			#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_Transfer (data_out, data_in, num, &SSI_ULP_MASTER_Resources,
		                &UDMA1_Resources,udma1_chnl_info,udmaHandle1);
#else
  return SPI_Transfer (data_out, data_in, num, &SSI_ULP_MASTER_Resources,
		                &UDMA1_Resources,udma1_chnl_info,udmaHandle1); 
	 #endif
 }
static uint32_t       SSI_ULP_MASTER_GetDataCount        (void)
{
			#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
       return ROMAPI_SSI_API->SPI_GetDataCount (&SSI_ULP_MASTER_Resources);
#else
return SPI_GetDataCount (&SSI_ULP_MASTER_Resources);
	#endif
}
static int32_t        SSI_ULP_MASTER_Control(uint32_t control, uint32_t arg)
{ 
 //!ROM bypass
 uint32_t ssi_get_clock=0;
  int32_t status         = 0;
 ssi_get_clock = RSI_CLK_GetBaseClock(ULPSS_SSI);
	#ifdef SPI_MULTI_SLAVE
  if((control & ARM_SPI_CONTROL_Msk )== ARM_SPI_CONTROL_SS )
	 {
	  spi_slavenumber =RSI_SPI_GetSlaveSelectNumber();
	/* check slave number is valied or not */
		if(spi_slavenumber == 0xA5)
		{
			/* Update the slave number by using RSI_SPI_SetSlaveSelectNumber API */
		return RSI_MUTLI_SLAVE_SELECT_ERROR; 
		}
	}
	 #endif
	#if (defined(SSI_ROMDRIVER_PRESENT))
  #if (defined(A11_ROM) && (defined(SLI_SI917)) && defined(SLI_SI917B0))
  return ROMAPI_SSI_API->SPI_Control(control, arg, &SSI_ULP_MASTER_Resources,ssi_get_clock,spi_slavenumber);
  #else
  return ROMAPI_SSI_API->SPI_Control(control, arg, &SSI_ULP_MASTER_Resources,ssi_get_clock);
  #endif
  #else
  status = SPI_Control(control, arg, &SSI_ULP_MASTER_Resources, ssi_get_clock, spi_slavenumber);
  #endif
  return status;
}
static ARM_SPI_STATUS SSI_ULP_MASTER_GetStatus           (void)
{
      #if ((defined(A11_ROM)) &&( defined(SSI_ROMDRIVER_PRESENT)) &&( defined(SLI_SI917)))
       return ROMAPI_SSI_API->SPI_GetStatus (&SSI_ULP_MASTER_Resources);
#else
return SPI_GetStatus (&SSI_ULP_MASTER_Resources);
	#endif
}
void SSI_ULP_MASTER_IRQHandler (void)
{
#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
        ROMAPI_SSI_API->SPI_IRQHandler (&SSI_ULP_MASTER_Resources);
#else
SPI_IRQHandler (&SSI_ULP_MASTER_Resources);
	#endif
}
#if defined(SSI_ULP_MASTER_TX_DMA_Instance) && (SSI_ULP_MASTER_TX_DMA_Instance == 1)
void          SSI_ULP_MASTER_UDMA_Tx_Event     (uint32_t event, uint32_t dmaCh)
{
			#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
        ROMAPI_SSI_API->SPI_UDMA_Tx_Event (event,(uint8_t)dmaCh, &SSI_ULP_MASTER_Resources);
#else
SPI_UDMA_Tx_Event (event,(uint8_t)dmaCh, &SSI_ULP_MASTER_Resources);
  #endif
}
#endif

#if defined(SSI_ULP_MASTER_RX_DMA_Instance) && (SSI_ULP_MASTER_RX_DMA_Instance == 1)
void SSI_ULP_MASTER_UDMA_Rx_Event (uint32_t event, uint32_t dmaCh)
{
			#if ( (defined(A11_ROM)) && (defined(SSI_ROMDRIVER_PRESENT)) && (defined(SLI_SI917)) )
        ROMAPI_SSI_API->SPI_UDMA_Rx_Event (event,(uint8_t)dmaCh, &SSI_ULP_MASTER_Resources);
#else
SPI_UDMA_Rx_Event (event,(uint8_t)dmaCh, &SSI_ULP_MASTER_Resources);
  #endif
}
#endif

// End SPI Interface

ARM_DRIVER_SPI Driver_SSI_ULP_MASTER = { SPI_GetVersion,
                                         SPI_GetCapabilities,
                                         SSI_ULP_MASTER_Initialize,
                                         SSI_ULP_MASTER_Uninitialize,
                                         SSI_ULP_MASTER_PowerControl,
                                         SSI_ULP_MASTER_Send,
                                         SSI_ULP_MASTER_Receive,
                                         SSI_ULP_MASTER_Transfer,
                                         SSI_ULP_MASTER_GetDataCount,
                                         SSI_ULP_MASTER_Control,
                                         SSI_ULP_MASTER_GetStatus };
#endif

// Set the value for receive sample delay
void SSI_SetRxSamplingDelay(uint8_t ssi_instance, uint32_t sample_delay)
{
  do {
    if (ssi_instance == SPI_MASTER_MODE) {
      SSI_MASTER_Resources.reg->RX_SAMPLE_DLY_b.RSD = sample_delay;
      break;
    }
    if (ssi_instance == SPI_SLAVE_MODE) {
      SSI_SLAVE_Resources.reg->RX_SAMPLE_DLY_b.RSD = sample_delay;
      break;
    }
    if (ssi_instance == SPI_ULP_MASTER_MODE) {
      SSI_ULP_MASTER_Resources.reg->RX_SAMPLE_DLY_b.RSD = sample_delay;
      break;
    }
  } while (false);
}

// To get the receive sample delay value.
uint32_t SSI_GetReceiveSampleDelay(uint8_t ssi_instance)
{
  uint32_t rx_sample_delay = 0;
  do {
    if (ssi_instance == SPI_MASTER_MODE) {
        rx_sample_delay = SSI_MASTER_Resources.reg->RX_SAMPLE_DLY_b.RSD;
      break;
    }
    if (ssi_instance == SPI_SLAVE_MODE) {
        rx_sample_delay = SSI_SLAVE_Resources.reg->RX_SAMPLE_DLY_b.RSD;
      break;
    }
    if (ssi_instance == SPI_ULP_MASTER_MODE) {
        rx_sample_delay = SSI_ULP_MASTER_Resources.reg->RX_SAMPLE_DLY_b.RSD;
      break;
    }
  } while (false);
  return (rx_sample_delay & 0x3F);
}
// Update Tx and Rx fifo threshold value
void SSI_SetFifoThreshold(uint8_t ssi_instance)
{
  do {
    if (ssi_instance == SPI_MASTER_MODE) {
      // Configuring DMA thresholds based on SSI instances.
#if defined SL_SSI_PRIMARY_DMA_CONFIG_ENABLE
#ifdef SSI_INSTANCE_CONFIG
      SSI_MASTER_Resources.reg->TXFTLR_b.TFT = SL_SSI_PRIMARY_TX_FIFO_THRESHOLD;
      SSI_MASTER_Resources.reg->RXFTLR_b.RFT = SL_SSI_PRIMARY_RX_FIFO_THRESHOLD;
#endif
#elif defined SL_SSI_MASTER_DMA_CONFIG_ENABLE
// DMA threshold configuration based on SSI unknown instance name.
#ifdef SSI_CONFIG
      SSI_MASTER_Resources.reg->TXFTLR_b.TFT = SL_SSI_TX_FIFO_THRESHOLD;
      SSI_MASTER_Resources.reg->RXFTLR_b.RFT = SL_SSI_RX_FIFO_THRESHOLD;
#endif
#endif
      break;
    }
    if (ssi_instance == SPI_SLAVE_MODE) {
      // Configuring DMA thresholds based on SSI instances.
#if defined SL_SSI_SECONDARY_DMA_CONFIG_ENABLE
#ifdef SSI_INSTANCE_CONFIG
      SSI_SLAVE_Resources.reg->TXFTLR_b.TFT = SL_SSI_SECONDARY_TX_FIFO_THRESHOLD;
      SSI_SLAVE_Resources.reg->RXFTLR_b.RFT = SL_SSI_SECONDARY_RX_FIFO_THRESHOLD;
#endif
#elif defined SL_SSI_SLAVE_DMA_CONFIG_ENABLE
// DMA threshold configuration based on SSI unknown instance name.
#ifdef SSI_CONFIG
      SSI_SLAVE_Resources.reg->TXFTLR_b.TFT = SL_SSI_TX_FIFO_THRESHOLD;
      SSI_SLAVE_Resources.reg->RXFTLR_b.RFT = SL_SSI_RX_FIFO_THRESHOLD;
#endif
#endif
      break;
    }
    if (ssi_instance == SPI_ULP_MASTER_MODE) {
      // Configuring DMA thresholds based on SSI instances.
#if defined SL_SSI_ULP_PRIMARY_DMA_CONFIG_ENABLE
#ifdef SSI_INSTANCE_CONFIG
      SSI_ULP_MASTER_Resources.reg->TXFTLR_b.TFT = SL_SSI_ULP_PRIMARY_TX_FIFO_THRESHOLD;
      SSI_ULP_MASTER_Resources.reg->RXFTLR_b.RFT = SL_SSI_ULP_PRIMARY_RX_FIFO_THRESHOLD;
#endif
#elif defined SL_SSI_ULP_MASTER_DMA_CONFIG_ENABLE
// DMA threshold configuration based on SSI unknown instance name.
#ifdef SSI_CONFIG
        SSI_ULP_MASTER_Resources.reg->TXFTLR_b.TFT = SL_SSI_TX_FIFO_THRESHOLD;
        SSI_ULP_MASTER_Resources.reg->RXFTLR_b.RFT = SL_SSI_RX_FIFO_THRESHOLD;
#endif
#endif
      break;
    }
  } while (false);
}

#ifdef SSI_DUAL_QUAD_COMPONENT
// Configure and send an SSI command for the master instance.
int32_t SSI_Command_Configure(uint32_t inst_len, uint32_t addr_len, sl_ssi_frf_t spi_frf, sl_ssi_xfer_type_t xfer_type)
{
  // Configure the command phase of the SPI transaction
  return SPI_Command_Configure(&SSI_MASTER_Resources, inst_len, addr_len, spi_frf, xfer_type);
}
#endif

// To get the Tx fifo threshold value
uint32_t SSI_GetTxFifoThreshold(uint8_t ssi_instance)
{
  uint32_t tx_fifo_threshold = 0;
  do {
    if (ssi_instance == SPI_MASTER_MODE) {
      tx_fifo_threshold = SSI_MASTER_Resources.reg->TXFTLR_b.TFT;
      break;
    }
    if (ssi_instance == SPI_SLAVE_MODE) {
      tx_fifo_threshold = SSI_SLAVE_Resources.reg->TXFTLR_b.TFT;
      break;
    }
    if (ssi_instance == SPI_ULP_MASTER_MODE) {
      tx_fifo_threshold = SSI_ULP_MASTER_Resources.reg->TXFTLR_b.TFT;
      break;
    }
  } while (false);
  return (tx_fifo_threshold & 0xF);
}
// To get the Rx fifo threshold value
uint32_t SSI_GetRxFifoThreshold(uint8_t ssi_instance)
{
  uint32_t rx_fifo_threshold = 0;
  do {
    if (ssi_instance == SPI_MASTER_MODE) {
      rx_fifo_threshold = SSI_MASTER_Resources.reg->RXFTLR_b.RFT;
      break;
    }
    if (ssi_instance == SPI_SLAVE_MODE) {
      rx_fifo_threshold = SSI_SLAVE_Resources.reg->RXFTLR_b.RFT;
      break;
    }
    if (ssi_instance == SPI_ULP_MASTER_MODE) {
      rx_fifo_threshold = SSI_ULP_MASTER_Resources.reg->RXFTLR_b.RFT;
      break;
    }
  } while (false);
  return (rx_fifo_threshold & 0xF);
}

// To get the clock division factor
uint32_t SSI_GetClockDivisionFactor(uint8_t ssi_instance)
{
  uint32_t division_factor = 0;
  do {
    if (ssi_instance == SPI_MASTER_MODE) {
      division_factor = SSI_MASTER_Resources.reg->BAUDR_b.SCKDV;
      break;
    }
    if (ssi_instance == SPI_SLAVE_MODE) {
      division_factor = SSI_SLAVE_Resources.reg->BAUDR_b.SCKDV;
      break;
    }
    if (ssi_instance == SPI_ULP_MASTER_MODE) {
      division_factor = SSI_ULP_MASTER_Resources.reg->BAUDR_b.SCKDV;
      break;
    }
  } while (false);
  return division_factor;
}

// To get the frame length
uint32_t SSI_GetFrameLength(uint8_t ssi_instance)
{
  uint32_t frame_length = 0;
  do {
    if (ssi_instance == SPI_MASTER_MODE) {
      frame_length = SSI_MASTER_Resources.reg->CTRLR0_b.DFS_32;
      break;
    }
    if (ssi_instance == SPI_SLAVE_MODE) {
      frame_length = SSI_SLAVE_Resources.reg->CTRLR0_b.DFS;
      break;
    }
    if (ssi_instance == SPI_ULP_MASTER_MODE) {
      frame_length = SSI_ULP_MASTER_Resources.reg->CTRLR0_b.DFS_32;
      break;
    }
  } while (false);
  /* Adding length offset because while setting the value to the register, 
     1 is substracted from the original value, so while returning adding 1 to the register value*/ 
  return frame_length + LENGTH_OFFSET;
}

// To set the frame length
int32_t SSI_SetFrameLength(uint8_t ssi_instance, uint8_t frame_length)
{
  int32_t status = ARM_DRIVER_OK;

  do {
    if (frame_length < 4U || frame_length > 32U) {
      status = ARM_SPI_ERROR_DATA_BITS;
      break;
    }
    if (ssi_instance == SPI_MASTER_MODE) {
      // For Dual mode, the frame length should be a multiple of 2.
      if((SSI_MASTER_Resources.reg->CTRLR0_b.SPI_FRF == DUAL_SPI_FORMAT) && ((frame_length % 2) != 0)) {
        status = ARM_SPI_ERROR_DATA_BITS;
        break;
      }
      // For Quad mode, the frame length should be a multiple of 4.
      else if ((SSI_MASTER_Resources.reg->CTRLR0_b.SPI_FRF == QUAD_SPI_FORMAT) && ((frame_length % 4) != 0)) {
        status = ARM_SPI_ERROR_DATA_BITS;
        break;
      }
      SSI_MASTER_Resources.reg->CTRLR0_b.DFS_32 = (frame_length - LENGTH_OFFSET);
      break;
    }
    if (ssi_instance == SPI_SLAVE_MODE) {
      if (frame_length < 4U || frame_length > 16U) {
        status = ARM_SPI_ERROR_DATA_BITS;
        break;
      }
      SSI_SLAVE_Resources.reg->CTRLR0_b.DFS = (frame_length - LENGTH_OFFSET);
      break;
    }
    if (ssi_instance == SPI_ULP_MASTER_MODE) {
      SSI_ULP_MASTER_Resources.reg->CTRLR0_b.DFS_32 = (frame_length - LENGTH_OFFSET);
      break;
    }
  } while (false);
  return status;
}

// Get SSI Init state
uint8_t SSI_GetInitState(uint8_t ssi_instance)
{
  uint8_t init_state = false;
  do {
    if (ssi_instance == SPI_MASTER_MODE) {
      init_state = (SSI_MASTER_Resources.info->state & SPI_INITIALIZED);
      break;
    }
    if (ssi_instance == SPI_SLAVE_MODE) {
      init_state = (SSI_SLAVE_Resources.info->state & SPI_INITIALIZED);
      break;
    }
    if (ssi_instance == SPI_ULP_MASTER_MODE) {
      init_state = (SSI_ULP_MASTER_Resources.info->state & SPI_INITIALIZED);
      break;
    }
  } while (false);
  return init_state;
}

//To get the transfer count
uint32_t SSI_GetTxCount(uint8_t ssi_instance)
{
  uint32_t count = 0;
  do {
    if (ssi_instance == SPI_MASTER_MODE) {
      if (!(SSI_MASTER_Resources.info->state & SPI_CONFIGURED)) {
        count = 0U;
        break;
      }
      count = SSI_MASTER_Resources.xfer->tx_cnt;
      break;
    }
    if (ssi_instance == SPI_SLAVE_MODE) {
      if (!(SSI_SLAVE_Resources.info->state & SPI_CONFIGURED)) {
        count = 0U;
        break;
      }
      count = SSI_SLAVE_Resources.xfer->tx_cnt;
      break;
    }
    if (ssi_instance == SPI_ULP_MASTER_MODE) {
      if (!(SSI_ULP_MASTER_Resources.info->state & SPI_CONFIGURED)) {
        count = 0U;
        break;
      }
      count = SSI_ULP_MASTER_Resources.xfer->tx_cnt;
    }
  } while (false);
  return count;
}

#ifdef SL_SI91X_SSI_DMA
/*******************************************************************************
 * Transfer complete callback function which is registered by SL_DMA driver
 * for GSPI peripheral-memory and memory-peripheral transfers. This function calls
 * GSPI Rx and Tx transfer complete event for respective channels.
 * *****************************************************************************/
void ssi_transfer_complete_callback(uint32_t channel, void *data)
{
  (void)(&data);
#if defined(SSI_MASTER_TX_DMA_Instance) && (SSI_MASTER_TX_DMA_Instance == 1)
  if (channel == SSI_MASTER_TX_DMA_Channel) {
    SSI_MASTER_UDMA_Tx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel);
  }
  if (channel == SSI_MASTER_RX_DMA_Channel) {
    SSI_MASTER_UDMA_Rx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel);
  }
#endif
#if defined(SSI_SLAVE_TX_DMA_Instance) && (SSI_SLAVE_TX_DMA_Instance == 1)
  if (channel == SSI_SLAVE_TX_DMA_Channel) {
    SSI_SLAVE_UDMA_Tx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel);
  }
  if (channel == SSI_SLAVE_RX_DMA_Channel) {
    SSI_SLAVE_UDMA_Rx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel);
  }
#endif
#if defined(SSI_ULP_MASTER_RX_DMA_Instance) && (SSI_ULP_MASTER_RX_DMA_Instance == 1)
  if (channel == SSI_ULP_MASTER_TX_DMA_Channel) {
    SSI_ULP_MASTER_UDMA_Tx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel);
  }
  if (channel == SSI_ULP_MASTER_RX_DMA_Channel) {
    SSI_ULP_MASTER_UDMA_Rx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel);
  }
#endif
	(void)(&channel);
}

/*******************************************************************************
 * Error callback function which is registered by SL_DMA driver for GSPI
 * peripheral-memory and memory-peripheral transfers. This function calls
 * GSPI Rx and Tx error event for respective channels.
 * *****************************************************************************/
void ssi_error_callback(uint32_t channel, void *data)
{
  (void)(&data);
#if defined(SSI_MASTER_TX_DMA_Instance) && (SSI_MASTER_TX_DMA_Instance == 1)
  if (channel == SSI_MASTER_TX_DMA_Channel) {
    SSI_MASTER_UDMA_Tx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel);
  }
  if (channel == SSI_MASTER_RX_DMA_Channel) {
    SSI_MASTER_UDMA_Rx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel);
  }
#endif
#if defined(SSI_SLAVE_TX_DMA_Instance) && (SSI_SLAVE_TX_DMA_Instance == 1)
  if (channel == SSI_SLAVE_TX_DMA_Channel) {
    SSI_SLAVE_UDMA_Tx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel);
  }
  if (channel == SSI_SLAVE_RX_DMA_Channel) {
    SSI_SLAVE_UDMA_Rx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel);
  }
#endif
#if defined(SSI_ULP_MASTER_RX_DMA_Instance) && (SSI_ULP_MASTER_RX_DMA_Instance == 1)
  if (channel == SSI_ULP_MASTER_TX_DMA_Channel) {
    SSI_ULP_MASTER_UDMA_Tx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel);
  }
  if (channel == SSI_ULP_MASTER_RX_DMA_Channel) {
    SSI_ULP_MASTER_UDMA_Rx_Event(UDMA_EVENT_XFER_DONE, (uint8_t)channel);
  }
#endif
(void)(&channel);
}
#endif /* SL_SI91X_GSPI_DMA */
