/***************************************************************************//**
 * @brief Functionality for rolling the network key by broadcasting the key update
 * and the NWK key switch messages.
 * This is the preferred mechanism for Home Automation and another networks
 * without individual link keys per device.
 *
 * The process is pretty straightforward.
 *   1. Broadcast new NWK key (randomly generated)
 *   2. Wait until the broadcast fades out of the network (~20 seconds)
 *   3. Broadcast NWK key switch message
 *
 * Note: This does not PERIODICALLY update the NWK key.  It just manages the
 * process when it is told to initate the key change.  Another software
 * module must determine the policy for periodically initiating this process
 * (such as the 'Trust Center NWK Key Update Periodic' App Framework Plugin).
 *
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories, Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of  Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software  is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/util/util.h"

#include "trust-center-nwk-key-update-broadcast.h"
#include "stack/include/zigbee-security-manager.h"

// *****************************************************************************
// Globals

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
sl_zigbee_af_event_t sl_zigbee_af_trust_center_nwk_key_update_broadcast_my_event;
#define myEvent (&sl_zigbee_af_trust_center_nwk_key_update_broadcast_my_event)
void sl_zigbee_af_trust_center_nwk_key_update_broadcast_my_event_handler(sl_zigbee_af_event_t * event);

#if defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT)
extern sl_status_t sl_zigbee_af_trust_center_start_broadcast_network_key_update(void);
// For testing, we need to support a single application that can do
// unicast AND broadcast key updates.  So we re-map the function name
// so both can be compiled in without conflict.
#define sl_zigbee_af_trust_center_start_network_key_update sl_zigbee_af_trust_center_start_broadcast_network_key_update
#endif

// *****************************************************************************

void sl_zigbee_af_trust_center_nwk_key_update_broadcast_my_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sl_status_t status;
  sl_zigbee_af_event_set_inactive(myEvent);

  status = sl_zigbee_broadcast_network_key_switch();
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_security_println("Failed to broadcast NWK key switch");
  }
  sl_zigbee_af_security_println("Sent NWK key switch.");
  sl_zigbee_af_network_key_update_complete_cb(status);
}

static bool nextNetworkKeyIsNewer(sl_zigbee_key_struct_t* nextNwkKey)
{
  sl_status_t status;
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_key_t plaintext_key;
  sl_zigbee_sec_man_network_key_info_t key_info;

  sl_zigbee_sec_man_init_context(&context);
  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_NETWORK;
  context.key_index = 1;

  status = sl_zigbee_sec_man_export_key(&context, &plaintext_key);
  memmove(&(nextNwkKey->key),
          &plaintext_key.key,
          SL_ZIGBEE_ENCRYPTION_KEY_SIZE);

  // It is assumed that the current nwk key has valid data.
  (void)sl_zigbee_sec_man_get_network_key_info(&key_info);
  if (status != SL_STATUS_OK
      || (timeGTorEqualInt8u(key_info.network_key_sequence_number,
                             key_info.alt_network_key_sequence_number))) {
    return false;
  }

  return true;
}

sl_status_t sl_zigbee_af_trust_center_start_network_key_update(void)
{
  sl_zigbee_key_struct_t nextNwkKey;
  sl_status_t status;

  if (sl_zigbee_af_get_node_id() != SL_ZIGBEE_TRUST_CENTER_NODE_ID
      || sl_zigbee_af_network_state() != SL_ZIGBEE_JOINED_NETWORK
      || sl_zigbee_af_event_is_scheduled(myEvent)
      ) {
    return SL_STATUS_INVALID_STATE;
  }

  if (!nextNetworkKeyIsNewer(&nextNwkKey)) {
    // Setting the key to all zeroes tells the stack
    // to randomly generate a new key and use that.
    memset(sl_zigbee_key_contents(&(nextNwkKey.key)),
           0,
           SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
  }

  status = sl_zigbee_broadcast_next_network_key(&(nextNwkKey.key));
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_security_println("Failed to broadcast next NWK key");
  } else {
    sl_zigbee_af_security_println("Broadcasting next NWK key");
    // The +2 is a fuzz factor
    sl_zigbee_af_event_set_delay_qs(myEvent,
                                    SL_ZIGBEE_BROADCAST_TABLE_TIMEOUT_QS + 2);
  }
  return status;
}

void sli_zigbee_af_trust_center_nwk_key_update_broadcast_init_callback(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_event_init(myEvent,
                          sl_zigbee_af_trust_center_nwk_key_update_broadcast_my_event_handler);
}
