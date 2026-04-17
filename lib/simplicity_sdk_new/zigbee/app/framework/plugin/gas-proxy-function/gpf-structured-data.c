/***************************************************************************//**
 * @file
 * @brief Routines and defines for the Gas Proxy Function plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/plugin/gbz-message-controller/gbz-message-controller.h"
#include "app/framework/plugin/gas-proxy-function/gas-proxy-function.h"
#include "app/framework/plugin/events-server/events-server.h"
#include "gpf-structured-data.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "gas-proxy-function-config.h"
// Needed because we reference SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS
#include "meter-mirror-config.h"
#include "zap-cluster-command-parser.h"

/**
 * This file adds support for the GPF Structured Data Items as defined in the
 * GBCS v0.8.1 section 10.4.
 *
 * Section 10.4.1: There are GPF requirements to store structured data items
 * which do not have a direct one to one mapping in ZSE, or the interpretation
 * may be uncertain.  These structured data items have to be constructed by
 * the GPF.
 */

#define GPF_INVALID_LOG_INDEX                       0xFF
#define GPF_UNUSED_ENDPOINT_ID                      0xFF
#define GPF_INVALID_ENTRY_INDEX                     0xFFFF

#define GPF_PROFILE_DATA_LOG_SAMPLE_ID              0x0000
#define GPF_DAILY_CONSUMPTION_LOG_SAMPLE_ID         0x0001

#define GPF_SNAPSHOT_CAUSE_GENERAL                  0x00000001
#define GPF_SNAPSHOT_CAUSE_END_OF_BILLING_PERIOD    0x00000002
#define GPF_SNAPSHOT_CAUSE_CHANGE_OF_TARIFF         0x00000008
#define GPF_SNAPSHOT_CAUSE_CHANGE_OF_SUPPLIER       0x00002000
#define GPF_SNAPSHOT_CAUSE_CHANGE_OF_PAYMENT_MODE   0x00004000

sl_zigbee_af_gpf_message_t * sl_zigbee_af_gas_proxy_function_get_current_message();

/*
 * The two types of snapshot payload's that we support as defined by GBCS are
 *
 * - SL_ZIGBEE_ZCL_SNAPSHOT_PAYLOAD_TYPE_TOU_INFORMATION_SET_DELIVERED_REGISTERS
 *
 *   The format of this payload consists of a set of fixed lengths fields
 *   totaling 24 bytes and the tier summation registers, each of which is 6
 *   bytes in length. The tier summation registers represent the "Tariff TOU
 *   Register Matrix" which is defined by SMETS as a 1 x 4 matrix. Therefore,
 *   the total expected length of snapshots with this payload type is
 *   24 + (4 * 6) or 48 bytes.
 *
 * - SL_ZIGBEE_ZCL_SNAPSHOT_PAYLOAD_TYPE_BLOCK_TIER_INFORMATION_SET_DELIVERED
 *
 *   The format of this payload consists of a set of fixed length fields
 *   totaling 25 bytes and two variable lengths fields that will include
 *   the tier summation registers and the tier block summation registers. The
 *   length of these registers are 6 bytes. There are 4 tier summation registers
 *   as defined above and there will also be 4 tier block summation registers.
 *   The tier block summation registers represent the "Tariff Block Counter Matrix"
 *   which is defined as a 4 x 1 matrix. Therefore, the total expected length
 *   of snapshots with this payload type is 25 + (4 * 6) + (4 * 6) or 73 bytes.
 *
 *  GBCS IRP328 wants to add support for two more snapshot's payload
 *
 * - SL_ZIGBEE_ZCL_SNAPSHOT_PAYLOAD_TYPE_TOU_INFORMATION_SET_DELIVERED_REGISTERS_NO_BILLING
 *
 *   The format of this payload consists of a set of fixed length fields
 *   totaling 7 byes and the tier summation registers, each of which is 6 bytes in length.
 *   The tier summation registers represent the "Tariff TOU Register Matrix" and there
 *   are 4 tier summation registers as defined above. Therefore, the total expected length
 *   of snapshots with this payload type is 7 + (4 * 6) = 31 bytes.
 *
 * - SL_ZIGBEE_ZCL_SNAPSHOT_PAYLOAD_TYPE_BLOCK_TIER_INFORMATION_SET_DELIVERED_NO_BILLING
 *
 *   The format of this payload consists of a set of fixed length fields totaling 8 byes
 *   and two variable lengths fields that will include the tier summation registers and
 *   the tier block summation registers. The length of each registers are 6 bytes.
 *   The tier block summation registers represent the "Tariff Block Counter Matrix"
 *   and there are 4 registers for both tier summation and block summation as defined above.
 *   Therefore, the total expected length of snapshots with this payload type is
 *   8 + (4 * 6) + (4 * 6) = 56 bytes.
 *
 * The types were obtained by looking at the response in the use case
 * description for GCS16a then also looking at the description of the billing
 * data log in GBCS v0.8.1 section 10.4.2.4 and comparing it to the description
 * of the daily read log in the SMETS v1.58 section 4.4.94.
 *
 * The following define is the max of the above payload lengths.
 */
#define GPF_MAX_SNAPSHOT_PAYLOAD_SIZE               73

// There is only one prepay snapshot payload type defined by the SE spec.
//
// - SL_ZIGBEE_ZCL_PREPAY_SNAPSHOT_PAYLOAD_TYPE_DEBT_CREDIT_STATUS
//
//   The format of this payload consists of a set of fixed lengths fields
//   totaling 24 bytes.
#define GPF_MAX_PREPAY_SNAPSHOT_PAYLOAD_SIZE        24

#define getFirstIndex(log) ((log->numberOfEntries < log->maxEntries) ? 0 : log->nextEntry)
#define getNextIndex(log, index) ((index == (log->maxEntries - 1)) ? 0 : (index + 1))
#define getPrevIndex(log, index) ((index == 0) ? (log->maxEntries - 1) : (index - 1))
#define getLastIndex(log) ((log->numberOfEntries == 0) ? 0 : getPrevIndex(log, log->nextEntry))
#define incNumberOfEntries(log) if (log->numberOfEntries < log->maxEntries) { log->numberOfEntries++; }

#define PREV_MIDNIGHT(utc) (((utc) / SECONDS_IN_DAY) * SECONDS_IN_DAY)
#define NEXT_MIDNIGHT(utc) PREV_MIDNIGHT(utc + SECONDS_IN_DAY)
#define SECONDS_IN_HALF_HOUR 1800
#define PREV_HALF_HOUR(utc) (((utc) / SECONDS_IN_HALF_HOUR) * SECONDS_IN_HALF_HOUR)
#define NEXT_HALF_HOUR(utc) PREV_HALF_HOUR(utc + SECONDS_IN_HALF_HOUR)

enum {
  GPF_FLAGS_SENT = 0x01,
};

typedef struct {
  uint32_t time;
  uint32_t sample;
} sli_zigbee_gpf_sample_entry_t;

typedef struct {
  uint32_t time;
  uint32_t snapshotId;
  uint32_t snapshotCause;
  uint8_t  snapshotPayloadType;
  uint16_t snapshotPayloadSize;
  uint8_t snapshotPayload[GPF_MAX_SNAPSHOT_PAYLOAD_SIZE];
} sli_zigbee_gpf_snapshot_entry_t;

typedef struct {
  uint32_t time;
  uint32_t snapshotId;
  uint32_t snapshotCause;
  uint8_t  snapshotPayloadType;
  uint16_t snapshotPayloadSize;
  uint8_t snapshotPayload[GPF_MAX_PREPAY_SNAPSHOT_PAYLOAD_SIZE];
} sli_zigbee_gpf_prepay_snapshot_entry_t;

typedef struct {
  uint8_t flags;
  uint8_t code[26];
  int32_t amount;
  uint32_t time;
} sli_zigbee_gpf_top_up_entry_t;

typedef struct {
  uint8_t flags;
  uint32_t collectionTime;
  uint32_t amountCollected;
  uint32_t outstandingDebt;
  uint8_t  debtType;
} sli_zigbee_gpf_debt_entry_t;

typedef struct {
  bool catchup;
  uint8_t catchupSequenceNumber;
  uint32_t catchupTime;
  uint16_t nextEntry;
  uint16_t numberOfEntries;
  uint16_t maxEntries;
  sli_zigbee_gpf_sample_entry_t *entries;
  uint16_t sampleId;
  uint16_t sampleInterval;
  uint32_t startTime;
  uint32_t prevSummation;
} sli_zigbee_gpf_sample_log_t;

typedef struct {
  bool catchup;
  uint8_t catchupSequenceNumber;
  uint8_t catchupSnapshotOffset;
  uint32_t catchupSnapshotCause;
  uint32_t catchupTime;
  uint16_t nextEntry;
  uint16_t numberOfEntries;
  uint16_t maxEntries;
  sli_zigbee_gpf_snapshot_entry_t *entries;
} sli_zigbee_gpf_snapshot_log_t;

typedef struct {
  bool catchup;
  uint8_t catchupSequenceNumber;
  uint8_t catchupSnapshotOffset;
  uint32_t catchupSnapshotCause;
  uint32_t catchupTime;
  uint16_t nextEntry;
  uint16_t numberOfEntries;
  uint16_t maxEntries;
  sli_zigbee_gpf_prepay_snapshot_entry_t *entries;
} sli_zigbee_gpf_prepay_snapshot_log_t;

typedef struct {
  bool catchup;
  uint8_t catchupSequenceNumber;
  uint16_t nextEntry;
  uint16_t numberOfEntries;
  uint16_t maxEntries;
  sli_zigbee_gpf_top_up_entry_t *entries;
} sli_zigbee_gpf_top_up_log_t;

typedef struct {
  bool catchup;
  uint8_t catchupSequenceNumber;
  uint16_t nextEntry;
  uint16_t numberOfEntries;
  uint16_t maxEntries;
  sli_zigbee_gpf_debt_entry_t *entries;
} sli_zigbee_gpf_debt_log_t;

typedef struct {
  sli_zigbee_gpf_snapshot_log_t snapshot;
  sli_zigbee_gpf_top_up_log_t topUp;
  sli_zigbee_gpf_debt_log_t debt;
  sli_zigbee_gpf_prepay_snapshot_log_t prepaySnapshot;
} sli_zigbee_gpf_billing_data_log_t;

typedef struct {
  bool catchup;
  uint32_t prevCurrentDayAlternativeConsumption;
  uint32_t prevCurrentDayAlternativeConsumptionTime;
} GpfAlternativeHistoricalConsumption;

typedef struct {
  bool catchup;
  uint32_t prevCurrentDayCostConsumption;
  uint32_t prevCurrentDayCostConsumptionTime;
} sli_zigbee_gpf_historical_cost_consumption_t;

typedef struct {
  sl_802154_long_addr_t deviceIeeeAddress;
  uint8_t endpoint;
  sli_zigbee_gpf_snapshot_log_t dailyReadLog;
  sli_zigbee_gpf_prepay_snapshot_log_t prepayDailyReadLog;
  sli_zigbee_gpf_billing_data_log_t billingDataLog;
  sli_zigbee_gpf_sample_log_t profileDataLog;
  sli_zigbee_gpf_sample_log_t dailyConsumptionLog;
  uint8_t remoteEndpoint;
  sl_802154_short_addr_t remoteNodeId;
  uint32_t functionalNotificationFlags;
  uint32_t notificationFlags4;
  uint32_t lastAttributeReportTime;
  GpfAlternativeHistoricalConsumption alternativeHistoricalConsumption;
  sli_zigbee_gpf_historical_cost_consumption_t historicalCostConsumption;
} sli_gpf_structured_data_t;

static sli_gpf_structured_data_t structuredData[SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS];

// Keep track of the logs currently in "catchup" mode.  This is only used for
// printing when catchup starts and when its ends.  Its really just a debugging
// aid and can be removed if desired.
static uint8_t logCatchupsInProgress = 0;

// Event used to handle work when GetNotifiedMessage is received in response
// to notification flags being set when catchup is started on any of the logs.
extern sl_zigbee_af_event_t sl_zigbee_af_gas_proxy_function_catchup_event;
#define gasProxyFunctionCatchupEventControl (&sl_zigbee_af_gas_proxy_function_catchup_event)
void sl_zigbee_af_gas_proxy_function_catchup_event_handler(sl_zigbee_af_event_t * event);
// The GPF uses this event to check on its logs that need to updated every
// so often. These logs are the Profile Data Log, the Daily Gas Consumption Log,
// the Daily Read Log, and the Prepayment Daily Read Log.
// See EMAPPFWKV2-1333 and section 10.4.2.8 in v0.8.1.
extern sl_zigbee_af_event_t sl_zigbee_af_gas_proxy_function_gsme_sync_endpoint_events[];
#define endpointEvent sl_zigbee_af_gas_proxy_function_gsme_sync_endpoint_events

//------------------------------------------------------------------------------
// Storage for the various logs making up the structured data.

/**
 * GBCS v0.8.1 section 10.4.2.1
 *
 * The GSME shall record the Daily Read Log data items at midnight UTC as
 * defined in SMETS.  In ZSE terms, the GSME shall take a snapshot of the
 * relevant items.  Note that the format and data of the snapshot taken is
 * dependent upon the operating tariff.  For example if the GSME tariff is
 * 'TOU only', the snapshot shall not capture the block values.
 *
 * The GSME shall use the snapshot cause 'General' (0x0001) for the snapshot
 * taken.
 *
 * The GSME shall push the snapshot to the GPF using the PublishSnapshot
 * command. It is not necessary for the GSME to report any attributes which
 * duplicate those contained in the snapshot.
 *
 * The GPF shall populate the relevant attributes upon receipt of the
 * PublishSnapshot command, providing the command is received between midnight
 * (UTC) and the next scheduled wake of the GSME.
 *
 * The GPF shall store the data contained in the PublishSnapshot command in
 * the GPF copy of the GSME Daily Read Log.
 *
 * In the event of a communications outage, the GPF shall retrieve missing
 * snapshots using the GetSnapshot command, with the UTC start time field
 * populated based on the last received snapshot timestamp, if one has been
 * received.
 */
static sli_zigbee_gpf_snapshot_entry_t dailyReadLogEntries[SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_DAILY_READ_LOG_ENTRIES];

/**
 * GBCS v0.8.1 section 10.4.2.2
 *
 * If the GSME is operating in prepayment mode it shall record the Prepayment
 * Daily Read Log data items at midnight UTC.  In ZSE terms, the GSME shall
 * take a prepayment snapshot of the relevant items.  The format and data of
 * the prepayment snapshot taken is defined in ZSE.
 *
 * The GSME shall use the snapshot cause 'General' (0x0001) for the prepayment
 * snapshot taken.
 *
 * The GSME shall push the prepayment snapshot to the GPF using the Publish
 * Prepay Snapshot command.
 *
 * The GPF shall populate the relevant attributes upon receipt of the Publish
 * Prepay Snapshot command, providing the command is received between midnight
 * (UTC) and the next scheduled wake of the GSME.
 *
 * The GPF shall store the data contained in the Publish Prepay Snapshot
 * command in the GPF copy of the GSME Prepayment Daily Read Log.
 *
 * In the event of a communications outage, the GPF shall retrieve missing
 * prepayment snapshots using the GetPrepaySnapshot command (and
 * GetPrepaySnapshot notification flag) with the UTC start time field
 * populated based on the last received prepayment snapshot timestamp, if one
 * has been received.
 */
static sli_zigbee_gpf_prepay_snapshot_entry_t prepayDailyReadLogEntries[SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_PREPAYMENT_DAILY_READ_LOG_ENTRIES];

/**
 * GBCS v0.8.1 section 10.4.2.3
 *
 * SMETS defines Billing Data Log as 'a log capable of storing the following UTC
 * date and time stamped entries:
 *   - twelve entries comprising Tariff TOU Register Matrix,
 *     the Consumption Register and Tariff Block Counter Matrix;
 *   - five entries comprising the value of prepayment credits;
 *   - ten entries comprising the value of payment-based debt payments; and
 *   - twelve entries comprising Meter Balance, Emergency Credit Balance,
 *     Accumulated Debt Register, Payment Debt Register and
 *     Time Debt Registers [1 ... 2].
 */
#define SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_BILLING_DATA_LOG_SNAPSHOT_ENTRIES 12
#define SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_BILLING_DATA_LOG_TOP_UP_ENTRIES 5
#define SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_BILLING_DATA_LOG_DEBT_ENTRIES 10
#define SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_BILLING_DATA_LOG_PREPAY_SNAPSHOT_ENTRIES 12

/**
 * GBCS v0.8.1 section 10.4.2.4
 *
 * The GSME shall capture this snapshot at the following trigger points:
 *   - End of Billing Cycle (snapshot cause "End of Billing Period" );
 *   - Change of Payment Mode (snapshot cause "Change of Meter Mode");
 *   - Change of Tariff (snapshot cause 'Change of Tariff Information'); and
 *   - as specified in Section 13.3.5.10 (snapshot cause 'Change of Supplier').
 *
 * When it next turns on its HAN Interface, the GSME shall push this snapshot
 * to the GPF using the PublishSnapshot Command.
 *
 * The GPF shall store the data contained in the PublishSnapshot command in
 * the GPF copy of the GSME Billing data Log.
 *
 * In the event of a communications outage, the GPF shall retrieve missing
 * snapshots using the GetSnapshot command (and the relevant notification
 * flag) with the UTC start time field populated based on the last received
 * snapshot timestamp, if one has been received, or 0x0000 otherwise.
 */
static sli_zigbee_gpf_snapshot_entry_t billingDataLogSnapshotEntries[SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_BILLING_DATA_LOG_SNAPSHOT_ENTRIES];

/**
 * GBCS v0.8.1 section 10.4.2.5
 *
 * Upon completion of processing of a valid prepayment top-up, the GSME shall
 * push the latest five prepayment top-ups to the GPF using the PublishTopUpLog
 * command.
 *
 * The GPF shall store the data contained in the PublishTopUpLog command in the
 * GPF copy of the GSME Billing data Log.
 *
 * If there has been a communications outage, the GPF shall use the GetTopUp
 * Log command to retrieve all prepayment top-ups that may have been processed
 * during the communications outage.  The GSME shall set the Date/Time field of
 * the GetTopUp Log command to the current UTC time.
 */
static sli_zigbee_gpf_top_up_entry_t billingDataLogTopUpEntries[SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_BILLING_DATA_LOG_TOP_UP_ENTRIES];

/**
 * GBCS v0.8.1 section 10.4.2.6
 *
 * Upon completion of processing of a valid prepayment top-up where the GSME has
 * made a debt payment using part of that top-up, the GSME shall push details of
 * that debt payment only to the GPF using the PublishDebtLog command.
 *
 * The GPF shall record the details provided in the GPF copy of the GSME Billing
 * Data Log.
 *
 * In cases of communications outages, the GPF shall request any outstanding
 * payment-based debt payments by use of the GetDebtRepaymentLog command (and
 * GetDebtRepaymentLog notification flag) with the Debt Type field set to
 * 0x02 (Debt 3).
 */
static sli_zigbee_gpf_debt_entry_t billingDataLogDebtEntries[SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_BILLING_DATA_LOG_DEBT_ENTRIES];

/**
 * GBCS v0.8.1 section 10.4.2.7
 *
 * The GSME shall capture this snapshot at the following trigger points:
 *   - End of Billing Cycle (snapshot cause bit 1 set: 'End of Billing Period',
 *     as per PublishSnapshot command);
 *   - Change of Payment Mode (snapshot cause bit 14 set: 'Change of Meter Mode');
 *   - Change of Tariff (snapshot cause at least one of the bits set: bit 3
 *     'Change of Tariff Information' and / or bit 4 'Change of Price Matrix'
 *     and / or bit 5 'Change of Block Thresholds'); and
 *   - as specified in Section 13.3.5.10 (snapshot cause 'Change of Supplier').
 *
 * When it next turns on its HAN Interface, the GSME shall push this snapshot
 * to the GPF using the Publish Prepay Snapshot command.
 *
 * The GPF shall store the data contained in the Publish Prepay Snapshot command
 * in the GPF copy of the GSME Billing Data Log.
 *
 * In the event of a communications outage, the GPF shall retrieve missing
 * snapshots using the GetPrepaySnapshot command (and GetPrepaySnapshot
 * notification flag) with the UTC start time field populated based on the last
 * received snapshot timestamp, if one has been received.
 */
static sli_zigbee_gpf_prepay_snapshot_entry_t billingDataLogPrepaySnapshotEntries[SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_BILLING_DATA_LOG_PREPAY_SNAPSHOT_ENTRIES];

/**
 * GBCS v0.8.1 section 10.4.2.8
 *
 * The GPF shall create the GPF Profile Data Log from the consumption
 * information pushed by the GSME each half hour.
 *
 * The GSME shall, on each half hour, record the following information and
 * push to the GPF:
 *
 *    - the CurrentSummationDelivered attribute containing total consumption
 *      value (with units of m3);
 *    - the CurrentDayAlternative ConsumptionDelivered attribute containing
 *      total consumption today (with units of kWh); and
 *    - the CurrentDayCostConsumptionDelivered attribute containing total cost
 *      of consumption today (with units of Currency Unit);
 *
 * Upon receipt of the pushed data, the GPF shall calculate the consumption
 * with units of m3 over the previous half hour by subtracting its previously
 * recorded total consumption value from the total consumption value now sent.
 *
 * The resulting value shall be stored in the GPF Profile Data Log.
 *
 * In the event that there are missing values in the GPF Profile Data Log,
 * the GPF shall interrogate the GSME Profile Data Log using the
 * GetSampledData (SampleID 0x0000) command and the GetSampledData
 * notification flag to retrieve missing values.
 */
static sli_zigbee_gpf_sample_entry_t profileDataLogEntries[SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_PROFILE_DATA_LOG_ENTRIES];

/**
 * GBCS v0.8.1 section 10.4.2.9
 *
 * The GPF shall create the GPF Daily Gas Consumption Log based on the values
 * pushed from the GSME.  The difference between last total consumption value
 * pushed from the GSME each UTC day and the last value pushed in the prior
 * UTC day shall be time stamped and stored in the GPF Daily Gas Consumption
 * Log, so that the values in the log represent consumption in that UTC day.
 *
 * In the event of communications outages resulting in the final daily value
 * being missed, the GPF shall retrieve the values from the GSME Profile Data
 * Log using the GetSampledData (SampleID 0x0000) command and GetSampledData
 * notification flag.
 */
static sli_zigbee_gpf_sample_entry_t dailyConsumptionLogEntries[SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_DAILY_CONSUMPTION_LOG_ENTRIES];

/*
 * GBCS v0.8.1 section 10.4.2.10
 *
 * As per Section 10.4.2.8, the GSME shall, on each half hour, record the
 * following information and push to the GPF:
 *   - total consumption value (with units of m3);
 *   - total consumption today (with units of kWh); and
 *   - total cost of consumption today (with units of Currency Unit);
 * Using the "total consumption today" value, the GPF shall update the
 * attributes of the mirrored Alternative Historical Consumption attribute set.
 */
static uint16_t altConsumptionDayAttrIds[] = {
  ZCL_CURRENT_ALTERNATIVE_DAY_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY2_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY3_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY4_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY5_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY6_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY7_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY8_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
};
#define NUM_ALT_CONSUMPTION_DAY_ATTRS (sizeof(altConsumptionDayAttrIds) / sizeof(altConsumptionDayAttrIds[0]))

static uint16_t altConsumptionWeekAttrIds[] = {
  ZCL_CURRENT_WEEK_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK2_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK3_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK4_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK5_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
};
#define NUM_ALT_CONSUMPTION_WEEK_ATTRS (sizeof(altConsumptionWeekAttrIds) / sizeof(altConsumptionWeekAttrIds[0]))

static uint16_t altConsumptionMonthAttrIds[] = {
  ZCL_CURRENT_MONTH_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH2_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH3_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH4_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH5_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH6_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH7_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH8_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH9_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH10_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH11_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH12_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH13_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
};
#define NUM_ALT_CONSUMPTION_MONTH_ATTRS (sizeof(altConsumptionMonthAttrIds) / sizeof(altConsumptionMonthAttrIds[0]))

/*
 * GBCS v0.8.1 section 10.4.2.10
 *
 * As per Section 10.4.2.8, the GSME shall, on each half hour, record the
 * following information and push to the GPF:
 *   - total consumption value (with units of m3);
 *   - total consumption today (with units of kWh); and
 *   - total cost of consumption today (with units of Currency Unit);
 * Using the 'total cost of consumption today' value, the GPF shall update the
 * attributes of the mirrored Historical Cost Consumption Information
 * attribute set.
 */
static uint16_t costConsumptionDayAttrIds[] = {
  ZCL_CURRENT_DAY_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY_2_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY_3_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY_4_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY_5_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY_6_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY_7_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_DAY_8_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
};
#define NUM_COST_CONSUMPTION_DAY_ATTRS (sizeof(costConsumptionDayAttrIds) / sizeof(costConsumptionDayAttrIds[0]))

static uint16_t costConsumptionWeekAttrIds[] = {
  ZCL_CURRENT_WEEK_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK_2_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK_3_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK_4_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_WEEK_5_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
};
#define NUM_COST_CONSUMPTION_WEEK_ATTRS (sizeof(costConsumptionWeekAttrIds) / sizeof(costConsumptionWeekAttrIds[0]))

static uint16_t costConsumptionMonthAttrIds[] = {
  ZCL_CURRENT_MONTH_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_2_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_3_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_4_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_5_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_6_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_7_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_8_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_9_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_10_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_11_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_12_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
  ZCL_PREVIOUS_MONTH_13_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
};
#define NUM_COST_CONSUMPTION_MONTH_ATTRS (sizeof(costConsumptionMonthAttrIds) / sizeof(costConsumptionMonthAttrIds[0]))

//------------------------------------------------------------------------------
// Internal Functions
static uint8_t findStructuredDataByEUI64(const sl_802154_long_addr_t deviceIeeeAddress)
{
  uint8_t i;

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS; i++) {
    if (memcmp(deviceIeeeAddress, structuredData[i].deviceIeeeAddress, EUI64_SIZE) == 0) {
      return i;
    }
  }

  return GPF_INVALID_LOG_INDEX;
}

static uint8_t findStructuredData(uint8_t endpoint)
{
  uint8_t i;

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS; i++) {
    if (structuredData[i].endpoint == endpoint) {
      return i;
    }
  }

  return GPF_INVALID_LOG_INDEX;
}

static void setNotificationFlag(uint8_t endpoint, uint16_t attrId, uint32_t flag)
{
  sl_zigbee_af_status_t status;
  uint32_t notificationFlags;

  status = sl_zigbee_af_read_client_attribute(endpoint,
                                              ZCL_SIMPLE_METERING_CLUSTER_ID,
                                              attrId,
                                              (uint8_t *)&notificationFlags,
                                              4);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_gas_proxy_function_println("Unable to read notification flags attribute 0x%04X: status 0x%02X", attrId, status);
    return;
  }

  notificationFlags |= flag;
  status = sl_zigbee_af_write_client_attribute(endpoint,
                                               ZCL_SIMPLE_METERING_CLUSTER_ID,
                                               attrId,
                                               (uint8_t *)&notificationFlags,
                                               ZCL_BITMAP32_ATTRIBUTE_TYPE);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_gas_proxy_function_println("Unable to write notification flags attribute 0x%04X: status 0x%02X", attrId, status);
    return;
  }
}

static void clearNotificationFlag(uint8_t endpoint, uint16_t attrId, uint32_t flag)
{
  sl_zigbee_af_status_t status;
  uint32_t notificationFlags;

  status = sl_zigbee_af_read_client_attribute(endpoint,
                                              ZCL_SIMPLE_METERING_CLUSTER_ID,
                                              attrId,
                                              (uint8_t *)&notificationFlags,
                                              4);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_gas_proxy_function_println("Unable to read notification flags attribute 0x%04X: status 0x%02X", attrId, status);
    return;
  }

  notificationFlags &= ~flag;
  status = sl_zigbee_af_write_client_attribute(endpoint,
                                               ZCL_SIMPLE_METERING_CLUSTER_ID,
                                               attrId,
                                               (uint8_t *)&notificationFlags,
                                               ZCL_BITMAP32_ATTRIBUTE_TYPE);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_gas_proxy_function_println("Unable to write notification flags attribute 0x%04X: status 0x%02X", attrId, status);
    return;
  }
}

static void sendSampleData(uint32_t earliestSampleTime,
                           uint16_t numberOfSamples,
                           sli_zigbee_gpf_sample_log_t *sampleLog)
{
  uint16_t i;
  uint16_t entryIndex;
  uint16_t totalSamples = 0;
  uint16_t includedSamples = 0;

  // First run through the samples and calculate the total number of samples
  // matching the given criteria.
  for (i = 0, entryIndex = getFirstIndex(sampleLog);
       i < sampleLog->numberOfEntries && totalSamples < numberOfSamples;
       i++, entryIndex = getNextIndex(sampleLog, entryIndex)) {
    if (sampleLog->entries[entryIndex].time >= earliestSampleTime) {
      totalSamples++;
    }
  }

  // loop through all the entries looking for the entries that are greater than
  // or equal to the the given earliest start time.
  for (i = 0, entryIndex = getFirstIndex(sampleLog);
       i < sampleLog->numberOfEntries && includedSamples < totalSamples;
       i++, entryIndex = getNextIndex(sampleLog, entryIndex)) {
    if (sampleLog->entries[entryIndex].time < earliestSampleTime) {
      continue;
    }

    // For GBCS use cases, we should be setting the disable default response bit.
    if (includedSamples == 0) {
      (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                                | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
#ifdef SL_CATALOG_ZIGBEE_GBCS_COMPATIBILITY_PRESENT
                                                | ZCL_DISABLE_DEFAULT_RESPONSE_MASK
#endif
                                                ),
                                               ZCL_SIMPLE_METERING_CLUSTER_ID,
                                               ZCL_GET_SAMPLED_DATA_RESPONSE_COMMAND_ID,
                                               "vwuvv",
                                               sampleLog->sampleId,
                                               sampleLog->entries[entryIndex].time,
                                               SL_ZIGBEE_ZCL_SAMPLE_TYPE_CONSUMPTION_DELIVERED,
                                               sampleLog->sampleInterval,
                                               totalSamples);
    }

    (void) sl_zigbee_af_put_int24u_in_resp(sampleLog->entries[entryIndex].sample);
    includedSamples++;
  }

  if (includedSamples != 0) {
    sl_zigbee_af_send_response();
    return;
  }

  sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_NOT_FOUND);
}

static void sendSnapshot(uint32_t earliestTime,
                         uint32_t latestTime,
                         uint8_t snapshotOffset,
                         uint32_t snapshotCause,
                         sli_zigbee_gpf_snapshot_log_t *snapshotLog)
{
  uint16_t i;
  uint16_t entryIndex;
  uint8_t totalSnapshots = 0;
  uint16_t skippedSnapshots = 0;

  // First run the the snapshot and calculate the total number of snapshots
  // matching the given criteria.
  for (i = 0, entryIndex = getFirstIndex(snapshotLog);
       i < snapshotLog->numberOfEntries;
       i++, entryIndex = getNextIndex(snapshotLog, entryIndex)) {
    if (((snapshotLog->entries[entryIndex].snapshotCause & snapshotCause) != 0)
        && snapshotLog->entries[entryIndex].time >= earliestTime
        && snapshotLog->entries[entryIndex].time < latestTime) {
      totalSnapshots++;
    }
  }

  // loop through all the entries looking for the entries that are greater than
  // or equal to the the given earliest start time and less than the latest end time.
  for (i = 0, entryIndex = getFirstIndex(snapshotLog);
       i < snapshotLog->numberOfEntries;
       i++, entryIndex = getNextIndex(snapshotLog, entryIndex)) {
    if (((snapshotLog->entries[entryIndex].snapshotCause & snapshotCause) == 0)
        || snapshotLog->entries[entryIndex].time < earliestTime
        || snapshotLog->entries[entryIndex].time >= latestTime) {
      continue;
    }

    if (snapshotOffset != skippedSnapshots) {
      skippedSnapshots++;
      continue;
    }

#ifdef SL_CATALOG_ZIGBEE_GBCS_COMPATIBILITY_PRESENT
    // GBCS explicitly lists some commands that need to be sent with "disable
    // default response" flag set. This is one of them.
    // We make it conditional on GBCS so it does not affect standard SE apps.
    sl_zigbee_af_set_disable_default_response(SL_ZIGBEE_AF_DISABLE_DEFAULT_RESPONSE_ONE_SHOT);
#endif

    sl_zigbee_af_fill_command_simple_metering_cluster_publish_snapshot(snapshotLog->entries[entryIndex].snapshotId,
                                                                       snapshotLog->entries[entryIndex].time,
                                                                       totalSnapshots,
                                                                       0,
                                                                       1,
                                                                       snapshotLog->entries[entryIndex].snapshotCause,
                                                                       snapshotLog->entries[entryIndex].snapshotPayloadType,
                                                                       snapshotLog->entries[entryIndex].snapshotPayload,
                                                                       snapshotLog->entries[entryIndex].snapshotPayloadSize);

    sl_zigbee_af_send_response();
    return;
  }

  sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_NOT_FOUND);
}

static void sendPrepaySnapshot(uint32_t earliestTime,
                               uint32_t latestTime,
                               uint8_t snapshotOffset,
                               uint32_t snapshotCause,
                               sli_zigbee_gpf_prepay_snapshot_log_t *prepaySnapshotLog)
{
  uint16_t i;
  uint16_t entryIndex;
  uint8_t totalSnapshots = 0;
  uint16_t skippedSnapshots = 0;

  // First run the the snapshot and calculate the total number of snapshots
  // matching the given criteria.
  for (i = 0, entryIndex = getFirstIndex(prepaySnapshotLog);
       i < prepaySnapshotLog->numberOfEntries;
       i++, entryIndex = getNextIndex(prepaySnapshotLog, entryIndex)) {
    if (((prepaySnapshotLog->entries[entryIndex].snapshotCause & snapshotCause) != 0)
        && prepaySnapshotLog->entries[entryIndex].time >= earliestTime
        && prepaySnapshotLog->entries[entryIndex].time < latestTime) {
      totalSnapshots++;
    }
  }

  // loop through all the entries looking for the entries that are greater than
  // or equal to the the given earliest start time and less than the latest end time.
  for (i = 0, entryIndex = getFirstIndex(prepaySnapshotLog);
       i < prepaySnapshotLog->numberOfEntries;
       i++, entryIndex = getNextIndex(prepaySnapshotLog, entryIndex)) {
    if (((prepaySnapshotLog->entries[entryIndex].snapshotCause & snapshotCause) == 0)
        || prepaySnapshotLog->entries[entryIndex].time < earliestTime
        || prepaySnapshotLog->entries[entryIndex].time >= latestTime) {
      continue;
    }

    if (snapshotOffset != skippedSnapshots) {
      skippedSnapshots++;
      continue;
    }

#ifdef SL_CATALOG_ZIGBEE_GBCS_COMPATIBILITY_PRESENT
    // GBCS explicitly lists some commands that need to be sent with "disable
    // default response" flag set. This is one of them.
    // We make it conditional on GBCS so it does not affect standard SE apps.
    sl_zigbee_af_set_disable_default_response(SL_ZIGBEE_AF_DISABLE_DEFAULT_RESPONSE_ONE_SHOT);
#endif

    sl_zigbee_af_fill_command_prepayment_cluster_publish_prepay_snapshot(prepaySnapshotLog->entries[entryIndex].snapshotId,
                                                                         prepaySnapshotLog->entries[entryIndex].time,
                                                                         totalSnapshots,
                                                                         0,
                                                                         1,
                                                                         prepaySnapshotLog->entries[entryIndex].snapshotCause,
                                                                         prepaySnapshotLog->entries[entryIndex].snapshotPayloadType,
                                                                         prepaySnapshotLog->entries[entryIndex].snapshotPayload,
                                                                         prepaySnapshotLog->entries[entryIndex].snapshotPayloadSize);

    sl_zigbee_af_send_response();
    return;
  }

  sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_NOT_FOUND);
}

static void sendTopUp(uint32_t earliestStartTime,
                      uint32_t latestEndTime,
                      uint8_t numberOfRecords,
                      sli_zigbee_gpf_top_up_log_t *topUpLog)
{
  uint16_t i;
  uint16_t entryIndex;
  uint16_t includedRecords = 0;

  // loop through all the entries looking for the entries that are greater than
  // or equal to the the given earliest start time. The SE spec says, "The
  // first returned Top Up record shall be the most recent record with its TopUp
  // Time equal to or older than the Latest End Time provided." This means we
  // need to to keep iterating through the topUpLog looking for the correct
  // entry to send next.
  while (numberOfRecords == 0 || includedRecords < numberOfRecords) {
    uint32_t referenceUtc = 0;
    uint16_t indexToSend = 0xFF;

    for (i = 0, entryIndex = getFirstIndex(topUpLog);
         i < topUpLog->numberOfEntries;
         i++, entryIndex = getNextIndex(topUpLog, entryIndex)) {
      if (!READBITS(topUpLog->entries[entryIndex].flags, GPF_FLAGS_SENT)
          && topUpLog->entries[entryIndex].time > referenceUtc
          && topUpLog->entries[entryIndex].time >= earliestStartTime
          && topUpLog->entries[entryIndex].time <= latestEndTime) {
        referenceUtc = topUpLog->entries[entryIndex].time;
        indexToSend = entryIndex;
      }
    }

    // If no top up entries were found, it either means there are
    // no top up entries at the specified time or we've already
    // found all of them in previous iterations.
    if (indexToSend == 0xFF) {
      break;
    }

    // For GBCS use cases, we should be setting the disable default response bit.
    if (includedRecords == 0) {
      (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                                | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
#ifdef SL_CATALOG_ZIGBEE_GBCS_COMPATIBILITY_PRESENT
                                                | ZCL_DISABLE_DEFAULT_RESPONSE_MASK
#endif
                                                ),
                                               ZCL_PREPAYMENT_CLUSTER_ID,
                                               ZCL_PUBLISH_TOP_UP_LOG_COMMAND_ID,
                                               "uu",
                                               0,
                                               1);
    }

    (void) sl_zigbee_af_put_string_in_resp(topUpLog->entries[indexToSend].code);
    (void) sl_zigbee_af_put_int32u_in_resp(topUpLog->entries[indexToSend].amount);
    (void) sl_zigbee_af_put_int32u_in_resp(topUpLog->entries[indexToSend].time);
    SETBITS(topUpLog->entries[indexToSend].flags, GPF_FLAGS_SENT);
    includedRecords++;
  }

  if (includedRecords != 0) {
    for (i = 0, entryIndex = getFirstIndex(topUpLog);
         i < topUpLog->numberOfEntries;
         i++, entryIndex = getNextIndex(topUpLog, entryIndex)) {
      if (READBITS(topUpLog->entries[entryIndex].flags, GPF_FLAGS_SENT)) {
        CLEARBITS(topUpLog->entries[entryIndex].flags, GPF_FLAGS_SENT);
      }
    }
    sl_zigbee_af_send_response();
    return;
  }

  sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_NOT_FOUND);
}

static void sendDebt(uint32_t earliestStartTime,
                     uint32_t latestEndTime,
                     uint8_t numberOfDebts,
                     uint8_t debtType,
                     sli_zigbee_gpf_debt_log_t *debtLog)
{
  uint16_t i;
  uint16_t entryIndex;
  uint16_t includedDebts = 0;

  // loop through all the entries looking for the entries that are greater than
  // or equal to the the given earliest start time. The SE spec says, "The
  // first returned debt repayment record shall be the the most recent record
  // with its Collection Time equal to or older than the Latest End Time provided."
  // This means we need to to keep iterating through the debtLog looking for the
  // correct entry to send next.
  while (numberOfDebts == 0 || includedDebts < numberOfDebts) {
    uint32_t referenceUtc = 0;
    uint16_t indexToSend = 0xFF;

    for (i = 0, entryIndex = getFirstIndex(debtLog);
         i < debtLog->numberOfEntries;
         i++, entryIndex = getNextIndex(debtLog, entryIndex)) {
      if (!READBITS(debtLog->entries[entryIndex].flags, GPF_FLAGS_SENT)
          && debtLog->entries[entryIndex].collectionTime > referenceUtc
          && debtLog->entries[entryIndex].collectionTime >= earliestStartTime
          && debtLog->entries[entryIndex].collectionTime <= latestEndTime
          && (debtType == 0xFF || debtLog->entries[entryIndex].debtType == debtType)) {
        referenceUtc = debtLog->entries[entryIndex].collectionTime;
        indexToSend = entryIndex;
      }
    }

    // If no debt entries were found, it either means there are
    // no debt entries at the specified time or we've already
    // found all of them in previous iterations.
    if (indexToSend == 0xFF) {
      break;
    }

    // For GBCS use cases, we should be setting the disable default response bit.
    if (includedDebts == 0) {
      (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                                | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
#ifdef SL_CATALOG_ZIGBEE_GBCS_COMPATIBILITY_PRESENT
                                                | ZCL_DISABLE_DEFAULT_RESPONSE_MASK
#endif
                                                ),
                                               ZCL_PREPAYMENT_CLUSTER_ID,
                                               ZCL_PUBLISH_DEBT_LOG_COMMAND_ID,
                                               "uu",
                                               0,
                                               1);
    }

    (void) sl_zigbee_af_put_int32u_in_resp(debtLog->entries[indexToSend].collectionTime);
    (void) sl_zigbee_af_put_int32u_in_resp(debtLog->entries[indexToSend].amountCollected);
    (void) sl_zigbee_af_put_int8u_in_resp(debtLog->entries[indexToSend].debtType);
    (void) sl_zigbee_af_put_int32u_in_resp(debtLog->entries[indexToSend].outstandingDebt);
    SETBITS(debtLog->entries[indexToSend].flags, GPF_FLAGS_SENT);
    includedDebts++;
  }

  if (includedDebts != 0) {
    for (i = 0, entryIndex = getFirstIndex(debtLog);
         i < debtLog->numberOfEntries;
         i++, entryIndex = getNextIndex(debtLog, entryIndex)) {
      if (READBITS(debtLog->entries[entryIndex].flags, GPF_FLAGS_SENT)) {
        CLEARBITS(debtLog->entries[entryIndex].flags, GPF_FLAGS_SENT);
      }
    }
    sl_zigbee_af_send_response();
    return;
  }

  sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_NOT_FOUND);
}

static void receiveSampleData(uint32_t time,
                              uint32_t sample,
                              sli_zigbee_gpf_sample_log_t *sampleLog)
{
  sampleLog->entries[sampleLog->nextEntry].time = time;
  sampleLog->entries[sampleLog->nextEntry].sample = sample;
  incNumberOfEntries(sampleLog);
  sampleLog->nextEntry = getNextIndex(sampleLog, sampleLog->nextEntry);
}

static void receiveSnapshot(uint32_t snapshotId,
                            uint32_t snapshotTime,
                            uint32_t snapshotCause,
                            uint8_t snapshotPayloadType,
                            uint8_t *snapshotPayload,
                            sli_zigbee_gpf_snapshot_log_t *snapshotLog)
{
  uint16_t snapshotPayloadLength = fieldLength(snapshotPayload);

  if (snapshotPayloadLength > GPF_MAX_SNAPSHOT_PAYLOAD_SIZE) {
    sl_zigbee_af_gas_proxy_function_println("GPF: WARN: received a PublishSnapshot command that is longer than the max length expected, truncating entry stored in log: received len=0x%04X, max expected len=0x%04X",
                                            snapshotPayloadLength, GPF_MAX_SNAPSHOT_PAYLOAD_SIZE);
    snapshotPayloadLength = GPF_MAX_SNAPSHOT_PAYLOAD_SIZE;
  }

  snapshotLog->entries[snapshotLog->nextEntry].snapshotId = snapshotId;
  snapshotLog->entries[snapshotLog->nextEntry].time = snapshotTime;
  snapshotLog->entries[snapshotLog->nextEntry].snapshotCause = snapshotCause;
  snapshotLog->entries[snapshotLog->nextEntry].snapshotPayloadType = snapshotPayloadType;
  snapshotLog->entries[snapshotLog->nextEntry].snapshotPayloadSize = snapshotPayloadLength;
  memcpy(snapshotLog->entries[snapshotLog->nextEntry].snapshotPayload,
         snapshotPayload,
         snapshotPayloadLength);
  incNumberOfEntries(snapshotLog);
  snapshotLog->nextEntry = getNextIndex(snapshotLog, snapshotLog->nextEntry);
}

static void receivePrepaySnapshot(uint32_t snapshotId,
                                  uint32_t snapshotTime,
                                  uint32_t snapshotCause,
                                  uint8_t snapshotPayloadType,
                                  uint8_t *snapshotPayload,
                                  sli_zigbee_gpf_prepay_snapshot_log_t *prepaySnapshotLog)
{
  uint16_t snapshotPayloadLength = fieldLength(snapshotPayload);

  if (snapshotPayloadLength > GPF_MAX_PREPAY_SNAPSHOT_PAYLOAD_SIZE) {
    sl_zigbee_af_gas_proxy_function_println("GPF: WARN: received a PublishPrepaySnapshot command that is longer than the max length expected, truncating entry stored in log: received len=0x%04X, max expected len=0x%04X",
                                            snapshotPayloadLength, GPF_MAX_PREPAY_SNAPSHOT_PAYLOAD_SIZE);
    snapshotPayloadLength = GPF_MAX_PREPAY_SNAPSHOT_PAYLOAD_SIZE;
  }

  prepaySnapshotLog->entries[prepaySnapshotLog->nextEntry].snapshotId = snapshotId;
  prepaySnapshotLog->entries[prepaySnapshotLog->nextEntry].time = snapshotTime;
  prepaySnapshotLog->entries[prepaySnapshotLog->nextEntry].snapshotCause = snapshotCause;
  prepaySnapshotLog->entries[prepaySnapshotLog->nextEntry].snapshotPayloadType = snapshotPayloadType;
  prepaySnapshotLog->entries[prepaySnapshotLog->nextEntry].snapshotPayloadSize = snapshotPayloadLength;
  memcpy(prepaySnapshotLog->entries[prepaySnapshotLog->nextEntry].snapshotPayload,
         snapshotPayload,
         snapshotPayloadLength);
  incNumberOfEntries(prepaySnapshotLog);
  prepaySnapshotLog->nextEntry = getNextIndex(prepaySnapshotLog, prepaySnapshotLog->nextEntry);
}

static void receiveTopUp(uint8_t *topUpPayloadCode,
                         uint32_t topUpPayloadAmount,
                         uint32_t topUpPayloadTime,
                         sli_zigbee_gpf_top_up_log_t *topUpLog)
{
  topUpLog->entries[topUpLog->nextEntry].flags = 0;
  sl_zigbee_af_copy_string(topUpLog->entries[topUpLog->nextEntry].code, topUpPayloadCode, 25);
  topUpLog->entries[topUpLog->nextEntry].amount = topUpPayloadAmount;
  topUpLog->entries[topUpLog->nextEntry].time = topUpPayloadTime;
  incNumberOfEntries(topUpLog);
  topUpLog->nextEntry = getNextIndex(topUpLog, topUpLog->nextEntry);
}

static void receiveDebt(uint32_t collectionTime,
                        uint32_t amountCollected,
                        uint32_t outstandingDebt,
                        uint8_t  debtType,
                        sli_zigbee_gpf_debt_log_t *debtLog)
{
  debtLog->entries[debtLog->nextEntry].flags = 0;
  debtLog->entries[debtLog->nextEntry].collectionTime = collectionTime;
  debtLog->entries[debtLog->nextEntry].amountCollected = amountCollected;
  debtLog->entries[debtLog->nextEntry].outstandingDebt = outstandingDebt;
  debtLog->entries[debtLog->nextEntry].debtType = debtType;
  incNumberOfEntries(debtLog);
  debtLog->nextEntry = getNextIndex(debtLog, debtLog->nextEntry);
}

static void getSampleData(uint8_t srcEndpoint,
                          uint8_t dstEndpoint,
                          sl_802154_short_addr_t dstNodeId,
                          sli_zigbee_gpf_sample_log_t *sampleLog)
{
  /**
   * GBCS v0.8.1 Section 10.4.2.8
   *
   * In the event that there are missing values in the GPF Profile Data Log,
   * the GPF shall interrogate the GSME Profile Data Log using the
   * GetSampledData (SampleID 0x0000) command and the GetSampledData notification
   * flag to retrieve missing values.
   *
   * GBCS v0.8.1 Section 10.4.2.9
   *
   * In the event of communications outages resulting in the final daily value
   * being missed, the GPF shall retrieve the values from the GSME Profile Data
   * Log using the GetSampledData (SampleID 0x0000) command and GetSampledData
   * notification flag.
   */
  sl_zigbee_af_fill_command_simple_metering_cluster_get_sampled_data(sampleLog->sampleId,
                                                                     sampleLog->catchupTime,
                                                                     SL_ZIGBEE_ZCL_SAMPLE_TYPE_CONSUMPTION_DELIVERED,
                                                                     sampleLog->maxEntries);

  // Save the sequence number so we can match up the response with this request
  sampleLog->catchupSequenceNumber = appResponseData[1];
  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, dstNodeId);
}

static void getSnapshot(uint8_t srcEndpoint,
                        uint8_t dstEndpoint,
                        sl_802154_short_addr_t dstNodeId,
                        sli_zigbee_gpf_snapshot_log_t *snapshotLog)
{
  /**
   * GBCS v0.8.1 Section 10.4.2.1
   *
   * In the event of a communications outage, the GPF shall retrieve missing
   * snapshots using the GetSnapshot command, with the UTC start time field
   * populated based on the last received snapshot timestamp, if one has been
   * received.
   *
   * GBCS v0.8.1 Section 10.4.2.4
   *
   * In the event of a communications outage, the GPF shall retrieve missing
   * snapshots using the GetSnapshot command (and the relevant notification f
   * lag) with the UTC start time field populated based on the last received
   * snapshot timestamp, if one has been received, or 0x0000 otherwise.
   */
  sl_zigbee_af_fill_command_simple_metering_cluster_get_snapshot(snapshotLog->catchupTime,
                                                                 0xFFFFFFFF,
                                                                 snapshotLog->catchupSnapshotOffset,
                                                                 snapshotLog->catchupSnapshotCause);

  // Save the sequence number so we can match up the response with this request
  snapshotLog->catchupSequenceNumber = appResponseData[1];
  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, dstNodeId);
}

static void getPrepaySnapshot(uint8_t srcEndpoint,
                              uint8_t dstEndpoint,
                              sl_802154_short_addr_t dstNodeId,
                              sli_zigbee_gpf_prepay_snapshot_log_t *prepaySnapshotLog)
{
  /**
   * GBCS v0.8.1 Section 10.4.2.2
   *
   * In the event of a communications outage, the GPF shall retrieve missing
   * prepayment snapshots using the GetPrepaySnapshot command (and
   * GetPrepaySnapshot notification flag) with the UTC start time field
   * populated based on the last received prepayment snapshot timestamp, if one
   * has been received.
   *
   * GBCS v0.8.1 Section 10.4.2.7
   *
   * In the event of a communications outage, the GPF shall retrieve missing
   * snapshots using the GetPrepaySnapshot command (and GetPrepaySnapshot
   * notification flag) with the UTC start time field populated based on the
   * last received snapshot timestamp, if one has been received.
   */
  sl_zigbee_af_fill_command_prepayment_cluster_get_prepay_snapshot(prepaySnapshotLog->catchupTime,
                                                                   0xFFFFFFFF,
                                                                   prepaySnapshotLog->catchupSnapshotOffset,
                                                                   prepaySnapshotLog->catchupSnapshotCause);

  // Save the sequence number so we can match up the response with this request
  prepaySnapshotLog->catchupSequenceNumber = appResponseData[1];
  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, dstNodeId);
}

static void getTopUp(uint8_t srcEndpoint,
                     uint8_t dstEndpoint,
                     sl_802154_short_addr_t dstNodeId,
                     sli_zigbee_gpf_top_up_log_t *topUpLog)
{
  /**
   * GBCS v0.8.1 Section 10.4.2.5
   *
   * If there has been a communications outage, the GPF shall use the Get Top Up
   * Log command to retrieve all prepayment top-ups that may have been processed
   * during the communications outage.  The GSME shall set the Date / Time field
   * of the Get Top Up Log command to the current UTC time.
   */
  sl_zigbee_af_fill_command_prepayment_cluster_get_top_up_log(0xFFFFFFFF,
                                                              topUpLog->maxEntries);

  // Save the sequence number so we can match up the response with this request
  topUpLog->catchupSequenceNumber = appResponseData[1];
  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, dstNodeId);
}

static void getDebt(uint8_t srcEndpoint,
                    uint8_t dstEndpoint,
                    sl_802154_short_addr_t dstNodeId,
                    sli_zigbee_gpf_debt_log_t *debtLog)
{
  /**
   * GBCS v0.8.1 Section 10.4.2.6
   *
   * In cases of communications outages, the GPF shall request any outstanding
   * payment-based debt payments by use of the GetDebtRepaymentLog command
   * (and GetDebtRepaymentLog notification flag) with the Debt Type field set
   * to 0x02 (Debt 3).
   */
  sl_zigbee_af_fill_command_prepayment_cluster_get_debt_repayment_log(0xFFFFFFFF,
                                                                      debtLog->maxEntries,
                                                                      SL_ZIGBEE_ZCL_REPAYMENT_DEBT_TYPE_DEBT3);

  // Save the sequence number so we can match up the response with this request
  debtLog->catchupSequenceNumber = appResponseData[1];
  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, dstNodeId);
}

static void incrementLogCatchupsInProgress(void)
{
  if (++logCatchupsInProgress == 1) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Log catch-up started");
  }
}

static void decrementLogCatchupsInProgress(void)
{
  if (--logCatchupsInProgress == 0) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Log catch-up complete");
  }
}

void sli_zigbee_af_print_log_catchups_in_progress(void)
{
  sl_zigbee_af_gas_proxy_function_println("GPF: Log catch-ups %d", logCatchupsInProgress);
}

static void resetAlternativeHistoricalConsumption(uint8_t endpoint,
                                                  GpfAlternativeHistoricalConsumption *altConsumption)
{
  uint8_t i;
  sl_zigbee_af_status_t status;
  uint8_t consumption[] = { 0, 0, 0, 0 };

  for (i = 0; i < NUM_ALT_CONSUMPTION_DAY_ATTRS; i++) {
    status = sl_zigbee_af_write_server_attribute(endpoint,
                                                 ZCL_SIMPLE_METERING_CLUSTER_ID,
                                                 altConsumptionDayAttrIds[i],
                                                 consumption,
                                                 ZCL_INT24U_ATTRIBUTE_TYPE);
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_af_gas_proxy_function_println("GPF: ERR: writing metering server attribute 0x%04X - status 0x%02X",
                                              altConsumptionDayAttrIds[i], status);
    }
  }

  for (i = 0; i < NUM_ALT_CONSUMPTION_WEEK_ATTRS; i++) {
    status = sl_zigbee_af_write_server_attribute(endpoint,
                                                 ZCL_SIMPLE_METERING_CLUSTER_ID,
                                                 altConsumptionWeekAttrIds[i],
                                                 consumption,
                                                 ZCL_INT24U_ATTRIBUTE_TYPE);
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_af_gas_proxy_function_println("GPF: ERR: writing metering server attribute 0x%04X - status 0x%02X",
                                              altConsumptionWeekAttrIds[i], status);
    }
  }

  for (i = 0; i < NUM_ALT_CONSUMPTION_MONTH_ATTRS; i++) {
    status = sl_zigbee_af_write_server_attribute(endpoint,
                                                 ZCL_SIMPLE_METERING_CLUSTER_ID,
                                                 altConsumptionMonthAttrIds[i],
                                                 consumption,
                                                 ZCL_INT32U_ATTRIBUTE_TYPE);
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_af_gas_proxy_function_println("GPF: ERR: writing metering server attribute 0x%04X - status 0x%02X",
                                              altConsumptionMonthAttrIds[i], status);
    }
  }

  altConsumption->catchup = true;
  altConsumption->prevCurrentDayAlternativeConsumption = 0;
  altConsumption->prevCurrentDayAlternativeConsumptionTime = sl_zigbee_af_get_current_time();

  setNotificationFlag(endpoint,
                      ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID,
                      SL_ZIGBEE_AF_METERING_FNF_PUSH_HISTORICAL_METERING_DATA_ATTRIBUTE_SET);
}

static void addToByteArray(uint8_t *data,
                           uint8_t len,
                           uint32_t toAdd)
{
#if (BIGENDIAN_CPU)
  int8_t loc  = len - 1;
  int8_t end  = -1;
  int8_t incr = -1;
#else
  int8_t loc  = 0;
  int8_t end  = len;
  int8_t incr = 1;
#endif
  uint16_t sum = 0;

  while ( loc != end ) {
    uint8_t t, s;
    t = data[loc];
    s = toAdd & 0xff;
    sum += t + s;
    data[loc] = sum & 0xff;
    sum >>= 8;
    toAdd >>= 8;
    loc += incr;
  }
}

/*
 * GBCS v0.8.1 section 10.4.2.10
 *
 * As per Section 10.4.2.8, the GSME shall, on each half hour, record the
 * following information and push to the GPF:
 *   - total consumption value (with units of m3);
 *   - total consumption today (with units of kWh); and
 *   - total cost of consumption today (with units of Currency Unit);
 * Using the 'total consumption today' value, the GPF shall update the
 * attributes of the mirrored Alternative Historical Consumption attribute set.
 */
static void updateAlternativeHistoricalConsumption(uint8_t endpoint,
                                                   uint32_t currentConsumptionTime,
                                                   uint32_t currentConsumption,
                                                   GpfAlternativeHistoricalConsumption *altConsumption)
{
  bool newDay = false;
  uint8_t i;
  uint8_t consumption24[] = { 0, 0, 0 };
  uint8_t consumption32[] = { 0, 0, 0, 0 };
  sl_zigbee_af_time_struct_t previousConsumptionTimeStruct;
  sl_zigbee_af_time_struct_t currentConsumptionTimeStruct;
  uint8_t previousConsumptionDayOfWeek;
  uint8_t currentConsumptionDayOfWeek;
  uint32_t consumption;

  sl_zigbee_af_fill_time_struct_from_utc(altConsumption->prevCurrentDayAlternativeConsumptionTime, &previousConsumptionTimeStruct);
  sl_zigbee_af_fill_time_struct_from_utc(currentConsumptionTime, &currentConsumptionTimeStruct);
  previousConsumptionDayOfWeek = sl_zigbee_af_get_weekday_from_utc(altConsumption->prevCurrentDayAlternativeConsumptionTime);
  currentConsumptionDayOfWeek = sl_zigbee_af_get_weekday_from_utc(currentConsumptionTime);
  consumption = currentConsumption - altConsumption->prevCurrentDayAlternativeConsumption;

  // If it's a new day then roll all the day attributes
  if (previousConsumptionTimeStruct.day != currentConsumptionTimeStruct.day) {
    newDay = true;
    for (i = NUM_ALT_CONSUMPTION_DAY_ATTRS - 1; i > 0; i--) {
      sl_zigbee_af_read_server_attribute(endpoint,
                                         ZCL_SIMPLE_METERING_CLUSTER_ID,
                                         altConsumptionDayAttrIds[i - 1],
                                         consumption24,
                                         3);
      sl_zigbee_af_write_server_attribute(endpoint,
                                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                                          altConsumptionDayAttrIds[i],
                                          consumption24,
                                          ZCL_INT24U_ATTRIBUTE_TYPE);
    }
    memset(consumption24, 0, 3);
    sl_zigbee_af_write_server_attribute(endpoint,
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        altConsumptionDayAttrIds[0],
                                        consumption24,
                                        ZCL_INT24U_ATTRIBUTE_TYPE);
  }

  //Update the current week consumption attribute
  sl_zigbee_af_read_server_attribute(endpoint,
                                     ZCL_SIMPLE_METERING_CLUSTER_ID,
                                     ZCL_CURRENT_WEEK_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
                                     consumption24,
                                     3);
  addToByteArray(consumption24, 3, consumption);
  sl_zigbee_af_write_server_attribute(endpoint,
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_CURRENT_WEEK_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
                                      consumption24,
                                      ZCL_INT24U_ATTRIBUTE_TYPE);

  // If it's a new week then roll all the week attributes
  if (previousConsumptionDayOfWeek != currentConsumptionDayOfWeek && currentConsumptionDayOfWeek == 0) {
    for (i = NUM_ALT_CONSUMPTION_WEEK_ATTRS - 1; i > 0; i--) {
      sl_zigbee_af_read_server_attribute(endpoint,
                                         ZCL_SIMPLE_METERING_CLUSTER_ID,
                                         altConsumptionWeekAttrIds[i - 1],
                                         consumption24,
                                         3);
      sl_zigbee_af_write_server_attribute(endpoint,
                                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                                          altConsumptionWeekAttrIds[i],
                                          consumption24,
                                          ZCL_INT24U_ATTRIBUTE_TYPE);
    }
    memset(consumption24, 0, 3);
    sl_zigbee_af_write_server_attribute(endpoint,
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        altConsumptionWeekAttrIds[0],
                                        consumption24,
                                        ZCL_INT24U_ATTRIBUTE_TYPE);
  }

  //Update the current month consumption attribute
  sl_zigbee_af_read_server_attribute(endpoint,
                                     ZCL_SIMPLE_METERING_CLUSTER_ID,
                                     ZCL_CURRENT_MONTH_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
                                     consumption32,
                                     4);
  addToByteArray(consumption32, 4, consumption);
  sl_zigbee_af_write_server_attribute(endpoint,
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_CURRENT_MONTH_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
                                      consumption32,
                                      ZCL_INT32U_ATTRIBUTE_TYPE);

  // If it's a new month then roll all the month attributes
  if (previousConsumptionTimeStruct.month != currentConsumptionTimeStruct.month) {
    for (i = NUM_ALT_CONSUMPTION_MONTH_ATTRS - 1; i > 0; i--) {
      sl_zigbee_af_read_server_attribute(endpoint,
                                         ZCL_SIMPLE_METERING_CLUSTER_ID,
                                         altConsumptionMonthAttrIds[i - 1],
                                         consumption32,
                                         4);
      sl_zigbee_af_write_server_attribute(endpoint,
                                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                                          altConsumptionMonthAttrIds[i],
                                          consumption32,
                                          ZCL_INT32U_ATTRIBUTE_TYPE);
    }
    memset(consumption32, 0, 4);
    sl_zigbee_af_write_server_attribute(endpoint,
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        altConsumptionMonthAttrIds[0],
                                        consumption32,
                                        ZCL_INT32U_ATTRIBUTE_TYPE);
  }

  altConsumption->prevCurrentDayAlternativeConsumptionTime = currentConsumptionTime;
  altConsumption->prevCurrentDayAlternativeConsumption = (newDay) ? 0 : currentConsumption;
}

static void resetHistoricalCostConsumptionInformation(uint8_t endpoint,
                                                      sli_zigbee_gpf_historical_cost_consumption_t *costConsumption)
{
  uint8_t i;
  sl_zigbee_af_status_t status;
  uint8_t cost[] = { 0, 0, 0, 0, 0, 0 };

  for (i = 0; i < NUM_COST_CONSUMPTION_DAY_ATTRS; i++) {
    status = sl_zigbee_af_write_server_attribute(endpoint,
                                                 ZCL_PREPAYMENT_CLUSTER_ID,
                                                 costConsumptionDayAttrIds[i],
                                                 cost,
                                                 ZCL_INT48U_ATTRIBUTE_TYPE);
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_af_gas_proxy_function_println("GPF: ERR: writing prepayment server attribute 0x%04X - status 0x%02X",
                                              costConsumptionDayAttrIds[i], status);
    }
  }

  for (i = 0; i < NUM_COST_CONSUMPTION_WEEK_ATTRS; i++) {
    status = sl_zigbee_af_write_server_attribute(endpoint,
                                                 ZCL_PREPAYMENT_CLUSTER_ID,
                                                 costConsumptionWeekAttrIds[i],
                                                 cost,
                                                 ZCL_INT48U_ATTRIBUTE_TYPE);
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_af_gas_proxy_function_println("GPF: ERR: writing prepayment server attribute 0x%04X - status 0x%02X",
                                              costConsumptionWeekAttrIds[i], status);
    }
  }

  for (i = 0; i < NUM_COST_CONSUMPTION_MONTH_ATTRS; i++) {
    status = sl_zigbee_af_write_server_attribute(endpoint,
                                                 ZCL_PREPAYMENT_CLUSTER_ID,
                                                 costConsumptionMonthAttrIds[i],
                                                 cost,
                                                 ZCL_INT48U_ATTRIBUTE_TYPE);
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_af_gas_proxy_function_println("GPF: ERR: writing prepayment server attribute 0x%04X - status 0x%02X",
                                              costConsumptionMonthAttrIds[i], status);
    }
  }

  costConsumption->catchup = true;
  costConsumption->prevCurrentDayCostConsumption = 0;
  costConsumption->prevCurrentDayCostConsumptionTime = sl_zigbee_af_get_current_time();

  setNotificationFlag(endpoint,
                      ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID,
                      SL_ZIGBEE_AF_METERING_FNF_PUSH_HISTORICAL_PREPAYMENT_DATA_ATTRIBUTE_SET);
}

/*
 * GBCS v0.8.1 section 10.4.2.10
 *
 * As per Section 10.4.2.8, the GSME shall, on each half hour, record the
 * following information and push to the GPF:
 *   - total consumption value (with units of m3);
 *   - total consumption today (with units of kWh); and
 *   - total cost of consumption today (with units of Currency Unit);
 * Using the 'total cost of consumption today' value, the GPF shall update the
 * attributes of the mirrored Historical Cost Consumption Information
 * attribute set.
 */
static void updateHistoricalCostConsumptionInformation(uint8_t endpoint,
                                                       uint32_t currentCostTime,
                                                       uint32_t currentCost,
                                                       sli_zigbee_gpf_historical_cost_consumption_t *costConsumption)
{
  bool newDay = false;
  uint8_t i;
  uint8_t cost48[] = { 0, 0, 0, 0, 0, 0 };
  sl_zigbee_af_time_struct_t previousCostTimeStruct;
  sl_zigbee_af_time_struct_t currentCostTimeStruct;
  uint8_t previousCostDayOfWeek;
  uint8_t currentCostDayOfWeek;
  uint32_t cost;

  sl_zigbee_af_fill_time_struct_from_utc(costConsumption->prevCurrentDayCostConsumptionTime, &previousCostTimeStruct);
  sl_zigbee_af_fill_time_struct_from_utc(currentCostTime, &currentCostTimeStruct);
  previousCostDayOfWeek = sl_zigbee_af_get_weekday_from_utc(costConsumption->prevCurrentDayCostConsumptionTime);
  currentCostDayOfWeek = sl_zigbee_af_get_weekday_from_utc(currentCostTime);
  cost = currentCost - costConsumption->prevCurrentDayCostConsumption;

  // If it's a new day then roll all the day attributes
  if (previousCostTimeStruct.day != currentCostTimeStruct.day) {
    newDay = true;
    for (i = NUM_COST_CONSUMPTION_DAY_ATTRS - 1; i > 0; i--) {
      sl_zigbee_af_read_server_attribute(endpoint,
                                         ZCL_PREPAYMENT_CLUSTER_ID,
                                         costConsumptionDayAttrIds[i - 1],
                                         cost48,
                                         6);
      sl_zigbee_af_write_server_attribute(endpoint,
                                          ZCL_PREPAYMENT_CLUSTER_ID,
                                          costConsumptionDayAttrIds[i],
                                          cost48,
                                          ZCL_INT48U_ATTRIBUTE_TYPE);
    }
    memset(cost48, 0, 6);
    sl_zigbee_af_write_server_attribute(endpoint,
                                        ZCL_PREPAYMENT_CLUSTER_ID,
                                        costConsumptionDayAttrIds[0],
                                        cost48,
                                        ZCL_INT48U_ATTRIBUTE_TYPE);
  }

  //Update the current week consumption attribute
  sl_zigbee_af_read_server_attribute(endpoint,
                                     ZCL_PREPAYMENT_CLUSTER_ID,
                                     ZCL_CURRENT_WEEK_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
                                     cost48,
                                     6);
  addToByteArray(cost48, 6, cost);
  sl_zigbee_af_write_server_attribute(endpoint,
                                      ZCL_PREPAYMENT_CLUSTER_ID,
                                      ZCL_CURRENT_WEEK_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
                                      cost48,
                                      ZCL_INT48U_ATTRIBUTE_TYPE);

  // If it's a new week then roll all the week attributes
  if (previousCostDayOfWeek != currentCostDayOfWeek && currentCostDayOfWeek == 0) {
    for (i = NUM_COST_CONSUMPTION_WEEK_ATTRS - 1; i > 0; i--) {
      sl_zigbee_af_read_server_attribute(endpoint,
                                         ZCL_PREPAYMENT_CLUSTER_ID,
                                         costConsumptionWeekAttrIds[i - 1],
                                         cost48,
                                         6);
      sl_zigbee_af_write_server_attribute(endpoint,
                                          ZCL_PREPAYMENT_CLUSTER_ID,
                                          costConsumptionWeekAttrIds[i],
                                          cost48,
                                          ZCL_INT48U_ATTRIBUTE_TYPE);
    }
    memset(cost48, 0, 6);
    sl_zigbee_af_write_server_attribute(endpoint,
                                        ZCL_PREPAYMENT_CLUSTER_ID,
                                        costConsumptionWeekAttrIds[0],
                                        cost48,
                                        ZCL_INT48U_ATTRIBUTE_TYPE);
  }

  //Update the current month consumption attribute
  sl_zigbee_af_read_server_attribute(endpoint,
                                     ZCL_PREPAYMENT_CLUSTER_ID,
                                     ZCL_CURRENT_MONTH_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
                                     cost48,
                                     6);
  addToByteArray(cost48, 6, cost);
  sl_zigbee_af_write_server_attribute(endpoint,
                                      ZCL_PREPAYMENT_CLUSTER_ID,
                                      ZCL_CURRENT_MONTH_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
                                      cost48,
                                      ZCL_INT48U_ATTRIBUTE_TYPE);

  // If it's a new month then roll all the month attributes
  if (previousCostTimeStruct.month != currentCostTimeStruct.month) {
    for (i = NUM_COST_CONSUMPTION_MONTH_ATTRS - 1; i > 0; i--) {
      sl_zigbee_af_read_server_attribute(endpoint,
                                         ZCL_PREPAYMENT_CLUSTER_ID,
                                         costConsumptionMonthAttrIds[i - 1],
                                         cost48,
                                         6);
      sl_zigbee_af_write_server_attribute(endpoint,
                                          ZCL_PREPAYMENT_CLUSTER_ID,
                                          costConsumptionMonthAttrIds[i],
                                          cost48,
                                          ZCL_INT48U_ATTRIBUTE_TYPE);
    }
    memset(cost48, 0, 6);
    sl_zigbee_af_write_server_attribute(endpoint,
                                        ZCL_PREPAYMENT_CLUSTER_ID,
                                        costConsumptionMonthAttrIds[0],
                                        cost48,
                                        ZCL_INT48U_ATTRIBUTE_TYPE);
  }

  costConsumption->prevCurrentDayCostConsumptionTime = currentCostTime;
  costConsumption->prevCurrentDayCostConsumption = (newDay) ? 0 : currentCost;
}

static void startSampleLogCatchup(uint8_t endpoint, sli_zigbee_gpf_sample_log_t *sampleLog, bool reset)
{
  sampleLog->catchup = true;
  sampleLog->catchupSequenceNumber = 0xFF;
  if (reset) {
    sampleLog->nextEntry = 0;
    sampleLog->numberOfEntries = 0;
    sampleLog->startTime = 0;
    sampleLog->prevSummation = 0;
    sampleLog->catchupTime = 0;
  }

  setNotificationFlag(endpoint,
                      ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID,
                      SL_ZIGBEE_AF_METERING_FNF_GET_SAMPLED_DATA);
  incrementLogCatchupsInProgress();
}

static void stopSampleLogCatchup(uint8_t endpoint, sli_zigbee_gpf_sample_log_t *sampleLog, sli_zigbee_gpf_sample_log_t *otherSampleLog)
{
  sampleLog->catchup = false;
  sampleLog->catchupSequenceNumber = 0xFF;
  if (!otherSampleLog->catchup) {
    clearNotificationFlag(endpoint,
                          ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID,
                          SL_ZIGBEE_AF_METERING_FNF_GET_SAMPLED_DATA);
  }
  decrementLogCatchupsInProgress();
}

static void startSnapshotLogCatchup(uint8_t endpoint, sli_zigbee_gpf_snapshot_log_t *snapshotLog, bool reset)
{
  snapshotLog->catchup = true;
  snapshotLog->catchupSequenceNumber = 0xFF;
  if (reset) {
    snapshotLog->catchupSnapshotOffset = 0;
    snapshotLog->catchupTime = 0;
    snapshotLog->nextEntry = 0;
    snapshotLog->numberOfEntries = 0;
  }

  setNotificationFlag(endpoint,
                      ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID,
                      SL_ZIGBEE_AF_METERING_FNF_GET_SNAPSHOT);
  incrementLogCatchupsInProgress();
}

static void stopSnapshotLogCatchup(uint8_t endpoint, sli_zigbee_gpf_snapshot_log_t *snapshotLog, sli_zigbee_gpf_snapshot_log_t *otherSnapshotLog)
{
  snapshotLog->catchup = false;
  snapshotLog->catchupSequenceNumber = 0xFF;
  if (!otherSnapshotLog->catchup) {
    clearNotificationFlag(endpoint,
                          ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID,
                          SL_ZIGBEE_AF_METERING_FNF_GET_SNAPSHOT);
  }
  decrementLogCatchupsInProgress();
}

static void startPrepaySnapshotLogCatchup(uint8_t endpoint, sli_zigbee_gpf_prepay_snapshot_log_t *prepaySnapshotLog, bool reset)
{
  prepaySnapshotLog->catchup = true;
  prepaySnapshotLog->catchupSequenceNumber = 0xFF;
  if (reset) {
    prepaySnapshotLog->catchupSnapshotOffset = 0;
    prepaySnapshotLog->catchupTime = 0;
    prepaySnapshotLog->nextEntry = 0;
    prepaySnapshotLog->numberOfEntries = 0;
  }

  setNotificationFlag(endpoint,
                      ZCL_NOTIFICATION_FLAGS_4_ATTRIBUTE_ID,
                      SL_ZIGBEE_AF_METERING_NF4_GET_PREPAY_SNAPSHOT);
  incrementLogCatchupsInProgress();
}

static void stopPrepaySnapshotLogCatchup(uint8_t endpoint, sli_zigbee_gpf_prepay_snapshot_log_t *prepaySnapshotLog, sli_zigbee_gpf_prepay_snapshot_log_t *otherPrepaySnapshotLog)
{
  prepaySnapshotLog->catchup = false;
  prepaySnapshotLog->catchupSequenceNumber = 0xFF;
  if (!otherPrepaySnapshotLog->catchup) {
    clearNotificationFlag(endpoint,
                          ZCL_NOTIFICATION_FLAGS_4_ATTRIBUTE_ID,
                          SL_ZIGBEE_AF_METERING_NF4_GET_PREPAY_SNAPSHOT);
  }
  decrementLogCatchupsInProgress();
}

static void startTopUpLogCatchup(uint8_t endpoint, sli_zigbee_gpf_top_up_log_t *topUpLog)
{
  topUpLog->catchup = true;
  topUpLog->catchupSequenceNumber = 0xFF;
  topUpLog->nextEntry = 0;
  topUpLog->numberOfEntries = 0;
  setNotificationFlag(endpoint,
                      ZCL_NOTIFICATION_FLAGS_4_ATTRIBUTE_ID,
                      SL_ZIGBEE_AF_METERING_NF4_GET_TOP_UP_LOG);
  incrementLogCatchupsInProgress();
}

static void stopTopUpLogCatchup(uint8_t endpoint, sli_zigbee_gpf_top_up_log_t *topUpLog)
{
  topUpLog->catchup = false;
  topUpLog->catchupSequenceNumber = 0xFF;
  clearNotificationFlag(endpoint,
                        ZCL_NOTIFICATION_FLAGS_4_ATTRIBUTE_ID,
                        SL_ZIGBEE_AF_METERING_NF4_GET_TOP_UP_LOG);
  decrementLogCatchupsInProgress();
}

static void startDebtLogCatchup(uint8_t endpoint, sli_zigbee_gpf_debt_log_t *debtLog)
{
  debtLog->catchup = true;
  debtLog->catchupSequenceNumber = 0xFF;
  debtLog->nextEntry = 0;
  debtLog->numberOfEntries = 0;
  setNotificationFlag(endpoint,
                      ZCL_NOTIFICATION_FLAGS_4_ATTRIBUTE_ID,
                      SL_ZIGBEE_AF_METERING_NF4_GET_DEBT_REPAYMENT_LOG);
  incrementLogCatchupsInProgress();
}

static void stopDebtLogCatchup(uint8_t endpoint, sli_zigbee_gpf_debt_log_t *debtLog)
{
  debtLog->catchup = false;
  debtLog->catchupSequenceNumber = 0xFF;
  clearNotificationFlag(endpoint,
                        ZCL_NOTIFICATION_FLAGS_4_ATTRIBUTE_ID,
                        SL_ZIGBEE_AF_METERING_NF4_GET_DEBT_REPAYMENT_LOG);
  decrementLogCatchupsInProgress();
}

/*
 *  Set the daily consumption log's previous summation value using the values
 *  from the profile log to subtract from the last summation recorded in the
 *  profile log back to the beginning of the day.
 */
static void setDailyConsumptionLogPrevSummation(sli_zigbee_gpf_sample_log_t *profileLog,
                                                sli_zigbee_gpf_sample_log_t *dailyLog)
{
  uint32_t dailyLogLastUpdateTime;
  uint32_t profileLogLastUpdateTime;
  uint32_t now = sl_zigbee_af_get_current_time();
  uint32_t dailyLogSummation;
  uint32_t dailyLogSummationTime;
  uint16_t i;
  uint16_t entryIndex;

  // We only set the previous summation if it has not yet been set.
  if (dailyLog->prevSummation != 0) {
    return;
  }

  // Determine the time when the last summation was or should have
  // been set within the daily consumption log.
  if (dailyLog->numberOfEntries > 0) {
    // Last summation time is the last entry in the daily log
    dailyLogLastUpdateTime = dailyLog->entries[getLastIndex(dailyLog)].time;
  } else if (dailyLog->startTime) {
    // Last summation time is when the log was started
    dailyLogLastUpdateTime = dailyLog->startTime;
  } else {
    // Last summation time is 12:00 AM this morning
    dailyLogLastUpdateTime = PREV_MIDNIGHT(now);
  }

  profileLogLastUpdateTime = (0 == profileLog->numberOfEntries) ? profileLog->startTime
                             : profileLog->entries[getLastIndex(profileLog)].time;
  if (profileLogLastUpdateTime < dailyLogLastUpdateTime) {
    // Nothing we can do.  The last entry in the profile log is older then the
    // last entry in the daily consumption log.
    return;
  }

  // Start with the last summation value stored in the profile log
  dailyLogSummation = profileLog->prevSummation;
  dailyLogSummationTime = profileLogLastUpdateTime;

  // Now loop backward through the profile log subtracting the sample value
  // of each entry from the summation.  We do this until we reach the time
  // of the last entry in the daily consumption log.
  for (i = 0, entryIndex = getLastIndex(profileLog);
       i < profileLog->numberOfEntries;
       i++, entryIndex = getPrevIndex(profileLog, entryIndex)) {
    // startTime should always be one interval earlier of the time of the first entry
    uint32_t prevTime = ((i + 1) == profileLog->numberOfEntries) ? profileLog->startTime
                        : profileLog->entries[getPrevIndex(profileLog, entryIndex)].time;

    // If the next sample going backwards in the log is in the previous day
    // then break out of the loop.
    if (prevTime < dailyLogLastUpdateTime) {
      break;
    }

    dailyLogSummation -= profileLog->entries[entryIndex].sample;
    dailyLogSummationTime = profileLog->entries[entryIndex].time;
  }

  if (dailyLog->startTime == 0) {
    dailyLog->startTime = dailyLogSummationTime;
  }
  dailyLog->prevSummation = dailyLogSummation;
}

static uint8_t getInt48u(uint8_t* message, uint16_t currentIndex, uint16_t msgLen, uint8_t *destination)
{
  if ((currentIndex + 6) > msgLen) {
    sl_zigbee_af_gas_proxy_function_println("GetInt48u, %02X bytes short", 6);
    return 0;
  }
#if (BIGENDIAN_CPU)
  sl_util_reverse_mem_copy(destination, message + currentIndex, 6);
#else
  memcpy(destination, message + currentIndex, 6);
#endif
  return 6;
}

static void updateSnapshotAttributes(uint8_t endpoint,
                                     uint8_t snapshotPayloadType,
                                     uint8_t *snapshotPayload)
{
  uint16_t snapshotPayloadLength = fieldLength(snapshotPayload);
  uint16_t snapshotPayloadIndex = 0;
  uint8_t summationDelivered[6];
  uint32_t billToDateDelivered;
  uint32_t billToDateTimeStampDelivered;
  uint32_t projectedBillDelivered;
  uint32_t projectedBillTimeStampDelivered;
  uint8_t billDeliveredTrailingDigit;
  uint8_t numberOfTiersInUse;
  uint8_t numberOfTiersAndBlockThresholdsInUse;
  uint8_t i;

  getInt48u(snapshotPayload, snapshotPayloadIndex, snapshotPayloadLength, summationDelivered);
  snapshotPayloadIndex += 6;
  sl_zigbee_af_write_server_attribute(endpoint,
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID,
                                      summationDelivered,
                                      ZCL_INT48U_ATTRIBUTE_TYPE);

  //Related to GBCS IRP328: snapshotPayloadType 4 and 6 do not support these attributes
  if (snapshotPayloadType  == SL_ZIGBEE_ZCL_SNAPSHOT_PAYLOAD_TYPE_TOU_INFORMATION_SET_DELIVERED_REGISTERS
      || snapshotPayloadType == SL_ZIGBEE_ZCL_SNAPSHOT_PAYLOAD_TYPE_BLOCK_TIER_INFORMATION_SET_DELIVERED) {
    billToDateDelivered = sl_zigbee_af_get_int32u(snapshotPayload, snapshotPayloadIndex, snapshotPayloadLength);
    snapshotPayloadIndex += 4;
    sl_zigbee_af_write_server_attribute(endpoint,
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        ZCL_BILL_TO_DATE_DELIVERED_ATTRIBUTE_ID,
                                        (uint8_t *)&billToDateDelivered,
                                        ZCL_INT32U_ATTRIBUTE_TYPE);
    billToDateTimeStampDelivered = sl_zigbee_af_get_int32u(snapshotPayload, snapshotPayloadIndex, snapshotPayloadLength);
    snapshotPayloadIndex += 4;
    sl_zigbee_af_write_server_attribute(endpoint,
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        ZCL_BILL_TO_DATE_TIME_STAMP_DELIVERED_ATTRIBUTE_ID,
                                        (uint8_t *)&billToDateTimeStampDelivered,
                                        ZCL_UTC_TIME_ATTRIBUTE_TYPE);
    projectedBillDelivered = sl_zigbee_af_get_int32u(snapshotPayload, snapshotPayloadIndex, snapshotPayloadLength);
    snapshotPayloadIndex += 4;
    sl_zigbee_af_write_server_attribute(endpoint,
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        ZCL_PROJECTED_BILL_DELIVERED_ATTRIBUTE_ID,
                                        (uint8_t *)&projectedBillDelivered,
                                        ZCL_INT32U_ATTRIBUTE_TYPE);
    projectedBillTimeStampDelivered = sl_zigbee_af_get_int32u(snapshotPayload, snapshotPayloadIndex, snapshotPayloadLength);
    snapshotPayloadIndex += 4;
    sl_zigbee_af_write_server_attribute(endpoint,
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        ZCL_PROJECTED_BILL_TIME_STAMP_DELIVERED_ATTRIBUTE_ID,
                                        (uint8_t *)&projectedBillTimeStampDelivered,
                                        ZCL_UTC_TIME_ATTRIBUTE_TYPE);
    billDeliveredTrailingDigit = sl_zigbee_af_get_int8u(snapshotPayload, snapshotPayloadIndex, snapshotPayloadLength);
    snapshotPayloadIndex += 1;
    sl_zigbee_af_write_server_attribute(endpoint,
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        ZCL_BILL_DELIVERED_TRAILING_DIGIT_ATTRIBUTE_ID,
                                        (uint8_t *)&billDeliveredTrailingDigit,
                                        ZCL_BITMAP8_ATTRIBUTE_TYPE);
  }
  numberOfTiersInUse = sl_zigbee_af_get_int8u(snapshotPayload, snapshotPayloadIndex, snapshotPayloadLength);
  snapshotPayloadIndex += 1;
  for (i = 0; i < numberOfTiersInUse; i++) {
    getInt48u(snapshotPayload, snapshotPayloadIndex, snapshotPayloadLength, summationDelivered);
    snapshotPayloadIndex += 6;
    sl_zigbee_af_write_server_attribute(endpoint,
                                        ZCL_SIMPLE_METERING_CLUSTER_ID,
                                        ZCL_CURRENT_TIER1_SUMMATION_DELIVERED_ATTRIBUTE_ID + (i * 2),
                                        summationDelivered,
                                        ZCL_INT48U_ATTRIBUTE_TYPE);
  }

  if (snapshotPayloadType == SL_ZIGBEE_ZCL_SNAPSHOT_PAYLOAD_TYPE_BLOCK_TIER_INFORMATION_SET_DELIVERED
      || snapshotPayloadType == SL_ZIGBEE_ZCL_SNAPSHOT_PAYLOAD_TYPE_BLOCK_TIER_INFORMATION_SET_DELIVERED_NO_BILLING) {
    numberOfTiersAndBlockThresholdsInUse = sl_zigbee_af_get_int8u(snapshotPayload, snapshotPayloadIndex, snapshotPayloadLength);
    snapshotPayloadIndex += 1;
    for (i = 0; i < numberOfTiersAndBlockThresholdsInUse; i++) {
      getInt48u(snapshotPayload, snapshotPayloadIndex, snapshotPayloadLength, summationDelivered);
      snapshotPayloadIndex += 6;
      sl_zigbee_af_write_server_attribute(endpoint,
                                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                                          ZCL_CURRENT_NO_TIER_BLOCK1_SUMMATION_DELIVERED_ATTRIBUTE_ID + (i),
                                          summationDelivered,
                                          ZCL_INT48U_ATTRIBUTE_TYPE);
    }
  }
}

static void updatePrepaySnapshotAttributes(uint8_t endpoint,
                                           uint8_t snapshotPayloadType,
                                           uint8_t *snapshotPayload)
{
  UNUSED_VAR(snapshotPayloadType);

  uint16_t snapshotPayloadLength = fieldLength(snapshotPayload);
  uint16_t snapshotPayloadIndex = 0;
  uint32_t accumulatedDebt;
  uint32_t type1DebtRemaining;
  uint32_t type2DebtRemaining;
  uint32_t type3DebtRemaining;
  uint32_t emergencyCreditRemaining;
  uint32_t creditRemaining;

  accumulatedDebt = sl_zigbee_af_get_int32u(snapshotPayload, snapshotPayloadIndex, snapshotPayloadLength);
  snapshotPayloadIndex += 4;
  sl_zigbee_af_write_server_attribute(endpoint,
                                      ZCL_PREPAYMENT_CLUSTER_ID,
                                      ZCL_ACCUMULATED_DEBT_ATTRIBUTE_ID,
                                      (uint8_t *)&accumulatedDebt,
                                      ZCL_INT32S_ATTRIBUTE_TYPE);
  type1DebtRemaining = sl_zigbee_af_get_int32u(snapshotPayload, snapshotPayloadIndex, snapshotPayloadLength);
  snapshotPayloadIndex += 4;
  sl_zigbee_af_write_server_attribute(endpoint,
                                      ZCL_PREPAYMENT_CLUSTER_ID,
                                      ZCL_DEBT_AMOUNT_1_ATTRIBUTE_ID,
                                      (uint8_t *)&type1DebtRemaining,
                                      ZCL_INT32U_ATTRIBUTE_TYPE);
  type2DebtRemaining = sl_zigbee_af_get_int32u(snapshotPayload, snapshotPayloadIndex, snapshotPayloadLength);
  snapshotPayloadIndex += 4;
  sl_zigbee_af_write_server_attribute(endpoint,
                                      ZCL_PREPAYMENT_CLUSTER_ID,
                                      ZCL_DEBT_AMOUNT_2_ATTRIBUTE_ID,
                                      (uint8_t *)&type2DebtRemaining,
                                      ZCL_INT32U_ATTRIBUTE_TYPE);
  type3DebtRemaining = sl_zigbee_af_get_int32u(snapshotPayload, snapshotPayloadIndex, snapshotPayloadLength);
  snapshotPayloadIndex += 4;
  sl_zigbee_af_write_server_attribute(endpoint,
                                      ZCL_PREPAYMENT_CLUSTER_ID,
                                      ZCL_DEBT_AMOUNT_3_ATTRIBUTE_ID,
                                      (uint8_t *)&type3DebtRemaining,
                                      ZCL_INT32U_ATTRIBUTE_TYPE);
  emergencyCreditRemaining = sl_zigbee_af_get_int32u(snapshotPayload, snapshotPayloadIndex, snapshotPayloadLength);
  snapshotPayloadIndex += 4;
  sl_zigbee_af_write_server_attribute(endpoint,
                                      ZCL_PREPAYMENT_CLUSTER_ID,
                                      ZCL_EMERGENCY_CREDIT_REMAINING_ATTRIBUTE_ID,
                                      (uint8_t *)&emergencyCreditRemaining,
                                      ZCL_INT32S_ATTRIBUTE_TYPE);
  creditRemaining = sl_zigbee_af_get_int32u(snapshotPayload, snapshotPayloadIndex, snapshotPayloadLength);
  snapshotPayloadIndex += 4;
  sl_zigbee_af_write_server_attribute(endpoint,
                                      ZCL_PREPAYMENT_CLUSTER_ID,
                                      ZCL_CREDIT_REMAINING_ATTRIBUTE_ID,
                                      (uint8_t *)&creditRemaining,
                                      ZCL_INT32S_ATTRIBUTE_TYPE);
}

// GBCS v0.8.1 section 10.4.2.8 - the profile data log should be updated
// every 30 minutes.
// GBCS v0.8.1 section 10.4.2.9 - the daily gas consumption log should be
// updated every day.
// GBCS v0.8.1 section 10.4.2.1 - the daily read log should be updated
// every day.
// GBCS v0.8.1 section 10.4.2.2 - the prepayment daily read log should be
// updated every day.

static bool sampleLogIsOutOfDate(sli_zigbee_gpf_sample_log_t *sampleLog,
                                 uint32_t lastExpectedEntryTime,
                                 uint32_t *lastEntryTime)
{
  *lastEntryTime = (sampleLog->numberOfEntries > 0
                    ? sampleLog->entries[getLastIndex(sampleLog)].time
                    : 0);

  return (*lastEntryTime == 0 ? true : lastExpectedEntryTime > *lastEntryTime);
}

static bool snapshotLogIsOutOfDate(sli_zigbee_gpf_snapshot_log_t *snapshotLog,
                                   uint32_t lastExpectedEntryTime,
                                   uint32_t *lastEntryTime)
{
  *lastEntryTime = (snapshotLog->numberOfEntries > 0
                    ? snapshotLog->entries[getLastIndex(snapshotLog)].time
                    : 0);

  return (*lastEntryTime == 0 ? true : lastExpectedEntryTime > *lastEntryTime);
}

static bool prepaySnapshotLogIsOutOfDate(sli_zigbee_gpf_prepay_snapshot_log_t *prepaySnapshotLog,
                                         uint32_t lastExpectedEntryTime,
                                         uint32_t *lastEntryTime)
{
  *lastEntryTime = (prepaySnapshotLog->numberOfEntries > 0
                    ? prepaySnapshotLog->entries[getLastIndex(prepaySnapshotLog)].time
                    : 0);

  return (*lastEntryTime == 0 ? true : lastExpectedEntryTime > *lastEntryTime);
}

//------------------------------------------------------------------------------
// API Functions
void sl_zigbee_af_gas_proxy_function_init_structured_data(void)
{
  uint8_t i;

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS; i++) {
    memset(structuredData[i].deviceIeeeAddress, 0, EUI64_SIZE);
    structuredData[i].endpoint = GPF_UNUSED_ENDPOINT_ID;

    // Initialize the constant data within each log.  The transient data is
    // initialize when the mirror is added.

    structuredData[i].dailyConsumptionLog.maxEntries = SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_DAILY_CONSUMPTION_LOG_ENTRIES;
    structuredData[i].dailyConsumptionLog.entries = dailyConsumptionLogEntries;
    structuredData[i].dailyConsumptionLog.sampleId = GPF_DAILY_CONSUMPTION_LOG_SAMPLE_ID;
    // GBCS v0.8.1 section 10.4.2.11, "The SampleRequestInterval field shall
    // contain 0xFFFF whenever the SampleID field is 0x0001."
    structuredData[i].dailyConsumptionLog.sampleInterval = 0xFFFF;

    structuredData[i].profileDataLog.maxEntries = SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_PROFILE_DATA_LOG_ENTRIES;
    structuredData[i].profileDataLog.entries = profileDataLogEntries;
    structuredData[i].profileDataLog.sampleId = GPF_PROFILE_DATA_LOG_SAMPLE_ID;
    structuredData[i].profileDataLog.sampleInterval = 1800;

    structuredData[i].dailyReadLog.maxEntries = SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_DAILY_READ_LOG_ENTRIES;
    structuredData[i].dailyReadLog.entries = dailyReadLogEntries;
    structuredData[i].dailyReadLog.catchupSnapshotCause = GPF_SNAPSHOT_CAUSE_GENERAL;

    structuredData[i].prepayDailyReadLog.maxEntries = SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_PREPAYMENT_DAILY_READ_LOG_ENTRIES;
    structuredData[i].prepayDailyReadLog.entries = prepayDailyReadLogEntries;
    structuredData[i].prepayDailyReadLog.catchupSnapshotCause = GPF_SNAPSHOT_CAUSE_GENERAL;

    structuredData[i].billingDataLog.snapshot.maxEntries = SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_BILLING_DATA_LOG_SNAPSHOT_ENTRIES;
    structuredData[i].billingDataLog.snapshot.entries = billingDataLogSnapshotEntries;
    structuredData[i].billingDataLog.snapshot.catchupSnapshotCause = (GPF_SNAPSHOT_CAUSE_END_OF_BILLING_PERIOD
                                                                      | GPF_SNAPSHOT_CAUSE_CHANGE_OF_TARIFF
                                                                      | GPF_SNAPSHOT_CAUSE_CHANGE_OF_SUPPLIER
                                                                      | GPF_SNAPSHOT_CAUSE_CHANGE_OF_PAYMENT_MODE);

    structuredData[i].billingDataLog.topUp.maxEntries = SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_BILLING_DATA_LOG_TOP_UP_ENTRIES;
    structuredData[i].billingDataLog.topUp.entries = billingDataLogTopUpEntries;

    structuredData[i].billingDataLog.debt.maxEntries = SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_BILLING_DATA_LOG_DEBT_ENTRIES;
    structuredData[i].billingDataLog.debt.entries = billingDataLogDebtEntries;

    structuredData[i].billingDataLog.prepaySnapshot.maxEntries = SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MAX_BILLING_DATA_LOG_PREPAY_SNAPSHOT_ENTRIES;
    structuredData[i].billingDataLog.prepaySnapshot.entries = billingDataLogPrepaySnapshotEntries;
    structuredData[i].billingDataLog.prepaySnapshot.catchupSnapshotCause = (GPF_SNAPSHOT_CAUSE_END_OF_BILLING_PERIOD
                                                                            | GPF_SNAPSHOT_CAUSE_CHANGE_OF_TARIFF
                                                                            | GPF_SNAPSHOT_CAUSE_CHANGE_OF_SUPPLIER
                                                                            | GPF_SNAPSHOT_CAUSE_CHANGE_OF_PAYMENT_MODE);
  }
}

//------------------------------------------------------------------------------
// Callback  Functions

/** @brief Mirror Added
 *
 * This function is called by the Meter Mirror plugin whenever a RequestMirror
 * command is successfully processed.
 *
 * @param requestingDeviceIeeeAddress   Ver.: always
 * @param endpoint   Ver.: always
 */
void sl_zigbee_af_meter_mirror_mirror_added_cb(const sl_802154_long_addr_t requestingDeviceIeeeAddress,
                                               uint8_t endpoint)
{
  uint8_t i;

  // If we already have a structured data entry for the given device just keep
  // using it but make sure the endpoint is updated as it may have changed
  i = findStructuredDataByEUI64(requestingDeviceIeeeAddress);
  if (i != GPF_INVALID_LOG_INDEX) {
    structuredData[i].endpoint = endpoint;
    return;
  }

  // Find an available slot in the array of structured data logs and assign it to
  // this endpoint;
  i = findStructuredData(GPF_UNUSED_ENDPOINT_ID);
  if (i == GPF_INVALID_LOG_INDEX) {
    sl_zigbee_af_gas_proxy_function_println("GPF: ERR: Mirror Added - Structured data not available on endpoint 0x%02X", endpoint);
    return;
  }

  memcpy(structuredData[i].deviceIeeeAddress, requestingDeviceIeeeAddress, EUI64_SIZE);
  structuredData[i].endpoint = endpoint;

  // Reset the transient data within each log and kick off the catchup functionality
  startSampleLogCatchup(endpoint, &structuredData[i].dailyConsumptionLog, true);        // reset?
  startSampleLogCatchup(endpoint, &structuredData[i].profileDataLog, true);             // reset?
  startSnapshotLogCatchup(endpoint, &structuredData[i].dailyReadLog, true);             // reset?
  startPrepaySnapshotLogCatchup(endpoint, &structuredData[i].prepayDailyReadLog, true); // reset?

  startSnapshotLogCatchup(endpoint, &structuredData[i].billingDataLog.snapshot, true);  // reset?
  startTopUpLogCatchup(endpoint, &structuredData[i].billingDataLog.topUp);
  startDebtLogCatchup(endpoint, &structuredData[i].billingDataLog.debt);
  startPrepaySnapshotLogCatchup(endpoint, &structuredData[i].billingDataLog.prepaySnapshot, true); // reset?

  resetAlternativeHistoricalConsumption(endpoint, &structuredData[i].alternativeHistoricalConsumption);
  resetHistoricalCostConsumptionInformation(endpoint, &structuredData[i].historicalCostConsumption);

  structuredData[i].remoteEndpoint = GPF_UNUSED_ENDPOINT_ID;
  structuredData[i].remoteNodeId = SL_ZIGBEE_UNKNOWN_NODE_ID;
  structuredData[i].functionalNotificationFlags = 0;
  structuredData[i].notificationFlags4 = 0;
  structuredData[i].lastAttributeReportTime = 0;

  sl_zigbee_af_gas_proxy_function_println("GPF: Structured Data initialized on endpoint 0x%02X", endpoint);

  // Set an event to make sure we keep our logs up to date with
  // our new GSME buddy at least every half hour (see 10.4.2.8 in v0.8.1).
  sl_zigbee_af_endpoint_event_set_delay_ms(endpointEvent,
                                           endpoint,
                                           SECONDS_IN_HALF_HOUR * MILLISECOND_TICKS_PER_SECOND);
}

/** @brief Mirror Removed
 *
 * This function is called by the Meter Mirror plugin whenever a RemoveMirror
 * command is successfully processed.
 *
 * @param requestingDeviceIeeeAddress   Ver.: always
 * @param endpoint   Ver.: always
 */
void sl_zigbee_af_meter_mirror_mirror_removed_cb(const sl_802154_long_addr_t requestingDeviceIeeeAddress,
                                                 uint8_t endpoint)
{
  UNUSED_VAR(requestingDeviceIeeeAddress);

  uint8_t i;

  // Find the structured data for the given endpoint
  i = findStructuredData(endpoint);
  if (i == GPF_INVALID_LOG_INDEX) {
    sl_zigbee_af_gas_proxy_function_println("GPF: ERR: Mirror Removed - Structured data not available on endpoint 0x%02X", endpoint);
    return;
  }

  // mark the structured data log unused
  structuredData[i].endpoint = GPF_UNUSED_ENDPOINT_ID;
  memset(structuredData[i].deviceIeeeAddress, 0, EUI64_SIZE);
  sl_zigbee_af_gas_proxy_function_println("GPF: Structured Data removed on endpoint 0x%02X", endpoint);

  // We don't care about this GSME checking in anymore.
  sl_zigbee_af_endpoint_event_set_inactive(endpointEvent,
                                           endpoint);
}

/** @brief Reporting Complete
 *
 * This function is called by the Meter Mirror plugin after processing an
 * AttributeReportingStatus attribute set to ReportingComplete. If the
 * application needs to do any post attribute reporting processing it can do it
 * from within this callback.
 *
 * @param endpoint   Ver.: always
 *
 */
void sl_zigbee_af_meter_mirror_reporting_complete_cb(uint8_t endpoint)
{
  uint8_t i;
  sl_zigbee_af_status_t status;
  uint8_t currentSummationDelivered[] = { 0, 0, 0, 0, 0, 0 };
  uint8_t currentDayAlternativeConsumptionDelivered[] = { 0, 0, 0 };
  uint8_t currentDayCostConsumptionDelivered[] = { 0, 0, 0, 0, 0, 0 };
  // currentDayAlternativeConsumption is a 32-bit variable that contains a 24-bit value.
  // Initialize it to ensure its uppermost 8 bits will be zero after the 24-bit value
  // is memcpyed into it. Thereafter it is used as a 32-bit value in a computation; an
  // uninitialized 4th byte corrupts the result.
  // currentSummation and currentDayCostConsumption represent full 32-bit values; but since
  // they are also populated via memcpy, zero them as well for peace of mind.
  uint32_t currentSummation = 0;
  uint32_t currentDayAlternativeConsumption = 0;
  uint32_t currentDayCostConsumption = 0;
  uint32_t now = sl_zigbee_af_get_current_time();

  // Find the structured data for the given endpoint
  i = findStructuredData(endpoint);
  if (i == GPF_INVALID_LOG_INDEX) {
    sl_zigbee_af_gas_proxy_function_println("GPF: ERR: Attribute Reporting Complete - Structured data not available on endpoint 0x%02X", endpoint);
    return;
  }

  status = sl_zigbee_af_read_server_attribute(endpoint,
                                              ZCL_SIMPLE_METERING_CLUSTER_ID,
                                              ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID,
                                              currentSummationDelivered,
                                              6);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_gas_proxy_function_println("GPF: ERR: can't read CurrentSummationDelivered attribute: status 0x%02X", status);
  }

  status = sl_zigbee_af_read_server_attribute(endpoint,
                                              ZCL_SIMPLE_METERING_CLUSTER_ID,
                                              ZCL_CURRENT_ALTERNATIVE_DAY_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
                                              currentDayAlternativeConsumptionDelivered,
                                              3);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_gas_proxy_function_println("GPF: ERR: can't read CurrentDayAlternativeConsumptionDelivered attribute: status 0x%02X", status);
  }

  status = sl_zigbee_af_read_server_attribute(endpoint,
                                              ZCL_PREPAYMENT_CLUSTER_ID,
                                              ZCL_CURRENT_DAY_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID,
                                              currentDayCostConsumptionDelivered,
                                              6);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_gas_proxy_function_println("GPF: ERR: can't read CurrentDayCostConsumptionDelivered attribute: status 0x%02X", status);
  }

  // We only care about the least significant 32 bits as the the various delivered
  // amounts should not change by more than a 32 bit value between attribute reports.
#if (BIGENDIAN_CPU)
  memcpy((uint8_t *)&currentSummation, &currentSummationDelivered[2], 4);
  memcpy(&((uint8_t *)&currentDayAlternativeConsumption)[1], &currentDayAlternativeConsumptionDelivered[0], 3);
  memcpy((uint8_t *)&currentDayCostConsumption, &currentDayCostConsumptionDelivered[2], 4);
#else
  memcpy((uint8_t *)&currentSummation, &currentSummationDelivered[0], 4);
  memcpy(&((uint8_t *)&currentDayAlternativeConsumption)[0], &currentDayAlternativeConsumptionDelivered[0], 3);
  memcpy((uint8_t *)&currentDayCostConsumption, &currentDayCostConsumptionDelivered[0], 4);
#endif

  // If we are "catching up" then don't process the reported attributes.  We wait
  // until we are completely caught up before we start processing these reports again.
  if (!structuredData[i].profileDataLog.catchup) {
    sli_zigbee_gpf_sample_log_t *sampleLog = &structuredData[i].profileDataLog;
    if (sampleLog->startTime != 0) {
      uint32_t prevTime = (0 == sampleLog->numberOfEntries) ? sampleLog->startTime
                          : sampleLog->entries[getLastIndex(sampleLog)].time;
      uint32_t nextTime = NEXT_HALF_HOUR(prevTime);
      if (nextTime <= now) {
        uint32_t sample = currentSummation - sampleLog->prevSummation;
        receiveSampleData(now, sample, sampleLog);
        sampleLog->prevSummation = currentSummation;
      }
    } else {
      sampleLog->startTime = now;
      sampleLog->prevSummation = currentSummation;
    }
  }

  if (!structuredData[i].dailyConsumptionLog.catchup) {
    sli_zigbee_gpf_sample_log_t *sampleLog = &structuredData[i].dailyConsumptionLog;
    if (sampleLog->startTime != 0) {
      uint32_t prevTime = (0 == sampleLog->numberOfEntries) ? sampleLog->startTime
                          : sampleLog->entries[getLastIndex(sampleLog)].time;
      uint32_t nextTime = NEXT_MIDNIGHT(prevTime);
      if (nextTime <= now) {
        uint32_t sample = currentSummation - sampleLog->prevSummation;
        receiveSampleData(now, sample, sampleLog);
        sampleLog->prevSummation = currentSummation;
      }
    } else {
      sampleLog->startTime = now;
      sampleLog->prevSummation = currentSummation;
    }
  }

  updateAlternativeHistoricalConsumption(endpoint,
                                         now,
                                         currentDayAlternativeConsumption,
                                         &structuredData[i].alternativeHistoricalConsumption);

  updateHistoricalCostConsumptionInformation(endpoint,
                                             now,
                                             currentDayCostConsumption,
                                             &structuredData[i].historicalCostConsumption);

  structuredData[i].lastAttributeReportTime = now;
}

/** @brief Simple Metering Cluster Get Sampled Data Response
 *
 * @param sampleId   Ver.: always
 * @param sampleStartTime   Ver.: always
 * @param sampleType   Ver.: always
 * @param sampleRequestInterval   Ver.: always
 * @param numberOfSamples   Ver.: always
 * @param samples   Ver.: always
 */
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_get_sampled_data_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_simple_metering_cluster_get_sampled_data_response_command_t cmd_data;

  if (zcl_decode_simple_metering_cluster_get_sampled_data_response_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  uint16_t sampleId = cmd_data.sampleId;
  uint32_t sampleStartTime = cmd_data.sampleStartTime;
  uint8_t sampleType = cmd_data.sampleType;
  uint16_t sampleRequestInterval = cmd_data.sampleRequestInterval;
  uint16_t numberOfSamples = cmd_data.numberOfSamples;
  uint8_t* samples = cmd_data.samples;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t i = findStructuredData(endpoint);
  sli_zigbee_gpf_sample_log_t *sampleLog;
  sli_zigbee_gpf_sample_log_t *otherSampleLog;
  uint16_t samplesLength = fieldLength(samples);
  uint16_t samplesIndex = 0;
  uint32_t sample;
  uint32_t sampleTime = sampleStartTime;
  sl_zigbee_af_status_t status;
  uint8_t currentSummationDelivered[] = { 0, 0, 0, 0, 0, 0 };
  uint32_t currentSummation;

  sl_zigbee_af_gas_proxy_function_println("GPF: GetSampledDataResponse 0x%04X 0x%08X 0x%02X 0x%04X 0x%04X 0x%04X",
                                          sampleId,
                                          sampleStartTime,
                                          sampleType,
                                          sampleRequestInterval,
                                          numberOfSamples,
                                          samplesLength);

  if (i == GPF_INVALID_LOG_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (sampleType != SL_ZIGBEE_ZCL_SAMPLE_TYPE_CONSUMPTION_DELIVERED) {
    sl_zigbee_af_gas_proxy_function_println("GPF: WARN: GetSampledDataResponse command received with invalid sampleType: 0x%02X", sampleType);
    return SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  }

  if (sampleId == GPF_DAILY_CONSUMPTION_LOG_SAMPLE_ID) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Receive Daily Consumption Log");
    sampleLog = &structuredData[i].dailyConsumptionLog;
    otherSampleLog = &structuredData[i].profileDataLog;
  } else if (sampleId == GPF_PROFILE_DATA_LOG_SAMPLE_ID) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Receive Profile Data Log");
    sampleLog = &structuredData[i].profileDataLog;
    otherSampleLog = &structuredData[i].dailyConsumptionLog;
  } else {
    sl_zigbee_af_gas_proxy_function_println("GPF: WARN: GetSampledDataResponse command received with invalid sampleId: 0x%04X", sampleId);
    return SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  }

  // The only times we should receive a GetSampledDataResponse are
  // the following:
  //   1) After a node restart where we send a GetSampledData request to obtain
  //      any data that may have been missed while this node was out of service.
  //   2) After we know that we are missing GSME Profile Data Log entries, and
  //      we send a GetSampledData to retrieve those entries.
  // These are both examples of a "catchup."
  // As such we will ignore any commands that we were not expecting.
  if (!sampleLog->catchup || sl_zigbee_af_current_command()->seqNum != sampleLog->catchupSequenceNumber) {
    sl_zigbee_af_gas_proxy_function_println("GPF: WARN: ignoring unexpected GetSampledDataResponse command");
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  // Don't overwrite our start time if we have already been receiving samples
  // from the GSME.
  if (sampleLog->numberOfEntries == 0) {
    sampleLog->startTime = sampleTime;
  }

  sampleLog->sampleInterval = sampleRequestInterval;
  while (samplesIndex < samplesLength) {
    sample = sl_zigbee_af_get_int24u(samples, samplesIndex, samplesLength);
    samplesIndex += 3;
    sampleTime = ((sampleId == GPF_PROFILE_DATA_LOG_SAMPLE_ID)
                  ? NEXT_HALF_HOUR(sampleTime) : NEXT_MIDNIGHT(sampleTime));
    receiveSampleData(sampleTime, sample, sampleLog);
  }

  // now that we are caught up we need to set the prev summation value so that
  // the next time the device reports consumption we can calculate the sample
  // correctly.  For the profile log it is easy, we just use the current
  // summation attribute which represents the last time it was reported. For
  // the daily consumption log it is a little more difficult. To set the prev
  // summation we need to start with the current summation then using the profile
  // log subtract the incremental values back to the beginning of the day.
  status = sl_zigbee_af_read_server_attribute(endpoint,
                                              ZCL_SIMPLE_METERING_CLUSTER_ID,
                                              ZCL_CURRENT_SUMMATION_DELIVERED_ATTRIBUTE_ID,
                                              currentSummationDelivered,
                                              6);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_gas_proxy_function_println("GPF: ERR: can't read CurrentSummationDelivered attribute: status 0x%02X", status);
  }
  // We only care about the least significant 32 bits as the the summation delivered
  // should not change by more than a 32 bit value between attribute reports.
#if (BIGENDIAN_CPU)
  memcpy((uint8_t *)&currentSummation, &currentSummationDelivered[2], 4);
#else
  memcpy((uint8_t *)&currentSummation, &currentSummationDelivered[0], 4);
#endif
  if (sampleId == GPF_PROFILE_DATA_LOG_SAMPLE_ID) {
    sampleLog->prevSummation = currentSummation;
    // in this case sampleLog is the profile data log and otherSamleLog is
    // the daily consumption log
    setDailyConsumptionLogPrevSummation(sampleLog, otherSampleLog);
  } else if (!otherSampleLog->catchup) {
    // in this case sampleLog is the daily consumption log and otherSampleLog is
    // the profile data log.
    setDailyConsumptionLogPrevSummation(otherSampleLog, sampleLog);
  }

  stopSampleLogCatchup(endpoint, sampleLog, otherSampleLog);

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

/** @brief Simple Metering Cluster Get Sampled Data
 *
 * @param sampleId   Ver.: always
 * @param earliestSampleTime   Ver.: always
 * @param sampleType   Ver.: always
 * @param numberOfSamples   Ver.: always
 */
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_get_sampled_data_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_simple_metering_cluster_get_sampled_data_command_t cmd_data;

  if (zcl_decode_simple_metering_cluster_get_sampled_data_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t i = findStructuredData(endpoint);
  sli_zigbee_gpf_sample_log_t *sampleLog;

  sl_zigbee_af_gas_proxy_function_println("GPF: GetSampledData 0x%04X 0x%08X 0x%02X 0x%04X",
                                          cmd_data.sampleId,
                                          cmd_data.earliestSampleTime,
                                          cmd_data.sampleType,
                                          cmd_data.numberOfSamples);

  if (i == GPF_INVALID_LOG_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (cmd_data.sampleType != SL_ZIGBEE_ZCL_SAMPLE_TYPE_CONSUMPTION_DELIVERED) {
    sl_zigbee_af_gas_proxy_function_println("GPF: WARN: GetSampledData command received with invalid sampleType: 0x%02X", cmd_data.sampleType);
    return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  }

  if (!sl_zigbee_af_gas_proxy_function_data_log_access_request_cb(sl_zigbee_af_gas_proxy_function_get_current_message(),
                                                                  sl_zigbee_af_current_command())) {
    return SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED;
  }

  if (cmd_data.sampleId == GPF_DAILY_CONSUMPTION_LOG_SAMPLE_ID) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Publish Daily Consumption Log");
    sampleLog = &structuredData[i].dailyConsumptionLog;
  } else if (cmd_data.sampleId == GPF_PROFILE_DATA_LOG_SAMPLE_ID) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Publish Profile Data Log");
    sampleLog = &structuredData[i].profileDataLog;
  } else {
    sl_zigbee_af_gas_proxy_function_println("GPF: WARN: GetSampledData command received with invalid sampleId: 0x%04X", cmd_data.sampleId);
    return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  }

  sendSampleData(cmd_data.earliestSampleTime, cmd_data.numberOfSamples, sampleLog);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

/** @brief Simple Metering Cluster Publish Snapshot
 *
 * @param snapshotId   Ver.: always
 * @param snapshotTime   Ver.: always
 * @param totalSnapshotsFound   Ver.: always
 * @param commandIndex   Ver.: always
 * @param totalCommands   Ver.: always
 * @param snapshotCause   Ver.: always
 * @param snapshotPayloadType   Ver.: always
 * @param snapshotPayload   Ver.: always
 */
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_publish_snapshot_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_simple_metering_cluster_publish_snapshot_command_t cmd_data;

  if (zcl_decode_simple_metering_cluster_publish_snapshot_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t i = findStructuredData(endpoint);
  sli_zigbee_gpf_snapshot_log_t *snapshotLog;
  sli_zigbee_gpf_snapshot_log_t *otherSnapshotLog;
  uint32_t now = sl_zigbee_af_get_current_time();

  sl_zigbee_af_gas_proxy_function_println("GPF: PublishSnapshot 0x%08X 0x%08X 0x%02X 0x%02X 0x%02X 0x%08X 0x%02X",
                                          cmd_data.snapshotId,
                                          cmd_data.snapshotTime,
                                          cmd_data.totalSnapshotsFound,
                                          cmd_data.commandIndex,
                                          cmd_data.totalCommands,
                                          cmd_data.snapshotCause,
                                          cmd_data.snapshotPayloadType);

  if (i == GPF_INVALID_LOG_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  // Both the Daily Read Log and Billing Data Log PublishSnapshot commands use
  // the same snapshotPayloadTypes (referenced below). The types were obtained
  // by looking at the response in the use case description for GCS16a then
  // also looking at the description of the billing data log in GBCS v0.8.1
  // section 10.4.2.4 and comparing it to the description of the daily read log
  // in the SMETS v1.58 section 4.4.94.
  // GBCS IRP328 wants to add support for SnapshotPayloadType 4 and 6 i.e
  // SL_ZIGBEE_ZCL_SNAPSHOT_PAYLOAD_TYPE_TOU_INFORMATION_SET_DELIVERED_REGISTERS_NO_BILLING
  // and SL_ZIGBEE_ZCL_SNAPSHOT_PAYLOAD_TYPE_BLOCK_TIER_INFORMATION_SET_DELIVERED_NO_BILLING respectively.
  if (cmd_data.snapshotPayloadType != SL_ZIGBEE_ZCL_SNAPSHOT_PAYLOAD_TYPE_TOU_INFORMATION_SET_DELIVERED_REGISTERS
      && cmd_data.snapshotPayloadType != SL_ZIGBEE_ZCL_SNAPSHOT_PAYLOAD_TYPE_BLOCK_TIER_INFORMATION_SET_DELIVERED
      && cmd_data.snapshotPayloadType != SL_ZIGBEE_ZCL_SNAPSHOT_PAYLOAD_TYPE_TOU_INFORMATION_SET_DELIVERED_REGISTERS_NO_BILLING
      && cmd_data.snapshotPayloadType != SL_ZIGBEE_ZCL_SNAPSHOT_PAYLOAD_TYPE_BLOCK_TIER_INFORMATION_SET_DELIVERED_NO_BILLING) {
    sl_zigbee_af_gas_proxy_function_println("GPF: WARN: PublishSnapshot command received with unsupported payloadType: 0x%02X", cmd_data.snapshotPayloadType);
    return SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  }

  if (cmd_data.snapshotCause == GPF_SNAPSHOT_CAUSE_GENERAL) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Receive Daily Read Log");
    snapshotLog = &structuredData[i].dailyReadLog;
    otherSnapshotLog = &structuredData[i].billingDataLog.snapshot;
  } else if (cmd_data.snapshotCause
             & (GPF_SNAPSHOT_CAUSE_END_OF_BILLING_PERIOD
                | GPF_SNAPSHOT_CAUSE_CHANGE_OF_TARIFF
                | GPF_SNAPSHOT_CAUSE_CHANGE_OF_SUPPLIER
                | GPF_SNAPSHOT_CAUSE_CHANGE_OF_PAYMENT_MODE)) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Receive Billing Data Log - Tariff TOU Register Matrix, the Consumption Register and Tariff Block Counter Matrix");
    snapshotLog = &structuredData[i].billingDataLog.snapshot;
    otherSnapshotLog = &structuredData[i].dailyReadLog;
  } else {
    sl_zigbee_af_gas_proxy_function_println("GPF: WARN: PublishSnapshot command received with unsupported cause: 0x%08X", cmd_data.snapshotCause);
    return SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  }

  // Ignore any commands that we were not expecting.
  if (snapshotLog->catchup && sl_zigbee_af_current_command()->seqNum != snapshotLog->catchupSequenceNumber) {
    sl_zigbee_af_gas_proxy_function_println("GPF: WARN: ignoring unexpected PublishSnapshot command");
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  receiveSnapshot(cmd_data.snapshotId, cmd_data.snapshotTime, cmd_data.snapshotCause, cmd_data.snapshotPayloadType, cmd_data.snapshotPayload, snapshotLog);

  if (snapshotLog->catchup) {
    snapshotLog->catchupSnapshotOffset++;
    if (snapshotLog->catchupSnapshotOffset >= cmd_data.totalSnapshotsFound) {
      stopSnapshotLogCatchup(endpoint, snapshotLog, otherSnapshotLog);
    } else {
      sl_zigbee_af_cluster_command_t *cmd_current = sl_zigbee_af_current_command();
      getSnapshot(endpoint, cmd_current->apsFrame->sourceEndpoint, cmd_current->source, snapshotLog);
    }
  } else {
    if (cmd_data.snapshotCause == GPF_SNAPSHOT_CAUSE_GENERAL) {
      /*
       * GBCS v0.8.1 Section 10.4.2.1
       *
       * The GPF shall populate the relevant attributes upon receipt of the
       * PublishSnapshot command, providing the command is received between
       * midnight (UTC) and the next scheduled wake of the GSME.
       */
      if (cmd_data.snapshotTime >= PREV_MIDNIGHT(now)
          && structuredData[i].lastAttributeReportTime < PREV_MIDNIGHT(now)) {
        updateSnapshotAttributes(endpoint, cmd_data.snapshotPayloadType, cmd_data.snapshotPayload);
      }
    } else {
      /*
       * CHTS v1.46 Section 4.5.2
       *
       * Where changes have been made to the GSME Billing Data Log in accordance
       * with the timetable set-out in the GSME Billing Calendar, the GPF shall be
       * capable of generating and sending an Alert containing the most recent
       * entries of the GSME Tariff TOU Register Matrix, the GSME Tariff Block
       * Counter Matrix and the GSME Consumption Register in the GSME Billing Data
       * Log.
       */
      sli_zigbee_af_gas_proxy_function_alert(GBCS_ALERT_BILLING_DATA_LOG_UPDATED,
                                             sl_zigbee_af_current_command(),
                                             GCS53_MESSAGE_CODE);
    }
  }

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

/** @brief Simple Metering Cluster Get Snapshot
 *
 * @param earliestStartTime   Ver.: always
 * @param latestEndTime   Ver.: always
 * @param snapshotOffset   Ver.: always
 * @param snapshotCause   Ver.: always
 */
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_get_snapshot_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_simple_metering_cluster_get_snapshot_command_t cmd_data;

  if (zcl_decode_simple_metering_cluster_get_snapshot_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t i = findStructuredData(endpoint);
  sli_zigbee_gpf_snapshot_log_t *snapshotLog;

  sl_zigbee_af_gas_proxy_function_println("GPF: GetSnapshot 0x%08X 0x%08X 0x%02X 0x%08X",
                                          cmd_data.earliestStartTime,
                                          cmd_data.latestEndTime,
                                          cmd_data.snapshotOffset,
                                          cmd_data.snapshotCause);

  if (i == GPF_INVALID_LOG_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (cmd_data.snapshotCause == GPF_SNAPSHOT_CAUSE_GENERAL) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Publish Daily Read Log");
    snapshotLog = &structuredData[i].dailyReadLog;
    if (!sl_zigbee_af_gas_proxy_function_data_log_access_request_cb(sl_zigbee_af_gas_proxy_function_get_current_message(),
                                                                    sl_zigbee_af_current_command())) {
      return SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED;
    }
  } else if (cmd_data.snapshotCause
             & (GPF_SNAPSHOT_CAUSE_END_OF_BILLING_PERIOD
                | GPF_SNAPSHOT_CAUSE_CHANGE_OF_TARIFF
                | GPF_SNAPSHOT_CAUSE_CHANGE_OF_SUPPLIER
                | GPF_SNAPSHOT_CAUSE_CHANGE_OF_PAYMENT_MODE)) {
    if (!sl_zigbee_af_gas_proxy_function_data_log_access_request_cb(sl_zigbee_af_gas_proxy_function_get_current_message(),
                                                                    sl_zigbee_af_current_command())) {
      return SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED;
    }

    sl_zigbee_af_gas_proxy_function_println("GPF: Publish Billing Data Log - Tariff TOU Register Matrix, the Consumption Register and Tariff Block Counter Matrix");
    snapshotLog = &structuredData[i].billingDataLog.snapshot;
  } else {
    sl_zigbee_af_gas_proxy_function_println("GPF: WARN: GetSnapshot command received with unsupported cause: 0x%08X", cmd_data.snapshotCause);
    return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  }

  sendSnapshot(cmd_data.earliestStartTime, cmd_data.latestEndTime, cmd_data.snapshotOffset, cmd_data.snapshotCause, snapshotLog);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

/** @brief Prepayment Cluster Publish Prepay Snapshot
 *
 * @param snapshotId   Ver.: always
 * @param snapshotTime   Ver.: always
 * @param totalSnapshotsFound   Ver.: always
 * @param commandIndex   Ver.: always
 * @param totalNumberOfCommands   Ver.: always
 * @param snapshotCause   Ver.: always
 * @param snapshotPayloadType   Ver.: always
 * @param snapshotPayload   Ver.: always
 */
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_publish_prepay_snapshot_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_prepayment_cluster_publish_prepay_snapshot_command_t cmd_data;

  if (zcl_decode_prepayment_cluster_publish_prepay_snapshot_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  uint32_t snapshotId = cmd_data.snapshotId;
  uint32_t snapshotTime = cmd_data.snapshotTime;
  uint8_t totalSnapshotsFound = cmd_data.totalSnapshotsFound;
  uint8_t commandIndex = cmd_data.commandIndex;
  uint8_t totalNumberOfCommands  = cmd_data.totalNumberOfCommands;
  uint32_t snapshotCause = cmd_data.snapshotCause;
  uint8_t snapshotPayloadType = cmd_data.snapshotPayloadType;
  uint8_t* snapshotPayload = cmd_data.snapshotPayload;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t i = findStructuredData(endpoint);
  sli_zigbee_gpf_prepay_snapshot_log_t *prepaySnapshotLog;
  sli_zigbee_gpf_prepay_snapshot_log_t *otherPrepaySnapshotLog;
  uint32_t now = sl_zigbee_af_get_current_time();

  sl_zigbee_af_gas_proxy_function_println("GPF: RX: PublishPrepaySnapshot, 0x%08X, 0x%08X, 0x%02X, 0x%02X, 0x%02X, 0x%08X, 0x%02X",
                                          snapshotId,
                                          snapshotTime,
                                          totalSnapshotsFound,
                                          commandIndex,
                                          totalNumberOfCommands,
                                          snapshotCause,
                                          snapshotPayloadType);

  if (i == GPF_INVALID_LOG_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (snapshotPayloadType != SL_ZIGBEE_ZCL_PREPAY_SNAPSHOT_PAYLOAD_TYPE_DEBT_CREDIT_STATUS) {
    sl_zigbee_af_gas_proxy_function_println("GPF: WARN: PublishPrepaySnapshot command received with unsupported payloadType: 0x%02X", snapshotPayloadType);
    return SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  }

  if (snapshotCause == GPF_SNAPSHOT_CAUSE_GENERAL) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Receive Prepay Daily Read Log");
    if (!sl_zigbee_af_gas_proxy_function_data_log_access_request_cb(sl_zigbee_af_gas_proxy_function_get_current_message(),
                                                                    sl_zigbee_af_current_command())) {
      return SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED;
    }

    prepaySnapshotLog = &structuredData[i].prepayDailyReadLog;
    otherPrepaySnapshotLog = &structuredData[i].billingDataLog.prepaySnapshot;
  } else if (snapshotCause
             & (GPF_SNAPSHOT_CAUSE_END_OF_BILLING_PERIOD
                | GPF_SNAPSHOT_CAUSE_CHANGE_OF_TARIFF
                | GPF_SNAPSHOT_CAUSE_CHANGE_OF_SUPPLIER
                | GPF_SNAPSHOT_CAUSE_CHANGE_OF_PAYMENT_MODE)) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Receive Billing Data Log - Meter Balance, Emergency Credit Balance, Accumulated Debt Register, Payment Debt Register and Time Debt Registers");
    prepaySnapshotLog = &structuredData[i].billingDataLog.prepaySnapshot;
    otherPrepaySnapshotLog = &structuredData[i].prepayDailyReadLog;
  } else {
    sl_zigbee_af_gas_proxy_function_println("GPF: WARN: PublishPrepaySnapshot command received with unsupported cause: 0x%08X", snapshotCause);
    return SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
  }

  // Ignore any commands that we were not expecting.
  if (prepaySnapshotLog->catchup && sl_zigbee_af_current_command()->seqNum != prepaySnapshotLog->catchupSequenceNumber) {
    sl_zigbee_af_gas_proxy_function_println("GPF: WARN: ignoring unexpected PublishPrepaySnapshot command");
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  receivePrepaySnapshot(snapshotId, snapshotTime, snapshotCause, snapshotPayloadType, snapshotPayload, prepaySnapshotLog);

  if (prepaySnapshotLog->catchup) {
    prepaySnapshotLog->catchupSnapshotOffset++;
    if (prepaySnapshotLog->catchupSnapshotOffset >= totalSnapshotsFound) {
      stopPrepaySnapshotLogCatchup(endpoint, prepaySnapshotLog, otherPrepaySnapshotLog);
    } else {
      sl_zigbee_af_cluster_command_t *cmd = sl_zigbee_af_current_command();
      getPrepaySnapshot(endpoint, cmd->apsFrame->sourceEndpoint, cmd->source, prepaySnapshotLog);
    }
  } else {
    if (snapshotCause == GPF_SNAPSHOT_CAUSE_GENERAL) {
      /*
       * GBCS v0.8.1 Section 10.4.2.2
       *
       * The GPF shall populate the relevant attributes upon receipt of the
       * Publish Prepay Snapshot command, providing the command is received
       * between midnight (UTC) and the next scheduled wake of the GSME.
       */
      if (snapshotTime >= PREV_MIDNIGHT(now)
          && structuredData[i].lastAttributeReportTime < PREV_MIDNIGHT(now)) {
        updatePrepaySnapshotAttributes(endpoint, snapshotPayloadType, snapshotPayload);
      }
    } else {
      /*
       * GBCS IRP328: added missing element in the usecase GCS53.
       * With this change, the GPF shall be capable of generating and sending an alert
       * containing the most entries of GSME Meter Balance, Emergency Credit Balance,
       * Accumulated Debt Register, Payment Debt Register and Time Debt Registers [1 ... 2]
       */
      sli_zigbee_af_gas_proxy_function_alert(GBCS_ALERT_BILLING_DATA_LOG_UPDATED,
                                             sl_zigbee_af_current_command(),
                                             GCS53_MESSAGE_CODE);
    }
  }

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

/** @brief Prepayment Cluster Get Prepay Snapshot
 *
 * @param earliestStartTime   Ver.: always
 * @param latestEndTime   Ver.: always
 * @param snapshotOffset   Ver.: always
 * @param snapshotCause   Ver.: always
 */
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_get_prepay_snapshot_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_prepayment_cluster_get_prepay_snapshot_command_t cmd_data;

  if (zcl_decode_prepayment_cluster_get_prepay_snapshot_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t i = findStructuredData(endpoint);
  sli_zigbee_gpf_prepay_snapshot_log_t *prepaySnapshotLog;

  sl_zigbee_af_gas_proxy_function_println("GPF: RX: GetPrepaySnapshot 0x%08X 0x%08X 0x%02X 0x%08X",
                                          cmd_data.earliestStartTime,
                                          cmd_data.latestEndTime,
                                          cmd_data.snapshotOffset,
                                          cmd_data.snapshotCause);

  if (i == GPF_INVALID_LOG_INDEX) {
    return false;
  }

  if (cmd_data.snapshotCause == GPF_SNAPSHOT_CAUSE_GENERAL) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Publish Prepay Daily Read Log");
    prepaySnapshotLog = &structuredData[i].prepayDailyReadLog;
  } else if (cmd_data.snapshotCause
             & (GPF_SNAPSHOT_CAUSE_END_OF_BILLING_PERIOD
                | GPF_SNAPSHOT_CAUSE_CHANGE_OF_TARIFF
                | GPF_SNAPSHOT_CAUSE_CHANGE_OF_SUPPLIER
                | GPF_SNAPSHOT_CAUSE_CHANGE_OF_PAYMENT_MODE)) {
    sl_zigbee_af_gas_proxy_function_println("GPF: Publish Billing Data Log - Meter Balance, Emergency Credit Balance, Accumulated Debt Register, Payment Debt Register and Time Debt Registers");
    prepaySnapshotLog = &structuredData[i].billingDataLog.prepaySnapshot;
  } else {
    sl_zigbee_af_gas_proxy_function_println("GPF: WARN: GetPrepaySnapshot command received with unsupported cause: 0x%08X", cmd_data.snapshotCause);
    return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  }

  sendPrepaySnapshot(cmd_data.earliestStartTime, cmd_data.latestEndTime, cmd_data.snapshotOffset, cmd_data.snapshotCause, prepaySnapshotLog);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

/** @brief Prepayment Cluster Publish Top Up Log
 *
 * @param commandIndex   Ver.: always
 * @param totalNumberOfCommands   Ver.: always
 * @param topUpPayload   Ver.: always
 */
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_publish_top_up_log_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_prepayment_cluster_publish_top_up_log_command_t cmd_data;

  if (zcl_decode_prepayment_cluster_publish_top_up_log_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t i = findStructuredData(endpoint);
  sli_zigbee_gpf_top_up_log_t *topUpLog;
  uint16_t topUpPayloadLength = fieldLength(cmd_data.topUpPayload);
  uint16_t topUpPayloadIndex = 0;
  uint8_t *topUpPayloadCode;
  uint32_t topUpPayloadAmount;
  uint32_t topUpPayloadTime;

  sl_zigbee_af_gas_proxy_function_println("GPF: PublishTopUpLog 0x%02X 0x%02X 0x%04X",
                                          cmd_data.commandIndex,
                                          cmd_data.totalNumberOfCommands,
                                          topUpPayloadLength);

  if (i == GPF_INVALID_LOG_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_gas_proxy_function_println("GPF: Receive Billing Data Log - value of prepayment credits");
  topUpLog = &structuredData[i].billingDataLog.topUp;

  // Ignore any commands that we were not expecting.
  if (topUpLog->catchup && sl_zigbee_af_current_command()->seqNum != topUpLog->catchupSequenceNumber) {
    sl_zigbee_af_gas_proxy_function_println("GPF: WARN: ignoring unexpected PublishTopUpLog command");
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  while (topUpPayloadIndex < topUpPayloadLength) {
    topUpPayloadCode = sl_zigbee_af_get_string(cmd_data.topUpPayload, topUpPayloadIndex, topUpPayloadLength);
    topUpPayloadIndex += sl_zigbee_af_string_length(topUpPayloadCode) + 1;
    topUpPayloadAmount = sl_zigbee_af_get_int32u(cmd_data.topUpPayload, topUpPayloadIndex, topUpPayloadLength);
    topUpPayloadIndex += 4;
    topUpPayloadTime = sl_zigbee_af_get_int32u(cmd_data.topUpPayload, topUpPayloadIndex, topUpPayloadLength);
    topUpPayloadIndex += 4;
    receiveTopUp(topUpPayloadCode, topUpPayloadAmount, topUpPayloadTime, topUpLog);
  }

  if (topUpLog->catchup) {
    stopTopUpLogCatchup(endpoint, topUpLog);
  }

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

/** @brief Prepayment Cluster Get Top Up Log
 *
 * @param latestEndTime   Ver.: always
 * @param numberOfRecords   Ver.: always
 */
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_get_top_up_log_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_prepayment_cluster_get_top_up_log_command_t cmd_data;

  if (zcl_decode_prepayment_cluster_get_top_up_log_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t i = findStructuredData(endpoint);
  sli_zigbee_gpf_top_up_log_t *topUpLog;
  uint32_t earliestStartTime;

  sl_zigbee_af_gas_proxy_function_println("GPF: GetTopUpLog 0x%08X 0x%02X",
                                          cmd_data.latestEndTime,
                                          cmd_data.numberOfRecords);

  if (i == GPF_INVALID_LOG_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_gas_proxy_function_println("GPF: Publish Billing Data Log - value of prepayment credits");
  topUpLog = &structuredData[i].billingDataLog.topUp;

  // GBCS adds an additional filter criteria so if this is use case GCS15e,
  // indicated by this being a loopback command, then grab the start time
  // from the GBZ parser.
  earliestStartTime = (sl_zigbee_af_current_command()->source == sl_zigbee_af_get_node_id())
                      ? sli_zigbee_af_gas_proxy_function_get_gbz_start_time() : 0;
  sendTopUp(earliestStartTime, cmd_data.latestEndTime, cmd_data.numberOfRecords, topUpLog);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

/** @brief Prepayment Cluster Publish Debt Log
 *
 * @param commandIndex   Ver.: always
 * @param totalNumberOfCommands   Ver.: always
 * @param debtPayload   Ver.: always
 */
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_publish_debt_log_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_prepayment_cluster_publish_debt_log_command_t cmd_data;

  if (zcl_decode_prepayment_cluster_publish_debt_log_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t i = findStructuredData(endpoint);
  sli_zigbee_gpf_debt_log_t *debtLog;
  uint16_t debtPayloadLength = fieldLength(cmd_data.debtPayload);
  uint16_t debtPayloadIndex = 0;
  uint32_t collectionTime;
  uint32_t amountCollected;
  uint32_t outstandingDebt;
  uint8_t  debtType;

  sl_zigbee_af_gas_proxy_function_println("GPF: PublishDebtLog 0x%02X 0x%02X 0x%04X",
                                          cmd_data.commandIndex,
                                          cmd_data.totalNumberOfCommands,
                                          debtPayloadLength);

  if (i == GPF_INVALID_LOG_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_gas_proxy_function_println("GPF: Receive Billing Data Log - payment-based debt payments");
  debtLog = &structuredData[i].billingDataLog.debt;

  // Ignore any commands that we were not expecting.
  if (debtLog->catchup && sl_zigbee_af_current_command()->seqNum != debtLog->catchupSequenceNumber) {
    sl_zigbee_af_gas_proxy_function_println("GPF: WARN: ignoring unexpected PublishDebtLog command");
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  while (debtPayloadIndex < debtPayloadLength) {
    collectionTime = sl_zigbee_af_get_int32u(cmd_data.debtPayload, debtPayloadIndex, debtPayloadLength);
    debtPayloadIndex += 4;
    amountCollected = sl_zigbee_af_get_int32u(cmd_data.debtPayload, debtPayloadIndex, debtPayloadLength);
    debtPayloadIndex += 4;
    debtType = sl_zigbee_af_get_int8u(cmd_data.debtPayload, debtPayloadIndex, debtPayloadLength);
    debtPayloadIndex += 1;
    outstandingDebt = sl_zigbee_af_get_int32u(cmd_data.debtPayload, debtPayloadIndex, debtPayloadLength);
    debtPayloadIndex += 4;
    receiveDebt(collectionTime, amountCollected, outstandingDebt, debtType, debtLog);
  }

  if (debtLog->catchup) {
    stopDebtLogCatchup(endpoint, debtLog);
  }

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

/** @brief Prepayment Cluster Get Debt Repayment Log
 *
 * @param latestEndTime   Ver.: always
 * @param numberOfDebts   Ver.: always
 * @param debtType   Ver.: always
 */
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_prepayment_cluster_get_debt_repayment_log_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_prepayment_cluster_get_debt_repayment_log_command_t cmd_data;

  if (zcl_decode_prepayment_cluster_get_debt_repayment_log_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t i = findStructuredData(endpoint);
  sli_zigbee_gpf_debt_log_t *debtLog;
  uint32_t earliestStartTime;

  sl_zigbee_af_gas_proxy_function_println("GPF: GetDebtRepaymentLog 0x%08X 0x%02X 0x%02X",
                                          cmd_data.latestEndTime,
                                          cmd_data.numberOfDebts,
                                          cmd_data.debtType);

  if (i == GPF_INVALID_LOG_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_gas_proxy_function_println("GPF: Publish Billing Data Log - payment-based debt payments");
  debtLog = &structuredData[i].billingDataLog.debt;

  // GBCS adds an additional filter criteria so if this is use case GCS15d,
  // indicated by this being a loopback command, then grab the start time
  // from the GBZ parser.
  earliestStartTime = (sl_zigbee_af_current_command()->source == sl_zigbee_af_get_node_id())
                      ? sli_zigbee_af_gas_proxy_function_get_gbz_start_time() : 0;
  sendDebt(earliestStartTime, cmd_data.latestEndTime, cmd_data.numberOfDebts, cmd_data.debtType, debtLog);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

// Catchup event handler used to retry previously attempted Get requests on
// the various logs.
void sl_zigbee_af_gas_proxy_function_catchup_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);

  uint8_t i;
  uint8_t localEndpoint, remoteEndpoint;
  sl_802154_short_addr_t remoteNodeId;

  sl_zigbee_af_event_set_inactive(gasProxyFunctionCatchupEventControl);

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS; i++) {
    if (structuredData[i].endpoint == GPF_UNUSED_ENDPOINT_ID) {
      continue;
    }

    localEndpoint = structuredData[i].endpoint;
    remoteEndpoint = structuredData[i].remoteEndpoint;
    remoteNodeId = structuredData[i].remoteNodeId;

    if (structuredData[i].functionalNotificationFlags & SL_ZIGBEE_AF_METERING_FNF_GET_SNAPSHOT) {
      sli_zigbee_gpf_snapshot_log_t *snapshotLog = &structuredData[i].dailyReadLog;
      if (snapshotLog->catchup) {
        snapshotLog->catchupSnapshotOffset = 0;
        getSnapshot(localEndpoint, remoteEndpoint, remoteNodeId, snapshotLog);
      }
      snapshotLog = &structuredData[i].billingDataLog.snapshot;
      if (snapshotLog->catchup) {
        snapshotLog->catchupSnapshotOffset = 0;
        getSnapshot(localEndpoint, remoteEndpoint, remoteNodeId, snapshotLog);
      }
      structuredData[i].functionalNotificationFlags &= ~SL_ZIGBEE_AF_METERING_FNF_GET_SNAPSHOT;
      sl_zigbee_af_event_set_delay_ms(gasProxyFunctionCatchupEventControl,
                                      MILLISECOND_TICKS_PER_SECOND);
      continue;
    }

    if (structuredData[i].functionalNotificationFlags & SL_ZIGBEE_AF_METERING_FNF_GET_SAMPLED_DATA) {
      sli_zigbee_gpf_sample_log_t *sampleLog = &structuredData[i].dailyConsumptionLog;
      if (sampleLog->catchup) {
        getSampleData(localEndpoint, remoteEndpoint, remoteNodeId, sampleLog);
      }
      sampleLog = &structuredData[i].profileDataLog;
      if (sampleLog->catchup) {
        getSampleData(localEndpoint, remoteEndpoint, remoteNodeId, sampleLog);
      }
      structuredData[i].functionalNotificationFlags &= ~SL_ZIGBEE_AF_METERING_FNF_GET_SAMPLED_DATA;
      sl_zigbee_af_event_set_delay_ms(gasProxyFunctionCatchupEventControl,
                                      MILLISECOND_TICKS_PER_SECOND);
      continue;
    }

    if (structuredData[i].notificationFlags4 & SL_ZIGBEE_AF_METERING_NF4_GET_PREPAY_SNAPSHOT) {
      sli_zigbee_gpf_prepay_snapshot_log_t *prepaySnapshotLog = &structuredData[i].prepayDailyReadLog;
      if (prepaySnapshotLog->catchup) {
        prepaySnapshotLog->catchupSnapshotOffset = 0;
        getPrepaySnapshot(localEndpoint, remoteEndpoint, remoteNodeId, prepaySnapshotLog);
      }
      prepaySnapshotLog = &structuredData[i].billingDataLog.prepaySnapshot;
      if (prepaySnapshotLog->catchup) {
        prepaySnapshotLog->catchupSnapshotOffset = 0;
        getPrepaySnapshot(localEndpoint, remoteEndpoint, remoteNodeId, prepaySnapshotLog);
      }
      structuredData[i].notificationFlags4 &= ~SL_ZIGBEE_AF_METERING_NF4_GET_PREPAY_SNAPSHOT;
      sl_zigbee_af_event_set_delay_ms(gasProxyFunctionCatchupEventControl,
                                      MILLISECOND_TICKS_PER_SECOND);
      continue;
    }

    if (structuredData[i].notificationFlags4 & SL_ZIGBEE_AF_METERING_NF4_GET_TOP_UP_LOG) {
      sli_zigbee_gpf_top_up_log_t *topUpLog = &structuredData[i].billingDataLog.topUp;
      if (topUpLog->catchup) {
        getTopUp(localEndpoint, remoteEndpoint, remoteNodeId, topUpLog);
      }
      structuredData[i].notificationFlags4 &= ~SL_ZIGBEE_AF_METERING_NF4_GET_TOP_UP_LOG;
      sl_zigbee_af_event_set_delay_ms(gasProxyFunctionCatchupEventControl,
                                      MILLISECOND_TICKS_PER_SECOND);
      continue;
    }

    if (structuredData[i].notificationFlags4 & SL_ZIGBEE_AF_METERING_NF4_GET_DEBT_REPAYMENT_LOG) {
      sli_zigbee_gpf_debt_log_t *debtLog = &structuredData[i].billingDataLog.debt;
      if (debtLog->catchup) {
        getDebt(localEndpoint, remoteEndpoint, remoteNodeId, debtLog);
      }
      structuredData[i].notificationFlags4 &= ~SL_ZIGBEE_AF_METERING_NF4_GET_DEBT_REPAYMENT_LOG;
      sl_zigbee_af_event_set_delay_ms(gasProxyFunctionCatchupEventControl,
                                      MILLISECOND_TICKS_PER_SECOND);
      continue;
    }
  }
}

void sl_zigbee_af_gas_proxy_function_gsme_sync_endpoint_event_handler(uint8_t endpoint)
{
  uint8_t i = findStructuredData(endpoint);
  uint32_t nowUTC, lastEntryTime, aDayAgo, halfAnHourAgo;

  sl_zigbee_af_endpoint_event_set_inactive(endpointEvent,
                                           endpoint);

  nowUTC        = sl_zigbee_af_get_current_time();
  aDayAgo       = nowUTC - SECONDS_IN_DAY;
  halfAnHourAgo = nowUTC - SECONDS_IN_HALF_HOUR;

  // If there is structured data that applies to this endpoint...
  if (i != GPF_INVALID_LOG_INDEX) {
    // ...for any of the temporal logs that are out of date, initiate a catchup.
    if (sampleLogIsOutOfDate(&structuredData[i].profileDataLog,
                             halfAnHourAgo,
                             &lastEntryTime)) {
      // We use the last entry time plus 1 second so we make sure not to get
      // any duplicate events, or we request all entries if the GSME has not
      // checked in at all.
      structuredData[i].profileDataLog.catchupTime = lastEntryTime + 1;
      startSampleLogCatchup(endpoint,
                            &structuredData[i].profileDataLog,
                            false); // reset?
    }
    if (sampleLogIsOutOfDate(&structuredData[i].dailyConsumptionLog,
                             aDayAgo,
                             &lastEntryTime)) {
      // We use the last entry time plus 1 second so we make sure not to get
      // any duplicate events, or we request all entries if the GSME has not
      // checked in at all.
      structuredData[i].dailyConsumptionLog.catchupTime = lastEntryTime + 1;
      startSampleLogCatchup(endpoint,
                            &structuredData[i].dailyConsumptionLog,
                            false); // reset?
    }
    if (snapshotLogIsOutOfDate(&structuredData[i].dailyReadLog,
                               aDayAgo,
                               &lastEntryTime)) {
      // Per 10.4.2.1, we populate the catchup time "with the UTC start time
      // field populated based on the last received snapshot timestamp, if one
      // has been received."
      structuredData[i].dailyReadLog.catchupTime = lastEntryTime;
      startSnapshotLogCatchup(endpoint,
                              &structuredData[i].dailyReadLog,
                              false); // reset?
    }
    if (prepaySnapshotLogIsOutOfDate(&structuredData[i].prepayDailyReadLog,
                                     aDayAgo,
                                     &lastEntryTime)) {
      // Per 10.4.2.2,  we populate the catchup time "with the UTC start time
      // field populated based on the last received prepayment snapshot
      // timestamp, if one has been received."
      structuredData[i].prepayDailyReadLog.catchupTime = lastEntryTime;
      startPrepaySnapshotLogCatchup(endpoint,
                                    &structuredData[i].prepayDailyReadLog,
                                    false); // reset?
    }

    // Fire this event next time we expect the GSME to have checked in.
    // By the v0.8.1 GBCS spec, in section 10.4.2.8, this should be
    // 30 minutes from now.
    sl_zigbee_af_endpoint_event_set_delay_ms(endpointEvent,
                                             endpoint,
                                             SECONDS_IN_HALF_HOUR * MILLISECOND_TICKS_PER_SECOND);
  }
}

/** @brief Simple Metering Cluster Get Notified Message
 *
 * @param notificationScheme   Ver.: always
 * @param notificationFlagAttributeId   Ver.: always
 * @param notificationFlagsN   Ver.: always
 */
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_get_notified_message_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_simple_metering_cluster_get_notified_message_command_t cmd_data;

  if (zcl_decode_simple_metering_cluster_get_notified_message_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  uint8_t notificationScheme = cmd_data.notificationScheme;
  uint16_t notificationFlagAttributeId = cmd_data.notificationFlagAttributeId;
  uint32_t notificationFlagsN = cmd_data.notificationFlagsN;
  sl_zigbee_af_cluster_command_t *cmd_current = sl_zigbee_af_current_command();
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t i = findStructuredData(endpoint);

  /*
   * From GBCS
   *
   * For clarity, the GSME:
   *
   *  - shall not action ZSE / ZCL commands received from the GPF in relation
   *    to any of the flags within NotificationFlags2, NotificationFlags3 and
   *    NotificationFlags5;
   *
   *  - for NotificationFlags4, shall only action ZSE / ZCL commands received
   *    from the GPF in relation to the flags specified below.
   *
   *      Bit Number    Waiting Command
   *      6             Get Prepay Snapshot
   *      7             Get Top Up Log
   *      9             Get Debt Repayment Log
   *
   *  - for FunctionalNotificationFlags, shall only action ZSE / ZCL commands
   *    received from the GPF in relation to the flags specified below
   *
   *      Bit Number   Waiting Command
   *      0            New OTA Firmware
   *      1            CBKE Update Request
   *      4            Stay Awake Request HAN
   *      5            Stay Awake Request WAN
   *      6-8          Push Historical Metering Data Attribute Set
   *      9-11         Push Historical Prepayment Data Attribute Set
   *      12           Push All Static Data - Basic Cluster
   *      13           Push All Static Data - Metering Cluster
   *      14           Push All Static Data - Prepayment Cluster
   *      15           NetworkKeyActive
   *      21           Tunnel Message Pending
   *      22           GetSnapshot
   *      23           GetSampledData
   */

  if (i == GPF_INVALID_LOG_INDEX || notificationScheme != 0x02
      || (notificationFlagAttributeId != ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID
          && notificationFlagAttributeId != ZCL_NOTIFICATION_FLAGS_4_ATTRIBUTE_ID)) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  // Since this request could result in many commands being sent back to the
  // sleepy device we schedule the work for the catchup event handler which can
  // deal with spacing out the commands.
  structuredData[i].remoteEndpoint = cmd_current->apsFrame->sourceEndpoint;
  structuredData[i].remoteNodeId = cmd_current->source;
  structuredData[i].functionalNotificationFlags |=
    (notificationFlagAttributeId == ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID) ? notificationFlagsN : 0;
  structuredData[i].notificationFlags4 |=
    (notificationFlagAttributeId == ZCL_NOTIFICATION_FLAGS_4_ATTRIBUTE_ID) ? notificationFlagsN : 0;
  sl_zigbee_af_event_set_active(gasProxyFunctionCatchupEventControl);

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

/** @brief Simple Metering Cluster Client Default Response
 *
 * This function is called when the client receives the default response from
 * the server.
 *
 * @param endpoint Destination endpoint  Ver.: always
 * @param commandId Command id  Ver.: always
 * @param status Status in default response  Ver.: always
 */
void sl_zigbee_af_simple_metering_cluster_client_default_response_cb(uint8_t endpoint,
                                                                     uint8_t commandId,
                                                                     sl_zigbee_af_status_t status)
{
  uint8_t i = findStructuredData(endpoint);

  if (commandId == ZCL_MIRROR_REMOVED_COMMAND_ID && status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_endpoint_enable_disable(endpoint, false);
    sl_zigbee_af_gas_proxy_function_println("GPF: Disabling endpoint %u", endpoint);
    return;
  }

  if (i == GPF_INVALID_LOG_INDEX) {
    return;
  }

  if (commandId == ZCL_GET_SAMPLED_DATA_COMMAND_ID) {
    sli_zigbee_gpf_sample_log_t *sampleLog = &structuredData[i].dailyConsumptionLog;
    sli_zigbee_gpf_sample_log_t *otherSampleLog = &structuredData[i].profileDataLog;
    if (sampleLog->catchup && sampleLog->catchupSequenceNumber == sl_zigbee_af_current_command()->seqNum) {
      stopSampleLogCatchup(endpoint, sampleLog, otherSampleLog);
    } else {
      sampleLog = &structuredData[i].profileDataLog;
      otherSampleLog = &structuredData[i].dailyConsumptionLog;
      if (sampleLog->catchup && sampleLog->catchupSequenceNumber == sl_zigbee_af_current_command()->seqNum) {
        stopSampleLogCatchup(endpoint, sampleLog, otherSampleLog);
      }
    }
  } else if (commandId == ZCL_GET_SNAPSHOT_COMMAND_ID) {
    sli_zigbee_gpf_snapshot_log_t *snapshotLog = &structuredData[i].dailyReadLog;
    sli_zigbee_gpf_snapshot_log_t *otherSnapshotLog = &structuredData[i].billingDataLog.snapshot;
    if (snapshotLog->catchup && snapshotLog->catchupSequenceNumber == sl_zigbee_af_current_command()->seqNum) {
      stopSnapshotLogCatchup(endpoint, snapshotLog, otherSnapshotLog);
    } else {
      snapshotLog = &structuredData[i].billingDataLog.snapshot;
      otherSnapshotLog = &structuredData[i].dailyReadLog;
      if (snapshotLog->catchup && snapshotLog->catchupSequenceNumber == sl_zigbee_af_current_command()->seqNum) {
        stopSnapshotLogCatchup(endpoint, snapshotLog, otherSnapshotLog);
      }
    }
  }
}

/** @brief Prepayment Cluster Client Default Response
 *
 * This function is called when the client receives the default response from
 * the server.
 *
 * @param endpoint Destination endpoint  Ver.: always
 * @param commandId Command id  Ver.: always
 * @param status Status in default response  Ver.: always
 */
void sl_zigbee_af_prepayment_cluster_client_default_response_cb(uint8_t endpoint,
                                                                uint8_t commandId,
                                                                sl_zigbee_af_status_t status)
{
  UNUSED_VAR(status);

  uint8_t i = findStructuredData(endpoint);

  if (i == GPF_INVALID_LOG_INDEX) {
    return;
  }

  if (commandId == ZCL_GET_PREPAY_SNAPSHOT_COMMAND_ID) {
    sli_zigbee_gpf_prepay_snapshot_log_t *prepaySnapshotLog = &structuredData[i].prepayDailyReadLog;
    sli_zigbee_gpf_prepay_snapshot_log_t *otherPrepaySnapshotLog = &structuredData[i].billingDataLog.prepaySnapshot;
    if (prepaySnapshotLog->catchup && prepaySnapshotLog->catchupSequenceNumber == sl_zigbee_af_current_command()->seqNum) {
      stopPrepaySnapshotLogCatchup(endpoint, prepaySnapshotLog, otherPrepaySnapshotLog);
    } else {
      prepaySnapshotLog = &structuredData[i].billingDataLog.prepaySnapshot;
      otherPrepaySnapshotLog = &structuredData[i].prepayDailyReadLog;
      if (prepaySnapshotLog->catchup && prepaySnapshotLog->catchupSequenceNumber == sl_zigbee_af_current_command()->seqNum) {
        stopPrepaySnapshotLogCatchup(endpoint, prepaySnapshotLog, otherPrepaySnapshotLog);
      }
    }
  } else if (commandId == ZCL_GET_TOP_UP_LOG_COMMAND_ID) {
    sli_zigbee_gpf_top_up_log_t *topUpLog = &structuredData[i].billingDataLog.topUp;
    if (topUpLog->catchup && topUpLog->catchupSequenceNumber == sl_zigbee_af_current_command()->seqNum) {
      stopTopUpLogCatchup(endpoint, topUpLog);
    }
  } else if (commandId == ZCL_GET_DEBT_REPAYMENT_LOG_COMMAND_ID) {
    sli_zigbee_gpf_debt_log_t *debtLog = &structuredData[i].billingDataLog.debt;
    if (debtLog->catchup && debtLog->catchupSequenceNumber == sl_zigbee_af_current_command()->seqNum) {
      stopDebtLogCatchup(endpoint, debtLog);
    }
  }
}

/** @brief Simple Metering Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 * @param attributeId Attribute that changed  Ver.: always
 */
void sl_zigbee_af_simple_metering_cluster_server_attribute_changed_cb(uint8_t endpoint,
                                                                      sl_zigbee_af_attribute_id_t attributeId)
{
  uint8_t i = findStructuredData(endpoint);

  if (i == GPF_INVALID_LOG_INDEX
      || !structuredData[i].alternativeHistoricalConsumption.catchup) {
    return;
  }

  // When the node reboots we reset the Alternative Historical Consumption
  // Attribute Set and then set the SL_ZIGBEE_AF_METERING_FNF_PUSH_HISTORICAL_METERING_DATA_ATTRIBUTE_SET
  // flag within the functional notification flags attribute.  This will indicate
  // to the GSME that is should push the alternative historical cost consumption attributes.
  // We don't check that every attribute is sent but we do need to reset the
  // functional notification flags when the GSME pushes this data so we check
  // for one of the attributes and when we receive it then the flag is reset.
  if (attributeId == ZCL_PREVIOUS_DAY_ALTERNATIVE_CONSUMPTION_DELIVERED_ATTRIBUTE_ID) {
    structuredData[i].alternativeHistoricalConsumption.catchup = false;
    clearNotificationFlag(endpoint,
                          ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID,
                          SL_ZIGBEE_AF_METERING_FNF_PUSH_HISTORICAL_METERING_DATA_ATTRIBUTE_SET);
  }
}

/** @brief Prepayment Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 * @param attributeId Attribute that changed  Ver.: always
 */
void sl_zigbee_af_prepayment_cluster_server_attribute_changed_cb(uint8_t endpoint,
                                                                 sl_zigbee_af_attribute_id_t attributeId)
{
  uint8_t i = findStructuredData(endpoint);

  if (i == GPF_INVALID_LOG_INDEX
      || !structuredData[i].historicalCostConsumption.catchup) {
    return;
  }

  // When the node reboots we reset the Historical Cost Consumption Information
  // Attribute Set and then set the SL_ZIGBEE_AF_METERING_FNF_PUSH_HISTORICAL_PREPAYMENT_DATA_ATTRIBUTE_SET
  // flag within the functional notification flags attribute.  This will indicate
  // to the GSME that is should push the historical cost consumption attributes.
  // We don't check that every attribute is sent but we do need to reset the
  // functional notification flags when the GSME pushes this data so we check
  // for one of the attributes and when we receive it then the flag is reset.
  if (attributeId == ZCL_PREVIOUS_DAY_COST_CONSUMPTION_DELIVERED_ATTRIBUTE_ID) {
    structuredData[i].historicalCostConsumption.catchup = false;
    clearNotificationFlag(endpoint,
                          ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID,
                          SL_ZIGBEE_AF_METERING_FNF_PUSH_HISTORICAL_PREPAYMENT_DATA_ATTRIBUTE_SET);
  }
}
