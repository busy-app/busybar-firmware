/***************************************************************************//**
 * @file
 * @brief CLI for the Price Server plugin.
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
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "price-server.h"
#include "zigbee_af_cluster_functions.h"

//=============================================================================
// Globals

static sl_zigbee_af_scheduled_price_t price;

//=============================================================================
// Functions
// plugin price-server init <endpoint:1>
void sli_zigbee_af_price_server_cli_init(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_cluster_server_init_cb(sl_cli_get_argument_uint8(arguments, 0));
}

// plugin price-server clear <endpoint:1>
void sli_zigbee_af_price_server_cli_clear(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_clear_price_table(sl_cli_get_argument_uint8(arguments, 0));
}

// plugin price-server who <provId:4> <label:1-13> <eventId:4>
void sli_zigbee_af_price_server_cli_who(sl_cli_command_arg_t *arguments)
{
  uint8_t length;
  price.providerId = sl_cli_get_argument_uint32(arguments, 0);
  length = sl_zigbee_copy_string_arg(arguments, 1,
                                     price.rateLabel + 1,
                                     ZCL_PRICE_CLUSTER_MAXIMUM_RATE_LABEL_LENGTH,
                                     false);
  price.rateLabel[0] = length;
  price.issuerEventID = sl_cli_get_argument_uint32(arguments, 2);
}

// plugin price-server what <unitOfMeas:1> <curr:2> <ptd:1> <PTRT:1> <bt:1>
void sli_zigbee_af_price_server_cli_what(sl_cli_command_arg_t *arguments)
{
  price.unitOfMeasure = sl_cli_get_argument_uint8(arguments, 0);
  price.currency = sl_cli_get_argument_uint16(arguments, 1);
  price.priceTrailingDigitAndTier = sl_cli_get_argument_uint8(arguments, 2);
  price.numberOfPriceTiersAndTier = sl_cli_get_argument_uint8(arguments, 3);
  price.numberOfBlockThresholds = sl_cli_get_argument_uint8(arguments, 4);
}

// plugin price-server when <startTime:4> <duration:2>
void sli_zigbee_af_price_server_cli_when(sl_cli_command_arg_t *arguments)
{
  price.startTime = sl_cli_get_argument_uint32(arguments, 0);
  price.duration = sl_cli_get_argument_uint16(arguments, 1);
}

// plugin price-server price <price:4> <ratio:1> <genPrice:4> <genRatio:1>
void sli_zigbee_af_price_server_cli_price(sl_cli_command_arg_t *arguments)
{
  price.price = sl_cli_get_argument_uint32(arguments, 0);
  price.priceRatio = sl_cli_get_argument_uint8(arguments, 1);
  price.generationPrice = sl_cli_get_argument_uint32(arguments, 2);
  price.generationPriceRatio = sl_cli_get_argument_uint8(arguments, 3);
}

// plugin price-server alternate <alternateCostDelivered:4> <alternateCostUnit:1> <alternateCostTrailingDigit:1>
void sli_zigbee_af_price_server_cli_alternate(sl_cli_command_arg_t *arguments)
{
  price.alternateCostDelivered = sl_cli_get_argument_uint32(arguments, 0);
  price.alternateCostUnit = sl_cli_get_argument_uint8(arguments, 1);
  price.alternateCostTrailingDigit = sl_cli_get_argument_uint8(arguments, 2);
}

// plugin price-server ack <req:1>
void sli_zigbee_af_price_server_cli_ack(sl_cli_command_arg_t *arguments)
{
  price.priceControl &= ~ZCL_PRICE_CLUSTER_PRICE_ACKNOWLEDGEMENT_MASK;
  if ( (sl_cli_get_argument_uint8(arguments, 0)) == 1u) {
    price.priceControl |= SL_ZIGBEE_ZCL_PRICE_CONTROL_ACKNOWLEDGEMENT_REQUIRED;
  }
}

// plugin price-server <valid | invalid | tinvalid> <endpoint:1> <index:1>
void sli_zigbee_af_price_server_cli_valid(sl_cli_command_arg_t *arguments)
{
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t index = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t position = sl_cli_get_command_count(arguments) - 1;
  uint8_t command_first_character = sl_cli_get_command_string(arguments, position)[0];
  if (command_first_character == 't') {
    if (!sl_zigbee_af_price_set_tariff_table_entry(endpoint,
                                                   index,
                                                   NULL,
                                                   NULL)) {
      sl_zigbee_af_price_cluster_println("tariff entry %d not present", index);
    }

    return;
  }

  if (!sl_zigbee_af_price_set_price_table_entry(endpoint,
                                                index,
                                                (command_first_character == 'v'
                                                 ? &price
                                                 : NULL))) {
    sl_zigbee_af_price_cluster_println("price entry %d not present", index);
  }
}

// plugin price-server get <endpoint:1> <index:1>
void sli_zigbee_af_price_server_cli_get(sl_cli_command_arg_t *arguments)
{
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t index = sl_cli_get_argument_uint8(arguments, 1);
  if (!sl_zigbee_af_price_get_price_table_entry(endpoint, index, &price)) {
    sl_zigbee_af_price_cluster_println("price entry %d not present", index);
  }
}

// plugin price-server print <endpoint:1>
void sli_zigbee_af_price_server_cli_print(sl_cli_command_arg_t *arguments)
{
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_af_price_print_price_table(endpoint);
}

// plugin price-server sprint
void sli_zigbee_af_price_server_cli_sprint(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_zigbee_af_price_print(&price);
}

// plugin price-server publish <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <priceIndex:1>
void sli_zigbee_af_price_server_cli_publish(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_publish_price_message((sl_802154_short_addr_t)sl_cli_get_argument_uint16(arguments, 0),
                                                  sl_cli_get_argument_uint8(arguments, 1),
                                                  sl_cli_get_argument_uint8(arguments, 2),
                                                  sl_cli_get_argument_uint8(arguments, 3));
}

// plugin price-server pub-tariff-info <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <tariffIndex:1>
void sli_zigbee_af_price_server_cli_pub_tariff_info(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_publish_tariff_message((sl_802154_short_addr_t)sl_cli_get_argument_uint16(arguments, 0),
                                                   sl_cli_get_argument_uint8(arguments, 1),
                                                   sl_cli_get_argument_uint8(arguments, 2),
                                                   sl_cli_get_argument_uint8(arguments, 3));
}

// plugin price-server block-period add <endpoint:1> <providerId:4> <issuerEventId:4> <blockPeriodStartTime:4> <blockPeriodDuration:4> <blockPeriodControl:1> <blockPeriodDurationType:1> <tariffType:1> <tariffResolutionPeriod:1>
void sli_zigbee_af_price_server_cli_block_period_add(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_block_period_add(sl_cli_get_argument_uint8(arguments, 0),
                                             sl_cli_get_argument_uint32(arguments, 1),
                                             sl_cli_get_argument_uint32(arguments, 2),
                                             sl_cli_get_argument_uint32(arguments, 3),
                                             sl_cli_get_argument_uint32(arguments, 4),
                                             sl_cli_get_argument_uint8(arguments, 5),
                                             sl_cli_get_argument_uint8(arguments, 6),
                                             sl_cli_get_argument_uint32(arguments, 7),
                                             sl_cli_get_argument_uint32(arguments, 8),
                                             sl_cli_get_argument_uint8(arguments, 9),
                                             sl_cli_get_argument_uint8(arguments, 10) );
}

// plugin price-server block-period print <endpoint:1> <index:1>
void sli_zigbee_af_price_server_cli_block_period_print(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_block_period_print(sl_cli_get_argument_uint8(arguments, 0),
                                               sl_cli_get_argument_uint8(arguments, 1) );
}

// plugin price-server block-period pub <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <index:1>
void sli_zigbee_af_price_server_cli_block_period_pub(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_block_period_pub(sl_cli_get_argument_uint16(arguments, 0),
                                             sl_cli_get_argument_uint8(arguments, 1),
                                             sl_cli_get_argument_uint8(arguments, 2),
                                             sl_cli_get_argument_uint8(arguments, 3));
}

// plugin price-server block-period clear <endpoint:1>
void sli_zigbee_af_price_server_cli_block_period_clear(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_clear_block_period_table(sl_cli_get_argument_uint8(arguments, 0));
}

// plugin price-server conversion-factor add <endpoint:1> <issuerEventId:4> <startTime:4> <conversionFactor:4> <conversionFactorTrailingDigit:1>
void sli_zigbee_af_price_server_cli_conversion_factor_add(sl_cli_command_arg_t *arguments)
{
  (void) sl_zigbee_af_price_server_conversion_factor_add(sl_cli_get_argument_uint8(arguments, 0),
                                                         sl_cli_get_argument_uint32(arguments, 1),
                                                         sl_cli_get_argument_uint32(arguments, 2),
                                                         sl_cli_get_argument_uint32(arguments, 3),
                                                         sl_cli_get_argument_uint8(arguments, 4) );
}

// plugin price-server conversion-factor clear <endpoint:1>
void sli_zigbee_af_price_server_cli_conversion_factor_clear(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_conversion_factor_clear(sl_cli_get_argument_uint8(arguments, 0) );
}

// plugin price-server conversion-factor pub <tableIndex:1>  <dst:2> <src endpoint:1>  <dst endpoint:1>
void sli_zigbee_af_price_server_cli_conversion_factor_pub(sl_cli_command_arg_t *arguments)
{
  uint8_t tableIndex = sl_cli_get_argument_uint8(arguments, 0);
  sl_802154_short_addr_t dstAddr = (sl_802154_short_addr_t)sl_cli_get_argument_uint16(arguments, 1);
  uint8_t srcEndpoint =  sl_cli_get_argument_uint8(arguments, 2);
  uint8_t dstEndpoint =  sl_cli_get_argument_uint8(arguments, 3);
  sl_zigbee_af_price_server_conversion_factor_pub(tableIndex, dstAddr, srcEndpoint, dstEndpoint);
}

// plugin price-server conversion-factor print <endpoint:1>
void sli_zigbee_af_price_server_cli_conversion_factor_print(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_print_conversion_table(sl_cli_get_argument_uint8(arguments, 0) );
}

// plugin price-server calorific-value add <endpoint:1> <issuerEventId:4> <startTime:4> <calorificValue:4> <calorificValueUnit:1> <calorificValueTrailingDigit:1>
void sli_zigbee_af_price_server_cli_calorific_value_add(sl_cli_command_arg_t *arguments)
{
  (void) sl_zigbee_af_price_server_calorific_value_add(sl_cli_get_argument_uint8(arguments, 0),
                                                       sl_cli_get_argument_uint32(arguments, 1),
                                                       sl_cli_get_argument_uint32(arguments, 2),
                                                       sl_cli_get_argument_uint32(arguments, 3),
                                                       sl_cli_get_argument_uint8(arguments, 4),
                                                       sl_cli_get_argument_uint8(arguments, 5));
}

// plugin price-server calorific-factor clear <endpoint:1>
void sli_zigbee_af_price_server_cli_calorific_value_clear(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_calorific_value_clear(sl_cli_get_argument_uint8(arguments, 0) );
}

// plugin price-server calorific-factor print <endpoint:1>
void sli_zigbee_af_price_server_cli_calorific_value_print(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_print_calorific_values_table(sl_cli_get_argument_uint8(arguments, 0) );
}

// plugin price-server co2-val add <endpoint:1> <issuerEventId:4> <startTime:4> <providerId:4> <tariffType:1> <co2Value:4> <co2ValueUnit:1> <co2ValueTrailingDigit:1>
void sli_zigbee_af_price_server_cli_co2_value_add(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_co2_value_add(sl_cli_get_argument_uint8(arguments, 0),
                                          sl_cli_get_argument_uint32(arguments, 1),
                                          sl_cli_get_argument_uint32(arguments, 2),
                                          sl_cli_get_argument_uint32(arguments, 3),
                                          sl_cli_get_argument_uint8(arguments, 4),
                                          sl_cli_get_argument_uint32(arguments, 5),
                                          sl_cli_get_argument_uint8(arguments, 6),
                                          sl_cli_get_argument_uint8(arguments, 7));
}

// plugin price-server co2-val clear <endpoint:1>
void sli_zigbee_af_price_server_cli_co2_value_clear(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_co2_value_clear(sl_cli_get_argument_uint8(arguments, 0) );
}

// plugin price-server co2-val print <endpoint:1>
void sli_zigbee_af_price_server_cli_co2_value_print(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_print_co2_values_table(sl_cli_get_argument_uint8(arguments, 0) );
}

// plugin price-server tier-label set
void sli_zigbee_af_price_server_cli_tier_label_set(sl_cli_command_arg_t *arguments)
{
  uint8_t len;
  uint8_t tierLabel[13];
  len = sl_zigbee_copy_string_arg(arguments, 7, tierLabel + 1, 12, false);

  tierLabel[0] = len;
  sl_zigbee_af_price_server_tier_label_set(sl_cli_get_argument_uint8(arguments, 0),
                                           sl_cli_get_argument_uint8(arguments, 1),
                                           sl_cli_get_argument_uint8(arguments, 2),
                                           sl_cli_get_argument_uint32(arguments, 3),
                                           sl_cli_get_argument_uint32(arguments, 4),
                                           sl_cli_get_argument_uint32(arguments, 5),
                                           sl_cli_get_argument_uint8(arguments, 6),
                                           tierLabel);
}

// plugin price-server tier-label add-label
void sli_zigbee_af_price_server_cli_tier_label_add_label(sl_cli_command_arg_t *arguments)
{
  uint8_t len;
  uint8_t tierLabel[13];
  uint32_t tariffId;
  len = sl_zigbee_copy_string_arg(arguments, 3, tierLabel + 1, 12, false);
  tierLabel[0] = len;

  tariffId = sl_cli_get_argument_uint32(arguments, 1);
  sl_zigbee_af_price_server_tier_label_add_label(sl_cli_get_argument_uint8(arguments, 0),
                                                 tariffId,
                                                 sl_cli_get_argument_uint8(arguments, 2),
                                                 tierLabel);
}

// plugin price-server tier-label print <endpoint:1>
void sli_zigbee_af_price_server_cli_tier_label_print(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_print_tier_labels_table(sl_cli_get_argument_uint8(arguments, 0) );
}

// plugin price-server tier-label pub  <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <tableIndex:1>
void sli_zigbee_af_price_server_cli_tier_label_pub(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_tier_label_pub(sl_cli_get_argument_uint16(arguments, 0),
                                           sl_cli_get_argument_uint8(arguments, 1),
                                           sl_cli_get_argument_uint8(arguments, 2),
                                           sl_cli_get_argument_uint8(arguments, 3));
}

// plugin price-server co2-val pub <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <tableIndex:1>
void sli_zigbee_af_price_server_cli_co2_value_pub(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_co2_label_pub(sl_cli_get_argument_uint16(arguments, 0),
                                          sl_cli_get_argument_uint8(arguments, 1),
                                          sl_cli_get_argument_uint8(arguments, 2),
                                          sl_cli_get_argument_uint8(arguments, 3));
}

// plugin price-server billing-period add <endpoint:1> <startTime:4>
//                                        <issuerEventId:4> <providerId:4>
//                                        <providerId:4> <billingPeriodDuration:4>
//                                        <billingPeriodDuration:1> <tariffType:1>
void sli_zigbee_af_price_server_cli_billing_period_add(sl_cli_command_arg_t *arguments)
{
  (void) sl_zigbee_af_price_server_billing_period_add(sl_cli_get_argument_uint8(arguments, 0),
                                                      sl_cli_get_argument_uint32(arguments, 1),
                                                      sl_cli_get_argument_uint32(arguments, 2),
                                                      sl_cli_get_argument_uint32(arguments, 3),
                                                      sl_cli_get_argument_uint32(arguments, 4),
                                                      sl_cli_get_argument_uint8(arguments, 5),
                                                      sl_cli_get_argument_uint8(arguments, 6));
}

// plugin price-server billing-period print <endpoint:1>
void sli_zigbee_af_price_server_cli_billing_period_print(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_print_billing_period_table(sl_cli_get_argument_uint8(arguments, 0) );
}

// plugin price-server billing-period pub
void sli_zigbee_af_price_server_cli_billing_period_pub(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_billing_period_pub(sl_cli_get_argument_uint16(arguments, 0),
                                               sl_cli_get_argument_uint8(arguments, 1),
                                               sl_cli_get_argument_uint8(arguments, 2),
                                               sl_cli_get_argument_uint8(arguments, 3));
}

void sli_zigbee_af_price_server_cli_billing_period_repeat(sl_cli_command_arg_t *arguments)
{
  sli_zigbee_af_price_server_billing_period_repeat = (bool)sl_cli_get_argument_uint8(arguments, 0);
}

// plugin price-server consol-bill add  <endpoint:1> <startTime:4>
//                                      <issuerEventId:4> <providerId:4>
//                                      <billingPeriodDuration:4> <billingPeriodDurationType:1>
//                                      <tariffType:1> <consolidatedBill:4>
//                                      <currency:2> <billTrailingDigit:1>
void sli_zigbee_af_price_server_cli_consolidated_bill_add(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_consolidated_bill_add(sl_cli_get_argument_uint8(arguments, 0),
                                                  sl_cli_get_argument_uint32(arguments, 1),
                                                  sl_cli_get_argument_uint32(arguments, 2),
                                                  sl_cli_get_argument_uint32(arguments, 3),
                                                  sl_cli_get_argument_uint32(arguments, 4),
                                                  sl_cli_get_argument_uint8(arguments, 5),
                                                  sl_cli_get_argument_uint8(arguments, 6),
                                                  sl_cli_get_argument_uint32(arguments, 7),
                                                  sl_cli_get_argument_uint16(arguments, 8),
                                                  sl_cli_get_argument_uint8(arguments, 9) );
}

// plugin price-server consol-bill print <endpoint:1> <index:1>
void sli_zigbee_af_price_server_cli_consolidated_bill_print(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_print_consolidated_bill_table_entry(sl_cli_get_argument_uint32(arguments, 0),
                                                   sl_cli_get_argument_uint32(arguments, 1) );
}

// plugin price-server consol-bill pub  <nodeId:2> <srcEp:1> <dstEp:1> <index:1>
void sli_zigbee_af_price_server_cli_consolidated_bill_pub(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_consolidated_bill_pub(sl_cli_get_argument_uint16(arguments, 0),
                                                  sl_cli_get_argument_uint8(arguments, 1),
                                                  sl_cli_get_argument_uint8(arguments, 2),
                                                  sl_cli_get_argument_uint8(arguments, 3));
}

// plugin price-server cpp-event set <endpoint:1> <valid:1> <providerId:4> <issuerEventId:4> <startTime:4>
//                                  <durationInMins:2> <tariffType:1> <cppPriceTier:1> <cppAuth:1>
void sli_zigbee_af_price_server_cli_cpp_event_set(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_cpp_event_set(sl_cli_get_argument_uint8(arguments, 0),
                                          sl_cli_get_argument_uint8(arguments, 1),
                                          sl_cli_get_argument_uint32(arguments, 2),
                                          sl_cli_get_argument_uint32(arguments, 3),
                                          sl_cli_get_argument_uint32(arguments, 4),
                                          sl_cli_get_argument_uint16(arguments, 5),
                                          sl_cli_get_argument_uint8(arguments, 6),
                                          sl_cli_get_argument_uint8(arguments, 7),
                                          sl_cli_get_argument_uint8(arguments, 8));
}

// plugin price-server cpp-event pub <nodeId:2> <srcEndpoint:1> <dstEndpoint:1>
void sli_zigbee_af_price_server_cli_cpp_event_pub(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_cpp_event_pub(sl_cli_get_argument_uint16(arguments, 0),
                                          sl_cli_get_argument_uint8(arguments, 1),
                                          sl_cli_get_argument_uint8(arguments, 2) );
}

// plugin price-server cpp-event print <endpoint:1>
void sli_zigbee_af_price_server_cli_cpp_event_print(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_cpp_event_print(sl_cli_get_argument_uint8(arguments, 0) );
}

// plugin price-server credit-pmt pub <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <index:1>
void sli_zigbee_af_price_server_cli_credit_payment_pub(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_credit_payment_pub(sl_cli_get_argument_uint16(arguments, 0),
                                               sl_cli_get_argument_uint8(arguments, 1),
                                               sl_cli_get_argument_uint8(arguments, 2),
                                               sl_cli_get_argument_uint8(arguments, 3));
}

// plugin price-server credit-pmt set <endpoint:1> <index:1> <valid:1>
//                      <providerId:4> <issuerEventId:4> <creditPaymentDueDate:4>
//                      <creditPaymentOverdueAmount:4> <creditPaymentStatus:1> <creditPayment:4> <creditPaymentDate:4>
//                      <creditPaymentRef:STRING>
void sli_zigbee_af_price_server_cli_credit_payment_set(sl_cli_command_arg_t *arguments)
{
  uint8_t len;
  uint8_t creditPaymentRef[21];
  len = sl_zigbee_copy_string_arg(arguments, 10, creditPaymentRef + 1, 20, false);
  creditPaymentRef[0] = len;
  sl_zigbee_af_price_server_credit_payment_set(sl_cli_get_argument_uint8(arguments, 0),
                                               sl_cli_get_argument_uint8(arguments, 1),
                                               sl_cli_get_argument_uint8(arguments, 2),
                                               sl_cli_get_argument_uint32(arguments, 3),
                                               sl_cli_get_argument_uint32(arguments, 4),
                                               sl_cli_get_argument_uint32(arguments, 5),
                                               sl_cli_get_argument_uint32(arguments, 6),
                                               sl_cli_get_argument_uint8(arguments, 7),
                                               sl_cli_get_argument_uint32(arguments, 8),
                                               sl_cli_get_argument_uint32(arguments, 9),
                                               creditPaymentRef);
}

// plugin price-server currency-conv pub <nodeId:2> <srcEndpoint:1> <dstEndpoint:1>
void sli_zigbee_af_price_server_cli_currency_conversion_pub(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_currency_conversion_pub(sl_cli_get_argument_uint16(arguments, 0),
                                                    sl_cli_get_argument_uint8(arguments, 1),
                                                    sl_cli_get_argument_uint8(arguments, 2) );
}

// plugin price-server currency-conv set <endpoint:1> <valid:1> <providerId:4> <issuerEventId:4> <startTime:4>
//                                       <oldCurrency:2> <newCurrency:2> <conversionFactor:4>
//                                       <conversionFactorTrailingDigit:1> <currencyChangeControlFlags:4>
void sli_zigbee_af_price_server_cli_currency_conversion_set(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_currency_conversion_set(sl_cli_get_argument_uint8(arguments, 0),
                                                    sl_cli_get_argument_uint8(arguments, 1),
                                                    sl_cli_get_argument_uint32(arguments, 2),
                                                    sl_cli_get_argument_uint32(arguments, 3),
                                                    sl_cli_get_argument_uint32(arguments, 4),
                                                    sl_cli_get_argument_uint16(arguments, 5),
                                                    sl_cli_get_argument_uint16(arguments, 6),
                                                    sl_cli_get_argument_uint32(arguments, 7),
                                                    sl_cli_get_argument_uint8(arguments, 8),
                                                    sl_cli_get_argument_uint32(arguments, 9) );
}

// plugin price-server tariff-cancel set <endpoint:1> <valid:1> <providerId:4> <issuerTariffId:4> <tariffType:1>
void sli_zigbee_af_price_server_cli_tariff_cancellation_set(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_tariff_cancellation_set(sl_cli_get_argument_uint8(arguments, 0),
                                                    sl_cli_get_argument_uint8(arguments, 1),
                                                    sl_cli_get_argument_uint32(arguments, 2),
                                                    sl_cli_get_argument_uint32(arguments, 3),
                                                    sl_cli_get_argument_uint8(arguments, 4));
}

// plugin price-server tariff-cancel pub <nodeId:2> <srcEndpoint:1> <dstEndpoint:1>
void sli_zigbee_af_price_server_cli_tariff_cancellation_pub(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_price_server_tariff_cancellation_pub(sl_cli_get_argument_uint16(arguments, 0),
                                                    sl_cli_get_argument_uint8(arguments, 1),
                                                    sl_cli_get_argument_uint8(arguments, 2) );
}
