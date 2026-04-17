/***************************************************************************//**
 * @file
 * @brief implementation of the ZigBee application support sublayer (APS)
 * Key management routines.
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

#ifndef SILABS_APS_KEYS_H
#define SILABS_APS_KEYS_H

#include "include/sl_zigbee.h"
#include "internal/inc/internal-defs-patch.h"

//----------------------------------------------------------------

extern sli_zigbee_event_t sli_zigbee_request_key_events[];
extern uint8_t sli_zigbee_request_key_timeout;
extern sli_zigbee_event_t sli_zigbee_partner_key_update_event;

// From Table 1 of the BDB spec
#define BDBC_TC_LINK_KEY_EXCHANGE_TIMEOUT_MS (5 * 1000)

#define UPDATE_TC_LINK_KEY_STATE_NONE             (0x00)
#define UPDATE_TC_LINK_KEY_STATE_NODE_DESCRIPTOR  (0x01)
#define UPDATE_TC_LINK_KEY_STATE_REQUEST_KEY      (0x02)
#define UPDATE_TC_LINK_KEY_STATE_VERIFY_KEY       (0x03)
// NOTE this technically comes before verify key in the sequence
#define UPDATE_TC_LINK_KEY_STATE_DLK_NEGOTIATION  (0x04)

#define UPDATE_APP_LINK_KEY_STATE_NONE                        (0x00)
#define UPDATE_APP_LINK_KEY_STATE_SECURITY_LEVEL_INITIATOR    (0x01) // Initiator has sent Get Auth Level to TC about target
#define UPDATE_APP_LINK_KEY_STATE_REQUEST_KEY_INITIATOR       (0x02) // Target has sent Request Key to TC for link key with target
#define UPDATE_APP_LINK_KEY_STATE_SECURITY_LEVEL_TARGET       (0x03) // Target has sent Get Auth Level to TC about initiator
#define UPDATE_APP_LINK_KEY_STATE_AWAIT_VERIFY_KEY_INITIATOR  (0x04) // Initiator waits for Verify Key from target
#define UPDATE_APP_LINK_KEY_STATE_VERIFY_KEY_TARGET           (0x05) // Target has sent Verify Key to initiator

// This key is "ZigBeeAlliance09"
#define ZIGBEE_DEFAULT_LINK_KEY                        \
  {                                                    \
    { 0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6C,  \
      0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39 } \
  }

typedef enum {
  EM_GET_KEY_BITMASK_NONE = 0,
  EM_GET_KEY_BITMASK_AUTHORIZED_ONLY = 1,
  EM_GET_KEY_BITMASK_TRANSIENT_KEY   = 2,
} sli_zigbee_get_key_bitmask;

void sli_zigbee_request_key_timeout_control(bool start, bool useBdbTimeoutValues);

#define sli_zigbee_start_request_key_timeout(useBdbTimeoutValues) \
  (sli_zigbee_request_key_timeout_control(true, useBdbTimeoutValues))
#define sli_zigbee_stop_request_key_timeout() \
  (sli_zigbee_request_key_timeout_control(false, false))

#define sli_zigbee_reset_incoming_tc_frame_counter() \
  (sli_zigbee_incoming_tc_link_key_frame_counter = 0)

void sli_zigbee_set_partner_key_update_timer(bool start, uint32_t timeout_ms);
sl_status_t sli_zigbee_stack_terminate_app_link_key_request(void);

//----------------------------------------------------------------

void sli_zigbee_aps_keys_init(void);

sl_status_t sli_zigbee_set_trust_center_link_key(sl_zigbee_key_data_t* keyData);

sl_status_t sli_zigbee_update_trust_center_link_key(sl_zigbee_key_data_t* keyData,
                                                    bool keyIsAuthorized);

sl_status_t sli_zigbee_get_link_key(sl_802154_long_addr_t partner,
                                    sl_zigbee_key_data_t *key,
                                    sl_zigbee_sec_man_context_t *context_in,
                                    uint32_t **frameCounterLocLoc,
                                    sli_zigbee_get_key_bitmask getKeyBitmask);

// Test function to set the frame counter to one more than the passed value.
void sli_zigbee_update_link_key_frame_counter(sl_802154_long_addr_t partner, uint32_t frameCounter);

bool sli_zigbee_have_trust_center_link_key(void);

void sli_zigbee_set_trust_center_data(uint32_t clearFlags,
                                      uint32_t setFlags,
                                      sl_802154_long_addr_t eui64,
                                      sl_zigbee_key_data_t* key);
void sli_zigbee_modify_security_state(uint32_t clearFlags, uint32_t setFlags);
#define sli_zigbee_clear_trust_center_eui64() \
  (sli_zigbee_modify_security_state(SL_ZIGBEE_HAVE_TRUST_CENTER_EUI64, 0))
bool sli_zigbee_get_security_token_data(sl_zigbee_initial_security_state_t* state);
void sli_zigbee_get_preconfigured_key(sl_zigbee_key_data_t *result);
void sli_zigbee_write_security_token(void);
void sli_zigbee_read_security_token(void);

bool sli_zigbee_is_token_data_initialized(uint8_t* data, uint8_t length);

//----------------------------------------------------------------
// Key requests

sl_status_t sli_zigbee_request_link_key(sl_802154_short_addr_t trustCenter, sl_802154_long_addr_t partner);
void sli_zigbee_request_key_event_handler(sli_zigbee_event_t *event);

void sli_zigbee_set_bdb_tc_link_key_exchange_attempts(uint8_t attempts);
uint8_t sli_zigbee_get_update_tc_link_key_state(void);
void sli_zigbee_set_update_tc_link_key_state(uint8_t state);
void sli_zigbee_request_key_process_node_descriptor_response(sl_802154_short_addr_t sender,
                                                             sli_zigbee_packet_header_t header);
uint8_t sli_zigbee_get_update_app_link_key_state();
void sli_zigbee_set_update_app_link_key_state(uint8_t state);

void sli_zigbee_partner_key_update_event_handler(sli_zigbee_event_t *event);
void sli_zigbee_partner_link_key_get_device(sl_802154_long_addr_t partner);
void sli_zigbee_partner_link_key_set_device(sl_802154_long_addr_t partner);
#endif // SILABS_APS_KEYS_H
