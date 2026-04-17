/******************************************************************************
* @file  sl_si91x_hspi_secondary.h
* @brief High Speed SPI Secondary API implementation
*******************************************************************************
* # License
* <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef __SL_SI91X_HSPI_SECONDARY_H_
#define __SL_SI91X_HSPI_SECONDARY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_si91x_peripheral_hspi_secondary.h"
#include "sl_si91x_hspi_secondary_drv_config.h"
#include "rsi_rom_gpdma.h"
#include "rsi_rom_clks.h"
#include "sl_status.h"

/***************************************************************************/
/**
 * @addtogroup HSPI High Speed SPI Secondary
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 *
 *******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#define SL_HSPI_NUM_GPDMA_DESC 30 ///< Number of GPDMA Descriptors

#define SL_HSPI_IRQHandler HIF1_IRQHandler ///< HIF1 interrupt handler.

#define SL_GPDMA_IRQHandler IRQ031_Handler ///< GPDMA interrupt handler.

/// HSPI Write FIFO Data Register.
#define SL_HSPI_RX_SOURCE_ADDR 0x20200380 ///< HSPI write FIFO Data Register.

#define SL_HSPI_TX_SOURCE_ADDR 0x202003C0 ///< HSPI Read FIFO Data Register.

/*******************************************************************************
 *****************************   DATA TYPES   *********************************
 ******************************************************************************/

/**

 * @brief Structure to hold the version numbers of the HSPI secondary API.
 * 
 * This structure defines the version numbers for the HSPI secondary API, including
 * the release, major, and minor version numbers.
 */
typedef struct {
  uint8_t release; ///< Release version number.
  uint8_t major;   ///< Major version number.
  uint8_t minor;   ///< Minor version number.
} sl_hspi_secondary_version_t;

/***************************************************************************/
/**
 * @brief Callback function for HSPI secondary interface events.
 *
 * This function pointer defines the prototype for a callback function that will be invoked 
 * when HSPI secondary interface events occur.
 *
 * @param[in] events The HSPI secondary interface events that triggered the callback.
 *                   This parameter is a bitmask representing one or more events.
 ******************************************************************************/
typedef void (*sl_hspi_secondary_callback_t)(uint8_t events);

/***************************************************************************/
/**
 * @brief Callback function for HSPI secondary interface GPDMA transfers.
 *
 * This function pointer defines the prototype for a callback function that will be invoked 
 * when HSPI secondary interface GPDMA transfer completes.
 *
 * @param[in] dma_ch The GPDMA channel used for the transfer.
 ******************************************************************************/
typedef void (*sl_hspi_secondary_gpdma_callback_t)(uint8_t dma_ch);

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************/
/**
 * @brief Initializes the HSPI secondary.
 * 
 * @pre Pre-condition:
 *      - The system clock must be configured.
 *      - The HSPI peripheral clock must be enabled.
 * 
 * @return sl_status_t Initialization status:
 *         - SL_STATUS_OK  - Initialization successful.
 *         - SL_STATUS_FAIL  - Initialization failed.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_hspi_secondary_init(void);

/***************************************************************************/
/**
 * @brief De-Initializes the HSPI secondary.
 *
 * @pre Pre-condition:
 *      - The system clock must be configured.
 *      - The HSPI peripheral clock must be enabled.
 *
 ******************************************************************************/
sl_status_t sl_si91x_hspi_secondary_deinit(void);

/***************************************************************************/
/**
 * @brief Sends data from the HSPI secondary to the host/primary device in non-blocking mode.
 *
 * @pre Pre-condition:
 *      - \ref sl_si91x_hspi_secondary_init must be called prior.
 *      - The GPDMA must be configured and enabled.
 *
 * @param[in] data_buf Pointer to the source data buffer.
 * @return sl_status_t Status of the send operation.
 ******************************************************************************/
sl_status_t sl_si91x_hspi_secondary_send_non_blocking(void *data_buf);

/***************************************************************************/
/**
 * @brief Receives data on the HSPI secondary in non-blocking mode from the host/primary device.
 *
 * @pre Pre-condition:
 *      - \ref sl_si91x_hspi_secondary_init must be called prior.
 *      - The GPDMA must be configured and enabled.
 *
 * @param[in] data_buf Reference to the destination buffer where the received data will be stored.
 * @return sl_status_t Status of the receive operation.
 ******************************************************************************/
sl_status_t sl_si91x_hspi_secondary_receive_non_blocking(void *data_buf);

/*******************************************************************************/
/**
 * @brief Writes data into HSPI FIFO in blocking mode.
 *
 * @pre Pre-condition:
 *      - \ref sl_si91x_hspi_secondary_init must be called prior.
 *
 * @param[in] data Data to be written in HSPI FIFO register.
 * @return Number of bytes written in HSPI FIFO register.
 ******************************************************************************/
uint32_t sl_si91x_hspi_secondary_send_blocking(const void *data);

/*******************************************************************************/
/**
 * 
 * @brief Receives data from SPI FIFO in blocking mode.
 *
 * @pre Pre-condition:
 *      - \ref sl_si91x_hspi_secondary_init must be called prior.
 *
 * @param[in] data Data pointer in which read data from HSPI FIFO register is copied.
 * @return The number of bytes read from SPI FIFO register.
 ******************************************************************************/
uint32_t sl_si91x_hspi_secondary_receive_blocking(void *data);

/***************************************************************************/
/**
 * @brief Registers the HSPI secondary user callback function.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_hspi_secondary_init must be called prior.
 * 
 * @param[in] callback_event Pointer to the function which needs to be called at the time of interrupt.
 * @return sl_status_t Registration status:
 *         - SL_STATUS_OK  - Success.
 *         - SL_STATUS_BUSY  - Driver is busy.
 *         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_hspi_secondary_register_event_callback(sl_hspi_secondary_callback_t callback_event);

/***************************************************************************/
/**
 * @brief Unregisters the HSPI secondary user callback function.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_hspi_secondary_init must be called prior.
 *      - \ref sl_si91x_hspi_secondary_register_event_callback must be called prior.
 * 
 ******************************************************************************/
void sl_si91x_hspi_secondary_unregister_event_callback(void);

/***************************************************************************/
/**
 * @brief Retrieves the HSPI Secondary Driver version.
 * 
 * @pre Pre-condition:
 *      - None.
 * 
 * @return sl_hspi_secondary_version_t Driver version.
 ******************************************************************************/
sl_hspi_secondary_version_t sl_si91x_hspi_secondary_get_version(void);

/** @} (end addtogroup High Speed SPI Secondary) */

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/** 
* @addtogroup  HSPI High Speed SPI Secondary
* @{
*   @details
*
*   @section HSPI_Intro Introduction
*
* HSPI demonstrates the HSPI Secondary transmitting and receiving data
* to and from the external SPI primary.
*
* @section HSPI_Config Configuration
*
* - The HSPI primary side drives the majority of HSPI configurations, specifically initialization, speed mode, and all data transfer operations (read, write, read-write).
*
* - For more information on configuring available parameters, see the respective HSPI Primary example.
*
* @section HSPI_Usage Usage
*
* - The HSPI Secondary undergoes initialization and configuration, and then enters a waiting state. 
*  It remains in this state until an interrupt from the primary triggers data transfer,
*  at which point the send and receive APIs configure the GPDMA descriptors for further transmissions.
*
* The HSPI can be initialized by multiple functions:
*
* 1. @ref sl_si91x_hspi_secondary_init : To initialize the HSPI secondary.
* 2. @ref sl_si91x_hspi_secondary_register_event_callback : To register the HSPI secondary user callback function.
* 3. @ref sl_si91x_hspi_secondary_send_non_blocking : To trigger the sequence to send data from the HSPI secondary in non-blocking mode to the host/primary device.
* 4. @ref sl_si91x_hspi_secondary_receive_non_blocking : To receive data on the HSPI secondary in non-blocking mode from the host/primary device using GPDMA.
*
* @} end group HSPI ********************************************************/

#ifdef __cplusplus
}
#endif

#endif //__SL_SI91X_HSPI_SECONDARY_H_