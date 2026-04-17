/* -----------------------------------------------------------------------------
 *
 * SPDX-License-Identifier: Zlib
 * Copyright (c) 2013-2014 ARM Ltd.
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
 * $Date:        24. Nov 2014
 * $Revision:    V2.02
 *
 * Project:      USART (Universal Synchronous Asynchronous Receiver Transmitter)
 *               Driver definitions
 * -------------------------------------------------------------------------- */
/*
 *  Version 1.00
 *    Initial release
 */
 
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif
  
#include "rsi_pll.h"
#include "rsi_ulpss_clk.h"
#include "Driver_USART.h"
#include "rsi_ccp_common.h"

#include "UDMA.h"

#define USART_INSTANCE_BIT 30          // USART Instance bit postion stored in event variable
#define USART_EVENT_MASK   0x3FFFFFFF  // USART Event Mask

// USART Transfer Information (Run-Time)
typedef struct _USART_TRANSFER_INFO {
  uint32_t                rx_num;        // Total number of data to be received
  uint32_t                tx_num;        // Total number of data to be send
#ifdef SLI_SI91X_MCU_RS485_DATA_BIT_9
  uint16_t                *rx_buf;        // Pointer to in data buffer
  uint16_t                *tx_buf;        // Pointer to out data buffer
#else
  uint8_t                *rx_buf;        // Pointer to in data buffer
  uint8_t                *tx_buf;        // Pointer to out data buffer
#endif
  uint32_t                rx_cnt;        // Number of data received
  uint32_t                tx_cnt;        // Number of data sent
  uint8_t                 tx_def_val;    // Transmit default value (used in USART_SYNC_MASTER_MODE_RX)
  uint8_t                 rx_dump_val;   // Receive dump value (used in USART_SYNC_MASTER_MODE_TX)
  uint8_t                 send_active;   // Send active flag
  uint8_t                 sync_mode;     // Synchronous mode
} USART_TRANSFER_INFO;

typedef struct _USART_RX_STATUS {
  uint8_t rx_busy;                       // Receiver busy flag
  uint8_t rx_overflow;                   // Receive data overflow detected (cleared on start of next receive operation)
  uint8_t rx_break;                      // Break detected on receive (cleared on start of next receive operation)
  uint8_t rx_framing_error;              // Framing error detected on receive (cleared on start of next receive operation)
  uint8_t rx_parity_error;               // Parity error detected on receive (cleared on start of next receive operation)
} USART_RX_STATUS;

// USART DMA
typedef  struct _USART0_DMA 
{
  RSI_UDMA_CHA_CONFIG_DATA_T control;
  uint8_t                 channel;       // DMA Channel number
  UDMA_SignalEvent_t      cb_event;      // DMA Event callback
} USART_DMA;


// USART flags
#define USART_FLAG_INITIALIZED       (1U << 0)
#define USART_FLAG_POWERED           (1U << 1)
#define USART_FLAG_CONFIGURED        (1U << 2)
#define USART_FLAG_TX_ENABLED        (1U << 3)
#define USART_FLAG_RX_ENABLED        (1U << 4)
#define USART_FLAG_SEND_ACTIVE       (1U << 5)
 
/*!< USART Configuration control bits (Line control Register)*/
#define USART_CNTL_DATALEN_5           (0x00 << 0)  	/*!< USART 5 bit length mode      */     
#define USART_CNTL_DATALEN_6           (0x01 << 0)   /*!< USART 6 bit length mode      */  
#define USART_CNTL_DATALEN_7           (0x02 << 0)   /*!< USART 7 bit length mode      */
#define USART_CNTL_DATALEN_8           (0x03 << 0)   /*!< USART 8 bit length mode      */
#define USART_CNTL_STOPBIT_1           (0x00 << 2)   /*!< USART One Stop Bit Select    */
#define USART_CNTL_STOPBIT_2           (0x01 << 2)   /*!< USART Two Stop Bits Select   */
#define USART_CNTL_STOPBIT_1P5         (0x01 << 2)   /*!< USART Two Stop Bits Select   */
#define USART_CNTL_PARITY_OFF          (0x00 << 3)   /*!< Parity Enabled              */
#define USART_CNTL_PARITY_ON           (0x01 << 3)   /*!< Parity Disabled             */
#define USART_CNTL_PARITY_ODD          (0x00 << 4)   /*!< Odd parity                  */
#define USART_CNTL_PARITY_EVEN         (0x01 << 4)   /*!< Even parity                 */
#define USART_PARITY_ENABLE            (0x01 << 3)   /*!< Set parity                 */ 
#define USART_CNTL_DLAB_SET            (0x01 << 7)
#define USART_CNTL_DLAB_RESET          (0x00)
#define USART_CNTL_DATALEN_9           (0x01 << 0)   /*!< USART 9 bit length mode      */

/*!< USART FIFO Configuration control bits (FIFO control Register)*/
#define USART_FIFO_ENABLE              (0x01 << 0)   /*!< FIFO Enable                 */
#define USART_FIFO_RX_RESET            (0x01 << 1)	  /*!< Receive FIFO Reset          */
#define USART_FIFO_TX_RESET            (0x01 << 2)   /*!< Transmit FIFO Reset         */
#define USART_FIFO_TX_EMPTY            (0x00 << 4)   /*!< Transmit Empty              */
#define USART_FIFO_TX_AEMPTY           (0x01 << 4)   /*!< Transmit Almost Empty       */ 
#define USART_FIFO_TX_QUARTER_FULL    (0x02 << 4)   /*!< Transmit FIFO Quarter full  */
#define USART_FIFO_TX_HALF_FULL        (0x03 << 4)   /*!< Transmit FIFO Half full     */
#define USART_FIFO_RX_AEMPTY           (0x00 << 6)   /*!< Receive FIFO AEMPTY         */
#define USART_FIFO_RX_QUARTER_FULL    (0x01 << 6)   /*!< Receive FIFO Quarter full   */
#define USART_FIFO_RX_HALF_FULL        (0x01 << 7)   /*!< Receive FIFO half full      */
#define USART_FIFO_RX_AFULL            (0x03 << 6)   /*!< RX FIFO Almost Full         */
#define USART_DMA_MODE_EN							 (1UL << 3)    /*!< Enable DMA MODE */

/*!< USART (Interrupt Enable Register)*/
#define USART_INTR_RX_DATA             (0x01 << 0)   /*!< Enable Received Data Available Interrupt.    */
#define USART_INTR_THRE                (0x01 << 1)   /*!< Enable Transmit Holding Register Empty Interrupt.*/
#define USART_INTR_RXRDY               (0x01 << 2)   /*!< Receive Ready Interrupt     */
#define USART_INTR_MODEM_STATUS        (0x01 << 3)   /*!< Enable Modem Status Interrupt */
#define USART_INTR_PROGRAMMABLE_THRE   (0x01 << 7)   /*!< Enable Programmable THRE Interrupt */

/*!< USART (Interrupt Identity Register)*/
#define USART_MODEM_STATUS_INTR        (0x00 << 0)
#define USART_NO_INTR_PENDING          (0x01 << 0)   /*!< NO Interrupt Pending        */
#define USART_THR_EMPTY                (0x01 << 1)   /*!< THR Empty                   */
#define USART_RX_DATA_AVAILABLE        (0x01 << 2)   /*!< Received Data Available     */
#define USART_RX_LINE_STATUS           (0x03 << 1)   /*!< Receiver line status        */ 
#define USART_BUSY_DETECT              (0x07 << 0)   /*!< USART busy detect            */ 
#define USART_IIR_FIFO_ENABLE          (0x03 << 6)   /*!< IIR FIFO enabled            */ 

/*!< USART (Modem status registers)*/
#define USART_MSR_DCTS                 (0x1 << 0)    /*!<Delta Clear to Send          */
#define USART_MSR_DDSR                 (0x1 << 1)    /*!<Delta Data Set Ready         */
#define USART_MSR_TERI                 (0x1 << 2)    /*!<Trailing Edge of Ring Indicator */
#define USART_MSR_DDCD                 (0x1 << 3)    /*!<Delta Data Carrier Detect */
 
/*< USART (Line status Register)*/
#define USART_OVERRUN_ERR              (0x01 << 1)  /*!< This is used to indicate the occurrence of an overrun error. This occurs
                                                         if a new data character was received before the previous data was read.        */ 
#define USART_PARITY_ERR               (0x01 << 2)  /*!< This is used to indicate the occurrence of a parity error in the receiver if
                                                         the Parity Enable                                                              */
#define USART_FRAMING_ERR              (0x01 << 3)  /*!< This is used to indicate the occurrence of a framing error in the receiver.
                                                         A framing error occurs when the receiver does not detect a valid STOP bit 
                                                         in the received data                                                           */
#define USART_BREAK_ERR                (0x01 << 4)  /*!< Break Interrupt bit. This is used to indicate the detection of a break sequence on the serial
                                                         input data.                                                                    */ 
#define USART_RECV_FIFO_ERR            (0x01 << 7)  /*!< This is used to indicate if there is at least one parity error,
                                                         framing error, or break indication in the FIFO.                                */
#define USART_LSR_THRE   (0x1 <<5)

/*!< USART (Modem control Register)*/
#define USART_MODEM_DTR_SET             (0x01 << 0)   /*!< Data Terminal Ready set     */
#define USART_MODEM_RTS_SET	            (0x01 << 1)   /*!< Request to send set         */
#define USART_MODEM_RTS_RESET	          (0x00 << 1)   /*!< Request to send set         */
#define USART_MODEM_LOOP_BACK_ENABLE    (0x01 << 4)   /*!< LoopBack Bit                */
#define USART_MODEM_AFCE_ENABLE         (0x01 << 5)   /*!< Auto Flow control Enable    */
#define USART_MODEM_AFCE_DISABLE        (0x00 << 5)   /*!< Auto Flow control Disable    */

/*!< USART (LCR Register)*/
#define USART_LCR_BC            (0x01 << 6)   

// UART Modem status register
#define UART_MSR_DCTS                (1U << 0)
#define UART_MSR_DDSR                (1U << 1)
#define UART_MSR_TERI                (1U << 2)
#define UART_MSR_DDCD                (1U << 3)
#define UART_MSR_CTS                 (1U << 4)
#define UART_MSR_DSR                 (1U << 5)
#define UART_MSR_RI                  (1U << 6)
#define UART_MSR_DCD                 (1U << 7)

/*!< API configuration macros*/
#define USART_DATA_LEN_5                        5
#define USART_DATA_LEN_6                        6
#define USART_DATA_LEN_7                        7
#define USART_DATA_LEN_8                        8
#define USART_DATA_LEN_9                        9

#define USART_STOP_BIT_1                        0
#define USART_STOP_BIT_1P5                      1
#define USART_STOP_BIT_2                        2

#define USART_TRIGGER_TX_EMPTY                  0

#define  USART_DMA_MODE0_EN                    (0 << 3)
#define  USART_DMA_MODE1_EN                    (1 << 3) 

#define USART_TRIGGER_RX_AEMPTY                 0

#define USART_ERROR_CALLBACK_INDEX              0
#define USART_EVENT_CALLBACK_INDEX              1

// USART synchronos xfer modes
#define USART_SYNC_MODE_TX           ( 1U )
#define USART_SYNC_MODE_RX           ( 2U )
#define USART_SYNC_MODE_TX_RX        (USART_SYNC_MODE_TX | \
                                      USART_SYNC_MODE_RX)
									
#define USART_LCR_TEMT                  0x80
#define USART_RESET_REG                 0x00   ///< USART reset register address  
 
typedef void (*ARM_USART_SignalEvent_t) (uint32_t event);  ///< Pointer to \ref ARM_USART_SignalEvent : Signal USART Event.

typedef struct _USART_CLOCK
{
   USART_CLK_SRC_SEL_T  uart_clock_src;
	 ULP_UART_CLK_SELECT_T ulp_uart_source;
	 uint32_t divfact; 
   uint32_t frac_div_en;
 }USART_CLOCK;
 
typedef const struct _USART_PIN {
  uint8_t port;                                        ///< CAN GPIO port
  uint8_t pin;                                         ///< CAN GPIO pin
  uint8_t mode;                                        ///< CAN GPIO mode
  uint8_t pad_sel;                                     ///< CAN GPIO pad selection
}USART_PIN;

// USART Information (Run-Time)
typedef struct _USART_INFO {
  ARM_USART_SignalEvent_t cb_event;      // Event callback
  USART_RX_STATUS         rx_status;     // Receive status flags
  USART_TRANSFER_INFO     xfer;          // Transfer information
  uint8_t                 mode;          // USART mode
  uint8_t                 flags;         // USART driver flags
  uint32_t                baudrate;      // Baudrate
} USART_INFO;

// USART Input/Output Configuration
typedef const struct _USART_IO {
	USART_PIN      *clock;            
	USART_PIN      *tx;          
	USART_PIN      *rx;             
	USART_PIN      *cts; 
	USART_PIN      *rts;             
	USART_PIN      *ir_tx;           
  USART_PIN      *ir_rx;            
} USART_IO;

 typedef struct _USART_MODE
 {
	 uint8_t  en_usart_mode;
	 uint8_t continuous_clock; 
 }USART_SYNC_MODE;
 
/*!<USART data structure */
typedef struct 
{
  ARM_USART_CAPABILITIES   capabilities;   ///< Capabilities
  USART0_Type              *pREGS; 
  IRQn_Type                irq_num; 
  USART_DMA                *dma_tx;
  USART_DMA                *dma_rx;
  USART_INFO               *info; 
	USART_IO                 io;    
  USART_CLOCK              clock;
  USART_SYNC_MODE          sync_mode;
} USART_RESOURCES;
ARM_USART_CAPABILITIES USART_GetCapabilities (const USART_RESOURCES *usart);
ARM_DRIVER_VERSION ARM_USARTx_GetVersion(void);
void IRQ038_Handler(void);
void IRQ039_Handler(void);
void IRQ012_Handler(void);
void RSI_M4SSUsart0Handler(void);
void RSI_M4SSUart1Handler(void);
void RSI_ULPUartHandler(void);

#ifdef __cplusplus
}
#endif
#endif // __USART_H__
