/***************************************************************************//**
 * @file
 * @brief malloc() and free() implemented on top of buffers.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef BUFFER_MALLOC_H
#define BUFFER_MALLOC_H

// An implementation of malloc() and free() that uses buffers.  malloc()
// returns a pointer to the contents of a buffer.  Freed buffers are kept
// on a list and reused; a new buffer is allocated only if no buffer on
// the freelist is large enough to be used.
//
// The buffers are not marked by this code.  sli_legacy_buffer_manager_malloc_free_list must be
// set to NULL_BUFFER before calling sli_legacy_buffer_manager_reclaim_unused_buffers().

extern sli_buffer_manager_buffer_t sli_legacy_buffer_manager_malloc_free_list;

void *sli_legacy_buffer_manager_buffer_malloc(uint16_t size);

void sli_legacy_buffer_manager_buffer_free(void *pointer);

#endif // BUFFER_MALLOC_H
