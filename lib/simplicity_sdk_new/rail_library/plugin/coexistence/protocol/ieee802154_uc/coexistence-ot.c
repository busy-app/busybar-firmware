/***************************************************************************//**
 * @file
 * @brief Openthread Specific Radio Coexistence callbacks.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "coexistence/protocol/ieee802154_uc/coexistence-ot.h"
#include "coexistence/protocol/ieee802154_uc/coexistence-common-802154.h"
#include "sl_common.h"

COEX_Events_t sli_rail_util_ot_coex_state_event_filter = COEX_EVENT_NONE;

#if COEX_STACK_EVENT_SUPPORT
void sli_rail_util_ot_coex_state_on_event(COEX_Events_t events)
{
  static sl_rail_util_coex_ot_event_t tx_Req;
  static sl_rail_util_coex_ot_event_t rx_Req;
  sl_rail_util_coex_ot_event_t ot_event = SL_RAIL_UTIL_COEX_OT_EVENT_NONE;
  if ((events & COEX_EVENT_TX_STATE) != 0U) {
    tx_Req = SL_RAIL_UTIL_COEX_OT_TX_REQUEST;
    rx_Req = SL_RAIL_UTIL_COEX_OT_EVENT_NONE;
  } else if ((events & COEX_EVENT_RX_STATE) != 0U) {
    tx_Req = SL_RAIL_UTIL_COEX_OT_EVENT_NONE;
    rx_Req = SL_RAIL_UTIL_COEX_OT_RX_REQUEST;
  } else if ((events & COEX_EVENT_REQUEST_ASSERTED) != 0U) {
    ot_event = SL_RAIL_UTIL_COEX_OT_EVENT_REQUESTED;
    if ((events & COEX_EVENT_GRANT_ASSERTED) != 0U) {
      sl_rail_util_coex_ot_events(ot_event | tx_Req | rx_Req);
      ot_event = SL_RAIL_UTIL_COEX_OT_EVENT_GRANTED_IMMEDIATE;
    }
  } else if ((events & COEX_EVENT_GRANT_ASSERTED) != 0U) {
    ot_event = SL_RAIL_UTIL_COEX_OT_EVENT_GRANTED;
  } else if ((events & COEX_EVENT_REQUEST_DENIED) != 0U) {
    ot_event = SL_RAIL_UTIL_COEX_OT_EVENT_DENIED;
  } else if ((events & COEX_EVENT_TX_ABORTED) != 0U) {
    ot_event = SL_RAIL_UTIL_COEX_OT_EVENT_GRANT_ABORTED;
  } else {
  }
  if (ot_event != SL_RAIL_UTIL_COEX_OT_EVENT_NONE) {
    sl_rail_util_coex_ot_events(ot_event | tx_Req | rx_Req);
  }
}

void sl_rail_util_ot_enable_coex_state_event_filter(void)
{
  sli_rail_util_ot_coex_state_event_filter |= COEX_EVENT_STATE_EVENTS;
}

void sl_rail_util_ot_disable_coex_state_event_filter(void)
{
  sli_rail_util_ot_coex_state_event_filter &= ~COEX_EVENT_STATE_EVENTS;
}

SL_WEAK void sl_rail_util_coex_ot_events(sl_rail_util_coex_ot_event_t event)
{
  (void)event;
}

#else //!COEX_STACK_EVENT_SUPPORT

void sli_rail_util_ot_coex_state_on_event(COEX_Events_t events)
{
  (void)events;
}

#endif //COEX_STACK_EVENT_SUPPORT
