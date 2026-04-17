/***************************************************************************//**
 * @file
 * @brief Stub implementation of memory profiler
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sli_memory_profiler.h"
#include "sl_status.h"

/* Create a memory tracker */
sl_status_t sli_memory_profiler_create_tracker(sli_memory_tracker_handle_t tracker_handle,
                                               const char *description)

{
  (void) tracker_handle;
  (void) description;
  return SL_STATUS_NOT_AVAILABLE;
}

/* Create a pool memory tracker */
sl_status_t sli_memory_profiler_create_pool_tracker(sli_memory_tracker_handle_t tracker_handle,
                                                    const char *description,
                                                    void* ptr,
                                                    size_t size)
{
  (void) tracker_handle;
  (void) description;
  (void) ptr;
  (void) size;
  return SL_STATUS_NOT_AVAILABLE;
}

/* Add or update a description to a previously created memory tracker */
void sli_memory_profiler_describe_tracker(sli_memory_tracker_handle_t tracker_handle,
                                          const char *description)
{
  (void) tracker_handle;
  (void) description;
}

/* Delete a memory tracker */
void sli_memory_profiler_delete_tracker(sli_memory_tracker_handle_t tracker_handle)
{
  (void) tracker_handle;
}

/* Track the allocation of a memory block */
void sli_memory_profiler_track_alloc(sli_memory_tracker_handle_t tracker_handle, void * ptr, size_t size)
{
  (void) tracker_handle;
  (void) ptr;
  (void) size;
}

/* Track the allocation of a memory block and record ownership */
void sli_memory_profiler_track_alloc_with_ownership(sli_memory_tracker_handle_t tracker_handle,
                                                    void * ptr,
                                                    size_t size,
                                                    void * pc)
{
  (void) tracker_handle;
  (void) ptr;
  (void) size;
  (void) pc;
}

/* Track the freeing of a memory block */
void sli_memory_profiler_track_free(sli_memory_tracker_handle_t tracker_handle, void * ptr)
{
  (void) tracker_handle;
  (void) ptr;
}

/* Track the transfer of memory allocation ownership */
void sli_memory_profiler_track_ownership(sli_memory_tracker_handle_t tracker_handle,
                                         void * ptr,
                                         void * pc)
{
  (void) tracker_handle;
  (void) ptr;
  (void) pc;
}

/* Trigger the creation of a snapshot of the current state */
void sli_memory_profiler_take_snapshot(const char *name)
{
  (void) name;
}

/* Send a generic log */
void sli_memory_profiler_log(uint32_t log_id,
                             uint32_t arg1,
                             uint32_t arg2,
                             uint32_t arg3,
                             void * pc)
{
  (void) log_id;
  (void) arg1;
  (void) arg2;
  (void) arg3;
  (void) pc;
}
