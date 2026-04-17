/*******************************************************************************
 * @file
 * @brief Provides definitions for indirect code classification of functions
 * in link_metric.hpp
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

#ifndef LINK_METRICS_HPP_
#define LINK_METRICS_HPP_

#include "openthread-core-config.h"
#include "sl_code_classification.h"
#if OPENTHREAD_CONFIG_MLE_LINK_METRICS_INITIATOR_ENABLE || OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE

#if (OPENTHREAD_CONFIG_THREAD_VERSION < OT_THREAD_VERSION_1_2)
#error "Thread 1.2 or higher version is required for OPENTHREAD_CONFIG_MLE_LINK_METRICS_INITIATOR_ENABLE" \
       "and OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE."
#endif

#include <openthread/link.h>
#include <openthread/link_metrics.h>

#include "common/as_core_type.hpp"
#include "common/callback.hpp"
#include "common/clearable.hpp"
#include "common/locator.hpp"
#include "common/message.hpp"
#include "common/non_copyable.hpp"
#include "common/pool.hpp"
#include "net/ip6_address.hpp"
#include "thread/link_metrics_tlvs.hpp"
#include "thread/link_quality.hpp"

namespace ot {
class Neighbor;
class UnitTester;

namespace LinkMetrics {

/**
 * @addtogroup core-link-metrics
 *
 * @brief
 *   This module includes definitions for Thread Link Metrics query and management.
 *
 * @{
 */

#if OPENTHREAD_CONFIG_MLE_LINK_METRICS_INITIATOR_ENABLE

/**
 * Implements the Thread Link Metrics Initiator.
 *
 * The Initiator makes queries, configures Link Metrics probing at the Subject and generates reports of the results.
 */
class Initiator : public InstanceLocator, private NonCopyable
{
public:
    // Initiator callbacks
    typedef otLinkMetricsReportCallback                ReportCallback;
    typedef otLinkMetricsMgmtResponseCallback          MgmtResponseCallback;
    typedef otLinkMetricsEnhAckProbingIeReportCallback EnhAckProbingIeReportCallback;

    /**
     * Provides the info used for appending MLE Link Metric Query TLV.
     */
    struct QueryInfo : public Clearable<QueryInfo>
    {
        uint8_t mSeriesId;             ///< Series ID.
        uint8_t mTypeIds[kMaxTypeIds]; ///< Type IDs.
        uint8_t mTypeIdCount;          ///< Number of entries in `mTypeIds[]`.
    };

    /**
     * Initializes an instance of the Initiator class.
     *
     * @param[in]  aInstance  A reference to the OpenThread interface.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    explicit Initiator(Instance &aInstance);

    /**
     * Sends an MLE Data Request containing Link Metrics Query TLV to query Link Metrics data.
     *
     * It could be either a Single Probe or a Forward Tracking Series.
     *
     * @param[in]  aDestination       A reference to the IPv6 address of the destination.
     * @param[in]  aSeriesId          The Series ID to query, 0 for single probe.
     * @param[in]  aMetrics           A pointer to metrics to query.
     *
     * @retval kErrorNone             Successfully sent a Link Metrics query message.
     * @retval kErrorNoBufs           Insufficient buffers to generate the MLE Data Request message.
     * @retval kErrorInvalidArgs      Type IDs are not valid or exceed the count limit.
     * @retval kErrorUnknownNeighbor  @p aDestination is not link-local or the neighbor is not found.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error Query(const Ip6::Address &aDestination, uint8_t aSeriesId, const Metrics *aMetrics);

    /**
     * Appends MLE Link Metrics Query TLV to a given message.
     *
     * @param[in] aMessage     The message to append to.
     * @param[in] aInfo        The link metrics query info to use to prepare the message.
     *
     * @retval kErrorNone     Successfully appended the TLV to the message.
     * @retval kErrorNoBufs   Insufficient buffers available to append the TLV.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error AppendLinkMetricsQueryTlv(Message &aMessage, const QueryInfo &aInfo);

    /**
     * Registers a callback to handle Link Metrics report received.
     *
     * @param[in]  aCallback  A pointer to a function that is called when a Link Metrics report is received.
     * @param[in]  aContext   A pointer to application-specific context.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetReportCallback(ReportCallback aCallback, void *aContext) { mReportCallback.Set(aCallback, aContext); }

    /**
     * Handles the received Link Metrics report contained in @p aMessage.
     *
     * @param[in]  aMessage      A reference to the message.
     * @param[in]  aOffsetRange  The offset range in @p aMessage where the metrics report sub-TLVs are present.
     * @param[in]  aAddress      A reference to the source address of the message.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void HandleReport(const Message &aMessage, OffsetRange &aOffsetRange, const Ip6::Address &aAddress);

    /**
     * Sends an MLE Link Metrics Management Request to configure/clear a Forward Tracking Series.
     *
     * @param[in] aDestination       A reference to the IPv6 address of the destination.
     * @param[in] aSeriesId          The Series ID.
     * @param[in] aSeriesFlags       The Series Flags info which specify what types of frames are to be accounted.
     * @param[in] aMetrics           A pointer to flags specifying what metrics to query.
     *
     * @retval kErrorNone             Successfully sent a Link Metrics Management Request message.
     * @retval kErrorNoBufs           Insufficient buffers to generate the MLE Link Metrics Management Request message.
     * @retval kErrorInvalidArgs      @p aSeriesId is not within the valid range.
     * @retval kErrorUnknownNeighbor  @p aDestination is not link-local or the neighbor is not found.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error SendMgmtRequestForwardTrackingSeries(const Ip6::Address &aDestination,
                                               uint8_t             aSeriesId,
                                               const SeriesFlags  &aSeriesFlags,
                                               const Metrics      *aMetrics);

    /**
     * Registers a callback to handle Link Metrics Management Response received.
     *
     * @param[in]  aCallback A pointer to a function that is called when a Link Metrics Management Response is received.
     * @param[in]  aContext  A pointer to application-specific context.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetMgmtResponseCallback(MgmtResponseCallback aCallback, void *aContext)
    {
        mMgmtResponseCallback.Set(aCallback, aContext);
    }

    /**
     * Sends an MLE Link Metrics Management Request to configure/clear a Enhanced-ACK Based Probing.
     *
     * @param[in] aDestination       A reference to the IPv6 address of the destination.
     * @param[in] aEnhAckFlags       Enh-ACK Flags to indicate whether to register or clear the probing. `0` to clear
     *                               and `1` to register. Other values are reserved.
     * @param[in] aMetrics           A pointer to flags specifying what metrics to query. Should be `nullptr` when
     *                               `aEnhAckFlags` is `0`.
     *
     * @retval kErrorNone             Successfully sent a Link Metrics Management Request message.
     * @retval kErrorNoBufs           Insufficient buffers to generate the MLE Link Metrics Management Request message.
     * @retval kErrorInvalidArgs      @p aEnhAckFlags is not a valid value or @p aMetrics isn't correct.
     * @retval kErrorUnknownNeighbor  @p aDestination is not link-local or the neighbor is not found.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error SendMgmtRequestEnhAckProbing(const Ip6::Address &aDestination,
                                       EnhAckFlags         aEnhAckFlags,
                                       const Metrics      *aMetrics);

    /**
     * Registers a callback to handle Link Metrics when Enh-ACK Probing IE is received.
     *
     * @param[in]  aCallback A pointer to a function that is called when Enh-ACK Probing IE is received is received.
     * @param[in]  aContext  A pointer to application-specific context.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetEnhAckProbingCallback(EnhAckProbingIeReportCallback aCallback, void *aContext)
    {
        mEnhAckProbingIeReportCallback.Set(aCallback, aContext);
    }

    /**
     * Handles the received Link Metrics Management Response contained in @p aMessage.
     *
     * @param[in]  aMessage    A reference to the message that contains the Link Metrics Management Response.
     * @param[in]  aAddress    A reference to the source address of the message.
     *
     * @retval kErrorNone     Successfully handled the Link Metrics Management Response.
     * @retval kErrorParse    Cannot parse sub-TLVs from @p aMessage successfully.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error HandleManagementResponse(const Message &aMessage, const Ip6::Address &aAddress);

    /**
     * Sends an MLE Link Probe message.
     *
     * @param[in] aDestination    A reference to the IPv6 address of the destination.
     * @param[in] aSeriesId       The Series ID which the Probe message targets at.
     * @param[in] aLength         The length of the data payload in Link Probe TLV, [0, 64].
     *
     * @retval kErrorNone             Successfully sent a Link Probe message.
     * @retval kErrorNoBufs           Insufficient buffers to generate the MLE Link Probe message.
     * @retval kErrorInvalidArgs      @p aSeriesId or @p aLength is not within the valid range.
     * @retval kErrorUnknownNeighbor  @p aDestination is not link-local or the neighbor is not found.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error SendLinkProbe(const Ip6::Address &aDestination, uint8_t aSeriesId, uint8_t aLength);

    /**
     * Processes received Enh-ACK Probing IE data.
     *
     * @param[in] aData      A pointer to buffer containing the Enh-ACK Probing IE data.
     * @param[in] aLength    The length of @p aData.
     * @param[in] aNeighbor  The neighbor from which the Enh-ACK Probing IE was received.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void ProcessEnhAckIeData(const uint8_t *aData, uint8_t aLength, const Neighbor &aNeighbor);

private:
    static constexpr uint8_t kLinkProbeMaxLen = 64; // Max length of data payload in Link Probe TLV.
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error FindNeighbor(const Ip6::Address &aDestination, Neighbor *&aNeighbor);

    Callback<ReportCallback>                mReportCallback;
    Callback<MgmtResponseCallback>          mMgmtResponseCallback;
    Callback<EnhAckProbingIeReportCallback> mEnhAckProbingIeReportCallback;
};

#endif // OPENTHREAD_CONFIG_MLE_LINK_METRICS_INITIATOR_ENABLE

#if OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE

/**
 * Implements the Thread Link Metrics Subject.
 *
 * The Subject responds queries with reports, handles Link Metrics Management Requests and Link Probe Messages.
 */
class Subject : public InstanceLocator, private NonCopyable
{
public:
    typedef otLinkMetricsEnhAckProbingIeReportCallback EnhAckProbingIeReportCallback;

    /**
     * Initializes an instance of the Subject class.
     *
     * @param[in]  aInstance  A reference to the OpenThread interface.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    explicit Subject(Instance &aInstance);

    /**
     * Appends a Link Metrics Report to a message according to the Link Metrics query.
     *
     * @param[out]  aMessage           A reference to the message to append report.
     * @param[in]   aRequestMessage    A reference to the message of the Data Request.
     * @param[in]   aNeighbor          A reference to the neighbor who queries the report.
     *
     * @retval kErrorNone         Successfully appended the Thread Discovery TLV.
     * @retval kErrorParse        Cannot parse query sub TLV successfully.
     * @retval kErrorInvalidArgs  QueryId is invalid or any Type ID is invalid.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error AppendReport(Message &aMessage, const Message &aRequestMessage, Neighbor &aNeighbor);

    /**
     * Handles the received Link Metrics Management Request contained in @p aMessage and return a status.
     *
     * @param[in]   aMessage     A reference to the message that contains the Link Metrics Management Request.
     * @param[in]   aNeighbor    A reference to the neighbor who sends the request.
     * @param[out]  aStatus      A reference to the status which indicates the handling result.
     *
     * @retval kErrorNone     Successfully handled the Link Metrics Management Request.
     * @retval kErrorParse    Cannot parse sub-TLVs from @p aMessage successfully.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error HandleManagementRequest(const Message &aMessage, Neighbor &aNeighbor, Status &aStatus);

    /**
     * Handles the Link Probe contained in @p aMessage.
     *
     * @param[in]   aMessage     A reference to the message that contains the Link Probe Message.
     * @param[out]  aSeriesId    A reference to Series ID that parsed from the message.
     *
     * @retval kErrorNone     Successfully handled the Link Metrics Management Response.
     * @retval kErrorParse    Cannot parse sub-TLVs from @p aMessage successfully.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error HandleLinkProbe(const Message &aMessage, uint8_t &aSeriesId);

    /**
     * Frees a SeriesInfo entry that was allocated from the Subject object.
     *
     * @param[in]  aSeries    A reference to the SeriesInfo to free.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void Free(SeriesInfo &aSeriesInfo);

private:
    // Max number of SeriesInfo that could be allocated by the pool.
#if OPENTHREAD_FTD
    static constexpr uint16_t kMaxSeriesSupported = OPENTHREAD_CONFIG_MLE_LINK_METRICS_MAX_SERIES_SUPPORTED;
#elif OPENTHREAD_MTD
    static constexpr uint16_t kMaxSeriesSupported = OPENTHREAD_CONFIG_MLE_LINK_METRICS_SERIES_MTD;
#endif
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    static Error ReadTypeIdsFromMessage(const Message &aMessage, const OffsetRange &aOffsetRange, Metrics &aMetrics);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    static Error AppendReportSubTlvToMessage(Message &aMessage, const MetricsValues &aValues);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Status ConfigureForwardTrackingSeries(uint8_t        aSeriesId,
                                          uint8_t        aSeriesFlags,
                                          const Metrics &aMetrics,
                                          Neighbor      &aNeighbor);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Status ConfigureEnhAckProbing(uint8_t aEnhAckFlags, const Metrics &aMetrics, Neighbor &aNeighbor);

    Pool<SeriesInfo, kMaxSeriesSupported> mSeriesInfoPool;
};

#endif // OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
uint8_t ScaleLinkMarginToRawValue(uint8_t aLinkMargin);
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
uint8_t ScaleRawValueToLinkMargin(uint8_t aRawValue);
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
uint8_t ScaleRssiToRawValue(int8_t aRssi);
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
int8_t  ScaleRawValueToRssi(uint8_t aRawValue);

/**
 * @}
 */

} // namespace LinkMetrics
} // namespace ot

#endif // OPENTHREAD_CONFIG_MLE_LINK_METRICS_INITIATOR_ENABLE || OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE

#endif // LINK_METRICS_HPP
