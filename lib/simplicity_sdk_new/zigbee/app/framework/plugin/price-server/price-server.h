/***************************************************************************//**
 * @file
 * @brief APIs and defines for the Price Server plugin.
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

#ifndef SILABS_PRICE_SERVER_H
#define SILABS_PRICE_SERVER_H

#include "app/framework/plugin/price-common/price-common.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESEN
#ifdef SL_CATALOG_ZIGBEE_PRICE_SERVER_PRESENT
#include "price-server-config.h"
#endif

/**
 * @defgroup price-server Price Server
 * @ingroup component cluster
 * @brief API and Callbacks for the Price Cluster Server Component
 *
 * This component provides an implementation of a Smart Energy price server.
 * The price server holds a table of prices locally and answers requests
 * to retrieve the list of commodity prices. It requires extension to
 * add / remove prices from the table and to send out new prices to
 * those devices that have subscribed to the ESI for price updates.
 *
 */

/**
 * @addtogroup price-server
 * @{
 */

#ifndef SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_PRICE_TABLE_SIZE
  #define SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_PRICE_TABLE_SIZE (5)
#endif

#ifndef SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE
  #define SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE (2)
#endif

#ifndef SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE
  #define SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE (2)
#endif

#ifndef SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_MAX_TIERS_PER_TARIFF
  #define SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_MAX_TIERS_PER_TARIFF (2)
#endif

#ifndef SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE
  #define SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE (2)
#endif

#ifndef SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE
  #define SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE (2)
#endif

#ifndef SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONSOLIDATED_BILL_TABLE_SIZE
  #define SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONSOLIDATED_BILL_TABLE_SIZE (5)
#endif

#ifndef SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CREDIT_PAYMENT_TABLE_SIZE
  #define SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CREDIT_PAYMENT_TABLE_SIZE (5)
#endif

#ifndef SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE
  #define SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE (2)
#endif

#ifndef SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE
  #define SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE (2)
#endif

#ifndef SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE
  #define SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE (2)
#endif

#define ZCL_PRICE_CLUSTER_BLOCK_THRESHOLDS_PAYLOAD_SIZE (6u)
#define ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUBPAYLOAD_BLOCK_SIZE     (5u)

#define fieldLength(field) \
  (sl_zigbee_af_current_command()->bufLen - (field - sl_zigbee_af_current_command()->buffer));

#define ZCL_PRICE_CLUSTER_MAX_TOU_BLOCKS (15u)
#define ZCL_PRICE_CLUSTER_MAX_TOU_BLOCK_TIERS (15u)
#define ZCL_PRICE_CLUSTER_MAX_TOU_TIERS (48u)
#define ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUB_PAYLOAD_ENTRY_SIZE (5u)

// To help keep track of the status of the tariffs in the table
// (also, corresponding price matrices).
#define CURRENT       BIT(1)
#define FUTURE        BIT(2)
#define PUBLISHED     BIT(3)

#define TARIFF_TYPE_MASK (0x0Fu)
#define CHARGING_SCHEME_MASK (0xF0u)

#define tariffIsCurrent(tariff) ((tariff)->status & CURRENT)
#define tariffIsFuture(tariff)  ((tariff)->status & FUTURE)
#define tariffIsPublished(tariff)  ((tariff)->status & PUBLISHED)
#define priceMatrixIsCurrent(pm) ((pm)->status & CURRENT)
#define priceMatrixIsFuture(pm) ((pm)->status & FUTURE)
#define priceMatrixIsPublished(pm) ((pm)->status & PUBLISHED)
#define blockThresholdsIsCurrent(bt) ((bt)->status & CURRENT)
#define blockThresholdsIsFuture(bt) ((bt) > status & FUTURE)
#define blockThresholdsIsPublished(bt) ((bt)->status & PUBLISHED)

/**
 * @brief The price and metadata used by the Price server plugin.
 *
 * The application can get and set the prices used by the plugin by calling
 * ::sl_zigbee_af_price_get_price_table_entry and
 * ::sl_zigbee_af_price_set_price_table_entry.
 */

/**
 * @brief Conversion factor infos by the Price server plugin.
 *
 */

typedef struct {
  uint32_t providerId;
  uint32_t rawBlockPeriodStartTime;
  uint32_t blockPeriodDuration;
  // The "thresholdMultiplier" and "threadholdDivisor" are included in this structure
  // since these should be specified with the block period.
  // These values are stored as the "Threshold Multiplier" and "Threshold Divisor"
  // attributes in the Block Period (Delivered) attribute set (D.4.2.2.3).
  uint32_t thresholdMultiplier;
  uint32_t thresholdDivisor;
  uint8_t  blockPeriodControl;
  uint8_t  blockPeriodDurationType;
  uint8_t  tariffType;
  uint8_t  tariffResolutionPeriod;
} sl_zigbee_af_price_block_period_t;

typedef struct {
  uint32_t providerId;
  uint32_t rawBillingPeriodStartTime;
  uint32_t billingPeriodDuration;
  uint8_t billingPeriodDurationType;
  uint8_t tariffType;
} sl_zigbee_af_price_billing_period_t;

typedef struct {
  uint32_t providerId;
  uint32_t durationInMinutes;
  uint8_t  tariffType;
  uint8_t  cppPriceTier;
  uint8_t  cppAuth;
} sl_zigbee_af_price_cpp_event_t;

typedef struct {
  uint32_t providerId;
  uint32_t rawStartTimeUtc;   // start time as received from caller, prior to any adjustments
  uint32_t billingPeriodDuration;
  uint32_t consolidatedBill;
  uint16_t currency;
  uint8_t  billingPeriodDurationType;
  uint8_t  tariffType;
  uint8_t  billTrailingDigit;
} sl_zigbee_af_price_consolidated_bills_t;

#define CREDIT_PAYMENT_REF_STRING_LEN (20u)
typedef struct {
  uint32_t providerId;
  uint32_t creditPaymentDueDate;
  uint32_t creditPaymentAmountOverdue;
  uint32_t creditPayment;
  uint32_t creditPaymentDate;
  uint8_t  creditPaymentStatus;
  uint8_t  creditPaymentRef[CREDIT_PAYMENT_REF_STRING_LEN + 1u];
} sl_zigbee_af_price_credit_payment_t;

typedef struct {
  uint32_t conversionFactor;
  uint8_t conversionFactorTrailingDigit;
} sl_zigbee_af_price_conversion_factor_t;

typedef struct {
  uint32_t calorificValue;
  uint8_t calorificValueUnit;
  uint8_t calorificValueTrailingDigit;
} sl_zigbee_af_price_calorific_value_t;

typedef struct {
  uint32_t providerId;
  uint32_t issuerTariffId;
  uint8_t  tariffType;
  bool valid;
} sl_zigbee_af_price_cancel_tariff_t;

typedef struct {
  uint32_t providerId;
  uint32_t co2Value;
  uint8_t tariffType;
  uint8_t co2ValueUnit;
  uint8_t co2ValueTrailingDigit;
} sl_zigbee_af_price_co2_value_t;

typedef struct {
  uint32_t providerId;
  uint16_t oldCurrency;
  uint16_t newCurrency;
  uint32_t conversionFactor;
  uint8_t  conversionFactorTrailingDigit;
  uint32_t currencyChangeControlFlags;
} sl_zigbee_af_price_currency_conversion_t;

#define TIER_LABEL_SIZE  (12u)
typedef struct {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t issuerTariffId;
  uint8_t  valid;
  uint8_t  numberOfTiers;
  uint8_t  tierIds[SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_MAX_TIERS_PER_TARIFF];
  uint8_t  tierLabels[SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_MAX_TIERS_PER_TARIFF][TIER_LABEL_SIZE + 1u];
} sl_zigbee_af_price_tier_label_value_t;

typedef struct {
  sl_zigbee_af_price_tier_label_value_t entry[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE];
//  uint8_t valid[SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE];
//  uint32_t providerId[SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE];
//  uint32_t issuerEventId[SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE];
//  uint32_t issuerTariffId[SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE];
//  uint8_t tierId[SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE];
//  uint8_t tierLabel[SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE][13];
} sl_zigbee_af_price_tier_label_table_t;

typedef struct {
  sl_zigbee_af_price_common_info_t commonInfos[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE];
  sl_zigbee_af_price_block_period_t blockPeriods[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE];
} sl_zigbee_af_price_block_period_table_t;

typedef struct {
  sl_zigbee_af_price_common_info_t commonInfos[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE];
  sl_zigbee_af_price_billing_period_t billingPeriods[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE];
} sl_zigbee_af_price_billing_period_table_t;

typedef struct {
  sl_zigbee_af_price_common_info_t commonInfos[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT];
  sl_zigbee_af_price_cpp_event_t cppEvent[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT];
} sl_zigbee_af_price_cpp_table_t;

typedef struct {
  sl_zigbee_af_price_common_info_t commonInfos[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONSOLIDATED_BILL_TABLE_SIZE];
  sl_zigbee_af_price_consolidated_bills_t consolidatedBills[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONSOLIDATED_BILL_TABLE_SIZE];
} sl_zigbee_af_price_consolidated_bills_table_t;

typedef struct {
  sl_zigbee_af_price_common_info_t commonInfos[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CREDIT_PAYMENT_TABLE_SIZE];
  sl_zigbee_af_price_credit_payment_t creditPayment[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CREDIT_PAYMENT_TABLE_SIZE];
} sl_zigbee_af_price_credit_payment_table_t;

typedef struct {
  sl_zigbee_af_price_common_info_t commonInfos[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE];
  sl_zigbee_af_price_conversion_factor_t priceConversionFactors[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE];
} sl_zigbee_af_price_conversion_factor_table_t;

typedef struct {
  sl_zigbee_af_price_common_info_t commonInfos[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE];
  sl_zigbee_af_price_calorific_value_t calorificValues[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE];
} sl_zigbee_af_price_calorific_value_table_t;

typedef struct {
  sl_zigbee_af_price_common_info_t commonInfos[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE];
  sl_zigbee_af_price_co2_value_t co2Values[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE];
} sl_zigbee_af_price_co2_table_t;

typedef struct {
  sl_zigbee_af_price_common_info_t commonInfos[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT];
  sl_zigbee_af_price_currency_conversion_t currencyConversion[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT];
} sl_zigbee_af_price_currency_conversion_table_t;

typedef struct {
  sl_zigbee_af_price_cancel_tariff_t cancelTariff[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT];
} sl_zigbee_af_price_cancel_tariff_table_t;

typedef uint8_t sli_zigbee_af_price_block_threshold[ZCL_PRICE_CLUSTER_BLOCK_THRESHOLDS_PAYLOAD_SIZE];
typedef struct {
  union {
    sli_zigbee_af_price_block_threshold blockAndTier[ZCL_PRICE_CLUSTER_MAX_TOU_BLOCK_TIERS][ZCL_PRICE_CLUSTER_MAX_TOU_BLOCKS - 1u];
    sli_zigbee_af_price_block_threshold block[ZCL_PRICE_CLUSTER_MAX_TOU_BLOCKS - 1];
  } thresholds;
  uint32_t providerId;
  uint32_t issuerTariffId;
  uint8_t status;
} sl_zigbee_af_scheduled_block_thresholds_t;

typedef struct {
  sl_zigbee_af_price_common_info_t commonInfos[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE];
  sl_zigbee_af_scheduled_block_thresholds_t scheduledBlockThresholds[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE];
} sl_zigbee_af_scheduled_block_thresholds_table_t;

typedef struct {
  uint32_t providerId;
  uint32_t issuerTariffId;
  uint8_t status;
  uint8_t tariffTypeChargingScheme;

  // below fields have corresponding zcl attributes.
  uint8_t  tariffLabel[ZCL_PRICE_CLUSTER_MAXIMUM_RATE_LABEL_LENGTH + 1u];
  uint8_t  numberOfPriceTiersInUse;
  uint8_t  numberOfBlockThresholdsInUse;
  uint8_t  tierBlockMode;
  uint8_t  unitOfMeasure;
  uint16_t currency;
  uint8_t  priceTrailingDigit;
  uint32_t standingCharge;
  uint32_t blockThresholdMultiplier;
  uint32_t blockThresholdDivisor;
} sl_zigbee_af_scheduled_tariff_t;

typedef struct {
  sl_zigbee_af_price_common_info_t commonInfos[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE];
  sl_zigbee_af_scheduled_tariff_t scheduledTariffs[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE];
} sl_zigbee_af_scheduled_tariff_table_t;

typedef struct {
  union {
    uint32_t blockAndTier[ZCL_PRICE_CLUSTER_MAX_TOU_BLOCK_TIERS][ZCL_PRICE_CLUSTER_MAX_TOU_BLOCKS];
    uint32_t tier[ZCL_PRICE_CLUSTER_MAX_TOU_TIERS];
  } matrix;
  uint32_t providerId;
  uint32_t issuerTariffId;
  uint8_t status;
} sl_zigbee_af_scheduled_price_matrix_t;

typedef struct {
  sl_zigbee_af_price_common_info_t commonInfos[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE];
  sl_zigbee_af_scheduled_price_matrix_t scheduledPriceMatrix[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE];
} sl_zigbee_af_scheduled_price_matrix_table_t;

typedef struct {
  sl_zigbee_af_price_block_period_table_t blockPeriodTable;
  sl_zigbee_af_price_conversion_factor_table_t conversionFactorTable;
  sl_zigbee_af_price_calorific_value_table_t calorificValueTable;
  sl_zigbee_af_price_co2_table_t co2ValueTable;
  sl_zigbee_af_price_tier_label_table_t tierLabelTable;
  sl_zigbee_af_price_billing_period_table_t billingPeriodTable;
  sl_zigbee_af_price_consolidated_bills_table_t consolidatedBillsTable;
  sl_zigbee_af_price_cpp_table_t cppTable;
  sl_zigbee_af_price_credit_payment_table_t creditPaymentTable;
  sl_zigbee_af_price_currency_conversion_table_t currencyConversionTable;
  sl_zigbee_af_price_cancel_tariff_table_t cancelTariffTable;
  sl_zigbee_af_scheduled_tariff_table_t scheduledTariffTable;
  sl_zigbee_af_scheduled_block_thresholds_table_t scheduledBlockThresholdsTable;
  sl_zigbee_af_scheduled_price_matrix_table_t  scheduledPriceMatrixTable;
} sl_zigbee_af_price_server_info_t;

extern sl_zigbee_af_price_server_info_t priceServerInfo;

/**
 * @name API
 * @{
 */

/**
 * @brief Send the next get scheduled prices command.
 *
 * @param endpoint The endpoint in question.
 **/
void sl_zigbee_af_price_server_send_get_scheduled_prices(uint8_t endpoint);

/**
 * @brief Return the number of seconds until the next get scheduled prices command should be sent.
 *
 * @return The number of seconds until the next get scheduled prices command should be sent.
 **/
uint32_t sl_zigbee_af_price_server_seconds_until_get_scheduled_prices_event(void);

/**
 * @brief Clear all prices in the price table.
 *
 * @param endpoint The endpoint in question.
 **/
void sl_zigbee_af_price_clear_price_table(uint8_t endpoint);

/**
 * @brief Clear all tariffs in the tariff table.
 *
 * @param endpoint The endpoint in question.
 */
void sl_zigbee_af_price_clear_tariff_table(uint8_t endpoint);

/**
 * @brief Clear all price matrices in the price matrix table.
 *
 * @param endpoint The endpoint in question.
 */
void sl_zigbee_af_price_clear_price_matrix_table(uint8_t endpoint);

/**
 * @brief Clear all block thresholds in the block thresholds table.
 *
 * @param endpoint The endpoint in question.
 */
void sl_zigbee_af_price_clear_block_thresholds_table(uint8_t endpoint);

/**
 * @brief Get a price used by the price server plugin.
 *
 * This function gets a price and metadata that the plugin will
 * send to clients. For "start now" prices that are current or scheduled, the
 * duration is adjusted to reflect how many minutes remain for the price.
 * Otherwise, the start time and duration of "start now" prices reflect the
 * actual start and the original duration.
 *
 * @param endpoint The relevant endpoint.
 * @param index The index in the price table.
 * @param price The ::sl_zigbee_af_scheduled_price_t structure describing the price.
 * @return True if the price was found or false is the index is invalid.
 */
bool sl_zigbee_af_price_get_price_table_entry(uint8_t endpoint,
                                              uint8_t index,
                                              sl_zigbee_af_scheduled_price_t *price);

/**
 * @brief Set values in the block period table.
 *
 * @param endpoint The relevant endpoint.
 * @param providerId A unique identifier for the commodity provider.
 * @param issuerEventId The event ID of the block period data.
 * @param blockPeriodStartTime Time at which the block period data is valid.
 * @param blockPeriodDuration The block period duration. Units are specified by the blockPeriodDurationType.
 * @param blockPeriodControl Identifies additional control options for the block period command.
 * @param blockPeriodDurationType A bitmap that indicates the units used in the block period.
 * @param tariffType A bitmap identifying the type of tariff published in this command.
 * @param tariffResolutionPeriod The resolution period for the block tariff.
 *
 **/
void sl_zigbee_af_price_server_block_period_add(uint8_t endpoint, uint32_t providerId, uint32_t issuerEventId,
                                                uint32_t blockPeriodStartTime, uint32_t blockPeriodDuration,
                                                uint8_t  blockPeriodControl, uint8_t blockPeriodDurationType,
                                                uint32_t thresholdMultiplier, uint32_t thresholdDivisor,
                                                uint8_t  tariffType, uint8_t tariffResolutionPeriod);

/**
 * @brief Send a publish block period command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 * @param index The index of the table whose data will be used in the command.
 *
 **/
void sl_zigbee_af_price_server_block_period_pub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint, uint8_t index);

/**
 * @brief Print data in the specified index of the block period table.
 *
 * @param endpoint The relevant endpoint.
 * @param index The index of the table whose index will be printed.
 *
 **/
void sl_zigbee_af_price_server_block_period_print(uint8_t endpoint, uint8_t index);

/**
 * @brief Return the number of seconds until the next block period event occurs.
 *
 * @param endpoint The relevant endpoint.
 * @return Returns the number of seconds until the next block period event.
 *
 **/
uint32_t sl_zigbee_af_price_server_seconds_until_block_period_event(uint8_t endpoint);

/**
 * @brief Update block period attributes to match the current block period.
 *
 * @param endpoint The relevant endpoint.
 * @param repeat   Indicates whether or not to try to repeat the current block period
 * in the next block period. This is also controlled by a bit in the
 * blockPeriodControl, saying whether or not the block period should repeat
 * on expiry.
 **/
void sl_zigbee_af_price_server_refresh_block_period(uint8_t endpoint, bool repeat);

/**
 * @brief Get a tariff used by the price server plugin.
 *
 * This function gets a tariff and associated metadata that
 * the plugin will send to clients.
 *
 * @param endpoint The relevant endpoint.
 * @param index    The index in the tariff table.
 * @param info   The ::sl_zigbee_af_price_common_info_t structure describing the tariff.
 * @param tariff   The ::sl_zigbee_af_scheduled_tariff_t structure describing the tariff.
 * @return         True if the tariff was found.
 */
bool sl_zigbee_af_price_get_tariff_table_entry(uint8_t endpoint,
                                               uint8_t index,
                                               sl_zigbee_af_price_common_info_t *info,
                                               sl_zigbee_af_scheduled_tariff_t *tariff);

/**
 * @brief Retrieve a price matrix entry by index.
 *
 * This function gets a price matrix and associated metadata that
 * the plugin will send to clients.
 *
 * @param endpoint The relevant endpoint.
 * @param index    The index in the price matrix table.
 * @param pm       The ::sl_zigbee_af_scheduled_price_matrix_t structure describing the price matrix.
 * @return         True if the price matrix was found.
 */
bool sl_zigbee_af_price_get_price_matrix(uint8_t endpoint,
                                         uint8_t index,
                                         sl_zigbee_af_price_common_info_t *info,
                                         sl_zigbee_af_scheduled_price_matrix_t *pm);

/**
 * @brief Get the block thresholds used by the Price server plugin.
 *
 * This function gets the block thresholds and associated metadata that
 * the plugin will send to clients.
 *
 * @param endpoint The relevant endpoint.
 * @param index    The index in the block thresholds table.
 * @param bt       The ::sl_zigbee_af_scheduled_block_thresholds_t structure describing the block thresholds.
 * @return         True if the block thresholds was found.
 */
bool sl_zigbee_af_price_get_block_thresholds_table_entry(uint8_t endpoint,
                                                         uint8_t index,
                                                         sl_zigbee_af_scheduled_block_thresholds_t *bt);

/**
 * @brief Get a tariff by issuer tariff ID and endpoint.
 *
 * @param endpoint        The relevant endpoint.
 * @param issuerTariffId  The issuer tariff ID.
 * @param info            The ::sl_zigbee_af_price_common_info_t structure describing the tariff.
 * @param tariff          The ::sl_zigbee_af_scheduled_tariff_t structure describing the tariff.
 * @return                True if the tariff was found.
 */
bool sl_zigbee_af_price_get_tariff_by_issuer_tariff_id(uint8_t endpoint,
                                                       uint32_t issuerTariffId,
                                                       sl_zigbee_af_price_common_info_t *info,
                                                       sl_zigbee_af_scheduled_tariff_t *tariff);

/**
 * @brief Get a price matrix by issuer tariff ID and endpoint.
 *
 * @param endpoint The relevant endpoint.
 * @param issuerTariffId  The issuer tariff ID.
 * @param pm       The ::sl_zigbee_af_scheduled_price_matrix_t structure describing the price matrix.
 * @return         True if the price matrix was found.
 */
bool sl_zigbee_af_price_get_price_matrix_by_issuer_tariff_id(uint8_t endpoint,
                                                             uint32_t issuerTariffId,
                                                             sl_zigbee_af_price_common_info_t *info,
                                                             sl_zigbee_af_scheduled_price_matrix_t *pm);

/**
 * @brief Get the block thresholds by issuer tariff ID and endpoint.
 *
 * @param endpoint The relevant endpoint.
 * @param issuerTariffId  The issuer tariff ID.
 * @param bt       The ::sl_zigbee_af_scheduled_block_thresholds_t structure describing the block thresholds.
 * @return         True if the block thresholds were found.
 */
bool sl_zigbee_af_price_get_block_thresholds_by_issuer_tariff_id(uint8_t endpoint,
                                                                 uint32_t issuerTariffId,
                                                                 sl_zigbee_af_price_common_info_t *info,
                                                                 sl_zigbee_af_scheduled_block_thresholds_t *bt);
/**
 * @brief Set a price used by the price server plugin.
 *
 * This function sets a price and metadata that the plugin will
 * send to clients. Setting the start time to zero instructs clients to start
 * the price now. For "start now" prices, the plugin will automatically adjust
 * the duration reported to clients based on the original start time of the
 * price.
 *
 * @param endpoint The relevant endpoint.
 * @param index The index in the price table.
 * @param price The ::sl_zigbee_af_scheduled_price_t structure describing the price. If
 * NULL, the price is removed from the server.
 * @return True if the price was set or removed or false is the index is
 * invalid.
 */
bool sl_zigbee_af_price_set_price_table_entry(uint8_t endpoint,
                                              uint8_t index,
                                              const sl_zigbee_af_scheduled_price_t *price);

/**
 * @brief Set a tariff used by the price server plugin.
 *
 * This function sets a tariff and metadata that the plugin
 * will send to clients.
 *
 * @param endpoint The relevant endpoint.
 * @param index    The index in the tariff table.
 * @param tariff   The ::sl_zigbee_af_scheduled_tariff_t structure describing the tariff.
 *                 If NULL, the tariff is removed from the server.
 * @return         True if the tariff was set or removed, or false if the
 *                 index is invalid.
 */
bool sl_zigbee_af_price_set_tariff_table_entry(uint8_t endpoint,
                                               uint8_t index,
                                               sl_zigbee_af_price_common_info_t *info,
                                               const sl_zigbee_af_scheduled_tariff_t *tariff);

/**
 * @brief Set a price matrix entry by index.
 *
 * This function sets a price matrix and metadata that the plugin
 * will send to clients.
 *
 * @param endpoint The relevant endpoint.
 * @param index    The index in the price matrix table.
 * @param pm       The ::sl_zigbee_af_scheduled_price_matrix_t structure describing the
 *                 price matrix. If NULL, the price matrix is removed from the
 *                 server.
 * @return         True if the price matrix was set or removed, or false if the
 *                 index is invalid.
 */
bool sl_zigbee_af_price_set_price_matrix(uint8_t endpoint,
                                         uint8_t index,
                                         sl_zigbee_af_price_common_info_t *info,
                                         const sl_zigbee_af_scheduled_price_matrix_t *pm);

/**
 * @brief Set the block thresholds used by the price server plugin.
 *
 * This function sets the block thresholds and metadata that the plugin
 * will send to clients.
 *
 * @param endpoint The relevant endpoint.
 * @param index    The index in the block thresholds table.
 * @param bt       The ::sl_zigbee_af_scheduled_block_thresholds_t structure describing the
 *                 block thresholds. If NULL, the block thresholds entry is removed
 *                 from the table.
 * @return         True if the block thresholds was set or removed, or false if the
 *                 index is invalid.
 */
bool sl_zigbee_af_price_set_block_thresholds_table_entry(uint8_t endpoint,
                                                         uint8_t index,
                                                         const sl_zigbee_af_price_common_info_t *info,
                                                         const sl_zigbee_af_scheduled_block_thresholds_t *bt);

/**
 * @brief Get the current price used by the price server plugin.
 *
 * This function gets the current price and metadata that the
 * plugin will send to clients. For "start now" prices, the duration is
 * adjusted to reflect how many minutes remain for the price. Otherwise, the
 * start time and duration reflect the actual start and the original duration.
 *
 * @param endpoint The relevant endpoint.
 * @param price The ::sl_zigbee_af_scheduled_price_t structure describing the price.
 * @return True if the current price was found or false is there is no current
 * price.
 */
bool sl_zigbee_af_get_current_price(uint8_t endpoint, sl_zigbee_af_scheduled_price_t *price);

/**
 * @brief Find the first free index in the price table.
 *
 * This function looks through the price table and determines whether
 * the entry is in-use or scheduled to be in use. If not, it's
 * considered "free" for the purposes of the user adding a new price
 * entry to the server's table and the index is returned.
 *
 * @param endpoint The relevant endpoint.
 * @return The index of the first free (unused/unscheduled) entry in
 * the requested endpoint's price table, or ZCL_PRICE_INVALID_INDEX
 * if no available entry could be found.
 */
uint8_t sl_zigbee_af_price_find_free_price_index(uint8_t endpoint);

/** @brief function to print price
 *
 * @param price
 */
void sl_zigbee_af_price_print(const sl_zigbee_af_scheduled_price_t *price);

/** Print the price table.
 *
 * @param endpoint relevant endpoint
 *
 */
void sl_zigbee_af_price_print_price_table(uint8_t endpoint);

/** Print tariff.
 *
 * @param info pointer to the common info struct
 * @param tarrif pointer to  the scheduled tariff struct
 *
 */
void sl_zigbee_af_price_print_tariff(const sl_zigbee_af_price_common_info_t *info,
                                     const sl_zigbee_af_scheduled_tariff_t *tariff);

/** @brief Print tariff table.
 *
 * @param endpoint relevant endpoint
 *
 */
void sl_zigbee_af_price_print_tariff_table(uint8_t endpoint);

/** @brief Print price matrix.
 *
 * @param endpoint relevant endpoint
 * @param info pointer to common info struct
 * @param pm pointer to scheduled price matrix struct
 *
 */
void sl_zigbee_af_price_print_price_matrix(uint8_t endpoint,
                                           const sl_zigbee_af_price_common_info_t *info,
                                           const sl_zigbee_af_scheduled_price_matrix_t *pm);

/** @brief Print price matrix table.
 *
 * @param endpoint relevant endpoint
 *
 */
void sl_zigbee_af_price_print_price_matrix_table(uint8_t endpoint);

/** @brief Print block thresholds.
 *
 * @param endpoint relevant endpoint
 * @param info pointer to common info struct
 * @param bt pointer to block threshold struct
 *
 */
void sl_zigbee_af_price_print_block_thresholds(uint8_t endpoint,
                                               const sl_zigbee_af_price_common_info_t *info,
                                               const sl_zigbee_af_scheduled_block_thresholds_t *bt);

/** @brief Print block threshold table.
 *
 * @param endpoint relevant endpoint
 *
 */
void sl_zigbee_af_price_print_block_thresholds_table(uint8_t endpoint);

/** @brief Publish price message.
 *
 * @param nodeId
 * @param srcEndpoint
 * @param dstEndpoint
 * @param priceIndex
 *
 */
void sl_zigbee_af_price_server_publish_price_message(sl_802154_short_addr_t nodeId,
                                                     uint8_t srcEndpoint,
                                                     uint8_t dstEndpoint,
                                                     uint8_t priceIndex);

/**
 * @brief Set parameters in the conversion factors table.
 *
 * @param endpoint The endpoint in question.
 * @param issuerEventId The event ID of the conversion factor data.
 * @param startTime The time when the conversion factor data is valid.
 * @param conversionFactor Accounts for changes in the volume of gas
 * based on temperature and pressure.
 * @param conversionFactorTrailingDigit Determines where the decimal
 * point is located in the conversion factor.
 **/
sl_zigbee_af_status_t sl_zigbee_af_price_server_conversion_factor_add(uint8_t endpoint,
                                                                      uint32_t issuerEventId,
                                                                      uint32_t startTime,
                                                                      uint32_t conversionFactor,
                                                                      uint8_t conversionFactorTrailingDigit);
/**
 * @brief Clear the conversion factors table and invalidates all entries.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void sl_zigbee_af_price_server_conversion_factor_clear(uint8_t endpoint);

/**
 * @brief Send a Publish Conversion Factor command using data
 * at the specified table index.
 *
 * @param tableIndex The index of the conversion factor table whose data
 * should be used in the publish conversion factor command.
 * @param dstAddr The destination address to which the command should be sent.
 * @param srcEp The source endpoint used in the transmission.
 * @param dstEp The destination endpoint used in the transmission.
 *
 **/
void sl_zigbee_af_price_server_conversion_factor_pub(uint8_t tableIndex,
                                                     sl_802154_short_addr_t dstAddr,
                                                     uint8_t srcEndpoint,
                                                     uint8_t dstEndpoint);

/**
 * @brief Return the number of seconds until the next conversion factor
 * event becomes active.
 *
 * @param endpoint The endpoint in question
 * @return The number of seconds until the next conversion factor
 * event becomes active.
 *
 **/
uint32_t sl_zigbee_af_price_server_seconds_until_conversion_factor_event(uint8_t endpoint);

/**
 * @brief Refresh the conversion factor information if necessary.
 * If the second conversion factor event is active, the first
 * is inactivated and the array is re-sorted.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void sl_zigbee_af_price_server_refresh_conversion_factor(uint8_t endpoint);

/**
 * @brief Set values in the calorific value table.
 *
 * @param endpoint The endpoint in question
 * @issuerEventId The event ID of the calorific value data.
 * @startTime The time at which the calorific value data is valid.
 * @calorificValue The amount of heat generated when a given mass
 * of fuel is burned.
 * @calorificValueTrailingDigit Determines where the decimal point
 * is located in the calorific value.
 *
 **/
sl_zigbee_af_status_t sl_zigbee_af_price_server_calorific_value_add(uint8_t endpoint,
                                                                    uint32_t issuerEventId,
                                                                    uint32_t startTime,
                                                                    uint32_t calorificValue,
                                                                    uint8_t calorificValueUnit,
                                                                    uint8_t calorificValueTrailingDigit);

/**
 * @brief Return the number of seconds until the next calorific value event will become active.
 *
 * @param endpoint The endpoint in question
 * @return The number of seconds until the next calorific value event becomes active.
 *
 **/
uint32_t sl_zigbee_af_price_server_seconds_until_calorific_value_event(uint8_t endpoint);

/**
 * @brief Refresh the caloric value information if necessary.
 * If the second calorific value event is active, the first is inactivated
 * and the array is re-sorted.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void sl_zigbee_af_price_server_refresh_calorific_value(uint8_t endpoint);

/**
 * @brief Clear the calorific value table and invalidates all entries.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void sl_zigbee_af_price_server_calorific_value_clear(uint8_t endpoint);

/**
 * @brief Send a publish tariff information command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 * @param tariffIndex The index of the tariff table whose data will
 * be used in the Publish Tariff Information command.
 *
 **/
void sl_zigbee_af_price_server_publish_tariff_message(sl_802154_short_addr_t nodeId,
                                                      uint8_t srcEndpoint,
                                                      uint8_t dstEndpoint,
                                                      uint8_t tariffIndex);

/**
 * @brief Print data in the conversion factor table.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void sl_zigbee_af_print_conversion_table(uint8_t endpoint);

/**
 * @brief Print data in the calorific values table.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void sl_zigbee_af_print_calorific_values_table(uint8_t endpoint);

/**
 * @brief Return the number of seconds until the next CO2
 * value event becomes active.
 *
 * @param endpoint The endpoint in question.
 * @return The number of seconds until the next CO2 value event becomes active.
 *
 **/
uint32_t sl_zigbee_af_price_server_seconds_until_co2_value_event(uint8_t endpoint);

/**
 * @brief Refresh the CO2 value information if necessary.
 * If the second CO2 value event is active, the first is inactivated
 * and the array is re-sorted.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void sl_zigbee_af_price_server_refresh_co2_value(uint8_t endpoint);

/**
 * @brief Set values in the CO2 value table.
 *
 * @param endpoint The endpoint in question.
 * @param issuerEventId The event ID of the CO2 value table data.
 * @param startTime The time at which the CO2 value data is valid.
 * @param providerId A unique identifier for the commodity provider.
 * @param tariffType Bitmap identifying the type of tariff published
 * in this command.
 * @param co2Value Used to calculate the amount of carbon dioxide
 * produced from energy use.
 * @param co2ValueUnit An enumeration which defines the unit of the
 * co2Value attribute.
 * @param co2ValueTrailingDigit Determines where the decimal point
 * is located in the co2Value.
 *
 **/
void sl_zigbee_af_price_server_co2_value_add(uint8_t endpoint,
                                             uint32_t issuerEventId,
                                             uint32_t startTime,
                                             uint32_t providerId,
                                             uint8_t tariffType,
                                             uint32_t co2Value,
                                             uint8_t co2ValueUnit,
                                             uint8_t co2ValueTrailingDigit);

/**
 * @brief Clear the CO2 value table and invalidates all entries.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void sl_zigbee_af_price_server_co2_value_clear(uint8_t endpoint);

/**
 * @brief Print data in the CO2 values table.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void sl_zigbee_af_print_co2_values_table(uint8_t endpoint);

/**
 * @brief Send a publish CO2 value command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 * @param index The index of the CO2 values table whose data will be used in the command.
 *
 **/
void sl_zigbee_af_price_server_co2_label_pub(uint16_t nodeId,
                                             uint8_t srcEndpoint,
                                             uint8_t dstEndpoint,
                                             uint8_t index);

/**
 * @brief Set values in the tier label table.
 *
 * @param endpoint The endpoint in question.
 * @param index The index of the billing period table whose data will be modified.
 * @param valid Indicates if data at this index is valid or not.
 * @param providerId A unique identifier for the commodity provider.
 * @param issuerEventId The event ID of the tier labels table data.
 * @param issuerTariffId A unique identifier that identifies which tariff the labels apply to.
 * @param tierId The tier number that associated tier label applies to.
 * @param tierLabel A character string descriptor for this tier.
 *
 **/
void sl_zigbee_af_price_server_tier_label_set(uint8_t  endpoint,
                                              uint8_t  index,
                                              uint8_t  valid,
                                              uint32_t providerId,
                                              uint32_t issuerEventId,
                                              uint32_t issuerTariffId,
                                              uint8_t tierId,
                                              uint8_t* tierLabel);
void sl_zigbee_af_print_print_tier_labels_table(void);

/**
 * @brief Add a tier label to the specified tier label table.
 *
 * @param endpoint The endpoint in question.
 * @param issuerTariffId A unique identifier that identifies which tariff the labels apply to.
 * @param tierId The tier number that associated tier label applies to.
 * @param tierLabel Character string descriptor for this tier.
 *
 **/
void sl_zigbee_af_price_server_tier_label_add_label(uint8_t endpoint,
                                                    uint32_t issuerTariffId,
                                                    uint8_t tierId,
                                                    uint8_t *tierLabel);

/**
 * @brief Print the tier labels table.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void sl_zigbee_af_print_tier_labels_table(uint8_t endpoint);

/**
 * @brief Send a publish tier labels command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 * @param index The index of the tier labels table whose data will
 * be used in the Publish Tier Labels command.
 *
 **/
void sl_zigbee_af_price_server_tier_label_pub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint, uint8_t index);

/**
 * @brief Return the number of seconds until the next billing period
 * event becomes active.
 *
 * @param endpoint The endpoint in question.
 * @return The number of seconds until the next billing period event becomes active.
 *
 **/
uint32_t sl_zigbee_af_price_server_seconds_until_billing_period_event(uint8_t endpoint);

/**
 * @brief Refresh the billing period information if necessary.
 * If the second billing period event is active, the first is inactivated
 * and the array is re-sorted.
 * If the force argument is set to true, the billing period will be forced to refresh.
 *
 * @param endpoint The endpoint in question.
 * @param force    Whether or not the billing period should be forced to refresh.
 *
 **/
void sl_zigbee_af_price_server_refresh_billing_period(uint8_t endpoint, bool force);

/**
 * @brief Set values in the billing period table.
 *
 * @param endpoint The endpoint in question.
 * @param startTime The time at which the billing period data is valid.
 * @param issuerEventId The event ID of the billing period data.
 * @param providerId A unique identifier for the commodity provider.
 * @param billingPeriodDuration The billing period duration.
 * Units are specified by the billingPeriodDurationType.
 * @param billingPeriodDurationType A bitmap that indicates
 * the units used in the billing period.
 * @param tariffType Bitmap identifying the type of tariff published in this command.
 *
 **/
sl_status_t sl_zigbee_af_price_server_billing_period_add(uint8_t endpoint,
                                                         uint32_t startTime,
                                                         uint32_t issuerEventId,
                                                         uint32_t providerId,
                                                         uint32_t billingPeriodDuration,
                                                         uint8_t billingPeriodDurationType,
                                                         uint8_t tariffType);
/**
 * @brief Send a publish billing period command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 * @param index The index of the table whose data will be used in the command.
 *
 **/
void sl_zigbee_af_price_server_billing_period_pub(uint16_t nodeId, uint8_t srcEndpoint,
                                                  uint8_t dstEndpoint, uint8_t index);

/**
 * @brief Print data in the billing period table for the specified endpoint.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void sl_zigbee_af_print_billing_period_table(uint8_t endpoint);

/**
 * @brief Print data in the consolidated bills table at the specified index.
 *
 * @param endpoint The endpoint in question.
 * @param index The index of the consolidated bills table whose data should be printed.
 *
 **/
void sl_zigbee_af_print_consolidated_bill_table_entry(uint8_t endpoint, uint8_t index);

/**
 * @brief Set values in the consolidated bills table.
 *
 * @param endpoint The endpoint in question.
 * @param startTime The time at which the consolidated bills data is valid.
 * @param issuerEventId The event ID of the consolidated bills data.
 * @param providerId A unique identifier for the commodity provider.
 * @param billingPeriodDuration The billing period duration. Units are specified by
 * the billingPeriodDurationType.
 * @param billingPeriodDurationType A bitmap that indicates the units used in the billing period.
 * @param tariffType Bitmap identifying the type of tariff published in this command.
 * @param consolidatedBill The consolidated bill value for the specified billing period.
 * @param currency The currency used in the consolidatedBill field.
 * @param billTrailingDigit Determines where the decimal point is located
 * in the consolidatedBill field.
 *
 **/
void sl_zigbee_af_price_server_consolidated_bill_add(uint8_t endpoint, uint32_t startTime,
                                                     uint32_t issuerEventId, uint32_t providerId,
                                                     uint32_t billingPeriodDuration, uint8_t billingPeriodDurationType,
                                                     uint8_t tariffType, uint32_t consolidatedBill,
                                                     uint16_t currency, uint8_t billTrailingDigit);

/**
 * @brief Send a publish consolidated bill command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 * @param index The index of the table whose data will be used in the command.
 *
 **/
void sl_zigbee_af_price_server_consolidated_bill_pub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint, uint8_t index);

/**
 * @brief Set values of the CPP event.
 *
 * @param endpoint The endpoint in question.
 * @param valid Indicates if the CPP Event data is valid or not.
 * @param providerId A unique identifier for the commodity provider.
 * @param issuerEventId The event ID of the CPP Event.
 * @param startTime The time at which the CPP Event data is valid.
 * @param durationInMinutes Defines the duration of the CPP Event.
 * @param tariffType Bitmap identifying the type of tariff published in this command.
 * @param cppPriceTier Indicates which CPP price tier should be used for the event.
 * @param cppAuth The status of the CPP event.
 *
 **/
void sl_zigbee_af_price_server_cpp_event_set(uint8_t endpoint, uint8_t valid, uint32_t providerId, uint32_t issuerEventId, uint32_t startTime,
                                             uint16_t durationInMinutes, uint8_t tariffType, uint8_t cppPriceTier, uint8_t cppAuth);

/**
 * @brief Send a publish CPP event command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 *
 **/
void sl_zigbee_af_price_server_cpp_event_pub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint);

/**
 * @brief Print data in the CPP Event.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void sl_zigbee_af_price_server_cpp_event_print(uint8_t endpoint);

/**
 * @brief Send a publish credit payment command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 * @param index The index of the table whose data will be used in the command.
 *
 **/
void sl_zigbee_af_price_server_credit_payment_pub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint, uint8_t index);

/**
 * @brief Set values in the credit payment table.
 *
 * @param endpoint The endpoint in question.
 * @param index The index of the credit payment table whose data will be modified.
 * @param valid Indicates if data at this index is valid or not.
 * @param providerId A unique identifier for the commodity provider.
 * @param issuerEventId The event ID of the credit payment data.
 * @param creditPaymentDueDate The time the next credit payment is due.
 * @param creditPaymentOverdueAmount The current amount that is overdue from the customer.
 * @param creditPaymentStatus Indicates the current credit payment status.
 * @param creditPayment The amount of the last credit payment.
 * @param creditPaymentDate The time at which the last credit payment was made.
 * @param creditPaymentRef A string used to denote the last credit payment
 * reference used by the energy supplier.
 *
 **/
void sl_zigbee_af_price_server_credit_payment_set(uint8_t endpoint, uint8_t index, uint8_t valid,
                                                  uint32_t providerId, uint32_t issuerEventId,
                                                  uint32_t creditPaymentDueDate, uint32_t creditPaymentOverdueAmount,
                                                  uint8_t creditPaymentStatus, uint32_t creditPayment,
                                                  uint32_t creditPaymentDate, uint8_t *creditPaymentRef);

//void sl_zigbee_af_price_server_credit_payment_print( void );

/**
 * @brief Send a publish currency conversion command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 *
 **/
void sl_zigbee_af_price_server_currency_conversion_pub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint);

/**
 * @brief Set values for the currency conversion command.
 *
 * @param endpoint The endpoint in question.
 * @param valid Indicates if the currency conversion data is valid or not.
 * @param providerId A unique identifier for the commodity provider.
 * @param issuerEventId The event ID of the currency conversion data.
 * @param startTime The time at which the currency conversion data is valid.
 * @param oldCurrency Information about the old unit of currency.
 * @param newCurrency Information about the new unit of currency.
 * @param conversionFactor Accounts for changes in the volume of gas based on temperature and pressure.
 * @param conversionFactorTrailingDigit Determines where the decimal point is located in the conversion factor.
 * @param currencyChangeControlFlags Denotes functions that are required to be carried out by the client.
 *
 **/
void sl_zigbee_af_price_server_currency_conversion_set(uint8_t endpoint, uint8_t valid,
                                                       uint32_t providerId, uint32_t issuerEventId,
                                                       uint32_t startTime, uint16_t oldCurrency, uint16_t newCurrency,
                                                       uint32_t conversionFactor, uint8_t conversionFactorTrailingDigit,
                                                       uint32_t currencyChangeControlFlags);

/**
 * @brief Set values in the tariff cancellation command.
 *
 * @param endpoint The endpoint in question.
 * @param valid Indicates if the tariff cancellation command is valid or not.
 * @param providerId A unique identifier for the commodity provider.
 * @param issuerTariffId Unique identifier that identifies which tariff should be canceled.
 * @param tariffType A bitmap identifying the type of tariff to be canceled.
 *
 **/
void sl_zigbee_af_price_server_tariff_cancellation_set(uint8_t endpoint, uint8_t valid, uint32_t providerId,
                                                       uint32_t issuerTariffId, uint8_t tariffType);

/**
 * @brief Send a cancel tariff command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 *
 **/
void sl_zigbee_af_price_server_tariff_cancellation_pub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint);

/** Get seconds until the next tariff info event.
 *
 * @param endpoint relevant endpoint
 *
 * @return unit32_t seconds until next tariff info event
 *
 */
uint32_t sl_zigbee_af_price_server_seconds_until_tariff_info_event(uint8_t endpoint);

/** @brief Refresh tariff information.
 *
 * @param endpoint relevant endpoint
 *
 */
void sl_zigbee_af_price_server_refresh_tariff_information(uint8_t endpoint);

/** @brief Add a tariff table entry.
 *
 * @param endpoint relevant endpoint
 * @param info pointer to common info struct
 * @param curTariff pointer to scheduled tariff struct
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_price_add_tariff_table_entry(uint8_t endpoint,
                                               sl_zigbee_af_price_common_info_t *info,
                                               const sl_zigbee_af_scheduled_tariff_t *curTariff);

/** @brief Add price matrix (raw).
 *
 * @param endpoint relevant endpoint
 * @param providerId
 * @param issuerEventId
 * @param commandIndex
 * @param numberOfCommands
 * @param subPayloadCommands
 * @param payload
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_price_add_price_matrix_raw(uint8_t endpoint,
                                             uint32_t providerId,
                                             uint32_t issuerEventId,
                                             uint32_t startTime,
                                             uint32_t issuerTariffId,
                                             uint8_t commandIndex,
                                             uint8_t numberOfCommands,
                                             uint8_t subPayloadControl,
                                             uint8_t* payload);

/** @brief Add price matrix.
 *
 * @param endpoint relevant endpoint
 * @param info pointer to the common info struct
 * @param pm pointer to the scheduled price matrix struct
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_price_add_price_matrix(uint8_t endpoint,
                                         sl_zigbee_af_price_common_info_t *info,
                                         sl_zigbee_af_scheduled_price_matrix_t * pm);
/** Add a table entry for block threshold table.
 *
 * @param endpoint relevant endpoint
 * @param providerId
 * @param isserEventId
 * @param startTime
 * @param isserTarrifId
 * @param commandIndex
 * @param numberOfCommands
 * @param subpayloadControl
 * @param payload
 *
 * @return bool true is success
 */
bool sl_zigbee_af_price_add_block_thresholds_table_entry(uint8_t endpoint,
                                                         uint32_t providerId,
                                                         uint32_t issuerEventId,
                                                         uint32_t startTime,
                                                         uint32_t issuerTariffId,
                                                         uint8_t commandIndex,
                                                         uint8_t numberOfCommands,
                                                         uint8_t subpayloadControl,
                                                         uint8_t* payload);

/** @brief Clean the block period table.
 *
 * @param endpoint relevant endpoint
 *
 */
void sl_zigbee_af_price_clear_block_period_table(uint8_t endpoint);
void sendValidCmdEntries(uint8_t cmdId,
                         uint8_t endpoint,
                         uint8_t* validEntries,
                         uint8_t validEntryCount);

/** @brief Update bindings.
 *
 */
void sl_zigbee_af_price_server_price_update_bindings(void);

/** @brief Get seconds until next active price matrix event.
 *
 * @param endpoint relevant endpoint
 *
 * @return uint32_t seconds until next price matrix event
 *
 */
uint32_t sl_zigbee_af_price_server_seconds_until_active_price_matrix_event(uint8_t endpoint);

/** @brief Refresh price matrix information.
 *
 * @param endpoint relevant endpoint
 *
 */
void sl_zigbee_af_price_server_refresh_price_matrix_information(uint8_t endpoint);

/** @brief Get seconds until next active block threshold event.
 *
 * @param endpoint relevant endpoint
 *
 * @return uint32_t seconds until next block threshold event
 *
 */
uint32_t sl_zigbee_af_price_server_seconds_until_active_block_thresholds_event(uint8_t endpoint);

/** @brief Refresh block threshold information.
 *
 * @param endpoint relevant endpoint
 *
 */
void sl_zigbee_af_price_server_refresh_block_thresholds_information(uint8_t endpoint);

/** @} */ // end of name API

/**
 * @name Callbacks
 * @{
 */

/**
 * @defgroup price_server_cb Price Server
 * @ingroup af_callback
 * @brief Callbacks for Price Server Component
 *
 */

/**
 * @addtogroup price_server_cb
 * @{
 */

/** @brief Initialize he price cluster server.
 *
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void sl_zigbee_af_price_cluster_server_init_cb(uint8_t endpoint);

/** @brief Notify the application that a new PriceMatrix has
 * become available.
 *
 * The application is responsible for updating proper ZCL
 * attributes to reflect the necessary vendor-specific behaviors.
 * priceCommonInfo contains common information, such as startTime.
 * priceMatrix  contains the PriceMatrix information.
 *
 * @param priceCommonInfo   Ver.: always
 * @param priceMatrix   Ver.: always
 */
void sl_zigbee_af_price_server_new_active_price_matrix_cb(const sl_zigbee_af_price_common_info_t *priceCommonInfo,
                                                          const sl_zigbee_af_scheduled_price_matrix_t *priceMatrix);
/** @brief Notify the application that a new tariff
 * information has become available.
 *
 * The application is responsible for updating
 * proper ZCL attributes to reflect the necessary vendor-specific behaviors.
 * priceCommonInfo contains common information, such as
 * startTime. priceTariffInfo contains the price matrix
 * information.
 *
 * @param priceCommonInfo   Ver.: always
 * @param priceTariffInfo   Ver.: always
 */
void sl_zigbee_af_price_server_new_active_tariff_information_cb(const sl_zigbee_af_price_common_info_t *priceCommonInfo,
                                                                const sl_zigbee_af_scheduled_tariff_t *priceTariffInfo);
/** @brief Notify the application that a new block thresholds
 * information has become available.
 *
 * The application is responsible for updating
 * proper ZCL attributes to reflect the necessary vendor-specific behaviors.
 * priceCommonInfo contains common information, such as
 * startTime. priceTariffInfo  contains the
 * BlockThresholds information.
 *
 * @param priceCommonInfo   Ver.: always
 * @param priceBlockThresholds   Ver.: always
 */
void sl_zigbee_af_price_server_new_active_block_thresholds_information_cb(const sl_zigbee_af_price_common_info_t *priceCommonInfo,
                                                                          const sl_zigbee_af_scheduled_block_thresholds_t *priceBlockThresholds);
/** @brief Notify the application that a new block period
 * information has become available.
 *
 * The application is responsible for updating
 * proper ZCL attributes to reflect the necessary vendor-specific behaviors.
 * priceCommonInfo contains common information, such as
 * startTime. priceBlockPeriod contains the BlockPeriod
 * information.
 *
 * @param priceCommonInfo   Ver.: always
 * @param priceBlockPeriods   Ver.: always
 */
void sl_zigbee_af_price_server_new_active_block_period_information_cb(const sl_zigbee_af_price_common_info_t *priceCommonInfo,
                                                                      const sl_zigbee_af_price_block_period_t *priceBlockPeriods);
/** @} */ // end of price_server_cb
/** @} */ // end of Callbacks
/** @} */ // end of price-server

extern bool sli_zigbee_af_price_server_billing_period_repeat;

#endif  // #ifndef _PRICE_SERVER_H_
