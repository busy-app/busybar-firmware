/***************************************************************************//**
 * @file app.c
 * @brief Application code
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cmsis_os2.h>

#include "sl_status.h"
#include "sl_assert.h"
#include "sl_component_catalog.h"
#include "sl_memory_manager.h"
#include "border_router/sl_wisun_br_api.h"
#include "sl_wisun_api.h"
#include "sl_wisun_keychain.h"
#include "sl_wisun_ip6string.h"
#include "sl_select_util.h"
#include "select.h"
#include "sl_wisun_config.h"
#include "sl_wisun_br_config.h"
#include "app.h"
#include "sl_wisun_br_dhcpv6_server.h"
#include "sl_wisun_types.h"
#include "border_router/sl_wisun_br_connection_params_api.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
/**************************************************************************//**
 * @brief Start the application
 *****************************************************************************/
static void app_start(void);

/**************************************************************************//**
 * @brief Convert ranges to mask
 * @details Convert ranges to mask
 * @param[in] str String containing ranges
 * @param[out] mask Pointer to the mask to be filled
 * @param[in] size Size of the mask
 * @return sl_status_t SL_STATUS_OK on success, otherwise SL_STATUS_FAIL
 *****************************************************************************/
static sl_status_t app_ranges_to_mask(const char *str, uint8_t *mask, uint32_t size);

/**************************************************************************//**
 * @brief Initialize the DHCPv6 socket
 *
 * @return sl_status_t
 *****************************************************************************/
static sl_status_t app_init_dhcpv6_socket(void);
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
static int app_dhcpv6_socket = SOCKET_INVALID_ID;
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
#ifndef SL_CATALOG_WISUN_EVENT_MGR_PRESENT  // Event Manager also defines this handler
/* Wi-SUN event handler */
void sl_wisun_on_event(sl_wisun_evt_t *evt)
{
  switch (evt->header.id) {
    case SL_WISUN_MSG_SOCKET_DATA_AVAILABLE_IND_ID:
      sl_wisun_check_read_sockfd_set();
      break;
    case SL_WISUN_MSG_SOCKET_DATA_SENT_IND_ID:
      sl_wisun_check_write_sockfd_set();
      break;
    default:
      break;
  }
  /////////////////////////////////////////////////////////////////////////////
  // Put your Wi-SUN event handling here!                                    //
  // This is called from Wi-SUN stack.                                       //
  // Do not call blocking functions from here!                               //
  // Protect your data during event handling!                                //
  /////////////////////////////////////////////////////////////////////////////
}
#endif

/* App task function */
void app_task(void *args)
{
  (void) args;

  printf("\nWi-SUN Border Router Empty Application\n");

  EFM_ASSERT(sl_wisun_br_dhcpv6_server_init() == SL_STATUS_OK);
  EFM_ASSERT(app_init_dhcpv6_socket() == SL_STATUS_OK);

  app_service_task_init();

  app_start();

  while (1) {
    ///////////////////////////////////////////////////////////////////////////
    // Put your application code here!                                       //
    ///////////////////////////////////////////////////////////////////////////
    osDelay(1UL);
  }
}

/* App DHCPv6 service task function */
void app_service_task(void *args)
{
  static uint8_t buffer[350] = { 0 };
  ssize_t count = 0;
  sockaddr_in6_t src_addr = { 0 };
  socklen_t addrlen = sizeof(sockaddr_in6_t);
  fd_set readfds = { 0 };
  int max_sd = -1;

  (void) args;

  while (1) {
    FD_ZERO(&readfds);
    if (app_dhcpv6_socket >= 0) {
      FD_SET(app_dhcpv6_socket, &readfds);
      max_sd = MAX(app_dhcpv6_socket, max_sd);
    }

    if (max_sd < 0
        || select(max_sd + 1, &readfds, NULL, NULL, NULL) < 0) {
      break;
    }

    if ((app_dhcpv6_socket >= 0) && (FD_ISSET(app_dhcpv6_socket, &readfds))) {
      count = recvfrom(app_dhcpv6_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&src_addr, &addrlen);
      if (count >= 0) {
        sl_wisun_br_dhcpv6_server_on_recv(buffer, count, src_addr.sin6_addr, src_addr.sin6_port);
      }
    }
  }
  osThreadExit();
}
// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
static sl_status_t app_init_dhcpv6_socket(void)
{
  int retval = 0;

  const sockaddr_in6_t bind_addr = {
    .sin6_family = AF_INET6,
    .sin6_port = htons(DHCPV6_SERVER_PORT),
    .sin6_flowinfo = 0,
    .sin6_addr = IN6ADDR_ANY_INIT,
    .sin6_scope_id = 0,
  };

  app_dhcpv6_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
  if (app_dhcpv6_socket == SOCKET_INVALID_ID) {
    printf("[Failed: failed to open DHCPv6 socket]\n");
    close(app_dhcpv6_socket);
    return SL_STATUS_FAIL;
  }
  retval = bind(app_dhcpv6_socket, (const struct sockaddr *)&bind_addr, sizeof(sockaddr_in6_t));
  if (retval < 0) {
    printf("[Failed: failed to bind DHCPv6 socket (%d)]\n", retval);
    close(app_dhcpv6_socket);
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

static void app_start(void)
{
  sl_wisun_channel_mask_t channel_mask = { 0 };
  sl_wisun_phy_config_t phy_config = { 0 };
  sl_wisun_br_connection_params_t params = { 0 };
  sl_wisun_br_lfn_params_t lfn_params = { 0 };
  uint8_t ipv6_prefix[IPV6_ADDR_SIZE] = { 0U };
  int_fast16_t ipv6_prefix_length = 0;
  uint8_t trustedca_count = 0U;
  sl_wisun_keychain_entry_t *trustedca = NULL;
  uint16_t certificate_options = 0U;
  sl_wisun_keychain_credential_t *credential = NULL;
  sl_wisun_mac_address_t address = { 0 };

  // Set Device Type
  EFM_ASSERT(sl_wisun_set_device_type(SL_WISUN_BORDER_ROUTER) == SL_STATUS_OK);

  // Set TX Power
#if defined(WISUN_CONFIG_TX_POWER)
  EFM_ASSERT(sl_wisun_set_tx_power_ddbm(WISUN_CONFIG_TX_POWER) == SL_STATUS_OK);
#else
  EFM_ASSERT(sl_wisun_set_tx_power_ddbm(200) == SL_STATUS_OK);
#endif

  // Set Connection Parameters
  switch (WISUN_CONFIG_NETWORK_SIZE) {
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
      EFM_ASSERT(0);
      break;
  }
  EFM_ASSERT(sl_wisun_br_set_connection_parameters(&params) == SL_STATUS_OK);

  // Set Neighbor Table
  EFM_ASSERT(sl_wisun_config_neighbor_table(SL_WISUN_BR_CONFIG_MAX_CHILD_COUNT,
                                            SL_WISUN_BR_CONFIG_MAX_NEIGHBOR_COUNT,
                                            SL_WISUN_BR_CONFIG_MAX_SECURITY_NEIGHBOR_COUNT) == SL_STATUS_OK);

  EFM_ASSERT((stoip6_prefix(SL_WISUN_BR_CONFIG_IPV6_PREFIX, ipv6_prefix, &ipv6_prefix_length) >= 0)
             && (ipv6_prefix_length >= 0) && (ipv6_prefix_length <= 64));

  // Set IPv6 Prefix
  EFM_ASSERT(sl_wisun_br_set_ipv6_prefix(ipv6_prefix, (uint8_t)ipv6_prefix_length) == SL_STATUS_OK);

  // Set LFN Parameters
#if defined(WISUN_CONFIG_DEVICE_PROFILE)
  switch (WISUN_CONFIG_DEVICE_PROFILE) {
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
      EFM_ASSERT(0);
      break;
  }
#else
  lfn_params = SL_WISUN_BR_PARAMS_LFN_TEST;
#endif

  EFM_ASSERT(sl_wisun_br_set_lfn_parameters(&lfn_params) == SL_STATUS_OK);

  // Convert allowed channels to mask
  EFM_ASSERT(app_ranges_to_mask(WISUN_CONFIG_ALLOWED_CHANNELS,
                                channel_mask.mask,
                                SL_WISUN_CHANNEL_MASK_SIZE) == SL_STATUS_OK);

  // Set Channel Mask
  EFM_ASSERT(sl_wisun_set_channel_mask(&channel_mask) == SL_STATUS_OK);

  // Set Unicast Settings
  EFM_ASSERT(sl_wisun_set_unicast_settings(SL_WISUN_BR_CONFIG_UC_DWELL_INTERVAL) == SL_STATUS_OK);

  // Set Broadcast Settings
  EFM_ASSERT(sl_wisun_br_set_broadcast_settings(SL_WISUN_BR_CONFIG_BC_INTERVAL,
                                                SL_WISUN_BR_CONFIG_BC_DWELL_INTERVAL) == SL_STATUS_OK);

  // Get trusted CA count
  trustedca_count = sl_wisun_keychain_get_trustedca_count();
  EFM_ASSERT(trustedca_count > 0U);

  certificate_options = SL_WISUN_CERTIFICATE_OPTION_IS_REF;
  for (uint8_t idx = 0U; idx < trustedca_count; ++idx) {
    trustedca = sl_wisun_keychain_get_trustedca(idx);
    EFM_ASSERT(trustedca != NULL);
    if (trustedca->keychain == SL_WISUN_KEYCHAIN_NVM) {
      printf("[Using NVM trusted CA #%u]\n", idx);
    } else if (trustedca->keychain == SL_WISUN_KEYCHAIN_BUILTIN) {
      printf("[Using built-in trusted CA #%u]\n", idx);
    }
    // Set Trusted CA
    EFM_ASSERT(sl_wisun_set_trusted_certificate(certificate_options,
                                                trustedca->data_length,
                                                trustedca->data) == SL_STATUS_OK);
    sl_free(trustedca);
    trustedca = NULL;
    certificate_options |= SL_WISUN_CERTIFICATE_OPTION_APPEND;
  }

  // Retrieve a device credential
  credential = sl_wisun_keychain_get_credential(SL_WISUN_BR_CONFIG_KEYCHAIN,
                                                SL_WISUN_BR_CONFIG_KEYCHAIN_INDEX);
  EFM_ASSERT(credential != NULL);
  if (credential->certificate.keychain == SL_WISUN_KEYCHAIN_NVM) {
    printf("[Using NVM device credentials]\n");
  } else if (credential->certificate.keychain == SL_WISUN_KEYCHAIN_BUILTIN) {
    printf("[Using built-in device credentials]\n");
  }

  // Set Device Certificate
  EFM_ASSERT(sl_wisun_set_br_device_certificate(SL_WISUN_CERTIFICATE_OPTION_IS_REF | SL_WISUN_CERTIFICATE_OPTION_HAS_KEY,
                                                credential->certificate.data_length,
                                                credential->certificate.data) == SL_STATUS_OK);
  if (credential->pk.type == SL_WISUN_KEYCHAIN_KEY_TYPE_PLAINTEXT) {
    // Set Device Private Key
    EFM_ASSERT(sl_wisun_set_device_private_key(SL_WISUN_PRIVATE_KEY_OPTION_IS_REF,
                                               credential->pk.u.plaintext.data_length,
                                               credential->pk.u.plaintext.data) == SL_STATUS_OK);
  } else {
    // Set Device Private Key ID
    EFM_ASSERT(sl_wisun_set_device_private_key_id(credential->pk.u.key_id) == SL_STATUS_OK);
  }

  sl_free(credential);
  credential = NULL;

  phy_config.config.fan11.reg_domain = WISUN_CONFIG_REGULATORY_DOMAIN;
  phy_config.config.fan11.chan_plan_id = WISUN_CONFIG_CHANNEL_PLAN_ID;
  phy_config.config.fan11.phy_mode_id = WISUN_CONFIG_PHY_MODE_ID;
  phy_config.type = SL_WISUN_PHY_CONFIG_FAN11;

  EFM_ASSERT(sl_wisun_get_mac_address(&address) == SL_STATUS_OK);
  EFM_ASSERT(sl_wisun_br_dhcpv6_server_start(app_dhcpv6_socket,
                                             ipv6_prefix,
                                             address.address,
                                             LIFETIME_INFINITE) == SL_STATUS_OK);

  // Start Border Router
  EFM_ASSERT(sl_wisun_br_start((const uint8_t *)WISUN_CONFIG_NETWORK_NAME, &phy_config) == SL_STATUS_OK);

  printf("[Border router started]\n");
}

static sl_status_t app_ranges_to_mask(const char *str, uint8_t *mask, uint32_t size)
{
  char *endptr = NULL;
  uint32_t cur = 0U;
  uint32_t end = 0U;
  uint32_t index = 0U;

  memset(mask, 0U, size * sizeof(uint8_t));

  do {
    if (*str == '\0') {
      return SL_STATUS_FAIL;
    }
    cur = strtoul(str, &endptr, 0);
    if (*endptr == '-') {
      str = endptr + 1;
      end = strtoul(str, &endptr, 0);
    } else {
      end = cur;
    }
    if (*endptr != '\0' && *endptr != ',') {
      return SL_STATUS_FAIL;
    }
    if (cur > end) {
      return SL_STATUS_FAIL;
    }
    for (; cur <= end; cur++) {
      index = cur / 8;
      if (index < size) {
        mask[index] |= 1 << (cur % 8);
      } else {
        return SL_STATUS_FAIL;
      }
    }
    str = endptr + 1;
  } while (*endptr != '\0');

  return SL_STATUS_OK;
}
