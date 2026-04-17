/*****************************************************************************/
/**
 * Copyright 2017 Silicon Laboratories, Inc.
 *
 *****************************************************************************/
//
// *** Generated file. Do not edit! ***
//
// Description: Functions for sending every EM260 frame and returning the result
// to the Host.

// RAIL tests define COMPONENT_CATALOG, but don't actually generate zap, so we
// key off of RAIL-defined SIMULATION_DEVICE to avoid pulling in zap-type.h
#if defined(SL_COMPONENT_CATALOG_PRESENT) && !defined(SIMULATION_DEVICE)
 #include "zap-type.h"
#else // SL_COMPONENT_CATALOG_PRESENT
 #include "app/framework/test/headers/zap-type.h"
#endif // SL_COMPONENT_CATALOG_PRESENT
#include "stack/include/sl_zigbee_dhc.h"


//------------------------------------------------------------------------------
// Configuration Frames
//------------------------------------------------------------------------------

uint8_t sl_zigbee_ezsp_version(
      uint8_t desiredProtocolVersion,
      uint8_t *stackType,
      uint16_t *stackVersion)
{
  uint8_t protocolVersion;
  startCommand(SL_ZIGBEE_EZSP_VERSION);
  appendInt8u(desiredProtocolVersion);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    protocolVersion = fetchInt8u();
    *stackType = fetchInt8u();
    *stackVersion = fetchInt16u();
    return protocolVersion;
  }
  return 255;
}

sl_status_t sl_zigbee_ezsp_get_configuration_value(
      sl_zigbee_ezsp_config_id_t configId,
      uint16_t *value)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_CONFIGURATION_VALUE);
  appendInt8u(configId);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    *value = fetchInt16u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_configuration_value(
      sl_zigbee_ezsp_config_id_t configId,
      uint16_t value)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_CONFIGURATION_VALUE);
  appendInt8u(configId);
  appendInt16u(value);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_zigbee_af_status_t sl_zigbee_ezsp_read_attribute(
      uint8_t endpoint,
      uint16_t cluster,
      uint16_t attributeId,
      uint8_t mask,
      uint16_t manufacturerCode,
      uint8_t *dataType,
      uint8_t *readLength,
      uint8_t *dataPtr)
{
  sl_zigbee_af_status_t af_status;
  uint8_t maxReadLength = *readLength;
  startCommand(SL_ZIGBEE_EZSP_READ_ATTRIBUTE);
  appendInt8u(endpoint);
  appendInt16u(cluster);
  appendInt16u(attributeId);
  appendInt8u(mask);
  appendInt16u(manufacturerCode);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    af_status = fetchInt8u();
    *dataType = fetchInt8u();
    *readLength = fetchInt8u();
  if (*readLength > maxReadLength) {
    return 255;
  }
    fetchInt8uArray(*readLength, dataPtr);
    return af_status;
  }
  return 255;
}

sl_zigbee_af_status_t sl_zigbee_ezsp_write_attribute(
      uint8_t endpoint,
      uint16_t cluster,
      uint16_t attributeId,
      uint8_t mask,
      uint16_t manufacturerCode,
      bool overrideReadOnlyAndDataType,
      bool justTest,
      uint8_t dataType,
      uint8_t dataLength,
      uint8_t *data)
{
  sl_zigbee_af_status_t af_status;
  startCommand(SL_ZIGBEE_EZSP_WRITE_ATTRIBUTE);
  appendInt8u(endpoint);
  appendInt16u(cluster);
  appendInt16u(attributeId);
  appendInt8u(mask);
  appendInt16u(manufacturerCode);
  appendInt8u(overrideReadOnlyAndDataType);
  appendInt8u(justTest);
  appendInt8u(dataType);
  appendInt8u(dataLength);
  appendInt8uArray(dataLength, data);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    af_status = fetchInt8u();
    return af_status;
  }
  return 255;
}

sl_status_t sl_zigbee_ezsp_add_endpoint(
      uint8_t endpoint,
      uint16_t profileId,
      uint16_t deviceId,
      uint8_t deviceVersion,
      uint8_t inputClusterCount,
      uint8_t outputClusterCount,
      uint16_t *inputClusterList,
      uint16_t *outputClusterList)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_ADD_ENDPOINT);
  appendInt8u(endpoint);
  appendInt16u(profileId);
  appendInt16u(deviceId);
  appendInt8u(deviceVersion);
  appendInt8u(inputClusterCount);
  appendInt8u(outputClusterCount);
  appendInt16uArray(inputClusterCount, inputClusterList);
  appendInt16uArray(outputClusterCount, outputClusterList);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_policy(
      sl_zigbee_ezsp_policy_id_t policyId,
      sl_zigbee_ezsp_decision_id_t decisionId)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_POLICY);
  appendInt8u(policyId);
  appendInt8u(decisionId);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_policy(
      sl_zigbee_ezsp_policy_id_t policyId,
      sl_zigbee_ezsp_decision_id_t *decisionId)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_POLICY);
  appendInt8u(policyId);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    *decisionId = fetchInt8u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

bool sl_zigbee_ezsp_send_pan_id_update(
      sl_802154_pan_id_t newPan)
{
  bool status;
  startCommand(SL_ZIGBEE_EZSP_SEND_PAN_ID_UPDATE);
  appendInt16u(newPan);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt8u();
    return status;
  }
  return false;
}

sl_status_t sl_zigbee_ezsp_get_value(
      sl_zigbee_ezsp_value_id_t valueId,
      uint8_t *valueLength,
      uint8_t *value)
{
  sl_status_t status;
  uint8_t maxValueLength = *valueLength;
  startCommand(SL_ZIGBEE_EZSP_GET_VALUE);
  appendInt8u(valueId);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    *valueLength = fetchInt8u();
  if (*valueLength > maxValueLength) {
    return SL_STATUS_INVALID_PARAMETER;
  }
    fetchInt8uArray(*valueLength, value);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_extended_value(
      sl_zigbee_ezsp_extended_value_id_t valueId,
      uint32_t characteristics,
      uint8_t *valueLength,
      uint8_t *value)
{
  sl_status_t status;
  uint8_t maxValueLength = *valueLength;
  startCommand(SL_ZIGBEE_EZSP_GET_EXTENDED_VALUE);
  appendInt8u(valueId);
  appendInt32u(characteristics);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    *valueLength = fetchInt8u();
  if (*valueLength > maxValueLength) {
    return SL_STATUS_INVALID_PARAMETER;
  }
    fetchInt8uArray(*valueLength, value);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_value(
      sl_zigbee_ezsp_value_id_t valueId,
      uint8_t valueLength,
      uint8_t *value)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_VALUE);
  appendInt8u(valueId);
  appendInt8u(valueLength);
  appendInt8uArray(valueLength, value);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_passive_ack_config(
      uint8_t config,
      uint8_t minAcksNeeded)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_PASSIVE_ACK_CONFIG);
  appendInt8u(config);
  appendInt8u(minAcksNeeded);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

void sl_zigbee_ezsp_set_pending_network_update_pan_id(
      uint16_t panId)
{
  startCommand(SL_ZIGBEE_EZSP_SET_PENDING_NETWORK_UPDATE_PAN_ID);
  appendInt16u(panId);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

void sl_zigbee_ezsp_set_pending_network_update_channel(
      uint8_t channel)
{
  startCommand(SL_ZIGBEE_EZSP_SET_PENDING_NETWORK_UPDATE_CHANNEL);
  appendInt8u(channel);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

uint8_t sl_zigbee_ezsp_get_endpoint(
      uint8_t index)
{
  uint8_t endpoint;
  startCommand(SL_ZIGBEE_EZSP_GET_ENDPOINT);
  appendInt8u(index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    endpoint = fetchInt8u();
    return endpoint;
  }
  return 255;
}

uint8_t sl_zigbee_ezsp_get_endpoint_count(void)
{
  uint8_t count;
  startCommand(SL_ZIGBEE_EZSP_GET_ENDPOINT_COUNT);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    count = fetchInt8u();
    return count;
  }
  return 255;
}

bool sl_zigbee_ezsp_get_endpoint_description(
      uint8_t endpoint,
      sl_zigbee_endpoint_description_t *result)
{
  bool success;
  startCommand(SL_ZIGBEE_EZSP_GET_ENDPOINT_DESCRIPTION);
  appendInt8u(endpoint);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    success = fetchInt8u();
    fetch_sl_zigbee_endpoint_description_t(result);
    return success;
  }
  return false;
}

uint16_t sl_zigbee_ezsp_get_endpoint_cluster(
      uint8_t endpoint,
      uint8_t listId,
      uint8_t listIndex)
{
  uint16_t endpoint_cluster;
  startCommand(SL_ZIGBEE_EZSP_GET_ENDPOINT_CLUSTER);
  appendInt8u(endpoint);
  appendInt8u(listId);
  appendInt8u(listIndex);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    endpoint_cluster = fetchInt16u();
    return endpoint_cluster;
  }
  return 255;
}

//------------------------------------------------------------------------------
// Utilities Frames
//------------------------------------------------------------------------------

void sl_zigbee_ezsp_nop(void)
{
  startCommand(SL_ZIGBEE_EZSP_NOP);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

uint8_t sl_zigbee_ezsp_echo(
      uint8_t dataLength,
      uint8_t *data,
      uint8_t *echo)
{
  uint8_t echoLength;
  startCommand(SL_ZIGBEE_EZSP_ECHO);
  appendInt8u(dataLength);
  appendInt8uArray(dataLength, data);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    echoLength = fetchInt8u();
    if (echoLength > dataLength) {
      return 0;
    }
    fetchInt8uArray(echoLength, echo);
    return echoLength;
  }
  return 0;
}

void sl_zigbee_ezsp_callback(void)
{
  startCommand(SL_ZIGBEE_EZSP_CALLBACK);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    callbackDispatch();
  }
}

sl_status_t sl_zigbee_ezsp_set_token(
      uint8_t tokenId,
      uint8_t *tokenData)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_TOKEN);
  appendInt8u(tokenId);
  appendInt8uArray(8, tokenData);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_token(
      uint8_t tokenId,
      uint8_t *tokenData)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_TOKEN);
  appendInt8u(tokenId);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetchInt8uArray(8, tokenData);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

uint8_t sl_zigbee_ezsp_get_mfg_token(
      sl_zigbee_ezsp_mfg_token_id_t tokenId,
      uint8_t *tokenData)
{
  uint8_t tokenDataLength;
  startCommand(SL_ZIGBEE_EZSP_GET_MFG_TOKEN);
  appendInt8u(tokenId);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    tokenDataLength = fetchInt8u();
    uint8_t expectedTokenDataLength = 0;
    // the size of corresponding the EZSP Mfg token,
    // please refer to app/util/ezsp/ezsp-enum.h
    switch (tokenId) {
      // 2 bytes
      case SL_ZIGBEE_EZSP_MFG_CUSTOM_VERSION:
      case SL_ZIGBEE_EZSP_MFG_MANUF_ID:
      case SL_ZIGBEE_EZSP_MFG_PHY_CONFIG:
      case SL_ZIGBEE_EZSP_MFG_CTUNE:
        expectedTokenDataLength = 2;
        break;
      // 8 bytes
      case SL_ZIGBEE_EZSP_MFG_SL_ZIGBEE_EZSP_STORAGE:
      case SL_ZIGBEE_EZSP_MFG_CUSTOM_EUI_64:
        expectedTokenDataLength = 8;
        break;
      // 16 bytes
      case SL_ZIGBEE_EZSP_MFG_STRING:
      case SL_ZIGBEE_EZSP_MFG_BOARD_NAME:
      case SL_ZIGBEE_EZSP_MFG_BOOTLOAD_AES_KEY:
        expectedTokenDataLength = 16;
        break;
      // 20 bytes
      case SL_ZIGBEE_EZSP_MFG_INSTALLATION_CODE:
        expectedTokenDataLength = 20;
        break;
      // 40 bytes
      case SL_ZIGBEE_EZSP_MFG_ASH_CONFIG:
        expectedTokenDataLength = 40;
        break;
      // 92 bytes
      case SL_ZIGBEE_EZSP_MFG_CBKE_DATA:
        expectedTokenDataLength = 92;
        break;
      default:
        break;
    }
    if (tokenDataLength != expectedTokenDataLength) {
      return 255;
    }
    fetchInt8uArray(tokenDataLength, tokenData);
    return tokenDataLength;
  }
  return 255;
}

sl_status_t sl_zigbee_ezsp_set_mfg_token(
      sl_zigbee_ezsp_mfg_token_id_t tokenId,
      uint8_t tokenDataLength,
      uint8_t *tokenData)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_MFG_TOKEN);
  appendInt8u(tokenId);
  appendInt8u(tokenDataLength);
  appendInt8uArray(tokenDataLength, tokenData);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_random_number(
      uint16_t *value)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_RANDOM_NUMBER);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    *value = fetchInt16u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_timer(
      uint8_t timerId,
      uint16_t time,
      sl_zigbee_event_units_t units,
      bool repeat)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_TIMER);
  appendInt8u(timerId);
  appendInt16u(time);
  appendInt8u(units);
  appendInt8u(repeat);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

uint16_t sl_zigbee_ezsp_get_timer(
      uint8_t timerId,
      sl_zigbee_event_units_t *units,
      bool *repeat)
{
  uint16_t time;
  startCommand(SL_ZIGBEE_EZSP_GET_TIMER);
  appendInt8u(timerId);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    time = fetchInt16u();
    *units = fetchInt8u();
    *repeat = fetchInt8u();
    return time;
  }
  return 255;
}

sl_status_t sl_zigbee_ezsp_debug_write(
      bool binaryMessage,
      uint8_t messageLength,
      uint8_t *messageContents)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_DEBUG_WRITE);
  appendInt8u(binaryMessage);
  appendInt8u(messageLength);
  appendInt8uArray(messageLength, messageContents);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

void sl_zigbee_ezsp_read_and_clear_counters(
      uint16_t *values)
{
  startCommand(SL_ZIGBEE_EZSP_READ_AND_CLEAR_COUNTERS);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    fetchInt16uArray(SL_ZIGBEE_COUNTER_TYPE_COUNT, values);
  }
}

void sl_zigbee_ezsp_read_counters(
      uint16_t *values)
{
  startCommand(SL_ZIGBEE_EZSP_READ_COUNTERS);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    fetchInt16uArray(SL_ZIGBEE_COUNTER_TYPE_COUNT, values);
  }
}

void sl_zigbee_ezsp_delay_test(
      uint16_t delay)
{
  startCommand(SL_ZIGBEE_EZSP_DELAY_TEST);
  appendInt16u(delay);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

sl_zigbee_library_status_t sl_zigbee_ezsp_get_library_status(
      sl_zigbee_library_id_t libraryId)
{
  sl_zigbee_library_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_LIBRARY_STATUS);
  appendInt8u(libraryId);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt8u();
    return status;
  }
  return SL_ZIGBEE_LIBRARY_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_xncp_info(
      uint16_t *manufacturerId,
      uint16_t *versionNumber)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_XNCP_INFO);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    *manufacturerId = fetchInt16u();
    *versionNumber = fetchInt16u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_custom_frame(
      uint8_t payloadLength,
      uint8_t *payload,
      uint8_t *replyLength,
      uint8_t *reply)
{
  sl_status_t status;
  uint8_t maxReplyLength = *replyLength;
  startCommand(SL_ZIGBEE_EZSP_CUSTOM_FRAME);
  appendInt8u(payloadLength);
  appendInt8uArray(payloadLength, payload);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    *replyLength = fetchInt8u();
  if (*replyLength > maxReplyLength) {
    return SL_STATUS_INVALID_PARAMETER;
  }
    fetchInt8uArray(*replyLength, reply);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

void sl_zigbee_ezsp_get_eui64(
      sl_802154_long_addr_t eui64)
{
  startCommand(SL_ZIGBEE_EZSP_GET_EUI64);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    fetchInt8uArray(8, eui64);
  }
}

sl_802154_short_addr_t sl_zigbee_ezsp_get_node_id(void)
{
  sl_802154_short_addr_t nodeId;
  startCommand(SL_ZIGBEE_EZSP_GET_NODE_ID);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    nodeId = fetchInt16u();
    return nodeId;
  }
  return 0xFFFE;
}

uint8_t sl_zigbee_ezsp_get_phy_interface_count(void)
{
  uint8_t interfaceCount;
  startCommand(SL_ZIGBEE_EZSP_GET_PHY_INTERFACE_COUNT);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    interfaceCount = fetchInt8u();
    return interfaceCount;
  }
  return 255;
}

sl_zigbee_entropy_source_t sl_zigbee_ezsp_get_true_random_entropy_source(void)
{
  sl_zigbee_entropy_source_t entropySource;
  startCommand(SL_ZIGBEE_EZSP_GET_TRUE_RANDOM_ENTROPY_SOURCE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    entropySource = fetchInt8u();
    return entropySource;
  }
  return 255;
}

sl_status_t sl_zigbee_ezsp_setup_delayed_join(
      uint8_t networkKeyTimeoutS)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SETUP_DELAYED_JOIN);
  appendInt8u(networkKeyTimeoutS);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

void sl_zigbee_ezsp_radio_get_scheduler_priorities(
      sl_802154_radio_priorities_t *priorities)
{
  startCommand(SL_ZIGBEE_EZSP_RADIO_GET_SCHEDULER_PRIORITIES);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    fetch_sl_802154_radio_priorities_t(priorities);
  }
}

void sl_zigbee_ezsp_radio_get_scheduler_sliptime(
      uint32_t *slipTime)
{
  startCommand(SL_ZIGBEE_EZSP_RADIO_GET_SCHEDULER_SLIPTIME);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    fetchInt32uArray(1, slipTime);
  }
}

void sl_zigbee_ezsp_radio_set_scheduler_sliptime(
      uint32_t slipTime)
{
  startCommand(SL_ZIGBEE_EZSP_RADIO_SET_SCHEDULER_SLIPTIME);
  appendInt32u(slipTime);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

bool sl_zigbee_ezsp_counter_requires_phy_index(
      sl_zigbee_counter_type_t counter)
{
  bool requires;
  startCommand(SL_ZIGBEE_EZSP_COUNTER_REQUIRES_PHY_INDEX);
  appendInt8u(counter);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    requires = fetchInt8u();
    return requires;
  }
  return false;
}

bool sl_zigbee_ezsp_counter_requires_destination_node_id(
      sl_zigbee_counter_type_t counter)
{
  bool requires;
  startCommand(SL_ZIGBEE_EZSP_COUNTER_REQUIRES_DESTINATION_NODE_ID);
  appendInt8u(counter);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    requires = fetchInt8u();
    return requires;
  }
  return false;
}

//------------------------------------------------------------------------------
// Networking Frames
//------------------------------------------------------------------------------

sl_status_t sl_zigbee_ezsp_set_manufacturer_code(
      uint16_t code)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_MANUFACTURER_CODE);
  appendInt16u(code);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

uint16_t sl_zigbee_ezsp_get_manufacturer_code(void)
{
  uint16_t code;
  startCommand(SL_ZIGBEE_EZSP_GET_MANUFACTURER_CODE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    code = fetchInt16u();
    return code;
  }
  return 255;
}

sl_status_t sl_zigbee_ezsp_set_power_descriptor(
      uint16_t descriptor)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_POWER_DESCRIPTOR);
  appendInt16u(descriptor);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_network_init(
      sl_zigbee_network_init_struct_t *networkInitStruct)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_NETWORK_INIT);
  append_sl_zigbee_network_init_struct_t(networkInitStruct);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_zigbee_network_status_t sl_zigbee_ezsp_network_state(void)
{
  sl_zigbee_network_status_t status;
  startCommand(SL_ZIGBEE_EZSP_NETWORK_STATE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt8u();
    return status;
  }
  return SL_ZIGBEE_NO_NETWORK;
}

sl_status_t sl_zigbee_ezsp_start_scan(
      sl_zigbee_ezsp_network_scan_type_t scanType,
      uint32_t channelMask,
      uint8_t duration)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_START_SCAN);
  appendInt8u(scanType);
  appendInt32u(channelMask);
  appendInt8u(duration);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_find_unused_pan_id(
      uint32_t channelMask,
      uint8_t duration)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_FIND_UNUSED_PAN_ID);
  appendInt32u(channelMask);
  appendInt8u(duration);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_stop_scan(void)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_STOP_SCAN);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_form_network(
      sl_zigbee_network_parameters_t *parameters)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_FORM_NETWORK);
  append_sl_zigbee_network_parameters_t(parameters);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_join_network(
      sl_zigbee_node_type_t nodeType,
      sl_zigbee_network_parameters_t *parameters)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_JOIN_NETWORK);
  appendInt8u(nodeType);
  append_sl_zigbee_network_parameters_t(parameters);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_join_network_directly(
      sl_zigbee_node_type_t localNodeType,
      sl_zigbee_beacon_data_t *beacon,
      int8_t radioTxPower,
      bool clearBeaconsAfterNetworkUp)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_JOIN_NETWORK_DIRECTLY);
  appendInt8u(localNodeType);
  append_sl_zigbee_beacon_data_t(beacon);
  appendInt8(radioTxPower);
  appendInt8u(clearBeaconsAfterNetworkUp);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_leave_network(
      sl_zigbee_leave_network_option_t options)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_LEAVE_NETWORK);
  appendInt8u(options);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_find_and_rejoin_network(
      bool haveCurrentNetworkKey,
      uint32_t channelMask,
      uint8_t reason,
      uint8_t nodeType)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_FIND_AND_REJOIN_NETWORK);
  appendInt8u(haveCurrentNetworkKey);
  appendInt32u(channelMask);
  appendInt8u(reason);
  appendInt8u(nodeType);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_permit_joining(
      uint8_t duration)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_PERMIT_JOINING);
  appendInt8u(duration);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_energy_scan_request(
      sl_802154_short_addr_t target,
      uint32_t scanChannels,
      uint8_t scanDuration,
      uint16_t scanCount)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_ENERGY_SCAN_REQUEST);
  appendInt16u(target);
  appendInt32u(scanChannels);
  appendInt8u(scanDuration);
  appendInt16u(scanCount);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_network_parameters(
      sl_zigbee_node_type_t *nodeType,
      sl_zigbee_network_parameters_t *parameters)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_NETWORK_PARAMETERS);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    *nodeType = fetchInt8u();
    fetch_sl_zigbee_network_parameters_t(parameters);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_radio_parameters(
      uint8_t phyIndex,
      sl_zigbee_multi_phy_radio_parameters_t *parameters)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_RADIO_PARAMETERS);
  appendInt8u(phyIndex);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_multi_phy_radio_parameters_t(parameters);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

uint8_t sl_zigbee_ezsp_get_parent_child_parameters(
      sl_802154_long_addr_t parentEui64,
      sl_802154_short_addr_t *parentNodeId)
{
  uint8_t childCount;
  startCommand(SL_ZIGBEE_EZSP_GET_PARENT_CHILD_PARAMETERS);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    childCount = fetchInt8u();
    fetchInt8uArray(8, parentEui64);
    *parentNodeId = fetchInt16u();
    return childCount;
  }
  return 255;
}

uint8_t sl_zigbee_ezsp_router_child_count(void)
{
  uint8_t routerChildCount;
  startCommand(SL_ZIGBEE_EZSP_ROUTER_CHILD_COUNT);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    routerChildCount = fetchInt8u();
    return routerChildCount;
  }
  return 255;
}

uint8_t sl_zigbee_ezsp_max_child_count(void)
{
  uint8_t maxChildCount;
  startCommand(SL_ZIGBEE_EZSP_MAX_CHILD_COUNT);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    maxChildCount = fetchInt8u();
    return maxChildCount;
  }
  return 255;
}

uint8_t sl_zigbee_ezsp_max_router_child_count(void)
{
  uint8_t maxRouterChildCount;
  startCommand(SL_ZIGBEE_EZSP_MAX_ROUTER_CHILD_COUNT);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    maxRouterChildCount = fetchInt8u();
    return maxRouterChildCount;
  }
  return 255;
}

uint32_t sl_zigbee_ezsp_get_parent_incoming_nwk_frame_counter(void)
{
  uint32_t parentIncomingNwkFrameCounter;
  startCommand(SL_ZIGBEE_EZSP_GET_PARENT_INCOMING_NWK_FRAME_COUNTER);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    parentIncomingNwkFrameCounter = fetchInt32u();
    return parentIncomingNwkFrameCounter;
  }
  return 255;
}

sl_status_t sl_zigbee_ezsp_set_parent_incoming_nwk_frame_counter(
      uint32_t value)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_PARENT_INCOMING_NWK_FRAME_COUNTER);
  appendInt32u(value);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

uint16_t sl_zigbee_ezsp_current_stack_tasks(void)
{
  uint16_t activeTasks;
  startCommand(SL_ZIGBEE_EZSP_CURRENT_STACK_TASKS);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    activeTasks = fetchInt16u();
    return activeTasks;
  }
  return 255;
}

bool sl_zigbee_ezsp_ok_to_nap(void)
{
  bool value;
  startCommand(SL_ZIGBEE_EZSP_OK_TO_NAP);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    value = fetchInt8u();
    return value;
  }
  return false;
}

bool sl_zigbee_ezsp_parent_token_set(void)
{
  bool indicator;
  startCommand(SL_ZIGBEE_EZSP_PARENT_TOKEN_SET);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    indicator = fetchInt8u();
    return indicator;
  }
  return false;
}

bool sl_zigbee_ezsp_ok_to_hibernate(void)
{
  bool indicator;
  startCommand(SL_ZIGBEE_EZSP_OK_TO_HIBERNATE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    indicator = fetchInt8u();
    return indicator;
  }
  return false;
}

bool sl_zigbee_ezsp_ok_to_long_poll(void)
{
  bool indicator;
  startCommand(SL_ZIGBEE_EZSP_OK_TO_LONG_POLL);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    indicator = fetchInt8u();
    return indicator;
  }
  return false;
}

void sl_zigbee_ezsp_stack_power_down(void)
{
  startCommand(SL_ZIGBEE_EZSP_STACK_POWER_DOWN);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

void sl_zigbee_ezsp_stack_power_up(void)
{
  startCommand(SL_ZIGBEE_EZSP_STACK_POWER_UP);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

sl_status_t sl_zigbee_ezsp_get_child_data(
      uint8_t index,
      sl_zigbee_child_data_t *childData)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_CHILD_DATA);
  appendInt8u(index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_child_data_t(childData);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_child_data(
      uint8_t index,
      sl_zigbee_child_data_t *childData)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_CHILD_DATA);
  appendInt8u(index);
  append_sl_zigbee_child_data_t(childData);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_802154_short_addr_t sl_zigbee_ezsp_child_id(
      uint8_t childIndex)
{
  sl_802154_short_addr_t childId;
  startCommand(SL_ZIGBEE_EZSP_CHILD_ID);
  appendInt8u(childIndex);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    childId = fetchInt16u();
    return childId;
  }
  return 0xFFFF;
}

int8_t sl_zigbee_ezsp_child_power(
      uint8_t childIndex)
{
  int8_t childPower;
  startCommand(SL_ZIGBEE_EZSP_CHILD_POWER);
  appendInt8u(childIndex);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    childPower = fetchInt8();
    return childPower;
  }
  return -128;
}

void sl_zigbee_ezsp_set_child_power(
      uint8_t childIndex,
      int8_t newPower)
{
  startCommand(SL_ZIGBEE_EZSP_SET_CHILD_POWER);
  appendInt8u(childIndex);
  appendInt8(newPower);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

uint8_t sl_zigbee_ezsp_child_index(
      sl_802154_short_addr_t childId)
{
  uint8_t childIndex;
  startCommand(SL_ZIGBEE_EZSP_CHILD_INDEX);
  appendInt16u(childId);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    childIndex = fetchInt8u();
    return childIndex;
  }
  return 255;
}

uint8_t sl_zigbee_ezsp_get_source_route_table_total_size(void)
{
  uint8_t sourceRouteTableTotalSize;
  startCommand(SL_ZIGBEE_EZSP_GET_SOURCE_ROUTE_TABLE_TOTAL_SIZE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    sourceRouteTableTotalSize = fetchInt8u();
    return sourceRouteTableTotalSize;
  }
  return 255;
}

uint8_t sl_zigbee_ezsp_get_source_route_table_filled_size(void)
{
  uint8_t sourceRouteTableFilledSize;
  startCommand(SL_ZIGBEE_EZSP_GET_SOURCE_ROUTE_TABLE_FILLED_SIZE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    sourceRouteTableFilledSize = fetchInt8u();
    return sourceRouteTableFilledSize;
  }
  return 255;
}

sl_status_t sl_zigbee_ezsp_get_source_route_table_entry(
      uint8_t index,
      sl_802154_short_addr_t *destination,
      uint8_t *closerIndex)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_SOURCE_ROUTE_TABLE_ENTRY);
  appendInt8u(index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    *destination = fetchInt16u();
    *closerIndex = fetchInt8u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_neighbor(
      uint8_t index,
      sl_zigbee_neighbor_table_entry_t *value)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_NEIGHBOR);
  appendInt8u(index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_neighbor_table_entry_t(value);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_neighbor_frame_counter(
      sl_802154_long_addr_t eui64,
      uint32_t *returnFrameCounter)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_NEIGHBOR_FRAME_COUNTER);
  appendInt8uArray(8, eui64);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    *returnFrameCounter = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_neighbor_frame_counter(
      sl_802154_long_addr_t eui64,
      uint32_t frameCounter)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_NEIGHBOR_FRAME_COUNTER);
  appendInt8uArray(8, eui64);
  appendInt32u(frameCounter);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_routing_shortcut_threshold(
      uint8_t costThresh)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_ROUTING_SHORTCUT_THRESHOLD);
  appendInt8u(costThresh);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

uint8_t sl_zigbee_ezsp_get_routing_shortcut_threshold(void)
{
  uint8_t routingShortcutThresh;
  startCommand(SL_ZIGBEE_EZSP_GET_ROUTING_SHORTCUT_THRESHOLD);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    routingShortcutThresh = fetchInt8u();
    return routingShortcutThresh;
  }
  return 255;
}

uint8_t sl_zigbee_ezsp_neighbor_count(void)
{
  uint8_t value;
  startCommand(SL_ZIGBEE_EZSP_NEIGHBOR_COUNT);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    value = fetchInt8u();
    return value;
  }
  return 255;
}

sl_status_t sl_zigbee_ezsp_get_route_table_entry(
      uint8_t index,
      sl_zigbee_route_table_entry_t *value)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_ROUTE_TABLE_ENTRY);
  appendInt8u(index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_route_table_entry_t(value);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_radio_power(
      int8_t power)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_RADIO_POWER);
  appendInt8(power);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_radio_channel(
      uint8_t channel)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_RADIO_CHANNEL);
  appendInt8u(channel);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

uint8_t sl_zigbee_ezsp_get_radio_channel(void)
{
  uint8_t channel;
  startCommand(SL_ZIGBEE_EZSP_GET_RADIO_CHANNEL);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    channel = fetchInt8u();
    return channel;
  }
  return 255;
}

sl_status_t sl_zigbee_ezsp_set_radio_ieee802154_cca_mode(
      uint8_t ccaMode)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_RADIO_IEEE802154_CCA_MODE);
  appendInt8u(ccaMode);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_concentrator(
      bool on,
      uint16_t concentratorType,
      uint16_t minTime,
      uint16_t maxTime,
      uint8_t routeErrorThreshold,
      uint8_t deliveryFailureThreshold,
      uint8_t maxHops)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_CONCENTRATOR);
  appendInt8u(on);
  appendInt16u(concentratorType);
  appendInt16u(minTime);
  appendInt16u(maxTime);
  appendInt8u(routeErrorThreshold);
  appendInt8u(deliveryFailureThreshold);
  appendInt8u(maxHops);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

void sl_zigbee_ezsp_concentrator_start_discovery(void)
{
  startCommand(SL_ZIGBEE_EZSP_CONCENTRATOR_START_DISCOVERY);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

void sl_zigbee_ezsp_concentrator_stop_discovery(void)
{
  startCommand(SL_ZIGBEE_EZSP_CONCENTRATOR_STOP_DISCOVERY);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

void sl_zigbee_ezsp_concentrator_note_route_error(
      sl_status_t status,
      sl_802154_short_addr_t nodeId)
{
  startCommand(SL_ZIGBEE_EZSP_CONCENTRATOR_NOTE_ROUTE_ERROR);
  appendInt32u(status);
  appendInt16u(nodeId);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

sl_status_t sl_zigbee_ezsp_set_broken_route_error_code(
      uint8_t errorCode)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_BROKEN_ROUTE_ERROR_CODE);
  appendInt8u(errorCode);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_multi_phy_start(
      uint8_t phyIndex,
      uint8_t page,
      uint8_t channel,
      int8_t power,
      sl_zigbee_multi_phy_nwk_config_t bitmask)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MULTI_PHY_START);
  appendInt8u(phyIndex);
  appendInt8u(page);
  appendInt8u(channel);
  appendInt8(power);
  appendInt8u(bitmask);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_multi_phy_stop(
      uint8_t phyIndex)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MULTI_PHY_STOP);
  appendInt8u(phyIndex);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_multi_phy_set_radio_power(
      uint8_t phyIndex,
      int8_t power)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MULTI_PHY_SET_RADIO_POWER);
  appendInt8u(phyIndex);
  appendInt8(power);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_send_link_power_delta_request(void)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEND_LINK_POWER_DELTA_REQUEST);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_multi_phy_set_radio_channel(
      uint8_t phyIndex,
      uint8_t page,
      uint8_t channel)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MULTI_PHY_SET_RADIO_CHANNEL);
  appendInt8u(phyIndex);
  appendInt8u(page);
  appendInt8u(channel);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_duty_cycle_state(
      sl_zigbee_duty_cycle_state_t *returnedState)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_DUTY_CYCLE_STATE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    *returnedState = fetchInt8u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_duty_cycle_limits_in_stack(
      sl_zigbee_duty_cycle_limits_t *limits)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_DUTY_CYCLE_LIMITS_IN_STACK);
  append_sl_zigbee_duty_cycle_limits_t(limits);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_duty_cycle_limits(
      sl_zigbee_duty_cycle_limits_t *returnedLimits)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_DUTY_CYCLE_LIMITS);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_duty_cycle_limits_t(returnedLimits);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_current_duty_cycle(
      uint8_t maxDevices,
      uint8_t *arrayOfDeviceDutyCycles)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_CURRENT_DUTY_CYCLE);
  appendInt8u(maxDevices);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetchInt8uArray(134, arrayOfDeviceDutyCycles);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_num_beacons_to_store(
      uint8_t numBeacons)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_NUM_BEACONS_TO_STORE);
  appendInt8u(numBeacons);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_stored_beacon(
      uint8_t beacon_number,
      sl_zigbee_beacon_data_t *beacon)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_STORED_BEACON);
  appendInt8u(beacon_number);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_beacon_data_t(beacon);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

uint8_t sl_zigbee_ezsp_get_num_stored_beacons(void)
{
  uint8_t numBeacons;
  startCommand(SL_ZIGBEE_EZSP_GET_NUM_STORED_BEACONS);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    numBeacons = fetchInt8u();
    return numBeacons;
  }
  return 255;
}

sl_status_t sl_zigbee_ezsp_clear_stored_beacons(void)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_CLEAR_STORED_BEACONS);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_logical_and_radio_channel(
      uint8_t radioChannel)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_LOGICAL_AND_RADIO_CHANNEL);
  appendInt8u(radioChannel);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_sleepy_to_sleepy_network_start(
      sl_zigbee_network_parameters_t *parameters,
      bool initiator)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SLEEPY_TO_SLEEPY_NETWORK_START);
  append_sl_zigbee_network_parameters_t(parameters);
  appendInt8u(initiator);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_send_zigbee_leave(
      sl_802154_pan_id_t destination,
      sl_zigbee_leave_request_flags_t flags)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEND_ZIGBEE_LEAVE);
  appendInt16u(destination);
  appendInt8u(flags);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

bool sl_zigbee_ezsp_get_permit_joining(void)
{
  bool joiningPermitted;
  startCommand(SL_ZIGBEE_EZSP_GET_PERMIT_JOINING);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    joiningPermitted = fetchInt8u();
    return joiningPermitted;
  }
  return false;
}

void sl_zigbee_ezsp_get_extended_pan_id(
      uint8_t *extendedPanId)
{
  startCommand(SL_ZIGBEE_EZSP_GET_EXTENDED_PAN_ID);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    fetchInt8uArray(8, extendedPanId);
  }
}

uint8_t sl_zigbee_ezsp_get_current_network(void)
{
  uint8_t index;
  startCommand(SL_ZIGBEE_EZSP_GET_CURRENT_NETWORK);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    index = fetchInt8u();
    return index;
  }
  return 255;
}

sl_status_t sl_zigbee_ezsp_set_initial_neighbor_outgoing_cost(
      uint8_t cost)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_INITIAL_NEIGHBOR_OUTGOING_COST);
  appendInt8u(cost);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

uint8_t sl_zigbee_ezsp_get_initial_neighbor_outgoing_cost(void)
{
  uint8_t cost;
  startCommand(SL_ZIGBEE_EZSP_GET_INITIAL_NEIGHBOR_OUTGOING_COST);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    cost = fetchInt8u();
    return cost;
  }
  return 255;
}

void sl_zigbee_ezsp_reset_rejoining_neighbors_frame_counter(
      bool reset)
{
  startCommand(SL_ZIGBEE_EZSP_RESET_REJOINING_NEIGHBORS_FRAME_COUNTER);
  appendInt8u(reset);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

bool sl_zigbee_ezsp_is_reset_rejoining_neighbors_frame_counter_enabled(void)
{
  bool getsReset;
  startCommand(SL_ZIGBEE_EZSP_IS_RESET_REJOINING_NEIGHBORS_FRAME_COUNTER_ENABLED);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    getsReset = fetchInt8u();
    return getsReset;
  }
  return false;
}

//------------------------------------------------------------------------------
// Binding Frames
//------------------------------------------------------------------------------

sl_status_t sl_zigbee_ezsp_clear_binding_table(void)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_CLEAR_BINDING_TABLE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_binding(
      uint8_t index,
      sl_zigbee_binding_table_entry_t *value)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_BINDING);
  appendInt8u(index);
  append_sl_zigbee_binding_table_entry_t(value);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_binding(
      uint8_t index,
      sl_zigbee_binding_table_entry_t *value)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_BINDING);
  appendInt8u(index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_binding_table_entry_t(value);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_delete_binding(
      uint8_t index)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_DELETE_BINDING);
  appendInt8u(index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

bool sl_zigbee_ezsp_binding_is_active(
      uint8_t index)
{
  bool active;
  startCommand(SL_ZIGBEE_EZSP_BINDING_IS_ACTIVE);
  appendInt8u(index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    active = fetchInt8u();
    return active;
  }
  return false;
}

sl_802154_short_addr_t sl_zigbee_ezsp_get_binding_remote_node_id(
      uint8_t index)
{
  sl_802154_short_addr_t nodeId;
  startCommand(SL_ZIGBEE_EZSP_GET_BINDING_REMOTE_NODE_ID);
  appendInt8u(index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    nodeId = fetchInt16u();
    return nodeId;
  }
  return 0xFFFE;
}

void sl_zigbee_ezsp_set_binding_remote_node_id(
      uint8_t index,
      sl_802154_short_addr_t nodeId)
{
  startCommand(SL_ZIGBEE_EZSP_SET_BINDING_REMOTE_NODE_ID);
  appendInt8u(index);
  appendInt16u(nodeId);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

//------------------------------------------------------------------------------
// Messaging Frames
//------------------------------------------------------------------------------

uint8_t sl_zigbee_ezsp_maximum_payload_length(void)
{
  uint8_t apsLength;
  startCommand(SL_ZIGBEE_EZSP_MAXIMUM_PAYLOAD_LENGTH);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    apsLength = fetchInt8u();
    return apsLength;
  }
  return 255;
}

sl_status_t sl_zigbee_ezsp_send_unicast(
      sl_zigbee_outgoing_message_type_t type,
      sl_802154_short_addr_t indexOrDestination,
      sl_zigbee_aps_frame_t *apsFrame,
      uint16_t messageTag,
      uint8_t messageLength,
      uint8_t *messageContents,
      uint8_t *sequence)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEND_UNICAST);
  appendInt8u(type);
  appendInt16u(indexOrDestination);
  append_sl_zigbee_aps_frame_t(apsFrame);
  appendInt16u(messageTag);
  appendInt8u(messageLength);
  appendInt8uArray(messageLength, messageContents);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    *sequence = fetchInt8u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_send_broadcast(
      sl_802154_short_addr_t alias,
      sl_802154_short_addr_t destination,
      uint8_t nwkSequence,
      sl_zigbee_aps_frame_t *apsFrame,
      uint8_t radius,
      uint16_t messageTag,
      uint8_t messageLength,
      uint8_t *messageContents,
      uint8_t *apsSequence)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEND_BROADCAST);
  appendInt16u(alias);
  appendInt16u(destination);
  appendInt8u(nwkSequence);
  append_sl_zigbee_aps_frame_t(apsFrame);
  appendInt8u(radius);
  appendInt16u(messageTag);
  appendInt8u(messageLength);
  appendInt8uArray(messageLength, messageContents);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    *apsSequence = fetchInt8u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_proxy_next_broadcast_from_long(
      uint8_t *euiSource)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_PROXY_NEXT_BROADCAST_FROM_LONG);
  appendInt8uArray(8, euiSource);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_send_multicast(
      sl_zigbee_aps_frame_t *apsFrame,
      uint8_t hops,
      uint16_t broadcastAddr,
      uint16_t alias,
      uint8_t nwkSequence,
      uint16_t messageTag,
      uint8_t messageLength,
      uint8_t *messageContents,
      uint8_t *sequence)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEND_MULTICAST);
  append_sl_zigbee_aps_frame_t(apsFrame);
  appendInt8u(hops);
  appendInt16u(broadcastAddr);
  appendInt16u(alias);
  appendInt8u(nwkSequence);
  appendInt16u(messageTag);
  appendInt8u(messageLength);
  appendInt8uArray(messageLength, messageContents);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    *sequence = fetchInt8u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_send_reply(
      sl_802154_short_addr_t sender,
      sl_zigbee_aps_frame_t *apsFrame,
      uint8_t messageLength,
      uint8_t *messageContents)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEND_REPLY);
  appendInt16u(sender);
  append_sl_zigbee_aps_frame_t(apsFrame);
  appendInt8u(messageLength);
  appendInt8uArray(messageLength, messageContents);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_send_many_to_one_route_request(
      uint16_t concentratorType,
      uint8_t radius)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEND_MANY_TO_ONE_ROUTE_REQUEST);
  appendInt16u(concentratorType);
  appendInt8u(radius);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_poll_for_data(
      uint16_t interval,
      sl_zigbee_event_units_t units,
      uint8_t failureLimit)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_POLL_FOR_DATA);
  appendInt16u(interval);
  appendInt8u(units);
  appendInt8u(failureLimit);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_message_flag(
      sl_802154_short_addr_t childId)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_MESSAGE_FLAG);
  appendInt16u(childId);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_clear_message_flag(
      sl_802154_short_addr_t childId)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_CLEAR_MESSAGE_FLAG);
  appendInt16u(childId);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_add_child(
      sl_802154_short_addr_t shortId,
      sl_802154_long_addr_t longId,
      sl_zigbee_node_type_t nodeType)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_ADD_CHILD);
  appendInt16u(shortId);
  appendInt8uArray(8, longId);
  appendInt8u(nodeType);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_remove_child(
      sl_802154_long_addr_t childEui64)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_REMOVE_CHILD);
  appendInt8uArray(8, childEui64);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

void sl_zigbee_ezsp_remove_neighbor(
      sl_802154_short_addr_t shortId,
      sl_802154_long_addr_t longId)
{
  startCommand(SL_ZIGBEE_EZSP_REMOVE_NEIGHBOR);
  appendInt16u(shortId);
  appendInt8uArray(8, longId);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

uint32_t sl_zigbee_ezsp_set_source_route_discovery_mode(
      uint8_t mode)
{
  uint32_t remainingTime;
  startCommand(SL_ZIGBEE_EZSP_SET_SOURCE_ROUTE_DISCOVERY_MODE);
  appendInt8u(mode);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    remainingTime = fetchInt32u();
    return remainingTime;
  }
  return 255;
}

sl_status_t sl_zigbee_ezsp_set_source_route(
      sl_802154_short_addr_t destination,
      uint8_t relayCount,
      uint16_t *relayList)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_SOURCE_ROUTE);
  appendInt16u(destination);
  appendInt8u(relayCount);
  appendInt16uArray(relayCount, relayList);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_unicast_current_network_key(
      sl_802154_short_addr_t targetShort,
      sl_802154_long_addr_t targetLong,
      sl_802154_short_addr_t parentShortId)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_UNICAST_CURRENT_NETWORK_KEY);
  appendInt16u(targetShort);
  appendInt8uArray(8, targetLong);
  appendInt16u(parentShortId);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

bool sl_zigbee_ezsp_address_table_entry_is_active(
      uint8_t addressTableIndex)
{
  bool active;
  startCommand(SL_ZIGBEE_EZSP_ADDRESS_TABLE_ENTRY_IS_ACTIVE);
  appendInt8u(addressTableIndex);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    active = fetchInt8u();
    return active;
  }
  return false;
}

sl_status_t sl_zigbee_ezsp_set_address_table_info(
      uint8_t addressTableIndex,
      sl_802154_long_addr_t eui64,
      sl_802154_short_addr_t id)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_ADDRESS_TABLE_INFO);
  appendInt8u(addressTableIndex);
  appendInt8uArray(8, eui64);
  appendInt16u(id);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_address_table_info(
      uint8_t addressTableIndex,
      sl_802154_short_addr_t *nodeId,
      sl_802154_long_addr_t eui64)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_ADDRESS_TABLE_INFO);
  appendInt8u(addressTableIndex);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    *nodeId = fetchInt16u();
    fetchInt8uArray(8, eui64);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_extended_timeout(
      sl_802154_long_addr_t remoteEui64,
      bool extendedTimeout)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_EXTENDED_TIMEOUT);
  appendInt8uArray(8, remoteEui64);
  appendInt8u(extendedTimeout);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_extended_timeout(
      sl_802154_long_addr_t remoteEui64)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_EXTENDED_TIMEOUT);
  appendInt8uArray(8, remoteEui64);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_replace_address_table_entry(
      uint8_t addressTableIndex,
      sl_802154_long_addr_t newEui64,
      sl_802154_short_addr_t newId,
      bool newExtendedTimeout,
      sl_802154_long_addr_t oldEui64,
      sl_802154_short_addr_t *oldId,
      bool *oldExtendedTimeout)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_REPLACE_ADDRESS_TABLE_ENTRY);
  appendInt8u(addressTableIndex);
  appendInt8uArray(8, newEui64);
  appendInt16u(newId);
  appendInt8u(newExtendedTimeout);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetchInt8uArray(8, oldEui64);
    *oldId = fetchInt16u();
    *oldExtendedTimeout = fetchInt8u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_lookup_node_id_by_eui64(
      sl_802154_long_addr_t eui64,
      sl_802154_short_addr_t *nodeId)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_LOOKUP_NODE_ID_BY_EUI64);
  appendInt8uArray(8, eui64);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    *nodeId = fetchInt16u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_lookup_eui64_by_node_id(
      sl_802154_short_addr_t nodeId,
      sl_802154_long_addr_t eui64)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_LOOKUP_EUI64_BY_NODE_ID);
  appendInt16u(nodeId);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetchInt8uArray(8, eui64);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_multicast_table_entry(
      uint8_t index,
      sl_zigbee_multicast_table_entry_t *value)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_MULTICAST_TABLE_ENTRY);
  appendInt8u(index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_multicast_table_entry_t(value);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_multicast_table_entry(
      uint8_t index,
      sl_zigbee_multicast_table_entry_t *value)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_MULTICAST_TABLE_ENTRY);
  appendInt8u(index);
  append_sl_zigbee_multicast_table_entry_t(value);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_write_node_data(
      bool erase)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_WRITE_NODE_DATA);
  appendInt8u(erase);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_send_raw_message(
      uint8_t messageLength,
      uint8_t *messageContents,
      uint8_t priority,
      bool useCca)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEND_RAW_MESSAGE);
  appendInt8u(messageLength);
  appendInt8uArray(messageLength, messageContents);
  appendInt8u(priority);
  appendInt8u(useCca);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

void sl_zigbee_ezsp_set_mac_poll_failure_wait_time(
      uint32_t waitBeforeRetryIntervalMs)
{
  startCommand(SL_ZIGBEE_EZSP_SET_MAC_POLL_FAILURE_WAIT_TIME);
  appendInt32u(waitBeforeRetryIntervalMs);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

uint8_t sl_zigbee_ezsp_get_max_mac_retries(void)
{
  uint8_t retries;
  startCommand(SL_ZIGBEE_EZSP_GET_MAX_MAC_RETRIES);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    retries = fetchInt8u();
    return retries;
  }
  return 255;
}

sl_status_t sl_zigbee_ezsp_set_beacon_classification_params(
      sl_zigbee_beacon_classification_params_t *param)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_BEACON_CLASSIFICATION_PARAMS);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_beacon_classification_params_t(param);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_beacon_classification_params(
      sl_zigbee_beacon_classification_params_t *param)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_BEACON_CLASSIFICATION_PARAMS);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_beacon_classification_params_t(param);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

bool sl_zigbee_ezsp_pending_acked_messages(void)
{
  bool pending_messages;
  startCommand(SL_ZIGBEE_EZSP_PENDING_ACKED_MESSAGES);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    pending_messages = fetchInt8u();
    return pending_messages;
  }
  return false;
}

sl_status_t sl_zigbee_ezsp_reschedule_link_status_msg(void)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_RESCHEDULE_LINK_STATUS_MSG);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_nwk_update_id(
      uint8_t nwkUpdateId,
      bool set_when_on_network)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_NWK_UPDATE_ID);
  appendInt8u(nwkUpdateId);
  appendInt8u(set_when_on_network);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

//------------------------------------------------------------------------------
// Security Frames
//------------------------------------------------------------------------------

sl_status_t sl_zigbee_ezsp_set_initial_security_state(
      sl_zigbee_initial_security_state_t *state)
{
  sl_status_t success;
  startCommand(SL_ZIGBEE_EZSP_SET_INITIAL_SECURITY_STATE);
  append_sl_zigbee_initial_security_state_t(state);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    success = fetchInt32u();
    return success;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_current_security_state(
      sl_zigbee_current_security_state_t *state)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_CURRENT_SECURITY_STATE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_current_security_state_t(state);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_sec_man_export_key(
      sl_zigbee_sec_man_context_t *context,
      sl_zigbee_sec_man_key_t *key)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEC_MAN_EXPORT_KEY);
  append_sl_zigbee_sec_man_context_t(context);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_sec_man_key_t(key);
    fetch_sl_zigbee_sec_man_context_t(context);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_sec_man_import_key(
      sl_zigbee_sec_man_context_t *context,
      sl_zigbee_sec_man_key_t *key)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEC_MAN_IMPORT_KEY);
  append_sl_zigbee_sec_man_context_t(context);
  append_sl_zigbee_sec_man_key_t(key);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_sec_man_context_t(context);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

uint8_t sl_zigbee_ezsp_find_key_table_entry(
      sl_802154_long_addr_t address,
      bool linkKey)
{
  uint8_t index;
  startCommand(SL_ZIGBEE_EZSP_FIND_KEY_TABLE_ENTRY);
  appendInt8uArray(8, address);
  appendInt8u(linkKey);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    index = fetchInt8u();
    return index;
  }
  return 255;
}

sl_status_t sl_zigbee_ezsp_send_trust_center_link_key(
      sl_802154_short_addr_t destinationNodeId,
      sl_802154_long_addr_t destinationEui64)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEND_TRUST_CENTER_LINK_KEY);
  appendInt16u(destinationNodeId);
  appendInt8uArray(8, destinationEui64);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_erase_key_table_entry(
      uint8_t index)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_ERASE_KEY_TABLE_ENTRY);
  appendInt8u(index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_clear_key_table(void)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_CLEAR_KEY_TABLE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_request_link_key(
      sl_802154_long_addr_t partner)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_REQUEST_LINK_KEY);
  appendInt8uArray(8, partner);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_update_tc_link_key(
      uint8_t maxAttempts)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_UPDATE_TC_LINK_KEY);
  appendInt8u(maxAttempts);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

void sl_zigbee_ezsp_clear_transient_link_keys(void)
{
  startCommand(SL_ZIGBEE_EZSP_CLEAR_TRANSIENT_LINK_KEYS);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

sl_status_t sl_zigbee_ezsp_sec_man_get_network_key_info(
      sl_zigbee_sec_man_network_key_info_t *network_key_info)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEC_MAN_GET_NETWORK_KEY_INFO);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_sec_man_network_key_info_t(network_key_info);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_sec_man_get_aps_key_info(
      sl_zigbee_sec_man_context_t *context,
      sl_zigbee_sec_man_aps_key_metadata_t *key_data)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEC_MAN_GET_APS_KEY_INFO);
  append_sl_zigbee_sec_man_context_t(context);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_sec_man_aps_key_metadata_t(key_data);
    fetch_sl_zigbee_sec_man_context_t(context);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_sec_man_import_link_key(
      uint8_t index,
      sl_802154_long_addr_t address,
      sl_zigbee_sec_man_key_t *plaintext_key)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEC_MAN_IMPORT_LINK_KEY);
  appendInt8u(index);
  appendInt8uArray(8, address);
  append_sl_zigbee_sec_man_key_t(plaintext_key);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_sec_man_export_link_key_by_index(
      uint8_t index,
      sl_zigbee_sec_man_context_t *context,
      sl_zigbee_sec_man_key_t *plaintext_key,
      sl_zigbee_sec_man_aps_key_metadata_t *key_data)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEC_MAN_EXPORT_LINK_KEY_BY_INDEX);
  appendInt8u(index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_sec_man_context_t(context);
    fetch_sl_zigbee_sec_man_key_t(plaintext_key);
    fetch_sl_zigbee_sec_man_aps_key_metadata_t(key_data);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_sec_man_export_link_key_by_eui(
      sl_802154_long_addr_t eui,
      sl_zigbee_sec_man_context_t *context,
      sl_zigbee_sec_man_key_t *plaintext_key,
      sl_zigbee_sec_man_aps_key_metadata_t *key_data)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEC_MAN_EXPORT_LINK_KEY_BY_EUI);
  appendInt8uArray(8, eui);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_sec_man_context_t(context);
    fetch_sl_zigbee_sec_man_key_t(plaintext_key);
    fetch_sl_zigbee_sec_man_aps_key_metadata_t(key_data);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_sec_man_check_key_context(
      sl_zigbee_sec_man_context_t *context)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEC_MAN_CHECK_KEY_CONTEXT);
  append_sl_zigbee_sec_man_context_t(context);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_sec_man_import_transient_key(
      sl_802154_long_addr_t eui64,
      sl_zigbee_sec_man_key_t *plaintext_key)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEC_MAN_IMPORT_TRANSIENT_KEY);
  appendInt8uArray(8, eui64);
  append_sl_zigbee_sec_man_key_t(plaintext_key);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_sec_man_export_transient_key_by_index(
      uint8_t index,
      sl_zigbee_sec_man_context_t *context,
      sl_zigbee_sec_man_key_t *plaintext_key,
      sl_zigbee_sec_man_aps_key_metadata_t *key_data)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEC_MAN_EXPORT_TRANSIENT_KEY_BY_INDEX);
  appendInt8u(index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_sec_man_context_t(context);
    fetch_sl_zigbee_sec_man_key_t(plaintext_key);
    fetch_sl_zigbee_sec_man_aps_key_metadata_t(key_data);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_sec_man_export_transient_key_by_eui(
      sl_802154_long_addr_t eui,
      sl_zigbee_sec_man_context_t *context,
      sl_zigbee_sec_man_key_t *plaintext_key,
      sl_zigbee_sec_man_aps_key_metadata_t *key_data)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEC_MAN_EXPORT_TRANSIENT_KEY_BY_EUI);
  appendInt8uArray(8, eui);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_sec_man_context_t(context);
    fetch_sl_zigbee_sec_man_key_t(plaintext_key);
    fetch_sl_zigbee_sec_man_aps_key_metadata_t(key_data);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

void sl_zigbee_ezsp_set_incoming_tc_link_key_frame_counter(
      uint32_t frameCounter)
{
  startCommand(SL_ZIGBEE_EZSP_SET_INCOMING_TC_LINK_KEY_FRAME_COUNTER);
  appendInt32u(frameCounter);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

sl_status_t sl_zigbee_ezsp_aps_crypt_message(
      bool encrypt,
      uint8_t length_combined_arg,
      uint8_t *message,
      uint8_t apsHeaderEndIndex,
      sl_802154_long_addr_t remoteEui64)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_APS_CRYPT_MESSAGE);
  appendInt8u(encrypt);
  appendInt8u(length_combined_arg);
  appendInt8uArray(length_combined_arg, message);
  appendInt8u(apsHeaderEndIndex);
  appendInt8uArray(8, remoteEui64);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetchInt8uArray(length_combined_arg, message);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

//------------------------------------------------------------------------------
// Trust Center Frames
//------------------------------------------------------------------------------

sl_status_t sl_zigbee_ezsp_broadcast_next_network_key(
      sl_zigbee_key_data_t *key)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_BROADCAST_NEXT_NETWORK_KEY);
  append_sl_zigbee_key_data_t(key);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_broadcast_network_key_switch(void)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_BROADCAST_NETWORK_KEY_SWITCH);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_aes_mmo_hash(
      sl_zigbee_aes_mmo_hash_context_t *context,
      bool finalize,
      uint8_t length,
      uint8_t *data,
      sl_zigbee_aes_mmo_hash_context_t *returnContext)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_AES_MMO_HASH);
  append_sl_zigbee_aes_mmo_hash_context_t(context);
  appendInt8u(finalize);
  appendInt8u(length);
  appendInt8uArray(length, data);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_aes_mmo_hash_context_t(returnContext);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_remove_device(
      sl_802154_short_addr_t destShort,
      sl_802154_long_addr_t destLong,
      sl_802154_long_addr_t targetLong)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_REMOVE_DEVICE);
  appendInt16u(destShort);
  appendInt8uArray(8, destLong);
  appendInt8uArray(8, targetLong);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_unicast_nwk_key_update(
      sl_802154_short_addr_t destShort,
      sl_802154_long_addr_t destLong,
      sl_zigbee_key_data_t *key)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_UNICAST_NWK_KEY_UPDATE);
  appendInt16u(destShort);
  appendInt8uArray(8, destLong);
  append_sl_zigbee_key_data_t(key);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

//------------------------------------------------------------------------------
// Certificate Based Key Exchange (CBKE) Frames
//------------------------------------------------------------------------------

sl_status_t sl_zigbee_ezsp_generate_cbke_keys(void)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GENERATE_CBKE_KEYS);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_calculate_smacs(
      bool amInitiator,
      sl_zigbee_certificate_data_t *partnerCertificate,
      sl_zigbee_public_key_data_t *partnerEphemeralPublicKey)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_CALCULATE_SMACS);
  appendInt8u(amInitiator);
  append_sl_zigbee_certificate_data_t(partnerCertificate);
  append_sl_zigbee_public_key_data_t(partnerEphemeralPublicKey);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_generate_cbke_keys_283k1(void)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GENERATE_CBKE_KEYS_283K1);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_calculate_smacs_283k1(
      bool amInitiator,
      sl_zigbee_certificate_283k1_data_t *partnerCertificate,
      sl_zigbee_public_key_283k1_data_t *partnerEphemeralPublicKey)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_CALCULATE_SMACS_283K1);
  appendInt8u(amInitiator);
  append_sl_zigbee_certificate_283k1_data_t(partnerCertificate);
  append_sl_zigbee_public_key_283k1_data_t(partnerEphemeralPublicKey);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_clear_temporary_data_maybe_store_link_key(
      bool storeLinkKey)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_CLEAR_TEMPORARY_DATA_MAYBE_STORE_LINK_KEY);
  appendInt8u(storeLinkKey);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_clear_temporary_data_maybe_store_link_key_283k1(
      bool storeLinkKey)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_CLEAR_TEMPORARY_DATA_MAYBE_STORE_LINK_KEY_283K1);
  appendInt8u(storeLinkKey);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_certificate(
      sl_zigbee_certificate_data_t *localCert)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_CERTIFICATE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_certificate_data_t(localCert);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_certificate_283k1(
      sl_zigbee_certificate_283k1_data_t *localCert)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_CERTIFICATE_283K1);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_certificate_283k1_data_t(localCert);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_dsa_sign(
      uint8_t messageLength,
      uint8_t *messageContents)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_DSA_SIGN);
  appendInt8u(messageLength);
  appendInt8uArray(messageLength, messageContents);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_dsa_verify(
      sl_zigbee_message_digest_t *digest,
      sl_zigbee_certificate_data_t *signerCertificate,
      sl_zigbee_signature_data_t *receivedSig)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_DSA_VERIFY);
  append_sl_zigbee_message_digest_t(digest);
  append_sl_zigbee_certificate_data_t(signerCertificate);
  append_sl_zigbee_signature_data_t(receivedSig);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_dsa_verify_283k1(
      sl_zigbee_message_digest_t *digest,
      sl_zigbee_certificate_283k1_data_t *signerCertificate,
      sl_zigbee_signature_283k1_data_t *receivedSig)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_DSA_VERIFY_283K1);
  append_sl_zigbee_message_digest_t(digest);
  append_sl_zigbee_certificate_283k1_data_t(signerCertificate);
  append_sl_zigbee_signature_283k1_data_t(receivedSig);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_preinstalled_cbke_data(
      sl_zigbee_public_key_data_t *caPublic,
      sl_zigbee_certificate_data_t *myCert,
      sl_zigbee_private_key_data_t *myKey)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_PREINSTALLED_CBKE_DATA);
  append_sl_zigbee_public_key_data_t(caPublic);
  append_sl_zigbee_certificate_data_t(myCert);
  append_sl_zigbee_private_key_data_t(myKey);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_save_preinstalled_cbke_data_283k1(void)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SAVE_PREINSTALLED_CBKE_DATA_283K1);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

//------------------------------------------------------------------------------
// Mfglib Frames
//------------------------------------------------------------------------------

sl_status_t mfglibInternalStart(
      bool rxCallback)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_START);
  appendInt8u(rxCallback);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t mfglibInternalEnd(void)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_END);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t mfglibInternalStartTone(void)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_START_TONE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t mfglibInternalStopTone(void)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_STOP_TONE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t mfglibInternalStartStream(void)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_START_STREAM);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t mfglibInternalStopStream(void)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_STOP_STREAM);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t mfglibInternalSendPacket(
      uint8_t packetLength,
      uint8_t *packetContents)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_SEND_PACKET);
  appendInt8u(packetLength);
  appendInt8uArray(packetLength, packetContents);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t mfglibInternalSetChannel(
      uint8_t channel)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_SET_CHANNEL);
  appendInt8u(channel);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

uint8_t mfglibInternalGetChannel(void)
{
  uint8_t channel;
  startCommand(SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_GET_CHANNEL);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    channel = fetchInt8u();
    return channel;
  }
  return 255;
}

sl_status_t mfglibInternalSetPower(
      uint16_t txPowerMode,
      int8_t power)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_SET_POWER);
  appendInt16u(txPowerMode);
  appendInt8(power);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

int8_t mfglibInternalGetPower(void)
{
  int8_t power;
  startCommand(SL_ZIGBEE_EZSP_MFGLIB_INTERNAL_GET_POWER);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    power = fetchInt8();
    return power;
  }
  return -128;
}

//------------------------------------------------------------------------------
// Bootloader Frames
//------------------------------------------------------------------------------

sl_status_t sl_zigbee_ezsp_launch_standalone_bootloader(
      bool enabled)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_LAUNCH_STANDALONE_BOOTLOADER);
  appendInt8u(enabled);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_send_bootload_message(
      bool broadcast,
      sl_802154_long_addr_t destEui64,
      uint8_t messageLength,
      uint8_t *messageContents)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SEND_BOOTLOAD_MESSAGE);
  appendInt8u(broadcast);
  appendInt8uArray(8, destEui64);
  appendInt8u(messageLength);
  appendInt8uArray(messageLength, messageContents);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

uint16_t sl_zigbee_ezsp_get_standalone_bootloader_version_plat_micro_phy(
      uint8_t *nodePlat,
      uint8_t *nodeMicro,
      uint8_t *nodePhy)
{
  uint16_t bootloader_version;
  startCommand(SL_ZIGBEE_EZSP_GET_STANDALONE_BOOTLOADER_VERSION_PLAT_MICRO_PHY);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    bootloader_version = fetchInt16u();
    *nodePlat = fetchInt8u();
    *nodeMicro = fetchInt8u();
    *nodePhy = fetchInt8u();
    return bootloader_version;
  }
  return 255;
}

void sl_zigbee_ezsp_aes_encrypt(
      uint8_t *plaintext,
      uint8_t *key,
      uint8_t *ciphertext)
{
  startCommand(SL_ZIGBEE_EZSP_AES_ENCRYPT);
  appendInt8uArray(16, plaintext);
  appendInt8uArray(16, key);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    fetchInt8uArray(16, ciphertext);
  }
}

sl_status_t sl_zigbee_ezsp_mfg_test_set_packet_mode(
      bool beginConfiguration)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MFG_TEST_SET_PACKET_MODE);
  appendInt8u(beginConfiguration);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_mfg_test_send_reboot_command(void)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MFG_TEST_SEND_REBOOT_COMMAND);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_mfg_test_send_eui64(
      sl_802154_long_addr_t newId)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MFG_TEST_SEND_EUI64);
  appendInt8uArray(8, newId);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_mfg_test_send_manufacturing_string(
      sl_zigbee_manufacturing_string_t newString)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MFG_TEST_SEND_MANUFACTURING_STRING);
  appendInt8uArray(16, newString);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_mfg_test_send_radio_parameters(
      uint8_t supportedBands,
      int8_t crystalOffset)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MFG_TEST_SEND_RADIO_PARAMETERS);
  appendInt8u(supportedBands);
  appendInt8(crystalOffset);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_mfg_test_send_command(
      uint8_t *command)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_MFG_TEST_SEND_COMMAND);
  appendInt8uArray(1, command);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

//------------------------------------------------------------------------------
// ZLL Frames
//------------------------------------------------------------------------------

sl_status_t sl_zigbee_ezsp_zll_network_ops(
      sl_zigbee_zll_network_t *networkInfo,
      sl_zigbee_ezsp_zll_network_operation_t op,
      int8_t radioTxPower)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_ZLL_NETWORK_OPS);
  append_sl_zigbee_zll_network_t(networkInfo);
  appendInt8u(op);
  appendInt8(radioTxPower);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_zll_set_initial_security_state(
      sl_zigbee_key_data_t *networkKey,
      sl_zigbee_zll_initial_security_state_t *securityState)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_ZLL_SET_INITIAL_SECURITY_STATE);
  append_sl_zigbee_key_data_t(networkKey);
  append_sl_zigbee_zll_initial_security_state_t(securityState);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_zll_set_security_state_without_key(
      sl_zigbee_zll_initial_security_state_t *securityState)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_ZLL_SET_SECURITY_STATE_WITHOUT_KEY);
  append_sl_zigbee_zll_initial_security_state_t(securityState);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_zll_start_scan(
      uint32_t channelMask,
      int8_t radioPowerForScan,
      sl_zigbee_node_type_t nodeType)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_ZLL_START_SCAN);
  appendInt32u(channelMask);
  appendInt8(radioPowerForScan);
  appendInt8u(nodeType);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_zll_set_rx_on_when_idle(
      uint32_t durationMs)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_ZLL_SET_RX_ON_WHEN_IDLE);
  appendInt32u(durationMs);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

void sl_zigbee_ezsp_zll_get_tokens(
      sl_zigbee_tok_type_stack_zll_data_t *data,
      sl_zigbee_tok_type_stack_zll_security_t *security)
{
  startCommand(SL_ZIGBEE_EZSP_ZLL_GET_TOKENS);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    fetch_sl_zigbee_tok_type_stack_zll_data_t(data);
    fetch_sl_zigbee_tok_type_stack_zll_security_t(security);
  }
}

void sl_zigbee_ezsp_zll_set_data_token(
      sl_zigbee_tok_type_stack_zll_data_t *data)
{
  startCommand(SL_ZIGBEE_EZSP_ZLL_SET_DATA_TOKEN);
  append_sl_zigbee_tok_type_stack_zll_data_t(data);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

void sl_zigbee_ezsp_zll_set_non_zll_network(void)
{
  startCommand(SL_ZIGBEE_EZSP_ZLL_SET_NON_ZLL_NETWORK);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

bool sl_zigbee_ezsp_is_zll_network(void)
{
  bool isZllNetwork;
  startCommand(SL_ZIGBEE_EZSP_IS_ZLL_NETWORK);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    isZllNetwork = fetchInt8u();
    return isZllNetwork;
  }
  return false;
}

void sl_zigbee_ezsp_zll_set_radio_idle_mode(
      sl_zigbee_radio_power_mode_t mode)
{
  startCommand(SL_ZIGBEE_EZSP_ZLL_SET_RADIO_IDLE_MODE);
  appendInt8u(mode);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

uint8_t sl_zigbee_ezsp_zll_get_radio_idle_mode(void)
{
  uint8_t radioIdleMode;
  startCommand(SL_ZIGBEE_EZSP_ZLL_GET_RADIO_IDLE_MODE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    radioIdleMode = fetchInt8u();
    return radioIdleMode;
  }
  return 255;
}

void sl_zigbee_ezsp_set_zll_node_type(
      sl_zigbee_node_type_t nodeType)
{
  startCommand(SL_ZIGBEE_EZSP_SET_ZLL_NODE_TYPE);
  appendInt8u(nodeType);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

void sl_zigbee_ezsp_set_zll_additional_state(
      uint16_t state)
{
  startCommand(SL_ZIGBEE_EZSP_SET_ZLL_ADDITIONAL_STATE);
  appendInt16u(state);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

bool sl_zigbee_ezsp_zll_operation_in_progress(void)
{
  bool zllOperationInProgress;
  startCommand(SL_ZIGBEE_EZSP_ZLL_OPERATION_IN_PROGRESS);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    zllOperationInProgress = fetchInt8u();
    return zllOperationInProgress;
  }
  return false;
}

bool sl_zigbee_ezsp_zll_rx_on_when_idle_get_active(void)
{
  bool zllRxOnWhenIdleGetActive;
  startCommand(SL_ZIGBEE_EZSP_ZLL_RX_ON_WHEN_IDLE_GET_ACTIVE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    zllRxOnWhenIdleGetActive = fetchInt8u();
    return zllRxOnWhenIdleGetActive;
  }
  return false;
}

void sl_zigbee_ezsp_zll_scanning_complete(void)
{
  startCommand(SL_ZIGBEE_EZSP_ZLL_SCANNING_COMPLETE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

uint32_t sl_zigbee_ezsp_get_zll_primary_channel_mask(void)
{
  uint32_t zllPrimaryChannelMask;
  startCommand(SL_ZIGBEE_EZSP_GET_ZLL_PRIMARY_CHANNEL_MASK);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    zllPrimaryChannelMask = fetchInt32u();
    return zllPrimaryChannelMask;
  }
  return 255;
}

uint32_t sl_zigbee_ezsp_get_zll_secondary_channel_mask(void)
{
  uint32_t zllSecondaryChannelMask;
  startCommand(SL_ZIGBEE_EZSP_GET_ZLL_SECONDARY_CHANNEL_MASK);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    zllSecondaryChannelMask = fetchInt32u();
    return zllSecondaryChannelMask;
  }
  return 255;
}

void sl_zigbee_ezsp_set_zll_primary_channel_mask(
      uint32_t zllPrimaryChannelMask)
{
  startCommand(SL_ZIGBEE_EZSP_SET_ZLL_PRIMARY_CHANNEL_MASK);
  appendInt32u(zllPrimaryChannelMask);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

void sl_zigbee_ezsp_set_zll_secondary_channel_mask(
      uint32_t zllSecondaryChannelMask)
{
  startCommand(SL_ZIGBEE_EZSP_SET_ZLL_SECONDARY_CHANNEL_MASK);
  appendInt32u(zllSecondaryChannelMask);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

void sl_zigbee_ezsp_zll_clear_tokens(void)
{
  startCommand(SL_ZIGBEE_EZSP_ZLL_CLEAR_TOKENS);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

//------------------------------------------------------------------------------
// Green Power Frames
//------------------------------------------------------------------------------

bool sl_zigbee_ezsp_gp_proxy_table_process_gp_pairing(
      uint32_t options,
      sl_zigbee_gp_address_t *addr,
      uint8_t commMode,
      uint16_t sinkNetworkAddress,
      uint16_t sinkGroupId,
      uint16_t assignedAlias,
      uint8_t *sinkIeeeAddress,
      sl_zigbee_key_data_t *gpdKey,
      uint32_t gpdSecurityFrameCounter,
      uint8_t forwardingRadius)
{
  bool gpPairingAdded;
  startCommand(SL_ZIGBEE_EZSP_GP_PROXY_TABLE_PROCESS_GP_PAIRING);
  appendInt32u(options);
  append_sl_zigbee_gp_address_t(addr);
  appendInt8u(commMode);
  appendInt16u(sinkNetworkAddress);
  appendInt16u(sinkGroupId);
  appendInt16u(assignedAlias);
  appendInt8uArray(8, sinkIeeeAddress);
  append_sl_zigbee_key_data_t(gpdKey);
  appendInt32u(gpdSecurityFrameCounter);
  appendInt8u(forwardingRadius);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    gpPairingAdded = fetchInt8u();
    return gpPairingAdded;
  }
  return false;
}

sl_status_t sl_zigbee_ezsp_d_gp_send(
      bool action,
      bool useCca,
      sl_zigbee_gp_address_t *addr,
      uint8_t gpdCommandId,
      uint8_t gpdAsduLength,
      uint8_t *gpdAsdu,
      uint8_t gpepHandle,
      uint16_t gpTxQueueEntryLifetimeMs)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_D_GP_SEND);
  appendInt8u(action);
  appendInt8u(useCca);
  append_sl_zigbee_gp_address_t(addr);
  appendInt8u(gpdCommandId);
  appendInt8u(gpdAsduLength);
  appendInt8uArray(gpdAsduLength, gpdAsdu);
  appendInt8u(gpepHandle);
  appendInt16u(gpTxQueueEntryLifetimeMs);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_gp_proxy_table_get_entry(
      uint8_t proxyIndex,
      sl_zigbee_gp_proxy_table_entry_t *entry)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GP_PROXY_TABLE_GET_ENTRY);
  appendInt8u(proxyIndex);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_gp_proxy_table_entry_t(entry);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

uint8_t sl_zigbee_ezsp_gp_proxy_table_lookup(
      sl_zigbee_gp_address_t *addr)
{
  uint8_t index;
  startCommand(SL_ZIGBEE_EZSP_GP_PROXY_TABLE_LOOKUP);
  append_sl_zigbee_gp_address_t(addr);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    index = fetchInt8u();
    return index;
  }
  return 255;
}

void sl_zigbee_ezsp_gp_proxy_table_remove_entry(
      uint8_t proxyIndex)
{
  startCommand(SL_ZIGBEE_EZSP_GP_PROXY_TABLE_REMOVE_ENTRY);
  appendInt8u(proxyIndex);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

void sl_zigbee_ezsp_gp_clear_proxy_table(void)
{
  startCommand(SL_ZIGBEE_EZSP_GP_CLEAR_PROXY_TABLE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

sl_status_t sl_zigbee_ezsp_gp_sink_table_get_entry(
      uint8_t sinkIndex,
      sl_zigbee_gp_sink_table_entry_t *entry)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GP_SINK_TABLE_GET_ENTRY);
  appendInt8u(sinkIndex);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_gp_sink_table_entry_t(entry);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

uint8_t sl_zigbee_ezsp_gp_sink_table_lookup(
      sl_zigbee_gp_address_t *addr)
{
  uint8_t index;
  startCommand(SL_ZIGBEE_EZSP_GP_SINK_TABLE_LOOKUP);
  append_sl_zigbee_gp_address_t(addr);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    index = fetchInt8u();
    return index;
  }
  return 255;
}

sl_status_t sl_zigbee_ezsp_gp_sink_table_set_entry(
      uint8_t sinkIndex,
      sl_zigbee_gp_sink_table_entry_t *entry)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GP_SINK_TABLE_SET_ENTRY);
  appendInt8u(sinkIndex);
  append_sl_zigbee_gp_sink_table_entry_t(entry);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

void sl_zigbee_ezsp_gp_sink_table_remove_entry(
      uint8_t sinkIndex)
{
  startCommand(SL_ZIGBEE_EZSP_GP_SINK_TABLE_REMOVE_ENTRY);
  appendInt8u(sinkIndex);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

uint8_t sl_zigbee_ezsp_gp_sink_table_find_or_allocate_entry(
      sl_zigbee_gp_address_t *addr)
{
  uint8_t index;
  startCommand(SL_ZIGBEE_EZSP_GP_SINK_TABLE_FIND_OR_ALLOCATE_ENTRY);
  append_sl_zigbee_gp_address_t(addr);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    index = fetchInt8u();
    return index;
  }
  return 255;
}

void sl_zigbee_ezsp_gp_sink_table_clear_all(void)
{
  startCommand(SL_ZIGBEE_EZSP_GP_SINK_TABLE_CLEAR_ALL);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

void sl_zigbee_ezsp_gp_sink_table_init(void)
{
  startCommand(SL_ZIGBEE_EZSP_GP_SINK_TABLE_INIT);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

void sl_zigbee_ezsp_gp_sink_table_set_security_frame_counter(
      uint8_t index,
      uint32_t sfc)
{
  startCommand(SL_ZIGBEE_EZSP_GP_SINK_TABLE_SET_SECURITY_FRAME_COUNTER);
  appendInt8u(index);
  appendInt32u(sfc);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

sl_status_t sl_zigbee_ezsp_gp_sink_commission(
      uint8_t options,
      uint16_t gpmAddrForSecurity,
      uint16_t gpmAddrForPairing,
      uint8_t sinkEndpoint)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GP_SINK_COMMISSION);
  appendInt8u(options);
  appendInt16u(gpmAddrForSecurity);
  appendInt16u(gpmAddrForPairing);
  appendInt8u(sinkEndpoint);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

void sl_zigbee_ezsp_gp_translation_table_clear(void)
{
  startCommand(SL_ZIGBEE_EZSP_GP_TRANSLATION_TABLE_CLEAR);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

uint8_t sl_zigbee_ezsp_gp_sink_table_get_number_of_active_entries(void)
{
  uint8_t number_of_entries;
  startCommand(SL_ZIGBEE_EZSP_GP_SINK_TABLE_GET_NUMBER_OF_ACTIVE_ENTRIES);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    number_of_entries = fetchInt8u();
    return number_of_entries;
  }
  return 0;
}

//------------------------------------------------------------------------------
// Token Interface Frames
//------------------------------------------------------------------------------

uint32_t sl_zigbee_ezsp_get_token_count(void)
{
  uint32_t count;
  startCommand(SL_ZIGBEE_EZSP_GET_TOKEN_COUNT);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    count = fetchInt32u();
    return count;
  }
  return 255;
}

sl_status_t sl_zigbee_ezsp_get_token_info(
      uint8_t index,
      sl_zigbee_token_info_t *tokenInfo)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_TOKEN_INFO);
  appendInt8u(index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_token_info_t(tokenInfo);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_get_token_data(
      uint32_t token,
      uint32_t index,
      sl_zigbee_token_data_t *tokenData)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GET_TOKEN_DATA);
  appendInt32u(token);
  appendInt32u(index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_token_data_t(tokenData);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_set_token_data(
      uint32_t token,
      uint32_t index,
      sl_zigbee_token_data_t *tokenData)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_SET_TOKEN_DATA);
  appendInt32u(token);
  appendInt32u(index);
  append_sl_zigbee_token_data_t(tokenData);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

void sl_zigbee_ezsp_reset_node(void)
{
  startCommand(SL_ZIGBEE_EZSP_RESET_NODE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

sl_status_t sl_zigbee_ezsp_gp_security_test_vectors(void)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_GP_SECURITY_TEST_VECTORS);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

void sl_zigbee_ezsp_token_factory_reset(
      bool excludeOutgoingFC,
      bool excludeBootCounter)
{
  startCommand(SL_ZIGBEE_EZSP_TOKEN_FACTORY_RESET);
  appendInt8u(excludeOutgoingFC);
  appendInt8u(excludeBootCounter);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("%s(): sendCommand() error: 0x%02X", __func__, sendStatus);
  }
}

//------------------------------------------------------------------------------
// Dynamic Hardware Configuration Frames
//------------------------------------------------------------------------------

sl_status_t sl_zigbee_ezsp_read_pa_descriptor(
      uint8_t index,
      sl_zigbee_dhc_pa_descriptor_t *descriptor)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_READ_PA_DESCRIPTOR);
  appendInt8u(index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_dhc_pa_descriptor_t(descriptor);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_write_pa_descriptor(
      uint8_t index,
      sl_zigbee_dhc_pa_descriptor_t *descriptor)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_WRITE_PA_DESCRIPTOR);
  appendInt8u(index);
  append_sl_zigbee_dhc_pa_descriptor_t(descriptor);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_read_pa_curve_segment(
      uint8_t index,
      uint8_t segment_index,
      sl_zigbee_dhc_pa_curve_segment_t *segment)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_READ_PA_CURVE_SEGMENT);
  appendInt8u(index);
  appendInt8u(segment_index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_dhc_pa_curve_segment_t(segment);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_write_pa_curve_segment(
      uint8_t index,
      uint8_t segment_index,
      sl_zigbee_dhc_pa_curve_segment_t *segment)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_WRITE_PA_CURVE_SEGMENT);
  appendInt8u(index);
  appendInt8u(segment_index);
  append_sl_zigbee_dhc_pa_curve_segment_t(segment);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_read_pa_curve(
      uint8_t index,
      sl_zigbee_dhc_pa_curve_t *curve)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_READ_PA_CURVE);
  appendInt8u(index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_dhc_pa_curve_t(curve);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_write_pa_curve(
      uint8_t index,
      sl_zigbee_dhc_pa_curve_t *curve)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_WRITE_PA_CURVE);
  appendInt8u(index);
  append_sl_zigbee_dhc_pa_curve_t(curve);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_read_pa_table(
      uint8_t index,
      sl_zigbee_dhc_pa_table_t *table)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_READ_PA_TABLE);
  appendInt8u(index);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_dhc_pa_table_t(table);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_write_pa_table(
      uint8_t index,
      sl_zigbee_dhc_pa_table_t *table)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_WRITE_PA_TABLE);
  appendInt8u(index);
  append_sl_zigbee_dhc_pa_table_t(table);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_read_rssi_offset(
      sl_zigbee_dhc_rssi_offset_t *rssi_offset)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_READ_RSSI_OFFSET);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_dhc_rssi_offset_t(rssi_offset);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_write_rssi_offset(
      sl_zigbee_dhc_rssi_offset_t *rssi_offset)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_WRITE_RSSI_OFFSET);
  append_sl_zigbee_dhc_rssi_offset_t(rssi_offset);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_read_pa_voltage(
      uint16_t *pa_voltage)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_READ_PA_VOLTAGE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    *pa_voltage = fetchInt16u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_write_pa_voltage(
      uint16_t pa_voltage)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_WRITE_PA_VOLTAGE);
  appendInt16u(pa_voltage);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_read_pa_mode(
      sl_zigbee_dhc_pa_mode_t *pa_mode)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_READ_PA_MODE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_dhc_pa_mode_t(pa_mode);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_write_pa_mode(
      sl_zigbee_dhc_pa_mode_t *pa_mode)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_WRITE_PA_MODE);
  append_sl_zigbee_dhc_pa_mode_t(pa_mode);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_read_ctune(
      sl_zigbee_dhc_ctune_t *ctune)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_READ_CTUNE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_dhc_ctune_t(ctune);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_write_ctune(
      sl_zigbee_dhc_ctune_t *ctune)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_WRITE_CTUNE);
  append_sl_zigbee_dhc_ctune_t(ctune);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_read_dhc_version(
      uint8_t *dhc_version)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_READ_DHC_VERSION);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    *dhc_version = fetchInt8u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_write_dhc_version(
      uint8_t dhc_version)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_WRITE_DHC_VERSION);
  appendInt8u(dhc_version);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_read_pa_version(
      sl_zigbee_dhc_pa_version_t *pa_version)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_READ_PA_VERSION);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_dhc_pa_version_t(pa_version);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_read_pa_signature(
      sl_zigbee_dhc_pa_signature_t *pa_signature)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_READ_PA_SIGNATURE);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_dhc_pa_signature_t(pa_signature);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_write_pa_signature(
      sl_zigbee_dhc_pa_signature_t *pa_signature)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_WRITE_PA_SIGNATURE);
  append_sl_zigbee_dhc_pa_signature_t(pa_signature);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_read_pa_metadata(
      sl_zigbee_dhc_pa_metadata_t *metadata)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_READ_PA_METADATA);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    fetch_sl_zigbee_dhc_pa_metadata_t(metadata);
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

sl_status_t sl_zigbee_ezsp_write_pa_metadata(
      sl_zigbee_dhc_pa_metadata_t *metadata)
{
  sl_status_t status;
  startCommand(SL_ZIGBEE_EZSP_WRITE_PA_METADATA);
  append_sl_zigbee_dhc_pa_metadata_t(metadata);
  sl_zigbee_ezsp_status_t sendStatus = sendCommand();
  sli_zigbee_ezsp_set_last_status(sendStatus);
  if (sendStatus == SL_ZIGBEE_EZSP_SUCCESS) {
    status = fetchInt32u();
    return status;
  }
  return SL_STATUS_ZIGBEE_EZSP_ERROR;
}

static void callbackDispatch(void)
{
  callbackPointerInit();

  switch (sli_zigbee_ezsp_get_frame_id()) {

  case SL_ZIGBEE_EZSP_NO_CALLBACKS: {
    sl_zigbee_ezsp_no_callbacks();
    break;
  }

  case SL_ZIGBEE_EZSP_STACK_TOKEN_CHANGED_HANDLER: {
    uint16_t tokenAddress;
    tokenAddress = fetchInt16u();
    sl_zigbee_ezsp_stack_token_changed_handler(tokenAddress);
    break;
  }

  case SL_ZIGBEE_EZSP_TIMER_HANDLER: {
    uint8_t timerId;
    timerId = fetchInt8u();
    sl_zigbee_ezsp_timer_handler(timerId);
    break;
  }

  case SL_ZIGBEE_EZSP_COUNTER_ROLLOVER_HANDLER: {
    sl_zigbee_counter_type_t type;
    type = fetchInt8u();
    sl_zigbee_ezsp_counter_rollover_handler(type);
    break;
  }

  case SL_ZIGBEE_EZSP_MUX_INVALID_RX_HANDLER: {
    uint8_t new_rx_channel;
    uint8_t old_rx_channel;
    new_rx_channel = fetchInt8u();
    old_rx_channel = fetchInt8u();
    sl_zigbee_ezsp_mux_invalid_rx_handler(new_rx_channel, old_rx_channel);
    break;
  }

  case SL_ZIGBEE_EZSP_CUSTOM_FRAME_HANDLER: {
    uint8_t payloadLength;
    uint8_t *payload;
    payloadLength = fetchInt8u();
    payload = (uint8_t *)fetchInt8uPointer(payloadLength);
    sl_zigbee_ezsp_custom_frame_handler(payloadLength, payload);
    break;
  }

  case SL_ZIGBEE_EZSP_STACK_STATUS_HANDLER: {
    sl_status_t status;
    status = fetchInt32u();
    sl_zigbee_ezsp_stack_status_handler(status);
    break;
  }

  case SL_ZIGBEE_EZSP_ENERGY_SCAN_RESULT_HANDLER: {
    uint8_t channel;
    int8_t maxRssiValue;
    channel = fetchInt8u();
    maxRssiValue = fetchInt8();
    sl_zigbee_ezsp_energy_scan_result_handler(channel, maxRssiValue);
    break;
  }

  case SL_ZIGBEE_EZSP_NETWORK_FOUND_HANDLER: {
    sl_zigbee_zigbee_network_t networkFound;
    uint8_t lastHopLqi;
    int8_t lastHopRssi;
    fetch_sl_zigbee_zigbee_network_t(&networkFound);
    lastHopLqi = fetchInt8u();
    lastHopRssi = fetchInt8();
    sl_zigbee_ezsp_network_found_handler(&networkFound, lastHopLqi, lastHopRssi);
    break;
  }

  case SL_ZIGBEE_EZSP_SCAN_COMPLETE_HANDLER: {
    uint8_t channel;
    sl_status_t status;
    channel = fetchInt8u();
    status = fetchInt32u();
    sl_zigbee_ezsp_scan_complete_handler(channel, status);
    break;
  }

  case SL_ZIGBEE_EZSP_UNUSED_PAN_ID_FOUND_HANDLER: {
    sl_802154_pan_id_t panId;
    uint8_t channel;
    panId = fetchInt16u();
    channel = fetchInt8u();
    sl_zigbee_ezsp_unused_pan_id_found_handler(panId, channel);
    break;
  }

  case SL_ZIGBEE_EZSP_CHILD_JOIN_HANDLER: {
    uint8_t index;
    bool joining;
    sl_802154_short_addr_t childId;
    uint8_t childEui64[8];
    sl_zigbee_node_type_t childType;
    index = fetchInt8u();
    joining = fetchInt8u();
    childId = fetchInt16u();
    fetchInt8uArray(8, childEui64);
    childType = fetchInt8u();
    sl_zigbee_ezsp_child_join_handler(index, joining, childId, childEui64, childType);
    break;
  }

  case SL_ZIGBEE_EZSP_DUTY_CYCLE_HANDLER: {
    uint8_t channelPage;
    uint8_t channel;
    sl_zigbee_duty_cycle_state_t state;
    uint8_t totalDevices;
    sl_zigbee_per_device_duty_cycle_t arrayOfDeviceDutyCycles;
    channelPage = fetchInt8u();
    channel = fetchInt8u();
    state = fetchInt8u();
    totalDevices = fetchInt8u();
    fetch_sl_zigbee_per_device_duty_cycle_t(&arrayOfDeviceDutyCycles);
    sl_zigbee_ezsp_duty_cycle_handler(channelPage, channel, state, totalDevices, &arrayOfDeviceDutyCycles);
    break;
  }

  case SL_ZIGBEE_EZSP_REMOTE_SET_BINDING_HANDLER: {
    sl_zigbee_binding_table_entry_t entry;
    uint8_t index;
    sl_status_t policyDecision;
    fetch_sl_zigbee_binding_table_entry_t(&entry);
    index = fetchInt8u();
    policyDecision = fetchInt32u();
    sl_zigbee_ezsp_remote_set_binding_handler(&entry, index, policyDecision);
    break;
  }

  case SL_ZIGBEE_EZSP_REMOTE_DELETE_BINDING_HANDLER: {
    uint8_t index;
    sl_status_t policyDecision;
    index = fetchInt8u();
    policyDecision = fetchInt32u();
    sl_zigbee_ezsp_remote_delete_binding_handler(index, policyDecision);
    break;
  }

  case SL_ZIGBEE_EZSP_MESSAGE_SENT_HANDLER: {
    sl_status_t status;
    sl_zigbee_outgoing_message_type_t type;
    uint16_t indexOrDestination;
    sl_zigbee_aps_frame_t apsFrame;
    uint16_t messageTag;
    uint8_t messageLength;
    uint8_t *messageContents;
    status = fetchInt32u();
    type = fetchInt8u();
    indexOrDestination = fetchInt16u();
    fetch_sl_zigbee_aps_frame_t(&apsFrame);
    messageTag = fetchInt16u();
    messageLength = fetchInt8u();
    messageContents = (uint8_t *)fetchInt8uPointer(messageLength);
    sl_zigbee_ezsp_message_sent_handler(status, type, indexOrDestination, &apsFrame, messageTag, messageLength, messageContents);
    break;
  }

  case SL_ZIGBEE_EZSP_POLL_COMPLETE_HANDLER: {
    sl_status_t status;
    status = fetchInt32u();
    sl_zigbee_ezsp_poll_complete_handler(status);
    break;
  }

  case SL_ZIGBEE_EZSP_POLL_HANDLER: {
    sl_802154_short_addr_t childId;
    bool transmitExpected;
    childId = fetchInt16u();
    transmitExpected = fetchInt8u();
    sl_zigbee_ezsp_poll_handler(childId, transmitExpected);
    break;
  }

  case SL_ZIGBEE_EZSP_INCOMING_MESSAGE_HANDLER: {
    sl_zigbee_incoming_message_type_t type;
    sl_zigbee_aps_frame_t apsFrame;
    sl_zigbee_rx_packet_info_t packetInfo;
    uint8_t messageLength;
    uint8_t *message;
    type = fetchInt8u();
    fetch_sl_zigbee_aps_frame_t(&apsFrame);
    fetch_sl_zigbee_rx_packet_info_t(&packetInfo);
    messageLength = fetchInt8u();
    message = (uint8_t *)fetchInt8uPointer(messageLength);
    sl_zigbee_ezsp_incoming_message_handler(type, &apsFrame, &packetInfo, messageLength, message);
    break;
  }

  case SL_ZIGBEE_EZSP_INCOMING_MANY_TO_ONE_ROUTE_REQUEST_HANDLER: {
    sl_802154_short_addr_t source;
    uint8_t longId[8];
    uint8_t cost;
    source = fetchInt16u();
    fetchInt8uArray(8, longId);
    cost = fetchInt8u();
    sl_zigbee_ezsp_incoming_many_to_one_route_request_handler(source, longId, cost);
    break;
  }

  case SL_ZIGBEE_EZSP_INCOMING_ROUTE_ERROR_HANDLER: {
    sl_status_t status;
    sl_802154_short_addr_t target;
    status = fetchInt32u();
    target = fetchInt16u();
    sl_zigbee_ezsp_incoming_route_error_handler(status, target);
    break;
  }

  case SL_ZIGBEE_EZSP_INCOMING_NETWORK_STATUS_HANDLER: {
    uint8_t errorCode;
    sl_802154_short_addr_t target;
    errorCode = fetchInt8u();
    target = fetchInt16u();
    sl_zigbee_ezsp_incoming_network_status_handler(errorCode, target);
    break;
  }

  case SL_ZIGBEE_EZSP_INCOMING_ROUTE_RECORD_HANDLER: {
    sl_802154_short_addr_t source;
    uint8_t sourceEui[8];
    uint8_t lastHopLqi;
    int8_t lastHopRssi;
    uint8_t relayCount;
    uint8_t *relayList;
    source = fetchInt16u();
    fetchInt8uArray(8, sourceEui);
    lastHopLqi = fetchInt8u();
    lastHopRssi = fetchInt8();
    relayCount = fetchInt8u();
    relayList = (uint8_t *)fetchInt8uPointer(relayCount*2);
    sl_zigbee_ezsp_incoming_route_record_handler(source, sourceEui, lastHopLqi, lastHopRssi, relayCount, relayList);
    break;
  }

  case SL_ZIGBEE_EZSP_ID_CONFLICT_HANDLER: {
    sl_802154_short_addr_t id;
    id = fetchInt16u();
    sl_zigbee_ezsp_id_conflict_handler(id);
    break;
  }

  case SL_ZIGBEE_EZSP_MAC_PASSTHROUGH_MESSAGE_HANDLER: {
    sl_zigbee_mac_passthrough_type_t messageType;
    sl_zigbee_rx_packet_info_t packetInfo;
    uint8_t messageLength;
    uint8_t *messageContents;
    messageType = fetchInt8u();
    fetch_sl_zigbee_rx_packet_info_t(&packetInfo);
    messageLength = fetchInt8u();
    messageContents = (uint8_t *)fetchInt8uPointer(messageLength);
    sl_zigbee_ezsp_mac_passthrough_message_handler(messageType, &packetInfo, messageLength, messageContents);
    break;
  }

  case SL_ZIGBEE_EZSP_MAC_FILTER_MATCH_MESSAGE_HANDLER: {
    sl_zigbee_mac_filter_match_data_t filterValueMatch;
    sl_zigbee_mac_passthrough_type_t legacyPassthroughType;
    sl_zigbee_rx_packet_info_t packetInfo;
    uint8_t messageLength;
    uint8_t *messageContents;
    filterValueMatch = fetchInt16u();
    legacyPassthroughType = fetchInt8u();
    fetch_sl_zigbee_rx_packet_info_t(&packetInfo);
    messageLength = fetchInt8u();
    messageContents = (uint8_t *)fetchInt8uPointer(messageLength);
    sl_zigbee_ezsp_mac_filter_match_message_handler(filterValueMatch, legacyPassthroughType, &packetInfo, messageLength, messageContents);
    break;
  }

  case SL_ZIGBEE_EZSP_RAW_TRANSMIT_COMPLETE_HANDLER: {
    uint8_t messageLength;
    uint8_t *messageContents;
    sl_status_t status;
    messageLength = fetchInt8u();
    messageContents = (uint8_t *)fetchInt8uPointer(messageLength);
    status = fetchInt32u();
    sl_zigbee_ezsp_raw_transmit_complete_handler(messageLength, messageContents, status);
    break;
  }

  case SL_ZIGBEE_EZSP_SWITCH_NETWORK_KEY_HANDLER: {
    uint8_t sequenceNumber;
    sequenceNumber = fetchInt8u();
    sl_zigbee_ezsp_switch_network_key_handler(sequenceNumber);
    break;
  }

  case SL_ZIGBEE_EZSP_ZIGBEE_KEY_ESTABLISHMENT_HANDLER: {
    uint8_t partner[8];
    sl_zigbee_key_status_t status;
    fetchInt8uArray(8, partner);
    status = fetchInt8u();
    sl_zigbee_ezsp_zigbee_key_establishment_handler(partner, status);
    break;
  }

  case SL_ZIGBEE_EZSP_TRUST_CENTER_POST_JOIN_HANDLER: {
    sl_802154_short_addr_t newNodeId;
    uint8_t newNodeEui64[8];
    sl_zigbee_device_update_t status;
    sl_zigbee_join_decision_t policyDecision;
    sl_802154_short_addr_t parentOfNewNodeId;
    newNodeId = fetchInt16u();
    fetchInt8uArray(8, newNodeEui64);
    status = fetchInt8u();
    policyDecision = fetchInt8u();
    parentOfNewNodeId = fetchInt16u();
    sl_zigbee_ezsp_trust_center_post_join_handler(newNodeId, newNodeEui64, status, policyDecision, parentOfNewNodeId);
    break;
  }

  case SL_ZIGBEE_EZSP_GENERATE_CBKE_KEYS_HANDLER: {
    sl_status_t status;
    sl_zigbee_public_key_data_t ephemeralPublicKey;
    status = fetchInt32u();
    fetch_sl_zigbee_public_key_data_t(&ephemeralPublicKey);
    sl_zigbee_ezsp_generate_cbke_keys_handler(status, &ephemeralPublicKey);
    break;
  }

  case SL_ZIGBEE_EZSP_CALCULATE_SMACS_HANDLER: {
    sl_status_t status;
    sl_zigbee_smac_data_t initiatorSmac;
    sl_zigbee_smac_data_t responderSmac;
    status = fetchInt32u();
    fetch_sl_zigbee_smac_data_t(&initiatorSmac);
    fetch_sl_zigbee_smac_data_t(&responderSmac);
    sl_zigbee_ezsp_calculate_smacs_handler(status, &initiatorSmac, &responderSmac);
    break;
  }

  case SL_ZIGBEE_EZSP_GENERATE_CBKE_KEYS_283K1_HANDLER: {
    sl_status_t status;
    sl_zigbee_public_key_283k1_data_t ephemeralPublicKey;
    status = fetchInt32u();
    fetch_sl_zigbee_public_key_283k1_data_t(&ephemeralPublicKey);
    sl_zigbee_ezsp_generate_cbke_keys_283k1_handler(status, &ephemeralPublicKey);
    break;
  }

  case SL_ZIGBEE_EZSP_CALCULATE_SMACS_283K1_HANDLER: {
    sl_status_t status;
    sl_zigbee_smac_data_t initiatorSmac;
    sl_zigbee_smac_data_t responderSmac;
    status = fetchInt32u();
    fetch_sl_zigbee_smac_data_t(&initiatorSmac);
    fetch_sl_zigbee_smac_data_t(&responderSmac);
    sl_zigbee_ezsp_calculate_smacs_283k1_handler(status, &initiatorSmac, &responderSmac);
    break;
  }

  case SL_ZIGBEE_EZSP_DSA_SIGN_HANDLER: {
    sl_status_t status;
    uint8_t messageLength;
    uint8_t *messageContents;
    status = fetchInt32u();
    messageLength = fetchInt8u();
    messageContents = (uint8_t *)fetchInt8uPointer(messageLength);
    sl_zigbee_ezsp_dsa_sign_handler(status, messageLength, messageContents);
    break;
  }

  case SL_ZIGBEE_EZSP_DSA_VERIFY_HANDLER: {
    sl_status_t status;
    status = fetchInt32u();
    sl_zigbee_ezsp_dsa_verify_handler(status);
    break;
  }

  case SL_ZIGBEE_EZSP_MFGLIB_RX_HANDLER: {
    uint8_t linkQuality;
    int8_t rssi;
    uint8_t packetLength;
    uint8_t *packetContents;
    linkQuality = fetchInt8u();
    rssi = fetchInt8();
    packetLength = fetchInt8u();
    packetContents = (uint8_t *)fetchInt8uPointer(packetLength);
    sl_zigbee_ezsp_mfglib_rx_handler(linkQuality, rssi, packetLength, packetContents);
    break;
  }

  case SL_ZIGBEE_EZSP_INCOMING_BOOTLOAD_MESSAGE_HANDLER: {
    uint8_t longId[8];
    sl_zigbee_rx_packet_info_t packetInfo;
    uint8_t messageLength;
    uint8_t *messageContents;
    fetchInt8uArray(8, longId);
    fetch_sl_zigbee_rx_packet_info_t(&packetInfo);
    messageLength = fetchInt8u();
    messageContents = (uint8_t *)fetchInt8uPointer(messageLength);
    sl_zigbee_ezsp_incoming_bootload_message_handler(longId, &packetInfo, messageLength, messageContents);
    break;
  }

  case SL_ZIGBEE_EZSP_BOOTLOAD_TRANSMIT_COMPLETE_HANDLER: {
    sl_status_t status;
    uint8_t messageLength;
    uint8_t *messageContents;
    status = fetchInt32u();
    messageLength = fetchInt8u();
    messageContents = (uint8_t *)fetchInt8uPointer(messageLength);
    sl_zigbee_ezsp_bootload_transmit_complete_handler(status, messageLength, messageContents);
    break;
  }

  case SL_ZIGBEE_EZSP_INCOMING_MFG_TEST_MESSAGE_HANDLER: {
    uint8_t messageType;
    uint8_t dataLength;
    uint8_t data[1];
    messageType = fetchInt8u();
    dataLength = fetchInt8u();
    fetchInt8uArray(1, data);
    sl_zigbee_ezsp_incoming_mfg_test_message_handler(messageType, dataLength, data);
    break;
  }

  case SL_ZIGBEE_EZSP_ZLL_NETWORK_FOUND_HANDLER: {
    sl_zigbee_zll_network_t networkInfo;
    bool isDeviceInfoNull;
    sl_zigbee_zll_device_info_record_t deviceInfo;
    sl_zigbee_rx_packet_info_t packetInfo;
    fetch_sl_zigbee_zll_network_t(&networkInfo);
    isDeviceInfoNull = fetchInt8u();
    fetch_sl_zigbee_zll_device_info_record_t(&deviceInfo);
    fetch_sl_zigbee_rx_packet_info_t(&packetInfo);
    sl_zigbee_ezsp_zll_network_found_handler(&networkInfo, isDeviceInfoNull, &deviceInfo, &packetInfo);
    break;
  }

  case SL_ZIGBEE_EZSP_ZLL_SCAN_COMPLETE_HANDLER: {
    sl_status_t status;
    status = fetchInt32u();
    sl_zigbee_ezsp_zll_scan_complete_handler(status);
    break;
  }

  case SL_ZIGBEE_EZSP_ZLL_ADDRESS_ASSIGNMENT_HANDLER: {
    sl_zigbee_zll_address_assignment_t addressInfo;
    sl_zigbee_rx_packet_info_t packetInfo;
    fetch_sl_zigbee_zll_address_assignment_t(&addressInfo);
    fetch_sl_zigbee_rx_packet_info_t(&packetInfo);
    sl_zigbee_ezsp_zll_address_assignment_handler(&addressInfo, &packetInfo);
    break;
  }

  case SL_ZIGBEE_EZSP_ZLL_TOUCH_LINK_TARGET_HANDLER: {
    sl_zigbee_zll_network_t networkInfo;
    fetch_sl_zigbee_zll_network_t(&networkInfo);
    sl_zigbee_ezsp_zll_touch_link_target_handler(&networkInfo);
    break;
  }

  case SL_ZIGBEE_EZSP_D_GP_SENT_HANDLER: {
    sl_status_t status;
    uint8_t gpepHandle;
    status = fetchInt32u();
    gpepHandle = fetchInt8u();
    sl_zigbee_ezsp_d_gp_sent_handler(status, gpepHandle);
    break;
  }

  case SL_ZIGBEE_EZSP_GPEP_INCOMING_MESSAGE_HANDLER: {
    sl_zigbee_gp_params_t param;
    fetch_sl_zigbee_gp_params_t(&param);
    sl_zigbee_ezsp_gpep_incoming_message_handler(&param);
    break;
  }

  default:
    sl_zigbee_ezsp_error_handler(SL_ZIGBEE_EZSP_ERROR_INVALID_FRAME_ID);
  }

}

