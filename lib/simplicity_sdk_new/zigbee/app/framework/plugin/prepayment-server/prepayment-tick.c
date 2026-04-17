/***************************************************************************//**
 * @file
 * @brief Implemented routines for managing prepayment tick.
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

#include "app/framework/include/af.h"
#include "prepayment-tick.h"
#include "prepayment-debt-schedule.h"
#include "prepayment-modes-table.h"

#include "prepayment-server-config.h"

static uint16_t PendingPrepaymentEvents;

void sl_zigbee_af_prepayment_cluster_clear_pending_event(uint16_t event)
{
  PendingPrepaymentEvents &= ~(event);
}

void sl_zigbee_af_prepayment_cluster_schedule_tick_cb(uint8_t endpoint, uint16_t event)
{
  PendingPrepaymentEvents |= event;

  // This call will execute any ready events and schedule the tick for the nearest future event.
  sl_zigbee_af_prepayment_cluster_server_tick_cb(endpoint);
}

void sl_zigbee_af_prepayment_cluster_server_tick_cb(uint8_t endpoint)
{
  uint32_t timeNowUtc;
  uint32_t minEventDelaySec = 0xFFFFFFFF;
  uint32_t eventDelaySec;
  uint16_t minTimeEvent = 0;

  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PREPAYMENT_CLUSTER_ID);
  sl_zigbee_af_prepayment_cluster_println("Prepayment Tick Callback, epx=%d", ep);
  if ( ep == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX ) {
    return;
  }

  timeNowUtc = sl_zigbee_af_get_current_time();

  // Look at all currently pending events and determine the min delay time for each.
  // If any are less than nextEventTimeSec, update nextEventTimeSec.

  if ( PendingPrepaymentEvents & PREPAYMENT_TICK_CHANGE_DEBT_EVENT ) {
    eventDelaySec = sl_zigbee_af_prepayment_server_seconds_until_debt_collection_event(timeNowUtc);
    if ( eventDelaySec == 0 ) {
      // Execute now
      sl_zigbee_af_prepayment_cluster_println("Prepayment Tick:  Handle Debt Collection");
      sl_zigbee_af_prepayment_server_set_debt_mode(endpoint, timeNowUtc);
      // Recalculate next delay time
      eventDelaySec = sl_zigbee_af_prepayment_server_seconds_until_debt_collection_event(timeNowUtc);
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEvent = PREPAYMENT_TICK_CHANGE_PAYMENT_MODE_EVENT;
    }
  }

  if ( PendingPrepaymentEvents & PREPAYMENT_TICK_CHANGE_PAYMENT_MODE_EVENT ) {
    eventDelaySec = sl_zigbee_af_prepayment_server_seconds_until_payment_mode_event(timeNowUtc);
    if ( eventDelaySec == 0 ) {
      // Execute now
      sl_zigbee_af_prepayment_cluster_println("Prepayment Tick:  Handle Payment Mode");
      sl_zigbee_af_prepayment_server_set_payment_mode(endpoint);
      // Recalculate next delay time
      eventDelaySec = sl_zigbee_af_prepayment_server_seconds_until_payment_mode_event(timeNowUtc);
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEvent = PREPAYMENT_TICK_CHANGE_PAYMENT_MODE_EVENT;
    }
  }
  sl_zigbee_af_prepayment_cluster_println("Scheduling Tick Callback in %d sec, eventBitFlag=%d, pendingEvents=%d", minEventDelaySec, minTimeEvent, PendingPrepaymentEvents);
  sl_zigbee_zcl_schedule_server_tick(endpoint, ZCL_PREPAYMENT_CLUSTER_ID, (minEventDelaySec * MILLISECOND_TICKS_PER_SECOND) );
}
