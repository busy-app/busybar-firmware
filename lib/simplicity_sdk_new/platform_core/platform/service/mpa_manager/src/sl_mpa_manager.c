/***************************************************************************//**
 * @file
 * @brief Memory Protection and Attribute Manager Implementation.
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

#include "sl_mpa_manager.h"
#include "sli_mpa_manager_device.h"
#include "sl_memory_manager.h"
#include "sl_common.h"
#include "sl_slist.h"
#include "sl_core.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

#define IS_NOT_32_BYTE_ALIGNED(address) \
  (address & 0x1F)

/*******************************************************************************
 ***************************   LOCAL VARIABLES   *******************************
 ******************************************************************************/

static bool is_mpa_manager_initialized = false;
static sl_slist_node_t *mpam_region_list = NULL;
static sl_slist_node_t *mpam_boundary_list = NULL;

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/
static bool is_region_handle_registered(sl_mpa_manager_region_t* handle);
static void register_region_handle(sl_mpa_manager_region_t* handle);
static void unregister_region_handle(sl_mpa_manager_region_t* handle);
static void insert_into_boundary_list(sli_mpa_manager_boundary_t* boundary);

/*******************************************************************************
 ***************************   GLOBAL FUNCTIONS  *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initializes the Memory Protection and Attribute Manager (MPAM).
 ******************************************************************************/
void sl_mpa_manager_init(void)
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  // Check if the MPA Manager is already initialized and return if it is. This
  // makes the function idempotent.
  if (is_mpa_manager_initialized) {
    CORE_EXIT_ATOMIC();
    return;
  }

  sl_slist_init(&mpam_region_list);
  sl_slist_init(&mpam_boundary_list);

  sli_mpa_manager_device_init();

  is_mpa_manager_initialized = true;
  CORE_EXIT_ATOMIC();
}

/***************************************************************************//**
 * @brief
 *   Configure an MPA region.
 ******************************************************************************/
sl_status_t sl_mpa_manager_configure_region(sl_mpa_manager_region_t* handle,
                                            void* base_address,
                                            size_t size,
                                            sl_mpa_manager_attribute_t attribute)
{
  sl_status_t status = SL_STATUS_OK;
  sl_mpa_manager_region_t initial_region;
  bool is_region_registered;

  if ( handle == NULL
       || IS_NOT_32_BYTE_ALIGNED((uint32_t)base_address)
       || IS_NOT_32_BYTE_ALIGNED((uint32_t)size)
       || attribute >= SL_MPA_MANAGER_ATTRIBUTE_MAX ) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Assert architecture specific limitations on valid mpa regions.
  EFM_ASSERT(sli_mpa_manager_device_region_validate((uint32_t)base_address,
                                                    (uint32_t)size));

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  // Copy initial region and its boundaries in case the region configuration
  // fails.
  initial_region = *handle;

  // Modify region and its boundaries
  *handle = (sl_mpa_manager_region_t) {
    .base_address = base_address,
    .size = size,
    .attribute = attribute,
    .start_boundary.region = (void*)handle,
    .start_boundary.address = (uint32_t)base_address,
    .start_boundary.is_end = false,
    .end_boundary.region = (void*)handle,
    .end_boundary.address = (uint32_t)base_address + (uint32_t)size,
    .end_boundary.is_end = true,
  };

  is_region_registered = is_region_handle_registered(handle);

  // A region being configured with a size of 0 is equivalent to modifying and
  // then disabling the region if it was previously enabled.
  if ( size == 0 ) {
    if (is_region_registered) {
      // If the region handle is registered, we need to disable it and
      // re-configure the hardware.
      status = sl_mpa_manager_disable_region(handle);
    }
    CORE_EXIT_ATOMIC();
    return status;
  }

  // Only register the region handle to the region list if it isn't
  // already registered.
  if ( !is_region_registered ) {
    register_region_handle(handle);
  }

  // Attempt to re-configure the hardware, restore the region handle to
  // its initial state if this fails.
  status = sli_mpa_manager_device_configure(mpam_boundary_list);
  if ( status != SL_STATUS_OK ) {
    *handle = initial_region;

    // Unregister the region handle from the region list if the region handle
    // wasn't already registered before this function was called.
    if ( !is_region_registered ) {
      unregister_region_handle(handle);
    }
  }
  CORE_EXIT_ATOMIC();

  return status;
}

/***************************************************************************//**
 * @brief
 *   Disable an MPA region.
 ******************************************************************************/
sl_status_t sl_mpa_manager_disable_region(sl_mpa_manager_region_t* handle)
{
  sl_status_t status = SL_STATUS_OK;

  if ( handle == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  unregister_region_handle(handle);
  status = sli_mpa_manager_device_configure(mpam_boundary_list);
  if ( status != SL_STATUS_OK ) {
    // The hardware was not re-configured successfully, add the region
    // handle back to the list and return the error.
    register_region_handle(handle);
  }

  CORE_EXIT_ATOMIC();
  return status;
}

/***************************************************************************//**
 * @brief
 *   Get the size of the MPA region data structure.
 ******************************************************************************/
size_t sl_mpa_manager_get_region_handle_size(void)
{
  return sizeof(sl_mpa_manager_region_t);
}

/***************************************************************************//**
 * @brief
 *   Allocate memory for an MPA region on the heap.
 ******************************************************************************/
sl_status_t sl_mpa_manager_alloc_region_handle(sl_mpa_manager_region_t** handle)
{
  sl_status_t status = SL_STATUS_OK;

  if ( handle == NULL ) {
    return SL_STATUS_NULL_POINTER;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  status = sl_memory_alloc(sizeof(sl_mpa_manager_region_t), BLOCK_TYPE_LONG_TERM, (void**)handle);

  CORE_EXIT_ATOMIC();

  return status;
}

/***************************************************************************//**
 * @brief
 *   Free the MPA region memory allocated.
 ******************************************************************************/
sl_status_t sl_mpa_manager_free_region_handle(sl_mpa_manager_region_t* handle)
{
  sl_status_t status = SL_STATUS_OK;

  if ( handle == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  // The region must be disabled before it can be freed.
  if (is_region_handle_registered(handle)) {
    CORE_EXIT_ATOMIC();
    return SL_STATUS_INVALID_STATE;
  }

  status = sl_memory_free((void*)handle);
  CORE_EXIT_ATOMIC();

  return status;
}

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Insert a boundary into the mpam_boundary_list. Boundaries are inserted in
 *   ASCENDING order of their addresses.
 ******************************************************************************/
static void insert_into_boundary_list(sli_mpa_manager_boundary_t* boundary)
{
  sli_mpa_manager_boundary_t* current_boundary = NULL;
  sli_mpa_manager_boundary_t* previous_boundary = NULL;

  // Check if the boundary is in the list, if it is, simply return, the boundary
  // does not need to be inserted.
  SL_SLIST_FOR_EACH_ENTRY(mpam_boundary_list, current_boundary, sli_mpa_manager_boundary_t, node) {
    if ( current_boundary == boundary ) {
      return;
    }
  }

  // Reach here if the boundary is not in the list, and needs to be inserted.
  if ( sl_slist_is_empty(mpam_boundary_list) ) {
    sl_slist_push(&mpam_boundary_list, &(boundary->node));
  } else {
    SL_SLIST_FOR_EACH_ENTRY(mpam_boundary_list, current_boundary, sli_mpa_manager_boundary_t, node) {
      // Place boundary in ASCENDING address order
      if (boundary->address <= current_boundary->address) {
        if ( previous_boundary == NULL ) {
          sl_slist_push(&mpam_boundary_list, &(boundary->node));
        } else {
          sl_slist_insert(&(boundary->node), &(previous_boundary->node));
        }
        return;
      }
      previous_boundary = current_boundary;
    }
    // Reached the end of the list, therefore the boundary will be placed
    // at the end of the list.
    sl_slist_insert(&(boundary->node), &(previous_boundary->node));
  }
}

/***************************************************************************//**
 * @brief
 *  Add an MPA region to the mpam_region_list, as well as its corresponding
 *  boundaries into the mpam_boundary_list.
 ******************************************************************************/
static void register_region_handle(sl_mpa_manager_region_t* handle)
{
  sl_slist_push(&mpam_region_list, &(handle->node));
  insert_into_boundary_list(&(handle->start_boundary));
  insert_into_boundary_list(&(handle->end_boundary));
}

/***************************************************************************//**
 * @brief
 *  Remove an MPA region from the mpam_region_list, as well as its corresponding
 *  boundaries from the mpam_boundary_list.
 ******************************************************************************/
static void unregister_region_handle(sl_mpa_manager_region_t* handle)
{
  sl_slist_remove(&mpam_region_list, &(handle->node));
  sl_slist_remove(&mpam_boundary_list, &(handle->start_boundary.node));
  sl_slist_remove(&mpam_boundary_list, &(handle->end_boundary.node));
}

/***************************************************************************//**
 * @brief
 *   Returns true if the MPA region is currently registered. Registered MPA
 *   regions are those that are present in the mpam_region_list.
 ******************************************************************************/
static bool is_region_handle_registered(sl_mpa_manager_region_t* handle)
{
  sl_slist_node_t* iter;

  SL_SLIST_FOR_EACH(mpam_region_list, iter) {
    if ( iter == &(handle->node) ) {
      return true;
    }
  }
  return false;
}

#ifdef __cplusplus
}
#endif
