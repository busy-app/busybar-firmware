/***************************************************************************//**
 * @file
 * @brief
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

#ifndef SILABS_CRYPTO_H
#define SILABS_CRYPTO_H

#define FRAME_COUNTER_UPDATE_INTERVAL_LOG 12
#define FRAME_COUNTER_UPDATE_INTERVAL  (1 << FRAME_COUNTER_UPDATE_INTERVAL_LOG)
#define FRAME_COUNTER_UPDATE_MASK      (FRAME_COUNTER_UPDATE_INTERVAL - 1)

// APS encryption
bool sli_zigbee_pro_aps_encrypt_packet(sli_zigbee_packet_header_t *header,
                                       uint8_t authenticationStartIndex,
                                       uint8_t auxFrameIndex);

// Functions for both APS and NWK decrypt
bool sli_zigbee_pro_decrypt_packet(sli_zigbee_packet_header_t header,
                                   uint8_t authenticationStartIndex,
                                   uint8_t auxFrameIndex,
                                   sl_802154_long_addr_t sourceEui64,
                                   bool nwkDecrypt);

// NWK encrypt only
void sli_zigbee_pro_network_encrypt_flat_packet(uint8_t* packet,
                                                uint8_t packetLength,
                                                uint8_t authenticationStartIndex,
                                                uint8_t auxFrameIndex);

// Following is only used in the gp test device - review and remove if possible.
void sli_zigbee_calculate_and_encrypt_header_mic(uint8_t *header,
                                                 uint8_t header_length,
                                                 uint8_t authenticationStartIndex,
                                                 uint8_t encryptionStartIndex,
                                                 uint8_t *nonce,
                                                 uint8_t *micResult);

// Used by gp-data to decrypt and/or authenticate a message-buffer payload
// Whne the message buffer needs decryption (for example using security level
// 3 in GPDF), supplied by the arguemnt decrypt and appropriate indexes, it
// performs decryption and authentication. Upon successful authentication of
// the message, the buffer content is replaced with the decrypted message.
// upon failure to authenticate, the buffer contents remains unchanged.
bool sli_zigbee_gp_decrypt_and_authenticate(bool decrypt,
                                            sli_buffer_manager_buffer_t header,
                                            uint8_t authenticationStartIndex,
                                            uint8_t encryptionStartIndex,
                                            uint8_t *nonce,
                                            uint8_t *rxMic);

// Routines for extracting timing data for the CCM* crypto operations.
// The code currently only measures NWK encryption.  Since we are only
// measuring CCM* performance: MIC create/check, Encrypt/Decrypt,
// and not the overall packet manipulation we don't bother to measure
// APS encryption.  It is the same as NWK but the max packet size we can
// operate over is smaller.

// We store separate results for sending and receiving
#define NUMBER_RESULTS 10

typedef struct {
  uint32_t micTimingMicroSeconds;
  uint8_t micPacketSize;
  uint32_t encryptDecryptTimingMicroSeconds;
  uint8_t encryptDecryptPacketSize;
  uint32_t frameCounter;
} sli_zigbee_crypto_timing_info_t;

void sli_zigbee_crypto_timing_record_enable(bool on);
void sli_zigbee_crypto_timing_reset(void);
const sli_zigbee_crypto_timing_info_t* sli_zigbee_crypto_timing_get_data(uint8_t index,
                                                                         bool outgoing);

// DON'T ADD Below here.  Add new stuff above the CCM* timing header block above.

#endif // SILABS_CRYPTO_H
