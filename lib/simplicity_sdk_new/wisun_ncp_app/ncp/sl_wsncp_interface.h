/***************************************************************************//**
 * @file sl_wsncp_interface.h
 * @brief Wi-SUN NCP interface function declarations
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef SL_WSNCP_INTERFACE_H
#define SL_WSNCP_INTERFACE_H

#include <stdint.h>
#include "sl_status.h"

/***************************************************************************//**
 * Initialize the interface layer
 *
 * This function is implemented by each interface layer.
 *
 * @retval SL_STATUS_OK Interface initialized successfully
 * @retval Other sl_status_t if error occurred
 ******************************************************************************/
sl_status_t sl_wsncp_interface_init(void);

/***************************************************************************//**
 * Deinitialize the interface layer
 *
 * This function is implemented by each interface layer.
 *
 * @retval SL_STATUS_OK Interface deinitialized successfully
 * @retval Other sl_status_t if error occurred
 ******************************************************************************/
sl_status_t sl_wsncp_interface_deinit(void);

/***************************************************************************//**
 * Transmit data through the interface layer
 *
 * This function is implemented by each interface layer and is called by the
 * NCP task to transmit data.
 *
 * @param[in] len Length of data to transmit
 * @param[in] data Pointer to data buffer
 * @retval SL_STATUS_OK Transmission started successfully
 * @retval Other sl_status_t if error occurred
 ******************************************************************************/
sl_status_t sl_wsncp_interface_transmit(uint16_t len, void *data);

/***************************************************************************//**
 * Callback function called when transmission is complete
 *
 * This function is implemented by the NCP task and is called by the interface
 * layer to notify the NCP task that a transmission has completed.
 *
 * @param[in] status Status of the transmission operation
 ******************************************************************************/
void sl_wsncp_on_transmit_complete_cb(sl_status_t status);

/***************************************************************************//**
 * Callback function called when data is received
 *
 * This function is implemented by the NCP task and is called by the interface
 * layer to notify the NCP task that data has been received.
 *
 * @param[in] status Status of the receive operation
 * @param[in] len Length of received data
 * @param[in] data Pointer to received data buffer
 ******************************************************************************/
void sl_wsncp_on_receive_cb(sl_status_t status, uint16_t len, void *data);

/***************************************************************************//**
 * Callback function called on receive timeout
 *
 * This function is implemented by the NCP task and is called by the interface
 * layer to notify the NCP task that a receive timeout has occurred. Given the
 * information provided, the NCP task will determine if enough data has been
 * received or if the timeout has been exceeded.
 *
 * @param[in] timeout_count Number of timeouts that have occurred
 * @param[in] len Length of data received so far
 * @param[in] data Pointer to received data buffer
 * @retval 1 if enough data has been received or timeout exceeded
 * @retval 0 if more data should be waited for
 ******************************************************************************/
int sl_wsncp_on_timeout_cb(uint32_t timeout_count, uint16_t len, void *data);

#endif // SL_WSNCP_INTERFACE_H 