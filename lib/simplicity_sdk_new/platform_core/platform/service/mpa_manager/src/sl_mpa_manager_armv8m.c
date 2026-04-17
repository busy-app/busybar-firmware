/***************************************************************************//**
 * @file
 * @brief Memory Protection and Attribute Manager ARMv8m Implementation.
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
#include "sl_enum.h"
#include "sl_slist.h"
#include "sl_status.h"
#include "sl_memory_manager.h"
#include "em_device.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

#define MPU_REGION_COUNT              16
#define MPU_ATTRIBUTE_COUNTER_WIDTH   8
#define MPU_ATTRIBUTE_COUNTER_MASK    0xFFU

/*******************************************************************************
 *********************************   ENUMS   ***********************************
 ******************************************************************************/

SL_ENUM(mpu_attribute_t) {
  MPU_ATTRIBUTE_NON_CACHEABLE = 0,
  MPU_ATTRIBUTE_NON_SHAREABLE,
  MPU_ATTRIBUTE_NON_EXECUTABLE,
  MPU_ATTRIBUTE_READ_ONLY,
  MPU_ATTRIBUTE_MAX,
};

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

/// ARM MPU Region attributes
typedef union mpu_region_attributes {
  struct {
    uint32_t non_cacheable  : 1;
    uint32_t non_shareable  : 1;
    uint32_t non_executable : 1;
    uint32_t read_only      : 1;
    uint32_t reserved       : 27;
  };
  uint32_t value;
} mpu_region_attributes_t;

/// ARM MPU Region attribute counters
typedef union mpu_attribute_counters {
  struct {
    uint32_t non_cacheable : MPU_ATTRIBUTE_COUNTER_WIDTH;
    uint32_t non_shareable : MPU_ATTRIBUTE_COUNTER_WIDTH;
    uint32_t non_executable : MPU_ATTRIBUTE_COUNTER_WIDTH;
    uint32_t read_only : MPU_ATTRIBUTE_COUNTER_WIDTH;
  };
  uint32_t value;
} mpu_attribute_counters_t;

/*******************************************************************************
 ***************************   LOCAL VARIABLES   *******************************
 ******************************************************************************/
static ARM_MPU_Region_t mpu_table_buffer[MPU_REGION_COUNT];

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/
static void buffer_mpu_region(uint32_t table_idx,
                              uint32_t base,
                              uint32_t limit,
                              mpu_region_attributes_t mpu_region_attributes);
static mpu_attribute_t map_region_attribute_to_mpu_attribute(sl_mpa_manager_attribute_t attribute);
static mpu_region_attributes_t encode_mpu_region_attributes(mpu_attribute_counters_t counters);
static void update_attribute_counters(mpu_attribute_counters_t * counters,
                                      sli_mpa_manager_boundary_t* boundary);
static void disable_and_clear_mpu(void);

/*******************************************************************************
 ***************************   GLOBAL FUNCTIONS  *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initializes the device-specific portions of the MPA Manager.
 ******************************************************************************/
void sli_mpa_manager_device_init(void)
{
  // The ARMv8 MPU has a limited number of Memory Attribute Indirection Registers.
  // Here, the MAIRs at indexes 0 and 1 are set-up and will be used to specify
  // whether the MPU regions are cacheable or non-cacheable.

  //MAIR0 has Cacheable, Non-Transient, Write-Through, Read-Allocate attributes.
  ARM_MPU_SetMemAttr(0,                                             // MAIR0
                     ARM_MPU_ATTR(ARM_MPU_ATTR_MEMORY_(1, 0, 1, 0), // Outer
                                  ARM_MPU_ATTR_MEMORY_(1, 0, 1, 0)) // Inner
                     );

  //MAIR1 has Non-Cacheable attributes.
  ARM_MPU_SetMemAttr(1,                                       // MAIR1
                     ARM_MPU_ATTR(ARM_MPU_ATTR_NON_CACHEABLE, // Outer
                                  ARM_MPU_ATTR_NON_CACHEABLE) // Inner
                     );

  // The MPU is disabled, cleared and then enabled. The default memory map
  // still applies for any addresses that are not covered by an MPU region.
  disable_and_clear_mpu();
  ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);
}

/***************************************************************************//**
 * @brief
 *   Attempts to re-configure the memory protection hardware in order to
 *   apply all currently configured mpa manager regions.
 ******************************************************************************/
sl_status_t sli_mpa_manager_device_configure(sl_slist_node_t* boundary_list)
{
  mpu_attribute_counters_t counters =
    (mpu_attribute_counters_t) {
    .value = 0,
  };
  mpu_region_attributes_t current_mpu_region_attributes =
    (mpu_region_attributes_t) {
    .value = 0
  };
  sli_mpa_manager_boundary_t* current_start_boundary = NULL;
  sli_mpa_manager_boundary_t* next_start_boundary = NULL;
  sl_slist_node_t* boundary_list_iterator = boundary_list;
  uint32_t current_mpu_region_base_address;
  uint32_t current_mpu_table_idx = 0;

  if ( sl_slist_is_empty(boundary_list) ) {
    // If the boundary list is empty, the MPU should not configure any
    // MPU regions, therefore clear it here.
    disable_and_clear_mpu();
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);
    return SL_STATUS_OK;
  }

  // The first boundary in the boundary list will always be the start of
  // the first MPU region, therefore we can consider it as the current
  // start boundary.
  current_start_boundary = SL_SLIST_ENTRY(boundary_list_iterator, sli_mpa_manager_boundary_t, node);
  update_attribute_counters(&counters, current_start_boundary);
  boundary_list_iterator = current_start_boundary->node.node;

  // Find and buffer each MPU region by iterating through the list of
  // boundaries.
  while ( boundary_list_iterator != NULL ) {
    // Boundaries can have the same address, therefore the current MPU region
    // will need to account for the attributes of those boundaries.
    SL_SLIST_FOR_EACH_ENTRY(boundary_list_iterator, next_start_boundary, sli_mpa_manager_boundary_t, node) {
      if ( current_start_boundary->address != next_start_boundary->address) {
        break;
      }
      update_attribute_counters(&counters, next_start_boundary);
      current_start_boundary = next_start_boundary;
    }

    // Update the master boundary list iterator in case we've fast-forwarded
    // through boundaries with the same address.
    boundary_list_iterator = &(next_start_boundary->node);

    // The address of the current start boundary is the base address of the current
    // MPU region. The current MPU region will be buffered once we've found its
    // limit.
    current_mpu_region_base_address = current_start_boundary->address;

    // The attributes of the current MPU region are determined by looking at
    // the MPU attribute counters, and any attribute counter that is non-zero
    // means that the attribute is active.
    current_mpu_region_attributes = encode_mpu_region_attributes(counters);

    // Count attributes until the encoded attributes change from the current
    // MPU region's attributes.
    SL_SLIST_FOR_EACH_ENTRY(boundary_list_iterator, next_start_boundary, sli_mpa_manager_boundary_t, node) {
      update_attribute_counters(&counters, next_start_boundary);

      // Check to see if the encoded attributes change after updating the
      // attribute counters.
      mpu_region_attributes_t next_mpu_region_attributes = encode_mpu_region_attributes(counters);
      if (current_mpu_region_attributes.value != next_mpu_region_attributes.value) {
        // Now that the next MPU region's start has been found, buffer an MPU
        // region for the current MPU region, only if the current MPU region
        // has any attributes to apply to the memory map. The address of the
        // next start boundary becomes the limit of the current MPU region start.
        if ( current_mpu_region_attributes.value != 0 ) {
          if ( current_mpu_table_idx >= MPU_REGION_COUNT ) {
            return SL_STATUS_ALLOCATION_FAILED;
          }

          buffer_mpu_region(current_mpu_table_idx,
                            current_mpu_region_base_address,  // base_address
                            next_start_boundary->address,     // limit
                            current_mpu_region_attributes);
          current_mpu_table_idx++;
        }

        // Go to the next iteration of the outer loop, the next_start_boundary
        // will be the current_start_boundary for the next iteration of the
        // loop.
        break;
      }
    }

    // If the next start boundary is NULL, we've reached the end of the list,
    // and all MPU regions have been buffered.
    if (next_start_boundary == NULL) {
      break;
    } else {
      // The next start boundary will be the current start boundary of the next
      // iteration.
      current_start_boundary = next_start_boundary;
      boundary_list_iterator = current_start_boundary->node.node;
    }
  }

  disable_and_clear_mpu();
  ARM_MPU_Load(0, (ARM_MPU_Region_t const*)&mpu_table_buffer, current_mpu_table_idx);
  ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * @brief
 *  Validates the base address and size of an MPA region to ensure that they
 *  respect the limitations that are imposed by the device's architecture.
 ******************************************************************************/
bool sli_mpa_manager_device_region_validate(uint32_t base_address,
                                            uint32_t size)
{
  // The base and end addresses must be within one of the memory ranges in the
  // ARMv8m system memory map that are designated as Normal memory types.
  uint32_t end_address = base_address + size;

  // CODE 0x00000000 - 0x1FFFFFFF
  // SRAM 0x20000000 - 0x3FFFFFFF
  if (base_address <= 0x3FFFFFFFul && end_address <= 0x3FFFFFFFul) {
    return true;
  }

  // RAM  0x60000000 - 0x7FFFFFFF
  // RAM  0x80000000 - 0x9FFFFFFF
  if ((base_address >= 0x60000000ul && end_address >= 0x60000000ul)
      && (base_address <= 0x9FFFFFFFul && end_address <= 0x9FFFFFFFul)) {
    return true;
  }

  return false;
}

/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *  Buffers an ARM_MPU_Region_t region in the statically allocated
 *  mpu_table_buffer at a given index, with a given set of MPU region
 *  attributes.
 ******************************************************************************/
static void buffer_mpu_region(uint32_t table_idx,
                              uint32_t base,
                              uint32_t limit,
                              mpu_region_attributes_t mpu_region_attributes)
{
  // Pull-back the MPU region limit address by 32 bytes to account for
  // the fact that the limit address is postfixed with 0x1F in hardware.
  ARM_MPU_Region_t arm_mpu_region = (ARM_MPU_Region_t) {
    .RBAR = ARM_MPU_RBAR(base,
                         mpu_region_attributes.non_shareable ? 0 : 1,
                         mpu_region_attributes.read_only ? 1 : 0,
                         1,       // Non-Privileged
                         mpu_region_attributes.non_executable ? 1 : 0),
    .RLAR = ARM_MPU_RLAR((limit - 32U),
                         mpu_region_attributes.non_cacheable ? 1 : 0),
  };
  mpu_table_buffer[table_idx] = arm_mpu_region;
}

/***************************************************************************//**
 * @brief
 *   Returns the equivalent MPU attribute for a given mpa manager attribute.
 ******************************************************************************/
static mpu_attribute_t map_region_attribute_to_mpu_attribute(sl_mpa_manager_attribute_t attribute)
{
  mpu_attribute_t mpu_attribute = MPU_ATTRIBUTE_MAX;

  switch ( attribute ) {
    case SL_MPA_MANAGER_ATTRIBUTE_NON_CACHEABLE:
      mpu_attribute = MPU_ATTRIBUTE_NON_CACHEABLE;
      break;

    case SL_MPA_MANAGER_ATTRIBUTE_NON_CACHEABLE_UNIFIED_CACHE:
#ifdef SL_DEVICE_HAS_UNIFIED_CACHE
      mpu_attribute = MPU_ATTRIBUTE_NON_CACHEABLE;
#else
      // mpu_attribute returned will be MPU_ATTRIBUTE_MAX, which
      // will be ignored when incrementing/decrementing the attribute
      // counters.
#endif
      break;

    case SL_MPA_MANAGER_ATTRIBUTE_NON_SHAREABLE:
      mpu_attribute = MPU_ATTRIBUTE_NON_SHAREABLE;
      break;

    case SL_MPA_MANAGER_ATTRIBUTE_NON_EXECUTABLE:
      mpu_attribute = MPU_ATTRIBUTE_NON_EXECUTABLE;
      break;

    case SL_MPA_MANAGER_ATTRIBUTE_READ_ONLY:
      mpu_attribute = MPU_ATTRIBUTE_READ_ONLY;
      break;

    case SL_MPA_MANAGER_ATTRIBUTE_MAX:
    default:
      EFM_ASSERT(false);
      break;
  }

  return mpu_attribute;
}

/***************************************************************************//**
 * @brief
 *  Encodes the current attribute counters into an integer with flags for each
 *  attribute that can be applied to an MPU region. These attributes are applied
 *  if the attribute counter of a given attribute is greater than zero.
 ******************************************************************************/
static mpu_region_attributes_t encode_mpu_region_attributes(mpu_attribute_counters_t counters)
{
  mpu_region_attributes_t attributes;
  attributes.value = 0;

  for ( mpu_attribute_t attribute = 0; attribute < MPU_ATTRIBUTE_MAX; attribute++) {
    uint32_t count = (counters.value
                      & (MPU_ATTRIBUTE_COUNTER_MASK << (MPU_ATTRIBUTE_COUNTER_WIDTH * attribute)));
    if ( count > 0 ) {
      attributes.value |= (1U << attribute);
    }
  }

  return attributes;
}

/***************************************************************************//**
 * @brief
 *  Increments the corresponding mpa region's attribute counter if the boundary
 *  is a start_boundary, and decrements the same counter if the boundary is
 *  an end_boundary.
 ******************************************************************************/
static void update_attribute_counters(mpu_attribute_counters_t * counters,
                                      sli_mpa_manager_boundary_t * boundary)
{
  sl_mpa_manager_region_t* region = (sl_mpa_manager_region_t*)boundary->region;
  sl_mpa_manager_attribute_t mpam_attr = region->attribute;
  mpu_attribute_t mpu_attr = map_region_attribute_to_mpu_attribute(mpam_attr);

  if (mpu_attr == MPU_ATTRIBUTE_MAX) {
    return;
  }

  if ( boundary->is_end ) {
    counters->value -= 1U << (mpu_attr * MPU_ATTRIBUTE_COUNTER_WIDTH);
  } else {
    counters->value += 1U << (mpu_attr * MPU_ATTRIBUTE_COUNTER_WIDTH);
  }
}

/***************************************************************************//**
 * @brief
 *  Resets the MPU by disabling it and clearing all the regions. Must be
 *  re-enabled after this function is called.
 ******************************************************************************/
static void disable_and_clear_mpu(void)
{
  ARM_MPU_Disable();
  // After buffering the new MPU regions, clear the MPU table and load the
  // MPU table buffer into the MPU.
  for ( uint8_t i = 0; i < MPU_REGION_COUNT; i++) {
    MPU->RNR = i;
    MPU->RBAR = 0U;
    MPU->RLAR = 0U;
  }
}

#ifdef __cplusplus
}
#endif
