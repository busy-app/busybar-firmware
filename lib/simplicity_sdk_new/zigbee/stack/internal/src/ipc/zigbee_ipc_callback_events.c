/***************************************************************************//**
 * @file zigbee_ipc_callback_events.c
 * @brief global dispatch for 'module' specific callback event handlers
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
// automatically generated from stack headers.  Do not manually edit
#include "stack/internal/src/ipc/zigbee_ipc_callback_events.h"
#include "stack/internal/src/ipc/binding-table-ipc-callback-events.h"
#include "stack/internal/src/ipc/bootload_ipc_callback_events.h"
#ifdef SL_CATALOG_ZIGBEE_CBKE_CORE_PRESENT
#include "stack/internal/src/ipc/cbke-crypto-engine-ipc-callback-events.h"
#endif
#include "stack/internal/src/ipc/child_ipc_callback_events.h"
#ifdef SL_CATALOG_HIGH_DATARATE_PHY_PRESENT
#include "stack/internal/src/ipc/high_datarate_phy_stack_interface_ipc_callback_events.h"
#endif
#include "stack/internal/src/ipc/message_ipc_callback_events.h"
#include "stack/internal/src/ipc/network-formation-ipc-callback-events.h"
#include "stack/internal/src/ipc/raw-message-ipc-callback-events.h"
#include "stack/internal/src/ipc/security_ipc_callback_events.h"
#include "stack/internal/src/ipc/sl_zigbee_duty_cycle_ipc_callback_events.h"
#ifdef SL_CATALOG_ZIGBEE_R23_SUPPORT_PRESENT
#include "stack/internal/src/ipc/sl_zigbee_dynamic_commissioning_ipc_callback_events.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_R23_SUPPORT_PRESENT
#include "stack/internal/src/ipc/sl_zigbee_zdo_management_ipc_callback_events.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_R23_SUPPORT_PRESENT
#include "stack/internal/src/ipc/sl_zigbee_zdo_security_ipc_callback_events.h"
#endif
#include "stack/internal/src/ipc/stack-info-ipc-callback-events.h"
#include "stack/internal/src/ipc/trust-center-ipc-callback-events.h"
#ifdef SL_CATALOG_ZIGBEE_LIGHT_LINK_PRESENT
#include "stack/internal/src/ipc/zll-api-ipc-callback-events.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_MAC_TEST_COMMANDS_SUPPORT_PRESENT
#include "stack/internal/src/ipc/sl_zigbee_mac_app_events_ipc_callback_events.h"
#endif

typedef void (*callback_dispatcher)(sl_zigbee_stack_cb_event_t *cb_event);

static const callback_dispatcher stack_callbacks[] = {
  sli_zigbee_binding_table_process_ipc_event,
  sli_zigbee_bootload_process_ipc_event,
  sli_zigbee_cbke_crypto_engine_process_ipc_event,
  sli_zigbee_child_process_ipc_event,
  sli_zigbee_high_datarate_phy_stack_interface_process_ipc_event,
  sli_zigbee_message_process_ipc_event,
  sli_zigbee_network_formation_process_ipc_event,
  sli_zigbee_raw_message_process_ipc_event,
  sli_zigbee_security_process_ipc_event,
  sli_zigbee_sl_zigbee_duty_cycle_process_ipc_event,
  sli_zigbee_sl_zigbee_dynamic_commissioning_process_ipc_event,
  sli_zigbee_sl_zigbee_zdo_management_process_ipc_event,
  sli_zigbee_sl_zigbee_zdo_security_process_ipc_event,
  sli_zigbee_stack_info_process_ipc_event,
  sli_zigbee_trust_center_process_ipc_event,
  sli_zigbee_zll_api_process_ipc_event,
  sli_zigbee_sl_zigbee_mac_app_events_process_ipc_event,
  NULL
};

void sli_zigbee_process_stack_callbacks_event(sl_event_t *sys_event)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) sys_event->event_data;
  for (int i = 0; stack_callbacks[i] != NULL; i++) {
    stack_callbacks[i](cb_event);
  }
}

sl_event_publisher_t sli_zigbee_ipc_publisher;
