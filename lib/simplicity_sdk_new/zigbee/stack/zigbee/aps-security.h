/***************************************************************************//**
 * @file
 * @brief implementation of the ZigBee application support sublayer (APS)
 * security.
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

#ifndef SILABS_APS_SECURITY_H
#define SILABS_APS_SECURITY_H

// All APS Command Frames begin with
//   FrameControl       (1)
//   APS Counter        (1)

//   APS Payload
//     CommandIdentifier  (1)
//     Command Data       (variable)

// NOTE:  All these offsets are relative to the start of the APS Command,
// not the start of the APS Frame.  They all include the APS Command ID
// as the first byte.

#define APS_COMMAND_IDENTIFIER_OFFSET 0

// Transport Key
//   KeyType            (1)
//   KeyDescriptor      (variable)

#define TRANSPORT_KEY_TYPE_OFFSET  1

// The possible key types and descriptors are:

// Type 1, Residential or Standard Network key
//   Key               (16)
//   SequenceNumber     (1)
//   DestinationAddress (8)
//   SourceAddress      (8) (the trust center itself)

#define NETWORK_KEY_TRANSPORT_KEY_OFFSET          2
#define NETWORK_KEY_TRANSPORT_SEQUENCE_OFFSET    18
#define NETWORK_KEY_TRANSPORT_DESTINATION_OFFSET 19
#define NETWORK_KEY_TRANSPORT_SOURCE_OFFSET      27

#define NETWORK_KEY_TRANSPORT_FRAME_SIZE         35

// Type 3, application link key
//   Key               (16)
//   PartnerAddress     (8) (the sender of the key)
//   InitiatorFlag      (1) (1 if requested by recipient, 0 if not)

#define APP_LINK_KEY_TRANSPORT_OFFSET               2
#define APP_LINK_KEY_TRANSPORT_PARTNER_OFFSET      18
#define APP_LINK_KEY_TRANSPORT_INITIATOR_OFFSET    26

#define APP_LINK_KEY_TRANSPORT_FRAME_SIZE          27

// Type 4, Trust Center Link Key
//   Key               (16)
//   DestinationAddress (8)
//   SourceAddress      (8) (the trust center itself)
#define TC_LINK_KEY_TRANSPORT_OFFSET              2
#define TC_LINK_KEY_TRANSPORT_DESTINATION_OFFSET 18
#define TC_LINK_KEY_TRANSPORT_SOURCE_OFFSET      26

#define TC_LINK_KEY_TRANSPORT_FRAME_SIZE         34

#define TRANSPORT_KEY_MAX_FRAME_SIZE  NETWORK_KEY_TRANSPORT_FRAME_SIZE

// Update Device
//   DeviceAddress      (8)
//   DeviceShortAddress (2)
//   Status             (1)
// The status is:
//   0 secured join
//   1 unsecured join
//   2 departed

#define UPDATE_DEVICE_ADDRESS_OFFSET         1
#define UPDATE_DEVICE_SHORT_ADDRESS_OFFSET   9
#define UPDATE_DEVICE_STATUS_OFFSET         11
#define UPDATE_DEVICE_FRAME_SIZE            12

// Remove Device
//   TargetAddress       (8)

#define REMOVE_DEVICE_TARGET_OFFSET 1
#define REMOVE_DEVICE_FRAME_SIZE 9

// Request Key
//   KeyType            (1) (only network and link (TC or app) keys are allowed)
//   PartnerAddress     (8) (only for application link keys)
#define REQUEST_KEY_TYPE_OFFSET 1
#define REQUEST_KEY_PARTNER_OFFSET 2

#define REQUEST_KEY_BASE_FRAME_SIZE   2
#define REQUEST_KEY_FRAME_SIZE        10

//
// Switch Key
//   SequenceNumber     (1)
#define SWITCH_KEY_SEQUENCE_NUMBER_OFFSET   1
#define SWITCH_KEY_FRAME_SIZE 2

// Section 4.4.9.7 of the Zigbee Pro R21 Draft.
// Verify Key Command
#define VERIFY_KEY_TYPE_OFFSET            1
#define VERIFY_SOURCE_ADDRESS_OFFSET      2
#define VERIFY_KEY_HASH_OFFSET            10
#define VERIFY_KEY_FRAME_SIZE             26

//Section 4.4.9.8 of the Zigbee Pro R21 draft.
// Verify Key Confirm Command
#define VERIFY_KEY_CONFIRM_STATUS_OFFSET              1
#define VERIFY_KEY_CONFIRM_KEY_TYPE_OFFSET            2
#define VERIFY_KEY_CONFIRM_DESTINATION_ADDRESS_OFFSET 3
#define VERIFY_KEY_CONFIRM_FRAME_SIZE 11

#define VERIFY_KEY_CONFIRM_STATUS_ILLEGAL_REQUEST  (0xA3)
#define VERIFY_KEY_CONFIRM_STATUS_NOT_SUPPORTED    (0xAA)
#define VERIFY_KEY_CONFIRM_STATUS_SECURITY_FAILURE (0xAD)
#define VERIFY_KEY_CONFIRM_STATUS_SUCCESS          (0x00)

// Tunnel Data  (Tunnel the passed APS Command)
//   Destination EUI64      (8)
//   APS Command            (1)
//   Aux Frame
//     Security Control     (1)
//       Level 5
//       Key-Transport Key
//       Extended Nonce
//     Frame Counter        (4)
//     Source Address       (8)
//   Tunnelled APS Command  (Variable)
#define TUNNEL_DATA_DESTINATION_OFFSET        1

#define TUNNEL_DATA_COMMAND_FRAME_SIZE        9 // APS Command byte + Dest EUI64

#define TUNNEL_DATA_MIN_FRAME_SIZE 28

#define MAX_APS_COMMAND_FRAME_SIZE TRANSPORT_KEY_MAX_FRAME_SIZE

// -----------------------------------------------------------------------------
// Security Policy for sending/accepting APS commands (Tables X.XX and Y.YY).
// -----------------------------------------------------------------------------
// APS COMMAND                            Unique Link Key        Global Link Key
//                                        APS Encryption         APS Encryption
//                                         (send/accept)          (send/accept)
// -----------------------------------------------------------------------------
// Transport Key (0x05)                   POLICY / POLICY        POLICY / POLICY
// Update Device (0x06)                   YES / YES              POLICY / NO
// Remove Device (0x07)                   YES / YES              YES / YES
// Request Key (0x08)                     POLICY / POLICY        NO / NO
// Switch Key (0x09)                      NO / NO                NO / NO
// Tunnel Data (0x0E)                     NO / NO                NO / NO
// -----------------------------------------------------------------------------

// More info regarding some of these commands:
// -----------------------------------------------------------------------------
// Transport Key (0x05): The accepting decision is delegated to the policy
//   implemented in processKeyCommand(). Using encryption in outgoing messages
//   is also delegated to the security policy.
// -----------------------------------------------------------------------------
// Update Device (0x06): If a Global Link Key is in use and the bit
//   SL_ZIGBEE_R18_STACK_BEHAVIOR in the Extended Security Bitmask is not set, we
//   send out two messages, one encrypted and one non-encrypted. If a Unique
//   Link Key is in use, we send out only an encrypted message.
// -----------------------------------------------------------------------------
// Request Key (0x08): If the node is in Unique Link Key mode:
//   Incoming: if the request is for a Trust Center Link Key we accept both
//     encrypted or non-encrypted, otherwise we only accept encrypted messages.
//   Outgoing: if the request is for a Trust Center Link Key we do not encrypt
//     the request, otherwise we encrypt it.

// Bits 0 and 15 are unused.
#define UNIQUE_LINK_KEY_SECURITY_POLICY_TABLE 0x01E0
#define GLOBAL_LINK_KEY_SECURITY_POLICY_TABLE 0x01A0

// Returns true if the command should be encrypted, false otherwise.
#define sli_zigbee_aps_command_security_policy(command) \
  (1U << (command))                                     \
  & ((sli_zigbee_using_global_link_key())               \
     ? GLOBAL_LINK_KEY_SECURITY_POLICY_TABLE            \
     : UNIQUE_LINK_KEY_SECURITY_POLICY_TABLE)

// Zigbee Values
enum {
  REQUEST_NETWORK_KEY           = 1,
  REQUEST_APP_LINK_KEY          = 2,
  // 3 is reserved
  REQUEST_TRUST_CENTER_LINK_KEY = 4,
};

bool sli_zigbee_is_network_key_type(uint8_t keyType);

typedef uint8_t sli_zigbee_send_aps_command_options_t;

#define firstKeySequenceHigherThanSecond(first, second) \
  (timeGTorEqualInt8u((first), (second)))

void sli_zigbee_set_network_key(sl_zigbee_key_data_t* key,
                                uint8_t keySequenceNumber,
                                bool current);
#define setActiveNetworkKey(key, sequence) \
  (sli_zigbee_set_network_key((key), (sequence), true))

#define setAlternateNetworkKey(key, sequence) \
  (sli_zigbee_set_network_key((key), (sequence), false))

// For now, assume Trust Center == Coordinator.
// TODO: this needs to change for the R21 feature, distributed trust center mode
#define sli_zigbee_get_trust_center_node_id() (SL_ZIGBEE_ZIGBEE_COORDINATOR_ADDRESS)

#if defined(SL_ZIGBEE_LEAF_STACK)  || (defined(SL_ZIGBEE_ROUTER_STACK))
#define sli_zigbee_am_trust_center false
#else
#define sli_zigbee_am_trust_center (sli_zigbee_stack_get_node_id() == sli_zigbee_get_trust_center_node_id())
#endif

void sli_zigbee_application_security_message_handler(sli_zigbee_packet_header_t header,
                                                     sl_802154_short_addr_t source,
                                                     bool wasEncrypted,
                                                     bool isTcKeyUnconfirmed,
                                                     bool isAppKeyUnconfirmed);

// The long destination is only needed for key transport messages.
bool sli_zigbee_send_aps_command(sl_802154_short_addr_t destination,
                                 sl_802154_long_addr_t longDestination,
                                 sli_buffer_manager_buffer_t payload,
                                 sli_zigbee_send_aps_command_options_t options);

bool sli_zigbee_is_null_key(sl_zigbee_key_data_t* key);

bool sli_zigbee_send_update_device(sl_802154_short_addr_t newDeviceShortId,
                                   sl_802154_long_addr_t  newDeviceLongId,
                                   sl_zigbee_device_update_t deviceStatus,
                                   void *joiner_tlvs);

// Switch to the alternate key if it is newer than the active key.
// If 'haveNewNumber' is true, then only switch if the alternate key
// has sequence number 'newNumber'.

bool sli_zigbee_switch_network_keys(bool haveNewNumber, uint8_t newNumber);
#define sli_zigbee_switch_to_this_network_key(newNumber) \
  (sli_zigbee_switch_network_keys(true, (newNumber)))

bool sli_zigbee_get_trust_center_eui64(sl_802154_long_addr_t address);
bool sli_zigbee_set_trust_center_eui64(sl_802154_long_addr_t address);
bool sli_zigbee_override_trust_center_eui64(sl_802154_long_addr_t address);

bool sli_zigbee_check_or_set_trust_center_eui64(sl_802154_long_addr_t address);

bool sli_zigbee_send_key(sl_802154_short_addr_t destinationShortId,
                         sl_802154_long_addr_t destinationLongId,
                         sl_802154_long_addr_t sourceOrPartnerLongId,
                         sli_zigbee_key_type_t keyType,
                         sl_zigbee_key_data_t* keyData,
                         sli_zigbee_send_aps_command_options_t options);

bool sli_zigbee_load_network_key(uint8_t sequenceNumber);

#define sli_zigbee_load_unique_key(key) sli_util_load_key_into_core(key)

// Right now, the only APS Security initialization is the Link Key frame
// counters.  But this #define makes it nicer to expand the init routine
// in the future.
#define sli_zigbee_aps_security_init()

void sli_zigbee_clear_security_data(void);

// bit 0:       Preconfig Key not Authorized (private bit, stored in token)
// bits 2-3:    Future use (private, stored in token)
// bit 4:       Joiner (router/end-device) global Link Key (public bit, stored in token)
// bits 5:      Frame counter reset flag (public, stored in token)
// bit 6:       Disable leave without rejoing (public, stored in token)
// bit 7:       Future use (public, stored in token)
// bit 8:       NWK Leave Request allowed (public, stored in RAM only)
// bit 9:       R18 specs behavior enabled (public, stored in RAM only)
// bit 10:      Future use (public, stored in token)
// bit 11:      Future use (public, stored in RAM only)
// bit 12:      Trust Center EUI has changed (private bit, stored in RAM only)
// bits 13-15:  Future use (private, stored in RAM only)
#define PUBLIC_EXTENDED_SECURITY_BITMASK 0x0772

// These values are the position of the bits of the extended security bitmask
// stored in the sli_zigbee_security_state_bitmask RAM 32 bits value.
#define JOINER_GLOBAL_LINK_KEY (((uint32_t)SL_ZIGBEE_JOINER_GLOBAL_LINK_KEY) << 16)
#define NWK_LEAVE_REQUEST_NOT_ALLOWED (((uint32_t)SL_ZIGBEE_NWK_LEAVE_REQUEST_NOT_ALLOWED) << 16)
#define NWK_LEAVE_WITHOUT_REJOIN_NOT_ALLOWED (((uint32_t)SL_ZIGBEE_NWK_LEAVE_WITHOUT_REJOIN_NOT_ALLOWED) << 16)
#define R18_STACK_BEHAVIOR (((uint32_t)SL_ZIGBEE_R18_STACK_BEHAVIOR) << 16)

// This mask drops the zdo leave request in an end device when the request is
// from a non parent.
#define ZDO_LEAVE_FROM_NON_PARENT_NOT_ALLOWED (((uint32_t)SL_ZIGBEE_ZDO_LEAVE_FROM_NON_PARENT_NOT_ALLOWED) << 16)

#define sli_zigbee_distributed_trust_center_mode_enabled() \
  (sli_zigbee_get_security_state(SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE))

sl_status_t sli_zigbee_really_get_network_key(sl_zigbee_key_data_t* keyData,
                                              uint8_t* sequenceNumber,
                                              bool wantCurrentKey);

sl_status_t sli_zigbee_security_okay_to_start_network_operation(bool coordinator,
                                                                bool joinQuietly);
void sli_zigbee_initialize_trust_center_security(void);
void sli_zigbee_trust_center_added_as_neighbor(void);

// Returns true if the node is using a Global Link Key, false if the node is
// using a Unique Link Key.
bool sli_zigbee_using_global_link_key(void);

bool sli_zigbee_using_default_link_key(void);

uint16_t sli_zigbee_get_min_rejoin_period_millisec(void);
//Min rejoin Period could be as big as 255 seconds
void sli_zigbee_set_min_rejoin_period_sec(uint8_t period);

sl_status_t sli_zigbee_send_message_to_verify_transient_link_key(void);

sl_status_t sli_zigbee_get_key_table_entry(uint8_t index, sl_zigbee_key_struct_t *result);

#if (defined SL_ZIGBEE_TEST || defined SL_ZIGBEE_GOLDEN_UNIT)
extern bool sli_zigbee_accept_only_non_encrypted_update_device_messages;
extern bool sli_zigbee_accept_only_encrypted_update_device_messages;
extern bool sli_zigbee_send_non_encrypted_remove_device;
extern bool sli_zigbee_send_only_non_encrypted_update_device;
#endif //(defined SL_ZIGBEE_TEST || defined SL_ZIGBEE_GOLDEN_UNIT)

// eui64.c
extern sl_802154_long_addr_t sli_802154mac_local_eui64;

#endif // SILABS_APS_SECURITY_H
