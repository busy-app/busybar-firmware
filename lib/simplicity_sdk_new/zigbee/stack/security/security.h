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
#ifndef __SECURITY_H__
#define __SECURITY_H__

// ZigBee definitions:

#define SECURITY_AUTHENTICATION_MASK 0x03
#define SECURITY_ENCRYPTION_MASK     0x04

// 802.15.4 definitions:

// MAC security nonce offsets
#define NONCE_FLAGS_OFFSET         15
#define NONCE_SOURCE_ADDR_OFFSET   7
#define NONCE_FRAME_COUNTER_OFFSET 3
#define NONCE_KEY_SEQ_OFFSET       2
#define NONCE_BLOCK_COUNTER_OFFSET 0

// M MIC size
#define MIC_SIZE_0   0
#define MIC_SIZE_4   4
#define MIC_SIZE_8   8
#define MIC_SIZE_16  16
#define MIC_SIZE_MAX MIC_SIZE_16

// MAC security field sizes
#define SECURITY_FRAME_COUNTER_SIZE 4
#define SECURITY_KEY_SEQ_NUM_SIZE   1

// Zero is not a valid 802.15.4 key sequence number.
#define INVALID_SEQUENCE_NUMBER 0

#if (!defined(SL_ZIGBEE_STACK_IP))
// Pro Stack
  #define SECURITY_AUX_HDR_LENGTH (SECURITY_FRAME_COUNTER_SIZE \
                                   + SECURITY_KEY_SEQ_NUM_SIZE)

// TODO: document this function.
bool sli_zigbee_security_init(void);

// This function implements the platform specific routines
// necesssary to initialize the security hardware.
void sli_zigbee_security_hardware_init(void);

#else
// IP Stack
//
// This assumes that we are always including the EUI64, which will hopefully
// change at some point.
  #define SECURITY_AUX_HDR_LENGTH (1      /* frame control */    \
                                   + SECURITY_FRAME_COUNTER_SIZE \
                                   + EUI64_SIZE                  \
                                   + SECURITY_KEY_SEQ_NUM_SIZE)

void sli_zigbee_set_nwk_frame_counter(uint32_t newFrameCounter);

#endif // !SL_ZIGBEE_STACK_IP

// Serves as a convenience for checking the over-air fcf's security flag.
#define USING_MAC_SECURITY(macInfoField) (macInfoField & MAC_INFO_MAC_SECURITY_MASK)

typedef struct {
  uint8_t keySequenceNumber;
  sl_802154_long_addr_t sourceEui64;
  uint8_t frameCounterBytes[SECURITY_FRAME_COUNTER_SIZE];
} sl_zigbee_security_nonce_t;

// We currently only support 4 byte MICs.
#define sli_zigbee_mic_length 4

// Equivalent to sli_zigbee_mic_length, providing a hint of encapsulation.
#define sli_zigbee_security_get_mic_length() sli_zigbee_mic_length

#if defined SL_ZIGBEE_TEST
extern uint32_t nextNwkFrameCounter;
extern uint32_t nextApsFrameCounter;
#endif

// Initializes the outgoing network frame counter with the value from the token.
void sli_zigbee_security_read_network_frame_counter_token(void);

// Increments the outgoing frame counter and manages its non-volatile storage.
// Returns the new counter value.
uint32_t sli_zigbee_security_increment_outgoing_frame_counter(void);

// Resets the outgoing NWK frame counter to zero.
// Updates the non-volatile storage.
void sli_zigbee_reset_nwk_outgoing_frame_counter(void);

// we expose these to interested parties
extern uint32_t sli_zigbee_security_nonce_frame_counter;

// TODO: why do we have APS-related security routine signatures at the PHY?
void sli_zigbee_security_read_aps_frame_counter_token(void);

// Retreives the current value of the outgoing APS Frame counter
uint32_t sli_zigbee_get_aps_frame_counter(void);

// Increments the current value for the outgoing APS Frame counter.
uint32_t sli_zigbee_next_aps_frame_counter(void);

// Resets the value for the outgoing APS frame counter.  Updates the
// non-volatile storage.
void sli_zigbee_reset_aps_frame_counter(void);

// API

// Authenticates and (maybe) encrypts the message corresponding
// to header.  This function generates an authentication tag (MIC)
// either 4, 8, or 16 bytes long that is appeded to the payload.
// The encryption is done in-situ.
bool sli_zigbee_encrypt_packet(sli_zigbee_packet_header_t header,
                               uint8_t authenticationStartIndex,
                               uint8_t auxFrameIndex);

#if (!defined(SL_ZIGBEE_STACK_IP))
// Pro Stack
//
// This does the same thing, except that the packet is in a flat buffer.
void sli_zigbee_network_encrypt_flat_packet(uint8_t* packet,
                                            uint8_t packetLength,
                                            uint8_t authenticationStartIndex,
                                            uint8_t auxFrameIndex);

// Authenticates and (maybe) decrypts the message corresponding to header.
// 'sourceEui64' is used if the auxiliary frame does not contain the
// senders EUI64.
// The decryption is done in-situ, and payload will be shorter after the
// call, reflecting the removal of the authentication tag.
bool sli_zigbee_decrypt_packet(sli_zigbee_packet_header_t header,
                               uint8_t authenticationStartIndex,
                               uint8_t auxFrameIndex,
                               sl_802154_long_addr_t sourceEui64);

#else
// IP Stack
//
// This does the same thing, except that the packet is in a flat buffer.
void sli_zigbee_encrypt_flat_packet(uint8_t* packet,
                                    uint8_t packetLength,
                                    uint8_t authenticationStartIndex,
                                    uint8_t auxFrameIndex,
                                    bool macMode);

  #define sli_zigbee_encrypt_flat_zigbee_packet(packet, length, authIndex, auxIndex) \
  (sli_zigbee_encrypt_flat_packet((packet), (length), (authIndex), (auxIndex), false))

  #define sli_zigbee_encrypt_flat802d15d4_packet(packet, length, authIndex, auxIndex) \
  (sli_zigbee_encrypt_flat_packet((packet), (length), (authIndex), (auxIndex), true))

// Authenticates and (maybe) decrypts the message corresponding to header.
// 'sourceEui64' is used if the auxiliary frame does not contain the
// senders EUI64.
// The decryption is done in-situ, and payload will be shorter after the
// call, reflecting the removal of the authentication tag.
bool sli_zigbee_decrypt_packet(sli_zigbee_packet_header_t header,
                               uint8_t authenticationStartIndex,
                               uint8_t auxFrameIndex,
                               sl_802154_long_addr_t sourceEui64,
                               bool macMode);

#endif // !SL_ZIGBEE_STACK_IP

#endif //__SECURITY_H__
