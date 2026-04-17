/*
 *  Copyright (c) 2025, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file defines the radio interface for RAIL operations used across radio modules.
 */

#ifndef RADIO_INTERFACE_H
#define RADIO_INTERFACE_H

#include <openthread-core-config.h>

#include <stdbool.h>
#include <stdint.h>

#include <openthread/instance.h>
#include <openthread/platform/radio.h>

#include "platform-band.h"
#include "sl_rail_types.h"

// PHY layer constants
#if RADIO_CONFIG_SUBGHZ_SUPPORT
#define PHY_HEADER_SIZE 2
#define SHR_SIZE 6 // 4 bytes preamble, 2 bytes SFD
#else
#define PHY_HEADER_SIZE 1
#define SHR_SIZE 5 // 4 bytes of preamble, 1 byte sync-word
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the RAIL handle
 *
 * @returns The RAIL handle
 */
sl_rail_handle_t sli_ot_radio_interface_get_rail_handle(void);

/**
 * Get band configuration for a specific channel
 *
 * @param[in] aChannel  The channel number
 *
 * @returns Pointer to band configuration, or nullptr if invalid channel
 */
efr32BandConfig *sli_ot_radio_interface_get_band_config(uint8_t aChannel);

/**
 * Load RAIL configuration for a band
 *
 * @param[in] aBandConfig  The band configuration to load
 * @param[in] aTxPower     The transmit power setting
 */
void sli_ot_radio_interface_load_rail_config(efr32BandConfig *aBandConfig, int8_t aTxPower);

/**
 * Get the current band configuration
 */
efr32BandConfig *sli_ot_radio_interface_get_current_band_config(void);

/**
 * Get pointer to common configuration structure
 */
efr32CommonConfig *sli_ot_radio_interface_get_common_config_ptr(void);

/**
 * Get pointer to band configuration structure
 */
efr32BandConfig *sli_ot_radio_interface_get_band_config_ptr(void);

/**
 * Set radio to receive mode on specified channel
 *
 * @param[in] aChannel  The channel to receive on
 *
 * @returns OT_ERROR_NONE on success, error code on failure
 */
otError sli_ot_radio_interface_set_rx(uint8_t aChannel);

/**
 * Check if radio is currently transmitting or scanning
 *
 * @returns true if radio is busy, false if idle
 */
bool sli_ot_radio_interface_is_busy(void);

/**
 * Set radio to idle state
 *
 * @param[in] aRailHandle  The RAIL handle
 * @param[in] aIdleMode    The idle mode to set
 * @param[in] aWait        Whether to wait for idle state
 */
void sli_ot_radio_interface_set_idle(sl_rail_handle_t aRailHandle, sl_rail_idle_mode_t aIdleMode, bool aWait);

/**
 * Configure RAIL RX options
 *
 * @param[in] aRailHandle  The RAIL handle
 * @param[in] aOptions     The RX options to configure
 * @param[in] aMask        The option mask
 *
 * @returns SL_RAIL_STATUS_NO_ERROR on success, error code on failure
 */
sl_rail_status_t sli_ot_radio_interface_config_rx_options(sl_rail_rx_options_t aOptions, sl_rail_rx_options_t aMask);

/**
 * Get RSSI value from radio
 *
 * @param[in] aWait        Whether to wait for RSSI reading
 *
 * @returns RSSI value in quarter dBm, or SL_RAIL_RSSI_INVALID on error
 */
int16_t sli_ot_radio_interface_get_rssi(bool aWait);

/**
 * Get symbol rate from radio
 *
 * @returns Symbol rate in symbols per second
 */
uint32_t sli_ot_radio_interface_get_symbol_rate(void);

/**
 * Set RAIL multi-timer
 *
 * @param[in] aTimer          The timer to set
 * @param[in] aTime           The time value
 * @param[in] aMode           The timer mode
 * @param[in] aCallback       The timer callback function
 * @param[in] aCallbackArg    The callback argument
 *
 * @returns SL_RAIL_STATUS_NO_ERROR on success, error code on failure
 */
sl_rail_status_t sli_ot_radio_interface_set_timer(struct sl_rail_multi_timer    *aTimer,
                                                  sl_rail_time_t                 aTime,
                                                  sl_rail_time_mode_t            aMode,
                                                  sl_rail_multi_timer_callback_t aCallback,
                                                  void                          *aCallbackArg);

/**
 * Cancel RAIL multi-timer
 *
 * @param[in] aTimer       The timer to cancel
 */
void sli_ot_radio_interface_cancel_timer(struct sl_rail_multi_timer *aTimer);

/**
 * Load channel configuration
 *
 * @param[in] aChannel  The channel number
 * @param[in] aTxPower  TX power in dBm
 *
 * @returns OT_ERROR_NONE on success, error code on failure
 */
otError sli_ot_radio_interface_load_channel_config(uint8_t aChannel, int8_t aTxPower);

/**
 * Initialize radio configuration
 *
 * This function initializes the radio configuration including band config,
 * RAIL setup, and PHY stack initialization.
 */
void sli_ot_radio_interface_init_config(void);

/**
 * Deinitialize radio configuration
 *
 * This function cleans up radio configuration resources.
 */
void sli_ot_radio_interface_deinit_config(void);

/**
 * Configure CCA mode in RAIL
 *
 * @param[in] aMode  CCA mode
 *
 * @returns SL_RAIL_STATUS_NO_ERROR on success, error code on failure
 */
sl_rail_status_t sli_ot_radio_interface_rail_config_cca_mode(uint8_t aMode);

/**
 * Set CCA mode
 *
 * @param[in] aMode  CCA mode
 *
 * @returns SL_RAIL_STATUS_NO_ERROR on success, error code on failure
 */
sl_rail_status_t sli_ot_radio_interface_set_cca_mode(uint8_t aMode);

/**
 * Set TX power in RAIL
 *
 * @param[in] aTxPower  TX power in dBm
 */
void sli_ot_radio_interface_set_tx_power(int8_t aTxPower);

/**
 * Set the current band configuration pointer
 *
 * @param[in] aBandConfig  Pointer to band configuration
 */
void sli_ot_radio_interface_set_current_band_config(efr32BandConfig *aBandConfig);

/**
 * Get CCA threshold
 *
 * @returns CCA threshold in dBm
 */
int8_t sli_ot_radio_interface_get_cca_threshold(void);

/**
 * Set CCA threshold
 *
 * @param[in] aThreshold  CCA threshold in dBm
 */
void sli_ot_radio_interface_set_cca_threshold(int8_t aThreshold);

// These functions provide direct access to RAIL operations while keeping RAIL handle management centralized
// encapsulated within the radio_interface module.

// RAIL state management functions
sl_rail_status_t      sli_ot_radio_interface_rail_idle(void);
sl_rail_status_t      sli_ot_radio_interface_rail_idle_abort(void);
sl_rail_status_t      sli_ot_radio_interface_rail_yield_radio(void);
sl_rail_radio_state_t sli_ot_radio_interface_rail_get_radio_state(void);

// RAIL TX operations
sl_rail_status_t sli_ot_radio_interface_rail_start_tx(uint8_t                         channel,
                                                      sl_rail_tx_options_t            options,
                                                      const sl_rail_scheduler_info_t *scheduler);
sl_rail_status_t sli_ot_radio_interface_rail_start_cca_csma_tx(uint8_t                         channel,
                                                               sl_rail_tx_options_t            options,
                                                               const sl_rail_csma_config_t    *csmaConfig,
                                                               const sl_rail_scheduler_info_t *scheduler);
sl_rail_status_t sli_ot_radio_interface_rail_start_scheduled_cca_csma_tx(
    uint8_t                              channel,
    sl_rail_tx_options_t                 options,
    const sl_rail_scheduled_tx_config_t *scheduledConfig,
    const sl_rail_csma_config_t         *csmaConfig,
    const sl_rail_scheduler_info_t      *scheduler);
sl_rail_status_t sli_ot_radio_interface_rail_write_tx_fifo(const uint8_t *data, uint16_t length, bool reset);

// RAIL IEEE 802.15.4 operations
sl_rail_status_t sli_ot_radio_interface_rail_set_pan_id(uint16_t panId, uint8_t panIndex);
sl_rail_status_t sli_ot_radio_interface_rail_set_long_address(const uint8_t *address, uint8_t panIndex);
sl_rail_status_t sli_ot_radio_interface_rail_set_short_address(uint16_t address, uint8_t panIndex);
sl_rail_status_t sli_ot_radio_interface_rail_set_promiscuous_mode(bool enable);

// RAIL packet operations
sl_rail_rx_packet_handle_t sli_ot_radio_interface_rail_get_rx_packet_info(sl_rail_rx_packet_handle_t handle,
                                                                          sl_rail_rx_packet_info_t  *packetInfo);
sl_rail_status_t           sli_ot_radio_interface_rail_copy_rx_packet(uint8_t                        *buffer,
                                                                      const sl_rail_rx_packet_info_t *packetInfo);
sl_rail_status_t           sli_ot_radio_interface_rail_get_rx_packet_details(sl_rail_rx_packet_handle_t   handle,
                                                                             sl_rail_rx_packet_details_t *packetDetails);

// RAIL utility functions
uint32_t sli_ot_radio_interface_rail_get_symbol_rate(void);
uint32_t sli_ot_radio_interface_rail_get_bit_rate(void);
uint16_t sli_ot_radio_interface_rail_get_channel_value(void);
int16_t  sli_ot_radio_interface_rail_get_rssi(sl_rail_time_t wait_timeout_us);

// RAIL timing configuration access
uint32_t sli_ot_radio_interface_rail_get_rx_to_tx_timing(void);

// RAIL configuration
sl_rail_status_t sli_ot_radio_interface_rail_config_channels(
    const sl_rail_channel_config_t         *channels,
    sl_rail_radio_config_changed_callback_t config_changed_callback);
sl_rail_status_t sli_ot_radio_interface_rail_config_events(sl_rail_events_t events, sl_rail_events_t mask);
sl_rail_status_t sli_ot_radio_interface_rail_config_sleep(const sl_rail_timer_sync_config_t *timer_sync_config);
sl_rail_status_t sli_ot_radio_interface_rail_config_rx_options(sl_rail_rx_options_t options, sl_rail_rx_options_t mask);

// Additional RAIL power management functions
sl_rail_status_t     sli_ot_radio_interface_rail_set_tx_power_dbm(sl_rail_tx_power_t powerDbm);
sl_rail_tx_pa_mode_t sli_ot_radio_interface_rail_get_tx_pa_mode(void);
sl_rail_tx_power_t   sli_ot_radio_interface_rail_get_tx_power_dbm(void);
sl_rail_status_t     sli_ot_radio_interface_rail_config_tx_power(sl_rail_tx_pa_mode_t pa_mode);
void                 sli_ot_radio_interface_rail_get_channel_ptr(uint16_t *channel);

// Additional RAIL functions needed for radio.cpp
sl_rail_status_t sli_ot_radio_interface_rail_get_rx_incoming_packet_info(sl_rail_rx_packet_info_t *packetInfo);
sl_rail_status_t sli_ot_radio_interface_rail_start_scheduled_rx(uint8_t                              channel,
                                                                const sl_rail_scheduled_rx_config_t *config,
                                                                const sl_rail_scheduler_info_t      *scheduler);
sl_rail_handle_t sli_ot_radio_interface_rail_init(efr32CommonConfig *commonConfig);
bool             sli_ot_radio_interface_rail_is_receiving_frame(void);

// Additional RAIL functions for remaining radio.cpp references
bool             sli_ot_radio_interface_rail_is_rx_auto_ack_paused(void);
sl_rail_status_t sli_ot_radio_interface_rail_get_rx_time_sync_word_end(sl_rail_rx_packet_details_t *packetDetails);
uint32_t         sli_ot_radio_interface_rail_get_phy_id(void);
sl_rail_status_t sli_ot_radio_interface_rail_config_radio(void);
sl_rail_status_t sli_ot_radio_interface_rail_pause_rx_auto_ack(bool pause);
void             sli_ot_radio_interface_rail_set_coex_counter_handler(void *handler);
sl_rail_status_t sli_ot_radio_interface_rail_set_cca_threshold(int8_t threshold);

// Additional RAIL functions for diag.c
sl_rail_status_t sli_ot_radio_interface_rail_get_channel(uint16_t *channel);
sl_rail_status_t sli_ot_radio_interface_rail_start_tx_stream(uint8_t               channel,
                                                             sl_rail_stream_mode_t mode,
                                                             sl_rail_tx_options_t  options);
sl_rail_status_t sli_ot_radio_interface_rail_stop_tx_stream(void);
sl_rail_status_t sli_ot_radio_interface_rail_start_rx(uint8_t channel, const sl_rail_scheduler_info_t *scheduler);

//------------------------------------------------------------------------------
// Accessor functions for global variables moved from radio.cpp

otRadioCaps sli_ot_radio_interface_get_capabilities(void);
bool        sli_ot_radio_interface_get_promiscuous(void);
void        sli_ot_radio_interface_set_promiscuous(bool enable);
bool        sli_ot_radio_interface_is_src_match_enabled(void);
void        sli_ot_radio_interface_set_src_match_enabled(bool enable);
bool        sli_ot_radio_interface_is_coex_enabled(void);
void        sli_ot_radio_interface_set_coex_enabled(bool enable);

/**
 * Get the current PTI radio configuration
 *
 * @returns The current IEEE 802.15.4 PHY configuration
 */
sl_rail_ieee802154_phy_t sli_ot_radio_interface_get_pti_radio_config(void);

#ifdef SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT
/**
 * Initialize antenna diversity configuration
 */
void sli_ot_radio_interface_init_antenna_config(void);
#endif // SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT

#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
/**
 * Get radio debug counters
 *
 * @param[out] aRadioCounters  Pointer to counter structure to fill
 * @return OT_ERROR_NONE on success, OT_ERROR_INVALID_ARGS if pointer is null
 */
otError sli_ot_radio_interface_get_radio_counters(struct efr32RadioCounters *aRadioCounters);

/**
 * Clear radio debug counters
 */
void sli_ot_radio_interface_clear_radio_counters(void);
#endif // RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT

#ifdef __cplusplus
}
#endif

#endif // RADIO_INTERFACE_H
