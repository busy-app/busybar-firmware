/***************************************************************************//**
 * @file network-formation-baremetal-wrapper.c
 * @brief internal implementations for 'network-formation' as a thin-wrapper
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
// automatically generated from network-formation.h.  Do not manually edit
#include "stack/include/network-formation.h"
#include "stack/internal/inc/network-formation-internal-def.h"

sl_status_t sl_zigbee_clear_stored_beacons(void)
{
  return sli_zigbee_stack_clear_stored_beacons();
}

void sl_zigbee_delayed_join_activate(bool activated)
{
  sli_zigbee_stack_delayed_join_activate(activated);
}

bool sl_zigbee_delayed_join_is_activated(void)
{
  return sli_zigbee_stack_delayed_join_is_activated();
}

sl_status_t sl_zigbee_find_and_rejoin_network(bool haveCurrentNetworkKey,
                                              uint32_t channelMask,
                                              sl_zigbee_rejoin_reason_t reason,
                                              sl_zigbee_node_type_t nodeType)
{
  return sli_zigbee_stack_find_and_rejoin_network(haveCurrentNetworkKey,
                                                  channelMask,
                                                  reason,
                                                  nodeType);
}

sl_status_t sl_zigbee_find_unused_pan_id(uint32_t channelMask,
                                         uint8_t duration)
{
  return sli_zigbee_stack_find_unused_pan_id(channelMask,
                                             duration);
}

sl_status_t sl_zigbee_form_network(sl_zigbee_network_parameters_t *parameters)
{
  return sli_zigbee_stack_form_network(parameters);
}

sl_status_t sl_zigbee_get_beacon_classification_params(sl_zigbee_beacon_classification_params_t *param)
{
  return sli_zigbee_stack_get_beacon_classification_params(param);
}

bool sl_zigbee_get_join_uses_install_code(void)
{
  return sli_zigbee_stack_get_join_uses_install_code();
}

sl_zigbee_leave_reason_t sl_zigbee_get_last_leave_reason(sl_802154_short_addr_t *returnNodeIdThatSentLeave)
{
  return sli_zigbee_stack_get_last_leave_reason(returnNodeIdThatSentLeave);
}

sl_zigbee_rejoin_reason_t sl_zigbee_get_last_rejoin_reason(void)
{
  return sli_zigbee_stack_get_last_rejoin_reason();
}

uint8_t sl_zigbee_get_network_key_timeout(void)
{
  return sli_zigbee_stack_get_network_key_timeout();
}

uint8_t sl_zigbee_get_num_stored_beacons(void)
{
  return sli_zigbee_stack_get_num_stored_beacons();
}

bool sl_zigbee_get_permit_joining(void)
{
  return sli_zigbee_stack_get_permit_joining();
}

sl_status_t sl_zigbee_get_stored_beacon(uint8_t beacon_number,
                                        sl_zigbee_beacon_data_t *beacon)
{
  return sli_zigbee_stack_get_stored_beacon(beacon_number,
                                            beacon);
}

sl_status_t sl_zigbee_init(void)
{
  return sli_zigbee_stack_init();
}

bool sl_zigbee_is_performing_rejoin(void)
{
  return sli_zigbee_stack_is_performing_rejoin();
}

sl_status_t sl_zigbee_join_network(sl_zigbee_node_type_t nodeType,
                                   sl_zigbee_network_parameters_t *parameters)
{
  return sli_zigbee_stack_join_network(nodeType,
                                       parameters);
}

sl_status_t sl_zigbee_join_network_directly(sl_zigbee_node_type_t localNodeType,
                                            sl_zigbee_beacon_data_t *beacon,
                                            int8_t radioTxPower,
                                            bool clearBeaconsAfterNetworkUp)
{
  return sli_zigbee_stack_join_network_directly(localNodeType,
                                                beacon,
                                                radioTxPower,
                                                clearBeaconsAfterNetworkUp);
}

sl_status_t sl_zigbee_leave_network(sl_zigbee_leave_network_option_t options)
{
  return sli_zigbee_stack_leave_network(options);
}

sl_status_t sl_zigbee_network_init(sl_zigbee_network_init_struct_t *networkInitStruct)
{
  return sli_zigbee_stack_network_init(networkInitStruct);
}

sl_status_t sl_zigbee_permit_joining(uint8_t duration)
{
  return sli_zigbee_stack_permit_joining(duration);
}

sl_status_t sl_zigbee_reschedule_link_status_msg(void)
{
  return sli_zigbee_stack_reschedule_link_status_msg();
}

bool sl_zigbee_send_pan_id_update(sl_802154_pan_id_t newPan)
{
  return sli_zigbee_stack_send_pan_id_update(newPan);
}

sl_status_t sl_zigbee_send_zigbee_leave(sl_802154_short_addr_t destination,
                                        sl_zigbee_leave_request_flags_t flags)
{
  return sli_zigbee_stack_send_zigbee_leave(destination,
                                            flags);
}

sl_status_t sl_zigbee_set_beacon_classification_params(sl_zigbee_beacon_classification_params_t *param)
{
  return sli_zigbee_stack_set_beacon_classification_params(param);
}

sl_status_t sl_zigbee_set_beacon_jitter_duration(uint8_t beaconJitterDuration)
{
  return sli_zigbee_stack_set_beacon_jitter_duration(beaconJitterDuration);
}

void sl_zigbee_set_join_uses_install_code(bool enable)
{
  sli_zigbee_stack_set_join_uses_install_code(enable);
}

sl_status_t sl_zigbee_set_num_beacons_to_store(uint8_t numBeacons)
{
  return sli_zigbee_stack_set_num_beacons_to_store(numBeacons);
}

sl_status_t sl_zigbee_set_nwk_update_id(uint8_t nwkUpdateId,
                                        bool set_when_on_network)
{
  return sli_zigbee_stack_set_nwk_update_id(nwkUpdateId,
                                            set_when_on_network);
}

void sl_zigbee_set_pending_network_update_pan_id(uint16_t panId)
{
  sli_zigbee_stack_set_pending_network_update_pan_id(panId);
}

sl_status_t sl_zigbee_set_tc_rejoins_using_well_known_key_allowed(bool allow)
{
  return sli_zigbee_stack_set_tc_rejoins_using_well_known_key_allowed(allow);
}

sl_status_t sl_zigbee_set_tc_rejoins_using_well_known_key_timeout_sec(uint16_t timeout_sec)
{
  return sli_zigbee_stack_set_tc_rejoins_using_well_known_key_timeout_sec(timeout_sec);
}

sl_status_t sl_zigbee_setup_delayed_join(uint8_t networkKeyTimeoutS)
{
  return sli_zigbee_stack_setup_delayed_join(networkKeyTimeoutS);
}

sl_status_t sl_zigbee_sleepy_to_sleepy_network_start(sl_zigbee_network_parameters_t *parameters,
                                                     bool initiator)
{
  return sli_zigbee_stack_sleepy_to_sleepy_network_start(parameters,
                                                         initiator);
}

sl_status_t sl_zigbee_start_scan(sl_zigbee_network_scan_type_t scanType,
                                 uint32_t channelMask,
                                 uint8_t duration)
{
  return sli_zigbee_stack_start_scan(scanType,
                                     channelMask,
                                     duration);
}

sl_status_t sl_zigbee_stop_scan(void)
{
  return sli_zigbee_stack_stop_scan();
}

sl_status_t sl_zigbee_survey_beacons(bool useStandardBeacons,
                                     uint32_t channel_mask,
                                     sl_zigbee_network_found_callback_t networkFoundCallback,
                                     sl_zigbee_scan_complete_callback_t scanCompleteCallback)
{
  return sli_zigbee_stack_survey_beacons(useStandardBeacons,
                                         channel_mask,
                                         networkFoundCallback,
                                         scanCompleteCallback);
}

bool sl_zigbee_tc_rejoins_using_well_known_key_allowed(void)
{
  return sli_zigbee_stack_tc_rejoins_using_well_known_key_allowed();
}

void slx_zigbee_change_pan_id_now(sl_802154_pan_id_t panId)
{
  slxi_zigbee_stack_change_pan_id_now(panId);
}
