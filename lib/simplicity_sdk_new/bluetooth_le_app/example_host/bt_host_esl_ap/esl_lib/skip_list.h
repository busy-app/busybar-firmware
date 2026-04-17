/***************************************************************************//**
 * @file
 * @brief Skip List (a probabilistic, ordered data structure) Interface.
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

#ifndef SKIP_LIST_H
#define SKIP_LIST_H

#include <stddef.h>
#include <stdint.h>
#include "esl_lib_memory.h"

// Implement Skip List concept from William Pugh, 1989
#ifdef __cplusplus
extern "C" {
#endif

// Memory allocator override macro
// Can be replaced to malloc() to make the implementation more generic
#define SKIP_LIST_ALLOC(S)  esl_lib_memory_allocate(S)
// Memory deallocator override macro
// Can be replaced to free() to make the implementation more generic
#define SKIP_LIST_FREE(P)   esl_lib_memory_free(P)

/// Generic comparison callback type
/// The implementation should return minus one for A less than B, zero if
/// equal, and plus one for greater than.
typedef int (*node_compare_fp)(void *a, void *b); ///< To be implemented by app

/// Skip List Node pointer forward declaration
typedef struct skip_node_s *skip_node_p;

/// Skip List pointer forward declaration
typedef struct skip_list_s *skip_list_p;

/**************************************************************************//**
 * Create a Skip List object with specified max level
 *
 * @param[in] max_height Maximum list levels of the Skip List.
 * @param[in] cmp Compare function pointer for node comparison.
 * @return Pointer to a new Skip List or NULL on allocation failure.
 *
 * @note The optimal maximum level for a given list size N is the closest
 *       integer to (log(N) / log(1 / P)), P is the probability of the "coin
 *       flipper" to be used for random bit generation.
 *       The user-provided compare function shall return 0 if the values are
 *       equal, -1 if A < B or 1 if A > B.
 *****************************************************************************/
skip_list_p skip_list_create_list(size_t max_height, node_compare_fp cmp);

/**************************************************************************//**
 * Destroy a Skip List object and free all of it's resources
 *
 * @param[in] list Pointer to a Skip List to destroy.
 *
 * @note Since the Skip List takes ownership of the node data when the node is
 *       created, the node data memory is also freed.
 *****************************************************************************/
void skip_list_destroy_list(skip_list_p list);

/**************************************************************************//**
 * Remove all items from a Skip List
 *
 * @param[in] list Pointer to a skip list to remove all addresses from.
 * @return Pointer to the Skip List or NULL on invalid NULL parameter input.
 *****************************************************************************/
skip_list_p skip_list_clear_list(skip_list_p list);

/**************************************************************************//**
 * Get node count of a Skip List
 *
 * @param[in] list Pointer to a Skip List of interest.
 * @return Node count in the list (returns 0 also for NULL pointer argument).
 *****************************************************************************/
size_t skip_list_get_node_count(skip_list_p list);

/**************************************************************************//**
 * Insert a new node with specified data to a Skip List
 *
 * @param[in] list Pointer to a skip list to insert the new node into.
 * @param[in] data Pointer to the data that is to be stored for the new node.
 *
 * @note Any given data will only be added to the list once, based on the
 * decision of the compare function - even if you try to add it more than once.
 * The list always remains sorted. The list takes ownership of the data if the
 * addition (creation) of the node succeeds, but if the allocation fails to
 * create the node, then this call returns NULL and the users must free the
 * allocated data themselves!
 * The exception is if the actual addition fails because the given data is
 * already in the list - then this call will perform the release of the
 * underlying memory as if the addition had been successful - and also return
 * a non-NULL pointer to the "old" instance in the list. (Does not free the
 * exact same storage object, though, on pointer equivalence.)
 *****************************************************************************/
skip_node_p skip_list_insert_node(skip_list_p list, void *data);

/**************************************************************************//**
 * Find node by data in the Skip List
 *
 * @param[in] list Pointer to a skip list to search the data in.
 * @param[in] data Pointer to the data to search for. Can be dynamically allocated,
 *            static or auto-allocated data (i.e. globals or variables either
 *            on the heap or stack), but it's the caller's responsibility to
 *            free the memory after the call if it's dynamic.
 *
 * @return    Pointer to the node or NULL if data not found.
 *
 * @note The data can be just a copy, or the same storage object already in
 *       the list. In the latter case, however, it MUST NOT be freed by the
 *       application after the node is found!
 *****************************************************************************/
skip_node_p skip_list_find_node(skip_list_p list, const void *const data);

/**************************************************************************//**
 * Get data of the the Skip List node
 *
 * @param[in] node Pointer to the node to get the data from
 * @return    Pointer to the node data or NULL if data not found.
 * @note Also returns NULL on input parameter error (e.g. NULL pointer pass)
 *****************************************************************************/
const void *skip_list_get_node_data(skip_node_p node);

/**************************************************************************//**
 * Remove a node (by data) from a Skip List
 *
 * @param[in] list Pointer to a skip list to remove the data from.
 * @param[in] data Pointer to the COPY of the data to be removed. Can be
 *            dynamically allocated, static, or auto-allocated data (i.e.,
 *            globals or variables either on the heap or stack), but it's the
 *            caller's responsibility to free the memory after the call if it's
 *            needed.
 *
 * @return Pointer to the data of removed node (if removed any, see note)
 *
 * @note Since the ownership of the node data, to which the input data is
 *       compared, has been taken over by the Skip List, the memory reserved
 *       for the node data is returned on successful removal.
 *       In the event that the function is invoked with data that is not
 *       present within the specified list, it will return a NULL value.
 *       However, this should be regarded as an idempotent operation, analogous
 *       to the successful removal of data. (I.e., it is "no longer" in the
 *       list, even though it was not in the list before.)
 *       It should be also noted that the actual return value may differ from
 *       the input parameter in the event that the data to be removed is
 *       passed as a copy. If it is a copy and dynamically allocated, both
 *       containers can be freed by the caller.
 *****************************************************************************/
void *skip_list_remove_node(skip_list_p list, const void *const data);

/**************************************************************************//**
 * Remove the first node from a Skip List
 *
 * @param[in] list Pointer to a skip list to remove the first node from.
 *
 * @return Pointer to the data of removed node if removed any, NULL otherwise
 *
 * @note Since the ownership of the node data was taken over by the Skip List
 *       at the time the node was added, the memory reserved for the
 *       returned data shall be freed by the caller after use.
 *       If the function is called on an empty list, or with NULL parameter,
 *       then it will return NULL.
 *****************************************************************************/
void *skip_list_pop_first_node(skip_list_p list);
#ifdef __cplusplus
};
#endif

#endif // SKIP_LIST_H
