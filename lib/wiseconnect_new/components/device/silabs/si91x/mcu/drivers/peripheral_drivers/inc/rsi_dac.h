/******************************************************************************
* @file  rsi_dac.h
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

// Includes Files

#ifndef RSI_AUX_DAC_H
#define RSI_AUX_DAC_H

#include "rsi_error.h"
#include "si91x_device.h"
#include "rsi_udma.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DIV_FAC_DAC      1023
#define MAX_DAC_FIFO_THR     7
#define MAX_DAC_SAMP_RATE    5000000
#define AUXDACREG0           0x11A
#define DAC_FIFO_THR         3 // Set FIFO threshold as 3
#define STATIC_MODE_EN       1
#define FIFO_MODE_EN         0
#define DAC_OUT_AGPIO15      15
#define DAC_OUT_AGPIO4       4
#define DAC_STATIC_MODE_INTR BIT(24)

// brief DAC channel Configuration structure
typedef enum { DAC_POWER_ON = 0, DAC_POWER_OFF } POWER_STATE_DAC;

// Call back handler
typedef void (*daccallbacFunc)(uint8_t event); // Call back function pointer

typedef struct {
  daccallbacFunc callback_event;
} dac_config_t;

// Call back event
#define DAC_UDMA_PING_PONG_CONFIG 1
#define DAC_STATIC_MODE_CALLBACK  2
#define DAC_INPUT_REG_ADDR        0x24043810

// Define bit positions for ADC and DAC
#define ADC_BIT_POS   0
#define DAC_BIT_POS   1
#define COMP_BIT_POS  2
#define OPAMP_BIT_POS 3

typedef union {
  uint8_t analog_power;

  struct {
    uint8_t power_en_adc : 1;
    uint8_t power_en_dac : 1;
    uint8_t power_en_comp : 1;
    uint8_t power_en_opamp : 1;
  } ap;
} analog_power_control_t;

extern analog_power_control_t analog_power_ctrl;

// Define macros for analog power states
#define ANALOG_POWERED_ON  1
#define ANALOG_POWERED_OFF 0

// Set power state
__STATIC_INLINE void analog_set_power_state(uint8_t bit_pos, bool power_en_state)
{
  switch (bit_pos) {
    case ADC_BIT_POS:
      analog_power_ctrl.ap.power_en_adc = power_en_state;
      break;
    case DAC_BIT_POS:
      analog_power_ctrl.ap.power_en_dac = power_en_state;
      break;
    case COMP_BIT_POS:
      analog_power_ctrl.ap.power_en_comp = power_en_state;
      break;
    case OPAMP_BIT_POS:
      analog_power_ctrl.ap.power_en_opamp = power_en_state;
      break;
    default:
      // Handle invalid bit position
      break;
  }
}

// Get power state
__STATIC_INLINE uint8_t analog_get_power_state(void)
{
  return analog_power_ctrl.analog_power;
}

__STATIC_INLINE float adc_dac_ceil(float x)
{
  int val_int = (int)x;

  return (x > (float)val_int) ? (float)(val_int + 1) : (float)val_int;
}

uint32_t DAC_Init(uint8_t operation_mode, uint32_t sampling_rate, daccallbacFunc event);

rsi_error_t DAC_WriteData_StaticMode(int16_t input_data);

rsi_error_t DAC_WriteData(uint8_t operation_mode, int16_t *wr_buf, uint16_t length);

rsi_error_t DAC_Start(uint8_t operation_mode);

rsi_error_t DAC_Stop(void);

rsi_error_t DAC_Deinit(void);

rsi_error_t DAC_PingPongReconfig(int16_t *wr_buf, uint16_t length);

// New internal API
rsi_error_t DAC_PinMux(uint8_t pin_sel);
#ifdef SL_SI91X_DAC
rsi_error_t DAC_PinMux_config(uint8_t pin, uint8_t port);
#endif

uint32_t dac_set_clock(uint32_t sampl_rate);

rsi_error_t RSI_DAC_ClkDivFactor(AUX_ADC_DAC_COMP_Type *pstcDAC, uint16_t div_factor);

void RSI_DAC_PowerControl(POWER_STATE_DAC state);

rsi_error_t RSI_DAC_Config(AUX_ADC_DAC_COMP_Type *pstcDAC,
                           uint8_t static_fifo_mode,
                           uint16_t aux_dac_out_mux_en,
                           uint16_t aux_dac_out_mux_sel,
                           uint8_t prbs_sel);

rsi_error_t RSI_DAC_WriteData(AUX_ADC_DAC_COMP_Type *pstcDAC,
                              const int16_t *data,
                              uint8_t static_fifo_mode,
                              uint16_t len);

rsi_error_t RSI_DAC_DynamicModeWriteData(AUX_ADC_DAC_COMP_Type *pstcDAC,
                                         uint16_t channel,
                                         const uint16_t *data,
                                         uint32_t len);

uint16_t RSI_DAC_DyanamicModeReadData(AUX_ADC_DAC_COMP_Type *pstcDAC, uint32_t channel, uint16_t data);

rsi_error_t RSI_DAC_Stop(AUX_ADC_DAC_COMP_Type *pstcDAC);

rsi_error_t RSI_DAC_Start(AUX_ADC_DAC_COMP_Type *pstcDAC, uint16_t aux_dac_en);

rsi_error_t RSI_DAC_DynamicModeConfig(AUX_ADC_DAC_COMP_Type *pstcDAC,
                                      uint16_t channel_no,
                                      uint16_t aux_dac_out_mux_en,
                                      uint16_t aux_dac_out_mux_sel);

rsi_error_t RSI_DAC_DynamicModeStart(AUX_ADC_DAC_COMP_Type *pstcDAC, uint32_t channel, uint16_t aux_dac_en);

rsi_error_t RSI_DAC_SetFifoThreshold(AUX_ADC_DAC_COMP_Type *pstcDAC, uint32_t fifo_threshold);

uint16_t RSI_DAC_ReadData(const AUX_ADC_DAC_COMP_Type *pstcDAC);

#ifdef CHIP_9118
rsi_error_t RSI_DAC_InterruptUnMask(AUX_ADC_DAC_COMP_Type *pstcDAC);
rsi_error_t RSI_DAC_InterruptMask(AUX_ADC_DAC_COMP_Type *pstcDAC);
#endif
#if defined(SLI_SI917)
rsi_error_t RSI_DAC_InterruptUnMask(AUX_ADC_DAC_COMP_Type *pstcDAC, uint8_t oper_mode);
rsi_error_t RSI_DAC_InterruptMask(AUX_ADC_DAC_COMP_Type *pstcDAC, uint8_t oper_mode);
#endif

rsi_error_t RSI_DAC_InterruptClr(const AUX_ADC_DAC_COMP_Type *pstcDAC);

void dac_udma_init(void);

void dac_udma_write(uint8_t ping_pong_write, uint16_t num_of_samples, int16_t *input_buff, uint8_t skip_flag);

void dac_udma_start(void);
void dac_udma_stop(void);
uint16_t RSI_DAC_DynamicModeReadData(const AUX_ADC_DAC_COMP_Type *pstcDAC, uint32_t channel, uint16_t data);
rsi_error_t RSI_DAC_DynamicModeStop(AUX_ADC_DAC_COMP_Type *pstcDAC, uint32_t channel);
void dac_udmaTransferComplete(RSI_UDMA_HANDLE_T udmaHandle, RSI_UDMA_DESC_T *pTranDesc, uint32_t channel_no);
void UDMA_DAC_Ping_Write(uint16_t num_of_samples, int16_t *input_buff, uint8_t pingreconfig);
void UDMA_DAC_Pong_Write(uint16_t num_of_samples, int16_t *input_buff, uint8_t pongreconfig);

#ifdef __cplusplus
}
#endif

#endif // RSI_AUX_DAC_H
