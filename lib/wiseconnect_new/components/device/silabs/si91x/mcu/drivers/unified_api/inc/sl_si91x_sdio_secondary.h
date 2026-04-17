/******************************************************************************
* @file  sl_si91x_sdio_secondary.h
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
#ifndef __SL_SI91X_SDIO_SECONDARY_H_
#define __SL_SI91X_SDIO_SECONDARY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"

/***************************************************************************/
/**
 * @addtogroup SDIO SDIO Secondary
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{
 *
 *******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#define NUMGPDMADESC 10 ///< Number of GPDMA Descriptors

/// Events for HIF irq handler
#define HOST_INTR_RECEIVE_EVENT BIT(0) ///< Host interrupt receive event.
#define HOST_INTR_SEND_EVENT    BIT(1) ///< Host interrupt send event.
#define HOST_INTR_CMD52_EVENT   BIT(2) ///< Host interrupt CMD52 event.
#define HOST_INTR_ERROR_EVENT   BIT(3) ///< Host interrupt error event.

/*******************************************************************************
 *****************************   DATA TYPES   *********************************
 ******************************************************************************/

/**
 * @brief Enumeration for SDIO slave receive interrupt status.
 * 
 * This enumeration defines the possible statuses for SDIO secondary receive interrupts,
 * indicating whether an interrupt from the host has been received or not.
 */
typedef enum {
  HOST_INTR_NOT_RECEIVED = 0, ///< Host interrupt not received.
  HOST_INTR_RECEIVED     = 1, ///< Host interrupt received.
} sl_sdio_slave_rx_intr_status_t;

/**
 * @brief Structure to hold the version numbers of the SDIO secondary API.
 * 
 * This structure defines the version numbers for the SDIO secondary API, including
 * the release, major, and minor version numbers.
 */
typedef struct {
  uint8_t release; ///< Release version number.
  uint8_t major;   ///< Major version number.
  uint8_t minor;   ///< Minor version number.
} sl_sdio_secondary_version_t;

/***************************************************************************/
/**
 * @brief Callback function for SDIO secondary interface events.
 *
 * This function pointer defines the prototype for a callback function that will be invoked 
 * when SDIO secondary interface events occur.
 *
 * @param[in] events The SDIO secondary interface events that triggered the callback.
 *                   This parameter is a bitmask representing one or more events.
 ******************************************************************************/
typedef void (*sl_sdio_secondary_callback_t)(uint8_t events);

/***************************************************************************/
/**
 * @brief Callback function for SDIO secondary interface GPDMA transfers.
 *
 * This function pointer defines the prototype for a callback function that will be invoked 
 * when SDIO secondary interface GPDMA transfer completes.
 *
 * @param[in] dma_ch The GPDMA channel used for the transfer.
 ******************************************************************************/
typedef void (*sl_sdio_secondary_gpdma_callback_t)(uint8_t dma_ch);

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************/
/**
 * @brief To initialize the SDIO secondary.
 * 
 * @details This API initializes the SDIO secondary interface. By default, the SDIO pin access is with NWP.
 * This API gives the pin access to M4, selects the SDIO mode, and enables the SDIO primary interrupts.
 * 
 * @return sl_status_t Initialization status:
*         - SL_STATUS_OK  - Initialization successful.
*         - SL_STATUS_FAIL  - Initialization failed.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sdio_secondary_init(void);

/***************************************************************************/
/**
 * @brief To trigger the sequence to send data from the SDIO secondary in non-blocking mode to the host/primary device.
 * 
 * @details This API accepts the number of blocks, where one block will have a block length
 *          (1 to 1024) bytes, and a pointer to the data buffer to be transferred.
 * 
 * @param[in] num_of_blocks Number of blocks to be sent.
 * @param[in] data_buf Reference to the source buffer.
 ******************************************************************************/
void sl_si91x_sdio_secondary_send(uint8_t num_of_blocks, uint8_t *data_buf);

/***************************************************************************/
/**
 * @brief To receive data on the SDIO secondary in non-blocking mode from the host/primary device using GPDMA.
 * 
 * @details This API receives data from the host/primary device and stores it in the provided destination buffer.
 * The transfer is done in non-blocking mode using GPDMA.
 * 
 * @param[in] data_buf Reference to the destination buffer where the received data will be stored.
 ******************************************************************************/
void sl_si91x_sdio_secondary_receive(uint8_t *data_buf);

/***************************************************************************/
/**
 * @brief To request permission to send data to the host/primary device.
 * 
 * @details This API signals the host/primary device that the SDIO secondary is ready
 *          to send data. It raises an interrupt to notify the host that data is available
 *          for transmission.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_sdio_secondary_init must be called prior.
 * 
 * @note Call this function before calling \ref sl_si91x_sdio_secondary_send
 *       to explicitly notify the host that data is ready for transmission.
 ******************************************************************************/
void sl_si91x_sdio_secondary_request_to_send(void);

/***************************************************************************/
/**
 * @brief To register the SDIO secondary user callback function.
 * 
 * @details This API registers a user-defined callback function for the SDIO secondary interface.
 * The callback function will be called when an interrupt occurs, based on the specified interrupt flag.
 * 
 * @pre Pre-condition:
 *      - \ref sl_si91x_sdio_secondary_init must be called prior.
 * 
 * @param[in] callback_event Pointer to the function which needs to be called at the time of interrupt.
 * @param[in] flag Interrupt flag to be registered.
 * 
 * @return sl_status_t Registration status:
*         - SL_STATUS_OK  - Success.
*         - SL_STATUS_BUSY  - Driver is busy.
*         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sdio_secondary_register_event_callback(sl_sdio_secondary_callback_t callback_event, uint32_t flag);

/***************************************************************************/
/**
 * @brief To unregister the SDIO secondary user callback function.
 * 
 * @details This API unregisters a user-defined callback function for the SDIO secondary interface.
 * The callback function will no longer be called when an interrupt occurs for the specified interrupt flag.
 *
 * @pre Pre-condition:
 *      - \ref sl_si91x_sdio_secondary_init must be called prior.
 *      - \ref sl_si91x_sdio_secondary_register_event_callback must be called prior.
 * 
 * @param[in] flag Interrupt flag to be unregistered.
 ******************************************************************************/
void sl_si91x_sdio_secondary_unregister_event_callback(uint32_t flag);

/***************************************************************************/
/**
 * @brief To register the SDIO secondary GPDMA user callback function.
 * 
 * @details This API registers a user-defined callback function for the SDIO secondary interface.
 * The callback function will be called when a GPDMA interrupt occurs.
 * 
 * @pre Pre-conditions:
 *      - \ref sl_si91x_sdio_secondary_init must be called prior.
 * 
 * @param[in] callback_event Pointer to the function which needs to be called at the time of interrupt.
 * 
 * @return sl_status_t Registration status:
*         - SL_STATUS_OK  - Success.
*         - SL_STATUS_BUSY  - Driver is busy.
*         - SL_STATUS_NULL_POINTER  - The parameter is a null pointer.
 * 
 * For more information on status codes, see [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_sdio_secondary_gpdma_register_event_callback(sl_sdio_secondary_gpdma_callback_t callback_event);

/***************************************************************************/
/**
 * @brief To unregister the SDIO secondary GPDMA user callback function.
 * 
 * @details This API unregisters a user-defined callback function for the SDIO secondary interface.
 * The callback function will no longer be called when a GPDMA interrupt occurs.
 ******************************************************************************/
void sl_si91x_sdio_secondary_gpdma_unregister_event_callback(void);

/***************************************************************************/
/**
 * @brief To get the SDIO Secondary Driver version.
 * 
 * @details This function retrieves the version of the SDIO Secondary Driver.
 * It can be used to verify compatibility and ensure the correct version of the driver is being used.
 * 
 * @return sl_sdio_secondary_version_t Driver version.
 ******************************************************************************/
sl_sdio_secondary_version_t sl_si91x_sdio_secondary_get_version(void);

/** @} (end addtogroup SDIO Secondary) */

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY !***********************
/** 
* @addtogroup  SDIO SDIO Secondary
* @{
*   @details
*
*   @section SDIO_Intro Introduction
*
* Secure Digital Input/Output (SDIO) is an extension of the SD (Secure Digital) card
* standard that adds support for input and output functions.
*
* SDIO demonstrates the SDIO Secondary transmitting and receiving data
* to and from the external SDIO primary.
*
* The following figure illustrates the framework for reading data in Multi-Block SDIO :
*
* @image html sdio_mblock_frame.png "SDIO Read Frame Format"
*   
* The following figure illustrates the framework for writing data in Multi-Block SDIO :
*
* @image html sdio_mblock_frame_wr.png "SDIO Write Frame Format" 
*
* - **Command Initiation**: A command (CMD) is received or generated to start.
* - **Data Acquisition**: Data is taken from different inputs (DATA0, DATA1, DATA2, DATA3).
* - **Data Processing**: The collected data undergoes various operations involving functions like Calla, CNC, Dala, Det, and E.
* - **Output Generation**: The processed data is potentially converted into a specific format (4-bit mode) and outputted.
*
* @note 
* In Non-block mode, the Block Mode bit is set to zero in CMD53. The size of the data payload is in the range of 
* 1-256 bytes (due to FIFO size restriction in SDIO Secondary) in non-block mode. The byte count for this transfer is 
* set in the command (CMD53), rather than the fixed block size.
*
* @section SDIO_Config Configuration
*
* - Most SDIO configurations take place on the primary side, where it enables read, write, and read-write operations.
*
* - For more information on configuring available parameters, see the respective peripheral example readme document.
*
* @section SDIO_Usage Usage
*
* The initialization and configuration of the SDIO Secondary will be completed, transitioning it to a waiting state.
* It will remain in this state until an interrupt is triggered from the primary. Subsequently, the send and receive APIs will
* configure all GPDMA descriptors and initiate the GPDMA to transmit and receive data. Continuous data reception will commence after two seconds.
*
* The SDIO can be initialized by multiple functions:
*
* 1. @ref sl_si91x_sdio_secondary_init : To initialize the SDIO secondary.
* 2. @ref sl_si91x_sdio_secondary_register_event_callback : To register the SDIO secondary user callback function.
* 3. @ref sl_si91x_sdio_secondary_send : To trigger the sequence to send data from the SDIO secondary in non-blocking mode to the host/primary device.
* 4. @ref sl_si91x_sdio_secondary_receive : To receive data on the SDIO secondary in non-blocking mode from the host/primary device using GPDMA.
*
* @} end group SDIO ********************************************************/

#ifdef __cplusplus
}
#endif

#endif //__SL_SI91X_SDIO_SECONDARY_H_
