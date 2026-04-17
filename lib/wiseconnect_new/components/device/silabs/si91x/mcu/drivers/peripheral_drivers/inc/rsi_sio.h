/******************************************************************************
 * @file  rsi_sio.h
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

// Include Files
#include "base_types.h"
#include "rsi_ccp_common.h"

#ifndef RSI_SIO_H
#define RSI_SIO_H

#ifdef __cplusplus
extern "C" {
#endif

// Enum for SIO channels
typedef enum en_sio_channels {
  SioCh0,
  SioCh1,
  SioCh2,
  SioCh3,
  SioCh4,
  SioCh5,
  SioCh6,
  SioCh7,
} en_sio_channels_t;

// SIO-I2S transfer states
typedef enum en_sio_i2s_states {
  SioI2sIdle,
  SioI2sBusy,
  SioI2sTrasmitDone,

} en_sio_i2s_states_t;

// SIO-I2S transfer states
typedef enum en_sio_uart_states {
  SioUartIdle,
  SioUartsBusy,
  SioUartTxComeplete,
  SioUartRxComeplete,
} en_sio_uart_states_t;

// SIO-SPI transfer states
typedef enum en_sio_spi_states { SioSpiIdle, SioSpiBusy, SioSpiTrasmitDone } en_sio_spi_states_t;

// SIO-I2S events for call back functions
typedef enum en_sio_i2s_events { SioI2sTxDone, SioI2sRxDone } en_sio_i2s_events_t;

// SIO-UART events for call back functions
typedef enum en_sio_Uart_events {
  SioUartTxDone,
  SioUartRxDone,
  SioUartParityError,
  SioUartFramingError,
  SioUartRecvChar
} en_sio_Uart_events_t;

// SIO-SPI events for call back functions
typedef enum en_sio_spi_events { SioSpiTxDone, SioSpiRxDone } en_sio_spi_events_t;

// SIO-SPI modes configurations
typedef enum en_sio_spi_mode {
  SioSpiMode0,
  SioSpiMode3,
} en_sio_spi_mode_t;

// SIO-I2S Audio bit depth
typedef enum en_i2s_audio_depth { i2sAudioDepth16Bit, i2sAudioDepth32Bit } en_i2s_audio_depth_t;

// SIO-I2C configuration structure
typedef struct stc_sio_i2c_config {
  volatile uint8_t u8SioI2cSda;
  volatile uint8_t u8SioI2cScl;
  uint8_t u8SioI2cOen;
  uint32_t u32SampleRate;
} stc_sio_i2c_config_t;

// call back function pointers
typedef void (*sio_i2s_func_ptr_t)(en_sio_i2s_events_t enI2sSio);
typedef void (*sio_Spi_func_ptr_t)(en_sio_spi_events_t enSpiSio);
typedef void (*sio_Uart_func_ptr_t)(en_sio_Uart_events_t enUartSio, uint16_t u16RxChar);

// SIO-I2S Transfer structure
typedef struct stc_sio_i2s_xfer {
  void *txBuff;        /* TX buffer pointer; Must be a uint16_t pointer when transfer
                  size is 16 bits and uint8_t pointer when transfer size
            is 8 bits (can be NULL only when *txCount is 0) */
  void *rxBuff;        /* RX buffer pointer; Must be uint16_t pointer when transfer
            size is 16 bits or must be uint8_t pointer when transfer is
            8-bits (can be NULL only when *txCount is 0) */
  int32_t txCount;     // Pointer to an int32_t memory (never initialize to NULL)
                       // that has the Size of the txBuff in items (not bytes), not
                       // modified by driver
  int32_t rxCount;     // Number of items (not bytes) to send in rxBuff buffer
                       // (Never initialize to NULL), not modified by driver
  uint8_t u8BitLen;    // number of bits to transfer 8 , 16 , 32
  int32_t txDoneCount; /* Total items (not bytes) transmitted (initialize to 0),
                                           modified by driver [In case of underflow
                          txDoneCount will be greater than *txCount] */
  int32_t rxDoneCount; // Total items (not bytes) received (initialize to 0),
                       // modified by driver [In case of over flow rxDoneCount
                       // will be greater than *rxCount]
  sio_i2s_func_ptr_t pfnCb;
  uint8_t u8Status;
} stc_sio_i2s_xfer_t;

// SIO-I2S internal global structure member
typedef struct stc_sio_i2s {
  // I2S
  uint8_t u8I2sDataOutCh; // u8I2sDataOutCh is used to hold the user configured
                          // I2S channel for internal API usage
  uint8_t u8I2sDataInCh;  // u8I2sDataInCh is used to hold the user configured
                          // I2S channel for internal API usage
  uint8_t u8I2sClkCh;     // u8I2sClkCh is used to hold the user configured I2S
                          // channel for internal API usage
  uint8_t u8I2sWsCh;      // u8I2sWsCh is used to hold the user configured I2S
                          // channel for internal API usage
  uint8_t u8I2sValid;
  uint8_t u8I2sWsWidth;
  stc_sio_i2s_xfer_t *pstcI2sXfer;
} stc_sio_i2s_t;

// SIO-I2C Internal global structure member
typedef struct stc_sio_i2c {
  // I2c
  uint8_t u8I2cSdaCh; // u8I2cSdaCh is used to hold the user configured i2c
                      // channel for internal API usage
  uint8_t u8I2cClkCh; // u8I2cClkCh is used to hold the user configured i2c
                      // channel for internal API usage
} stc_sio_i2c_t;

// SIO-SPI configuration structure
typedef struct stc_sio_spi_cfg {

  volatile uint8_t u8SpiMosiCh; // u8SpiMosiCh is used to use with SIO
  volatile uint8_t u8SpiMisoCh; // u8SpiMisoCh is used to use with SIO
  uint8_t u8SpiClkCh;           // u8SpiClkCh is used to use with SIO
  uint8_t u8SpiCsCh;            // u8SpiCCh is used to use with SIO
  uint32_t u32SpiClockFrq;      // u32SpiClockFrq is used to use with SIO
  uint8_t u8BitOrder;           // u8BitOrder is used to use with SIO
  uint8_t u8Mode;               // Mode 0 , and mode 3 are supported
  uint8_t u8BitLen;
} stc_sio_spi_cfg_t;

// SIO-SPI transfer structure
typedef struct stc_sio_spi_xfer {

  void *txBuff;        /* TX buffer pointer; Must be a uint16_t pointer when transfer
                 size is 16 bits and uint8_t pointer when transfer size is
             8 bits (can be NULL only when *txCount is 0) */
  void *rxBuff;        /* RX buffer pointer; Must be uint16_t pointer when transfer
             size is 16 bits or must be uint8_t pointer when transfer is
             8-bits (can be NULL only when *txCount is 0) */
  int32_t txCount;     // Size of the txBuff in items (not bytes), not
                       // modified by driver
  int32_t rxCount;     // Number of items (not bytes) to send in rxBuff buffer
                       // (Never initialize to NULL), not modified by driver
  uint8_t sselNum;     // CS number assigned to this transfer
  uint8_t u8BitLen;    // number of bits to transfer 8 , 16 , 32
  int32_t txDoneCount; // Total items (not bytes) transmitted (initialize to 0),
                       // modified by driver [In case of underflow txDoneCount
                       // will be greater than *txCount]
  int32_t rxDoneCount; // Total items (not bytes) received (initialize to 0),
                       // modified by driver [In case of over flow rxDoneCount
                       // will be greater than *rxCount]
  uint8_t u8Status;
  sio_Spi_func_ptr_t pfnCb;
} stc_sio_spi_xfer_t;

// SIO-SPI Internal global structure member
typedef struct stc_sio_spi {
  // SPI
  uint8_t u8SpiMosiCh; // u8SpiMosiCh is used to hold the user configured SPI
                       // channel for internal API usage
  uint8_t u8SpiMisoCh; // u8SpiMisoCh is used to hold the user configured SPI
                       // channel for internal API usage
  uint8_t u8SpiClkCh;  // u8SpiClkCh is used to hold the user configured SPI
                       // channel for internal API usage
  uint8_t u8SpiCCh;    // u8SpiCCh is used to hold the user configured SPI channel
                       // for internal API usage
  uint8_t u8SpiValid;
  stc_sio_spi_xfer_t *pstscSpiXfer;
} stc_sio_spi_t;

// SIO-I2S Configuration structure
typedef struct stc_sio_i2s_config {
  uint8_t u8SioI2sDataOutChannel;
  uint8_t u8SioI2sDataInChannel;
  uint8_t u8SioI2sWsChannel;
  uint8_t u8SioI2sClkChannel;
  uint32_t u32SampleRate;
  uint8_t u8BitLen;
  uint8_t u8BitOrder;
  uint8_t u8I2sWsWidth;
} stc_sio_i2s_config_t;

// SIO-UART Configuration structure
typedef struct stc_sio_uart_config {
  volatile uint8_t u8SioUartTxChannel;
  volatile uint8_t u8SioUartRxChannel;
  uint8_t u8StopBits;
  uint8_t u8Parity;
  uint32_t u32BaudRate;
  uint8_t u8Bitlen;
  sio_Uart_func_ptr_t pfn;
} stc_sio_uart_config_t;

// SIO-UART internal global structure member
typedef struct stc_sio_uart {
  // UART
  uint8_t u8UartTxCh; // u8UartTxCh is used to hold the user configured UART
                      // channel for internal API usage
  uint8_t u8UartRxCh; // u8UartRxCh is used to hold the user configured UART
                      // channel for internal API usage
  uint16_t u16UartTxDone;
  uint16_t u16UartRxDone;
  uint16_t u16UartTxCnt;
  uint16_t u16UartRxCnt;
  void *pvUartTx;
  void *pvUartRx;
  uint8_t u8UartValid;
  uint8_t u8ParityEn;
  uint32_t u32SioUartRxData;
  en_sio_uart_states_t volatile u8Status;
  en_sio_uart_states_t volatile enRxStatus;
  stc_sio_uart_config_t *pstcSioUart;
} stc_sio_uart_t;

// Global structure to hold the channels for all protocols
typedef struct stc_sio_cb {
  // I2C SIO channel information structure
  stc_sio_i2c_t i2c_sio;
  // UART SIO channel information structure
  stc_sio_uart_t uart_sio;
  // SPI SIO channel information structure
  stc_sio_spi_t spi_sio;
  // I2S SIO channel information structure
  stc_sio_i2s_t i2s_sio;
} stc_sio_cb_t;

// brief Interrupt Configurations.
typedef enum {
  INTERRUPT_RISE_EDGE  = 0, // interrupt when rise edge is detected
  INTERRUPT_FALL_EDGE  = 1, // interrupt when fall edge is detected
  INTERRUPT_LEVEL_ZERO = 2, // interrupt when level 0 is detected
  INTERRUPT_LEVEL_ONE  = 3, // interrupt when level 1 is detected
} interrupt_flag_t;

// Pattern match
typedef enum {
  PATTERN_UNMATCH,
  PATTERN_MATCH,
} pattern_match_t;

// Clock selection
typedef enum {
  INTERNAL_CLOCK,
  EXTERNAL_CLOCK,
} clock_type_t;

// Flow control bit
typedef enum {
  FLOW_CONTROL_DISABLE,
  FLOW_CONTROL_ENABLE,
} flow_control_t;

// Reverse load
typedef enum {
  REVERSE_DISABLE,
  REVERSE_ENABLE,
} reverse_load_t;

// Edge selection
typedef enum {
  POS_EDGE,
  NEG_EDGE,
} edge_select_t;

// Global structure instance
extern stc_sio_cb_t gstcSioCb;

//  SIO_SIO apis function prototype
void RSI_SIO_ClockEnable(void);
void RSI_SIO_I2cGenerateStop(volatile SIO_Type *pstcSio);
rsi_error_t RSI_SIO_Init(volatile SIO_Type *pstcSio);
rsi_error_t RSI_SIO_InitSpi(volatile SIO_Type *pstcSio, stc_sio_spi_cfg_t *pstcSpiConfig);
void RSI_SIO_SpiCsAssert(volatile SIO_Type *pstcSio, uint8_t u8CsNo);
void RSI_SIO_SpiCsDeAssert(volatile SIO_Type *pstcSio, uint8_t u8CsNo);
rsi_error_t RSI_SIO_SpiTrasnfer(volatile SIO_Type *pstcSio, stc_sio_spi_xfer_t *pstcSpiXfer);

//  SIO_UART apis function prototype
uint8_t RSI_SIO_UartInit(SIO_Type *pstcSio, stc_sio_uart_config_t *pstcConfig);
uint8_t RSI_SIO_UartGetParity(uint16_t u16Data);
uint16_t RSI_SIO_UartFramePrepare(uint8_t u8StopBits, uint16_t u16Data, uint8_t u8ParitySet, uint8_t u8DataLen);
uint16_t RSI_SIO_UARTSend(SIO_Type *pstcSio, const void *u16ptr, uint16_t u16Len);
uint16_t RSI_SIO_UartFrameExtract(uint8_t u8StopBits,
                                  uint16_t u16UartDataFrame,
                                  uint8_t u8ParitySet,
                                  uint8_t u8DataLen);
int RSI_SIO_UARTSendBlocking(SIO_Type *pstcSio, const void *data, int numBytes);
int RSI_SIO_UARTRead(volatile SIO_Type *pstcSio, void *data, int numBytes);
int RSI_SIO_UARTReadBlocking(volatile SIO_Type *pstcSio, void *data, int numBytes);

//  SIO_I2C apis function prototype
uint32_t RSI_SIO_I2cPrepareWrite(uint8_t u8InDat);
uint32_t RSI_SIO_I2cPrepareRead(uint32_t u32Indata);
rsi_error_t RSI_SIO_I2cWrite(volatile SIO_Type *pstcSio,
                             stc_sio_i2c_config_t *pstcConfig,
                             uint8_t u8SlaveAddr,
                             uint8_t *u8Data,
                             uint16_t u16Len);
rsi_error_t RSI_SIO_I2cRead(volatile SIO_Type *pstcSio,
                            stc_sio_i2c_config_t *pstcConfig,
                            uint8_t u8SlaveAddr,
                            uint8_t *u8Data,
                            uint16_t u16Len);
rsi_error_t RSI_SIO_I2cTransfer(volatile SIO_Type *pstcSio,
                                stc_sio_i2c_config_t *pstcConfig,
                                uint8_t u8SlaveAddr,
                                uint8_t *u8PtrTxDat,
                                uint16_t u16TxLen,
                                uint8_t *u8PtrRxDat,
                                uint16_t u16RxLen);
void RSI_SIO_I2cGenerateStart(volatile SIO_Type *pstcSio);
void RSI_SIO_I2cGenerateStop(volatile SIO_Type *pstcSio);

rsi_error_t RSI_SIO_Initialization(void);
void RSI_SIO_Configure_Interrupt(volatile SIO_Type *pstcSio, en_sio_channels_t channel, interrupt_flag_t flag);
void RSI_SIO_Match_Pattern(volatile SIO_Type *pstcSio,
                           en_sio_channels_t channel,
                           pattern_match_t pattern,
                           uint8_t slice,
                           uint32_t slice_pattern);
void RSI_SIO_Shift_Clock(volatile SIO_Type *pstcSio, uint32_t Divider, en_sio_channels_t channel);
void RSI_SIO_Select_Clock(volatile SIO_Type *pstcSio, en_sio_channels_t channel, clock_type_t clock);
void RSI_SIO_Position_Counter(volatile SIO_Type *pstcSio, en_sio_channels_t channel, uint32_t data_shift);
void RSI_SIO_Control_Flow(volatile SIO_Type *pstcSio, en_sio_channels_t channel, flow_control_t flow_control);
void RSI_SIO_Reverse_Load(volatile SIO_Type *pstcSio, en_sio_channels_t channel, reverse_load_t reverse);
void RSI_SIO_Set_Interrupt(volatile SIO_Type *pstcSio, en_sio_channels_t channel);
void RSI_SIO_Clear_Interrupt(volatile SIO_Type *pstcSio, en_sio_channels_t channel);
void RSI_SIO_Mask_Interrupt(volatile SIO_Type *pstcSio, en_sio_channels_t channel);
void RSI_SIO_UnMask_Interrupt(volatile SIO_Type *pstcSio, en_sio_channels_t channel);
uint32_t RSI_SIO_Get_Interrupt_Status(volatile SIO_Type *pstcSio);
void RSI_SIO_Set_Shift_Interrupt(volatile SIO_Type *pstcSio, en_sio_channels_t channel);
void RSI_SIO_Clear_Shift_Interrupt(volatile SIO_Type *pstcSio, en_sio_channels_t channel);
void RSI_SIO_Mask_Shift_Interrupt(volatile SIO_Type *pstcSio, en_sio_channels_t channel);
void RSI_SIO_UnMask_Shift_Interrupt(volatile SIO_Type *pstcSio, en_sio_channels_t channel);
uint32_t RSI_SIO_Shift_Interrupt_Status(volatile SIO_Type *pstcSio);
void RSI_SIO_Edge_Select(volatile SIO_Type *pstcSio, en_sio_channels_t channel, edge_select_t edge_sel);
uint32_t RSI_SIO_Read_Buffer(volatile SIO_Type *pstcSio, en_sio_channels_t channel);
void RSI_SIO_Write_Buffer(volatile SIO_Type *pstcSio, en_sio_channels_t channel, uint32_t data);

#ifdef __cplusplus
}
#endif

#endif // RSI_SIO_H
