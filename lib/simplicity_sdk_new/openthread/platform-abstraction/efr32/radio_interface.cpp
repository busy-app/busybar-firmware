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
 *   This file implements the shared radio interface layer that provides common RAIL operations
 *   and hardware abstraction used across multiple radio modules (radio.cpp, radio_energy_scan.cpp, etc.).
 */

#include "radio_interface.h"

#include <openthread-core-config.h>
#include <openthread/platform/alarm-micro.h>
#include <openthread/platform/radio.h>
#include <openthread/platform/time.h>

#include "common/code_utils.hpp"
#include "common/debug.hpp"
#include "common/logging.hpp"
#include "utils/link_metrics.h"

#include "platform-efr32.h"
#include "radio_channel_switching.h"
#include "radio_events.h"
#include "radio_instance.h"
#include "radio_power_manager.h"
#include "radio_security.h"
#include "radio_state.h"

#include "soft_source_match_table.h"
#include <openthread/link.h>
#include "utils/code_utils.h"
#include "utils/mac_frame.h"

extern "C" {
#include "rail_config.h"
#include "sl_openthread_radio_config.h"
#include "sl_rail.h"
#include "sl_rail_ieee802154.h"
#include "sl_rail_util_compatible_pa.h"
#if defined(_SILICON_LABS_32B_SERIES_2)
#include "em_system.h"
#else
#include "sl_hal_system.h"
#endif

#ifdef SL_CATALOG_RAIL_UTIL_COEX_PRESENT
#include "coexistence-802154.h"
#include "coexistence-ot.h"
#endif

#ifdef SL_CATALOG_RAIL_MULTIPLEXER_PRESENT
#include "sl_rail_mux_rename.h"
#endif

#ifdef SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT
#include "sl_rail_util_ant_div.h"
#include "sl_rail_util_ant_div_config.h"
#endif // SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT

#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_PHY_SELECT_PRESENT
#include "sl_rail_util_ieee802154_phy_select.h"
#endif // SL_CATALOG_RAIL_UTIL_IEEE802154_PHY_SELECT_PRESENT
} // extern "C"

#ifdef SL_CATALOG_RAIL_MULTIPLEXER_PRESENT
sl_rail_handle_t gRailHandle;
#else
sl_rail_handle_t emPhyRailHandle;
#endif

efr32BandConfig *sCurrentBandConfig = nullptr;

#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
struct efr32RadioCounters railDebugCounters;
#endif

static constexpr uint8_t CCA_THRESHOLD_DEFAULT = -75; // dbm - default for 2.4GHz 802.15.4

static efr32CommonConfig sCommonConfig;
static efr32BandConfig   sBandConfig;
static int8_t            sCcaThresholdDbm = CCA_THRESHOLD_DEFAULT;

const sl_rail_ieee802154_config_t sRailIeee802154Config = {
    .p_addresses = nullptr,
    .ack_config =
        {
            .enable         = true,
            .ack_timeout_us = 672,
            .rx_transitions =
                {
                    .success = SL_RAIL_RF_STATE_RX,
                    .error   = SL_RAIL_RF_STATE_RX,
                },
            .tx_transitions =
                {
                    .success = SL_RAIL_RF_STATE_RX,
                    .error   = SL_RAIL_RF_STATE_RX,
                },
        },
    .timings =
        {
            .idle_to_rx = 100,
            .tx_to_rx   = 192 - 10,
            .idle_to_tx = 100,
#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
            .rx_to_tx = 256, // accommodate enhanced ACKs
#else
            .rx_to_tx = 192,
#endif
            .rxsearch_timeout       = 0,
            .tx_to_rxsearch_timeout = 0,
            .tx_to_tx               = 0,
        },
    .frames_mask                            = SL_RAIL_IEEE802154_ACCEPT_STANDARD_FRAMES,
    .promiscuous_mode                       = false,
    .is_pan_coordinator                     = false,
    .default_frame_pending_in_outgoing_acks = false,
};

// External function declarations
extern void sli_update_tx_power_after_config_update(sl_rail_tx_pa_mode_t pa_mode, int8_t aTxPower);
extern void sli_set_tx_power_in_rail(int8_t aTxPower);

// External functions from radio.cpp
extern bool sl_rail_util_coex_is_enabled(void);

static otRadioCaps sRadioCapabilities =
    (OT_RADIO_CAPS_ACK_TIMEOUT | OT_RADIO_CAPS_CSMA_BACKOFF | OT_RADIO_CAPS_ENERGY_SCAN | OT_RADIO_CAPS_SLEEP_TO_TX
#if (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)
     | OT_RADIO_CAPS_TRANSMIT_SEC
     // When scheduled tx is required, we support sl_rail_start_scheduled_cca_csma_tx
     // (delay is indicated in tx frame info set in MAC)
     | OT_RADIO_CAPS_TRANSMIT_TIMING
     // When scheduled rx is required, we support sl_rail_start_scheduled_rx in our
     // implementation of otPlatRadioReceiveAt
     | OT_RADIO_CAPS_RECEIVE_TIMING
#endif
    );

static bool sPromiscuous       = false;
static bool sIsSrcMatchEnabled = false;
static bool sRadioCoexEnabled  = true;

// Constants needed for moved functions
#ifndef USERDATA_MFG_CUSTOM_EUI_64
#define USERDATA_MFG_CUSTOM_EUI_64 (2)
#endif

#ifndef OT_EXT_ADDRESS_SIZE
#define OT_EXT_ADDRESS_SIZE (8)
#endif

// Device capability macro for MCU enable check
#if defined(_SILICON_LABS_32B_SERIES_2)
#define DEVICE_CAPABILITY_MCU_EN (DEVINFO->SWCAPA1 & _DEVINFO_SWCAPA1_RFMCUEN_MASK)
#else
#define DEVICE_CAPABILITY_MCU_EN (DEVINFO->SWCAPA & _DEVINFO_SWCAPA_RFMCUEN_MASK)
#endif

//------------------------------------------------------------------------------
// Function Implementations

// Functions moved from radio.cpp
#if RADIO_CONFIG_ENABLE_CUSTOM_EUI_SUPPORT && defined(_SILICON_LABS_32B_SERIES_2)

#ifndef USERDATA_END
#define USERDATA_END (USERDATA_BASE + FLASH_PAGE_SIZE)
#endif

/*
 * This API reads the UserData page on the given EFR device.
 */
static int readUserData(void *aBuffer, uint16_t aIndex, int aLen, bool aChangeByteOrder)
{
    const uint8_t *readLocation  = (uint8_t *)USERDATA_BASE + aIndex;
    uint8_t       *writeLocation = (uint8_t *)aBuffer;

    // Sanity check to verify if the ouput buffer is valid and the index and len are valid.
    // If invalid, change the len to -1 and return.
    otEXPECT_ACTION((writeLocation != nullptr) && ((readLocation + aLen) <= (uint8_t *)USERDATA_END), aLen = -1);

    // Copy the contents of flash into output buffer.

    for (int idx = 0; idx < aLen; idx++)
    {
        if (aChangeByteOrder)
        {
            writeLocation[idx] = readLocation[(aLen - 1) - idx];
        }
        else
        {
            writeLocation[idx] = readLocation[idx];
        }
    }

exit:
    // Return len, len was changed to -1 to indicate failure.
    return aLen;
}
#endif

#ifdef SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT
void sli_ot_radio_interface_init_antenna_config(void)
{
    sl_rail_status_t status;
    sl_rail_util_ant_div_init();
    status = sl_rail_util_ant_div_update_antenna_config();
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
}
#endif // SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT

sl_rail_handle_t sli_ot_radio_interface_get_rail_handle(void)
{
#ifdef SL_CATALOG_RAIL_MULTIPLEXER_PRESENT
    return gRailHandle;
#else
    return emPhyRailHandle;
#endif
}

efr32BandConfig *sli_ot_radio_interface_get_current_band_config(void)
{
    return sCurrentBandConfig;
}

efr32CommonConfig *sli_ot_radio_interface_get_common_config_ptr(void)
{
    return &sCommonConfig;
}

efr32BandConfig *sli_ot_radio_interface_get_band_config_ptr(void)
{
    return &sBandConfig;
}

otError sli_ot_radio_interface_set_rx(uint8_t aChannel)
{
    otError          error = OT_ERROR_NONE;
    sl_rail_status_t status;

    sl_rail_scheduler_info_t bgRxSchedulerInfo = {.priority         = SL_802154_RADIO_PRIO_BACKGROUND_RX_VALUE,
                                                  .slip_time        = 0,
                                                  .transaction_time = 0};

#if FAST_CHANNEL_SWITCHING_SUPPORT && OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    if (sli_ot_radio_channel_switching_is_multi_channel_enabled())
    {
        // Calling sl_rail_start_rx with a channel not listed in the channel
        // switching config is a bug.
        OT_ASSERT(sli_ot_radio_channel_switching_get_channel_index(aChannel) != INVALID_INTERFACE_INDEX);

        sli_ot_radio_state_set_idle();
        status = sl_rail_ieee802154_config_rx_channel_switching(gRailHandle,
                                                                sli_ot_radio_channel_switching_get_config_ptr());
        OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
        status = sl_rail_config_rx_options(gRailHandle,
                                           SL_RAIL_RX_OPTION_CHANNEL_SWITCHING,
                                           SL_RAIL_RX_OPTION_CHANNEL_SWITCHING);
        OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
    }
    else
    {
        status = sl_rail_config_rx_options(gRailHandle, SL_RAIL_RX_OPTION_CHANNEL_SWITCHING, SL_RAIL_RX_OPTIONS_NONE);
        OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
    }
#endif

    status = sl_rail_start_rx(gRailHandle, aChannel, &bgRxSchedulerInfo);
    if (status != SL_RAIL_STATUS_NO_ERROR)
    {
        error = OT_ERROR_FAILED;
        return error;
    }

    sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_LISTEN, 0U);

    otLogInfoPlat("State=OT_RADIO_STATE_RECEIVE");

    return error;
}

bool sli_ot_radio_interface_is_busy(void)
{
    return sli_ot_radio_state_is_transmitting_or_scanning();
}

void sli_ot_radio_interface_set_idle(sl_rail_handle_t aRailHandle, sl_rail_idle_mode_t aIdleMode, bool aWait)
{
    sl_rail_idle(aRailHandle, aIdleMode, aWait);
}

sl_rail_status_t sli_ot_radio_interface_config_rx_options(sl_rail_rx_options_t aOptions, sl_rail_rx_options_t aMask)
{
    return sl_rail_config_rx_options(gRailHandle, aOptions, aMask);
}

int16_t sli_ot_radio_interface_get_rssi(bool aWait)
{
    return sl_rail_get_rssi(gRailHandle, aWait ? SL_RAIL_GET_RSSI_WAIT_WITHOUT_TIMEOUT : SL_RAIL_GET_RSSI_NO_WAIT);
}

uint32_t sli_ot_radio_interface_get_symbol_rate(void)
{
    return sl_rail_get_symbol_rate(gRailHandle);
}

sl_rail_status_t sli_ot_radio_interface_set_timer(struct sl_rail_multi_timer    *aTimer,
                                                  sl_rail_time_t                 aTime,
                                                  sl_rail_time_mode_t            aMode,
                                                  sl_rail_multi_timer_callback_t aCallback,
                                                  void                          *aCallbackArg)
{
    return sl_rail_set_multi_timer(gRailHandle, aTimer, aTime, aMode, aCallback, aCallbackArg);
}

void sli_ot_radio_interface_cancel_timer(struct sl_rail_multi_timer *aTimer)
{
    sl_rail_cancel_multi_timer(gRailHandle, aTimer);
}

efr32BandConfig *sli_ot_radio_interface_get_band_config(uint8_t aChannel)
{
    efr32BandConfig *config = nullptr;

    if (!((sBandConfig.mChannelMin <= aChannel) && (aChannel <= sBandConfig.mChannelMax)))
    {
        return nullptr;
    }
    config = &sBandConfig;

    return config;
}

void sli_ot_radio_interface_load_rail_config(efr32BandConfig *aBandConfig, int8_t aTxPower)
{
    sl_rail_status_t     status;
    sl_rail_tx_pa_mode_t pa_mode = SL_RAIL_TX_PA_MODE_INVALID;

    if (aBandConfig->mChannelConfig != nullptr)
    {
        status = sl_rail_ieee802154_set_phy_id(gRailHandle, RAIL_IEEE802154_PTI_RADIO_CONFIG_915MHZ_R23_NA_EXT);
        OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
        status = sl_rail_config_channels(gRailHandle, aBandConfig->mChannelConfig, nullptr);
        OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
        uint16_t firstChannel = sl_rail_get_first_channel(gRailHandle, aBandConfig->mChannelConfig);
        OT_ASSERT(firstChannel == aBandConfig->mChannelMin);
        status = sl_rail_prepare_channel(gRailHandle, firstChannel);
        OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
        status = sl_rail_ieee802154_config_g_options(gRailHandle,
                                                     SL_RAIL_IEEE802154_G_OPTION_GB868,
                                                     SL_RAIL_IEEE802154_G_OPTION_GB868);
        OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);

        pa_mode = SL_RAIL_TX_PA_MODE_SUB_GHZ;
    }
    else
    {
#if defined(SL_CATALOG_RAIL_UTIL_IEEE802154_PHY_SELECT_PRESENT)
        status = sl_rail_util_ieee802154_config_radio(gRailHandle);
#else
        status = sl_rail_ieee802154_config_2p4_ghz_radio(gRailHandle);
#endif // SL_CATALOG_RAIL_UTIL_IEEE802154_PHY_SELECT_PRESENT
        OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);

        pa_mode = SL_RAIL_TX_PA_MODE_2P4_GHZ;
    }

#if (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)
    // 802.15.4E support (only on platforms that support it, so error checking is disabled)
    // Note: This has to be called after sl_rail_ieee802154_config_2p4_ghz_radio due to a bug where this call
    // can overwrite options set below.
    sl_rail_ieee802154_config_e_options(gRailHandle,
                                        (SL_RAIL_IEEE802154_E_OPTION_GB868 | SL_RAIL_IEEE802154_E_OPTION_ENH_ACK),
                                        (SL_RAIL_IEEE802154_E_OPTION_GB868 | SL_RAIL_IEEE802154_E_OPTION_ENH_ACK));
#endif // (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)

    if (aTxPower != SL_INVALID_TX_POWER)
    {
        sli_update_tx_power_after_config_update(pa_mode, aTxPower);
    }

    // Set the current band config
    sCurrentBandConfig = aBandConfig;
}

otError sli_ot_radio_interface_load_channel_config(uint8_t aChannel, int8_t aTxPower)
{
    otError          error      = OT_ERROR_NONE;
    efr32BandConfig *bandConfig = sli_ot_radio_interface_get_band_config(aChannel);

    otEXPECT_ACTION(bandConfig != nullptr, error = OT_ERROR_INVALID_ARGS);

    otEXPECT_ACTION(bandConfig != sCurrentBandConfig, sli_ot_radio_interface_set_tx_power(aTxPower));

    sli_ot_radio_interface_rail_idle();
    sli_ot_radio_interface_load_rail_config(bandConfig, aTxPower);

exit:
    return error;
}

void sli_ot_radio_interface_init_config(void)
{
    // Initialize band config with proper channel range
#if RADIO_CONFIG_2P4GHZ_OQPSK_SUPPORT
    sBandConfig.mChannelConfig = nullptr;
    sBandConfig.mChannelMin    = OT_RADIO_2P4GHZ_OQPSK_CHANNEL_MIN; // 11
    sBandConfig.mChannelMax    = OT_RADIO_2P4GHZ_OQPSK_CHANNEL_MAX; // 26
#else
    sBandConfig.mChannelConfig = channelConfigs[0];
    sBandConfig.mChannelMin    = SL_CHANNEL_MIN;
    sBandConfig.mChannelMax    = SL_CHANNEL_MAX;
#endif
}

void sli_ot_radio_interface_deinit_config(void)
{
    // Clear the current band config
    sCurrentBandConfig = nullptr;
}

sl_rail_status_t sli_ot_radio_interface_rail_config_cca_mode(uint8_t aCcaMode)
{
    return sl_rail_ieee802154_config_cca_mode(gRailHandle, aCcaMode);
}

sl_rail_status_t sli_ot_radio_interface_set_cca_mode(uint8_t aCcaMode)
{
    return sli_ot_radio_interface_rail_config_cca_mode(aCcaMode);
}

void sli_ot_radio_interface_set_tx_power(int8_t aTxPower)
{
    sli_set_tx_power_in_rail(aTxPower);
}

void sli_ot_radio_interface_set_current_band_config(efr32BandConfig *aBandConfig)
{
    sCurrentBandConfig = aBandConfig;
}

int8_t sli_ot_radio_interface_get_cca_threshold(void)
{
    return sCcaThresholdDbm;
}

void sli_ot_radio_interface_set_cca_threshold(int8_t aThreshold)
{
    sCcaThresholdDbm = aThreshold;
}

// These functions provide direct access to RAIL operations while keeping gRailHandle
// encapsulated within this single file.

// RAIL state management functions
sl_rail_status_t sli_ot_radio_interface_rail_idle(void)
{
    return sl_rail_idle(gRailHandle, SL_RAIL_IDLE, true);
}

sl_rail_status_t sli_ot_radio_interface_rail_idle_abort(void)
{
    return sl_rail_idle(gRailHandle, SL_RAIL_IDLE_ABORT, true);
}

sl_rail_status_t sli_ot_radio_interface_rail_yield_radio(void)
{
    return sl_rail_yield_radio(gRailHandle);
}

sl_rail_radio_state_t sli_ot_radio_interface_rail_get_radio_state(void)
{
    return sl_rail_get_radio_state(gRailHandle);
}

// RAIL TX operations
sl_rail_status_t sli_ot_radio_interface_rail_start_tx(uint8_t                         aChannel,
                                                      sl_rail_tx_options_t            aOptions,
                                                      const sl_rail_scheduler_info_t *aScheduler)
{
    return sl_rail_start_tx(gRailHandle, aChannel, aOptions, aScheduler);
}

sl_rail_status_t sli_ot_radio_interface_rail_start_cca_csma_tx(uint8_t                         aChannel,
                                                               sl_rail_tx_options_t            aOptions,
                                                               const sl_rail_csma_config_t    *aCsmaConfig,
                                                               const sl_rail_scheduler_info_t *aScheduler)
{
    return sl_rail_start_cca_csma_tx(gRailHandle, aChannel, aOptions, aCsmaConfig, aScheduler);
}

sl_rail_status_t sli_ot_radio_interface_rail_start_scheduled_cca_csma_tx(
    uint8_t                              aChannel,
    sl_rail_tx_options_t                 aOptions,
    const sl_rail_scheduled_tx_config_t *aScheduledConfig,
    const sl_rail_csma_config_t         *aCsmaConfig,
    const sl_rail_scheduler_info_t      *aScheduler)
{
    return sl_rail_start_scheduled_cca_csma_tx(gRailHandle,
                                               aChannel,
                                               aOptions,
                                               aScheduledConfig,
                                               aCsmaConfig,
                                               aScheduler);
}

sl_rail_status_t sli_ot_radio_interface_rail_write_tx_fifo(const uint8_t *aData, uint16_t aLength, bool aReset)
{
    return sl_rail_write_tx_fifo(gRailHandle, aData, aLength, aReset);
}

// RAIL IEEE 802.15.4 operations
sl_rail_status_t sli_ot_radio_interface_rail_set_pan_id(uint16_t aPanId, uint8_t aPanIndex)
{
    return sl_rail_ieee802154_set_pan_id(gRailHandle, aPanId, aPanIndex);
}

sl_rail_status_t sli_ot_radio_interface_rail_set_long_address(const uint8_t *aAddress, uint8_t aPanIndex)
{
    return sl_rail_ieee802154_set_long_address(gRailHandle, aAddress, aPanIndex);
}

sl_rail_status_t sli_ot_radio_interface_rail_set_short_address(uint16_t aAddress, uint8_t aPanIndex)
{
    return sl_rail_ieee802154_set_short_address(gRailHandle, aAddress, aPanIndex);
}

sl_rail_status_t sli_ot_radio_interface_rail_set_promiscuous_mode(bool aEnable)
{
    return sl_rail_ieee802154_set_promiscuous_mode(gRailHandle, aEnable);
}

// RAIL packet operations
sl_rail_rx_packet_handle_t sli_ot_radio_interface_rail_get_rx_packet_info(sl_rail_rx_packet_handle_t aHandle,
                                                                          sl_rail_rx_packet_info_t  *aPacketInfo)
{
    return sl_rail_get_rx_packet_info(gRailHandle, aHandle, aPacketInfo);
}

sl_rail_status_t sli_ot_radio_interface_rail_copy_rx_packet(uint8_t                        *aBuffer,
                                                            const sl_rail_rx_packet_info_t *aPacketInfo)
{
    return sl_rail_copy_rx_packet(gRailHandle, aBuffer, aPacketInfo);
}

sl_rail_status_t sli_ot_radio_interface_rail_get_rx_packet_details(sl_rail_rx_packet_handle_t   aHandle,
                                                                   sl_rail_rx_packet_details_t *aPacketDetails)
{
    return sl_rail_get_rx_packet_details(gRailHandle, aHandle, aPacketDetails);
}

// RAIL utility functions
uint32_t sli_ot_radio_interface_rail_get_symbol_rate(void)
{
    return sl_rail_get_symbol_rate(gRailHandle);
}

uint32_t sli_ot_radio_interface_rail_get_bit_rate(void)
{
    return sl_rail_get_bit_rate(gRailHandle);
}

uint16_t sli_ot_radio_interface_rail_get_channel_value(void)
{
    uint16_t channel;
    sl_rail_get_channel(gRailHandle, &channel);
    return channel;
}

int16_t sli_ot_radio_interface_rail_get_rssi(sl_rail_time_t aWaitTimeoutUs)
{
    return sl_rail_get_rssi(gRailHandle, aWaitTimeoutUs);
}

// RAIL timing configuration access
uint32_t sli_ot_radio_interface_rail_get_rx_to_tx_timing(void)
{
    return sRailIeee802154Config.timings.rx_to_tx;
}

// RAIL configuration
sl_rail_status_t sli_ot_radio_interface_rail_config_channels(
    const sl_rail_channel_config_t         *aChannels,
    sl_rail_radio_config_changed_callback_t aConfigChangedCallback)
{
    return sl_rail_config_channels(gRailHandle, aChannels, aConfigChangedCallback);
}

sl_rail_status_t sli_ot_radio_interface_rail_config_events(sl_rail_events_t aEvents, sl_rail_events_t aMask)
{
    return sl_rail_config_events(gRailHandle, aEvents, aMask);
}

sl_rail_status_t sli_ot_radio_interface_rail_config_sleep(const sl_rail_timer_sync_config_t *aTimerSyncConfig)
{
    return sl_rail_config_sleep(gRailHandle, aTimerSyncConfig);
}

sl_rail_status_t sli_ot_radio_interface_rail_config_rx_options(sl_rail_rx_options_t aOptions,
                                                               sl_rail_rx_options_t aMask)
{
    return sl_rail_config_rx_options(gRailHandle, aOptions, aMask);
}

// Additional RAIL power management functions
sl_rail_status_t sli_ot_radio_interface_rail_set_tx_power_dbm(sl_rail_tx_power_t aPowerDbm)
{
    return sl_rail_set_tx_power_dbm(gRailHandle, aPowerDbm);
}

sl_rail_tx_pa_mode_t sli_ot_radio_interface_rail_get_tx_pa_mode(void)
{
    return sl_rail_get_pa_mode(gRailHandle);
}

sl_rail_tx_power_t sli_ot_radio_interface_rail_get_tx_power_dbm(void)
{
    return sl_rail_get_tx_power_dbm(gRailHandle);
}

sl_rail_status_t sli_ot_radio_interface_rail_config_tx_power(sl_rail_tx_pa_mode_t pa_mode)
{
    return sl_rail_util_pa_post_init(gRailHandle, pa_mode);
}

void sli_ot_radio_interface_rail_get_channel_ptr(uint16_t *aChannel)
{
    sl_rail_get_channel(gRailHandle, aChannel);
}

// Additional RAIL functions needed for radio.cpp
sl_rail_status_t sli_ot_radio_interface_rail_get_rx_incoming_packet_info(sl_rail_rx_packet_info_t *aPacketInfo)
{
    return sl_rail_get_rx_incoming_packet_info(gRailHandle, aPacketInfo);
}

sl_rail_status_t sli_ot_radio_interface_rail_start_scheduled_rx(uint8_t                              aChannel,
                                                                const sl_rail_scheduled_rx_config_t *aConfig,
                                                                const sl_rail_scheduler_info_t      *aScheduler)
{
    return sl_rail_start_scheduled_rx(gRailHandle, aChannel, aConfig, aScheduler);
}

sl_rail_handle_t sli_ot_radio_interface_rail_init(efr32CommonConfig *aCommonConfig)
{
    sl_rail_status_t status;
    sl_rail_handle_t handle = SL_RAIL_EFR32_HANDLE;

#if !OPENTHREAD_RADIO
    OT_ASSERT(DEVICE_CAPABILITY_MCU_EN);
#endif

    aCommonConfig->mRailConfig.rx_packet_queue_entries = sl_rail_builtin_rx_packet_queue_entries;
    aCommonConfig->mRailConfig.p_rx_packet_queue       = sl_rail_builtin_rx_packet_queue_ptr;

    aCommonConfig->mRailConfig.rx_fifo_bytes    = sl_rail_builtin_rx_fifo_bytes;
    aCommonConfig->mRailConfig.p_rx_fifo_buffer = sl_rail_builtin_rx_fifo_ptr;

    aCommonConfig->mRailConfig.tx_fifo_bytes    = sizeof(aCommonConfig->mRailTxFifo.fifo);
    aCommonConfig->mRailConfig.p_tx_fifo_buffer = (sl_rail_fifo_buffer_align_t *)aCommonConfig->mRailTxFifo.fifo;

    status = sl_rail_init(&handle, &aCommonConfig->mRailConfig, nullptr);
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    status = sl_rail_init_power_manager();
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
#endif // SL_CATALOG_POWER_MANAGER_PRESENT

    status = sl_rail_config_cal(handle, SL_RAIL_CAL_ALL);
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);

    status = sl_rail_set_pti_protocol(handle, SL_RAIL_PTI_PROTOCOL_THREAD);
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);

    status = sl_rail_ieee802154_init(handle, &sRailIeee802154Config);
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);

#if (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)
    // Enhanced Frame Pending
    status = sl_rail_ieee802154_enable_early_frame_pending(handle, true);
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);

    status = sl_rail_ieee802154_enable_data_frame_pending(handle, true);
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);

    // Initialize security module
    sli_ot_radio_security_init();
#endif // (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)

    // Enable RAIL multi-timer
    sl_rail_config_multi_timer(handle, true);

    gRailHandle = handle;
    OT_ASSERT(gRailHandle != nullptr);
    return gRailHandle;
}

bool sli_ot_radio_interface_rail_is_receiving_frame(void)
{
    return (sl_rail_get_radio_state(gRailHandle) & SL_RAIL_RF_STATE_RX_ACTIVE) == SL_RAIL_RF_STATE_RX_ACTIVE;
}

// Additional RAIL functions for remaining radio.cpp references
bool sli_ot_radio_interface_rail_is_rx_auto_ack_paused(void)
{
    return sl_rail_is_rx_auto_ack_paused(gRailHandle);
}

sl_rail_status_t sli_ot_radio_interface_rail_get_rx_time_sync_word_end(sl_rail_rx_packet_details_t *aPacketDetails)
{
    return sl_rail_get_rx_time_sync_word_end(gRailHandle, aPacketDetails);
}

uint32_t sli_ot_radio_interface_rail_get_phy_id(void)
{
    return sl_rail_ieee802154_get_phy_id(gRailHandle);
}

#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_PHY_SELECT_PRESENT
sl_rail_status_t sli_ot_radio_interface_rail_config_radio(void)
{
    sl_rail_status_t status;
#if defined(SL_CATALOG_RAIL_UTIL_IEEE802154_PHY_SELECT_PRESENT)
    status = sl_rail_util_ieee802154_config_radio(gRailHandle);
#else
    status = sl_rail_ieee802154_config_2p4_ghz_radio(gRailHandle);
#endif
    return status;
}
#endif // SL_CATALOG_RAIL_UTIL_IEEE802154_PHY_SELECT_PRESENT

sl_rail_status_t sli_ot_radio_interface_rail_pause_rx_auto_ack(bool aPause)
{
    return sl_rail_pause_rx_auto_ack(gRailHandle, aPause);
}

void sli_ot_radio_interface_rail_set_coex_counter_handler(void *aHandler)
{
#ifdef SL_CATALOG_RAIL_MULTIPLEXER_PRESENT
    sl_rail_mux_set_coex_counter_handler(gRailHandle, (COEX_CounterHandler_t)aHandler);
#else
    OT_UNUSED_VARIABLE(aHandler);
#endif
}

sl_rail_status_t sli_ot_radio_interface_rail_set_cca_threshold(int8_t aThreshold)
{
    return sl_rail_set_cca_threshold(gRailHandle, aThreshold);
}

sl_rail_status_t sli_ot_radio_interface_rail_get_channel(uint16_t *aChannel)
{
    return sl_rail_get_channel(gRailHandle, aChannel);
}

sl_rail_status_t sli_ot_radio_interface_rail_start_tx_stream(uint8_t               aChannel,
                                                             sl_rail_stream_mode_t aMode,
                                                             sl_rail_tx_options_t  aOptions)
{
    return sl_rail_start_tx_stream(gRailHandle, aChannel, aMode, aOptions);
}

sl_rail_status_t sli_ot_radio_interface_rail_stop_tx_stream(void)
{
    return sl_rail_stop_tx_stream(gRailHandle);
}

sl_rail_status_t sli_ot_radio_interface_rail_start_rx(uint8_t aChannel, const sl_rail_scheduler_info_t *aScheduler)
{
    return sl_rail_start_rx(gRailHandle, aChannel, aScheduler);
}

//------------------------------------------------------------------------------
// OpenThread Platform Radio API Functions (Instance-Independent)
// These functions don't use the otInstance parameter and can be moved here

uint64_t otPlatRadioGetNow(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return otPlatTimeGet();
}

void otPlatRadioGetIeeeEui64(otInstance *aInstance, uint8_t *aIeeeEui64)
{
    OT_UNUSED_VARIABLE(aInstance);

#if (RADIO_CONFIG_ENABLE_CUSTOM_EUI_SUPPORT) && defined(_SILICON_LABS_32B_SERIES_2)
    // Invalid EUI
    uint8_t nullEui[] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};

    // Read the Custom EUI and compare it to nullEui
    if ((readUserData(aIeeeEui64, USERDATA_MFG_CUSTOM_EUI_64, OT_EXT_ADDRESS_SIZE, true) == -1)
        || (memcmp(aIeeeEui64, nullEui, OT_EXT_ADDRESS_SIZE) == 0))
#endif
    {
        uint64_t       eui64;
        const uint8_t *eui64Ptr = nullptr;

#if defined(_SILICON_LABS_32B_SERIES_2)
        eui64 = SYSTEM_GetUnique();
#else
        eui64 = sl_hal_system_get_unique();
#endif
        eui64Ptr = (const uint8_t *)&eui64;

        for (uint8_t i = 0; i < OT_EXT_ADDRESS_SIZE; i++)
        {
            aIeeeEui64[i] = eui64Ptr[(OT_EXT_ADDRESS_SIZE - 1) - i];
        }
    }
}

otRadioState otPlatRadioGetState(otInstance *aInstance)
{
    otRadioState radioState = OT_RADIO_STATE_INVALID;

    OT_UNUSED_VARIABLE(aInstance);

    switch (sli_ot_radio_interface_rail_get_radio_state())
    {
    case SL_RAIL_RF_STATE_RX:
    case SL_RAIL_RF_STATE_RX_ACTIVE:
        radioState = OT_RADIO_STATE_RECEIVE;
        break;

    case SL_RAIL_RF_STATE_TX:
    case SL_RAIL_RF_STATE_TX_ACTIVE:
        radioState = OT_RADIO_STATE_TRANSMIT;
        break;

    case SL_RAIL_RF_STATE_IDLE:
        radioState = OT_RADIO_STATE_SLEEP;
        break;

    case SL_RAIL_RF_STATE_INACTIVE:
        radioState = OT_RADIO_STATE_DISABLED;
        break;
    default:
        break;
    }

    return radioState;
}

bool otPlatRadioIsEnabled(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return (sli_ot_radio_state_is_initialized());
}

otError otPlatRadioSleep(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    otError error = OT_ERROR_NONE;

    otEXPECT(!sli_ot_radio_state_is_tx_data_ongoing());

    otLogInfoPlat("State=OT_RADIO_STATE_SLEEP");
    sli_ot_radio_state_set_scheduled_rx_pending(false);
    sli_ot_radio_state_set_idle();

exit:
    return error;
}

otRadioCaps otPlatRadioGetCaps(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return sli_ot_radio_interface_get_capabilities();
}

bool otPlatRadioGetPromiscuous(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return sli_ot_radio_interface_get_promiscuous();
}

void otPlatRadioSetPromiscuous(otInstance *aInstance, bool aEnable)
{
    sl_rail_status_t status;

    otEXPECT(sl_ot_rtos_task_can_access_pal());

    sli_ot_radio_interface_set_promiscuous(aEnable);

    status = sli_ot_radio_interface_rail_set_promiscuous_mode(aEnable);
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);

exit:
    OT_UNUSED_VARIABLE(aInstance);
    return;
}

void otPlatRadioEnableSrcMatch(otInstance *aInstance, bool aEnable)
{
    otEXPECT(sl_ot_rtos_task_can_access_pal());

    // set Frame Pending bit for all outgoing ACKs if aEnable is false
    sli_ot_radio_interface_set_src_match_enabled(aEnable);

exit:
    OT_UNUSED_VARIABLE(aInstance);
    return;
}

otError otPlatRadioGetTransmitPower(otInstance *aInstance, int8_t *aPower)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError error = OT_ERROR_NONE;

    otEXPECT_ACTION(aPower != nullptr, error = OT_ERROR_INVALID_ARGS);
    // sl_rail_get_tx_power_dbm() returns power in deci-dBm (0.1dBm)
    // Divide by 10 because aPower is supposed be in units dBm
    *aPower = static_cast<int8_t>(sli_ot_radio_interface_rail_get_tx_power_dbm());

exit:
    return error;
}

otError otPlatRadioGetCcaEnergyDetectThreshold(otInstance *aInstance, int8_t *aThreshold)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError error = OT_ERROR_NONE;
    otEXPECT_ACTION(aThreshold != nullptr, error = OT_ERROR_INVALID_ARGS);

    *aThreshold = sli_ot_radio_interface_get_cca_threshold();

exit:
    return error;
}

otError otPlatRadioSetCcaEnergyDetectThreshold(otInstance *aInstance, int8_t aThreshold)
{
    OT_UNUSED_VARIABLE(aInstance);

    otError error = OT_ERROR_NONE;

    otEXPECT_ACTION(sl_ot_rtos_task_can_access_pal(), error = OT_ERROR_REJECTED);
    sli_ot_radio_interface_set_cca_threshold(aThreshold);

exit:
    return error;
}

int8_t otPlatRadioGetReceiveSensitivity(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return SL_OPENTHREAD_RECEIVE_SENSITIVITY;
}

uint8_t otPlatRadioGetCslAccuracy(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return static_cast<uint8_t>(otPlatTimeGetXtalAccuracy());
}

uint8_t otPlatRadioGetCslUncertainty(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return SL_OPENTHREAD_CSL_TX_UNCERTAINTY;
}

bool otPlatRadioIsCoexEnabled(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return (sRadioCoexEnabled && sl_rail_util_coex_is_enabled());
}

//------------------------------------------------------------------------------
// Accessor functions for global variables moved from radio.cpp

otRadioCaps sli_ot_radio_interface_get_capabilities(void)
{
    return sRadioCapabilities;
}

bool sli_ot_radio_interface_get_promiscuous(void)
{
    return sPromiscuous;
}

void sli_ot_radio_interface_set_promiscuous(bool aEnable)
{
    sPromiscuous = aEnable;
}

bool sli_ot_radio_interface_is_src_match_enabled(void)
{
    return sIsSrcMatchEnabled;
}

void sli_ot_radio_interface_set_src_match_enabled(bool aEnable)
{
    sIsSrcMatchEnabled = aEnable;
}

bool sli_ot_radio_interface_is_coex_enabled(void)
{
    return sRadioCoexEnabled;
}

void sli_ot_radio_interface_set_coex_enabled(bool aEnable)
{
    sRadioCoexEnabled = aEnable;
}

//------------------------------------------------------------------------------
// Link Metrics Enhanced ACK Probing

#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
#if OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE
otError otPlatRadioConfigureEnhAckProbing(otInstance          *aInstance,
                                          otLinkMetrics        aLinkMetrics,
                                          const otShortAddress aShortAddress,
                                          const otExtAddress  *aExtAddress)
{
    otError error;

    OT_UNUSED_VARIABLE(aInstance);
    otEXPECT_ACTION(sl_ot_rtos_task_can_access_pal(), error = OT_ERROR_REJECTED);

    error = otLinkMetricsConfigureEnhAckProbing(aShortAddress, aExtAddress, aLinkMetrics);

exit:
    return error;
}
#endif // OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE
#endif // OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2

//------------------------------------------------------------------------------
// PTI Radio Configuration

sl_rail_ieee802154_phy_t sli_ot_radio_interface_get_pti_radio_config(void)
{
    return static_cast<sl_rail_ieee802154_phy_t>(sli_ot_radio_interface_rail_get_phy_id());
}

//------------------------------------------------------------------------------
// Radio Debug Counters

#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
otError sli_ot_radio_interface_get_radio_counters(efr32RadioCounters *aRadioCounters)
{
    otError error = OT_ERROR_NONE;

    otEXPECT_ACTION(aRadioCounters != nullptr, error = OT_ERROR_INVALID_ARGS);
    *aRadioCounters = railDebugCounters;

exit:
    return error;
}

void sli_ot_radio_interface_clear_radio_counters(void)
{
    memset(&railDebugCounters, 0, sizeof(railDebugCounters));
}
#endif // RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
