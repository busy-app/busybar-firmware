/***************************************************************************//**
 * @file
 * @brief Functions for reading and writing command and response frames.
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

#include PLATFORM_HEADER

#include <stdarg.h>
#include <stddef.h> // for NULL
#include "stack/include/sl_zigbee_types.h"
#include "ezsp-protocol.h"
#include "ezsp-frame-utilities.h"
#include "sl_zigbee_dhc.h"

uint8_t* ezspReadPointer;
uint8_t* ezspWritePointer;

static sl_zigbee_ezsp_status_t mostRecentFrameEzspStatus;

uint8_t fetchInt8u(void)
{
  return *ezspReadPointer++;
}

void fetchInt8uArray(uint8_t length, uint8_t *contents)
{
  memmove(contents, ezspReadPointer, length);
  ezspReadPointer += length;
}

uint8_t *fetchInt8uPointer(uint8_t length)
{
  uint8_t *result = ezspReadPointer;
  ezspReadPointer += length;
  return result;
}

void appendInt8u(uint8_t value)
{
  if (ezspWritePointer - ezspFrameContents < EZSP_MAX_FRAME_LENGTH) {
    *ezspWritePointer = value;
  }
  ezspWritePointer++;
}

void appendInt16u(uint16_t value)
{
  appendInt8u(LOW_BYTE(value));
  appendInt8u(HIGH_BYTE(value));
}

uint16_t fetchInt16u(void)
{
  uint8_t low = fetchInt8u();
  uint8_t high = fetchInt8u();
  return HIGH_LOW_TO_INT(high, low);
}

void appendInt32u(uint32_t value)
{
  appendInt16u((uint16_t)(value & 0xFFFF));
  appendInt16u((uint16_t)(value >> 16 & 0xFFFF));
}

uint32_t fetchInt32u(void)
{
  uint16_t low = fetchInt16u();
  return (((uint32_t) fetchInt16u()) << 16) + low;
}

void appendInt8uArray(uint8_t length, uint8_t *contents)
{
  if (ezspWritePointer - ezspFrameContents + length <= EZSP_MAX_FRAME_LENGTH) {
    memmove(ezspWritePointer, contents, length);
  }
  ezspWritePointer += length;
}

void appendInt16uArray(uint8_t length, uint16_t *contents)
{
  uint8_t i;
  for (i = 0; i < length; i++) {
    appendInt16u(contents[i]);
  }
}

void fetchInt16uArray(uint8_t length, uint16_t *contents)
{
  uint8_t i;
  for (i = 0; i < length; i++) {
    contents[i] = fetchInt16u();
  }
}

void appendInt32uArray(uint8_t length, uint32_t *contents)
{
  uint8_t i;
  for (i = 0; i < length; i++) {
    appendInt32u(contents[i]);
  }
}

void fetchInt32uArray(uint8_t length, uint32_t *contents)
{
  uint8_t i;
  for (i = 0; i < length; i++) {
    contents[i] = fetchInt32u();
  }
}

void fetch_sl_zigbee_network_parameters_t(sl_zigbee_network_parameters_t *value)
{
  fetchInt8uArray(EXTENDED_PAN_ID_SIZE, value->extendedPanId);
  value->panId = fetchInt16u();
  value->radioTxPower = fetchInt8u();
  value->radioChannel = fetchInt8u();
  value->joinMethod = fetchInt8u();
  value->nwkManagerId = fetchInt16u();
  value->nwkUpdateId = fetchInt8u();
  value->channels = fetchInt32u();
}

void fetch_sl_zigbee_multi_phy_radio_parameters_t(sl_zigbee_multi_phy_radio_parameters_t *parameters)
{
  parameters->radioTxPower = fetchInt8u();
  parameters->radioPage = fetchInt8u();
  parameters->radioChannel =  fetchInt8u();
}

void append_sl_zigbee_aps_frame_t(sl_zigbee_aps_frame_t *value)
{
  appendInt16u(value->profileId);
  appendInt16u(value->clusterId);
  appendInt8u(value->sourceEndpoint);
  appendInt8u(value->destinationEndpoint);
  appendInt16u(value->options);
  appendInt16u(value->groupId);
  appendInt8u(value->sequence);
}

void fetch_sl_zigbee_aps_frame_t(sl_zigbee_aps_frame_t *value)
{
  value->profileId = fetchInt16u();
  value->clusterId = fetchInt16u();
  value->sourceEndpoint = fetchInt8u();
  value->destinationEndpoint = fetchInt8u();
  value->options = fetchInt16u();
  value->groupId = fetchInt16u();
  value->sequence = fetchInt8u();
}

void fetch_sl_zigbee_rx_packet_info_t(sl_zigbee_rx_packet_info_t *value)
{
  value->sender_short_id = fetchInt16u();
  fetchInt8uArray(EUI64_SIZE, value->sender_long_id);
  value->binding_index = fetchInt8u();
  value->address_index = fetchInt8u();
  value->last_hop_lqi = fetchInt8u();
  value->last_hop_rssi = (int8_t)fetchInt8u();
  value->last_hop_timestamp = fetchInt32u();
}

void append_sl_zigbee_rx_packet_info_t(sl_zigbee_rx_packet_info_t *value)
{
  appendInt16u(value->sender_short_id);
  appendInt8uArray(EUI64_SIZE, value->sender_long_id);
  appendInt8u(value->binding_index);
  appendInt8u(value->address_index);
  appendInt8u(value->last_hop_lqi);
  appendInt8u(value->last_hop_rssi);
  appendInt32u(value->last_hop_timestamp);
}

void append_sl_zigbee_multicast_table_entry_t(sl_zigbee_multicast_table_entry_t *value)
{
  appendInt16u(value->multicastId);
  appendInt8u(value->endpoint);
}

void fetch_sl_zigbee_multicast_table_entry_t(sl_zigbee_multicast_table_entry_t *value)
{
  value->multicastId = fetchInt16u();
  value->endpoint    = fetchInt8u();
}

void fetch_sl_zigbee_beacon_classification_params_t(sl_zigbee_beacon_classification_params_t *value)
{
  value->minRssiForReceivingPkts  = fetchInt8u();  //there is no fetchInt8(), is it ok?
  value->beaconClassificationMask = fetchInt16u();
}

void append_sl_zigbee_beacon_classification_params_t(sl_zigbee_beacon_classification_params_t *value)
{
  appendInt8u(value->minRssiForReceivingPkts);
  appendInt16u(value->beaconClassificationMask);
}

void fetch_sl_zigbee_neighbor_table_entry_t(sl_zigbee_neighbor_table_entry_t *value)
{
  value->shortId = fetchInt16u();
  value->averageLqi = fetchInt8u();
  value->inCost = fetchInt8u();
  value->outCost = fetchInt8u();
  value->age = fetchInt8u();
  fetchInt8uArray(EUI64_SIZE, value->longId);
}

void fetch_sl_zigbee_route_table_entry_t(sl_zigbee_route_table_entry_t *value)
{
  value->destination = fetchInt16u();
  value->nextHop = fetchInt16u();
  value->status = fetchInt8u();
  value->age = fetchInt8u();
  value->concentratorType = fetchInt8u();
  value->routeRecordState = fetchInt8u();
}

void append_sl_zigbee_initial_security_state_t(sl_zigbee_initial_security_state_t *value)
{
  appendInt16u(value->bitmask);
  append_sl_zigbee_key_data_t(&(value->preconfiguredKey));
  append_sl_zigbee_key_data_t(&(value->networkKey));
  appendInt8u(value->networkKeySequenceNumber);
  appendInt8uArray(EUI64_SIZE, value->preconfiguredTrustCenterEui64);
}

void fetch_sl_zigbee_current_security_state_t(sl_zigbee_current_security_state_t *value)
{
  value->bitmask = fetchInt16u();
  fetchInt8uArray(EUI64_SIZE, value->trustCenterLongAddress);
}

void fetch_sl_zigbee_key_struct_t(sl_zigbee_key_struct_t *value)
{
  value->bitmask = fetchInt16u();
  value->type = fetchInt8u();
  if (value->bitmask & SL_ZIGBEE_KEY_HAS_PSA_ID) {
    value->psa_id = fetchInt32u();
  } else {
    fetch_sl_zigbee_key_data_t(&(value->key));
  }
  value->outgoingFrameCounter = fetchInt32u();
  value->incomingFrameCounter = fetchInt32u();
  value->sequenceNumber = fetchInt8u();
  fetchInt8uArray(EUI64_SIZE, value->partnerEUI64);
}

void fetch_sl_zigbee_child_data_t(sl_zigbee_child_data_t *value)
{
  fetchInt8uArray(EUI64_SIZE, value->eui64);
  value->type = fetchInt8u();
  value->id = fetchInt16u();
  value->phy = fetchInt8u();
  value->power = fetchInt8u();
  value->timeout = fetchInt8u();
  value->remainingTimeout = fetchInt32u();
}

void fetch_sl_zigbee_zigbee_network_t(sl_zigbee_zigbee_network_t *value)
{
  value->channel = fetchInt8u();
  value->panId = fetchInt16u();
  fetchInt8uArray(EXTENDED_PAN_ID_SIZE, value->extendedPanId);
  value->allowingJoin = fetchInt8u();
  value->stackProfile = fetchInt8u();
  value->nwkUpdateId = fetchInt8u();
}

void append_sl_zigbee_zigbee_network_t(sl_zigbee_zigbee_network_t *value)
{
  appendInt8u(value->channel);
  appendInt16u(value->panId);
  appendInt8uArray(EXTENDED_PAN_ID_SIZE, value->extendedPanId);
  appendInt8u(value->allowingJoin);
  appendInt8u(value->stackProfile);
  appendInt8u(value->nwkUpdateId);
}

void append_sl_zigbee_aes_mmo_hash_context_t(sl_zigbee_aes_mmo_hash_context_t* context)
{
  appendInt8uArray(SL_ZIGBEE_AES_HASH_BLOCK_SIZE, context->result);
  appendInt32u(context->length);
}

void fetch_sl_zigbee_aes_mmo_hash_context_t(sl_zigbee_aes_mmo_hash_context_t* context)
{
  fetchInt8uArray(SL_ZIGBEE_AES_HASH_BLOCK_SIZE, context->result);
  context->length = fetchInt32u();
}

void append_sl_zigbee_network_init_struct_t(const sl_zigbee_network_init_struct_t* networkInitStruct)
{
  appendInt16u(networkInitStruct->bitmask);
}

void fetch_sl_zigbee_network_init_struct_t(sl_zigbee_network_init_struct_t* networkInitStruct)
{
  networkInitStruct->bitmask = fetchInt16u();
}

void append_sl_zigbee_zll_network_t(sl_zigbee_zll_network_t *network)
{
  append_sl_zigbee_zigbee_network_t(&(network->zigbeeNetwork));
  append_sl_zigbee_zll_security_algorithm_data_t(&(network->securityAlgorithm));
  appendInt8uArray(EUI64_SIZE, network->eui64);
  appendInt16u(network->nodeId);
  appendInt16u(network->state);
  appendInt8u(network->nodeType);
  appendInt8u(network->numberSubDevices);
  appendInt8u(network->totalGroupIdentifiers);
  appendInt8u(network->rssiCorrection);
}

void fetch_sl_zigbee_zll_network_t(sl_zigbee_zll_network_t* network)
{
  fetch_sl_zigbee_zigbee_network_t(&(network->zigbeeNetwork));
  fetch_sl_zigbee_zll_security_algorithm_data_t(&(network->securityAlgorithm));
  fetchInt8uArray(EUI64_SIZE, network->eui64);
  network->nodeId = fetchInt16u();
  network->state = fetchInt16u();
  network->nodeType = fetchInt8u();
  network->numberSubDevices = fetchInt8u();
  network->totalGroupIdentifiers = fetchInt8u();
  network->rssiCorrection = fetchInt8u();
}

void append_sl_zigbee_zll_security_algorithm_data_t(sl_zigbee_zll_security_algorithm_data_t* data)
{
  appendInt32u(data->transactionId);
  appendInt32u(data->responseId);
  appendInt16u(data->bitmask);
}

void fetch_sl_zigbee_zll_security_algorithm_data_t(sl_zigbee_zll_security_algorithm_data_t* data)
{
  data->transactionId = fetchInt32u();
  data->responseId    = fetchInt32u();
  data->bitmask       = fetchInt16u();
}

void append_sl_zigbee_zll_initial_security_state_t(sl_zigbee_zll_initial_security_state_t* state)
{
  appendInt32u(state->bitmask);
  appendInt8u(state->keyIndex);
  append_sl_zigbee_key_data_t(&((state)->encryptionKey));
  append_sl_zigbee_key_data_t(&((state)->preconfiguredKey));
}

void append_sl_zigbee_tok_type_stack_zll_data_t(sl_zigbee_tok_type_stack_zll_data_t *data)
{
  appendInt32u(data->bitmask);
  appendInt16u(data->freeNodeIdMin);
  appendInt16u(data->freeNodeIdMax);
  appendInt16u(data->myGroupIdMin);
  appendInt16u(data->freeGroupIdMin);
  appendInt16u(data->freeGroupIdMax);
  appendInt8u(data->rssiCorrection);
}

void fetch_sl_zigbee_tok_type_stack_zll_data_t(sl_zigbee_tok_type_stack_zll_data_t *data)
{
  data->bitmask = fetchInt32u();
  data->freeNodeIdMin = fetchInt16u();
  data->freeNodeIdMax = fetchInt16u();
  data->myGroupIdMin = fetchInt16u();
  data->freeGroupIdMin = fetchInt16u();
  data->freeGroupIdMax = fetchInt16u();
  data->rssiCorrection = fetchInt8u();
}

void append_sl_zigbee_tok_type_stack_zll_security_t(sl_zigbee_tok_type_stack_zll_security_t *security)
{
  appendInt32u(security->bitmask);
  appendInt8u(security->keyIndex);
  appendInt8uArray(16, security->encryptionKey);
  appendInt8uArray(16, security->preconfiguredKey);
}

void fetch_sl_zigbee_tok_type_stack_zll_security_t(sl_zigbee_tok_type_stack_zll_security_t *security)
{
  security->bitmask = fetchInt32u();
  security->keyIndex = fetchInt8u();
  fetchInt8uArray(SL_ZIGBEE_ENCRYPTION_KEY_SIZE, security->encryptionKey);
  fetchInt8uArray(SL_ZIGBEE_ENCRYPTION_KEY_SIZE, security->preconfiguredKey);
}

void append_sl_zigbee_gp_address_t(sl_zigbee_gp_address_t *value)
{
  appendInt8u(value->applicationId);
  if (value->applicationId == 0) {
    appendInt32u(value->id.sourceId);
    appendInt32u(value->id.sourceId); //filler
  } else if (value->applicationId == 2 ) {
    appendInt8uArray(8, value->id.gpdIeeeAddress);
  }
  appendInt8u(value->endpoint);
}

void fetch_sl_zigbee_gp_address_t(sl_zigbee_gp_address_t *value)
{
  value->applicationId = fetchInt8u();
  if (value->applicationId == 0) {
    value->id.sourceId = fetchInt32u();
    value->id.sourceId = fetchInt32u();
  } else if (value->applicationId == 2) {
    fetchInt8uArray(EUI64_SIZE, value->id.gpdIeeeAddress);
  }
  value->endpoint = fetchInt8u();
}

void fetch_sl_zigbee_gp_sink_list_t(sl_zigbee_gp_sink_list_entry_t *value)
{
  for (uint8_t i = 0; i < 2; i++) {
    value->type = (sl_zigbee_gp_sink_type_t) fetchInt8u();

    switch (value->type) {
      case SL_ZIGBEE_GP_SINK_TYPE_FULL_UNICAST:
      case SL_ZIGBEE_GP_SINK_TYPE_LW_UNICAST:
      case SL_ZIGBEE_GP_SINK_TYPE_UNUSED:
      default:
        value->target.unicast.sinkNodeId = fetchInt16u();
        fetchInt8uArray(EUI64_SIZE, value->target.unicast.sinkEUI);
        break;

      case SL_ZIGBEE_GP_SINK_TYPE_D_GROUPCAST:
      case SL_ZIGBEE_GP_SINK_TYPE_GROUPCAST:
        value->target.groupcast.alias = fetchInt16u();
        value->target.groupcast.groupID = fetchInt16u();
        fetchInt16u(); //filler
        fetchInt16u();
        fetchInt16u();
        break;
    }

    value++;
  }
}

static void appendEmberGpSinkList(sl_zigbee_gp_sink_list_entry_t *value)
{
  for (uint8_t i = 0; i < 2; i++) {
    appendInt8u(value->type);
    switch (value->type) {
      case SL_ZIGBEE_GP_SINK_TYPE_FULL_UNICAST:
      case SL_ZIGBEE_GP_SINK_TYPE_LW_UNICAST:
      case SL_ZIGBEE_GP_SINK_TYPE_UNUSED:
      default:
        appendInt16u(value->target.unicast.sinkNodeId);
        appendInt8uArray(8, value->target.unicast.sinkEUI);

        break;

      case SL_ZIGBEE_GP_SINK_TYPE_D_GROUPCAST:
      case SL_ZIGBEE_GP_SINK_TYPE_GROUPCAST:
        appendInt16u(value->target.groupcast.alias);
        appendInt16u(value->target.groupcast.groupID);
        //filler
        appendInt16u(value->target.groupcast.alias);
        appendInt16u(value->target.groupcast.groupID);
        appendInt16u(value->target.groupcast.alias);
        break;
    }

    value++;
  }
}

void fetch_sl_zigbee_gp_proxy_table_entry_t(sl_zigbee_gp_proxy_table_entry_t *value)
{
  value->status = fetchInt8u();
  value->options = fetchInt32u();
  fetch_sl_zigbee_gp_address_t(&(value->gpd));
  value->assignedAlias = fetchInt16u();
  value->securityOptions  = fetchInt8u();
  value->gpdSecurityFrameCounter = fetchInt32u();
  fetch_sl_zigbee_key_data_t(&(value->gpdKey));
  fetch_sl_zigbee_gp_sink_list_t(value->sinkList);
  value->groupcastRadius = fetchInt8u();
  value->searchCounter = fetchInt8u();
}
void append_sl_zigbee_gp_proxy_table_entry_t(sl_zigbee_gp_proxy_table_entry_t *value)
{
  appendInt8u(value->status);
  appendInt32u(value->options);
  append_sl_zigbee_gp_address_t(&(value->gpd));
  appendInt16u(value->assignedAlias);
  appendInt8u(value->securityOptions);
  appendInt32u(value->gpdSecurityFrameCounter);
  append_sl_zigbee_key_data_t(&(value->gpdKey));
  appendEmberGpSinkList(value->sinkList);
  appendInt8u(value->groupcastRadius);
  appendInt8u(value->searchCounter);
}

void fetch_sl_zigbee_gp_sink_table_entry_t(sl_zigbee_gp_sink_table_entry_t *value)
{
  value->status = fetchInt8u();
  value->options = fetchInt16u();
  fetch_sl_zigbee_gp_address_t(&(value->gpd));
  value->deviceId = fetchInt8u();
  fetch_sl_zigbee_gp_sink_list_t(value->sinkList);
  value->assignedAlias = fetchInt16u();
  value->groupcastRadius = fetchInt8u();
  value->securityOptions  = fetchInt8u();
  value->gpdSecurityFrameCounter = fetchInt32u();
  fetch_sl_zigbee_key_data_t(&(value->gpdKey));
}

void append_sl_zigbee_gp_sink_table_entry_t(sl_zigbee_gp_sink_table_entry_t *value)
{
  appendInt8u(value->status);
  appendInt16u(value->options);
  append_sl_zigbee_gp_address_t(&(value->gpd));
  appendInt8u(value->deviceId);
  appendEmberGpSinkList(value->sinkList);
  appendInt16u(value->assignedAlias);
  appendInt8u(value->groupcastRadius);
  appendInt8u(value->securityOptions);
  appendInt32u(value->gpdSecurityFrameCounter);
  append_sl_zigbee_key_data_t(&(value->gpdKey));
}

void fetch_sl_zigbee_gp_params_t(sl_zigbee_gp_params_t *param)
{
  param->status = fetchInt8u();
  param->gpdLink = fetchInt8u();
  param->sequenceNumber = fetchInt8u();
  fetch_sl_zigbee_gp_address_t(&(param->addr));
  param->gpdfSecurityLevel = fetchInt8u();
  param->gpdfSecurityKeyType = fetchInt8u();
  param->autoCommissioning = fetchInt8u();
  param->bidirectionalInfo = fetchInt8u();
  param->gpdSecurityFrameCounter = fetchInt32u();
  param->gpdCommandId = fetchInt8u();
  param->mic = fetchInt32u();
  param->proxyTableIndex = fetchInt8u();
  param->gpdCommandPayloadLength = fetchInt8u();
  fetchInt8uArray(param->gpdCommandPayloadLength, param->gpdCommandPayload);
  fetch_sl_zigbee_rx_packet_info_t(&(param->packetInfo));
}

void append_sl_zigbee_duty_cycle_limits_t(sl_zigbee_duty_cycle_limits_t* limits)
{
  appendInt16u(limits->limitThresh);
  appendInt16u(limits->critThresh);
  appendInt16u(limits->suspLimit);
}

void fetch_sl_zigbee_duty_cycle_limits_t(sl_zigbee_duty_cycle_limits_t* limits)
{
  limits->limitThresh = fetchInt16u();
  limits->critThresh = fetchInt16u();
  limits->suspLimit = fetchInt16u();
}

void append_sl_zigbee_per_device_duty_cycle_t(uint8_t maxDevices,
                                              sl_zigbee_per_device_duty_cycle_t* arrayOfDeviceDutyCycles)
{
  uint8_t i;

  appendInt16u(maxDevices);
  for (i = 0; i < maxDevices; i++) {
    appendInt16u(arrayOfDeviceDutyCycles[i].nodeId);
    appendInt16u(arrayOfDeviceDutyCycles[i].dutyCycleConsumed);
  }
}

void fetch_sl_zigbee_per_device_duty_cycle_t(sl_zigbee_per_device_duty_cycle_t* arrayOfDeviceDutyCycles)
{
  uint8_t i;
  uint8_t maxDevices;

  maxDevices = fetchInt8u();
  for (i = 0; i < maxDevices; i++) {
    arrayOfDeviceDutyCycles[i].nodeId = fetchInt16u();
    arrayOfDeviceDutyCycles[i].dutyCycleConsumed = (sl_zigbee_duty_cycle_hecto_pct_t) fetchInt16u();
  }
}

void format_bytes_in_to_struct_sl_zigbee_per_device_duty_cycle_t(uint8_t* perDeviceDutyCycles,
                                                                 sl_zigbee_per_device_duty_cycle_t* arrayOfDeviceDutyCycles)
{
  uint8_t i;
  uint8_t maxDevices;

  maxDevices = perDeviceDutyCycles[0];
  perDeviceDutyCycles++;

  for (i = 0; i < maxDevices; i++) {
    arrayOfDeviceDutyCycles[i].nodeId = HIGH_LOW_TO_INT(perDeviceDutyCycles[1],
                                                        perDeviceDutyCycles[0]);
    arrayOfDeviceDutyCycles[i].dutyCycleConsumed = HIGH_LOW_TO_INT(perDeviceDutyCycles[3],
                                                                   perDeviceDutyCycles[2]);
    perDeviceDutyCycles = perDeviceDutyCycles + 4;
  }
}

void format_struct_sl_zigbee_per_device_duty_cycle_into_bytes_t(uint8_t maxDevices,
                                                                sl_zigbee_per_device_duty_cycle_t *arrayOfDeviceDutyCycles,
                                                                uint8_t* perDeviceDutyCycles)
{
  uint8_t i;

  perDeviceDutyCycles[0] = maxDevices;
  perDeviceDutyCycles++;

  for (i = 0; i < maxDevices; i++) {
    perDeviceDutyCycles[0]  = LOW_BYTE(arrayOfDeviceDutyCycles[i].nodeId);
    perDeviceDutyCycles[1]  = HIGH_BYTE(arrayOfDeviceDutyCycles[i].nodeId);
    perDeviceDutyCycles[2]  = LOW_BYTE(arrayOfDeviceDutyCycles[i].dutyCycleConsumed);
    perDeviceDutyCycles[3]  = HIGH_BYTE(arrayOfDeviceDutyCycles[i].dutyCycleConsumed);

    perDeviceDutyCycles = perDeviceDutyCycles + 4;
  }
}

void sli_zigbee_af_fetch_or_append(bool fetch, const char * format, ...)
{
  char *c = (char *)format;
  uintptr_t ptr;
  va_list args;
  va_start(args, format);

  while (*c) {
    ptr = va_arg(args, uintptr_t);
    switch (*c) {
      case 's': {
        int8_t *realPointer = (int8_t *)ptr;
        if (fetch) {
          *realPointer = (int8_t)fetchInt8u();
        } else {
          appendInt8u((uint8_t)*realPointer);
        }
      }
      break;
      case 'u': {
        uint8_t *realPointer = (uint8_t *)ptr;
        if (fetch) {
          *realPointer = fetchInt8u();
        } else {
          appendInt8u(*realPointer);
        }
      }
      break;
      case 'v': {
        uint16_t *realPointer = (uint16_t *)ptr;
        if (fetch) {
          *realPointer = fetchInt16u();
        } else {
          appendInt16u(*realPointer);
        }
      }
      break;
      case 'w': {
        uint32_t *realPointer = (uint32_t *)ptr;
        if (fetch) {
          *realPointer = fetchInt32u();
        } else {
          appendInt32u((uint32_t)*realPointer);
        }
      }
      break;
      case 'b': {
        uint8_t *realArray = (uint8_t *)ptr;
        uint8_t *lengthPointer = (uint8_t *)va_arg(args, uintptr_t);
        if (fetch) {
          fetchInt8uArray(*lengthPointer, realArray);
        } else {
          appendInt8uArray(*lengthPointer, realArray);
        }
      }
      break;
      case 'p': {
        uint8_t **realPointer = (uint8_t **)ptr;
        uint8_t *lengthPointer = (uint8_t *)va_arg(args, uintptr_t);
        if (fetch) {
          *realPointer = fetchInt8uPointer(*lengthPointer);
        } else {
          ; // cannot append a pointer
        }
      }
      break;
      default:
        ; // meh
    }
    c++;
  }

  va_end(args);
}

void append_sl_zigbee_token_data_t(sl_zigbee_token_data_t * tokenData)
{
  appendInt32u(tokenData->size);
  appendInt8uArray((uint8_t)tokenData->size, (uint8_t *)tokenData->data);
}

void fetch_sl_zigbee_token_data_t(sl_zigbee_token_data_t * tokenData)
{
  tokenData->size = fetchInt32u();
  fetchInt8uArray((uint8_t)tokenData->size, (uint8_t *)tokenData->data);
}

void fetch_sl_zigbee_token_info_t(sl_zigbee_token_info_t *tokenInfo)
{
  tokenInfo->nvm3Key = fetchInt32u();
  tokenInfo->isCnt = fetchInt8u();
  tokenInfo->isIdx = fetchInt8u();
  tokenInfo->size = fetchInt32u();
  tokenInfo->arraySize = fetchInt8u();
}

void append_sl_zigbee_token_info_t(sl_zigbee_token_info_t *tokenInfo)
{
  appendInt32u(tokenInfo->nvm3Key);
  appendInt8u(tokenInfo->isCnt);
  appendInt8u(tokenInfo->isIdx);
  appendInt32u(tokenInfo->size);
  appendInt8u(tokenInfo->arraySize);
}

void append_sl_zigbee_sec_man_context_t(sl_zigbee_sec_man_context_t *context)
{
  appendInt8u(context->core_key_type);
  appendInt8u(context->key_index);
  appendInt16u(context->derived_type);
  appendInt8uArray(EUI64_SIZE, context->eui64);
  appendInt8u(context->multi_network_index);
  appendInt8u(context->flags);
  appendInt32u(context->psa_key_alg_permission);
}

void fetch_sl_zigbee_sec_man_context_t(sl_zigbee_sec_man_context_t *context)
{
  context->core_key_type = fetchInt8u();
  context->key_index = fetchInt8u();
  context->derived_type = fetchInt16u();
  fetchInt8uArray(EUI64_SIZE, context->eui64);
  context->multi_network_index = fetchInt8u();
  context->flags = fetchInt8u();
  context->psa_key_alg_permission = fetchInt32u();
}

void append_sl_zigbee_sec_man_aps_key_metadata_t(sl_zigbee_sec_man_aps_key_metadata_t *info)
{
  appendInt16u(info->bitmask);
  appendInt32u(info->outgoing_frame_counter);
  appendInt32u(info->incoming_frame_counter);
  appendInt16u(info->ttl_in_seconds);
}

void fetch_sl_zigbee_sec_man_aps_key_metadata_t(sl_zigbee_sec_man_aps_key_metadata_t *info)
{
  info->bitmask = fetchInt16u();
  info->outgoing_frame_counter = fetchInt32u();
  info->incoming_frame_counter = fetchInt32u();
  info->ttl_in_seconds = fetchInt16u();
}

void append_sl_zigbee_dhc_pa_metadata_t(sl_zigbee_dhc_pa_metadata_t *metadata)
{
  appendInt8u(metadata->version);
  appendInt8u(metadata->num_descriptors);
  appendInt16u(metadata->pa_voltage);
  appendInt32u(metadata->signature);
}

void fetch_sl_zigbee_dhc_pa_metadata_t(sl_zigbee_dhc_pa_metadata_t *metadata)
{
  metadata->version = fetchInt8u();
  metadata->num_descriptors = fetchInt8u();
  metadata->pa_voltage = fetchInt16u();
  metadata->signature = fetchInt32u();
}

void append_sl_zigbee_dhc_pa_descriptor_t(sl_zigbee_dhc_pa_descriptor_t *descriptor)
{
  appendInt8u(descriptor->algorithm);
  appendInt8u(descriptor->num_segments_or_entries);
  appendInt16u((uint16_t)descriptor->min_ddbm);
  appendInt16u((uint16_t)descriptor->max_ddbm);
}

void fetch_sl_zigbee_dhc_pa_descriptor_t(sl_zigbee_dhc_pa_descriptor_t *descriptor)
{
  descriptor->algorithm = fetchInt8u();
  descriptor->num_segments_or_entries = fetchInt8u();
  descriptor->min_ddbm = (int16_t)fetchInt16u();
  descriptor->max_ddbm = (int16_t)fetchInt16u();
}

void append_sl_zigbee_dhc_pa_curve_segment_t(sl_zigbee_dhc_pa_curve_segment_t *segment)
{
  appendInt8u(segment->maxPowerLevel);
  appendInt32u((uint32_t)segment->slope);
  appendInt32u((uint32_t)segment->intercept);
}
void fetch_sl_zigbee_dhc_pa_curve_segment_t(sl_zigbee_dhc_pa_curve_segment_t *segment)
{
  segment->maxPowerLevel = fetchInt8u();
  segment->slope = (int32_t)fetchInt32u();
  segment->intercept = (int32_t)fetchInt32u();
}

void append_sl_zigbee_dhc_pa_curve_t(sl_zigbee_dhc_pa_curve_t *curve)
{
  appendInt16u((uint16_t)curve->curve_min_ddbm);
  appendInt16u((uint16_t)curve->curve_max_ddbm);
  for (uint8_t i = 0; i < SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT; i++) {
    append_sl_zigbee_dhc_pa_curve_segment_t(&curve->segments[i]);
  }
}

void fetch_sl_zigbee_dhc_pa_curve_t(sl_zigbee_dhc_pa_curve_t *curve)
{
  curve->curve_min_ddbm = (int16_t)fetchInt16u();
  curve->curve_max_ddbm = (int16_t)fetchInt16u();
  for (uint8_t i = 0; i < SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT; i++) {
    fetch_sl_zigbee_dhc_pa_curve_segment_t(&curve->segments[i]);
  }
}

void append_sl_zigbee_dhc_pa_table_t(sl_zigbee_dhc_pa_table_t *table)
{
  for (uint8_t i = 0; i < SL_ZIGBEE_DHC_TABLE_ENTRY_COUNT; i++) {
    appendInt16u((uint16_t)table->ddbm_values[i]);
  }
}

void fetch_sl_zigbee_dhc_pa_table_t(sl_zigbee_dhc_pa_table_t *table)
{
  for (uint8_t i = 0; i < SL_ZIGBEE_DHC_TABLE_ENTRY_COUNT; i++) {
    table->ddbm_values[i] = (int16_t)fetchInt16u();
  }
}

void append_sl_zigbee_dhc_pa_version_t(sl_zigbee_dhc_pa_version_t *pa_version)
{
  appendInt8u(pa_version->pa_version);
}

void fetch_sl_zigbee_dhc_pa_version_t(sl_zigbee_dhc_pa_version_t *pa_version)
{
  pa_version->pa_version = fetchInt8u();
}

void append_sl_zigbee_dhc_pa_signature_t(sl_zigbee_dhc_pa_signature_t *pa_signature)
{
  appendInt32u(pa_signature->pa_signature);
}

void fetch_sl_zigbee_dhc_pa_signature_t(sl_zigbee_dhc_pa_signature_t *pa_signature)
{
  pa_signature->pa_signature = fetchInt32u();
}

void append_sl_zigbee_dhc_pa_mode_t(sl_zigbee_dhc_pa_mode_t *pa_mode)
{
  appendInt8u(pa_mode->pa_mode);
}

void fetch_sl_zigbee_dhc_pa_mode_t(sl_zigbee_dhc_pa_mode_t *pa_mode)
{
  pa_mode->pa_mode = fetchInt8u();
}

void append_sl_zigbee_dhc_rssi_offset_t(sl_zigbee_dhc_rssi_offset_t *rssi_offset)
{
  appendInt8u((uint8_t)rssi_offset->rssi_offset);
}

void fetch_sl_zigbee_dhc_rssi_offset_t(sl_zigbee_dhc_rssi_offset_t *rssi_offset)
{
  rssi_offset->rssi_offset = (int8_t)fetchInt8u();
}

void append_sl_zigbee_dhc_ctune_t(sl_zigbee_dhc_ctune_t *ctune)
{
  appendInt32u(ctune->ctune);
}

void fetch_sl_zigbee_dhc_ctune_t(sl_zigbee_dhc_ctune_t *ctune)
{
  ctune->ctune = fetchInt32u();
}

void sli_zigbee_ezsp_set_last_status(sl_zigbee_ezsp_status_t status)
{
  mostRecentFrameEzspStatus = status;
}

sl_zigbee_ezsp_status_t sl_zigbee_ezsp_get_last_status()
{
  return mostRecentFrameEzspStatus;
}
