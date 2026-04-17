/***************************************************************************//**
 * @file
 * @brief This file handles common routines for applications
 * that implement basic Security.
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

bool isSecurityStateValid(void);
void printYesOrNo(uint16_t yes);
void setTrustCenterPolicyCommand(SL_CLI_COMMAND_ARG);
void getSecurityCommand(SL_CLI_COMMAND_ARG);
void networkKeyCommand(void);
void keyUpdateCommand(SL_CLI_COMMAND_ARG);
void keySwitchCommand(SL_CLI_COMMAND_ARG);
void requestKeyCommand(SL_CLI_COMMAND_ARG);
void setSecurityStateBitmaskCommand(void);
void setExtendedSecurityBitmaskCommand(SL_CLI_COMMAND_ARG);
void getExtendedSecurityBitmaskCommand(SL_CLI_COMMAND_ARG);

// Platform specific functions
extern uint8_t joinDecision;
bool setTrustCenterJoinDecision(uint8_t decision);
bool getKeyFromCore(sl_zigbee_key_data_t* key);
uint8_t getSecurityLevel(void);
void printSecurityInfo(void);
sl_status_t sendKeyUpdateToTarget(sl_802154_short_addr_t targetShort,
                                  sl_802154_long_addr_t targetLong,
                                  sl_zigbee_key_data_t* newKey);

// Additional sl_zigbee_join_decision_t for the set_tc_policy command.
#define BECOME_TRUST_CENTER 0x80

extern const char * carriageReturn;

#define COMMON_SECURITY_COMMANDS                                                                     \
  /* A default sl_zigbee_join_decision_t or SL_ZIGBEE_NO_ACTION + 1, which causes */                 \
  /* the recipient to become the trust center. */                                                    \
  sl_zigbee_command_entry_action("tc_policy", setTrustCenterPolicyCommand, "u",                      \
                                 "Set Trust Center Join Policy"),                                    \
                                                                                                     \
  /* Print Security Info */                                                                          \
  /* security level, network key token, and key currently being used. */                             \
  sl_zigbee_command_entry_action("get_security", getSecurityCommand, "",                             \
                                 "Prints the current security settings"),                            \
                                                                                                     \
  /* Set the Alternate Network Key and transport it to all the nodes. */                             \
  /* Only for use by the Trust Center/Coordinator. */                                                \
  /* Parameters: <target-node-id> <16-byte key in hex> */                                            \
  /* You can specify "" or {} for the key to make the stack randomly */                              \
  /* generate a new key. */                                                                          \
  sl_zigbee_command_entry_action("key_update", keyUpdateCommand, "vb",                               \
                                 "Sets the Alt NWK key and sends that out to the specified target"), \
                                                                                                     \
  /* Send a network broadcast to switch to the new network key */                                    \
  /* Only for use by the Trust Center/Coordinator. */                                                \
  sl_zigbee_command_entry_action("key_switch", keySwitchCommand, "",                                 \
                                 "Sends a network broadcast to switch to the new key"),              \
                                                                                                     \
  /* Request a key from the Trust Center with the Partner Device */                                  \
  /* Param:  IEEE address of partner. */                                                             \
  /* If "" or {} is passed in, the Trust Center IEEE is assumed. */                                  \
  sl_zigbee_command_entry_action("request_key", requestKeyCommand, "b",                              \
                                 "Requests a key from the trust center"),                            \
                                                                                                     \
  /* Set the Extended Security Bitmask. It can only be set before */                                 \
  /* the node has joined a network */                                                                \
  sl_zigbee_command_entry_action("set_ext_bmask", setExtendedSecurityBitmaskCommand, "v",            \
                                 "Sets the extended security bitmask"),                              \
  /* It prints out the current Extended Security Bitmask. This value */                              \
  /* can only be retrieved if the node has already joined a network */                               \
  sl_zigbee_command_entry_action("get_ext_bmask", getExtendedSecurityBitmaskCommand, "",             \
                                 "Prints the current extended security bitmask"),

#define MIN_BROADCAST_ADDRESS 0xFFF8

#define isBroadcastAddress(address) \
  (MIN_BROADCAST_ADDRESS <= ((uint16_t) (address)))
