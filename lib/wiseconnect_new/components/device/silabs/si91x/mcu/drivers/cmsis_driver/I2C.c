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

#include "I2C.h"
#include "rsi_i2c.h"

#if defined(A11_ROM)
#include "rsi_rom_table_si91x.h" 
#endif

#define ARM_I2C_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(2, 0) /* driver version */
#if ((defined(RTE_Drivers_I2C0) || \
		defined(RTE_Drivers_I2C1) || \
		defined(RTE_Drivers_I2C2))   \
		&& !RTE_I2C0                  \
		&& !RTE_I2C1                  \
		&& !RTE_I2C2)

#error "I2C not configured in RTE_Device_917.h!"
#endif
/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
		ARM_I2C_API_VERSION,
		ARM_I2C_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_I2C_CAPABILITIES DriverCapabilities = {
		1  /* supports 10-bit addressing */
};

#if RTE_I2C0 
static  I2C_PIN i2c0_scl = { RTE_I2C0_SCL_PORT ,RTE_I2C0_SCL_PIN ,RTE_I2C0_SCL_MUX ,RTE_I2C0_SCL_PAD };
static  I2C_PIN i2c0_sda = { RTE_I2C0_SDA_PORT ,RTE_I2C0_SDA_PIN ,RTE_I2C0_SDA_MUX ,RTE_I2C0_SDA_PAD };

/* I2C0 Control Information */
static I2C_CTRL I2C0_Ctrl = { 0 };

/* I2C0 Resources */
static I2C_RESOURCES I2C0_Resources = {
		(I2C0_Type *)I2C0,
		I2C0_IRQn,
		&I2C0_Ctrl,
		{
		&i2c0_scl, //pins
		&i2c0_sda  },
};
#endif

#if (RTE_I2C1)
static  I2C_PIN i2c1_scl = { RTE_I2C1_SCL_PORT ,RTE_I2C1_SCL_PIN ,RTE_I2C1_SCL_MUX ,RTE_I2C1_SCL_PAD };
static  I2C_PIN i2c1_sda = { RTE_I2C1_SDA_PORT ,RTE_I2C1_SDA_PIN ,RTE_I2C1_SDA_MUX ,RTE_I2C1_SDA_PAD };

/* I2C1 Control Information */
I2C_CTRL I2C1_Ctrl = { 0 };

/* I2C1 Resources */
static I2C_RESOURCES I2C1_Resources = {
		(I2C0_Type *)I2C1,
		I2C1_IRQn,
		&I2C1_Ctrl ,
		{
		&i2c1_scl,
		&i2c1_sda },
};
#endif /* RTE_I2C1 */

#if (RTE_I2C2)
#ifdef SL_CATALOG_I2C_PRESENT
static  I2C_PIN i2c2_scl = { RTE_I2C2_SCL_PORT ,RTE_I2C2_SCL_PIN ,RTE_I2C2_SCL_MUX ,0 };
static  I2C_PIN i2c2_sda = { RTE_I2C2_SDA_PORT ,RTE_I2C2_SDA_PIN ,RTE_I2C2_SDA_MUX ,0 };
#else
#define SL_I2C_SCL_PIN ((RTE_I2C2_SCL_PIN > GPIO_MAX_PIN) ? (RTE_I2C2_SCL_PIN - GPIO_MAX_PIN) : RTE_I2C2_SCL_PIN)
#define SL_I2C_SDA_PIN ((RTE_I2C2_SDA_PIN > GPIO_MAX_PIN) ? (RTE_I2C2_SDA_PIN - GPIO_MAX_PIN) : RTE_I2C2_SDA_PIN)
static  I2C_PIN i2c2_scl = { RTE_I2C2_SCL_PORT ,SL_I2C_SCL_PIN ,RTE_I2C2_SCL_MUX ,0 };
static  I2C_PIN i2c2_sda = { RTE_I2C2_SDA_PORT ,SL_I2C_SDA_PIN ,RTE_I2C2_SDA_MUX ,0 };
#endif

/* I2C2 Control Information */
static I2C_CTRL I2C2_Ctrl = { 0 };

/* I2C2 Resources */
static I2C_RESOURCES I2C2_Resources = {
		(I2C0_Type *)I2C2,
		I2C2_IRQn,
		&I2C2_Ctrl ,
		{
		&i2c2_scl,
		&i2c2_sda},
};
#endif /* RTE_I2C2 */


///////////////////////////////////////////////////////////////
//                         Functions                         //
///////////////////////////////////////////////////////////////

static ARM_DRIVER_VERSION I2C_GetVersion(void)
{
	return DriverVersion;
}

static ARM_I2C_CAPABILITIES I2C_GetCapabilities(void)
{
	return DriverCapabilities;
}

uint32_t GetI2CClockFreq (I2C_RESOURCES *i2c) 
{
  (void)i2c;
	return(SystemCoreClock);
} 
// End I2C Interface
#if (RTE_I2C0)
/* I2C0 Driver wrapper functions */
static int32_t I2C0_Initialize (ARM_I2C_SignalEvent_t cb_event) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_Initialize(cb_event, &I2C0_Resources);
#else
	return (I2Cx_Initialize(cb_event, &I2C0_Resources));  	
#endif	
}

static int32_t I2C0_Uninitialize (void) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_Uninitialize (&I2C0_Resources);
#else
	return (I2Cx_Uninitialize (&I2C0_Resources)); 
#endif
}

static int32_t I2C0_PowerControl (ARM_POWER_STATE state) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_PowerControl (state, &I2C0_Resources);
#else
	return (I2Cx_PowerControl (state, &I2C0_Resources)); 
#endif
}

static int32_t I2C0_MasterTransmit (uint32_t addr, const uint8_t *data, uint32_t num, bool xfer_pending) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_MasterTransmit (addr, data, num, xfer_pending, &I2C0_Resources);
#else
	return (I2Cx_MasterTransmit (addr, data, num, xfer_pending, &I2C0_Resources));
#endif
}

static int32_t I2C0_MasterReceive (uint32_t addr, uint8_t *data, uint32_t num, bool xfer_pending) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_MasterReceive (addr, data, num, xfer_pending, &I2C0_Resources);
#else
	return (I2Cx_MasterReceive (addr, data, num, xfer_pending, &I2C0_Resources));  	
#endif
}

static int32_t I2C0_SlaveTransmit (const uint8_t *data, uint32_t num) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_SlaveTransmit (data, num, &I2C0_Resources);
#else
	return (I2Cx_SlaveTransmit (data, num, &I2C0_Resources));  	
#endif
}

static int32_t I2C0_SlaveReceive (uint8_t *data, uint32_t num) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_SlaveReceive (data, num, &I2C0_Resources);
#else
	return (I2Cx_SlaveReceive (data, num, &I2C0_Resources));  	
#endif	
}
static int32_t I2C0_GetDataCount (void) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_GetDataCount (&I2C0_Resources);
#else
	return (I2Cx_GetDataCount (&I2C0_Resources));  	
#endif
}

static int32_t I2C0_Control (uint32_t control, uint32_t arg) 
{
	uint32_t clk_src = 0;
	clk_src = GetI2CClockFreq(&I2C0_Resources);	
#if  defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_Control (control, arg, &I2C0_Resources,clk_src);
#else
	return (I2Cx_Control (control, arg, &I2C0_Resources,clk_src));  	
#endif
}

static ARM_I2C_STATUS I2C0_GetStatus (void) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_GetStatus (&I2C0_Resources);
#else
	return (I2Cx_GetStatus (&I2C0_Resources));  	
#endif
}

void I2C0_IRQHandler (void) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	ROMAPI_I2C_API->I2Cx_IRQHandler (&I2C0_Resources);
#else
	I2Cx_IRQHandler (&I2C0_Resources);
#endif	
}

/* I2C0 Driver Control Block */
ARM_DRIVER_I2C Driver_I2C0 = {
		I2C_GetVersion,
		I2C_GetCapabilities,
		I2C0_Initialize,
		I2C0_Uninitialize,
		I2C0_PowerControl,
		I2C0_MasterTransmit,
		I2C0_MasterReceive,
		I2C0_SlaveTransmit,
		I2C0_SlaveReceive,
		I2C0_GetDataCount,
		I2C0_Control,
		I2C0_GetStatus
};
#endif

#if (RTE_I2C1)
/* I2C1 Driver wrapper functions */
static int32_t I2C1_Initialize (ARM_I2C_SignalEvent_t cb_event) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_Initialize (cb_event, &I2C1_Resources);
#else
	return (I2Cx_Initialize (cb_event, &I2C1_Resources));  	
#endif
}

static int32_t I2C1_Uninitialize (void) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_Uninitialize (&I2C1_Resources);
#else
	return (I2Cx_Uninitialize (&I2C1_Resources));  	
#endif
}

static int32_t I2C1_PowerControl (ARM_POWER_STATE state)
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_PowerControl (state, &I2C1_Resources);
#else
	return (I2Cx_PowerControl (state, &I2C1_Resources));  	
#endif
}
static int32_t I2C1_MasterTransmit (uint32_t addr, const uint8_t *data, uint32_t num, bool xfer_pending) 
{
#if  defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_MasterTransmit (addr, data, num, xfer_pending, &I2C1_Resources);
#else
	return (I2Cx_MasterTransmit (addr, data, num, xfer_pending, &I2C1_Resources));  	
#endif
}
static int32_t I2C1_MasterReceive (uint32_t addr, uint8_t *data, uint32_t num, bool xfer_pending) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_MasterReceive (addr, data, num, xfer_pending, &I2C1_Resources);
#else
	return (I2Cx_MasterReceive (addr, data, num, xfer_pending, &I2C1_Resources));  	
#endif
}
static int32_t I2C1_SlaveTransmit (const uint8_t *data, uint32_t num) 
{
#if  defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_SlaveTransmit (data, num, &I2C1_Resources);
#else
	return (I2Cx_SlaveTransmit (data, num, &I2C1_Resources));  	
#endif
}
static int32_t I2C1_SlaveReceive (uint8_t *data, uint32_t num) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_SlaveReceive (data, num, &I2C1_Resources);		
#else
	return (I2Cx_SlaveReceive (data, num, &I2C1_Resources));  	
#endif
}
static int32_t I2C1_GetDataCount (void) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_GetDataCount (&I2C1_Resources);
#else
	return (I2Cx_GetDataCount (&I2C1_Resources));  	
#endif
}
static int32_t I2C1_Control (uint32_t control, uint32_t arg) 
{
	uint32_t clk_src = 0;
	clk_src = GetI2CClockFreq(&I2C1_Resources);	
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_Control (control, arg, &I2C1_Resources,clk_src);
#else
	return (I2Cx_Control (control, arg, &I2C1_Resources,clk_src));  	
#endif
}
static ARM_I2C_STATUS I2C1_GetStatus (void) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_GetStatus (&I2C1_Resources);
#else
	return (I2Cx_GetStatus (&I2C1_Resources));  	
#endif
}
void I2C1_IRQHandler (void) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	ROMAPI_I2C_API->I2Cx_IRQHandler (&I2C1_Resources);
#else
	I2Cx_IRQHandler (&I2C1_Resources);
#endif
}

/* I2C1 Driver Control Block */
ARM_DRIVER_I2C Driver_I2C1 = {
		I2C_GetVersion,
		I2C_GetCapabilities,
		I2C1_Initialize,
		I2C1_Uninitialize,
		I2C1_PowerControl,
		I2C1_MasterTransmit,
		I2C1_MasterReceive,
		I2C1_SlaveTransmit,
		I2C1_SlaveReceive,
		I2C1_GetDataCount,
		I2C1_Control,
		I2C1_GetStatus
};
#endif

#if (RTE_I2C2)
/* I2C2 Driver wrapper functions */
static int32_t I2C2_Initialize (ARM_I2C_SignalEvent_t cb_event) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_Initialize (cb_event, &I2C2_Resources);
#else
	return (I2Cx_Initialize (cb_event, &I2C2_Resources));  	
#endif
}
static int32_t I2C2_Uninitialize (void) 
{
	RSI_PS_UlpssPeriPowerDown(ULPSS_PWRGATE_ULP_I2C);
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_Uninitialize (&I2C2_Resources);	
#else
	return (I2Cx_Uninitialize (&I2C2_Resources));  	
#endif
}
static int32_t I2C2_PowerControl (ARM_POWER_STATE state) 
{
#if  defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_PowerControl (state, &I2C2_Resources);
#else
	return (I2Cx_PowerControl (state, &I2C2_Resources));  	
#endif	
}
static int32_t I2C2_MasterTransmit (uint32_t addr, const uint8_t *data, uint32_t num, bool xfer_pending) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_MasterTransmit (addr, data, num, xfer_pending, &I2C2_Resources);
#else
	return (I2Cx_MasterTransmit (addr, data, num, xfer_pending, &I2C2_Resources));  	
#endif	
}
static int32_t I2C2_MasterReceive (uint32_t addr, uint8_t *data, uint32_t num, bool xfer_pending) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_MasterReceive (addr, data, num, xfer_pending, &I2C2_Resources);	
#else
	return (I2Cx_MasterReceive (addr, data, num, xfer_pending, &I2C2_Resources));  	
#endif	
}
static int32_t I2C2_SlaveTransmit (const uint8_t *data, uint32_t num) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_SlaveTransmit (data, num, &I2C2_Resources);	
#else
	return (I2Cx_SlaveTransmit (data, num, &I2C2_Resources));  	
#endif	
}
static int32_t I2C2_SlaveReceive (uint8_t *data, uint32_t num) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_SlaveReceive (data, num, &I2C2_Resources);
#else
	return (I2Cx_SlaveReceive (data, num, &I2C2_Resources));  	
#endif	
}
static int32_t I2C2_GetDataCount (void) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_GetDataCount (&I2C2_Resources);		
#else
	return (I2Cx_GetDataCount (&I2C2_Resources));  	
#endif	
}
static int32_t I2C2_Control (uint32_t control, uint32_t arg) 
{
	uint32_t clk_src = 0;
	clk_src = GetI2CClockFreq(&I2C2_Resources);	
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_Control (control, arg, &I2C2_Resources,clk_src);			
#else
	return (I2Cx_Control (control, arg, &I2C2_Resources,clk_src));  	
#endif	
}
static ARM_I2C_STATUS I2C2_GetStatus (void) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	return ROMAPI_I2C_API->I2Cx_GetStatus (&I2C2_Resources);
#else
	return (I2Cx_GetStatus (&I2C2_Resources)); 
#endif	
}

void I2C2_IRQHandler (void) 
{
#if defined(A11_ROM) && defined(ROMDRIVER_PRESENT)
	ROMAPI_I2C_API->I2Cx_IRQHandler (&I2C2_Resources);
#else
	I2Cx_IRQHandler (&I2C2_Resources);
#endif	
}

/* I2C1 Driver Control Block */
ARM_DRIVER_I2C Driver_I2C2 = {
		I2C_GetVersion,
		I2C_GetCapabilities,
		I2C2_Initialize,
		I2C2_Uninitialize,
		I2C2_PowerControl,
		I2C2_MasterTransmit,
		I2C2_MasterReceive,
		I2C2_SlaveTransmit,
		I2C2_SlaveReceive,
		I2C2_GetDataCount,
		I2C2_Control,
		I2C2_GetStatus
};
#endif
