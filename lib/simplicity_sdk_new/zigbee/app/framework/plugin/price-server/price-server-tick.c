/***************************************************************************//**
 * @file
 * @brief Implemented routines for managing price tick.
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
#include "app/framework/util/af-event.h"
#include "price-server-tick.h"
#include "price-server.h"

bool sli_zigbee_af_price_server_billing_period_repeat = true;

static sl_zigbee_af_price_server_pending_events_t PendingPriceEvents;

void sl_zigbee_af_price_cluster_server_init_tick(void)
{
  PendingPriceEvents = SL_ZIGBEE_AF_PRICE_SERVER_NO_PRICES_EVENT_MASK;
}

void sl_zigbee_af_price_cluster_clear_pending_event(sl_zigbee_af_price_server_pending_events_t event)
{
  PendingPriceEvents &= ~(event);
}

void sl_zigbee_af_price_cluster_server_tick_cb(uint8_t endpoint)
{
  uint32_t timeNowUtc;
  uint32_t minEventDelaySec = ZCL_PRICE_CLUSTER_END_TIME_NEVER;
  uint32_t eventDelaySec;
  uint16_t minTimeEventMask = 0;
  uint32_t delayMSec;

  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_PRICE_CLUSTER_ID);
  sl_zigbee_af_price_cluster_println("Price Tick Callback, ep=%d", ep);
  if ( ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    return;
  }

  timeNowUtc = sl_zigbee_af_get_current_time();
  UNUSED_VAR(timeNowUtc);
  UNUSED_VAR(minTimeEventMask);

  // Look at all currently pending events and determine the min delay time for each.
  if ((PendingPriceEvents & SL_ZIGBEE_AF_PRICE_SERVER_GET_SCHEDULED_PRICES_EVENT_MASK) != 0U) {
    eventDelaySec = sl_zigbee_af_price_server_seconds_until_get_scheduled_prices_event();
    if ( eventDelaySec == 0u ) {
      // Execute now
      sl_zigbee_af_price_cluster_println("Price Tick:  Get Scheduled Prices");
      sl_zigbee_af_price_server_send_get_scheduled_prices(endpoint);
      // Recalculate next delay time
      eventDelaySec = sl_zigbee_af_price_server_seconds_until_get_scheduled_prices_event();
    }
    if ( eventDelaySec <= minEventDelaySec ) { // This should preempt any other events.
      minEventDelaySec = eventDelaySec;
      minTimeEventMask = SL_ZIGBEE_AF_PRICE_SERVER_GET_SCHEDULED_PRICES_EVENT_MASK;
    }
    if ( eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS ) {
      sl_zigbee_af_price_cluster_clear_pending_event(SL_ZIGBEE_AF_PRICE_SERVER_GET_SCHEDULED_PRICES_EVENT_MASK);
    }
  }

  if ((PendingPriceEvents & SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_BILLING_PERIOD_EVENT_MASK) != 0U) {
    eventDelaySec = sl_zigbee_af_price_server_seconds_until_billing_period_event(endpoint);
    if ( eventDelaySec == 0u ) {
      // Execute now
      sl_zigbee_af_price_cluster_println("Price Tick:  Billing Period");
      sl_zigbee_af_price_server_refresh_billing_period(endpoint, false);   // force?
      // Recalculate next delay time
      eventDelaySec = sl_zigbee_af_price_server_seconds_until_billing_period_event(endpoint);
    }
    if (eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS) {
      // There is no next billing event, so we need to schedule one with the
      // same duration as the current event. See Billing Period Duration
      // description in D.4.2.4.10.2 of the SE spec.
      if (sli_zigbee_af_price_server_billing_period_repeat) {
        sl_zigbee_af_price_server_refresh_billing_period(endpoint, true); // force?
        // Recalculate next delay time
        eventDelaySec = sl_zigbee_af_price_server_seconds_until_billing_period_event(endpoint);
      } else {
        sl_zigbee_af_price_cluster_clear_pending_event(SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_BILLING_PERIOD_EVENT_MASK);
      }
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEventMask = SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_BILLING_PERIOD_EVENT_MASK;
    }
  }

  if ((PendingPriceEvents & SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_BLOCK_PERIOD_EVENT_MASK) != 0U) {
    eventDelaySec = sl_zigbee_af_price_server_seconds_until_block_period_event(endpoint);
    if ( eventDelaySec == 0u ) {
      // Execute now
      sl_zigbee_af_price_cluster_println("Price Tick:  Block Period");
      sl_zigbee_af_price_server_refresh_block_period(endpoint, false);   // repeat?

      // Per SE1.2 Spec D4.4.3,
      // PublishPrice is required at the start of a Block Period
      sl_zigbee_af_price_server_price_update_bindings();

      // Recalculate next delay time
      eventDelaySec = sl_zigbee_af_price_server_seconds_until_block_period_event(endpoint);
    }
    if (eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS) {
      // If there are no future block periods scheduled, that means we may
      // need to schedule a future event for when the current one expires.
      // See D.4.2.4.2 of the SE spec.
      sl_zigbee_af_price_server_refresh_block_period(endpoint, true); // repeat?
      // Recalculate next delay time
      eventDelaySec = sl_zigbee_af_price_server_seconds_until_block_period_event(endpoint);
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEventMask = SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_BLOCK_PERIOD_EVENT_MASK;
    }
    if ( eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS ) {
      sl_zigbee_af_price_cluster_clear_pending_event(SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_BLOCK_PERIOD_EVENT_MASK);
    }
  }

  if ((PendingPriceEvents & SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_CALORIFIC_VALUE_EVENT_MASK) != 0U) {
    eventDelaySec = sl_zigbee_af_price_server_seconds_until_calorific_value_event(endpoint);
    if ( eventDelaySec == 0u ) {
      // Execute now
      sl_zigbee_af_price_cluster_println("Price Tick:  Calorific Value");
      sl_zigbee_af_price_server_refresh_calorific_value(endpoint);
      // Recalculate next delay time
      eventDelaySec = sl_zigbee_af_price_server_seconds_until_calorific_value_event(endpoint);
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEventMask = SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_CALORIFIC_VALUE_EVENT_MASK;
    }
    if ( eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS ) {
      sl_zigbee_af_price_cluster_clear_pending_event(SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_CALORIFIC_VALUE_EVENT_MASK);
    }
  }

  if ((PendingPriceEvents & SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_CO2_VALUE_EVENT_MASK) != 0U) {
    eventDelaySec = sl_zigbee_af_price_server_seconds_until_co2_value_event(endpoint);
    if ( eventDelaySec == 0u ) {
      // Execute now
      sl_zigbee_af_price_cluster_println("Price Tick:  CO2 Value");
      sl_zigbee_af_price_server_refresh_co2_value(endpoint);
      // Recalculate next delay time
      eventDelaySec = sl_zigbee_af_price_server_seconds_until_co2_value_event(endpoint);
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEventMask = SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_CO2_VALUE_EVENT_MASK;
    }
    if ( eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS ) {
      sl_zigbee_af_price_cluster_clear_pending_event(SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_CO2_VALUE_EVENT_MASK);
    }
  }

  if ((PendingPriceEvents & SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_CONVERSION_FACTOR_EVENT_MASK) != 0U) {
    eventDelaySec = sl_zigbee_af_price_server_seconds_until_conversion_factor_event(endpoint);
    if ( eventDelaySec == 0u ) {
      // Execute now
      sl_zigbee_af_price_cluster_println("Price Tick:  Conversion Factor");
      sl_zigbee_af_price_server_refresh_conversion_factor(endpoint);
      // Recalculate next delay time
      eventDelaySec = sl_zigbee_af_price_server_seconds_until_conversion_factor_event(endpoint);
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEventMask = SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_CONVERSION_FACTOR_EVENT_MASK;
    }
    if ( eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS ) {
      sl_zigbee_af_price_cluster_clear_pending_event(SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_CONVERSION_FACTOR_EVENT_MASK);
    }
  }

  if ((PendingPriceEvents & SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_TARIFF_INFORMATION_EVENT_MASK) != 0U) {
    eventDelaySec = sl_zigbee_af_price_server_seconds_until_tariff_info_event(endpoint);
    if ( eventDelaySec == 0u ) {
      // Execute now
      sl_zigbee_af_price_cluster_println("Price Tick: Tariff Information");
      sl_zigbee_af_price_server_refresh_tariff_information(endpoint);
      // Recalculate next delay time
      eventDelaySec = sl_zigbee_af_price_server_seconds_until_tariff_info_event(endpoint);
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEventMask = SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_TARIFF_INFORMATION_EVENT_MASK;
    }
    if ( eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS ) {
      sl_zigbee_af_price_cluster_clear_pending_event(SL_ZIGBEE_AF_PRICE_SERVER_CHANGE_TARIFF_INFORMATION_EVENT_MASK);
    }
  }

  if ((PendingPriceEvents & SL_ZIGBEE_AF_PRICE_SERVER_ACTIVATED_PRICE_MATRIX_EVENT_MASK) != 0U) {
    eventDelaySec = sl_zigbee_af_price_server_seconds_until_active_price_matrix_event(endpoint);
    if ( eventDelaySec == 0u ) {
      // Execute now
      sl_zigbee_af_price_cluster_println("Price Tick: PriceMatrix Information activated");
      sl_zigbee_af_price_server_refresh_price_matrix_information(endpoint);
      // Recalculate next delay time
      eventDelaySec = sl_zigbee_af_price_server_seconds_until_active_price_matrix_event(endpoint);
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEventMask = SL_ZIGBEE_AF_PRICE_SERVER_ACTIVATED_PRICE_MATRIX_EVENT_MASK;
    }
    if ( eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS ) {
      sl_zigbee_af_price_cluster_clear_pending_event(SL_ZIGBEE_AF_PRICE_SERVER_ACTIVATED_PRICE_MATRIX_EVENT_MASK);
    }
  }

  if ((PendingPriceEvents & SL_ZIGBEE_AF_PRICE_SERVER_ACTIVATED_BLOCK_THRESHOLD_EVENT_MASK) != 0U) {
    eventDelaySec = sl_zigbee_af_price_server_seconds_until_active_block_thresholds_event(endpoint);
    if ( eventDelaySec == 0u ) {
      // Execute now
      sl_zigbee_af_price_cluster_println("Price Tick: BlockThresholds activated");
      sl_zigbee_af_price_server_refresh_block_thresholds_information(endpoint);
      // Recalculate next delay time
      eventDelaySec = sl_zigbee_af_price_server_seconds_until_active_block_thresholds_event(endpoint);
    }
    if ( eventDelaySec < minEventDelaySec ) {
      minEventDelaySec = eventDelaySec;
      minTimeEventMask = SL_ZIGBEE_AF_PRICE_SERVER_ACTIVATED_BLOCK_THRESHOLD_EVENT_MASK;
    }
    if ( eventDelaySec == PRICE_EVENT_TIME_NO_PENDING_EVENTS ) {
      sl_zigbee_af_price_cluster_clear_pending_event(SL_ZIGBEE_AF_PRICE_SERVER_ACTIVATED_BLOCK_THRESHOLD_EVENT_MASK);
    }
  }

  if ( minEventDelaySec == 0u ) {
    // Get Scheduled Prices wants to send each price with a 250ms delay between them (< 1 second).
    // To accommodate this behavior, any event that returns 0 for its delay time after execution
    // will be padded with a 250ms delay.
    delayMSec = MILLISECOND_TICKS_PER_QUARTERSECOND;
  } else if ( minEventDelaySec != 0xFFFFFFFFU ) {
    delayMSec = (minEventDelaySec * MILLISECOND_TICKS_PER_SECOND);
  } else {
    //delayMSec = minEventDelaySec;
    return;   // Nothing left to do.
  }

  if (PendingPriceEvents != 0x00) {
    UNUSED_VAR(minTimeEventMask); // for print info only; unused if emberAfXxxPrint macro is empty
    sl_zigbee_af_price_cluster_println("Scheduling Tick Callback in %d sec, eventBitFlag=%d, pendingEvents=%d", minEventDelaySec, minTimeEventMask, PendingPriceEvents);
  } else {
    sl_zigbee_af_price_cluster_println("Scheduling Tick Callback in %d msec", delayMSec);
  }
  (void) sl_zigbee_zcl_schedule_server_tick(endpoint, ZCL_PRICE_CLUSTER_ID, delayMSec);
}

void sl_zigbee_af_price_cluster_schedule_tick_cb(uint8_t endpoint, sl_zigbee_af_price_server_pending_events_t event)
{
  PendingPriceEvents |= event;

  // This call will execute any ready events and schedule the tick for the nearest future event.
  sl_zigbee_af_price_cluster_server_tick_cb(endpoint);
}
