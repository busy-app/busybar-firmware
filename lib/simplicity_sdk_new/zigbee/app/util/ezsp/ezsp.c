/***************************************************************************//**
 * @file
 * @brief Host EZSP layer. Provides functions that allow the Host
 * application to send every EZSP command to the EM260. The command and response
 * parameters are defined in the datasheet.
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

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#include "stack/include/sl_zigbee_types.h"

#include "hal/hal.h"

#include "app/util/ezsp/ezsp-protocol.h"
#include "app/util/ezsp/ezsp.h"
#include "app/util/ezsp/serial-interface.h"
#include "app/util/ezsp/ezsp-frame-utilities.h"

#ifdef EZSP_ASH
  #include "app/ezsp-host/ezsp-host-priv.h"
  #define EZSP_ASH_TRACE(...) sl_zigbee_ezsp_trace_ezsp_verbose(__VA_ARGS__)
#else
  #define EZSP_ASH_TRACE(...)
#endif

#if defined(SL_ZIGBEE_TEST)
  #define SL_ZIGBEE_TEST_ASSERT(x) assert(x)
#else
  #define SL_ZIGBEE_TEST_ASSERT(x)
#endif

//----------------------------------------------------------------
// Forward Declarations
static void startCommand(uint16_t command);
static sl_zigbee_ezsp_status_t sendCommand(void);
static void callbackDispatch(void);
static void callbackPointerInit(void);

//----------------------------------------------------------------
// Global Variables

uint8_t sli_zigbee_supported_networks = SL_ZIGBEE_SUPPORTED_NETWORKS;

uint8_t ezspSleepMode = EZSP_FRAME_CONTROL_IDLE;

static bool initialEzspVersionSent = false;
static bool sendingCommand = false;
static uint8_t ezspSequence = 0;

// Multi-network support: this variable is equivalent to the
// sli_zigbee_application_network_index vaiable for SOC. It stores the ezsp network index.
// It gets included in the frame control of every EZSP message to the NCP.
// The public APIs sl_zigbee_get_current_network() and sl_zigbee_set_current_network() set/get
// this value.
uint8_t ezspApplicationNetworkIndex = 0;

// Multi-network support: this variable is set when we receive a callback-related
// EZSP message from the NCP. The sl_zigbee_get_callback_network() API returns this
// value.
uint8_t ezspCallbackNetworkIndex = 0;

// Some callbacks from EZSP to the application include a pointer parameter. For
// example, messageContents in sl_zigbee_incoming_message_handler(). Copying the
// callback and then giving the application a pointer to this copy means it is
// safe for the application to call EZSP functions inside the callback. To save
// RAM, the application can define EZSP_DISABLE_CALLBACK_COPY. The application
// must then not read from the pointer after calling an EZSP function inside the
// callback.
#ifndef EZSP_DISABLE_CALLBACK_COPY
static uint8_t ezspCallbackStorage[EZSP_MAX_FRAME_LENGTH];
#endif

bool ncpHasCallbacks;

//------------------------------------------------------------------------------
// Retrieving the new version info

sl_status_t sl_zigbee_ezsp_get_version_struct(sl_zigbee_version_t* returnData)
{
  uint8_t data[7];  // sizeof(sl_zigbee_version_t)
  uint8_t dataLength = 7;
  sl_status_t status = sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_VERSION_INFO,
                                                &dataLength,
                                                data);

  SL_ZIGBEE_TEST_ASSERT(dataLength == 7);

  if (status == SL_STATUS_OK) {
    returnData->build   = data[0] + (((uint16_t)data[1]) << 8);
    returnData->major   = data[2];
    returnData->minor   = data[3];
    returnData->patch   = data[4];
    returnData->special = data[5];
    returnData->type    = data[6];
  }

  return status;
}

//------------------------------------------------------------------------------
// Functions for manipulating the endpoints flags on the NCP

sl_status_t sl_zigbee_ezsp_set_endpoint_flags(uint8_t endpoint,
                                              sl_zigbee_ezsp_endpoint_flags_t flags)
{
  uint8_t data[3];
  data[0] = endpoint;
  data[1] = (uint8_t)flags;
  data[2] = (uint8_t)(flags >> 8);
  return sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_ENDPOINT_FLAGS,
                                  3,
                                  data);
}

sl_status_t sl_zigbee_ezsp_get_endpoint_flags(uint8_t endpoint,
                                              sl_zigbee_ezsp_endpoint_flags_t* returnFlags)
{
  uint8_t value[2];
  uint8_t valueLength = 2;

  sl_status_t status = sl_zigbee_ezsp_get_extended_value(SL_ZIGBEE_EZSP_EXTENDED_VALUE_ENDPOINT_FLAGS,
                                                         endpoint,
                                                         &valueLength,
                                                         value);
  *returnFlags = HIGH_LOW_TO_INT(value[1], value[0]);
  return status;
}

// //----------------------------------------------------------------
// // Special Handling for AES functions.

// This is a copy of the function available on the SOC.  It would be a waste
// to have this be an actual EZSP call.
void sl_zigbee_aes_mmo_hash_init(sl_zigbee_aes_mmo_hash_context_t *context)
{
  memset(context, 0, sizeof(sl_zigbee_aes_mmo_hash_context_t));
}

// Here we convert the normal Ember AES hash call to the specialized EZSP call.
// This came about because we cannot pass a block of data that is
// both input and output into EZSP.  The block must be broken up into two
// elements.  We unify the two pieces here to make it invisible to
// the users.

static sl_status_t aesMmoHash(sl_zigbee_aes_mmo_hash_context_t *context,
                              bool finalize,
                              uint32_t length,
                              uint8_t *data)
{
  sl_zigbee_aes_mmo_hash_context_t returnData;
  sl_status_t status;
  if (length > 255) {
    return SL_STATUS_INVALID_STATE;
  }
  // In theory we could use 'context' structure as the 'returnData',
  // however that could be risky if the EZSP function tries to memset() the
  // 'returnData' prior to storing data in it.
  status = sl_zigbee_ezsp_aes_mmo_hash(context,
                                       finalize,
                                       (uint8_t)length,
                                       data,
                                       &returnData);
  memmove(context, &returnData, sizeof(sl_zigbee_aes_mmo_hash_context_t));
  return status;
}

sl_status_t sl_zigbee_aes_mmo_hash_update(sl_zigbee_aes_mmo_hash_context_t *context,
                                          uint32_t length,
                                          uint8_t *data)
{
  return aesMmoHash(context,
                    false,   // finalize?
                    length,
                    data);
}

sl_status_t sl_zigbee_aes_mmo_hash_final(sl_zigbee_aes_mmo_hash_context_t *context,
                                         uint32_t length,
                                         uint8_t *data)
{
  return aesMmoHash(context,
                    true,    // finalize?
                    length,
                    data);
}

uint8_t sl_zigbee_get_max_nwk_retries(void)
{
  uint8_t valueLength = 1;
  uint8_t data;
  sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_MAX_NWK_RETRIES, &valueLength, &data);
  return data;
}

// This is a convenience routine for hashing short blocks of data,
// less than 255 bytes.
sl_status_t sl_zigbee_aes_hash_simple(uint8_t totalLength,
                                      const uint8_t* data,
                                      uint8_t* result)
{
  sl_status_t status;
  sl_zigbee_aes_mmo_hash_context_t context;
  sl_zigbee_aes_mmo_hash_init(&context);
  status = sl_zigbee_aes_mmo_hash_final(&context,
                                        totalLength,
                                        (uint8_t*)data);
  memmove(result, context.result, 16);
  return status;
}

//------------------------------------------------------------------------------
// SOC function names that are available on the host in a different
// form to save code space.

sl_status_t sl_zigbee_set_mfg_security_config(uint32_t magicNumber,
                                              const sl_zigbee_mfg_security_struct_t* settings)
{
  uint8_t data[4 + 2];  // 4 bytes for magic number, 2 bytes for key settings
  data[0] = (uint8_t)(magicNumber         & 0xFF);
  data[1] = (uint8_t)((magicNumber >> 8)  & 0xFF);
  data[2] = (uint8_t)((magicNumber >> 16) & 0xFF);
  data[3] = (uint8_t)((magicNumber >> 24) & 0xFF);
  data[4] = (uint8_t)(settings->keySettings        & 0xFF);
  data[5] = (uint8_t)((settings->keySettings >> 8) & 0xFF);
  return sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_MFG_SECURITY_CONFIG, 6, data);
}

sl_status_t sl_zigbee_get_mfg_security_config(sl_zigbee_mfg_security_struct_t* settings)
{
  uint8_t data[2];
  uint8_t length = 2;
  sl_status_t status = sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_MFG_SECURITY_CONFIG,
                                                &length,
                                                data);
  settings->keySettings = data[0] + (data[1] << 8);
  return status;
}

sl_status_t sl_zigbee_set_child_timeout_option_mask(uint16_t mask)
{
  uint8_t value[2];
  value[0] = LOW_BYTE(mask);
  value[1] = HIGH_BYTE(mask);
  return sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_END_DEVICE_TIMEOUT_OPTIONS_MASK, 2, value);
}

sl_status_t sl_zigbee_set_keep_alive_mode(uint8_t mode)
{
  return sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_END_DEVICE_KEEP_ALIVE_SUPPORT_MODE, 1, &mode);
}

sl_zigbee_keep_alive_mode_t sl_zigbee_get_keep_alive_mode(void)
{
  uint8_t value = SL_ZIGBEE_KEEP_ALIVE_SUPPORT_UNKNOWN;
  uint8_t valueLength = 1;
  sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_END_DEVICE_KEEP_ALIVE_SUPPORT_MODE,
                           &valueLength,
                           &value);
  return value;
}

uint8_t sl_zigbee_get_network_key_timeout(void)
{
  uint8_t value = 0;
  uint8_t valueLength = 1;
  sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_NWK_KEY_TIMEOUT,
                           &valueLength,
                           &value);
  return value;
}

sl_status_t sl_zigbee_start_writing_stack_tokens(void)
{
  uint8_t i = 1;
  return sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_STACK_TOKEN_WRITING, 1, &i);
}

sl_status_t sl_zigbee_stop_writing_stack_tokens(void)
{
  uint8_t i = 0;
  return sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_STACK_TOKEN_WRITING, 1, &i);
}

bool sl_zigbee_writing_stack_tokens_enabled(void)
{
  uint8_t value;
  uint8_t valueLength = 1;
  sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_STACK_TOKEN_WRITING, &valueLength, &value);
  return value;
}

bool sl_zigbee_is_performing_rejoin(void)
{
  uint8_t value = 0;
  uint8_t valueLength = 1;
  sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_STACK_IS_PERFORMING_REJOIN,
                           &valueLength,
                           &value);
  return value;
}

sl_status_t sl_zigbee_send_remove_device(sl_802154_short_addr_t destShort,
                                         sl_802154_long_addr_t destLong,
                                         sl_802154_long_addr_t deviceToRemoveLong)
{
  return sl_zigbee_ezsp_remove_device(destShort, destLong, deviceToRemoveLong);
}

sl_status_t sl_zigbee_send_unicast_network_key_update(sl_802154_short_addr_t targetShort,
                                                      sl_802154_long_addr_t  targetLong,
                                                      const sl_zigbee_key_data_t* newKey)
{
  return sl_zigbee_ezsp_unicast_nwk_key_update(targetShort,
                                               targetLong,
                                               (sl_zigbee_key_data_t*)newKey);
}

sl_status_t sl_zigbee_set_extended_security_bitmask(sl_zigbee_extended_security_bitmask_t mask)
{
  uint8_t value[2];
  value[0] = LOW_BYTE(mask);
  value[1] = HIGH_BYTE(mask);
  if (sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_EXTENDED_SECURITY_BITMASK, 2, value)
      == SL_ZIGBEE_EZSP_SUCCESS) {
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_INVALID_STATE;
  }
}

sl_status_t sl_zigbee_get_extended_security_bitmask(sl_zigbee_extended_security_bitmask_t* mask)
{
  uint8_t value[2];
  uint8_t valueLength = 2;
  if (sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_EXTENDED_SECURITY_BITMASK, &valueLength, value)
      == SL_STATUS_OK) {
    *mask = HIGH_LOW_TO_INT(value[1], value[0]);
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_INVALID_STATE;
  }
}

sl_status_t sl_zigbee_set_node_id(sl_802154_short_addr_t nodeId)
{
  uint8_t value[2];
  value[0] = LOW_BYTE(nodeId);
  value[1] = HIGH_BYTE(nodeId);
  if (sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_NODE_SHORT_ID, 2, value) == SL_STATUS_OK) {
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_INVALID_STATE;
  }
}

sl_status_t sl_zigbee_set_node_descriptor(sl_zigbee_node_descriptor_info_t *descriptor)
{
  uint8_t value[2];
  value[0] = LOW_BYTE(descriptor->max_incoming_transfer_size);
  value[1] = HIGH_BYTE(descriptor->max_incoming_transfer_size);

  if (sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_MAXIMUM_INCOMING_TRANSFER_SIZE, 2, value) == SL_STATUS_OK) {
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_INVALID_STATE;
  }

  value[0] = LOW_BYTE(descriptor->max_outgoing_transfer_size);
  value[1] = HIGH_BYTE(descriptor->max_outgoing_transfer_size);

  if (sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_MAXIMUM_OUTGOING_TRANSFER_SIZE, 2, value) == SL_STATUS_OK) {
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_INVALID_STATE;
  }

  value[0] = descriptor->capability;

  if (sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_DESCRIPTOR_CAPABILITY, 1, value) == SL_STATUS_OK) {
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_INVALID_STATE;
  }
}

sl_status_t sl_zigbee_get_node_descriptor(sl_zigbee_node_descriptor_info_t *descriptor)
{
  sl_status_t status;
  uint8_t valueLength = 1;
  status = sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_DESCRIPTOR_CAPABILITY,
                                    &valueLength,
                                    (uint8_t*)&(descriptor->capability));
  if (status != SL_STATUS_OK) {
    return status;
  }
  valueLength = 2;
  status = sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_MAXIMUM_INCOMING_TRANSFER_SIZE,
                                    &valueLength,
                                    (uint8_t*)&(descriptor->max_incoming_transfer_size));
  if (status != SL_STATUS_OK) {
    return status;
  }
  status = sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_MAXIMUM_OUTGOING_TRANSFER_SIZE,
                                    &valueLength,
                                    (uint8_t*)&(descriptor->max_outgoing_transfer_size));
  return status;
}

uint8_t sl_zigbee_get_last_stack_zig_dev_request_sequence(void)
{
  uint8_t value = 0;
  uint8_t valueLength = 1;
  sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_STACK_DEVICE_REQUEST_SEQUENCE_NUMBER,
                           &valueLength,
                           &value);
  return value;
}

uint8_t sl_zigbee_get_current_network(void)
{
  return ezspApplicationNetworkIndex;
}

uint8_t sl_zigbee_get_callback_network(void)
{
  return ezspCallbackNetworkIndex;
}

sl_status_t sl_zigbee_set_current_network(uint8_t index)
{
  if (index < sli_zigbee_supported_networks) {
    ezspApplicationNetworkIndex = index;
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_INVALID_INDEX;
  }
}

sl_status_t sl_zigbee_find_and_rejoin_network(bool haveCurrentNetworkKey,
                                              uint32_t channelMask,
                                              sl_zigbee_rejoin_reason_t reason,
                                              sl_zigbee_node_type_t nodeType)
{
  return sl_zigbee_ezsp_find_and_rejoin_network(haveCurrentNetworkKey,
                                                channelMask, reason, nodeType);
}

sl_zigbee_rejoin_reason_t sl_zigbee_get_last_rejoin_reason(void)
{
  sl_zigbee_rejoin_reason_t reason = SL_ZIGBEE_REJOIN_REASON_NONE;
  uint8_t length = 1;
  sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_LAST_REJOIN_REASON,
                           &length,
                           &reason);
  return reason;
}

sl_zigbee_leave_reason_t sl_zigbee_get_last_leave_reason(sl_802154_short_addr_t * id)
{
  uint8_t length = 3;
  uint8_t data[3];
  sl_zigbee_ezsp_get_extended_value(SL_ZIGBEE_EZSP_EXTENDED_VALUE_LAST_LEAVE_REASON,
                                    0,   // characteristics
                                    &length,
                                    data);
  if (id != NULL) {
    *id = (((uint16_t)data[1])
           + ((uint16_t)(data[2] << 8)));
  }

  return data[0];
}
uint8_t sl_zigbee_ezsp_get_source_route_overhead(sl_802154_short_addr_t destination)
{
  uint8_t valueLength = 1;
  uint8_t value;
  sl_zigbee_ezsp_get_extended_value(SL_ZIGBEE_EZSP_EXTENDED_VALUE_GET_SOURCE_ROUTE_OVERHEAD,
                                    destination,   // characteristics
                                    &valueLength,
                                    &value);
  return value;
}

uint32_t sl_zigbee_ezsp_get_memory_usage_data(uint32_t type)
{
  uint8_t valueLength = 4;
  uint8_t value[valueLength];
  sl_zigbee_ezsp_get_extended_value(SL_ZIGBEE_EZSP_EXTENDED_VALUE_MEMORY_USAGE_DATA,
                                    type,   // characteristics
                                    &valueLength,
                                    value);
  uint32_t memory_usage_value = ((uint32_t)value[0])
                                + ((uint32_t)(value[1] << 8))
                                + ((uint32_t)(value[2] << 16))
                                + ((uint32_t)(value[3] << 24));
  return memory_usage_value;
}

sl_status_t sl_zigbee_set_preinstalled_cbke_data_283k1(sl_zigbee_public_key_283k1_data_t * caPublic,
                                                       sl_zigbee_certificate_283k1_data_t * myCert,
                                                       sl_zigbee_private_key_283k1_data_t * myKey)
{
  sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_CERTIFICATE_283K1, SL_ZIGBEE_CERTIFICATE_283K1_SIZE, sl_zigbee_certificate_283k1_contents(myCert));
  sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_PUBLIC_KEY_283K1, SL_ZIGBEE_PUBLIC_KEY_283K1_SIZE, sl_zigbee_public_key_283k1_contents(caPublic));
  sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_PRIVATE_KEY_283K1, SL_ZIGBEE_PRIVATE_KEY_283K1_SIZE, sl_zigbee_private_key_283k1_contents(myKey));
  return sl_zigbee_ezsp_save_preinstalled_cbke_data_283k1();
}

//------------------------------------------------------------------------------

#include "command-functions.h"

//----------------------------------------------------------------
// EZSP Utilities
static void startCommand(uint16_t command)
{
  // Send initial SL_ZIGBEE_EZSP_VERSION command with old packet format for old Hosts/NCPs
  if (command == SL_ZIGBEE_EZSP_VERSION && !initialEzspVersionSent) {
    ezspWritePointer = ezspFrameContents + EZSP_PARAMETERS_INDEX;
    serialSetCommandByte(EZSP_FRAME_ID_INDEX, LOW_BYTE(command));
  } else {
    // convert to extended frame format
    ezspWritePointer = ezspFrameContents + EZSP_EXTENDED_PARAMETERS_INDEX;
    serialSetCommandByte(EZSP_EXTENDED_FRAME_ID_LB_INDEX, LOW_BYTE(command));
    serialSetCommandByte(EZSP_EXTENDED_FRAME_ID_HB_INDEX, HIGH_BYTE(command));
  }
}

static uint8_t responseReceived(void)
{
  sl_zigbee_ezsp_status_t status;
  uint16_t responseFrameControl;
  uint16_t responseFrameId;
  uint8_t parametersIndex;

  status = serialResponseReceived();

  if (status == SL_ZIGBEE_EZSP_SPI_WAITING_FOR_RESPONSE
      || status == SL_ZIGBEE_EZSP_NO_RX_DATA) {
    //tell sendCommand that response is not yet received
    return SL_ZIGBEE_EZSP_NO_RX_DATA;
  }

  if ((serialGetResponseByte(EZSP_EXTENDED_FRAME_CONTROL_HB_INDEX) & EZSP_EXTENDED_FRAME_FORMAT_VERSION_MASK)
      == EZSP_EXTENDED_FRAME_FORMAT_VERSION) {
    // use extended ezsp frame format
    responseFrameControl = sli_zigbee_ezsp_get_frame_control();
    responseFrameId = sli_zigbee_ezsp_get_frame_id();
    parametersIndex = EZSP_EXTENDED_PARAMETERS_INDEX;
    if (HIGH_BYTE(responseFrameControl) & EZSP_EXTENDED_FRAME_CONTROL_RESERVED_MASK) {
      // reject if unsupported frame
      status = SL_ZIGBEE_EZSP_ERROR_UNSUPPORTED_CONTROL;
    }
  } else {
    // use legacy ezsp frame format
    responseFrameControl = serialGetResponseByte(EZSP_FRAME_CONTROL_INDEX);
    responseFrameId = serialGetResponseByte(EZSP_FRAME_ID_INDEX);
    parametersIndex = EZSP_PARAMETERS_INDEX;
  }

  if (status == SL_ZIGBEE_EZSP_SUCCESS) {
    if (responseFrameId == SL_ZIGBEE_EZSP_INVALID_COMMAND) {
      status = serialGetResponseByte(parametersIndex);
    }

    if ((responseFrameControl & EZSP_FRAME_CONTROL_DIRECTION_MASK)
        != EZSP_FRAME_CONTROL_RESPONSE) {
      status = SL_ZIGBEE_EZSP_ERROR_WRONG_DIRECTION;
    }
    if ((responseFrameControl & EZSP_FRAME_CONTROL_TRUNCATED_MASK)
        == EZSP_FRAME_CONTROL_TRUNCATED) {
      status = SL_ZIGBEE_EZSP_ERROR_TRUNCATED;
    }
    if ((responseFrameControl & EZSP_FRAME_CONTROL_OVERFLOW_MASK)
        == EZSP_FRAME_CONTROL_OVERFLOW) {
      status = SL_ZIGBEE_EZSP_ERROR_OVERFLOW;
    }
    if ((responseFrameControl & EZSP_FRAME_CONTROL_PENDING_CB_MASK)
        == EZSP_FRAME_CONTROL_PENDING_CB) {
      ncpHasCallbacks = true;
    } else {
      ncpHasCallbacks = false;
    }

    // Set the callback network
    ezspCallbackNetworkIndex =
      (responseFrameControl & EZSP_FRAME_CONTROL_NETWORK_INDEX_MASK)
      >> EZSP_FRAME_CONTROL_NETWORK_INDEX_OFFSET;
  }

  if (status != SL_ZIGBEE_EZSP_SUCCESS) {
    EZSP_ASH_TRACE("responseReceived(): sl_zigbee_ezsp_error_handler(): 0x%02X", status);
    sl_zigbee_ezsp_error_handler(status);
  }

  ezspReadPointer = ezspFrameContents + parametersIndex;

  // An overflow does not indicate a comms failure;
  // The system can still communicate but buffers are running critically low.
  // This is almost always due to network congestion and goes away
  // when the network becomes quieter.
  if (status == SL_ZIGBEE_EZSP_ERROR_OVERFLOW) {
    return SL_ZIGBEE_EZSP_SUCCESS;
  }
  return status;
}

static sl_zigbee_ezsp_status_t sendCommand(void)
{
  sl_zigbee_ezsp_status_t status;
  serialSetCommandByte(EZSP_SEQUENCE_INDEX, ezspSequence);
  ezspSequence++;
  serialSetCommandByte(EZSP_EXTENDED_FRAME_CONTROL_LB_INDEX,
                       (EZSP_FRAME_CONTROL_COMMAND
                        | (ezspSleepMode & EZSP_FRAME_CONTROL_SLEEP_MODE_MASK)
                        | ((ezspApplicationNetworkIndex   // we always set the network index in the
                            << EZSP_FRAME_CONTROL_NETWORK_INDEX_OFFSET)
                           & EZSP_FRAME_CONTROL_NETWORK_INDEX_MASK)));   // ezsp frame control.

  // Send initial SL_ZIGBEE_EZSP_VERSION command with old packet format for old Hosts/NCPs
  if (serialGetResponseByte(EZSP_FRAME_ID_INDEX) == SL_ZIGBEE_EZSP_VERSION && (!initialEzspVersionSent)) {
    initialEzspVersionSent = true;
  } else {
    serialSetCommandByte(EZSP_EXTENDED_FRAME_CONTROL_HB_INDEX, EZSP_EXTENDED_FRAME_FORMAT_VERSION);
  }

  uint16_t length = ezspWritePointer - ezspFrameContents;
  if (length > EZSP_MAX_FRAME_LENGTH) {
    sl_zigbee_ezsp_error_handler(SL_ZIGBEE_EZSP_ERROR_COMMAND_TOO_LONG);
    return SL_ZIGBEE_EZSP_ERROR_COMMAND_TOO_LONG;
  }
  serialSetCommandLength(length);
  // Ensure that a second command is not sent before the response to the first
  // command has been processed.
  assert(!sendingCommand);
  sendingCommand = true;
  status = serialSendCommand();
  if (status == SL_ZIGBEE_EZSP_SUCCESS) {
    status = responseReceived();
    while (status == SL_ZIGBEE_EZSP_NO_RX_DATA) {
      status = responseReceived();
      sl_zigbee_ezsp_waiting_for_response();
    }
  } else {
    EZSP_ASH_TRACE("sendCommand(): sl_zigbee_ezsp_error_handler(): 0x%02X", status);
    sl_zigbee_ezsp_error_handler(status);
  }
  sendingCommand = false;
  return status;
}

static void callbackPointerInit(void)
{
#ifndef EZSP_DISABLE_CALLBACK_COPY
  uint8_t parametersIndex = EZSP_PARAMETERS_INDEX;

  if ((serialGetResponseByte(EZSP_EXTENDED_FRAME_CONTROL_HB_INDEX) & EZSP_EXTENDED_FRAME_FORMAT_VERSION_MASK)
      == EZSP_EXTENDED_FRAME_FORMAT_VERSION) {
    parametersIndex = EZSP_EXTENDED_PARAMETERS_INDEX;
  }

  memmove(ezspCallbackStorage, ezspFrameContents, EZSP_MAX_FRAME_LENGTH);
  ezspReadPointer = ezspCallbackStorage + parametersIndex;
#endif
}

void sl_zigbee_ezsp_tick(void)
{
  uint8_t count = serialPendingResponseCount() + 1;
  // Ensure that we are not being called from within a command.
  assert(!sendingCommand);
  while (count > 0 && responseReceived() == SL_ZIGBEE_EZSP_SUCCESS) {
    callbackDispatch();
    count--;
  }
  simulatedTimePasses();
}

// ZLL methods

sl_status_t zllNetworkOps(sl_zigbee_zll_network_t * networkInfo,
                          sl_zigbee_ezsp_zll_network_operation_t op,
                          uint8_t radioTxPower)
{
  return sl_zigbee_ezsp_zll_network_ops(networkInfo, op, radioTxPower);
}

/** @brief This will set the device type as a router or end device
 * (depending on the passed nodeType) and setup a ZLL
 * commissioning network with the passed parameters.  If panId is 0xFFFF,
 * a random PAN ID will be generated.  If extendedPanId is set to all F's,
 * then a random extended pan ID will be generated.  If channel is 0xFF,
 * then channel 11 will be used.
 * If all F values are passed for PAN Id or Extended PAN ID then the
 * randomly generated values will be returned in the passed structure.
 *
 * @param networkInfo A pointer to an ::sl_zigbee_zll_network_t struct indicating the
 *   network parameters to use when forming the network.  If random values are
 *   requested, the stack's randomly generated values will be returned in the
 *   structure.
 * @param radioTxPower the radio output power at which a node is to operate.
 *
 * @return An ::sl_status_t value indicating whether the operation
 *   succeeded, or why it failed.
 */
sl_status_t sl_zigbee_zll_form_network(sl_zigbee_zll_network_t * networkInfo,
                                       int8_t radioTxPower)
{
  return zllNetworkOps(networkInfo, SL_ZIGBEE_EZSP_ZLL_FORM_NETWORK, radioTxPower);
}

/** @brief This call will cause the device to send a NWK start or join to the
 *  target device and cause the remote AND local device to start operating
 *  on a network together.  If the local device is a factory new device
 *  then it will send a ZLL NWK start to the target requesting that the
 *  target generate new network parameters.  If the device is
 *  not factory new then the local device will send a NWK join request
 *  using the current network parameters.
 *
 * @param targetNetworkInfo A pointer to an ::sl_zigbee_zll_network_t structure that
 *   indicates the info about what device to send the NWK start/join
 *   request to.  This information must have previously been returned
 *   from a ZLL scan.
 *
 * @return An ::sl_status_t value indicating whether the operation
 *   succeeded, or why it failed.
 */
sl_status_t sl_zigbee_zll_join_target(const sl_zigbee_zll_network_t * targetNetworkInfo)
{
  return zllNetworkOps((sl_zigbee_zll_network_t *) targetNetworkInfo, SL_ZIGBEE_EZSP_ZLL_JOIN_TARGET, 0);
}

void sl_zigbee_zll_get_token_stack_zll_data(sl_zigbee_tok_type_stack_zll_data_t * token)
{
  sl_zigbee_tok_type_stack_zll_security_t security;
  zllGetTokens(token, &security);
}

void sl_zigbee_zll_get_token_stack_zll_security(sl_zigbee_tok_type_stack_zll_security_t * token)
{
  sl_zigbee_tok_type_stack_zll_data_t data;
  zllGetTokens(&data, token);
}

// This function is used to pull the source Routing Overhead from the ezsp
// command frame SL_ZIGBEE_EZSP_INCOMING_MESSAGE_HANDLER.
uint8_t getSourceRouteOverhead(uint8_t messageLength)
{
  UNUSED_VAR(messageLength);
  return fetchInt8u();
}

sl_status_t sl_zigbee_get_current_duty_cycle(uint8_t maxDevices,
                                             sl_zigbee_per_device_duty_cycle_t * arrayOfDeviceDutyCycles)
{
  if (arrayOfDeviceDutyCycles == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // maxSize of arrayOfDeviceDutyCycles = SL_ZIGBEE_MAX_END_DEVICE_CHILDREN (32)
  //                                    + 1 (local device).
  // In theory SL_ZIGBEE_MAX_END_DEVICE_CHILDREN can be 64 but
  // duty cycle feature limits to 32 end devices.
  // max bytes over ezsp = 33 * 4 (size of sl_zigbee_per_device_duty_cycle_t)
  //                       + 1 (length of actual payload)
  //                     = 134 (bytes pointer --> perDeviceDutyCycles)
  uint8_t perDeviceDutyCyclesInBytes[134];
  uint8_t status;

  status = sl_zigbee_ezsp_get_current_duty_cycle(maxDevices, perDeviceDutyCyclesInBytes);

  if (status == SL_ZIGBEE_EZSP_SUCCESS) {
    format_bytes_in_to_struct_sl_zigbee_per_device_duty_cycle_t(perDeviceDutyCyclesInBytes, arrayOfDeviceDutyCycles);
  }

  return status;
}

sl_status_t sl_zigbee_set_duty_cycle_limits_in_stack(const sl_zigbee_duty_cycle_limits_t * limits)
{
  if (limits == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_zigbee_duty_cycle_limits_t copyLimits;

  copyLimits.limitThresh = limits->limitThresh;
  copyLimits.critThresh = limits->critThresh;
  copyLimits.suspLimit = limits->suspLimit;

  return sl_zigbee_ezsp_set_duty_cycle_limits_in_stack(&copyLimits);
}

sl_status_t sl_zigbee_setup_delayed_join(uint8_t networkKeyTimeoutS)
{
  return sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_NWK_KEY_TIMEOUT, 1, &networkKeyTimeoutS);
}

int8_t sl_zigbee_get_radio_power(void)
{
  sl_zigbee_multi_phy_radio_parameters_t parameters;
  //does this need to support multi-PHY as well?
  (void)sl_zigbee_ezsp_get_radio_parameters(0, &parameters);
  return parameters.radioTxPower;
}

uint8_t sl_zigbee_next_zigbee_sequence_number(void)
{
  uint8_t value = 0;
  uint8_t valueLength = 1;
  sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_NEXT_ZIGBEE_SEQUENCE_NUMBER,
                           &valueLength,
                           &value);
  return value;
}

sl_status_t sl_zigbee_set_tx_power_mode(uint16_t mode)
{
  return sl_zigbee_ezsp_set_configuration_value(SL_ZIGBEE_EZSP_CONFIG_TX_POWER_MODE,
                                                mode);
}

uint8_t sl_zigbee_get_rejoin_mode(void)
{
  uint8_t value = 0;
  uint8_t valueLength = 1;
  sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_REJOIN_MODE,
                           &valueLength,
                           &value);
  return value;
}

void sl_zigbee_set_rejoin_mode(uint8_t mode)
{
  (void) sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_RETRY_DEVICE_TYPE,
                                  1,
                                  &mode);
}

sl_status_t sl_zigbee_set_tc_rejoins_using_well_known_key_allowed(bool allow)
{
  return sl_zigbee_ezsp_set_policy(SL_ZIGBEE_EZSP_TC_REJOINS_USING_WELL_KNOWN_KEY_POLICY, (allow));
}

sl_status_t sl_zigbee_set_tc_rejoins_using_well_known_key_timeout_sec(uint16_t timeout_sec)
{
  return sl_zigbee_ezsp_set_configuration_value(SL_ZIGBEE_EZSP_CONFIG_TC_REJOINS_USING_WELL_KNOWN_KEY_TIMEOUT_S, (timeout_sec));
}
