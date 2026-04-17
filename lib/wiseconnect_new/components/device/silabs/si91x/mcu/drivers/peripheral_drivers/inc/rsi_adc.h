/******************************************************************************
* @file  rsi_adc.h
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

#include "rsi_ccp_common.h"
#include "base_types.h"

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif // UNUSED_PARAMETER

#ifndef RSI_AUX_ADC_H
#define RSI_AUX_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

//#define DONOT_READ_DATA_FROM_MEM            // Enable this macro for ADC power measurement
//#define  PS2_STATE_ADC_DATA_AQU_ENABLE
//#define  GAIN_OFFSET_CAL_EN                 // Enable the gain and offset calculation on ADC samples
//#define  MAX_SAMPLING_RATE
//#define  ADC_MULTICHANNEL_WITH_EXT_DMA

#define ADC_PING_MEMORY_ADDRESS 0x24060800 // Configure BANK1 and BANK2 for ADC operation
#define UDMA_PING_PONG_LENGTH   1024

#define ADC_FIFOMODE_ENABLE         0
#define ADC_STATICMODE_ENABLE       1
#define ADC_STATICMODE_THRESHOLD_EN 2
#define INTERNAL_DMA_EN             1
#define EXTERNAL_DMA_EN             2
#define DYNAMIC_MODE_EN             1
#define DYNAMIC_MODE_DI             0
#define AEMPTY_THRSHOLD             3
#define AFULL_THRSHOLD              3
#define ADC_PING_EN                 1
#define ADC_PONG_EN                 1
#define ADC_IRQ_Handler             IRQ011_Handler

// List of Mask Bits
#define POS0  0
#define POS1  1
#define POS2  2
#define POS3  3
#define POS4  4
#define POS5  5
#define POS6  6
#define POS7  7
#define POS8  8
#define POS9  9
#define POS10 10
#define POS11 11
#define POS12 12
#define POS13 13
#define POS14 14
#define POS15 15
#define POS16 16
#define POS17 17
#define POS18 18
#define POS19 19
#define POS20 20
#define POS21 21
#define POS22 22
#define POS23 23
#define POS24 24
#define POS25 25
#define POS26 26
#define POS27 27
#define POS28 28
#define POS29 29
#define POS30 30
#define POS31 31

#define ADC_CHNL0_INTR  0
#define ADC_CHNL1_INTR  1
#define ADC_CHNL2_INTR  2
#define ADC_CHNL3_INTR  3
#define ADC_CHNL4_INTR  4
#define ADC_CHNL5_INTR  5
#define ADC_CHNL6_INTR  6
#define ADC_CHNL7_INTR  7
#define ADC_CHNL8_INTR  8
#define ADC_CHNL9_INTR  9
#define ADC_CHNL10_INTR 10
#define ADC_CHNL11_INTR 11
#define ADC_CHNL12_INTR 12
#define ADC_CHNL13_INTR 13
#define ADC_CHNL14_INTR 14
#define ADC_CHNL15_INTR 15

#define AUXADCREG0          0x110
#define AUXADCREG1          0x111
#define AUXADCREG2          0x112
#define SPAREREG2           0x1C1
#define MULTI_CHANNEL_EVENT 1
#define FIFO_MODE_EVENT     0
#define MAX_CHNL_NO         15
#define PING_LEN_MAX        1023
#define PONG_LEN_MAX        1023
#define PING_ADDR_MAX       4294967295
#define PONG_ADDR_MAX       4294967295
#define MAX_THRS_VAL        7
#define MAX_POS_IN_SEL      31
#define MAX_NEG_IN_SEL      15
#define MAX_DIFF_MODE       1
#define MAX_TOL_DUR_TIME    1023
#define MAX_ON_DUR_TIME     511

#define MAX_SINGCH_SAMPLING_RATE 2500000
#define ADC_MASK_VALUE           0xF7FF
#define ADC_FIFO_THR             3
#define SDIO_HOST_CONTROL_IN_M4  0x41300004

#define ADCGPIO0      0
#define ADCGPIO1      1
#define ADCGPIO2      2
#define ADCGPIO3      3
#define ADCGPIO4      4
#define ADCGPIO5      5
#define ADCGPIO6      6
#define ADCGPIO7      7
#define ADCGPIO8      8
#define ADCGPIO9      9
#define ADCGPIO10     10
#define ADCGPIO11     11
#define ADCGPIO12     12
#define ADCGPIO13     13
#define ADCGPIO14     14
#define ADCGPIO15     15
#define ADC_TOPGPIO23 23
#define ADC_TOPGPIO24 24
#define ADC_TOPGPIO25 25
#define ADC_TOPGPIO26 26
#define ADC_TOPGPIO27 27
#define ADC_TOPGPIO28 28
#define ADC_TOPGPIO29 29
#define ADC_TOPGPIO30 30

// New Changes
#define ANALOG_MODE               7
#define TGPIO_ANALOG_MODE         14
#define ADC_OUTPUT_REG            0x24043814 // ADC output register
#define ADC_EXUDMA_PING_ADDR_MULT 2
#define ADC_EXUDMA_PONG_ADDR_MULT 0

#define INTERNAL_DMA               0
#define EXTERNAL_DMA_RECONFIG      1
#define MULTI_CHANNEL_DATA_OCCOURE 2
#define ADC_STATIC_MODE_CALLBACK   4
#define ADC_THRSHOLD_CALLBACK      5
#define ADC_DATA_AQU_BUF_PS4       0x9000
#define ADC_DATA_AQU_BUF_PS2       0x24060800

// Required delay with respect to HP Non-inverting OPAMP gain
#define GAIN2_DELAY_HP    (float)6.07 / (float)100000000
#define GAIN2_5_DELAY_HP  (float)1.31 / (float)10000000
#define GAIN3_DELAY_HP    (float)2.03 / (float)10000000
#define GAIN4_DELAY_HP    (float)4.40 / (float)10000000
#define GAIN5_DELAY_HP    (float)5.34 / (float)10000000
#define GAIN7_DELAY_HP    (float)8.46 / (float)10000000
#define GAIN8_DELAY_HP    (float)1.03 / (float)1000000
#define GAIN9_DELAY_HP    (float)1.25 / (float)1000000
#define GAIN12_5_DELAY_HP (float)2.04 / (float)1000000
#define GAIN17_DELAY_HP   (float)2.90 / (float)1000000
#define GAIN25_DELAY_HP   (float)4.70 / (float)1000000
#define GAIN51_DELAY_HP   (float)9.35 / (float)1000000

#define ULP_TIMER_EXT_TRIGGER  1
#define ULP_GPIO_EXT_TRIGGER   2
#define M4_CT_EXT_TRIGGER      3
#define DETECTION1             1
#define DETECTION2             2
#define DETECTION3             3
#define DETECTION4             4
#define EXT_TRIGGER_SEL1       1
#define EXT_TRIGGER_SEL2       2
#define EXT_TRIGGER_SEL3       3
#define EXT_TRIGGER_SEL4       4
#define EXT_TRIGGER_CH_ID0     0
#define EXT_TRIGGER_CH_ID1     1
#define EXT_TRIGGER_CH_ID2     2
#define EXT_TRIGGER_CH_ID3     3
#define EXT_TRIGGER_CH_ID4     4
#define EXT_TRIGGER_CH_ID5     5
#define EXT_TRIGGER_CH_ID6     6
#define EXT_TRIGGER_CH_ID7     7
#define EXT_TRIGGER_CH_ID8     8
#define EXT_TRIGGER_CH_ID9     9
#define EXT_TRIGGER_CH_ID10    10
#define EXT_TRIGGER_CH_ID11    11
#define EXT_TRIGGER_CH_ID12    12
#define EXT_TRIGGER_CH_ID13    13
#define EXT_TRIGGER_CH_ID14    14
#define EXT_TRIGGER_CH_ID15    15
#define NEGATIVE_EDGE          1
#define POSITIVE_EDGE          2
#define POS_NEG_EDGE           3
#define ADC_THRESHOLD_DET_INTR BIT(0)
#define ADC_STATIC_MODE_INTR   BIT(23)
#define THRESHOLD1_EQ          1
#define THRESHOLD1_GRT         2
#define THRESHOLD1_LESS        3
#define THRESHOLD2_EQ          1
#define THRESHOLD2_GRT         2
#define THRESHOLD2_LESS        3

typedef enum { ADC_POWER_ON = 0, ADC_POWER_OFF } POWER_STATE;

// brief ADC channel Configuration structure

typedef struct {
  uint8_t channel;
  uint8_t input_type[16];
  uint32_t sampling_rate[16];
  uint8_t pos_inp_sel[16];
  uint8_t neg_inp_sel[16];
  float opamp_gain[16];
  int16_t *rx_buf[16];         // Pointer to in data buffer
  uint16_t num_of_samples[16]; // Number of samples in 1 integration
  uint32_t chnl_ping_address[16];
  uint32_t chnl_pong_address[16];
} adc_ch_config_t;

typedef struct {
  uint8_t operation_mode;
  uint8_t num_of_channel_enable; // Number of channel enable for ADC operation
} adc_config_t;

typedef struct {
  uint32_t ping_addr[16];
  uint32_t pong_addr[16];
  uint16_t ping_length[16];
  uint16_t pong_length[16];
  uint32_t ping_mem1[16];
  uint32_t ping_mem2[16];
  uint32_t ping_mem3[16];
  uint32_t pong_mem1[16];
  uint32_t pong_mem2[16];
  uint32_t pong_mem3[16];
  uint16_t ch_offset_val[16];
  uint16_t ch_sampling_factor[16];
  uint32_t achived_sampling_rate[16];
  uint8_t input_type[16];
  int16_t *rx_buf[16];         // Pointer to in data buffer
  uint16_t num_of_samples[16]; // Number of samples in 1 integration
} adc_inter_config_t;

typedef void (*adccallbacFunc)(uint8_t channel, uint8_t event); // Call back function pointer

typedef struct {
  uint16_t on_clk;
  uint16_t total_clk;
  uint32_t adc_clk_src;
  uint16_t adc_sing_offset;
  float adc_sing_gain;
  uint16_t adc_diff_offset;
  float adc_diff_gain;
  adccallbacFunc call_back_event;
  uint8_t num_of_channel_enable; // Number of channel enable for ADC operation
} adc_commn_config_t;

typedef struct {
  uint16_t trigger_type;
  uint16_t trigger_num;
  uint16_t detection_edge_sel;
  uint16_t trigger_sel;
  uint16_t trigger_sel_val;
} adc_extr_config_t;

typedef struct {
  uint32_t ping_addr;
  uint32_t pong_addr;
  uint16_t ping_length;
  uint16_t pong_length;
  uint8_t ping_enable;
  uint8_t pong_enable;
} adc_ping_pong_memory_adr_config_t;

// Function Declarations
rsi_error_t ADC_Init(adc_ch_config_t adcChnfig, adc_config_t adcConfig, adccallbacFunc event);

rsi_error_t ADC_Per_Channel_Init(adc_ch_config_t adcChConfig, adc_config_t adcConfig, adccallbacFunc event);

rsi_error_t ADC_ChannelConfig(adc_ch_config_t adcChConfig, adc_config_t adcConfig);

rsi_error_t ADC_Per_ChannelConfig(adc_ch_config_t adcChConfig, adc_config_t adcConfig);

#ifdef CHIP_9118
rsi_error_t ADC_Start(void);

rsi_error_t ADC_Deinit(void);

rsi_error_t ADC_Stop(void);

rsi_error_t RSI_ADC_Config(AUX_ADC_DAC_COMP_Type *pstcADC,
                           uint8_t multi_channel_en,
                           uint8_t static_fifo_mode,
                           uint8_t fifo_threshold,
                           uint8_t internal_dma_en);
rsi_error_t RSI_ADC_Start(AUX_ADC_DAC_COMP_Type *pstcADC);
rsi_error_t RSI_ADC_Stop(AUX_ADC_DAC_COMP_Type *pstcADC);
rsi_error_t RSI_ADC_ChnlIntrUnMask(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel);
rsi_error_t RSI_ADC_ChnlIntrMask(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel);
#endif
#if defined(SLI_SI917)
rsi_error_t ADC_Start(adc_config_t adcConfig);
rsi_error_t ADC_Deinit(adc_config_t adcConfig);
rsi_error_t ADC_Stop(adc_config_t adcConfig);
rsi_error_t RSI_ADC_Config(AUX_ADC_DAC_COMP_Type *pstcADC,
                           uint8_t multi_channel_en,
                           uint8_t static_fifo_mode,
                           uint8_t a_empty_threshold,
                           uint8_t a_full_threshold,
                           uint8_t internal_dma_en);
rsi_error_t RSI_ADC_Start(AUX_ADC_DAC_COMP_Type *pstcADC, uint8_t mode);
rsi_error_t RSI_ADC_Stop(AUX_ADC_DAC_COMP_Type *pstcADC, uint8_t mode);
rsi_error_t RSI_ADC_ChnlIntrUnMask(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel, uint8_t oper_mode);
rsi_error_t RSI_ADC_ChnlIntrMask(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel, uint8_t oper_mode);
rsi_error_t RSI_ADC_ExtTrigConfig(const AUX_ADC_DAC_COMP_Type *pstcADC, adc_extr_config_t ExtrTrigConfig);
rsi_error_t RSI_ADC_ChnlTriggSel(uint8_t ext_sel_num, uint8_t chnl_id);
rsi_error_t RSI_ADC_DectEdgeSel(uint8_t detection_num, uint8_t edge_sel);
uint8_t RSI_ADC_ExtTrigStatusRead(const AUX_ADC_DAC_COMP_Type *pstcADC, adc_extr_config_t ExtrTrigConfig);
rsi_error_t RSI_ADC_ExtTrigStatusClear(const AUX_ADC_DAC_COMP_Type *pstcADC, adc_extr_config_t ExtrTrigConfig);
#endif
rsi_error_t ADC_PingPongReconfig(uint8_t event, uint8_t channel_num);

// Internal APIs
rsi_error_t RSI_ADC_PingPongMemoryAdrConfig(AUX_ADC_DAC_COMP_Type *pstcADC,
                                            uint32_t channel,
                                            adc_ping_pong_memory_adr_config_t PingPongMemoryAdrConfig);

void RSI_ADC_Calibration(void);

rsi_error_t RSI_ADC_PingpongEnable(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel);

rsi_error_t RSI_ADC_InternalPerChnlDmaEnable(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel);

rsi_error_t RSI_ADC_ChannelConfig(AUX_ADC_DAC_COMP_Type *pstcADC,
                                  uint32_t channel,
                                  uint8_t an_perif_adc_ip_sel,
                                  uint8_t an_perif_adc_in_sel,
                                  uint8_t an_perif_adc_diffmode);

rsi_error_t RSI_ADC_ChannelSamplingRate(AUX_ADC_DAC_COMP_Type *pstcADC,
                                        uint32_t channel,
                                        uint16_t adc_ch_offset,
                                        uint16_t adc_ch_freq_val);

rsi_error_t RSI_ADC_StaticMode(AUX_ADC_DAC_COMP_Type *pstcADC,
                               uint16_t an_perif_adc_ip_sel,
                               uint16_t an_perif_adc_in_sel,
                               uint8_t an_perif_adc_diffmode);

rsi_error_t RSI_ADC_ChnlEnable(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel);

rsi_error_t RSI_ADC_ChnlDisable(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel);

rsi_error_t RSI_ADC_ClkDivfactor(AUX_ADC_DAC_COMP_Type *pstcADC, uint16_t adc_on_time, uint16_t adc_total_duration);

rsi_error_t RSI_ADC_ChnlIntrClr(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel);

uint32_t RSI_ADC_ChnlIntrStatus(const AUX_ADC_DAC_COMP_Type *pstcADC);

void RSI_ADC_PowerControl(POWER_STATE state);

rsi_error_t RSI_ADC_NoiseAvgMode(AUX_ADC_DAC_COMP_Type *pstcADC, bool en_disable);

rsi_error_t RSI_ADC_TempSensorEnable(AUX_ADC_DAC_COMP_Type *pstcADC);

rsi_error_t RSI_ADC_ThresholdConfig(AUX_ADC_DAC_COMP_Type *pstcADC,
                                    int16_t threshold1,
                                    uint8_t *threshold1_cond,
                                    int16_t threshold2,
                                    uint8_t *threshold2_cond,
                                    uint8_t range);

rsi_error_t RSI_ADC_Bbp(AUX_ADC_DAC_COMP_Type *pstcADC, uint8_t adc_bbp_en, uint8_t bbp_en, uint8_t bbp_id);

rsi_error_t RSI_ADC_InterruptHandler(const AUX_ADC_DAC_COMP_Type *pstcADC);

void RSI_ADC_ThreshInterruptClr(AUX_ADC_DAC_COMP_Type *pstcADC);

rsi_error_t RSI_ADC_PingPongReInit(AUX_ADC_DAC_COMP_Type *pstcADC,
                                   uint8_t channel,
                                   uint8_t ping_enable,
                                   uint8_t pong_enable);

rsi_error_t RSI_ADC_InternalPerChnlDmaDisable(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel);

rsi_error_t RSI_ADC_PingpongDisable(AUX_ADC_DAC_COMP_Type *pstcADC, uint32_t channel);

rsi_error_t RSI_ADC_FifoMode(AUX_ADC_DAC_COMP_Type *pstcADC,
                             uint16_t channel_no,
                             uint16_t an_perif_adc_ip_sel,
                             uint16_t an_perif_adc_in_sel,
                             uint8_t an_perif_adc_diffmode);

int16_t RSI_ADC_ReadDataStatic(const AUX_ADC_DAC_COMP_Type *pstcADC, uint8_t data_process_en, uint8_t diff_en);

int16_t RSI_ADC_GainOffsetCal(int16_t data, uint8_t diff_en);

void RSI_ADC_AUXLdoConfig(AUX_ADC_DAC_COMP_Type *pstcADC, uint16_t bypass_en, uint16_t value);

float RSI_ADC_VrefCal(void);

rsi_error_t RSI_ADC_ReadData(int16_t *data,
                             uint8_t ping_pong,
                             uint16_t channel,
                             uint8_t data_process_en,
                             uint8_t diff_en);

uint32_t max_sample_rate_achive(float min_sampling_time);

uint32_t get_adc_clock(uint32_t f_sample_rate_achive);

rsi_error_t ADC_PinMux(uint8_t pos_input_pinsel, uint8_t neg_input_pinsel, uint8_t input_type);

rsi_error_t RSI_ADC_AUXLdoConfiguration(float verf_val);

rsi_error_t ADC_ChannelsDataSort(uint8_t data_select);

rsi_error_t ADC_Channel_freq_offset_cal(adc_ch_config_t *);

uint8_t check_power_two(uint16_t num);

uint32_t roundupto_pwr2(uint16_t num);

rsi_error_t cal_adc_channel_offset(void);

float get_min_sampling_time(uint8_t number_of_channel, adc_ch_config_t adcChConfig);

uint32_t ADC_GetSamplingRate(uint8_t ch_num);

void UDMA_ADC_Start(void);

void UDMA_ADC_Pong_Write(uint8_t reconfig);

void UDMA_ADC_Ping_Write(uint8_t ping_reconfig);

void ADC_DMA_Transfer_Complete(uint32_t channel, void *data);
void ADC_DMA_Error_Callback(uint32_t channel, void *data);

void UDMA_ADC_Init(void);
void IRQ011_Handler(void);
#ifdef __cplusplus
}
#endif

#endif // RSI_AUX_ADC_H