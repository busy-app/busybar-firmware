/***************************************************************************//**
 * @file
 * @brief Host EZSP layer. Provides functions that allow the Host
 * application to send every EZSP command to the NCP. The command and response
 * parameters are defined in the datasheet.
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

#ifndef SILABS_EZSP_H
#define SILABS_EZSP_H

// Reset the NCP and initialize the serial protocol (SPI or UART). After this
// function returns SL_ZIGBEE_EZSP_SUCCESS, the NCP has finished rebooting and is ready
// to accept a command.
sl_zigbee_ezsp_status_t sl_zigbee_ezsp_init(void);

// For ezsp-uart, must be called before setting sleep mode and enabling
// synchronous callbacks (read via sl_zigbee_ezsp_callback()).
void sl_zigbee_ezsp_enable_ncp_sleep(bool enable);

// Returns true if there are one or more callbacks queued up on the NCP
// awaiting collection.
bool sl_zigbee_ezsp_callback_pending(void);

// The sleep mode to use in the frame control of every command sent. The Host
// application can set this to the desired NCP sleep mode. Subsequent commands
// will pass this value to the NCP.
extern uint8_t ezspSleepMode;

// Wakes the NCP up from deep sleep.
void sl_zigbee_ezsp_wake_up(void);

// For ezsp-uart, indicate whether the Host can sleep without disrupting the
// uart protocol. In particular the Host may not sleep unless the NCP is
// already asleep.
bool sl_zigbee_ezsp_ok_to_sleep(void);

// The Host application must call this function periodically to allow the EZSP
// layer to handle asynchronous events.
void sl_zigbee_ezsp_tick(void);

// The EZSP layer calls this function after sending a command while waiting for
// the response. The Host application can use this function to perform any tasks
// that do not involve the NCP.
void sl_zigbee_ezsp_waiting_for_response(void);

// Callback from the EZSP layer indicating that the transaction with the NCP
// could not be completed due to a serial protocol error or that the response
// received from the NCP reported an error. The status parameter provides more
// information about the error. This function must be provided by the Host
// application.
void sl_zigbee_ezsp_error_handler(sl_zigbee_ezsp_status_t status);

// Cleanly close down the serial protocol (SPI or UART). After this function has
// been called, sl_zigbee_ezsp_init() must be called to resume communication with the
// NCP.
void sl_zigbee_ezsp_close(void);

//----------------------------------------------------------------
// Functions with special handling

sl_status_t sl_zigbee_ezsp_get_version_struct(sl_zigbee_version_t* returnData);

sl_status_t sl_zigbee_ezsp_set_endpoint_flags(uint8_t endpoint,
                                              sl_zigbee_ezsp_endpoint_flags_t flags);
sl_status_t sl_zigbee_ezsp_get_endpoint_flags(uint8_t endpoint,
                                              sl_zigbee_ezsp_endpoint_flags_t* returnFlags);

uint8_t sl_zigbee_ezsp_get_source_route_overhead(sl_802154_short_addr_t destination);

#define sl_zigbee_ezsp_enable_endpoint(endpoint) \
  sl_zigbee_ezsp_set_endpoint_flags((endpoint), SL_ZIGBEE_EZSP_ENDPOINT_ENABLED)

bool sl_zigbee_is_performing_rejoin(void);
sl_status_t sl_zigbee_set_mfg_security_config(uint32_t magicNumber,
                                              const sl_zigbee_mfg_security_struct_t* settings);
sl_status_t sl_zigbee_get_mfg_security_config(sl_zigbee_mfg_security_struct_t* settings);
sl_status_t sl_zigbee_set_child_timeout_option_mask(uint16_t mask);
sl_status_t sl_zigbee_set_keep_alive_mode(uint8_t mode);
sl_zigbee_keep_alive_mode_t sl_zigbee_get_keep_alive_mode(void);
uint8_t sl_zigbee_get_network_key_timeout(void);
sl_status_t sl_zigbee_start_writing_stack_tokens(void);
sl_status_t sl_zigbee_stop_writing_stack_tokens(void);
sl_status_t sl_zigbee_set_extended_security_bitmask(sl_zigbee_extended_security_bitmask_t mask);
sl_status_t sl_zigbee_get_extended_security_bitmask(sl_zigbee_extended_security_bitmask_t* mask);
sl_status_t sl_zigbee_set_node_id(sl_802154_short_addr_t nodeId);
uint8_t sl_zigbee_get_max_nwk_retries(void);
uint8_t sl_zigbee_get_last_stack_zig_dev_request_sequence(void);
sl_status_t sl_zigbee_send_unicast_network_key_update(sl_802154_short_addr_t targetShort,
                                                      sl_802154_long_addr_t  targetLong,
                                                      const sl_zigbee_key_data_t* newKey);
sl_status_t sl_zigbee_aes_hash_simple(uint8_t totalLength,
                                      const uint8_t* data,
                                      uint8_t* result);
uint8_t sl_zigbee_get_current_network(void);
sl_status_t sl_zigbee_set_current_network(uint8_t index);
uint8_t sl_zigbee_get_callback_network(void);

sl_status_t sl_zigbee_find_and_rejoin_network(bool haveCurrentNetworkKey,
                                              uint32_t channelMask,
                                              sl_zigbee_rejoin_reason_t reason,
                                              sl_zigbee_node_type_t nodeType);

sl_zigbee_rejoin_reason_t sl_zigbee_get_last_rejoin_reason(void);
sl_zigbee_leave_reason_t sl_zigbee_get_last_leave_reason(sl_802154_short_addr_t* id);
uint8_t getSourceRouteOverhead(uint8_t messageLength);
sl_status_t sl_zigbee_set_preinstalled_cbke_data_283k1(sl_zigbee_public_key_283k1_data_t *caPublic,
                                                       sl_zigbee_certificate_283k1_data_t *myCert,
                                                       sl_zigbee_private_key_283k1_data_t *myKey);

#define sl_zigbee_is_node_id_valid(nodeId) ((nodeId) < SL_ZIGBEE_DISCOVERY_ACTIVE_NODE_ID)


//----------------------------------------------------------------
// ZLL methods

void sl_zigbee_zll_get_token_stack_zll_data(sl_zigbee_tok_type_stack_zll_data_t *token);
void sl_zigbee_zll_get_token_stack_zll_security(sl_zigbee_tok_type_stack_zll_security_t *token);
sl_status_t sl_zigbee_zll_form_network(sl_zigbee_zll_network_t* networkInfo,
                                       int8_t radioTxPower);
sl_status_t sl_zigbee_zll_join_target(const sl_zigbee_zll_network_t* targetNetworkInfo);

//----------------------------------------------------------------
#include "rename-ezsp-functions.h"
#include "command-prototypes.h"
#endif // __EZSP_H__
