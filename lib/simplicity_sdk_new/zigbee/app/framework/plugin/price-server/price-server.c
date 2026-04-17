/***************************************************************************//**
 * @file
 * @brief Routines for the Price Server plugin, which implements the server side
 *        of the Price cluster.
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

#include "../../include/af.h"
#include "../../util/common.h"
#include "price-server.h"
#include "price-server-tick.h"

#include "zap-cluster-command-parser.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
#include "app/framework/plugin/test-harness/test-harness.h"
#endif  // SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT

static sl_zigbee_af_scheduled_price_t priceTable[SL_ZIGBEE_ZCL_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_PRICE_TABLE_SIZE];

// Global
sl_zigbee_af_price_server_info_t priceServerInfo;

// Forward declaration

static void sl_zigbee_af_price_server_block_period_update_bindings(uint8_t blockPeriodEntryIndex);
static void sl_zigbee_af_price_clear_tier_label_table(uint8_t endpoint);
static void sl_zigbee_af_price_clear_calorific_value_table(uint8_t endpoint);
static void sl_zigbee_af_price_server_clear_co2_value(uint8_t endpoint);
static void sl_zigbee_af_price_clear_consolidated_bills_table(uint8_t endpoint);
static void sl_zigbee_af_price_clear_conversion_factor_table(uint8_t endpoint);
static void sl_zigbee_af_price_clear_billing_period_table(uint8_t endpoint);
static void sl_zigbee_af_price_clear_cancel_tariff_table(uint8_t endpoint);
static void sl_zigbee_af_price_clear_currency_conversion_table(uint8_t endpoint);
static void sli_zigbee_af_price_server_schedule_get_scheduled_prices(uint8_t endpoint);
static void sortCreditPaymentEntries(uint8_t *entries, uint8_t numValidEntries, sl_zigbee_af_price_credit_payment_t *table);
static void sl_zigbee_af_price_update_conversion_factor_attribs(uint8_t endpoint, uint32_t conversionFactor, uint8_t conversionFactorTrailingDigit);
static void sl_zigbee_af_price_update_calorific_value_attribs(uint8_t endpoint, uint32_t calorificValue,
                                                              uint8_t calorificValueUnit, uint8_t calorificValueTrailingDigit);

// Bits 1 through 7 are reserved in the price control field.  These are used
// internally to represent whether the message is valid, active, or is a "start
// now" price.
#define VALID  BIT(1)
#define ACTIVE BIT(2)
#define NOW    BIT(3)
#define priceIsValid(price)   (((price)->priceControl & VALID) != 0u)
#define priceIsActive(price)  (((price)->priceControl & ACTIVE) != 0u)
#define priceIsNow(price)     (((price)->priceControl & NOW) != 0u)
#define priceIsForever(price) ((price)->duration == ZCL_PRICE_CLUSTER_DURATION16_UNTIL_CHANGED)
#define TARIFF_TYPE_DONT_CARE (0xFFu)

// Returns true if the price will be current or scheduled at the given time.
static bool priceIsCurrentOrScheduled(const sl_zigbee_af_scheduled_price_t *price,
                                      uint32_t time)
{
  return (priceIsValid(price)
          && priceIsActive(price)
          && (priceIsForever(price)
              || time < price->startTime + (uint32_t)price->duration * 60u));
}

// Returns the number of all current or scheduled prices.
static uint8_t scheduledPriceCount(uint8_t endpoint, uint32_t startTime)
{
  uint8_t i, count = 0;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return 0;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_PRICE_TABLE_SIZE; i++) {
    if (priceIsCurrentOrScheduled(&priceTable[ep][i], startTime)) {
      count++;
    }
  }
  return count;
}

static void fillPublishPriceCommand(sl_zigbee_af_scheduled_price_t price)
{
  char * const args[] = { "wswwuvuuwvwuwu", "wswwuvuuwvwuwuwuuuu", "wswwuvuuwvwuwuwuuuuuuuuu" };
  UNUSED_VAR(args);
  (void) sl_zigbee_af_fill_external_buffer(ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                                           | SL_ZIGBEE_AF_DEFAULT_RESPONSE_POLICY_RESPONSES,
                                           ZCL_PRICE_CLUSTER_ID,
                                           ZCL_PUBLISH_PRICE_COMMAND_ID,
    #if defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT)
                                           args[sendSE11PublishPriceCommand],
    #else // EMBER_AF_SE12.
                                           "wswwuvuuwvwuwuwuuuuuuuuu",
    #endif
                                           price.providerId,
                                           price.rateLabel,
                                           price.issuerEventID,
                                           sl_zigbee_af_get_current_time(),
                                           price.unitOfMeasure,
                                           price.currency,
                                           price.priceTrailingDigitAndTier,
                                           price.numberOfPriceTiersAndTier,
                                           price.startTime,
                                           price.duration,
                                           price.price,
                                           price.priceRatio,
                                           price.generationPrice,
                                           price.generationPriceRatio,
                                           price.alternateCostDelivered,
                                           price.alternateCostUnit,
                                           price.alternateCostTrailingDigit,
                                           price.numberOfBlockThresholds,
                                           price.priceControl,
                                           // TODO: these are optional; if we want to implement, we should!
                                           0, // NumberOfGenerationTiers
                                           0x01, // GenerationTier
                                           0, // ExtendedNumberOfPriceTiers
                                           0, // ExtendedPriceTier
                                           0); // ExtendedRegisterTier
}

typedef struct {
  bool isIntraPan;
  union {
    struct {
      sl_802154_short_addr_t nodeId;
      uint8_t       clientEndpoint;
      uint8_t       serverEndpoint;
    } intra;
    struct {
      sl_802154_long_addr_t eui64;
      sl_802154_pan_id_t panId;
    } inter;
  } pan;
  uint8_t  sequence;
  uint8_t  index;
  uint32_t startTime;
  uint8_t  numberOfEvents;
} sli_zigbee_get_scheduled_prices_partner_t;
static sli_zigbee_get_scheduled_prices_partner_t partner;

void sl_zigbee_af_price_clear_price_table(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_PRICE_TABLE_SIZE; i++) {
    priceTable[ep][i].priceControl &= ~VALID;
  }
}

// Retrieves the price at the index.  Returns false if the index is invalid.
bool sl_zigbee_af_price_get_price_table_entry(uint8_t endpoint,
                                              uint8_t index,
                                              sl_zigbee_af_scheduled_price_t *price)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX || index == 0xFFu) {
    return false;
  }

  if (index < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_PRICE_TABLE_SIZE) {
    (void) memmove(price, &priceTable[ep][index], sizeof(sl_zigbee_af_scheduled_price_t));

    // Clear out our internal bits from the price control.
    price->priceControl &= ~ZCL_PRICE_CLUSTER_RESERVED_MASK;

    // If the price is expired or it has an absolute time, set the start time
    // and duration to the original start time and duration.  For "start now"
    // prices that are current or scheduled, set the start time to the special
    // value for "now" and set the duration to the remaining time, if it is not
    // already the special value for "until changed."
    if (priceIsCurrentOrScheduled(&priceTable[ep][index], sl_zigbee_af_get_current_time())
        && priceIsNow(&priceTable[ep][index])) {
      price->startTime = ZCL_PRICE_CLUSTER_START_TIME_NOW;
      if (!priceIsForever(&priceTable[ep][index])) {
        price->duration -= ((sl_zigbee_af_get_current_time()
                             - priceTable[ep][index].startTime)
                            / 60u);
      }
    }
    return true;
  }

  return false;
}

// Sets the price at the index.  Returns false if the index is invalid.
bool sl_zigbee_af_price_set_price_table_entry(uint8_t endpoint,
                                              uint8_t index,
                                              const sl_zigbee_af_scheduled_price_t *price)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return false;
  }

  if (index < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_PRICE_TABLE_SIZE) {
    if (price == NULL) {
      priceTable[ep][index].priceControl &= ~ACTIVE;
      return true;
    }

    (void) memmove(&priceTable[ep][index], price, sizeof(sl_zigbee_af_scheduled_price_t));

    // Rember if this is a "start now" price, but store the start time as the
    // current time so the duration can be adjusted.
    if (priceTable[ep][index].startTime == ZCL_PRICE_CLUSTER_START_TIME_NOW) {
      priceTable[ep][index].priceControl |= NOW;
      priceTable[ep][index].startTime = sl_zigbee_af_get_current_time();
    } else {
      priceTable[ep][index].priceControl &= ~NOW;
    }

    priceTable[ep][index].priceControl |= (VALID | ACTIVE);
    return true;
  }
  return false;
}

// Returns the index in the price table of the current price.  The first price
// in the table that starts in the past and ends in the future in considered
// the current price.
static uint8_t getCurrentPriceIndex(uint8_t endpoint)
{
  uint32_t now = sl_zigbee_af_get_current_time();
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  uint8_t i;

  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return ZCL_PRICE_INVALID_ENDPOINT_INDEX;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_PRICE_TABLE_SIZE; i++) {
    if (priceIsValid(&priceTable[ep][i]) != 0U) {
      uint32_t endTime = ((priceTable[ep][i].duration
                           == ZCL_PRICE_CLUSTER_DURATION16_UNTIL_CHANGED)
                          ? ZCL_PRICE_CLUSTER_END_TIME_NEVER
                          : priceTable[ep][i].startTime + priceTable[ep][i].duration * 60u);

      sl_zigbee_af_price_cluster_print("checking price %02X, currTime %08X, start %08X, end %08X ",
                                       i,
                                       now,
                                       priceTable[ep][i].startTime,
                                       endTime);

      if (priceTable[ep][i].startTime <= now && now < endTime) {
        sl_zigbee_af_price_cluster_println("valid");
        sl_zigbee_af_price_cluster_flush();
        return i;
      } else {
        sl_zigbee_af_price_cluster_println("no");
        sl_zigbee_af_price_cluster_flush();
      }
    }
  }

  return ZCL_PRICE_INVALID_INDEX;
}

// Retrieves the current price.  Returns false is there is no current price.
bool sl_zigbee_af_get_current_price(uint8_t endpoint, sl_zigbee_af_scheduled_price_t *price)
{
  return sl_zigbee_af_price_get_price_table_entry(endpoint, getCurrentPriceIndex(endpoint), price);
}

void sl_zigbee_af_price_print(const sl_zigbee_af_scheduled_price_t *price)
{
  sl_zigbee_af_price_cluster_print("  label: ");
  sl_zigbee_af_price_cluster_print_string(price->rateLabel);

  sl_zigbee_af_price_cluster_print("(%02X)\r\n  uom/cur: 0x%02X/0x%04X"
                                   "\r\n  pid/eid: 0x%08X/0x%08X"
                                   "\r\n  ct/st/dur: 0x%08X/0x%08X/",
                                   sl_zigbee_af_string_length(price->rateLabel),
                                   price->unitOfMeasure,
                                   price->currency,
                                   price->providerId,
                                   price->issuerEventID,
                                   sl_zigbee_af_get_current_time(),
                                   price->startTime);
  if (price->duration == ZCL_PRICE_CLUSTER_DURATION16_UNTIL_CHANGED) {
    sl_zigbee_af_price_cluster_print("INF");
  } else {
    sl_zigbee_af_price_cluster_print("0x%04X", price->duration);
  }
  sl_zigbee_af_price_cluster_flush();
  sl_zigbee_af_price_cluster_println("\r\n  ptdt/ptrt: 0x%02X/0x%02X"
                                     "\r\n  p/pr: 0x%08X/0x%02X"
                                     "\r\n  gp/gpr: 0x%08X/0x%02X"
                                     "\r\n  acd/acu/actd: 0x%08X/0x%02X/0x%02X",
                                     price->priceTrailingDigitAndTier,
                                     price->numberOfPriceTiersAndTier,
                                     price->price,
                                     price->priceRatio,
                                     price->generationPrice,
                                     price->generationPriceRatio,
                                     price->alternateCostDelivered,
                                     price->alternateCostUnit,
                                     price->alternateCostTrailingDigit);
  sl_zigbee_af_price_cluster_flush();
  sl_zigbee_af_price_cluster_println("  nobt: 0x%02X", price->numberOfBlockThresholds);
  sl_zigbee_af_price_cluster_println("  pc: 0x%02X",
                                     (price->priceControl
                                      & ZCL_PRICE_CLUSTER_RESERVED_MASK));
  sl_zigbee_af_price_cluster_print("  price is valid from time 0x%08X until ",
                                   price->startTime);
  if (price->duration == ZCL_PRICE_CLUSTER_DURATION16_UNTIL_CHANGED) {
    sl_zigbee_af_price_cluster_println("eternity");
  } else {
    sl_zigbee_af_price_cluster_println("0x%08X",
                                       (price->startTime + (price->duration * 60)));
  }
  sl_zigbee_af_price_cluster_flush();
}

void sl_zigbee_af_price_print_price_table(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  uint8_t currPriceIndex = getCurrentPriceIndex(endpoint);

  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX || currPriceIndex == 0xFF) {
    return;
  }

  sl_zigbee_af_price_cluster_flush();
  sl_zigbee_af_price_cluster_println("Configured Prices: (total %02X, curr index %02X)",
                                     scheduledPriceCount(endpoint, 0),
                                     currPriceIndex);
  sl_zigbee_af_price_cluster_flush();
  sl_zigbee_af_price_cluster_println("  Note: ALL values given in HEX\r\n");
  sl_zigbee_af_price_cluster_flush();
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_PRICE_TABLE_SIZE; i++) {
    if (!priceIsValid(&priceTable[ep][i])) {
      continue;
    }
    sl_zigbee_af_price_cluster_println("= PRICE %02X =%s",
                                       i,
                                       (i == currPriceIndex ? " (Current Price)" : ""));
    sl_zigbee_af_price_print(&priceTable[ep][i]);
  }
}

void sl_zigbee_af_print_billing_period_table(uint8_t endpoint)
{
  uint8_t i = 0;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  sl_zigbee_af_price_common_info_t * curInfo = NULL;
  sl_zigbee_af_price_billing_period_t * billingPeriod = NULL;

  sl_zigbee_af_price_cluster_println("= Billing Periods =");
  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE; i++) {
    curInfo = &priceServerInfo.billingPeriodTable.commonInfos[ep][i];
    billingPeriod = &priceServerInfo.billingPeriodTable.billingPeriods[ep][i];

    sl_zigbee_af_price_cluster_println("  [%d]: valid: 0x%02X", i, curInfo->valid);
    sl_zigbee_af_price_cluster_println("  [%d]: startTime: 0x%08X", i, curInfo->startTime);
    sl_zigbee_af_price_cluster_println("  [%d]: issuerEventId: 0x%08X", i, curInfo->issuerEventId);
    sl_zigbee_af_price_cluster_println("  [%d]: providerId: 0x%08X", i, billingPeriod->providerId);
    sl_zigbee_af_price_cluster_println("  [%d]: billingPeriodDuration: 0x%08X", i, billingPeriod->billingPeriodDuration);
    sl_zigbee_af_price_cluster_println("  [%d]: billingPeriodDurationType: 0x%02X", i, billingPeriod->billingPeriodDurationType);
    sl_zigbee_af_price_cluster_println("  [%d]: tariffType: 0x%02X", i, billingPeriod->tariffType);
  }
}

void sl_zigbee_af_print_consolidated_bill_table_entry(uint8_t endpoint, uint8_t index)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return;
  }
  sl_zigbee_af_price_cluster_println("= Consolidated Bill Table[%d] =", index);
  //uint8_t i = 0;
  //for ( i=0; i<SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE; i++){
  if ( index < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONSOLIDATED_BILL_TABLE_SIZE ) {
    sl_zigbee_af_price_cluster_println("  [%d]: valid: 0x%02X", index, priceServerInfo.consolidatedBillsTable.commonInfos[ep][index].valid);
    sl_zigbee_af_price_cluster_println("  [%d]: providerId: 0x%08X", index, priceServerInfo.consolidatedBillsTable.consolidatedBills[ep][index].providerId);
    sl_zigbee_af_price_cluster_println("  [%d]: issuerEventId: 0x%02X", index, priceServerInfo.consolidatedBillsTable.commonInfos[ep][index].issuerEventId);
    sl_zigbee_af_price_cluster_println("  [%d]: startTime: 0x%08X", index, priceServerInfo.consolidatedBillsTable.commonInfos[ep][index].startTime);
    sl_zigbee_af_price_cluster_println("  [%d]: duration (sec): 0x%08X", index, priceServerInfo.consolidatedBillsTable.commonInfos[ep][index].durationSec);
    sl_zigbee_af_price_cluster_println("  [%d]: billingPeriodDuration: 0x%08X", index, priceServerInfo.consolidatedBillsTable.consolidatedBills[ep][index].billingPeriodDuration);
    sl_zigbee_af_price_cluster_println("  [%d]: billingPeriodType: 0x%02X", index, priceServerInfo.consolidatedBillsTable.consolidatedBills[ep][index].billingPeriodDurationType);
    sl_zigbee_af_price_cluster_println("  [%d]: tariffType: 0x%02X", index, priceServerInfo.consolidatedBillsTable.consolidatedBills[ep][index].tariffType);
    sl_zigbee_af_price_cluster_println("  [%d]: consolidatedBill: 0x%02X", index, priceServerInfo.consolidatedBillsTable.consolidatedBills[ep][index].consolidatedBill);
    sl_zigbee_af_price_cluster_println("  [%d]: currency: 0x%02X", index, priceServerInfo.consolidatedBillsTable.consolidatedBills[ep][index].currency);
    sl_zigbee_af_price_cluster_println("  [%d]: billTrailingDigit: 0x%02X", index, priceServerInfo.consolidatedBillsTable.consolidatedBills[ep][index].billTrailingDigit);
  }
}

void sl_zigbee_af_print_conversion_table(uint8_t endpoint)
{
  uint8_t i = 0;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return;
  }
  sl_zigbee_af_price_cluster_println("= Conversion Factors =");
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE; i++) {
    sl_zigbee_af_price_cluster_println("  [%d]: startTime: 0x%08X", i, priceServerInfo.conversionFactorTable.commonInfos[ep][i].startTime);
    sl_zigbee_af_price_cluster_println("  [%d]: valid: 0x%02X", i, priceServerInfo.conversionFactorTable.commonInfos[ep][i].valid);
    sl_zigbee_af_price_cluster_println("  [%d]: issuerEventId: 0x%02X", i, priceServerInfo.conversionFactorTable.commonInfos[ep][i].issuerEventId);
    sl_zigbee_af_price_cluster_println("  [%d]: conversionFactor: 0x%08X", i, priceServerInfo.conversionFactorTable.priceConversionFactors[ep][i].conversionFactor);
    sl_zigbee_af_price_cluster_println("  [%d]: conversionFactorTrailingDigit: 0x%02X", i, priceServerInfo.conversionFactorTable.priceConversionFactors[ep][i].conversionFactorTrailingDigit);
  }
}

void sl_zigbee_af_print_co2_values_table(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return;
  }
  sl_zigbee_af_price_cluster_println("= Co2 Values =");
  uint8_t i = 0;
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE; i++) {
    sl_zigbee_af_price_cluster_println("  [%d]: valid: 0x%02X", i, priceServerInfo.co2ValueTable.commonInfos[ep][i].valid);
    sl_zigbee_af_price_cluster_println("  [%d]: providerId: 0x%08X", i, priceServerInfo.co2ValueTable.co2Values[ep][i].providerId);
    sl_zigbee_af_price_cluster_println("  [%d]: issuerEventId: 0x%08X", i, priceServerInfo.co2ValueTable.commonInfos[ep][i].issuerEventId);
    sl_zigbee_af_price_cluster_print("  [%d]: startTime: ", i);
    sl_zigbee_af_print_time(priceServerInfo.co2ValueTable.commonInfos[ep][i].startTime);
    sl_zigbee_af_price_cluster_println("  [%d]: tariffType: 0x%02X", i, priceServerInfo.co2ValueTable.co2Values[ep][i].tariffType);
    sl_zigbee_af_price_cluster_println("  [%d]: co2Value: 0x%08X", i, priceServerInfo.co2ValueTable.co2Values[ep][i].co2Value);
    sl_zigbee_af_price_cluster_println("  [%d]: co2ValueUnit: 0x%02X", i, priceServerInfo.co2ValueTable.co2Values[ep][i].co2ValueUnit);
    sl_zigbee_af_price_cluster_println("  [%d]: co2ValueTrailingDigit: 0x%02X", i, priceServerInfo.co2ValueTable.co2Values[ep][i].co2ValueTrailingDigit);
  }
}

void sl_zigbee_af_print_tier_labels_table(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return;
  }
  uint8_t i = 0;
  uint8_t j;
  sl_zigbee_af_price_cluster_println("= Tier Labels =");
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE; i++) {
    sl_zigbee_af_price_cluster_println("  [%d]: valid: 0x%02X", i, priceServerInfo.tierLabelTable.entry[ep][i].valid);
    sl_zigbee_af_price_cluster_println("  [%d]: providerId: 0x%08X", i, priceServerInfo.tierLabelTable.entry[ep][i].providerId);
    sl_zigbee_af_price_cluster_println("  [%d]: issuerEventId: 0x%08X", i, priceServerInfo.tierLabelTable.entry[ep][i].issuerEventId);
    sl_zigbee_af_price_cluster_println("  [%d]: issuerTariffId: 0x%08X", i, priceServerInfo.tierLabelTable.entry[ep][i].issuerTariffId);
    for ( j = 0; j < priceServerInfo.tierLabelTable.entry[ep][i].numberOfTiers; j++ ) {
      sl_zigbee_af_price_cluster_println("  [%d]: tierId[%d]: 0x%02X", i, j, priceServerInfo.tierLabelTable.entry[ep][i].tierIds[j]);
      sl_zigbee_af_price_cluster_print("  [%d]: tierLabel[%d]: ", i, j);
      sl_zigbee_af_price_cluster_print_string(priceServerInfo.tierLabelTable.entry[ep][i].tierLabels[j]);
      sl_zigbee_af_price_cluster_println("");
    }
  }
}

void sl_zigbee_af_print_calorific_values_table(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return;
  }
  sl_zigbee_af_price_cluster_println("= Calorific Values =");
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE; i++) {
    sl_zigbee_af_price_cluster_println("  [%d]: startTime: 0x%08X", i, priceServerInfo.calorificValueTable.commonInfos[ep][i].startTime);
    sl_zigbee_af_price_cluster_println("  [%d]: valid: 0x%08X", i, priceServerInfo.calorificValueTable.commonInfos[ep][i].valid);
    sl_zigbee_af_price_cluster_println("  [%d]: issuerEventId: 0x%02X", i, priceServerInfo.calorificValueTable.commonInfos[ep][i].issuerEventId);
    sl_zigbee_af_price_cluster_println("  [%d]: calorificValue: 0x%08X", i, priceServerInfo.calorificValueTable.calorificValues[ep][i].calorificValue);
    sl_zigbee_af_price_cluster_println("  [%d]: calorificValueUnit: 0x%02X", i, priceServerInfo.calorificValueTable.calorificValues[ep][i].calorificValueUnit);
    sl_zigbee_af_price_cluster_println("  [%d]: calorificValueTrailingDigit: 0x%02X", i, priceServerInfo.calorificValueTable.calorificValues[ep][i].calorificValueTrailingDigit);
  }
}

void sl_zigbee_af_price_cluster_server_init_cb(uint8_t endpoint)
{
  // set the first entry in the price table
  sl_zigbee_af_scheduled_price_t price;
  price.providerId = 0x00000001;

  // label of "Normal"
  price.rateLabel[0] = 6;
  price.rateLabel[1] = (uint8_t) 'N';
  price.rateLabel[2] = (uint8_t) 'o';
  price.rateLabel[3] = (uint8_t) 'r';
  price.rateLabel[4] = (uint8_t) 'm';
  price.rateLabel[5] = (uint8_t) 'a';
  price.rateLabel[6] = (uint8_t) 'l';

  // first event
  price.issuerEventID = 0x00000001;

  price.unitOfMeasure = SL_ZIGBEE_ZCL_AMI_UNIT_OF_MEASURE_KILO_WATT_HOURS;

  // this is USD = US dollars
  price.currency = 840;

  // top nibble means 2 digits to right of decimal point
  // bottom nibble the current price tier.
  // Valid values are from 1-15 (0 is not valid)
  // and correspond to the tier labels, 1-15.
  price.priceTrailingDigitAndTier = 0x21;

  // initialize the numberOfPriceTiersAndTier
  price.numberOfPriceTiersAndTier =
    (SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_PRICE_TABLE_SIZE << 4) + 0x00;

  // start time is 0, so it is always valid
  price.startTime = 0x00000000;

  // valid for 1 hr = 60 minutes
  price.duration = 60;

  // price is 0.09 per Kw/Hr
  // we set price as 9 and two digits to right of decimal
  price.price = 9;

  // the next fields arent used
  price.priceRatio = 0xFF;
  price.generationPrice = 0xFFFFFFFFUL;
  price.generationPriceRatio = 0xFF;
  price.alternateCostDelivered = 0xFFFFFFFFUL;
  price.alternateCostUnit = 0xFF;
  price.alternateCostTrailingDigit = 0xFF;
  price.numberOfBlockThresholds = 0xFF;
  price.priceControl = 0x00;

  sl_zigbee_af_price_set_price_table_entry(endpoint, 0, &price);

  partner.index = ZCL_PRICE_INVALID_INDEX;

  // Initialize the price server tick
  sl_zigbee_af_price_cluster_server_init_tick();

  // Clear the block period table.
  sl_zigbee_af_price_clear_block_period_table(endpoint);

  // Clear the tariff table (and hence, initialize it)
  sl_zigbee_af_price_clear_tariff_table(endpoint);

  // Same with Tier Label table
  sl_zigbee_af_price_clear_tier_label_table(endpoint);

  // Do likewise with the price matrix table
  sl_zigbee_af_price_clear_price_matrix_table(endpoint);

  // And also the block thresholds
  sl_zigbee_af_price_clear_block_thresholds_table(endpoint);

  // Clear the Calorific Value table
  sl_zigbee_af_price_clear_calorific_value_table(endpoint);

  // Clear the CO2 Value table.
  sl_zigbee_af_price_server_clear_co2_value(endpoint);
  // Clear the Consolidated bills table
  sl_zigbee_af_price_clear_consolidated_bills_table(endpoint);

  // Clear the Conversion Factor table
  sl_zigbee_af_price_clear_conversion_factor_table(endpoint);

  // Clear the Billing Period table
  sl_zigbee_af_price_clear_billing_period_table(endpoint);

  // Invalidate the tariff cancellation
  sl_zigbee_af_price_clear_cancel_tariff_table(endpoint);

  // Invalidate the currency conversion
  sl_zigbee_af_price_clear_currency_conversion_table(endpoint);
}

static uint32_t sli_zigbee_af_price_server_seconds_until_next_get_scheduled_prices;  // Time until next Get Scheduled Prices should be sent.

void sl_zigbee_af_price_server_send_get_scheduled_prices(uint8_t endpoint)
{
  sl_zigbee_af_scheduled_price_t price;
  bool isCurrentOrScheduled;
  uint8_t ep;

  ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    //  sl_zigbee_af_price_cluster_println("== ABORT, ep=0xFF, endpoint=%d", endpoint );
    sli_zigbee_af_price_server_seconds_until_next_get_scheduled_prices = PRICE_EVENT_TIME_NO_PENDING_EVENTS;
    return;
  }

  while ( partner.index < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_PRICE_TABLE_SIZE ) {
    isCurrentOrScheduled = priceIsCurrentOrScheduled(&priceTable[ep][partner.index],
                                                     partner.startTime);
    partner.index++;
    if (isCurrentOrScheduled) {
      sl_zigbee_af_price_cluster_println("TX price at index %02X", partner.index - 1);
      sl_zigbee_af_price_get_price_table_entry(endpoint, partner.index - 1u, &price);
      fillPublishPriceCommand(price);

      // Rewrite the sequence number of the response so it matches the request.
      appResponseData[1] = partner.sequence;
      if (partner.isIntraPan) {
        sl_zigbee_af_set_command_endpoints(partner.pan.intra.serverEndpoint,
                                           partner.pan.intra.clientEndpoint);
        sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, partner.pan.intra.nodeId);
      } else {
        sl_zigbee_af_send_command_inter_pan(partner.pan.inter.panId,
                                            partner.pan.inter.eui64,
                                            SL_ZIGBEE_NULL_NODE_ID,
                                            0, // multicast id - unused
                                            SE_PROFILE_ID);
      }

      partner.numberOfEvents--;
      break;
    }
  }

  if ( (partner.numberOfEvents != 0u)
       && (partner.index < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_PRICE_TABLE_SIZE) ) {
//      minEventDelayQSec = 0;    //MILLISECOND_TICKS_PER_QUARTERSECOND;
    sli_zigbee_af_price_server_seconds_until_next_get_scheduled_prices = 0;
  } else {
    partner.index = ZCL_PRICE_INVALID_INDEX;
//    minEventDelayQSec = PRICE_EVENT_TIME_NO_PENDING_EVENTS;
    sli_zigbee_af_price_server_seconds_until_next_get_scheduled_prices = PRICE_EVENT_TIME_NO_PENDING_EVENTS;
  }
}

uint32_t sl_zigbee_af_price_server_seconds_until_get_scheduled_prices_event(void)
{
  return sli_zigbee_af_price_server_seconds_until_next_get_scheduled_prices;
}

static void sli_zigbee_af_price_server_schedule_get_scheduled_prices(uint8_t endpoint)
{
  sli_zigbee_af_price_server_seconds_until_next_get_scheduled_prices = 0;
  sl_zigbee_af_price_cluster_schedule_tick_cb(endpoint, SL_ZIGBEE_AF_PRICE_SERVER_GET_SCHEDULED_PRICES_EVENT_MASK);
}

void sl_zigbee_af_price_server_publish_price_message(sl_802154_short_addr_t nodeId,
                                                     uint8_t srcEndpoint,
                                                     uint8_t dstEndpoint,
                                                     uint8_t priceIndex)
{
  sl_status_t status;
  sl_zigbee_af_scheduled_price_t price;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(srcEndpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return;
  }

  if (!sl_zigbee_af_price_get_price_table_entry(srcEndpoint, priceIndex, &price)) {
    sl_zigbee_af_price_cluster_println("Invalid price table entry at index %02X", priceIndex);
    return;
  }
  sl_zigbee_af_price_cluster_println("Filling cluster");
  fillPublishPriceCommand(price);

  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_price_cluster_println("Error in publish price %02X", status);
  }
}

void sl_zigbee_af_price_server_publish_tariff_message(sl_802154_short_addr_t nodeId,
                                                      uint8_t srcEndpoint,
                                                      uint8_t dstEndpoint,
                                                      uint8_t tariffIndex)
{
  sl_status_t status;
  sl_zigbee_af_scheduled_tariff_t tariff;
  sl_zigbee_af_price_common_info_t priceInfo;

  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(srcEndpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return;
  }

  if (!sl_zigbee_af_price_get_tariff_table_entry(srcEndpoint, tariffIndex, &priceInfo, &tariff)
      || (!priceInfo.valid)) {
    sl_zigbee_af_price_cluster_println("Invalid tariff table entry at index %02X", tariffIndex);
    return;
  }

  (void) sl_zigbee_af_fill_command_price_cluster_publish_tariff_information(tariff.providerId,
                                                                            priceInfo.issuerEventId,
                                                                            tariff.issuerTariffId,
                                                                            priceInfo.startTime,
                                                                            tariff.tariffTypeChargingScheme,
                                                                            tariff.tariffLabel,
                                                                            tariff.numberOfPriceTiersInUse,
                                                                            tariff.numberOfBlockThresholdsInUse,
                                                                            tariff.unitOfMeasure,
                                                                            tariff.currency,
                                                                            tariff.priceTrailingDigit,
                                                                            tariff.standingCharge,
                                                                            tariff.tierBlockMode,
                                                                            tariff.blockThresholdMultiplier,
                                                                            tariff.blockThresholdDivisor);

  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_price_cluster_println("Error in publish tariff %02X", status);
  }
}

// Look through the price table to find the first free/unused entry;
// return ZCL_PRICE_INVALID_INDEX if price table is full
uint8_t sl_zigbee_af_price_find_free_price_index(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint,
                                                               ZCL_PRICE_CLUSTER_ID);
  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return ZCL_PRICE_INVALID_INDEX;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_PRICE_TABLE_SIZE; i++) {
    if (!priceIsCurrentOrScheduled(&priceTable[ep][i],
                                   sl_zigbee_af_get_current_time())) {
      return i;
    }
  }

  sl_zigbee_af_price_cluster_println("Error: Price table full");
  return ZCL_PRICE_INVALID_INDEX;
}

// send replies
void sendValidCmdEntries(uint8_t cmdId,
                         uint8_t endpoint,
                         uint8_t* validEntries,
                         uint8_t validEntryCount)
{
  uint8_t i, j;
  uint8_t sentCmd = 0;
  if (validEntryCount == 0u) {
    goto kickout;
  }

  for (i = 0; i < validEntryCount; i++) {
    uint8_t tableIndex = validEntries[i];

    // discard command entries. this is used to satisfy additional logic
    // required by command such as CO2Value
    if (tableIndex == 0xFFu) {
      continue;
    }

    if (cmdId == ZCL_PUBLISH_CONVERSION_FACTOR_COMMAND_ID) {
      sl_zigbee_af_price_cluster_println("Sending conversion factor: table[%d]", tableIndex);
      (void) sl_zigbee_af_fill_command_price_cluster_publish_conversion_factor(priceServerInfo.conversionFactorTable.commonInfos[endpoint][tableIndex].issuerEventId,
                                                                               priceServerInfo.conversionFactorTable.commonInfos[endpoint][tableIndex].startTime,
                                                                               priceServerInfo.conversionFactorTable.priceConversionFactors[endpoint][tableIndex].conversionFactor,
                                                                               priceServerInfo.conversionFactorTable.priceConversionFactors[endpoint][tableIndex].conversionFactorTrailingDigit);
    } else if (cmdId == ZCL_PUBLISH_CALORIFIC_VALUE_COMMAND_ID) {
      sl_zigbee_af_price_cluster_println("Sending calorific value: table[%d]", tableIndex);
      (void) sl_zigbee_af_fill_command_price_cluster_publish_calorific_value(priceServerInfo.calorificValueTable.commonInfos[endpoint][tableIndex].issuerEventId,
                                                                             priceServerInfo.calorificValueTable.commonInfos[endpoint][tableIndex].startTime,
                                                                             priceServerInfo.calorificValueTable.calorificValues[endpoint][tableIndex].calorificValue,
                                                                             priceServerInfo.calorificValueTable.calorificValues[endpoint][tableIndex].calorificValueUnit,
                                                                             priceServerInfo.calorificValueTable.calorificValues[endpoint][tableIndex].calorificValueTrailingDigit);
    } else if (cmdId == ZCL_PUBLISH_CO2_VALUE_COMMAND_ID) {
      sl_zigbee_af_price_cluster_println("Sending Co2 value: table[%d]", tableIndex);
      (void) sl_zigbee_af_fill_command_price_cluster_publish_co2_value(priceServerInfo.co2ValueTable.co2Values[endpoint][tableIndex].providerId,
                                                                       priceServerInfo.co2ValueTable.commonInfos[endpoint][tableIndex].issuerEventId,
                                                                       priceServerInfo.co2ValueTable.commonInfos[endpoint][tableIndex].startTime,
                                                                       priceServerInfo.co2ValueTable.co2Values[endpoint][tableIndex].tariffType,
                                                                       priceServerInfo.co2ValueTable.co2Values[endpoint][tableIndex].co2Value,
                                                                       priceServerInfo.co2ValueTable.co2Values[endpoint][tableIndex].co2ValueUnit,
                                                                       priceServerInfo.co2ValueTable.co2Values[endpoint][tableIndex].co2ValueTrailingDigit);
    } else if ( cmdId == ZCL_PUBLISH_TIER_LABELS_COMMAND_ID ) {
      sl_zigbee_af_price_cluster_println("Sending tier labels: table[%d]", tableIndex);
      (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND                                              \
                                                | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT),                                    \
                                               ZCL_PRICE_CLUSTER_ID,                                                      \
                                               ZCL_PUBLISH_TIER_LABELS_COMMAND_ID,                                        \
                                               "wwwuuu",                                                                  \
                                               priceServerInfo.tierLabelTable.entry[endpoint][tableIndex].providerId,     \
                                               priceServerInfo.tierLabelTable.entry[endpoint][tableIndex].issuerEventId,  \
                                               priceServerInfo.tierLabelTable.entry[endpoint][tableIndex].issuerTariffId, \
                                               0,                                                                         \
                                               0,                                                                         \
                                               priceServerInfo.tierLabelTable.entry[endpoint][tableIndex].numberOfTiers);
      for ( j = 0; j < priceServerInfo.tierLabelTable.entry[endpoint][tableIndex].numberOfTiers; j++ ) {
        (void) sl_zigbee_af_put_int8u_in_resp(priceServerInfo.tierLabelTable.entry[endpoint][tableIndex].tierIds[j]);
        (void) sl_zigbee_af_put_string_in_resp(priceServerInfo.tierLabelTable.entry[endpoint][tableIndex].tierLabels[j]);
      }
    } else if (cmdId == ZCL_PUBLISH_BILLING_PERIOD_COMMAND_ID) {
      sl_zigbee_af_price_cluster_println("Sending billing period: table[%d]", tableIndex);
      (void) sl_zigbee_af_fill_command_price_cluster_publish_billing_period(priceServerInfo.billingPeriodTable.billingPeriods[endpoint][tableIndex].providerId,
                                                                            priceServerInfo.billingPeriodTable.commonInfos[endpoint][tableIndex].issuerEventId,
                                                                            priceServerInfo.billingPeriodTable.billingPeriods[endpoint][tableIndex].rawBillingPeriodStartTime,
                                                                            priceServerInfo.billingPeriodTable.billingPeriods[endpoint][tableIndex].billingPeriodDuration,
                                                                            priceServerInfo.billingPeriodTable.billingPeriods[endpoint][tableIndex].billingPeriodDurationType,
                                                                            priceServerInfo.billingPeriodTable.billingPeriods[endpoint][tableIndex].tariffType);
    } else if ( cmdId == ZCL_PUBLISH_CONSOLIDATED_BILL_COMMAND_ID ) {
      sl_zigbee_af_price_cluster_println("Sending Consolidated Bill: table[%d]", tableIndex);
      (void) sl_zigbee_af_fill_command_price_cluster_publish_consolidated_bill(priceServerInfo.consolidatedBillsTable.consolidatedBills[endpoint][tableIndex].providerId,
                                                                               priceServerInfo.consolidatedBillsTable.commonInfos[endpoint][tableIndex].issuerEventId,
                                                                               priceServerInfo.consolidatedBillsTable.consolidatedBills[endpoint][tableIndex].rawStartTimeUtc,
                                                                               priceServerInfo.consolidatedBillsTable.consolidatedBills[endpoint][tableIndex].billingPeriodDuration,
                                                                               priceServerInfo.consolidatedBillsTable.consolidatedBills[endpoint][tableIndex].billingPeriodDurationType,
                                                                               priceServerInfo.consolidatedBillsTable.consolidatedBills[endpoint][tableIndex].tariffType,
                                                                               priceServerInfo.consolidatedBillsTable.consolidatedBills[endpoint][tableIndex].consolidatedBill,
                                                                               priceServerInfo.consolidatedBillsTable.consolidatedBills[endpoint][tableIndex].currency,
                                                                               priceServerInfo.consolidatedBillsTable.consolidatedBills[endpoint][tableIndex].billTrailingDigit);
    } else if ( cmdId == ZCL_PUBLISH_CREDIT_PAYMENT_COMMAND_ID ) {
      sl_zigbee_af_price_cluster_println("Sending Credit Payment: table[%d]", tableIndex);
      (void) sl_zigbee_af_fill_command_price_cluster_publish_credit_payment(priceServerInfo.creditPaymentTable.creditPayment[endpoint][tableIndex].providerId,
                                                                            priceServerInfo.creditPaymentTable.commonInfos[endpoint][tableIndex].issuerEventId,
                                                                            priceServerInfo.creditPaymentTable.creditPayment[endpoint][tableIndex].creditPaymentDueDate,
                                                                            priceServerInfo.creditPaymentTable.creditPayment[endpoint][tableIndex].creditPaymentAmountOverdue,
                                                                            priceServerInfo.creditPaymentTable.creditPayment[endpoint][tableIndex].creditPaymentStatus,
                                                                            priceServerInfo.creditPaymentTable.creditPayment[endpoint][tableIndex].creditPayment,
                                                                            priceServerInfo.creditPaymentTable.creditPayment[endpoint][tableIndex].creditPaymentDate,
                                                                            priceServerInfo.creditPaymentTable.creditPayment[endpoint][tableIndex].creditPaymentRef);
    } else if ( cmdId == ZCL_PUBLISH_BLOCK_PERIOD_COMMAND_ID ) {
      sl_zigbee_af_price_cluster_println("Sending Block Period: table[%d]", tableIndex);
      (void) sl_zigbee_af_fill_command_price_cluster_publish_block_period(priceServerInfo.blockPeriodTable.blockPeriods[endpoint][tableIndex].providerId,
                                                                          priceServerInfo.blockPeriodTable.commonInfos[endpoint][tableIndex].issuerEventId,
                                                                          priceServerInfo.blockPeriodTable.blockPeriods[endpoint][tableIndex].rawBlockPeriodStartTime,
                                                                          priceServerInfo.blockPeriodTable.blockPeriods[endpoint][tableIndex].blockPeriodDuration,
                                                                          priceServerInfo.blockPeriodTable.blockPeriods[endpoint][tableIndex].blockPeriodControl,
                                                                          priceServerInfo.blockPeriodTable.blockPeriods[endpoint][tableIndex].blockPeriodDurationType,
                                                                          priceServerInfo.blockPeriodTable.blockPeriods[endpoint][tableIndex].tariffType,
                                                                          priceServerInfo.blockPeriodTable.blockPeriods[endpoint][tableIndex].tariffResolutionPeriod);
    } else if (cmdId == ZCL_PUBLISH_TARIFF_INFORMATION_COMMAND_ID ) {
      sl_zigbee_af_scheduled_tariff_t * tariff = &priceServerInfo.scheduledTariffTable.scheduledTariffs[endpoint][tableIndex];
      sl_zigbee_af_price_common_info_t * priceServerInfos = &priceServerInfo.scheduledTariffTable.commonInfos[endpoint][tableIndex];

      sl_zigbee_af_price_cluster_println("TX: PublishTariffInfo: table[%d]", tableIndex);
      (void) sl_zigbee_af_fill_command_price_cluster_publish_tariff_information(tariff->providerId,
                                                                                priceServerInfos->issuerEventId,
                                                                                tariff->issuerTariffId,
                                                                                priceServerInfos->startTime,
                                                                                tariff->tariffTypeChargingScheme,
                                                                                tariff->tariffLabel,
                                                                                tariff->numberOfPriceTiersInUse,
                                                                                tariff->numberOfBlockThresholdsInUse,
                                                                                tariff->unitOfMeasure,
                                                                                tariff->currency,
                                                                                tariff->priceTrailingDigit,
                                                                                tariff->standingCharge,
                                                                                tariff->tierBlockMode,
                                                                                tariff->blockThresholdMultiplier,
                                                                                tariff->blockThresholdDivisor);
    } else {
      goto kickout;
    }

    sl_zigbee_af_send_response();
    sentCmd++;
    sl_zigbee_af_price_cluster_println("  sent=%d", sentCmd);
  }

  if (sentCmd != 0u) {
    return;
  }

  kickout:
  sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_NOT_FOUND);
}

// CONVERSION FACTOR
static void sl_zigbee_af_price_clear_conversion_factor_table(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE; i++ ) {
    priceServerInfo.conversionFactorTable.commonInfos[ep][i].valid = false;
  }
}

sl_zigbee_af_status_t sl_zigbee_af_price_server_conversion_factor_add(uint8_t endpoint,
                                                                      uint32_t issuerEventId,
                                                                      uint32_t startTime,
                                                                      uint32_t conversionFactor,
                                                                      uint8_t conversionFactorTrailingDigit)
{
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  uint8_t index;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    sl_zigbee_af_price_cluster_println("Error: Index endpoint!");
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
    goto kickout;
  }
  index = sl_zigbee_af_price_common_get_common_matching_or_unused_index(priceServerInfo.conversionFactorTable.commonInfos[ep],
                                                                        SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE,
                                                                        issuerEventId, startTime, true);

  if (index >= SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE) {
    sl_zigbee_af_price_cluster_println("Error: Index out of bound!");
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
    goto kickout;
  }

  priceServerInfo.conversionFactorTable.commonInfos[ep][index].startTime = startTime;
  priceServerInfo.conversionFactorTable.commonInfos[ep][index].valid = true;
  priceServerInfo.conversionFactorTable.commonInfos[ep][index].issuerEventId = issuerEventId;
  priceServerInfo.conversionFactorTable.commonInfos[ep][index].durationSec = ZCL_PRICE_CLUSTER_DURATION_SEC_UNTIL_CHANGED;
  priceServerInfo.conversionFactorTable.priceConversionFactors[ep][index].conversionFactor = conversionFactor;
  priceServerInfo.conversionFactorTable.priceConversionFactors[ep][index].conversionFactorTrailingDigit = conversionFactorTrailingDigit;
  sl_zigbee_af_price_common_sort(priceServerInfo.conversionFactorTable.commonInfos[ep],
                                 (uint8_t *)priceServerInfo.conversionFactorTable.priceConversionFactors[ep],
                                 (uint16_t)sizeof(sl_zigbee_af_price_conversion_factor_t),
                                 SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE);
  sl_zigbee_af_price_common_update_duration_for_overlapping_events(priceServerInfo.conversionFactorTable.commonInfos[ep],
                                                                   SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE);

  if (startTime <= sl_zigbee_af_get_current_time()) {
    sl_zigbee_af_price_update_conversion_factor_attribs(endpoint, conversionFactor, conversionFactorTrailingDigit);
  }
  sl_zigbee_af_price_cluster_schedule_tick_cb(endpoint, SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_CONVERSION_FACTOR_EVENT_MASK);

  kickout:
  return status;
}

static void sl_zigbee_af_price_update_conversion_factor_attribs(uint8_t endpoint, uint32_t conversionFactor, uint8_t conversionFactorTrailingDigit)
{
  // Assumes the conversion factor table is already sorted.
  (void) sl_zigbee_af_write_server_attribute(endpoint,
                                             ZCL_PRICE_CLUSTER_ID,
                                             ZCL_CONVERSION_FACTOR_ATTRIBUTE_ID,
                                             (uint8_t *)&conversionFactor,
                                             ZCL_INT32U_ATTRIBUTE_TYPE);
  (void) sl_zigbee_af_write_server_attribute(endpoint,
                                             ZCL_PRICE_CLUSTER_ID,
                                             ZCL_CONVERSION_FACTOR_TRAILING_DIGIT_ATTRIBUTE_ID,
                                             (uint8_t *)&conversionFactorTrailingDigit,
                                             ZCL_INT8U_ATTRIBUTE_TYPE);
}

uint32_t sl_zigbee_af_price_server_seconds_until_conversion_factor_event(uint8_t endpoint)
{
  uint32_t secondsTillNext;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return 0xFFFFFFFFU;
  }

  secondsTillNext = sl_zigbee_af_price_common_seconds_until_second_index_active(priceServerInfo.conversionFactorTable.commonInfos[ep],
                                                                                SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE);

  return secondsTillNext;
}

void sl_zigbee_af_price_server_refresh_conversion_factor(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }

  if ( 0u == sl_zigbee_af_price_common_seconds_until_second_index_active(priceServerInfo.conversionFactorTable.commonInfos[ep],
                                                                         SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE) ) {
    // Replace current with next.
    priceServerInfo.conversionFactorTable.commonInfos[ep][0].valid = false;
    sl_zigbee_af_price_common_sort(priceServerInfo.conversionFactorTable.commonInfos[ep],
                                   (uint8_t *)priceServerInfo.conversionFactorTable.priceConversionFactors[ep],
                                   (uint16_t)sizeof(sl_zigbee_af_price_conversion_factor_t),
                                   SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE);
    sl_zigbee_af_price_common_update_duration_for_overlapping_events(priceServerInfo.conversionFactorTable.commonInfos[ep],
                                                                     SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE);
    if ( priceServerInfo.conversionFactorTable.commonInfos[ep][0].valid ) {
      sl_zigbee_af_price_update_conversion_factor_attribs(endpoint,
                                                          priceServerInfo.conversionFactorTable.priceConversionFactors[ep][0].conversionFactor,
                                                          priceServerInfo.conversionFactorTable.priceConversionFactors[ep][0].conversionFactorTrailingDigit);
    }
  }
}

// CALORIFIC VALUE FUNCTIONS
static void sl_zigbee_af_price_clear_calorific_value_table(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE; i++ ) {
    priceServerInfo.calorificValueTable.commonInfos[ep][i].valid = false;
  }
}

sl_zigbee_af_status_t sl_zigbee_af_price_server_calorific_value_add(uint8_t endpoint,
                                                                    uint32_t issuerEventId,
                                                                    uint32_t startTime,
                                                                    uint32_t calorificValue,
                                                                    uint8_t calorificValueUnit,
                                                                    uint8_t calorificValueTrailingDigit)
{
  //Lint requires initialization.
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  uint8_t index;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    sl_zigbee_af_price_cluster_println("Error: Invalid endpoint!");
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
    goto kickout;
  }

  index = sl_zigbee_af_price_common_get_common_matching_or_unused_index(priceServerInfo.calorificValueTable.commonInfos[ep],
                                                                        SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE,
                                                                        issuerEventId, startTime, true);

  if (index >= SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE) {
    sl_zigbee_af_price_cluster_println("Error: Index out of bound!");
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
    goto kickout;
  }

  priceServerInfo.calorificValueTable.commonInfos[ep][index].startTime = startTime;
  priceServerInfo.calorificValueTable.commonInfos[ep][index].valid = true;
  priceServerInfo.calorificValueTable.commonInfos[ep][index].issuerEventId = issuerEventId;
  priceServerInfo.calorificValueTable.commonInfos[ep][index].durationSec = ZCL_PRICE_CLUSTER_DURATION_SEC_UNTIL_CHANGED;
  priceServerInfo.calorificValueTable.calorificValues[ep][index].calorificValue = calorificValue;
  priceServerInfo.calorificValueTable.calorificValues[ep][index].calorificValueUnit = calorificValueUnit;
  priceServerInfo.calorificValueTable.calorificValues[ep][index].calorificValueTrailingDigit = calorificValueTrailingDigit;
  sl_zigbee_af_price_common_sort(priceServerInfo.calorificValueTable.commonInfos[ep],
                                 (uint8_t *)priceServerInfo.calorificValueTable.calorificValues[ep],
                                 (uint16_t)sizeof(sl_zigbee_af_price_calorific_value_t),
                                 SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE);
  sl_zigbee_af_price_common_update_duration_for_overlapping_events(priceServerInfo.calorificValueTable.commonInfos[ep],
                                                                   SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE);

  if (startTime <= sl_zigbee_af_get_current_time()) {
    sl_zigbee_af_price_update_calorific_value_attribs(endpoint, calorificValue, calorificValueUnit, calorificValueTrailingDigit);
  }
  sl_zigbee_af_price_cluster_schedule_tick_cb(endpoint, SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_CALORIFIC_VALUE_EVENT_MASK);
  kickout:
  return status;
}

static void sl_zigbee_af_price_update_calorific_value_attribs(uint8_t endpoint, uint32_t calorificValue,
                                                              uint8_t calorificValueUnit, uint8_t calorificValueTrailingDigit)
{
  // Assumes the conversion factor table is already sorted.
  (void) sl_zigbee_af_write_server_attribute(endpoint,
                                             ZCL_PRICE_CLUSTER_ID,
                                             ZCL_CALORIFIC_VALUE_ATTRIBUTE_ID,
                                             (uint8_t *)&calorificValue,
                                             ZCL_INT32U_ATTRIBUTE_TYPE);

  (void) sl_zigbee_af_write_server_attribute(endpoint,
                                             ZCL_PRICE_CLUSTER_ID,
                                             ZCL_CALORIFIC_VALUE_UNIT_ATTRIBUTE_ID,
                                             (uint8_t *)&calorificValueUnit,
                                             ZCL_ENUM8_ATTRIBUTE_TYPE);

  (void) sl_zigbee_af_write_server_attribute(endpoint,
                                             ZCL_PRICE_CLUSTER_ID,
                                             ZCL_CALORIFIC_VALUE_TRAILING_DIGIT_ATTRIBUTE_ID,
                                             (uint8_t *)&calorificValueTrailingDigit,
                                             ZCL_BITMAP8_ATTRIBUTE_TYPE);
}

uint32_t sl_zigbee_af_price_server_seconds_until_calorific_value_event(uint8_t endpoint)
{
  uint32_t secondsTillNext;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return 0xFFFFFFFFU;
  }

  secondsTillNext = sl_zigbee_af_price_common_seconds_until_second_index_active(priceServerInfo.calorificValueTable.commonInfos[ep],
                                                                                SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE);
  return secondsTillNext;
}

void sl_zigbee_af_price_server_refresh_calorific_value(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }

  if ( 0u == sl_zigbee_af_price_common_seconds_until_second_index_active(priceServerInfo.calorificValueTable.commonInfos[ep],
                                                                         SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE) ) {
    // Replace current with next.
    priceServerInfo.calorificValueTable.commonInfos[ep][0].valid = false;
    sl_zigbee_af_price_common_sort(priceServerInfo.calorificValueTable.commonInfos[ep],
                                   (uint8_t *)priceServerInfo.calorificValueTable.calorificValues[ep],
                                   (uint16_t)sizeof(sl_zigbee_af_price_calorific_value_t),
                                   SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE);
    sl_zigbee_af_price_common_update_duration_for_overlapping_events(priceServerInfo.calorificValueTable.commonInfos[ep],
                                                                     SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE);
    if ( priceServerInfo.calorificValueTable.commonInfos[ep][0].valid ) {
      sl_zigbee_af_price_update_calorific_value_attribs(endpoint,
                                                        priceServerInfo.calorificValueTable.calorificValues[ep][0].calorificValue,
                                                        priceServerInfo.calorificValueTable.calorificValues[ep][0].calorificValueUnit,
                                                        priceServerInfo.calorificValueTable.calorificValues[ep][0].calorificValueTrailingDigit);
    }
  }
}

void sl_zigbee_af_price_server_calorific_value_clear(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  (void) memset(&priceServerInfo.calorificValueTable, 0, sizeof(sl_zigbee_af_price_calorific_value_table_t));
}

void sl_zigbee_af_price_server_conversion_factor_clear(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  (void) memset(&priceServerInfo.conversionFactorTable, 0, sizeof(sl_zigbee_af_price_conversion_factor_table_t));
}

void sl_zigbee_af_price_server_co2_value_clear(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  (void) memset(&priceServerInfo.co2ValueTable, 0, sizeof(sl_zigbee_af_price_co2_table_t));
}

void sl_zigbee_af_price_server_conversion_factor_pub(uint8_t tableIndex,
                                                     sl_802154_short_addr_t dstAddr,
                                                     uint8_t srcEndpoint,
                                                     uint8_t dstEndpoint)
{
  sl_status_t status;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(srcEndpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  if (tableIndex >= SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE) {
    sl_zigbee_af_price_cluster_println("Error: index out of bound!");
    return;
  }

  if (priceServerInfo.conversionFactorTable.commonInfos[ep][tableIndex].valid == 0) {
    sl_zigbee_af_price_cluster_println("Error: Table entry not valid!");
    return;
  }

  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  (void) sl_zigbee_af_fill_command_price_cluster_publish_conversion_factor(priceServerInfo.conversionFactorTable.commonInfos[ep][tableIndex].issuerEventId,
                                                                           priceServerInfo.conversionFactorTable.commonInfos[ep][tableIndex].startTime,
                                                                           priceServerInfo.conversionFactorTable.priceConversionFactors[ep][tableIndex].conversionFactor,
                                                                           priceServerInfo.conversionFactorTable.priceConversionFactors[ep][tableIndex].conversionFactorTrailingDigit);
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, dstAddr);

  if (status != SL_STATUS_OK) {
    sl_zigbee_af_price_cluster_println("Unable to send PublishConversionFactor to 0x%04X: 0x%02X", dstAddr, status);
  }
}

static void sl_zigbee_af_price_clear_tier_label_table(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE; i++ ) {
    priceServerInfo.tierLabelTable.entry[ep][i].valid = 0;
    priceServerInfo.tierLabelTable.entry[ep][i].numberOfTiers = 0;
  }
}

void sl_zigbee_af_price_server_tier_label_set(uint8_t  endpoint,
                                              uint8_t  index,
                                              uint8_t  valid,
                                              uint32_t providerId,
                                              uint32_t issuerEventId,
                                              uint32_t issuerTariffId,
                                              uint8_t tierId,
                                              uint8_t* tierLabel)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  if (index >= SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE) {
    sl_zigbee_af_price_cluster_println("Error: Index out of bound!");
    return;
  }

  priceServerInfo.tierLabelTable.entry[ep][index].valid = valid;
  priceServerInfo.tierLabelTable.entry[ep][index].providerId = providerId;
  priceServerInfo.tierLabelTable.entry[ep][index].issuerEventId = issuerEventId;
  priceServerInfo.tierLabelTable.entry[ep][index].issuerTariffId = issuerTariffId;;
  priceServerInfo.tierLabelTable.entry[ep][index].numberOfTiers = 1;    // This command forces only 1 label.
  priceServerInfo.tierLabelTable.entry[ep][index].tierIds[0] = tierId;
  // Truncate string length to not exceed maximum length.
  if ( tierLabel[0] > TIER_LABEL_SIZE ) {
    tierLabel[0] = TIER_LABEL_SIZE;
  }
  (void) memcpy(priceServerInfo.tierLabelTable.entry[ep][index].tierLabels[0], tierLabel, (TIER_LABEL_SIZE + 1));
}

void sl_zigbee_af_price_server_tier_label_add_label(uint8_t endpoint, uint32_t issuerTariffId, uint8_t tierId, uint8_t *tierLabel)
{
  uint8_t i;
  uint8_t numberOfTiers;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE; i++ ) {
    if ( (priceServerInfo.tierLabelTable.entry[ep][i].issuerTariffId == issuerTariffId)
         && (priceServerInfo.tierLabelTable.entry[ep][i].numberOfTiers < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_MAX_TIERS_PER_TARIFF)
         && (priceServerInfo.tierLabelTable.entry[ep][i].valid != 0u) ) {
      // Found matching entry with room for another label.
      // Truncate string length to not exceed maximum length.
      if ( tierLabel[0] > TIER_LABEL_SIZE ) {
        tierLabel[0] = TIER_LABEL_SIZE;
      }
      numberOfTiers = priceServerInfo.tierLabelTable.entry[ep][i].numberOfTiers;
      priceServerInfo.tierLabelTable.entry[ep][i].tierIds[numberOfTiers] = tierId;
      (void) memcpy(priceServerInfo.tierLabelTable.entry[ep][i].tierLabels[numberOfTiers], tierLabel, (TIER_LABEL_SIZE + 1));
      priceServerInfo.tierLabelTable.entry[ep][i].numberOfTiers++;
      break;
    }
  }
  if ( i >= SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE ) {
    sl_zigbee_af_price_cluster_println("Error: Tier Label Add failed - available entry not found.");
  }
}

void sl_zigbee_af_price_server_tier_label_pub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint, uint8_t index)
{
  sl_status_t status;
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(srcEndpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }

  if (index >= SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE) {
    sl_zigbee_af_price_cluster_println("Error: Index out of bound!");
    return;
  }
  if ( priceServerInfo.tierLabelTable.entry[ep][index].numberOfTiers >= SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_MAX_TIERS_PER_TARIFF ) {
    sl_zigbee_af_price_cluster_println("Error: Invalid number of labels!");
    return;
  }

  (void) sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND                                   \
                                            | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT),                         \
                                           ZCL_PRICE_CLUSTER_ID,                                           \
                                           ZCL_PUBLISH_TIER_LABELS_COMMAND_ID,                             \
                                           "wwwuuu",                                                       \
                                           priceServerInfo.tierLabelTable.entry[ep][index].providerId,     \
                                           priceServerInfo.tierLabelTable.entry[ep][index].issuerEventId,  \
                                           priceServerInfo.tierLabelTable.entry[ep][index].issuerTariffId, \
                                           0,                                                              \
                                           0,                                                              \
                                           priceServerInfo.tierLabelTable.entry[ep][index].numberOfTiers);
  for ( i = 0; i < priceServerInfo.tierLabelTable.entry[ep][index].numberOfTiers; i++ ) {
    (void) sl_zigbee_af_put_int8u_in_resp(priceServerInfo.tierLabelTable.entry[ep][index].tierIds[i]);
    (void) sl_zigbee_af_put_string_in_resp(priceServerInfo.tierLabelTable.entry[ep][index].tierLabels[i]);
  }

  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_price_cluster_println("Error: PublishTierLabels failed: %02X", status);
  }
}

///  CO2 VALUE
static void sl_zigbee_af_price_server_clear_co2_value(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE; i++ ) {
    priceServerInfo.co2ValueTable.commonInfos[ep][i].valid = false;
  }
}

uint32_t sl_zigbee_af_price_server_seconds_until_co2_value_event(uint8_t endpoint)
{
  uint32_t secondsTillNext;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return 0xFFFFFFFFU;
  }

  secondsTillNext = sl_zigbee_af_price_common_seconds_until_second_index_active(priceServerInfo.co2ValueTable.commonInfos[ep],
                                                                                SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE);
  return secondsTillNext;
}

void sl_zigbee_af_price_server_refresh_co2_value(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }

  if ( 0 == sl_zigbee_af_price_common_seconds_until_second_index_active(priceServerInfo.co2ValueTable.commonInfos[ep],
                                                                        SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE) ) {
    // Replace current with next.
    priceServerInfo.co2ValueTable.commonInfos[ep][0].valid = false;
    sl_zigbee_af_price_common_sort(priceServerInfo.co2ValueTable.commonInfos[ep],
                                   (uint8_t *)priceServerInfo.co2ValueTable.co2Values[ep],
                                   (uint16_t)sizeof(sl_zigbee_af_price_co2_value_t),
                                   SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE);
    sl_zigbee_af_price_common_update_duration_for_overlapping_events(priceServerInfo.co2ValueTable.commonInfos[ep],
                                                                     SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE);
  }
}

void sl_zigbee_af_price_server_co2_value_add(uint8_t endpoint,
                                             uint32_t issuerEventId,
                                             uint32_t startTime,
                                             uint32_t providerId,
                                             uint8_t tariffType,
                                             uint32_t co2Value,
                                             uint8_t co2ValueUnit,
                                             uint8_t co2ValueTrailingDigit)
{
  uint8_t index;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  index = sl_zigbee_af_price_common_get_common_matching_or_unused_index(priceServerInfo.co2ValueTable.commonInfos[ep],
                                                                        SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE,
                                                                        issuerEventId, startTime, true);
  if (index >= SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE) {
    sl_zigbee_af_price_cluster_println("ERR: Unable to apply new CO2 Value!");
    return;
  }

  priceServerInfo.co2ValueTable.commonInfos[ep][index].startTime = startTime;
  priceServerInfo.co2ValueTable.commonInfos[ep][index].valid = true;
  priceServerInfo.co2ValueTable.commonInfos[ep][index].issuerEventId = issuerEventId;
  priceServerInfo.co2ValueTable.commonInfos[ep][index].durationSec = ZCL_PRICE_CLUSTER_DURATION_SEC_UNTIL_CHANGED;
  priceServerInfo.co2ValueTable.co2Values[ep][index].providerId = providerId;
  priceServerInfo.co2ValueTable.co2Values[ep][index].tariffType = tariffType;
  priceServerInfo.co2ValueTable.co2Values[ep][index].co2Value = co2Value;
  priceServerInfo.co2ValueTable.co2Values[ep][index].co2ValueUnit = co2ValueUnit;
  priceServerInfo.co2ValueTable.co2Values[ep][index].co2ValueTrailingDigit = co2ValueTrailingDigit;
  sl_zigbee_af_price_common_sort(priceServerInfo.co2ValueTable.commonInfos[ep],
                                 (uint8_t *)priceServerInfo.co2ValueTable.co2Values[ep],
                                 (uint16_t)sizeof(sl_zigbee_af_price_co2_value_t),
                                 SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE);
  sl_zigbee_af_price_common_update_duration_for_overlapping_events(priceServerInfo.co2ValueTable.commonInfos[ep],
                                                                   SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE);
  sl_zigbee_af_price_cluster_schedule_tick_cb(endpoint, SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_CO2_VALUE_EVENT_MASK);
}

void sl_zigbee_af_price_server_co2_label_pub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint, uint8_t index)
{
  sl_status_t status;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(srcEndpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  if (index >= SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE) {
    sl_zigbee_af_price_cluster_println("Error: Index out of bound!");
    return;
  }
  (void) sl_zigbee_af_fill_command_price_cluster_publish_co2_value(priceServerInfo.co2ValueTable.co2Values[ep][index].providerId,
                                                                   priceServerInfo.co2ValueTable.commonInfos[ep][index].issuerEventId,
                                                                   priceServerInfo.co2ValueTable.commonInfos[ep][index].startTime,
                                                                   priceServerInfo.co2ValueTable.co2Values[ep][index].tariffType,
                                                                   priceServerInfo.co2ValueTable.co2Values[ep][index].co2Value,
                                                                   priceServerInfo.co2ValueTable.co2Values[ep][index].co2ValueUnit,
                                                                   priceServerInfo.co2ValueTable.co2Values[ep][index].co2ValueTrailingDigit);
  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_price_cluster_println("Error: PublishCo2Label failed: %02X", status);
  }
}

/// BILLING PERIOD
static void sl_zigbee_af_price_clear_billing_period_table(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE; i++ ) {
    priceServerInfo.billingPeriodTable.commonInfos[ep][i].valid = false;
  }
}

uint32_t sl_zigbee_af_price_server_seconds_until_billing_period_event(uint8_t endpoint)
{
  uint32_t secondsTillNext;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return 0xFFFFFFFFU;
  }

  secondsTillNext = sl_zigbee_af_price_common_seconds_until_second_index_active(priceServerInfo.billingPeriodTable.commonInfos[ep],
                                                                                SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE);
  return secondsTillNext;
}

void sl_zigbee_af_price_server_refresh_billing_period(uint8_t endpoint, bool force)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  uint32_t secondsUntilSecondIndexActive;
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }

  secondsUntilSecondIndexActive
    = sl_zigbee_af_price_common_seconds_until_second_index_active(priceServerInfo.billingPeriodTable.commonInfos[ep],
                                                                  SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE);

  if (secondsUntilSecondIndexActive == 0u) {
    // Replace current with next.
    priceServerInfo.billingPeriodTable.commonInfos[ep][0].valid = false;
    sl_zigbee_af_price_common_sort(priceServerInfo.billingPeriodTable.commonInfos[ep],
                                   (uint8_t *)priceServerInfo.billingPeriodTable.billingPeriods[ep],
                                   (uint16_t)sizeof(sl_zigbee_af_price_billing_period_t),
                                   SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE);
    sl_zigbee_af_price_common_update_duration_for_overlapping_events(priceServerInfo.billingPeriodTable.commonInfos[ep],
                                                                     SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE);
  } else if (secondsUntilSecondIndexActive == 0xFFFFFFFFU
             && force
             && priceServerInfo.billingPeriodTable.commonInfos[ep][0].valid) {
    // The .startTime and .durationSec members are both UTC values measured
    // in seconds. The nextStartTime is therefore a UTC value. Set the start
    // time of the next period to the end time of the current period.
    uint32_t nextStartTime
      = (priceServerInfo.billingPeriodTable.commonInfos[ep][0].startTime
         + priceServerInfo.billingPeriodTable.commonInfos[ep][0].durationSec);
    // The event ID is supposed to be a unique value for each different piece
    // of priceServerInformation from the commodity provider. A higher event ID is supposed
    // to mean that the priceServerInformation is newer. Therefore, let's make sure this
    // next billing period has a higher event ID than the current period.
    uint32_t nextIssuerEventId
      = (priceServerInfo.billingPeriodTable.commonInfos[ep][0].issuerEventId + 1u);
    sl_zigbee_af_price_server_billing_period_add(endpoint,
                                                 nextStartTime,
                                                 nextIssuerEventId,
                                                 priceServerInfo.billingPeriodTable.billingPeriods[ep][0].providerId,
                                                 priceServerInfo.billingPeriodTable.billingPeriods[ep][0].billingPeriodDuration,
                                                 priceServerInfo.billingPeriodTable.billingPeriods[ep][0].billingPeriodDurationType,
                                                 priceServerInfo.billingPeriodTable.billingPeriods[ep][0].tariffType);
  } else {
    // MISRA requires ..else if.. to have terminating else.
  }
}

sl_status_t sl_zigbee_af_price_server_billing_period_add(uint8_t endpoint,
                                                         uint32_t startTime,
                                                         uint32_t issuerEventId,
                                                         uint32_t providerId,
                                                         uint32_t billingPeriodDuration,
                                                         uint8_t billingPeriodDurationType,
                                                         uint8_t tariffType)
{
  uint8_t index;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  index = sl_zigbee_af_price_common_get_common_matching_or_unused_index(priceServerInfo.billingPeriodTable.commonInfos[ep],
                                                                        SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE,
                                                                        issuerEventId,
                                                                        startTime,
                                                                        false);
  if (index >= SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE) {
    sl_zigbee_af_price_cluster_println("Error: Index out of bounds!");
    return SL_STATUS_INVALID_PARAMETER;
  }

  // checking for valid tariffType.
  if (tariffType > SL_ZIGBEE_ZCL_TARIFF_TYPE_DELIVERED_AND_RECEIVED_TARIFF) {
    sl_zigbee_af_price_cluster_println("Error: Invalid tariff type!");
    return SL_STATUS_INVALID_PARAMETER;
  }

  priceServerInfo.billingPeriodTable.commonInfos[ep][index].valid = true;
  priceServerInfo.billingPeriodTable.commonInfos[ep][index].startTime = sl_zigbee_af_price_common_cluster_get_adjusted_start_time(startTime, billingPeriodDurationType);
  priceServerInfo.billingPeriodTable.commonInfos[ep][index].issuerEventId = issuerEventId;
  priceServerInfo.billingPeriodTable.billingPeriods[ep][index].providerId = providerId;
  priceServerInfo.billingPeriodTable.billingPeriods[ep][index].rawBillingPeriodStartTime = startTime;
  priceServerInfo.billingPeriodTable.commonInfos[ep][index].durationSec =
    sl_zigbee_af_price_common_cluster_convert_duration_to_seconds(startTime, billingPeriodDuration, billingPeriodDurationType);

  priceServerInfo.billingPeriodTable.billingPeriods[ep][index].billingPeriodDuration = billingPeriodDuration;
  priceServerInfo.billingPeriodTable.billingPeriods[ep][index].billingPeriodDurationType = billingPeriodDurationType;
  priceServerInfo.billingPeriodTable.billingPeriods[ep][index].tariffType = tariffType;

  sl_zigbee_af_price_common_sort(priceServerInfo.billingPeriodTable.commonInfos[ep],
                                 (uint8_t *)priceServerInfo.billingPeriodTable.billingPeriods[ep],
                                 (uint16_t)sizeof(sl_zigbee_af_price_billing_period_t),
                                 SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE);
  sl_zigbee_af_price_common_update_duration_for_overlapping_events(priceServerInfo.billingPeriodTable.commonInfos[ep],
                                                                   SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE);

  sl_zigbee_af_price_cluster_schedule_tick_cb(endpoint, SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_BILLING_PERIOD_EVENT_MASK);
  return SL_STATUS_OK;
}

void sl_zigbee_af_price_server_billing_period_pub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint, uint8_t index)
{
  sl_status_t status;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(srcEndpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  if (index >= SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE) {
    sl_zigbee_af_price_cluster_println("Error: Index out of bounds!");
    return;
  }
  (void) sl_zigbee_af_fill_command_price_cluster_publish_billing_period(priceServerInfo.billingPeriodTable.billingPeriods[ep][index].providerId,
                                                                        priceServerInfo.billingPeriodTable.commonInfos[ep][index].issuerEventId,
                                                                        priceServerInfo.billingPeriodTable.billingPeriods[ep][index].rawBillingPeriodStartTime,
                                                                        priceServerInfo.billingPeriodTable.billingPeriods[ep][index].billingPeriodDuration,
                                                                        priceServerInfo.billingPeriodTable.billingPeriods[ep][index].billingPeriodDurationType,
                                                                        priceServerInfo.billingPeriodTable.billingPeriods[ep][index].tariffType);

  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_price_cluster_println("Error: PublishBillingPeriod failed: 0x%02X", status);
  }
}

static void sl_zigbee_af_price_clear_consolidated_bills_table(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONSOLIDATED_BILL_TABLE_SIZE; i++ ) {
    priceServerInfo.consolidatedBillsTable.commonInfos[ep][i].valid = false;
  }
}

void sl_zigbee_af_price_server_consolidated_bill_add(uint8_t endpoint,
                                                     uint32_t startTime,
                                                     uint32_t issuerEventId,
                                                     uint32_t providerId,
                                                     uint32_t billingPeriodDuration,
                                                     uint8_t billingPeriodDurationType,
                                                     uint8_t tariffType,
                                                     uint32_t consolidatedBill,
                                                     uint16_t currency,
                                                     uint8_t billTrailingDigit)
{
  uint32_t adjStartTime;
  uint8_t index;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }

  adjStartTime = sl_zigbee_af_price_common_cluster_get_adjusted_start_time(startTime, billingPeriodDurationType);
  index = sl_zigbee_af_price_common_get_common_matching_or_unused_index(priceServerInfo.consolidatedBillsTable.commonInfos[ep],
                                                                        SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONSOLIDATED_BILL_TABLE_SIZE,
                                                                        issuerEventId, adjStartTime, false);
  if (index >= SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONSOLIDATED_BILL_TABLE_SIZE) {
    sl_zigbee_af_price_cluster_println("Error: Index out of bound!");
    return;
  }

  priceServerInfo.consolidatedBillsTable.commonInfos[ep][index].valid = true;
  priceServerInfo.consolidatedBillsTable.commonInfos[ep][index].startTime = adjStartTime;
  priceServerInfo.consolidatedBillsTable.commonInfos[ep][index].durationSec =
    sl_zigbee_af_price_common_cluster_convert_duration_to_seconds(startTime, billingPeriodDuration, billingPeriodDurationType);

  priceServerInfo.consolidatedBillsTable.commonInfos[ep][index].issuerEventId = issuerEventId;

  priceServerInfo.consolidatedBillsTable.consolidatedBills[ep][index].providerId = providerId;
  priceServerInfo.consolidatedBillsTable.consolidatedBills[ep][index].rawStartTimeUtc = startTime;
  priceServerInfo.consolidatedBillsTable.consolidatedBills[ep][index].billingPeriodDuration = billingPeriodDuration;
  priceServerInfo.consolidatedBillsTable.consolidatedBills[ep][index].billingPeriodDurationType = billingPeriodDurationType;
  priceServerInfo.consolidatedBillsTable.consolidatedBills[ep][index].tariffType = tariffType;
  priceServerInfo.consolidatedBillsTable.consolidatedBills[ep][index].consolidatedBill = consolidatedBill;
  priceServerInfo.consolidatedBillsTable.consolidatedBills[ep][index].currency = currency;
  priceServerInfo.consolidatedBillsTable.consolidatedBills[ep][index].billTrailingDigit = billTrailingDigit;

  sl_zigbee_af_price_common_sort(priceServerInfo.consolidatedBillsTable.commonInfos[ep],
                                 (uint8_t *)priceServerInfo.consolidatedBillsTable.consolidatedBills[ep],
                                 (uint16_t)sizeof(sl_zigbee_af_price_consolidated_bills_t),
                                 SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONSOLIDATED_BILL_TABLE_SIZE);
  sl_zigbee_af_price_common_update_duration_for_overlapping_events(priceServerInfo.consolidatedBillsTable.commonInfos[ep],
                                                                   SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONSOLIDATED_BILL_TABLE_SIZE);
}

void sl_zigbee_af_price_server_consolidated_bill_pub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint, uint8_t index)
{
  sl_status_t status;
  sl_zigbee_af_price_consolidated_bills_t *pbill;

  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(srcEndpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  if (index >= SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONSOLIDATED_BILL_TABLE_SIZE) {
    sl_zigbee_af_price_cluster_println("Error: Index out of bounds!");
    return;
  }
  pbill = &priceServerInfo.consolidatedBillsTable.consolidatedBills[ep][index];

  (void) sl_zigbee_af_fill_command_price_cluster_publish_consolidated_bill(pbill->providerId,
                                                                           priceServerInfo.consolidatedBillsTable.commonInfos[ep][index].issuerEventId,
                                                                           pbill->rawStartTimeUtc,
                                                                           pbill->billingPeriodDuration,
                                                                           pbill->billingPeriodDurationType,
                                                                           pbill->tariffType,
                                                                           pbill->consolidatedBill,
                                                                           pbill->currency,
                                                                           pbill->billTrailingDigit);

  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_price_cluster_println("Error: PublishConsolidatedBill failed: %02X", status);
  }
}

// CPP EVENT

void sl_zigbee_af_price_server_cpp_event_set(uint8_t endpoint, uint8_t valid, uint32_t providerId, uint32_t issuerEventId, uint32_t startTime,
                                             uint16_t durationInMinutes, uint8_t tariffType, uint8_t cppPriceTier, uint8_t cppAuth)
{
// For now, just a single CPP event.  Support multiple??
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  priceServerInfo.cppTable.commonInfos[ep].valid = (valid != 0u);
  priceServerInfo.cppTable.commonInfos[ep].issuerEventId = issuerEventId;
  priceServerInfo.cppTable.commonInfos[ep].startTime = startTime;
  priceServerInfo.cppTable.commonInfos[ep].durationSec = (durationInMinutes * 60u);
  priceServerInfo.cppTable.cppEvent[ep].durationInMinutes = durationInMinutes;
  priceServerInfo.cppTable.cppEvent[ep].providerId = providerId;
  priceServerInfo.cppTable.cppEvent[ep].tariffType = tariffType;
  priceServerInfo.cppTable.cppEvent[ep].cppPriceTier = cppPriceTier;
  priceServerInfo.cppTable.cppEvent[ep].cppAuth = cppAuth;
}

void sl_zigbee_af_price_server_cpp_event_print(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  sl_zigbee_af_price_cluster_println("Cpp Event:");
  sl_zigbee_af_price_cluster_println("  providerId=%d", priceServerInfo.cppTable.cppEvent[ep].providerId);
  sl_zigbee_af_price_cluster_println("  issuerEventId=%d", priceServerInfo.cppTable.commonInfos[ep].issuerEventId);
  sl_zigbee_af_price_cluster_println("  startTime=0x%08X", priceServerInfo.cppTable.commonInfos[ep].startTime);
  sl_zigbee_af_price_cluster_println("  durationInMins=%d", priceServerInfo.cppTable.cppEvent[ep].durationInMinutes);
  sl_zigbee_af_price_cluster_println("  tariffType=%d", priceServerInfo.cppTable.cppEvent[ep].tariffType);
  sl_zigbee_af_price_cluster_println("  cppPriceTier=%d", priceServerInfo.cppTable.cppEvent[ep].cppPriceTier);
  sl_zigbee_af_price_cluster_println("  cppAuth=%d", priceServerInfo.cppTable.cppEvent[ep].cppAuth);
}

void sl_zigbee_af_price_server_cpp_event_pub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint)
{
  sl_status_t status;
  sl_zigbee_af_price_cpp_event_t *cppEvent;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(srcEndpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }

  cppEvent = &priceServerInfo.cppTable.cppEvent[ep];
  (void) sl_zigbee_af_fill_command_price_cluster_publish_cpp_event(cppEvent->providerId, priceServerInfo.cppTable.commonInfos[ep].issuerEventId,
                                                                   priceServerInfo.cppTable.commonInfos[ep].startTime,
                                                                   cppEvent->durationInMinutes, cppEvent->tariffType,
                                                                   cppEvent->cppPriceTier, cppEvent->cppAuth);
  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_price_cluster_println("Error: PublishCppEvent failed: %02X", status);
  }
}

static void sortCreditPaymentEntries(uint8_t *entries, uint8_t numValidEntries, sl_zigbee_af_price_credit_payment_t *table)
{
  // The valid entries should be sorted from latest "credit payment date" to earliest "credit payment date".
  uint32_t latestPaymentDate = 0xFFFFFFFFU;
  uint32_t latestRemainingDate;
  uint32_t latestRemainingIndex;
  uint32_t paymentDate;
  uint8_t i, j;
  uint8_t sortedEntries[255];

  for ( i = 0; i < numValidEntries; i++ ) {
    latestRemainingDate = 0;  // Reset this, then go through all entries to find the latest.
    latestRemainingIndex = 0;
    for ( j = 0; j < numValidEntries; j++ ) {
      // Find OLDEST from remaining entries.
      paymentDate = table[entries[j]].creditPaymentDate;
      if ( (paymentDate < latestPaymentDate)  && (paymentDate > latestRemainingDate) ) {
        latestRemainingDate = paymentDate;
        latestRemainingIndex = j;
      }
    }
    // Store the oldest found
    latestPaymentDate = table[entries[latestRemainingIndex]].creditPaymentDate;
    sortedEntries[i] = entries[latestRemainingIndex];
  }
  // Now copy the sorted entries back into the "entries" table.
  (void) memcpy(entries, sortedEntries, numValidEntries);
}

void sl_zigbee_af_price_server_credit_payment_pub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint, uint8_t index)
{
  sl_status_t status;
  sl_zigbee_af_price_credit_payment_t *pcredit;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(srcEndpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }

  if (index >= SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CREDIT_PAYMENT_TABLE_SIZE) {
    sl_zigbee_af_price_cluster_println("Error: Index out of bound!");
    return;
  } else if ( !priceServerInfo.creditPaymentTable.commonInfos[ep][index].valid ) {
    sl_zigbee_af_price_cluster_println("Error: Invalid index!");
    return;
  } else {
    // MISRA requires ..else if.. to have terminating else.
  }
  pcredit = &priceServerInfo.creditPaymentTable.creditPayment[ep][index];

  (void) sl_zigbee_af_fill_command_price_cluster_publish_credit_payment(pcredit->providerId,
                                                                        priceServerInfo.creditPaymentTable.commonInfos[ep][index].issuerEventId,
                                                                        pcredit->creditPaymentDueDate,
                                                                        pcredit->creditPaymentAmountOverdue,
                                                                        pcredit->creditPaymentStatus,
                                                                        pcredit->creditPayment,
                                                                        pcredit->creditPaymentDate,
                                                                        pcredit->creditPaymentRef);

  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_price_cluster_println("Error: PublishCreditPayment failed: %02X", status);
  }
}

void sl_zigbee_af_price_server_credit_payment_set(uint8_t endpoint, uint8_t index, uint8_t valid,
                                                  uint32_t providerId, uint32_t issuerEventId,
                                                  uint32_t creditPaymentDueDate, uint32_t creditPaymentOverdueAmount,
                                                  uint8_t creditPaymentStatus, uint32_t creditPayment,
                                                  uint32_t creditPaymentDate, uint8_t *creditPaymentRef)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  if ( index >= SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CREDIT_PAYMENT_TABLE_SIZE) {
    sl_zigbee_af_price_cluster_println("Error: Index out of bound!");
    return;
  }

  priceServerInfo.creditPaymentTable.commonInfos[ep][index].valid = (valid != 0u);
  priceServerInfo.creditPaymentTable.commonInfos[ep][index].issuerEventId = issuerEventId;
  priceServerInfo.creditPaymentTable.commonInfos[ep][index].durationSec = ZCL_PRICE_CLUSTER_DURATION_SEC_UNTIL_CHANGED;
  priceServerInfo.creditPaymentTable.creditPayment[ep][index].providerId = providerId;
  priceServerInfo.creditPaymentTable.creditPayment[ep][index].creditPaymentDueDate = creditPaymentDueDate;
  priceServerInfo.creditPaymentTable.creditPayment[ep][index].creditPaymentAmountOverdue = creditPaymentOverdueAmount;
  priceServerInfo.creditPaymentTable.creditPayment[ep][index].creditPaymentStatus = creditPaymentStatus;
  priceServerInfo.creditPaymentTable.creditPayment[ep][index].creditPayment = creditPayment;
  priceServerInfo.creditPaymentTable.creditPayment[ep][index].creditPaymentDate = creditPaymentDate;
  // Truncates reference string if necessary.
  sl_zigbee_af_copy_string(priceServerInfo.creditPaymentTable.creditPayment[ep][index].creditPaymentRef, creditPaymentRef, CREDIT_PAYMENT_REF_STRING_LEN);
}

// CURRENCY CONVERSION
static void sl_zigbee_af_price_clear_currency_conversion_table(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep != ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    priceServerInfo.currencyConversionTable.commonInfos[ep].valid = false;
  }
}

void sl_zigbee_af_price_server_currency_conversion_pub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint)
{
  sl_status_t status;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(srcEndpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  if ( priceServerInfo.currencyConversionTable.commonInfos[ep].valid ) {
    (void) sl_zigbee_af_fill_command_price_cluster_publish_currency_conversion(priceServerInfo.currencyConversionTable.currencyConversion[ep].providerId,
                                                                               priceServerInfo.currencyConversionTable.commonInfos[ep].issuerEventId,
                                                                               priceServerInfo.currencyConversionTable.commonInfos[ep].startTime,
                                                                               priceServerInfo.currencyConversionTable.currencyConversion[ep].oldCurrency,
                                                                               priceServerInfo.currencyConversionTable.currencyConversion[ep].newCurrency,
                                                                               priceServerInfo.currencyConversionTable.currencyConversion[ep].conversionFactor,
                                                                               priceServerInfo.currencyConversionTable.currencyConversion[ep].conversionFactorTrailingDigit,
                                                                               priceServerInfo.currencyConversionTable.currencyConversion[ep].currencyChangeControlFlags);
    sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
    sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
    status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
    if (status != SL_STATUS_OK) {
      sl_zigbee_af_price_cluster_println("Error: PublishCreditPayment failed: %02X", status);
    }
  } else {
    sl_zigbee_af_price_cluster_println("Error: Invalid Currency Conversion");
  }
}

void sl_zigbee_af_price_server_currency_conversion_set(uint8_t endpoint, uint8_t valid, uint32_t providerId, uint32_t issuerEventId,
                                                       uint32_t startTime, uint16_t oldCurrency, uint16_t newCurrency,
                                                       uint32_t conversionFactor, uint8_t conversionFactorTrailingDigit,
                                                       uint32_t currencyChangeControlFlags)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  sl_zigbee_af_price_cluster_println("Set Currency Conversion: val=%d, pid=%d, eid=%d, start=%d, flag=%d",
                                     valid, providerId, issuerEventId, startTime, currencyChangeControlFlags);
  priceServerInfo.currencyConversionTable.commonInfos[ep].valid = (valid != 0u);
  priceServerInfo.currencyConversionTable.currencyConversion[ep].providerId = providerId;
  priceServerInfo.currencyConversionTable.commonInfos[ep].issuerEventId = issuerEventId;
  priceServerInfo.currencyConversionTable.commonInfos[ep].startTime = startTime;
  priceServerInfo.currencyConversionTable.currencyConversion[ep].oldCurrency = oldCurrency;
  priceServerInfo.currencyConversionTable.currencyConversion[ep].newCurrency = newCurrency;
  priceServerInfo.currencyConversionTable.currencyConversion[ep].conversionFactor = conversionFactor;
  priceServerInfo.currencyConversionTable.currencyConversion[ep].conversionFactorTrailingDigit = conversionFactorTrailingDigit;
  priceServerInfo.currencyConversionTable.currencyConversion[ep].currencyChangeControlFlags = currencyChangeControlFlags;
}

// TARIFF CANCELLATION
static void sl_zigbee_af_price_clear_cancel_tariff_table(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep != ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    priceServerInfo.cancelTariffTable.cancelTariff[ep].valid = false;
  }
}

void sl_zigbee_af_price_server_tariff_cancellation_set(uint8_t endpoint, uint8_t valid, uint32_t providerId, uint32_t issuerTariffId, uint8_t tariffType)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  priceServerInfo.cancelTariffTable.cancelTariff[ep].valid = (valid != 0u);
  priceServerInfo.cancelTariffTable.cancelTariff[ep].providerId = providerId;
  priceServerInfo.cancelTariffTable.cancelTariff[ep].issuerTariffId = issuerTariffId;
  priceServerInfo.cancelTariffTable.cancelTariff[ep].tariffType = tariffType;
}

void sl_zigbee_af_price_server_tariff_cancellation_pub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint)
{
  sl_status_t status;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(srcEndpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  if ( priceServerInfo.cancelTariffTable.cancelTariff[ep].valid ) {
    (void) sl_zigbee_af_fill_command_price_cluster_cancel_tariff(priceServerInfo.cancelTariffTable.cancelTariff[ep].providerId,
                                                                 priceServerInfo.cancelTariffTable.cancelTariff[ep].issuerTariffId,
                                                                 priceServerInfo.cancelTariffTable.cancelTariff[ep].tariffType);
    sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
    sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
    status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
    if ( status != SL_STATUS_OK ) {
      sl_zigbee_af_price_cluster_println("Error: Tariff Cancellation failed: 0x%02X", status);
    }
  } else {
    sl_zigbee_af_price_cluster_println("Error: Invalid Cancel Tariff");
  }
}

/// BLOCK PERIOD
void sl_zigbee_af_price_clear_block_period_table(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE; i++ ) {
    priceServerInfo.blockPeriodTable.commonInfos[ep][i].valid = false;
  }
}

void sl_zigbee_af_price_server_block_period_add(uint8_t endpoint,
                                                uint32_t providerId,
                                                uint32_t issuerEventId,
                                                uint32_t blockPeriodStartTime,
                                                uint32_t blockPeriodDuration,
                                                uint8_t  blockPeriodControl,
                                                uint8_t blockPeriodDurationType,
                                                uint32_t thresholdMultiplier,
                                                uint32_t thresholdDivisor,
                                                uint8_t  tariffType,
                                                uint8_t tariffResolutionPeriod)
{
  uint8_t index;
  uint32_t adjStartTime;

  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }

  adjStartTime = sl_zigbee_af_price_common_cluster_get_adjusted_start_time(blockPeriodStartTime,
                                                                           blockPeriodDurationType);

  index = sl_zigbee_af_price_common_get_common_matching_or_unused_index(priceServerInfo.blockPeriodTable.commonInfos[ep],
                                                                        SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE,
                                                                        issuerEventId,
                                                                        adjStartTime,
                                                                        true);

  if ( index < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE ) {
    sl_zigbee_af_price_common_info_t *curInfo = &priceServerInfo.blockPeriodTable.commonInfos[ep][index];
    sl_zigbee_af_price_block_period_t *curBlockPeriod = &priceServerInfo.blockPeriodTable.blockPeriods[ep][index];
    curInfo->valid = true;
    curInfo->actionsPending = true;
    curInfo->startTime = adjStartTime;

    curInfo->durationSec = sl_zigbee_af_price_common_cluster_convert_duration_to_seconds(blockPeriodStartTime,
                                                                                         blockPeriodDuration,
                                                                                         blockPeriodDurationType);
    curInfo->issuerEventId = issuerEventId;

    curBlockPeriod->providerId = providerId;
    if (blockPeriodStartTime == 0x00u) {
      curBlockPeriod->rawBlockPeriodStartTime = adjStartTime;
    } else {
      curBlockPeriod->rawBlockPeriodStartTime = blockPeriodStartTime;
    }

    curBlockPeriod->blockPeriodDuration     = blockPeriodDuration;
    curBlockPeriod->blockPeriodControl      = blockPeriodControl;
    curBlockPeriod->blockPeriodDurationType = blockPeriodDurationType;
    curBlockPeriod->thresholdMultiplier     = thresholdMultiplier;
    curBlockPeriod->thresholdDivisor        = thresholdDivisor;
    curBlockPeriod->tariffType              = tariffType;
    curBlockPeriod->tariffResolutionPeriod  = tariffResolutionPeriod;

    sl_zigbee_af_price_server_block_period_update_bindings(index);

    sl_zigbee_af_price_common_sort(priceServerInfo.blockPeriodTable.commonInfos[ep],
                                   (uint8_t *)priceServerInfo.blockPeriodTable.blockPeriods[ep],
                                   (uint16_t)sizeof(sl_zigbee_af_price_block_period_t),
                                   SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE);
    sl_zigbee_af_price_common_update_duration_for_overlapping_events(priceServerInfo.blockPeriodTable.commonInfos[ep],
                                                                     SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE);
    sl_zigbee_af_price_cluster_schedule_tick_cb(endpoint, SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_BLOCK_PERIOD_EVENT_MASK);
  } else {
    sl_zigbee_af_price_cluster_println("ERR: Unable to add new BlockPeriod entry!");
  }
}

void sl_zigbee_af_price_server_block_period_print(uint8_t endpoint, uint8_t index)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  if ( index < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE ) {
    sl_zigbee_af_price_cluster_println("= Block Period [%d]:", index);
    sl_zigbee_af_price_cluster_println("  valid=%d", priceServerInfo.blockPeriodTable.commonInfos[ep][index].valid);
    sl_zigbee_af_price_cluster_println("  providerId=0x%08X", priceServerInfo.blockPeriodTable.blockPeriods[ep][index].providerId);
    sl_zigbee_af_price_cluster_println("  issuerEventId=0x%08X", priceServerInfo.blockPeriodTable.commonInfos[ep][index].issuerEventId);
    sl_zigbee_af_price_cluster_println("  startTime=0x%08X", priceServerInfo.blockPeriodTable.commonInfos[ep][index].startTime);
    sl_zigbee_af_price_cluster_println("  duration=0x%08X", priceServerInfo.blockPeriodTable.commonInfos[ep][index].durationSec);
    sl_zigbee_af_price_cluster_println("  rawBlkPeriodStartTime=0x%08X", priceServerInfo.blockPeriodTable.blockPeriods[ep][index].rawBlockPeriodStartTime);
    sl_zigbee_af_price_cluster_println("  rawBlkPeriodDuration=0x%08X", priceServerInfo.blockPeriodTable.blockPeriods[ep][index].blockPeriodDuration);
    sl_zigbee_af_price_cluster_println("  durationType=0x%02X", priceServerInfo.blockPeriodTable.blockPeriods[ep][index].blockPeriodDurationType);
    sl_zigbee_af_price_cluster_println("  blockPeriodControl=0x%02X", priceServerInfo.blockPeriodTable.blockPeriods[ep][index].blockPeriodControl);
    sl_zigbee_af_price_cluster_println("  tariffType=0x%02X", priceServerInfo.blockPeriodTable.blockPeriods[ep][index].tariffType);
    sl_zigbee_af_price_cluster_println("  tariffResolutionPeriod=0x%02X", priceServerInfo.blockPeriodTable.blockPeriods[ep][index].tariffResolutionPeriod);
  }
}

static void sl_zigbee_af_price_server_block_period_update_bindings(uint8_t blockPeriodEntryIndex)
{
  // Only process BlockPeriod commands for now.
  sl_zigbee_binding_table_entry_t candidate;
  uint8_t i;

  // search for binding entry with Simple Metering cluster.
  for (i = 0; i < SL_ZIGBEE_BINDING_TABLE_SIZE; i++) {
    if (sl_zigbee_get_binding(i, &candidate) == SL_STATUS_OK
        && candidate.type == SL_ZIGBEE_UNICAST_BINDING
        && candidate.clusterId == ZCL_PRICE_CLUSTER_ID) {
      sl_802154_short_addr_t nodeId;
      sl_status_t status = sl_zigbee_lookup_node_id_by_eui64(candidate.identifier, &nodeId);
      if (status == SL_STATUS_OK) {
        sl_zigbee_af_price_server_block_period_pub(nodeId,
                                                   candidate.local,
                                                   candidate.remote,
                                                   blockPeriodEntryIndex);
      } else {
        sl_zigbee_af_price_cluster_println("ERR: Unable to find nodeId for binding entry(%d).", i);
      }
    }
  }
}

void sl_zigbee_af_price_server_price_update_bindings()
{
  // Only process BlockPeriod commands for now.
  sl_zigbee_binding_table_entry_t candidate;
  uint8_t i;

  // search for binding entry with Simple Metering cluster.
  for (i = 0; i < SL_ZIGBEE_BINDING_TABLE_SIZE; i++) {
    if (sl_zigbee_get_binding(i, &candidate) == SL_STATUS_OK
        && candidate.type == SL_ZIGBEE_UNICAST_BINDING
        && candidate.clusterId == ZCL_PRICE_CLUSTER_ID) {
      bool sentMsg = false;
      sl_802154_short_addr_t nodeId;
      sl_status_t status = sl_zigbee_lookup_node_id_by_eui64(candidate.identifier, &nodeId);
      if (status == SL_STATUS_OK) {
        uint8_t priceEntryIndex = getCurrentPriceIndex(candidate.local);
        if (priceEntryIndex != 0xFFu) {
          sl_zigbee_af_price_server_publish_price_message(nodeId,
                                                          candidate.local,
                                                          candidate.remote,
                                                          priceEntryIndex);
          sentMsg = true;
        }
      }

      if (!sentMsg) {
        sl_zigbee_af_price_cluster_println("ERR: Unable to sent PublishPrice command to binding entry(%d).", i);
      }
    }
  }
}

void sl_zigbee_af_price_server_block_period_pub(uint16_t nodeId,
                                                uint8_t srcEndpoint,
                                                uint8_t dstEndpoint,
                                                uint8_t index)
{
  sl_status_t status;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(srcEndpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }
  if ( (index < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE)
       && (priceServerInfo.blockPeriodTable.commonInfos[ep][index].valid) ) {
    (void) sl_zigbee_af_fill_command_price_cluster_publish_block_period(priceServerInfo.blockPeriodTable.blockPeriods[ep][index].providerId,
                                                                        priceServerInfo.blockPeriodTable.commonInfos[ep][index].issuerEventId,
                                                                        priceServerInfo.blockPeriodTable.blockPeriods[ep][index].rawBlockPeriodStartTime,
                                                                        priceServerInfo.blockPeriodTable.blockPeriods[ep][index].blockPeriodDuration,
                                                                        priceServerInfo.blockPeriodTable.blockPeriods[ep][index].blockPeriodControl,
                                                                        priceServerInfo.blockPeriodTable.blockPeriods[ep][index].blockPeriodDurationType,
                                                                        priceServerInfo.blockPeriodTable.blockPeriods[ep][index].tariffType,
                                                                        priceServerInfo.blockPeriodTable.blockPeriods[ep][index].tariffResolutionPeriod);
    sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
    sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
    status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
    if ( status != SL_STATUS_OK ) {
      sl_zigbee_af_price_cluster_println("Error: Block Period failed: 0x%02X", status);
    }
  } else {
    sl_zigbee_af_price_cluster_println("Error: Invalid Block Period");
  }
}

uint32_t sl_zigbee_af_price_server_seconds_until_tariff_info_event(uint8_t endpoint)
{
  uint32_t secTillNext;
  uint8_t futureEntryIndex;
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return 0;
  }

  // First, check if the Active index is currently applied.
  i = sl_zigbee_af_price_common_server_get_active_index((sl_zigbee_af_price_common_info_t *)priceServerInfo.scheduledTariffTable.commonInfos[ep],
                                                        SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE);
  if ((i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE)
      && (priceServerInfo.scheduledTariffTable.commonInfos[ep][i].actionsPending)) {
    // action req'ed.
    return 0;
  }

  futureEntryIndex = sl_zigbee_af_price_common_server_get_future_index((sl_zigbee_af_price_common_info_t *)priceServerInfo.scheduledTariffTable.commonInfos[ep],
                                                                       SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE,
                                                                       &secTillNext);
  if ((futureEntryIndex == ZCL_PRICE_INVALID_INDEX)
      || !priceServerInfo.scheduledTariffTable.commonInfos[ep][futureEntryIndex].actionsPending) {
    secTillNext = ZCL_PRICE_CLUSTER_END_TIME_NEVER;
  }

  return secTillNext;
}

uint32_t sl_zigbee_af_price_server_seconds_until_active_price_matrix_event(uint8_t endpoint)
{
  uint32_t secTillNext;
  uint8_t futureEntryIndex;
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return 0;
  }

  // First, check if the Active index is currently applied.
  i = sl_zigbee_af_price_common_server_get_active_index((sl_zigbee_af_price_common_info_t *)priceServerInfo.scheduledPriceMatrixTable.commonInfos[ep],
                                                        SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE);
  if ((i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE)
      && (priceServerInfo.scheduledPriceMatrixTable.commonInfos[ep][i].actionsPending)) {
    // action req'ed.
    return 0;
  }

  futureEntryIndex = sl_zigbee_af_price_common_server_get_future_index((sl_zigbee_af_price_common_info_t *)priceServerInfo.scheduledPriceMatrixTable.commonInfos[ep],
                                                                       SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE,
                                                                       &secTillNext);
  if ((futureEntryIndex == ZCL_PRICE_INVALID_INDEX)
      || !priceServerInfo.scheduledPriceMatrixTable.commonInfos[ep][futureEntryIndex].actionsPending) {
    secTillNext = ZCL_PRICE_CLUSTER_END_TIME_NEVER;
  }

  return secTillNext;
}

uint32_t sl_zigbee_af_price_server_seconds_until_active_block_thresholds_event(uint8_t endpoint)
{
  uint32_t secTillNext;
  uint8_t futureEntryIndex;
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  sl_zigbee_af_price_common_info_t *inf = priceServerInfo.scheduledBlockThresholdsTable.commonInfos[ep];

  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return 0;
  }

  // First, check if the Active index is currently applied.
  i = sl_zigbee_af_price_common_server_get_active_index(inf,
                                                        SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE);

  if ((i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE) && (inf[i].actionsPending)) {
    // action req'ed.
    return 0;
  }

  futureEntryIndex = sl_zigbee_af_price_common_server_get_future_index(inf,
                                                                       SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE,
                                                                       &secTillNext);
  if ((futureEntryIndex == ZCL_PRICE_INVALID_INDEX)
      || !inf[futureEntryIndex].actionsPending) {
    secTillNext = ZCL_PRICE_CLUSTER_END_TIME_NEVER;
  }

  return secTillNext;
}

uint32_t sl_zigbee_af_price_server_seconds_until_block_period_event(uint8_t endpoint)
{
  uint32_t secTillNext;
  uint32_t startOfBlockPeriod;
  sl_zigbee_af_status_t status;
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return 0xFFFFFFFFU;
  }

  // First, check if the Active index is currently applied.
  i = sl_zigbee_af_price_common_server_get_active_index( (sl_zigbee_af_price_common_info_t *)&priceServerInfo.blockPeriodTable.commonInfos[ep], SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE);
  if ( i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE ) {
    // Check if the block period attributes match this block period.
    status = sl_zigbee_af_read_attribute(endpoint,
                                         ZCL_PRICE_CLUSTER_ID,
                                         ZCL_START_OF_BLOCK_PERIOD_ATTRIBUTE_ID,
                                         CLUSTER_MASK_SERVER,
                                         (uint8_t *)&startOfBlockPeriod,
                                         4,
                                         NULL);
    if ( (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS)
         && (startOfBlockPeriod != priceServerInfo.blockPeriodTable.commonInfos[ep][i].startTime)
         && (priceServerInfo.blockPeriodTable.commonInfos[ep][i].actionsPending)) {
      // Attributes are not current.
      return 0;
    }
  }

  sl_zigbee_af_price_common_server_get_future_index(priceServerInfo.blockPeriodTable.commonInfos[ep],
                                                    SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE,
                                                    &secTillNext);
  return secTillNext;
}

void sl_zigbee_af_price_server_refresh_block_period(uint8_t endpoint, bool repeat)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }

  i = sl_zigbee_af_price_common_server_get_active_index((sl_zigbee_af_price_common_info_t *)&priceServerInfo.blockPeriodTable.commonInfos[ep],
                                                        SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE);

  if ( i >=  SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE ) {
    return;
  }

  sl_zigbee_af_price_common_info_t *curInfo = &priceServerInfo.blockPeriodTable.commonInfos[ep][i];
  sl_zigbee_af_price_block_period_t *curBlockPeriod = &priceServerInfo.blockPeriodTable.blockPeriods[ep][i];

  if (repeat) {
    // Make sure to check whether the current block period calls for it to
    // repeat, before we add a new entry.
    if (curInfo->valid
        && ((curBlockPeriod->blockPeriodControl
             & SL_ZIGBEE_AF_BLOCK_PERIOD_CONTROL_REPEATING_BLOCK) != 0u)) {
      // The .startTime and .durationSec members are both UTC values measured
      // in seconds. The nextStartTime is therefore a UTC value. Set the start
      // time of the next period to the end time of the current period.
      uint32_t nextStartTime = (curInfo->startTime + curInfo->durationSec);
      // The event ID is supposed to be a unique value for each different piece
      // of priceServerInformation from the commodity provider. A higher event ID is supposed
      // to mean that the priceServerInformation is newer. Therefore, let's make sure this
      // next billing period has a higher event ID than the current period.
      uint32_t nextIssuerEventId = (curInfo->issuerEventId + 1u);
      sl_zigbee_af_price_server_block_period_add(endpoint,
                                                 curBlockPeriod->providerId,
                                                 nextIssuerEventId,
                                                 nextStartTime,
                                                 curBlockPeriod->blockPeriodDuration,
                                                 curBlockPeriod->blockPeriodControl,
                                                 curBlockPeriod->blockPeriodDurationType,
                                                 curBlockPeriod->thresholdMultiplier,
                                                 curBlockPeriod->thresholdDivisor,
                                                 curBlockPeriod->tariffType,
                                                 curBlockPeriod->tariffResolutionPeriod);
    }
    return;
  }

  (void) sl_zigbee_af_write_server_attribute(endpoint,
                                             ZCL_PRICE_CLUSTER_ID,
                                             ZCL_START_OF_BLOCK_PERIOD_ATTRIBUTE_ID,
                                             (uint8_t *)&curBlockPeriod->rawBlockPeriodStartTime,
                                             ZCL_UTC_TIME_ATTRIBUTE_TYPE);
  (void) sl_zigbee_af_write_server_attribute(endpoint,
                                             ZCL_PRICE_CLUSTER_ID,
                                             ZCL_BLOCK_PERIOD_DURATION_MINUTES_ATTRIBUTE_ID,
                                             (uint8_t *)&curBlockPeriod->blockPeriodDuration,
                                             ZCL_INT24U_ATTRIBUTE_TYPE);
  (void) sl_zigbee_af_write_server_attribute(endpoint,
                                             ZCL_PRICE_CLUSTER_ID,
                                             ZCL_THRESHOLD_MULTIPLIER_ATTRIBUTE_ID,
                                             (uint8_t *)&curBlockPeriod->thresholdMultiplier,
                                             ZCL_INT24U_ATTRIBUTE_TYPE);
  (void) sl_zigbee_af_write_server_attribute(endpoint,
                                             ZCL_PRICE_CLUSTER_ID,
                                             ZCL_THRESHOLD_DIVISOR_ATTRIBUTE_ID,
                                             (uint8_t *)&curBlockPeriod->thresholdDivisor,
                                             ZCL_INT24U_ATTRIBUTE_TYPE);
  (void) sl_zigbee_af_write_server_attribute(endpoint,
                                             ZCL_PRICE_CLUSTER_ID,
                                             ZCL_BLOCK_PERIOD_DURATION_TYPE_ATTRIBUTE_ID,
                                             (uint8_t *)&curBlockPeriod->blockPeriodDurationType,
                                             ZCL_BITMAP8_ATTRIBUTE_TYPE);

  sl_zigbee_af_price_server_new_active_block_period_information_cb(curInfo,
                                                                   curBlockPeriod);
  curInfo->actionsPending = false;
}

void sl_zigbee_af_price_server_refresh_price_matrix_information(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return;
  }

  i = sl_zigbee_af_price_common_server_get_active_index((sl_zigbee_af_price_common_info_t *)priceServerInfo.scheduledPriceMatrixTable.commonInfos[ep],
                                                        SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE);

  if ((i >= SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE)
      || !priceServerInfo.scheduledPriceMatrixTable.commonInfos[ep][i].actionsPending) {
    return;
  }

  // Notify application
  sl_zigbee_af_price_common_info_t *curInfo = &priceServerInfo.scheduledPriceMatrixTable.commonInfos[ep][i];
  sl_zigbee_af_scheduled_price_matrix_t *curPm = &priceServerInfo.scheduledPriceMatrixTable.scheduledPriceMatrix[ep][i];
  sl_zigbee_af_price_server_new_active_price_matrix_cb(curInfo, curPm);
  curInfo->actionsPending = false;
}

void sl_zigbee_af_price_server_refresh_block_thresholds_information(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return;
  }

  i = sl_zigbee_af_price_common_server_get_active_index((sl_zigbee_af_price_common_info_t *)priceServerInfo.scheduledBlockThresholdsTable.commonInfos[ep],
                                                        SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE);

  if ((i >= SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE)
      || !priceServerInfo.scheduledBlockThresholdsTable.commonInfos[ep][i].actionsPending) {
    return;
  }

  // Notify application
  sl_zigbee_af_price_common_info_t *curInfo = &priceServerInfo.scheduledBlockThresholdsTable.commonInfos[ep][i];
  sl_zigbee_af_scheduled_block_thresholds_t *curBt = &priceServerInfo.scheduledBlockThresholdsTable.scheduledBlockThresholds[ep][i];
  sl_zigbee_af_price_server_new_active_block_thresholds_information_cb(curInfo, curBt);
  curInfo->actionsPending = false;
}

void sl_zigbee_af_price_server_refresh_tariff_information(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return;
  }

  i = sl_zigbee_af_price_common_server_get_active_index((sl_zigbee_af_price_common_info_t *)priceServerInfo.scheduledTariffTable.commonInfos[ep],
                                                        SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE);

  if ((i >= SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE)
      || !priceServerInfo.scheduledTariffTable.commonInfos[ep][i].actionsPending) {
    return;
  }

  // start updating attributes
  sl_zigbee_af_price_common_info_t *curInfo = &priceServerInfo.scheduledTariffTable.commonInfos[ep][i];
  sl_zigbee_af_scheduled_tariff_t *tariff = &priceServerInfo.scheduledTariffTable.scheduledTariffs[ep][i];
  sl_zigbee_af_tariff_type_t tariffType = (sl_zigbee_af_tariff_type_t)(tariff->tariffTypeChargingScheme & TARIFF_TYPE_MASK);

  // notify application for newly activated Tariff Information
  sl_zigbee_af_price_server_new_active_tariff_information_cb(curInfo, tariff);

  if ((tariffType == SL_ZIGBEE_ZCL_TARIFF_TYPE_DELIVERED_TARIFF)
      || (tariffType == SL_ZIGBEE_ZCL_TARIFF_TYPE_DELIVERED_AND_RECEIVED_TARIFF)) {
    sl_zigbee_af_price_cluster_println("Info: Updating ZCL attributes with following tariff priceServerInformation.");
    (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PRICE_CLUSTER_ID,
                                        ZCL_TARIFF_LABEL_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                        (uint8_t*)&(tariff->tariffLabel), ZCL_OCTET_STRING_ATTRIBUTE_TYPE);
    (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PRICE_CLUSTER_ID,
                                        ZCL_NUMBER_OF_PRICE_TIERS_IN_USE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                        &tariff->numberOfPriceTiersInUse, ZCL_INT8U_ATTRIBUTE_TYPE);
    (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PRICE_CLUSTER_ID,
                                        ZCL_NUMBER_OF_BLOCK_THRESHOLDS_IN_USE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                        &tariff->numberOfBlockThresholdsInUse, ZCL_INT8U_ATTRIBUTE_TYPE);
    (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PRICE_CLUSTER_ID,
                                        ZCL_TIER_BLOCK_MODE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                        &tariff->tierBlockMode, ZCL_INT8U_ATTRIBUTE_TYPE);
    (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PRICE_CLUSTER_ID,
                                        ZCL_TARIFF_UNIT_OF_MEASURE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                        &tariff->unitOfMeasure, ZCL_INT8U_ATTRIBUTE_TYPE);
    (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PRICE_CLUSTER_ID,
                                        ZCL_TARIFF_CURRENCY_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                        (uint8_t *)&tariff->currency, ZCL_INT16U_ATTRIBUTE_TYPE);
    (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PRICE_CLUSTER_ID,
                                        ZCL_TARIFF_PRICE_TRAILING_DIGIT_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                        &tariff->priceTrailingDigit, ZCL_INT8U_ATTRIBUTE_TYPE);
    (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PRICE_CLUSTER_ID,
                                        ZCL_STANDING_CHARGE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                        (uint8_t *)&tariff->standingCharge, ZCL_INT32U_ATTRIBUTE_TYPE);
    (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PRICE_CLUSTER_ID,
                                        ZCL_THRESHOLD_MULTIPLIER_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                        (uint8_t *)&tariff->blockThresholdMultiplier, ZCL_INT32U_ATTRIBUTE_TYPE);
    (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PRICE_CLUSTER_ID,
                                        ZCL_THRESHOLD_DIVISOR_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                        (uint8_t *)&tariff->blockThresholdDivisor, ZCL_INT32U_ATTRIBUTE_TYPE);
    curInfo->actionsPending = false;
  }

  if ((tariffType == SL_ZIGBEE_ZCL_TARIFF_TYPE_RECEIVED_TARIFF)
      || (tariffType == SL_ZIGBEE_ZCL_TARIFF_TYPE_DELIVERED_AND_RECEIVED_TARIFF)) {
    (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PRICE_CLUSTER_ID,
                                        ZCL_RX_TARIFF_LABEL_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                        (uint8_t*)&(tariff->tariffLabel), ZCL_OCTET_STRING_ATTRIBUTE_TYPE);
    (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PRICE_CLUSTER_ID,
                                        ZCL_RX_NUMBER_OF_PRICE_TIERS_IN_USE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                        &tariff->numberOfPriceTiersInUse, ZCL_INT8U_ATTRIBUTE_TYPE);
    (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PRICE_CLUSTER_ID,
                                        ZCL_RX_NUMBER_OF_BLOCK_THRESHOLDS_IN_USE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                        &tariff->numberOfBlockThresholdsInUse, ZCL_INT8U_ATTRIBUTE_TYPE);
    (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PRICE_CLUSTER_ID,
                                        ZCL_RX_TIER_BLOCK_MODE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                        &tariff->tierBlockMode, ZCL_INT8U_ATTRIBUTE_TYPE);
    (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PRICE_CLUSTER_ID,
                                        ZCL_RX_THRESHOLD_MULTIPLIER_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                        (uint8_t *)&tariff->blockThresholdMultiplier, ZCL_INT32U_ATTRIBUTE_TYPE);
    (void) sl_zigbee_af_write_attribute(endpoint, ZCL_PRICE_CLUSTER_ID,
                                        ZCL_RX_THRESHOLD_DIVISOR_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                        (uint8_t *)&tariff->blockThresholdDivisor, ZCL_INT32U_ATTRIBUTE_TYPE);
    curInfo->actionsPending = false;
  }
}

//-----------------------
// ZCL Commands Callbacks

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_get_currency_conversion_command_cb(void)
{
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }
  sl_zigbee_af_price_cluster_println("Rx: GetCurrencyConversion");
  if ( priceServerInfo.currencyConversionTable.commonInfos[ep].valid ) {
    (void) sl_zigbee_af_fill_command_price_cluster_publish_currency_conversion(priceServerInfo.currencyConversionTable.currencyConversion[ep].providerId,
                                                                               priceServerInfo.currencyConversionTable.commonInfos[ep].issuerEventId,
                                                                               priceServerInfo.currencyConversionTable.commonInfos[ep].startTime,
                                                                               priceServerInfo.currencyConversionTable.currencyConversion[ep].oldCurrency,
                                                                               priceServerInfo.currencyConversionTable.currencyConversion[ep].newCurrency,
                                                                               priceServerInfo.currencyConversionTable.currencyConversion[ep].conversionFactor,
                                                                               priceServerInfo.currencyConversionTable.currencyConversion[ep].conversionFactorTrailingDigit,
                                                                               priceServerInfo.currencyConversionTable.currencyConversion[ep].currencyChangeControlFlags);
    sl_zigbee_af_send_response();
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }
  return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_get_tariff_cancellation_cb(void)
{
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }
  sl_zigbee_af_price_cluster_println("Rx: GetTariffCancellation");
  if ( priceServerInfo.cancelTariffTable.cancelTariff[ep].valid ) {
    (void) sl_zigbee_af_fill_command_price_cluster_cancel_tariff(priceServerInfo.cancelTariffTable.cancelTariff[ep].providerId,
                                                                 priceServerInfo.cancelTariffTable.cancelTariff[ep].issuerTariffId,
                                                                 priceServerInfo.cancelTariffTable.cancelTariff[ep].tariffType);
    sl_zigbee_af_send_response();
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }
  return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_get_current_price_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_get_current_price_command_t cmd_data;
  sl_zigbee_af_scheduled_price_t price;

  if (zcl_decode_price_cluster_get_current_price_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_price_cluster_println("RX: GetCurrentPrice 0x%02X", cmd_data.commandOptions);
  if (!sl_zigbee_af_get_current_price(sl_zigbee_af_current_endpoint(), &price)) {
    sl_zigbee_af_price_cluster_println("no valid price to return!");
    return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  }
  fillPublishPriceCommand(price);
  sl_zigbee_af_send_response();
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_get_scheduled_prices_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_get_scheduled_prices_command_t cmd_data;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();

  if (zcl_decode_price_cluster_get_scheduled_prices_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_price_cluster_println("RX: GetScheduledPrices 0x%08X, 0x%02X",
                                     cmd_data.startTime,
                                     cmd_data.numberOfEvents);

  // Only one GetScheduledPrices can be processed at a time.
  if (partner.index != ZCL_PRICE_INVALID_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_FAILURE;
  }

  partner.startTime = (cmd_data.startTime == ZCL_PRICE_CLUSTER_START_TIME_NOW
                       ? sl_zigbee_af_get_current_time()
                       : cmd_data.startTime);
  uint8_t scheduledPrices = scheduledPriceCount(endpoint, partner.startTime);

  if (cmd_data.numberOfEvents == ZCL_PRICE_CLUSTER_NUMBER_OF_EVENTS_ALL) {
    partner.numberOfEvents = scheduledPrices;
  } else {
    partner.numberOfEvents = (scheduledPrices > cmd_data.numberOfEvents)
                             ? cmd_data.numberOfEvents
                             : scheduledPrices;
  }

  if (partner.numberOfEvents == 0u) {
    sl_zigbee_af_price_cluster_println("no valid price to return!");
    return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  }

  partner.isIntraPan = (cmd->interPanHeader == NULL);
  if (partner.isIntraPan) {
    partner.pan.intra.nodeId = cmd->source;
    partner.pan.intra.clientEndpoint = cmd->apsFrame->sourceEndpoint;
    partner.pan.intra.serverEndpoint = cmd->apsFrame->destinationEndpoint;
  } else {
    partner.pan.inter.panId = cmd->interPanHeader->panId;
    (void) memmove(partner.pan.inter.eui64, cmd->interPanHeader->longAddress, EUI64_SIZE);
  }
  partner.sequence = cmd->seqNum;
  partner.index = 0;

  sli_zigbee_af_price_server_schedule_get_scheduled_prices(sl_zigbee_af_current_endpoint());
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_price_acknowledgement_cb(sl_zigbee_af_cluster_command_t *cmd)
{
#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
  sl_zcl_price_cluster_price_acknowledgement_command_t cmd_data;

  if (zcl_decode_price_cluster_price_acknowledgement_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_price_cluster_println("RX: PriceAcknowledgement 0x%08X, 0x%08X, 0x%08X, 0x%02X",
                                     cmd_data.providerId,
                                     cmd_data.issuerEventId,
                                     cmd_data.priceAckTime,
                                     cmd_data.control);
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_get_block_periods_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_get_block_periods_command_t cmd_data;
  uint8_t  validEntries[SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE];
  uint8_t  validEntriesIndex = 0;
  uint32_t endTimeUtc;
  uint8_t  i;

  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ((zcl_decode_price_cluster_get_block_periods_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_price_cluster_println("RX: Get Block Period");
  if ( cmd_data.startTime == 0u ) {
    cmd_data.startTime = sl_zigbee_af_get_current_time();
  }

  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE; i++ ) {
    if ( priceServerInfo.blockPeriodTable.commonInfos[ep][i].valid ) {
      if ((cmd_data.tariffType == TARIFF_TYPE_UNSPECIFIED)
          || ((priceServerInfo.blockPeriodTable.blockPeriods[ep][i].tariffType & TARIFF_TYPE_MASK) == (cmd_data.tariffType & TARIFF_TYPE_MASK))) {
        // According to "Get Consolidated Bill" command documentation, this response should
        // include consolidated bills that are already active.  So, send back any with
        // end time that is > earliestStartTime.
        endTimeUtc = priceServerInfo.blockPeriodTable.commonInfos[ep][i].startTime + priceServerInfo.blockPeriodTable.commonInfos[ep][i].durationSec;
        if ( (cmd_data.startTime < endTimeUtc) ) {
          validEntries[validEntriesIndex] = i;
          // NOTE:  Incrementing validEntriesIndex first ensures that all entries are sent if numberOfEvents == 0.
          validEntriesIndex++;
          if ( validEntriesIndex == cmd_data.numberOfEvents ) {
            break;
          }
        }
      }
    }
  }
  // Have a set of valid block periods.  Total number == validEntriesIndex.
  sendValidCmdEntries(ZCL_PUBLISH_BLOCK_PERIOD_COMMAND_ID, ep, validEntries, validEntriesIndex);

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_get_conversion_factor_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_get_conversion_factor_command_t cmd_data;
  uint8_t cmdCount;
  uint8_t validCmds[SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE];
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ((zcl_decode_price_cluster_get_conversion_factor_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  cmdCount = sl_zigbee_af_price_common_find_valid_entries(validCmds,
                                                          SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE,
                                                          priceServerInfo.conversionFactorTable.commonInfos[ep],
                                                          cmd_data.earliestStartTime,
                                                          cmd_data.minIssuerEventId,
                                                          cmd_data.numberOfCommands);

  sendValidCmdEntries(ZCL_PUBLISH_CONVERSION_FACTOR_COMMAND_ID, ep,
                      validCmds,
                      cmdCount);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_get_calorific_value_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_get_calorific_value_command_t cmd_data;
  uint8_t cmdCount;
  uint8_t validCmds[SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE];
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ((zcl_decode_price_cluster_get_calorific_value_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  cmdCount = sl_zigbee_af_price_common_find_valid_entries(validCmds,
                                                          SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE,
                                                          priceServerInfo.calorificValueTable.commonInfos[ep],
                                                          cmd_data.earliestStartTime,
                                                          cmd_data.minIssuerEventId,
                                                          cmd_data.numberOfCommands);

  sendValidCmdEntries(ZCL_PUBLISH_CALORIFIC_VALUE_COMMAND_ID, ep,
                      validCmds,
                      cmdCount);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_get_co2_value_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_get_co2_value_command_t cmd_data;
  uint8_t cmdCount;
  uint8_t validCmds[SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE];
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ((zcl_decode_price_cluster_get_co2_value_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }
  cmdCount = sl_zigbee_af_price_common_find_valid_entries(validCmds,
                                                          SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE,
                                                          priceServerInfo.co2ValueTable.commonInfos[ep],
                                                          cmd_data.earliestStartTime,
                                                          cmd_data.minIssuerEventId,
                                                          cmd_data.numberOfCommands);

  // eliminate commands with mismatching tariffType
  // upper nibble is reserved. we'll ignore them.
  if (cmd_data.tariffType != TARIFF_TYPE_UNSPECIFIED) {
    uint8_t i;
    for (i = 0; i < cmdCount; i++) {
      uint8_t index = validCmds[i];
      if ((priceServerInfo.co2ValueTable.co2Values[ep][index].tariffType & TARIFF_TYPE_MASK) != (cmd_data.tariffType & TARIFF_TYPE_MASK)) {
        validCmds[i] = ZCL_PRICE_INVALID_INDEX;
      }
    }
  }
  sendValidCmdEntries(ZCL_PUBLISH_CO2_VALUE_COMMAND_ID, ep, validCmds, cmdCount);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_get_tier_labels_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_get_tier_labels_command_t cmd_data;
  uint8_t i;
  uint8_t validCmds[SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE];
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t validCmdCnt = 0;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ((zcl_decode_price_cluster_get_tier_labels_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE; i++) {
    if ( (priceServerInfo.tierLabelTable.entry[ep][i].issuerTariffId == cmd_data.issuerTariffId)
         && (priceServerInfo.tierLabelTable.entry[ep][i].valid != 0u) ) {
      validCmds[validCmdCnt] = i;
      validCmdCnt++;
    }
  }
  sendValidCmdEntries(ZCL_PUBLISH_TIER_LABELS_COMMAND_ID, ep, validCmds, validCmdCnt);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_get_billing_period_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_get_billing_period_command_t cmd_data;
  uint32_t issuerEventId;
  uint32_t startTime;
  uint32_t durationSec;
  uint8_t  curTariffType;
  bool  validEntry;

  uint8_t validEntries[SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE];
  uint8_t validEntryIndex = 0;
  uint8_t requestEntryCount = SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE;
  uint8_t i;

  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ((zcl_decode_price_cluster_get_billing_period_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }
  sl_zigbee_af_price_cluster_println("RX: GetBillingPeriod, 0x%08X, 0x%08X, 0x%02X, 0x%02X",
                                     cmd_data.earliestStartTime,
                                     cmd_data.minIssuerEventId,
                                     cmd_data.numberOfCommands,
                                     cmd_data.tariffType);

  if ((cmd_data.numberOfCommands != 0x00u) && (requestEntryCount > cmd_data.numberOfCommands)) {
    requestEntryCount = cmd_data.numberOfCommands;
  }
  if ( cmd_data.earliestStartTime == 0u ) {
    cmd_data.earliestStartTime = sl_zigbee_af_get_current_time();
  }

  (void) memset(validEntries, 0xFF, SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE);

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE; i++) {
    issuerEventId = priceServerInfo.billingPeriodTable.commonInfos[ep][i].issuerEventId;
    startTime = priceServerInfo.billingPeriodTable.commonInfos[ep][i].startTime;
    durationSec = priceServerInfo.billingPeriodTable.commonInfos[ep][i].durationSec;
    curTariffType = priceServerInfo.billingPeriodTable.billingPeriods[ep][i].tariffType;
    validEntry = priceServerInfo.billingPeriodTable.commonInfos[ep][i].valid;

    if (!validEntry) {
      continue;
    }
    sl_zigbee_af_price_cluster_println("priceServerInfo for entry index: %d", i);
    sl_zigbee_af_price_cluster_println("start time: 0x%08X", startTime);
    sl_zigbee_af_price_cluster_println("end time: 0x%08X", startTime + durationSec);
    sl_zigbee_af_price_cluster_println("duration: 0x%08X", durationSec);

    // invalid entry tariffType
    if (cmd_data.tariffType != TARIFF_TYPE_DONT_CARE) {
      if ((cmd_data.tariffType & TARIFF_TYPE_MASK) !=  (curTariffType & TARIFF_TYPE_MASK)) {
        continue;
      }
    }

    if (cmd_data.earliestStartTime > (startTime + durationSec)) {
      continue;
    }

    if ((cmd_data.minIssuerEventId != 0xFFFFFFFFU)
        && (cmd_data.minIssuerEventId > issuerEventId)) {
      continue;
    }

    // valid entry
    validEntries[validEntryIndex] = i;
    validEntryIndex++;
    if (requestEntryCount == validEntryIndex) {
      break;
    }
  }

  sendValidCmdEntries(ZCL_PUBLISH_BILLING_PERIOD_COMMAND_ID, ep, validEntries, validEntryIndex);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_get_consolidated_bill_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_get_consolidated_bill_command_t cmd_data;
  uint8_t  validEntries[SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONSOLIDATED_BILL_TABLE_SIZE];
  uint8_t  validEntriesIndex = 0;
  uint32_t endTimeUtc;
  uint8_t  i;

  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ((zcl_decode_price_cluster_get_consolidated_bill_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_price_cluster_println("RX: GetConsolidatedBill, 0x%08X, 0x%08X, 0x%02X, 0x%02X",
                                     cmd_data.earliestStartTime,
                                     cmd_data.minIssuerEventId,
                                     cmd_data.numberOfCommands,
                                     cmd_data.tariffType);

  if ( cmd_data.minIssuerEventId == EVENT_ID_UNSPECIFIED ) {
    cmd_data.minIssuerEventId = 0;   // Allow all event IDs
  }
  if ( cmd_data.earliestStartTime == 0u ) {
    cmd_data.earliestStartTime = sl_zigbee_af_get_current_time();
  }

  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CONSOLIDATED_BILL_TABLE_SIZE; i++ ) {
    if ( priceServerInfo.consolidatedBillsTable.commonInfos[ep][i].valid ) {
      sl_zigbee_af_price_cluster_println("===  i=%d, st=0x%08X, ev=%d, tar=%d,   st[]=0x%08X, ev[]=%d, tar[]=%d",
                                         i, cmd_data.earliestStartTime, cmd_data.minIssuerEventId, cmd_data.tariffType,
                                         priceServerInfo.consolidatedBillsTable.commonInfos[ep][i].startTime,
                                         priceServerInfo.consolidatedBillsTable.commonInfos[ep][i].issuerEventId,
                                         priceServerInfo.consolidatedBillsTable.consolidatedBills[ep][i].tariffType);

      if ((cmd_data.tariffType == TARIFF_TYPE_UNSPECIFIED)
          || ((priceServerInfo.consolidatedBillsTable.consolidatedBills[ep][i].tariffType & TARIFF_TYPE_MASK)
              == (cmd_data.tariffType & TARIFF_TYPE_MASK))) {
        // According to "Get Consolidated Bill" command documentation, this response should
        // include consolidated bills that are already active.  So, send back any with
        // end time that is > earliestStartTime.
        endTimeUtc = priceServerInfo.consolidatedBillsTable.commonInfos[ep][i].startTime + priceServerInfo.consolidatedBillsTable.commonInfos[ep][i].durationSec;
        if ( (cmd_data.earliestStartTime < endTimeUtc) && (cmd_data.minIssuerEventId <= priceServerInfo.consolidatedBillsTable.commonInfos[ep][i].issuerEventId) ) {
          validEntries[validEntriesIndex] = i;
          validEntriesIndex++;
          // NOTE:  Incrementing validEntriesIndex first ensures that all entries are sent if numberOfCommands == 0.
          if ( validEntriesIndex == cmd_data.numberOfCommands ) {
            break;
          }
        }
      }
    }
  }
  // Have a set of valid consolidated bills.  Total number == validEntriesIndex.
  sendValidCmdEntries(ZCL_PUBLISH_CONSOLIDATED_BILL_COMMAND_ID, ep, validEntries, validEntriesIndex);

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_cpp_event_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_cpp_event_response_command_t cmd_data;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ((zcl_decode_price_cluster_cpp_event_response_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_price_cluster_println("Rx: Cpp Event Response, issuerEventId=0x%08X, auth=%d,   storedCppAuth=%d", cmd_data.issuerEventId, cmd_data.cppAuth,
                                     priceServerInfo.cppTable.cppEvent[ep].cppAuth);
  if ( priceServerInfo.cppTable.cppEvent[ep].cppAuth == SL_ZIGBEE_AF_PLUGIN_PRICE_CPP_AUTH_PENDING ) {
    // Update the CPP auth status based on the Cpp Event Response we received from the client.
    // Send another CPP Event with the updated status to confirm.  See D.4.2.4.12.4,  Fig D-88.
//    priceServerInfo.cppEvent.cppAuth = cppAuth;
//    priceServerInfo.cppEvent.commonInfos.issuerEventId++;    // Test 12.68 Item 5 requires this message have a different event ID than
    // the event ID used in the Pending status message.  IS THIS RIGHT?
    sl_zigbee_af_price_cluster_println("Send CPP Event, stat=%d", cmd_data.cppAuth);
    (void) sl_zigbee_af_fill_command_price_cluster_publish_cpp_event(priceServerInfo.cppTable.cppEvent[ep].providerId,
                                                                     priceServerInfo.cppTable.commonInfos[ep].issuerEventId,
                                                                     priceServerInfo.cppTable.commonInfos[ep].startTime,
                                                                     priceServerInfo.cppTable.cppEvent[ep].durationInMinutes,
                                                                     priceServerInfo.cppTable.cppEvent[ep].tariffType,
                                                                     priceServerInfo.cppTable.cppEvent[ep].cppPriceTier,
                                                                     cmd_data.cppAuth);

    sl_zigbee_af_send_response();
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }
  //else{
  // PER SE TEST 12.70, item 3-5, server should not send a message if an asynch CPP Event Response is received.
  //sl_zigbee_af_send_immediate_default_response( SL_ZIGBEE_ZCL_STATUS_SUCCESS );
  //}
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_get_credit_payment_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_get_credit_payment_command_t cmd_data;
  uint8_t  validEntries[SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CREDIT_PAYMENT_TABLE_SIZE];
  uint8_t  validEntriesIndex = 0;
  uint8_t  i;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);

  if ((zcl_decode_price_cluster_get_credit_payment_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_price_cluster_println("Rx: GetCreditPayment, endTime=%d, nr=%d", cmd_data.latestEndTime, cmd_data.numberOfRecords);
  if ( cmd_data.numberOfRecords == 0u ) {
    cmd_data.numberOfRecords = SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CREDIT_PAYMENT_TABLE_SIZE;
  }
  if ( cmd_data.latestEndTime == 0u ) {
    cmd_data.latestEndTime = sl_zigbee_af_get_current_time();
  }

  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_PRICE_SERVER_CREDIT_PAYMENT_TABLE_SIZE; i++ ) {
    if ( (priceServerInfo.creditPaymentTable.commonInfos[ep][i].valid) && (priceServerInfo.creditPaymentTable.creditPayment[ep][i].creditPaymentDate <= cmd_data.latestEndTime) ) {
      validEntries[validEntriesIndex] = i;
      validEntriesIndex++;
      // NOTE:  Incrementing validEntriesIndex first ensures that all entries are sent if numberOfEvents == 0.
      if ( validEntriesIndex == cmd_data.numberOfRecords ) {
        break;
      }
    }
  }
  if ( validEntriesIndex == 0u ) {
    sl_zigbee_af_price_cluster_println("No matching credit payments");
  }
  sortCreditPaymentEntries(validEntries, validEntriesIndex, priceServerInfo.creditPaymentTable.creditPayment[ep]);

  sendValidCmdEntries(ZCL_PUBLISH_CREDIT_PAYMENT_COMMAND_ID, ep, validEntries, validEntriesIndex);

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_get_tariff_information_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_get_price_matrix_cb(sl_zigbee_af_cluster_command_t *cmd);
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_get_block_thresholds_cb(sl_zigbee_af_cluster_command_t *cmd);

uint32_t sl_zigbee_af_price_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                         sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_GET_CURRENT_PRICE_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_get_current_price_cb(cmd);
        break;
      }
      case ZCL_GET_SCHEDULED_PRICES_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_get_scheduled_prices_cb(cmd);
        break;
      }
      case ZCL_PRICE_ACKNOWLEDGEMENT_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_price_acknowledgement_cb(cmd);
        break;
      }
      case ZCL_GET_BLOCK_PERIODS_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_get_block_periods_cb(cmd);
        break;
      }
      case ZCL_GET_CONVERSION_FACTOR_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_get_conversion_factor_cb(cmd);
        break;
      }
      case ZCL_GET_CALORIFIC_VALUE_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_get_calorific_value_cb(cmd);
        break;
      }
      case ZCL_GET_TARIFF_INFORMATION_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_get_tariff_information_cb(cmd);
        break;
      }
      case ZCL_GET_PRICE_MATRIX_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_get_price_matrix_cb(cmd);
        break;
      }
      case ZCL_GET_BLOCK_THRESHOLDS_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_get_block_thresholds_cb(cmd);
        break;
      }
      case ZCL_GET_CO2_VALUE_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_get_co2_value_cb(cmd);
        break;
      }
      case ZCL_GET_TIER_LABELS_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_get_tier_labels_cb(cmd);
        break;
      }
      case ZCL_GET_BILLING_PERIOD_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_get_billing_period_cb(cmd);
        break;
      }
      case ZCL_GET_CONSOLIDATED_BILL_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_get_consolidated_bill_cb(cmd);
        break;
      }
      case ZCL_CPP_EVENT_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_cpp_event_response_cb(cmd);
        break;
      }
      case ZCL_GET_CREDIT_PAYMENT_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_get_credit_payment_cb(cmd);
        break;
      }
      case ZCL_GET_CURRENCY_CONVERSION_COMMAND_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_get_currency_conversion_command_cb();
        break;
      }
      case ZCL_GET_TARIFF_CANCELLATION_COMMAND_ID:
      {
        status = sl_zigbee_af_price_cluster_get_tariff_cancellation_cb();
        break;
      }
      default:
        break;
    }
  }

  return status;
}
