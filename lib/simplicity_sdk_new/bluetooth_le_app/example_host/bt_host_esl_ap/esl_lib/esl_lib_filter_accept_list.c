/***************************************************************************//**
 * @file
 * @brief ESL Host Library Filter Accept List implementation.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esl_lib_filter_accept_list.h"

#define UNIVERSAL_ADDRESS_COMPARISON 0

// -----------------------------------------------------------------------------
// Local Function Definitions

#if UNIVERSAL_ADDRESS_COMPARISON
// Alternative for u64-based comparison: for an address size of only 7 bytes instead of 8 bytes.
// It works like memcmp but takes the bytes in reverse byte order because of the nature of bd_addr type
static inline int memrcmp(const void *s1, const void *s2, size_t n)
{
  if (s1 == s2) {
    // Immediate return if the objects are the same
    return 0;
  } else {
    const unsigned char *a = s1;
    const unsigned char *b = s2;

    while (n) {
      --n;
      if (a[n] != b[n]) {
        break;
      }
    }

    return a[n] < b[n] ? -1 : n > 0;
  }
}

// This compare method may be more tolerant of arbitrary sizes of node data, since it only compares
// the data up to the size of the address part, including the address type.
// However, the first field of the data must be of type esl_lib_address_t using this method!
static inline int compare_address(void *a, void *b)
{
  return memrcmp(((filter_data_p)a)->data.cmd_connect.address.bytes,
                 ((filter_data_p)b)->data.cmd_connect.address.bytes,
                 sizeof(esl_lib_address_t));
}

#else // UNIVERSAL_ADDRESS_COMPARISON
// This method is faster than the memrcmp()-based compare_address() method.
// It treats the pointed objects as 64-bit unsigned integers and compares them.
// Can be used only if the size of esl_lib_address_t is exactly 8 bytes - use compare_address(), otherwise.
// Note that this method depends on the system endianness.
static inline int compare_address(void *a, void *b)
{
  register uint64_t A = ((filter_data_p)a)->data.cmd_connect.address.u64;
  register uint64_t B = ((filter_data_p)b)->data.cmd_connect.address.u64;

  return A > B ? 1 : A < B ? -1 : 0;
}
#endif // UNIVERSAL_ADDRESS_COMPARISON

// -----------------------------------------------------------------------------
// Public Function Definitions

int filter_accept_list_compare(void *a, void *b)
{
  return compare_address(a, b);
}

filter_list_p filter_accept_list_create_list(size_t max_height, node_compare_fp cmp)
{
  return skip_list_create_list(max_height, cmp);
}

void filter_accept_list_destroy_list(filter_list_p *list)
{
  if (list != NULL && *list != NULL) {
    skip_list_destroy_list(*list);
    *list = NULL;
  }
}

void filter_accept_list_clear_list(filter_list_p list)
{
  (void)skip_list_clear_list(list);
}

size_t filter_accept_list_get_size(filter_list_p list)
{
  return skip_list_get_node_count(list);
}

filter_node_p filter_accept_list_insert_command(filter_list_p list, filter_data_p command)
{
  return skip_list_insert_node(list, (void *)command);
}

filter_data_p filter_accept_list_remove_command(filter_list_p list, filter_data_p command)
{
  return (filter_data_p)skip_list_remove_node(list, (const void *const)command);
}

filter_data_p filter_accept_list_remove_command_by_address(filter_list_p list,
                                                           bd_addr *address,
                                                           uint8_t address_type)
{
  esl_lib_command_list_cmd_t stub;

  stub.data.cmd_connect.address.u64 = 0;
  memcpy(stub.data.cmd_connect.address.addr, address->addr, sizeof(stub.data.cmd_connect.address.addr));
  stub.data.cmd_connect.address.address_type = address_type;

  return (filter_data_p)skip_list_remove_node(list, (void *)&stub);
}

const void *filter_accept_list_get_command_by_address(filter_list_p list,
                                                      bd_addr *address,
                                                      uint8_t address_type)
{
  esl_lib_command_list_cmd_t stub;
  filter_node_p node = NULL;

  stub.data.cmd_connect.address.u64 = 0;
  memcpy(stub.data.cmd_connect.address.addr, address->addr, sizeof(stub.data.cmd_connect.address.addr));
  stub.data.cmd_connect.address.address_type = address_type;
  node = skip_list_find_node(list, (void *)&stub);

  return skip_list_get_node_data(node);
}

filter_data_p filter_accept_list_pop_first_node(filter_list_p list)
{
  return (filter_data_p)skip_list_pop_first_node(list);
}
