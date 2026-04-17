/***************************************************************************//**
 * @file library_baremetal_wrapper.c
 * @brief internal implementations for 'library' as a thin-wrapper
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
// automatically generated from library.h.  Do not manually edit
#include "stack/include/library.h"
#include "stack/internal/inc/library_internal_def.h"

sl_zigbee_library_status_t sl_zigbee_get_library_status(sl_zigbee_library_id_t libraryId)
{
  return sli_zigbee_stack_get_library_status(libraryId);
}
