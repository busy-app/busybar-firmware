/***************************************************************************//**
 * @file
 * @brief Miscellaneous Network Commands such as sending bindings requests,
 * manipulating groups, sending ZDO commands, etc.
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

void printFailedToErrorMessage(const char * message);

void setEui64(SL_CLI_COMMAND_ARG);

bool lookupGroupEndpoint(uint8_t endpoint, sl_zigbee_multicast_id_t groupId);

void pollEventHandler0(sl_zigbee_af_event_t * event);
void pollEventHandler1(sl_zigbee_af_event_t * event);
void pollEventHandler2(sl_zigbee_af_event_t * event);
void pollEventHandler3(sl_zigbee_af_event_t * event);
void pollEventHandler(uint8_t nwkIndex);
void pollCommand(SL_CLI_COMMAND_ARG);
void zigbeeLeaveCommand(SL_CLI_COMMAND_ARG);
void zigbeeEvictCommand(SL_CLI_COMMAND_ARG);
void scanCommand(SL_CLI_COMMAND_ARG);
void allowBeaconsCommand(SL_CLI_COMMAND_ARG);

// Node only
void beaconSuppressionTick(void);
void sleepCommand(SL_CLI_COMMAND_ARG);

// Host only
void printNotSupported(void);

//------------------------------------------------------------------------------

extern uint32_t pollDelay[4];
extern sl_zigbee_af_event_t *pollEvent[4];

#define POLL_EVENT_DEFINITION           \
  { &pollEvent[0], pollEventHandler0 }, \
  { &pollEvent[1], pollEventHandler1 }, \
  { &pollEvent[2], pollEventHandler2 }, \
  { &pollEvent[3], pollEventHandler3 },

//------------------------------------------------------------------------------

#define MISC_COMMON_COMMANDS                                       \
  /* poll interval (milliseconds) */                               \
  sl_zigbee_command_entry_action("poll", pollCommand, "v",         \
                                 "Set MAC data polling interval"), \
                                                                   \
  sl_zigbee_command_entry_action("scan", scanCommand, "?uw",       \
                                 "Starts an active scan"),         \
  /* new eui64 */                                                  \
  sl_zigbee_command_entry_action("set_eui", setEui64, "b",         \
                                 "Sets local EUI64 (not persistent)"),

#define MISC_ONBOARD_COMMANDS                                                       \
  /* Onboard application support commands. */                                       \
                                                                                    \
  /* Tell this node to leave */                                                     \
  /* remove children? - parameter is deprecated */                                  \
  sl_zigbee_command_entry_action("zleave", zigbeeLeaveCommand, "u",                 \
                                 "Tell specified node to leave network"),           \
                                                                                    \
  /* Kick out devices: */                                                           \
  /* If I am the parent, send a NWK Leave */                                        \
  /* If I am the Coordinator, send an APS Remove Device to parent. */               \
  /* params: parent node ID, child EUI64, remove children? */                       \
  /* Note: remove children? parameter is deprecated. */                             \
  sl_zigbee_command_entry_action("evict", zigbeeEvictCommand, "vbu",                \
                                 "Kick device out of network, one way or another"), \
                                                                                    \
                                                                                    \
  /* Turn on and off the radio to simulate failed links */                          \
  sl_zigbee_command_entry_action("radio_sleep", radioCommand, "",                   \
                                 "Put the radio to sleep"),                         \
  sl_zigbee_command_entry_action("radio_wakeup", radioCommand, "",                  \
                                 "Wake up the radio"),                              \
                                                                                    \
  /* Use approved method for powering down non-sleepy. */                           \
  /* Commented out to save flash. */                                                \
  /* { "sleep",        sleepCommand,          "w"},    */                           \
                                                                                    \
  /* Toggle beacon transmission */                                                  \
  sl_zigbee_command_entry_action("send_beacons", allowBeaconsCommand, "u1",         \
                                 "Toggle beacon transmission"),

//------------------------------------------------------------------------------
