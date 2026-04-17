/***************************************************************************//**
 * @file
 * @brief GP security code
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
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#else // !SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "green-power-adapter.h"
#endif //SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

#include "stack/include/zigbee-security-manager.h"

extern void sli_util_load_key_into_core(const uint8_t* key);
extern void sli_zigbee_get_key_from_core(const uint8_t* key);

#define SL_ZIGBEE_GP_SECURITY_MIC_LENGTH 4
#define SECURITY_BLOCK_SIZE 16

// A nonce contains the following fields:
#define STANDALONE_NONCE_SOURCE_ADDR_INDEX       0
#define STANDALONE_NONCE_FRAME_COUNTER_INDEX     8
#define STANDALONE_NONCE_SECURITY_CONTROL_INDEX 12

#ifndef MAX_PAYLOAD_LENGTH
#define MAX_PAYLOAD_LENGTH   70
#endif

// NWK FC := [Ext NWK Header = 0b1 || Auto-Commissioning =0b0|| ZigBee Protocol 0b0011 || Frame type =0b00] = 0x8c
#define HEADER_FLAG_NWK_FC 0x8C

//NWK FC Extended = [Direction = 0b0 || RxAfterTx = 0b0 || SecurityKey = 0b0 ||SecurityLevel(2 bits) || ApplID(3 bits)]
#define MAKE_HEADER_FLAG_NWK_FC_EXT(direction, rxAfterTx, keyType, securityLevel, appId) ((direction << 7) + (rxAfterTx << 6) + (keyType << 5) + (securityLevel << 3) + appId)

#define GPD_APPLICATION_ID_SRC_ID 0
#define GPD_APPLICATION_ID_IEEE   2
#define NONCE_SECURITY_CONTROL 0x05
#define NONCE_SECURITY_CONTROL_APP_ID2_OUTGOING 0xC5

// The direction
//   gpd->gpp : Incoming
//   gpp->gpd : Outgoing
typedef enum {
  DIRECTION_GPD_TO_GPP = 0x00,
  DIRECTION_GPP_TO_GPD = 0x01
} EmberGpdfDirection;

sl_zigbee_af_status_t sl_zigbee_af_green_power_server_derive_shared_key_from_sink_attribute(uint8_t * gpsSecurityKeyTypeAtrribute,
                                                                                            sl_zigbee_key_data_t * gpSharedKeyAttribute,
                                                                                            sl_zigbee_gp_address_t * gpdAddr);

static void initializeNonce(bool directionIncoming,
                            uint8_t * nonce,
                            sl_zigbee_gp_address_t * gpdAddr,
                            uint32_t gpdSecurityFrameCounter)
{
  // ScrId
  if (gpdAddr->applicationId == GPD_APPLICATION_ID_IEEE) {
    memmove(nonce,
            gpdAddr->id.gpdIeeeAddress,
            EUI64_SIZE);
  } else {
    if (directionIncoming) { // from GPD --> GPP/GPS : direction bit = 0 in NW FC Header
      sl_util_store_low_high_int32u(&nonce[STANDALONE_NONCE_SOURCE_ADDR_INDEX], gpdAddr->id.sourceId);
      sl_util_store_low_high_int32u(&nonce[STANDALONE_NONCE_SOURCE_ADDR_INDEX + 4], gpdAddr->id.sourceId);
    } else { // from GPP/GPS --> GPD : direction bit = 1 in NW FC Header
      sl_util_store_low_high_int32u(&nonce[STANDALONE_NONCE_SOURCE_ADDR_INDEX], 0);
      sl_util_store_low_high_int32u(&nonce[STANDALONE_NONCE_SOURCE_ADDR_INDEX + 4], gpdAddr->id.sourceId);
    }
  }
  // Frame counter.
  sl_util_store_low_high_int32u(&nonce[STANDALONE_NONCE_FRAME_COUNTER_INDEX], gpdSecurityFrameCounter);
  //Security control
  if (gpdAddr->applicationId == GPD_APPLICATION_ID_SRC_ID
      || directionIncoming) {
    nonce[STANDALONE_NONCE_SECURITY_CONTROL_INDEX] = NONCE_SECURITY_CONTROL;
  } else if (!directionIncoming
             && gpdAddr->applicationId == GPD_APPLICATION_ID_IEEE) {
    nonce[STANDALONE_NONCE_SECURITY_CONTROL_INDEX] = NONCE_SECURITY_CONTROL_APP_ID2_OUTGOING;
  } else {
    // MISRA : terminating else , assign 0 to the member.
    nonce[STANDALONE_NONCE_SECURITY_CONTROL_INDEX] = 0;
  }
}

static uint8_t prepareHeader(EmberGpdfDirection direction,
                             bool rxAfterTx,
                             uint8_t * header,
                             sl_zigbee_gp_address_t * gpdAddr,
                             uint8_t keyType,
                             uint8_t securityLevel,
                             uint32_t gpdSecurityFrameCounter)
{
  uint8_t headerLength = 0;
  header[headerLength++] = HEADER_FLAG_NWK_FC;
  header[headerLength++] = MAKE_HEADER_FLAG_NWK_FC_EXT(direction, rxAfterTx, keyType, securityLevel, gpdAddr->applicationId);
  if (gpdAddr->applicationId == GPD_APPLICATION_ID_IEEE) {
    header[headerLength++] = gpdAddr->endpoint;
  } else {
    sl_util_store_low_high_int32u(&header[headerLength], gpdAddr->id.sourceId);
    headerLength += 4;
  }
  if (securityLevel >= SL_ZIGBEE_GP_SECURITY_LEVEL_FC_MIC) {
    sl_util_store_low_high_int32u(&header[headerLength], gpdSecurityFrameCounter);
    headerLength += 4;
  }
  return headerLength;
}

static uint8_t appendPayload(uint8_t * dst,
                             uint8_t gpdCommandId,
                             uint8_t * gpdCommandPayload)
{
  if (NULL == dst) {
    return 0;
  }
  uint8_t length = 0;
  dst[length++] = gpdCommandId;
  if (NULL == gpdCommandPayload) {
    return length;
  }
  if (gpdCommandPayload[0] == 0xFF) {
    dst[length++] = 0xFF;
  } else if (gpdCommandPayload[0] < MAX_PAYLOAD_LENGTH) {
    for (int i = 0; i < gpdCommandPayload[0]; i++) {
      dst[length++] = gpdCommandPayload[1 + i];
    }
  } else {
    return 0;
  }
  return length;
}
// Secure outgoing GPDF frame creation
// Inputs :
//   gpdAddr                      : Gpd address
//   gpdSecurityFrameCounter      : Security framecounter
//   keyType                      : Key Type [Shared Key = 0, Individual Key = 1]
//   securityLevel                : The security Level must be one valid level as 0, 2 or 3.
//   gpdCommandId                 : GPD Command Id
//   gpdCommandPayload            : Payload Format to specefy payload = [payloadlength, <payload bytes>]
//   securedOutgoingGpdf          : Memory pointer to collect the secured GPDF
//   securedOutgoingGpdfMaxLength : Allocated memory size to colllect the result.
// Outputs :
//   securedOutgoingGpdf          : Memory pointer to collect the secured GPDF
// Returns :
//   totalLength                  : Length of the secured GPDF
uint8_t sli_zigbee_af_gp_outgoing_command_encrypt(sl_zigbee_gp_address_t * gpdAddr,
                                                  uint32_t gpdSecurityFrameCounter,
                                                  uint8_t keyType,
                                                  uint8_t securityLevel,
                                                  uint8_t gpdCommandId,
                                                  uint8_t * gpdCommandPayload,
                                                  uint8_t * securedOutgoingGpdf,
                                                  uint8_t securedOutgoingGpdfMaxLength)
{
  if (securedOutgoingGpdf == NULL) {
    return 0;
  }
  // GP Spec Ref : A.1.4.1 Generic GPDF Frame Format
  // The secure frame storage need, the supplied storage must be greater than the folowing min length
  // NWK FC | EXT NWK FC | EP/SRC ID | FC | GPD Command Id | Payload    | MIC |
  //   1    |    1       |  1/4      | 0/4|       1        | 0-Variable | 0/4 |
  uint8_t minframeLength = 1
                           + 1
                           + ((gpdAddr->applicationId == GPD_APPLICATION_ID_IEEE) ? 1 : 4)
                           + ((securityLevel == SL_ZIGBEE_GP_SECURITY_LEVEL_NONE) ? 0 : 4)
                           + 1
                           + ((gpdCommandPayload == NULL) ? 0 : ((gpdCommandPayload[0] == 0xFF) ? 1 : gpdCommandPayload[0]))
                           + ((securityLevel == SL_ZIGBEE_GP_SECURITY_LEVEL_NONE) ? 0 : 4);
  if (securedOutgoingGpdfMaxLength < minframeLength) {
    return 0;
  }
  uint8_t headerLength = prepareHeader(DIRECTION_GPP_TO_GPD,
                                       false,
                                       securedOutgoingGpdf,
                                       gpdAddr,
                                       keyType,
                                       securityLevel,
                                       gpdSecurityFrameCounter);
  uint8_t payloadLength = appendPayload(&securedOutgoingGpdf[headerLength],
                                        gpdCommandId,
                                        gpdCommandPayload);
  uint8_t totalLength = headerLength + payloadLength;
  if (securityLevel >= SL_ZIGBEE_GP_SECURITY_LEVEL_FC_MIC) {
    // The following code for the security level 2 or 3
    // Prepare nonce
    uint8_t nonce[SECURITY_BLOCK_SIZE] = { 0 };
    uint8_t temp[MAX_PAYLOAD_LENGTH + SL_ZIGBEE_GP_SECURITY_MIC_LENGTH];
    initializeNonce(false,
                    nonce,
                    gpdAddr,
                    gpdSecurityFrameCounter);
    uint8_t authenticationStartIndex = 0;
    uint8_t encryptionStartIndex;
    uint8_t authenticationLength;
    if (securityLevel == SL_ZIGBEE_GP_SECURITY_LEVEL_FC_MIC) {
      encryptionStartIndex = totalLength;
      authenticationLength = totalLength;
    } else {
      encryptionStartIndex = headerLength;
      authenticationLength = headerLength;
    }
    // Calculate the MIC of unencrypted payload.
    // A key needs to be loaded into core first.
    sl_zigbee_sec_man_aes_ccm(nonce,
                              true,
                              securedOutgoingGpdf + authenticationStartIndex,
                              authenticationLength,
                              totalLength,
                              temp);
    memmove(&securedOutgoingGpdf[totalLength], &temp[totalLength], SL_ZIGBEE_GP_SECURITY_MIC_LENGTH);
    totalLength += 4;
    if (securityLevel == SL_ZIGBEE_GP_SECURITY_LEVEL_FC_MIC_ENCRYPTED) {
      // Include encrypted payload in GPDF
      memmove(&securedOutgoingGpdf[encryptionStartIndex], &temp[encryptionStartIndex], payloadLength);
    }
  }
  return totalLength;
}

bool sli_zigbee_af_gp_calculate_incoming_command_decrypt(sl_zigbee_gp_address_t * gpdAddr,
                                                         uint32_t gpdSecurityFrameCounter,
                                                         uint8_t payloadLength,
                                                         uint8_t * payload)
{
  uint8_t nonce[SECURITY_BLOCK_SIZE] = { 0 };
  initializeNonce(true,
                  nonce,
                  gpdAddr,
                  gpdSecurityFrameCounter);
  uint8_t temp[MAX_PAYLOAD_LENGTH + SL_ZIGBEE_GP_SECURITY_MIC_LENGTH];
  sl_zigbee_sec_man_aes_ccm(nonce, false, payload, 0, payloadLength, temp);
  memmove(payload, &temp, payloadLength);
  return true;
}

bool sli_zigbee_af_gp_calculate_incoming_command_mic(sl_zigbee_gp_address_t * gpdAddr,
                                                     bool rxAfterTx,
                                                     uint8_t keyType,
                                                     uint8_t securityLevel,
                                                     uint32_t gpdSecurityFrameCounter,
                                                     uint8_t gpdCommandId,
                                                     uint8_t * gpdCommandPayload,
                                                     bool encryptedPayload,
                                                     uint8_t mic[4])
{
  uint8_t nonce[SECURITY_BLOCK_SIZE] = { 0 };
  uint8_t temp[MAX_PAYLOAD_LENGTH + SL_ZIGBEE_GP_SECURITY_MIC_LENGTH];
  initializeNonce(true,
                  nonce,
                  gpdAddr,
                  gpdSecurityFrameCounter);
  uint8_t payload[MAX_PAYLOAD_LENGTH] = { 0 };
  uint8_t headerLength = prepareHeader(DIRECTION_GPD_TO_GPP,
                                       rxAfterTx,
                                       payload,
                                       gpdAddr,
                                       keyType,
                                       securityLevel,
                                       gpdSecurityFrameCounter);

  uint8_t payloadLength = appendPayload(&payload[headerLength],
                                        gpdCommandId,
                                        gpdCommandPayload);
  uint8_t totalLength = headerLength + payloadLength;
  uint8_t authenticationStartIndex = 0;
  uint8_t encryptionStartIndex;
  uint8_t authenticationLength;
  uint8_t encryptionLength;
  if (securityLevel == SL_ZIGBEE_GP_SECURITY_LEVEL_FC_MIC) {
    encryptionStartIndex = totalLength;
    authenticationLength = totalLength;
    encryptionLength = 0;
  } else {
    encryptionStartIndex = headerLength;
    authenticationLength = headerLength;
    encryptionLength = payloadLength;
  }
  sl_zigbee_af_green_power_cluster_println("Calculating MIC (%s) : ", __FUNCTION__);

  sl_zigbee_sec_man_key_t key;
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_init_context(&context);
  //check this; using internal as a safe option for replacing
  //crypto keys right now (due to uncertainty whether these
  //keys live somewhere else in storage)
  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_INTERNAL;
  sl_zigbee_sec_man_export_key(&context, &key);

  sl_zigbee_af_green_power_cluster_print("Using KeyType = %d fc = %08X Key :[", keyType, gpdSecurityFrameCounter);
  for (int i = 0; i < 16; i++) {
    sl_zigbee_af_green_power_cluster_print("%02X ", key.key[i]);
  }
  sl_zigbee_af_green_power_cluster_print("]\n");
  sl_zigbee_af_green_power_cluster_print("Prepared Nonce :[");
  for (int i = 0; i < NONCE_LENGTH; i++) {
    sl_zigbee_af_green_power_cluster_print("%02X ", nonce[i]);
  }
  sl_zigbee_af_green_power_cluster_print("]\n");
  sl_zigbee_af_green_power_cluster_print("Prepared Payload :[");
  for (int i = 0; i < totalLength; i++) {
    sl_zigbee_af_green_power_cluster_print("%02X ", payload[i]);
  }
  sl_zigbee_af_green_power_cluster_print("]\n");
  sl_zigbee_af_green_power_cluster_println("encryptionStartIndex :%02X,totalLength :%02X payloadLength:%02X",
                                           encryptionStartIndex,
                                           totalLength,
                                           payloadLength);
  if (securityLevel == SL_ZIGBEE_GP_SECURITY_LEVEL_FC_MIC_ENCRYPTED && encryptedPayload) {
    // Decrypt and then calculate the MIC
    sl_zigbee_sec_man_aes_ccm(nonce, true, &payload[encryptionStartIndex], 0, payloadLength, temp);
    memmove(&payload[encryptionStartIndex], &temp, payloadLength);
    //sl_zigbee_af_green_power_cluster_print("Decrypted (same as one more encryption) Payload :[");
    //for (int i = 0; i < payloadLength; i++) {
    // sl_zigbee_af_green_power_cluster_print("%02X ",payload[encryptionStartIndex + i]);
    //}
    //sl_zigbee_af_green_power_cluster_print("]\n");
  }
  sl_zigbee_sec_man_aes_ccm(nonce,
                            true,
                            payload + authenticationStartIndex,
                            authenticationLength,
                            authenticationLength + encryptionLength,
                            temp);
  memmove(mic, &temp[totalLength], 4);
  return true;
}

static void initialiseKeyDerivationNonce(uint8_t * nonce,
                                         sl_zigbee_gp_address_t * gpdAddr,
                                         uint32_t gpdSecurityFrameCounter,
                                         bool directionIncoming)
{
  memset(nonce, 0, SECURITY_BLOCK_SIZE);
  // SrcId : [1] - [8]
  if (gpdAddr->applicationId == GPD_APPLICATION_ID_IEEE) {
    memmove(nonce,
            gpdAddr->id.gpdIeeeAddress,
            EUI64_SIZE);
  } else {
    if (directionIncoming) {
      sl_util_store_low_high_int32u(&(nonce[STANDALONE_NONCE_SOURCE_ADDR_INDEX]), gpdAddr->id.sourceId);
    } else {
      sl_util_store_low_high_int32u(&(nonce[STANDALONE_NONCE_SOURCE_ADDR_INDEX]), 0);
    }
    sl_util_store_low_high_int32u(&(nonce[STANDALONE_NONCE_SOURCE_ADDR_INDEX + 4]), gpdAddr->id.sourceId);
  }
  // Frame Counter : [9] - [12]
  if (directionIncoming) { // Decrypt
    if (gpdAddr->applicationId == GPD_APPLICATION_ID_IEEE) {
      memmove(nonce + STANDALONE_NONCE_FRAME_COUNTER_INDEX, gpdAddr->id.gpdIeeeAddress, 4);
    } else {
      sl_util_store_low_high_int32u(&nonce[STANDALONE_NONCE_FRAME_COUNTER_INDEX], gpdAddr->id.sourceId);
    }
  } else { // encrypt
    sl_util_store_low_high_int32u(&nonce[STANDALONE_NONCE_FRAME_COUNTER_INDEX], gpdSecurityFrameCounter);
  }
  // nonce[13] :
  // Security Control : 0x05 : all incoming and outgoing appId 0
  //                  : 0xC5 : outgoing appId 2
  if (gpdAddr->applicationId == GPD_APPLICATION_ID_IEEE
      && !directionIncoming) {
    nonce[STANDALONE_NONCE_SECURITY_CONTROL_INDEX] = NONCE_SECURITY_CONTROL_APP_ID2_OUTGOING;
  } else {
    nonce[STANDALONE_NONCE_SECURITY_CONTROL_INDEX] = NONCE_SECURITY_CONTROL;
  }
}
// Key Derivation for OOB Commissioing GPDF - incomming(decryption) and outgoing(encryption)
bool sli_zigbee_af_gp_key_tc_lk_derivation(sl_zigbee_gp_address_t * gpdAddr,
                                           uint32_t gpdSecurityFrameCounter,
                                           uint8_t mic[4],
                                           sl_zigbee_key_data_t * key,
                                           bool directionIncoming)
{
  uint8_t nonce[SECURITY_BLOCK_SIZE] = { 0 };
  initialiseKeyDerivationNonce(nonce,
                               gpdAddr,
                               gpdSecurityFrameCounter,
                               directionIncoming);
  // The size is always 20 because the header is of 4 bytes and the Key is 16 bytes
  uint8_t payload[20] = { 0 };
  // Temporary buffer to store output for the new CCM* API
  uint8_t temp[24];
  if (gpdAddr->applicationId == GPD_APPLICATION_ID_IEEE) {
    memmove(payload, gpdAddr->id.gpdIeeeAddress, 4);
  } else {
    sl_util_store_low_high_int32u(payload, gpdAddr->id.sourceId);
  }
  if (directionIncoming) {
    // Decrypt the incoming Key first then calculate the MIC
    sl_zigbee_sec_man_aes_ccm(nonce, true, key->contents, 0, SL_ZIGBEE_ENCRYPTION_KEY_SIZE, temp);
    memcpy(payload + 4, temp, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
    memcpy(key->contents, temp, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
  } else {
    // Take the Key in payload for MIC calculation first then encrypt the keys
    memcpy(payload + 4, key->contents, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
    sl_zigbee_sec_man_aes_ccm(nonce, true, key->contents, 0, SL_ZIGBEE_ENCRYPTION_KEY_SIZE, temp);
    memcpy(key->contents, temp, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
  }
  sl_zigbee_sec_man_aes_ccm(nonce, true, payload, 4, 20, temp);
  memcpy(mic, &temp[20], 4);
  return true;
}
