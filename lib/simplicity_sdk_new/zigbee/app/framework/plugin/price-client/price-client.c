/***************************************************************************//**
 * @file
 * @brief The Price client plugin is responsible for keeping track of the current
 * and future prices.
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
#include "app/framework/plugin/price-common/price-common.h"
#include "app/framework/plugin/price-common/price-common-time.h"
#include "price-client.h"

#include "zap-cluster-command-parser.h"

#define VALID  BIT(1)
#define CANCELLATION_START_TIME  0xFFFFFFFF

// keep track of last seen issuerEventId to discard old commands.
static uint32_t lastSeenIssuerEventId = 0x00;
static sl_zigbee_af_price_client_info_t priceInfo;

// Used by the CLI and CppEventCallback to determine the default policy for non-forced CPP Events.
uint8_t sl_zigbee_af_price_cluster_default_cpp_event_authorization = SL_ZIGBEE_AF_PLUGIN_PRICE_CPP_AUTH_ACCEPTED;

static void initPrice(sl_zigbee_af_plugin_price_client_price_t *price);
static void printPrice(sl_zigbee_af_plugin_price_client_price_t *price);
static void scheduleTick(uint8_t endpoint);
static void sl_zigbee_af_price_init_block_period(uint8_t endpoint);
static void sl_zigbee_af_price_init_block_thresholds_table(uint8_t endpoint);
static void sl_zigbee_af_price_init_conversion_factor_table(uint8_t endpoint);
static void sl_zigbee_af_price_init_calorific_value_table(uint8_t endpoint);
static void sl_zigbee_af_price_cluster_init_cpp(uint8_t endpoint);
static void sl_zigbee_af_price_cluster_init_co2_table(uint8_t endpoint);
static void sli_zigbee_af_price_client_tier_labels_init(uint8_t endpoint);
static void sl_zigbee_af_price_cluster_init_credit_payment_table(uint8_t endpoint);
static void sl_zigbee_af_price_cluster_init_currency_conversion_table(uint8_t endpoint);
static void sl_zigbee_af_price_init_billing_period_table(uint8_t endpoint);
static void sli_zigbee_af_price_client_add_tier_label(uint8_t endpoint, uint32_t providerId, uint32_t issuerEventId, uint32_t issuerTariffId,
                                                      uint8_t numberOfLabels, uint8_t *tierLabelsPayload);

static uint8_t sli_zigbee_af_price_common_get_matching_or_unused_index(sl_zigbee_af_price_client_common_info_t *pcommon, uint8_t numElements,
                                                                       uint32_t newProviderId, uint32_t newIssuerEventId);
static uint8_t sli_zigbee_af_price_common_get_matching_index(sl_zigbee_af_price_client_common_info_t *pcommon, uint8_t numElements, uint32_t issuerEventId);
static uint8_t sli_zigbee_af_price_common_get_active_index(sl_zigbee_af_price_client_common_info_t *pcommon, uint8_t numElements);

static sl_zigbee_af_plugin_price_client_price_t priceTable[SL_ZIGBEE_ZCL_PRICE_CLUSTER_CLIENT_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_TABLE_SIZE];

void sli_zigbee_af_price_clear_price_table(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint,
                                                               ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    assert(false);
    return;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_TABLE_SIZE; i++) {
    initPrice(&priceTable[ep][i]);
  }
}

void sl_zigbee_af_price_cluster_client_init_cb(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint,
                                                               ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_TABLE_SIZE; i++) {
    initPrice(&priceTable[ep][i]);
  }
  sl_zigbee_af_price_init_consolidated_bills_table(endpoint);
  sli_zigbee_af_price_client_tier_labels_init(endpoint);
  sl_zigbee_af_price_init_block_period(endpoint);
  sl_zigbee_af_price_init_block_thresholds_table(endpoint);
  sl_zigbee_af_price_cluster_init_credit_payment_table(endpoint);
  sl_zigbee_af_price_cluster_init_currency_conversion_table(endpoint);
  sl_zigbee_af_price_cluster_init_cpp(endpoint);
  sl_zigbee_af_price_cluster_init_co2_table(endpoint);
  sl_zigbee_af_price_init_conversion_factor_table(endpoint);
  sl_zigbee_af_price_init_calorific_value_table(endpoint);
  sl_zigbee_af_price_init_billing_period_table(endpoint);
}

void sl_zigbee_af_price_cluster_client_tick_cb(uint8_t endpoint)
{
  scheduleTick(endpoint);
}

//-----------------------
// ZCL commands callbacks

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_price_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_price_command_t cmd_data;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint,
                                                               ZCL_PRICE_CLUSTER_ID);
  sl_zigbee_af_plugin_price_client_price_t *price = NULL, *last;
  sl_zigbee_af_status_t status;
  uint32_t endTime, now = sl_zigbee_af_get_current_time();
  uint8_t i;

  if ((zcl_decode_price_cluster_publish_price_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == 0xFF) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  last = &priceTable[ep][0];

  sl_zigbee_af_price_cluster_print("RX: PublishPrice 0x%08X, \"", cmd_data.providerId);
  sl_zigbee_af_price_cluster_print_string(cmd_data.rateLabel);
  sl_zigbee_af_price_cluster_print("\"");
  sl_zigbee_af_price_cluster_flush();
  sl_zigbee_af_price_cluster_print(", 0x%08X, 0x%08X, 0x%02X, 0x%04X, 0x%02X, 0x%02X, 0x%08X",
                                   cmd_data.issuerEventId,
                                   cmd_data.currentTime,
                                   cmd_data.unitOfMeasure,
                                   cmd_data.currency,
                                   cmd_data.priceTrailingDigitAndPriceTier,
                                   cmd_data.numberOfPriceTiersAndRegisterTier,
                                   cmd_data.startTime);
  sl_zigbee_af_price_cluster_flush();
  sl_zigbee_af_price_cluster_print(", 0x%04X, 0x%08X, 0x%02X, 0x%08X, 0x%02X, 0x%08X, 0x%02X",
                                   cmd_data.durationInMinutes,
                                   cmd_data.price,
                                   cmd_data.priceRatio,
                                   cmd_data.generationPrice,
                                   cmd_data.generationPriceRatio,
                                   cmd_data.alternateCostDelivered,
                                   cmd_data.alternateCostUnit);
  sl_zigbee_af_price_cluster_flush();
  sl_zigbee_af_price_cluster_println(", 0x%02X, 0x%02X, 0x%02X",
                                     cmd_data.alternateCostTrailingDigit,
                                     cmd_data.numberOfBlockThresholds,
                                     cmd_data.priceControl);
  sl_zigbee_af_price_cluster_flush();

  if (cmd_data.startTime == ZCL_PRICE_CLUSTER_START_TIME_NOW) {
    cmd_data.startTime = now;
  }
  endTime = (cmd_data.durationInMinutes == ZCL_PRICE_CLUSTER_DURATION_UNTIL_CHANGED
             ? ZCL_PRICE_CLUSTER_END_TIME_NEVER
             : cmd_data.startTime + cmd_data.durationInMinutes * 60);

  // If the price has already expired, don't bother with it.
  if (endTime <= now) {
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
    goto kickout;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_TABLE_SIZE; i++) {
    // Ignore invalid prices, but remember the empty slot for later.
    if (!priceTable[ep][i].valid) {
      if (price == NULL) {
        price = &priceTable[ep][i];
      }
      continue;
    }

    // Reject duplicate prices based on the issuer event id.  This assumes that
    // issuer event ids are unique and that pricing priceInformation associated with
    // an issuer event id never changes.
    if (priceTable[ep][i].issuerEventId == cmd_data.issuerEventId) {
      status = SL_ZIGBEE_ZCL_STATUS_DUPLICATE_EXISTS;
      goto kickout;
    }

    // Nested and overlapping prices are not allowed.  Prices with the newer
    // issuer event ids takes priority over all nested and overlapping prices.
    // The only exception is when a price with a newer issuer event id overlaps
    // with the end of the current active price.  In this case, the duration of
    // the current active price is changed to "until changed" and it will expire
    // when the new price starts.
    if (priceTable[ep][i].startTime < endTime
        && priceTable[ep][i].endTime > cmd_data.startTime) {
      if (priceTable[ep][i].issuerEventId < cmd_data.issuerEventId) {
        if (priceTable[ep][i].active && now < cmd_data.startTime) {
          priceTable[ep][i].endTime = cmd_data.startTime;
          priceTable[ep][i].durationInMinutes = ZCL_PRICE_CLUSTER_DURATION_UNTIL_CHANGED;
        } else {
          if (priceTable[ep][i].active) {
            priceTable[ep][i].active = false;
            sl_zigbee_af_price_client_price_expired_cb(&priceTable[ep][i]);
          }
          initPrice(&priceTable[ep][i]);
        }
      } else {
        status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
        goto kickout;
      }
    }

    // Along the way, search for an empty slot for this new price and find the
    // price in the table with the latest start time.  If there are no empty
    // slots, we will either have to drop this price or the last one, depending
    // on the start times.
    if (price == NULL) {
      if (!priceTable[ep][i].valid) {
        price = &priceTable[ep][i];
      } else if (last->startTime < priceTable[ep][i].startTime) {
        last = &priceTable[ep][i];
      }
    }
  }

  // If there were no empty slots and this price starts after all of the other
  // prices in the table, drop this price.  Otherwise, drop the price with the
  // latest start time and replace it with this one.
  if (price == NULL) {
    if (last->startTime < cmd_data.startTime) {
      status = SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
      goto kickout;
    } else {
      price = last;
    }
  }

  price->valid                             = true;
  price->active                            = false;
  price->clientEndpoint                    = endpoint;
  price->providerId                        = cmd_data.providerId;
  sl_zigbee_af_copy_string(price->rateLabel, cmd_data.rateLabel, ZCL_PRICE_CLUSTER_MAXIMUM_RATE_LABEL_LENGTH);
  price->issuerEventId                     = cmd_data.issuerEventId;
  price->currentTime                       = cmd_data.currentTime;
  price->unitOfMeasure                     = cmd_data.unitOfMeasure;
  price->currency                          = cmd_data.currency;
  price->priceTrailingDigitAndPriceTier    = cmd_data.priceTrailingDigitAndPriceTier;
  price->numberOfPriceTiersAndRegisterTier = cmd_data.numberOfPriceTiersAndRegisterTier;
  price->startTime                         = cmd_data.startTime;
  price->endTime                           = endTime;
  price->durationInMinutes                 = cmd_data.durationInMinutes;
  price->price                             = cmd_data.price;
  price->priceRatio                        = cmd_data.priceRatio;
  price->generationPrice                   = cmd_data.generationPrice;
  price->generationPriceRatio              = cmd_data.generationPriceRatio;
  price->alternateCostDelivered            = cmd_data.alternateCostDelivered;
  price->alternateCostUnit                 = cmd_data.alternateCostUnit;
  price->alternateCostTrailingDigit        = cmd_data.alternateCostTrailingDigit;
  price->numberOfBlockThresholds           = cmd_data.numberOfBlockThresholds;
  price->priceControl                      = cmd_data.priceControl;

  // Now that we have saved the price in our table, we may have to reschedule
  // our tick to activate or expire prices.
  scheduleTick(endpoint);

  // If the acknowledgement is required, send it immediately.  Otherwise, a
  // default response is sufficient.
  if (cmd_data.priceControl & ZCL_PRICE_CLUSTER_PRICE_ACKNOWLEDGEMENT_MASK) {
    sl_zigbee_af_fill_command_price_cluster_price_acknowledgement(cmd_data.providerId,
                                                                  cmd_data.issuerEventId,
                                                                  now,
                                                                  cmd_data.priceControl);
    sl_zigbee_af_send_response();
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  } else {
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  kickout:
  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_block_period_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_block_period_command_t cmd_data;
  uint8_t i;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ((zcl_decode_price_cluster_publish_block_period_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == 0xFF) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_price_cluster_println("RX: PublishBlockPeriod, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%02X, 0x%02X, 0x%02X, 0x%02X",
                                     cmd_data.providerId,
                                     cmd_data.issuerEventId,
                                     cmd_data.blockPeriodStartTime,
                                     cmd_data.blockPeriodDuration, // int24u
                                     cmd_data.blockPeriodControl,
                                     cmd_data.blockPeriodDurationType,
                                     cmd_data.tariffType,
                                     cmd_data.tariffResolutionPeriod);

  if ( cmd_data.blockPeriodStartTime == 0 ) {
    cmd_data.blockPeriodStartTime = sl_zigbee_af_get_current_time();
  }

  // Find the entry to update
  // We will update any invalid entry, or the oldest event ID.
  i = sli_zigbee_af_price_common_get_matching_or_unused_index(priceInfo.blockPeriodTable.commonInfos[ep],
                                                              SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_BLOCK_PERIOD_TABLE_SIZE,
                                                              cmd_data.providerId, cmd_data.issuerEventId);
  if ( i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_BLOCK_PERIOD_TABLE_SIZE ) {
    priceInfo.blockPeriodTable.commonInfos[ep][i].valid = true;
    priceInfo.blockPeriodTable.commonInfos[ep][i].providerId = cmd_data.providerId;
    priceInfo.blockPeriodTable.commonInfos[ep][i].issuerEventId = cmd_data.issuerEventId;
    priceInfo.blockPeriodTable.commonInfos[ep][i].startTime = sl_zigbee_af_price_common_cluster_get_adjusted_start_time(cmd_data.blockPeriodStartTime,
                                                                                                                        cmd_data.blockPeriodDurationType);
    priceInfo.blockPeriodTable.commonInfos[ep][i].durationSec =
      sl_zigbee_af_price_common_cluster_convert_duration_to_seconds(cmd_data.blockPeriodStartTime,
                                                                    cmd_data.blockPeriodDuration,
                                                                    cmd_data.blockPeriodDurationType);
    priceInfo.blockPeriodTable.blockPeriod[ep][i].blockPeriodStartTime = cmd_data.blockPeriodStartTime;
    priceInfo.blockPeriodTable.blockPeriod[ep][i].blockPeriodDuration = cmd_data.blockPeriodDuration;
    priceInfo.blockPeriodTable.blockPeriod[ep][i].blockPeriodControl = cmd_data.blockPeriodControl;
    priceInfo.blockPeriodTable.blockPeriod[ep][i].blockPeriodDurationType = cmd_data.blockPeriodDurationType;
    priceInfo.blockPeriodTable.blockPeriod[ep][i].tariffType = cmd_data.tariffType;
    priceInfo.blockPeriodTable.blockPeriod[ep][i].tariffResolutionPeriod = cmd_data.tariffResolutionPeriod;
  }

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_conversion_factor_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_conversion_factor_command_t cmd_data;
  uint8_t  i;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ((zcl_decode_price_cluster_publish_conversion_factor_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == 0xFF) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_price_cluster_println("RX: PublishConversionFactor 0x%08X, 0x%08X, 0x%08X, 0x%02X",
                                     cmd_data.issuerEventId,
                                     cmd_data.startTime,
                                     cmd_data.conversionFactor,
                                     cmd_data.conversionFactorTrailingDigit);

  if ( cmd_data.startTime == 0 ) {
    cmd_data.startTime = sl_zigbee_af_get_current_time();
  }

  i = sli_zigbee_af_price_common_get_matching_or_unused_index(priceInfo.conversionFactorTable.commonInfos[ep],
                                                              SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CONVERSION_FACTOR_TABLE_SIZE,
                                                              UNSPECIFIED_PROVIDER_ID,
                                                              cmd_data.issuerEventId);
  if ( i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CONVERSION_FACTOR_TABLE_SIZE ) {
    // Only update the entry if the new eventID is greater than this one.
    priceInfo.conversionFactorTable.commonInfos[ep][i].valid = true;
    priceInfo.conversionFactorTable.commonInfos[ep][i].providerId = UNSPECIFIED_PROVIDER_ID;
    priceInfo.conversionFactorTable.commonInfos[ep][i].issuerEventId = cmd_data.issuerEventId;
    priceInfo.conversionFactorTable.commonInfos[ep][i].startTime = cmd_data.startTime;
    priceInfo.conversionFactorTable.commonInfos[ep][i].durationSec = UNSPECIFIED_DURATION;
    priceInfo.conversionFactorTable.conversionFactor[ep][i].conversionFactor = cmd_data.conversionFactor;
    priceInfo.conversionFactorTable.conversionFactor[ep][i].conversionFactorTrailingDigit = cmd_data.conversionFactorTrailingDigit;
  }
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_calorific_value_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_calorific_value_command_t cmd_data;
  uint8_t  i;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ((zcl_decode_price_cluster_publish_calorific_value_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == 0xFF) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_price_cluster_println("RX: PublishCalorificValue 0x%08X, 0x%08X, 0x%08X, 0x%02X, 0x%02X",
                                     cmd_data.issuerEventId,
                                     cmd_data.startTime,
                                     cmd_data.calorificValue,
                                     cmd_data.calorificValueUnit,
                                     cmd_data.calorificValueTrailingDigit);

  if ( cmd_data.startTime == 0 ) {
    cmd_data.startTime = sl_zigbee_af_get_current_time();
  }

  i = sli_zigbee_af_price_common_get_matching_or_unused_index(priceInfo.calorificValueTable.commonInfos[ep], SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CALORIFIC_VALUE_TABLE_SIZE,
                                                              UNSPECIFIED_PROVIDER_ID, cmd_data.issuerEventId);

  if ( i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CALORIFIC_VALUE_TABLE_SIZE ) {
    // Only update the entry if the new eventID is greater than this one.
    priceInfo.calorificValueTable.commonInfos[ep][i].valid = true;
    priceInfo.calorificValueTable.commonInfos[ep][i].issuerEventId = cmd_data.issuerEventId;
    priceInfo.calorificValueTable.commonInfos[ep][i].startTime = cmd_data.startTime;
    priceInfo.calorificValueTable.calorificValue[ep][i].calorificValue = cmd_data.calorificValue;
    priceInfo.calorificValueTable.calorificValue[ep][i].calorificValueUnit = cmd_data.calorificValueUnit;
    priceInfo.calorificValueTable.calorificValue[ep][i].calorificValueTrailingDigit = cmd_data.calorificValueTrailingDigit;
  }
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_tariff_information_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_tariff_information_command_t cmd_data;

  if (zcl_decode_price_cluster_publish_tariff_information_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_price_cluster_print("RX: PublishTariffInformation 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%02X, \"",
                                   cmd_data.providerId,
                                   cmd_data.issuerEventId,
                                   cmd_data.issuerTariffId,
                                   cmd_data.startTime,
                                   cmd_data.tariffTypeChargingScheme);

  sl_zigbee_af_price_cluster_print_string(cmd_data.tariffLabel);
  sl_zigbee_af_price_cluster_print("\"");
  sl_zigbee_af_price_cluster_print(", 0x%02X, 0x%02X, 0x%02X, 0x%04X, 0x%02X",
                                   cmd_data.numberOfPriceTiersInUse,
                                   cmd_data.numberOfBlockThresholdsInUse,
                                   cmd_data.unitOfMeasure,
                                   cmd_data.currency,
                                   cmd_data.priceTrailingDigit);
  sl_zigbee_af_price_cluster_println(", 0x%08X, 0x%02X, 0x%08X, 0x%08X",
                                     cmd_data.standingCharge,
                                     cmd_data.tierBlockMode,
                                     cmd_data.blockThresholdMultiplier,
                                     cmd_data.blockThresholdDivisor);
  sl_zigbee_af_price_cluster_flush();

  if (lastSeenIssuerEventId >= cmd_data.issuerEventId) {
    // reject old command.
    sl_zigbee_af_price_cluster_println("Rejected command due to old issuer event id (0x%08X)!",
                                       cmd_data.issuerEventId);
  } else {
    // accept command.
    // optional attributes are not updated for now.
    lastSeenIssuerEventId = cmd_data.issuerEventId;
  }

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

#define BLOCK_THRESHOLD_SUB_PAYLOAD_ALL_TOU_TIERS  (1 << 0)

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_block_thresholds_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_block_thresholds_command_t cmd_data;
  uint8_t tierNumber;
  uint8_t numThresholds;
  uint8_t i;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ((zcl_decode_price_cluster_publish_block_thresholds_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == 0xFF) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }
  sl_zigbee_af_price_cluster_println("RX: PublishBlockThresholds 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%02X, 0x%02X, 0x%02X",
                                     cmd_data.providerId,
                                     cmd_data.issuerEventId,
                                     cmd_data.startTime,
                                     cmd_data.issuerTariffId,
                                     cmd_data.commandIndex,
                                     cmd_data.numberOfCommands,
                                     cmd_data.subPayloadControl);

  i = sli_zigbee_af_price_common_get_matching_or_unused_index(priceInfo.blockThresholdTable.commonInfos[ep],
                                                              cmd_data.numberOfCommands, cmd_data.providerId, cmd_data.issuerEventId);

  if ( i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_BLOCK_THRESHOLD_TABLE_SIZE ) {
    // Only update the entry if the new eventID is greater than this one.
    tierNumber = cmd_data.payload[0] >> 4;
    numThresholds = cmd_data.payload[0] & 0x0F;
    if ( (numThresholds >= SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_MAX_NUMBER_BLOCK_THRESHOLDS)
         || (tierNumber >= SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_MAX_NUMBER_TIERS) ) {
      // Out of range
      return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
    }
    priceInfo.blockThresholdTable.commonInfos[ep][i].valid = true;
    priceInfo.blockThresholdTable.commonInfos[ep][i].providerId = cmd_data.providerId;
    priceInfo.blockThresholdTable.commonInfos[ep][i].issuerEventId = cmd_data.issuerEventId;
    priceInfo.blockThresholdTable.commonInfos[ep][i].startTime = cmd_data.startTime;
    priceInfo.blockThresholdTable.commonInfos[ep][i].durationSec = UNSPECIFIED_DURATION;
    priceInfo.blockThresholdTable.blockThreshold[ep][i].issuerTariffId = cmd_data.issuerTariffId;
    priceInfo.blockThresholdTable.blockThreshold[ep][i].subPayloadControl = cmd_data.subPayloadControl;
    if ( cmd_data.subPayloadControl & BLOCK_THRESHOLD_SUB_PAYLOAD_ALL_TOU_TIERS ) {
      priceInfo.blockThresholdTable.blockThreshold[ep][i].tierNumberOfBlockThresholds[0] = cmd_data.payload[0];
      memcpy(priceInfo.blockThresholdTable.blockThreshold[ep][i].blockThreshold[0], &cmd_data.payload[1], (numThresholds * 6) );
    } else {
      // TODO:   Note that multiple tier/NumberOfBlockThresholds fields could be present in this case.
      // eg. The payload could specify 5 blocks for tier 1, 2 blocks for tier 2, etc.
      // However, we don't know how many bytes are in "payload", so we can't read them all.  =(
      // For now, read the first tiers worth of data.
      //x = 0;

      priceInfo.blockThresholdTable.blockThreshold[ep][i].tierNumberOfBlockThresholds[tierNumber] = cmd_data.payload[0];
      memcpy(priceInfo.blockThresholdTable.blockThreshold[ep][i].blockThreshold[tierNumber], &cmd_data.payload[1], (numThresholds * 6) );
    }
  }

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_co2_value_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_co2_value_command_t cmd_data;
  uint8_t  i;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ((zcl_decode_price_cluster_publish_co2_value_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == 0xFF) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_price_cluster_println("RX: Publish CO2 Value");

  if ( cmd_data.startTime == 0 ) {
    cmd_data.startTime = sl_zigbee_af_get_current_time();
  }
  i = sli_zigbee_af_price_common_get_matching_or_unused_index(priceInfo.co2ValueTable.commonInfos[ep], SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CO2_TABLE_SIZE,
                                                              cmd_data.providerId, cmd_data.issuerEventId);

  if ( i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CO2_TABLE_SIZE ) {
    // Do NOT overwrite data if the entry is valid and the new
    // data has a smaller (or equal) event ID.
    priceInfo.co2ValueTable.commonInfos[ep][i].providerId = cmd_data.providerId;
    priceInfo.co2ValueTable.commonInfos[ep][i].issuerEventId = cmd_data.issuerEventId;
    priceInfo.co2ValueTable.commonInfos[ep][i].startTime = cmd_data.startTime;
    priceInfo.co2ValueTable.co2Value[ep][i].tariffType = cmd_data.tariffType;
    priceInfo.co2ValueTable.commonInfos[ep][i].valid = true;
    priceInfo.co2ValueTable.co2Value[ep][i].cO2Value = cmd_data.cO2Value;
    priceInfo.co2ValueTable.co2Value[ep][i].cO2ValueUnit = cmd_data.cO2ValueUnit;
    priceInfo.co2ValueTable.co2Value[ep][i].cO2ValueTrailingDigit = cmd_data.cO2ValueTrailingDigit;
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_tier_labels_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_tier_labels_command_t cmd_data;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();

  if (zcl_decode_price_cluster_publish_tier_labels_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_price_cluster_println("RX: PublishTierLabels");
  sli_zigbee_af_price_client_add_tier_label(endpoint, cmd_data.providerId,
                                            cmd_data.issuerEventId, cmd_data.issuerTariffId,
                                            cmd_data.numberOfLabels, cmd_data.tierLabelsPayload);

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_billing_period_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_billing_period_command_t cmd_data;
  uint8_t  i;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ((zcl_decode_price_cluster_publish_billing_period_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == 0xFF) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_price_cluster_println("RX: PublishBillingPeriod, issuerId=%d,  start=0x%08X", cmd_data.issuerEventId, cmd_data.billingPeriodStartTime);

  if ( cmd_data.billingPeriodStartTime == 0 ) {
    cmd_data.billingPeriodStartTime = sl_zigbee_af_get_current_time();
  }

  i = sli_zigbee_af_price_common_get_matching_or_unused_index(priceInfo.billingPeriodTable.commonInfos[ep],
                                                              SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_BILLING_PERIOD_TABLE_SIZE,
                                                              cmd_data.providerId, cmd_data.issuerEventId);

  if ( i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_BILLING_PERIOD_TABLE_SIZE ) {
    if ( cmd_data.billingPeriodStartTime == CANCELLATION_START_TIME ) {
      priceInfo.billingPeriodTable.commonInfos[ep][i].valid = false;
      sl_zigbee_af_price_cluster_println("Canceling eventId=%d", cmd_data.issuerEventId);
    } else {
      priceInfo.billingPeriodTable.commonInfos[ep][i].valid = true;
      priceInfo.billingPeriodTable.commonInfos[ep][i].providerId = cmd_data.providerId;
      priceInfo.billingPeriodTable.commonInfos[ep][i].issuerEventId = cmd_data.issuerEventId;
      priceInfo.billingPeriodTable.commonInfos[ep][i].startTime =
        sl_zigbee_af_price_common_cluster_get_adjusted_start_time(cmd_data.billingPeriodStartTime,
                                                                  cmd_data.billingPeriodDurationType);
      priceInfo.billingPeriodTable.commonInfos[ep][i].durationSec =
        sl_zigbee_af_price_common_cluster_convert_duration_to_seconds(cmd_data.billingPeriodStartTime,
                                                                      cmd_data.billingPeriodDuration,
                                                                      cmd_data.billingPeriodDurationType);

      priceInfo.billingPeriodTable.billingPeriod[ep][i].billingPeriodStartTime = cmd_data.billingPeriodStartTime;
      priceInfo.billingPeriodTable.billingPeriod[ep][i].billingPeriodDuration = cmd_data.billingPeriodDuration;
      priceInfo.billingPeriodTable.billingPeriod[ep][i].billingPeriodDurationType = cmd_data.billingPeriodDurationType;
      priceInfo.billingPeriodTable.billingPeriod[ep][i].tariffType = cmd_data.tariffType;
    }
  }

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_cpp_event_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_cpp_event_command_t cmd_data;
  uint8_t responseCppAuth;
  sl_zigbee_af_status_t status;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ((zcl_decode_price_cluster_publish_cpp_event_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == 0xFF) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_price_cluster_println("RX: PublishCppEvent:");
  if ( (cmd_data.cppPriceTier > 1) || (cmd_data.cppAuth >= SL_ZIGBEE_AF_PLUGIN_PRICE_CPP_AUTH_RESERVED) ) {
    sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD);
  } else {
    if ( cmd_data.startTime == 0x00000000 ) {
      cmd_data.startTime = sl_zigbee_af_get_current_time();
    } else if ( cmd_data.startTime == CANCELLATION_START_TIME ) {
      // Cancellation attempt.
      if ( (priceInfo.cppEventTable.commonInfos[ep].providerId == cmd_data.providerId)
           && (priceInfo.cppEventTable.commonInfos[ep].issuerEventId == cmd_data.issuerEventId) ) {
        sl_zigbee_af_price_cluster_println("CPP Event Cancelled");
        priceInfo.cppEventTable.commonInfos[ep].valid = false;
        status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
      } else {
        status = SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
      }
      return status;
    }

    // If the cppAuth is PENDING, the client may decide to accept or reject the CPP event.
    if ( cmd_data.cppAuth == SL_ZIGBEE_AF_PLUGIN_PRICE_CPP_AUTH_PENDING ) {
      responseCppAuth = sl_zigbee_af_price_client_pending_cpp_event_cb(cmd_data.cppAuth);
      sl_zigbee_af_price_cluster_println("  Pending CPP Event, status=%d", responseCppAuth);
      sl_zigbee_af_fill_command_price_cluster_cpp_event_response(cmd_data.issuerEventId, responseCppAuth);
      sl_zigbee_af_send_response();
    } else {
      if ( (cmd_data.cppAuth == SL_ZIGBEE_AF_PLUGIN_PRICE_CPP_AUTH_ACCEPTED)
           || (cmd_data.cppAuth == SL_ZIGBEE_AF_PLUGIN_PRICE_CPP_AUTH_FORCED) ) {
        // Apply the CPP
        sl_zigbee_af_price_cluster_println("CPP Event Accepted");
        priceInfo.cppEventTable.commonInfos[ep].providerId = cmd_data.providerId;
        priceInfo.cppEventTable.commonInfos[ep].issuerEventId = cmd_data.issuerEventId;
        priceInfo.cppEventTable.commonInfos[ep].startTime = cmd_data.startTime;
        priceInfo.cppEventTable.commonInfos[ep].durationSec = (cmd_data.durationInMinutes * 60);
        priceInfo.cppEventTable.commonInfos[ep].valid = true;
        priceInfo.cppEventTable.cppEvent[ep].durationInMinutes = cmd_data.durationInMinutes;
        priceInfo.cppEventTable.cppEvent[ep].tariffType = cmd_data.tariffType;
        priceInfo.cppEventTable.cppEvent[ep].cppPriceTier = cmd_data.cppPriceTier;
        priceInfo.cppEventTable.cppEvent[ep].cppAuth = cmd_data.cppAuth;
      } else if ( cmd_data.cppAuth == SL_ZIGBEE_AF_PLUGIN_PRICE_CPP_AUTH_REJECTED ) {
        sl_zigbee_af_price_cluster_println("CPP Event Rejected");
      }
      return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    }
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_credit_payment_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_credit_payment_command_t cmd_data;
  uint8_t i;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ((zcl_decode_price_cluster_publish_credit_payment_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == 0xFF) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  i = sli_zigbee_af_price_common_get_matching_or_unused_index(priceInfo.creditPaymentTable.commonInfos[ep],
                                                              SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CREDIT_PAYMENT_TABLE_SIZE,
                                                              cmd_data.providerId, cmd_data.issuerEventId);

  sl_zigbee_af_price_cluster_println("RX: PublishCreditPayment [%d], issuerEventId=%d", i, cmd_data.issuerEventId);
  if ( i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CREDIT_PAYMENT_TABLE_SIZE ) {
    priceInfo.creditPaymentTable.commonInfos[ep][i].valid = true;
    priceInfo.creditPaymentTable.commonInfos[ep][i].providerId = cmd_data.providerId;
    priceInfo.creditPaymentTable.commonInfos[ep][i].issuerEventId = cmd_data.issuerEventId;
    priceInfo.creditPaymentTable.creditPayment[ep][i].creditPaymentDueDate = cmd_data.creditPaymentDueDate;
    priceInfo.creditPaymentTable.creditPayment[ep][i].creditPaymentOverDueAmount = cmd_data.creditPaymentOverDueAmount;
    priceInfo.creditPaymentTable.creditPayment[ep][i].creditPaymentStatus = cmd_data.creditPaymentStatus;
    priceInfo.creditPaymentTable.creditPayment[ep][i].creditPayment = cmd_data.creditPayment;
    priceInfo.creditPaymentTable.creditPayment[ep][i].creditPaymentDate = cmd_data.creditPaymentDate;
    sl_zigbee_af_copy_string(priceInfo.creditPaymentTable.creditPayment[ep][i].creditPaymentRef, cmd_data.creditPaymentRef, SL_ZIGBEE_AF_PLUGIN_PRICE_CLUSTER_MAX_CREDIT_PAYMENT_REF_LENGTH);
  }

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_currency_conversion_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_currency_conversion_command_t cmd_data;
  uint8_t i;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ((zcl_decode_price_cluster_publish_currency_conversion_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == 0xFF) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_price_cluster_println("RX: Currency Conversion, start=%d, pid=%d, eid=%d", cmd_data.startTime, cmd_data.providerId, cmd_data.issuerEventId);

  if ( cmd_data.startTime == 0 ) {
    cmd_data.startTime = sl_zigbee_af_get_current_time();
  }

  i = sli_zigbee_af_price_common_get_matching_or_unused_index(priceInfo.currencyConversionTable.commonInfos[ep],
                                                              SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CURRENCY_CONVERSION_TABLE_SIZE,
                                                              cmd_data.providerId, cmd_data.issuerEventId);

  if ( i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CURRENCY_CONVERSION_TABLE_SIZE ) {
    if ( cmd_data.startTime == CANCELLATION_START_TIME ) {
      priceInfo.currencyConversionTable.commonInfos[ep][i].valid = false;
    } else {
      priceInfo.currencyConversionTable.commonInfos[ep][i].valid = true;
      priceInfo.currencyConversionTable.commonInfos[ep][i].providerId = cmd_data.providerId;
      priceInfo.currencyConversionTable.commonInfos[ep][i].issuerEventId = cmd_data.issuerEventId;
      priceInfo.currencyConversionTable.commonInfos[ep][i].startTime = cmd_data.startTime;
      priceInfo.currencyConversionTable.currencyConversion[ep][i].newCurrency = cmd_data.newCurrency;
      priceInfo.currencyConversionTable.currencyConversion[ep][i].conversionFactor = cmd_data.conversionFactor;
      priceInfo.currencyConversionTable.currencyConversion[ep][i].conversionFactorTrailingDigit = cmd_data.conversionFactorTrailingDigit;
      priceInfo.currencyConversionTable.currencyConversion[ep][i].currencyChangeControlFlags = cmd_data.currencyChangeControlFlags;
    }
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_cancel_tariff_cb(sl_zigbee_af_cluster_command_t *cmd)
{
#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
  sl_zcl_price_cluster_cancel_tariff_command_t cmd_data;

  if (zcl_decode_price_cluster_cancel_tariff_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_price_cluster_println("RX: Cancel Tariff, providerId=%d, issuerTariffId=%d, tariffType=%d",
                                     cmd_data.providerId, cmd_data.issuerTariffId, cmd_data.tariffType);
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

void sli_zigbee_af_price_client_print_info(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    return;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_TABLE_SIZE; i++) {
    sl_zigbee_af_price_cluster_flush();
    sl_zigbee_af_price_cluster_println("= Price %d =", i);
    printPrice(&priceTable[ep][i]);
    sl_zigbee_af_price_cluster_flush();
  }
}

void sli_zigbee_af_price_client_print_by_event_id(uint8_t endpoint, uint32_t issuerEventId)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_TABLE_SIZE; i++ ) {
    if ( priceTable[ep][i].issuerEventId == issuerEventId ) {
      sl_zigbee_af_price_cluster_println("Matching Price [%d]", i);
      printPrice(&priceTable[ep][i]);
      break;
    }
  }
  if ( i >= SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_TABLE_SIZE ) {
    sl_zigbee_af_price_cluster_println("Error: Event ID %d not in price table", issuerEventId);
  }
}

static void initPrice(sl_zigbee_af_plugin_price_client_price_t *price)
{
  price->valid                             = false;
  price->active                            = false;
  price->clientEndpoint                    = 0xFF;
  price->providerId                        = 0x00000000UL;
  price->rateLabel[0]                      = 0;
  price->issuerEventId                     = 0x00000000UL;
  price->currentTime                       = 0x00000000UL;
  price->unitOfMeasure                     = 0x00;
  price->currency                          = 0x0000;
  price->priceTrailingDigitAndPriceTier    = 0x00;
  price->numberOfPriceTiersAndRegisterTier = 0x00;
  price->startTime                         = 0x00000000UL;
  price->endTime                           = 0x00000000UL;
  price->durationInMinutes                 = 0x0000;
  price->price                             = 0x00000000UL;
  price->priceRatio                        = ZCL_PRICE_CLUSTER_PRICE_RATIO_NOT_USED;
  price->generationPrice                   = ZCL_PRICE_CLUSTER_GENERATION_PRICE_NOT_USED;
  price->generationPriceRatio              = ZCL_PRICE_CLUSTER_GENERATION_PRICE_RATIO_NOT_USED;
  price->alternateCostDelivered            = ZCL_PRICE_CLUSTER_ALTERNATE_COST_DELIVERED_NOT_USED;
  price->alternateCostUnit                 = ZCL_PRICE_CLUSTER_ALTERNATE_COST_UNIT_NOT_USED;
  price->alternateCostTrailingDigit        = ZCL_PRICE_CLUSTER_ALTERNATE_COST_TRAILING_DIGIT_NOT_USED;
  price->numberOfBlockThresholds           = ZCL_PRICE_CLUSTER_NUMBER_OF_BLOCK_THRESHOLDS_NOT_USED;
  price->priceControl                      = ZCL_PRICE_CLUSTER_PRICE_CONTROL_NOT_USED;
}

static void printPrice(sl_zigbee_af_plugin_price_client_price_t *price)
{
  sl_zigbee_af_price_cluster_println("    vld: %s", (price->valid ? "YES" : "NO"));
  sl_zigbee_af_price_cluster_println("    act: %s", (price->active ? "YES" : "NO"));
  sl_zigbee_af_price_cluster_println("    pid: 0x%08X", price->providerId);
  sl_zigbee_af_price_cluster_flush();
  sl_zigbee_af_price_cluster_print("     rl: \"");
  sl_zigbee_af_price_cluster_print_string(price->rateLabel);
  sl_zigbee_af_price_cluster_println("\"");
  sl_zigbee_af_price_cluster_println("   ieid: 0x%08X", price->issuerEventId);
  sl_zigbee_af_price_cluster_println("     ct: 0x%08X", price->currentTime);
  sl_zigbee_af_price_cluster_println("    uom: 0x%02X", price->unitOfMeasure);
  sl_zigbee_af_price_cluster_println("      c: 0x%04X", price->currency);
  sl_zigbee_af_price_cluster_println(" ptdapt: 0x%02X", price->priceTrailingDigitAndPriceTier);
  sl_zigbee_af_price_cluster_println("noptart: 0x%02X", price->numberOfPriceTiersAndRegisterTier);
  sl_zigbee_af_price_cluster_flush();
  sl_zigbee_af_price_cluster_println("     st: 0x%08X", price->startTime);
  sl_zigbee_af_price_cluster_println("     et: 0x%08X", price->endTime);
  sl_zigbee_af_price_cluster_println("    dim: 0x%04X", price->durationInMinutes);
  sl_zigbee_af_price_cluster_println("      p: 0x%08X", price->price);
  sl_zigbee_af_price_cluster_println("     pr: 0x%02X", price->priceRatio);
  sl_zigbee_af_price_cluster_flush();
  sl_zigbee_af_price_cluster_println("     gp: 0x%08X", price->generationPrice);
  sl_zigbee_af_price_cluster_println("    gpr: 0x%02X", price->generationPriceRatio);
  sl_zigbee_af_price_cluster_println("    acd: 0x%08X", price->alternateCostDelivered);
  sl_zigbee_af_price_cluster_println("    acu: 0x%02X", price->alternateCostUnit);
  sl_zigbee_af_price_cluster_println("   actd: 0x%02X", price->alternateCostTrailingDigit);
  sl_zigbee_af_price_cluster_println("   nobt: 0x%02X", price->numberOfBlockThresholds);
  sl_zigbee_af_price_cluster_println("     pc: 0x%02X", price->priceControl);
}

static void scheduleTick(uint8_t endpoint)
{
  uint32_t next = ZCL_PRICE_CLUSTER_END_TIME_NEVER;
  uint32_t now = sl_zigbee_af_get_current_time();
  bool active = false;
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint,
                                                               ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    return;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_TABLE_SIZE; i++) {
    if (!priceTable[ep][i].valid) {
      continue;
    }

    // Remove old prices from the table.  This may result in the active price
    // being expired, which requires notifying the application.
    if (priceTable[ep][i].endTime <= now) {
      if (priceTable[ep][i].active) {
        priceTable[ep][i].active = false;
        sl_zigbee_af_price_client_price_expired_cb(&priceTable[ep][i]);
      }
      initPrice(&priceTable[ep][i]);
      continue;
    }

    // If we don't have a price that should be active right now, we will need to
    // schedule the tick to wake us up when the next active price should start,
    // so keep track of the price with the start time soonest after the current
    // time.
    if (!active && priceTable[ep][i].startTime < next) {
      next = priceTable[ep][i].startTime;
    }

    // If we have a price that should be active now, a tick is scheduled for the
    // time remaining in the duration to wake us up and expire the price.  If
    // the price is transitioning from inactive to active for the first time, we
    // also need to notify the application the application.
    if (priceTable[ep][i].startTime <= now) {
      if (!priceTable[ep][i].active) {
        priceTable[ep][i].active = true;
        sl_zigbee_af_price_client_price_started_cb(&priceTable[ep][i]);
      }
      active = true;
      next = priceTable[ep][i].endTime;
    }
  }

  // We need to wake up again to activate a new price or expire the current
  // price.  Otherwise, we don't have to do anything until we receive a new
  // price from the server.
  if (next != ZCL_PRICE_CLUSTER_END_TIME_NEVER) {
    sl_zigbee_zcl_schedule_client_tick(endpoint,
                                       ZCL_PRICE_CLUSTER_ID,
                                       (next - now) * MILLISECOND_TICKS_PER_SECOND);
  } else {
    sl_zigbee_zcl_deactivate_client_tick(endpoint, ZCL_PRICE_CLUSTER_ID);
  }
}

// Get the index of the sl_zigbee_af_price_client_common_info_t structure whose event ID matches.
static uint8_t sli_zigbee_af_price_common_get_matching_index(sl_zigbee_af_price_client_common_info_t *pcommon, uint8_t numElements, uint32_t issuerEventId)
{
  uint8_t i;
  for ( i = 0; i < numElements; i++ ) {
    if ( pcommon[i].valid && (pcommon[i].issuerEventId == issuerEventId) ) {
      break;
    }
  }
  return i;
}

static uint8_t sli_zigbee_af_price_common_get_active_index(sl_zigbee_af_price_client_common_info_t *pcommon, uint8_t numElements)
{
  uint8_t i;
  uint32_t largestEventId = 0;
  uint8_t  largestEventIdIndex = 0xFF;
  uint32_t timeNow = sl_zigbee_af_get_current_time();

  for ( i = 0; i < numElements; i++ ) {
    if ( pcommon[i].valid
         && ((pcommon[i].startTime <= timeNow)
             || (pcommon[i].startTime == 0))
         && (pcommon[i].issuerEventId > largestEventId) ) {
      //(PriceClientCo2Table[i].startTime > nearestTime) ){
      // Found entry that is closer to timeNow
      //nearestTime = PriceClientCo2Table[i].startTime;
      largestEventId = pcommon[i].issuerEventId;
      largestEventIdIndex = i;
    }
  }
  return largestEventIdIndex;
}

// Parses the sl_zigbee_af_price_client_common_info_t structure to find a matching, unused, or oldest (smallest eventId)
// element that can be overwritten with new data.
static uint8_t sli_zigbee_af_price_common_get_matching_or_unused_index(sl_zigbee_af_price_client_common_info_t *pcommon, uint8_t numElements,
                                                                       uint32_t newProviderId, uint32_t newIssuerEventId)
{
  uint32_t smallestEventId = 0xFFFFFFFF;
  uint8_t  smallestEventIdIndex = 0xFF;
  uint8_t  i;

  for ( i = 0; i < numElements; i++ ) {
    if ( !pcommon[i].valid ) {
      // Use the first invalid entry unless a matching entry is found.
      if ( smallestEventId > 0 ) {
        smallestEventId = 0;
        smallestEventIdIndex = i;
      }
    } else {
      if ( (pcommon[i].providerId == newProviderId) && (pcommon[i].issuerEventId == newIssuerEventId) ) {
        // Match found
        smallestEventIdIndex = i;
        smallestEventId = pcommon[i].issuerEventId;
        break;
      } else if ( pcommon[i].issuerEventId < smallestEventId ) {
        smallestEventId = pcommon[i].issuerEventId;
        smallestEventIdIndex = i;
      }
    }
  }
  // Do quick sanity check here to validate the index.
  // Expect the indexed entry to either:
  //  1. Be invalid, or
  //  2. Have an issuerEventId <= newIssuerEventId
  if ( (smallestEventIdIndex < numElements)
       && pcommon[smallestEventIdIndex].valid
       && (smallestEventId > newIssuerEventId) ) {
    // FAIL above conditions - return invalid index
    smallestEventIdIndex = 0xFF;
  }
  return smallestEventIdIndex;
}

static void sl_zigbee_af_price_init_block_period(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_BLOCK_PERIOD_TABLE_SIZE; i++ ) {
    priceInfo.blockPeriodTable.commonInfos[ep][i].valid = false;
  }
}

uint8_t sli_zigbee_af_price_get_block_period_table_index_by_event_id(uint8_t endpoint, uint32_t issuerEventId)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return 0xFF;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_BLOCK_PERIOD_TABLE_SIZE; i++ ) {
    if ( priceInfo.blockPeriodTable.commonInfos[ep][i].valid
         && (priceInfo.blockPeriodTable.commonInfos[ep][i].issuerEventId == issuerEventId) ) {
      break;
    }
  }
  return i;
}

void sli_zigbee_af_price_print_block_period_table_index(uint8_t endpoint, uint8_t index)
{
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  if ( index < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_BLOCK_PERIOD_TABLE_SIZE ) {
    sl_zigbee_af_price_cluster_println("Print Block Period [%d]", index);
    sl_zigbee_af_price_cluster_println("  valid=%d", priceInfo.blockPeriodTable.commonInfos[ep][index].valid);
    sl_zigbee_af_price_cluster_println("  providerId=%d", priceInfo.blockPeriodTable.commonInfos[ep][index].providerId);
    sl_zigbee_af_price_cluster_println("  issuerEventId=%d", priceInfo.blockPeriodTable.commonInfos[ep][index].issuerEventId);
    sl_zigbee_af_price_cluster_println("  startTime=0x%08X", priceInfo.blockPeriodTable.commonInfos[ep][index].startTime);
    sl_zigbee_af_price_cluster_println("  duration=%d", priceInfo.blockPeriodTable.commonInfos[ep][index].durationSec);
    sl_zigbee_af_price_cluster_println("  rawStartTime=0x%08X", priceInfo.blockPeriodTable.blockPeriod[ep][index].blockPeriodStartTime);
    sl_zigbee_af_price_cluster_println("  rawDuration=%d", priceInfo.blockPeriodTable.blockPeriod[ep][index].blockPeriodDuration);
    sl_zigbee_af_price_cluster_println("  durationType=%d", priceInfo.blockPeriodTable.blockPeriod[ep][index].blockPeriodDurationType);
    sl_zigbee_af_price_cluster_println("  blockPeriodControl=%d", priceInfo.blockPeriodTable.blockPeriod[ep][index].blockPeriodControl);
    sl_zigbee_af_price_cluster_println("  tariffType=%d", priceInfo.blockPeriodTable.blockPeriod[ep][index].tariffType);
    sl_zigbee_af_price_cluster_println("  tariffResolutionPeriod=%d", priceInfo.blockPeriodTable.blockPeriod[ep][index].tariffResolutionPeriod);
  } else {
    sl_zigbee_af_price_cluster_println("Error: Block Period NOT FOUND");
  }
}

static void sl_zigbee_af_price_init_block_thresholds_table(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_BLOCK_THRESHOLD_TABLE_SIZE; i++ ) {
    priceInfo.blockThresholdTable.commonInfos[ep][i].valid = false;
  }
}

static void sl_zigbee_af_price_init_conversion_factor_table(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CONVERSION_FACTOR_TABLE_SIZE; i++ ) {
    priceInfo.conversionFactorTable.commonInfos[ep][i].providerId = UNSPECIFIED_PROVIDER_ID;
    priceInfo.conversionFactorTable.commonInfos[ep][i].valid = false;
  }
}

uint8_t sli_zigbee_af_price_get_conversion_factor_index_by_event_id(uint8_t endpoint, uint32_t issuerEventId)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return 0xFF;
  }

  i = sli_zigbee_af_price_common_get_matching_index(priceInfo.conversionFactorTable.commonInfos[ep],
                                                    SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CONVERSION_FACTOR_TABLE_SIZE,
                                                    issuerEventId);
  return i;
}

void sli_zigbee_af_price_print_conversion_factor_entry_index(uint8_t endpoint, uint8_t i)
{
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  if ( i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CONVERSION_FACTOR_TABLE_SIZE ) {
    sl_zigbee_af_price_cluster_println("Print Conversion Factor [%d]", i);
    sl_zigbee_af_price_cluster_println("  issuerEventId=%d", priceInfo.conversionFactorTable.commonInfos[ep][i].issuerEventId);
    sl_zigbee_af_price_cluster_println("  startTime=%d", priceInfo.conversionFactorTable.commonInfos[ep][i].startTime);
    sl_zigbee_af_price_cluster_println("  conversionFactor=%d", priceInfo.conversionFactorTable.conversionFactor[ep][i].conversionFactor);
    sl_zigbee_af_price_cluster_println("  conversionFactorTrailingDigit=%d", priceInfo.conversionFactorTable.conversionFactor[ep][i].conversionFactorTrailingDigit);
  } else {
    sl_zigbee_af_price_cluster_println("Conversion Factor NOT FOUND");
  }
}

static void sl_zigbee_af_price_init_calorific_value_table(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CALORIFIC_VALUE_TABLE_SIZE; i++ ) {
    priceInfo.calorificValueTable.commonInfos[ep][i].valid = false;
    priceInfo.calorificValueTable.commonInfos[ep][i].providerId = UNSPECIFIED_PROVIDER_ID;
  }
}

uint8_t sli_zigbee_af_price_get_calorific_value_index_by_event_id(uint8_t endpoint, uint32_t issuerEventId)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return 0xFF;
  }
  i = sli_zigbee_af_price_common_get_matching_index(priceInfo.calorificValueTable.commonInfos[ep],
                                                    SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CALORIFIC_VALUE_TABLE_SIZE,
                                                    issuerEventId);
  return i;
}

void sli_zigbee_af_price_print_calorific_value_entry_index(uint8_t endpoint, uint8_t i)
{
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  if ( i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CALORIFIC_VALUE_TABLE_SIZE ) {
    sl_zigbee_af_price_cluster_println("Print Calorific Value [%d]", i);
    sl_zigbee_af_price_cluster_println("  issuerEventId=%d", priceInfo.calorificValueTable.commonInfos[ep][i].issuerEventId);
    sl_zigbee_af_price_cluster_println("  startTime=%d", priceInfo.calorificValueTable.commonInfos[ep][i].startTime);
    sl_zigbee_af_price_cluster_println("  calorificValue=%d", priceInfo.calorificValueTable.calorificValue[ep][i].calorificValue);
    sl_zigbee_af_price_cluster_println("  calorificValueUnit=%d", priceInfo.calorificValueTable.calorificValue[ep][i].calorificValueUnit);
    sl_zigbee_af_price_cluster_println("  calorificValueTrailingDigit=%d", priceInfo.calorificValueTable.calorificValue[ep][i].calorificValueTrailingDigit);
  } else {
    sl_zigbee_af_price_cluster_println("Calorific Value NOT FOUND");
  }
}

static void sl_zigbee_af_price_cluster_init_co2_table(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CO2_TABLE_SIZE; i++ ) {
    priceInfo.co2ValueTable.commonInfos[ep][i].valid = false;
  }
}

uint8_t sl_zigbee_af_price_cluster_get_active_co2_value_index(uint8_t endpoint)
{
  uint8_t i;

  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return 0xFF;
  }
  i = sli_zigbee_af_price_common_get_active_index(priceInfo.co2ValueTable.commonInfos[ep],
                                                  SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CO2_TABLE_SIZE);
  return i;
}

void sli_zigbee_af_price_print_co2_value_table_print_index(uint8_t endpoint, uint8_t i)
{
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  if ( i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CO2_TABLE_SIZE ) {
    sl_zigbee_af_price_cluster_println("Print CO2 Value [%d]", i);
    sl_zigbee_af_price_cluster_println("  isValid=%d", priceInfo.co2ValueTable.commonInfos[ep][i].valid);
    sl_zigbee_af_price_cluster_println("  providerId=0x%08X", priceInfo.co2ValueTable.commonInfos[ep][i].providerId);
    sl_zigbee_af_price_cluster_println("  issuerEventId=0x%08X", priceInfo.co2ValueTable.commonInfos[ep][i].issuerEventId);
    sl_zigbee_af_price_cluster_print("  startTime= ");
    sl_zigbee_af_print_time(priceInfo.co2ValueTable.commonInfos[ep][i].startTime);
    sl_zigbee_af_price_cluster_println("  tariffType=0x%02X", priceInfo.co2ValueTable.co2Value[ep][i].tariffType);
    sl_zigbee_af_price_cluster_println("  cO2Value=0x%08X", priceInfo.co2ValueTable.co2Value[ep][i].cO2Value);
    sl_zigbee_af_price_cluster_println("  cO2ValueUnit=0x%02X", priceInfo.co2ValueTable.co2Value[ep][i].cO2ValueUnit);
    sl_zigbee_af_price_cluster_println("  cO2ValueTrailingDigit=0x%02X", priceInfo.co2ValueTable.co2Value[ep][i].cO2ValueTrailingDigit);
  }
}

static void sli_zigbee_af_price_client_tier_labels_init(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_TIER_LABELS_TABLE_SIZE; i++ ) {
    priceInfo.tierLabelsTable.commonInfos[ep][i].valid = false;
  }
}

static void sli_zigbee_af_price_client_add_tier_label(uint8_t endpoint, uint32_t providerId, uint32_t issuerEventId, uint32_t issuerTariffId,
                                                      uint8_t numberOfLabels, uint8_t *tierLabelsPayload)
{
  uint8_t i, j, x;
  uint8_t tierLabelLen;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }

  i = sli_zigbee_af_price_common_get_matching_or_unused_index(priceInfo.tierLabelsTable.commonInfos[ep],
                                                              SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_TIER_LABELS_TABLE_SIZE,
                                                              providerId, issuerEventId);
  if ( i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_TIER_LABELS_TABLE_SIZE ) {
    priceInfo.tierLabelsTable.commonInfos[ep][i].providerId = providerId;
    priceInfo.tierLabelsTable.commonInfos[ep][i].issuerEventId = issuerEventId;
    priceInfo.tierLabelsTable.tierLabels[ep][i].issuerTariffId = issuerTariffId;
    priceInfo.tierLabelsTable.commonInfos[ep][i].valid = true;
    if ( numberOfLabels > SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_MAX_TIERS_PER_TARIFF ) {
      numberOfLabels = SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_MAX_TIERS_PER_TARIFF;
    }
    priceInfo.tierLabelsTable.tierLabels[ep][i].numberOfLabels = numberOfLabels;
    x = 0;
    for ( j = 0; j < numberOfLabels; j++ ) {
      priceInfo.tierLabelsTable.tierLabels[ep][i].tierIds[j] = tierLabelsPayload[x];
      tierLabelLen = tierLabelsPayload[x + 1];
      if ( tierLabelLen > 12 ) {
        tierLabelLen = 12;
        tierLabelsPayload[x + 1] = tierLabelLen;
      }
      memcpy(priceInfo.tierLabelsTable.tierLabels[ep][i].tierLabels[j], &tierLabelsPayload[x + 1], tierLabelLen + 1);
      x += (tierLabelLen + 2);
    }
  }
}

void sli_zigbee_af_price_print_tier_label_table_entry_index(uint8_t endpoint, uint8_t i)
{
  uint8_t j;
  uint8_t numLabels;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }

  if ( i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_TIER_LABELS_TABLE_SIZE ) {
    sl_zigbee_af_price_cluster_println("= CLIENT TIER LABEL TABLE [%d] =", i);
    sl_zigbee_af_price_cluster_println("  providerId=%d", priceInfo.tierLabelsTable.commonInfos[ep][i].providerId);
    sl_zigbee_af_price_cluster_println("  issuerEventId=%d", priceInfo.tierLabelsTable.commonInfos[ep][i].issuerEventId);
    sl_zigbee_af_price_cluster_println("  issuerTariffId=%d", priceInfo.tierLabelsTable.tierLabels[ep][i].issuerTariffId);
    sl_zigbee_af_price_cluster_println("  numberOfLabels=%d", priceInfo.tierLabelsTable.tierLabels[ep][i].numberOfLabels);
    numLabels = priceInfo.tierLabelsTable.tierLabels[ep][i].numberOfLabels;
    if ( numLabels > SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_MAX_TIERS_PER_TARIFF ) {
      numLabels = SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_MAX_TIERS_PER_TARIFF;
    }
    for ( j = 0; j < numLabels; j++ ) {
      sl_zigbee_af_price_cluster_println("  tierId[%d]=%d", j, priceInfo.tierLabelsTable.tierLabels[ep][i].tierIds[j]);
    }
  } else {
    sl_zigbee_af_price_cluster_println("Error:  Tier Label index %d not valid", i);
  }
}

uint8_t sli_zigbee_af_price_get_active_tier_label_table_index_by_tariff_id(uint8_t endpoint, uint32_t tariffId)
{
  uint32_t largestEventId = 0;
  uint8_t  largestEventIdIndex = 0xFF;
  uint8_t  i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return 0xFF;
  }

  sl_zigbee_af_price_cluster_println("===========   TIER LABEL TABLE CHECK, tariff=%d", tariffId);
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_TIER_LABELS_TABLE_SIZE; i++ ) {
    sl_zigbee_af_price_cluster_println("  i=%d, val=%d, tariffId=%d, eventId=%d", i,
                                       priceInfo.tierLabelsTable.commonInfos[ep][i].valid, priceInfo.tierLabelsTable.tierLabels[ep][i].issuerTariffId,
                                       priceInfo.tierLabelsTable.commonInfos[ep][i].issuerEventId);

    if ( priceInfo.tierLabelsTable.commonInfos[ep][i].valid
         && (priceInfo.tierLabelsTable.tierLabels[ep][i].issuerTariffId == tariffId)
         && (priceInfo.tierLabelsTable.commonInfos[ep][i].issuerEventId > largestEventId) ) {
      largestEventId = priceInfo.tierLabelsTable.commonInfos[ep][i].issuerEventId;
      largestEventIdIndex = i;
      sl_zigbee_af_price_cluster_println("   ___ UPDATING i=%d", i);
    }
  }
  sl_zigbee_af_price_cluster_println("- RETURN %d", largestEventIdIndex);
  return largestEventIdIndex;
}

static void sl_zigbee_af_price_init_billing_period_table(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_BILLING_PERIOD_TABLE_SIZE; i++ ) {
    priceInfo.billingPeriodTable.commonInfos[ep][i].valid = false;
  }
}

uint8_t sli_zigbee_af_price_get_active_billing_period_index(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return 0xFF;
  }
  // Get the event that started before current time, with largest event Id.
  i = sli_zigbee_af_price_common_get_active_index(priceInfo.billingPeriodTable.commonInfos[ep],
                                                  SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_BILLING_PERIOD_TABLE_SIZE);

  return i;
}

void sli_zigbee_af_price_print_billing_period_table_entry_index(uint8_t endpoint, uint8_t i)
{
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  if ( i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_BILLING_PERIOD_TABLE_SIZE ) {
    sl_zigbee_af_price_cluster_println("Print Billing Period Table [%d]", i);
    sl_zigbee_af_price_cluster_println("  providerId=%d", priceInfo.billingPeriodTable.commonInfos[ep][i].providerId);
    sl_zigbee_af_price_cluster_println("  issuerEventId=%d", priceInfo.billingPeriodTable.commonInfos[ep][i].issuerEventId);
    sl_zigbee_af_price_cluster_println("  startTime=%d", priceInfo.billingPeriodTable.billingPeriod[ep][i].billingPeriodStartTime);
    sl_zigbee_af_price_cluster_println("  duration=%d", priceInfo.billingPeriodTable.billingPeriod[ep][i].billingPeriodDuration);
    sl_zigbee_af_price_cluster_println("  durationType=%d", priceInfo.billingPeriodTable.billingPeriod[ep][i].billingPeriodDurationType);
    sl_zigbee_af_price_cluster_println("  tariffType=%d", priceInfo.billingPeriodTable.billingPeriod[ep][i].tariffType);
  } else {
    sl_zigbee_af_price_cluster_println("Billing Period Entry NOT FOUND");
  }
}

static void sl_zigbee_af_price_cluster_init_cpp(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  priceInfo.cppEventTable.commonInfos[ep].valid = false;
  priceInfo.cppEventTable.commonInfos[ep].issuerEventId = 0;
}

void sl_zigbee_af_price_print_cpp_event(uint8_t endpoint)
{
  uint32_t timeNow;
  uint32_t cppEndTime;
  bool cppActive;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }

  timeNow = sl_zigbee_af_get_current_time();
  cppEndTime = priceInfo.cppEventTable.commonInfos[ep].startTime + priceInfo.cppEventTable.commonInfos[ep].durationSec;

  cppActive = (priceInfo.cppEventTable.commonInfos[ep].valid
               && (priceInfo.cppEventTable.commonInfos[ep].startTime <= timeNow)
               && (cppEndTime >= timeNow) );

  sl_zigbee_af_price_cluster_println("  == startTime=0x%08X, endTime=0x%08X,  timeNow=0x%08X",
                                     priceInfo.cppEventTable.commonInfos[ep].startTime, cppEndTime, priceInfo.cppEventTable.commonInfos[ep].durationSec);

  sl_zigbee_af_price_cluster_println("= CPP Event =");
  sl_zigbee_af_price_cluster_println("  active=%d", cppActive);
  sl_zigbee_af_price_cluster_println("  valid=%d", priceInfo.cppEventTable.commonInfos[ep].valid);
  sl_zigbee_af_price_cluster_println("  providerId=%d", priceInfo.cppEventTable.commonInfos[ep].providerId);
  sl_zigbee_af_price_cluster_println("  issuerEventId=%d", priceInfo.cppEventTable.commonInfos[ep].issuerEventId);
  sl_zigbee_af_price_cluster_println("  startTime=0x%08X", priceInfo.cppEventTable.commonInfos[ep].startTime);
  sl_zigbee_af_price_cluster_println("  durationInMinutes=%d", priceInfo.cppEventTable.cppEvent[ep].durationInMinutes);
  sl_zigbee_af_price_cluster_println("  tariffType=%d", priceInfo.cppEventTable.cppEvent[ep].tariffType);
  sl_zigbee_af_price_cluster_println("  cppPriceTier=%d", priceInfo.cppEventTable.cppEvent[ep].cppPriceTier);
  sl_zigbee_af_price_cluster_println("  cppAuth=%d", priceInfo.cppEventTable.cppEvent[ep].cppAuth);
}

static void sl_zigbee_af_price_cluster_init_credit_payment_table(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CREDIT_PAYMENT_TABLE_SIZE; i++ ) {
    priceInfo.creditPaymentTable.commonInfos[ep][i].valid = false;
  }
}

uint8_t sli_zigbee_af_price_credit_payment_table_get_index_with_event_id(uint8_t endpoint, uint32_t issuerEventId)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return 0xFF;
  }

  i = sli_zigbee_af_price_common_get_matching_index(priceInfo.creditPaymentTable.commonInfos[ep],
                                                    SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CREDIT_PAYMENT_TABLE_SIZE,
                                                    issuerEventId);
  return i;
}

void sli_zigbee_af_price_print_credit_payment_table_index(uint8_t endpoint, uint8_t index)
{
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  if ( index >= SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CREDIT_PAYMENT_TABLE_SIZE ) {
    sl_zigbee_af_price_cluster_println("Error: Print index %d out of bounds.", index);
  } else if ( priceInfo.creditPaymentTable.commonInfos[ep][index].valid == false ) {
    sl_zigbee_af_price_cluster_println("Error: Entry %d invalid", index);
  } else {
    sl_zigbee_af_price_cluster_println("Print Credit Payment [%d]", index);
    sl_zigbee_af_price_cluster_println("  valid=%d", priceInfo.creditPaymentTable.commonInfos[ep][index].valid);
    sl_zigbee_af_price_cluster_println("  providerId=%d", priceInfo.creditPaymentTable.commonInfos[ep][index].providerId);
    sl_zigbee_af_price_cluster_println("  issuerEventId=%d", priceInfo.creditPaymentTable.commonInfos[ep][index].issuerEventId);
    sl_zigbee_af_price_cluster_println("  dueDate=%d", priceInfo.creditPaymentTable.creditPayment[ep][index].creditPaymentDueDate);
    sl_zigbee_af_price_cluster_println("  overDueAmount=%d", priceInfo.creditPaymentTable.creditPayment[ep][index].creditPaymentOverDueAmount);
    sl_zigbee_af_price_cluster_println("  status=%d", priceInfo.creditPaymentTable.creditPayment[ep][index].creditPaymentStatus);
    sl_zigbee_af_price_cluster_println("  payment=%d", priceInfo.creditPaymentTable.creditPayment[ep][index].creditPayment);
    sl_zigbee_af_price_cluster_println("  paymentDate=%d", priceInfo.creditPaymentTable.creditPayment[ep][index].creditPaymentDate);
    sl_zigbee_af_price_cluster_println("  paymentRef=%d", priceInfo.creditPaymentTable.creditPayment[ep][index].creditPaymentRef);
  }
}

static void sl_zigbee_af_price_cluster_init_currency_conversion_table(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CURRENCY_CONVERSION_TABLE_SIZE; i++ ) {
    priceInfo.currencyConversionTable.commonInfos[ep][i].valid = false;
  }
}

uint8_t sl_zigbee_af_price_cluster_get_active_currency_index(uint8_t endpoint)
{
  uint8_t i;
  uint32_t timeNow = sl_zigbee_af_get_current_time();
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return 0xFF;
  }

  i = sli_zigbee_af_price_common_get_active_index(priceInfo.currencyConversionTable.commonInfos[ep],
                                                  SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CURRENCY_CONVERSION_TABLE_SIZE);

  if ( i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CURRENCY_CONVERSION_TABLE_SIZE ) {
    sl_zigbee_af_price_cluster_println("GET ACTIVE:  i=%d, startTime=%d,  currTime=%d",
                                       i, priceInfo.currencyConversionTable.commonInfos[ep][i].startTime, timeNow);
  }

  return i;
}

uint8_t sl_zigbee_af_price_cluster_currency_conversion_table_get_index_by_event_id(uint8_t endpoint, uint32_t issuerEventId)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return 0xFF;
  }
  i = sli_zigbee_af_price_common_get_matching_index(priceInfo.currencyConversionTable.commonInfos[ep],
                                                    SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CURRENCY_CONVERSION_TABLE_SIZE,
                                                    issuerEventId);
  return i;
}

void sli_zigbee_af_price_print_currency_conversion_table_index(uint8_t endpoint, uint8_t i)
{
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    return;
  }
  if ( i < SL_ZIGBEE_AF_PLUGIN_PRICE_CLIENT_CURRENCY_CONVERSION_TABLE_SIZE ) {
    sl_zigbee_af_price_cluster_println("= Print Currency Conversion Table [%d]", i);
    sl_zigbee_af_price_cluster_println("  providerId=%d", priceInfo.currencyConversionTable.commonInfos[ep][i].providerId);
    sl_zigbee_af_price_cluster_println("  issuerEventId=%d", priceInfo.currencyConversionTable.commonInfos[ep][i].issuerEventId);
    sl_zigbee_af_price_cluster_println("  startTime=%d", priceInfo.currencyConversionTable.commonInfos[ep][i].startTime);
    sl_zigbee_af_price_cluster_println("  newCurrency=%d", priceInfo.currencyConversionTable.currencyConversion[ep][i].newCurrency);
    sl_zigbee_af_price_cluster_println("  conversionFactor=%d", priceInfo.currencyConversionTable.currencyConversion[ep][i].conversionFactor);
    sl_zigbee_af_price_cluster_println("  conversionFactorTrailingDigit=%d", priceInfo.currencyConversionTable.currencyConversion[ep][i].conversionFactorTrailingDigit);
    sl_zigbee_af_price_cluster_println("  currencyChangeControlFlags=%d", priceInfo.currencyConversionTable.currencyConversion[ep][i].currencyChangeControlFlags);
  }
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_consolidated_bill_cb(sl_zigbee_af_cluster_command_t *cmd);

uint32_t sl_zigbee_af_price_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                         sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_PUBLISH_PRICE_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_price_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_BLOCK_PERIOD_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_block_period_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_CONVERSION_FACTOR_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_conversion_factor_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_CALORIFIC_VALUE_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_calorific_value_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_TARIFF_INFORMATION_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_tariff_information_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_BLOCK_THRESHOLDS_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_block_thresholds_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_CO2_VALUE_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_co2_value_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_TIER_LABELS_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_tier_labels_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_BILLING_PERIOD_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_billing_period_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_CONSOLIDATED_BILL_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_consolidated_bill_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_CPP_EVENT_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_cpp_event_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_CREDIT_PAYMENT_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_credit_payment_cb(cmd);
        break;
      }
      case ZCL_PUBLISH_CURRENCY_CONVERSION_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_publish_currency_conversion_cb(cmd);
        break;
      }
      case ZCL_CANCEL_TARIFF_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_cancel_tariff_cb(cmd);
        break;
      }
    }
  }

  return status;
}
