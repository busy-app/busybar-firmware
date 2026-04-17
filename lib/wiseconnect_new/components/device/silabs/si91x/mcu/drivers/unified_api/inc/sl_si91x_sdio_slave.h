/******************************************************************************
* @file  sl_si91x_sdio_slave.h
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
#ifndef __SL_SI91X_SDIO_SLAVE_H_
#define __SL_SI91X_SDIO_SLAVE_H_

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#define SDIO_MODE_SELECT    0x0705
#define MASK_HOST_INTERRUPT 0xF0

#define M4_MISC_CONFIG_BASE 0x46008000
#define SDIO_BASE           0x20200000

#define M4_HOST_INTR_MASK_REG   (*(volatile uint32_t *)(M4_MISC_CONFIG_BASE + 0x00))
#define M4_HOST_INTR_STATUS_REG (*(volatile uint32_t *)(M4_MISC_CONFIG_BASE + 0x04))
#define M4_HOST_INTR_CLEAR      (*(volatile uint32_t *)(M4_MISC_CONFIG_BASE + 0x08))
#define MISC_CFG_HOST_CTRL      (*(volatile uint32_t *)(M4_MISC_CONFIG_BASE + 0x0C))

#define SDIO_FN1_INT_CLEAR      (*(volatile uint32_t *)(SDIO_BASE + 0x00))
#define SDIO_FN1_INT_ENABLE     (*(volatile uint32_t *)(SDIO_BASE + 0x04))
#define SDIO_FN1_INT_MASK_REG   (*(volatile uint16_t *)(SDIO_BASE + 0x08))
#define SDIO_FN1_INT_UNMASK_REG (*(volatile uint32_t *)(SDIO_BASE + 0x0C))
#define SDIO_BLK_LEN_REG        (*(volatile uint32_t *)(SDIO_BASE + 0x10))
#define SDIO_BLK_CNT            (*(volatile uint16_t *)(SDIO_BASE + 0x14))
#define SDIO_ADDRESS_REG        (*(volatile uint16_t *)(SDIO_BASE + 0x18))
#define RX_NUM_CHUNKS           (*(volatile uint32_t *)(SDIO_BASE + 0x242))

#define NUMGPDMADESC 10

/// SDIO Write FIFO Data Register
#define RX_SOURCE_ADDR 0x20200080

/// SDIO Read FIFO Data Register
#define TX_SOURCE_ADDR 0x20200040

/// Events for HIF irq handler
#define HOST_INTR_RECEIVE_EVENT BIT(0)

/// Bits masks for registers
#define INT_STAT_TX_TMOUT  BIT(15)
#define INT_STAT_ABRT_SPI  BIT(14)
#define INT_STAT_ABRT_SDIO BIT(6)
#define INT_STAT_CRC_SDIO  BIT(11)
#define INT_STAT_CMD53_RD  BIT(1)
#define INT_STAT_CMD53_WR  BIT(0)
#define INT_STAT_WR_RD_REQ BIT(8)
#define INT_EN_CMD53_WR    BIT(0)
#define INT_STAT_CMD53_WR  BIT(0)

/*******************************************************************************
 *****************************   DATA TYPES   *********************************
 ******************************************************************************/

/**
 * @brief Enumeration for SDIO slave receive interrupt status.
 * 
 * This enumeration defines the possible statuses for SDIO slave receive interrupts,
 * indicating whether an interrupt from the host has been received or not.
 */
typedef enum {
  HOST_INTR_NOT_RECEIVED = 0, ///< Host interrupt not received.
  HOST_INTR_RECEIVED     = 1, ///< Host interrupt received.
} sl_sdio_slave_rx_intr_status_t;

/**
 * @brief Callback function type for SDIO slave interface events.
 * 
 * This typedef defines the prototype for a callback function that will be invoked
 * when SDIO slave interface events occur.
 * 
 * @param[in] events The SDIO slave interface events that triggered the callback.
 *                   This parameter is a bitmask representing one or more events.
 */
typedef void (*sl_sdio_slave_callback_t)(uint8_t events);

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************/
/**
 * @brief To initialize the SDIO slave peripheral for transfers.
 * 
 * @details This API initializes the SDIO slave peripheral and sets up the callback function
 * for handling host interrupt events. The callback function will be called whenever an
 * interrupt event occurs from the host.
 * 
 * @param[in] event_callback
 *   Address of the application callback function for host interrupt events.
 ******************************************************************************/
void sl_si91x_sdio_slave_init(sl_sdio_slave_callback_t event_callback);

/***************************************************************************/
/**
 * @brief To trigger the sequence to send data from SDIO slave to host/master device.
 * 
 * @details This API initiates the transfer of data from the SDIO slave to the host/master device.
 * The data is sent in blocks, where each block has a predefined length (1-2048 bytes).
 *
 * @pre Pre-conditions:
 *      - @see sl_si91x_sdio_slave_init must be called prior.
 * 
 * @param[in] num_of_blocks Number of blocks to be sent.
 * @param[in] data_buf Reference to the source buffer containing the data to be sent.
 ******************************************************************************/
void sl_si91x_sdio_slave_send(uint8_t num_of_blocks, uint8_t *data_buf);

/***************************************************************************/
/**
 * @brief To receive data on SDIO slave from host/master device.
 * 
 * @details This API triggers the sequence to receive data from the host/master device
 * and stores it in the provided destination buffer.
 * 
 * @pre Pre-conditions:
 *      - @see sl_si91x_sdio_slave_init must be called prior.
 * @param[in] data_buf
 *   Reference to the destination buffer where the received data will be stored.
 ******************************************************************************/
void sl_si91x_sdio_slave_receive(uint8_t *data_buf);

#endif //__SL_SI91X_SDIO_SLAVE_H_