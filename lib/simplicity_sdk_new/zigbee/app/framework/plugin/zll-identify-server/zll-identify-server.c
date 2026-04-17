/***************************************************************************//**
 * @file
 * @brief Routines for the ZLL Identify Server plugin, which contains additions
 *        to the Identify server cluster.
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

// this file contains all the common includes for clusters in the util
#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "zll-identify-server-config.h"
#include "zap-cluster-command-parser.h"
#include "sl_component_catalog.h"
#ifdef SL_CATALOG_SIMPLE_LED_PRESENT
#include "sl_simple_led_instances.h"
#endif // SL_CATALOG_SIMPLE_LED_PRESENT

#ifndef EZSP_HOST
#include "hal/hal.h"
#endif

typedef struct {
  bool active;
  bool cancel;
  sl_zigbee_af_identify_effect_identifier_t effectId;
  sl_zigbee_af_identify_effect_variant_t commandVariant;
  uint8_t eventsRemaining;
  uint16_t eventDelay;
} sli_zigbee_af_zll_identify_state;

void sli_zigbee_af_zll_identify_server_blink_effect(uint8_t endpoint);

void sli_zigbee_af_zll_identify_server_breathe_effect(uint8_t endpoint);

void sli_zigbee_af_zll_identify_server_okay_effect(uint8_t endpoint);

void sli_zigbee_af_zll_identify_server_channel_change_effect(uint8_t endpoint);

static sl_zigbee_af_event_t triggerEffectEndpointEvents[ZCL_FIXED_ENDPOINT_COUNT];

static sli_zigbee_af_zll_identify_state stateTable[SL_ZIGBEE_ZCL_IDENTIFY_CLUSTER_SERVER_ENDPOINT_COUNT];

static sli_zigbee_af_zll_identify_state *getZllIdentifyState(uint8_t endpoint);

static void deactivateZllIdentify(sli_zigbee_af_zll_identify_state *state, uint8_t endpoint);

static sli_zigbee_af_zll_identify_state *getZllIdentifyState(uint8_t endpoint)
{
  uint8_t index = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_IDENTIFY_CLUSTER_ID);
  return (index == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX ? NULL : &stateTable[index]);
}

static void deactivateZllIdentify(sli_zigbee_af_zll_identify_state *state, uint8_t endpoint)
{
  if (state == NULL) {
    return;
  }

  state->active = false;
  state->cancel = false;

  sl_zigbee_af_endpoint_event_set_inactive(triggerEffectEndpointEvents, endpoint);
}

static void triggerEffectEndpointEventHandler(uint8_t endpoint)
{
  sli_zigbee_af_zll_identify_state *state = getZllIdentifyState(endpoint);

  if (state == NULL) {
    return;
  }

  switch (state->effectId) {
    case SL_ZIGBEE_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK:
      sli_zigbee_af_zll_identify_server_blink_effect(endpoint);
      break;
    case SL_ZIGBEE_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE:
      sli_zigbee_af_zll_identify_server_breathe_effect(endpoint);
      break;
    case SL_ZIGBEE_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY:
      sli_zigbee_af_zll_identify_server_okay_effect(endpoint);
      break;
    case SL_ZIGBEE_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE:
      sli_zigbee_af_zll_identify_server_channel_change_effect(endpoint);
      break;
    case SL_ZIGBEE_ZCL_IDENTIFY_EFFECT_IDENTIFIER_FINISH_EFFECT: // At this point, these are functionally equivalent
    case SL_ZIGBEE_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT:
    default:
      deactivateZllIdentify(state, endpoint);
      return;
  }
  if (state->cancel) {
    deactivateZllIdentify(state, endpoint);
    return;
  }

  if (state->active) {
    sl_zigbee_af_endpoint_event_set_delay_ms(triggerEffectEndpointEvents,
                                             endpoint,
                                             state->eventDelay);
  }
}

void sli_zigbee_af_zll_identify_server_init_callback(uint8_t init_level)
{
  (void)init_level;

  uint8_t endpoint_array[ZCL_FIXED_ENDPOINT_COUNT] = ZCL_FIXED_ENDPOINT_ARRAY;
  uint8_t i;

  for (i = 0; i < ZCL_FIXED_ENDPOINT_COUNT; i++) {
    sl_zigbee_af_endpoint_event_init(&triggerEffectEndpointEvents[i],
                                     triggerEffectEndpointEventHandler,
                                     endpoint_array[i]);
  }
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_identify_cluster_trigger_effect_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_identify_cluster_trigger_effect_command_t cmd_data;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  sli_zigbee_af_zll_identify_state *state = getZllIdentifyState(endpoint);

  sl_zigbee_af_status_t status = zcl_decode_identify_cluster_trigger_effect_command(cmd, &cmd_data);

  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    goto default_response;
  }

  if (state == NULL) {
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
    goto default_response;
  }

  sl_zigbee_af_identify_cluster_println("RX identify:trigger effect 0x%02X variant 0x%02X", cmd_data.effectId, cmd_data.effectVariant);

  if (state->active) {
    switch (state->effectId) {
      case SL_ZIGBEE_ZCL_IDENTIFY_EFFECT_IDENTIFIER_FINISH_EFFECT:
        state->cancel = true;
        status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
        break;
      case SL_ZIGBEE_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT:
        deactivateZllIdentify(state, endpoint);
        status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
        goto default_response;
      default:
        status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
        break;
    }
  } else {
    switch (cmd_data.effectId) {
      case SL_ZIGBEE_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK:
        state->eventsRemaining = SL_ZIGBEE_AF_PLUGIN_ZLL_IDENTIFY_SERVER_BLINK_EVENTS;
        break;
      case SL_ZIGBEE_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE:
        state->eventsRemaining = SL_ZIGBEE_AF_PLUGIN_ZLL_IDENTIFY_SERVER_BREATHE_EVENTS;
        break;
      case SL_ZIGBEE_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY:
        state->eventsRemaining = SL_ZIGBEE_AF_PLUGIN_ZLL_IDENTIFY_SERVER_OKAY_EVENTS;
        break;
      case SL_ZIGBEE_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE:
        state->eventsRemaining = SL_ZIGBEE_AF_PLUGIN_ZLL_IDENTIFY_SERVER_CHANNEL_CHANGE_EVENTS;
        break;
      case SL_ZIGBEE_ZCL_IDENTIFY_EFFECT_IDENTIFIER_FINISH_EFFECT: // At this point, these are functionally equivalent
      case SL_ZIGBEE_ZCL_IDENTIFY_EFFECT_IDENTIFIER_STOP_EFFECT:
        status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
        goto default_response;
      default:
        status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
        goto default_response;
    }
    state->active = true;
    state->cancel = false;
    state->effectId = (sl_zigbee_af_identify_effect_identifier_t)cmd_data.effectId;
    state->commandVariant = (sl_zigbee_af_identify_effect_variant_t)cmd_data.effectVariant;
    state->eventDelay = SL_ZIGBEE_AF_PLUGIN_ZLL_IDENTIFY_SERVER_EVENT_DELAY;
    sl_zigbee_af_endpoint_event_set_delay_ms(triggerEffectEndpointEvents,
                                             endpoint,
                                             state->eventDelay);
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  default_response:
  return status;
}

void sli_zigbee_af_zll_identify_server_blink_effect(uint8_t endpoint)
{
  sli_zigbee_af_zll_identify_state *state = getZllIdentifyState(endpoint);

  if (state == NULL || state->eventsRemaining == 0) {
    deactivateZllIdentify(state, endpoint);
    return;
  }

#ifndef EZSP_HOST
#ifdef SL_CATALOG_SIMPLE_LED_LED0_PRESENT
  sl_led_toggle(&sl_led_led0);
#endif // SL_CATALOG_SIMPLE_LED_LED0_PRESENT
#ifdef SL_CATALOG_SIMPLE_LED_LED1_PRESENT
  sl_led_toggle(&sl_led_led1);
#endif // SL_CATALOG_SIMPLE_LED_LED1_PRESENT
#ifdef SL_CATALOG_SIMPLE_LED_LED2_PRESENT
  sl_led_toggle(&sl_led_led2);
#endif // SL_CATALOG_SIMPLE_LED_LED2_PRESENT
#ifdef SL_CATALOG_SIMPLE_LED_LED3_PRESENT
  sl_led_toggle(&sl_led_led3);
#endif // SL_CATALOG_SIMPLE_LED_LED3_PRESENT
#endif

  state->eventsRemaining = state->eventsRemaining - 1;
}

void sli_zigbee_af_zll_identify_server_breathe_effect(uint8_t endpoint)
{
  sli_zigbee_af_zll_identify_server_blink_effect(endpoint);
}

void sli_zigbee_af_zll_identify_server_okay_effect(uint8_t endpoint)
{
  sli_zigbee_af_zll_identify_server_blink_effect(endpoint);
}

void sli_zigbee_af_zll_identify_server_channel_change_effect(uint8_t endpoint)
{
  sli_zigbee_af_zll_identify_server_blink_effect(endpoint);
}

uint32_t sl_zigbee_af_zll_identify_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                                sl_service_function_context_t *context)
{
  (void)opcode;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  if (!cmd->mfgSpecific) {
    if (cmd->commandId == ZCL_TRIGGER_EFFECT_COMMAND_ID) {
      status = sl_zigbee_af_identify_cluster_trigger_effect_cb(cmd);
    }
  }

  return status;
}
