/*******************************************************************************
 * @file
 * @brief Provides definitions for indirect code classification of functions
 * in mac.hpp.
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

#ifndef MAC_HPP_
#define MAC_HPP_

#include "openthread-core-config.h"

#include <openthread/platform/radio.h>
#include <openthread/platform/time.h>

#include "common/clearable.hpp"
#include "common/locator.hpp"
#include "common/log.hpp"
#include "common/non_copyable.hpp"
#include "common/tasklet.hpp"
#include "common/time.hpp"
#include "common/timer.hpp"
#include "mac/channel_mask.hpp"
#include "mac/mac_filter.hpp"
#include "mac/mac_frame.hpp"
#include "mac/mac_links.hpp"
#include "mac/mac_types.hpp"
#include "mac/sub_mac.hpp"
#include "radio/trel_link.hpp"
#include "thread/key_manager.hpp"
#include "thread/link_quality.hpp"

#include "sl_code_classification.h"

namespace ot {

class Neighbor;

/**
 * @addtogroup core-mac
 *
 * @brief
 *   This module includes definitions for the IEEE 802.15.4 MAC
 *
 * @{
 */

namespace Mac {

constexpr uint32_t kDataPollTimeout =
    OPENTHREAD_CONFIG_MAC_DATA_POLL_TIMEOUT; ///< Timeout for receiving Data Frame (in msec).
constexpr uint32_t kSleepDelay = 300;        ///< Max sleep delay when frame is pending (in msec).

constexpr uint16_t kScanDurationDefault = OPENTHREAD_CONFIG_MAC_SCAN_DURATION; ///< Duration per channel (in msec).

constexpr uint8_t kMaxCsmaBackoffsDirect   = OPENTHREAD_CONFIG_MAC_MAX_CSMA_BACKOFFS_DIRECT;
constexpr uint8_t kMaxCsmaBackoffsIndirect = OPENTHREAD_CONFIG_MAC_MAX_CSMA_BACKOFFS_INDIRECT;
constexpr uint8_t kMaxCsmaBackoffsCsl      = 0;

constexpr uint8_t kDefaultMaxFrameRetriesDirect   = OPENTHREAD_CONFIG_MAC_DEFAULT_MAX_FRAME_RETRIES_DIRECT;
constexpr uint8_t kDefaultMaxFrameRetriesIndirect = OPENTHREAD_CONFIG_MAC_DEFAULT_MAX_FRAME_RETRIES_INDIRECT;
constexpr uint8_t kMaxFrameRetriesCsl             = 0;

constexpr uint8_t kTxNumBcast = OPENTHREAD_CONFIG_MAC_TX_NUM_BCAST; ///< Num of times broadcast frame is tx.

/**
 * Specifies the number of microseconds ahead of time that the MAC layer should deliver a CSL frame to the sub-MAC
 * layer.
 */
constexpr uint16_t kCslRequestAhead = OPENTHREAD_CONFIG_MAC_CSL_REQUEST_AHEAD_US;

constexpr uint16_t kMinCslIePeriod = OPENTHREAD_CONFIG_MAC_CSL_MIN_PERIOD;

constexpr uint32_t kDefaultWedListenInterval = OPENTHREAD_CONFIG_WED_LISTEN_INTERVAL;
constexpr uint32_t kDefaultWedListenDuration = OPENTHREAD_CONFIG_WED_LISTEN_DURATION;

/**
 * Defines the function pointer called on receiving an IEEE 802.15.4 Beacon during an Active Scan.
 */
typedef otHandleActiveScanResult ActiveScanHandler;

/**
 * Defines an Active Scan result.
 */
typedef otActiveScanResult ActiveScanResult;

/**
 * Defines the function pointer which is called during an Energy Scan when the scan result for a channel is
 * ready or when the scan completes.
 */
typedef otHandleEnergyScanResult EnergyScanHandler;

/**
 * Defines an Energy Scan result.
 */
typedef otEnergyScanResult EnergyScanResult;

/**
 * Implements the IEEE 802.15.4 MAC.
 */
class Mac : public InstanceLocator, private NonCopyable
{
    friend class ot::Instance;

public:
    /**
     * Initializes the MAC object.
     *
     * @param[in]  aInstance  A reference to the OpenThread instance.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    explicit Mac(Instance &aInstance);

    /**
     * Starts an IEEE 802.15.4 Active Scan.
     *
     * @param[in]  aScanChannels  A bit vector indicating which channels to scan. Zero is mapped to all channels.
     * @param[in]  aScanDuration  The time in milliseconds to spend scanning each channel. Zero duration maps to
     *                            default value `kScanDurationDefault` = 300 ms.
     * @param[in]  aHandler       A pointer to a function that is called on receiving an IEEE 802.15.4 Beacon.
     * @param[in]  aContext       A pointer to an arbitrary context (used when invoking `aHandler` callback).
     *
     * @retval kErrorNone  Successfully scheduled the Active Scan request.
     * @retval kErrorBusy  Could not schedule the scan (a scan is ongoing or scheduled).
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ActiveScan(uint32_t aScanChannels, uint16_t aScanDuration, ActiveScanHandler aHandler, void *aContext);

    /**
     * Starts an IEEE 802.15.4 Energy Scan.
     *
     * @param[in]  aScanChannels     A bit vector indicating on which channels to scan. Zero is mapped to all channels.
     * @param[in]  aScanDuration     The time in milliseconds to spend scanning each channel. If the duration is set to
     *                               zero, a single RSSI sample will be taken per channel.
     * @param[in]  aHandler          A pointer to a function called to pass on scan result or indicate scan completion.
     * @param[in]  aContext          A pointer to an arbitrary context (used when invoking @p aHandler callback).
     *
     * @retval kErrorNone  Accepted the Energy Scan request.
     * @retval kErrorBusy  Could not start the energy scan.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error EnergyScan(uint32_t aScanChannels, uint16_t aScanDuration, EnergyScanHandler aHandler, void *aContext);

    /**
     * Indicates the energy scan for the current channel is complete.
     *
     * @param[in]  aEnergyScanMaxRssi  The maximum RSSI encountered on the scanned channel.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void EnergyScanDone(int8_t aEnergyScanMaxRssi);

    /**
     * Indicates whether or not IEEE 802.15.4 Beacon transmissions are enabled.
     *
     * @retval TRUE   If IEEE 802.15.4 Beacon transmissions are enabled.
     * @retval FALSE  If IEEE 802.15.4 Beacon transmissions are not enabled.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    bool IsBeaconEnabled(void) const { return mBeaconsEnabled; }

    /**
     * Enables/disables IEEE 802.15.4 Beacon transmissions.
     *
     * @param[in]  aEnabled  TRUE to enable IEEE 802.15.4 Beacon transmissions, FALSE otherwise.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetBeaconEnabled(bool aEnabled) { mBeaconsEnabled = aEnabled; }

    /**
     * Indicates whether or not rx-on-when-idle is enabled.
     *
     * @retval TRUE   If rx-on-when-idle is enabled.
     * @retval FALSE  If rx-on-when-idle is not enabled.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    bool GetRxOnWhenIdle(void) const { return mRxOnWhenIdle; }

    /**
     * Sets the rx-on-when-idle mode.
     *
     * @param[in]  aRxOnWhenIdle  The rx-on-when-idle mode.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetRxOnWhenIdle(bool aRxOnWhenIdle);

    /**
     * Requests a direct data frame transmission.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void RequestDirectFrameTransmission(void);

#if OPENTHREAD_FTD
    /**
     * Requests an indirect data frame transmission.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void RequestIndirectFrameTransmission(void);
#endif

#if OPENTHREAD_CONFIG_MAC_CSL_TRANSMITTER_ENABLE
    /**
     * Requests `Mac` to start a CSL tx operation after a delay of @p aDelay time.
     *
     * @param[in]  aDelay  Delay time for `Mac` to start a CSL tx, in units of milliseconds.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void RequestCslFrameTransmission(uint32_t aDelay);
#endif

#if OPENTHREAD_CONFIG_WAKEUP_COORDINATOR_ENABLE
    /**
     * Requests `Mac` to start a wake-up frame transmission.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void RequestWakeupFrameTransmission(void);
#endif

    /**
     * Requests transmission of a data poll (MAC Data Request) frame.
     *
     * @retval kErrorNone          Data poll transmission request is scheduled successfully.
     * @retval kErrorInvalidState  The MAC layer is not enabled.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error RequestDataPollTransmission(void);

    /**
     * Returns a reference to the IEEE 802.15.4 Extended Address.
     *
     * @returns A pointer to the IEEE 802.15.4 Extended Address.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    const ExtAddress &GetExtAddress(void) const { return mLinks.GetExtAddress(); }

    /**
     * Sets the IEEE 802.15.4 Extended Address.
     *
     * @param[in]  aExtAddress  A reference to the IEEE 802.15.4 Extended Address.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetExtAddress(const ExtAddress &aExtAddress) { mLinks.SetExtAddress(aExtAddress); }

    /**
     * Returns the IEEE 802.15.4 Short Address.
     *
     * @returns The IEEE 802.15.4 Short Address.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    ShortAddress GetShortAddress(void) const { return mLinks.GetShortAddress(); }

    /**
     * Sets the IEEE 802.15.4 Short Address.
     *
     * @param[in]  aShortAddress  The IEEE 802.15.4 Short Address.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetShortAddress(ShortAddress aShortAddress) { mLinks.SetShortAddress(aShortAddress); }

    /**
     * Gets the alternate short address.
     *
     * @returns The alternate short address, or `kShortAddrInvalid` if there is no alternate address.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    ShortAddress GetAlternateShortAddress(void) const { return mLinks.GetAlternateShortAddress(); }

    /**
     * Sets the alternate short address.
     *
     * @param[in] aShortAddress   The alternate short address. Use `kShortAddrInvalid` to clear the alternate address.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetAlternateShortAddress(ShortAddress aShortAddress) { mLinks.SetAlternateShortAddress(aShortAddress); }

    /**
     * Returns the IEEE 802.15.4 PAN Channel.
     *
     * @returns The IEEE 802.15.4 PAN Channel.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    uint8_t GetPanChannel(void) const { return mPanChannel; }

    /**
     * Sets the IEEE 802.15.4 PAN Channel.
     *
     * @param[in]  aChannel  The IEEE 802.15.4 PAN Channel.
     *
     * @retval kErrorNone          Successfully set the IEEE 802.15.4 PAN Channel.
     * @retval kErrorInvalidArgs   The @p aChannel is not in the supported channel mask.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error SetPanChannel(uint8_t aChannel);

    /**
     * Sets the temporary IEEE 802.15.4 radio channel.
     *
     * Allows user to temporarily change the radio channel and use a different channel (during receive)
     * instead of the PAN channel (from `SetPanChannel()`). A call to `ClearTemporaryChannel()` would clear the
     * temporary channel and adopt the PAN channel again. The `SetTemporaryChannel()` can be used multiple times in row
     * (before a call to `ClearTemporaryChannel()`) to change the temporary channel.
     *
     * @param[in]  aChannel            A IEEE 802.15.4 channel.
     *
     * @retval kErrorNone          Successfully set the temporary channel
     * @retval kErrorInvalidArgs   The @p aChannel is not in the supported channel mask.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error SetTemporaryChannel(uint8_t aChannel);

    /**
     * Clears the use of a previously set temporary channel and adopts the PAN channel.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void ClearTemporaryChannel(void);

    /**
     * Returns the supported channel mask.
     *
     * @returns The supported channel mask.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    const ChannelMask &GetSupportedChannelMask(void) const { return mSupportedChannelMask; }

    /**
     * Sets the supported channel mask
     *
     * @param[in] aMask   The supported channel mask.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetSupportedChannelMask(const ChannelMask &aMask);

    /**
     * Returns the IEEE 802.15.4 PAN ID.
     *
     * @returns The IEEE 802.15.4 PAN ID.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    PanId GetPanId(void) const { return mPanId; }

    /**
     * Sets the IEEE 802.15.4 PAN ID.
     *
     * @param[in]  aPanId  The IEEE 802.15.4 PAN ID.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetPanId(PanId aPanId);

    /**
     * Returns the maximum number of frame retries during direct transmission.
     *
     * @returns The maximum number of retries during direct transmission.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    uint8_t GetMaxFrameRetriesDirect(void) const { return mMaxFrameRetriesDirect; }

    /**
     * Sets the maximum number of frame retries during direct transmission.
     *
     * @param[in]  aMaxFrameRetriesDirect  The maximum number of retries during direct transmission.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetMaxFrameRetriesDirect(uint8_t aMaxFrameRetriesDirect) { mMaxFrameRetriesDirect = aMaxFrameRetriesDirect; }

#if OPENTHREAD_FTD
    /**
     * Returns the maximum number of frame retries during indirect transmission.
     *
     * @returns The maximum number of retries during indirect transmission.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    uint8_t GetMaxFrameRetriesIndirect(void) const { return mMaxFrameRetriesIndirect; }

    /**
     * Sets the maximum number of frame retries during indirect transmission.
     *
     * @param[in]  aMaxFrameRetriesIndirect  The maximum number of retries during indirect transmission.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetMaxFrameRetriesIndirect(uint8_t aMaxFrameRetriesIndirect)
    {
        mMaxFrameRetriesIndirect = aMaxFrameRetriesIndirect;
    }
#endif

    /**
     * Is called to handle a received frame.
     *
     * @param[in]  aFrame  A pointer to the received frame, or `nullptr` if the receive operation was aborted.
     * @param[in]  aError  kErrorNone when successfully received a frame,
     *                     kErrorAbort when reception was aborted and a frame was not received.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void HandleReceivedFrame(RxFrame *aFrame, Error aError);

    /**
     * Records CCA status (success/failure) for a frame transmission attempt.
     *
     * @param[in] aCcaSuccess   TRUE if the CCA succeeded, FALSE otherwise.
     * @param[in] aChannel      The channel on which CCA was performed.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void RecordCcaStatus(bool aCcaSuccess, uint8_t aChannel);

    /**
     * Records the status of a frame transmission attempt, updating MAC counters.
     *
     * Unlike `HandleTransmitDone` which is called after all transmission attempts of frame to indicate final status
     * of a frame transmission request, this method is invoked on all frame transmission attempts.
     *
     * @param[in] aFrame      The transmitted frame.
     * @param[in] aError      kErrorNone when the frame was transmitted successfully,
     *                        kErrorNoAck when the frame was transmitted but no ACK was received,
     *                        kErrorChannelAccessFailure tx failed due to activity on the channel,
     *                        kErrorAbort when transmission was aborted for other reasons.
     * @param[in] aRetryCount Indicates number of transmission retries for this frame.
     * @param[in] aWillRetx   Indicates whether frame will be retransmitted or not. This is applicable only
     *                        when there was an error in transmission (i.e., `aError` is not NONE).
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void RecordFrameTransmitStatus(const TxFrame &aFrame, Error aError, uint8_t aRetryCount, bool aWillRetx);

    /**
     * Is called to handle transmit events.
     *
     * @param[in]  aFrame      The frame that was transmitted.
     * @param[in]  aAckFrame   A pointer to the ACK frame, `nullptr` if no ACK was received.
     * @param[in]  aError      kErrorNone when the frame was transmitted successfully,
     *                         kErrorNoAck when the frame was transmitted but no ACK was received,
     *                         kErrorChannelAccessFailure when the tx failed due to activity on the channel,
     *                         kErrorAbort when transmission was aborted for other reasons.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void HandleTransmitDone(TxFrame &aFrame, RxFrame *aAckFrame, Error aError);

    /**
     * Returns if an active scan is in progress.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    bool IsActiveScanInProgress(void) const { return IsActiveOrPending(kOperationActiveScan); }

    /**
     * Returns if an energy scan is in progress.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    bool IsEnergyScanInProgress(void) const { return IsActiveOrPending(kOperationEnergyScan); }

#if OPENTHREAD_FTD
    /**
     * Indicates whether the MAC layer is performing an indirect transmission (in middle of a tx).
     *
     * @returns TRUE if in middle of an indirect transmission, FALSE otherwise.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    bool IsPerformingIndirectTransmit(void) const { return (mOperation == kOperationTransmitDataIndirect); }
#endif

    /**
     * Returns if the MAC layer is in transmit state.
     *
     * The MAC layer is in transmit state during CSMA/CA, CCA, transmission of Data, Beacon or Data Request frames and
     * receiving of ACK frames. The MAC layer is not in transmit state during transmission of ACK frames or Beacon
     * Requests.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    bool IsInTransmitState(void) const;

    /**
     * Registers a callback to provide received raw IEEE 802.15.4 frames.
     *
     * @param[in]  aPcapCallback     A pointer to a function that is called when receiving an IEEE 802.15.4 link frame
     *                               or `nullptr` to disable the callback.
     * @param[in]  aCallbackContext  A pointer to application-specific context.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetPcapCallback(otLinkPcapCallback aPcapCallback, void *aCallbackContext)
    {
        mLinks.SetPcapCallback(aPcapCallback, aCallbackContext);
    }

    /**
     * Indicates whether or not promiscuous mode is enabled at the link layer.
     *
     * @retval true   Promiscuous mode is enabled.
     * @retval false  Promiscuous mode is not enabled.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    bool IsPromiscuous(void) const { return mPromiscuous; }

    /**
     * Enables or disables the link layer promiscuous mode.
     *
     * Promiscuous mode keeps the receiver enabled, overriding the value of mRxOnWhenIdle.
     *
     * @param[in]  aPromiscuous  true to enable promiscuous mode, or false otherwise.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetPromiscuous(bool aPromiscuous);

    /**
     * Resets mac counters
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void ResetCounters(void) { ClearAllBytes(mCounters); }

    /**
     * Returns the MAC counter.
     *
     * @returns A reference to the MAC counter.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    otMacCounters &GetCounters(void) { return mCounters; }

#if OPENTHREAD_CONFIG_MAC_RETRY_SUCCESS_HISTOGRAM_ENABLE
    /**
     * Returns the MAC retry histogram for direct transmission.
     *
     * @param[out]  aNumberOfEntries    A reference to where the size of returned histogram array is placed.
     *
     * @returns     A pointer to the histogram of retries (in a form of an array).
     *              The n-th element indicates that the packet has been sent with n-th retry.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    const uint32_t *GetDirectRetrySuccessHistogram(uint8_t &aNumberOfEntries);

#if OPENTHREAD_FTD
    /**
     * Returns the MAC retry histogram for indirect transmission.
     *
     * @param[out]  aNumberOfEntries    A reference to where the size of returned histogram array is placed.
     *
     * @returns     A pointer to the histogram of retries (in a form of an array).
     *              The n-th element indicates that the packet has been sent with n-th retry.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    const uint32_t *GetIndirectRetrySuccessHistogram(uint8_t &aNumberOfEntries);
#endif

    /**
     * Resets MAC retry histogram.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void ResetRetrySuccessHistogram(void);
#endif // OPENTHREAD_CONFIG_MAC_RETRY_SUCCESS_HISTOGRAM_ENABLE

    /**
     * Returns the noise floor value (currently use the radio receive sensitivity value).
     *
     * @returns The noise floor value in dBm.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    int8_t GetNoiseFloor(void) const { return mLinks.GetNoiseFloor(); }

    /**
     * Computes the link margin for a given a received signal strength value using noise floor.
     *
     * @param[in] aRss The received signal strength in dBm.
     *
     * @returns The link margin for @p aRss in dB based on noise floor.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    uint8_t ComputeLinkMargin(int8_t aRss) const;

    /**
     * Returns the current CCA (Clear Channel Assessment) failure rate.
     *
     * The rate is maintained over a window of (roughly) last `OPENTHREAD_CONFIG_CCA_FAILURE_RATE_AVERAGING_WINDOW`
     * frame transmissions.
     *
     * @returns The CCA failure rate with maximum value `0xffff` corresponding to 100% failure rate.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    uint16_t GetCcaFailureRate(void) const { return mCcaSuccessRateTracker.GetFailureRate(); }

    /**
     * Starts/Stops the Link layer. It may only be used when the Netif Interface is down.
     *
     * @param[in]  aEnable The requested State for the MAC layer. true - Start, false - Stop.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetEnabled(bool aEnable);

    /**
     * Indicates whether or not the link layer is enabled.
     *
     * @retval true   Link layer is enabled.
     * @retval false  Link layer is not enabled.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    bool IsEnabled(void) const { return mEnabled; }

    /**
     * Clears the Mode2Key stored in PSA ITS.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void ClearMode2Key(void) { mMode2KeyMaterial.Clear(); }

#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
    /**
     * Gets the CSL channel.
     *
     * @returns CSL channel.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    uint8_t GetCslChannel(void) const { return mCslChannel; }

    /**
     * Sets the CSL channel.
     *
     * @param[in]  aChannel  The CSL channel.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetCslChannel(uint8_t aChannel);

    /**
     * Sets whether the MLE layer is capable of starting CSL.
     *
     * @retval TRUE   If MLE layer is capable of starting CSL.
     * @retval FALSE  If MLE layer is not capable of starting CSL.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetCslCapable(bool aIsCslCapable);

    /**
     * Gets the CSL period.
     *
     * @returns CSL period in units of 10 symbols.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    uint16_t GetCslPeriod(void) const { return mCslPeriod; }

    /**
     * Gets the CSL period in milliseconds.
     *
     * If the CSL period cannot be represented exactly in milliseconds, return the rounded value to the nearest
     * millisecond.
     *
     * @returns CSL period in milliseconds.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    uint32_t GetCslPeriodInMsec(void) const;

    /**
     * Sets the CSL period.
     *
     * @param[in]  aPeriod  The CSL period in 10 symbols.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetCslPeriod(uint16_t aPeriod);

    /**
     * This method converts a given CSL period in units of 10 symbols to microseconds.
     *
     * @param[in] aPeriodInTenSymbols   The CSL period in unit of 10 symbols.
     *
     * @returns The converted CSL period value in microseconds corresponding to @p aPeriodInTenSymbols.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    static uint32_t CslPeriodToUsec(uint16_t aPeriodInTenSymbols);

    /**
     * Indicates whether CSL is started at the moment.
     *
     * @retval TRUE   If CSL is enabled.
     * @retval FALSE  If CSL is not enabled.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    bool IsCslEnabled(void) const { return mIsCslEnabled; }

    /**
     * Returns parent CSL accuracy (clock accuracy and uncertainty).
     *
     * @returns The parent CSL accuracy.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    const CslAccuracy &GetCslParentAccuracy(void) const { return mLinks.GetSubMac().GetCslParentAccuracy(); }

    /**
     * Sets parent CSL accuracy.
     *
     * @param[in] aCslAccuracy  The parent CSL accuracy.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetCslParentAccuracy(const CslAccuracy &aCslAccuracy)
    {
        mLinks.GetSubMac().SetCslParentAccuracy(aCslAccuracy);
    }
#endif // OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE

#if OPENTHREAD_CONFIG_MAC_FILTER_ENABLE && OPENTHREAD_CONFIG_RADIO_LINK_IEEE_802_15_4_ENABLE
    /**
     * Enables/disables the 802.15.4 radio filter.
     *
     * When radio filter is enabled, radio is put to sleep instead of receive (to ensure device does not receive any
     * frame and/or potentially send ack). Also the frame transmission requests return immediately without sending the
     * frame over the air (return "no ack" error if ack is requested, otherwise return success).
     *
     * @param[in] aFilterEnabled    TRUE to enable radio filter, FALSE to disable.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetRadioFilterEnabled(bool aFilterEnabled);

    /**
     * Indicates whether the 802.15.4 radio filter is enabled or not.
     *
     * @retval TRUE   If the radio filter is enabled.
     * @retval FALSE  If the radio filter is disabled.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    bool IsRadioFilterEnabled(void) const { return mLinks.GetSubMac().IsRadioFilterEnabled(); }
#endif

    /**
     * Sets the region code.
     *
     * The radio region format is the 2-bytes ascii representation of the ISO 3166 alpha-2 code.
     *
     * @param[in]  aRegionCode  The radio region code. The `aRegionCode >> 8` is first ascii char
     *                          and the `aRegionCode & 0xff` is the second ascii char.
     *
     * @retval  kErrorFailed          Other platform specific errors.
     * @retval  kErrorNone            Successfully set region code.
     * @retval  kErrorNotImplemented  The feature is not implemented.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error SetRegion(uint16_t aRegionCode);

    /**
     * Get the region code.
     *
     * The radio region format is the 2-bytes ascii representation of the ISO 3166 alpha-2 code.
     *
     * @param[out] aRegionCode  The radio region code. The `aRegionCode >> 8` is first ascii char
     *                          and the `aRegionCode & 0xff` is the second ascii char.
     *
     * @retval  kErrorFailed          Other platform specific errors.
     * @retval  kErrorNone            Successfully set region code.
     * @retval  kErrorNotImplemented  The feature is not implemented.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error GetRegion(uint16_t &aRegionCode) const;

    /**
     * Gets the wake-up channel.
     *
     * @returns wake-up channel.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    uint8_t GetWakeupChannel(void) const { return mWakeupChannel; }

#if OPENTHREAD_CONFIG_WAKEUP_COORDINATOR_ENABLE || OPENTHREAD_CONFIG_WAKEUP_END_DEVICE_ENABLE
    /**
     * Sets the wake-up channel.
     *
     * @param[in]  aChannel  The wake-up channel.
     *
     * @retval kErrorNone          Successfully set the wake-up channel.
     * @retval kErrorInvalidArgs   The @p aChannel is not in the supported channel mask.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error SetWakeupChannel(uint8_t aChannel);
#endif

#if OPENTHREAD_CONFIG_WAKEUP_END_DEVICE_ENABLE
    /**
     * Gets the wake-up listen parameters.
     *
     * @param[out]  aInterval  A reference to return the wake-up listen interval in microseconds.
     * @param[out]  aDuration  A reference to return the wake-up listen duration in microseconds.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void GetWakeupListenParameters(uint32_t &aInterval, uint32_t &aDuration) const;

    /**
     * Sets the wake-up listen parameters.
     *
     * The listen interval must be greater than the listen duration.
     * The listen duration must be greater or equal than `kMinWakeupListenDuration`.
     *
     * @param[in]  aInterval  The wake-up listen interval in microseconds.
     * @param[in]  aDuration  The wake-up listen duration in microseconds.
     *
     * @retval kErrorNone          Successfully set the wake-up listen parameters.
     * @retval kErrorInvalidArgs   Configured listen interval is not greater than listen duration.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error SetWakeupListenParameters(uint32_t aInterval, uint32_t aDuration);

    /**
     * Enables/disables listening for wake-up frames.
     *
     * @param[in]  aEnable  TRUE to enable listening for wake-up frames, FALSE otherwise
     *
     * @retval kErrorNone          Successfully enabled/disabled listening for wake-up frames.
     * @retval kErrorInvalidArgs   Configured listen interval is not greater than listen duration.
     * @retval kErrorInvalidState  Could not enable/disable listening for wake-up frames.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error SetWakeupListenEnabled(bool aEnable);

    /**
     * Returns whether listening for wake-up frames is enabled.
     *
     * @retval TRUE   If listening for wake-up frames is enabled.
     * @retval FALSE  If listening for wake-up frames is not enabled.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    bool IsWakeupListenEnabled(void) const { return mWakeupListenEnabled; }
#endif // OPENTHREAD_CONFIG_WAKEUP_END_DEVICE_ENABLE

    /**
     * Calculates the radio bus transfer time (in microseconds) for a given frame size based on `Radio::GetBusSpeed()`
     * and `Radio::GetBusLatency()`.
     *
     * @param[in] aFrameSize   The frame size to calculate for, in bytes.
     *
     * @returns The calculated radio bus transfer time in microseconds.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    uint32_t CalculateRadioBusTransferTime(uint16_t aFrameSize) const;

private:
    static constexpr uint16_t kMaxCcaSampleCount = OPENTHREAD_CONFIG_CCA_FAILURE_RATE_AVERAGING_WINDOW;

    enum Operation : uint8_t
    {
        kOperationIdle = 0,
        kOperationActiveScan,
        kOperationEnergyScan,
        kOperationTransmitBeacon,
        kOperationTransmitDataDirect,
        kOperationTransmitPoll,
        kOperationWaitingForData,
#if OPENTHREAD_FTD
        kOperationTransmitDataIndirect,
#endif
#if OPENTHREAD_CONFIG_MAC_CSL_TRANSMITTER_ENABLE
        kOperationTransmitDataCsl,
#endif
#if OPENTHREAD_CONFIG_WAKEUP_COORDINATOR_ENABLE
        kOperationTransmitWakeup,
#endif
    };

#if OPENTHREAD_CONFIG_MAC_RETRY_SUCCESS_HISTOGRAM_ENABLE
    struct RetryHistogram
    {
        /**
         * Histogram of number of retries for a single direct packet until success
         * [0 retry: packet count, 1 retry: packet count, 2 retry : packet count ...
         *  until max retry limit: packet count]
         *
         *  The size of the array is OPENTHREAD_CONFIG_MAC_RETRY_SUCCESS_HISTOGRAM_MAX_SIZE_COUNT_DIRECT.
         */
        uint32_t mTxDirectRetrySuccess[OPENTHREAD_CONFIG_MAC_RETRY_SUCCESS_HISTOGRAM_MAX_SIZE_COUNT_DIRECT];

        /**
         * Histogram of number of retries for a single indirect packet until success
         * [0 retry: packet count, 1 retry: packet count, 2 retry : packet count ...
         *  until max retry limit: packet count]
         *
         *  The size of the array is OPENTHREAD_CONFIG_MAC_RETRY_SUCCESS_HISTOGRAM_MAX_SIZE_COUNT_INDIRECT.
         */
        uint32_t mTxIndirectRetrySuccess[OPENTHREAD_CONFIG_MAC_RETRY_SUCCESS_HISTOGRAM_MAX_SIZE_COUNT_INDIRECT];
    };
#endif // OPENTHREAD_CONFIG_MAC_RETRY_SUCCESS_HISTOGRAM_ENABLE
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessReceiveSecurity(RxFrame &aFrame, const Address &aSrcAddr, Neighbor *aNeighbor);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void  ProcessTransmitSecurity(TxFrame &aFrame);
#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessEnhAckSecurity(TxFrame &aTxFrame, RxFrame &aAckFrame);
#endif
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void     UpdateIdleMode(void);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    bool     IsPending(Operation aOperation) const { return mPendingOperations & (1U << aOperation); }
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    bool     IsActiveOrPending(Operation aOperation) const;
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void     SetPending(Operation aOperation) { mPendingOperations |= (1U << aOperation); }
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void     ClearPending(Operation aOperation) { mPendingOperations &= ~(1U << aOperation); }
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void     StartOperation(Operation aOperation);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void     FinishOperation(void);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void     PerformNextOperation(void);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    TxFrame *PrepareBeaconRequest(void);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    TxFrame *PrepareBeacon(void);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    bool     ShouldSendBeacon(void) const;
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    bool     IsJoinable(void) const;
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void     BeginTransmit(void);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error    FilterDestShortAddress(ShortAddress aDestAddress) const;
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void     UpdateNeighborLinkInfo(Neighbor &aNeighbor, const RxFrame &aRxFrame);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    bool     HandleMacCommand(RxFrame &aFrame);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void     HandleTimer(void);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void  Scan(Operation aScanOperation, uint32_t aScanChannels, uint16_t aScanDuration);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error UpdateScanChannel(void);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void  PerformActiveScan(void);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void  ReportActiveScanResult(const RxFrame *aBeaconFrame);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ConvertBeaconToActiveScanResult(const RxFrame *aBeaconFrame, ActiveScanResult &aResult);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void  PerformEnergyScan(void);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void  ReportEnergyScanResult(int8_t aRssi);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void LogFrameRxFailure(const RxFrame *aFrame, Error aError) const;
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void LogFrameTxFailure(const TxFrame &aFrame, Error aError, uint8_t aRetryCount, bool aWillRetx) const;
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void LogBeacon(const char *aActionText) const;

#if OPENTHREAD_CONFIG_TIME_SYNC_ENABLE
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    uint8_t GetTimeIeOffset(const Frame &aFrame);
#endif

#if OPENTHREAD_CONFIG_MAC_CSL_TRANSMITTER_ENABLE
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void ProcessCsl(const RxFrame &aFrame, const Address &aSrcAddr);
#endif
#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void UpdateCslParameters(void);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void UpdateCslState(void);
#endif
#if OPENTHREAD_CONFIG_MLE_LINK_METRICS_INITIATOR_ENABLE
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void ProcessEnhAckProbing(const RxFrame &aFrame, const Neighbor &aNeighbor);
#endif
#if OPENTHREAD_CONFIG_WAKEUP_END_DEVICE_ENABLE
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error HandleWakeupFrame(const RxFrame &aFrame);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void  UpdateWakeupListening(void);
#endif
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    static const char *OperationToString(Operation aOperation);

    using OperationTask = TaskletIn<Mac, &Mac::PerformNextOperation>;
    using MacTimer      = TimerMilliIn<Mac, &Mac::HandleTimer>;

    static const otExtAddress sMode2ExtAddress;

    bool mEnabled : 1;
    bool mShouldTxPollBeforeData : 1;
    bool mRxOnWhenIdle : 1;
    bool mPromiscuous : 1;
    bool mBeaconsEnabled : 1;
    bool mUsingTemporaryChannel : 1;
#if OPENTHREAD_CONFIG_MAC_STAY_AWAKE_BETWEEN_FRAGMENTS
    bool mShouldDelaySleep : 1;
    bool mDelayingSleep : 1;
#endif
#if OPENTHREAD_CONFIG_WAKEUP_END_DEVICE_ENABLE
    bool mWakeupListenEnabled : 1;
#endif
    Operation   mOperation;
    uint16_t    mPendingOperations;
    uint8_t     mBeaconSequence;
    uint8_t     mDataSequence;
    uint8_t     mBroadcastTransmitCount;
    PanId       mPanId;
    uint8_t     mPanChannel;
    uint8_t     mRadioChannel;
    ChannelMask mSupportedChannelMask;
    uint8_t     mScanChannel;
    uint16_t    mScanDuration;
    ChannelMask mScanChannelMask;
    uint8_t     mMaxFrameRetriesDirect;
#if OPENTHREAD_FTD
    uint8_t mMaxFrameRetriesIndirect;
#endif
#if OPENTHREAD_CONFIG_MAC_CSL_TRANSMITTER_ENABLE
    TimeMilli mCslTxFireTime;
#endif
#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
    bool mIsCslEnabled : 1;
    bool mIsCslCapable : 1;
    // When Mac::mCslChannel is 0, it indicates that CSL channel has not been specified by the upper layer.
    uint8_t  mCslChannel;
    uint16_t mCslPeriod;
#endif
    uint8_t mWakeupChannel;
#if OPENTHREAD_CONFIG_WAKEUP_END_DEVICE_ENABLE
    uint32_t mWakeupListenInterval;
    uint32_t mWakeupListenDuration;
#endif
    union
    {
        ActiveScanHandler mActiveScanHandler;
        EnergyScanHandler mEnergyScanHandler;
    };

    void *mScanHandlerContext;

    Links              mLinks;
    OperationTask      mOperationTask;
    MacTimer           mTimer;
    otMacCounters      mCounters;
    uint32_t           mKeyIdMode2FrameCounter;
    SuccessRateTracker mCcaSuccessRateTracker;
    uint16_t           mCcaSampleCount;
#if OPENTHREAD_CONFIG_MAC_RETRY_SUCCESS_HISTOGRAM_ENABLE
    RetryHistogram mRetryHistogram;
#endif

#if OPENTHREAD_CONFIG_MULTI_RADIO
    RadioTypes mTxPendingRadioLinks;
    RadioTypes mTxBeaconRadioLinks;
    Error      mTxError;
#endif

#if OPENTHREAD_CONFIG_MAC_FILTER_ENABLE
    Filter mFilter;
#endif

    KeyMaterial mMode2KeyMaterial;
};

/**
 * @}
 */

} // namespace Mac
} // namespace ot

#endif // MAC_HPP_
