/*
 * EVALUATION AND USE OF THIS SOFTWARE IS SUBJECT TO THE TERMS AND
 * CONDITIONS OF THE CONTROLLING LICENSE AGREEMENT FOUND AT LICENSE.md
 * IN THIS SDK. IF YOU DO NOT AGREE TO THE LICENSE TERMS AND CONDITIONS,
 * PLEASE RETURN ALL SOURCE FILES TO SILICON LABORATORIES.
 * (c) Copyright 2018, Silicon Laboratories Inc.  All rights reserved.
 */
#include "sl_net_for_lwip.h"
#include "sl_status.h"
#include "lwip/tcpip.h"
#include "lwip/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "lwip/ethip6.h"
#include "lwip/ip6_addr.h"
#include "lwip/timeouts.h"
#include "cmsis_os2.h"
#include "sl_cmsis_utility.h"
#include "sl_si91x_host_interface.h"
#include "sl_wifi.h"
#include "sl_utility.h"
#include "sl_wifi_callback_framework.h"
#include "sl_net_wifi_types.h"
#include "sli_net_common_utility.h"
#include "sli_wifi_constants.h"
#include "sl_net.h"
#include <string.h>
#include "sl_rsi_utility.h"
#include "sli_wifi_utility.h"
#include "sli_net_types.h"

// External reference to async state for DHCP completion
extern sli_net_async_if_state_t sli_async_state[];

#define MAC_48_BIT_SET       (1)
#define LWIP_FRAME_ALIGNMENT 60
#define STRUCT_PBUF          ((struct pbuf *)0)
#define INTERFACE_NAME_0     'w' ///< Network interface name 0
#define INTERFACE_NAME_1     'l' ///< Network interface name 1
#define MAX_TRANSFER_UNIT    1500
#define get_netif(i)         ((i & SL_WIFI_CLIENT_INTERFACE) ? &wifi_client_context->netif : &wifi_ap_context->netif)

sl_net_wifi_lwip_context_t *wifi_client_context = NULL;
sl_net_wifi_lwip_context_t *wifi_ap_context     = NULL;
uint32_t gOverrunCount                          = 0;
static bool lwip_initialized                    = false;

// Async DHCP monitoring state
typedef struct {
  bool active;                    // Whether DHCP monitoring is active
  sl_net_profile_id_t profile_id; // Profile ID for this async operation
#if !LWIP_NETIF_STATUS_CALLBACK
  osTimerId_t check_timer; // Timer for periodic DHCP check (fallback)
#endif
} sli_net_dhcp_monitor_state_t;

// Per-interface DHCP monitoring state array
static sli_net_dhcp_monitor_state_t dhcp_monitor_state[SL_NET_INTERFACE_MAX] = { 0 };
static osMutexId_t dhcp_monitor_mutex                                        = NULL;

#if !LWIP_NETIF_STATUS_CALLBACK
#define DHCP_CHECK_INTERVAL_MS 1000 // 1 second check interval (fallback mode)
#endif

// Forward declarations
#if LWIP_IPV4 && LWIP_DHCP
static void sli_handle_dhcp_completion(sl_net_interface_t interface);
#endif
static void update_profile_from_netif(sl_net_wifi_client_profile_t *profile);
static void configure_static_ip(const sl_net_wifi_client_profile_t *profile);
#if LWIP_NETIF_STATUS_CALLBACK
static void sli_netif_status_callback(struct netif *netif);
static sl_net_interface_t sli_get_interface_from_netif(struct netif *netif);
#else
static void sli_dhcp_check_callback(void *argument);
#endif

/******************************************************************************
                                Static Functions
******************************************************************************/
#if LWIP_NETIF_STATUS_CALLBACK
/**
 * @brief Helper to determine which interface a netif belongs to
 *
 * @param[in] netif Pointer to the network interface
 * @return sl_net_interface_t The interface identifier
 */
static sl_net_interface_t sli_get_interface_from_netif(struct netif *netif)
{
  if (wifi_client_context != NULL && netif == &(wifi_client_context->netif)) {
    return SL_NET_WIFI_CLIENT_INTERFACE;
  }
  if (wifi_ap_context != NULL && netif == &(wifi_ap_context->netif)) {
    return SL_NET_WIFI_AP_INTERFACE;
  }
  // Default to client interface if we can't determine
  return SL_NET_WIFI_CLIENT_INTERFACE;
}
#endif /* LWIP_NETIF_STATUS_CALLBACK */

static void low_level_init(struct netif *netif)
{
  uint32_t status = 0;
  sl_mac_address_t mac_addr;

  // set netif MAC hardware address length
  netif->hwaddr_len = ETH_HWADDR_LEN;

  // Request MAC address
  status = sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, &mac_addr);
  if (status != SL_STATUS_OK) {
    SL_DEBUG_LOG("\r\n MAC address failed \r\n");
    return;
  }

  // set netif MAC hardware address
  netif->hwaddr[0] = mac_addr.octet[0];
  netif->hwaddr[1] = mac_addr.octet[1];
  netif->hwaddr[2] = mac_addr.octet[2];
  netif->hwaddr[3] = mac_addr.octet[3];
  netif->hwaddr[4] = mac_addr.octet[4];
  netif->hwaddr[5] = mac_addr.octet[5];

  // set netif maximum transfer unit
  netif->mtu = MAX_TRANSFER_UNIT;

  // Accept broadcast address and ARP traffic
  netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP;

  // Set netif link flag
  // netif->flags |= NETIF_FLAG_LINK_UP;

#if LWIP_IPV6_MLD
  netif->flags |= NETIF_FLAG_MLD6;
#endif /* LWIP_IPV6_MLD */
}

static void low_level_input(struct netif *netif, uint8_t *b, uint16_t len)
{
  struct pbuf *p, *q;
  uint32_t bufferoffset;

  if (len <= 0) {
    return;
  }

  if (len < LWIP_FRAME_ALIGNMENT) { /* 60 : LWIP frame alignment */
    len = LWIP_FRAME_ALIGNMENT;
  }

  // Drop packets originated from the same interface and is not destined for the said interface
  const uint8_t *src_mac = b + netif->hwaddr_len;
  const uint8_t *dst_mac = b;

#if LWIP_IPV6
  if ((ip6_addr_istentative(netif_ip6_addr_state(netif, 0))) && (memcmp(netif->hwaddr, src_mac, netif->hwaddr_len) == 0)
      && (memcmp(netif->hwaddr, dst_mac, netif->hwaddr_len) != 0)) {
    SL_DEBUG_LOG("!!! [%02x:%02x:%02x:%02x:%02x:%02x]<-[%02x:%02x:%02x:%02x:%02x:%02x] type=%02x%02x\n",
                 // DESTINATION MAC
                 dst_mac[0],
                 dst_mac[1],
                 dst_mac[2],
                 dst_mac[3],
                 dst_mac[4],
                 dst_mac[5],
                 // SOURCE MAC
                 src_mac[0],
                 src_mac[1],
                 src_mac[2],
                 src_mac[3],
                 src_mac[4],
                 src_mac[5],
                 // ETH PKT TYPE
                 b[12],
                 b[13]);
    return;
  }
#endif

  /* We allocate a pbuf chain of pbufs from the Lwip buffer pool
   * and copy the data to the pbuf chain
   */
  if ((p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL)) != STRUCT_PBUF) {
    for (q = p, bufferoffset = 0; q != NULL; q = q->next) {
      memcpy((uint8_t *)q->payload, (uint8_t *)b + bufferoffset, q->len);
      bufferoffset += q->len;
    }

    SL_DEBUG_LOG("<<< (%03d): [%02x:%02x:%02x:%02x:%02x:%02x]<-[%02x:%02x:%02x:%02x:%02x:%02x] type=%02x%02x\n",
                 // PKT SIZE
                 bufferoffset,
                 // DESTINATION MAC
                 dst_mac[0],
                 dst_mac[1],
                 dst_mac[2],
                 dst_mac[3],
                 dst_mac[4],
                 dst_mac[5],
                 // SOURCE MAC
                 src_mac[0],
                 src_mac[1],
                 src_mac[2],
                 src_mac[3],
                 src_mac[4],
                 src_mac[5],
                 // ETH PKT TYPE
                 b[12],
                 b[13]);

    if (netif->input(p, netif) != ERR_OK) {
      gOverrunCount++;
      pbuf_free(p);
    }
  } else {
    gOverrunCount++;
  }

  return;
}

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
  UNUSED_PARAMETER(netif);
  sl_status_t status;

  // Extract and print the destination MAC address
  const uint8_t *dst_mac = (uint8_t *)p->payload;
  const uint8_t *src_mac = (uint8_t *)p->payload + netif->hwaddr_len;

  SL_DEBUG_LOG(">>> (%03d): [%02x:%02x:%02x:%02x:%02x:%02x]->[%02x:%02x:%02x:%02x:%02x:%02x]\n",
               // PKT SIZE
               p->len,
               // SOURCE MAC
               src_mac[0],
               src_mac[1],
               src_mac[2],
               src_mac[3],
               src_mac[4],
               src_mac[5],
               // DESTINATION MAC
               dst_mac[0],
               dst_mac[1],
               dst_mac[2],
               dst_mac[3],
               dst_mac[4],
               dst_mac[5]);

  status = sl_wifi_send_raw_data_frame(SL_WIFI_CLIENT_INTERFACE, (uint8_t *)p->payload, p->len);
  if (status != SL_STATUS_OK) {
    return ERR_IF;
  }
  return ERR_OK;
}

static err_t sta_ethernetif_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip_sta";
#endif /* LWIP_NETIF_HOSTNAME */
  //! Set the netif name to identify the interface
  netif->name[0] = INTERFACE_NAME_0;
  netif->name[1] = INTERFACE_NAME_1;

  //! Assign handler/function for the interface
  // netif->input      = tcpip_input;
#if LWIP_IPV4 && LWIP_ARP
  netif->output = etharp_output;
#endif /* #if LWIP_IPV4 && LWIP_ARP */
#if LWIP_IPV6 && LWIP_ETHERNET
  netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 && LWIP_ETHERNET */
  netif->linkoutput = low_level_output;

  //! initialize the hardware
  low_level_init(netif);
  //  sta_netif = *netif;

  return ERR_OK;
}

static void sta_netif_config(void)
{
#if LWIP_IPV4
  ip_addr_t sta_ipaddr;
  ip_addr_t sta_netmask;
  ip_addr_t sta_gw;

  /* Initialize the Station information */
  ip_addr_set_zero_ip4(&sta_ipaddr);
  ip_addr_set_zero_ip4(&sta_netmask);
  ip_addr_set_zero_ip4(&sta_gw);
#endif /* LWIP_IPV4 */

  /* Add station interfaces */
  netif_add(&(wifi_client_context->netif),
#if LWIP_IPV4
            (const ip4_addr_t *)&sta_ipaddr,
            (const ip4_addr_t *)&sta_netmask,
            (const ip4_addr_t *)&sta_gw,
#endif /* LWIP_IPV4 */
            NULL,
            &sta_ethernetif_init,
            &tcpip_input);

  /* Registers the default network interface */
  netif_set_default(&(wifi_client_context->netif));
}

static sl_status_t set_sta_link_up(sl_net_wifi_client_profile_t *profile)
{
  netifapi_netif_set_up(&(wifi_client_context->netif));
  netifapi_netif_set_link_up(&(wifi_client_context->netif));

  if (SL_IP_MANAGEMENT_STATIC_IP == profile->ip.mode) {
    configure_static_ip(profile);
  }

  if (SL_IP_MANAGEMENT_DHCP == profile->ip.mode) {
#if LWIP_IPV4 && LWIP_DHCP
    ip_addr_set_zero_ip4(&(wifi_client_context->netif.ip_addr));
    ip_addr_set_zero_ip4(&(wifi_client_context->netif.netmask));
    ip_addr_set_zero_ip4(&(wifi_client_context->netif.gw));
    dhcp_start(&(wifi_client_context->netif));
    //! Wait for DHCP to acquire IP Address
    while (!dhcp_supplied_address(&(wifi_client_context->netif))) {
      osDelay(SLI_SYSTEM_MS_TO_TICKS(100));
    }
    SL_DEBUG_LOG("DHCP IP: %s\n", ip4addr_ntoa((const ip4_addr_t *)&wifi_client_context->netif.ip_addr));
#endif /* LWIP_IPV4 && LWIP_DHCP */
       /*
      * Enable DHCPv6 with IPV6
      */

    // Stateless DHCPv6
#if LWIP_IPV6 && LWIP_IPV6_AUTOCONFIG
    // Automatically configure global addresses from Router Advertisements
    netif_set_ip6_autoconfig_enabled(&(wifi_client_context->netif), 1);
    // Create and set the link-local address
    netif_create_ip6_linklocal_address(&(wifi_client_context->netif), MAC_48_BIT_SET);
    SL_DEBUG_LOG("IPv6 Address %s\n", ip6addr_ntoa(netif_ip6_addr(&(wifi_client_context->netif), 0)));

    // Wait for the link-local address to up
    while (ip6_addr_istentative(netif_ip6_addr_state(&(wifi_client_context->netif), 0))) {
      osDelay(200);
    }

    if (!ip6_addr_ispreferred(netif_ip6_addr_state(&(wifi_client_context->netif), 0))) {
      return SL_STATUS_SI91X_IP_ADDRESS_ERROR;
    }
#endif /* LWIP_IPV6 && LWIP_IPV6_AUTOCONFIG */
  }

  return SL_STATUS_OK;
}

static void set_sta_link_down(void)
{
#if LWIP_IPV4 && LWIP_DHCP
  SL_DEBUG_LOG("DHCP Link down\n");
  dhcp_stop(&(wifi_client_context->netif));
#endif /* LWIP_IPV4 && LWIP_DHCP */
  netifapi_netif_set_link_down(&(wifi_client_context->netif));
  netifapi_netif_set_down(&(wifi_client_context->netif));
}

/**
 * @brief Helper: Update profile with IP addresses from netif
 *
 * @param[in,out] profile Profile to update with IP addresses
 */
static void update_profile_from_netif(sl_net_wifi_client_profile_t *profile)
{
  if (wifi_client_context == NULL || profile == NULL) {
    return;
  }

#if LWIP_IPV4 && LWIP_IPV6
  if ((profile->ip.type & SL_IPV4) == SL_IPV4) {
    ip_addr_t *addr;
    addr = &wifi_client_context->netif.ip_addr;
    memcpy(profile->ip.ip.v4.ip_address.bytes, &addr->u_addr.ip4.addr, sizeof(addr->u_addr.ip4.addr));
    addr = &wifi_client_context->netif.gw;
    memcpy(profile->ip.ip.v4.gateway.bytes, &addr->u_addr.ip4.addr, sizeof(addr->u_addr.ip4.addr));
    addr = &wifi_client_context->netif.netmask;
    memcpy(profile->ip.ip.v4.netmask.bytes, &addr->u_addr.ip4.addr, sizeof(addr->u_addr.ip4.addr));
  }
  if ((profile->ip.type & SL_IPV6) == SL_IPV6) {
    for (int i = 0; i < 4; i++) {
      profile->ip.ip.v6.link_local_address.value[i] = ntohl(wifi_client_context->netif.ip6_addr[0].u_addr.ip6.addr[i]);
      profile->ip.ip.v6.global_address.value[i]     = ntohl(wifi_client_context->netif.ip6_addr[1].u_addr.ip6.addr[i]);
      profile->ip.ip.v6.gateway.value[i]            = ntohl(wifi_client_context->netif.ip6_addr[2].u_addr.ip6.addr[i]);
    }
  }
#else /* LWIP_IPV4 && LWIP_IPV6 */
#if LWIP_IPV4
  u32_t *addr;
  addr = &wifi_client_context->netif.ip_addr.addr;
  memcpy(profile->ip.ip.v4.ip_address.bytes, addr, sizeof(*addr));
  addr = &wifi_client_context->netif.gw.addr;
  memcpy(profile->ip.ip.v4.gateway.bytes, addr, sizeof(*addr));
  addr = &wifi_client_context->netif.netmask.addr;
  memcpy(profile->ip.ip.v4.netmask.bytes, addr, sizeof(*addr));
#elif LWIP_IPV6
  for (int i = 0; i < 4; i++) {
    profile->ip.ip.v6.link_local_address.value[i] = ntohl(wifi_client_context->netif.ip6_addr[0].addr[i]);
    profile->ip.ip.v6.global_address.value[i]     = ntohl(wifi_client_context->netif.ip6_addr[1].addr[i]);
    profile->ip.ip.v6.gateway.value[i]            = ntohl(wifi_client_context->netif.ip6_addr[2].addr[i]);
  }
#endif /* LWIP_IPV6 */
#endif /* LWIP_IPV4 && LWIP_IPV6 */
}

/**
 * @brief Helper: Configure static IP address on netif
 *
 * @param[in] profile Profile containing static IP configuration
 */
static void configure_static_ip(const sl_net_wifi_client_profile_t *profile)
{
  if (wifi_client_context == NULL || profile == NULL) {
    return;
  }

#if LWIP_IPV4 && LWIP_IPV6
  ip_addr_t ipaddr  = { 0 };
  ip_addr_t gateway = { 0 };
  ip_addr_t netmask = { 0 };

  if ((profile->ip.type & SL_IPV4) == SL_IPV4) {
    uint8_t *address = (uint8_t *)&(profile->ip.ip.v4.ip_address.bytes[0]);
    IP4_ADDR(&ipaddr.u_addr.ip4, address[0], address[1], address[2], address[3]);
    address = (uint8_t *)&(profile->ip.ip.v4.gateway.bytes[0]);
    IP4_ADDR(&gateway.u_addr.ip4, address[0], address[1], address[2], address[3]);
    address = (uint8_t *)&(profile->ip.ip.v4.netmask.bytes[0]);
    IP4_ADDR(&netmask.u_addr.ip4, address[0], address[1], address[2], address[3]);

    netifapi_netif_set_addr(&(wifi_client_context->netif),
                            &ipaddr.u_addr.ip4,
                            &netmask.u_addr.ip4,
                            &gateway.u_addr.ip4);
  }
  if ((profile->ip.type & SL_IPV6) == SL_IPV6) {
    uint32_t *address = (uint32_t *)&(profile->ip.ip.v6.link_local_address.value[0]);
    IP6_ADDR(&ipaddr.u_addr.ip6, address[0], address[1], address[2], address[3]);
    address = (uint32_t *)&(profile->ip.ip.v6.global_address.value[0]);
    IP6_ADDR(&gateway.u_addr.ip6, address[0], address[1], address[2], address[3]);
    address = (uint32_t *)&(profile->ip.ip.v6.gateway.value[0]);
    IP6_ADDR(&netmask.u_addr.ip6, address[0], address[1], address[2], address[3]);

    netif_ip6_addr_set(&(wifi_client_context->netif), 0, &ipaddr.u_addr.ip6);
    netif_ip6_addr_set(&(wifi_client_context->netif), 1, &gateway.u_addr.ip6);
    netif_ip6_addr_set(&(wifi_client_context->netif), 2, &netmask.u_addr.ip6);

    netif_ip6_addr_set_state(&(wifi_client_context->netif), 0, IP6_ADDR_PREFERRED);
    netif_ip6_addr_set_state(&(wifi_client_context->netif), 1, IP6_ADDR_PREFERRED);
    netif_ip6_addr_set_state(&(wifi_client_context->netif), 2, IP6_ADDR_PREFERRED);
  }
#elif LWIP_IPV4
  ip4_addr_t ipaddr  = { 0 };
  ip4_addr_t gateway = { 0 };
  ip4_addr_t netmask = { 0 };
  uint8_t *address   = (uint8_t *)&(profile->ip.ip.v4.ip_address.bytes[0]);

  IP4_ADDR(&ipaddr, address[0], address[1], address[2], address[3]);
  address = (uint8_t *)&(profile->ip.ip.v4.gateway.bytes[0]);
  IP4_ADDR(&gateway, address[0], address[1], address[2], address[3]);
  address = (uint8_t *)&(profile->ip.ip.v4.netmask.bytes[0]);
  IP4_ADDR(&netmask, address[0], address[1], address[2], address[3]);

  netifapi_netif_set_addr(&(wifi_client_context->netif), &ipaddr, &netmask, &gateway);
#elif LWIP_IPV6
  ip6_addr_t link_local_address = { 0 };
  ip6_addr_t global_address     = { 0 };
  ip6_addr_t gateway            = { 0 };
  uint32_t *address             = (uint32_t *)&(profile->ip.ip.v6.link_local_address.value[0]);

  IP6_ADDR(&link_local_address, address[0], address[1], address[2], address[3]);
  address = (uint32_t *)&(profile->ip.ip.v6.global_address.value[0]);
  IP6_ADDR(&global_address, address[0], address[1], address[2], address[3]);
  address = (uint32_t *)&(profile->ip.ip.v6.gateway.value[0]);
  IP6_ADDR(&gateway, address[0], address[1], address[2], address[3]);
  netif_ip6_addr_set(&(wifi_client_context->netif), 0, &link_local_address);
  netif_ip6_addr_set(&(wifi_client_context->netif), 1, &global_address);
  netif_ip6_addr_set(&(wifi_client_context->netif), 2, &gateway);

  netif_ip6_addr_set_state(&(wifi_client_context->netif), 0, IP6_ADDR_PREFERRED);
  netif_ip6_addr_set_state(&(wifi_client_context->netif), 1, IP6_ADDR_PREFERRED);
  netif_ip6_addr_set_state(&(wifi_client_context->netif), 2, IP6_ADDR_PREFERRED);
#endif /* LWIP_IPV6 */
}

/**
 * @brief Brings up the network interface with IP configuration
 *
 * @details
 *   For static IP: Configures the interface immediately and returns
 *     SL_STATUS_OK.
 *   For DHCP: Starts DHCP client and returns SL_STATUS_IN_PROGRESS to
 *     indicate async operation.
 *
 * @param[in] profile Pointer to the WiFi client profile containing IP
 *                    configuration
 *
 * @return sl_status_t
 *   - SL_STATUS_OK: Static IP configured successfully
 *   - SL_STATUS_IN_PROGRESS: DHCP started, will complete asynchronously
 *   - Other: Error code indicating failure
 */
static sl_status_t set_sta_link_up_async(sl_net_wifi_client_profile_t *profile)
{
  if (wifi_client_context == NULL) {
    SL_DEBUG_LOG("WiFi client context is NULL\n");
    return SL_STATUS_FAIL;
  }

  netifapi_netif_set_up(&(wifi_client_context->netif));
  netifapi_netif_set_link_up(&(wifi_client_context->netif));

  if (SL_IP_MANAGEMENT_STATIC_IP == profile->ip.mode) {
    configure_static_ip(profile);
    // Static IP configured immediately - async operation complete
    return SL_STATUS_OK;
  }

  if (SL_IP_MANAGEMENT_DHCP == profile->ip.mode) {
    bool async_started = false;

    // Start IPv4 DHCP if IPv4 is configured
    if (profile->ip.type & SL_IPV4) {
#if LWIP_IPV4 && LWIP_DHCP
      ip_addr_set_zero_ip4(&(wifi_client_context->netif.ip_addr));
      ip_addr_set_zero_ip4(&(wifi_client_context->netif.netmask));
      ip_addr_set_zero_ip4(&(wifi_client_context->netif.gw));
      dhcp_start(&(wifi_client_context->netif));
      SL_DEBUG_LOG("DHCP started asynchronously\n");
      async_started = true;
#else
      SL_DEBUG_LOG("IPv4 DHCP requested but not supported (LWIP_IPV4 and LWIP_DHCP not enabled)\n");
      return SL_STATUS_NOT_SUPPORTED;
#endif /* LWIP_IPV4 && LWIP_DHCP */
    }

    // Start IPv6 autoconfig if IPv6 is configured
    if (profile->ip.type & SL_IPV6) {
#if LWIP_IPV6 && LWIP_IPV6_AUTOCONFIG
      // Automatically configure global addresses from Router Advertisements
      netif_set_ip6_autoconfig_enabled(&(wifi_client_context->netif), 1);
      // Create and set the link-local address
      netif_create_ip6_linklocal_address(&(wifi_client_context->netif), MAC_48_BIT_SET);
      SL_DEBUG_LOG("IPv6 autoconfig started - Address %s\n",
                   ip6addr_ntoa(netif_ip6_addr(&(wifi_client_context->netif), 0)));
      async_started = true;
#else
      SL_DEBUG_LOG("IPv6 autoconfig requested but not supported (LWIP_IPV6 and LWIP_IPV6_AUTOCONFIG not enabled)\n");
      if (!async_started) {
        return SL_STATUS_NOT_SUPPORTED;
      }
#endif /* LWIP_IPV6 && LWIP_IPV6_AUTOCONFIG */
    }

    // Return IN_PROGRESS if either IPv4 DHCP or IPv6 autoconfig was started
    if (async_started) {
      return SL_STATUS_IN_PROGRESS;
    }
  }

  return SL_STATUS_OK;
}

/**
 * @brief Handles DHCP completion and notifies the application
 *
 * @details
 *   This function is called when DHCP successfully obtains an IP address.
 *   It updates the network profile with the assigned IP configuration,
 *   cleans up monitoring resources (callbacks or timers), and sends
 *   SL_NET_IP_CONFIG_EVENT to the application.
 *
 * @param[in] interface The network interface for which DHCP completed
 *
 * @note This function is called by both event-driven (netif callback)
 *       and timer-based (periodic check) DHCP monitoring mechanisms.
 */
#if LWIP_IPV4 && LWIP_DHCP
static void sli_handle_dhcp_completion(sl_net_interface_t interface)
{
  if (wifi_client_context == NULL) {
    SL_DEBUG_LOG("WiFi client context is NULL\n");
    return;
  }

  // Validate interface index
  if (interface >= SL_NET_INTERFACE_MAX) {
    SL_DEBUG_LOG("Invalid interface index: %d\n", interface);
    return;
  }

  SL_DEBUG_LOG("DHCP completed successfully for interface %d\n", interface);

  // Get the profile and update it with the DHCP-assigned IP
  sl_net_wifi_client_profile_t profile = { 0 };
  sl_status_t status = sl_net_get_profile(interface, dhcp_monitor_state[interface].profile_id, &profile);

  if (status == SL_STATUS_OK) {
    // Update profile with DHCP-assigned addresses
    update_profile_from_netif(&profile);

    // Set the updated profile
    sl_net_set_profile(interface, dhcp_monitor_state[interface].profile_id, &profile);

    SL_DEBUG_LOG("DHCP IP: %s\n", ip4addr_ntoa((const ip4_addr_t *)&wifi_client_context->netif.ip_addr));
  }

  // Cleanup: Stop monitoring and cleanup resources
  // Note: active flag is already cleared by caller (atomic check-and-set)
#if LWIP_NETIF_STATUS_CALLBACK
  // Clear the netif status callback
  netif_set_status_callback(&(wifi_client_context->netif), NULL);
#else
  // Stop and delete the check timer
  if (dhcp_monitor_mutex != NULL) {
    osMutexAcquire(dhcp_monitor_mutex, osWaitForever);
  }
  if (dhcp_monitor_state[interface].check_timer != NULL) {
    osTimerStop(dhcp_monitor_state[interface].check_timer);
    osTimerDelete(dhcp_monitor_state[interface].check_timer);
    dhcp_monitor_state[interface].check_timer = NULL;
  }
  if (dhcp_monitor_mutex != NULL) {
    osMutexRelease(dhcp_monitor_mutex);
  }
#endif

  // Update async state to CONNECTED
  sli_async_state[interface].state = SLI_NET_ASYNC_CONNECTED;

  // Notify IP_CONFIG_EVENT with success to app
  sli_notify_net_event_handler(SL_NET_IP_CONFIG_EVENT, SL_STATUS_OK, NULL, 0);
}
#endif /* LWIP_IPV4 && LWIP_DHCP */

#if LWIP_NETIF_STATUS_CALLBACK
/**
 * @brief lwIP network interface status callback for DHCP monitoring
 *
 * @details
 *   This callback is registered with lwIP's netif_set_status_callback()
 *   and is invoked whenever the network interface status changes.
 *   It checks if DHCP has completed and calls sli_handle_dhcp_completion()
 *   if an IP address has been assigned.
 *
 * @param[in] netif Pointer to the network interface
 *
 * @note Only compiled when LWIP_NETIF_STATUS_CALLBACK is enabled
 */
static void sli_netif_status_callback(struct netif *netif)
{
#if LWIP_IPV4 && LWIP_DHCP
  // Determine which interface this netif belongs to
  sl_net_interface_t interface = sli_get_interface_from_netif(netif);

  // Validate interface index
  if (interface >= SL_NET_INTERFACE_MAX) {
    return;
  }

  // Check if DHCP has completed (IP address is assigned and not 0.0.0.0)
  if (!dhcp_supplied_address(netif)) {
    return; // DHCP not yet complete
  }

  // Atomic check-and-set: only the first callback clears the active flag
  bool should_handle = false;
  if (dhcp_monitor_mutex != NULL) {
    osMutexAcquire(dhcp_monitor_mutex, osWaitForever);
    if (dhcp_monitor_state[interface].active) {
      dhcp_monitor_state[interface].active = false; // Clear flag atomically
      should_handle                        = true;
    }
    osMutexRelease(dhcp_monitor_mutex);
  } else {
    // No mutex available - use simple check (non-thread-safe fallback)
    if (dhcp_monitor_state[interface].active) {
      dhcp_monitor_state[interface].active = false;
      should_handle                        = true;
    }
  }

  // Only the first callback to clear the flag proceeds
  if (should_handle) {
    sli_handle_dhcp_completion(interface);
  }
#endif /* LWIP_IPV4 && LWIP_DHCP */
}
#else /* !LWIP_NETIF_STATUS_CALLBACK */
/**
 * @brief Periodic timer callback for DHCP completion monitoring (fallback)
 *
 * @details
 *   This callback is used when LWIP_NETIF_STATUS_CALLBACK is not available.
 *   It's invoked periodically by a CMSIS-RTOS timer to check if DHCP has
 *   completed. If DHCP has obtained an IP address, it calls
 *   sli_handle_dhcp_completion().
 *
 * @param[in] argument Timer argument containing the interface ID
 *
 * @note Only compiled when LWIP_NETIF_STATUS_CALLBACK is NOT enabled
 */
static void sli_dhcp_check_callback(void *argument)
{
  // Suppress unused parameter warning when DHCP is disabled at build time
  (void)argument;
#if LWIP_IPV4 && LWIP_DHCP
  // Extract interface from timer argument
  sl_net_interface_t interface = (sl_net_interface_t)(uintptr_t)argument;

  // Validate interface index
  if (interface >= SL_NET_INTERFACE_MAX) {
    return;
  }

  if (wifi_client_context == NULL) {
    return;
  }

  // Check if DHCP has completed
  if (!dhcp_supplied_address(&(wifi_client_context->netif))) {
    return; // DHCP not yet complete
  }

  // Atomic check-and-set: only the first callback clears the active flag
  bool should_handle = false;
  if (dhcp_monitor_mutex != NULL) {
    osMutexAcquire(dhcp_monitor_mutex, osWaitForever);
    if (dhcp_monitor_state[interface].active) {
      dhcp_monitor_state[interface].active = false; // Clear flag atomically
      should_handle                        = true;
    }
    osMutexRelease(dhcp_monitor_mutex);
  } else {
    // No mutex available - use simple check (non-thread-safe fallback)
    if (dhcp_monitor_state[interface].active) {
      dhcp_monitor_state[interface].active = false;
      should_handle                        = true;
    }
  }

  // Only the first callback to clear the flag proceeds
  if (should_handle) {
    sli_handle_dhcp_completion(interface);
  }
#endif /* LWIP_IPV4 && LWIP_DHCP */
}
#endif /* LWIP_NETIF_STATUS_CALLBACK */

/**
 * @brief Starts asynchronous IP configuration for a network interface
 *
 * @details
 *   This function retrieves the network profile and initiates IP
 *   configuration. For static IP, configuration completes immediately.
 *   For DHCP, it starts the DHCP client and sets up monitoring using
 *   either event-driven callbacks or periodic timers depending on
 *   LWIP_NETIF_STATUS_CALLBACK availability.
 *
 * @param[in] interface Network interface to configure
 * @param[in] profile_id Profile ID containing IP configuration settings
 *
 * @return sl_status_t
 *   - SL_STATUS_OK: Static IP configured successfully
 *   - SL_STATUS_IN_PROGRESS: DHCP started, will complete asynchronously
 *   - Other: Error code from profile retrieval or IP configuration
 */
sl_status_t sli_start_async_ip_config(sl_net_interface_t interface, sl_net_profile_id_t profile_id)
{
  // Validate interface bounds to prevent buffer overflow
  if (interface >= SL_NET_INTERFACE_MAX) {
    SL_DEBUG_LOG("Invalid interface index: %d\n", interface);
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (wifi_client_context == NULL) {
    SL_DEBUG_LOG("WiFi client context is NULL\n");
    return SL_STATUS_FAIL;
  }

  // Get the profile
  sl_net_wifi_client_profile_t profile = { 0 };
  sl_status_t status                   = sl_net_get_profile(interface, profile_id, &profile);
  if (status != SL_STATUS_OK) {
    SL_DEBUG_LOG("Failed to get profile: 0x%lx\n", status);
    return status;
  }

  // Start IP configuration (async for DHCP, immediate for static)
  status = set_sta_link_up_async(&profile);

  if (status == SL_STATUS_OK) {
    // Static IP configured successfully - update profile
    update_profile_from_netif(&profile);

    // Update the profile with configured IP
    sl_net_set_profile(interface, profile_id, &profile);
    return SL_STATUS_OK;

  } else if (status == SL_STATUS_IN_PROGRESS) {
    // DHCP started - set up monitoring (thread-safe)
    if (dhcp_monitor_mutex != NULL) {
      osMutexAcquire(dhcp_monitor_mutex, osWaitForever);
    }
    dhcp_monitor_state[interface].active     = true;
    dhcp_monitor_state[interface].profile_id = profile_id;
    if (dhcp_monitor_mutex != NULL) {
      osMutexRelease(dhcp_monitor_mutex);
    }

#if LWIP_NETIF_STATUS_CALLBACK
    // Event-driven: Register netif status callback
    netif_set_status_callback(&(wifi_client_context->netif), sli_netif_status_callback);
    SL_DEBUG_LOG("DHCP monitoring started (event-driven) for interface %d\n", interface);
#else
    // Fallback: Create periodic check timer
    const osTimerAttr_t timer_attr = { .name = "dhcp_check" };
    // Pass interface as timer argument
    dhcp_monitor_state[interface].check_timer =
      osTimerNew(sli_dhcp_check_callback, osTimerPeriodic, (void *)(uintptr_t)interface, &timer_attr);
    if (dhcp_monitor_state[interface].check_timer == NULL) {
      SL_DEBUG_LOG("Failed to create DHCP check timer for interface %d\n", interface);
      // Cleanup: Stop DHCP since we can't monitor it
#if LWIP_IPV4 && LWIP_DHCP
      dhcp_stop(&(wifi_client_context->netif));
#endif
      if (dhcp_monitor_mutex != NULL) {
        osMutexAcquire(dhcp_monitor_mutex, osWaitForever);
      }
      dhcp_monitor_state[interface].active = false;
      if (dhcp_monitor_mutex != NULL) {
        osMutexRelease(dhcp_monitor_mutex);
      }
      return SL_STATUS_FAIL;
    }
    osStatus_t timer_status = osTimerStart(dhcp_monitor_state[interface].check_timer, DHCP_CHECK_INTERVAL_MS);
    if (timer_status != osOK) {
      SL_DEBUG_LOG("Failed to start DHCP check timer: %d\n", timer_status);
      osTimerDelete(dhcp_monitor_state[interface].check_timer);
      dhcp_monitor_state[interface].check_timer = NULL;
#if LWIP_IPV4 && LWIP_DHCP
      dhcp_stop(&(wifi_client_context->netif));
#endif
      if (dhcp_monitor_mutex != NULL) {
        osMutexAcquire(dhcp_monitor_mutex, osWaitForever);
      }
      dhcp_monitor_state[interface].active = false;
      if (dhcp_monitor_mutex != NULL) {
        osMutexRelease(dhcp_monitor_mutex);
      }
      return SL_STATUS_FAIL;
    }
    SL_DEBUG_LOG("DHCP monitoring started (timer-based, %dms) for interface %d\n", DHCP_CHECK_INTERVAL_MS, interface);
#endif

    return SL_STATUS_IN_PROGRESS;
  }

  return status;
}

/******************************************************************************
                                Public Functions
******************************************************************************/
sl_status_t sl_net_wifi_ap_init(sl_net_interface_t interface,
                                const void *configuration,
                                const void *workspace,
                                sl_net_event_handler_t event_handler)
{
  UNUSED_PARAMETER(interface);
  UNUSED_PARAMETER(configuration);
  UNUSED_PARAMETER(workspace);
  UNUSED_PARAMETER(event_handler);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_net_wifi_ap_deinit(sl_net_interface_t interface)
{
  UNUSED_PARAMETER(interface);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_net_wifi_ap_up(sl_net_interface_t interface, sl_net_profile_id_t profile_id)
{
  UNUSED_PARAMETER(interface);
  UNUSED_PARAMETER(profile_id);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_net_wifi_ap_down(sl_net_interface_t interface)
{
  UNUSED_PARAMETER(interface);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_net_wifi_client_init(sl_net_interface_t interface,
                                    const void *configuration,
                                    void *context,
                                    sl_net_event_handler_t event_handler)
{
  UNUSED_PARAMETER(interface);

  // Register the event handler for network events
  sli_net_register_event_handler(event_handler);

  sl_status_t status = sl_wifi_init(configuration, NULL, sl_wifi_default_event_handler);
  if (status != SL_STATUS_OK) {
    return status;
  }
  wifi_client_context = context;

  // Initialize LwIP stack only once
  if (!lwip_initialized) {
    tcpip_init(NULL, NULL);
    lwip_initialized = true;
  }

  // Initialize DHCP monitor mutex if not already initialized
  if (dhcp_monitor_mutex == NULL) {
    dhcp_monitor_mutex = osMutexNew(NULL);
    if (dhcp_monitor_mutex == NULL) {
      SL_DEBUG_LOG("Failed to create DHCP monitor mutex\n");
      return SL_STATUS_FAIL;
    }
  }

  sta_netif_config();
  return SL_STATUS_OK;
}

sl_status_t sl_net_wifi_client_deinit(sl_net_interface_t interface)
{
#if LWIP_TESTMODE
  struct sys_timeo **list_head = NULL;

  //! Free all timers
  for (int i = 0; i < lwip_num_cyclic_timers; i++) {
    list_head = sys_timeouts_get_next_timeout();
    if (list_head != NULL && *list_head != NULL)
      sys_untimeout((*list_head)->h, (*list_head)->arg);
  }
#endif

  // Validate interface index
  if (interface >= SL_NET_INTERFACE_MAX) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Cleanup DHCP monitoring resources for this interface
  if (dhcp_monitor_mutex != NULL) {
    osMutexAcquire(dhcp_monitor_mutex, osWaitForever);
    dhcp_monitor_state[interface].active = false;
    osMutexRelease(dhcp_monitor_mutex);
  }

#if !LWIP_NETIF_STATUS_CALLBACK
  // Stop and delete DHCP check timer if it exists for this interface
  if (dhcp_monitor_state[interface].check_timer != NULL) {
    osTimerStop(dhcp_monitor_state[interface].check_timer);
    osTimerDelete(dhcp_monitor_state[interface].check_timer);
    dhcp_monitor_state[interface].check_timer = NULL;
  }
#endif

  // Check if any other interface is still using DHCP monitoring
  bool any_interface_active = false;
  if (dhcp_monitor_mutex != NULL) {
    osMutexAcquire(dhcp_monitor_mutex, osWaitForever);
    for (size_t i = 0; i < SL_NET_INTERFACE_MAX; i++) {
      if (dhcp_monitor_state[i].active) {
        any_interface_active = true;
        break;
      }
    }
    osMutexRelease(dhcp_monitor_mutex);
  }

  // Only delete the mutex if no interfaces are using it
  if (!any_interface_active && dhcp_monitor_mutex != NULL) {
    osStatus_t status = osMutexDelete(dhcp_monitor_mutex);
    if (status != osOK) {
      SL_DEBUG_LOG("Failed to delete DHCP monitor mutex: %d\n", status);
    }
    dhcp_monitor_mutex = NULL;
  }

  // Remove netif and cleanup context
  if (wifi_client_context != NULL) {
    netif_remove(&(wifi_client_context->netif));
    wifi_client_context = NULL;
  }

  return sl_wifi_deinit();
}

sl_status_t sl_net_wifi_client_up(sl_net_interface_t interface, sl_net_profile_id_t profile_id)
{
  UNUSED_PARAMETER(interface);
  uint32_t status = 0;

  // Load profile and connect here
  sl_net_wifi_client_profile_t profile = { 0 };

  // Connect to the Wi-Fi network
  if (profile_id == SL_NET_AUTO_JOIN) {
    return sli_network_manager_auto_join_request(interface, profile_id);
  }

  status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, profile_id, &profile);
  VERIFY_STATUS_AND_RETURN(status);

  status = sl_wifi_connect(SL_WIFI_CLIENT_INTERFACE, &profile.config, SLI_WIFI_CONNECT_TIMEOUT);
  VERIFY_STATUS_AND_RETURN(status);

  status = set_sta_link_up(&profile);
  if (status != SL_STATUS_OK) {
    // Disconnect WiFi on IP configuration failure
    sl_status_t disconnect_status = sl_wifi_disconnect(SL_WIFI_CLIENT_INTERFACE);
    if (disconnect_status != SL_STATUS_OK) {
      SL_DEBUG_LOG("WiFi disconnect failed: 0x%lx\n", disconnect_status);
    }
    return status;
  }

  // Update profile with IP addresses from netif
  update_profile_from_netif(&profile);

  // Set the client profile
  status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, profile_id, &profile);

  return SL_STATUS_OK;
}

sl_status_t sl_net_wifi_client_down(sl_net_interface_t interface)
{
  UNUSED_PARAMETER(interface);
  set_sta_link_down();

  return sl_wifi_disconnect(SL_WIFI_CLIENT_INTERFACE);
}

sl_status_t sl_si91x_host_process_data_frame(sl_wifi_interface_t interface, sl_wifi_buffer_t *buffer)
{
  struct netif *ifp;
  sl_wifi_system_packet_t *packet;
  packet = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  SL_DEBUG_LOG("\nRX len : %d\n", packet->length);

  /* get the network interface for STATION interface,
   * and forward the received frame buffer to LWIP
   */
  if ((ifp = get_netif(interface)) != NULL) {
    low_level_input(ifp, packet->data, packet->length);
  }

  return SL_STATUS_OK;
}

sl_status_t sl_net_configure_ip(sl_net_interface_t interface,
                                const sl_net_ip_configuration_t *ip_config,
                                uint32_t timeout)
{
  UNUSED_PARAMETER(interface);
  UNUSED_PARAMETER(ip_config);
  UNUSED_PARAMETER(timeout);
  return SL_STATUS_WIFI_UNSUPPORTED;
}

sl_status_t sl_net_get_ip_address(sl_net_interface_t interface, sl_net_ip_address_t *ip_address, uint32_t timeout)
{
  UNUSED_PARAMETER(interface);
  UNUSED_PARAMETER(ip_address);
  UNUSED_PARAMETER(timeout);
  return SL_STATUS_WIFI_UNSUPPORTED;
}
