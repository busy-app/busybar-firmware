/***************************************************************************//**
 * @file app_event_log.c
 * @brief Application Event Log
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

#include <stdio.h>
#include <string.h>
#include <cmsis_os2.h>
#include "sl_slist.h"
#include "sl_assert.h"
#include "sl_memory_manager.h"
#include "sl_wisun_api.h"
#include "sl_rail.h"
#include "sl_wisun_rf_test.h"
#include "sl_sleeptimer.h"
#include "app_event_log.h"

#define APP_EVENT_LOG_TASK_PRIORITY osPriorityBelowNormal7
#define APP_EVENT_LOG_TASK_STACK_SIZE 500 // in units of CPU_INT32U
#define APP_EVENT_LOG_POOL_SIZE 10
#define APP_EVENT_QUEUE_SIZE 5

typedef struct {
  sl_slist_node_t node;
  uint32_t timestamp;
  sl_wisun_logger_event_t logger_event;
} app_event_log_event_t;

static sl_slist_node_t *app_event_log_list;
static sl_memory_pool_t app_event_log_pool;

static const char * const APP_EVENT_FRAME_TYPE[] =
{
  "PAS",
  "PA",
  "LPAS",
  "LPA",
  "EAPOL",
  "PCS",
  "PC",
  "LPCS",
  "LPC",
  "LTS",
  "DATA",
};

static void app_event_log_print_event(const app_event_log_event_t *evt)
{
  char address[24];
  snprintf(address, sizeof(address), "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
    evt->logger_event.address.address[0], evt->logger_event.address.address[1],
    evt->logger_event.address.address[2], evt->logger_event.address.address[3],
    evt->logger_event.address.address[4], evt->logger_event.address.address[5],
    evt->logger_event.address.address[6], evt->logger_event.address.address[7]);

  switch (evt->logger_event.type) {
    case SL_WISUN_LOGGER_EVENT_TYPE_NEIGHBOR_LIFETIME_CHANGED:
      if (evt->logger_event.u.neighbor_lifetime_changed.lifetime) {
        printf("Neighbor %s lifetime changed to %lu", address, evt->logger_event.u.neighbor_lifetime_changed.lifetime);
      } else {
        printf("Neighbor %s expired", address);
      }
      break;
    case SL_WISUN_LOGGER_EVENT_TYPE_FRAME_RECEIVED:
      printf("RX %s from %s @ %d dBm",
        APP_EVENT_FRAME_TYPE[evt->logger_event.u.frame_received.type],
        address, evt->logger_event.u.frame_received.rssi);
        if (evt->logger_event.u.frame_received.type == SL_WISUN_LOGGER_EVENT_FRAME_TYPE_PA) {
          printf(", PAN_ID: %d, load_factor: %d, routing_cost: %d, pan_size: %d", evt->logger_event.u.frame_received.data.pa.pan_id,
          evt->logger_event.u.frame_received.data.pa.load_factor,
          evt->logger_event.u.frame_received.data.pa.routing_cost,
          evt->logger_event.u.frame_received.data.pa.pan_size);
        }
      break;
    case SL_WISUN_LOGGER_EVENT_TYPE_FRAME_COUNTER_FAILURE:
      printf("RX from %s discarded due to frame counter", address);
      break;
    case SL_WISUN_LOGGER_EVENT_TYPE_TX_FAILURE:
      printf("TX %s to %s failed",
        APP_EVENT_FRAME_TYPE[evt->logger_event.u.tx_failure.type],
        address);
      break;
    case SL_WISUN_LOGGER_EVENT_TYPE_RF_TEST:
      if (evt->logger_event.u.rf_test.events & SL_RAIL_EVENT_RX_PACKET_RECEIVED) {
        printf("RF test RX");
      } else if (evt->logger_event.u.rf_test.events & SL_RAIL_EVENT_TX_PACKET_SENT) {
        printf("RF test TX");
      } else {
        printf("RF test Rail event");
      }
      break;
    default:
      printf("Unknown event %llu", evt->logger_event.type);
      break;
  }
}

sl_status_t app_event_log_init(void)
{
  sl_status_t status;

  status = sl_memory_create_pool(sizeof(app_event_log_event_t), APP_EVENT_LOG_POOL_SIZE, &app_event_log_pool);

  return status;
}

void app_event_log_print(const sl_wisun_mac_address_t *address)
{
  app_event_log_event_t *evt = NULL;
  (void)address;

  printf("!  Age\t\tEvent\r\n");
  SL_SLIST_FOR_EACH_ENTRY(app_event_log_list, evt, app_event_log_event_t, node) {
    // List is oldest-first
    if (!memcmp(&evt->logger_event.address, address, SL_WISUN_MAC_ADDRESS_SIZE)) {
      printf("!  %lus ago\t", sl_sleeptimer_get_time() - evt->timestamp);
      app_event_log_print_event(evt);
      printf("\r\n");
    }
  }
  printf("!\r\n");
}


void app_handle_event_logger_ind(sl_wisun_evt_t *evt) {
  sl_status_t status;
  app_event_log_event_t *event_log_event;
  sl_slist_node_t *item;
  sl_wisun_logger_event_t logger_event = evt->evt.logger_event.logger_event;

  status = sl_memory_pool_alloc(&app_event_log_pool, (void **)&event_log_event);
  if (status != SL_STATUS_OK) {
    // No more free entries, discard the oldest entry
    item = sl_slist_pop(&app_event_log_list);
    EFM_ASSERT(item != NULL);
    event_log_event = SL_SLIST_ENTRY(item, app_event_log_event_t, node);
  }
  event_log_event->timestamp = sl_sleeptimer_get_time();
  memcpy(&event_log_event->logger_event, &logger_event, sizeof(sl_wisun_logger_event_t));
  // List is oldest-first
  sl_slist_push_back(&app_event_log_list, &event_log_event->node);
  printf("[");
  app_event_log_print_event(event_log_event);
  printf("]\r\n");

  if (logger_event.type == SL_WISUN_LOGGER_EVENT_TYPE_RF_TEST) {
    sl_wisun_rf_test_event_callback(logger_event.u.rf_test.events, logger_event.u.rf_test.u.rx.rssi);
  }
}