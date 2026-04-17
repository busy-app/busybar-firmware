/***************************************************************************//**
 * @file
 * @brief GPD commissioning functions.
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
#include "gpd-components-common.h"
#include "gpd-apps-cb.h"
// Build commissioning frame based on the GPD configuration.
// most of the fields are compile time configuration.
static uint8_t gpdBuildCommissioningCommand(sl_zigbee_gpd_t_t * gpd,
                                            bool keyPresentInComRequest,
                                            uint8_t keyType,
                                            uint8_t* key,
                                            uint32_t frameCounter,
                                            uint8_t* cmd)
{
  uint8_t index = 0;
  cmd[index++] = GP_CMD_COMMISSIONING;
  // ZGP Device ID
  cmd[index++] = SL_ZIGBEE_AF_PLUGIN_APPS_APP_DEVICE_ID;

  // Options field ------------------------------------------------------------
  cmd[index] = (SL_ZIGBEE_AF_PLUGIN_APPS_MAC_SEQ ? SL_ZIGBEE_GPD_COM_OPT_MAC_SEQ_NBR_CAP_MASK : 0)     // b0 - MAC Seq Nbr capability sub-field
               | (SL_ZIGBEE_AF_PLUGIN_APPS_BIDIR_CAPABLE ? SL_ZIGBEE_GPD_COM_OPT_RX_CAP_MASK : 0)      // b1 - RxOn capability sub-field
               | (SL_ZIGBEE_AF_PLUGIN_APPS_APP_INFO ? SL_ZIGBEE_GPD_COM_OPT_APP_INFO_MASK : 0)         // b2 - Application Information sub-field
               | (SL_ZIGBEE_AF_PLUGIN_APPS_PAN_ID_REQUEST ? SL_ZIGBEE_GPD_COM_OPT_PANID_REQ_MASK : 0); // b4 - Pan ID Request sub-field
  // Key Request sub-field
  // do not request key if shared key already configured or bidirectional is false
  if ((keyType == SL_ZIGBEE_GPD_SECURITY_KEY_TYPE_NWK)
      || (keyType == SL_ZIGBEE_GPD_SECURITY_KEY_TYPE_GROUP)
      || (keyType == SL_ZIGBEE_GPD_SECURITY_KEY_TYPE_GROUP_DERIVED)
      || (SL_ZIGBEE_AF_PLUGIN_APPS_BIDIR_CAPABLE == false)
      || SL_ZIGBEE_AF_PLUGIN_APPS_SECURITY_LEVEL == SL_ZIGBEE_GPD_SECURITY_LEVEL_NONE) {
    // No setting of Key Request flag
  } else {
    // else set the key request based on the application configuration
    cmd[index] |= (SL_ZIGBEE_AF_PLUGIN_APPS_KEY_REQUEST ? SL_ZIGBEE_GPD_COM_OPT_KEY_REQ_MASK : 0);     // b5 - GP Security Key Request sub-field
  }
  cmd[index] |= (SL_ZIGBEE_AF_PLUGIN_APPS_FIXED_LOCATION ? SL_ZIGBEE_GPD_COM_OPT_FIXED_LOC_MASK : 0);  // b6 - Fixed location sub-field

  // Extended option sub-field
#if defined(SL_ZIGBEE_AF_PLUGIN_APPS_SECURITY_LEVEL) && (SL_ZIGBEE_AF_PLUGIN_APPS_SECURITY_LEVEL == SL_ZIGBEE_GPD_SECURITY_LEVEL_NONE)
  // If the security level is 0, then there is no extended option as none of its fields are present
  // There is argument of adding this field and make it 0 for such condition.
#elif defined(SL_ZIGBEE_AF_PLUGIN_APPS_SECURITY_LEVEL) && (SL_ZIGBEE_AF_PLUGIN_APPS_SECURITY_LEVEL > SL_ZIGBEE_GPD_SECURITY_LEVEL_NONE)
  // set if: security level capability > none, set the ext option bit
  cmd[index] |= SL_ZIGBEE_GPD_COM_OPT_EXT_OPT_MASK;                                                // b7 - Fixed location sub-field

  index++; // Move to Extended options field

  // Extended Option field-----------------------------------------------------
  // Security capability sub-field
  cmd[index] = (SL_ZIGBEE_GPD_EXT_COM_OPT_SECURITY_LEVEL_CAP_MASK & SL_ZIGBEE_AF_PLUGIN_APPS_SECURITY_LEVEL) // b0-b1 Security Level  sub-field
               | (keyType << SL_ZIGBEE_GPD_EXT_COM_OPT_KEY_TYPE_OFFSET)                                  // b2-b3-b4 Key type  sub-field
               | (keyPresentInComRequest ? SL_ZIGBEE_GPD_EXT_COM_OPT_KEY_PRESENT_MASK : 0)               // b5 Key Present  sub-field
               | (SL_ZIGBEE_AF_PLUGIN_APPS_KEY_ENCRYPT ? SL_ZIGBEE_GPD_EXT_COM_OPT_KEY_ENCRYPT_MASK : 0)     // b6 Key encrypted sub-field
               | (SL_ZIGBEE_GPD_EXT_COM_OPT_OUT_COUNTER_PRESENT_MASK);                                   // b7 OG Frame Counter
  // Extended Option field-----------------------------------------------------

  index++; // Move to next fields

  // add the key
  if (keyPresentInComRequest) {
#if defined(SL_ZIGBEE_AF_PLUGIN_APPS_KEY_ENCRYPT) && (SL_ZIGBEE_AF_PLUGIN_APPS_KEY_ENCRYPT == true)
    uint8_t * protectedKey = &(cmd[index]);
    index += SL_ZIGBEE_AF_PLUGIN_APPS_KEY_LENGTH;
    uint8_t * mic = &(cmd[index]);
    index += SL_ZIGBEE_GPD_MIC_LENGTH;
    sl_zigbee_gpd_security_encrypt_key(&(gpd->addr), key, protectedKey, mic);
#else
    (void) memcpy(&(cmd[index]), gpd->securityKey, SL_ZIGBEE_AF_PLUGIN_APPS_KEY_LENGTH);
#endif
  }
  // Add security counter
  sl_zigbee_gpd_utility_copy_4_bytes(&cmd[index], frameCounter);
  index += sizeof(frameCounter);
#endif

#if defined(SL_ZIGBEE_AF_PLUGIN_APPS_APP_INFO) && (SL_ZIGBEE_AF_PLUGIN_APPS_APP_INFO == true)
  uint8_t *appInfo = &(cmd[index]); // remember the application info location
  *appInfo = 0; //Initialise to 0

  index++; // Move to next fields

  // Add application information

#if SL_ZIGBEE_AF_PLUGIN_APPS_APPS_GPD_MANUFACTURE_ID_PRESENT
  *appInfo |= SL_ZIGBEE_GPD_APP_INFO_COM_MS_ID_MASK;                          // b0 Manuf Id present
  // include MS ID
  cmd[index++] = LO_UINT16(SL_ZIGBEE_AF_PLUGIN_APPS_APPS_GPD_MANUFACTURE_ID);
  cmd[index++] = HI_UINT16(SL_ZIGBEE_AF_PLUGIN_APPS_APPS_GPD_MANUFACTURE_ID);
#endif

#if SL_ZIGBEE_AF_PLUGIN_APPS_APPS_GPD_MODEL_ID_PRESENT
  *appInfo |= SL_ZIGBEE_GPD_APP_INFO_COM_MODEL_ID_MASK;                       // b1 Model Id present
  // include Model ID
  cmd[index++] = LO_UINT16(SL_ZIGBEE_AF_PLUGIN_APPS_APPS_GPD_MODEL_ID);
  cmd[index++] = HI_UINT16(SL_ZIGBEE_AF_PLUGIN_APPS_APPS_GPD_MODEL_ID);
#endif

#if (SL_ZIGBEE_AF_PLUGIN_APPS_APPS_GPD_CMD_LIST_LENGTH > 0)
  *appInfo |= SL_ZIGBEE_GPD_APP_INFO_COM_CMDLIST_MASK;                        // b2 GpdCmd List present
  cmd[index++] = SL_ZIGBEE_AF_PLUGIN_APPS_APPS_GPD_CMD_LIST_LENGTH;
  // GPD CMD List
  uint8_t gpdCmdList[] = SL_ZIGBEE_AF_PLUGIN_APPS_APPS_GPD_CMD_LIST;
  for (int i = 0; i < SL_ZIGBEE_AF_PLUGIN_APPS_APPS_GPD_CMD_LIST_LENGTH; i++) {
    cmd[index++] = gpdCmdList[i];
  }
#endif

#if (SL_ZIGBEE_AF_PLUGIN_APPS_APPS_IN_CLUS_LIST_LENGTH > 0) || (SL_ZIGBEE_AF_PLUGIN_APPS_APPS_OUT_CLUS_LIST_LENGTH > 0)
  *appInfo |= SL_ZIGBEE_GPD_APP_INFO_COM_CLUSTERLIST_MASK;                    // b3 Cluster List present
  // Cluster list length representation
  cmd[index++] = SL_ZIGBEE_AF_PLUGIN_APPS_APPS_IN_CLUS_LIST_LENGTH \
                 + (SL_ZIGBEE_AF_PLUGIN_APPS_APPS_OUT_CLUS_LIST_LENGTH << 4);

#if (SL_ZIGBEE_AF_PLUGIN_APPS_APPS_IN_CLUS_LIST_LENGTH > 0)
  // In Cluster list
  uint16_t inClusterList[] = SL_ZIGBEE_AF_PLUGIN_APPS_APPS_IN_CLUS_LIST;
  for (int i = 0; i < SL_ZIGBEE_AF_PLUGIN_APPS_APPS_IN_CLUS_LIST_LENGTH; i++) {
    cmd[index++] = LO_UINT16(inClusterList[i]);
    cmd[index++] = HI_UINT16(inClusterList[i]);
  }
#endif

#if (SL_ZIGBEE_AF_PLUGIN_APPS_APPS_OUT_CLUS_LIST_LENGTH > 0)
  // Out cluster list
  uint16_t outClusterList[] = SL_ZIGBEE_AF_PLUGIN_APPS_APPS_OUT_CLUS_LIST;
  for (int i = 0; i < SL_ZIGBEE_AF_PLUGIN_APPS_APPS_OUT_CLUS_LIST_LENGTH; i++) {
    cmd[index++] = LO_UINT16(outClusterList[i]);
    cmd[index++] = HI_UINT16(outClusterList[i]);
  }
#endif
#endif //cluster list

#ifdef SL_ZIGBEE_AF_PLUGIN_APPS_GPD_SWITCH_INFO_PRESENT
  *appInfo |= SL_ZIGBEE_GPD_APP_INFO_COM_SWITCH_INFORMATION_PRESENT_MASK;     // b4 Switch Info present
  // include switch information
  cmd[index++] = 2; // as per current spec the switch info has a length of 2
  cmd[index++] = SL_ZIGBEE_AF_PLUGIN_APPS_GPD_SWITCH_INFO;
  cmd[index++] = sl_zigbee_gpd_af_plugin_get_switch_contact_stutus_cb();
#endif

#ifdef SL_ZIGBEE_AF_PLUGIN_APPS_GPD_APP_DESCRIPTION_FOLLOWS
  *appInfo |= SL_ZIGBEE_GPD_APP_INFO_COM_APP_DESCRIPTION_FOLLOWS_MASK;        // b5 App description follows
#endif

#endif // SL_ZIGBEE_AF_PLUGIN_APPS_APP_INFO

  // return length of commissioning command payload
  return index;
}

// Commissioning Request send function.
// A commissioning request is send unsecured at the GPD network layer, hence it resets
// the security level to none, it is hence advised that the calling function must push and pop
// the security level in case of battery powered implementation.
static void sendCommissioningRequest(sl_zigbee_gpd_t_t * gpd)
{
  uint8_t command[SL_ZIGBEE_GPD_CMD_MAX_PAYLOAD_SIZE];
  uint8_t key[] = SL_ZIGBEE_AF_PLUGIN_APPS_KEY;
  // Commissioning Request is with security level 0 until it receives the
  // commissioning reply to set the security level to be used further.
  gpd->securityLevel = SL_ZIGBEE_GPD_SECURITY_LEVEL_NONE;
  uint8_t commandLength = gpdBuildCommissioningCommand(gpd,
                                                       true,
                                                       SL_ZIGBEE_AF_PLUGIN_APPS_SECURITY_KEY_TYPE,
                                                       key,
                                                       gpd->securityFrameCounter,
                                                       command);
  sl_zigbee_af_gpdf_send(SL_ZIGBEE_GPD_NWK_FC_FRAME_TYPE_DATA,
                         gpd,
                         command,
                         commandLength,
                         SL_ZIGBEE_AF_PLUGIN_APPS_CMD_RESEND_NUMBER);
}

// Sends a Success command for bidirectional commissioning following commissioning reply,
// security used based on the negotiated security material at the commissioning reply
static void sendSuccess(sl_zigbee_gpd_t_t * gpd)
{
  uint8_t command[] = { GP_CMD_COMMISSIONING_SUCCESS };
  sl_zigbee_af_gpdf_send(SL_ZIGBEE_GPD_NWK_FC_FRAME_TYPE_DATA,
                         gpd,
                         command,
                         sizeof(command),
                         SL_ZIGBEE_AF_PLUGIN_APPS_CMD_RESEND_NUMBER);
}

// Sends out a Channel Request, with channel toggling behaviour payload as configured
static void sendChannelRequest(sl_zigbee_gpd_t_t * gpd,
                               uint8_t nextChannel,
                               uint8_t secondNextChannel)
{
  uint8_t command[] = { GP_CMD_CHANNEL_RQST, 0x00 };
  // Channel toggling payload at index 1 :
  //                  b7-b4 - Second next channel
  //                  b3-b0 - next channel
  command[1] = (nextChannel - 11) + ((secondNextChannel - 11) << 4);
  sl_zigbee_af_gpdf_send(SL_ZIGBEE_GPD_NWK_FC_FRAME_TYPE_MAINT,
                         gpd,
                         command,
                         sizeof(command),
                         SL_ZIGBEE_AF_PLUGIN_APPS_CMD_RESEND_NUMBER);
}

// --------------------------------------------------------------------------------------------
// --------------------------Application Description ------------------------------------------
// --------------------------------------------------------------------------------------------
// This block of code has the implementation of the application description commands to be
// sent with ensuring in case of a bidirectional commissioning, it encloses the correct setting
// of rxAfterTx flag to collect the commissioning reply.

#if (defined SL_ZIGBEE_AF_PLUGIN_APPS_GPD_APP_DESCRIPTION_FOLLOWS)
// Sub states of the application description
#define SL_ZIGBEE_GPD_COMM_STATE_APP_DESC_STEP_SEND_COMMISSIONING_REQ 0
#define SL_ZIGBEE_GPD_COMM_STATE_APP_DESC_STEP_SEND_APP_DESCRIPTION_REQ 1
#define SL_ZIGBEE_GPD_COMM_STATE_APP_DESC_STEP_SEND_APP_DESCRIPTION_FINAL_REQ 2

// Static variable to hold the substate
static uint8_t applicationDescriptionState = SL_ZIGBEE_GPD_COMM_STATE_APP_DESC_STEP_SEND_COMMISSIONING_REQ;

// Application Description Command, the payload of this command is build by the
// a callback so that the user need to provide the payload appropriately.
static uint8_t gpdBuildCommissioningApplicationDescriptionCommand(uint8_t* cmd,
                                                                  bool * last)
{
  uint8_t index = 0;
  cmd[index++] = GP_CMD_COMMISSIONING_APP_DESCR;
  // Copy the Application description in rest of the payload
  index += sl_zigbee_gpd_af_plugin_get_application_description_cb(&cmd[index],
                                                                  SL_ZIGBEE_GPD_CMD_MAX_PAYLOAD_SIZE,
                                                                  last);
  return index;
}

// Calls the user to provide the application description payload and then sends the
// command out. It also takes the decision to set or clear the rxAfterTx if the current
// application is the last application description frame to be sent out.
static bool sendApplicationDescriptionCommand(sl_zigbee_gpd_t_t * gpd)
{
  uint8_t command[SL_ZIGBEE_GPD_CMD_MAX_PAYLOAD_SIZE];
  bool last = true;
  uint8_t commandLength = gpdBuildCommissioningApplicationDescriptionCommand(command,
                                                                             &last);

#if defined SL_ZIGBEE_AF_PLUGIN_APPS_BIDIR_CAPABLE && (SL_ZIGBEE_AF_PLUGIN_APPS_BIDIR_CAPABLE == true)
  // Based ont the last application descriptor, the rxAfteTx flag is set to receive
  // the commissioning reply incase of a bidirectional commissioning
  gpd->rxAfterTx = (last ? true : false);
#endif

  sl_zigbee_af_gpdf_send(SL_ZIGBEE_GPD_NWK_FC_FRAME_TYPE_DATA,
                         gpd,
                         command,
                         commandLength,
                         SL_ZIGBEE_AF_PLUGIN_APPS_CMD_RESEND_NUMBER);
  return last;
}

// Application Description State machine with 3 main states :
// first : send a commissioning command (E0) with application description follows flag set
//         (and no rxAfterTx in case of bidirectional)
// second : send the series of application description frames until last but one frame is
//          sent out. Example, for 3 report descriptors, lets say 3 application description frames.
// last : send the last frame (for the bidirectional, ensure to set the rxAfterTx) couple of times.
static void gpdCommissioningStateMachineWithAppDescription(sl_zigbee_gpd_t_t * gpd)
{
  // Step 1 : First send Commissioning without rxAfterTx set and indication the
  // application description to follow is already set in the commissioning command
  // formation.
  if (applicationDescriptionState == SL_ZIGBEE_GPD_COMM_STATE_APP_DESC_STEP_SEND_COMMISSIONING_REQ) {
    gpd->rxAfterTx = false;
    for (int i = 0; i < SL_ZIGBEE_AF_PLUGIN_APPS_GPD_APP_NUMBER_OF_COMMISSIONING_REQUEST; i++) {
      sendCommissioningRequest(gpd);
    }
    applicationDescriptionState = SL_ZIGBEE_GPD_COMM_STATE_APP_DESC_STEP_SEND_APP_DESCRIPTION_REQ;
  } else if (applicationDescriptionState == SL_ZIGBEE_GPD_COMM_STATE_APP_DESC_STEP_SEND_APP_DESCRIPTION_REQ) {
    // Step 2 to n : Get from user the Application Description  packet with asking
    // if more to follow or not, repeat based on upto n times where n iteration is
    // no more application description to follow.
    bool last = sendApplicationDescriptionCommand(gpd);
    if (last) {
      // go to step n+1 else re-send it again, asking user if more to follow;
      applicationDescriptionState = SL_ZIGBEE_GPD_COMM_STATE_APP_DESC_STEP_SEND_APP_DESCRIPTION_FINAL_REQ;
    }
  } else if (applicationDescriptionState == SL_ZIGBEE_GPD_COMM_STATE_APP_DESC_STEP_SEND_APP_DESCRIPTION_FINAL_REQ) {
    // Step n+1 : send the last application description command
    // with rxAfterTx true pull the commissioning reply.
    for (int i = 0; i < SL_ZIGBEE_AF_PLUGIN_APPS_GPD_APP_NUMBER_OF_COMMISSIONING_REQUEST; i++) {
      sendApplicationDescriptionCommand(gpd);
      if (gpd->gpdState == SL_ZIGBEE_GPD_APP_STATE_COMMISSIONING_REPLY_RECEIVED) {
        applicationDescriptionState = SL_ZIGBEE_GPD_COMM_STATE_APP_DESC_STEP_SEND_COMMISSIONING_REQ;
        return;
      }
    }
    // at this point no commissioning reply then , it is not commissioned!
    gpd->gpdState = SL_ZIGBEE_GPD_APP_STATE_NOT_COMMISSIONED;
    applicationDescriptionState = SL_ZIGBEE_GPD_COMM_STATE_APP_DESC_STEP_SEND_COMMISSIONING_REQ;
  } else {
    // just reset the  sub state state so it enters
    applicationDescriptionState = SL_ZIGBEE_GPD_COMM_STATE_APP_DESC_STEP_SEND_COMMISSIONING_REQ;
  }
}
#endif
// --------------------------------------------------------------------------------------------
// --------------------------End of Application Description -----------------------------------
// --------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------
// ----------------------------------Channel Request ------------------------------------------
// --------------------------------------------------------------------------------------------
// This block of code is for Channel request by the GPD to find the operational channel of the
// Proxy/Combo in case of bidirectional commissioning.
#if (defined SL_ZIGBEE_AF_PLUGIN_APPS_BIDIR_CAPABLE) && (SL_ZIGBEE_AF_PLUGIN_APPS_BIDIR_CAPABLE == true)

#define SL_ZIGBEE_GPD_COMM_STATE_CHANNEL_REQ_ON_ALL_CHANNELS 0
#define SL_ZIGBEE_GPD_COMM_STATE_CHANNEL_REQ_ON_RX_CHANNEL   1

static uint8_t chReqSubstate = SL_ZIGBEE_GPD_COMM_STATE_CHANNEL_REQ_ON_ALL_CHANNELS;

// Sends channel request maint frame with auto commissioning set on all the enabled channels
// with auto commissioning set and rxAfterTx cleared.
static void gpdCommissioningChannelRequestStateOnAllChannel(sl_zigbee_gpd_t_t * gpd)
{
  // Sub state 1
  // Send channel request on configured channel with auto commissioning set and rxAfterTx cleared
  gpd->rxAfterTx = false;
  gpd->autoCommissioning = true;
  uint8_t channel[] = SL_ZIGBEE_AF_PLUGIN_APPS_APP_CHANNEL_SET;
  for (int j = 0; j < SL_ZIGBEE_AF_PLUGIN_APPS_GPD_APP_NUMBER_OF_CHANNEL_REQUEST_PER_CHANNEL_WITH_AC_SET; j++) {
    for (int i = 0; i < SL_ZIGBEE_AF_PLUGIN_APPS_APP_CHANNEL_SET_LENGTH; i++) {
      gpd->channel = channel[i];
      sendChannelRequest(gpd,
                         SL_ZIGBEE_AP_PLUGIN_APPS_APP_NEXT_RX_CHANNEL,
                         SL_ZIGBEE_AP_PLUGIN_APPS_APP_SECOND_NEXT_RX_CHANNEL);
    }
  }
}

// Sends the channel request on the rx channel with auto-commissioning cleared and rxAfterTx set
static void gpdCommissioningChannelRequestOnRxChannel(sl_zigbee_gpd_t_t * gpd)
{
  // Sub state 2
  // Send channel request on rx channel and next-rx channel with auto commissioning clear.
  gpd->rxAfterTx = true;
  gpd->autoCommissioning = false;
  for (int i = 0; i < SL_ZIGBEE_AF_PLUGIN_APPS_GPD_APP_NUMBER_OF_CHANNEL_REQUEST_PER_CHANNEL; i++) {
    gpd->channel = SL_ZIGBEE_AP_PLUGIN_APPS_APP_NEXT_RX_CHANNEL;
    sendChannelRequest(gpd,
                       SL_ZIGBEE_AP_PLUGIN_APPS_APP_NEXT_RX_CHANNEL,
                       SL_ZIGBEE_AP_PLUGIN_APPS_APP_SECOND_NEXT_RX_CHANNEL);
    if (gpd->gpdState == SL_ZIGBEE_GPD_APP_STATE_CHANNEL_RECEIVED) {
      return;
    }
  }
  gpd->gpdState = SL_ZIGBEE_GPD_APP_STATE_NOT_COMMISSIONED;
}

// Channel Request state machine : mainly two states
// first : Send the channel request with channel toggling behaviour (includes rx channel)
//         on all enabled channels.
// Second : Send the channel request on the rx channel with receive window so that it can
//          receive back the channel configuration.
static void gpdCommissioningChannelRequestStateMachine(sl_zigbee_gpd_t_t * gpd)
{
  gpd->securityLevel = SL_ZIGBEE_GPD_SECURITY_LEVEL_NONE;
  gpd->securityKeyType = SL_ZIGBEE_GPD_SECURITY_KEY_TYPE_NOKEY;
  gpd->skipCca = true;
  if (chReqSubstate == SL_ZIGBEE_GPD_COMM_STATE_CHANNEL_REQ_ON_ALL_CHANNELS) {
    gpdCommissioningChannelRequestStateOnAllChannel(gpd);
    chReqSubstate = SL_ZIGBEE_GPD_COMM_STATE_CHANNEL_REQ_ON_RX_CHANNEL;
  } else if (chReqSubstate == SL_ZIGBEE_GPD_COMM_STATE_CHANNEL_REQ_ON_RX_CHANNEL) {
    gpdCommissioningChannelRequestOnRxChannel(gpd);
    chReqSubstate = SL_ZIGBEE_GPD_COMM_STATE_CHANNEL_REQ_ON_ALL_CHANNELS;
  } else {
    chReqSubstate = SL_ZIGBEE_GPD_COMM_STATE_CHANNEL_REQ_ON_ALL_CHANNELS;
  }
}
#endif
// --------------------------------------------------------------------------------------------
// --------------------------------- End of Channel Request -----------------------------------
// --------------------------------------------------------------------------------------------

#if defined SL_ZIGBEE_AF_PLUGIN_APPS_BIDIR_CAPABLE && (SL_ZIGBEE_AF_PLUGIN_APPS_BIDIR_CAPABLE == true)
static void gpdBidirCommissioningStateMachine(sl_zigbee_gpd_t_t * gpd)
{
#if (defined SL_ZIGBEE_AF_PLUGIN_APPS_GPD_APP_DESCRIPTION_FOLLOWS)
  gpdCommissioningStateMachineWithAppDescription(gpd);
#else
  for (int i = 0; i < SL_ZIGBEE_AF_PLUGIN_APPS_GPD_APP_NUMBER_OF_COMMISSIONING_REQUEST; i++) {
    sendCommissioningRequest(gpd);
    if (gpd->gpdState == SL_ZIGBEE_GPD_APP_STATE_COMMISSIONING_REPLY_RECEIVED) {
      return;
    }
  }
  gpd->gpdState = SL_ZIGBEE_GPD_APP_STATE_CHANNEL_RECEIVED;
#endif
}
#endif

#if (defined SL_ZIGBEE_AF_PLUGIN_APPS_BIDIR_CAPABLE) && (SL_ZIGBEE_AF_PLUGIN_APPS_BIDIR_CAPABLE == false)
static void sendUndirCommissioningRequest(sl_zigbee_gpd_t_t * gpd)
{
  // Save the configured security level and send the commissioning request
  // with security level 0.
  uint8_t securityLevel = gpd->securityLevel;
#if (defined SL_ZIGBEE_AF_PLUGIN_APPS_GPD_APP_DESCRIPTION_FOLLOWS)
  gpdCommissioningStateMachineWithAppDescription(gpd);
#else
  // Send commissioning request
  sendCommissioningRequest(gpd);
#endif
  // Get back to the security level and keys as configured.
  gpd->securityLevel = securityLevel;
  sl_zigbee_gpd_mbedtls_ccm_setkey_wraper(gpd->securityKey);
}
#endif

int8_t sl_zigbee_gpd_process_commissioning_reply(sl_zigbee_gpd_t_t * gpd,
                                                 uint8_t payload_size,
                                                 uint8_t* pRxbuffer)
{
  // Unused
  (void)payload_size;

  // local variable
  uint8_t index = 0;

  uint8_t * newKeyEncrypted;
  uint8_t * newKeyMic;
  uint8_t * rxSecCounter;

  uint8_t options = pRxbuffer[index++];
  bool panIdPresent = (options & SL_ZIGBEE_GPD_COM_REP_OPT_PANID_PRESENT_MASK) ? true : false;
  bool keyPresent   = (options & SL_ZIGBEE_GPD_COM_REP_OPT_KEY_PRESENT_MASK) ? true : false;
  bool keyEncrypted = (options & SL_ZIGBEE_GPD_COM_REP_OPT_KEY_ENCRYPT_MASK) ? true : false;
  uint8_t securityLevel = (options & SL_ZIGBEE_GPD_COM_REP_OPT_SECURITY_LEVEL_MASK) >> SL_ZIGBEE_GPD_COM_REP_OPT_SECURITY_LEVEL_OFFSET;
  uint8_t securityKeyType = (options & SL_ZIGBEE_GPD_COM_REP_OPT_KEY_TYPE_MASK) >> SL_ZIGBEE_GPD_COM_REP_OPT_KEY_TYPE_OFFSET;

  if (panIdPresent) {
    // Read 'panId' but don't use it
    (void)(pRxbuffer[index] + (pRxbuffer[index + 1] << 8));
    index += sizeof(uint16_t);
  }
  // TODO: we don't reconfigure PAN ID in our system
  if (keyPresent) {
    // Check if key delivered encrypted
    if (keyEncrypted) {
#if defined(SL_ZIGBEE_AF_PLUGIN_APPS_KEY_ENCRYPT) && (SL_ZIGBEE_AF_PLUGIN_APPS_KEY_ENCRYPT == true)
      // get encrypted key
      newKeyEncrypted = &pRxbuffer[index];
      index += SL_ZIGBEE_AF_PLUGIN_APPS_KEY_LENGTH;
      // get MIC
      newKeyMic = &pRxbuffer[index];
      index += SL_ZIGBEE_GPD_MIC_LENGTH;
      // get security frame counter
      rxSecCounter = &pRxbuffer[index];
      index += sizeof(uint32_t);

      // Unsecured the key
      if (SUCCESS != sl_zigbee_gpd_security_decrypt_key(&(gpd->addr),
                                                        newKeyEncrypted,
                                                        newKeyMic,
                                                        rxSecCounter,
                                                        gpd->securityKey)) {
        return FAILED;
      }
#endif
    } else {
      // copy the new key
      for (int i = 0; i < SL_ZIGBEE_AF_PLUGIN_APPS_KEY_LENGTH; i++) {
        gpd->securityKey[i] = pRxbuffer[index++];
      }
    }
  } else {   // key not present
    // TODO: check rule for not key delivering in com and op
  }
  gpd->securityKeyType = securityKeyType;
  gpd->securityLevel = securityLevel;
  sl_zigbee_gpd_mbedtls_ccm_setkey_wraper(gpd->securityKey);
  return SUCCESS;
}

void sl_zigbee_gpd_af_plugin_commissioning_channel_config_cb(uint8_t channel)
{
  sl_zigbee_gpd_set_channel(channel);
  sl_zigbee_gpd_set_state(SL_ZIGBEE_GPD_APP_STATE_CHANNEL_RECEIVED);
}

void sl_zigbee_gpd_af_plugin_commissioning_reply_cb(uint8_t length,
                                                    uint8_t * commReplyPayload)
{
  if (SUCCESS == sl_zigbee_gpd_process_commissioning_reply(sl_zigbee_gpd_get_gpd(),
                                                           length,
                                                           commReplyPayload)) {
    sl_zigbee_gpd_set_state(SL_ZIGBEE_GPD_APP_STATE_COMMISSIONING_REPLY_RECEIVED);
  }
}

void sl_zigbee_gpd_af_plugin_de_commission(sl_zigbee_gpd_t_t * gpd)
{
  uint8_t command[] = { GP_CMD_DECOMMISSIONING };
  gpd->rxAfterTx = false;
  sl_zigbee_af_gpdf_send(SL_ZIGBEE_GPD_NWK_FC_FRAME_TYPE_DATA,
                         gpd,
                         command,
                         sizeof(command),
                         SL_ZIGBEE_AF_PLUGIN_APPS_CMD_RESEND_NUMBER);
  sl_zigbee_gpd_load_gpd_default_configuration(gpd);
}

void sl_zigbee_gpd_af_plugin_commission(sl_zigbee_gpd_t_t * gpd)
{
#if (defined SL_ZIGBEE_AF_PLUGIN_APPS_BIDIR_CAPABLE) && (SL_ZIGBEE_AF_PLUGIN_APPS_BIDIR_CAPABLE == true)
  if (gpd->gpdState == SL_ZIGBEE_GPD_APP_STATE_NOT_COMMISSIONED) {
    gpdCommissioningChannelRequestStateMachine(gpd);
  } else if (gpd->gpdState == SL_ZIGBEE_GPD_APP_STATE_CHANNEL_RECEIVED) {
    gpdBidirCommissioningStateMachine(gpd);
  } else if (gpd->gpdState == SL_ZIGBEE_GPD_APP_STATE_COMMISSIONING_REPLY_RECEIVED) {
    sendSuccess(gpd);
    sl_zigbee_gpd_set_state(SL_ZIGBEE_GPD_APP_STATE_COMMISSIONING_SUCCESS_REQUEST);
  } else if (gpd->gpdState == SL_ZIGBEE_GPD_APP_STATE_COMMISSIONING_SUCCESS_REQUEST
             || gpd->gpdState == SL_ZIGBEE_GPD_APP_STATE_OPERATIONAL) {
    sendSuccess(gpd);
    sl_zigbee_gpd_set_state(SL_ZIGBEE_GPD_APP_STATE_OPERATIONAL);
  } else {
    gpd->gpdState = SL_ZIGBEE_GPD_APP_STATE_NOT_COMMISSIONED;
  }
#else
  sendUndirCommissioningRequest(gpd);
#endif
}
