/***************************************************************************//**
 * @file app.c
 * @brief Application code
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#define _DEFAULT_SOURCE  // To get strnlen()

#include <stdio.h>
#include <string.h>
#include "sl_assert.h"
#include <cmsis_os2.h>
#include "errno.h"
#include "em_device.h"
#include "sl_memory_manager.h"
#include "sl_sleeptimer.h"
#include "sl_cli.h"
#include "border_router/sl_wisun_br_api.h"
#include "sl_wisun_api.h"
#include "sl_wisun_trace_api.h"
#include "app_settings.h"
#include "sl_wisun_cli_core.h"
#include "sl_wisun_cli_util.h"
#include "sl_wisun_version.h"
#include "sl_wisun_keychain.h"
#include "sl_rail_features.h"
#include "socket/socket.h"
#include "arpa/inet.h"
#include "sl_wisun_ip6string.h"
#include "sl_select_util.h"
#include "select.h"
#include "sl_main_init.h"
#include "sl_wisun_crash_handler.h"

#if defined(SL_CATALOG_MICRIUMOS_KERNEL_PRESENT)
#include "os.h"
#endif
#if defined(SL_CATALOG_FREERTOS_KERNEL_PRESENT)
#include "FreeRTOS.h"
#include "task.h"
#endif
#ifdef SL_CATALOG_WISUN_BR_LWIP_PRESENT
#include "sl_wisun_br_lwip.h"
#endif
#ifdef SL_CATALOG_WISUN_BR_WIFI_PRESENT
#include "sl_wisun_br_wifi.h"
#endif
#ifdef SL_CATALOG_WISUN_BR_DHCPV6_RELAY_PRESENT
#include "sl_wisun_br_dhcpv6_relay.h"
#endif
#ifdef SL_CATALOG_WISUN_BR_DHCPV6_SERVER_PRESENT
#include "sl_wisun_br_dhcpv6_server.h"
#endif

#define APP_TASK_PRIORITY osPriorityLow1  // (CLI related task) must be the lowest priority in the system
#define APP_TASK_STACK_SIZE 500 // in units of CPU_INT32U

#define APP_ICMPV6_TYPE_ECHO_REQUEST 128
#define APP_ICMPV6_TYPE_ECHO_RESPONSE 129
#define APP_ICMPV6_CODE_ECHO_REQUEST 0
#define APP_ICMPV6_CODE_ECHO_RESPONSE 0
#define APP_ICMPV6_PORT 0

/// Maximum size of a certificate
#define APP_CERTIFICATE_MAX_SIZE 2048

/// All certificates index
#define APP_CERTIFICATE_INDEX_ALL 255

#define APP_SERVICE_TASK_NAME  "app_service_task"
#define APP_SERVICE_TASK_PRIORITY  osPriorityBelowNormal1
#define APP_SERVICE_TASK_STACK_SIZE  500 // in units of CPU_INT32U

SL_PACK_START(1)
typedef struct {
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
  uint16_t identifier;
  uint16_t sequence_number;
  uint8_t payload[];
} SL_ATTRIBUTE_PACKED app_icmpv6_echo_request_t;
SL_PACK_END()

#ifdef SL_CATALOG_WISUN_BR_WIFI_PRESENT
static const char *app_name = "Wi-SUN Border Router CLI Application with Wi-Fi Connectivity";
#elif defined SL_CATALOG_WISUN_BR_LWIP_PRESENT
static const char *app_name = "Wi-SUN Border Router CLI Application with Backhaul Connectivity";
#else
static const char *app_name = "Wi-SUN Border Router CLI Application";
#endif

static const app_enum_t app_socket_event_mode[] =
{
  { "indication", 0 },
  { "polling", 1 },
  { NULL, 0 }
};

static const app_enum_t app_socket_nonblock[] =
{
  { "disable", 0 },
  { "enable", 1 },
  { NULL, 0 }
};

static const app_enum_t app_remote_address_enum[] =
{
  { "br", SL_WISUN_IP_ADDRESS_TYPE_BORDER_ROUTER },
  { "border_router", SL_WISUN_IP_ADDRESS_TYPE_BORDER_ROUTER },
  { "p1", SL_WISUN_IP_ADDRESS_TYPE_PRIMARY_PARENT },
  { "parent1", SL_WISUN_IP_ADDRESS_TYPE_PRIMARY_PARENT },
  { "p2", SL_WISUN_IP_ADDRESS_TYPE_SECONDARY_PARENT },
  { "parent2", SL_WISUN_IP_ADDRESS_TYPE_SECONDARY_PARENT },
  { NULL, 0 }
};

static const app_enum_t app_mac_enum[] =
{
  { "all", 0 },
  { "any", 0 },
  { NULL, 0 }
};

static const app_enum_t app_regulation_tx_level_enum[] =
{
  { "low", SL_WISUN_REGULATION_TX_LEVEL_LOW },
  { "warning", SL_WISUN_REGULATION_TX_LEVEL_WARNING },
  { "alert", SL_WISUN_REGULATION_TX_LEVEL_ALERT },
  { NULL, 0 }
};

/// socket options
SL_PACK_START(1)
typedef union {
  /// Socket event mode
  uint32_t event_mode;
  /// Socket non-blocking mode
  uint32_t non_blocking_mode;
  /// Socket send buffer limit
  int32_t send_buffer_limit;
  /// Socket multicast group
  ipv6_mreq_t multicast_group;
  /// Socket EDFE mode
  int32_t edfe_mode;
  /// Socket receive buffer size
  int32_t receive_buff_size;
  /// Socket unicast hop limit
  int32_t unicast_hop_limit;
  /// Socket multicast hop limit
  int32_t multicast_hop_limit;
} SL_ATTRIBUTE_PACKED app_socket_option_data_t;
SL_PACK_END()

typedef sl_status_t (*app_socket_option_handler)(app_socket_option_data_t *option_data,
                                                 const char *option_data_str);

static sl_status_t app_socket_event_mode_handler(app_socket_option_data_t *option_data,
                                                 const char *option_data_str);

static sl_status_t app_socket_nonblocking_mode_handler(app_socket_option_data_t *option_data,
                                                       const char *option_data_str);

static sl_status_t app_socket_receive_buff_size_handler(app_socket_option_data_t *option_data,
                                                        const char *option_data_str);

static sl_status_t app_socket_send_buffer_limit_handler (app_socket_option_data_t *option_data,
                                                         const char *option_data_str);

static sl_status_t app_socket_unicast_hop_limit_handler(app_socket_option_data_t *option_data,
                                                        const char *option_data_str);

static sl_status_t app_socket_multicast_hop_limit_handler(app_socket_option_data_t *option_data,
                                                          const char *option_data_str);

static sl_status_t app_socket_multicast_group_handler(app_socket_option_data_t *option_data,
                                                      const char *option_data_str);

typedef struct {
  char *option;
  uint32_t option_name;
  uint32_t option_level;
  uint32_t option_length;
  app_socket_option_handler handler;
} app_socket_option_t;

static const app_socket_option_t app_set_socket_options[] =
{
  { "SO_EVENT_MODE", SO_EVENT_MODE, APP_LEVEL_SOCKET, MEMBER_SIZE(app_socket_option_data_t, event_mode), app_socket_event_mode_handler },
  { "SO_NONBLOCK", SO_NONBLOCK, APP_LEVEL_SOCKET, MEMBER_SIZE(app_socket_option_data_t, non_blocking_mode), app_socket_nonblocking_mode_handler },
  { "SO_RCVBUF", SO_RCVBUF, SOL_SOCKET, MEMBER_SIZE(app_socket_option_data_t, receive_buff_size), app_socket_receive_buff_size_handler },
  { "SO_SNDBUF", SO_SNDBUF, SOL_SOCKET, MEMBER_SIZE(app_socket_option_data_t, send_buffer_limit), app_socket_send_buffer_limit_handler },
  { "IPV6_UNICAST_HOPS", IPV6_UNICAST_HOPS, IPPROTO_IPV6, MEMBER_SIZE(app_socket_option_data_t, unicast_hop_limit), app_socket_unicast_hop_limit_handler },
  { "IPV6_MULTICAST_HOPS", IPV6_MULTICAST_HOPS, IPPROTO_IPV6, MEMBER_SIZE(app_socket_option_data_t, multicast_hop_limit), app_socket_multicast_hop_limit_handler },
  { "IPV6_JOIN_GROUP", IPV6_JOIN_GROUP, IPPROTO_IPV6, MEMBER_SIZE(app_socket_option_data_t, multicast_group), app_socket_multicast_group_handler },
  { "IPV6_LEAVE_GROUP", IPV6_LEAVE_GROUP, IPPROTO_IPV6, MEMBER_SIZE(app_socket_option_data_t, multicast_group), app_socket_multicast_group_handler },
  { NULL, 0, 0, 0, NULL }
};

static const app_socket_option_t app_get_socket_options[] =
{
  { "SO_RCVBUF", SO_RCVBUF, SOL_SOCKET, MEMBER_SIZE(app_socket_option_data_t, receive_buff_size), NULL },
  { "SO_SNDBUF", SO_SNDBUF, SOL_SOCKET, MEMBER_SIZE(app_socket_option_data_t, send_buffer_limit), NULL },
  { "IPV6_UNICAST_HOPS", IPV6_UNICAST_HOPS, IPPROTO_IPV6, MEMBER_SIZE(app_socket_option_data_t, unicast_hop_limit), NULL },
  { "IPV6_MULTICAST_HOPS", IPV6_MULTICAST_HOPS, IPPROTO_IPV6, MEMBER_SIZE(app_socket_option_data_t, multicast_hop_limit), NULL },
  { NULL, 0, 0, 0, NULL }
};

static const app_enum_t app_frame_type_enum[] = {
  { "pa", SL_WISUN_FRAME_TYPE_PA },
  { "pc", SL_WISUN_FRAME_TYPE_PC },
  { "dio", SL_WISUN_FRAME_TYPE_DIO },
  { NULL, 0 }
};

typedef app_icmpv6_echo_request_t app_icmpv6_echo_response_t;

static int app_ping_socket_id = SOCKET_INVALID_ID;
static uint32_t app_ping_tick_count;

static const in6_addr_t APP_IN6ADDR_ANY = { 0 };

static const sl_wisun_mac_address_t APP_BROADCAST_MAC =
{
  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }
};

const char * const APP_SOCKET_TYPE_STR[] =
{
  "UDP client",
  "UDP server",
  "TCP client",
  "TCP server"
};

const char * const APP_SOCKET_STATE_STR[] =
{
  "ACTIVE",
  "LISTENING",
  "CONNECTING",
  "CLOSING"
};

typedef enum {
  APP_SOCKET_TYPE_UDP_CLIENT,
  APP_SOCKET_TYPE_UDP_SERVER,
  APP_SOCKET_TYPE_TCP_CLIENT,
  APP_SOCKET_TYPE_TCP_SERVER
} app_socket_type_t;

typedef enum {
  APP_SOCKET_STATE_ACTIVE,
  APP_SOCKET_STATE_LISTENING,
  APP_SOCKET_STATE_CONNECTING,
  APP_SOCKET_STATE_CLOSING
} app_socket_state_t;

typedef struct
{
  sl_slist_node_t node;
  int socket_id;
  app_socket_type_t socket_type;
  app_socket_state_t socket_state;
  in6_addr_t remote_address;
  uint16_t remote_port;
} app_socket_entry_t;

#define APP_MAX_SOCKET_ENTRIES 10
static sl_slist_node_t *app_socket_entry_list_free;
static sl_slist_node_t *app_socket_entry_list;
static app_socket_entry_t app_socket_entries[APP_MAX_SOCKET_ENTRIES];

static bool app_started;
static int app_dhcpv6_socket = SOCKET_INVALID_ID;
static char crash_buff[300] = { 0 };

static void app_check_crash(void)
{
  const sl_wisun_crash_t *crash = sl_wisun_crash_handler_read();

  if (crash) {
    switch (crash->type) {
      case SL_WISUN_CRASH_TYPE_ASSERT:
        sprintf(crash_buff, "ASSERT in %s on line %u", crash->u.assert.file, crash->u.assert.line);
        break;
      case SL_WISUN_CRASH_TYPE_RAIL_ASSERT:
        sprintf(crash_buff, "RAIL ASSERT %lu", crash->u.rail_assert.error_code);
        break;
      case SL_WISUN_CRASH_TYPE_STACK_OVERFLOW:
        sprintf(crash_buff, "STACK OVERFLOW failure in task \"%s\"", crash->u.stack_overflow.task);
        break;
      case SL_WISUN_CRASH_TYPE_STACK_PROTECTOR:
        sprintf(crash_buff, "STACK PROTECTOR failure in 0x%08lx", crash->u.stack_protector.lr);
        break;
      case SL_WISUN_CRASH_TYPE_FAULT:
        sprintf(crash_buff, "FAULT CFSR: 0x%08lx, R0: 0x%08lx, R1: 0x%08lx, R2: 0x%08lx, R3: 0x%08lx "
                            "R12: 0x%08lx, LR: 0x%08lx, RET: 0x%08lx, XPSR: 0x%08lx "
                            "HFSR: 0x%08lx, MMFAR: 0x%08lx, BFAR: 0x%08lx, AFSR: 0x%08lx",
                            crash->u.fault.cfsr, crash->u.fault.r0, crash->u.fault.r1, crash->u.fault.r2, crash->u.fault.r3,
                            crash->u.fault.r12, crash->u.fault.lr, crash->u.fault.return_address, crash->u.fault.xpsr,
                            crash->u.fault.hfsr, crash->u.fault.mmfar, crash->u.fault.bfar, crash->u.fault.afsr);
        break;
      case SL_WISUN_CRASH_TYPE_CRUN_ERROR:
        sprintf(crash_buff, "C-RUN error 0x%08lx", crash->u.crun_error.error_code);
        break;
      case SL_WISUN_CRASH_TYPE_EXIT:
        sprintf(crash_buff, "EXIT status %d", crash->u.exit.status);
        break;
      default:
        break;
    }
    printf("%s", crash_buff);
    sl_wisun_trace_error("%s", crash_buff);
  }
  sl_wisun_crash_handler_clear();
}

static void app_handle_join_state_ind(sl_wisun_evt_t *evt)
{
#ifdef SL_CATALOG_WISUN_BR_LWIP_PRESENT
  sl_wisun_ip_address_t ll_addr;
  sl_wisun_ip_address_t gua_addr;
  sl_wisun_ip_address_t dodagid_addr;
  sl_status_t status;

  if (evt->evt.join_state.join_state == SL_WISUN_JOIN_STATE_OPERATIONAL) {
    status = sl_wisun_br_get_ip_addresses(ll_addr.address, gua_addr.address, dodagid_addr.address);
    if (status != SL_STATUS_OK) {
      printf("[app_handle_join_state_ind: sl_wisun_br_get_ip_addresses failed]\r\n");
      return;
    }
    sl_wisun_br_lwip_pan_up(ll_addr.address, gua_addr.address);
  } else {
    sl_wisun_br_lwip_pan_down();
  }
#else
  (void)evt;
#endif
}

static sl_status_t app_get_ip_address(in6_addr_t *value,
                                      const char *value_str)
{
  const app_enum_t *value_enum;

  if (!value_str) {
    return SL_STATUS_FAIL;
  }

  value_enum = app_util_get_enum_by_string(app_remote_address_enum, value_str);
  if (value_enum) {
    // Resolve the enumerated address
    return sl_wisun_get_ip_address((sl_wisun_ip_address_type_t)value_enum->value, value);
  }

  // Convert address string to binary
  if (!stoip6(value_str, strlen(value_str), &value->address[0])) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

static const char *app_get_ip_address_str(const in6_addr_t *value)
{
  static char remote_address_str[40];

  ip6tos(value->address, remote_address_str);

  return remote_address_str;
}

static app_socket_entry_t *app_socket_alloc_entry()
{
  sl_slist_node_t *item;
  app_socket_entry_t *entry = NULL;

  item = sl_slist_pop(&app_socket_entry_list_free);
  if (item) {
    entry = SL_SLIST_ENTRY(item, app_socket_entry_t, node);
    memset(entry, 0, sizeof(app_socket_entry_t));
    entry->socket_id = SOCKET_INVALID_ID;
  }

  return entry;
}

static void app_socket_free_entry(app_socket_entry_t *entry)
{
  if (!entry) {
    return;
  }

  // Remove the entry from the active entries list
  sl_slist_remove(&app_socket_entry_list, &entry->node);

  // Push the entry to the free entries list
  sl_slist_push(&app_socket_entry_list_free, &entry->node);
}

static app_socket_entry_t *app_socket_entry(int socket_id)
{
  app_socket_entry_t *entry = NULL;

  SL_SLIST_FOR_EACH_ENTRY(app_socket_entry_list, entry, app_socket_entry_t, node) {
    if (entry->socket_id == socket_id) {
      return entry;
    }
  }

  return NULL;
}

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

static void app_handle_ping_socket_data(sl_wisun_evt_t *evt)
{
  app_icmpv6_echo_response_t *packet;
  uint32_t tick_count, time_ms;

  packet = (app_icmpv6_echo_response_t *)evt->evt.socket_data.data;

  if (packet->type != APP_ICMPV6_TYPE_ECHO_RESPONSE) {
    printf("unexpected ICMP message type: %hu\r\n", packet->type);
    return;
  }

  tick_count = sl_sleeptimer_get_tick_count();
  time_ms = sl_sleeptimer_tick_to_ms(tick_count - app_ping_tick_count);

  printf("%u bytes from %s: icmp_seq=%u time=%lu ms\r\n",
         evt->evt.socket_data.data_length,
         app_get_ip_address_str(&evt->evt.socket_data.remote_address),
         htons(packet->sequence_number),
         time_ms);
}

static void app_handle_socket_data(sl_wisun_evt_t *evt)
{
  app_printable_data_ctx_t printable_data_ctx;
  char *printable_data;

  printf("[Data from %s (%u): %lu,%u",
         app_get_ip_address_str(&evt->evt.socket_data.remote_address),
         ntohs(evt->evt.socket_data.remote_port),
         evt->evt.socket_data.socket_id, evt->evt.socket_data.data_length);
  if (app_settings_app.printable_data_length) {
    printable_data = app_util_printable_data_init(&printable_data_ctx,
                                                  evt->evt.socket_data.data,
                                                  evt->evt.socket_data.data_length,
                                                  app_settings_app.printable_data_as_hex,
                                                  app_settings_app.printable_data_length);
    while (printable_data) {
      printf("\r\n%s", printable_data);
      printable_data = app_util_printable_data_next(&printable_data_ctx);
    }
  }
  printf("]\r\n");
}

static void app_handle_socket_data_ind(sl_wisun_evt_t *evt)
{
  if (evt->evt.socket_data.socket_id == app_ping_socket_id) {
    app_handle_ping_socket_data(evt);
  } else {
    app_handle_socket_data(evt);
  }
}

static void app_handle_socket_data_available_ind(sl_wisun_evt_t *evt)
{
  app_socket_entry_t *entry;

  entry = app_socket_entry(evt->evt.socket_data_sent.socket_id);
  if (!entry) {
    return;
  }
  printf("[Data available: %lu,%u]\r\n",
         evt->evt.socket_data_available.socket_id,
         evt->evt.socket_data_available.data_length);
}

static void app_handle_socket_connected_ind(sl_wisun_evt_t *evt)
{
  app_socket_entry_t *entry;

  entry = app_socket_entry(evt->evt.socket_connected.socket_id);
  if (!entry) {
    return;
  }

  if (evt->evt.socket_connected.status == SL_STATUS_OK) {
    entry->socket_state = APP_SOCKET_STATE_ACTIVE;
    printf("[Opened: %lu]\r\n", evt->evt.socket_connected.socket_id);
  } else {
    close(entry->socket_id);
    app_socket_free_entry(entry);
    printf("[Open failed: %lu]\r\n", evt->evt.socket_connected.status);
  }
}

static void app_handle_socket_connection_available_ind(sl_wisun_evt_t *evt)
{
  app_socket_entry_t *entry;
  sockaddr_in6_t ipv6_remote_addr;

  uint32_t addr_length = sizeof(sockaddr_in6_t);

  printf("[Socket connection available: %lu]\r\n",
         evt->evt.socket_connection_available.socket_id);

  entry = app_socket_alloc_entry();
  if (!entry) {
    printf("[Failed: unable to allocate a socket entry]\r\n");
    return;
  }

  entry->socket_type = APP_SOCKET_TYPE_TCP_CLIENT;
  entry->socket_state = APP_SOCKET_STATE_ACTIVE;

  entry->socket_id = accept(evt->evt.socket_connection_available.socket_id,
                            (struct sockaddr *) &ipv6_remote_addr,
                            &addr_length);

  if (entry->socket_id == SOCKET_INVALID_ID) {
    printf("[Failed: unable to accept a connection]\r\n");
    return;
  }

  entry->remote_port = ntohs(ipv6_remote_addr.sin6_port);
  memcpy(entry->remote_address.address,
         ipv6_remote_addr.sin6_addr.address,
         IPV6_ADDR_SIZE);

  // Push the entry to the active entries list
  sl_slist_push_back(&app_socket_entry_list, &entry->node);

  printf("[Accepted %s (%u): %d]\r\n", app_get_ip_address_str(&entry->remote_address),
    entry->remote_port, entry->socket_id);
}

static void app_handle_socket_closing_ind(sl_wisun_evt_t *evt)
{
  app_socket_entry_t *entry;

  entry = app_socket_entry(evt->evt.socket_closing.socket_id);
  if (!entry) {
    return;
  }

  entry->socket_state = APP_SOCKET_STATE_CLOSING;

  printf("[Closing: %lu]\r\n", evt->evt.socket_closing.socket_id);
}


static void app_handle_socket_data_sent(sl_wisun_evt_t *evt)
{
  app_socket_entry_t *entry;

  entry = app_socket_entry(evt->evt.socket_data_sent.socket_id);
  if (!entry) {
    return;
  }

  if (evt->evt.socket_data_sent.status == SL_STATUS_OK) {
    printf("[Data sent: %lu,%lu]\r\n", evt->evt.socket_data_sent.socket_id, evt->evt.socket_data_sent.socket_space_left);
  } else {
    printf("[Failed: data sent, error %lu (socket %lu)]\r\n", evt->evt.socket_data_sent.status, evt->evt.socket_data_sent.socket_id);
  }
}

static void app_handle_socket_data_sent_ind(sl_wisun_evt_t *evt)
{
  // Handle socket data sent indications only for UDP and TCP sockets.
  if (evt->evt.socket_data_sent.socket_id != app_ping_socket_id) {
    app_handle_socket_data_sent(evt);
  }
}

static void app_handle_regulation_tx_level_ind(sl_wisun_evt_t *evt)
{
  const app_enum_t *ptr;

  ptr = app_util_get_enum_by_integer(app_regulation_tx_level_enum,
                                     evt->evt.regulation_tx_level.tx_level);
  if (ptr) {
    printf("[Regulation TX level: %s (%lu) (%lu ms)]\r\n", ptr->value_str, ptr->value, evt->evt.regulation_tx_level.tx_duration_ms);
  }
}

static void app_handle_dhcp_vendor_data_ind(sl_wisun_evt_t *evt)
{
  printf("[Vendor data received: Enterprise Number %"PRIu32"]\r\nPayload: ", evt->evt.dhcp_vendor_data.enterprise_number);
  for (uint8_t i = 0; i < evt->evt.dhcp_vendor_data.data_length; i++) {
    printf("%02x ", evt->evt.dhcp_vendor_data.data[i]);
  }
  printf("\r\n");
}

static void app_handle_br_stopped_ind(sl_wisun_evt_t *evt)
{
  if (evt->evt.br_stopped.status == SL_STATUS_OK) {
    printf("[Border Router stopped]\r\n");
  } else {
    printf("[Failed to stop Border Router]\r\n");
  }
}

static void app_handle_br_routing_table_update_ind(sl_wisun_evt_t *evt)
{
  switch (evt->evt.br_routing_table_update.event) {
    case SL_WISUN_ROUTING_TABLE_UPDATE_ROUTE_CHANGED:
      printf("[Routing table update: route changed]\r\n");
      break;
    default:
      printf("[Routing table update: unknown flag %lu]\r\n", evt->evt.br_routing_table_update.event);
  }
}

static sl_status_t channel_spacing_khz_to_id(uint32_t channel_spacing_khz, uint8_t *channel_spacing_id)
{
  sl_status_t result = SL_STATUS_OK;

  switch (channel_spacing_khz) {
    case 100:
      *channel_spacing_id = SL_WISUN_CHANNEL_SPACING_100KHZ;
      break;
    case 200:
      *channel_spacing_id = SL_WISUN_CHANNEL_SPACING_200KHZ;
      break;
    case 250:
      *channel_spacing_id = SL_WISUN_CHANNEL_SPACING_250KHZ;
      break;
    case 400:
      *channel_spacing_id = SL_WISUN_CHANNEL_SPACING_400KHZ;
      break;
    case 600:
      *channel_spacing_id = SL_WISUN_CHANNEL_SPACING_600KHZ;
      break;
    case 800:
      *channel_spacing_id = SL_WISUN_CHANNEL_SPACING_800KHZ;
      break;
    case 1200:
      *channel_spacing_id = SL_WISUN_CHANNEL_SPACING_1200KHZ;
      break;
    default:
      result = SL_STATUS_FAIL;
  }

  return result;
}

static void app_service_task(void* arguments)
{
  ssize_t count;
  sockaddr_in6_t src_addr;
  socklen_t addrlen;
  static uint8_t buffer[350];
  fd_set readfds;
  int retval, max_sd = -1;
  (void)arguments;
  sl_wisun_trace_info("app_service_task: service task starting");
  while (1) {
    FD_ZERO(&readfds);
    if (app_dhcpv6_socket >= 0) {
      FD_SET(app_dhcpv6_socket, &readfds);
      max_sd = MAX(app_dhcpv6_socket, max_sd);
    }
    if (max_sd < 0) {
      sl_wisun_trace_warn("app_service_task: no sockets to be read");
      goto cleanup;
    }
    retval = select(max_sd + 1, &readfds, NULL, NULL, NULL);
    if (retval < 0) {
      sl_wisun_trace_error("app_service_task: select error %d", retval);
      goto cleanup;
    }
    if (app_dhcpv6_socket >= 0 && FD_ISSET(app_dhcpv6_socket, &readfds)) {
      addrlen = sizeof(sockaddr_in6_t);
      count = recvfrom(app_dhcpv6_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&src_addr, &addrlen);
      if (count < 0) {
        sl_wisun_trace_error("dhcpv6_server_task: recvfrom error %d", count);
        continue;
      }
#ifdef SL_CATALOG_WISUN_BR_DHCPV6_SERVER_PRESENT
      sl_wisun_br_dhcpv6_server_on_recv(buffer, count, src_addr.sin6_addr, src_addr.sin6_port);
#endif
#ifdef SL_CATALOG_WISUN_BR_DHCPV6_RELAY_PRESENT
      sl_wisun_br_dhcpv6_relay_pan_on_recv(buffer, count, src_addr.sin6_addr, src_addr.sin6_port);
#endif
    }
  }
cleanup:
  sl_wisun_trace_error("app_service_task: stopped");
  osThreadExit();
}

static sl_status_t app_service_task_start(void)
{
  osThreadId_t service_task_id;
  osThreadAttr_t task_attribute = {
      APP_SERVICE_TASK_NAME,
      osThreadDetached,
      NULL,
      0,
      NULL,
      (APP_SERVICE_TASK_STACK_SIZE * sizeof(void *)) & 0xFFFFFFF8u,
      APP_SERVICE_TASK_PRIORITY,
      0,
      0
  };
  service_task_id = osThreadNew(app_service_task, NULL, &task_attribute);
  if (!service_task_id) {
    sl_wisun_trace_error("app_service_task_start: osThreadNew failed");
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

#if defined(SL_CATALOG_WISUN_BR_DHCPV6_SERVER_PRESENT) || \
    defined(SL_CATALOG_WISUN_BR_DHCPV6_RELAY_PRESENT)
static sl_status_t app_init_dhcpv6_socket(void) {

  int retval;

  const sockaddr_in6_t bind_addr = {
    .sin6_family = AF_INET6,
    .sin6_port = htons(DHCPV6_SERVER_PORT),
    .sin6_flowinfo = 0,
    .sin6_addr = IN6ADDR_ANY_INIT,
    .sin6_scope_id = 0,
  };
  app_dhcpv6_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
  if (app_dhcpv6_socket == SOCKET_INVALID_ID) {
    printf("[Failed: failed to open DHCPv6 socket]\r\n");
    goto cleanup;
  }
  retval = bind(app_dhcpv6_socket, (const struct sockaddr *)&bind_addr, sizeof(sockaddr_in6_t));
  if (retval < 0) {
    printf("[Failed: failed to bind DHCPv6 socket (%d)]\r\n", retval);
    goto cleanup;
  }
  return SL_STATUS_OK;
cleanup:
  close(app_dhcpv6_socket);
  return SL_STATUS_FAIL;
}
#endif

static void app_start(sl_wisun_phy_config_type_t phy_config_type)
{
#if SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH
  bool set_pom_ie = false;
#endif
  sl_wisun_channel_mask_t channel_mask;
  sl_wisun_phy_config_t phy_config;
  sl_wisun_br_connection_params_t params;
  sl_wisun_br_lfn_params_t lfn_params;
  sl_status_t status = SL_STATUS_OK;
  uint16_t key_index;
  uint8_t phy_mode_id_count, is_mdr_command_capable;
  uint8_t phy_mode_id[SL_WISUN_MAX_PHY_MODE_ID_COUNT];
  uint8_t ipv6_prefix[IPV6_ADDR_SIZE];
  uint8_t ipv6_addr[IPV6_ADDR_SIZE];
  int_fast16_t ipv6_prefix_length;
  uint8_t *phy_mode_id_p, *phy_mode_id_count_p;
  uint8_t channel_spacing_id;
  uint8_t trustedca_count;
  uint8_t idx;
  sl_wisun_keychain_entry_t *trustedca = NULL;
  sl_wisun_keychain_credential_t *credential = NULL;
  uint16_t certificate_options;
  sl_wisun_mac_address_t address;
  int retval;
  const sl_wisun_regulation_params_t *regulation_params;

  app_wisun_cli_mutex_lock();

  if (app_started) {
    printf("[Failed: already started]\r\n");
    goto cleanup;
  }

  app_started = true;

  for (key_index = 0; key_index < 4; key_index++) {
    if (app_settings_wisun.gtk_set & (1 << key_index)) {
      status = sl_wisun_br_set_gtk(app_settings_wisun.gtks[key_index], key_index);
      if (status != SL_STATUS_OK && status != SL_STATUS_ALREADY_EXISTS) {
        printf("[Failed: unable to set GTK%"PRIu16" (%"PRIu32")]\r\n", key_index + 1, status);
      }
    }
  }

  for (key_index = 0; key_index < 3; key_index++) {
    if (app_settings_wisun.gtk_set & (1 << (key_index + 4))) {
      status = sl_wisun_br_set_gtk(app_settings_wisun.lgtks[key_index], key_index + 4);
      if (status != SL_STATUS_OK && status != SL_STATUS_ALREADY_EXISTS) {
        printf("[Failed: unable to set LGTK%"PRIu16" (%"PRIu32")]\r\n", key_index + 1, status);
      }
    }
  }

  status = sl_wisun_set_device_type(SL_WISUN_BORDER_ROUTER);
  if (status != SL_STATUS_OK) {
    printf("[Failed: unable to set device type (%"PRIu32")]\r\n", status);
    goto cleanup;
  }

  status = sl_wisun_set_rx_fifo_size(app_settings_wisun.rx_fifo_size);
  if (status != SL_STATUS_OK) {
    printf("[Failed: unable to set RX FIFO size: %lu]\r\n", status);
  }

  status = sl_wisun_set_tx_power_ddbm(app_settings_wisun.tx_power_ddbm);
  if (status != SL_STATUS_OK) {
    printf("Failed: unable to set TX power (%"PRIu32")]\r\n", status);
    goto cleanup;
  }

  status = sl_wisun_set_fan_tps_version(app_settings_wisun.fan_tps_version);
  if (status != SL_STATUS_OK) {
    printf("[Failed: unable to set FAN TPS version: %lu]\r\n", status);
    goto cleanup;
  }

  switch (app_settings_wisun.network_size) {
    case SL_WISUN_NETWORK_SIZE_SMALL:
      params = SL_WISUN_BR_PARAMS_PROFILE_SMALL;
      break;
    case SL_WISUN_NETWORK_SIZE_MEDIUM:
      params = SL_WISUN_BR_PARAMS_PROFILE_MEDIUM;
      break;
    case SL_WISUN_NETWORK_SIZE_LARGE:
      params = SL_WISUN_BR_PARAMS_PROFILE_LARGE;
      break;
    case SL_WISUN_NETWORK_SIZE_TEST:
      params = SL_WISUN_BR_PARAMS_PROFILE_TEST;
      break;
    default:
      printf("[Failed: unsupported network size %"PRIu8"]\r\n", app_settings_wisun.network_size);
      status = SL_STATUS_INVALID_CONFIGURATION;
      goto cleanup;
  }
  params.traffic.lowpan_mtu = app_settings_wisun.lowpan_mtu;
  params.traffic.ipv6_mru = app_settings_wisun.ipv6_mru;
  params.traffic.max_edfe_fragment_count = app_settings_wisun.max_edfe_fragment_count;
  params.mac.min_be = app_settings_mac.min_be;
  params.mac.max_be = app_settings_mac.max_be;
  params.mac.backoff_period_us = app_settings_mac.backoff_period_us;
  params.mac.max_cca_retries = app_settings_mac.max_cca_retries;
  params.mac.max_frame_retries = app_settings_mac.max_frame_retries;
  status = sl_wisun_br_set_connection_parameters(&params);
  if (status != SL_STATUS_OK) {
    printf("[Failed: unable to set parameters (%"PRIu32")]\r\n", status);
    goto cleanup;
  }

  status = sl_wisun_config_neighbor_table(app_settings_wisun.max_child_count, app_settings_wisun.max_neighbor_count, app_settings_wisun.max_security_neighbor_count);
  if (status != SL_STATUS_OK) {
    printf("[Failed: unable to set neighbor table size (%"PRIu32")]\r\n", status);
    goto cleanup;
  }

  retval = stoip6_prefix(app_settings_wisun.ipv6_prefix, ipv6_prefix, &ipv6_prefix_length);
  if (retval < 0 || ipv6_prefix_length < 0 || ipv6_prefix_length > 128) {
    printf("[Failed: IPv6 prefix could not be parsed (%d)]\r\n", retval);
    status = SL_STATUS_INVALID_CONFIGURATION;
    goto cleanup;
  }
  status = sl_wisun_br_set_ipv6_prefix(ipv6_prefix, (uint8_t)ipv6_prefix_length);
  if (status != SL_STATUS_OK) {
    printf("[Failed: unable to set ipv6 prefix (%"PRIu32")]\r\n", status);
    goto cleanup;
  }

  switch (app_settings_wisun.lfn_profile) {
    case SL_WISUN_LFN_PROFILE_TEST:
      lfn_params = SL_WISUN_BR_PARAMS_LFN_TEST;
      break;
    case SL_WISUN_LFN_PROFILE_BALANCED:
      lfn_params = SL_WISUN_BR_PARAMS_LFN_BALANCED;
      break;
    case SL_WISUN_LFN_PROFILE_ECO:
      lfn_params = SL_WISUN_BR_PARAMS_LFN_ECO;
      break;
    default:
      printf("[Failed: unsupported LFN profile %"PRIu8"]\r\n", app_settings_wisun.lfn_profile);
      status = SL_STATUS_INVALID_CONFIGURATION;
      goto cleanup;
  }
  status = sl_wisun_br_set_lfn_parameters(&lfn_params);
  if (status != SL_STATUS_OK) {
    printf("[Failed: unable to set LFN parameters (%"PRIu32")]\r\n", status);
    goto cleanup;
  }

  status = app_settings_get_channel_mask(app_settings_wisun.allowed_channels, &channel_mask);
  if (status != SL_STATUS_OK) {
    printf("Failed: unable to retrieve channel mask (%"PRIu32")]\r\n", status);
    goto cleanup;
  }

  status = sl_wisun_set_channel_mask(&channel_mask);
  if (status != SL_STATUS_OK) {
    printf("Failed: unable to set channel mask (%"PRIu32")]\r\n", status);
    goto cleanup;
  }

  status = sl_wisun_set_unicast_settings(app_settings_wisun.uc_dwell_interval_ms);
  if (status != SL_STATUS_OK) {
    printf("Failed: unable to set unicast settings (%"PRIu32")]\r\n", status);
    goto cleanup;
  }

  status = sl_wisun_br_set_broadcast_settings(app_settings_wisun.bc_interval_ms,
                                              app_settings_wisun.bc_dwell_interval_ms);
  if (status != SL_STATUS_OK) {
    printf("[Failed: unable to set broadcast settings (%"PRIu32")]\r\n", status);
    goto cleanup;
  }

  trustedca_count = sl_wisun_keychain_get_trustedca_count();
  if (!trustedca_count) {
    printf("[Failed: unable to locate trusted CAs]\r\n");
    status = SL_STATUS_INVALID_CONFIGURATION;
    goto cleanup;
  }

  certificate_options = SL_WISUN_CERTIFICATE_OPTION_IS_REF;
  for (idx = 0; idx < trustedca_count; ++idx) {
    trustedca = sl_wisun_keychain_get_trustedca(idx);
    if (!trustedca) {
      printf("[Failed: unable to load trusted CA %u]\r\n", idx);
      status = SL_STATUS_INVALID_CONFIGURATION;
      goto cleanup;
    }
    if (trustedca->keychain == SL_WISUN_KEYCHAIN_NVM) {
      printf("[Using NVM trusted CA #%u]\r\n", idx);
    } else if (trustedca->keychain == SL_WISUN_KEYCHAIN_BUILTIN) {
      printf("[Using built-in trusted CA #%u]\r\n", idx);
    }

    status = sl_wisun_set_trusted_certificate(certificate_options,
                                              trustedca->data_length,
                                              trustedca->data);
    if (status != SL_STATUS_OK) {
      printf("[Failed: unable to set the trusted certificate (%"PRIu32")]\r\n", status);
      goto cleanup;
    }

    sl_free(trustedca);
    trustedca = NULL;
    certificate_options |= SL_WISUN_CERTIFICATE_OPTION_APPEND;
  }

  credential = sl_wisun_keychain_get_credential((sl_wisun_keychain_t)app_settings_wisun.keychain, app_settings_wisun.keychain_index);
  if (!credential) {
    printf("[Failed: unable to load credential]\r\n");
    status = SL_STATUS_INVALID_CONFIGURATION;
    goto cleanup;
  }
  if (credential->certificate.keychain == SL_WISUN_KEYCHAIN_NVM) {
    printf("[Using NVM device credentials]\r\n");
  } else if (credential->certificate.keychain == SL_WISUN_KEYCHAIN_BUILTIN) {
    printf("[Using built-in device credentials]\r\n");
  }

  status = sl_wisun_set_br_device_certificate(SL_WISUN_CERTIFICATE_OPTION_IS_REF | SL_WISUN_CERTIFICATE_OPTION_HAS_KEY,
                                              credential->certificate.data_length,
                                              credential->certificate.data);
  if (status != SL_STATUS_OK) {
    printf("[Failed: unable to set the device certificate (%"PRIu32")]\r\n", status);
    goto cleanup;
  }

  if (credential->pk.type == SL_WISUN_KEYCHAIN_KEY_TYPE_PLAINTEXT) {
    status = sl_wisun_set_device_private_key(SL_WISUN_PRIVATE_KEY_OPTION_IS_REF,
                                             credential->pk.u.plaintext.data_length,
                                             credential->pk.u.plaintext.data);
  } else {
    status = sl_wisun_set_device_private_key_id(credential->pk.u.key_id);
  }
  if (status != SL_STATUS_OK) {
    printf("[Failed: unable to set the device private key (%"PRIu32")]\r\n", status);
    goto cleanup;
  }

    switch(app_settings_wisun.regulation) {
    case SL_WISUN_REGULATION_NONE:
      regulation_params = &SL_WISUN_REGULATION_PARAMS_NONE;
      break;
    case SL_WISUN_REGULATION_ARIB:
      regulation_params = &SL_WISUN_REGULATION_PARAMS_ARIB;
      break;
    case SL_WISUN_REGULATION_WPC:
      regulation_params = &SL_WISUN_REGULATION_PARAMS_WPC;
      break;
    case SL_WISUN_REGULATION_ETSI:
      regulation_params = &SL_WISUN_REGULATION_PARAMS_ETSI;
      break;
    default:
      printf("[Failed: unsupported regulation]\r\n");
      goto cleanup;
  }

  status = sl_wisun_set_regulation_parameters(regulation_params);
  if (status != SL_STATUS_OK) {
    printf("[Failed: unable to set regional regulation (%"PRIu32")]\r\n", status);
    goto cleanup;
  }

  status = sl_wisun_set_regulation_tx_thresholds(app_settings_wisun.regulation_warning_threshold,
                                                 app_settings_wisun.regulation_alert_threshold);
  if (status != SL_STATUS_OK) {
    printf("[Failed: unable to set regulation TX thresholds (%"PRIu32")]\r\n", status);
    goto cleanup;
  }

  switch (phy_config_type) {
      case SL_WISUN_PHY_CONFIG_FAN10:
        phy_config.config.fan10.reg_domain = app_settings_wisun.regulatory_domain;
        phy_config.config.fan10.op_class = app_settings_wisun.operating_class;
        phy_config.config.fan10.op_mode = app_settings_wisun.operating_mode;
        phy_config.config.fan10.fec = app_settings_wisun.fec;
        break;
      case SL_WISUN_PHY_CONFIG_FAN11:
        phy_config.config.fan11.reg_domain = app_settings_wisun.regulatory_domain;
        phy_config.config.fan11.chan_plan_id = app_settings_wisun.chan_plan_id;
        phy_config.config.fan11.phy_mode_id = app_settings_wisun.phy_mode_id;
        break;
      case SL_WISUN_PHY_CONFIG_EXPLICIT:
        phy_config.config.explicit_plan.ch0_frequency_khz = app_settings_wisun.ch0_frequency;
        phy_config.config.explicit_plan.number_of_channels = app_settings_wisun.number_of_channels;
        status = channel_spacing_khz_to_id(app_settings_wisun.channel_spacing, &channel_spacing_id);
        if (status != SL_STATUS_OK) {
          printf("[Failed: invalid channel spacing %"PRIu16"kHz]\r\n", app_settings_wisun.channel_spacing);
          goto cleanup;
        }
        phy_config.config.explicit_plan.channel_spacing = channel_spacing_id;
        phy_config.config.explicit_plan.phy_mode_id = app_settings_wisun.phy_mode_id;
        memcpy(phy_config.config.explicit_plan.channel_mask, channel_mask.mask, SL_WISUN_CHANNEL_MASK_SIZE);
        break;
      case SL_WISUN_PHY_CONFIG_IDS:
        phy_config.config.ids.protocol_id  = app_settings_wisun.protocol_id;
        phy_config.config.ids.channel_id   = app_settings_wisun.channel_id;
        phy_config.config.ids.phy_mode_id  = app_settings_wisun.phy_mode_id;
        break;
      case SL_WISUN_PHY_CONFIG_CUSTOM_FSK:
        phy_config.config.custom_fsk.ch0_frequency_khz = app_settings_wisun.ch0_frequency;
        phy_config.config.custom_fsk.number_of_channels = app_settings_wisun.number_of_channels;
        phy_config.config.custom_fsk.channel_spacing_khz = app_settings_wisun.channel_spacing;
        phy_config.config.custom_fsk.phy_mode_id = app_settings_wisun.phy_mode_id;
        phy_config.config.custom_fsk.crc_type = app_settings_wisun.crc_type;
        phy_config.config.custom_fsk.preamble_length = app_settings_wisun.preamble_length;
        break;
      case SL_WISUN_PHY_CONFIG_CUSTOM_OFDM:
        phy_config.config.custom_ofdm.ch0_frequency_khz = app_settings_wisun.ch0_frequency;
        phy_config.config.custom_ofdm.number_of_channels = app_settings_wisun.number_of_channels;
        phy_config.config.custom_ofdm.channel_spacing_khz = app_settings_wisun.channel_spacing;
        phy_config.config.custom_ofdm.phy_mode_id = app_settings_wisun.phy_mode_id;
        phy_config.config.custom_ofdm.crc_type = app_settings_wisun.crc_type;
        phy_config.config.custom_ofdm.stf_length = app_settings_wisun.stf_length;
        break;
      case SL_WISUN_PHY_CONFIG_CUSTOM_OQPSK:
        phy_config.config.custom_oqpsk.ch0_frequency_khz = app_settings_wisun.ch0_frequency;
        phy_config.config.custom_oqpsk.number_of_channels = app_settings_wisun.number_of_channels;
        phy_config.config.custom_oqpsk.channel_spacing_khz = app_settings_wisun.channel_spacing;
        phy_config.config.custom_oqpsk.phy_mode_id = app_settings_wisun.phy_mode_id;
        phy_config.config.custom_oqpsk.crc_type = app_settings_wisun.crc_type;
        phy_config.config.custom_oqpsk.preamble_length = app_settings_wisun.preamble_length;
        break;
      default:
        printf("[Failed: unsupported PHY configuration type %"PRIu8"]\r\n", phy_config_type);
        status = SL_STATUS_INVALID_CONFIGURATION;
        goto cleanup;
  }

  app_settings_wisun.phy_config_type = phy_config.type = phy_config_type;

  status = sl_wisun_set_pti_state(app_settings_app.pti_state);
  if (status == SL_STATUS_FAIL) {
    printf("[Failed: unable to set PTI state (%"PRIu32")]\r\n", status);
    goto cleanup;
  }

#if SLI_WISUN_DISABLE_SECURITY
  status = sl_wisun_set_security_state(app_security_state);
  if (status != SL_STATUS_OK) {
    printf("[Failed: unable to set security state %"PRIu32" (%"PRIu32")]\r\n", app_security_state, status);
    goto cleanup;
  }
#endif
#ifdef SL_CATALOG_WISUN_BR_DHCPV6_SERVER_PRESENT
  status = sl_wisun_get_mac_address(&address);
  if (status != SL_STATUS_OK) {
    printf("[Failed: could not get MAC address to start DHCPv6 Server (%"PRIu32")]\r\n", status);
    goto cleanup;
  }
  status = sl_wisun_br_dhcpv6_server_start(app_dhcpv6_socket, ipv6_prefix, address.address, LIFETIME_INFINITE);
  if (status != SL_STATUS_OK) {
    printf("[Failed: could not start DHCPv6 Server]\r\n");
    goto cleanup;
  }
#else
  (void)address;
#endif
#ifdef SL_CATALOG_WISUN_BR_DHCPV6_RELAY_PRESENT
  retval = stoip6(app_settings_wisun.dhcpv6_server, strlen(app_settings_wisun.dhcpv6_server), ipv6_addr);
  if (!retval) {
    printf("[Failed: could not parse DHCPv6 Server's IPv6 address (%d)]\r\n", retval);
    goto cleanup;
  }
  status = sl_wisun_br_dhcpv6_relay_start(app_dhcpv6_socket, ipv6_addr);
  if (status != SL_STATUS_OK) {
    printf("[Failed: could not start DHCPv6 Relay]\r\n");
    goto cleanup;
  }
#else
  (void)ipv6_addr;
#endif

  status = sl_wisun_br_start((const uint8_t *)app_settings_wisun.network_name, &phy_config);
  if (status != SL_STATUS_OK) {
    printf("[Failed: unable to start border router (%"PRIu32")]\r\n", status);
    goto cleanup;
  }

#if SL_RAIL_IEEE802154_SUPPORTS_G_MODE_SWITCH
  // Configure POM-IE
  // If PhyModeIds are set by user, send them to the stack, otherwise
  // retrieve the default PhyModeIds from the stack first
  if (app_settings_wisun.rx_phy_mode_ids_count == -1) {
    // Check if default PhyList can be retrieved from device
    status = sl_wisun_get_pom_ie(&phy_mode_id_count, phy_mode_id, &is_mdr_command_capable);
    if (status != SL_STATUS_OK) {
      // POM-IE not available
      printf("[Failed: unable to get default PHY list (%"PRIu32")]\r\n", status);
      sl_wisun_br_stop();
      goto cleanup;
    }

    phy_mode_id_p = phy_mode_id;
    phy_mode_id_count_p = &phy_mode_id_count;

    if (is_mdr_command_capable != app_settings_wisun.rx_mdr_capable) {
      set_pom_ie = true;
    }
  } else {
    set_pom_ie = true;
    phy_mode_id_p = app_settings_wisun.rx_phy_mode_ids;
    phy_mode_id_count_p = (uint8_t *)&app_settings_wisun.rx_phy_mode_ids_count;
  }

  if (set_pom_ie) {
    status = sl_wisun_set_pom_ie(*phy_mode_id_count_p,
                              phy_mode_id_p,
                              app_settings_wisun.rx_mdr_capable);
    if (status != SL_STATUS_OK) {
      printf("[Failed: unable to set RX PhyModeId list (%"PRIu32")]\r\n", status);
      sl_wisun_br_stop();
      goto cleanup;
    }
  }
#else
  (void)phy_mode_id_count;
  (void)is_mdr_command_capable;
  (void)phy_mode_id;
  (void)phy_mode_id_p;
  (void)phy_mode_id_count_p;
#endif

  printf("[Border router started]\r\n");

cleanup:

  if (trustedca) {
    sl_free(trustedca);
  }
  if (credential) {
    sl_free(credential);
  }

  if (status != SL_STATUS_OK) {
    app_started = false;
  }

  app_wisun_cli_mutex_unlock();
}

void app_start_fan10(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  app_start(SL_WISUN_PHY_CONFIG_FAN10);
}

void app_start_fan11(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  app_start(SL_WISUN_PHY_CONFIG_FAN11);
}

void app_start_explicit(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  app_start(SL_WISUN_PHY_CONFIG_EXPLICIT);
}

void app_start_ids(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  app_start(SL_WISUN_PHY_CONFIG_IDS);
}

void app_start_custom_fsk(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  app_start(SL_WISUN_PHY_CONFIG_CUSTOM_FSK);
}

void app_start_custom_ofdm(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  app_start(SL_WISUN_PHY_CONFIG_CUSTOM_OFDM);
}

void app_start_custom_oqpsk(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  app_start(SL_WISUN_PHY_CONFIG_CUSTOM_OQPSK);
}

void app_stop(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t ret;

  app_wisun_cli_mutex_lock();

  if (!app_started) {
    printf("[Failed: already stopped]\r\n");
    goto cleanup;
  }

  ret = sl_wisun_br_stop();
  if (ret != SL_STATUS_OK) {
    printf("[Failed: unable to stop Border Router: %lu]\r\n", ret);
    goto cleanup;
  }

#ifdef SL_CATALOG_WISUN_BR_DHCPV6_SERVER_PRESENT
  sl_wisun_br_dhcpv6_server_stop();
#endif
#ifdef SL_CATALOG_WISUN_BR_DHCPV6_RELAY_PRESENT
  sl_wisun_br_dhcpv6_relay_stop();
#endif

  app_started = false;

cleanup:

  app_wisun_cli_mutex_unlock();
}

void app_about(void)
{
  sl_status_t status;
  uint8_t major, minor, patch;
  uint16_t build;

  printf("%s\r\n", app_name);
  printf("Versions:\r\n");
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
  printf("  * Compiler (GCC): %u.%u.%u\r\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif
#if defined(__ICCARM__) && defined(__VER__)
  printf("  * Compiler (IAR): %lu.%lu.%lu\r\n", __VER__ / 1000000UL, (__VER__ % 1000000UL) / 1000UL, __VER__ % 1000UL);
#endif
#if defined(SL_CATALOG_MICRIUMOS_KERNEL_PRESENT) && defined(OS_VERSION)
  printf("  * Micrium OS kernel: %lu.%lu.%lu\r\n", OS_VERSION / 10000UL, (OS_VERSION % 10000UL) / 100UL, OS_VERSION % 100UL);
#endif
#if defined(SL_CATALOG_FREERTOS_KERNEL_PRESENT) && defined(tskKERNEL_VERSION_MAJOR) && defined(tskKERNEL_VERSION_MINOR) && defined(tskKERNEL_VERSION_BUILD)
  printf("  * FreeRTOS kernel: %u.%u.%u\r\n", tskKERNEL_VERSION_MAJOR, tskKERNEL_VERSION_MINOR, tskKERNEL_VERSION_BUILD);
#endif
  status = sl_wisun_get_stack_version(&major, &minor, &patch, &build);
  if (status == SL_STATUS_OK) {
    printf("  * Wi-SUN: %u.%u.%u.%"PRIu16"\r\n", major, minor, patch, build);
  } else {
    printf("  * Wi-SUN: error %"PRIu32" getting stack version\r\n", status);
  }
}

void app_reboot(sl_cli_command_arg_t *arguments)
{
  (void) arguments;

  app_wisun_cli_mutex_lock();

  NVIC_SystemReset();

  app_wisun_cli_mutex_unlock();
}

static const app_enum_t app_trace_level_type_enum[] =
{
  {"all",  0xFF},
  {NULL,      0}
};

static const app_enum_t app_trace_level_enum[] =
{
  {"none",    0},
  {"error",   1},
  {"err",     1},
  {"warning", 2},
  {"warn",    2},
  {"info",    3},
  {"debug",   4},
  {"dbg",     4},
  {NULL,      0}
};

static bool trace_level_add_entry(sl_wisun_trace_group_config_t *trace_config, uint8_t *group_count, uint8_t group_id, uint8_t trace_level)
{
  uint8_t i;

  for (i = 0; i < *group_count; i++) {
    if (trace_config[i].group_id == group_id) {
      // Found existing group. Update it.
      trace_config[i].trace_level = trace_level;
      return true;
    }
  }

  // group not found. Add new one.
  trace_config[i].group_id = group_id;
  trace_config[i].trace_level = trace_level;
  (*group_count)++;
  return false;
}

void app_set_trace_level(sl_cli_command_arg_t *arguments)
{
  sl_status_t ret;
  uint8_t i, group_count = 0;
  char *trace_config_string;
  uint32_t trace_level, group_id;
  char group_id_str[20], trace_level_str[20];
  sl_wisun_trace_group_config_t *trace_config = NULL;
  int res;
  const app_enum_t *value_enum;

  app_wisun_cli_mutex_lock();

  trace_config_string = sl_cli_get_argument_string(arguments, 0);
  trace_config = sl_malloc(SL_WISUN_TRACE_GROUP_COUNT * sizeof(sl_wisun_trace_group_config_t));
  trace_config_string = strtok(trace_config_string, ";");
  while (trace_config_string != NULL) {

    res = sscanf(trace_config_string, "%[^,],%s", group_id_str, trace_level_str);
    if (res != 2) {
      // Error parsing string
      ret = SL_STATUS_INVALID_PARAMETER;
      goto cleanup;
    }

    // Check group_id enum
    value_enum = app_util_get_enum_by_string(app_trace_level_type_enum, group_id_str);
    if (value_enum) {
      // group_id is an enum (only "all" accepted)

      // Check trace_level enum
      value_enum = app_util_get_enum_by_string(app_trace_level_enum, trace_level_str);

      if (value_enum) {
        trace_level = value_enum->value;
      } else {
        ret = app_util_get_integer(&trace_level, trace_level_str, NULL, false);
        if (ret != SL_STATUS_OK) {
          // Not a valid level value
          ret = SL_STATUS_INVALID_TYPE;
          goto cleanup;
        }
      }

      for (i = 0; i < SL_WISUN_TRACE_GROUP_COUNT; i++) {
        trace_level_add_entry(trace_config, &group_count, i, trace_level);
      }
    } else {
      // group_id is not an enum

      ret = app_util_get_integer(&group_id, group_id_str, NULL, false);
      if (ret != SL_STATUS_OK) {
        // Not a valid group value
        ret = SL_STATUS_INVALID_TYPE;
        goto cleanup;
      }

      // Check trace_level enum
      value_enum = app_util_get_enum_by_string(app_trace_level_enum, trace_level_str);

      if (value_enum) {
        trace_level = value_enum->value;
      } else {
        ret = app_util_get_integer(&trace_level, trace_level_str, NULL, false);
        if (ret != SL_STATUS_OK) {
          // Not a valid level value
          ret = SL_STATUS_INVALID_TYPE;
          goto cleanup;
        }
      }

      trace_level_add_entry(trace_config, &group_count, group_id, trace_level);
    }
    trace_config_string = strtok(NULL, ";");
  }

  ret = sl_wisun_set_trace_level(group_count, trace_config);
  if (ret != SL_STATUS_OK) {
    goto cleanup;
  }

cleanup:
  sl_free(trace_config);
  if (ret == SL_STATUS_OK) {
    printf("[Set %d trace groups]\r\n", group_count);
  } else {
    printf("[Error when setting trace level: %lu]\r\n", ret);
  }

  app_wisun_cli_mutex_unlock();
}

void app_clear_credential_cache(sl_cli_command_arg_t *arguments)
{
  sl_status_t ret;
  (void)arguments;

  app_wisun_cli_mutex_lock();

  ret = sl_wisun_clear_credential_cache();
  if (ret == SL_STATUS_OK) {
    printf("[Credential cache cleared]\r\n");
  } else {
    printf("[Credential cache clear failed: %lu]\r\n", ret);
  }

  app_wisun_cli_mutex_unlock();
}

void app_reset_statistics(sl_cli_command_arg_t *arguments)
{
  sl_status_t ret;
  (void)arguments;

  app_wisun_cli_mutex_lock();

  ret = sl_wisun_reset_statistics(SL_WISUN_STATISTICS_TYPE_PHY);
  if (ret != SL_STATUS_OK) {
    goto cleanup;
  }

  ret = sl_wisun_reset_statistics(SL_WISUN_STATISTICS_TYPE_MAC);
  if (ret != SL_STATUS_OK) {
    goto cleanup;
  }

  ret = sl_wisun_reset_statistics(SL_WISUN_STATISTICS_TYPE_FHSS);
  if (ret != SL_STATUS_OK) {
    goto cleanup;
  }

  ret = sl_wisun_reset_statistics(SL_WISUN_STATISTICS_TYPE_WISUN);
  if (ret != SL_STATUS_OK) {
    goto cleanup;
  }

  ret = sl_wisun_reset_statistics(SL_WISUN_STATISTICS_TYPE_NETWORK);
  if (ret != SL_STATUS_OK) {
    goto cleanup;
  }

cleanup:

  if (ret == SL_STATUS_OK) {
    printf("[Statistics reset]\r\n");
  } else {
    printf("[Statistics reset failed: %lu]\r\n", ret);
  }

  app_wisun_cli_mutex_unlock();
}

void app_set_lfn_support(sl_cli_command_arg_t *arguments)
{
  sl_status_t ret;
  uint8_t lfn_limit;
  bool lfn_support_pan;

  app_wisun_cli_mutex_lock();

  lfn_limit = sl_cli_get_argument_uint8(arguments, 0);
  lfn_support_pan = (bool)sl_cli_get_argument_uint8(arguments, 1);

  ret = sl_wisun_br_set_lfn_support(lfn_limit, lfn_support_pan);
  if (ret != SL_STATUS_OK) {
    printf("[Failed: unable to set LFN support: %lu]\r\n", ret);
  } else {
    printf("[LFN support set]\r\n");
  }

  app_wisun_cli_mutex_unlock();
}

#ifdef SL_CATALOG_WISUN_BR_DHCPV6_SERVER_PRESENT
void app_set_dhcp_vendor_data(sl_cli_command_arg_t *arguments)
{
  sl_status_t ret;
  uint32_t enterprise_number;
  size_t data_length = 0;
  uint8_t *data = NULL;

  app_wisun_cli_mutex_lock();

  enterprise_number = sl_cli_get_argument_uint32(arguments, 0);
  data = sl_cli_get_argument_hex(arguments, 1, &data_length);
  printf("%d\r\n", data_length);

  ret = sl_wisun_br_dhcpv6_set_vendor_data(enterprise_number, (uint16_t)data_length, data);
  if (ret != SL_STATUS_OK) {
    printf("[Failed: unable to set DHCP Vendor data: %lu]\r\n", ret);
  } else {
    printf("[DHCP Vendor data set]\r\n");
  }

  app_wisun_cli_mutex_unlock();
}
#endif

void app_pan_defect_advertise(sl_cli_command_arg_t *arguments)
{
  sl_status_t ret;
  bool defect_advertise;
  uint32_t min_scan_duration_s;
  uint32_t max_scan_duration_s;

  app_wisun_cli_mutex_lock();

  if(sl_cli_get_argument_count(arguments) != 3) {
    printf("Failed: invalid number of arguments");
    goto cleanup;
  }

  defect_advertise    = (bool)sl_cli_get_argument_uint8(arguments, 0);
  min_scan_duration_s = sl_cli_get_argument_uint32(arguments, 1);
  max_scan_duration_s = sl_cli_get_argument_uint32(arguments, 2);

  ret = sl_wisun_br_pan_defect_advertise(defect_advertise,
                                         min_scan_duration_s,
                                         max_scan_duration_s);
  if (ret != SL_STATUS_OK) {
    printf("[Failed: unable to set PAN Defect: %lu]\r\n", ret);
  } else {
    printf("[PAN Defect advertisement %s]\r\n", defect_advertise ? "started" : "cancelled");
  }
cleanup:
  app_wisun_cli_mutex_unlock();
}

/* CLI app PHY mode switch */
void app_mode_switch(sl_cli_command_arg_t *arguments)
{
  sl_status_t res;
  sl_wisun_mac_address_t address;
  uint8_t mode;
  uint8_t phy_mode_id;
  char *address_str = "ff:ff:ff:ff:ff:ff:ff:ff";

  app_wisun_cli_mutex_lock();

  mode = sl_cli_get_argument_uint8(arguments, 0);
  phy_mode_id = sl_cli_get_argument_uint8(arguments, 1);

  if (sl_cli_get_argument_count(arguments) > 2) {
    // to get the third argument that is the specified address
    address_str = sl_cli_get_argument_string(arguments, 2);
    if (address_str == NULL) {
      printf("[Failed: invalid address string argument]\r\n");
      goto cleanup;
    }
  }

  // Attempt to convert the MAC address string
  res = app_util_get_mac_address(&address, address_str);
  if (res != SL_STATUS_OK) {
    printf("[Failed: unable to parse the MAC address: %lu]\r\n", res);
    goto cleanup;
  }

  res = sl_wisun_set_mode_switch(mode, phy_mode_id, &address);
  switch (res) {
    case SL_STATUS_OK:
      printf("[Mode switch succeeded]\r\n");
      break;
    case SL_STATUS_NOT_SUPPORTED:
      printf("[Mode switch feature not supported on this chip]\r\n");
      break;
    default:
      printf("[Mode switch failed]\r\n");
  }

cleanup:

  app_wisun_cli_mutex_unlock();
}

/* CLI app concurrent detection */
void app_concurrent_detection(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  bool enable;
  uint8_t reserved = 0;

  app_wisun_cli_mutex_lock();

  enable = sl_cli_get_argument_uint8(arguments, 0);

  status = sl_wisun_config_concurrent_detection(enable, reserved);
  switch (status) {
    case SL_STATUS_OK:
      printf("[Concurrent detection succeeded]\r\n");
      break;
    case SL_STATUS_NOT_SUPPORTED:
      printf("[Concurrent detection feature not supported on this chip]\r\n");
      break;
    default:
      printf("[Concurrent detection failure %"PRIu32"]\r\n", status);
  }
  app_wisun_cli_mutex_unlock();
}


#ifdef SL_CATALOG_WISUN_BR_WIFI_PRESENT

void app_wifi_connect(void)
{
  sl_status_t status;
  const uint8_t *ssid = app_settings_wifi.ssid;
  uint8_t ssid_length = strlen((char *)app_settings_wifi.ssid);
  const uint8_t *passphrase = app_settings_wifi.passphrase;
  printf("[wifi: connecting to %s]\r\n", ssid);
  status = sl_wisun_br_wifi_connect(ssid, ssid_length, app_settings_wifi.security_type, passphrase);
  if (status != SL_STATUS_OK) {
    printf("[wifi: connection failure 0x%08"PRIx32"]\r\n", status);
  }
}

void app_wifi_disconnect(void)
{
  sl_wisun_br_wifi_disconnect();
}

void app_wifi_on_join(bool connected)
{
  if (connected) {
    printf("[wifi: connection successful]\r\n");
    if (app_settings_app.autostart) {
      app_start(app_settings_wisun.phy_config_type);
    }
  } else {
    printf("[wifi: connection failure]\r\n");
  }
}

#endif  // SL_CATALOG_WISUN_BR_WIFI_PRESENT

static void app_task(void *argument)
{
  sl_status_t status;
  bool ret;

  (void)argument;
  (void)status;
  (void)ret;

  printf("%s\r\n", app_name);

  app_check_crash();

#ifdef SL_CATALOG_WISUN_BR_LWIP_PRESENT
  sl_wisun_br_set_ipv6_up_handler(sl_wisun_br_lwip_pan_input);
  sl_wisun_br_lwip_init();
#endif
#ifdef SL_CATALOG_WISUN_BR_WIFI_PRESENT
  status = sl_wisun_br_wifi_init();
  if (status != SL_STATUS_OK) {
    printf("[Failed: could not initialize Wi-Fi]\r\n");
    osThreadExit();
  }
  sl_wisun_br_wifi_set_join_handler(app_wifi_on_join);
#endif
#ifdef SL_CATALOG_WISUN_BR_DHCPV6_RELAY_PRESENT
  status = sl_wisun_br_dhcpv6_relay_init();
  if (status != SL_STATUS_OK) {
    printf("[Failed: could not init DHCPv6 Relay]\r\n");
    osThreadExit();
  }
#endif
#ifdef SL_CATALOG_WISUN_BR_DHCPV6_SERVER_PRESENT
  status = sl_wisun_br_dhcpv6_server_init();
  if (status != SL_STATUS_OK) {
    printf("[Failed: could not init DHCPv6 Server]\r\n");
    osThreadExit();
  }
#endif
#if defined(SL_CATALOG_WISUN_BR_DHCPV6_SERVER_PRESENT) || \
    defined(SL_CATALOG_WISUN_BR_DHCPV6_RELAY_PRESENT)
  status = app_init_dhcpv6_socket();
  if (status != SL_STATUS_OK) {
    printf("[Failed: could not open DHCPv6 socket (%"PRIu32")]\r\n", status);
    osThreadExit();
  }
  status = app_service_task_start();
  if (status != SL_STATUS_OK) {
    printf("[Failed: could not start service task]\r\n");
    osThreadExit();
  }
#endif
  if (app_settings_app.autostart) {
#ifdef SL_CATALOG_WISUN_BR_WIFI_PRESENT
    app_wifi_connect();
#else
    app_start((sl_wisun_phy_config_type_t)app_settings_wisun.phy_config_type);
#endif
  }

  osThreadExit();
}

void app_init(void)
{
  osThreadId_t task_id;
  osThreadAttr_t task_attribute = {
    "App Task",
    osThreadDetached,
    NULL,
    0,
    NULL,
    (APP_TASK_STACK_SIZE * sizeof(void *)) & 0xFFFFFFF8u,
    APP_TASK_PRIORITY,
    0,
    0
  };
  int i;

  sl_wisun_crash_handler_init();

  // Initialize socket entry lists
  sl_slist_init(&app_socket_entry_list_free);
  sl_slist_init(&app_socket_entry_list);

  for (i = 0; i < APP_MAX_SOCKET_ENTRIES; ++i) {
    sl_slist_push(&app_socket_entry_list_free, &app_socket_entries[i].node);
  }

  task_id = osThreadNew(app_task, NULL, &task_attribute);
  (void)task_id; // prevents unused variable warning in release builds
  EFM_ASSERT(task_id != 0);
}

void sl_wisun_on_event(sl_wisun_evt_t *evt)
{
  sl_status_t result;
  char mac_str[24];

  app_wisun_cli_mutex_lock();

  switch (evt->header.id) {
    case SL_WISUN_MSG_SOCKET_DATA_IND_ID:
      app_handle_socket_data_ind(evt);
      break;
    case SL_WISUN_MSG_SOCKET_DATA_AVAILABLE_IND_ID:
      app_handle_socket_data_available_ind(evt);
      sl_wisun_check_read_sockfd_set();
      break;
    case SL_WISUN_MSG_SOCKET_CONNECTED_IND_ID:
      app_handle_socket_connected_ind(evt);
      break;
    case SL_WISUN_MSG_SOCKET_CONNECTION_AVAILABLE_IND_ID:
      app_handle_socket_connection_available_ind(evt);
      break;
    case SL_WISUN_MSG_SOCKET_CLOSING_IND_ID:
      app_handle_socket_closing_ind(evt);
      break;
    case SL_WISUN_MSG_SOCKET_DATA_SENT_IND_ID:
      app_handle_socket_data_sent_ind(evt);
      sl_wisun_check_write_sockfd_set();
      break;
    case SL_WISUN_MSG_REGULATION_TX_LEVEL_IND_ID:
      app_handle_regulation_tx_level_ind(evt);
      break;
    case SL_WISUN_MSG_MODE_SWITCH_FALLBACK_IND_ID:
      result = app_util_get_mac_address_string(mac_str, &evt->evt.mode_switch_fallback.address);
      printf ("[mode switch fallback for %s !!]\r\n", result == SL_STATUS_OK ? mac_str : "invalid address");
      break;
    case SL_WISUN_MSG_JOIN_STATE_IND_ID:
      app_handle_join_state_ind(evt);
      break;
    case SL_WISUN_MSG_DHCP_VENDOR_DATA_IND_ID:
      app_handle_dhcp_vendor_data_ind(evt);
      break;
    case SL_WISUN_BR_MSG_STOPPED_IND_ID:
      app_handle_br_stopped_ind(evt);
      break;
    case SL_WISUN_BR_MSG_ROUTING_TABLE_UPDATE_IND_ID:
      app_handle_br_routing_table_update_ind(evt);
      break;
    default:
      printf("[Unknown event: %d]\r\n", evt->header.id);
  }

  app_wisun_cli_mutex_unlock();
}

void app_ping(sl_cli_command_arg_t *arguments)
{
  sl_status_t ret;
  char *arg_remote_address;
  app_icmpv6_echo_request_t *packet;
  uint16_t packet_data_length;
  uint8_t *packet_data = NULL;
  uint16_t payload_data_length;
  uint8_t *payload_data = NULL;
  uint32_t socket_option_value = SL_WISUN_SOCKET_EVENT_MODE_INDICATION;
  int32_t socket_retval = SOCKET_RETVAL_ERROR;
  uint8_t pattern_length = strlen(app_settings_ping.pattern);
  sockaddr_in6_t dest_addr = {
    .sin6_family   = AF_INET6,
    .sin6_port     = htons(APP_ICMPV6_PORT),
    .sin6_flowinfo = 0,
    .sin6_addr     = APP_IN6ADDR_ANY,
    .sin6_scope_id = 0
  };

  (void)arguments;

  app_wisun_cli_mutex_lock();

  arg_remote_address = sl_cli_get_argument_string(arguments, 0);
  ret = app_get_ip_address(&dest_addr.sin6_addr, arg_remote_address);
  if (ret != SL_STATUS_OK) {
    printf("[Failed: invalid remote address parameter]\r\n");
    goto cleanup;
  }

  if (app_ping_socket_id != SOCKET_INVALID_ID) {
    close(app_ping_socket_id);
    app_ping_socket_id = SOCKET_INVALID_ID;
  }

  app_ping_socket_id = socket(AF_INET6, (SOCK_RAW | SOCK_NONBLOCK), IPPROTO_ICMP);
  if (app_ping_socket_id == SOCKET_INVALID_ID) {
    printf("[Failed: unable to open a socket]\r\n");
    goto cleanup;
  }

  socket_retval = setsockopt(app_ping_socket_id,
                             APP_LEVEL_SOCKET,
                             SOCKET_EVENT_MODE,
                             &socket_option_value,
                             sizeof(uint32_t));
  if (socket_retval == SOCKET_RETVAL_ERROR) {
    printf("[Failed: unable to set SOCKET_EVENT_MODE socket option]\r\n");
    goto error_handler;
  }


  if (sl_cli_get_argument_count(arguments) == 2) {
    packet_data_length = sl_cli_get_argument_uint16(arguments, 1);
  } else {
    packet_data_length = app_settings_ping.packet_length;
  }

  if (packet_data_length > 2048) {
    // Larger than default socket size
    socket_option_value = packet_data_length;
    socket_retval = setsockopt(app_ping_socket_id,
                               SOL_SOCKET,
                               SO_SNDBUF,
                               &socket_option_value,
                               sizeof(uint32_t));
    if (socket_retval == SOCKET_RETVAL_ERROR) {
      printf("[Failed: unable to set SO_SNDBUF socket option]\r\n");
      goto error_handler;
    }
  }

  socket_option_value = app_settings_wisun.socket_rx_buffer_size;
  socket_retval = setsockopt(app_ping_socket_id,
                             SOL_SOCKET,
                             SO_RCVBUF,
                             &socket_option_value,
                             sizeof(uint32_t));
  if (socket_retval == SOCKET_RETVAL_ERROR) {
    printf("[Failed: unable to set SO_RCVBUF socket option]\r\n");
    goto error_handler;
  }

  payload_data_length = packet_data_length - sizeof(app_icmpv6_echo_request_t);
  packet_data = sl_malloc(packet_data_length);
  if (!packet_data) {
    printf("[Failed: unable to allocate memory for an ICMP packet]\r\n");
    goto error_handler;
  }

  // ICMPv6 Echo Request header, checksum is calculated by the stack
  packet = (app_icmpv6_echo_request_t *)packet_data;
  packet->type = APP_ICMPV6_TYPE_ECHO_REQUEST;
  packet->code = APP_ICMPV6_CODE_ECHO_REQUEST;
  packet->checksum = 0;
  packet->identifier = htons(app_settings_ping.identifier);
  packet->sequence_number = htons(app_settings_ping.sequence_number);

  // ICMPv6 Echo Request payload
  payload_data = packet->payload;
  while (payload_data_length) {
    if (payload_data_length < pattern_length) {
      memcpy(payload_data, app_settings_ping.pattern, payload_data_length);
      payload_data += payload_data_length;
      payload_data_length = 0;
    } else {
      memcpy(payload_data, app_settings_ping.pattern, pattern_length);
      payload_data += pattern_length;
      payload_data_length -= pattern_length;
    }
  }

  socket_retval = sendto(app_ping_socket_id,
                         packet_data,
                         packet_data_length,
                         0,
                         (const struct sockaddr *) &dest_addr,
                         sizeof(sockaddr_in6_t));

  if (socket_retval > 0) {
    app_ping_tick_count = sl_sleeptimer_get_tick_count();
    printf("PING %s: %u data bytes\r\n", app_get_ip_address_str(&dest_addr.sin6_addr), packet_data_length);
    goto cleanup;
  } else {
    printf("[Failed: unable to send an ICMP packet: %lu]\r\n", ret);
    goto error_handler;
  }

error_handler:

  if (app_ping_socket_id != SOCKET_INVALID_ID) {
    close(app_ping_socket_id);
    app_ping_socket_id = SOCKET_INVALID_ID;
  }

cleanup:

  sl_free(packet_data);
  packet_data = NULL;

  app_wisun_cli_mutex_unlock();
}

void app_tcp_client(sl_cli_command_arg_t *arguments)
{
  sl_status_t ret;
  app_socket_entry_t *entry;
  char *arg_remote_address;
  uint32_t socket_option_value = SL_WISUN_SOCKET_EVENT_MODE_INDICATION;
  int32_t socket_retval = SOCKET_RETVAL_ERROR;
  sockaddr_in6_t ipv6_remote_addr = {
    .sin6_family    = AF_INET6,
    .sin6_addr      = in6addr_any,
    .sin6_port      = 0,
    .sin6_flowinfo  = 0,
    .sin6_scope_id  = 0
  };

  app_wisun_cli_mutex_lock();

  // Parameters
  arg_remote_address = sl_cli_get_argument_string(arguments, 0);
  ret = app_get_ip_address(&ipv6_remote_addr.sin6_addr, arg_remote_address);
  if (ret != SL_STATUS_OK) {
    printf("[Failed: invalid remote address parameter]\r\n");
    goto cleanup;
  }

  ipv6_remote_addr.sin6_port = htons(sl_cli_get_argument_uint16(arguments, 1));
  if (!ipv6_remote_addr.sin6_port) {
    printf("[Failed: invalid remote port parameter]\r\n");
    goto cleanup;
  }

  entry = app_socket_alloc_entry();
  if (!entry) {
    printf("[Failed: unable to allocate a socket entry]\r\n");
    goto cleanup;
  }

  entry->socket_type = APP_SOCKET_TYPE_TCP_CLIENT;
  entry->socket_state = APP_SOCKET_STATE_CONNECTING;
  entry->remote_address = ipv6_remote_addr.sin6_addr;
  entry->remote_port = ntohs(ipv6_remote_addr.sin6_port);

  entry->socket_id = socket(AF_INET6, (SOCK_STREAM | SOCK_NONBLOCK), IPPROTO_TCP);
  if (entry->socket_id == SOCKET_INVALID_ID) {
    printf("[Failed: unable to open a socket]\r\n");
    goto error_handler;
  }

  socket_retval = setsockopt(entry->socket_id,
                             APP_LEVEL_SOCKET,
                             SOCKET_EVENT_MODE,
                             &socket_option_value,
                             sizeof(uint32_t));
  if (socket_retval == SOCKET_RETVAL_ERROR) {
    printf("[Failed: unable to set socket option]\r\n");
    goto error_handler;
  }

  socket_retval = connect(entry->socket_id, (const struct sockaddr *) &ipv6_remote_addr, sizeof(sockaddr_in6_t));
  if (socket_retval == SOCKET_RETVAL_ERROR && errno != EINPROGRESS) {
    printf("[Failed: unable to connect a socket: %ld]\r\n", socket_retval);
    goto error_handler;
  }

  // Push the entry to the active entries list
  sl_slist_push_back(&app_socket_entry_list, &entry->node);

  printf("[Opening: %s (%u): %d]\r\n",
          app_get_ip_address_str(&ipv6_remote_addr.sin6_addr),
          ntohs(ipv6_remote_addr.sin6_port),
          entry->socket_id);
  goto cleanup;

error_handler:

  if (entry->socket_id != SOCKET_INVALID_ID) {
    close(entry->socket_id);
  }
  app_socket_free_entry(entry);

cleanup:

  app_wisun_cli_mutex_unlock();

}

void app_tcp_server(sl_cli_command_arg_t *arguments)
{
  app_socket_entry_t *entry;
  uint32_t socket_option_value = SL_WISUN_SOCKET_EVENT_MODE_INDICATION;
  int32_t socket_retval = SOCKET_RETVAL_ERROR;
  sockaddr_in6_t app_sockaddr = {
    .sin6_family   = AF_INET6,
    .sin6_port     = 0,
    .sin6_flowinfo = 0,
    .sin6_addr     = APP_IN6ADDR_ANY,
    .sin6_scope_id = 0
  };

  app_wisun_cli_mutex_lock();

  // Parameters
  app_sockaddr.sin6_port = htons(sl_cli_get_argument_uint16(arguments, 0));
  if (!app_sockaddr.sin6_port) {
    printf("[Failed: invalid local port parameter]\r\n");
    goto cleanup;
  }

  entry = app_socket_alloc_entry();
  if (!entry) {
    printf("[Failed: unable to allocate a socket entry]\r\n");
    goto cleanup;
  }

  entry->socket_type = APP_SOCKET_TYPE_TCP_SERVER;
  entry->socket_state = APP_SOCKET_STATE_LISTENING;
  entry->remote_port = ntohs(app_sockaddr.sin6_port);

  entry->socket_id = socket(AF_INET6, (SOCK_STREAM | SOCK_NONBLOCK), IPPROTO_TCP);
  if (entry->socket_id == SOCKET_INVALID_ID) {
    printf("[Failed: unable to open a socket]\r\n");
    goto error_handler;
  }

  socket_retval = setsockopt(entry->socket_id,
                             APP_LEVEL_SOCKET,
                             SOCKET_EVENT_MODE,
                             &socket_option_value,
                             sizeof(uint32_t));
  if (socket_retval == SOCKET_RETVAL_ERROR) {
    printf("[Failed: unable to set socket option]\r\n");
    goto error_handler;
  }

  socket_retval = bind(entry->socket_id, (const struct sockaddr *) &app_sockaddr, sizeof(sockaddr_in6_t));
  if (socket_retval == SOCKET_RETVAL_ERROR) {
    printf("[Failed: unable to bind a socket: %lu]\r\n", socket_retval);
    goto error_handler;
  }

  socket_retval = listen(entry->socket_id, 0);
  if (socket_retval == SOCKET_RETVAL_ERROR) {
    printf("[Failed: unable to listen on a socket: %lu]\r\n", socket_retval);
    goto error_handler;
  }

  // Push the entry to the active entries list
  sl_slist_push_back(&app_socket_entry_list, &entry->node);

  printf("[Listening: %d]\r\n", entry->socket_id);
  goto cleanup;

error_handler:

  if (entry->socket_id != SOCKET_INVALID_ID) {
    close(entry->socket_id);
  }
  app_socket_free_entry(entry);

cleanup:

  app_wisun_cli_mutex_unlock();
}

void app_udp_client(sl_cli_command_arg_t *arguments)
{
  sl_status_t ret;
  app_socket_entry_t *entry;
  char *arg_remote_address;
  uint32_t socket_option_value = SL_WISUN_SOCKET_EVENT_MODE_INDICATION;
  int32_t socket_retval = SOCKET_RETVAL_ERROR;
  sockaddr_in6_t ipv6_remote_addr = {
    .sin6_family   = AF_INET6,
    .sin6_addr     = in6addr_any,
    .sin6_port     = 0,
    .sin6_flowinfo = 0,
    .sin6_scope_id = 0
  };

  app_wisun_cli_mutex_lock();

  // Parameters
  arg_remote_address = sl_cli_get_argument_string(arguments, 0);
  ret = app_get_ip_address(&ipv6_remote_addr.sin6_addr, arg_remote_address);
  if (ret != SL_STATUS_OK) {
    printf("[Failed: invalid remote address parameter]\r\n");
    goto cleanup;
  }

  ipv6_remote_addr.sin6_port = htons(sl_cli_get_argument_uint16(arguments, 1));
  if (!ipv6_remote_addr.sin6_port) {
    printf("[Failed: invalid remote port parameter]\r\n");
    goto cleanup;
  }

  entry = app_socket_alloc_entry();
  if (!entry) {
    printf("[Failed: unable to allocate a socket entry]\r\n");
    goto cleanup;
  }

  entry->socket_type = APP_SOCKET_TYPE_UDP_CLIENT;
  entry->socket_state = APP_SOCKET_STATE_ACTIVE;
  entry->remote_address = ipv6_remote_addr.sin6_addr;
  entry->remote_port = ntohs(ipv6_remote_addr.sin6_port);

  entry->socket_id = socket(AF_INET6, (SOCK_DGRAM | SOCK_NONBLOCK), IPPROTO_UDP);
  if (entry->socket_id == SOCKET_INVALID_ID) {
    printf("[Failed: unable to open a socket]\r\n");
    goto error_handler;
  }

  socket_retval = setsockopt(entry->socket_id,
                             APP_LEVEL_SOCKET,
                             SOCKET_EVENT_MODE,
                             &socket_option_value,
                             sizeof(uint32_t));
  if (socket_retval == SOCKET_RETVAL_ERROR) {
    printf("[Failed: unable to set socket option]\r\n");
    goto error_handler;
  }

  socket_retval = connect(entry->socket_id, (const struct sockaddr *) &ipv6_remote_addr, sizeof(sockaddr_in6_t));
  if (socket_retval == SOCKET_RETVAL_ERROR) {
    printf("[Failed: unable to connect a socket: %lu]\r\n", socket_retval);
    goto error_handler;
  }

  // Push the entry to the active entries list
  sl_slist_push_back(&app_socket_entry_list, &entry->node);

  printf("[Opened: %d]\r\n", entry->socket_id);
  goto cleanup;

error_handler:

  if (entry->socket_id != SOCKET_INVALID_ID) {
    close(entry->socket_id);
  }
  app_socket_free_entry(entry);

cleanup:

  app_wisun_cli_mutex_unlock();
}

void app_udp_server(sl_cli_command_arg_t *arguments)
{
  app_socket_entry_t *entry;
  uint32_t socket_option_value = SL_WISUN_SOCKET_EVENT_MODE_INDICATION;
  int32_t socket_retval = SOCKET_RETVAL_ERROR;
  sockaddr_in6_t app_sockaddr = {
    .sin6_family   = AF_INET6,
    .sin6_port     = 0,
    .sin6_flowinfo = 0,
    .sin6_addr     = APP_IN6ADDR_ANY,
    .sin6_scope_id = 0
  };

  app_wisun_cli_mutex_lock();

  // Parameters
  app_sockaddr.sin6_port = htons(sl_cli_get_argument_uint16(arguments, 0));
  if (!app_sockaddr.sin6_port) {
    printf("[Failed: invalid local port parameter]\r\n");
    goto cleanup;
  }

  entry = app_socket_alloc_entry();
  if (!entry) {
    printf("[Failed: unable to allocate a socket entry]\r\n");
    goto cleanup;
  }

  entry->socket_type = APP_SOCKET_TYPE_UDP_SERVER;
  entry->socket_state = APP_SOCKET_STATE_ACTIVE;
  entry->remote_port = ntohs(app_sockaddr.sin6_port);

  entry->socket_id = socket(AF_INET6, (SOCK_DGRAM | SOCK_NONBLOCK), IPPROTO_UDP);
  if (entry->socket_id ==  SOCKET_INVALID_ID) {
    printf("[Failed: unable to open a socket]\r\n");
    goto error_handler;
  }

  socket_retval = setsockopt(entry->socket_id,
                             APP_LEVEL_SOCKET,
                             SOCKET_EVENT_MODE,
                             &socket_option_value,
                             sizeof(uint32_t));
  if (socket_retval == SOCKET_RETVAL_ERROR) {
    printf("[Failed: unable to set socket option]\r\n");
    goto error_handler;
  }

  socket_retval = bind(entry->socket_id, (const struct sockaddr *) &app_sockaddr, sizeof(sockaddr_in6_t));
  if (socket_retval == SOCKET_RETVAL_ERROR) {
    printf("[Failed: unable to bind a socket: %ld]\r\n", socket_retval);
    goto error_handler;
  }

  // Push the entry to the active entries list
  sl_slist_push_back(&app_socket_entry_list, &entry->node);

  printf("[Listening: %d]\r\n", entry->socket_id);
  goto cleanup;

error_handler:

  if (entry->socket_id != SOCKET_INVALID_ID) {
    close(entry->socket_id);
  }
  app_socket_free_entry(entry);

cleanup:

  app_wisun_cli_mutex_unlock();
}

void app_socket_close(sl_cli_command_arg_t *arguments)
{
  app_socket_entry_t *entry;
  int socket_id;

  int32_t socket_retval = SOCKET_RETVAL_ERROR;

  app_wisun_cli_mutex_lock();

  // Parameters
  socket_id = (int)sl_cli_get_argument_uint32(arguments, 0);

  entry = app_socket_entry(socket_id);
  if (!entry) {
    printf("[Failed: unable to find the specified socket]\r\n");
    goto cleanup;
  }

  socket_retval = close(socket_id);
  if (socket_retval == SOCKET_RETVAL_ERROR) {
    printf("[Failed: unable to close a socket: %ld]\r\n", socket_retval);
    goto cleanup;
  }

  app_socket_free_entry(entry);

  printf("[Closed: %d]\r\n", socket_id);

cleanup:

  app_wisun_cli_mutex_unlock();
}

void app_socket_read(sl_cli_command_arg_t *arguments)
{
  app_socket_entry_t *entry;
  uint16_t data_length;
  uint8_t *data = NULL;
  int socket_id;
  app_printable_data_ctx_t printable_data_ctx;
  char *printable_data;

  int32_t socket_retval = SOCKET_RETVAL_ERROR;
  socklen_t addr_length = sizeof(sockaddr_in6_t);
  sockaddr_in6_t remote_address = {
    .sin6_family   = AF_INET6,
    .sin6_port     = 0,
    .sin6_flowinfo = 0,
    .sin6_addr     = APP_IN6ADDR_ANY,
    .sin6_scope_id = 0
  };

  app_wisun_cli_mutex_lock();

  // Parameters
  socket_id = (int)sl_cli_get_argument_uint32(arguments, 0);

  data_length = sl_cli_get_argument_uint16(arguments, 1);
  if (!data_length) {
    printf("[Failed: invalid amount of bytes to read]\r\n");
    goto cleanup;
  }

  data = sl_malloc(data_length);
  if (!data) {
    printf("[Failed: cannot allocate read buffer]\r\n");
    goto cleanup;
  }

  entry = app_socket_entry(socket_id);
  if (!entry) {
    printf("[Failed: unable to find the specified socket]\r\n");
    goto cleanup;
  }

  if (entry->socket_type == APP_SOCKET_TYPE_TCP_SERVER) {
    printf("[Failed: TCP server socket is not readable]\r\n");
    goto cleanup;
  }

  socket_retval = recvfrom(socket_id, data, data_length, 0, (struct sockaddr *) &remote_address, &addr_length);
  if (socket_retval == SOCKET_RETVAL_ERROR) {
    printf("[Failed: unable to read from a socket: %ld]\r\n", socket_retval);
    goto cleanup;
  }

  printf("[Data from %s (%u): %d,%ld",
         app_get_ip_address_str(&remote_address.sin6_addr),
         ntohs(remote_address.sin6_port),
         socket_id,
         socket_retval);

  if (app_settings_app.printable_data_length) {
    printable_data = app_util_printable_data_init(&printable_data_ctx,
                                                  data,
                                                  socket_retval,
                                                  app_settings_app.printable_data_as_hex,
                                                  app_settings_app.printable_data_length);
    while (printable_data) {
      printf("\r\n%s", printable_data);
      printable_data = app_util_printable_data_next(&printable_data_ctx);
    }
  }
  printf("]\r\n");

cleanup:
  if (data) {
    sl_free(data);
  }
  app_wisun_cli_mutex_unlock();
}

void app_socket_write(sl_cli_command_arg_t *arguments)
{
  app_socket_entry_t *entry;
  const char *data;
  int socket_id;

  int32_t socket_retval = SOCKET_RETVAL_ERROR;

  app_wisun_cli_mutex_lock();

  // Parameters
  socket_id = (int)sl_cli_get_argument_uint32(arguments, 0);

  data = sl_cli_get_argument_string(arguments, 1);
  if (!data || !strlen(data)) {
    printf("[Failed: invalid data to be written]\r\n");
    goto cleanup;
  }

  entry = app_socket_entry(socket_id);
  if (!entry) {
    printf("[Failed: unable to find the specified socket]\r\n");
    goto cleanup;
  }

  if (entry->socket_type == APP_SOCKET_TYPE_TCP_SERVER) {
    printf("[Failed: TCP server socket is not writable]\r\n");
    goto cleanup;
  }

  if (entry->socket_type == APP_SOCKET_TYPE_UDP_SERVER) {
    printf("[Failed: UDP server socket requires socket_writeto]\r\n");
    goto cleanup;
  }

  socket_retval = send(socket_id, (const void *) data, strlen(data), 0);
  if (socket_retval == SOCKET_RETVAL_ERROR) {
    printf("[Failed: unable to write to a socket: %ld]\r\n", socket_retval);
    goto cleanup;
  }

  printf("[Wrote %ld bytes]\r\n", socket_retval);

cleanup:

  app_wisun_cli_mutex_unlock();
}

void app_socket_writeto(sl_cli_command_arg_t *arguments)
{
  sl_status_t ret;
  app_socket_entry_t *entry;
  char *arg_remote_address;
  const char *data;
  int socket_id;

  int32_t socket_retval = SOCKET_RETVAL_ERROR;
  sockaddr_in6_t ipv6_dest_addr = {
    .sin6_family   = AF_INET6,
    .sin6_port     = 0,
    .sin6_flowinfo = 0,
    .sin6_addr     = APP_IN6ADDR_ANY,
    .sin6_scope_id = 0
  };

  app_wisun_cli_mutex_lock();

  // Parameters
  socket_id = (int)sl_cli_get_argument_uint32(arguments, 0);

  arg_remote_address = sl_cli_get_argument_string(arguments, 1);
  ret = app_get_ip_address(&ipv6_dest_addr.sin6_addr, arg_remote_address);
  if (ret != SL_STATUS_OK) {
    printf("[Failed: invalid remote address parameter]\r\n");
    goto cleanup;
  }

  ipv6_dest_addr.sin6_port = htons(sl_cli_get_argument_uint16(arguments, 2));
  if (!ipv6_dest_addr.sin6_port) {
    printf("[Failed: invalid remote port parameter]\r\n");
    goto cleanup;
  }

  data = sl_cli_get_argument_string(arguments, 3);
  if (!data || !strlen(data)) {
    printf("[Failed: invalid data to be written]\r\n");
    goto cleanup;
  }

  entry = app_socket_entry(socket_id);
  if (!entry) {
    printf("[Failed: unable to find the specified socket]\r\n");
    goto cleanup;
  }

  if (entry->socket_type == APP_SOCKET_TYPE_TCP_SERVER) {
    printf("[Failed: TCP server socket is not writable]\r\n");
    goto cleanup;
  }

  socket_retval = sendto(socket_id,
                         (const void *)data,
                         strlen(data),
                         0,
                         (const struct sockaddr *) &ipv6_dest_addr,
                         sizeof(sockaddr_in6_t));
  if (socket_retval == SOCKET_RETVAL_ERROR) {
    printf("[Failed: unable to write to a socket: %ld]\r\n", socket_retval);
    goto cleanup;
  }

  printf("[Wrote %ld bytes]\r\n", socket_retval);

cleanup:

  app_wisun_cli_mutex_unlock();
}

void app_socket_list(sl_cli_command_arg_t *arguments)
{
  app_socket_entry_t *entry = NULL;
  char info_str[64];
  (void)arguments;

  app_wisun_cli_mutex_lock();

  printf("!  ##  Type        Info\r\n");

  SL_SLIST_FOR_EACH_ENTRY(app_socket_entry_list, entry, app_socket_entry_t, node) {
    sprintf(info_str, "%s (%u) (%s)", app_get_ip_address_str(&entry->remote_address),
      entry->remote_port, APP_SOCKET_STATE_STR[entry->socket_state]);
    printf("#  %2d  %10s  %s\r\n", entry->socket_id, APP_SOCKET_TYPE_STR[entry->socket_type], info_str);
  }

  printf("!\r\n");

  app_wisun_cli_mutex_unlock();
}

void app_socket_set_option(sl_cli_command_arg_t *arguments)
{
  sl_status_t ret = SL_STATUS_OK;
  app_socket_entry_t *entry;
  int socket_id;
  char *arg_option;
  char *arg_option_data;
  const app_socket_option_t *iter;
  SL_ALIGN(4) app_socket_option_data_t option_data SL_ATTRIBUTE_ALIGN(4);

  int32_t socket_retval = SOCKET_RETVAL_ERROR;

  app_wisun_cli_mutex_lock();

  // Parameters
  socket_id = (int)sl_cli_get_argument_uint32(arguments, 0);

  arg_option = sl_cli_get_argument_string(arguments, 1);
  if (!arg_option || !strlen(arg_option)) {
    printf("[Failed: invalid option parameter]\r\n");
    goto cleanup;
  }

  arg_option_data = sl_cli_get_argument_string(arguments, 2);
  if (!arg_option || !strlen(arg_option)) {
    printf("[Failed: invalid option data parameter]\r\n");
    goto cleanup;
  }

  entry = app_socket_entry(socket_id);
  if (!entry) {
    printf("[Failed: unable to find the specified socket]\r\n");
    goto cleanup;
  }

  iter = app_set_socket_options;
  while (iter->option) {
    if (!strcmp(iter->option, arg_option)) {
      if (iter->handler) {
        ret = iter->handler(&option_data, arg_option_data);
        break;
      }
    }
    iter++;
  }

  if (!iter->option) {
    printf("[Failed: invalid option parameter]\r\n");
    goto cleanup;
  }

  if (ret != SL_STATUS_OK) {
    printf("[Failed: invalid option data parameter]\r\n");
    goto cleanup;
  }

  socket_retval = setsockopt(socket_id,
                             iter->option_level,
                             iter->option_name,
                             (void *) &option_data,
                             iter->option_length);

  if (socket_retval == SOCKET_RETVAL_ERROR) {
    printf("[Failed: unable to set socket option: %ld]\r\n", socket_retval);
    goto cleanup;
  }

  printf("[Socket option set]\r\n");

cleanup:

  app_wisun_cli_mutex_unlock();
}

static sl_status_t app_socket_event_mode_handler(app_socket_option_data_t *option_data,
                                                 const char *option_data_str)
{
  // The caller guarantees the aligment of the option data,
  // thus the warning can be ignored.
  #if defined __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wpragmas"
  #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
  #elif defined __ICCARM__
  #pragma diag_suppress=Pa039
  #endif

  return app_util_get_integer(&option_data->event_mode, option_data_str, app_socket_event_mode, false);

  // Restore the defaults
  #ifdef __GNUC__
  #pragma GCC diagnostic pop
  #elif defined __ICCARM__
  #pragma diag_default=Pa039
  #endif
}

static sl_status_t app_socket_nonblocking_mode_handler(app_socket_option_data_t *option_data,
                                                       const char *option_data_str)
{
  // The caller guarantees the aligment of the option data,
  // thus the warning can be ignored.
  #if defined __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wpragmas"
  #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
  #elif defined __ICCARM__
  #pragma diag_suppress=Pa039
  #endif

  return app_util_get_integer(&option_data->non_blocking_mode, option_data_str, app_socket_nonblock, false);

  // Restore the defaults
  #ifdef __GNUC__
  #pragma GCC diagnostic pop
  #elif defined __ICCARM__
  #pragma diag_default=Pa039
  #endif
}

static sl_status_t app_socket_receive_buff_size_handler(app_socket_option_data_t *option_data,
                                                        const char *option_data_str)
{
    // The caller guarantees the aligment of the option data,
  // thus the warning can be ignored.
  #if defined __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wpragmas"
  #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
  #elif defined __ICCARM__
  #pragma diag_suppress=Pa039
  #endif

  return app_util_get_integer((uint32_t *)&option_data->receive_buff_size, option_data_str, NULL, true);

  // Restore the defaults
  #ifdef __GNUC__
  #pragma GCC diagnostic pop
  #elif defined __ICCARM__
  #pragma diag_default=Pa039
  #endif
}

static sl_status_t app_socket_send_buffer_limit_handler(app_socket_option_data_t *option_data,
                                                        const char *option_data_str)
{
  // The caller guarantees the aligment of the option data,
  // thus the warning can be ignored.
  #if defined __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wpragmas"
  #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
  #elif defined __ICCARM__
  #pragma diag_suppress=Pa039
  #endif

  return app_util_get_integer((uint32_t *)&option_data->send_buffer_limit, option_data_str, NULL, true);

  // Restore the defaults
  #ifdef __GNUC__
  #pragma GCC diagnostic pop
  #elif defined __ICCARM__
  #pragma diag_default=Pa039
  #endif
}

static sl_status_t app_socket_unicast_hop_limit_handler(app_socket_option_data_t *option_data,
                                                        const char *option_data_str)
{
    // The caller guarantees the aligment of the option data,
  // thus the warning can be ignored.
  #if defined __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wpragmas"
  #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
  #elif defined __ICCARM__
  #pragma diag_suppress=Pa039
  #endif

  return app_util_get_integer((uint32_t *)&option_data->unicast_hop_limit, option_data_str, NULL, true);

  // Restore the defaults
  #ifdef __GNUC__
  #pragma GCC diagnostic pop
  #elif defined __ICCARM__
  #pragma diag_default=Pa039
  #endif
}

static sl_status_t app_socket_multicast_hop_limit_handler(app_socket_option_data_t *option_data,
                                                          const char *option_data_str)
{
    // The caller guarantees the aligment of the option data,
  // thus the warning can be ignored.
  #if defined __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wpragmas"
  #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
  #elif defined __ICCARM__
  #pragma diag_suppress=Pa039
  #endif

  return app_util_get_integer((uint32_t *)&option_data->multicast_hop_limit, option_data_str, NULL, true);

  // Restore the defaults
  #ifdef __GNUC__
  #pragma GCC diagnostic pop
  #elif defined __ICCARM__
  #pragma diag_default=Pa039
  #endif
}

static sl_status_t app_socket_multicast_group_handler(app_socket_option_data_t *option_data,
                                                      const char *option_data_str)
{
  // The caller guarantees the aligment of the option data,
  // thus the warning can be ignored.
  #ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wpragmas"
  #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
  #elif defined __ICCARM__
  #pragma diag_suppress=Pa039
  #endif

  if (!stoip6(option_data_str, strlen(option_data_str), option_data->multicast_group.ipv6mr_multiaddr.address)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  option_data->multicast_group.ipv6mr_ifindex = 0;

  return SL_STATUS_OK;

  // Restore the defaults
  #ifdef __GNUC__
  #pragma GCC diagnostic pop
  #elif defined __ICCARM__
  #pragma diag_default=Pa039
  #endif
}

void app_mac_allow(sl_cli_command_arg_t *arguments)
{
  sl_status_t ret;
  char *address_str = NULL;
  sl_wisun_mac_address_t address;
  const app_enum_t *value_enum;

  app_wisun_cli_mutex_lock();

  address_str = sl_cli_get_argument_string(arguments, 0);

  value_enum = app_util_get_enum_by_string(app_mac_enum, address_str);
  if (value_enum) {
    // Assume enumeration means a broadcast address
    address = APP_BROADCAST_MAC;
  } else {
    // Attempt to convert the MAC address string
    ret = app_util_get_mac_address(&address, address_str);
    if (ret != SL_STATUS_OK) {
      printf("[Failed: unable to parse the MAC address: %lu]\r\n", ret);
      goto cleanup;
    }
  }

  ret = sl_wisun_allow_mac_address(&address);
  if (ret != SL_STATUS_OK) {
    printf("[Failed: unable to add the MAC address to the access list: %lu]\r\n", ret);
    goto cleanup;
  }

  if (!memcmp(&address, &APP_BROADCAST_MAC, sizeof(sl_wisun_mac_address_t))) {
    printf("[Access list cleared]\r\n");
  } else {
    printf("[MAC address added to the access list]\r\n");
  }

cleanup:

  app_wisun_cli_mutex_unlock();
}

void app_mac_deny(sl_cli_command_arg_t *arguments)
{
  sl_status_t ret;
  char *address_str = NULL;
  sl_wisun_mac_address_t address;
  const app_enum_t *value_enum;

  app_wisun_cli_mutex_lock();

  address_str = sl_cli_get_argument_string(arguments, 0);

  value_enum = app_util_get_enum_by_string(app_mac_enum, address_str);
  if (value_enum) {
    // Assume enumeration means a broadcast address
    address = APP_BROADCAST_MAC;
  } else {
    // Attempt to convert the MAC address string
    ret = app_util_get_mac_address(&address, address_str);
    if (ret != SL_STATUS_OK) {
      printf("[Failed: unable to parse the MAC address: %lu]\r\n", ret);
      goto cleanup;
    }
  }

  ret = sl_wisun_deny_mac_address(&address);
  if (ret != SL_STATUS_OK) {
    printf("[Failed: unable to add the MAC address to the deny list: %lu]\r\n", ret);
    goto cleanup;
  }

  if (!memcmp(&address, &APP_BROADCAST_MAC, sizeof(sl_wisun_mac_address_t))) {
    printf("[Access list cleared]\r\n");
  } else {
    printf("[MAC address added to the deny list]\r\n");
  }

cleanup:

  app_wisun_cli_mutex_unlock();
}

void app_socket_get_option(sl_cli_command_arg_t *arguments)
{
  app_socket_entry_t *entry;
  int socket_id;
  char *arg_option;
  const app_socket_option_t *iter;
  SL_ALIGN(4) app_socket_option_data_t option_data SL_ATTRIBUTE_ALIGN(4);

  int32_t socket_retval = SOCKET_RETVAL_ERROR;
  socklen_t option_length = 0;

  app_wisun_cli_mutex_lock();

  if (sl_cli_get_argument_count(arguments) == 0) {
    iter = app_get_socket_options;
    printf("Options list :\r\n");
    while (iter->option) {
      printf("%s\r\n", iter->option);
      iter++;
    }
    goto cleanup;
  }

  if (sl_cli_get_argument_count(arguments) != 2) {
    printf("[Failed: invalid number of arguments]\r\n");
    goto cleanup;
  }

  // Parameters
  arg_option = sl_cli_get_argument_string(arguments, 0);
  if (sscanf(arg_option, "%d", &socket_id) != 1) {
    printf("[Failed: invalid option parameter]\r\n");
  }

  arg_option = sl_cli_get_argument_string(arguments, 1);
  if (!arg_option || !strlen(arg_option)) {
    printf("[Failed: invalid option parameter]\r\n");
    goto cleanup;
  }

  entry = app_socket_entry(socket_id);
  if (!entry) {
    printf("[Failed: unable to find the specified socket]\r\n");
    goto cleanup;
  }

  iter = app_get_socket_options;
  while (iter->option) {
    if (!strcmp(iter->option, arg_option)) {
      break;
    }
    iter++;
  }

  if (!iter->option) {
    printf("[Failed: invalid option parameter]\r\n");
    goto cleanup;
  }

  option_length = iter->option_length;
  socket_retval = getsockopt(socket_id,
                              iter->option_level,
                              iter->option_name,
                              (void *)&option_data,
                              &option_length);

  if (socket_retval == SOCKET_RETVAL_ERROR) {
    printf("[Failed: unable to get socket option: %ld]\r\n", socket_retval);
    goto cleanup;
  }

  // Every read socket option is int32_t.
  printf("%s = %"PRIi32"\r\n", arg_option, *(int32_t *)&option_data);

cleanup:

  app_wisun_cli_mutex_unlock();
}

void app_trigger_frame(sl_cli_command_arg_t *arguments)
{
  char *value_str;
  uint32_t frame_type;
  sl_status_t ret;

  app_wisun_cli_mutex_lock();

  value_str = sl_cli_get_argument_string(arguments, 0);
  if (value_str == NULL) {
    printf("[Failed: missing frame type]\r\n");
    goto cleanup;
  }

  ret = app_util_get_integer(&frame_type,
                             value_str,
                             app_frame_type_enum,
                             false);
  if (ret != SL_STATUS_OK) {
    printf("[Failed: invalid frame type: %s]\r\n", value_str);
    goto cleanup;
  }

  ret = sl_wisun_trigger_frame((sl_wisun_frame_type_t)frame_type);
  if (ret != SL_STATUS_OK) {
    printf("[Failed: unable to trigger frame %s: %lu]\r\n", value_str, ret);
    goto cleanup;
  }

  printf("[Frame %s triggered successfully]\r\n", value_str);

cleanup:

  app_wisun_cli_mutex_unlock();
}

void app_getsockname(sl_cli_command_arg_t *arguments)
{
  int ret;
  int sock_id;

  socklen_t address_length = sizeof(sockaddr_in6_t);
  sockaddr_in6_t local_address = {0};

  app_wisun_cli_mutex_lock();

  sock_id = sl_cli_get_argument_uint32(arguments, 0);

  ret = getsockname(sock_id, (struct sockaddr *) &local_address, &address_length);
  if (ret == -1) {
    printf("[Failed: invalid socket ID]\r\n");
  } else {
    printf("[local IPv6 address: %s, local port: %d]\r\n",
           app_get_ip_address_str(&local_address.sin6_addr), ntohs(local_address.sin6_port));
  }

  app_wisun_cli_mutex_unlock();
}

void app_getpeername(sl_cli_command_arg_t *arguments)
{
  int ret;
  int sock_id;

  socklen_t address_length = sizeof(sockaddr_in6_t);
  sockaddr_in6_t remote_address = {0};

  app_wisun_cli_mutex_lock();

  sock_id = sl_cli_get_argument_uint32(arguments, 0);

  ret = getpeername(sock_id, (struct sockaddr *) &remote_address, &address_length);
  if (ret == -1) {
    printf("[Failed: invalid socket ID]\r\n");
  } else {
    printf("[remote IPv6 address: %s, remote port: %d]\r\n",
           app_get_ip_address_str(&remote_address.sin6_addr), ntohs(remote_address.sin6_port));
  }

  app_wisun_cli_mutex_unlock();
}

void app_set_phy_sensitivity(sl_cli_command_arg_t *arguments)
{
  sl_status_t ret;
  uint8_t phy_mode_id;
  int16_t sensitivity;

  app_wisun_cli_mutex_lock();

  phy_mode_id = sl_cli_get_argument_uint8(arguments, 0);
  sensitivity = sl_cli_get_argument_int16(arguments, 1);

  ret = sl_wisun_set_phy_sensitivity(phy_mode_id, sensitivity);
  if (ret == SL_STATUS_OK) {
    printf("[PHY sensitivity set]\r\n");
  } else {
    printf("[Failed: unable to set PHY sensitivity: %lu]\r\n", ret);
  }

  app_wisun_cli_mutex_unlock();
}

void app_trigger_global_repair(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t ret;
  app_wisun_cli_mutex_lock();
  ret = sl_wisun_br_trigger_global_repair();
  if (ret == SL_STATUS_OK) {
    printf("[Global repair triggered]\r\n");
  } else {
    printf("[Failed: unable to trigger global repair: %lu]\r\n", ret);
  }
  app_wisun_cli_mutex_unlock();
}

void app_get_routing_table(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t ret;
  sl_wisun_br_routing_table_entry_t *routing_table = NULL;
  uint16_t routing_table_size = 0;

  app_wisun_cli_mutex_lock();

  ret = sl_wisun_br_get_routing_table_entry_count(&routing_table_size);
  if (ret != SL_STATUS_OK) {
    printf("[Failed: unable to get routing table entries count: %lu]\r\n", ret);
    goto cleanup;
  }

  if (routing_table_size != 0) {
    routing_table = sl_malloc(routing_table_size * sizeof(sl_wisun_br_routing_table_entry_t));
    if (!routing_table) {
      printf("[Failed: unable to allocate routing table]\r\n");
      goto cleanup;
    }

    memset(routing_table, 0, routing_table_size * sizeof(sl_wisun_br_routing_table_entry_t));

    ret = sl_wisun_br_get_routing_table(&routing_table_size, routing_table);
    if (ret != SL_STATUS_OK) {
      printf("[Failed: unable to get routing table: %lu]\r\n", ret);
      goto cleanup;
    }
  }

  printf("{  ##  %-39s  Parent\r\n", "Node");
  for (uint16_t i = 0; i < routing_table_size; i++) {
    printf("#%-4d  ", i);
    printf("%-39s", app_get_ip_address_str(&routing_table[i].target));
    printf("  %s", app_get_ip_address_str(&routing_table[i].preferred));

    if (memcmp(&routing_table[i].backup, &APP_IN6ADDR_ANY, sizeof(routing_table[i].backup)) != 0) {
      printf("\r\n");
      printf("#%-4d  ", i);
      printf("%-39s", app_get_ip_address_str(&routing_table[i].target));
      printf("  %s", app_get_ip_address_str(&routing_table[i].backup));
    }
    printf("\r\n");
  }
  printf("}\r\n");

cleanup:

  if (routing_table) {
    sl_free(routing_table);
  }

  app_wisun_cli_mutex_unlock();
}

void app_get_routing_table_from_index(sl_cli_command_arg_t *arguments)
{
  sl_status_t ret;
  sl_wisun_br_routing_table_entry_t *routing_table = NULL;
  uint16_t entry_count;
  uint16_t index;

  app_wisun_cli_mutex_lock();

  index = sl_cli_get_argument_uint16(arguments, 0);
  entry_count = sl_cli_get_argument_uint16(arguments, 1);

  if (entry_count != 0) {
    routing_table = sl_calloc(entry_count, sizeof(sl_wisun_br_routing_table_entry_t));
    if (!routing_table) {
      printf("[Failed: unable to allocate routing table]\r\n");
      goto cleanup;
    }

    ret = sl_wisun_br_get_routing_table_from_index(index, &entry_count, routing_table);
    if (ret != SL_STATUS_OK) {
      printf("[Failed: unable to get routing table: %lu]\r\n", ret);
      goto cleanup;
    }
  }

  printf("{  ##  %-39s  Parent\r\n", "Node");
  for (uint16_t i = 0; i < entry_count; i++) {
    printf("#%-4d  ", i);
    printf("%-39s", app_get_ip_address_str(&routing_table[i].target));
    printf("  %s", app_get_ip_address_str(&routing_table[i].preferred));

    if (memcmp(&routing_table[i].backup, &APP_IN6ADDR_ANY, sizeof(routing_table[i].backup)) != 0) {
      printf("\r\n");
      printf("#%-4d  ", i);
      printf("%-39s", app_get_ip_address_str(&routing_table[i].target));
      printf("  %s", app_get_ip_address_str(&routing_table[i].backup));
    }
    printf("\r\n");
  }
  printf("}\r\n");

cleanup:

  if (routing_table) {
    sl_free(routing_table);
  }

  app_wisun_cli_mutex_unlock();
}

void app_revoke_pairwise_keys(sl_cli_command_arg_t *arguments)
{
  sl_status_t ret;
  char *address_str = NULL;
  sl_wisun_mac_address_t address;
  const app_enum_t *value_enum;

  app_wisun_cli_mutex_lock();

  address_str = sl_cli_get_argument_string(arguments, 0);

  value_enum = app_util_get_enum_by_string(app_mac_enum, address_str);
  if (value_enum) {
    printf("[Failed: use a unicast address]\r\n");
    goto cleanup;
  } else {
    // Attempt to convert the MAC address string
    ret = app_util_get_mac_address(&address, address_str);
    if (ret != SL_STATUS_OK) {
      printf("[Failed: unable to parse the MAC address: %lu]\r\n", ret);
      goto cleanup;
    }
  }

  ret = sl_wisun_br_revoke_pairwise_keys(&address);
  if (ret == SL_STATUS_OK) {
    printf("[Pairwise keys revoked]\r\n");
  } else {
    printf("[Failed: unable to revoke pairwise keys: %lu]\r\n", ret);
  }

cleanup:
  app_wisun_cli_mutex_unlock();
}

void app_revoke_gtks(sl_cli_command_arg_t *arguments)
{
  sl_status_t ret;
  bool revoke_lgtks;
  char *gtk_str = NULL;
  uint8_t gtk_array[16];
  bool has_gtk = false;

  app_wisun_cli_mutex_lock();

  revoke_lgtks = (bool)sl_cli_get_argument_uint8(arguments, 0);

  if (sl_cli_get_argument_count(arguments) > 1) {
    // to get the second argument that is the specified gtk
    gtk_str = sl_cli_get_argument_string(arguments, 1);
    if (gtk_str == NULL) {
      printf("[Failed: invalid %sGTK string argument]\r\n", revoke_lgtks ? "L" : "");
      goto cleanup;
    }
    ret = app_util_get_byte_array(gtk_str, gtk_array, 16);
    if (ret != SL_STATUS_OK) {
      printf("[Failed: unable to parse the %sGTK string: %lu]\r\n", revoke_lgtks ? "L" : "", ret);
      goto cleanup;
    }
    has_gtk = true;
  }

  ret = sl_wisun_br_revoke_gtks(revoke_lgtks, has_gtk ? gtk_array : NULL);
  if (ret == SL_STATUS_OK) {
    printf("[%sGTKs revoked]\r\n", revoke_lgtks ? "L" : "");
  } else {
    printf("[Failed: unable to revoke %sGTKs: %lu]\r\n", revoke_lgtks ? "L" : "", ret);
  }

cleanup:
  app_wisun_cli_mutex_unlock();
}

void app_reset_duty_cycle(sl_cli_command_arg_t *arguments)
{
  sl_status_t status;
  (void)arguments;

  app_wisun_cli_mutex_lock();

  status = sl_wisun_reset_regulation_duty_cycle();
  if (status != SL_STATUS_OK) {
    printf("[Failed: unable to reset the duty cycle counters: %lu]\r\n", status);
    goto cleanup;
  }
  printf("[Duty cycle counters reset]\r\n");

cleanup:

  app_wisun_cli_mutex_unlock();
}
