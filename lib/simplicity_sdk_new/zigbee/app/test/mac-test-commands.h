/***************************************************************************//**
 * @file
 * @brief Cli commands for mac certification.
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
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
void enableMacCertficationTestMode(SL_CLI_COMMAND_ARG);
void setNodeTypeCommand(SL_CLI_COMMAND_ARG);
void setParentIdCommand(SL_CLI_COMMAND_ARG);
void setIdsCommand(SL_CLI_COMMAND_ARG);
void associateCommand(SL_CLI_COMMAND_ARG);
void setBeaconCommand(SL_CLI_COMMAND_ARG);
void allowJoiningCommand(SL_CLI_COMMAND_ARG);
void setChildTableSizeCommand(void);
void setPanAtCapacityCommand(SL_CLI_COMMAND_ARG);
void performScanning(SL_CLI_COMMAND_ARG);
void sendMacCommand(SL_CLI_COMMAND_ARG);
void setMacRetriesCommand(SL_CLI_COMMAND_ARG);
void delayNextPolledPacketTransmit(SL_CLI_COMMAND_ARG);
void purgeMacQueueCommand(SL_CLI_COMMAND_ARG);
void enableEnahncedBeaconRequestWithUnknownIEs(SL_CLI_COMMAND_ARG);
uint32_t sli_zigbee_map_ember_error_codes_to_mac_certification_tests(sl_status_t status);
bool macTestPassthroughFilterHandler(uint8_t *macHeader, uint8_t macPayloadLength);
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
void sl_zigbee_energy_scan_result_handler_mac_test(uint8_t channel, int8_t maxRssiValue);
void sl_zigbee_orphan_notification_handler_mac_test(sl_802154_long_addr_t longId);
#endif
#define MAC_CERTIFICATION_TEST_COMMANDS                                                                   \
                                                                                                          \
  sl_zigbee_command_entry_action("mac_cert_mode", enableMacCertficationTestMode, "u",                     \
                                 "Enable 802.15.4 mac certification test mode, 1 : enable \
                          0: disable. It is not persistent across reboots."),                             \
                                                                                                          \
  sl_zigbee_command_entry_action("set_node_type", setNodeTypeCommand, "u",                                \
                                 "Set Node type, 1 : Coordinator 2: Router 3: End device \
                          4: Sleepy End device."),                                                        \
                                                                                                          \
  sl_zigbee_command_entry_action("set_ids", setIdsCommand, "vvu*",                                        \
                                 "Set the short address and the PAN ID. If short \
                          address is 0xFFFE then device chooses to send beacon with  \
                          extended src addressing mode in response to beacon request. \
                          Also optional provision to use either custom/no beacon payload  \
                          or zigbee beacon payload, default is custom/no beacon payload."),               \
                                                                                                          \
  sl_zigbee_command_entry_action("set_pan_id", setIdsCommand, "v",                                        \
                                 "Set the pan id."),                                                      \
                                                                                                          \
  sl_zigbee_command_entry_action("set_node_id", setIdsCommand, "v",                                       \
                                 "Set the node id."),                                                     \
                                                                                                          \
  sl_zigbee_command_entry_action("set_parent_id", setParentIdCommand, "v",                                \
                                 "Set the parent id."),                                                   \
                                                                                                          \
  sl_zigbee_command_entry_action("associate", associateCommand, "vv",                                     \
                                 "Associate to provided parent, PAN ID and optional phy interface. \
                          Capabilities are used based on node type."),                                    \
                                                                                                          \
  sl_zigbee_command_entry_action("set_beacon", setBeaconCommand, "b",                                     \
                                 "Send beacon with provided payload."),                                   \
                                                                                                          \
  sl_zigbee_command_entry_action("allow_joining", allowJoiningCommand, "v",                               \
                                 "Use short address in the next association response."),                  \
                                                                                                          \
  sl_zigbee_command_entry_action("set_pan_at_capacity", setPanAtCapacityCommand, "",                      \
                                 "It sets the stack so that it responds to an association request \
                          with an association response with status = 0x01 = 'pan at capacity'."),         \
                                                                                                          \
  sl_zigbee_command_entry_action("scan-active", performScanning, "wu",                                    \
                                 "Perform active scan on provided channel mask and duration."),           \
                                                                                                          \
  sl_zigbee_command_entry_action("scan-energy", performScanning, "wu",                                    \
                                 "Perform energy scan on provided channel mask and duration."),           \
                                                                                                          \
  sl_zigbee_command_entry_action("scan-orphan", performScanning, "wu",                                    \
                                 "Perform orphan scan on provided channel mask and duration."),           \
                                                                                                          \
  sl_zigbee_command_entry_action("enable_unknown_ie_ebr", enableEnahncedBeaconRequestWithUnknownIEs, "u", \
                                 "Enable enhanced beacon request with unknown information elements."),    \
                                                                                                          \
  sl_zigbee_command_entry_action("send_mac", sendMacCommand, "b",                                         \
                                 "Transmit the given packet"),                                            \
                                                                                                          \
  sl_zigbee_command_entry_action("set_mac_retries", setMacRetriesCommand, "u",                            \
                                 "The maximum number of times the mac will attempt retransmission \
                          of an unacknowledged packet."),                                                 \
                                                                                                          \
  sl_zigbee_command_entry_action("purge_mac_queue", purgeMacQueueCommand, "",                             \
                                 "Removes the packets enqued in the indirect queue."),                    \
                                                                                                          \
  sl_zigbee_command_entry_action("delay_polled_pkt_tx_ms", delayNextPolledPacketTransmit, "u",            \
                                 "Add delay in milliseconds to transmit next polled packet to  \
                          a sleepy device."),

#endif // MAC_TEST_COMMANDS_SUPPORT
