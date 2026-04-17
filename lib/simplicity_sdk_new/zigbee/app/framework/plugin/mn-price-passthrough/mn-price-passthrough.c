/***************************************************************************//**
 * @file
 * @brief Routines for the MN Price Passthrough plugin, which handles
 *        demonstrates client and server Price cluster passthrough on a multi-
 *        network device.
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
#include "mn-price-passthrough.h"
#include "app/framework/plugin/price-common/price-common.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "mn-price-passthrough-config.h"
#include "zap-cluster-command-parser.h"
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
#include "test-harness.h"
#endif

#define VALID  BIT(1)

static sl_802154_short_addr_t forwardingId;
static uint8_t forwardingEndpoint, proxyEsiEndpoint;
static sl_zigbee_af_scheduled_price_t currentPrice;

static sl_zigbee_af_event_t pollAndForwardEsiEvent;
static void pollAndForwardEsiEventHandler(sl_zigbee_af_event_t * event);

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

static void pollAndForwardEsiEventHandler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sl_status_t status;

  // Ensure that the endpoint for the proxy ESI has been set
  if (forwardingId == SL_ZIGBEE_NULL_NODE_ID
      || forwardingEndpoint == 0xFF
      || proxyEsiEndpoint == 0xFF) {
    sl_zigbee_af_price_cluster_println("Routing parameters not properly established: node %02X forwarding %02X proxy %02X",
                                       forwardingId,
                                       forwardingEndpoint,
                                       proxyEsiEndpoint);
    goto reschedule;
  }

  // Poll the real ESI
  sl_zigbee_af_fill_command_price_cluster_get_current_price(0x00);
  sl_zigbee_af_set_command_endpoints(forwardingEndpoint, proxyEsiEndpoint);
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, forwardingId);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_price_cluster_println("Error in poll and forward event handler %02X", status);
  }

  reschedule:
  // Reschedule the event
  sl_zigbee_af_event_set_delay_minutes(&pollAndForwardEsiEvent,
                                       SL_ZIGBEE_AF_PLUGIN_MN_PRICE_PASSTHROUGH_POLL_RATE);
}

void sl_zigbee_af_mn_price_passthrough_init_cb(uint8_t init_level)
{
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_EVENT:
    {
      sl_zigbee_af_event_init(&pollAndForwardEsiEvent,
                              pollAndForwardEsiEventHandler);
      break;
    }

    case SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA:
    {
      // Initialize the proxy ESI endpoint
      sli_zigbee_af_mn_price_passthrough_routing_setup(SL_ZIGBEE_NULL_NODE_ID, 0xFF, 0xFF);
      break;
    }
  }
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_price_cluster_publish_price_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_price_cluster_publish_price_command_t cmd_data;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint,
                                                               ZCL_PRICE_CLUSTER_ID);
  if ((zcl_decode_price_cluster_publish_price_command(cmd, &cmd_data)
       != SL_ZIGBEE_ZCL_STATUS_SUCCESS)
      || ep == 0xFF) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

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

  currentPrice.providerId = cmd_data.providerId;
  sl_zigbee_af_copy_string(currentPrice.rateLabel, cmd_data.rateLabel, ZCL_PRICE_CLUSTER_MAXIMUM_RATE_LABEL_LENGTH);
  currentPrice.issuerEventID = cmd_data.issuerEventId;
  currentPrice.startTime = cmd_data.currentTime;
  currentPrice.unitOfMeasure = cmd_data.unitOfMeasure;
  currentPrice.currency = cmd_data.currency;
  currentPrice.priceTrailingDigitAndTier = cmd_data.priceTrailingDigitAndPriceTier;
  currentPrice.numberOfPriceTiersAndTier = cmd_data.numberOfPriceTiersAndRegisterTier;
  currentPrice.startTime = cmd_data.startTime;
  currentPrice.duration = cmd_data.durationInMinutes;
  currentPrice.price = cmd_data.price;
  currentPrice.priceRatio = cmd_data.priceRatio;
  currentPrice.generationPrice = cmd_data.generationPrice;
  currentPrice.generationPriceRatio = cmd_data.generationPriceRatio;
  currentPrice.alternateCostDelivered = cmd_data.alternateCostDelivered;
  currentPrice.alternateCostUnit = cmd_data.alternateCostUnit;
  currentPrice.alternateCostTrailingDigit = cmd_data.alternateCostTrailingDigit;
  currentPrice.numberOfBlockThresholds = cmd_data.numberOfBlockThresholds;
  currentPrice.priceControl = cmd_data.priceControl;
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

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
    }
  }

  return status;
}

void sli_zigbee_af_mn_price_passthrough_routing_setup(sl_802154_short_addr_t fwdId,
                                                      uint8_t fwdEndpoint,
                                                      uint8_t esiEndpoint)
{
  forwardingId = fwdId;
  forwardingEndpoint = fwdEndpoint;
  proxyEsiEndpoint = esiEndpoint;
}

void sli_zigbee_af_mn_price_passthrough_start_poll_and_forward(void)
{
  sl_zigbee_af_price_cluster_println("Starting %s", "poll and forward");
  sl_zigbee_af_event_set_delay_ms(&pollAndForwardEsiEvent,
                                  MILLISECOND_TICKS_PER_SECOND);
}

void sli_zigbee_af_mn_price_passthrough_stop_poll_and_forward(void)
{
  sl_zigbee_af_price_cluster_println("Stopping %s", "poll and forward");
  sl_zigbee_af_event_set_inactive(&pollAndForwardEsiEvent);
}

bool sl_zigbee_af_price_cluster_get_current_price_cb(uint8_t commandOptions)
{
  sl_zigbee_af_price_cluster_println("RX: GetCurrentPrice 0x%02X", commandOptions);
  if (currentPrice.priceControl & VALID) {
    sl_zigbee_af_price_cluster_println("no valid price to return!");
    sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_NOT_FOUND);
  } else {
    fillPublishPriceCommand(currentPrice);
    sl_zigbee_af_send_response();
  }
  return true;
}

void sli_zigbee_af_mn_price_passthrough_print_current_price(void)
{
  sl_zigbee_af_price_cluster_println("Current configured price:");
  sl_zigbee_af_price_cluster_println("  Note: ALL values given in HEX\r\n");

  sl_zigbee_af_price_cluster_println("= CURRENT PRICE =");
  sl_zigbee_af_price_cluster_print("  label: ");
  sl_zigbee_af_price_cluster_print_string(currentPrice.rateLabel);

  sl_zigbee_af_price_cluster_print("(%02X)\r\n  uom/cur: 0x%02X/0x%04X"
                                   "\r\n  pid/eid: 0x%08X/0x%08X"
                                   "\r\n  ct/st/dur: 0x%08X/0x%08X/",
                                   sl_zigbee_af_string_length(currentPrice.rateLabel),
                                   currentPrice.unitOfMeasure,
                                   currentPrice.currency,
                                   currentPrice.providerId,
                                   currentPrice.issuerEventID,
                                   sl_zigbee_af_get_current_time(),
                                   currentPrice.startTime);
  if (currentPrice.duration == ZCL_PRICE_CLUSTER_DURATION_UNTIL_CHANGED) {
    sl_zigbee_af_price_cluster_print("INF");
  } else {
    sl_zigbee_af_price_cluster_print("0x%04X", currentPrice.duration);
  }
  sl_zigbee_af_price_cluster_println("\r\n  ptdt/ptrt: 0x%02X/0x%02X"
                                     "\r\n  p/pr: 0x%08X/0x%02X"
                                     "\r\n  gp/gpr: 0x%08X/0x%02X"
                                     "\r\n  acd/acu/actd: 0x%08X/0x%02X/0x%02X",
                                     currentPrice.priceTrailingDigitAndTier,
                                     currentPrice.numberOfPriceTiersAndTier,
                                     currentPrice.price,
                                     currentPrice.priceRatio,
                                     currentPrice.generationPrice,
                                     currentPrice.generationPriceRatio,
                                     currentPrice.alternateCostDelivered,
                                     currentPrice.alternateCostUnit,
                                     currentPrice.alternateCostTrailingDigit);
  sl_zigbee_af_price_cluster_println("  nobt: 0x%02X", currentPrice.numberOfBlockThresholds);
  sl_zigbee_af_price_cluster_println("  pc: 0x%02X",
                                     (currentPrice.priceControl
                                      & ZCL_PRICE_CLUSTER_RESERVED_MASK));
  sl_zigbee_af_price_cluster_print("  price is valid from time 0x%08X until ",
                                   currentPrice.startTime);
  if (currentPrice.duration == ZCL_PRICE_CLUSTER_DURATION_UNTIL_CHANGED) {
    sl_zigbee_af_price_cluster_println("eternity");
  } else {
    sl_zigbee_af_price_cluster_println("0x%08X",
                                       (currentPrice.startTime + (currentPrice.duration * 60)));
  }
}
