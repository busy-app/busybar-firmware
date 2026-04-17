/***************************************************************************//**
 * @file
 * @brief Host-specific code related to the reception and processing of interpan
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
#include "app/util/ezsp/ezsp-frame-utilities.h"

//------------------------------------------------------------------------------

void sli_zigbee_af_interpan_mac_filter_match_message_callback(sl_zigbee_mac_filter_match_data_t filterValueMatch,
                                                              sl_zigbee_mac_passthrough_type_t legacyPassthroughType,
                                                              sl_zigbee_rx_packet_info_t* packetInfo,
                                                              uint8_t messageLength,
                                                              uint8_t *messageContents)
{
  (void)filterValueMatch;
  (void)packetInfo;
  sli_zigbee_af_interpan_process_message(legacyPassthroughType,
                                         messageLength,
                                         messageContents);
}

sl_status_t sli_zigbee_af_interpan_send_raw_message(uint8_t length, uint8_t* message)
{
  return sl_zigbee_ezsp_send_raw_message(length, message, SL_802154_TRANSMIT_PRIORITY_NORMAL, true);
}

void sl_zigbee_af_interpan_ncp_init_cb(void)
{
  interpanPluginInit(SL_ZIGBEE_INIT_LEVEL_EVENT);
  interpanPluginInit(SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA);

  sl_zigbee_mac_filter_match_data_t filters[] = {
    SL_ZIGBEE_AF_PLUGIN_INTERPAN_FILTER_LIST
  };
  sl_status_t status;
  status = sl_zigbee_ezsp_set_configuration_value(SL_ZIGBEE_EZSP_CONFIG_MAC_FILTER_TABLE_SIZE,
                                                  (sizeof(filters)
                                                   / sizeof(sl_zigbee_mac_filter_match_data_t)) + 1);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_app_println("%s%s failed 0x%08X",
                             "Error: ",
                             "Sizing MAC filter table",
                             status);
    return;
  }
  interpanPluginSetMacMatchFilterEnable(true);
}

void interpanPluginSetMacMatchFilterEnable(bool enable)
{
  sl_zigbee_mac_filter_match_data_t filters[] = {
    SL_ZIGBEE_AF_PLUGIN_INTERPAN_FILTER_LIST
  };

  uint8_t value[2 * sizeof(filters) / sizeof(sl_zigbee_mac_filter_match_data_t)] = { 0 };

  if (enable) {
    uint8_t i;
    for (i = 0; i < sizeof(value) / 2; i++) {
      value[i * 2]     =  LOW_BYTE(filters[i]);
      value[i * 2 + 1] = HIGH_BYTE(filters[i]);
    }
  }

  sl_status_t status = sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_MAC_FILTER_LIST,
                                                sizeof(value), value);
  if (status != SL_ZIGBEE_EZSP_SUCCESS) {
    sl_zigbee_af_app_println("ERR: failed %s inter-PAN MAC filter (0x%02X)",
                             enable ? "enabling" : "disabling",
                             status);
  }
}

sl_status_t sli_zigbee_af_interpan_aps_crypt_message(bool encrypt,
                                                     uint8_t* message,
                                                     uint8_t* messageLength,
                                                     uint8_t apsHeaderEndIndex,
                                                     sl_802154_long_addr_t remoteEui64)
{
  (void)encrypt;
  (void)message;
  (void)messageLength;
  (void)apsHeaderEndIndex;
  (void)remoteEui64;
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
    *messageLength -= APS_CRYPT_EXTRA_BYTES_NEEDED;
  }

#endif // ALLOW_APS_ENCRYPTED_MESSAGES

  return status;
}
