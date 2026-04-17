/***************************************************************************//**
 * @file
 * @brief Functions for reading and writing command and response frames.
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

#ifndef SILABS_EZSP_FRAME_UTILITIES_H
#define SILABS_EZSP_FRAME_UTILITIES_H

#include "stack/include/zll-types.h"

#include "ezsp-enum.h" //for sl_zigbee_ezsp_status_t

#include "sl_zigbee_dhc.h"

// The contents of the current EZSP frame.  This pointer can be used inside
// sl_zigbee_ezsp_error_handler() to obtain information about the command that preceded
// the error (such as the command ID, index EZSP_FRAME_ID_INDEX).
extern uint8_t* ezspFrameContents;

// This pointer steps through the received frame as the contents are read.
extern uint8_t* ezspReadPointer;

// This pointer steps through the to-be-transmitted frame as the contents
// are written.
extern uint8_t* ezspWritePointer;

uint8_t fetchInt8u(void);
uint8_t *fetchInt8uPointer(uint8_t length);
void appendInt8u(uint8_t value);

// generated ezsp has a call to this
#define fetchInt16uPointer fetchInt8uPointer

uint16_t fetchInt16u(void);
void appendInt16u(uint16_t value);

void appendInt32u(uint32_t value);
uint32_t fetchInt32u(void);

void appendInt8uArray(uint8_t length, uint8_t *contents);
void fetchInt8uArray(uint8_t length, uint8_t *contents);

void appendInt16uArray(uint8_t length, uint16_t *contents);
void fetchInt16uArray(uint8_t length, uint16_t *contents);

void appendInt32uArray(uint8_t length, uint32_t *contents);
void fetchInt32uArray(uint8_t length, uint32_t *contents);

//Defined separately to workaround signedness warnings
#define fetchInt8 fetchInt8u
#define appendInt8 appendInt8u
#define fetchInt8Array(length, contents) fetchInt8uArray(length, (uint8_t*)contents)
#define appendInt8Array(length, contents) appendInt8uArray(length, (uint8_t*)contents)

// For functions that are used only once in the em260 code we use macros
// in order to save code space.

#define append_sl_zigbee_network_parameters_t(value)                \
  do {                                                              \
    appendInt8uArray(EXTENDED_PAN_ID_SIZE, (value)->extendedPanId); \
    appendInt16u((value)->panId);                                   \
    appendInt8u((value)->radioTxPower);                             \
    appendInt8u((value)->radioChannel);                             \
    appendInt8u((value)->joinMethod);                               \
    appendInt16u((value)->nwkManagerId);                            \
    appendInt8u((value)->nwkUpdateId);                              \
    appendInt32u((value)->channels);                                \
  } while (0)

#define append_sl_zigbee_multi_phy_radio_parameters_t(value) \
  do {                                                       \
    appendInt8u((value)->radioTxPower);                      \
    appendInt8u((value)->radioPage);                         \
    appendInt8u((value)->radioChannel);                      \
  } while (0)

void fetch_sl_zigbee_network_parameters_t(sl_zigbee_network_parameters_t *value);
void fetch_sl_zigbee_multi_phy_radio_parameters_t(sl_zigbee_multi_phy_radio_parameters_t *parameters);

void append_sl_zigbee_aps_frame_t(sl_zigbee_aps_frame_t *value);
void fetch_sl_zigbee_aps_frame_t(sl_zigbee_aps_frame_t *apsFrame);

void append_sl_zigbee_rx_packet_info_t(sl_zigbee_rx_packet_info_t *value);
void fetch_sl_zigbee_rx_packet_info_t(sl_zigbee_rx_packet_info_t *value);

#define append_sl_zigbee_binding_table_entry_t(value)  \
  do {                                                 \
    appendInt8u((value)->type);                        \
    appendInt8u((value)->local);                       \
    appendInt16u((value)->clusterId);                  \
    appendInt8u((value)->remote);                      \
    appendInt8uArray(EUI64_SIZE, (value)->identifier); \
    appendInt8u((value)->networkIndex);                \
  } while (0)

#define fetch_sl_zigbee_binding_table_entry_t(value)  \
  do {                                                \
    (value)->type         = fetchInt8u();             \
    (value)->local        = fetchInt8u();             \
    (value)->clusterId    = fetchInt16u();            \
    (value)->remote       = fetchInt8u();             \
    fetchInt8uArray(EUI64_SIZE, (value)->identifier); \
    (value)->networkIndex = fetchInt8u();             \
  } while (0)

void append_sl_zigbee_multicast_table_entry_t(sl_zigbee_multicast_table_entry_t *value);
void fetch_sl_zigbee_multicast_table_entry_t(sl_zigbee_multicast_table_entry_t *value);

void fetch_sl_zigbee_beacon_classification_params_t(sl_zigbee_beacon_classification_params_t *value);
void append_sl_zigbee_beacon_classification_params_t(sl_zigbee_beacon_classification_params_t *value);

void append_sl_zigbee_neighbor_table_entry_t(sl_zigbee_neighbor_table_entry_t *value);
void fetch_sl_zigbee_neighbor_table_entry_t(sl_zigbee_neighbor_table_entry_t *value);

#define append_sl_zigbee_neighbor_table_entry_t(value) \
  do {                                                 \
    appendInt16u((value)->shortId);                    \
    appendInt8u((value)->averageLqi);                  \
    appendInt8u((value)->inCost);                      \
    appendInt8u((value)->outCost);                     \
    appendInt8u((value)->age);                         \
    appendInt8uArray(EUI64_SIZE, (value)->longId);     \
  } while (0)

#define append_sl_zigbee_route_table_entry_t(value) \
  do {                                              \
    appendInt16u((value)->destination);             \
    appendInt16u((value)->nextHop);                 \
    appendInt8u((value)->status);                   \
    appendInt8u((value)->age);                      \
    appendInt8u((value)->concentratorType);         \
    appendInt8u((value)->routeRecordState);         \
  } while (0)

void fetch_sl_zigbee_route_table_entry_t(sl_zigbee_route_table_entry_t *value);

#define append_sl_zigbee_key_data_t(value) \
  (appendInt8uArray(SL_ZIGBEE_ENCRYPTION_KEY_SIZE, (value)->contents))

#define fetch_sl_zigbee_key_data_t(value) \
  (fetchInt8uArray(SL_ZIGBEE_ENCRYPTION_KEY_SIZE, (value)->contents))

#define append_sl_zigbee_sec_man_key_t(value) \
  (appendInt8uArray(SL_ZIGBEE_ENCRYPTION_KEY_SIZE, (value)->key))

#define fetch_sl_zigbee_sec_man_key_t(value) \
  (fetchInt8uArray(SL_ZIGBEE_ENCRYPTION_KEY_SIZE, (value)->key))

void append_sl_zigbee_sec_man_context_t(sl_zigbee_sec_man_context_t *context);
void fetch_sl_zigbee_sec_man_context_t(sl_zigbee_sec_man_context_t *context);

#define append_sl_zigbee_sec_man_network_key_info_t(value) \
  do {                                                     \
    appendInt8u((value)->network_key_set);                 \
    appendInt8u((value)->alternate_network_key_set);       \
    appendInt8u((value)->network_key_sequence_number);     \
    appendInt8u((value)->alt_network_key_sequence_number); \
    appendInt32u((value)->network_key_frame_counter);      \
  } while (0)

#define fetch_sl_zigbee_sec_man_network_key_info_t(value)    \
  do {                                                       \
    (value)->network_key_set = fetchInt8u();                 \
    (value)->alternate_network_key_set = fetchInt8u();       \
    (value)->network_key_sequence_number = fetchInt8u();     \
    (value)->alt_network_key_sequence_number = fetchInt8u(); \
    (value)->network_key_frame_counter = fetchInt32u();      \
  } while (0)

void append_sl_zigbee_sec_man_aps_key_metadata_t(sl_zigbee_sec_man_aps_key_metadata_t *info);
void fetch_sl_zigbee_sec_man_aps_key_metadata_t(sl_zigbee_sec_man_aps_key_metadata_t *info);

#define append_sl_802154_radio_priorities_t(value) \
  do {                                             \
    appendInt8u((value)->background_rx);           \
    appendInt8u((value)->min_tx_priority);         \
    appendInt8u((value)->tx_step);                 \
    appendInt8u((value)->max_tx_priority);         \
    appendInt8u((value)->active_rx);               \
  } while (0)

#define fetch_sl_802154_radio_priorities_t(value) \
  do {                                            \
    (value)->background_rx = fetchInt8u();        \
    (value)->min_tx_priority = fetchInt8u();      \
    (value)->tx_step = fetchInt8u();              \
    (value)->max_tx_priority = fetchInt8u();      \
    (value)->active_rx = fetchInt8u();            \
  } while (0)

#define append_sl_zigbee_endpoint_description_t(value) \
  do {                                                 \
    appendInt16u((value)->profileId);                  \
    appendInt16u((value)->deviceId);                   \
    appendInt8u((value)->deviceVersion);               \
    appendInt8u((value)->inputClusterCount);           \
    appendInt8u((value)->outputClusterCount);          \
  } while (0)

#define fetch_sl_zigbee_endpoint_description_t(value) \
  do {                                                \
    (value)->profileId = fetchInt16u();               \
    (value)->deviceId = fetchInt16u();                \
    (value)->deviceVersion = fetchInt8u();            \
    (value)->inputClusterCount = fetchInt8u();        \
    (value)->outputClusterCount = fetchInt8u();       \
  } while (0)

#define append_sl_zigbee_transient_key_data_t(value)  \
  do {                                                \
    appendInt8uArray(EUI64_SIZE, (value)->eui64);     \
    append_sl_zigbee_key_data_t(&((value)->keyData)); \
    appendInt32u((value)->incomingFrameCounter);      \
    appendInt16u((value)->bitmask);                   \
    appendInt16u((value)->remainingTimeSeconds);      \
    appendInt8u((value)->networkIndex);               \
  } while (0)

#define fetch_sl_zigbee_transient_key_data_t(value)  \
  do {                                               \
    fetchInt8uArray(EUI64_SIZE, (value)->eui64);     \
    fetch_sl_zigbee_key_data_t(&((value)->keyData)); \
    (value)->incomingFrameCounter = fetchInt32u();   \
    (value)->bitmask = fetchInt16u();                \
    (value)->remainingTimeSeconds = fetchInt16u();   \
    (value)->networkIndex = fetchInt8u();            \
  } while (0)

void append_sl_zigbee_initial_security_state_t(sl_zigbee_initial_security_state_t *value);

#define fetch_sl_zigbee_initial_security_state_t(value)                  \
  do {                                                                   \
    (value)->bitmask = fetchInt16u();                                    \
    fetch_sl_zigbee_key_data_t(&((value)->preconfiguredKey));            \
    fetch_sl_zigbee_key_data_t(&((value)->networkKey));                  \
    (value)->networkKeySequenceNumber = fetchInt8u();                    \
    fetchInt8uArray(EUI64_SIZE, (value)->preconfiguredTrustCenterEui64); \
  } while (0)

#define append_sl_zigbee_current_security_state_t(value)           \
  do {                                                             \
    appendInt16u((value)->bitmask);                                \
    appendInt8uArray(EUI64_SIZE, (value)->trustCenterLongAddress); \
  } while (0)

void fetch_sl_zigbee_current_security_state_t(sl_zigbee_current_security_state_t *value);

#define append_sl_zigbee_key_struct_t(value)             \
  do {                                                   \
    appendInt16u((value)->bitmask);                      \
    appendInt8u((value)->type);                          \
    if ((value)->bitmask & SL_ZIGBEE_KEY_HAS_PSA_ID) {   \
      appendInt32u((value)->psa_id);                     \
    } else {                                             \
      append_sl_zigbee_key_data_t(&((value)->key));      \
    }                                                    \
    appendInt32u((value)->outgoingFrameCounter);         \
    appendInt32u((value)->incomingFrameCounter);         \
    appendInt8u((value)->sequenceNumber);                \
    appendInt8uArray(EUI64_SIZE, (value)->partnerEUI64); \
  } while (0)

void fetch_sl_zigbee_key_struct_t(sl_zigbee_key_struct_t *value);

#define append_sl_zigbee_child_data_t(value)      \
  do {                                            \
    appendInt8uArray(EUI64_SIZE, (value)->eui64); \
    appendInt8u((value)->type);                   \
    appendInt16u((value)->id);                    \
    appendInt8u((value)->phy);                    \
    appendInt8u((value)->power);                  \
    appendInt8u((value)->timeout);                \
    appendInt32u((value)->remainingTimeout);      \
  } while (0)

void fetch_sl_zigbee_child_data_t(sl_zigbee_child_data_t *value);

void fetch_sl_zigbee_zigbee_network_t(sl_zigbee_zigbee_network_t *value);
void append_sl_zigbee_zigbee_network_t(sl_zigbee_zigbee_network_t *value);

#define append_sl_zigbee_certificate_data_t(value) \
  (appendInt8uArray(SL_ZIGBEE_CERTIFICATE_SIZE, (value)->contents))
#define fetch_sl_zigbee_certificate_data_t(value) \
  (fetchInt8uArray(SL_ZIGBEE_CERTIFICATE_SIZE, (value)->contents))
#define append_sl_zigbee_public_key_data_t(value) \
  (appendInt8uArray(SL_ZIGBEE_PUBLIC_KEY_SIZE, (value)->contents))
#define fetch_sl_zigbee_public_key_data_t(value) \
  (fetchInt8uArray(SL_ZIGBEE_PUBLIC_KEY_SIZE, (value)->contents))
#define append_sl_zigbee_private_key_data_t(value) \
  (appendInt8uArray(SL_ZIGBEE_PRIVATE_KEY_SIZE, (value)->contents))
#define fetch_sl_zigbee_private_key_data_t(value) \
  (fetchInt8uArray(SL_ZIGBEE_PRIVATE_KEY_SIZE, (value)->contents))
#define append_sl_zigbee_smac_data_t(value) \
  (appendInt8uArray(SL_ZIGBEE_SMAC_SIZE, (value)->contents))
#define fetch_sl_zigbee_smac_data_t(value) \
  (fetchInt8uArray(SL_ZIGBEE_SMAC_SIZE, (value)->contents))
#define append_sl_zigbee_signature_data_t(value) \
  (appendInt8uArray(SL_ZIGBEE_SIGNATURE_SIZE, (value)->contents))
#define fetch_sl_zigbee_signature_data_t(value) \
  (fetchInt8uArray(SL_ZIGBEE_SIGNATURE_SIZE, (value)->contents))

#define append_sl_zigbee_certificate_283k1_data_t(value) \
  (appendInt8uArray(SL_ZIGBEE_CERTIFICATE_283K1_SIZE, (value)->contents))
#define fetch_sl_zigbee_certificate_283k1_data_t(value) \
  (fetchInt8uArray(SL_ZIGBEE_CERTIFICATE_283K1_SIZE, (value)->contents))
#define append_sl_zigbee_public_key_283k1_data_t(value) \
  (appendInt8uArray(SL_ZIGBEE_PUBLIC_KEY_283K1_SIZE, (value)->contents))
#define fetch_sl_zigbee_public_key_283k1_data_t(value) \
  (fetchInt8uArray(SL_ZIGBEE_PUBLIC_KEY_283K1_SIZE, (value)->contents))
#define append_sl_zigbee_private_key_283k1_data_t(value) \
  (appendInt8uArray(SL_ZIGBEE_PRIVATE_KEY_283K1_SIZE, (value)->contents))
#define fetch_sl_zigbee_private_key_283k1_data_t(value) \
  (fetchInt8uArray(SL_ZIGBEE_PRIVATE_KEY_283K1_SIZE, (value)->contents))
#define append_sl_zigbee_signature_283k1_data_t(value) \
  (appendInt8uArray(SL_ZIGBEE_SIGNATURE_283K1_SIZE, (value)->contents))
#define fetch_sl_zigbee_signature_283k1_data_t(value) \
  (fetchInt8uArray(SL_ZIGBEE_SIGNATURE_283K1_SIZE, (value)->contents))

void append_sl_zigbee_aes_mmo_hash_context_t(sl_zigbee_aes_mmo_hash_context_t* context);
void fetch_sl_zigbee_aes_mmo_hash_context_t(sl_zigbee_aes_mmo_hash_context_t* context);

#define append_sl_zigbee_message_digest_t(value) \
  (appendInt8uArray(SL_ZIGBEE_AES_HASH_BLOCK_SIZE, (value)->contents))
#define fetch_sl_zigbee_message_digest_t(value) \
  (fetchInt8uArray(SL_ZIGBEE_AES_HASH_BLOCK_SIZE, (value)->contents))

void append_sl_zigbee_network_init_struct_t(const sl_zigbee_network_init_struct_t* networkInitStruct);
void fetch_sl_zigbee_network_init_struct_t(sl_zigbee_network_init_struct_t* networkInitStruct);

void append_sl_zigbee_zll_network_t(sl_zigbee_zll_network_t* network);
void fetch_sl_zigbee_zll_network_t(sl_zigbee_zll_network_t* network);
void fetch_sl_zigbee_zll_security_algorithm_data_t(sl_zigbee_zll_security_algorithm_data_t* data);
void fetch_sl_zigbee_tok_type_stack_zll_data_t(sl_zigbee_tok_type_stack_zll_data_t *data);
void fetch_sl_zigbee_tok_type_stack_zll_security_t(sl_zigbee_tok_type_stack_zll_security_t *security);
void append_sl_zigbee_zll_security_algorithm_data_t(sl_zigbee_zll_security_algorithm_data_t* data);
void append_sl_zigbee_zll_initial_security_state_t(sl_zigbee_zll_initial_security_state_t* state);
void append_sl_zigbee_tok_type_stack_zll_data_t(sl_zigbee_tok_type_stack_zll_data_t *data);
void append_sl_zigbee_tok_type_stack_zll_security_t(sl_zigbee_tok_type_stack_zll_security_t *security);

void append_sl_zigbee_gp_address_t(sl_zigbee_gp_address_t *value);
void fetch_sl_zigbee_gp_address_t(sl_zigbee_gp_address_t *value);
void fetch_sl_zigbee_gp_params_t(sl_zigbee_gp_params_t *param);
void fetch_sl_zigbee_gp_sink_list_t(sl_zigbee_gp_sink_list_entry_t *value);
void fetch_sl_zigbee_gp_proxy_table_entry_t(sl_zigbee_gp_proxy_table_entry_t *value);
void append_sl_zigbee_gp_proxy_table_entry_t(sl_zigbee_gp_proxy_table_entry_t *value);

void fetch_sl_zigbee_gp_sink_table_entry_t(sl_zigbee_gp_sink_table_entry_t *value);
void append_sl_zigbee_gp_sink_table_entry_t(sl_zigbee_gp_sink_table_entry_t *value);

void append_sl_zigbee_duty_cycle_limits_t(sl_zigbee_duty_cycle_limits_t* limits);
void fetch_sl_zigbee_duty_cycle_limits_t(sl_zigbee_duty_cycle_limits_t* limits);
void append_sl_zigbee_per_device_duty_cycle_t(uint8_t maxDevices,
                                              sl_zigbee_per_device_duty_cycle_t* arrayOfDeviceDutyCycles);
void fetch_sl_zigbee_per_device_duty_cycle_t(sl_zigbee_per_device_duty_cycle_t* arrayOfDeviceDutyCycles);
void format_bytes_in_to_struct_sl_zigbee_per_device_duty_cycle_t(uint8_t* perDeviceDutyCycles,
                                                                 sl_zigbee_per_device_duty_cycle_t* arrayOfDeviceDutyCycles);
void format_struct_sl_zigbee_per_device_duty_cycle_into_bytes_t(uint8_t maxDevices,
                                                                sl_zigbee_per_device_duty_cycle_t *arrayOfDeviceDutyCycles,
                                                                uint8_t* perDeviceDutyCycles);

#define fetch_sl_zigbee_zll_device_info_record_t(value) \
  do {                                                  \
    fetchInt8uArray(EUI64_SIZE, (value)->ieeeAddress);  \
    (value)->endpointId         = fetchInt8u();         \
    (value)->profileId          = fetchInt16u();        \
    (value)->deviceId           = fetchInt16u();        \
    (value)->version            = fetchInt8u();         \
    (value)->groupIdCount       = fetchInt8u();         \
  } while (0)

#define fetch_sl_zigbee_zll_initial_security_state_t(value)   \
  do {                                                        \
    (value)->bitmask = fetchInt32u();                         \
    (value)->keyIndex = fetchInt8u();                         \
    fetch_sl_zigbee_key_data_t(&((value)->encryptionKey));    \
    fetch_sl_zigbee_key_data_t(&((value)->preconfiguredKey)); \
  } while (0)

#define fetch_sl_zigbee_zll_address_assignment_t(value) \
  do {                                                  \
    (value)->nodeId         = fetchInt16u();            \
    (value)->freeNodeIdMin  = fetchInt16u();            \
    (value)->freeNodeIdMax  = fetchInt16u();            \
    (value)->groupIdMin     = fetchInt16u();            \
    (value)->groupIdMax     = fetchInt16u();            \
    (value)->freeGroupIdMin = fetchInt16u();            \
    (value)->freeGroupIdMax = fetchInt16u();            \
  } while (0)

#define append_sl_zigbee_beacon_data_t(value)                       \
  do {                                                              \
    appendInt8u((value)->channel);                                  \
    appendInt8u((value)->lqi);                                      \
    appendInt8u((value)->rssi);                                     \
    appendInt8u((value)->depth);                                    \
    appendInt8u((value)->nwkUpdateId);                              \
    appendInt8u((value)->power);                                    \
    appendInt8u((value)->parentPriority);                           \
    appendInt8u((value)->enhanced);                                 \
    appendInt8u((value)->permitJoin);                               \
    appendInt8u((value)->hasCapacity);                              \
    appendInt16u((value)->panId);                                   \
    appendInt16u((value)->sender);                                  \
    appendInt8uArray(EXTENDED_PAN_ID_SIZE, (value)->extendedPanId); \
  } while (0)
#define fetch_sl_zigbee_beacon_data_t(value)                       \
  do {                                                             \
    memset((value), 0, sizeof(sl_zigbee_beacon_data_t));           \
    (value)->channel        = fetchInt8u();                        \
    (value)->lqi            = fetchInt8u();                        \
    (value)->rssi           = fetchInt8u();                        \
    (value)->depth          = fetchInt8u();                        \
    (value)->nwkUpdateId    = fetchInt8u();                        \
    (value)->power          = fetchInt8u();                        \
    (value)->parentPriority = fetchInt8u();                        \
    (value)->enhanced       = fetchInt8u();                        \
    (value)->permitJoin     = fetchInt8u();                        \
    (value)->hasCapacity    = fetchInt8u();                        \
    (value)->panId          = fetchInt16u();                       \
    (value)->sender         = fetchInt16u();                       \
    fetchInt8uArray(EXTENDED_PAN_ID_SIZE, (value)->extendedPanId); \
  } while (0)

// Format the same as in command-interpreter2.h, with the addition of
// a 'p' for a pointer.
// Make sure to pass references to all of the variables! And if an array is
// passed, the length needs to come directly after it. This length should
// be a reference as well.
void sli_zigbee_af_fetch_or_append(bool fetch, const char * format, ...);

void append_sl_zigbee_token_data_t(sl_zigbee_token_data_t * tokenData);
void fetch_sl_zigbee_token_data_t(sl_zigbee_token_data_t * tokenData);
void append_sl_zigbee_token_info_t(sl_zigbee_token_info_t *tokenInfo);
void fetch_sl_zigbee_token_info_t(sl_zigbee_token_info_t *tokenInfo);

void append_sl_zigbee_dhc_pa_metadata_t(sl_zigbee_dhc_pa_metadata_t *metadata);
void fetch_sl_zigbee_dhc_pa_metadata_t(sl_zigbee_dhc_pa_metadata_t *metadata);
void append_sl_zigbee_dhc_pa_descriptor_t(sl_zigbee_dhc_pa_descriptor_t *descriptor);
void fetch_sl_zigbee_dhc_pa_descriptor_t(sl_zigbee_dhc_pa_descriptor_t *descriptor);
void append_sl_zigbee_dhc_pa_curve_segment_t(sl_zigbee_dhc_pa_curve_segment_t *segment);
void fetch_sl_zigbee_dhc_pa_curve_segment_t(sl_zigbee_dhc_pa_curve_segment_t *segment);
void append_sl_zigbee_dhc_pa_curve_t(sl_zigbee_dhc_pa_curve_t *curve);
void fetch_sl_zigbee_dhc_pa_curve_t(sl_zigbee_dhc_pa_curve_t *curve);
void append_sl_zigbee_dhc_pa_table_t (sl_zigbee_dhc_pa_table_t *table);
void fetch_sl_zigbee_dhc_pa_table_t (sl_zigbee_dhc_pa_table_t *table);
void append_sl_zigbee_dhc_pa_version_t(sl_zigbee_dhc_pa_version_t *pa_version);
void fetch_sl_zigbee_dhc_pa_version_t(sl_zigbee_dhc_pa_version_t *pa_version);
void append_sl_zigbee_dhc_pa_signature_t(sl_zigbee_dhc_pa_signature_t *pa_signature);
void fetch_sl_zigbee_dhc_pa_signature_t(sl_zigbee_dhc_pa_signature_t *pa_signature);
void append_sl_zigbee_dhc_pa_mode_t(sl_zigbee_dhc_pa_mode_t *pa_mode);
void fetch_sl_zigbee_dhc_pa_mode_t(sl_zigbee_dhc_pa_mode_t *pa_mode);
void append_sl_zigbee_dhc_rssi_offset_t(sl_zigbee_dhc_rssi_offset_t *rssi_offset);
void fetch_sl_zigbee_dhc_rssi_offset_t(sl_zigbee_dhc_rssi_offset_t *rssi_offset);
void append_sl_zigbee_dhc_ctune_t(sl_zigbee_dhc_ctune_t *ctune);
void fetch_sl_zigbee_dhc_ctune_t(sl_zigbee_dhc_ctune_t *ctune);

//Writes an sl_zigbee_ezsp_status_t value into the location that sl_zigbee_ezsp_get_last_status
//reads from.  Used by EZSP (application code should only need to read it).
void sli_zigbee_ezsp_set_last_status(sl_zigbee_ezsp_status_t status);

#endif // __EZSP_FRAME_UTILITIES_H__
