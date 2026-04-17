/***************************************************************************//**
 * @file
 * @brief Memory Protection and Attribute Manager Interface.
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

#ifndef _SL_MPA_MANAGER_H
#define _SL_MPA_MANAGER_H

#include "sl_common.h"
#include "sl_enum.h"
#include "sl_slist.h"
#include "sl_status.h"
#include "sli_mpa_manager_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * @addtogroup mpa_manager Memory Protection and Attribute Manager (MPAM)
 *
 * @details
 *
 * ## Overview
 * The Memory Protection and Attribute Manager (MPAM) is a software module
 * that provides an interface to configure memory protection and attributes
 * for different regions of memory. It is designed to work with the
 * hardware memory protection features of the device, such as the ARM Cortex-M
 * MPU.
 *
 * The MPAM allows you to define memory regions with specific attributes,
 * such as non-cacheable, non-shareable, non-executable, and read-only. It also
 * provides functions to configure or disable these regions, as well as to
 * allocate/free memory for MPA region handles.
 *
 * The MPAM supports the following operations:
 *   - Allocate memory for an MPA region handle using
 *     @ref sl_mpa_manager_alloc_region_handle
 *     "sl_mpa_manager_alloc_region_handle".
 *   - Configure a memory region with a specific attribute using
 *     @ref sl_mpa_manager_configure_region "sl_mpa_manager_configure_region".
 *   - Disable a memory region using @ref sl_mpa_manager_disable_region
 *   "sl_mpa_manager_disable_region".
 *   - Free the memory allocated for an MPA region handle using
 *   @ref sl_mpa_manager_free_region_handle
 *   "sl_mpa_manager_free_region_handle".
 *
 *
 * ## Initialization
 *
 * The MPAM is initialized using the @ref sl_mpa_manager_init
 * "sl_mpa_manager_init" function. This function must be called
 * before any other MPAM functions can be used.
 *
 * ## Usage
 *
 * The following code snippet shows how to allocate memory for an MPA region
 * handle and configure a memory region with specific attributes:
 *
 * @code{.c}
 * sl_status_t status = SL_STATUS_OK;
 * sl_mpa_manager_region_t* handle = NULL;
 *
 * // Allocate memory for an MPA region handle
 * status = sl_mpa_manager_alloc_region_handle(&handle);
 * if (status != SL_STATUS_OK) {
 *   // Handle the error.
 * }
 *
 * // Configure a memory region with specific attributes
 * status = sl_mpa_manager_configure_region(handle,
 *                                          (void*)0x20000000,
 *                                          0x1000,
 *                                          SL_MPA_MANAGER_ATTRIBUTE_NON_CACHEABLE);
 * if (status != SL_STATUS_OK) {
 *   // Handle the error.
 * }
 * @endcode
 *
 * Disabling a memory region will remove the configuration from the hardware
 * and free the resources associated with the region handle. Assuming a region
 * was configured as in the previous code snippet, the following code snippet
 * shows how to disable the same MPA region and free the MPA region handle:
 *
 * @code{.c}
 * // Disable the MPA region
 * status = sl_mpa_manager_disable_region(handle);
 * if (status != SL_STATUS_OK) {
 *   // Handle the error.
 * }
 *
 * // Free the memory allocated for the MPA region handle
 * status = sl_mpa_manager_free_region_handle(handle);
 * if (status != SL_STATUS_OK) {
 *   // Handle the error.
 * }
 * @endcode
 *
 * ## Architecture Support
 *
 * The Memory Protection and Attribute Manager supports the following
 * architecture(s):
 * - ARMv8m
 *
 * ### ARMv8m Limitation
 * For ARMv8m devices, MPA regions can only be defined for memory addresses
 * that are of the Normal memory type in the ARMv8m system memory map. These
 * Normal memory type address spaces are as follows:
 *
 *  Region        | Address Space
 *  ------------- | ------------------------
 *  CODE          | 0x00000000 to 0x1FFFFFFF
 *  SRAM          | 0x20000000 to 0x3FFFFFFF
 *  RAM           | 0x60000000 to 0x7FFFFFFF
 *  RAM           | 0x80000000 to 0x9FFFFFFF
 *
 * More information about the ARMv8m system memory map can be found in the
 * [ARMv8m Architecture Reference Manual](https://developer.arm.com/
 * documentation/ddi0553/latest/).
 * @{
 *****************************************************************************/

/****************************************************************************//*
 *********************************   ENUMS   ***********************************
 ******************************************************************************/

/// @brief MPA Manager region attribute type.
SL_ENUM(sl_mpa_manager_attribute_t) {
  /// Region is non-cacheable.
  SL_MPA_MANAGER_ATTRIBUTE_NON_CACHEABLE = 0,
  /// Region is non-cacheable only when the device has a unified cache.
  SL_MPA_MANAGER_ATTRIBUTE_NON_CACHEABLE_UNIFIED_CACHE,
  /// Region is shareable.
  SL_MPA_MANAGER_ATTRIBUTE_NON_SHAREABLE,
  /// Region is non-executable.
  SL_MPA_MANAGER_ATTRIBUTE_NON_EXECUTABLE,
  /// Region is read-only
  SL_MPA_MANAGER_ATTRIBUTE_READ_ONLY,
  SL_MPA_MANAGER_ATTRIBUTE_MAX,
};

/****************************************************************************//*
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief MPA Manager region structure.
 * @details
 *  This structure is used to represent a memory region, with a given base
 *  address and size, that is configured with a specific attribute. Multiple
 *  MPA regions can overlap, and the MPA Manager will handle the configuration
 *  of the hardware to ensure that the memory protection and attributes are
 *  applied correctly.
 ******************************************************************************/
typedef struct sl_mpa_manager_region {
  /// List node for the MPA Manager region.
  sl_slist_node_t                   node;
  /// Base address of the memory region (Multiple of 32 bytes).
  void*                             base_address;
  /// Size of the memory region, in bytes (Multiple of 32 bytes).
  size_t                            size;
  /// Attribute to be applied to the memory region (Cannot be OR-ed).
  sl_mpa_manager_attribute_t        attribute;
  /// The start boundary of the MPA region.
  sli_mpa_manager_boundary_t        start_boundary;
  /// The end boundary of the MPA region.
  sli_mpa_manager_boundary_t        end_boundary;
} sl_mpa_manager_region_t;

/****************************************************************************//*
 ******************************** PROTOTYPES ***********************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief Initializes the MPA Manager.
 *
 * @details
 * This function intializes the MPA manager. By default, the
 * default memory map attributes for the target hardware applies.
 ******************************************************************************/
void sl_mpa_manager_init(void);

/***************************************************************************//**
 * @brief Configure an MPA region.
 *
 * @details
 * This function will attempt to configure an MPA region in the MPA Manager.
 * Doing this triggers a re-configuration of the hardware, and if the
 * hardware cannot be re-configured, then the MPA region will not be configured
 * in the MPA Manager. Whenever this function is not successful, the state of
 * the program will remain as it was before the function was called.
 *
 * This function can be used to add a new, or modify an existing MPA region
 * for which the caller has a <tt>handle</tt>, provided that the modification can be
 * accomodated by the hardware.
 *
 * @param[in] handle
 *     The MPA region handle.
 * @param[in] base_address
 *     The base address of the memory region (Multiple of 32 bytes).
 * @param[in] size
 *     The size of the memory region, in bytes (Multiple of 32 bytes).
 * @param[in] attribute
 *     The attribute to be applied to the memory region (Cannot be OR-ed).
 *
 * @return <b>SL_STATUS_OK</b>:
 *     - the MPA region is configured successfully.
 * @return <b>SL_STATUS_INVALID_PARAMETER</b>:
 *     - the MPA region <tt>handle</tt> is NULL.
 *     - the <tt>base_address</tt> is not a multiple of 32 bytes.
 *     - the <tt>size</tt> is not a multiple of 32 bytes.
 *     - the <tt>attribute</tt> is not valid.
 * @return <b>SL_STATUS_FAIL</b>:
 *     - the hardware is not able to be re-configured with the MPA region
 *     provided.
 *
 * @note If <tt>base_address</tt> or <tt>size</tt> parameters are both not a
 * multiple of 32 bytes, the caller of this function must round them to the
 * nearest 32 bytes.
 *
 * @note The <tt>attribute</tt> parameter is an enum type which cannot be OR-ed,
 * therefore, in order to apply multiple attributes to a memory range,
 * individual MPA regions must be configured.
 ******************************************************************************/
sl_status_t sl_mpa_manager_configure_region(sl_mpa_manager_region_t* handle,
                                            void* base_address,
                                            size_t size,
                                            sl_mpa_manager_attribute_t attribute);

/***************************************************************************//**
 * @brief Disable an MPA region.
 *
 * @details
 * This function will attempt to disable an MPA region in the MPA Manager. Doing
 * this triggers a re-configuration of the hardware, and if the hardware cannot
 * be re-configured, then the MPA region will not be disabled in the MPA
 * Manager. Whenever this function is not successful, the state of the program
 * will remain as it was before the function was called.
 *
 * @param[in] handle
 *     The MPA region handle.
 *
 * @return <b>SL_STATUS_OK</b>:
 *     - the MPA region is disabled successfully.
 * @return <b>SL_INVALID_PARAMETER</b>:
 *     - the MPA region <tt>handle</tt> is NULL.
 * @return <b>SL_STATUS_FAIL</b>:
 *     - the hardware is not able to be re-configured after disabling the MPA
 *     region.
 ******************************************************************************/
sl_status_t sl_mpa_manager_disable_region(sl_mpa_manager_region_t* handle);

/***************************************************************************//**
 * @brief Get the size of the MPA region data structure.
 *
 * @details
 * This function is used to get the data structure size of an MPA
 * region which can be used for custom memory allocation of an MPA region's
 * data.
 *
 * @return The size of the MPA region data structure.
 ******************************************************************************/
size_t sl_mpa_manager_get_region_handle_size(void);

/***************************************************************************//**
 * @brief Allocate memory for an MPA region on the heap.
 *
 * @details
 * This function allocates memory for an MPA region on the heap
 * using the memory manager, and then returns a <tt>handle</tt> for that MPA
 * region.
 *
 * @param[out] handle
 *      Pointer to the handle of the allocated MPA region. NULL if the
 *      allocation fails.
 *
 * @return <b>SL_STATUS_OK</b>:
 *     - the MPA region memory is allocated successfully.
 * @return <b>SL_STATUS_NULL_POINTER</b>:
 *     - the pointer to the <tt>handle</tt> is a NULL pointer.
 * @return <b>SL_STATUS_ALLOCATION_FAILED</b>:
 *     - the failed to allocate memory for the region.
 ******************************************************************************/
sl_status_t sl_mpa_manager_alloc_region_handle(sl_mpa_manager_region_t** handle);

/***************************************************************************//**
 * @brief Free the MPA region memory allocated.
 *
 * @details
 * This function frees the MPA region handle. If the MPA region is
 * not disabled, the MPA region <tt>handle</tt> will not be freed.
 *
 * @param[in] handle
 *     Pointer to the handle of an MPA region.
 *
 * @return <b>SL_STATUS_OK</b>:
 *     - the MPA region's data was freed successfully.
 * @return <b>SL_INVALID_PARAMETER</b>:
 *     - the MPA region <tt>handle</tt> is NULL.
 * @return <b>SL_STATUS_INVALID_STATE</b>:
 *     - the MPA region is currently configured and must be disabled before
 *     freeing it.
 * @return <b>SL_STATUS_FAIL</b>:
 *     - the MPA region's data was not freed successfully.
 ******************************************************************************/
sl_status_t sl_mpa_manager_free_region_handle(sl_mpa_manager_region_t* handle);

/** @} (end addtogroup mpa_manager) */

#ifdef __cplusplus
}
#endif

#endif /* _SL_MPA_MANAGER_H */
