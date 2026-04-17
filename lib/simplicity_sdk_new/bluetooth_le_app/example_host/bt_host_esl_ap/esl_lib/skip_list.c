/***************************************************************************//**
 * @file
 * @brief Skip List implementation.
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
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "skip_list.h"

// internal state array for WELL PRNG
static uint32_t well_prng_state[16];
// internal state array index for WELL PRNG
static uint8_t well_state_index = 0;

// initialize state array to (pseudo) random bits
static void well512_init()
{
  // For lazy init...
  static bool initialized = false;

  if (!initialized) {
    well_state_index = sizeof(well_prng_state) / sizeof(uint32_t);

    srand(time(NULL));
    do {
      well_prng_state[--well_state_index] = rand();
    } while (well_state_index);
    initialized = true;
  }
}

// 32 bit well equidistributed long-period linear pseudo random generator by Chris L., PhD.
uint32_t well512_prng(void)
{
  uint32_t a, b, c, d;

  a = well_prng_state[well_state_index];
  c = well_prng_state[(well_state_index + 13) & 15];
  b = a ^ c ^ (a << 16) ^ (c << 15);
  c = well_prng_state[(well_state_index + 9) & 15];
  c ^= (c >> 11);
  a = well_prng_state[well_state_index] = b ^ c;
  d = a ^ ((a << 5) & 0xda442d24ul);
  well_state_index = (well_state_index + 15) & 15;
  a = well_prng_state[well_state_index];
  well_prng_state[well_state_index] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);

  return well_prng_state[well_state_index];
}

// Internal structure type definitions
// Note that these structures are hidden (not defined in any public header) to
// prevent users from writing code that relies directly on the fields.
// Skip List node type
typedef struct skip_node_s {
  size_t      level;
  void        *data;
  skip_node_p next[];
} skip_node_t;

// Skip List type
typedef struct skip_list_s {
  skip_node_t     *head;
  size_t          size;
  size_t          height;
  node_compare_fp compare;
} skip_list_t;

// -----------------------------------------------------------------------------
// Local Function Definitions

// Takes ownership of the data, skip_list_clear_list() and skip_list_destroy_list()
// invocations will free data as well!
static skip_node_t *skip_list_create_node(void *data, size_t level)
{
  const size_t level_count = level + 1;
  skip_node_t *node = SKIP_LIST_ALLOC(sizeof(skip_node_t) + level_count * sizeof(skip_node_p));

  if (node != NULL) {
    node->level = level;
    node->data  = data;
    memset(node->next, 0, level_count * sizeof(skip_node_p));
  }

  return node;
}

static void *skip_list_destroy_node(skip_node_t *node)
{
  void *data = NULL;

  if (node != NULL) {
    // Save data pointer
    data = node->data;
    // Free node
    SKIP_LIST_FREE(node);
  }

  return data;
}

static skip_node_t *skip_list_search_data(skip_list_t *list,
                                          skip_node_t *update[],
                                          const void *const data)
{
  skip_node_t *node = NULL;

  if (list != NULL && update != NULL && data != NULL) {
    skip_node_t *current = list->head;
    skip_node_t *already_checked = NULL;

    // Search from the top to the bottom
    for (int level = (signed)list->height; level >= 0; --level) {
      while (current->next[level] != already_checked
             && list->compare(current->next[level]->data, (void *)data) < 0) {
        current = current->next[level];
      }
      already_checked = current->next[level];
      update[level] = current;
    }

    current = current->next[0];
    node = current && list->compare(current->data, (void *)data) == 0 ? current : NULL;
  }

  return node;
}

// Each element in a skip list is represented by a node object with associated data.
// The skip list level of the new nodes is chosen randomly when inserting them into the list.
static size_t skip_list_get_random_level(size_t max_level)
{
  size_t level;

  do {
    uint8_t bit = 0;

    level = 0;
    // According to the rand() man page: "...on older implementations, and on
    // current implementations on various systems, the lower-order bits are much
    // less random than the higher-order bits."
    // So we use a custom well512() implementation from the public domain instead,
    // which claims to give much better randomness on all bits.
    long random_bits = well512_prng();

    // Make sure that our simple level generator will stop at the highest
    // acceptable value
    random_bits &= (long)~(1 << max_level);

    // Use P=0.5 probability with the assumption that each bit in the random
    // value has the same chance of being 0 or 1.
    while (random_bits & (1 << bit++)) {
      level++;
    }
  } while (level >= max_level); // avoid counting bias at the top level

  return level;
}

// -----------------------------------------------------------------------------
// Test Function Definitions

// Internal height getter helper function for testing only
size_t skip_list_get_height(skip_list_p list)
{
  return list ? list->height : 0;
}

// Internal head node getter helper function for testing only
skip_node_p skip_list_get_head(skip_list_p list)
{
  return list ? list->head : NULL;
}

// Internal node getter helper function for testing only
skip_node_p skip_list_get_next_level(skip_node_p node, size_t level)
{
  return node ? node->next[level] : NULL;
}

// -----------------------------------------------------------------------------
// Public Function Definitions

skip_list_p skip_list_create_list(size_t max_height, node_compare_fp cmp)
{
  if (max_height == 0 || cmp == NULL) {
    return NULL; // Usage error
  }

  skip_list_p list = SKIP_LIST_ALLOC(sizeof(skip_list_t));

  if (list != NULL) {
    // Do lazy init on WELL PRNG for first instance
    well512_init();
    // We need a head node with max height
    list->head = skip_list_create_node(NULL, max_height);
    // The head node doesn't count for size
    list->size = 0;
    list->height = 0;
    // Set the compare function for node comparisons
    list->compare = cmp;
  }

  if (list->head == NULL) {
    // In case of allocation failure for the head node free the list and return NULL
    SKIP_LIST_FREE(list);
    list = NULL;
  }

  return list;
}

void skip_list_destroy_list(skip_list_p list)
{
  // Destroy all nodes and their data, first
  // skip_list_clear_list() will check if "list" is NULL.
  if (skip_list_clear_list(list) != NULL) {
    // If it was a valid list, then destroy head node as well
    (void)skip_list_destroy_node(list->head);
    // And finally, free the memory space that was allocated to it.
    SKIP_LIST_FREE(list);
  }
}

skip_list_p skip_list_clear_list(skip_list_p list)
{
  // Avoid de-referencing a NULL pointer
  if (list != NULL) {
    skip_node_t *node = list->head->next[0];
    skip_node_t *next = NULL;

    // Destroy all nodes and their data!
    while (node) {
      void* data;
      next = node->next[0];
      data = skip_list_destroy_node(node);
      SKIP_LIST_FREE(data);
      node = next;
    }
    // Clear list size and level
    list->size = 0;
    list->height = 0;
    // Clear head node links
    memset(list->head->next, 0, list->head->level * sizeof(skip_node_p));
  }
  // Please note that it returns NULL for a reason if the input pointer is invalid.
  return list;
}

size_t skip_list_get_node_count(skip_list_p list)
{
  return list ? list->size : 0;
}

skip_node_p skip_list_insert_node(skip_list_p list, void *data)
{
  skip_node_t *current = NULL;

  if (list != NULL && data != NULL) {
    current = list->head;

    // Forward pointers should be updated at each level of the node before the insertion position.
    skip_node_t *update[list->head->level + 1];    // include level 0
    memset(update, 0, (list->head->level + 1) * sizeof(skip_node_t *));

    // The internal search function will prepare the pointer update array
    current = skip_list_search_data(list, update, data);

    // If search returns non-NULL, it means same data already in list
    if (current == NULL) {  // We don't allow duplicate items in our sorted list
      size_t random_level = skip_list_get_random_level(list->head->level);

      // If the random level is greater than the list's current level, set the
      // update value to the list's head for future use.
      if (random_level > list->height) {
        for (size_t level = (random_level + 1); level >= list->height + 1; --level) {
          update[level] = list->head;
        }
        list->height = random_level; // current max level (<= list->max_level).
      }
      // Adjust the pointer and update the nodes.
      current = skip_list_create_node(data, random_level);

      if (current != NULL) {
        for (int level = random_level; level >= 0; --level) {
          current->next[level] = update[level]->next[level];
          update[level]->next[level] = current;
        }

        ++list->size;
      }
    } else if (data != skip_list_get_node_data(current)) {
      // As if the data had just been added to the list, we take ownership and
      // destroy its copy since it is no longer needed.
      SKIP_LIST_FREE(data);
    }
  }

  return current;
}

skip_node_p skip_list_find_node(skip_list_p list,
                                const void *const data)
{
  skip_node_t *current = NULL;

  if (list != NULL && list->size != 0 && data != NULL) {
    current = list->head;

    // Search method needs an update array for the forward pointer updates, create a temporary one
    skip_node_t *update[current->level + 1];

    // The internal search function will prepare the pointer update array
    current = skip_list_search_data(list, update, data);
  }

  return current;
}

const void *skip_list_get_node_data(skip_node_p node)
{
  return node ? node->data : NULL;
}

void *skip_list_remove_node(skip_list_p list, const void *const data)
{
  void *node_data = NULL;

  if (list != NULL && list->size != 0 && data != NULL) {
    skip_node_t *current = list->head;

    // Forward pointers should be updated at each level of the node before the delete position.
    skip_node_t *update[list->head->level + 1];    // include level 0
    memset(update, 0, (list->head->level + 1) * sizeof(skip_node_t *));

    // The internal search function will prepare the pointer update array
    current = skip_list_search_data(list, update, data);

    // If the search returns NULL, then there was no such data in the list.
    if (current != NULL) {
      for (int level = list->height; level >= 0; --level) {
        if (update[level]->next[level] == current) {
          update[level]->next[level] = current->next[level];
        }
      }
      // Remove list levels that have no items left
      while (list->height > 0 && list->head->next[list->height] == NULL) {
        --list->height;
      }
      --list->size;
      node_data = skip_list_destroy_node(current);
    }
  }

  return node_data;
}

void *skip_list_pop_first_node(skip_list_p list)
{
  void *node_data = NULL;

  if (list != NULL && list->size != 0) {
    skip_node_t *const head = list->head;
    skip_node_t *first = head->next[0];

    // Update head forward pointers
    for (int level = list->height; level >= 0; --level) {
      if (head->next[level] == first) {
        head->next[level] = first->next[level];
      }
    }
    // Remove list levels that have no items left
    while (list->height > 0 && head->next[list->height] == NULL) {
      --list->height;
    }
    --list->size;
    node_data = skip_list_destroy_node(first);
  }

  return node_data;
}
