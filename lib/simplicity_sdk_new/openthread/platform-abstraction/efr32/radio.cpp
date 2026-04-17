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
 *   This file implements the main OpenThread platform abstraction for radio communication on EFR32.
 *   It provides the core radio API implementation, packet handling, and integration with RAIL.
 */

#include "radio_channel_switching.h"
#include "radio_csl.h"
#include "radio_energy_scan.hpp"
#include "radio_events.h"
#include "radio_instance.h"
#include "radio_interface.h"
#include "radio_security.h"
#include "radio_state.h"
#include <assert.h>
#include <openthread-core-config.h>
#include <openthread-system.h>
#include <openthread/link.h>
#include <openthread/platform/alarm-micro.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/radio.h>
#include <openthread/platform/time.h>
#include "common/code_utils.hpp"
#include "common/debug.hpp"
#include "common/logging.hpp"
#include "utils/code_utils.h"
#include "utils/link_metrics.h"
#include "utils/mac_frame.h"

extern "C" {
#include "em_device.h"
#include "sl_core.h"
#if defined _SILICON_LABS_32B_SERIES_2
#include "em_system.h"
#else
#include "sl_hal_system.h"
#endif
#include "ieee802154mac.h"
#include "platform-band.h"
#include "platform-efr32.h"
#include "radio_coex.h"
#include "radio_multi_channel.h"
#include "rail_config.h"
#include "sl_memory_manager.h"
#include "sl_packet_utils.h"
#include "sl_rail.h"
#include "sl_rail_ieee802154.h"

#include "sl_openthread_radio_config.h"
#include "sl_rail_util_compatible_pa.h"
#include "soft_source_match_table.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#ifdef SL_CATALOG_RAIL_MULTIPLEXER_PRESENT
#include "sl_rail_mux_rename.h"
#endif

#ifdef SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT
#include "sl_rail_util_ant_div.h"
#include "sl_rail_util_ant_div_config.h"
#endif // SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT

#ifdef SL_CATALOG_RAIL_UTIL_COEX_PRESENT
#include "coexistence-802154.h"
#include "coexistence-ot.h"
#endif // SL_CATALOG_RAIL_UTIL_COEX_PRESENT

#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
#include "sl_rail_util_ieee802154_stack_event.h"
#endif // SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT

#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_PHY_SELECT_PRESENT
#include "sl_rail_util_ieee802154_phy_select.h"
#endif // #ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_PHY_SELECT_PRESENT

#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT
#include "sl_rail_util_ieee802154_fast_channel_switching_config.h"
#endif // SL_CATALOG_RAIL_UTIL_IEEE802154_FAST_CHANNEL_SWITCHING_PRESENT

#include "circular_queue.h"
#include "radio_power_manager.h"
#include "sl_gp_interface.h"
}

//------------------------------------------------------------------------------
// Enums, macros and static variables

#if SL_OPENTHREAD_RADIO_RX_BUFFER_COUNT > CIRCULAR_QUEUE_LEN_MAX
#error "Rx buffer count cannot be greater than max circular queue length."
#endif

#define US_IN_MS 1000

extern "C" {
void packetSentCallback(bool isAck);
void packetReceivedCallback(void);
void txFailedCallback(bool isAck, uint32_t status);
void ackTimeoutCallback(void);
void dataRequestCommandCallback(sl_rail_handle_t aRailHandle);
void schedulerEventCallback(sl_rail_handle_t aRailHandle);
void sl_ot_update_active_radio_config(void);
}

// Static inline helper - forward declaration
static inline bool txWaitingForAck(void);
#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
static bool phyStackEventIsEnabled(void);
#endif // SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT

using rxPacketDetails = struct
{
    uint8_t        length;
    uint8_t        channel;
    uint8_t        lqi;
    int8_t         rssi;
    otInstance    *instance;
    sl_rail_time_t timestamp;
};

typedef struct
{
    rxPacketDetails packetInfo;
    uint8_t         psdu[IEEE802154_MAX_LENGTH];
} rxBuffer;

typedef uint8_t rxBufferIndex_t;

using radioFrame = struct
{
    otRadioFrame frame;
    otInstance  *instance;
    uint8_t      currentRadioTxPriority;
};

static Queue_t          sRxPacketQueue;
static sl_memory_pool_t sRxPacketMemPoolHandle = {};
static uint8_t          sReceiveAckPsdu[IEEE802154_MAX_LENGTH];
static radioFrame       sReceive;
static radioFrame       sReceiveAck;
static otError          sReceiveError;

static radioFrame  sTransmitBuffer[RADIO_REQUEST_BUFFER_COUNT];
static uint8_t     sTransmitPsdu[RADIO_REQUEST_BUFFER_COUNT][IEEE802154_MAX_LENGTH];
static radioFrame *sCurrentTxPacket = nullptr;
static uint8_t     sLastLqi         = 0;
static int8_t      sLastRssi        = 0;
otExtAddress       sExtAddress[RADIO_EXT_ADDR_COUNT];

#if (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)
#define IEEE802154_2015_ENH_ACK_TIMING_RX_TO_TX_US 256
#endif
#define CSL_CSMA_BACKOFF_TIME_IN_US 150
sl_rail_csma_config_t csmaConfig    = SL_RAIL_CSMA_CONFIG_802_15_4_2003_2P4_GHZ_OQPSK_CSMA;
sl_rail_csma_config_t cslCsmaConfig = SL_RAIL_CSMA_CONFIG_SINGLE_CCA;

#define SCHEDULE_TX_DELAY_US 3000

#if OPENTHREAD_CONFIG_MAC_HEADER_IE_SUPPORT
static otRadioIeInfo sTransmitIeInfo[RADIO_REQUEST_BUFFER_COUNT];
#endif

#define CCA_THRESHOLD_UNINIT 127

#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
extern struct efr32RadioCounters railDebugCounters;
#define rxDebugStep (railDebugCounters.mRadioDebugData.m8[RX_DEBUG_COUNTER0])
#endif

#define SHR_DURATION_US 160

#ifdef SL_CATALOG_RAIL_UTIL_COEX_PRESENT
enum
{
    RHO_INACTIVE = 0,
    RHO_EXT_ACTIVE,
    RHO_INT_ACTIVE, // Not used
    RHO_BOTH_ACTIVE,
};

static uint8_t sRhoActive = RHO_INACTIVE;
static bool    sPtaGntEventReported;

#endif // SL_CATALOG_RAIL_UTIL_COEX_PRESENT

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
static bool rxPacketQueueOverflowCallback(const Queue_t *queue, void *data)
{
    OT_UNUSED_VARIABLE(queue);
    OT_UNUSED_VARIABLE(data);

    return false;
}

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE

// Case 1: Packet was directed towards broadcast address or broadcast PAN ID
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
static bool isFilterMaskBroadcast(uint8_t mask)
{
    return ((mask & RADIO_BCAST_PANID_FILTER_MASK) != 0) || ((mask & RADIO_BCAST_ADDR_FILTER_MASK) != 0);
}

// Case 2: Packet was directed to one of our valid address/PANID combos
// (Compare all non-bcast PANID filters against their corresponding address filters for same IID)
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
static bool isFilterMaskInstanceSpecific(uint8_t mask)
{
    // Find any non-broadcast PAN ID match and compare it to address matches for same IID
    return (((RADIO_GET_PANID_FILTER_MASK(mask) >> RADIO_PANID_FILTER_SHIFT)
             & (RADIO_GET_ADDR_FILTER_MASK(mask) >> RADIO_ADDR_FILTER_SHIFT))
            != 0);
}

// Case 3: Packet is missing either destination addressing field or destination PAN ID
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
static bool isFilterMaskMissingAddressing(uint8_t mask)
{
    return ((RADIO_GET_PANID_FILTER_MASK(mask)) == 0) || ((RADIO_GET_ADDR_FILTER_MASK(mask)) == 0);
}

#endif // OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
static bool isFilterMaskValid(uint8_t mask)
{
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    return isFilterMaskBroadcast(mask) || isFilterMaskInstanceSpecific(mask) || isFilterMaskMissingAddressing(mask);
#else
    (void)mask;
    return true;
#endif
}

#if (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)

// Enhanced ACK IE data
static uint8_t sAckIeData[OT_ACK_IE_MAX_SIZE];
static uint8_t sAckIeDataLength = 0;

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
static uint8_t generateAckIeData(otInstance   *aInstance,
                                 uint8_t      *aLinkMetricsIeData,
                                 uint8_t       aLinkMetricsIeDataLen,
                                 otRadioFrame *aReceivedFrame)
{
    OT_UNUSED_VARIABLE(aLinkMetricsIeData);
    OT_UNUSED_VARIABLE(aLinkMetricsIeDataLen);
    OT_UNUSED_VARIABLE(aReceivedFrame);

    uint8_t offset = 0;

    // If instance is nullptr (broadcast packet), skip IE data generation
    otEXPECT(aInstance != nullptr);

#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
    offset += sli_ot_radio_csl_generate_ack_ie_data(aInstance, aReceivedFrame, sAckIeData);
#endif

#if OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE
    if (aLinkMetricsIeData != nullptr && aLinkMetricsIeDataLen > 0)
    {
        offset += otMacFrameGenerateEnhAckProbingIe(sAckIeData, aLinkMetricsIeData, aLinkMetricsIeDataLen);
    }
#endif

exit:
    return offset;
}

#endif // (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
static uint8_t readInitialPacketData(sl_rail_rx_packet_info_t *packetInfo,
                                     uint8_t                   expected_data_bytes_max,
                                     uint8_t                   expected_data_bytes_min,
                                     uint8_t                  *buffer,
                                     uint8_t                   buffer_len)
{
    uint8_t                  packetBytesRead = 0;
    sl_rail_rx_packet_info_t adjustedPacketInfo;

    // Check if we have enough buffer
    OT_ASSERT((buffer_len >= expected_data_bytes_max) || (packetInfo != nullptr));

    // Read the packet info
    sli_ot_radio_interface_rail_get_rx_incoming_packet_info(packetInfo);

    // We are trying to get the packet info of a packet before it is completely received.
    // We do this to evaluate the FP bit in response and add IEs to ACK if needed.
    // Check to see if we have received atleast minimum number of bytes requested.
    otEXPECT_ACTION(packetInfo->packet_bytes >= expected_data_bytes_min, packetBytesRead = 0);

    adjustedPacketInfo = *packetInfo;

    // Only extract what we care about
    if (packetInfo->packet_bytes > expected_data_bytes_max)
    {
        adjustedPacketInfo.packet_bytes = expected_data_bytes_max;
        // Check if the initial portion of the packet received so far exceeds the max value requested.
        if (packetInfo->first_portion_bytes >= expected_data_bytes_max)
        {
            // If we have received more, make sure to copy only the required bytes into the buffer.
            adjustedPacketInfo.first_portion_bytes = expected_data_bytes_max;
            adjustedPacketInfo.p_last_portion_data = nullptr;
        }
    }

    // Copy number of bytes as indicated in `packetInfo->first_portion_bytes` into the buffer.
    sli_ot_radio_interface_rail_copy_rx_packet(buffer, &adjustedPacketInfo);
    // Put it back to packetBytes.
    packetBytesRead = (uint8_t)adjustedPacketInfo.packet_bytes;

exit:
    return packetBytesRead;
}

//------------------------------------------------------------------------------
// Forward Declarations

static void efr32PhyStackInit(void);

#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
static void updateIeInfoTxFrame(uint32_t shrTxTime);
#endif

#ifdef SL_CATALOG_RAIL_UTIL_COEX_PRESENT
static void efr32CoexInit(void);
// Try to transmit the current outgoing frame subject to MAC-level PTA
static void tryTxCurrentPacket(void);
#else
// Transmit the current outgoing frame.
void txCurrentPacket(void);
#define tryTxCurrentPacket txCurrentPacket
#endif // SL_CATALOG_RAIL_UTIL_COEX_PRESENT

static bool validatePacketDetails(sl_rail_rx_packet_handle_t      packetHandle,
                                  sl_rail_rx_packet_details_t    *pPacketDetails,
                                  const sl_rail_rx_packet_info_t *pPacketInfo,
                                  uint16_t                       *packetLength);
static bool validatePacketTimestamp(sl_rail_rx_packet_details_t *pPacketDetails, uint16_t packetLength);

static void updateRxFrameTimestamp(bool aIsAckFrame, sl_rail_time_t aTimestamp);

static otError skipRxPacketLengthBytes(sl_rail_rx_packet_info_t *pPacketInfo);

//==============================================================================
// Radio State Management (from radio_state.cpp)
// This section can be extracted to a separate file for future modularization.
// These are timing-critical functions that need to be inline-optimized.
//==============================================================================

// Internal state flags
#define FLAG_RADIO_INIT_DONE 0x00000001
#define FLAG_ONGOING_TX_DATA 0x00000002
#define FLAG_ONGOING_TX_ACK 0x00000004
#define FLAG_WAITING_FOR_ACK 0x00000008
#define FLAG_CURRENT_TX_USE_CSMA 0x00000010
#define FLAG_SCHEDULED_RX_PENDING 0x00000020
#define FLAG_SCHEDULED_TX_PENDING 0x00000040

// Internal state variables
static volatile uint32_t sMiscRadioState = 0;
static bool              sEmPendingData  = false;

// Core flag operations - static inline for internal use only
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
static inline void setInternalFlag(uint32_t aFlag, bool aVal)
{
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_ATOMIC();
    sMiscRadioState = (aVal ? (sMiscRadioState | aFlag) : (sMiscRadioState & ~aFlag));
    CORE_EXIT_ATOMIC();
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
static inline bool getInternalFlag(uint32_t aFlag)
{
    bool isFlagSet;
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_ATOMIC();
    isFlagSet = (sMiscRadioState & aFlag) ? true : false;
    CORE_EXIT_ATOMIC();

    return isFlagSet;
}

// Public API - these have external linkage for other radio modules
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
void sli_ot_radio_state_set_internal_flag(uint32_t aFlag, bool aVal)
{
    setInternalFlag(aFlag, aVal);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
bool sli_ot_radio_state_get_internal_flag(uint32_t aFlag)
{
    return getInternalFlag(aFlag);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
void sli_ot_radio_state_set_idle(void)
{
    if (sli_ot_radio_interface_rail_get_radio_state() != SL_RAIL_RF_STATE_IDLE)
    {
        sli_ot_radio_interface_rail_idle();
#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
        sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_IDLED, 0U);
        sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_IDLED, 0U);
#endif
    }
    sli_ot_radio_interface_rail_yield_radio();
}

// Internal-only wrappers - static inline for ISR performance
static inline bool isTransmitting(void)
{
    return (getInternalFlag(FLAG_ONGOING_TX_DATA) || getInternalFlag(FLAG_ONGOING_TX_ACK));
}

static inline bool isTxDataOngoing(void)
{
    return getInternalFlag(FLAG_ONGOING_TX_DATA);
}

static inline bool hasTxEvents(void)
{
    return getInternalFlag(RADIO_TX_EVENTS);
}

// External API functions - regular linkage for other modules
bool sli_ot_radio_state_is_transmitting(void)
{
    return isTransmitting();
}

bool sli_ot_radio_state_is_transmitting_or_scanning(void)
{
    return (sli_ot_energy_scan_is_in_progress() || isTxDataOngoing() || hasTxEvents());
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
bool sli_ot_radio_state_is_waiting_for_ack(void)
{
    return isTxDataOngoing() && getInternalFlag(FLAG_WAITING_FOR_ACK);
}

bool sli_ot_radio_state_is_tx_scheduled(void)
{
    return getInternalFlag(FLAG_SCHEDULED_TX_PENDING | EVENT_SCHEDULED_TX_STARTED);
}

void sli_ot_radio_state_set_scheduled_rx_pending(bool aPending)
{
    setInternalFlag(FLAG_SCHEDULED_RX_PENDING, aPending);
}

bool sli_ot_radio_state_is_rx_scheduled(void)
{
    return getInternalFlag(FLAG_SCHEDULED_RX_PENDING);
}

void sli_ot_radio_state_set_scheduled_rx_started(bool aStarted)
{
    setInternalFlag(EVENT_SCHEDULED_RX_STARTED, aStarted);
}

bool sli_ot_radio_state_is_initialized(void)
{
    return getInternalFlag(FLAG_RADIO_INIT_DONE);
}

void sli_ot_radio_state_mark_initialized(void)
{
    setInternalFlag(FLAG_RADIO_INIT_DONE, true);
}

bool sli_ot_radio_state_is_tx_data_ongoing(void)
{
    return isTxDataOngoing();
}

void sli_ot_radio_state_set_tx_data_ongoing(bool aOngoing)
{
    setInternalFlag(FLAG_ONGOING_TX_DATA, aOngoing);
}

bool sli_ot_radio_state_is_tx_ack_ongoing(void)
{
    return getInternalFlag(FLAG_ONGOING_TX_ACK);
}

void sli_ot_radio_state_set_tx_ack_ongoing(bool aOngoing)
{
    setInternalFlag(FLAG_ONGOING_TX_ACK, aOngoing);
}

bool sli_ot_radio_state_is_using_csma(void)
{
    return getInternalFlag(FLAG_CURRENT_TX_USE_CSMA);
}

void sli_ot_radio_state_set_using_csma(bool aUseCsma)
{
    setInternalFlag(FLAG_CURRENT_TX_USE_CSMA, aUseCsma);
}

void sli_ot_radio_state_set_waiting_for_ack(bool aWaiting)
{
    setInternalFlag(FLAG_WAITING_FOR_ACK, aWaiting);
}

void sli_ot_radio_state_set_scheduled_tx_pending(bool aPending)
{
    setInternalFlag(FLAG_SCHEDULED_TX_PENDING, aPending);
}

void sli_ot_radio_state_set_scheduled_tx_started(bool aStarted)
{
    setInternalFlag(EVENT_SCHEDULED_TX_STARTED, aStarted);
}

bool sli_ot_radio_state_has_tx_events(void)
{
    return hasTxEvents();
}

void sli_ot_radio_state_clear_all_tx_events(void)
{
    setInternalFlag(RADIO_TX_EVENTS, false);
}

bool sli_ot_radio_state_has_tx_success(void)
{
    return getInternalFlag(EVENT_TX_SUCCESS);
}

void sli_ot_radio_state_set_tx_success(bool aSuccess)
{
    setInternalFlag(EVENT_TX_SUCCESS, aSuccess);
}

bool sli_ot_radio_state_has_tx_cca_failed(void)
{
    return getInternalFlag(EVENT_TX_CCA_FAILED);
}

void sli_ot_radio_state_set_tx_cca_failed(bool aFailed)
{
    setInternalFlag(EVENT_TX_CCA_FAILED, aFailed);
}

bool sli_ot_radio_state_has_tx_no_ack(void)
{
    return getInternalFlag(EVENT_TX_NO_ACK);
}

void sli_ot_radio_state_set_tx_no_ack(bool aNoAck)
{
    setInternalFlag(EVENT_TX_NO_ACK, aNoAck);
}

bool sli_ot_radio_state_has_tx_failed(void)
{
    return getInternalFlag(EVENT_TX_FAILED);
}

void sli_ot_radio_state_set_tx_failed(bool aFailed)
{
    setInternalFlag(EVENT_TX_FAILED, aFailed);
}

void sli_ot_radio_state_clear_tx_data_and_wait_for_ack(void)
{
    setInternalFlag(FLAG_ONGOING_TX_DATA | FLAG_WAITING_FOR_ACK | EVENT_SCHEDULED_TX_STARTED, false);
}

void sli_ot_radio_state_clear_all_scheduled_events(void)
{
    setInternalFlag(FLAG_SCHEDULED_RX_PENDING | FLAG_SCHEDULED_TX_PENDING | EVENT_SCHEDULED_TX_STARTED, false);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
bool sli_ot_radio_state_is_receiving_frame(void)
{
#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
    sl_rail_handle_t railHandle = sli_ot_radio_interface_get_rail_handle();

    return (sl_rail_get_radio_state(railHandle) & SL_RAIL_RF_STATE_RX_ACTIVE) == SL_RAIL_RF_STATE_RX_ACTIVE;
#else
    return false;
#endif
}

void sli_ot_radio_state_set_em_pending_data(bool aPending)
{
    sEmPendingData = aPending;
}

bool sli_ot_radio_state_get_em_pending_data(void)
{
    return sEmPendingData;
}

void sli_ot_radio_state_init(void)
{
    sMiscRadioState = 0;
    sEmPendingData  = false;
}

void sli_ot_radio_state_deinit(void)
{
    sMiscRadioState = 0;
    sEmPendingData  = false;
}

//==============================================================================
// Radio Event Processing
//==============================================================================

// Internal event state
static sl_rail_events_t sCurrentEventConfig = SL_RAIL_EVENTS_NONE;

// Forward declarations for internal event processing functions
static void processTxPacketSentEvent(void);
static void processTxChannelBusyEvent(void);
static void processTxBlockedEvent(void);
static void processTxUnderflowAbortedEvent(void);
static void processTxCcaEvents(sl_rail_events_t aEvents);
static void processRxPacketReceivedEvent(void);
#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
static void processRxSyncDetectedEvent(void);
static void processRxFilterPassedEvent(void);
static void processRxFrameErrorEvent(void);
static void processRxFilteredEvent(void);
#endif // SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
static void processAckSentEvent(void);
static void processAckAbortedEvent(void);
static void processAckBlockedEvent(void);
static void processScheduledTxEvent(void);
static void processScheduledTxMissedEvent(void);
static void processScheduledRxEvent(void);
static void processScheduledRxEndMissedEvent(void);

#ifdef SL_CATALOG_RAIL_UTIL_COEX_PRESENT
static void processCoexSignalDetectedEvent(void);
#endif // SL_CATALOG_RAIL_UTIL_COEX_PRESENT

static void processDataRequestCommandEvent(sl_rail_handle_t aRailHandle);

void sli_ot_radio_events_init(void)
{
    sCurrentEventConfig = SL_RAIL_EVENTS_NONE;
}

void sli_ot_radio_events_deinit(void)
{
    sCurrentEventConfig = SL_RAIL_EVENTS_NONE;
}

void sli_ot_radio_events_update_config(sl_rail_events_t mask, sl_rail_events_t values)
{
    sl_rail_status_t status;
    sl_rail_events_t newEventConfig = (sCurrentEventConfig & ~mask) | (values & mask);

    if (newEventConfig != sCurrentEventConfig)
    {
        sl_rail_handle_t railHandle = sli_ot_radio_interface_get_rail_handle();
        if (railHandle != nullptr)
        {
            status = sl_rail_config_events(railHandle, mask, values);

            if (status != SL_RAIL_STATUS_NO_ERROR)
            {
                otLogWarnPlat("Failed to configure radio events: %lu", status);
            }
            sCurrentEventConfig = newEventConfig;
        }
    }
}

#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
sl_rail_util_ieee802154_stack_event_t sli_ot_radio_events_handle_phy_stack_event_with_status(
    sl_rail_util_ieee802154_stack_event_t stackEvent,
    uint32_t                              supplement)
{
    sl_rail_util_ieee802154_stack_event_t status = SL_RAIL_UTIL_IEEE802154_STACK_STATUS_SUCCESS;

    if (phyStackEventIsEnabled())
    {
#ifdef SL_CATALOG_RAIL_MULTIPLEXER_PRESENT
        sl_rail_handle_t railHandle = sli_ot_radio_interface_get_rail_handle();
        otEXPECT_ACTION(railHandle != nullptr, status = SL_RAIL_UTIL_IEEE802154_STACK_STATUS_HOLDOFF);
        status = sl_rail_mux_ieee802154_on_event(railHandle, stackEvent, supplement);
#else
        status = sl_rail_util_ieee802154_on_event(stackEvent, supplement);
#endif
    }

#ifdef SL_CATALOG_RAIL_MULTIPLEXER_PRESENT
exit:
#endif
    return status;
}

void sli_ot_radio_events_handle_phy_stack_event(sl_rail_util_ieee802154_stack_event_t stackEvent, uint32_t supplement)
{
    sli_ot_radio_events_handle_phy_stack_event_with_status(stackEvent, supplement);
}
#endif // SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT

void sli_ot_radio_events_process_callback(sl_rail_handle_t aRailHandle, sl_rail_events_t aEvents)
{
    // Process RX sync detection events first
#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
    if (aEvents & (SL_RAIL_EVENT_RX_SYNC_0_DETECT | SL_RAIL_EVENT_RX_SYNC_1_DETECT))
    {
        processRxSyncDetectedEvent();
    }
#endif

    // Process coexistence events
#ifdef SL_CATALOG_RAIL_UTIL_COEX_PRESENT
    if (aEvents & SL_RAIL_EVENT_SIGNAL_DETECTED)
    {
        processCoexSignalDetectedEvent();
    }
#endif

    // Process data request command events
    if ((aEvents & SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND)
#ifdef SL_CATALOG_RAIL_UTIL_COEX_PRESENT
        && !sl_rail_is_rx_auto_ack_paused(aRailHandle)
#endif
    )
    {
        processDataRequestCommandEvent(aRailHandle);
    }

    // Process RX filter passed events
#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
    if (aEvents & SL_RAIL_EVENT_RX_FILTER_PASSED)
    {
        processRxFilterPassedEvent();
    }
#endif

    // Process TX events
    sli_ot_radio_events_process_tx_events(aEvents);

    // Process scheduled events for Thread 1.2+
#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
    if (sli_ot_radio_state_is_rx_scheduled())
    {
        sli_ot_radio_events_process_scheduled_rx_events(aEvents);
    }
    else
    {
        sli_ot_radio_events_process_scheduled_tx_events(aEvents);
    }
#endif

    // Process RX packet received events
    if (aEvents & SL_RAIL_EVENT_RX_PACKET_RECEIVED)
    {
        processRxPacketReceivedEvent();
    }

    // Process RX error events
#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
    if (aEvents & SL_RAIL_EVENT_RX_FRAME_ERROR)
    {
        processRxFrameErrorEvent();
    }

    if (aEvents
        & (SL_RAIL_EVENT_RX_PACKET_ABORTED | SL_RAIL_EVENT_RX_ADDRESS_FILTERED | SL_RAIL_EVENT_RX_FIFO_OVERFLOW))
    {
        processRxFilteredEvent();
    }
#endif

    // Process ACK events
    sli_ot_radio_events_process_ack_events(aEvents);

    if (aEvents & SL_RAIL_EVENT_CONFIG_UNSCHEDULED)
    {
        sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_IDLED, 0U);
#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
        railDebugCounters.mRailEventConfigUnScheduled++;
#endif
    }

    if (aEvents & SL_RAIL_EVENT_CONFIG_SCHEDULED)
    {
#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
        railDebugCounters.mRailEventConfigScheduled++;
#endif
    }

    if (aEvents & SL_RAIL_EVENT_SCHEDULER_STATUS)
    {
        schedulerEventCallback(aRailHandle);
    }

    if (aEvents & SL_RAIL_EVENT_CAL_NEEDED)
    {
        sl_rail_status_t status;

        status = sl_rail_calibrate(aRailHandle, NULL, SL_RAIL_CAL_ALL_PENDING);
        // Non-RTOS DMP case fails but is unsupported
#if (!defined(SL_CATALOG_BLUETOOTH_PRESENT) || defined(SL_CATALOG_KERNEL_PRESENT))
        // TEMPORARY - this asserts on Mux - OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
        OT_UNUSED_VARIABLE(status);
#else
        OT_UNUSED_VARIABLE(status);
#endif

#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
        railDebugCounters.mRailEventCalNeeded++;
#endif
    }

    // scheduled and unscheduled config events happen very often,
    // especially in a DMP situation where there is an active BLE connection.
    // Waking up the OT RTOS task on every one of these occurrences causes
    // a lower priority Serial task to starve and makes it appear like a code lockup
    // There is no reason to wake the OT task for these events!
    if (!(aEvents & SL_RAIL_EVENT_CONFIG_SCHEDULED) && !(aEvents & SL_RAIL_EVENT_CONFIG_UNSCHEDULED))
    {
        otSysEventSignalPending();
    }
}

void sli_ot_radio_events_process_tx_events(sl_rail_events_t aEvents)
{
    if (aEvents & SL_RAIL_EVENT_TX_PACKET_SENT)
    {
        processTxPacketSentEvent();
    }
    else if (aEvents & SL_RAIL_EVENT_TX_CHANNEL_BUSY)
    {
        processTxChannelBusyEvent();
    }
    else if (aEvents & SL_RAIL_EVENT_TX_BLOCKED)
    {
        processTxBlockedEvent();
    }
    else if (aEvents & (SL_RAIL_EVENT_TX_UNDERFLOW | SL_RAIL_EVENT_TX_ABORTED))
    {
        processTxUnderflowAbortedEvent();
    }
    else
    {
        // Process CCA-related events
        processTxCcaEvents(aEvents);
    }
}

void sli_ot_radio_events_process_rx_events(sl_rail_events_t aEvents)
{
    // RX events are processed in the main callback
    // This function is provided for future extensibility
    OT_UNUSED_VARIABLE(aEvents);
}

void sli_ot_radio_events_process_scheduled_tx_events(sl_rail_events_t aEvents)
{
    if (aEvents & SL_RAIL_EVENT_TX_SCHEDULED_TX_STARTED)
    {
        processScheduledTxEvent();
    }
    else if (aEvents & SL_RAIL_EVENT_TX_SCHEDULED_TX_MISSED)
    {
        processScheduledTxMissedEvent();
    }
}

void sli_ot_radio_events_process_scheduled_rx_events(sl_rail_events_t aEvents)
{
    if (aEvents & SL_RAIL_EVENT_RX_SCHEDULED_RX_STARTED)
    {
        processScheduledRxEvent();
    }

    if (aEvents & SL_RAIL_EVENT_RX_SCHEDULED_RX_END || aEvents & SL_RAIL_EVENT_RX_SCHEDULED_RX_MISSED)
    {
        processScheduledRxEndMissedEvent();
    }
}

void sli_ot_radio_events_process_ack_events(sl_rail_events_t aEvents)
{
    if (aEvents & SL_RAIL_EVENT_TXACK_PACKET_SENT)
    {
        processAckSentEvent();
    }

    if (aEvents & (SL_RAIL_EVENT_TXACK_ABORTED | SL_RAIL_EVENT_TXACK_UNDERFLOW))
    {
        processAckAbortedEvent();
    }

    if (aEvents & SL_RAIL_EVENT_TXACK_BLOCKED)
    {
        processAckBlockedEvent();
    }

    // Deal with ACK timeout after possible RX completion in case RAIL
    // notifies us of the ACK and the timeout simultaneously -- we want
    // the ACK to win over the timeout.
    if ((aEvents & SL_RAIL_EVENT_RX_ACK_TIMEOUT) && (sli_ot_radio_state_get_internal_flag(FLAG_WAITING_FOR_ACK)))
    {
        ackTimeoutCallback();
    }
}

void sli_ot_radio_events_process_coex_events(sl_rail_events_t aEvents)
{
    // Coexistence events are processed in the main callback
    // This function is provided for future extensibility
    OT_UNUSED_VARIABLE(aEvents);
}

void sli_ot_radio_events_process_data_request_events(sl_rail_handle_t aRailHandle, sl_rail_events_t aEvents)
{
    if (aEvents & SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND)
    {
        processDataRequestCommandEvent(aRailHandle);
    }
}

void sli_ot_radio_events_process_error_events(sl_rail_events_t aEvents)
{
    // Error events are processed in the main callback
    // This function is provided for future extensibility
    OT_UNUSED_VARIABLE(aEvents);
}

// Internal event processing functions
static inline void processTxPacketSentEvent(void)
{
    packetSentCallback(false);
}

static inline void processTxChannelBusyEvent(void)
{
    sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_BLOCKED,
                                               static_cast<uint32_t>(txWaitingForAck()));
    txFailedCallback(false, EVENT_TX_CCA_FAILED);
}

static inline void processTxBlockedEvent(void)
{
    sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_BLOCKED,
                                               static_cast<uint32_t>(txWaitingForAck()));
    txFailedCallback(false, EVENT_TX_FAILED);
}

static inline void processTxUnderflowAbortedEvent(void)
{
    sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ABORTED,
                                               static_cast<uint32_t>(txWaitingForAck()));
    txFailedCallback(false, EVENT_TX_FAILED);
}

static inline void processTxCcaEvents(sl_rail_events_t aEvents)
{
#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
    if (aEvents & SL_RAIL_EVENT_TX_START_CCA)
    {
        sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_CCA_SOON, 0U);
    }

    if (aEvents & SL_RAIL_EVENT_TX_CCA_RETRY)
    {
        sl_rail_handle_t railHandle = sli_ot_radio_interface_get_rail_handle();
        sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_CCA_BUSY,
                                                   static_cast<uint32_t>(sl_rail_is_next_cca_now(railHandle)));
    }

    if (aEvents & SL_RAIL_EVENT_TX_CHANNEL_CLEAR)
    {
        sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_STARTED, 0U);
    }
#else
    OT_UNUSED_VARIABLE(aEvents);
#endif
}

static inline void processRxPacketReceivedEvent(void)
{
    packetReceivedCallback();
}

#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
static inline void processRxSyncDetectedEvent(void)
{
    sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_STARTED,
                                               static_cast<uint32_t>(sli_ot_radio_state_is_receiving_frame()));
}

static inline void processRxFilterPassedEvent(void)
{
    sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ACCEPTED,
                                               static_cast<uint32_t>(sli_ot_radio_state_is_receiving_frame()));
}

static inline void processRxFrameErrorEvent(void)
{
    sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_CORRUPTED,
                                               static_cast<uint32_t>(sli_ot_radio_state_is_receiving_frame()));
}

static inline void processRxFilteredEvent(void)
{
    sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_FILTERED,
                                               static_cast<uint32_t>(sli_ot_radio_state_is_receiving_frame()));
}
#endif // SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT

static inline void processAckSentEvent(void)
{
    sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ACK_SENT,
                                               static_cast<uint32_t>(sli_ot_radio_state_is_receiving_frame()));
    packetSentCallback(true);
}

static inline void processAckAbortedEvent(void)
{
    sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ACK_ABORTED,
                                               static_cast<uint32_t>(sli_ot_radio_state_is_receiving_frame()));
    txFailedCallback(true, 0xFF);
}

static inline void processAckBlockedEvent(void)
{
    sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ACK_BLOCKED,
                                               static_cast<uint32_t>(sli_ot_radio_state_is_receiving_frame()));
}

static inline void processScheduledTxEvent(void)
{
    sli_ot_radio_state_set_internal_flag(EVENT_SCHEDULED_TX_STARTED, true);
    sli_ot_radio_state_set_internal_flag(FLAG_SCHEDULED_TX_PENDING, false);
}

static inline void processScheduledTxMissedEvent(void)
{
    sli_ot_radio_state_set_internal_flag(FLAG_SCHEDULED_TX_PENDING, false);
    txFailedCallback(false, EVENT_TX_SCHEDULER_ERROR);
}

static inline void processScheduledRxEvent(void)
{
    sli_ot_radio_state_set_internal_flag(EVENT_SCHEDULED_RX_STARTED, true);
}

static inline void processScheduledRxEndMissedEvent(void)
{
    sli_ot_radio_state_set_internal_flag(FLAG_SCHEDULED_RX_PENDING | EVENT_SCHEDULED_RX_STARTED, false);
    sli_ot_radio_state_set_idle();
}

#ifdef SL_CATALOG_RAIL_UTIL_COEX_PRESENT
static inline void processCoexSignalDetectedEvent(void)
{
    sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_SIGNAL_DETECTED, 0U);
}
#endif // SL_CATALOG_RAIL_UTIL_COEX_PRESENT

static inline void processDataRequestCommandEvent(sl_rail_handle_t aRailHandle)
{
    dataRequestCommandCallback(aRailHandle);
}
//==============================================================================

//------------------------------------------------------------------------------
// Helper Functions

#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
static bool phyStackEventIsEnabled(void)
{
    bool result = false;

#if (defined(SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT) && SL_RAIL_UTIL_ANT_DIV_RX_RUNTIME_PHY_SELECT)
    result = true;
#endif // SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT

#ifdef SL_CATALOG_RAIL_UTIL_COEX_PRESENT
    if (sli_ot_radio_interface_is_coex_enabled())
    {
        result |= sl_rail_util_coex_is_enabled();
#ifdef SL_RAIL_UTIL_COEX_RUNTIME_PHY_SELECT
        result |= SL_RAIL_UTIL_COEX_RUNTIME_PHY_SELECT;
#endif
    }
#endif // SL_CATALOG_RAIL_UTIL_COEX_PRESENT

    return result;
}

#endif // SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
static inline bool txWaitingForAck(void)
{
    return (sli_ot_radio_state_is_tx_data_ongoing() && sCurrentTxPacket != nullptr
            && ((sCurrentTxPacket->frame.mPsdu[0] & IEEE802154_FRAME_FLAG_ACK_REQUIRED) != 0));
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
static bool txIsDataRequest(void)
{
    bool     isDataRequest = false;
    uint16_t fcf;

    otEXPECT(sli_ot_radio_state_is_tx_data_ongoing() && sCurrentTxPacket != nullptr);

    fcf = (uint16_t)sCurrentTxPacket->frame.mPsdu[IEEE802154_FCF_OFFSET]
          | (uint16_t)(sCurrentTxPacket->frame.mPsdu[IEEE802154_FCF_OFFSET + 1] << 8);

    isDataRequest = ((fcf & IEEE802154_FRAME_TYPE_MASK) == IEEE802154_FRAME_TYPE_COMMAND);

exit:
    return isDataRequest;
}

#if (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)
static otError radioScheduleRx(uint8_t aChannel, uint32_t aStart, uint32_t aDuration)
{
    otError          error = OT_ERROR_NONE;
    sl_rail_status_t status;

#if FAST_CHANNEL_SWITCHING_SUPPORT && OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    // UID 1327639: Schedule Rx and Concurrent listening when used together
    // will cause undefined behavior. Therefore it was decided to assert upon detecting
    // this condition
    assert(0);
#endif // FAST_CHANNEL_SWITCHING_SUPPORT && OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE

    sl_rail_scheduler_info_t bgRxSchedulerInfo = {.priority         = SL_802154_RADIO_PRIO_BACKGROUND_RX_VALUE,
                                                  .slip_time        = 0,
                                                  .transaction_time = 0};

    // Configure scheduled receive as requested
    sl_rail_scheduled_rx_config_t rxCfg = {
        .start                      = aStart,
        .start_mode                 = SL_RAIL_TIME_ABSOLUTE,
        .end                        = aDuration,
        .end_mode                   = SL_RAIL_TIME_DELAY,
        .rx_transition_end_schedule = 0, // To stay in schedule Rx state after packet receive.
        .hard_window_end            = 0};           // This lets us receive a packet near a window-end-event

    status = sli_ot_radio_interface_rail_start_scheduled_rx(aChannel, &rxCfg, &bgRxSchedulerInfo);
    otEXPECT_ACTION(status == SL_RAIL_STATUS_NO_ERROR, error = OT_ERROR_FAILED);

    sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_LISTEN, 0U);
exit:
    return error;
}
#endif // (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)

template <typename EventCallback> static void efr32ConfigInit(EventCallback aEventCallback)
{
    // Set event callback in common config
    efr32CommonConfig *commonConfig           = sli_ot_radio_interface_get_common_config_ptr();
    commonConfig->mRailConfig.events_callback = aEventCallback;

    sli_init_power_manager();

    OT_ASSERT(sli_ot_radio_interface_rail_init(commonConfig) != nullptr);

    sli_ot_radio_events_update_config(SL_RAIL_EVENTS_ALL,
                                      (0 | SL_RAIL_EVENT_RX_ACK_TIMEOUT | SL_RAIL_EVENT_RX_PACKET_RECEIVED
#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
                                       | SL_RAIL_EVENT_TX_SCHEDULED_TX_STARTED | SL_RAIL_EVENT_TX_SCHEDULED_TX_MISSED
                                       | SL_RAIL_EVENT_RX_SCHEDULED_RX_STARTED | SL_RAIL_EVENT_RX_SCHEDULED_RX_END
                                       | SL_RAIL_EVENT_RX_SCHEDULED_RX_MISSED
#endif
                                       | SL_RAIL_EVENTS_TXACK_COMPLETION | SL_RAIL_EVENTS_TX_COMPLETION
                                       | SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND
#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT || RADIO_CONFIG_DMP_SUPPORT
                                       | SL_RAIL_EVENT_CONFIG_SCHEDULED | SL_RAIL_EVENT_CONFIG_UNSCHEDULED
                                       | SL_RAIL_EVENT_SCHEDULER_STATUS
#endif
                                       | SL_RAIL_EVENT_CAL_NEEDED));

    sli_ot_radio_interface_load_rail_config(sli_ot_radio_interface_get_band_config_ptr(),
                                            OPENTHREAD_CONFIG_DEFAULT_TRANSMIT_POWER);
}

void efr32RadioInit(void)
{
    if (sli_ot_radio_state_is_initialized())
    {
        return;
    }
    sl_rail_status_t            status;
    sl_status_t                 rxMemPoolStatus;
    bool                        queueStatus;
    sl_rail_timer_sync_config_t timer_sync_config = SL_RAIL_TIMER_SYNC_DEFAULT;

    // check if RAIL_TX_FIFO_SIZE is power of two..
    OT_ASSERT((RAIL_TX_FIFO_SIZE & (RAIL_TX_FIFO_SIZE - 1)) == 0);

    // check the limits of the RAIL_TX_FIFO_SIZE.
    OT_ASSERT((RAIL_TX_FIFO_SIZE >= SL_RAIL_MINIMUM_FIFO_BYTES) || (RAIL_TX_FIFO_SIZE <= SL_RAIL_MAXIMUM_FIFO_BYTES));

    sli_ot_radio_events_init();
    sli_ot_radio_interface_init_config();
    efr32ConfigInit(sli_ot_radio_events_process_callback);
    sli_ot_radio_state_mark_initialized();
    status = sli_ot_radio_interface_rail_config_sleep(&timer_sync_config);
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);

    sReceive.frame.mLength = 0;
    sReceive.frame.mPsdu   = nullptr;

    sReceiveAck.frame.mLength = 0;
    sReceiveAck.frame.mPsdu   = sReceiveAckPsdu;

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    sli_ot_radio_instance_init_command_queue();
#endif

    for (uint8_t i = 0; i < RADIO_REQUEST_BUFFER_COUNT; i++)
    {
        // Initialize the tx buffer params.
        sTransmitBuffer[i].instance      = nullptr;
        sTransmitBuffer[i].frame.mLength = 0;
        sTransmitBuffer[i].frame.mPsdu   = sTransmitPsdu[i];

#if OPENTHREAD_CONFIG_MAC_HEADER_IE_SUPPORT
        sTransmitBuffer[i].frame.mInfo.mTxInfo.mIeInfo = &sTransmitIeInfo[i];
#endif
    }

#if OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE
    otLinkMetricsInit(SL_OPENTHREAD_RECEIVE_SENSITIVITY);
#endif
    sli_ot_radio_interface_set_current_band_config(
        sli_ot_radio_interface_get_band_config(OPENTHREAD_CONFIG_DEFAULT_CHANNEL));
    OT_ASSERT(sli_ot_radio_interface_get_current_band_config() != nullptr);

    sl_rail_util_pa_init();
    sli_ot_radio_interface_set_tx_power(OPENTHREAD_CONFIG_DEFAULT_TRANSMIT_POWER);

    status = sli_ot_radio_interface_rail_config_rx_options(SL_RAIL_RX_OPTION_TRACK_ABORTED_FRAMES,
                                                           SL_RAIL_RX_OPTION_TRACK_ABORTED_FRAMES);
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
    efr32PhyStackInit();
    sli_ot_radio_interface_set_cca_mode(SL_OPENTHREAD_RADIO_CCA_MODE);

    sli_ot_energy_scan_init();

#if FAST_CHANNEL_SWITCHING_SUPPORT && OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    sli_ot_radio_channel_switching_init();
#endif

#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
    sli_ot_radio_csl_init();
#endif

    // Initialize the queue for received packets.
    queueStatus = queueInit(&sRxPacketQueue, SL_OPENTHREAD_RADIO_RX_BUFFER_COUNT);
    OT_ASSERT(queueStatus);

    // Specify a callback to be called upon queue overflow.
    queueStatus = queueOverflow(&sRxPacketQueue, &rxPacketQueueOverflowCallback);
    OT_ASSERT(queueStatus);

    // Initialize the memory pool for rx packets.
    rxMemPoolStatus =
        sl_memory_create_pool(sizeof(rxBuffer), SL_OPENTHREAD_RADIO_RX_BUFFER_COUNT, &sRxPacketMemPoolHandle);
    OT_ASSERT(rxMemPoolStatus == SL_STATUS_OK);

    otLogInfoPlat("Initialized");
}

void efr32RadioDeinit(void)
{
    sl_rail_status_t status;

    sli_ot_radio_interface_rail_idle_abort();
    status = sli_ot_radio_interface_rail_config_events(SL_RAIL_EVENTS_ALL, 0);
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);

    sli_ot_radio_interface_set_current_band_config(nullptr);
    sli_ot_radio_events_deinit();

    // Clean up any ongoing energy scan
    sli_ot_energy_scan_deinit();

#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
    sli_ot_radio_csl_deinit();
#endif

    sl_memory_delete_pool(&sRxPacketMemPoolHandle);
}

//------------------------------------------------------------------------------
// Stack support

void otPlatRadioSetPanId(otInstance *aInstance, uint16_t aPanId)
{
    sl_rail_status_t status;
    panIndex_t       panIndex = sli_ot_radio_instance_get_pan_index(aInstance);

    otEXPECT(sl_ot_rtos_task_can_access_pal());
    otLogInfoPlat("PANID=%X index=%u", aPanId, panIndex);
    utilsSoftSrcMatchSetPanId(aInstance, aPanId);

    status = sli_ot_radio_interface_rail_set_pan_id(aPanId, panIndex);
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    sli_ot_radio_instance_update_rail_filter_mask_for_pan_id(aPanId, panIndex);
#endif
exit:
    return;
}

void otPlatRadioSetExtendedAddress(otInstance *aInstance, const otExtAddress *aAddress)
{
    sl_rail_status_t status;
    panIndex_t       panIndex = sli_ot_radio_instance_get_pan_index(aInstance);

    otEXPECT(sl_ot_rtos_task_can_access_pal());

    for (size_t i = 0; i < sizeof(*aAddress); i++)
    {
        sExtAddress[panIndex].m8[i] = aAddress->m8[sizeof(*aAddress) - 1 - i];
    }

    otLogInfoPlat("ExtAddr=%X%X%X%X%X%X%X%X index=%u",
                  aAddress->m8[7],
                  aAddress->m8[6],
                  aAddress->m8[5],
                  aAddress->m8[4],
                  aAddress->m8[3],
                  aAddress->m8[2],
                  aAddress->m8[1],
                  aAddress->m8[0],
                  panIndex);

    status = sli_ot_radio_interface_rail_set_long_address((const uint8_t *)aAddress->m8, panIndex);
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);

exit:
    return;
}

void otPlatRadioSetShortAddress(otInstance *aInstance, uint16_t aAddress)
{
    sl_rail_status_t status;
    panIndex_t       panIndex = sli_ot_radio_instance_get_pan_index(aInstance);

    otEXPECT(sl_ot_rtos_task_can_access_pal());
    otLogInfoPlat("ShortAddr=%X index=%u", aAddress, panIndex);

    status = sli_ot_radio_interface_rail_set_short_address(aAddress, panIndex);
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);

exit:
    return;
}

otError otPlatRadioEnable(otInstance *aInstance)
{
    otError error = OT_ERROR_NONE;

    otEXPECT(!otPlatRadioIsEnabled(aInstance));

    otLogInfoPlat("State=OT_RADIO_STATE_SLEEP");

exit:
    return error;
}

otError otPlatRadioDisable(otInstance *aInstance)
{
    otError error = OT_ERROR_NONE;

    otEXPECT_ACTION(sl_ot_rtos_task_can_access_pal(), error = OT_ERROR_REJECTED);
    otEXPECT_ACTION(otPlatRadioIsEnabled(aInstance), error = OT_ERROR_INVALID_STATE);

    otLogInfoPlat("State=OT_RADIO_STATE_DISABLED");

exit:
    return error;
}

otError otPlatRadioReceive(otInstance *aInstance, uint8_t aChannel)
{
    otError          error = OT_ERROR_NONE;
    sl_rail_status_t status;
    int8_t           txPower;

    otEXPECT_ACTION(sl_ot_rtos_task_can_access_pal(), error = OT_ERROR_REJECTED);
    otEXPECT_ACTION(!sli_ot_radio_state_is_tx_data_ongoing() && !sli_ot_energy_scan_is_blocking_receive(aInstance),
                    error = OT_ERROR_INVALID_STATE);

#if (FAST_CHANNEL_SWITCHING_SUPPORT && OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE)
    sli_ot_radio_channel_switching_configure(aInstance, aChannel);
#endif

    txPower = sl_get_tx_power_for_current_channel(aInstance);
    error   = sli_ot_radio_interface_load_channel_config(aChannel, txPower);
    otEXPECT(error == OT_ERROR_NONE);

    status = sli_ot_radio_interface_set_rx(aChannel);
    otEXPECT_ACTION(status == SL_RAIL_STATUS_NO_ERROR, error = OT_ERROR_FAILED);
    sli_ot_radio_state_set_scheduled_rx_pending(false);

    sReceive.frame.mChannel    = aChannel;
    sReceiveAck.frame.mChannel = aChannel;

exit:
    return error;
}

#if (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)
otError otPlatRadioReceiveAt(otInstance *aInstance, uint8_t aChannel, uint32_t aStart, uint32_t aDuration)
{
    otError error   = OT_ERROR_NONE;
    int8_t  txPower = sl_get_tx_power_for_current_channel(aInstance);

    // We can only have one schedule request i.e. either Rx or Tx as they use the
    // same RAIL resources.
    otEXPECT_ACTION(!sli_ot_radio_state_is_tx_scheduled(), error = OT_ERROR_FAILED);

    otEXPECT_ACTION(sl_ot_rtos_task_can_access_pal(), error = OT_ERROR_REJECTED);
    OT_UNUSED_VARIABLE(aInstance);

    error = sli_ot_radio_interface_load_channel_config(aChannel, txPower);
    otEXPECT(error == OT_ERROR_NONE);

    // Set the flag first and then schedule the Rx as the rail scheduler can trigger the events even before
    // sl_rail_start_scheduled_rx() API returns the status if start time is too close to the current time which could
    // otherwise cause the race condition.
    sli_ot_radio_state_set_scheduled_rx_pending(true);
    error = radioScheduleRx(aChannel, aStart, aDuration);
    otEXPECT_ACTION(error == OT_ERROR_NONE, sli_ot_radio_state_set_scheduled_rx_pending(false));

    // Set channel for global receive state
    sReceive.frame.mChannel    = aChannel;
    sReceiveAck.frame.mChannel = aChannel;

exit:
    return error;
}
#endif // (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)

otError otPlatRadioTransmit(otInstance *aInstance, otRadioFrame *aFrame)
{
    otError         error      = OT_ERROR_NONE;
    int8_t          txPower    = sl_get_tx_power_for_current_channel(aInstance);
    instanceIndex_t txBufIndex = sli_ot_radio_instance_get_index(aInstance);

    otEXPECT_ACTION(sl_ot_rtos_task_can_access_pal(), error = OT_ERROR_REJECTED);

#if OPENTHREAD_CONFIG_MULTIPAN_RCP_ENABLE
    // Accept GP packets even if radio is not in required state.
    if ((sl_gp_intf_get_state() != SL_GP_STATE_SEND_RESPONSE) && sl_gp_intf_should_buffer_pkt(aInstance, aFrame, false))
    {
        sl_gp_intf_buffer_pkt(aInstance);
    }
    else
#endif
    {
        OT_ASSERT(txBufIndex < RADIO_REQUEST_BUFFER_COUNT);
        OT_ASSERT(aFrame == &sTransmitBuffer[txBufIndex].frame);
        OT_ASSERT(aFrame->mPsdu == sTransmitPsdu[txBufIndex]);

        if (!aFrame->mInfo.mTxInfo.mIsARetx)
        {
            sTransmitBuffer[txBufIndex].currentRadioTxPriority = SL_802154_RADIO_PRIO_TX_MIN;
        }
        else if (sTransmitBuffer[txBufIndex].currentRadioTxPriority > SL_802154_RADIO_PRIO_TX_STEP)
        {
            sTransmitBuffer[txBufIndex].currentRadioTxPriority -= SL_802154_RADIO_PRIO_TX_STEP;
        }
        // TX priority is always bounded by the maximum priority configured
        if (sTransmitBuffer[txBufIndex].currentRadioTxPriority < SL_802154_RADIO_PRIO_TX_MAX)
        {
            sTransmitBuffer[txBufIndex].currentRadioTxPriority = SL_802154_RADIO_PRIO_TX_MAX;
        }

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
        // Push pending transmit and exit if radio is busy.
        if (sli_ot_radio_state_is_transmitting_or_scanning())
        {
            sli_ot_radio_instance_queue_transmit(aInstance, aFrame);
            ExitNow(error = OT_ERROR_NONE);
        }
#endif
        error = sli_ot_radio_interface_load_channel_config(aFrame->mChannel, txPower);
        otEXPECT(error == OT_ERROR_NONE);

        OT_ASSERT(!sli_ot_radio_state_is_tx_data_ongoing());

        sli_ot_radio_state_clear_all_tx_events();
        sTransmitBuffer[txBufIndex].instance = aInstance;
        sCurrentTxPacket                     = &sTransmitBuffer[txBufIndex];

        sli_ot_radio_state_set_using_csma(aFrame->mInfo.mTxInfo.mCsmaCaEnabled);

#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
        sli_ot_radio_csl_set_present(aInstance, aFrame->mInfo.mTxInfo.mCslPresent);
#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
        if (sli_ot_radio_csl_get_period(aInstance) > 0 && sCurrentTxPacket->frame.mInfo.mTxInfo.mTxDelay == 0)
        {
            // Only called for CSL children (CSL period > 0)
            // Note: Our SSEDs "schedule" transmissions to their parent in order to know
            // exactly when in the future the data packets go out so they can calculate
            // the accurate CSL phase to send to their parent.
            sCurrentTxPacket->frame.mInfo.mTxInfo.mTxDelayBaseTime = sl_rail_get_time(SL_RAIL_EFR32_HANDLE);
            sCurrentTxPacket->frame.mInfo.mTxInfo.mTxDelay =
                SCHEDULE_TX_DELAY_US; // Chosen after internal certification testing
            sli_ot_radio_csl_set_present(aInstance, true);
        }
#endif
        updateIeInfoTxFrame(sCurrentTxPacket->frame.mInfo.mTxInfo.mTxDelayBaseTime
                            + sCurrentTxPacket->frame.mInfo.mTxInfo.mTxDelay + SHR_DURATION_US);

        // Note - we need to call this outside of txCurrentPacket as for Series 2,
        // this results in calling the SE interface from a critical section which is not permitted.
        (void)sli_ot_radio_security_process_transmit(&sCurrentTxPacket->frame, sCurrentTxPacket->instance);
#endif // OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2

        CORE_DECLARE_IRQ_STATE;
        CORE_ENTER_ATOMIC();
        sli_ot_radio_state_set_tx_data_ongoing(true);
        tryTxCurrentPacket();
        CORE_EXIT_ATOMIC();

        if (sli_ot_radio_state_has_tx_failed())
        {
            otPlatRadioTxStarted(aInstance, aFrame);
        }
    }
exit:
    return error;
}

#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
void updateIeInfoTxFrame(uint32_t shrTxTime)
{
    OT_ASSERT(sCurrentTxPacket != nullptr);

#if OPENTHREAD_CONFIG_MAC_HEADER_IE_SUPPORT && OPENTHREAD_CONFIG_TIME_SYNC_ENABLE
    // Seek the time sync offset and update the rendezvous time
    if (sCurrentTxPacket->frame.mInfo.mTxInfo.mIeInfo->mTimeIeOffset != 0)
    {
        uint8_t *timeIe = sCurrentTxPacket->frame.mPsdu + sCurrentTxPacket->frame.mInfo.mTxInfo.mIeInfo->mTimeIeOffset;
        uint64_t time   = otPlatTimeGet();

        // Use the same mechanism as CSL children to schedule tx for the future without CSMA
        // so that the value written to the time IE exactly matches the actual tx time of the packet
        sCurrentTxPacket->frame.mInfo.mTxInfo.mTxDelayBaseTime =
            (uint32_t)time; // Bottom 32 bits are sl_rail_get_time()
        sCurrentTxPacket->frame.mInfo.mTxInfo.mTxDelay = SCHEDULE_TX_DELAY_US;
        time += sCurrentTxPacket->frame.mInfo.mTxInfo.mIeInfo->mNetworkTimeOffset
                + sCurrentTxPacket->frame.mInfo.mTxInfo.mTxDelay + SHR_DURATION_US;
        *timeIe = sCurrentTxPacket->frame.mInfo.mTxInfo.mIeInfo->mTimeSyncSeq;

        *(++timeIe) = (uint8_t)(time & 0xff);
        for (uint8_t i = 1; i < sizeof(uint64_t); i++)
        {
            time        = time >> 8;
            *(++timeIe) = (uint8_t)(time & 0xff);
        }
    }
#endif // OPENTHREAD_CONFIG_MAC_HEADER_IE_SUPPORT && OPENTHREAD_CONFIG_TIME_SYNC_ENABLE

#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
    // Update IE data in the 802.15.4 header with the newest CSL period / phase
    otInstance *instance = sCurrentTxPacket->instance;
    if (sli_ot_radio_csl_get_period(instance) > 0 && !sCurrentTxPacket->frame.mInfo.mTxInfo.mIsHeaderUpdated)
    {
        otMacFrameSetCslIe(&sCurrentTxPacket->frame,
                           (uint16_t)sli_ot_radio_csl_get_period(instance),
                           sli_ot_radio_csl_get_phase(instance, shrTxTime));
    }
#else
    OT_UNUSED_VARIABLE(shrTxTime);
#endif // OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
}
#endif // OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2

void txCurrentPacket(void)
{
    OT_ASSERT(sli_ot_radio_state_is_tx_data_ongoing());
    OT_ASSERT(sCurrentTxPacket != nullptr);

    sl_rail_tx_options_t txOptions = SL_RAIL_TX_OPTIONS_DEFAULT;
    sl_rail_status_t     status    = SL_RAIL_STATUS_INVALID_STATE;
    uint8_t              frameLength;
    bool                 ackRequested;

#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
    railDebugCounters.mRailPlatTxTriggered++;
#endif
    // signalling this event earlier, as this event can OT_ASSERT REQ (expecially for a
    // non-CSMA transmit) giving the Coex master a little more time to grant or deny.
    sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_PENDED_PHY,
                                               static_cast<uint32_t>(sli_ot_radio_state_is_using_csma()));

    frameLength = (uint8_t)sCurrentTxPacket->frame.mLength;

    if (PHY_HEADER_SIZE == 1)
    {
        sli_ot_radio_interface_rail_write_tx_fifo(&frameLength, sizeof frameLength, true);
    }
    else
    { // 2 byte PHR for Sub-GHz
        uint8_t PHRByte1 = (0x08U /*FCS=2byte*/ | 0x10U /*Whiten=enabled*/);
        uint8_t PHRByte2 = (uint8_t)(__RBIT(frameLength) >> 24);

        sli_ot_radio_interface_rail_write_tx_fifo(&PHRByte1, sizeof PHRByte1, true);
        sli_ot_radio_interface_rail_write_tx_fifo(&PHRByte2, sizeof PHRByte2, false);
    }
    sli_ot_radio_interface_rail_write_tx_fifo(sCurrentTxPacket->frame.mPsdu, frameLength - 2, false);

    sl_rail_scheduler_info_t txSchedulerInfo = {
        .priority         = sCurrentTxPacket->currentRadioTxPriority,
        .slip_time        = RADIO_SCHEDULER_CHANNEL_SLIP_TIME,
        .transaction_time = 0 // will be calculated later if DMP is used
    };

    ackRequested = (sCurrentTxPacket->frame.mPsdu[0] & IEEE802154_FRAME_FLAG_ACK_REQUIRED);
    if (ackRequested)
    {
        txOptions |= SL_RAIL_TX_OPTION_WAIT_FOR_ACK;

#if RADIO_CONFIG_DMP_SUPPORT
        // time we wait for ACK
        if (sli_ot_radio_interface_rail_get_symbol_rate() > 0)
        {
            txSchedulerInfo.transaction_time +=
                (sl_rail_time_t)(12 * 1e6 / sli_ot_radio_interface_rail_get_symbol_rate());
        }
        else
        {
            txSchedulerInfo.transaction_time += 12 * RADIO_TIMING_DEFAULT_SYMBOLTIME_US;
        }
#endif
    }

#ifdef SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT
    // Update Tx options to use currently-selected antenna.
    // If antenna diverisity on Tx is disabled, leave both options 0
    // so Tx antenna tracks Rx antenna.
    if (sl_rail_util_ant_div_get_tx_antenna_mode() != SL_RAIL_UTIL_ANT_DIV_DISABLED)
    {
        txOptions |= ((sl_rail_util_ant_div_get_tx_antenna_selected() == SL_RAIL_UTIL_ANTENNA_SELECT_ANTENNA1)
                          ? SL_RAIL_TX_OPTION_ANTENNA_0
                          : SL_RAIL_TX_OPTION_ANTENNA_1);
    }
#endif // SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT

#if RADIO_CONFIG_DMP_SUPPORT
    // time needed for the frame itself
    // 4B preamble, 1B SFD, 1B PHR is not counted in frameLength
    if (sli_ot_radio_interface_rail_get_bit_rate() > 0)
    {
        txSchedulerInfo.transaction_time +=
            (sl_rail_time_t)((frameLength + 4 + 1 + 1) * 8 * 1e6 / sli_ot_radio_interface_rail_get_bit_rate());
    }
    else
    { // assume 250kbps
        txSchedulerInfo.transaction_time += (frameLength + 4 + 1 + 1) * RADIO_TIMING_DEFAULT_BYTETIME_US;
    }
#endif

    // Prioritize the Tx over schedule Rx to avoid missing data check-ins such as data polls.
    if (sli_ot_radio_state_is_rx_scheduled())
    {
        sli_ot_radio_interface_rail_idle();
        sli_ot_radio_state_set_scheduled_rx_pending(false);
        sli_ot_radio_state_set_scheduled_rx_started(false);
    }

    if (sCurrentTxPacket->frame.mInfo.mTxInfo.mTxDelay == 0)
    {
        if (sli_ot_radio_state_is_using_csma())
        {
#if RADIO_CONFIG_DMP_SUPPORT
            // time needed for CSMA/CA
            txSchedulerInfo.transaction_time += RADIO_TIMING_CSMA_OVERHEAD_US;
#endif
            csmaConfig.csma_tries        = sCurrentTxPacket->frame.mInfo.mTxInfo.mMaxCsmaBackoffs;
            csmaConfig.cca_threshold_dbm = sli_ot_radio_interface_get_cca_threshold();

            status = sli_ot_radio_interface_rail_start_cca_csma_tx(sCurrentTxPacket->frame.mChannel,
                                                                   txOptions,
                                                                   &csmaConfig,
                                                                   &txSchedulerInfo);
        }
        else
        {
            status =
                sli_ot_radio_interface_rail_start_tx(sCurrentTxPacket->frame.mChannel, txOptions, &txSchedulerInfo);
        }

        if (status == SL_RAIL_STATUS_NO_ERROR)
        {
            sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_STARTED, 0U);
        }
    }
    else
    {
        // For CSL transmitters (FTDs):
        // mTxDelayBaseTime = rx-timestamp (end of sync word) when we received CSL-sync with IEs
        // mTxDelay = Delay starting from mTxDelayBaseTime
        //
        // For CSL receivers (SSEDs):
        // mTxDelayBaseTime = timestamp when otPlatRadioTransmit is called
        // mTxDelay = Chosen value in the future where transmit is scheduled, so we know exactly
        // when to calculate the phase (we can't do this on-the-fly as the packet is going out
        // due to platform limitations.  see radioScheduleRx)
        //
        // Note that both use single CCA config, overriding any CCA/CSMA configs from the stack
        //
#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
        sl_rail_scheduled_tx_config_t scheduleTxOptions = {
            .when = sCurrentTxPacket->frame.mInfo.mTxInfo.mTxDelayBaseTime
                    + sCurrentTxPacket->frame.mInfo.mTxInfo.mTxDelay - SHR_DURATION_US,
            .mode         = SL_RAIL_TIME_ABSOLUTE,
            .tx_during_rx = SL_RAIL_SCHEDULED_TX_DURING_RX_POSTPONE_TX};

        // Set ccaBackoff to some constant value, so we have predictable radio warmup time for schedule tx.
        cslCsmaConfig.cca_backoff_us = CSL_CSMA_BACKOFF_TIME_IN_US;
        scheduleTxOptions.when -= cslCsmaConfig.cca_backoff_us;

        // CSL transmissions don't use CSMA but MAC accounts for single CCA time.
        // cslCsmaConfig is set to SL_RAIL_CSMA_CONFIG_SINGLE_CCA above.
        status = sli_ot_radio_interface_rail_start_scheduled_cca_csma_tx(sCurrentTxPacket->frame.mChannel,
                                                                         txOptions,
                                                                         &scheduleTxOptions,
                                                                         &cslCsmaConfig,
                                                                         &txSchedulerInfo);

        if (status == SL_RAIL_STATUS_NO_ERROR)
        {
            sli_ot_radio_state_set_scheduled_tx_pending(true);
#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
            railDebugCounters.mRailEventsScheduledTxTriggeredCount++;
#endif
            sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_STARTED, 0U);
        }
#endif
    }

    if (status == SL_RAIL_STATUS_NO_ERROR)
    {
#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
        railDebugCounters.mRailTxStarted++;
#endif
    }
    else
    {
#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
        railDebugCounters.mRailTxStartFailed++;
#endif
        sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_BLOCKED,
                                                   static_cast<uint32_t>(ackRequested));
        txFailedCallback(false, EVENT_TX_FAILED);

        otSysEventSignalPending();
    }
}

// This API gets called from init procedure so instance to IID mapping does not exist
// at that point. Also this api will get called sequentially so assign new transmit
// buffer for the given instance.
otRadioFrame *otPlatRadioGetTransmitBuffer(otInstance *aInstance)
{
    uint8_t       index       = 0;
    otRadioFrame *aRadioFrame = nullptr;

    otEXPECT(sl_ot_rtos_task_can_access_pal());

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    index = otInstanceGetIndex(aInstance);
#else
    // In single-instance mode, always use index 0
    OT_UNUSED_VARIABLE(aInstance);
#endif

    aRadioFrame = &sTransmitBuffer[index].frame;

exit:
    return aRadioFrame;
}

int8_t otPlatRadioGetRssi(otInstance *aInstance)
{
    int8_t  rssi    = OT_RADIO_RSSI_INVALID;
    uint8_t channel = sReceive.frame.mChannel;

    otEXPECT(sl_ot_rtos_task_can_access_pal());
    otEXPECT(!sli_ot_radio_state_is_tx_data_ongoing());

#if (FAST_CHANNEL_SWITCHING_SUPPORT && OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE)
    channel = sli_ot_radio_channel_switching_get_channel(aInstance);
#endif

    sli_ot_energy_scan(aInstance, channel, SL_OPENTHREAD_RSSI_AVERAGING_TIME, &rssi);

exit:
    return rssi;
}

otError otPlatRadioSetTransmitPower(otInstance *aInstance, int8_t aPower)
{
    return sli_set_default_tx_power(aInstance, aPower);
}

// Required for RCP error recovery
// See src/lib/spinel/radio_spinel.cpp::RestoreProperties()
otError otPlatRadioSetChannelMaxTransmitPower(otInstance *aInstance, uint8_t aChannel, int8_t aMaxPower)
{
    return sli_set_channel_max_tx_power(aInstance, aChannel, aMaxPower);
}

otError otPlatRadioEnergyScan(otInstance *aInstance, uint8_t aScanChannel, uint16_t aScanDuration)
{
    otError error       = OT_ERROR_NONE;
    bool    shouldDefer = false;

    otEXPECT_ACTION(sl_ot_rtos_task_can_access_pal(), error = OT_ERROR_REJECTED);

    shouldDefer = sli_ot_radio_instance_energy_scan_should_defer();
    otEXPECT_ACTION(!shouldDefer, error = OT_ERROR_BUSY);

    error = sli_ot_energy_scan_status_to_ot_error(
        sli_ot_energy_scan_async(aInstance, aScanChannel, (sl_rail_time_t)aScanDuration * US_IN_MS));

exit:
    return error;
}

//------------------------------------------------------------------------------
// Radio Config: Thread 1.2 transmit security support

#if (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)
void otPlatRadioSetMacKey(otInstance             *aInstance,
                          uint8_t                 aKeyIdMode,
                          uint8_t                 aKeyId,
                          const otMacKeyMaterial *aPrevKey,
                          const otMacKeyMaterial *aCurrKey,
                          const otMacKeyMaterial *aNextKey,
                          otRadioKeyType          aKeyType)
{
    sli_ot_radio_security_set_mac_key(aInstance, aKeyIdMode, aKeyId, aPrevKey, aCurrKey, aNextKey, aKeyType);
}

void otPlatRadioSetMacFrameCounter(otInstance *aInstance, uint32_t aMacFrameCounter)
{
    sli_ot_radio_security_set_mac_frame_counter(aInstance, aMacFrameCounter);
}

void otPlatRadioSetMacFrameCounterIfLarger(otInstance *aInstance, uint32_t aMacFrameCounter)
{
    sli_ot_radio_security_set_mac_frame_counter_if_larger(aInstance, aMacFrameCounter);
}
#endif // (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)

#if (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)
//------------------------------------------------------------------------------
// Radio implementation: Enhanced ACKs, CSL

// Return false if we should generate an immediate ACK
// Return true otherwise
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
static bool writeIeee802154EnhancedAck(sl_rail_handle_t          aRailHandle,
                                       sl_rail_rx_packet_info_t *packetInfoForEnhAck,
                                       uint32_t                  rxTimestamp,
                                       uint8_t                  *initialPktReadBytes,
                                       uint8_t                  *receivedPsdu)
{
    // RAIL will generate an Immediate ACK for us.
    // For an Enhanced ACK, we need to generate the whole packet ourselves.

    // An 802.15.4 packet from RAIL should look like:
    // 1/2 |   1/2  | 0/1  |  0/2   | 0/2/8  |  0/2   | 0/2/8  |   14
    // PHR | MacFCF | Seq# | DstPan | DstAdr | SrcPan | SrcAdr | SecHdr

    // With sl_rail_ieee802154_enable_early_frame_pending(), SL_RAIL_EVENT_IEEE802154_DATA_REQUEST_COMMAND
    // is triggered after receiving through the SrcAdr field of Version 0/1 packets,
    // and after receiving through the SecHdr for Version 2 packets.

    otRadioFrame receivedFrame, enhAckFrame;
    uint8_t      enhAckPsdu[IEEE802154_MAX_LENGTH];

#define EARLY_FRAME_PENDING_EXPECTED_BYTES (2U + 2U + 1U + 2U + 8U + 2U + 8U + 14U)
#define FINAL_PACKET_LENGTH_WITH_IE (EARLY_FRAME_PENDING_EXPECTED_BYTES + OT_ACK_IE_MAX_SIZE)

    otMacAddress     aSrcAddress;
    uint8_t          linkMetricsDataLen;
    uint8_t         *dataPtr;
    bool             setFramePending;
    uint8_t         *macFcfPointer;
    otInstance      *instance = nullptr;
    sl_rail_status_t enhAckStatus;

    otEXPECT((packetInfoForEnhAck != nullptr) && (initialPktReadBytes != nullptr) && (receivedPsdu != nullptr));

    *initialPktReadBytes = readInitialPacketData(packetInfoForEnhAck,
                                                 EARLY_FRAME_PENDING_EXPECTED_BYTES,
                                                 (PHY_HEADER_SIZE + 2),
                                                 receivedPsdu,
                                                 FINAL_PACKET_LENGTH_WITH_IE);

    if (*initialPktReadBytes == 0U)
    {
        return true; // Nothing to read, which means generating an immediate ACK is also pointless
    }

    receivedFrame.mPsdu = receivedPsdu + PHY_HEADER_SIZE;
    // This should be set to the expected length of the packet is being received.
    // We consider this while calculating the phase value below.
    receivedFrame.mLength = packetInfoForEnhAck->p_first_portion_data[0];
    enhAckFrame.mPsdu     = enhAckPsdu + PHY_HEADER_SIZE;

    if (!otMacFrameIsVersion2015(&receivedFrame))
    {
        return false;
    }

    linkMetricsDataLen = 0;
    dataPtr            = nullptr;
    setFramePending    = false;

    instance = sli_ot_radio_instance_from_filter_mask(packetInfoForEnhAck->filter_mask);

    (void)otMacFrameGetSrcAddr(&receivedFrame, &aSrcAddress);

    if (instance != nullptr && sli_ot_radio_interface_is_src_match_enabled()
        && (aSrcAddress.mType != OT_MAC_ADDRESS_TYPE_NONE))
    {
        setFramePending = (aSrcAddress.mType == OT_MAC_ADDRESS_TYPE_EXTENDED
                               ? (utilsSoftSrcMatchExtFindEntry(instance, &aSrcAddress.mAddress.mExtAddress) >= 0)
                               : (utilsSoftSrcMatchShortFindEntry(instance, aSrcAddress.mAddress.mShortAddress) >= 0));
    }

    // Generate our IE header.
    // Write IE data for enhanced ACK (link metrics + allocate bytes for CSL)

#if OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE
    uint8_t linkMetricsData[OT_ENH_PROBING_IE_DATA_MAX_SIZE];

    linkMetricsDataLen = otLinkMetricsEnhAckGenData(&aSrcAddress, sLastLqi, sLastRssi, linkMetricsData);

    if (linkMetricsDataLen > 0)
    {
        dataPtr = linkMetricsData;
    }
#endif

    sAckIeDataLength = generateAckIeData(instance, dataPtr, linkMetricsDataLen, &receivedFrame);

    otEXPECT(otMacFrameGenerateEnhAck(&receivedFrame, setFramePending, sAckIeData, sAckIeDataLength, &enhAckFrame)
             == OT_ERROR_NONE);

#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
    sli_ot_radio_csl_update_enh_ack_ie(instance,
                                       &enhAckFrame,
                                       rxTimestamp,
                                       packetInfoForEnhAck->packet_bytes,
                                       receivedFrame.mLength);
#else
    OT_UNUSED_VARIABLE(rxTimestamp);
#endif

    if (otMacFrameIsSecurityEnabled(&enhAckFrame))
    {
        otEXPECT(sli_ot_radio_security_process_transmit(&enhAckFrame, instance) == OT_ERROR_NONE);
    }

    // Before we're done, store some important info in reserved bits in the
    // MAC header (cleared later)
    // Check whether frame pending is set.
    // Check whether enhanced ACK is secured.
    otEXPECT((skipRxPacketLengthBytes(packetInfoForEnhAck)) == OT_ERROR_NONE);
    macFcfPointer = ((packetInfoForEnhAck->first_portion_bytes == 0) ? packetInfoForEnhAck->p_last_portion_data
                                                                     : packetInfoForEnhAck->p_first_portion_data);

    if (otMacFrameIsSecurityEnabled(&enhAckFrame))
    {
        *macFcfPointer |= IEEE802154_SECURED_OUTGOING_ENHANCED_ACK;
    }

    if (setFramePending)
    {
        *macFcfPointer |= IEEE802154_FRAME_PENDING_SET_IN_OUTGOING_ACK;
    }

    // Fill in PHR now that we know Enh-ACK's length
    if (PHY_HEADER_SIZE == 2U)
    { // Not true till SubGhz implementation is in place
        enhAckPsdu[0] = (0x08U /*FCS=2byte*/ | 0x10U /*Whiten=enabled*/);
        enhAckPsdu[1] = (uint8_t)(__RBIT(enhAckFrame.mLength) >> 24);
    }
    else
    {
        enhAckPsdu[0] = enhAckFrame.mLength;
    }

    enhAckStatus = sl_rail_ieee802154_write_enh_ack(aRailHandle, enhAckPsdu, enhAckFrame.mLength);
#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
    otEXPECT_ACTION(enhAckStatus == SL_RAIL_STATUS_NO_ERROR, railDebugCounters.mRailEventsEnhAckTxFailed++);
#else
    otEXPECT(enhAckStatus == SL_RAIL_STATUS_NO_ERROR);
#endif

exit:
    return true;
}
#endif // (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)

//------------------------------------------------------------------------------
// RAIL callbacks

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
void dataRequestCommandCallback(sl_rail_handle_t aRailHandle)
{
#define MAX_EXPECTED_BYTES (2U + 2U + 1U) // PHR + FCF + DSN

    uint8_t                  receivedPsdu[IEEE802154_MAX_LENGTH];
    uint8_t                  pktOffset = PHY_HEADER_SIZE;
    uint8_t                  initialPktReadBytes;
    sl_rail_rx_packet_info_t packetInfo;
    uint32_t                 rxCallbackTimestamp = otPlatAlarmMicroGetNow();

    // This callback occurs after the address fields of an incoming
    // ACK-requesting CMD or DATA frame have been received and we
    // can do a frame pending check.  We must also figure out what
    // kind of ACK is being requested -- Immediate or Enhanced.

#if (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)
    if (writeIeee802154EnhancedAck(aRailHandle, &packetInfo, rxCallbackTimestamp, &initialPktReadBytes, receivedPsdu))
    {
        // We also return true above if there were failures in
        // generating an enhanced ACK.
        return;
    }
#else
    OT_UNUSED_VARIABLE(rxCallbackTimestamp);
    initialPktReadBytes =
        readInitialPacketData(&packetInfo, MAX_EXPECTED_BYTES, pktOffset + 2, receivedPsdu, MAX_EXPECTED_BYTES);
#endif

    // Calculate frame pending for immediate-ACK
    // If not, RAIL will send an immediate ACK, but we need to do FP lookup.
    sl_rail_status_t status = SL_RAIL_STATUS_NO_ERROR;

    // Check if we read the FCF, if not, set macFcf to 0
    uint16_t macFcf = (initialPktReadBytes <= pktOffset) ? 0U : receivedPsdu[pktOffset];

    bool framePendingSet = false;

    if (sli_ot_radio_interface_is_src_match_enabled())
    {
        sl_rail_ieee802154_address_t sourceAddress;

        status = sl_rail_ieee802154_get_address(aRailHandle, &sourceAddress);
        otEXPECT(status == SL_RAIL_STATUS_NO_ERROR);

        otInstance *instance = sli_ot_radio_instance_from_filter_mask(packetInfo.filter_mask);
        if (instance != nullptr
            && ((sourceAddress.address_length == SL_RAIL_IEEE802154_LONG_ADDRESS
                 && utilsSoftSrcMatchExtFindEntry(instance, (otExtAddress *)sourceAddress.long_address) >= 0)
                || (sourceAddress.address_length == SL_RAIL_IEEE802154_SHORT_ADDRESS
                    && utilsSoftSrcMatchShortFindEntry(instance, sourceAddress.short_address) >= 0)))
        {
            status = sl_rail_ieee802154_toggle_frame_pending(aRailHandle);
            otEXPECT(status == SL_RAIL_STATUS_NO_ERROR);
            framePendingSet = true;
        }
    }
    else if ((macFcf & IEEE802154_FRAME_TYPE_MASK) != IEEE802154_FRAME_TYPE_DATA)
    {
        status = sl_rail_ieee802154_toggle_frame_pending(aRailHandle);
        otEXPECT(status == SL_RAIL_STATUS_NO_ERROR);
        framePendingSet = true;
    }

    if (framePendingSet)
    {
        // Store whether frame pending was set in the outgoing ACK in a reserved
        // bit of the MAC header (cleared later)

        otEXPECT((skipRxPacketLengthBytes(&packetInfo)) == OT_ERROR_NONE);
        uint8_t *macFcfPointer =
            ((packetInfo.first_portion_bytes == 0) ? packetInfo.p_last_portion_data : packetInfo.p_first_portion_data);
        *macFcfPointer |= IEEE802154_FRAME_PENDING_SET_IN_OUTGOING_ACK;
    }

exit:
    if (status == SL_RAIL_STATUS_INVALID_STATE)
    {
        otLogWarnPlat("Too late to modify outgoing FP");
    }
    else
    {
        OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
    }
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
void packetReceivedCallback(void)
{
    sl_rail_rx_packet_info_t    packetInfo;
    sl_rail_rx_packet_details_t packetDetails;
    uint16_t                    length            = 0;
    bool                        framePendingInAck = false;
    bool                        dropPacket        = false;
    otInstance                 *instance          = nullptr;
    sl_status_t                 status;
    bool                        isRxPacketQueued;
    rxBuffer                   *rxPacketBuf = nullptr;

    sl_rail_rx_packet_handle_t packetHandle =
        sli_ot_radio_interface_rail_get_rx_packet_info(SL_RAIL_RX_PACKET_HANDLE_NEWEST, &packetInfo);
    otEXPECT_ACTION((packetHandle != SL_RAIL_RX_PACKET_HANDLE_INVALID
                     && packetInfo.packet_status == SL_RAIL_RX_PACKET_READY_SUCCESS),
                    dropPacket = true);

    otEXPECT_ACTION(validatePacketDetails(packetHandle, &packetDetails, &packetInfo, &length), dropPacket = true);

    uint8_t macFcf;
    otEXPECT_ACTION((skipRxPacketLengthBytes(&packetInfo)) == OT_ERROR_NONE, dropPacket = true);

    macFcf = ((packetInfo.first_portion_bytes == 0) ? packetInfo.p_last_portion_data[0]
                                                    : packetInfo.p_first_portion_data[0]);

    instance = sli_ot_radio_instance_from_filter_mask(packetInfo.filter_mask);

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    // Drop packet if instance is nullptr and it's not a valid case where nullptr is expected
    // Valid nullptr cases: Case 1 (broadcast) or Case 3 (missing addressing info)
    otEXPECT_ACTION(!(instance == nullptr && !isFilterMaskBroadcast(packetInfo.filter_mask)
                      && !isFilterMaskMissingAddressing(packetInfo.filter_mask)),
                    dropPacket = true);
#endif

    if (packetDetails.is_ack)
    {
        otEXPECT_ACTION(
            (length >= IEEE802154_MIN_LENGTH && (macFcf & IEEE802154_FRAME_TYPE_MASK) == IEEE802154_FRAME_TYPE_ACK),
            dropPacket = true);

        // read packet
        sli_ot_radio_interface_rail_copy_rx_packet(sReceiveAck.frame.mPsdu, &packetInfo);

#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
        railDebugCounters.mRailEventAcksReceived++;
#endif
        sReceiveAck.frame.mLength = length;

        sli_ot_radio_events_handle_phy_stack_event(
            SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ENDED,
            static_cast<uint32_t>(sli_ot_radio_interface_rail_is_receiving_frame()));

        if (txWaitingForAck()
            && (sReceiveAck.frame.mPsdu[IEEE802154_DSN_OFFSET] == sCurrentTxPacket->frame.mPsdu[IEEE802154_DSN_OFFSET]))
        {
            otEXPECT_ACTION(validatePacketTimestamp(&packetDetails, length), dropPacket = true);

            sReceiveAck.frame.mInfo.mRxInfo.mRssi = packetDetails.rssi_dbm;
            sReceiveAck.frame.mInfo.mRxInfo.mLqi  = packetDetails.lqi;
            sReceiveAck.instance                  = instance;
            updateRxFrameTimestamp(true, packetDetails.time_received.packet_time);

            // Processing the ACK frame in ISR context avoids the Tx state to be messed up,
            // in case the Rx FIFO queue gets wiped out in a DMP situation.
            sli_ot_radio_state_set_tx_success(true);
            sli_ot_radio_state_clear_tx_data_and_wait_for_ack();

            framePendingInAck = ((macFcf & IEEE802154_FRAME_FLAG_FRAME_PENDING) != 0);
            sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ACK_RECEIVED,
                                                       static_cast<uint32_t>(framePendingInAck));

            if (txIsDataRequest() && framePendingInAck)
            {
                sli_ot_radio_state_set_em_pending_data(true);
            }
        }
        // Yield the radio upon receiving an ACK as long as it is not related to
        // a data request.
        if (!txIsDataRequest())
        {
            sli_ot_radio_interface_rail_yield_radio();
        }
    }
    else
    {
        otEXPECT_ACTION(sli_ot_radio_interface_get_promiscuous() || (length >= IEEE802154_MIN_DATA_LENGTH),
                        dropPacket = true);

        otEXPECT_ACTION(validatePacketTimestamp(&packetDetails, length), dropPacket = true);

        // Drop the packet if queue is full.
        otEXPECT_ACTION(!queueIsFull(&sRxPacketQueue), dropPacket = true);
        // Allocate a block from memory pool for the received packet.
        status = sl_memory_pool_alloc(&sRxPacketMemPoolHandle, (void **)&rxPacketBuf);
        // Drop the packet if no more memory block present in the pool to store it.
        otEXPECT_ACTION(status == SL_STATUS_OK && rxPacketBuf != nullptr, dropPacket = true);

        // read packet
        sli_ot_radio_interface_rail_copy_rx_packet(rxPacketBuf->psdu, &packetInfo);

        rxPacketBuf->packetInfo.length    = (uint8_t)length;
        rxPacketBuf->packetInfo.channel   = (uint8_t)packetDetails.channel;
        rxPacketBuf->packetInfo.rssi      = packetDetails.rssi_dbm;
        rxPacketBuf->packetInfo.lqi       = packetDetails.lqi;
        rxPacketBuf->packetInfo.timestamp = packetDetails.time_received.packet_time;
        // Set instance to nullptr for broadcast packets in multi-instance mode, specific instance otherwise
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
        rxPacketBuf->packetInfo.instance =
            sli_ot_radio_instance_is_filter_mask_broadcast_pan(packetInfo.filter_mask) ? nullptr : instance;
#else
        rxPacketBuf->packetInfo.instance = instance;
#endif

        // Queue the rx packet or drop it if queueing fails and free the memory block.
        isRxPacketQueued = queueAdd(&sRxPacketQueue, (void *)rxPacketBuf);
        otEXPECT_ACTION(isRxPacketQueued, dropPacket = true);

#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
        railDebugCounters.mRailPlatRadioReceiveProcessedCount++;
#endif

        if (macFcf & IEEE802154_FRAME_FLAG_ACK_REQUIRED)
        {
            sli_ot_radio_events_handle_phy_stack_event(
                (sli_ot_radio_interface_rail_is_rx_auto_ack_paused()
                     ? SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ACK_BLOCKED
                     : SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ACKING),
                static_cast<uint32_t>(sli_ot_radio_interface_rail_is_receiving_frame()));
            sli_ot_radio_state_set_tx_ack_ongoing(true);
        }
        else
        {
            sli_ot_radio_events_handle_phy_stack_event(
                SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ENDED,
                static_cast<uint32_t>(sli_ot_radio_interface_rail_is_receiving_frame()));
            // We received a frame that does not require an ACK as result of a data
            // poll: we yield the radio here.
            if (sli_ot_radio_state_get_em_pending_data())
            {
                sli_ot_radio_interface_rail_yield_radio();
                sli_ot_radio_state_set_em_pending_data(false);
            }
        }
    }
exit:
    if (dropPacket)
    {
        sli_ot_radio_events_handle_phy_stack_event(
            SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_CORRUPTED,
            static_cast<uint32_t>(sli_ot_radio_interface_rail_is_receiving_frame()));

        (void)sl_memory_pool_free(&sRxPacketMemPoolHandle, rxPacketBuf);
    }
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
void packetSentCallback(bool isAck)
{
    if (isAck)
    {
        // We successfully sent out an ACK.
        sli_ot_radio_state_set_tx_ack_ongoing(false);
        // We acked the packet we received after a poll: we can yield now.
        if (sli_ot_radio_state_get_em_pending_data())
        {
            sli_ot_radio_interface_rail_yield_radio();
            sli_ot_radio_state_set_em_pending_data(false);
        }
    }
    else if (sli_ot_radio_state_is_tx_data_ongoing())
    {
        sli_ot_radio_state_set_using_csma(false);

        if (txWaitingForAck())
        {
            sli_ot_radio_state_set_waiting_for_ack(true);
            sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ACK_WAITING, 0U);
        }
        else
        {
            sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ENDED, 0U);
            sli_ot_radio_interface_rail_yield_radio();
            sli_ot_radio_state_set_tx_success(true);
            // Broadcast packet clear the ONGOING flag here.
            sli_ot_radio_state_set_tx_data_ongoing(false);
            sli_ot_radio_state_set_scheduled_tx_started(false);
        }
#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
        railDebugCounters.mRailEventPacketSent++;
#endif
    }
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
void txFailedCallback(bool isAck, uint32_t status)
{
    if (isAck)
    {
        sli_ot_radio_state_set_tx_ack_ongoing(false);
    }
    else if (sli_ot_radio_state_is_tx_data_ongoing())
    {
        if (status == EVENT_TX_CCA_FAILED)
        {
            sli_ot_radio_state_set_tx_cca_failed(true);
            sli_ot_radio_state_set_using_csma(false);
#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
            railDebugCounters.mRailEventChannelBusy++;
#endif
        }
        else
        {
            sli_ot_radio_state_set_tx_failed(true);
#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
            railDebugCounters.mRailEventTxAbort++;
#endif
        }
        sli_ot_radio_state_clear_tx_data_and_wait_for_ack();
        sli_ot_radio_interface_rail_yield_radio();
    }
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
void ackTimeoutCallback(void)
{
    OT_ASSERT(txWaitingForAck());
    OT_ASSERT(sli_ot_radio_state_is_waiting_for_ack());

    sli_ot_radio_state_set_tx_no_ack(true);
    sli_ot_radio_state_set_tx_data_ongoing(false);
    sli_ot_radio_state_set_scheduled_tx_started(false);

#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
    railDebugCounters.mRailEventNoAck++;
#endif

#ifdef SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT
    // If antenna diversity is enabled toggle the selected antenna.
    sl_rail_util_ant_div_toggle_tx_antenna();
#endif // SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT
    // TO DO: Check if we have an OT function that
    // provides the number of mac retry attempts left
    sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ACK_TIMEDOUT, 0);

    sli_ot_radio_state_set_waiting_for_ack(false);
    sli_ot_radio_interface_rail_yield_radio();
    sli_ot_radio_state_set_em_pending_data(false);
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
void schedulerEventCallback(sl_rail_handle_t aRailHandle)
{
    sl_rail_scheduler_status_t scheduler_status;
    sl_rail_status_t           rail_status;
    sl_rail_get_scheduler_status(aRailHandle, &scheduler_status, &rail_status);
    if (scheduler_status != SL_RAIL_SCHEDULER_STATUS_NO_ERROR)
    {
        sli_ot_radio_state_clear_all_scheduled_events();
        if (sli_ot_radio_state_is_tx_ack_ongoing())
        {
            sli_ot_radio_events_handle_phy_stack_event(
                SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ACK_ABORTED,
                static_cast<uint32_t>(sli_ot_radio_interface_rail_is_receiving_frame()));
            txFailedCallback(true, EVENT_TX_FAILED);
        }
        // We were in the process of TXing a data frame, treat it as a CCA_FAIL.
        if (sli_ot_radio_state_is_tx_data_ongoing())
        {
            sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_BLOCKED,
                                                       static_cast<uint32_t>(txWaitingForAck()));
            txFailedCallback(false, EVENT_TX_CCA_FAILED);
        }
        // We are waiting for an ACK: we will never get the ACK we were waiting for.
        // We want to call ackTimeoutCallback() only if the PACKET_SENT event
        // already fired (which would clear the FLAG_ONGOING_TX_DATA flag).
        if (sli_ot_radio_state_is_waiting_for_ack())
        {
            ackTimeoutCallback();
        }

#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
        railDebugCounters.mRailEventSchedulerStatusError++;
#endif
    }
}

//------------------------------------------------------------------------------
// Main thread packet handling

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
static bool validatePacketDetails(sl_rail_rx_packet_handle_t      packetHandle,
                                  sl_rail_rx_packet_details_t    *pPacketDetails,
                                  const sl_rail_rx_packet_info_t *pPacketInfo,
                                  uint16_t                       *packetLength)
{
    bool             pktValid = true;
    sl_rail_status_t rStatus;
#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
    rxDebugStep = 0;
#endif

    rStatus = sli_ot_radio_interface_rail_get_rx_packet_details(packetHandle, pPacketDetails);
    otEXPECT_ACTION(rStatus == SL_RAIL_STATUS_NO_ERROR, pktValid = false);
#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
    rxDebugStep++;
#endif

    otEXPECT_ACTION(isFilterMaskValid(pPacketInfo->filter_mask), pktValid = false);
#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
    rxDebugStep++;
#endif

    // RAIL's packetBytes includes the (1 or 2 byte) PHY header but not the 2-byte CRC.
    // We want *packetLength to match the PHY header length so we add 2 for CRC
    // and subtract the PHY header size.
    *packetLength = pPacketInfo->packet_bytes + 2U - PHY_HEADER_SIZE;

    if (PHY_HEADER_SIZE == 1)
    {
        otEXPECT_ACTION(*packetLength == pPacketInfo->p_first_portion_data[0], pktValid = false);
    }
    else
    {
        uint8_t lengthByte = ((pPacketInfo->first_portion_bytes > 1) ? pPacketInfo->p_first_portion_data[1]
                                                                     : pPacketInfo->p_last_portion_data[0]);
        otEXPECT_ACTION(*packetLength == (uint16_t)(__RBIT(lengthByte) >> 24), pktValid = false);
    }
#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
    rxDebugStep++;
#endif

    // check the length validity of recv packet; RAIL should take care of this.
    otEXPECT_ACTION((*packetLength >= IEEE802154_MIN_LENGTH && *packetLength <= IEEE802154_MAX_LENGTH),
                    pktValid = false);
#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
    rxDebugStep++;
#endif

exit:
#if (OPENTHREAD_CONFIG_LOG_LEVEL == OT_LOG_LEVEL_DEBG)
    if (!pktValid)
    {
        otLogDebgPlat("RX Pkt Invalid: rStatus=0x%lX, filterMask=0x%2X, pktLen=%i",
                      rStatus,
                      pPacketInfo->filter_mask,
                      *packetLength);
#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
        otLogDebgPlat("RX debug step=%i", rxDebugStep);
#endif
    }
#endif
    return pktValid;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
static bool validatePacketTimestamp(sl_rail_rx_packet_details_t *pPacketDetails, uint16_t packetLength)
{
    bool rxTimestampValid = true;

    // Get the timestamp when the SFD was received
    otEXPECT_ACTION(pPacketDetails->time_received.time_position != SL_RAIL_PACKET_TIME_INVALID,
                    rxTimestampValid = false);

    // + PHY HEADER SIZE for PHY header
    // We would not need this if PHR is not included and we want the MHR
    pPacketDetails->time_received.total_packet_bytes = packetLength + PHY_HEADER_SIZE;

    otEXPECT_ACTION((sli_ot_radio_interface_rail_get_rx_time_sync_word_end(pPacketDetails) == SL_RAIL_STATUS_NO_ERROR),
                    rxTimestampValid = false);
exit:
    return rxTimestampValid;
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
static void updateRxFrameTimestamp(bool aIsAckFrame, sl_rail_time_t aTimestamp)
{
    // Current time > sync-receive timestamp
    // Therefore lower 32 bits of current time should always be greater than lower 32 bits
    // of sync-rx timestamp unless there is a overflow.  In such cases, we do not want to
    // take overflow into consideration for sync-rx timestamp.
    uint64_t railUsTimeNow    = otPlatTimeGet();
    uint32_t railUsTimerWraps = railUsTimeNow >> 32;

    // Address multiple overflows, such as what would happen if the current time overflows
    // from 0x00000001FFFFFFFF to 0x0000000200000000 (leave the higher 32 bits as 0)
    if ((railUsTimeNow & 0xFFFFFFFF) <= aTimestamp)
    {
        railUsTimerWraps--;
    }

    if (aIsAckFrame)
    {
        sReceiveAck.frame.mInfo.mRxInfo.mTimestamp = aTimestamp + ((uint64_t)railUsTimerWraps << 32);
    }
    else
    {
        sReceive.frame.mInfo.mRxInfo.mTimestamp = aTimestamp + ((uint64_t)railUsTimerWraps << 32);
    }
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OT_PLATFORM_ABSTRACTION, SL_CODE_CLASS_TIME_CRITICAL)
static otError skipRxPacketLengthBytes(sl_rail_rx_packet_info_t *pPacketInfo)
{
    otError error = OT_ERROR_NONE;
    otEXPECT_ACTION(pPacketInfo->first_portion_bytes > 0, error = OT_ERROR_FAILED);

    pPacketInfo->p_first_portion_data += PHY_HEADER_SIZE;
    pPacketInfo->packet_bytes -= PHY_HEADER_SIZE;

    if (PHY_HEADER_SIZE == 1 || pPacketInfo->first_portion_bytes > 1)
    {
        pPacketInfo->first_portion_bytes -= PHY_HEADER_SIZE;
    }
    else
    {
        pPacketInfo->first_portion_bytes = 0U;
        // Increment lastPortionData to skip the second byte of the PHY header
        otEXPECT_ACTION(pPacketInfo->p_last_portion_data != nullptr, error = OT_ERROR_FAILED);
        pPacketInfo->p_last_portion_data++;
    }

exit:
    return error;
}

static rxBuffer *prepareNextRxPacketforCb(void)
{
    rxBuffer *rxPacketBuf = (rxBuffer *)queueRemove(&sRxPacketQueue);
    OT_ASSERT(rxPacketBuf != nullptr);
    uint8_t *psdu = rxPacketBuf->psdu;

    // Check reserved bits in MAC header for enhanced ACK security status
    sReceive.frame.mInfo.mRxInfo.mAckedWithSecEnhAck = ((*psdu & IEEE802154_SECURED_OUTGOING_ENHANCED_ACK) != 0);
    *psdu &= ~IEEE802154_SECURED_OUTGOING_ENHANCED_ACK;

    // Check reserved bits in MAC header for frame pending status
    sReceive.frame.mInfo.mRxInfo.mAckedWithFramePending = ((*psdu & IEEE802154_FRAME_PENDING_SET_IN_OUTGOING_ACK) != 0);
    *psdu &= ~IEEE802154_FRAME_PENDING_SET_IN_OUTGOING_ACK;

    sReceive.frame.mChannel = rxPacketBuf->packetInfo.channel;
    sReceive.frame.mLength  = rxPacketBuf->packetInfo.length;
    sReceive.frame.mPsdu    = rxPacketBuf->psdu;

    sReceive.frame.mInfo.mRxInfo.mRssi = rxPacketBuf->packetInfo.rssi;
    sLastRssi                          = rxPacketBuf->packetInfo.rssi;

    sReceive.frame.mInfo.mRxInfo.mLqi = rxPacketBuf->packetInfo.lqi;
    sLastLqi                          = rxPacketBuf->packetInfo.lqi;

    // Set instance (nullptr indicates broadcast packet)
    sReceive.instance = rxPacketBuf->packetInfo.instance;

    // Security key fields are populated per-instance in deliverRxPacketToInstance()

    updateRxFrameTimestamp(false, rxPacketBuf->packetInfo.timestamp);
    return rxPacketBuf;
}

static inline bool isRxPacketBroadcast(void)
{
    return (sReceive.instance == nullptr);
}

static void deliverRxPacketToInstance(otInstance *aInstance)
{
    OT_ASSERT(aInstance != nullptr);

#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
    sReceive.frame.mInfo.mRxInfo.mAckKeyId        = sli_ot_radio_security_get_ack_key_id(aInstance);
    sReceive.frame.mInfo.mRxInfo.mAckFrameCounter = sli_ot_radio_security_get_ack_frame_counter(aInstance);
#endif

#if OPENTHREAD_CONFIG_MULTIPAN_RCP_ENABLE
    sl_gp_intf_should_buffer_pkt(aInstance, &sReceive.frame, true);
#else
    bool isGpPacket = sl_gp_intf_is_gp_pkt(&sReceive.frame);
    otEXPECT(!isGpPacket);
#endif

    otPlatRadioReceiveDone(aInstance, &sReceive.frame, sReceiveError);

#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
    railDebugCounters.mRailPlatRadioReceiveDoneCbCount++;
#endif

#if !OPENTHREAD_CONFIG_MULTIPAN_RCP_ENABLE
exit:
    return;
#endif
}

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
static void processBroadcastRxPacket(void)
{
    for (uint8_t i = 0; i < RADIO_INTERFACE_COUNT; i++)
    {
        otInstance *instance = sli_ot_radio_instance_get(i);
        if (instance != nullptr && otInstanceIsInitialized(instance))
        {
            deliverRxPacketToInstance(instance);
        }
    }
}
#endif

static void processNextRxPacket(otInstance *aInstance)
{
    sReceiveError         = OT_ERROR_NONE;
    rxBuffer *rxPacketBuf = prepareNextRxPacketforCb();

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    OT_UNUSED_VARIABLE(aInstance);

    if (isRxPacketBroadcast())
    {
        processBroadcastRxPacket();
    }
    else
    {
        deliverRxPacketToInstance(sReceive.instance);
    }
#else
    deliverRxPacketToInstance(aInstance);
#endif

    (void)sl_memory_pool_free(&sRxPacketMemPoolHandle, rxPacketBuf);
    otSysEventSignalPending();
}

static void processRxPackets(otInstance *aInstance)
{
    while (!queueIsEmpty(&sRxPacketQueue))
    {
        processNextRxPacket(aInstance);
    }
}

static void processTxComplete(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    otError       txStatus;
    otRadioFrame *ackFrame = nullptr;

    if (sli_ot_radio_state_has_tx_events())
    {
        if (sli_ot_radio_state_has_tx_success())
        {
            txStatus = OT_ERROR_NONE;

            if (sCurrentTxPacket->frame.mPsdu[0] & IEEE802154_FRAME_FLAG_ACK_REQUIRED)
            {
                ackFrame = &sReceiveAck.frame;
            }

            sli_ot_radio_state_set_tx_success(false);
        }
        else if (sli_ot_radio_state_has_tx_cca_failed())
        {
            txStatus = OT_ERROR_CHANNEL_ACCESS_FAILURE;
            sli_ot_radio_state_set_tx_cca_failed(false);
        }
        else if (sli_ot_radio_state_has_tx_no_ack())
        {
            txStatus = OT_ERROR_NO_ACK;
            sli_ot_radio_state_set_tx_no_ack(false);
        }
        else
        {
            txStatus = OT_ERROR_ABORT;
            sli_ot_radio_state_set_tx_failed(false);
        }

        if (txStatus != OT_ERROR_NONE)
        {
            otLogDebgPlat("Transmit failed ErrorCode=%d", txStatus);
        }

        // Clear any internally-set txDelays so future transmits are not affected.
        sCurrentTxPacket->frame.mInfo.mTxInfo.mTxDelayBaseTime = 0;
        sCurrentTxPacket->frame.mInfo.mTxInfo.mTxDelay         = 0;

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
        CORE_DECLARE_IRQ_STATE;
        CORE_ENTER_ATOMIC();

        sli_ot_radio_instance_set_tx_busy(false);

        CORE_EXIT_ATOMIC();
#endif
        otPlatRadioTxDone(sCurrentTxPacket->instance, &sCurrentTxPacket->frame, ackFrame, txStatus);

#if RADIO_CONFIG_DEBUG_COUNTERS_SUPPORT
        railDebugCounters.mRailPlatRadioTxDoneCbCount++;
#endif
        otSysEventSignalPending();
    }
}

void efr32RadioProcess(otInstance *aInstance)
{
    sli_ot_radio_events_handle_phy_stack_event(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TICK, 0U);

    // We should process the received packet first. Adding it at the end of this function,
    // will delay the stack notification until the next call to efr32RadioProcess()
    processRxPackets(aInstance);
    processTxComplete(aInstance);
    sli_ot_energy_scan_process(aInstance);

#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    sli_ot_radio_instance_process_commands();
#endif // OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
}

//------------------------------------------------------------------------------
// Antenna Diversity, Wifi coexistence and Runtime PHY select support

#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_PHY_SELECT_PRESENT

otError setRadioState(otRadioState state)
{
    otError error = OT_ERROR_NONE;

    // Defer idling the radio if we have an ongoing TX task
    otEXPECT_ACTION(!sli_ot_radio_state_is_tx_data_ongoing(), error = OT_ERROR_FAILED);

    switch (state)
    {
    case OT_RADIO_STATE_RECEIVE:
        otEXPECT_ACTION(sli_ot_radio_interface_set_rx(sReceive.frame.mChannel) == OT_ERROR_NONE,
                        error = OT_ERROR_FAILED);
        break;
    case OT_RADIO_STATE_SLEEP:
        sli_ot_radio_state_set_idle();
        break;
    default:
        error = OT_ERROR_FAILED;
    }
exit:
    return error;
}

void sl_ot_update_active_radio_config(void)
{
    CORE_DECLARE_IRQ_STATE;
    otRadioState currentState;

    CORE_ENTER_ATOMIC();

    // Proceed with PHY selection only if 2.4 GHz band is used
    otEXPECT(sli_ot_radio_interface_get_band_config_ptr()->mChannelConfig == nullptr);

    currentState = otPlatRadioGetState(nullptr);
    otEXPECT(setRadioState(OT_RADIO_STATE_SLEEP) == OT_ERROR_NONE);
    sli_ot_radio_interface_rail_config_radio();
    otEXPECT(setRadioState(currentState) == OT_ERROR_NONE);

exit:
    CORE_EXIT_ATOMIC();
    return;
}
#endif // SL_CATALOG_RAIL_UTIL_IEEE802154_PHY_SELECT_PRESENT

#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT

static void changeDynamicEvents(void)
{
    /* clang-format off */
  const sl_rail_events_t eventMask = SL_RAIL_EVENTS_NONE
                                     | SL_RAIL_EVENT_RX_SYNC_0_DETECT
                                     | SL_RAIL_EVENT_RX_SYNC_1_DETECT
                                     | SL_RAIL_EVENT_RX_FRAME_ERROR
                                     | SL_RAIL_EVENT_RX_FIFO_OVERFLOW
                                     | SL_RAIL_EVENT_RX_ADDRESS_FILTERED
                                     | SL_RAIL_EVENT_RX_PACKET_ABORTED
                                     | SL_RAIL_EVENT_RX_FILTER_PASSED
                                     | SL_RAIL_EVENT_TX_CHANNEL_CLEAR
                                     | SL_RAIL_EVENT_TX_CCA_RETRY
                                     | SL_RAIL_EVENT_TX_START_CCA
                                     | SL_RAIL_EVENT_SIGNAL_DETECTED;
    /* clang-format on */
    sl_rail_events_t eventValues = SL_RAIL_EVENTS_NONE;

    if (phyStackEventIsEnabled())
    {
        eventValues |= eventMask;
    }
    sli_ot_radio_events_update_config(eventMask, eventValues);
}
#endif // SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT

static void efr32PhyStackInit(void)
{
#ifdef SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT
    sli_ot_radio_interface_init_antenna_config();
#endif // SL_CATALOG_RAIL_UTIL_ANT_DIV_PRESENT

#ifdef SL_CATALOG_RAIL_UTIL_COEX_PRESENT
    efr32CoexInit();
#endif // SL_CATALOG_RAIL_UTIL_COEX_PRESENT

#ifdef SL_CATALOG_RAIL_UTIL_IEEE802154_STACK_EVENT_PRESENT
    changeDynamicEvents();
#endif
}

#ifdef SL_CATALOG_RAIL_UTIL_COEX_PRESENT

static void emRadioEnableAutoAck(void)
{
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_ATOMIC();

    if (sli_ot_radio_state_is_initialized())
    {
        if ((sRhoActive >= RHO_INT_ACTIVE) // Internal always holds ACKs
            || ((sRhoActive > RHO_INACTIVE)
                && ((sl_rail_util_coex_get_options() & SL_RAIL_UTIL_COEX_OPT_ACK_HOLDOFF)
                    != SL_RAIL_UTIL_COEX_OPT_DISABLED)))
        {
            sli_ot_radio_interface_rail_pause_rx_auto_ack(true);
        }
        else
        {
            sli_ot_radio_interface_rail_pause_rx_auto_ack(false);
        }
    }
    CORE_EXIT_ATOMIC();
}

static void emRadioEnablePta(bool enable)
{
    halInternalInitPta();

    // When PTA is enabled, we want to negate PTA_REQ as soon as an incoming
    // frame is aborted, e.g. due to filtering.  To do that we must turn off
    // the TRACKABFRAME feature that's normally on to benefit sniffing on PTI.
    sl_rail_status_t status = sli_ot_radio_interface_rail_config_rx_options(
        SL_RAIL_RX_OPTION_TRACK_ABORTED_FRAMES,
        (enable ? SL_RAIL_RX_OPTIONS_NONE : SL_RAIL_RX_OPTION_TRACK_ABORTED_FRAMES));
    OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
}

static void efr32CoexInit(void)
{
#if SL_OPENTHREAD_COEX_COUNTER_ENABLE && defined(SL_CATALOG_RAIL_MULTIPLEXER_PRESENT)
    sli_ot_radio_interface_rail_set_coex_counter_handler((void *)(&sli_ot_radio_coex_counter_on_event));
#else
    sli_radio_coex_reset();
#endif // SL_OPENTHREAD_COEX_COUNTER_ENABLE && defined(SL_CATALOG_RAIL_MULTIPLEXER_PRESENT)

#if OPENTHREAD_CONFIG_PLATFORM_RADIO_COEX_ENABLE
#if defined(SL_RAIL_UTIL_COEX_REQ_GPIO) || defined(SL_RAIL_UTIL_COEX_REQ_PORT) || defined(SL_RAIL_UTIL_COEX_GNT_GPIO) \
    || defined(SL_RAIL_UTIL_COEX_GNT_PORT) || SL_RAIL_UTIL_COEX_RUNTIME_PHY_SELECT
    sl_rail_util_ot_enable_coex_state_event_filter();
#endif
#endif // OPENTHREAD_CONFIG_PLATFORM_RADIO_COEX_ENABLE

    sl_rail_util_coex_options_t coexOptions = sl_rail_util_coex_get_options();

#if SL_OPENTHREAD_COEX_MAC_HOLDOFF_ENABLE
    coexOptions |= SL_RAIL_UTIL_COEX_OPT_MAC_HOLDOFF;
#else
    if (sl_rail_util_coex_get_radio_holdoff())
    {
        coexOptions |= SL_RAIL_UTIL_COEX_OPT_MAC_HOLDOFF;
    }
#endif // SL_OPENTHREAD_COEX_MAC_HOLDOFF_ENABLE

    sl_rail_util_coex_set_options(coexOptions);

    emRadioEnableAutoAck(); // Might suspend AutoACK if RHO already in effect
    emRadioEnablePta(sl_rail_util_coex_is_enabled());
}

// Managing radio transmission
static void onPtaGrantTx(sl_rail_util_coex_req_t ptaStatus)
{
    // Only pay attention to first PTA Grant callback, ignore any further ones
    if (sPtaGntEventReported)
    {
        return;
    }
    sPtaGntEventReported = true;

    OT_ASSERT(ptaStatus == SL_RAIL_UTIL_COEX_REQCB_GRANTED);
    // PTA is telling us we've gotten GRANT and should send ASAP *without* CSMA
    sli_ot_radio_state_set_using_csma(false);
    txCurrentPacket();
}

static void tryTxCurrentPacket(void)
{
    OT_ASSERT(sli_ot_radio_state_is_tx_data_ongoing());

    sPtaGntEventReported = false;
    sl_rail_util_ieee802154_stack_event_t ptaStatus =
        sli_ot_radio_events_handle_phy_stack_event_with_status(SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_PENDED_MAC,
                                                               reinterpret_cast<uint32_t>(&onPtaGrantTx));
    if (ptaStatus == SL_RAIL_UTIL_IEEE802154_STACK_STATUS_SUCCESS)
    {
        // Normal case where PTA allows us to start the (CSMA) transmit below
        txCurrentPacket();
    }
    else if (ptaStatus == SL_RAIL_UTIL_IEEE802154_STACK_STATUS_CB_PENDING)
    {
        // onPtaGrantTx() callback will take over (and might already have)
    }
    else if (ptaStatus == SL_RAIL_UTIL_IEEE802154_STACK_STATUS_HOLDOFF)
    {
        txFailedCallback(false, EVENT_TX_FAILED);
    }
}

// Managing CCA Threshold
static void setCcaThreshold(void)
{
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_ATOMIC();
    int8_t thresholddBm = sli_ot_radio_interface_get_cca_threshold();

    if (sli_ot_radio_state_is_initialized())
    {
        if (sRhoActive > RHO_INACTIVE)
        {
            thresholddBm = SL_RAIL_RSSI_INVALID_DBM;
        }
        sl_rail_status_t status = sli_ot_radio_interface_rail_set_cca_threshold(thresholddBm);
        OT_ASSERT(status == SL_RAIL_STATUS_NO_ERROR);
    }
    CORE_EXIT_ATOMIC();
}

static void emRadioHoldOffInternalIsr(uint8_t active)
{
    if (active != sRhoActive)
    {
        sRhoActive = active; // Update sRhoActive early
        if (sli_ot_radio_state_is_initialized())
        {
            setCcaThreshold();
            emRadioEnableAutoAck();
        }
    }
}

extern "C" {
// External API used by Coex Component
SL_WEAK void emRadioHoldOffIsr(bool active)
{
    emRadioHoldOffInternalIsr((uint8_t)active | (sRhoActive & ~RHO_EXT_ACTIVE));
}
} // extern "C"

#endif // SL_CATALOG_RAIL_UTIL_COEX_PRESENT
