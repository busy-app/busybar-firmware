/***************************************************************************//**
 * @file sl_iostream_ble_spp.h
 * @brief I/O Stream BLE Serial Port Profile
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

#ifndef SL_IOSTREAM_BLE_SPP_H
#define SL_IOSTREAM_BLE_SPP_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>

#include "sl_iostream.h"
#include "sl_iostream_handles.h"
#include "sl_status.h"
#include "sl_enum.h"
#include "sl_bluetooth.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
// Thread loop definition
#ifndef SL_IOSTREAM_BLE_SPP_THREAD_LOOP
  #define SL_IOSTREAM_BLE_SPP_THREAD_LOOP while (1)
#endif

// I/O Stream BLE SPP modes
SL_ENUM(sl_iostream_ble_spp_mode_t) {
  // SPP mode off
  SL_IOSTREAM_BLE_SPP_MODE_OFF = 0U,
  // SPP mode on
  SL_IOSTREAM_BLE_SPP_MODE_ON
};
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
// Global I/O Stream handle for SPP
extern sl_iostream_t *iostream_spp_handle;
// Global I/O Stream instance info for SPP
extern sl_iostream_instance_info_t iostream_instance_spp_info;
// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @brief Initialize the I/O Stream BLE SPP module.
 * @details This function initializes the I/O Stream BLE SPP module.
 *****************************************************************************/
void sl_iostream_ble_spp_init(void);

/**************************************************************************//**
 * @brief Callback for BLE SPP I/O Stream module.
 * @details This callback function initializes the BLE SPP I/O Stream.
 *
 * @return SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_iostream_spp_init_cb(void);

/**************************************************************************//**
 * @brief Save BLE connection handle.
 * @details This function saves the BLE connection handle.
 *
 * @param[in] conn The BLE connection handle.
 *****************************************************************************/
void sl_iostream_ble_spp_save_connection_handle(const uint8_t conn);

/**************************************************************************//**
 * @brief Save SPP mode.
 * @details This function saves the SPP mode.
 *
 * @param[in] mode The SPP mode.
 *****************************************************************************/
void sl_iostream_ble_spp_save_mode(const sl_iostream_ble_spp_mode_t mode);

/**************************************************************************//**
 * @brief Get BLE connection handle.
 * @details This function gets the BLE connection handle.
 *
 * @return The BLE connection handle.
 *****************************************************************************/
uint8_t sl_iostream_ble_spp_get_connection_handle(void);

/**************************************************************************//**
 * @brief Get SPP mode.
 * @details This function gets the SPP mode.
 *
 * @param[out] mode The SPP mode.
 *
 * @return SL_STATUS_OK if successful, error code otherwise
 *****************************************************************************/
sl_status_t sl_iostream_ble_spp_get_mode(sl_iostream_ble_spp_mode_t * const mode);

/**************************************************************************//**
 * @brief RX FIFO write.
 * @details This function writes data to the RX FIFO.
 *
 * @param[in] data The data to write.
 * @param[in] size The size of the data.
 *
 * @return SL_STATUS_OK if successful, error code otherwise
 *****************************************************************************/
sl_status_t sl_iostream_ble_spp_rx_fifo_write(const uint8_t * const data, const size_t size);

/**************************************************************************//**
 * @brief RX FIFO read.
 * @details This function reads data from the RX FIFO.
 *
 * @param[out] data The data read.
 * @param[out] size The size of the data.
 *
 * @return SL_STATUS_OK if successful, error code otherwise
 *****************************************************************************/
sl_status_t sl_iostream_ble_spp_rx_fifo_read(uint8_t *const data, size_t * const size);

/**************************************************************************//**
 * @brief TX FIFO write.
 * @details This function writes data to the TX FIFO.
 *
 * @param[in] data The data to write.
 * @param[in] size The size of the data.
 *
 * @return SL_STATUS_OK if successful, error code otherwise
 *****************************************************************************/
sl_status_t sl_iostream_ble_spp_tx_fifo_write(const uint8_t *data, const size_t size);

/**************************************************************************//**
 * @brief TX FIFO read.
 * @details This function reads data from the TX FIFO.
 *
 * @param[out] data The data read.
 * @param[out] size The size of the data.
 *
 * @return SL_STATUS_OK if successful, error code otherwise
 *****************************************************************************/
sl_status_t sl_iostream_ble_spp_tx_fifo_read(uint8_t * const data, size_t * const size);

/***************************************************************************//**
 * Callback for Bluetooth events
 *
 * @param[in] evt event coming from the Bluetooth stack
 ******************************************************************************/
void sl_iostream_ble_spp_on_event(sl_bt_msg_t *evt);

#ifdef __cplusplus
}
#endif

#endif // SL_IOSTREAM_BLE_SPP_H
