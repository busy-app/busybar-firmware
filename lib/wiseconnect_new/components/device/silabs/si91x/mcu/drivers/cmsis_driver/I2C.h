/* -----------------------------------------------------------------------------
 * SPDX-License-Identifier: Zlib
 * Copyright (c) 2013-2016 ARM Ltd.
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
 * $Date:        29. March 2016
 * $Revision:    V2.6
 *
 * Project:      Inter Integrated Circuit(I2C) Definitions for Silicon Labs MCU
 * -------------------------------------------------------------------------- */

#ifndef __I2C_H
#define __I2C_H

#include "rsi_pll.h"
#include "rsi_ulpss_clk.h"
#include "Driver_I2C.h"
#include "rsi_ccp_common.h"


/* I2C Interrupt Events */
#define 	RSI_I2C_EVENT_RECEIVE_UNDER       		(1UL << 0)  /*!< read the receive buffer when it is empty inerrupt  */ 
#define 	RSI_I2C_EVENT_RECEIVE_OVER						(1UL << 1)  /*!< receive buffer is completely filled interrupt      */ 
#define 	RSI_I2C_EVENT_RECEIVE_FULL	      		(1UL << 2)  /*!< receive buffer above the RX_TL threshold interrupt */ 
#define 	RSI_I2C_EVENT_TRANSMIT_OVER       		(1UL << 3)  /*!< issue write command after transmit buffer is filled completely */
#define 	RSI_I2C_EVENT_TRANSMIT_EMPTY      		(1UL << 4)  /*!< TX FIFO is empty interrupt	*/ 
#define 	RSI_I2C_EVENT_READ_REQ     						(1UL << 5)  /*!< In slave mode holds SCL if another I2C master is attempting to read */
#define 	RSI_I2C_EVENT_TRANSMIT_ABRT		    		(1UL << 6)  /*!< Transmit abort interrupt									 */
#define 	RSI_I2C_EVENT_RECEIVE_DONE        		(1UL << 7)  /*!< receive done interrupt 									 */
#define 	RSI_I2C_EVENT_ACTIVITY_ON_BUS     		(1UL << 8)  /*!< I2C activity interrupt										 */ 
#define 	RSI_I2C_EVENT_STOP_DETECT     				(1UL << 9)  /*!< Stop condition on bus detection interrupt */ 
#define 	RSI_I2C_EVENT_START_DETECT     				(1UL << 10) /*!< START or RESTART condition interrupt			 */ 
#define 	RSI_I2C_EVENT_GENERAL_CALL		    		(1UL << 11) /*!< General call interrupt										 */
#define   RSI_I2C_EVENT_RESTART_DET							(1UL << 12) /*!< RESTART condition in slave mode		       */
#define   RSI_I2C_EVENT_MST_ON_HOLD							(1UL << 13)	/*!< master is holding the bus and the Tx FIFO is empty	*/							 
#define   RSI_I2C_EVENT_SCL_STUCK_AT_LOW        (1UL << 14)

#define 	I2C_RECEIVE_UNDER       		          (0)         /*!< read the receive buffer when it is empty inerrupt  */ 
#define 	I2C_RECEIVE_OVER						          (1)         /*!< receive buffer is completely filled interrupt      */ 
#define 	I2C_RECEIVE_FULL	      		          (2)         /*!< receive buffer above the RX_TL threshold interrupt */ 
#define 	I2C_TRANSMIT_OVER       		          (3)         /*!< issue write command after transmit buffer is filled completely */
#define 	I2C_TRANSMIT_EMPTY      		          (4)         /*!< TX FIFO is empty interrupt	*/ 
#define 	I2C_READ_REQ     						          (5)         /*!< In slave mode holds SCL if another I2C master is attempting to read */
#define 	I2C_TRANSMIT_ABRT		    		          (6)         /*!< Transmit abort interrupt									 */
#define 	I2C_RECEIVE_DONE        		          (7)         /*!< receive done interrupt 									 */
#define 	I2C_ACTIVITY_ON_BUS     		          (8)         /*!< I2C activity interrupt										 */ 
#define 	I2C_STOP_DETECT     				          (9)         /*!< Stop condition on bus detection interrupt */ 
#define 	I2C_START_DETECT     				          (10)        /*!< START or RESTART condition interrupt			 */ 
#define 	I2C_GENERAL_CALL		    		          (11)        /*!< General call interrupt										 */
#define   I2C_RESTART_DET							          (12)        /*!< RESTART condition in slave mode		       */
#define   I2C_MST_ON_HOLD							          (13)	      /*!< master is holding the bus and the Tx FIFO is empty	*/							 
#define   I2C_SCL_STUCK_AT_LOW                  (14)

/* TX Abort Sources */
#define   I2C_TX_ABRT_7B_ADDR_NOACK     			  (15)
#define   I2C_TX_ABRT_10ADDR1_NOACK     			  (16)
#define   I2C_TX_ABRT_10ADDR2_NOACK		 			    (17)
#define   I2C_TX_ABRT_TXDATA_NOACK      			  (18)
#define   I2C_TX_ABRT_GCALL_NOACK			 			    (19)
#define   I2C_TX_ABRT_GCALL_READ							  (20)
#define   I2C_TX_ABRT_HS_ACKDET				  		    (21)
#define   I2C_TX_ABRT_SBYTE_ACKDET						  (22)
#define   I2C_TX_ABRT_HS_NORSTRT							  (23)
#define   I2C_TX_ABRT_SBYTE_NORSTRT		  		    (24)
#define   I2C_TX_ABRT_10B_RD_NORSTRT					  (25)
#define   I2C_TX_ABRT_MASTER_DIS							  (26)
#define   I2C_TX_ARB_LOST							  		    (27)
#define   I2C_TX_ABRT_SLVFLUSH_TXFIFO	  		    (28)
#define   I2C_TX_ABRT_SLV_ARBLOST			  		    (29)
#define   I2C_TX_ABRT_SLVRD_INTX							  (30)
#define   I2C_TX_ABRT_USER_ABRT					        (31)
#define   I2C_TX_ABRT_SDA_STUCK_AT_LOW	    	  (32)
#define   I2C_TX_ABRT_DEVICE_NOACK			    	  (33)
#define   I2C_TX_ABRT_DEVICE_SLVADDR_NOACK      (34)
#define   I2C_TX_ABRT_DEVICE_WRITE						  (35)
#define   I2C_TX_TX_FLUSH_CNT								    (36)

#define   SLAVE_ADDR_CHECK_ARG									0x01
#define   SLAVE_RX_ARG													0x02
#define   SLAVE_TX_ARG													0x03

#define   NACK																	1
#define   ACK																		0

#define   F_RX_UNDER      											0
#define   F_RX_OVER       											1
#define   F_RX_FULL  		  											2
#define   F_TX_OVER       											3
#define   F_TX_EMPTY      											4
#define   F_RD_REQ        											5
#define   F_TX_ABRT       											6
#define   F_RX_DONE       											7
#define   F_ACTIVITY      											8
#define   F_STOP_DET      											9
#define   F_START_DET    												10
#define   F_GEN_CALL      											11
#define   F_RESTART_DET													12
#define   F_MST_ON_HOLD													13

#define   I2C_ACTIVITY   	 											(1UL << 0)
#define   I2C_TFNF       	 											(1UL << 1)
#define   I2C_TFE        	 											(1UL << 2)
#define   I2C_RFNE       	 											(1UL << 3)
#define   I2C_RFF        	 											(1UL << 4)
#define   I2C_MST_ACTIVITY 											(1UL << 5)
#define   I2C_SLV_ACTIVITY 											(1UL << 6)
 

#define   I2C_STAT_ACTIVITY   	 								0
#define   I2C_STAT_TFNF       	 								1
#define   I2C_STAT_TFE        	 								2
#define   I2C_STAT_RFNE       	 								3
#define   I2C_STAT_RFF        	 								4
#define   I2C_STAT_MST_ACTIVITY  								5
#define   I2C_STAT_SLV_ACTIVITY  								6
#define   I2C_TX_ABRT_SOURCE		 								7

#define   TX_ABRT_7B_ADDR_NOACK							   (1UL << 0)
#define   TX_ABRT_10ADDR1_NOACK							   (1UL << 1)
#define   TX_ABRT_10ADDR2_NOACK							   (1UL << 2)
#define   TX_ABRT_TXDATA_NOACK							   (1UL << 3)
#define   TX_ABRT_GCALL_NOACK								   (1UL << 4)
#define   TX_ABRT_GCALL_READ								   (1UL << 5)
#define   TX_ABRT_HS_ACKDET									   (1UL << 6)
#define   TX_ABRT_SBYTE_ACKDET							   (1UL << 7)
#define   TX_ABRT_HS_NORSTRT								   (1UL << 8)
#define   TX_ABRT_SBYTE_NORSTRT			    		   (1UL << 9)
#define   TX_ABRT_10B_RD_NORSTRT		    		   (1UL << 10)
#define   TX_ABRT_MASTER_DIS				    		   (1UL << 11)
#define   TX_ARB_LOST								    		   (1UL << 12)
#define   TX_ABRT_SLVFLUSH_TXFIFO		    		   (1UL << 13)
#define   TX_ABRT_SLV_ARBLOST				    		   (1UL << 14)
#define   TX_ABRT_SLVRD_INTX				    		   (1UL << 15)
#define   TX_ABRT_USER_ABRT					    		   (1UL << 16)
#define   TX_ABRT_SDA_STUCK_AT_LOW	    		   (1UL << 17)
#define   TX_ABRT_DEVICE_NOACK			    		   (1UL << 18)
#define   TX_ABRT_DEVICE_SLVADDR_NOACK  		   (1UL << 19)
#define   TX_ABRT_DEVICE_WRITE							   (1UL << 20)
#define   TX_TX_FLUSH_CNT										   (1UL << 23)								

/* bit defines */
#define     I2C2_BUS_CLK_ENABLE				         (1UL << 8)
#define     I2C_STATIC_CLK_EN					         (1UL << 17)
#define     UDMA_HCLK_ENBALE					         (1UL << 6)
#define     ULP_UDMA_STATIC_CLK_EN		         (1UL << 17)
#define     I2C2_STATIC_CLK_EN				         (1UL << 18)
#define     I2C_FIFO_DEPTH                      8


/* I2C example defines */
#define    CHNL_30									30
#define    CHNL_31									31
#define    PRIMARY     							0
#define    ALTERNATE   							1


#define    I2C_RESTART_EN  					0x1

#define    I2C_MST_ADDR_7BIT_MODE		0x0
#define    I2C_MST_ADDR_10BIT_MODE	0x1
#define    I2C_SLV_ADDR_7BIT_MODE		0x0

#define    I2C_SLV_ADDR							0x50

#define    I2C_SCL_LOW_CLK					0x6
#define    I2C_SCL_HIGH_CLK					0x1

#define    I2C_TX_TL								0x1
#define    I2C_RX_TL								0x4
#define    ADDRESS_WIDTH						8

#define    MULTI_PAGE_WRITE 				1
#define    SINGLE_PAGE_WRITE				0

#define    BIT_WIDTH_8							0
#define    BIT_WIDTH_16							1
#define    BIT_WIDTH_32							2

#define    READ_CMD									1
#define    WRITE_CMD								0

#define    TX_EN										1
#define    RX_EN										1

#define    I2C_DMA_TX_REQ						0x2
#define    I2C_DMA_RX_REQ						0x2
#define    DMA_ACK                 (1UL << 7)

/* PAD pin selection */
#define    I2C0_PAD_SEL_6_8					1
#define    GPIO_6_REN								6
#define    GPIO_8_REN								8

#define    I2CM_BLOCKING_XFER       1
#define    I2CM_NONBLOCKING_XFER    0


#define SS_MIN_SCL_HIGH         5200
#define SS_MIN_SCL_LOW          4700
#define FS_MIN_SCL_HIGH         1160
#define FS_MIN_SCL_LOW          1300
#define HS_MIN_SCL_HIGH_400PF   274
#define HS_MIN_SCL_LOW_400PF    333
#define HS_MIN_SCL_HIGH_100PF   60
#define HS_MIN_SCL_LOW_100PF    120

#define RSI_I2C_HIGH_SPEED      0x3

/* I2C Driver state flags */
#define I2C_FLAG_INIT       (1 << 0)        // Driver initialized
#define I2C_FLAG_POWER      (1 << 1)        // Driver power on
#define I2C_FLAG_SETUP      (1 << 2)        // Master configured, clock set
#define I2C_FLAG_SLAVE_RX   (1 << 3)        // Slave receive registered
#define I2C_FLAG_STOP_DET   (1 << 4)

/* I2C Stalled Status flags */
#define I2C_MASTER          (1 << 0)        // Master stalled
#define I2C_SLAVE_TX        (1 << 1)        // Slave stalled on transmit
#define I2C_SLAVE_RX        (1 << 2)        // Slave stalled on receive
#define I2C_SLAVE           (I2C_SLAVE_TX | I2C_SLAVE_RX)

/* I2C Interrupt Events */
#define 	I2C_EVENT_RECEIVE_UNDER       		(1UL << 0)  /*!< read the receive buffer when it is empty inerrupt  */ 
#define 	I2C_EVENT_RECEIVE_OVER						(1UL << 1)  /*!< receive buffer is completely filled interrupt      */ 
#define 	I2C_EVENT_RECEIVE_FULL	      		(1UL << 2)  /*!< receive buffer above the RX_TL threshold interrupt */ 
#define 	I2C_EVENT_TRANSMIT_OVER       		(1UL << 3)  /*!< issue write command after transmit buffer is filled completely */
#define 	I2C_EVENT_TRANSMIT_EMPTY      		(1UL << 4)  /*!< TX FIFO is empty interrupt	*/ 
#define 	I2C_EVENT_READ_REQ     						(1UL << 5)  /*!< In slave mode holds SCL if another I2C master is attempting to read */
#define 	I2C_EVENT_TRANSMIT_ABRT		    		(1UL << 6)  /*!< Transmit abort interrupt									 */
#define 	I2C_EVENT_RECEIVE_DONE        		(1UL << 7)  /*!< receive done interrupt 									 */
#define 	I2C_EVENT_ACTIVITY_ON_BUS     		(1UL << 8)  /*!< I2C activity interrupt										 */ 
#define 	I2C_EVENT_STOP_DETECT     				(1UL << 9)  /*!< Stop condition on bus detection interrupt */ 
#define 	I2C_EVENT_START_DETECT     				(1UL << 10) /*!< START or RESTART condition interrupt			 */ 
#define 	I2C_EVENT_GENERAL_CALL		    		(1UL << 11) /*!< General call interrupt										 */
#define   I2C_EVENT_RESTART_DET							(1UL << 12) /*!< RESTART condition in slave mode		       */
#define   I2C_EVENT_MST_ON_HOLD							(1UL << 13)	/*!< master is holding the bus and the Tx FIFO is empty	*/							 
#define   I2C_EVENT_SCL_STUCK_AT_LOW        (1UL << 14)

#define TRANSMITTER            0
#define RECEIVER               1

void I2C0_IRQHandler (void);
void I2C1_IRQHandler (void);
void I2C2_IRQHandler (void);
/* I2C Control Information */
typedef struct 
{
  ARM_I2C_SignalEvent_t cb_event;           // Event callback
  ARM_I2C_STATUS        status;             // Status flags
  uint8_t               flags;              // Control and state flags
  uint16_t              sla_rw;             // Slave address and RW bit
  bool                  pending;            // Transfer pending (no STOP)
  uint8_t               stalled;            // Stall mode status flags
  uint8_t               con_aa;             // I2C slave CON flag
  int32_t               cnt;                // Master transfer count
  uint8_t              *data;               // Master data to transfer
  uint32_t              num;                // Number of bytes to transfer
  uint8_t              *sdata;              // Slave data to transfer
  uint32_t              snum;               // Number of bytes to transfer
} I2C_CTRL;

typedef const struct _I2C_PIN {
  uint8_t port;                                        ///< CAN GPIO port
  uint8_t pin;                                         ///< CAN GPIO pin
  uint8_t mode;                                        ///< CAN GPIO mode
  uint8_t pad_sel;                                     ///< CAN GPIO pad selection
}I2C_PIN;
 
typedef const struct _I2C_IO {
	I2C_PIN      *scl;            
	I2C_PIN      *sda;                     
} I2C_IO; 

/* I2C Resource Configuration */
typedef struct 
{
  I2C0_Type             *reg;               // I2C register interface
  IRQn_Type             i2c_ev_irq;         // I2C Event IRQ Numbe
  I2C_CTRL              *ctrl;              // Run-Time control information
  I2C_IO                 io; //pins
} I2C_RESOURCES;

#endif

