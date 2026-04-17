/***************************************************************************//**
 * @file
 * @brief Cli commands implementation for 802.15.4 MAC certification.
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

#ifdef MAC_TEST_COMMANDS_SUPPORT
#include PLATFORM_HEADER
#include "hal.h"
#include "stack/core/sl_zigbee_stack.h"
#include "serial/serial.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "framework/packet-header.h"
#include "core/sl_zigbee_multi_network.h"
#include "core/sl_zigbee_multi_phy.h"
#include "indirect-queue.h" // unified-mac
#include "mac-child.h"      // unified-mac

#include "stack/routing/zigbee/association.h"
#include "stack/routing/zigbee/beacon-handling.h"
#include "stack/include/sl_zigbee_types_internal.h"

#include "stack/mac/lower-mac-multi-phy.h"
#include "stack/mac/mac-dispatch.h"
#include "stack/routing/zigbee/child.h"
#include "stack/mac/command.h"
#include "lower-mac.h"
#include "app/util/common/common.h" //printBigEndianEui64
#include "mac-test-commands.h"
#include "mac/zigbee-upper-mac.h"
#include "mac/mac-info-element-parsing.h"
#include "stack/routing/zigbee/enhanced-beacon-request.h"
#include "event_queue/event-queue.h"

#include "stack/include/pro_compliance_stack_interface.h"

//------------------------------------------------------------------------------
// Globals

typedef struct {
  uint8_t coordAddrMode;
  uint16_t coordPanId;
  uint8_t coordAddress[8];
  uint8_t logicalChannel;
  uint8_t channelPage;
  uint16_t superFrameSpec;
  bool gtsPermit;
  uint8_t lqi;
  uint32_t timeStamp;
} PanDescriptor;

#define PAN_DESCRIPTOR_TABLE_SIZE 5

// This is to delay in ms to transmit a data frame after sending
// an ack to a data request with frame pending bit set to a sleepy device.
// We need this for tester/test-harness to validate MAC/Data-04 test case.
uint8_t delayPolledPacketTransmitTimeMs = 0;

//------------------------------------------------------------------------------
// Forward Declarations
static uint8_t scanType;
static uint32_t scanChannels;
static uint8_t channelCount = 0;
static uint8_t scanEnergyDetectValue[SL_ZIGBEE_MAX_CHANNELS_PER_PAGE];
static uint8_t panDescriptorCount;
static PanDescriptor panDescriptorList[PAN_DESCRIPTOR_TABLE_SIZE];
static bool collectPanDescriptors = false;

static void processIncomingMacData(uint8_t *macHeader, uint8_t macPayloadLength);
static void printPanDescriptor(PanDescriptor *pd);

static const char * const nodeTypeNames[] = {
  "Unknown",
  "Coordinator",
  "Router",
  "End Device",
  "Sleepy",
};

#ifdef MAC_DUAL_PRESENT
// To associate dual phy device for BEACON-MANAGEMENT 04-06 tests.
uint8_t sli_zigbee_association_mac_index = 0;
#endif

#if defined(SL_ZIGBEE_TEST) && !defined(ZIGBEE_STACK_ON_HOST)
// To turn this off, since it is expected to get invalid nwk header
// in mac certification testing.
extern bool sli_zigbee_packet_validate_assert_on_invalid;
#else
bool sli_zigbee_packet_validate_assert_on_invalid = true;

  #define PACKET_VALIDATE_DROP(x)                          \
  do {                                                     \
    DROP_PACKET(x);                                        \
    assert(!sli_zigbee_packet_validate_assert_on_invalid); \
  } while (0)

#endif

void sl_zigbee_scan_return(uint8_t channel, sl_status_t status);
// Print information elements from received enhanced beacon request (Ebr).
void printInformationElementsFromEbr(sli_zigbee_packet_header_t header,
                                     sli_802154mac_frame_info_element_parse_result result,
                                     sli_802154mac_info_element_field* infoElementsArray);
static void printMacHeaderInformation(uint8_t* macHeader);
//----------------------------------------------------------------------------
// External Declarations

// stack/routing/zigbee/association.c
extern sli_zigbee_event_t sli_zigbee_association_event;
// app/util/common.c
extern uint8_t serialPort;
// stack/routing/zigbee/random-id.c
extern sl_802154_short_addr_t nextChildId;
// stack/routing/zigbee/beacon-handling.c
extern uint8_t sli_zigbee_get_beacon_payload(sli_zigbee_packet_header_t beacon, uint8_t *returnPayload);
extern uint8_t sli_zigbee_beacon_payload_buffer[BEACON_PAYLOAD_SIZE];
extern uint8_t sli_zigbee_beacon_payload_size;
extern bool includeUnknownIEsInEbr;

//----------------------------------------------------------------------------
// Handlers for znet Mac test app
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sl_zigbee_energy_scan_result_handler_mac_test(uint8_t channel, int8_t maxRssiValue)
#else
void sl_zigbee_energy_scan_result_handler(uint8_t channel, int8_t maxRssiValue)
#endif
{
  if (!sli_zigbee_enable_mac_certification_test_mode) {
    return;
  }

  assert(sli_802154mac_pg_chan_ch(channel) < SL_ZIGBEE_MAX_CHANNELS_PER_PAGE);
  uint8_t edValue = sli_mac_lower_mac_convert_rssi_to_ed(sli_802154mac_pg_chan_pg(channel) == 0
                                                         // 0: 2.4Ghz, 1: subGhz;
                                                         ? PHY_INDEX_NATIVE : PHY_INDEX_PRO2PLUS,
                                                         maxRssiValue);

  sl_zigbee_core_debug_println("energy scan on ch %u: %d", channel, edValue);

  scanEnergyDetectValue[sli_802154mac_pg_chan_ch(channel)] = edValue;
  channelCount++;
}

// sl_zigbee_incoming_command_handler cannot be consumed because mac-address-filtering
// consumes it. Instead, we allow the stack to call us directly after filtering
// is done. This is only done for the zigbee_pro_compliance app.
// This runs as a privileged callback (since it takes in a buffer)
void sli_802154mac_test_commands_incoming_beacon_handler(sli_buffer_manager_buffer_t commandBuffer,
                                                         uint8_t indexOfCommand,
                                                         void *data)
{
  UNUSED_VAR(indexOfCommand);
  UNUSED_VAR(data);
  if (!sli_zigbee_enable_mac_certification_test_mode) {
    return;
  }

  //(void) sli_legacy_serial_printf_line(1, "emMacTestCommandsIncomingCommandHandler scanType %d", scanType);

  if (scanType == SL_ZIGBEE_ACTIVE_SCAN) {
    // Print beacon payload
    uint8_t startPayloadIndex = sli_zigbee_get_beacon_payload(commandBuffer, sli_zigbee_beacon_payload_buffer);
    uint8_t bufferLength = sli_mac_payload_length(commandBuffer);
    uint8_t payloadSize = bufferLength - startPayloadIndex;

    if (payloadSize > bufferLength) {
      sl_zigbee_core_debug_println("Invalid beacon payload size %d", payloadSize);
      sl_zigbee_core_debug_println("bufferLength %d startPayloadIndex %d", bufferLength, startPayloadIndex);
      return;
    } else {
      uint8_t i;
      sl_zigbee_core_debug_println("beacon: size = %d ", payloadSize);
      sl_zigbee_core_debug_print("beacon payload = \"");
      for (i = 0; i < payloadSize; i++) {
        sl_zigbee_core_debug_print("0x%02X ",
                                   sli_zigbee_beacon_payload_buffer[i]);
      }
      sl_zigbee_core_debug_println("\"");
    }

    if (collectPanDescriptors
        && panDescriptorCount < PAN_DESCRIPTOR_TABLE_SIZE) {
      PanDescriptor *pd = panDescriptorList + panDescriptorCount;
      uint8_t *contents = sli_mac_payload_pointer(commandBuffer);
      pd->coordAddrMode = sl_util_fetch_low_high_int16u(contents + BEACON_FRAME_CONTROL_OFFSET) >> 14;
      pd->coordPanId = sl_util_fetch_low_high_int16u(contents + BEACON_PAN_ID_OFFSET);
      pd->logicalChannel = sli_802154mac_pg_chan_ch(sli_zigbee_current_channel);
      pd->channelPage = sli_802154mac_pg_chan_pg(sli_zigbee_current_channel) ? (sli_802154mac_pg_chan_pg(sli_zigbee_current_channel) | 0x18)
                        : 0;
      if (pd->coordAddrMode == 0x02) {
        memcpy(pd->coordAddress, contents + BEACON_SENDER_ID_OFFSET, 2);
        pd->superFrameSpec = sl_util_fetch_low_high_int16u(contents + BEACON_SUPERFRAME_OFFSET);
        pd->gtsPermit = contents[BEACON_GTS_OFFSET] & 0x01;
      } else {
        memcpy(pd->coordAddress, contents + BEACON_SENDER_ID_OFFSET, 8);
        pd->superFrameSpec = sl_util_fetch_low_high_int16u(contents + BEACON_SUPERFRAME_OFFSET + 6);
        pd->gtsPermit = contents[BEACON_GTS_OFFSET + 6] & 0x01;
      }
      pd->lqi = sli_zigbee_current_lqi;
      pd->timeStamp = halCommonGetInt32uMillisecondTick();
      panDescriptorCount++;
    }
  }
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sl_zigbee_orphan_notification_handler_mac_test(sl_802154_long_addr_t longId)
#else
void sl_zigbee_orphan_notification_handler(sl_802154_long_addr_t longId)
#endif
{
  if (!sli_zigbee_enable_mac_certification_test_mode) {
    return;
  }

  sl_zigbee_core_debug_print("Received an Orphan scan from ");
  printBigEndianEui64(serialPort, longId);
  // Printing following information to make test house happy
  // and get expected print on terminal. I don't know why we even need this.
  sl_zigbee_core_debug_print(" Security level = 0x00 KeySource = NULL");

  sl_802154_short_addr_t shortId = sl_mac_find_child_short_id(longId);
  // We always call this handler in the reception of orphan notification but
  // does not send coordinator realignment if child entry is not present.
  if (shortId != SL_ZIGBEE_NULL_NODE_ID) {
    sl_zigbee_core_debug_println(", responding with a Coordinator"
                                 " Realignment for child 0x%02x", shortId);
  } else {
    sl_zigbee_core_debug_println(", child entry not present.");
  }
}

bool macTestPassthroughFilterHandler(uint8_t *macHeader, uint8_t macPayloadLength)
{
  if (!sli_zigbee_enable_mac_certification_test_mode) {
    return false;
  }

  uint16_t control = sl_util_fetch_low_high_int16u(macHeader);
  bool hasSecurity = (control & MAC_FRAME_FLAG_SECURITY_ENABLED) != 0;

  // We should not get this handler if security bit is enabled.
  // Packet gets reject in sli_802154phy_radio_receive_mac_header_callback if data packet has
  // security enabled.
  if (hasSecurity) {
    sl_zigbee_core_debug_print("Secured MAC frame discarded. Comm status indication: ");
    printMacHeaderInformation(macHeader);
    sl_zigbee_core_debug_println("SecurityLevel = 0x00  Status = xx");  // status don't care
  } else {
    processIncomingMacData(macHeader, macPayloadLength);
  }

  return false;
}

//----------------------------------------------------------------------------
// Application functions

static void processIncomingMacData(uint8_t *macHeader, uint8_t macPayloadLength)
{
  uint8_t i;

  uint8_t macHeaderLength = sli_mac_flat_mac_header_length(macHeader, false);
  uint8_t macPayloadOffset = macHeaderLength;

  sl_zigbee_core_debug_print("Data indication: ");

  if (macPayloadLength) {
    printMacHeaderInformation(macHeader); // pass mac header index, index zero contains phy packet length.

    // Security level is always zero
    sl_zigbee_core_debug_print("SecurityLevel = 0x00 ");

    sl_zigbee_core_debug_print("Length %d RX ", macPayloadLength);
    for (i = 0; i < macPayloadLength; i++) {
      sl_zigbee_core_debug_print("0x%02X ",
                                 macHeader[macPayloadOffset + i]);
    }
    sl_zigbee_core_debug_print("\r\n");
  }
  //cacheMacRawHeaderForMacTest = SL_ZIGBEE_NULL_MESSAGE_BUFFER;
}

void enableMacCertficationTestMode(SL_CLI_COMMAND_ARG)
{
  sli_zigbee_enable_mac_certification_test_mode = (uint8_t)sl_cli_get_argument_uint32(arguments, 0);

#ifdef SL_ZIGBEE_TEST
  // Turn this off, since it is expected to get invalid nwk header
  // in mac certification testing
  sli_zigbee_packet_validate_assert_on_invalid = false;
#endif
}

void setNodeTypeCommand(SL_CLI_COMMAND_ARG)
{
  sl_zigbee_node_type_t nodeType = sl_cli_get_argument_uint16(arguments, 0);
  assert(nodeType <= SL_ZIGBEE_SLEEPY_END_DEVICE);
  sli_zigbee_set_node_type(nodeType);
  sl_mac_set_coordinator(nodeType == SL_ZIGBEE_COORDINATOR);
  sli_zigbee_state = NETWORK_JOINED;
  sli_zigbee_set_multi_phy_state(SL_STATUS_NETWORK_UP);
#ifdef MAC_DUAL_PRESENT
  sli_zigbee_current_phy2_network->optionsMask = (sl_zigbee_multi_phy_nwk_config_t)(SL_ZIGBEE_MULTI_PHY_ROUTERS_ALLOWED
                                                                                    | SL_ZIGBEE_MULTI_PHY_BROADCASTS_ENABLED);
  // Native radio gets powered from sli_zigbee_set_node_type
  if (sli_zigbee_node_type_is_sleepy()) {
    sl_mac_lower_mac_set_radio_idle_mode(PHY_INDEX_PRO2PLUS, SL_ZIGBEE_RADIO_POWER_MODE_OFF);
  } else {
    sl_mac_lower_mac_set_radio_idle_mode(PHY_INDEX_PRO2PLUS, SL_ZIGBEE_RADIO_POWER_MODE_RX_ON);
  }
#endif
  // Update token for node type
  sli_zigbee_write_radio_and_network_tokens(sl_zigbee_get_eui64(), 0);
  sl_zigbee_core_debug_println("node type %s", nodeTypeNames[sli_zigbee_node_type]);
}

void setParentIdCommand(SL_CLI_COMMAND_ARG)
{
  //ToDo: Figure out how to set parentId when MAC_DUAL_PRESENT.
  sli_zigbee_parent_id = sl_cli_get_argument_uint16(arguments, 0);
}

void setIdsCommand(SL_CLI_COMMAND_ARG)
{
  uint8_t length;
  uint8_t *command = sl_zigbee_cli_get_argument_string_and_length(arguments, -1, &length);

  if (command[4] == 'p') {
    sl_zigbee_set_pan_id(sl_cli_get_argument_uint16(arguments, 0));
    sl_zigbee_core_debug_println("set pan id 0x%02x", sl_zigbee_get_pan_id());
  } else if (command[4] == 'n' ) {
    sl_zigbee_set_node_id(sl_cli_get_argument_uint16(arguments, 0));
    sl_zigbee_core_debug_println("set node id 0x%02x", sl_zigbee_get_node_id());
  } else if (command[4] == 'i') {
    sl_zigbee_set_node_id(sl_cli_get_argument_uint16(arguments, 0));
    sl_zigbee_set_pan_id(sl_cli_get_argument_uint16(arguments, 1));
    if (sl_cli_get_argument_count(arguments)  > 2) {
      // Because subghz beacon management tests expect enhanced beacon request
      // followed by association if it is valid.
      useZigbeeBeaconPayload = (bool)sl_cli_get_argument_uint32(arguments, 2);
    }
    sl_zigbee_core_debug_println("set node id 0x%02x pan id 0x%02x",
                                 sl_zigbee_get_node_id(), sl_zigbee_get_pan_id());
  } else {
    sl_zigbee_core_debug_println("Unknown Command");
    return;
  }

  SET_RADIO_PARAMETERS();
  sl_mac_kickstart(0);  // Trigger a network switch if we've newly added an rx on network

  // Update tokens
  sli_zigbee_write_node_data(false);
}

void associateCommand(SL_CLI_COMMAND_ARG)
{
  sl_802154_short_addr_t parentId = sl_cli_get_argument_uint16(arguments, 0);
  int16u panId         = sl_cli_get_argument_int16(arguments, 1);
  sl_status_t status = sl_mac_test_associate_command(parentId, panId);
  if (status == SL_STATUS_OK) {
    sl_zigbee_core_debug_println("starting associate");
  } else {
    sl_zigbee_core_debug_println("failed to start associate");
  }
}

void setBeaconCommand(SL_CLI_COMMAND_ARG)
{
  sli_zigbee_beacon_payload_size
    = sl_zigbee_copy_hex_arg(arguments, 0,
                             sli_zigbee_beacon_payload_buffer,
                             BEACON_PAYLOAD_SIZE,
                             false);
  sl_zigbee_core_debug_println("set beacon payload of size %d", sli_zigbee_beacon_payload_size);
}

void allowJoiningCommand(SL_CLI_COMMAND_ARG)
{
  nextChildId = sl_cli_get_argument_uint16(arguments, 0);
  sl_zigbee_permit_joining(0xFF); // 0xFF is always on
}

void setPanAtCapacityCommand(SL_CLI_COMMAND_ARG)
{
#ifdef SL_CATALOG_CLI_PRESENT
  UNUSED_VAR(arguments);
#endif // SL_CATALOG_CLI_PRESENT
  uint8_t i;
  uint8_t maxChildren = sl_zigbee_get_max_end_device_children();

  // Erase child(s) if any so that parent always tries to assign newId
  // from sli_zigbee_add_rejoined_child in association process and hit
  // (sli_zigbee_max_end_device_children == sli_zigbee_end_device_child_count)
  for (i = 0; i < maxChildren; i++) {
    if (sl_mac_get_child_info_flags(i) & SL_MAC_CHILD_IS_PRESENT) {
      sli_zigbee_erase_child(i);
    }
  }

  sli_zigbee_end_device_child_count = sl_zigbee_get_max_end_device_children();
  sl_zigbee_core_debug_println("Pan set at capacity");
}

void setMacRetriesCommand(SL_CLI_COMMAND_ARG)
{
  sl_mac_csma_parameters_t macParams;
  sl_mac_get_csma_params(&macParams);
  macParams.maxRetries = sl_cli_get_argument_uint8(arguments, 0);
  sl_mac_set_csma_params(&macParams);
  sl_zigbee_core_debug_println("Set mac retries to %d", macParams.maxRetries);
}

void delayNextPolledPacketTransmit(SL_CLI_COMMAND_ARG)
{
  delayPolledPacketTransmitTimeMs = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_core_debug_println("Delay next polled packet for %d ms", delayPolledPacketTransmitTimeMs);
}

void purgeMacQueueCommand(SL_CLI_COMMAND_ARG)
{
#ifdef SL_CATALOG_CLI_PRESENT
  UNUSED_VAR(arguments);
#endif // SL_CATALOG_CLI_PRESENT
  sl_mac_indirect_purge(sli_zigbee_get_current_network_index());
  sl_zigbee_core_debug_println("MAC indirect queue purged");
}

void sendMacCommand(SL_CLI_COMMAND_ARG)
{
  size_t macContentsLength;
  uint8_t *macContents = sl_cli_get_argument_hex(arguments, 0, &macContentsLength);
  sl_mac_test_send_mac_command((uint8_t)macContentsLength, macContents);
}

// Relocated the mac orphan scanning here from the zigbee scan.
// Runs as privileged (stack-context) callback.
extern uint8_t min_channel;
sl_mac_scan_request_result_t orphan_scan_request_callback(uint8_t channel, sli_buffer_manager_buffer_t *scan_packet)
{
  UNUSED_VAR(channel);
  if (*scan_packet == NULL_BUFFER) { // first time calling this callback druring scan
    *scan_packet = sli_mac_make_orphan_notification(0, sli_zigbee_get_current_network_index());
    if (*scan_packet == NULL_BUFFER) {
      // When the orphan notification can not be allocated for some reason (e.g there is a heavy traffic)
      // SL_MAC_SCAN_ABORT should be returned then the scan state will be cleaned up
      return SL_MAC_SCAN_ABORT;
    }
  }
  return SL_MAC_SCAN_GOTO_NEXT_CHANNEL;
}

//runs as privileged callback
void orphanScanCompleteHandler(uint32_t channelMask)
{
  sli_zigbee_set_current_network(
    sli_zigbee_get_zigbee_event_network_index(ZIGBEE_NWK_INDEX_OFFSET_SCAN_EVENT));

  if (channelMask) {
    for (uint8_t i = min_channel;
         i <= SL_ZIGBEE_MAX_802_15_4_CHANNEL_NUMBER;
         i++) {
      if ((channelMask & BIT32(i)) != 0) {
        // When the scan is complete,
        // any 1's in this mask represent send failures.
        sl_zigbee_scan_return(i,
                              SL_STATUS_MAC_COMMAND_TRANSMIT_FAILURE);
      }
    }
  }
  sl_zigbee_scan_return(SL_ZIGBEE_MAX_802_15_4_CHANNEL_NUMBER,
                        SL_STATUS_OK);

  sli_zigbee_restore_current_network();
}

void performScanning(SL_CLI_COMMAND_ARG)
{
  uint8_t length;
  sl_status_t status;
  uint8_t *command = sl_zigbee_cli_get_argument_string_and_length(arguments, -1, &length);
  uint8_t scanDuration = sl_cli_get_argument_uint8(arguments, 1);
  scanChannels = sl_cli_get_argument_uint32(arguments, 0);

  if (command[5] == 'a') {
    scanType = SL_ZIGBEE_ACTIVE_SCAN;
  } else if (command[5] == 'e') {
    scanType = SL_ZIGBEE_ENERGY_SCAN;
  } else if (command[5] == 'o') {
    scanType = SLI_ZIGBEE_ORPHAN_SCAN;
    sl_zigbee_test_perform_raw_active_scan(scanChannels, scanDuration);
  } else {
    sl_zigbee_core_debug_println("Unknown Command");
    return;
  }

#ifdef MAC_DUAL_PRESENT
  // Turn off native radio, as it gets turn on by default while setting up node type
  // (expected on non dual phy) to avoid spurious tx/rx packets.
  if (scanChannels & SL_ZIGBEE_ALL_CHANNEL_PAGE_MASK) {
    sl_mac_lower_mac_set_radio_idle_mode(PHY_INDEX_NATIVE, SL_ZIGBEE_RADIO_POWER_MODE_OFF);
  } else {
    sl_mac_lower_mac_set_radio_idle_mode(PHY_INDEX_PRO2PLUS, SL_ZIGBEE_RADIO_POWER_MODE_OFF);
  }
#endif
  // Mac test specification uses regular beacon for scanning on subghz interface too.
  useEnhancedBeacon = false;
  // Orphan scan is already started above, call zigbee stack api for other than that.
  if (scanType !=  SLI_ZIGBEE_ORPHAN_SCAN) {
    status = sl_zigbee_start_scan(scanType, scanChannels, scanDuration);
    if ( status != SL_STATUS_OK ) {
      sl_zigbee_core_debug_println("Scan attempt failed, 0x%0x", status);
    }
  }
  collectPanDescriptors = true;
  panDescriptorCount = 0;
  channelCount = 0;
}

void sl_zigbee_scan_return(uint8_t channel, sl_status_t status)
{
  uint8_t i;
  collectPanDescriptors = false;
  uint32_t unScannedChannels = 0;

  //We get sli_zigbee_scan_complete_handler per channel scan failure
  if (status == SL_STATUS_MAC_COMMAND_TRANSMIT_FAILURE) {
    unScannedChannels |= BIT(channel);
    return;
  }

  sl_zigbee_core_debug_println("scan complete status 0x%02X",
                               status);
  if (scanType == SL_ZIGBEE_ENERGY_SCAN
      || (scanType == SL_ZIGBEE_ACTIVE_SCAN && panDescriptorCount > 0)
      || scanType == SLI_ZIGBEE_ORPHAN_SCAN) {
    sl_zigbee_core_debug_println("Status=%s", status == 0 ? "SUCCESS" : "FAILED");
  } else {
    sl_zigbee_core_debug_println("Status=NO_BEACON");
  }
  // Print mapped Oprhan scan Id (0x03) as per mac test spec.
  sl_zigbee_core_debug_println("ScanType=0x%02X",
                               (scanType == SLI_ZIGBEE_ORPHAN_SCAN) ? 3 : scanType);
  // The 16 channels in the 2.4GHz band are contained in channel page 0.
  sl_zigbee_core_debug_println("ChannelPage= %lu", (unsigned long)(SL_ZIGBEE_ALL_CHANNEL_PAGE_MASK & scanChannels)
                               >> SL_ZIGBEE_MAX_CHANNELS_PER_PAGE);
  if (scanType == SL_ZIGBEE_ENERGY_SCAN) {
    sl_zigbee_core_debug_println("UnscannedChannels=not valid");
    sl_zigbee_core_debug_println("ResultListSize=%d", channelCount);
    sl_zigbee_core_debug_println("--- EnergyDetectList ---");
    for (i = SL_ZIGBEE_MIN_SUBGHZ_CHANNEL_NUMBER;
         i < SL_ZIGBEE_MAX_CHANNELS_PER_PAGE;
         i++) {
      if (BIT(i) & scanChannels) {
        sl_zigbee_core_debug_println("Channel=%u ED=%d",
                                     i, scanEnergyDetectValue[i]);
      }
    }
    sl_zigbee_core_debug_println("PANDescriptorList=NULL");
  } else {
    sl_zigbee_core_debug_println("UnscannedChannels=0x%04lx", (unsigned long)unScannedChannels);
    sl_zigbee_core_debug_println("ResultListSize=0x%02X",
                                 panDescriptorCount);
    sl_zigbee_core_debug_println("EnergyDetectList=NULL");
    if (panDescriptorCount) {
      sl_zigbee_core_debug_println("--- PANDescriptorList ---");
      for (i = 0; i < panDescriptorCount; i++) {
        printPanDescriptor(&panDescriptorList[i]);
        sl_zigbee_core_debug_print("\r\n");
      }
    } else {
      sl_zigbee_core_debug_println("PANDescriptorList=NULL");
    }
  }
}

static void printPanDescriptor(PanDescriptor *pd)
{
  uint8_t addrSize = (pd->coordAddrMode == 0x2) ? 2 : 8;
  int8_t i;

  sl_zigbee_core_debug_println("CoordAddrMode=0x%02X",
                               pd->coordAddrMode);
  sl_zigbee_core_debug_println("CoordPanId=0x%04X",
                               pd->coordPanId);

  sl_zigbee_core_debug_print("CoordAddress=0x");
  for (i = addrSize - 1; i >= 0; i--) {
    sl_zigbee_core_debug_print("%02X",
                               pd->coordAddress[i]);
  }
  sl_zigbee_core_debug_print("\r\n");

  sl_zigbee_core_debug_println("LogicalChannel=0x%02X",
                               pd->logicalChannel);
  sl_zigbee_core_debug_println("ChannelPage=%d", pd->channelPage);
  sl_zigbee_core_debug_println("SuperFrameSpec=0x%04X",
                               pd->superFrameSpec);
  if (pd->gtsPermit) {
    sl_zigbee_core_debug_println("GTSPermit=true");
  } else {
    sl_zigbee_core_debug_println("GTSPermit=false");
  }
  sl_zigbee_core_debug_println("sli_link_quality_t=0x%02X",
                               pd->lqi);
  sl_zigbee_core_debug_println("TimeStamp=0x%08lX",
                               (unsigned long)pd->timeStamp);
  sl_zigbee_core_debug_println("------------------------------");
}

void printInformationElementsFromEbr(sli_zigbee_packet_header_t header,
                                     sli_802154mac_frame_info_element_parse_result result,
                                     sli_802154mac_info_element_field* infoElementsArray)
{
  if (!sli_zigbee_enable_mac_certification_test_mode) {
    return;
  }

  if (result != EM_MAC_FRAME_INFO_ELEMENTS_PRESENT_AND_VALID) {
    return;
  }

  uint8_t i, j;

  sl_zigbee_core_debug_println("Received an EBR with information elements:");
  for (i = 0; i < EM_MAC_FRAME_MAX_INFO_ELEMENT_COUNT; i++) {
    sl_zigbee_core_debug_println("  Group Id = 0x%02x PIE 0x%02x", infoElementsArray[i].id,
                                 HIGH_LOW_TO_INT(sl_legacy_buffer_manager_get_linked_buffers_byte(header, infoElementsArray[i].indexInPacket + 1),
                                                 sl_legacy_buffer_manager_get_linked_buffers_byte(header, infoElementsArray[i].indexInPacket + 0)));
    if (infoElementsArray[i].id == 0x0F) {  // Termination IE
      break;
    }

    sl_zigbee_core_debug_print("    Sub-IE payload in hex");
    for (j = 0; j < infoElementsArray[i].length; j++) {
      sl_zigbee_core_debug_print(" %02X",
                                 sl_legacy_buffer_manager_get_linked_buffers_byte(header, infoElementsArray[i].indexInPacket + j + 2));
    }
    sl_zigbee_core_debug_println("");
  }
}

void enableEnahncedBeaconRequestWithUnknownIEs(SL_CLI_COMMAND_ARG)
{
  includeUnknownIEsInEbr = (bool)sl_cli_get_argument_uint32(arguments, 0);
}

// Mac test specifications specify error codes as per IEEE802.15.4 spec
// hence test house expect the same. This function maps Silabs error
// codes with specified in mac tests.
uint32_t sli_zigbee_map_ember_error_codes_to_mac_certification_tests(sl_status_t status)
{
  uint32_t mappedErrorCode;
  switch (status) {
    case SL_STATUS_MAC_NO_ACK_RECEIVED:
      mappedErrorCode = 0xE9;
      break;
    case SL_STATUS_TRANSMIT_BLOCKED:
    case SL_STATUS_TRANSMIT_SCHEDULER_FAIL:
    case SL_STATUS_CCA_FAILURE:
      mappedErrorCode = 0xE1;
      break;
    case SL_STATUS_MAC_INDIRECT_TIMEOUT:
      mappedErrorCode = 0xF0;
      break;
    default:
      mappedErrorCode = status;
      break;
  }
  return mappedErrorCode;
}

// This is one of mac primitives mac certification test expect, since we don't have primitives
// based implementation, this is a hook to print status indication.
// The MLME-COMM-STATUS.indication primitive is generated by the MLME and issued to its next
// higher layer either following a transmission instigated through a response primitive or on
// receipt of a frame that generates an error in its security processing.

void sl_802154mac_communication_status_indication_handler(uint8_t mac_index,
                                                          sl_status_t status,
                                                          uint8_t packet_length,
                                                          uint8_t *packet_data)
{
  // NOTE copied this type-cast from sli_mac_header_mac_info_frame_type
  (void)mac_index;
  (void)packet_length;
  sl_mac_in_memory_overhead_t *in_memory_packet = (sl_mac_in_memory_overhead_t *)packet_data;
  if ((in_memory_packet->info.mac_info_flags
       & (SL_802154_INFO_TYPE_BEACON
          | SL_802154_INFO_TYPE_DATA
          | SL_802154_INFO_TYPE_PASSTHROUGH
          | SL_802154_INFO_TYPE_MAC_COMMAND)) == SL_802154_INFO_TYPE_MAC_COMMAND) {
    uint8_t *macHeaderPointer = in_memory_packet->payload;
    uint8_t macHeaderLength = sli_mac_flat_mac_header_length(macHeaderPointer, false);
    macHeaderLength = (macHeaderLength > PACKET_BUFFER_SIZE ? PACKET_BUFFER_SIZE : macHeaderLength);

    uint8_t macHeaderContents[PACKET_BUFFER_SIZE];
    memcpy(macHeaderContents, &packet_data[SL_802154_IN_MEMORY_OVERHEAD], macHeaderLength);
    uint8_t commandId = packet_data[SL_802154_IN_MEMORY_OVERHEAD + macHeaderLength];
    switch (commandId) {
      case MAC_ASSOCIATION_RESPONSE:
      case MAC_COORDINATOR_REALIGN:
        sl_zigbee_core_debug_print("Comm status indication: ");
        printMacHeaderInformation(macHeaderContents);
        sl_zigbee_core_debug_println("SecurityLevel = 0x00  Status = 0x%02X",
                                     sli_zigbee_map_ember_error_codes_to_mac_certification_tests(status));
        break;
      default:
        // Do nothing
        break;
    }
  }
}

static void printMacHeaderInformation(uint8_t* macHeader)
{
  if (macHeader == NULL) {
    return;
  }

  uint16_t frameControl = sl_util_fetch_low_high_int16u(&macHeader[0]);
  uint8_t finger = 3; // points to dest panId index

  // Check for intra/inter pan and print information
  if (frameControl & MAC_FRAME_FLAG_INTRA_PAN) {
    sl_zigbee_core_debug_print("SrcPanId = DstPanId = 0x%04X",
                               sl_util_fetch_low_high_int16u(&macHeader[finger]));
  } else {
    sl_zigbee_core_debug_print("DstPanId = 0x%04X",
                               sl_util_fetch_low_high_int16u(&macHeader[finger]));
  }
  finger += 2;

  // Check for Dst mode and print information
  if ((frameControl & MAC_FRAME_DESTINATION_MODE_MASK) == MAC_FRAME_DESTINATION_MODE_LONG) {
    sl_zigbee_core_debug_print(" dstAddr mode 0x%02X",
                               (MAC_FRAME_DESTINATION_MODE_LONG >> 10));
    sl_zigbee_core_debug_print(" dstAddr ");
    printBigEndianEui64(serialPort, &macHeader[finger]);
    sl_zigbee_core_debug_print(" ");
    finger += EUI64_SIZE;
  } else {
    sl_zigbee_core_debug_print(" dstAddr mode 0x%02X",
                               (MAC_FRAME_DESTINATION_MODE_SHORT >> 10));
    sl_zigbee_core_debug_print(" dstAddr 0x%04X",
                               sl_util_fetch_low_high_int16u(&macHeader[finger]));
    finger += 2;
  }

  // If it is a inter pan packet, print srcPandId
  if (!(frameControl & MAC_FRAME_FLAG_INTRA_PAN)) {
    sl_zigbee_core_debug_print(" SrcPanId = 0x%04X",
                               sl_util_fetch_low_high_int16u(&macHeader[finger]));
    finger += 2;
  }

  // Check for src mode and print information
  if ((frameControl & MAC_FRAME_SOURCE_MODE_MASK) == MAC_FRAME_SOURCE_MODE_LONG) {
    sl_zigbee_core_debug_print(" srcAddr mode 0x%02X",
                               (MAC_FRAME_SOURCE_MODE_LONG >> 14));
    sl_zigbee_core_debug_print(" srcAddr ");
    printBigEndianEui64(serialPort, &macHeader[finger]);
    sl_zigbee_core_debug_print(" ");
  } else {
    sl_zigbee_core_debug_print(" srcAddr mode 0x%02X",
                               (MAC_FRAME_SOURCE_MODE_SHORT >> 14));
    sl_zigbee_core_debug_print(" srcAddr 0x%04X ",
                               sl_util_fetch_low_high_int16u(&macHeader[finger]));
  }
}
#endif  // MAC_TEST_COMMANDS_SUPPORT
