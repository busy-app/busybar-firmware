/***************************************************************************//**
 * @file
 * @brief  *
 * Client Operation:
 *   1. Look for ZDO device announce notification.
 *   2. Perform ZDO match descriptor on device.
 *   3. If supports IAS Zone Server, Add that server to our known list.
 *     Write CIE Address.
 *   4. Read CIE address, verify it is ours.  This is done mostly because
 *     the test case requires it.
 *   5. Read the IAS Zone Server attributes.
 *     Record in table.
 *   6. When we get an enroll request, give them our (only) zone ID.
 *   7. When we get a notification, read their attributes.
 *
 * Improvements that could be made:
 *   Add support for multiple endpoints on server.  Most often this is a
 *   legacy security system retrofitted with a single ZigBee radio.  Therefore
 *   each sensor is on a different endpoint.  Right now our client only
 *   handles a single endpoint per node.
 *
 *   Integration with Poll Control.  When the device boots we should configure
 *   its polling to make it possible to read/write its attributes.
 *
 *   Update the sl_zigbee_af_ias_zone_client_known_servers list when we know a server
 *   un-enrolls. Right now, we don't have any way to tell when we don't need
 *   to keep track of a server anymore, i.e., when it un-enrolls. Therefore,
 *   we could potentially keep adding servers to our known list, and run out
 *   of room to add more. Fortunately, we have two things working for us:
 *     1. Servers will most likely stay around in a network. It is unlikely
 *        that an IAS Zone Client in production will have to handle 254
 *        different servers.
 *     2. If a server un-enrolls and then enrolls again, it will get the same
 *        Zone ID and have a spot in the list, since we store servers by
 *        long address.
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
#include "ias-zone-client.h"

#include "ias-zone-client-config.h"
#include "zap-cluster-command-parser.h"
#include "stack/include/zigbee-device-stack.h"
//-----------------------------------------------------------------------------
// Globals

sli_zigbee_ias_zone_device_t sl_zigbee_af_ias_zone_client_known_servers[SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES];

typedef enum {
  IAS_ZONE_CLIENT_STATE_NONE,
  IAS_ZONE_CLIENT_STATE_DISCOVER_ENDPOINT,
  IAS_ZONE_CLIENT_STATE_SET_CIE_ADDRESS,
  IAS_ZONE_CLIENT_STATE_READ_CIE_ADDRESS,
  IAS_ZONE_CLIENT_STATE_READ_ATTRIBUTES,
} sli_zigbee_ias_zone_client_state_t;

static sli_zigbee_ias_zone_client_state_t iasZoneClientState = IAS_ZONE_CLIENT_STATE_NONE;
static uint8_t currentIndex = NO_INDEX;
static uint8_t myEndpoint = 0;
static bool allowCieAddress = true;

static sl_zigbee_af_event_t stateMachineEvent;
static void stateMachineEventHandler(sl_zigbee_af_event_t * event);
//-----------------------------------------------------------------------------
// Forward Declarations

static void readIasZoneServerAttributes(sl_802154_short_addr_t nodeId);
static void iasClientSaveCommand(void);
static void iasClientLoadCommand(void);

//-----------------------------------------------------------------------------
// Functions
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sl_zigbee_ias_client_allow_set_cie_address(bool allow)
{
  allowCieAddress = allow;
}
#endif
void sl_zigbee_af_ias_zone_cluster_client_init_cb(uint8_t endpoint)
{
  sl_zigbee_af_event_init(&stateMachineEvent,
                          stateMachineEventHandler);
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
  sl_zigbee_ias_client_allow_set_cie_address(true);
#endif
  sli_zigbee_af_clear_servers();
  myEndpoint = endpoint;
  iasClientLoadCommand();
}

void sli_zigbee_af_clear_servers(void)
{
  memset(sl_zigbee_af_ias_zone_client_known_servers, 0xFF,
         sizeof(sli_zigbee_ias_zone_device_t)
         * SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES);
}

static void clearState(void)
{
  currentIndex = 0;
  iasZoneClientState = IAS_ZONE_CLIENT_STATE_NONE;
}

static void setServerZoneStatus(uint8_t serverIndex, uint16_t zoneStatus)
{
  sl_zigbee_af_ias_zone_client_known_servers[serverIndex].zoneStatus = zoneStatus;
  iasClientSaveCommand();
}

static void setServerIeee(uint8_t serverIndex, uint8_t* ieeeAddress)
{
  memcpy(sl_zigbee_af_ias_zone_client_known_servers[serverIndex].ieeeAddress, ieeeAddress, EUI64_SIZE);
  iasClientSaveCommand();
}

static void clearServerIeee(uint8_t serverIndex)
{
  memset(sl_zigbee_af_ias_zone_client_known_servers[serverIndex].ieeeAddress,
         0xFF,
         sizeof(sli_zigbee_ias_zone_device_t));
  iasClientSaveCommand();
}

static void setServerNodeId(uint8_t serverIndex, sl_802154_short_addr_t nodeId)
{
  sl_zigbee_af_ias_zone_client_known_servers[serverIndex].nodeId = nodeId;
}

static void clearServerNodeId(uint8_t serverIndex)
{
  sl_zigbee_af_ias_zone_client_known_servers[serverIndex].nodeId = SL_ZIGBEE_NULL_NODE_ID;
}

static void setServerZoneState(uint8_t serverIndex, uint8_t zoneState)
{
  sl_zigbee_af_ias_zone_client_known_servers[serverIndex].zoneState = zoneState;
  iasClientSaveCommand();
}

static void setServerEndpoint(uint8_t serverIndex, uint8_t endpoint)
{
  sl_zigbee_af_ias_zone_client_known_servers[serverIndex].endpoint = endpoint;
  iasClientSaveCommand();
}

static void setServerZoneType(uint8_t serverIndex, uint16_t zoneType)
{
  sl_zigbee_af_ias_zone_client_known_servers[serverIndex].zoneType = zoneType;
  iasClientSaveCommand();
}

static void setServerZoneId(uint8_t serverIndex, uint16_t zoneId)
{
  sl_zigbee_af_ias_zone_client_known_servers[serverIndex].zoneId = zoneId;
  iasClientSaveCommand();
}

static void setCurrentIndex(uint8_t serverIndex)
{
  currentIndex = serverIndex;
  iasClientSaveCommand();
}

static void setIasZoneClientState(uint8_t clientState)
{
  iasZoneClientState = clientState;
  iasClientSaveCommand();
}

static void iasClientSaveCommand(void)
{
#if defined(EZSP_HOST) && !defined(SL_ZIGBEE_TEST) && defined(UNIX_HOST)
  FILE *fp;
  uint16_t i, j;

  // save zone server list
  fp = fopen("iaszone.txt", "w");

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES; i++) {
    if (sl_zigbee_af_ias_zone_client_known_servers[i].zoneId != 0xFF) {
      fprintf(fp, "%02X %02X %02X %02X %02X ", sl_zigbee_af_ias_zone_client_known_servers[i].zoneId,
              sl_zigbee_af_ias_zone_client_known_servers[i].zoneStatus,
              sl_zigbee_af_ias_zone_client_known_servers[i].zoneState,
              sl_zigbee_af_ias_zone_client_known_servers[i].endpoint,
              sl_zigbee_af_ias_zone_client_known_servers[i].zoneType);
      for (j = 0; j < 8; j++) {
        fprintf(fp, "%02X ", sl_zigbee_af_ias_zone_client_known_servers[i].ieeeAddress[j]);
      }
    }
  }
  // Write something to mark the end of the file.
  fprintf(fp, "ff");
  assert(fclose(fp) == 0);
#endif //#if defined(EZSP_HOST) && !defined(SL_ZIGBEE_TEST) && defined(UNIX_HOST)
}

static void iasClientLoadCommand(void)
{
#if defined(EZSP_HOST) && !defined(SL_ZIGBEE_TEST) && defined(UNIX_HOST)
  FILE *fp;
  uint16_t i, j;

  unsigned int data1, data2, data3, data4, data5;

  fp = fopen("iaszone.txt", "r");

  if (!fp) {
    return;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES; i++) {
    if (feof(fp)) {
      break;
    }
    fscanf(fp, "%02X ", &data1);
    if (data1 == 0xff) {
      break;
    }
    fscanf(fp,
           "%02X %02X %02X %02X ",
           &data2,
           &data3,
           &data4,
           &data5);

    sl_zigbee_af_ias_zone_client_known_servers[i].zoneId = (uint8_t) data1;
    sl_zigbee_af_ias_zone_client_known_servers[i].zoneStatus = (uint16_t) data2;
    sl_zigbee_af_ias_zone_client_known_servers[i].zoneState = (uint8_t) data3;
    sl_zigbee_af_ias_zone_client_known_servers[i].endpoint = (uint8_t) data4;
    sl_zigbee_af_ias_zone_client_known_servers[i].zoneType = (uint16_t) data5;

    for (j = 0; j < 8; j++) {
      fscanf(fp, "%02X ", &data1);
      sl_zigbee_af_ias_zone_client_known_servers[i].ieeeAddress[j] = (uint8_t) data1;
    }
  }
  assert(fclose(fp) == 0);
#endif // #if defined(EZSP_HOST) && !defined(SL_ZIGBEE_TEST) && defined(UNIX_HOST)
}

static uint8_t findIasZoneServerByIeee(uint8_t* ieeeAddress)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES; i++) {
    if (0 == memcmp(ieeeAddress,
                    sl_zigbee_af_ias_zone_client_known_servers[i].ieeeAddress,
                    EUI64_SIZE)) {
      return i;
    }
  }
  return NO_INDEX;
}

static uint8_t findIasZoneServerByNodeId(sl_802154_short_addr_t nodeId)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES; i++) {
    if (nodeId == sl_zigbee_af_ias_zone_client_known_servers[i].nodeId) {
      return i;
    }
  }

  // If we didn't find the node ID in the table, see if the stack knows about
  // it.
  sl_802154_long_addr_t eui64;
  if (sl_zigbee_lookup_eui64_by_node_id(nodeId, eui64) == SL_STATUS_OK) {
    i = findIasZoneServerByIeee(eui64);
    if (i != NO_INDEX) {
      setServerNodeId(i, nodeId);
    }
  }

  return i;
}

//----------------------------
// Commands callbacks

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_ias_zone_cluster_zone_status_change_notification_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_ias_zone_cluster_zone_status_change_notification_command_t cmd_data;
  uint8_t serverIndex = findIasZoneServerByNodeId(cmd->source);
  uint8_t status = SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;

  if (zcl_decode_ias_zone_cluster_zone_status_change_notification_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if (serverIndex != NO_INDEX) {
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    setServerZoneStatus(serverIndex, cmd_data.zoneStatus);

    sl_zigbee_af_ias_zone_cluster_println("Zone %d status change, 0x%04X from 0x%04X",
                                          cmd_data.zoneId,
                                          cmd_data.zoneStatus,
                                          cmd->source);

    // The Test case calls for readding attributes after status change.
    //   that is silly for the production device.
    // readIasZoneServerAttributes(cmd->source);
  }
  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_ias_zone_cluster_zone_enroll_request_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zigbee_af_ias_enroll_response_code_t responseCode = SL_ZIGBEE_ZCL_IAS_ENROLL_RESPONSE_CODE_NO_ENROLL_PERMIT;
  uint8_t zoneId = UNKNOWN_ZONE_ID;
  uint8_t serverIndex = findIasZoneServerByNodeId(cmd->source);
  sl_status_t status;

  if (serverIndex != NO_INDEX) {
    responseCode = SL_ZIGBEE_ZCL_IAS_ENROLL_RESPONSE_CODE_SUCCESS;
    zoneId = serverIndex;
    setServerZoneId(serverIndex, zoneId);
  }
  sl_zigbee_af_fill_command_ias_zone_cluster_zone_enroll_response(responseCode,
                                                                  zoneId);
  // Need to send this command with our source EUI because the server will
  // check our EUI64 against his CIE Address to see if we're his CIE.
  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  status = sl_zigbee_af_send_response();
  sl_zigbee_af_core_println("Sent enroll response with responseCode: 0x%02X, zoneId: 0x%02X, status: 0x%02X",
                            responseCode,
                            zoneId,
                            status);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

static void stateMachineEventHandler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sl_zigbee_af_ias_zone_cluster_println("IAS Zone Client Timeout waiting for message response.");
  sl_zigbee_af_event_set_inactive(&stateMachineEvent);
  clearState();
}

static uint8_t addServer(sl_802154_short_addr_t nodeId, uint8_t* ieeeAddress)
{
  uint8_t i = findIasZoneServerByIeee(ieeeAddress);
  if (i != NO_INDEX) {
    return i;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES; i++) {
    const uint8_t unsetEui64[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    if (0 == memcmp(sl_zigbee_af_ias_zone_client_known_servers[i].ieeeAddress,
                    unsetEui64,
                    EUI64_SIZE)) {
      setServerIeee(i, ieeeAddress);
      setServerNodeId(i, nodeId);
      setServerEndpoint(i, UNKNOWN_ENDPOINT);
      return i;
    }
  }
  return NO_INDEX;
}

static void removeServer(uint8_t* ieeeAddress)
{
  uint8_t index = findIasZoneServerByIeee(ieeeAddress);
  clearServerIeee(index);
  clearServerNodeId(index);
}

static sl_status_t sendCommand(sl_802154_short_addr_t destAddress)
{
  sl_zigbee_af_set_command_endpoints(myEndpoint, sl_zigbee_af_ias_zone_client_known_servers[currentIndex].endpoint);
  sl_status_t status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, destAddress);
  sl_zigbee_af_ias_zone_cluster_println("Sent IAS Zone Client Command to 0x%04X (%d -> %d) status: 0x%02X",
                                        destAddress,
                                        myEndpoint,
                                        sl_zigbee_af_ias_zone_client_known_servers[currentIndex].endpoint,
                                        status);
  if (status != SL_STATUS_OK) {
    clearState();
  }
  return status;
}

static void setCieAddress(sl_802154_short_addr_t destAddress)
{
  uint8_t writeAttributes[] = {
    LOW_BYTE(ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID),
    ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE,
    0, 0, 0, 0, 0, 0, 0, 0,  // ieee (filled in later)
  };
  sl_zigbee_af_get_eui64(&writeAttributes[3]);
  sl_zigbee_af_fill_command_global_client_to_server_write_attributes(ZCL_IAS_ZONE_CLUSTER_ID,
                                                                     writeAttributes,
                                                                     sizeof(writeAttributes));
  sl_zigbee_af_ias_zone_cluster_println("Writing CIE Address to IAS Zone Server");
  if (SL_STATUS_OK == sendCommand(destAddress)) {
    setIasZoneClientState(IAS_ZONE_CLIENT_STATE_SET_CIE_ADDRESS);
  }
}

static void iasZoneClientServiceDiscoveryCallback(const sl_zigbee_af_service_discovery_result_t* result)
{
  if (result->status == SL_ZIGBEE_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE
      && result->zdoRequestClusterId == MATCH_DESCRIPTORS_REQUEST
      && allowCieAddress) {
    const sl_zigbee_af_endpoint_list_t* endpointList = (const sl_zigbee_af_endpoint_list_t *)result->responseData;
    if (endpointList->count > 0) {
      setServerEndpoint(currentIndex, endpointList->list[0]);
      sl_zigbee_af_ias_zone_cluster_println("Device 0x%04X supports IAS Zone Server",
                                            result->matchAddress);
      setCieAddress(result->matchAddress);
      return;
    }
  }
  clearState();
}

static void checkForIasZoneServer(sl_802154_short_addr_t sl_zigbee_node_id, uint8_t* ieeeAddress)
{
  uint8_t endpointIndex = sl_zigbee_af_index_from_endpoint(myEndpoint);
  uint16_t profileId = sl_zigbee_af_profile_id_from_index(endpointIndex);
  uint8_t serverIndex = addServer(sl_zigbee_node_id, ieeeAddress);

  if (serverIndex == NO_INDEX) {
    sl_zigbee_af_ias_zone_cluster_println("Error: Could not add IAS Zone server.");
    return;
  }

  setCurrentIndex(serverIndex);

  if (sl_zigbee_af_ias_zone_client_known_servers[serverIndex].endpoint != UNKNOWN_ENDPOINT
      && allowCieAddress) {
    // If a remote endpoint that you have already seen announces itself,
    // write your IEEE in them just in case they left and are rejoining. --agkeesle
    // Bug: EMAPPFWKV2-1078
    setCieAddress(sl_zigbee_node_id);
    sl_zigbee_af_ias_zone_cluster_println("Node 0x%04X already known to IAS client", sl_zigbee_node_id);
    return;
  }

  sl_status_t status = sl_zigbee_af_find_devices_by_profile_and_cluster(sl_zigbee_node_id,
                                                                        profileId,
                                                                        ZCL_IAS_ZONE_CLUSTER_ID,
                                                                        true, // server cluster?
                                                                        iasZoneClientServiceDiscoveryCallback);

  if (status != SL_STATUS_OK) {
    sl_zigbee_af_ias_zone_cluster_println("Error: Failed to initiate service discovery for IAS Zone Server 0x%04X", sl_zigbee_node_id);
    clearState();
  }
}

void sl_zigbee_af_ias_zone_client_zdo_message_received_cb(sl_802154_short_addr_t sl_zigbee_node_id,
                                                          sl_zigbee_aps_frame_t* apsFrame,
                                                          uint8_t* message,
                                                          uint16_t length)
{
  UNUSED_VAR(length);

  sl_zigbee_af_ias_zone_cluster_println("Incoming ZDO, Cluster: 0x%04X", apsFrame->clusterId);
  if (apsFrame->clusterId == END_DEVICE_ANNOUNCE) {
    checkForIasZoneServer(sl_zigbee_node_id, &(message[3]));
  }
}

static void readIasZoneServerAttributes(sl_802154_short_addr_t nodeId)
{
  uint8_t iasZoneAttributeIds[] = {
    LOW_BYTE(ZCL_ZONE_STATE_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_ZONE_STATE_ATTRIBUTE_ID),

    LOW_BYTE(ZCL_ZONE_TYPE_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_ZONE_TYPE_ATTRIBUTE_ID),

    LOW_BYTE(ZCL_ZONE_STATUS_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_ZONE_STATUS_ATTRIBUTE_ID),
  };
  sl_zigbee_af_fill_command_global_client_to_server_read_attributes(ZCL_IAS_ZONE_CLUSTER_ID,
                                                                    iasZoneAttributeIds,
                                                                    sizeof(iasZoneAttributeIds));
  if (SL_STATUS_OK == sendCommand(nodeId)) {
    setIasZoneClientState(IAS_ZONE_CLIENT_STATE_READ_ATTRIBUTES);
  }
}

static void readIasZoneServerCieAddress(sl_802154_short_addr_t nodeId)
{
  uint8_t iasZoneAttributeIds[] = {
    LOW_BYTE(ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID),
    HIGH_BYTE(ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID),
  };
  sl_zigbee_af_fill_command_global_client_to_server_read_attributes(ZCL_IAS_ZONE_CLUSTER_ID,
                                                                    iasZoneAttributeIds,
                                                                    sizeof(iasZoneAttributeIds));
  if (SL_STATUS_OK == sendCommand(nodeId)) {
    setIasZoneClientState(IAS_ZONE_CLIENT_STATE_READ_CIE_ADDRESS);
  }
}

void sl_zigbee_af_ias_zone_client_write_attributes_response_cb(sl_zigbee_af_cluster_id_t clusterId,
                                                               uint8_t * buffer,
                                                               uint16_t bufLen)
{
  UNUSED_VAR(bufLen);

  if (clusterId == ZCL_IAS_ZONE_CLUSTER_ID
      && iasZoneClientState == IAS_ZONE_CLIENT_STATE_SET_CIE_ADDRESS
      && buffer[0] == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    readIasZoneServerCieAddress(sl_zigbee_af_current_command()->source);
    return;
  }
  return;
}

void sl_zigbee_af_ias_zone_client_read_attributes_response_cb(sl_zigbee_af_cluster_id_t clusterId,
                                                              uint8_t * buffer,
                                                              uint16_t bufLen)
{
  uint16_t zoneStatus, zoneType;
  uint8_t zoneState;
  if (clusterId == ZCL_IAS_ZONE_CLUSTER_ID
      && (iasZoneClientState == IAS_ZONE_CLIENT_STATE_READ_ATTRIBUTES
          || iasZoneClientState == IAS_ZONE_CLIENT_STATE_READ_CIE_ADDRESS)) {
    uint16_t i = 0;
    while ((i + 3) <= bufLen) {  // 3 to insure we can read at least the attribute ID
                                 // and the status
      uint16_t attributeId = buffer[i] + (buffer[i + 1] << 8);
      uint8_t status = buffer[i + 2];
      i += 3;
      //sl_zigbee_af_ias_zone_cluster_println("Parsing Attribute 0x%04X, Status: 0x%02X", attributeId, status);
      if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
        if ((i + 1) > bufLen) {
          // Too short, dump the message.
          return;
        }
        i++;  // skip the type of the attribute.  We already know what it should be.
        switch (attributeId) {
          case ZCL_ZONE_STATUS_ATTRIBUTE_ID:
            if ((i + 2) > bufLen) {
              // Too short, dump the message.
              return;
            }
            zoneStatus = (buffer[i] + (buffer[i + 1] << 8));
            setServerZoneStatus(currentIndex, zoneStatus);
            i += 2;
            break;
          case ZCL_ZONE_TYPE_ATTRIBUTE_ID:
            if ((i + 2) > bufLen) {
              // Too short, dump the message.
              return;
            }
            zoneType = (buffer[i] + (buffer[i + 1] << 8));
            setServerZoneType(currentIndex, zoneType);
            i += 2;
            break;
          case ZCL_ZONE_STATE_ATTRIBUTE_ID:
            if ((i + 1) > bufLen) {
              // Too short, dump the message
              return;
            }
            zoneState = buffer[i];
            setServerZoneState(currentIndex, zoneState);
            i++;
            break;
          case ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID: {
            uint8_t myIeee[EUI64_SIZE];
            sl_zigbee_af_get_eui64(myIeee);
            if ((i + 8) > bufLen) {
              // Too short, dump the message
            } else if (0 != memcmp(&(buffer[i]), myIeee, EUI64_SIZE)) {
              sl_zigbee_af_ias_zone_cluster_println("CIE Address not set to mine, removing IAS zone server.");
              removeServer(&(buffer[i]));
              clearState();
            } else {
              readIasZoneServerAttributes(sl_zigbee_af_current_command()->source);
            }
            return;
          }
        }
      }
    }
    sl_zigbee_af_ias_zone_cluster_println("Retrieved IAS Zone Server attributes from 0x%04X",
                                          sl_zigbee_af_current_command()->source);
    clearState();
  }
}

uint32_t sl_zigbee_af_ias_zone_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                            sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_ZONE_STATUS_CHANGE_NOTIFICATION_COMMAND_ID:
      {
        status = sl_zigbee_af_ias_zone_cluster_zone_status_change_notification_cb(cmd);
        break;
      }
      case ZCL_ZONE_ENROLL_REQUEST_COMMAND_ID:
      {
        status = sl_zigbee_af_ias_zone_cluster_zone_enroll_request_cb(cmd);
        break;
      }
    }
  }

  return status;
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sli_zigbee_af_ias_zone_client_allow_set_cie_address(sl_cli_command_arg_t *arguments)
{
  bool allowCieAddress = ((uint8_t)sl_cli_get_argument_uint8(arguments, 0) != 0);
  sl_zigbee_ias_client_allow_set_cie_address(allowCieAddress);
}
#endif
