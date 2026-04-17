/**************************************************************************//**
 * Copyright 2017 Silicon Laboratories, Inc.
 *
 *****************************************************************************/

#include "mpsi-message-ids.h"

#ifdef EMBER_STACK_ZIGBEE
  #define getNetworkParameters(nodeTypePtr, parametersPtr) \
  sl_zigbee_get_network_parameters(nodeTypePtr, parametersPtr);
 #define addTransientLinkKey sl_zb_sec_man_import_transient_link_key
#endif // EMBER_STACK_ZIGBEE

// Message callbacks
uint8_t mpsiHandleMessageGetAppsInfo(MpsiMessage_t *mpsiMessage);
uint8_t mpsiHandleMessageAppsInfo(MpsiMessage_t *mpsiMessage);
uint8_t mpsiHandleMessageBootloadSlot(MpsiMessage_t *mpsiMessage);
uint8_t mpsiHandleMessageError(MpsiMessage_t *mpsiMessage);
uint8_t mpsiHandleMessageInitiateJoining(MpsiMessage_t *mpsiMessage);
uint8_t mpsiHandleMessageGetZigbeeJoiningDeviceInfo(MpsiMessage_t *mpsiMessage);
uint8_t mpsiHandleMessageZigbeeJoiningDeviceInfo(MpsiMessage_t *mpsiMessage);
uint8_t mpsiHandleMessageSetZigbeeJoiningDeviceInfo(MpsiMessage_t *mpsiMessage);
uint8_t mpsiHandleMessageGetZigbeeTrustCenterJoiningCredentials(
  MpsiMessage_t *mpsiMessage);
uint8_t mpsiHandleMessageZigbeeTrustCenterJoiningCredentials(
  MpsiMessage_t *mpsiMessage);
uint8_t mpsiHandleMessageSetZigbeeTrustCenterJoiningCredentials(
  MpsiMessage_t *mpsiMessage);
