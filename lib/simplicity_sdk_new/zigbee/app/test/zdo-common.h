/***************************************************************************//**
 * @file
 * @brief ZDO related functionality common to host applications and onboard (i.e.
 * 250/2420) ones.
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

void sendBindUnbindCommand(SL_CLI_COMMAND_ARG);

void eraseBindingsCommand(SL_CLI_COMMAND_ARG);
void printBindingsCommand(SL_CLI_COMMAND_ARG);
void remoteBindingsCommand(SL_CLI_COMMAND_ARG);

void zdoRequestCommand(SL_CLI_COMMAND_ARG);
void serverRequestCommand(SL_CLI_COMMAND_ARG);
void sendSimpleRequestCommand(SL_CLI_COMMAND_ARG);

bool handleZdoClusterMessage(sl_zigbee_aps_frame_t *apsFrame,
                             sl_zigbee_rx_packet_info_t *packetInfo,
                             uint8_t messageLength,
                             uint8_t *message);

void sendMatchRequestCommand(SL_CLI_COMMAND_ARG);

void rtgRequestCommand(SL_CLI_COMMAND_ARG);

void sendZdoLeave(SL_CLI_COMMAND_ARG);
void sendZdoLeaveRequest(SL_CLI_COMMAND_ARG);
void sendNodeDescriptorRequest(SL_CLI_COMMAND_ARG);
void zdoMgmtLqiCommandZCP(SL_CLI_COMMAND_ARG);
//------------------------------------------------------------------------------

#define ZDO_COMMON_COMMANDS                                                               \
  /* target, */                                                                           \
  /* source EUI64, source endpoint, */                                                    \
  /* destination EUI64, destination endpoint, */                                          \
  /* clusterID */                                                                         \
  /* Although dest endpoint is only 1 byte, we make it 2 so it can also be */             \
  /* used for group address (2 bytes) in group bind/unbind below. */                      \
  sl_zigbee_command_entry_action("send_bind", sendBindUnbindCommand, "vbubvv",            \
                                 "Sends a ZDO bind command to target"),                   \
  sl_zigbee_command_entry_action("send_unbind", sendBindUnbindCommand, "vbubvv",          \
                                 "Sends a ZDO UN-bind command to target"),                \
                                                                                          \
  /* Binding/Unbinding commands using groups */                                           \
  /* target */                                                                            \
  /* source EUI64, source endpoint */                                                     \
  /* destination EUI64 (ignored, but needed for 'send_bind/send_unbind' */                \
  /* above), destination group, clusterID */                                              \
  sl_zigbee_command_entry_action("send_g_bind", sendBindUnbindCommand, "vbubvv",          \
                                 "Sends a ZDO binding command for a group address"),      \
  sl_zigbee_command_entry_action("send_g_unbi", sendBindUnbindCommand, "vbubvv",          \
                                 "Sends a ZDO UN-binding command for a group address"),   \
                                                                                          \
  sl_zigbee_command_entry_action("print_binds", printBindingsCommand, "",                 \
                                 "Print binding table"),                                  \
  sl_zigbee_command_entry_action("remote_binds", remoteBindingsCommand, "v",              \
                                 "Send a request to retrieve bindings of remote device"), \
  sl_zigbee_command_entry_action("erase_binds", eraseBindingsCommand, "",                 \
                                 "Remove all local bindings"),                            \
                                                                                          \
  /* Send unsupported ZDO Request.  These messages may not be correctly */                \
  /* formatted since our stack assumes the request requires only a  */                    \
  /* transaction ID and a target address */                                               \
  /* Params: */                                                                           \
  /*   request cluster (node descriptor = 0x02, power descriptor = 0x03, */               \
  /*                    active endpoints = 0x05), */                                      \
  /*   target */                                                                          \
  /*   payload (can be empty) */                                                          \
  sl_zigbee_command_entry_action("zdo_request", zdoRequestCommand, "uvb",                 \
                                 "Send arbitrary ZDO request command"),                   \
                                                                                          \
  /* Send MGMT_RTG_REQ (Management Routing Request) */                                    \
  /* Params: target, start index */                                                       \
  /*   (Commented out to save flash/const) */                                             \
  /*sl_zigbee_command_entry_action("rtg_request", rtgRequestCommand, "vu", */             \
  /* "Send a MGMT_RTG_REQ command") */                                                    \
                                                                                          \
  /* target, endpoint */                                                                  \
  sl_zigbee_command_entry_action("simple_desc", sendSimpleRequestCommand, "vu",           \
                                 "Send a ZDO simple descriptor request"),                 \
                                                                                          \
  /* System Server Discovery Request: target, server mask */                              \
  sl_zigbee_command_entry_action("ser_req", serverRequestCommand, "vv",                   \
                                 "Send a ZDO System server Discovery Request"),           \
                                                                                          \
  /* target, profile, in clusters, out clusters */                                        \
  /* (Cluster lists are 2 bytes long and big endian */                                    \
  /* e.g. "00548002" specifies a list of 0x0054, 0x8002) */                               \
  sl_zigbee_command_entry_action("match_desc", sendMatchRequestCommand, "vvbb",           \
                                 "Send a Match Descriptor Request"),                      \
                                                                                          \
  /* ZDO leave */                                                                         \
  /*   Params: Target (node id), remove children (bool), */                               \
  /*     rejoin network (bool), aps options */                                            \
  /* Commented out due to lack of DATA space. */                                          \
  /* sl_zigbee_command_entry_action("zdo_leave",  sendZdoLeave, "vuuv", */                \
  /* "Send a ZDO leave request") */                                                       \
                                                                                          \
  /* ZDO leave request*/                                                                  \
  /*   Params: Target (node id), Target EUI, */                                           \
  /*     leave request flags, aps options */                                              \
  sl_zigbee_command_entry_action("zdo_leave_req", sendZdoLeaveRequest, "vbuv",            \
                                 "Send a ZDO leave request"),                             \
  sl_zigbee_command_entry_action("node_desc", sendNodeDescriptorRequest, "vv",            \
                                 "Send a ZDO leave request"),                             \
  sl_zigbee_command_entry_action("zdo_mgmt_lqi", zdoMgmtLqiCommandZCP, "vu",              \
                                 "Send a ZDO MGMT-LQI Request"),
