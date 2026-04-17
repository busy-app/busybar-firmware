/***************************************************************************/ /**
 * @file at_utility.c
 * @brief Top level application functions
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

#include "at_utility.h"
#include <stddef.h>

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

osMutexId_t print_mutex;

/******************************************************
 *               Function Definitions
 ******************************************************/

void at_print_char_buffer(char *buffer, uint32_t buffer_length)
{
  if (buffer == NULL)
    return;
  for (uint32_t index = 0; index < buffer_length; index++) {
    AT_PRINTF("%c", buffer[index]);
  }
}

sl_status_t sl_net_inet_addr_auto(const char *addr, sl_ip_address_t *ip_address)
{
  if ((NULL == addr) || (NULL == ip_address)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Check if the address contains a colon, which indicates IPv6
#ifdef SLI_SI91X_ENABLE_IPV6
  if (strchr(addr, ':') != NULL) {
    // IPv6 address
    ip_address->type                               = SL_IPV6;
    unsigned char hex_addr[SL_IPV6_ADDRESS_LENGTH] = { 0 };
    int ret = sl_inet_pton6(addr, addr + strlen(addr), hex_addr, (unsigned int *)ip_address->ip.v6.value);

    if (ret != 0x1) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    return SL_STATUS_OK;
  } else
#endif
  {
    // IPv4 address
    ip_address->type = SL_IPV4;
    return sl_net_inet_addr(addr, &ip_address->ip.v4.value);
  }
}
