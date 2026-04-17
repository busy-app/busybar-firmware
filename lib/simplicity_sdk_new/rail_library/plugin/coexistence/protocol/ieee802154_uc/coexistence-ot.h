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

#ifndef __COEXISTENCE_OT_H__
#define __COEXISTENCE_OT_H__

#include "coexistence/common/coexistence.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#ifdef SL_CATALOG_RAIL_UTIL_COEX_PRESENT
#include "sl_rail_util_coex_config.h"
#endif // SL_CATALOG_RAIL_UTIL_COEX_PRESENT

/**
 * @enum sl_rail_util_coex_state_event_t
 *
 * Openthread events triggered to provide the state changes in different COEX GPIOs
 *
 * These events are passed to sl_rail_util_coex_ot_events.
 */
typedef enum {
  SL_RAIL_UTIL_COEX_OT_EVENT_NONE = 0,
  /** Request assert initiated */
  SL_RAIL_UTIL_COEX_OT_EVENT_REQUESTED = 1,
  /** Request denied */
  SL_RAIL_UTIL_COEX_OT_EVENT_DENIED = 2,
  /** Grant asserted */
  SL_RAIL_UTIL_COEX_OT_EVENT_GRANTED = 3,
  /** Grant asserted immediately*/
  SL_RAIL_UTIL_COEX_OT_EVENT_GRANTED_IMMEDIATE = 4,
  /** Grant aborted mid packet */
  SL_RAIL_UTIL_COEX_OT_EVENT_GRANT_ABORTED = 5,
  /** Tx request - OR'd as a mask with the other values */
  SL_RAIL_UTIL_COEX_OT_TX_REQUEST = (1 << 5),
  /** Rx request - OR'd as a mask with the other values */
  SL_RAIL_UTIL_COEX_OT_RX_REQUEST = (1 << 6),
} sl_rail_util_coex_ot_event_t;

void sl_rail_util_coex_ot_events(sl_rail_util_coex_ot_event_t event);

// OT coexistence event callback
void sli_rail_util_ot_coex_state_on_event(COEX_Events_t events);

// OT COEX state event filter
extern COEX_Events_t sli_rail_util_ot_coex_state_event_filter;

// Enable OT COEX state event filter
void sl_rail_util_ot_enable_coex_state_event_filter(void);

// Disable OT COEX state event filter
void sl_rail_util_ot_disable_coex_state_event_filter(void);

#endif //__COEXISTENCE_OT_H__
