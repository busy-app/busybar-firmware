/***************************************************************************//**
 * @file
 * @brief Routines for the GBCS Message Controller plugin.
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

#include <stdlib.h>     // malloc, free
#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "gbz-message-controller.h"

// forward declarations
static uint16_t append_in_new_mem(sl_zigbee_af_gbz_message_creator_state_t * state,
                                  sl_zigbee_af_gbz_zcl_command_t * zclCmd,
                                  sl_zigbee_af_gbz_message_data_t * data);
static uint16_t append_in_place(sl_zigbee_af_gbz_message_creator_state_t * state,
                                sl_zigbee_af_gbz_zcl_command_t * zclCmd,
                                sl_zigbee_af_gbz_message_data_t * data);
static uint16_t get_length(sl_zigbee_af_gbz_zcl_command_t * cmd, sl_zigbee_af_gbz_message_data_t * msg);
static uint16_t get_responses_length(sli_zigbee_af_gbz_use_case_specific_component * comp);
static uint16_t append_response(uint8_t * dst,
                                uint16_t maxLen,
                                sli_zigbee_af_gbz_use_case_specific_component * responses);
static void free_response(sl_zigbee_af_gbz_message_creator_state_t * comp);
static void free_header(sli_zigbee_af_gbz_payload_header * header);

void sl_zigbee_af_gbz_message_controller_next_command(sl_zigbee_af_gbz_message_parser_state_t * state,
                                                      sl_zigbee_af_gbz_zcl_command_t * gbzZclCommand)
{
  if (state == NULL || gbzZclCommand == NULL) {
    sl_zigbee_af_gbz_message_controller_println("GBZ: ERR: invalid pointers");
    return;
  }

  memset(gbzZclCommand, 0x00, sizeof(sl_zigbee_af_gbz_zcl_command_t));
  if (state->componentsParsed < state->componentsSize) {
    uint8_t extendedHeaderControlField = 0;
    uint16_t gbzCmdLength = 0;
    bool encryption = false;

    // Most significant nibble:
    //  0x0 if 'From Date Time' not present
    //  Or
    //  0x1 if 'From Date Time' present

    //  Least significant nibble:
    //  0x0 (if not the last GBZ Use Case Specific Component in this Message)
    //  0x1 (if the last GBZ Use Case Specific Component in this Message)
    //  0x02(encrypted) (if not the last GBZ Use Case Specific Component in this Message)
    //  0x03(encrypted) (if the last GBZ Use Case Specific Component in this Message)

    extendedHeaderControlField = (uint8_t) state->command[state->parseIndex++];

    if (extendedHeaderControlField & GAS_PROXY_FUNCTION_GBZ_COMPONENT_ENCRYPTED_MSG_MASK) {
      encryption = true;
    } else {
      encryption = false;
    }

    if (extendedHeaderControlField & GAS_PROXY_FUNCTION_GBZ_COMPONENT_FROM_DATE_TIME_MASK) {
      gbzZclCommand->hasFromDateTime = true;
    } else {
      gbzZclCommand->hasFromDateTime = false;
    }

    gbzZclCommand->clusterId = sl_util_fetch_high_low_int16u(state->command + state->parseIndex);
    state->parseIndex += 2;

    gbzCmdLength = sl_util_fetch_high_low_int16u(state->command + state->parseIndex);
    state->parseIndex += 2;

    if (gbzZclCommand->hasFromDateTime) {
      gbzZclCommand->fromDateTime = sl_util_fetch_high_low_int32u(state->command + state->parseIndex);
      state->parseIndex += 4;
    }

    gbzZclCommand->frameControl = (uint8_t) state->command[state->parseIndex++];
    gbzZclCommand->clusterSpecific = gbzZclCommand->frameControl & ZCL_CLUSTER_SPECIFIC_COMMAND;
    gbzZclCommand->mfgSpecific = gbzZclCommand->frameControl & ZCL_MANUFACTURER_SPECIFIC_MASK;
    gbzZclCommand->direction = ((gbzZclCommand->frameControl & ZCL_FRAME_CONTROL_DIRECTION_MASK)
                                ? ZCL_DIRECTION_SERVER_TO_CLIENT
                                : ZCL_DIRECTION_CLIENT_TO_SERVER);

    gbzZclCommand->transactionSequenceNumber = state->command[state->parseIndex++];
    gbzZclCommand->commandId = state->command[state->parseIndex++];

    if (encryption) {
      sl_zigbee_af_gbz_message_data_t data;
      data.encryption = true;

      data.encryptedPayloadLength = sl_util_fetch_high_low_int16u(state->command + state->parseIndex);
      data.encryptedPayload = &state->command[state->parseIndex + 2];

      sl_zigbee_af_gbz_message_controller_decrypt_data_cb(&data);
      if (data.encryption) {
        sl_zigbee_af_gbz_message_controller_println("GBZ: ERR: Unable to decrypt ZCL payload: ");
        sl_zigbee_af_gbz_message_controller_print_buffer(data.encryptedPayload,
                                                         data.encryptedPayloadLength,
                                                         true);

        gbzZclCommand->payload = data.encryptedPayload;
        gbzZclCommand->payloadLength = data.encryptedPayloadLength;
      } else {
        // decryption successful
        memcpy(&state->command[state->parseIndex + 2], data.plainPayload, data.plainPayloadLength);
        gbzZclCommand->payload = &state->command[state->parseIndex + 2];
        gbzZclCommand->payloadLength = data.plainPayloadLength;
      }
    } else {
      gbzZclCommand->payload = &state->command[state->parseIndex];
      gbzZclCommand->payloadLength = gbzCmdLength - GAS_PROXY_FUNCTION_GBZ_COMPONENT_ZCL_HEADER_LENGTH;
    }
    state->parseIndex += gbzCmdLength - 3;
  }
  state->componentsParsed += 1;
}

/*
 * @ Get ZCL portions (header & payload) of the GBZ messages' command length
 */
static uint16_t get_command_length(sl_zigbee_af_gbz_zcl_command_t * cmd,
                                   sl_zigbee_af_gbz_message_data_t * data)
{
  uint16_t len = get_length(cmd, data);
  if (len != 0) {
    return len - GAS_PROXY_FUNCTION_GBZ_COMPONENT_EXT_HEADER_FIELDS_LENGTH;
  } else {
    return len;
  }
}

/*
 * @ Get the length of the overall GBZ message
 */
static uint16_t get_length(sl_zigbee_af_gbz_zcl_command_t * cmd,
                           sl_zigbee_af_gbz_message_data_t * msg)
{
  uint16_t len = GAS_PROXY_FUNCTION_GBZ_COMPONENT_EXT_HEADER_FIELDS_LENGTH;

  // this isn't a valid configuration according to the GBCS spec.
  if (cmd->hasFromDateTime && cmd->encryption) {
    return 0;
  }

  if (cmd->encryption) {
    len += GAS_PROXY_FUNCTION_GBZ_COMPONENT_ENCRYPTION_HEADER_FIELDS_LENGTH;
  } else if (cmd->hasFromDateTime) {
    len += GAS_PROXY_FUNCTION_GBZ_COMPONENT_FROM_DATE_TIME_LENGTH;
  }

  len += GAS_PROXY_FUNCTION_GBZ_COMPONENT_ZCL_HEADER_LENGTH;

  if (cmd->encryption) {
    len += GAS_PROXY_FUNCTION_GBZ_COMPONENT_ENCRYPTION_CIPHERED_INFO_LENGTH;
  }

  if (msg != NULL && msg->encryption) {
    len += msg->encryptedPayloadLength;
  } else {
    len += cmd->payloadLength;
  }

  return len;
}

void sl_zigbee_af_gbz_message_controller_print_command_info(sl_zigbee_af_gbz_zcl_command_t * cmd)
{
  sl_zigbee_af_gbz_message_controller_println("GBZ: ZCL command: cluster id: 0x%04X", cmd->clusterId);
  sl_zigbee_af_gbz_message_controller_println("GBZ: ZCL command: command id: 0x%02X", cmd->commandId);
  sl_zigbee_af_gbz_message_controller_println("GBZ: ZCL command: frame control: 0x%02X", cmd->frameControl);
  sl_zigbee_af_gbz_message_controller_println("GBZ: ZCL command: direction: 0x%02X", cmd->direction);
  sl_zigbee_af_gbz_message_controller_println("GBZ: ZCL command: cluster specific: 0x%02X", cmd->clusterSpecific);
  sl_zigbee_af_gbz_message_controller_println("GBZ: ZCL command: mfg specific: 0x%02X", cmd->mfgSpecific);
  sl_zigbee_af_gbz_message_controller_println("GBZ: ZCL command: trans. seq. number: 0x%02X", cmd->transactionSequenceNumber);
  sl_zigbee_af_gbz_message_controller_print("GBZ: ZCL command: payload: ");
  sl_zigbee_af_gbz_message_controller_print_buffer(cmd->payload, cmd->payloadLength, true);
  sl_zigbee_af_gbz_message_controller_println("");
}

bool sl_zigbee_af_gbz_message_controller_has_next_command(sl_zigbee_af_gbz_message_parser_state_t * state)
{
  if (state == NULL) {
    return false;
  }
  return (state->componentsParsed < state->componentsSize);
}

uint8_t sl_zigbee_af_gbz_message_controller_get_component_size(sl_zigbee_af_gbz_message_parser_state_t * state)
{
  if (state == NULL) {
    return 0;
  }
  return (state->componentsSize);
}

bool sl_zigbee_af_gbz_message_controller_parser_init(sl_zigbee_af_gbz_message_parser_state_t * state,
                                                     sl_zigbee_af_gbz_message_type_t type,
                                                     uint8_t * gbzCommand,
                                                     uint16_t gbzCommandLength,
                                                     bool copyGbzCommand,
                                                     uint16_t messageCode)
{
  uint16_t profileId;
  uint8_t *gbzCommandCopy;

  if (state == NULL) {
    return false;
  }

  memset(state, 0x00, sizeof(sl_zigbee_af_gbz_message_parser_state_t));

  profileId =  (gbzCommand[0] << 8) | gbzCommand[1];

  if (profileId != SE_PROFILE_ID) {
    sl_zigbee_af_gbz_message_controller_println("GBZ: ERR: invalid profile id: 0x%04X ", profileId);
    return false;
  }

  if (copyGbzCommand) {
    gbzCommandCopy = (uint8_t *) malloc(gbzCommandLength);
    if (gbzCommandCopy == NULL) {
      sl_zigbee_af_gbz_message_controller_println("GBZ: ERR: cannot malloc %d bytes", gbzCommandLength);
      return false;
    }
    memcpy(gbzCommandCopy, gbzCommand, gbzCommandLength);
  } else {
    gbzCommandCopy = gbzCommand;
  }

  state->messageCode = messageCode;
  state->type = type;
  state->nextComponentZclSequenceNumber = 0;
  state->componentsParsed = 0;
  state->freeRequired = copyGbzCommand;
  state->command = gbzCommandCopy;
  state->parseIndex = 2; // skip profileId since we already extracted it
  state->length = gbzCommandLength;
  state->profileId = profileId;
  state->componentsSize = state->command[state->parseIndex];
  state->parseIndex += 1;

  if (type == SL_ZIGBEE_AF_GBZ_MESSAGE_ALERT) {
    state->alertCode = sl_util_fetch_high_low_int16u(state->command + state->parseIndex);
    state->parseIndex += 2;
    state->alertTimestamp = sl_util_fetch_high_low_int32u(state->command + state->parseIndex);
    state->parseIndex += 4;
  }

  return true;
}

void sl_zigbee_af_gbz_message_controller_parser_cleanup(sl_zigbee_af_gbz_message_parser_state_t * state)
{
  if (state->freeRequired) {
    free(state->command);
  }
  memset(state, 0x00, sizeof(sl_zigbee_af_gbz_message_parser_state_t));
}

uint16_t sl_zigbee_af_gbz_message_controller_creator_init(sl_zigbee_af_gbz_message_creator_state_t * state,
                                                          sl_zigbee_af_gbz_message_type_t type,
                                                          uint16_t alertCode,
                                                          uint32_t timestamp,
                                                          uint16_t messageCode,
                                                          uint8_t * gbzCommand,
                                                          uint16_t gbzCommandLength)
{
  uint8_t totalByteWritten = 0;
  if (state == NULL) {
    return 0;
  }

  // init
  memset(state, 0x00, sizeof(sl_zigbee_af_gbz_message_creator_state_t));

  if (gbzCommand == NULL) { //inits for appends that requires mem allocation
    // inits
    uint8_t headerSize = 0;
    uint8_t * headerPayload = NULL;
    uint8_t headerWriteIndex = 0;

    state->allocateMemoryForResponses = true;
    state->responses = NULL;

    if (type == SL_ZIGBEE_AF_GBZ_MESSAGE_ALERT) {
      headerSize = GAS_PROXY_FUNCTION_GBZ_MESSAGE_ALERT_HEADER_LENGTH;
    } else {
      headerSize = GAS_PROXY_FUNCTION_GBZ_MESSAGE_COMMAND_HEADER_LENGTH;
    }

    // allocate mem for the header struct / header payload
    state->header = (sli_zigbee_af_gbz_payload_header *)malloc(sizeof(sli_zigbee_af_gbz_payload_header));
    if (state->header == NULL) {
      sl_zigbee_af_gbz_message_controller_println("GBZ: ERR: GBZ creator unable to allocate memory!");
      return 0;
    }

    state->header->payload = (uint8_t *)malloc(sizeof(uint8_t) * headerSize);
    if (state->header == NULL) {
      sl_zigbee_af_gbz_message_controller_println("GBZ: ERR: GBZ creator unable to allocate memory!");
      return 0;
    }
    state->header->payloadLength = headerSize;
    headerPayload = state->header->payload;

    headerPayload[headerWriteIndex++] = HIGH_BYTE(SE_PROFILE_ID);
    headerPayload[headerWriteIndex++] = LOW_BYTE(SE_PROFILE_ID);

    sl_zigbee_af_copy_int8u(headerPayload, headerWriteIndex, 0);
    state->componentsCount = &headerPayload[headerWriteIndex];
    headerWriteIndex += 1;
    if (type == SL_ZIGBEE_AF_GBZ_MESSAGE_ALERT) {
      //writing in big-endian format EMAPPFWKV2-1423
      headerPayload[headerWriteIndex++] = HIGH_BYTE(alertCode);
      headerPayload[headerWriteIndex++] = LOW_BYTE(alertCode);
      headerPayload[headerWriteIndex++] = (uint8_t) ((timestamp >> 24) & 0xFF);
      headerPayload[headerWriteIndex++] = (uint8_t) ((timestamp >> 16) & 0xFF);
      headerPayload[headerWriteIndex++] = (uint8_t) ((timestamp >> 8) & 0xFF);
      headerPayload[headerWriteIndex++] = (uint8_t) (timestamp & 0xFF);
    }

    totalByteWritten = headerSize;
  } else {
    // bookkeeping inits
    state->allocateMemoryForResponses = false;
    state->responses = NULL;

    memset(gbzCommand, 0, gbzCommandLength);
    state->command = gbzCommand;
    state->commandLength = gbzCommandLength;
    state->commandIndex = 0;
    state->nextEncryptedComponentZclSequenceNumber = 0;

    state->command[state->commandIndex++] = HIGH_BYTE(SE_PROFILE_ID);
    state->command[state->commandIndex++] = LOW_BYTE(SE_PROFILE_ID);

    sl_zigbee_af_copy_int8u(state->command, state->commandIndex, 0);
    state->componentsCount = &state->command[state->commandIndex];
    state->commandIndex += 1;
    if (type == SL_ZIGBEE_AF_GBZ_MESSAGE_ALERT) {
      //writing in big-endian format Jira: EMAPPFWKV2-1423
      state->command[state->commandIndex++] = HIGH_BYTE(alertCode);
      state->command[state->commandIndex++] = LOW_BYTE(alertCode);
      state->command[state->commandIndex++] = (uint8_t) ((timestamp >> 24) & 0xFF);
      state->command[state->commandIndex++] = (uint8_t) ((timestamp >> 16) & 0xFF);
      state->command[state->commandIndex++] = (uint8_t) ((timestamp >> 8) & 0xFF);
      state->command[state->commandIndex++] = (uint8_t) (timestamp & 0xFF);
    }

    totalByteWritten = state->commandIndex;
  }

  state->nextComponentZclSequenceNumber = 0;
  state->nextAdditionalHeaderFrameCounter = 0;
  state->lastExtHeaderControlField = NULL;
  state->messageCode = messageCode;
  return totalByteWritten;
}

uint16_t sl_zigbee_af_gbz_message_controller_append_command(sl_zigbee_af_gbz_message_creator_state_t * creator,
                                                            sl_zigbee_af_gbz_zcl_command_t * zclCmd)
{
  sl_zigbee_af_gbz_message_data_t data = { 0 };
  uint16_t result;

  if (creator == NULL || zclCmd == NULL) {
    sl_zigbee_af_gbz_message_controller_println("GBZ: ERR: Unable to append due to NULL pointer!");
    return 0;
  }

  // test encryption mechanism as sanity check before modifying any
  // internal bookkeeping attributes.
  data.encryption = false;
  data.plainPayload = zclCmd->payload;
  data.plainPayloadLength = zclCmd->payloadLength;

  if (sl_zigbee_af_gbz_message_controller_get_encrypt_payload_flag(creator, zclCmd)) {
    sl_zigbee_af_gbz_message_controller_encrypt_data_cb(&data);
    if (!data.encryption) {
      sl_zigbee_af_gbz_message_controller_println("GBZ: ERR: Failed to encrypt following ZCL payload: ");
      sl_zigbee_af_gbz_message_controller_print_buffer(zclCmd->payload, zclCmd->payloadLength, true);
      sl_zigbee_af_gbz_message_controller_println("");
      return 0;
    }
    zclCmd->encryption = true;
  } else {
    zclCmd->encryption = false;
  }

  if (creator->allocateMemoryForResponses) {
    result = append_in_new_mem(creator,
                               zclCmd,
                               &data);
  } else {
    result = append_in_place(creator,
                             zclCmd,
                             &data);
  }
  return result;
}

sl_zigbee_af_gbz_message_creator_result_t * sl_zigbee_af_gbz_message_controller_creator_assemble(sl_zigbee_af_gbz_message_creator_state_t * state)
{
  uint16_t len = 0;

  sl_zigbee_af_gbz_message_creator_result_t  * result = &state->result;

  result->freeRequired = false;
  result->payloadLength = 0;
  result->payload = NULL;

  if (state->allocateMemoryForResponses) {
    // get total length from all responses
    // allocate memory
    // stuff each response
    // into the main payload
    // free the responses
    uint8_t * responsePayload;
    if (state->header == NULL) {
      sl_zigbee_af_gbz_message_controller_println("GBZ: ERR: Header not intialized! Unable to combine responses.");
      result->payloadLength = 0;
      return NULL;
    }

    // count overall length
    len = state->header->payloadLength;
    len += get_responses_length(state->responses);

    responsePayload = (uint8_t *)malloc(sizeof(uint8_t) * len);
    if (responsePayload == NULL) {
      sl_zigbee_af_gbz_message_controller_println("GBZ: ERR: Unable to allocate memory!");
      result->payloadLength = 0;
      return NULL;
    } else {
      result->freeRequired = true;
      result->payload = responsePayload;
      result->payloadLength = len;
    }

    // copy over payload
    memcpy(responsePayload, state->header->payload, state->header->payloadLength);
    append_response(&responsePayload[state->header->payloadLength],
                    len - state->header->payloadLength,
                    state->responses);

    // clean up
    free_response(state);
    state->lastResponse = NULL;
    free_header(state->header);
  } else {
    result->payload = state->command;
    result->payloadLength = state->commandIndex;
    result->freeRequired = false;
  }

  return &state->result;
}

void sl_zigbee_af_gbz_message_controller_creator_cleanup(sl_zigbee_af_gbz_message_creator_state_t * state)
{
  // Try to free up respones as well in case user never called Assemble().
  if (state->allocateMemoryForResponses) {
    free_response(state);
  }

  if (state->result.freeRequired) {
    free(state->result.payload);
  }
  memset(state, 0x00, sizeof(sl_zigbee_af_gbz_message_creator_state_t));
}

static uint16_t append_in_new_mem(sl_zigbee_af_gbz_message_creator_state_t * state,
                                  sl_zigbee_af_gbz_zcl_command_t * zclCmd,
                                  sl_zigbee_af_gbz_message_data_t * data)
{
  uint16_t len = get_length(zclCmd, data);
  uint16_t index = 0;

  if (len > 0) {
    sli_zigbee_af_gbz_use_case_specific_component * comp;
    uint8_t * payload;
    comp = (sli_zigbee_af_gbz_use_case_specific_component*) malloc(sizeof(sli_zigbee_af_gbz_use_case_specific_component));
    comp->payload = payload = (uint8_t *) malloc(sizeof(uint8_t) * len);
    comp->payloadLength = len;
    comp->next = NULL;

    // mark old "last component" as no longer the last component.
    if (state->lastExtHeaderControlField != NULL) {
      *(state->lastExtHeaderControlField) &= ~GAS_PROXY_FUNCTION_GBZ_COMPONENT_LAST_MSG_MASK;
    }

    *state->componentsCount += 1;

    //Extended Header Control Field
    state->lastExtHeaderControlField = payload + index++;

    if (zclCmd->encryption) {
      *(state->lastExtHeaderControlField) = SL_ZIGBEE_AF_GBZ_LAST_ENCRYPTED_MESSAGE;
    } else {
      *(state->lastExtHeaderControlField) = SL_ZIGBEE_AF_GBZ_LAST_UNENCRYPTED_MESSAGE;
    }

    //Extended Header Cluster ID
    payload[index++] = HIGH_BYTE(zclCmd->clusterId);
    payload[index++] = LOW_BYTE(zclCmd->clusterId);

    //Extended Header GBZ Command Length
    {
      uint16_t len = get_command_length(zclCmd, data);
      payload[index++] = HIGH_BYTE(len);
      payload[index++] = LOW_BYTE(len);
    }

    if (data->encryption) {
      // Additional Header Control
      payload[index++] = 0x00; // reserved value

      // Additional Header Frame Counter
      payload[index++] = state->nextAdditionalHeaderFrameCounter++;
    }

    // ZCL header
    payload[index++] = zclCmd->frameControl;
    payload[index++] = state->nextComponentZclSequenceNumber++;
    payload[index++] = zclCmd->commandId;

    // ZCL payload
    if (data->encryption) {
      // data should be encrypted payloads by now.
      payload[index++] = HIGH_BYTE(data->encryptedPayloadLength);
      payload[index++] = LOW_BYTE(data->encryptedPayloadLength);

      memcpy(&payload[index],
             data->encryptedPayload,
             data->encryptedPayloadLength);
      index += data->encryptedPayloadLength;

      free(data->encryptedPayload);
      data->encryptedPayload = NULL;
    } else {
      memcpy(&payload[index], zclCmd->payload, zclCmd->payloadLength);
      index += zclCmd->payloadLength;
    }

    // append linklist node to end
    if (state->responses == NULL) {
      state->responses = comp;
      state->lastResponse = comp;
    } else {
      state->lastResponse->next = comp;
      state->lastResponse = comp;
    }
  }

  return len;
}

static uint16_t append_in_place(sl_zigbee_af_gbz_message_creator_state_t * state,
                                sl_zigbee_af_gbz_zcl_command_t * zclCmd,
                                sl_zigbee_af_gbz_message_data_t * data)
{
  // will new cmd fit into the current gbz message struct?
  uint16_t zclCommandLength;
  uint16_t numberOfAppendedBytes = state->commandIndex; // snapshot of state->commandIndex

  zclCommandLength = get_length(zclCmd, data);

  if (state->commandLength < state->commandIndex + zclCommandLength) {
    sl_zigbee_af_gbz_message_controller_println("GBZ: ERR: Unable to append ZCL command to GBZ message: out of space (%d/%d).",
                                                state->commandIndex + zclCommandLength,
                                                state->commandLength);
    return 0;
  }

  // mark old "last component" as no longer the last component.
  if (state->lastExtHeaderControlField != NULL) {
    *(state->lastExtHeaderControlField) &= ~GAS_PROXY_FUNCTION_GBZ_COMPONENT_LAST_MSG_MASK;
  }

  // Component Size
  *state->componentsCount += 1;

  // point lastExtHeaderControlField to the new header control field.
  state->lastExtHeaderControlField = state->command + state->commandIndex++;

  // Extended Header Control Field
  if (zclCmd->encryption) {
    *(state->lastExtHeaderControlField) = SL_ZIGBEE_AF_GBZ_LAST_ENCRYPTED_MESSAGE;
  } else {
    *(state->lastExtHeaderControlField) = SL_ZIGBEE_AF_GBZ_LAST_UNENCRYPTED_MESSAGE;
  }

  if (zclCmd->hasFromDateTime) {
    *(state->lastExtHeaderControlField) |= GAS_PROXY_FUNCTION_GBZ_COMPONENT_FROM_DATE_TIME_MASK;
  }

  // allocate extended header cluster id
  state->command[state->commandIndex++] = HIGH_BYTE(zclCmd->clusterId);
  state->command[state->commandIndex++] = LOW_BYTE(zclCmd->clusterId);

  // allocate extended header command length
  {
    uint16_t len = get_command_length(zclCmd, data);
    state->command[state->commandIndex++] = HIGH_BYTE(len);
    state->command[state->commandIndex++] = LOW_BYTE(len);
  }

  if (zclCmd->encryption) {
    // append additional header control
    state->command[state->commandIndex++] = 0x00;

    // append additional header frame counter
    state->command[state->commandIndex++] = state->nextEncryptedComponentZclSequenceNumber++;
  } else if (zclCmd->hasFromDateTime) {
    // append "From Date Time" field
    sl_zigbee_af_copy_int32u(state->command,
                             state->commandIndex,
                             zclCmd->fromDateTime);
    state->commandIndex += GAS_PROXY_FUNCTION_GBZ_COMPONENT_FROM_DATE_TIME_LENGTH;
  }

  state->command[state->commandIndex++] = zclCmd->frameControl;
  state->command[state->commandIndex++] = state->nextComponentZclSequenceNumber++;
  state->command[state->commandIndex++] = zclCmd->commandId;

  if (zclCmd->encryption) {
    // append "Length of Ciphered information"
    state->command[state->commandIndex++] = HIGH_BYTE(data->encryptedPayloadLength);
    state->command[state->commandIndex++] = LOW_BYTE(data->encryptedPayloadLength);

    // ZCl payload
    memcpy(&state->command[state->commandIndex],
           data->encryptedPayload,
           data->encryptedPayloadLength);
    state->commandIndex += data->encryptedPayloadLength;

    free(data->encryptedPayload);
    data->encryptedPayload = NULL;
    data->encryptedPayloadLength = 0;
  } else {
    // ZCl payload
    memcpy(&state->command[state->commandIndex],
           zclCmd->payload,
           zclCmd->payloadLength);
    state->commandIndex += zclCmd->payloadLength;
  }

  numberOfAppendedBytes = state->commandIndex - numberOfAppendedBytes;
  return numberOfAppendedBytes;
}

sl_zigbee_af_status_t sl_zigbee_af_gbz_message_controller_get_zcl_default_response(sl_zigbee_af_gbz_zcl_command_t * cmd)
{
  if (cmd->clusterSpecific) {
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  } else if ((!cmd->clusterSpecific) && cmd->payloadLength > 1) {
    return (sl_zigbee_af_status_t) cmd->payload[1];
  } else {
    return SL_ZIGBEE_ZCL_STATUS_FAILURE;
  }
}

static void free_response(sl_zigbee_af_gbz_message_creator_state_t * state)
{
  sli_zigbee_af_gbz_use_case_specific_component * next;
  sli_zigbee_af_gbz_use_case_specific_component * cur = state->responses;

  while (cur != NULL) {
    next = cur->next;
    free(cur->payload);
    cur->payloadLength = 0;
    cur->next = NULL;
    free(cur);
    cur = next;
  }

  state->responses = NULL;
}

/*
 * @brief Cleanup dynamic memory used to store GBZ message header.
 * */
static void free_header(sli_zigbee_af_gbz_payload_header * header)
{
  if (header == NULL) {
    return;
  }

  free(header->payload);
  free(header);
}

/*
 *  @brief Total storage needed to store all Use Case Specific Components.
 *
 *  @return number of bytes
 *
 */
static uint16_t get_responses_length(sli_zigbee_af_gbz_use_case_specific_component * comp)
{
  uint16_t length = 0;
  sli_zigbee_af_gbz_use_case_specific_component * next;
  sli_zigbee_af_gbz_use_case_specific_component * cur = comp;
  while (cur != NULL) {
    next = cur->next;
    length += cur->payloadLength;
    cur = next;
  }
  return length;
}

/*
 * @brief returns number of appended bytes
 */
static uint16_t append_response(uint8_t * dst,
                                uint16_t maxLen,
                                sli_zigbee_af_gbz_use_case_specific_component * responses)
{
  uint16_t index = 0;
  sli_zigbee_af_gbz_use_case_specific_component * next;
  sli_zigbee_af_gbz_use_case_specific_component * cur = responses;
  while (cur != NULL) {
    next = cur->next;

    if (index + cur->payloadLength <= maxLen) {
      memcpy(&dst[index], cur->payload, cur->payloadLength);
      index += cur->payloadLength;
    }

    cur = next;
  }
  return index;
}

bool sl_zigbee_af_gbz_message_controller_get_encrypt_payload_flag(sl_zigbee_af_gbz_message_creator_state_t * creator,
                                                                  sl_zigbee_af_gbz_zcl_command_t * resp)
{
  uint8_t * commandId =  &(resp->commandId);
  uint16_t * clusterId = &(resp->clusterId);

  if (creator->messageCode == TEST_ENCRYPTED_MESSAGE_CODE) {
    return true;
  }
  if (creator->messageCode == TEST_MESSAGE_CODE) {
    return false;
  }

  if (!resp->clusterSpecific) {
    // we do not encrypt default responses.
    if (*commandId == ZCL_DEFAULT_RESPONSE_COMMAND_ID) {
      return false;
    } else if (*commandId == ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID) {
      if ((creator->messageCode == GCS13a_MESSAGE_CODE)
          || (creator->messageCode == GCS13b_MESSAGE_CODE)
          || (creator->messageCode == GCS13c_MESSAGE_CODE)) {
        return true;
      } else if ((creator->messageCode == GCS21b_MESSAGE_CODE)
                 || (creator->messageCode == GCS21f_MESSAGE_CODE)
                 || (creator->messageCode == GCS21j_MESSAGE_CODE)
                 || (creator->messageCode == GCS33_MESSAGE_CODE)
                 || (creator->messageCode == GCS38_MESSAGE_CODE)
                 || (creator->messageCode == GCS46_MESSAGE_CODE)
                 || (creator->messageCode == GCS60_MESSAGE_CODE)) {
        return false;
      } else if (creator->messageCode == GCS14_MESSAGE_CODE) {
        uint16_t bufIndex = 0;
        uint16_t bufLen = resp->payloadLength;
        uint8_t * buffer = resp->payload;

        if (resp->clusterId != ZCL_PREPAYMENT_CLUSTER_ID) {
          return false;
        }

        // parse the response and act accordingly.
        // we are doing the manual parsing here since a plugin specific
        // callback has to parse the payload manually anyways.
        // Encrypt the payload if we see any of following attributes:
        //
        // Prepayment Info: Accumulated Debt
        // Prepayment Info: Emergency CreditRemaining
        // Prepayment Info: Credit Remaining
        // Payment-based Debt
        // Time-based Debt(1)
        // Time-based Debt(2)
        //
        // Each record in the response has a two-byte attribute id and a one-byte
        // status.  If the status is SUCCESS, there will also be a one-byte type and
        // variable-length data.

        while (bufIndex + 3 <= bufLen) {
          sl_zigbee_af_status_t status;
          sl_zigbee_af_attribute_id_t attributeId;
          attributeId = (sl_zigbee_af_attribute_id_t)sl_zigbee_af_get_int16u(buffer,
                                                                             bufIndex,
                                                                             bufLen);
          bufIndex += 2;
          status = (sl_zigbee_af_status_t)sl_zigbee_af_get_int8u(buffer, bufIndex, bufLen);
          bufIndex++;
          if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
            if (bufLen - bufIndex < 1) {
              // premature end of buffer after success status
              break;
            }
            uint8_t dataType = sl_zigbee_af_get_int8u(buffer, bufIndex, bufLen);
            bufIndex++;

            uint16_t dataSize = sl_zigbee_af_attribute_value_size(dataType,
                                                                  buffer + bufIndex,
                                                                  bufLen - bufIndex);

            if (dataSize != 0) {
              if (attributeId == ZCL_DEBT_AMOUNT_1_ATTRIBUTE_ID
                  || (attributeId == ZCL_DEBT_AMOUNT_2_ATTRIBUTE_ID)
                  || (attributeId == ZCL_DEBT_AMOUNT_3_ATTRIBUTE_ID)) {
                return true;
              }
              bufIndex += dataSize;
            } else {
              // dataSize exceeds buffer length, terminate loop
              break; // while
            }
          }
        }
      }
      return false;
    }
  } else {
    switch (creator->messageCode) {
      case GCS15b_MESSAGE_CODE:
        if ((*clusterId == ZCL_PREPAYMENT_CLUSTER_ID)
            && (*commandId == ZCL_PUBLISH_PREPAY_SNAPSHOT_COMMAND_ID)) {
          return true;
        } else if ((*clusterId == ZCL_SIMPLE_METERING_CLUSTER_ID)
                   && (*commandId == ZCL_PUBLISH_SNAPSHOT_COMMAND_ID)) {
          return true;
        }
        break;
      case GCS15c_MESSAGE_CODE:
        if ((*clusterId == ZCL_SIMPLE_METERING_CLUSTER_ID)
            && (*commandId == ZCL_PUBLISH_SNAPSHOT_COMMAND_ID)) {
          return true;
        } else if ((*clusterId == ZCL_PREPAYMENT_CLUSTER_ID)
                   && (*commandId == ZCL_PUBLISH_PREPAY_SNAPSHOT_COMMAND_ID)) {
          return true;
        }
        break;
      case GCS15d_MESSAGE_CODE:
        if ((*clusterId == ZCL_PREPAYMENT_CLUSTER_ID)
            && (*commandId == ZCL_PUBLISH_DEBT_LOG_COMMAND_ID)) {
          return true;
        }
        break;
      case GCS16a_MESSAGE_CODE:
        if ((*clusterId == ZCL_SIMPLE_METERING_CLUSTER_ID)
            && (*commandId == ZCL_PUBLISH_SNAPSHOT_COMMAND_ID)) {
          return true;
        }
        break;
      case GCS16b_MESSAGE_CODE:
        if ((*clusterId == ZCL_PREPAYMENT_CLUSTER_ID)
            && (*commandId == ZCL_PUBLISH_PREPAY_SNAPSHOT_COMMAND_ID)) {
          return true;
        }
        break;
      case GCS17_MESSAGE_CODE:
        if ((*clusterId == ZCL_SIMPLE_METERING_CLUSTER_ID)
            && (*commandId == ZCL_GET_SAMPLED_DATA_RESPONSE_COMMAND_ID)) {
          return true;
        }
        break;
      case GCS53_MESSAGE_CODE:
        if ((*clusterId == ZCL_SIMPLE_METERING_CLUSTER_ID)
            && (*commandId == ZCL_PUBLISH_SNAPSHOT_COMMAND_ID)) {
          return true;
        }
        break;
      case GCS61_MESSAGE_CODE:
        if ((*clusterId == ZCL_SIMPLE_METERING_CLUSTER_ID)
            && (*commandId == ZCL_GET_SAMPLED_DATA_RESPONSE_COMMAND_ID)) {
          return true;
        }
        break;
      case GCS21d_MESSAGE_CODE:
        if ((*clusterId == ZCL_PRICE_CLUSTER_ID)
            && (*commandId == ZCL_PUBLISH_BILLING_PERIOD_COMMAND_ID)) {
          return false;
        }
        break;
    }
  }

  return false;
}
