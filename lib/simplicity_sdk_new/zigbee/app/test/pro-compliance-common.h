/***************************************************************************//**
 * @file
 * @brief Common functionality between the host and onboard versions
 * of Ember's Zigbee Pro Compliance application.
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

void nwkUpdateCommand(SL_CLI_COMMAND_ARG);
void manyToOneCommand(SL_CLI_COMMAND_ARG);
void setSendOptionsCommand(SL_CLI_COMMAND_ARG);
void linkKeyCommand(SL_CLI_COMMAND_ARG);
void deleteKeyCommand(SL_CLI_COMMAND_ARG);
void clearKeysCommand(SL_CLI_COMMAND_ARG);
void keysCommand(SL_CLI_COMMAND_ARG);
void noteIncomingCommand(SL_CLI_COMMAND_ARG);
void sendViaBind(SL_CLI_COMMAND_ARG);
void printEncryptionKey2(uint8_t port, uint8_t* key);

void networkFoundHandler(sl_zigbee_zigbee_network_t *networkFound);
void scanCompleteHandler(uint8_t channel, sl_status_t status);
void stackStatusHandler(sl_status_t status);
void keyEstablishmentHandler(sl_802154_long_addr_t partner, sl_zigbee_key_status_t status);
void childJoinHandler(uint8_t index,
                      bool joining,
                      sl_802154_short_addr_t newNodeId,
                      sl_802154_long_addr_t childEui64,
                      sl_zigbee_node_type_t childType);
void incomingRouteErrorHandler(sl_status_t status, sl_802154_short_addr_t target);
void incomingMessageHandler(sl_zigbee_incoming_message_type_t type,
                            sl_zigbee_aps_frame_t *apsFrame,
                            sl_zigbee_rx_packet_info_t *packetInfo,
                            uint8_t messageLength,
                            uint8_t *message);

void noteIncoming(sl_zigbee_incoming_message_type_t type,
                  sl_zigbee_aps_frame_t *apsFrame,
                  sl_zigbee_rx_packet_info_t *packetInfo,
                  uint8_t messageLength,
                  uint8_t *message);

void printMessagePayload(sl_zigbee_aps_frame_t *apsFrame,
                         uint8_t messageLength,
                         uint8_t *message);

#define NO_DERIVED_KEY           0xFF // arbitrary indicator
#define DERIVE_TRANSPORT_KEY_TAG 0x00 // real value
#define DERIVE_LOAD_KEY_TAG      0x02 // ""

extern bool sli_zigbee_note_incoming_packet_enabled;

//------------------------------------------------------------------------------

#define PRO_COMPLIANCE_COMMON_COMMANDS                                                          \
  /* Send NWK Update Request */                                                                 \
  /*   Params: Target, Channel Mask (0-15, 16-31), Scan Duration,  */                           \
  /*     Nwk Manager Address (if Scan Duration == 0xFF) or Scan Count */                        \
  /* Examples: */                                                                               \
  /*   nwk_update 0xFFFF 0xF800 0x07FF 0xFF 0x1234 */                                           \
  /*     Broadcast NWK manager update.  New NWK Manager is set */                               \
  /*     to 0x1234 the channel mask is set to 11-26 */                                          \
  /*   nwk_update 0x4567 0xF800 0x0000 0x5 0x2 */                                               \
  /*     Tell 0x4567 to scan channels 11-15 with duration 5 and count 2. */                     \
  /*   nwk_update 0xFFFF 0x1000 0x0000 0xFE 0x0 */                                              \
  /*     Broadcast Channel change to change to channel 12 */                                    \
  sl_zigbee_command_entry_action("nwk_update", nwkUpdateCommand, "vvvuv",                       \
                                 "Send a NWK update request"),                                  \
                                                                                                \
  /* Create aggregation routes */                                                               \
  /* Parameters: high ram?, radius */                                                           \
  sl_zigbee_command_entry_action("many_to_one", manyToOneCommand, "uu",                         \
                                 "Send a many-to-one route request"),                           \
                                                                                                \
  /* Change outgoing message type used by sli_zigbee_stack_send_unicast()  */                   \
  sl_zigbee_command_entry_action("send_options", setSendOptionsCommand, "u",                    \
                                 "Change default sending options"),                             \
                                                                                                \
  /* Send to all recipients in the binding table that match the */                              \
  /* passed source endpoint and cluster ID.  Only Buffer Test request */                        \
  /* and freeform request are supported.  The last param will be the */                         \
  /* first byte of the message. */                                                              \
  /* Params: <source-endpoint> <cluster-id> <data> */                                           \
  sl_zigbee_command_entry_action("send_v_bind", sendViaBind, "uvu",                             \
                                 "Send to all recipients in binding table that match request"), \
                                                                                                \
  /* Delete a Link Key */                                                                       \
  /*   Param:  Partner EUI64 */                                                                 \
  sl_zigbee_command_entry_action("delete_key", deleteKeyCommand, "b",                           \
                                 "Delete an entry in the key table"),                           \
                                                                                                \
  /* Delete all keys */                                                                         \
  sl_zigbee_command_entry_action("clear_keys", clearKeysCommand, "",                            \
                                 "Clear all entries in the key table"),                         \
                                                                                                \
  /* Set a Link Key between local node and partner */                                           \
  /* Used in Standard Security Policy only. */                                                  \
  /* Params: Partner EUI64, Link Key (or "" for random) */                                      \
  sl_zigbee_command_entry_action("link_key", linkKeyCommand, "bb",                              \
                                 "Set a link key entry in the table"),                          \
                                                                                                \
  /* Derived Keys */                                                                            \
  /*   Print out all the derived keys (key-transport and key-link) */                           \
  /*   for all the keys in the Link Key table */                                                \
  /* (Commented out to save stack space). */                                                    \
  /*  { "derived_keys",    derivedKeysCommand, "" }, */                                         \
                                                                                                \
  /* Print the Master and Link Keys */                                                          \
  sl_zigbee_command_entry_action("keys", keysCommand, "",                                       \
                                 "Print the key table"),                                        \
  /* Enable/disable printing incoming frames  */                                                \
  /* Params <enable>  */                                                                        \
  sl_zigbee_command_entry_action("note_incoming", noteIncomingCommand, "u",                     \
                                 "Enable/disable note incoming"),
