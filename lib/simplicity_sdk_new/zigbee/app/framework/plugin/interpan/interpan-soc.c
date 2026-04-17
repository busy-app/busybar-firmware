/***************************************************************************//**
 * @file
 * @brief SOC-specific code related to the reception and processing of interpan
 * messages.
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
#include "interpan.h"
#include "app/framework/util/af-main.h"

static const sl_zigbee_mac_filter_match_data_t filters[] = {
  SL_ZIGBEE_AF_PLUGIN_INTERPAN_FILTER_LIST
    (SL_802154_FILTER_MATCH_END), // terminator
};

//------------------------------------------------------------------------------

void sli_zigbee_af_interpan_mac_filter_match_message_callback(sl_zigbee_mac_filter_match_data_t filterValueMatch,
                                                              sl_zigbee_mac_passthrough_type_t legacyPassthroughType,
                                                              sl_zigbee_rx_packet_info_t *packetInfo,
                                                              uint8_t messageLength,
                                                              uint8_t *messageContents)
{
  (void)filterValueMatch;
  (void)legacyPassthroughType;
  (void)packetInfo;
  sli_zigbee_af_interpan_process_message(legacyPassthroughType,
                                         messageLength,
                                         messageContents);
}

sl_status_t sli_zigbee_af_interpan_send_raw_message(uint8_t length, uint8_t* message)
{
  sl_status_t status;

  status = sl_zigbee_send_raw_message(message, length, SL_802154_TRANSMIT_PRIORITY_NORMAL, true);
  return status;
}

void sl_zigbee_af_interpan_init_cb(uint8_t init_level)
{
  interpanPluginInit(init_level);
  interpanPluginSetMacMatchFilterEnable(true);
}

void interpanPluginSetMacMatchFilterEnable(bool enable)
{
  const sl_zigbee_mac_filter_match_data_t* matchData = enable ? filters : NULL;

  sl_status_t status = sl_zigbee_set_mac_filter_match_list(matchData, sizeof(filters) / sizeof(sl_zigbee_mac_filter_match_data_t));
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_app_println("ERR: failed %s inter-PAN MAC filter (0x%02X)",
                             enable ? "enabling" : "disabling",
                             status);
  }
}

// NOTE:  It is expected that when encrypting, the message buffer
// pointed to by *apsFrame is big enough to hold additional
// space for the Auxiliary security header and the MIC.
#define APS_CRYPT_EXTRA_BYTES_NEEDED 9

sl_status_t sli_zigbee_af_interpan_aps_crypt_message(bool encrypt,
                                                     uint8_t* apsFrame,
                                                     uint8_t* messageLength,
                                                     uint8_t apsHeaderLength,
                                                     sl_802154_long_addr_t remoteEui64)
{
  sl_status_t status = SL_STATUS_NOT_AVAILABLE;

#if defined(ALLOW_APS_ENCRYPTED_MESSAGES)

  uint8_t apsFrameSize;
  if (encrypt) {
    apsFrameSize = *messageLength + APS_CRYPT_EXTRA_BYTES_NEEDED;
  } else {
    apsFrameSize = *messageLength;
  }

  status = sl_zigbee_aps_crypt_message(encrypt,
                                       apsFrameSize,
                                       apsFrame,
                                       apsHeaderLength,
                                       remoteEui64);
  if (status == SL_STATUS_OK) {
    // It is expected that when encrypting, the message is big enough to hold
    // the additional data (AUX header and MIC)
    // Decrypting will shrink the message, removing the AUX header and MIC.
    if (encrypt) {
      *messageLength += APS_CRYPT_EXTRA_BYTES_NEEDED;
    } else {
      *messageLength -= APS_CRYPT_EXTRA_BYTES_NEEDED;
    }
  }
#else // ALLOW_APS_ENCRYPTED_MESSAGES
  (void)encrypt;
  (void)apsFrame;
  (void)messageLength;
  (void)apsHeaderLength;
  (void)remoteEui64;
#endif // ALLOW_APS_ENCRYPTED_MESSAGES

  return status;
}
