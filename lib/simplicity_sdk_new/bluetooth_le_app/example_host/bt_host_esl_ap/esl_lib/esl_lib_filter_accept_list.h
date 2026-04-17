/***************************************************************************//**
 * @file
 * @brief ESL Host Library Filter Accept List header.
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

#ifndef ESL_LIB_FILTER_ACCEPT_LIST_H
#define ESL_LIB_FILTER_ACCEPT_LIST_H

#include <stddef.h>
#include <stdint.h>
#include "sl_bgapi.h"
#include "skip_list.h"
#include "esl_lib_command_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/// The optimal maximum level is the closest integer to (log(N) / log(1 / P)),
/// where N is the expected maximum number of list items and P is the
/// probability of the coin flipper we use for random generation.
/// Our values for ESL lib for N and P are 127 and 0.5, respectively, which
/// results in about 8 average compare operations* on a list of 127 entries,
/// close to the worst case of binary search on a static array of addresses of
/// the same size, but without the need for heavy memmove() operations on
/// almost every insert and removal.
/// *NOTE that due to the probabilistic nature of the Skip Lists - the basis
///       of our Filter Accept List implementation - the comparison count may
///       be higher in some worst case scenarios.
#define ESL_LIB_SKIPLIST_MAX_LEVEL_STACK  7  // LL supports up to 127 devices

/// ESL library internal list, however, it's larger to improve performance
#define ESL_LIB_SKIPLIST_MAX_LEVEL_LIB    12 // Supports up to 256 requests

/// Alias for @ref esl_lib_command_list_cmd_t type as filter node data
typedef esl_lib_command_list_cmd_t *filter_data_p;

/// Alias for @ref skip_list_p type as filter accept list object type
typedef skip_list_p filter_list_p;

/// Alias for @ref skip_node_p type as filter accept list node object type
typedef skip_node_p filter_node_p;

/**************************************************************************//**
 * Compare Filter Accept List elements
 *
 * @param[in] a Pointer to value A to compare
 * @param[in] b Pointer to value B to compare
 *
 * @return -1 if A < B, 1 if A > B or 0 if the values are equal
 *****************************************************************************/
int filter_accept_list_compare(void *a, void *b);

/**************************************************************************//**
 * Create a Filter Accept List object with @ref ESL_LIB_SKIPLIST_MAX_LEVEL.
 *
 * @param[in] max_height Maximum list levels of the underlying Skip List.
 * @param[in] cmp Compare function pointer for node comparison.
 * @return Pointer to a new Filter Accept List or NULL on allocation failure.
 *****************************************************************************/
filter_list_p filter_accept_list_create_list(size_t max_height,
                                             node_compare_fp cmp);

/**************************************************************************//**
 * Destroy a Filter Accept List object and free all of it's resources
 *
 * @param[in] list Pointer to a Filter Accept List pointer to destroy.
 *
 * @note Since the Filter Accept List takes ownership of the node data when the
 *       node is created, the node data memory is also freed.
 *       The pointed pointer will be NULL after call
 *****************************************************************************/
void filter_accept_list_destroy_list(filter_list_p *list);

/**************************************************************************//**
 * Remove all BLE addresses from a Filter Accept List
 *
 * @param[in] list Pointer to a Filter Accept List to remove all addresses from.
 *
 * @note Since the Filter Accept List takes ownership of the node data when the
 *       node is created, the node data memory is also freed.
 *       The pointed pointer will be NULL after call
 *****************************************************************************/
void filter_accept_list_clear_list(filter_list_p list);

/**************************************************************************//**
 * Get node count of a Filter Accept List
 *
 * @param[in] list Pointer to a Filter Accept List of interest.
 * @return Node count in the list (returns 0 also for NULL pointer argument).
 *****************************************************************************/
size_t filter_accept_list_get_size(filter_list_p list);

/**************************************************************************//**
 * Insert a @ref esl_lib_command_list_cmd_connect_t command to a Filter Accept
 * List (of BLE remote device addresses to connect to)
 *
 * @param[in] list Pointer to a Filter Accept List to insert the command into.
 * @param[in] command Pointer to the connect command to a Bluetooth LE address,
 *            stored in the @ref filter_data_p structure. Must be dynamically
 *            allocated beforehand, PLEASE DO NOT pass static or auto allocated
 *            commands (i.e. globals or variables on the stack).
 *
 * @return Pointer to the newly inserted node - or an existing one, see notes.
 *
 * @note A connect command to a given address will be added to the list only
 *       once, even if you try to add it more than once or more command for the
 *       same device. The list (of addresses) will always remain sorted.
 *       Takes ownership of the command if it succeeds (by creating the node),
 *       but if the allocation fails to create the node, then this call returns
 *       NULL and users must free the previosly allocated command themselves!
 *       In the event that a new command is to be added to the list with the
 *       same target address as a command that has previously been added to the
 *       list, it should be noted that the return pointer will point to
 *       the previously added node and the storage space for the command
 *       passed as parameter will be released automatically.
 *****************************************************************************/
filter_node_p filter_accept_list_insert_command(filter_list_p list,
                                                filter_data_p command);

/**************************************************************************//**
 * Remove a connection request from a Filter Accept List
 *
 * @param[in] list Pointer to a Filter Accept List to remove the address from.
 * @param[in] command command Pointer to the connect command to a Bluetooth LE
 *            address, stored in the @ref filter_data_p structure. Can be an
 *            exact pointer to a command already in the list or just a
 *            substitute command with a target BLE address already in the list
 *            - either way the matching address will be removed.
 *            Can be dynamically allocated, static, or auto-allocated command
 *            (i.e., globals or variables either on the heap or stack), but if
 *            it's not the exact node pointer then it's the caller's
 *            responsibility to free the memory after the call.
 * @return Pointer to the data of removed node (if removed any, see note)
 *
 * @note Returns the ownership of the stored command through the return value
 *       on success to which the input address is compared under the hood, so
 *       the memory reserved for it shall be freed by tha caller.
 *       It should be also noted that the actual return value may differ from
 *       the input parameter in the event that the command to be removed is
 *       passed as a copy. If it is a copy and dynamically allocated, both
 *       containers can be freed by the caller.
 *       If it is called with a command whose target address is not in the
 *       list, the function returns a NULL pointer (i.e., it is an idempotent
 *       operation).
 *****************************************************************************/
filter_data_p filter_accept_list_remove_command(filter_list_p list,
                                                filter_data_p command);

/**************************************************************************//**
 * Remove a connection request for a BLE address from a Filter Accept List
 *
 * @param[in] list Pointer to a Filter Accept List to remove the address from.
 * @param[in] address Pointer to the Bluetooth LE address stored in the @ref
 *            bd_addr (or equivalent) structure. Can be dynamically allocated,
 *            static, or auto-allocated addresses (i.e., globals or variables
 *            either on the heap or stack), but it's the caller's
 *            responsibility to free the memory after the call.
 * @param[in] address_type BLE address type.
 *
 * @return Pointer to the data of removed node (if removed any, see note)
 *
 * @note See note @filter_accept_list_remove_command - the same applies here.
 *****************************************************************************/
filter_data_p filter_accept_list_remove_command_by_address(filter_list_p list,
                                                           bd_addr *address,
                                                           uint8_t address_type);

/**************************************************************************//**
 * Find a BLE address in a Filter Accept List and return its node data
 *
 * @param[in] Pointer to a Filter Accept List to search the address in.
 * @param[in] address Pointer to the Bluetooth LE address stored in the @ref
 *            bd_addr (or equivalent) structure. Can be dynamically allocated,
 *            static, or auto-allocated addresses (i.e., globals or variables
 *            either on the heap or stack), but it's the caller's
 *            responsibility to free the memory after the call.
 * @param[in] address_type BLE address type.
 *
 * @return    Pointer to node data (connect command) or NULL if address not
 *            found on the list
 *****************************************************************************/
const void *filter_accept_list_get_command_by_address(filter_list_p list,
                                                      bd_addr *address,
                                                      uint8_t address_type);

/**************************************************************************//**
 * Remove the first node from a Filter Accept List
 *
 * @param[in] list Pointer to a Filter Accept List to pop the first node from.
 *
 * @return Pointer to the command of removed node or NULL.
 *
 * @note Since the ownership of the command was taken over by the Filter
 *       Accept List at the time the command was added, the memory reserved
 *       for it shall be freed by the caller after use.
 *       If the function is called on an empty list, or with NULL parameter,
 *       then it will return NULL.
 *****************************************************************************/
filter_data_p filter_accept_list_pop_first_node(filter_list_p list);

#ifdef __cplusplus
};
#endif

#endif // ESL_LIB_FILTER_ACCEPT_LIST_H
