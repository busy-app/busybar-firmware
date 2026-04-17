/***************************************************************************//**
 * @file sl_wisun_rf_test.h
 * @brief Wi-SUN RF test API
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_WISUN_RF_TEST_H
#define SL_WISUN_RF_TEST_H

#include "sl_status.h"

/**************************************************************************//**
 * @addtogroup SL_WISUN_RF_TEST RF Test
 * @{
 *****************************************************************************/

/**************************************************************************//**
 * Start transmitting a random stream of characters to enable
 * the measurement of radio modulation.
 *
 * @param[in] channel Index of the physical channel to transmit on
 * @return One of the following:
 *  - SL_STATUS_OK if the stream transmission started successfully.
 *  - SL_STATUS_NOT_READY if called before the stack initialization.
 *  - SL_STATUS_BUSY if a test is already running.
 *  - SL_STATUS_NETWORK_UP if a connection is already established or in progress.
 *  - SL_STATUS_INVALID_PARAMETER if an invalid channel is configured.
 *
 * Transmit a PN9 bytes sequence. See sl_rail_start_tx_stream() for more
 * information.
 *****************************************************************************/
sl_status_t sl_wisun_start_stream(uint16_t channel);


/**************************************************************************//**
 * Stop a previously started stream of characters.
 *
 * @return One of the following:
 *  - SL_STATUS_OK if the stream transmission stopped successfully.
 *  - SL_STATUS_INVALID_STATE if while not transmitting a stream.
 *
 * See sl_rail_stop_tx_stream() for more information.
 *****************************************************************************/
sl_status_t sl_wisun_stop_stream();

/**************************************************************************//**
 * Start transmitting an unmodulated tone.
 *
 * @param[in] channel Index of the physical channel to transmit on
 * @return One of the following:
 *  - SL_STATUS_OK if the stream transmission started successfully.
 *  - SL_STATUS_NOT_READY if called before the stack initialization.
 *  - SL_STATUS_BUSY if a test is already running.
 *  - SL_STATUS_NETWORK_UP if a connection is already established or in progress.
 *  - SL_STATUS_INVALID_PARAMETER if an invalid channel is configured.
 *
 * Transmit a PN9 bytes sequence. See sl_rail_start_tx_stream() for more
 * information.
 *****************************************************************************/
sl_status_t sl_wisun_start_tone(uint16_t channel);

/**************************************************************************//**
 * Stop a previously started tone.
 *
 * @return One of the following:
 *  - SL_STATUS_OK if the tone stopped successfully.
 *  - SL_STATUS_INVALID_STATE if while not transmitting a tone.
 *
 * See sl_rail_stop_tx_stream() for more information.
 *****************************************************************************/
sl_status_t sl_wisun_stop_tone();

/**************************************************************************//**
 * Set transmit power.
 *
 * @param[in] tx_power Transmit power in units of dBm, can be negative.
 * @return always SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wisun_set_test_tx_power(int8_t tx_power);

/**************************************************************************//**
 * Must be called before sl_wisun_rf_test_start_tx(), sl_wisun_rf_test_start_rx(),
 * sl_wisun_start_stream() and sl_wisun_start_tone().
 * Sets the phy configuration for the subsequent RF test operations.
 *
 * @param[in] phy_config Pointer to PHY configuration structure
 * @return One of the following:
 *  - SL_STATUS_OK if the PHY configuration was set successfully.
 *  - SL_STATUS_NOT_READY if called before the stack initialization.
 *  - SL_STATUS_BUSY if a test is already running.
 *  - SL_STATUS_NETWORK_UP if a connection is already established or in progress.
 *  - SL_STATUS_NOT_FOUND if the PHY configuration was not found.
 *****************************************************************************/
sl_status_t sl_wisun_rf_test_set_phy_config(sl_wisun_phy_config_t *phy_config);

/**************************************************************************//**
 * Return the current status of the RF test plugin.
 *
 * @return One of the following:
 *  - True if a test is running.
 *  - False otherwise.
 *****************************************************************************/
bool sl_wisun_is_running_rf_test();

/**************************************************************************//**
 * Start an RF test packet transmission sequence.
 *
 * This API schedules repeated packet transmissions on a fixed channel using
 * the currently configured RF test PHY (see sl_wisun_rf_test_set_phy_config()).
 * The packets are sent at a constant interval until the total number of
 * transmissions is reached.
 * If @p cca_enabled is true, a single CCA check is performed before the
 * initial transmit using the default single-CCA CSMA settings.
 * To receive the packets, another application can use sl_wisun_rf_test_start_rx() on the same channel.
 *
 * @param[in] channel Index of the physical channel to transmit on
 * @param[in] count Number of packets to transmit (must greater than 0)
 * @param[in] data_length Length of data in each packet (must be between 1 and 2047 - CRC length, typically 2045 for FSK, 2043 for OFDM or OQPSK)
 * @param[in] data Pointer to data to transmit; if NULL a default ramp pattern is used
 * @param[in] interval Interval between transmissions in milliseconds (must be greater than 0)
 * @param[in] cca_enabled Set true to perform a single CCA check before the initial transmit
 * @return One of the following:
 *  - SL_STATUS_OK if the transmission started successfully.
 *  - SL_STATUS_NOT_READY if called before the stack initialization.
 *  - SL_STATUS_BUSY if a test is already running.
 *  - SL_STATUS_NETWORK_UP if a connection is already established or in progress.
 *  - SL_STATUS_INVALID_PARAMETER if an invalid parameter is provided.
 *  - SL_STATUS_NOT_SUPPORTED if the feature is not implemented.
 *****************************************************************************/
sl_status_t sl_wisun_rf_test_start_tx(uint16_t channel,
                                uint16_t count,
                                uint16_t data_length,
                                uint8_t *data,
                                uint32_t interval,
                                bool cca_enabled);

/**************************************************************************//**
 * Start RF test RX reception on a fixed channel.
 *
 * This API puts the radio into continuous RX on the given channel for the
 * requested duration. If duration is 0, RX continues until explicitly stopped.
 *
 * @param[in] channel Index of the physical channel to transmit on
 * @param[in] duration Duration in milliseconds (0 = run until stopped)
 * @return One of the following:
 *  - SL_STATUS_OK if the RX test started successfully.
 *  - SL_STATUS_NOT_READY if called before the stack initialization.
 *  - SL_STATUS_BUSY if a test is already running.
 *  - SL_STATUS_NETWORK_UP if a connection is already established or in progress.
 *  - SL_STATUS_INVALID_PARAMETER if an invalid parameter is provided.
 *  - SL_STATUS_NOT_SUPPORTED if the feature is not implemented.
 *****************************************************************************/
sl_status_t sl_wisun_rf_test_start_rx(uint16_t channel, uint32_t duration);

/**************************************************************************//**
 * Stop RF test RX reception.
 *
 * This API ends an active RF test RX session started with sl_wisun_rf_test_start_rx().
 *
 * @return One of the following:
 *  - SL_STATUS_OK if the RX test stopped successfully.
 *  - SL_STATUS_INVALID_STATE if not currently receiving.
 *  - SL_STATUS_NOT_SUPPORTED if the feature is not implemented.
 *****************************************************************************/
sl_status_t sl_wisun_rf_test_rx_stop(void);

/**************************************************************************//**
 * Handles RF events generated during RF tests.
 *
 * This callback updates the RF test state based on the RAIL event and should
 * be called when an event of type SL_WISUN_LOGGER_EVENT_TYPE_RF_TEST is received by the application.
 *
 * @param[in] events RAIL event mask associated with the RF test event
 * @param[in] rssi   RSSI in dBm; valid when SL_RAIL_EVENT_RX_PACKET_RECEIVED is set in events, otherwise SL_WISUN_RF_TEST_RSSI_NOT_AVAILABLE
 *****************************************************************************/
void sl_wisun_rf_test_event_callback(uint64_t events, int8_t rssi);
/** @} (end addtogroup SL_WISUN_RF_TEST) */

#endif // SL_WISUN_RF_TEST_H
