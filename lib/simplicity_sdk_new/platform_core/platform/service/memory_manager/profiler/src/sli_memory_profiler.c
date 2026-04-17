/***************************************************************************//**
 * @file
 * @brief Implementation of memory profiler
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

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "sli_memory_profiler.h"
#include "sli_memory_profiler_config.h"
#include "sl_common.h"
#include "em_core.h"
#include "SEGGER_RTT.h"
#include "sl_rtt_buffer_index.h"

// The type byte is composed of flag bits and a field for the event ID
#define SLI_MEMPROF_EVENT_FLAG_HAS_CHECKSUM ((uint8_t) 0x80)
#define SLI_MEMPROF_EVENT_ID_MASK           ((uint8_t) 0x0F)

// Event IDs used in "id" field of "sli_memprof_evt_hdr_t"
#define SLI_MEMPROF_EVENT_INIT_ID                 1
#define SLI_MEMPROF_EVENT_CREATE_POOL_TRACKER_ID  2
#define SLI_MEMPROF_EVENT_CREATE_TRACKER_ID       3
#define SLI_MEMPROF_EVENT_DESCRIBE_TRACKER_ID     4
#define SLI_MEMPROF_EVENT_DELETE_TRACKER_ID       5
#define SLI_MEMPROF_EVENT_TRACK_ALLOC_ID          6
#define SLI_MEMPROF_EVENT_TRACK_REALLOC_ID        7
#define SLI_MEMPROF_EVENT_TRACK_FREE_ID           8
#define SLI_MEMPROF_EVENT_TRACK_OWNERSHIP_ID      9
#define SLI_MEMPROF_EVENT_TAKE_SNAPSHOT_ID        10
#define SLI_MEMPROF_EVENT_LOG_ID                  11

// Version of the binary format for the events we send over RTT
#define SLI_MEMPROF_EVENT_FORMAT_VERSION 9

/**
 * @brief Maximum length of a tracker description string
 */
#define MAX_TRACKER_DESCRIPTION_LEN 32

/**
 * @brief Maximum length of snapshot name
 */
#define MAX_SNAPSHOT_NAME_LEN 32

/**
 * @brief Event header structure
 */
typedef __PACKED_STRUCT {
  uint8_t seq_num; ///< Sequence number of the event
  uint8_t type;    ///< Flag bits and the ID of the event
  uint8_t len;     ///< Length of the event payload
} sli_memprof_evt_hdr_t;

/**
 * @brief Event structure for the "init" event
 */
typedef __PACKED_STRUCT {
  sli_memprof_evt_hdr_t header; ///< The common event header
  uint8_t format_version;       ///< Version of the event format to verify binary compatibility
} sli_memprof_evt_init_t;

/**
 * @brief Event structure for the "create_pool_tracker" event
 */
typedef __PACKED_STRUCT {
  sli_memprof_evt_hdr_t header;                       ///< The common event header
  uint32_t tracker_handle;                            ///< Handle of the pool tracker that was created
  uint32_t ptr;                                       ///< Pointer to the pool block allocated from the parent memory
  uint32_t size;                                      ///< Size of the pool block allocated from the parent memory
  uint8_t  flags;                                     ///< Reserved for future flag bits. Set to 0 for now.
  uint8_t  description[MAX_TRACKER_DESCRIPTION_LEN];  ///< Short human-readable description
} sli_memprof_evt_create_pool_tracker_t;

/**
 * @brief Event structure for the "create_tracker" event
 */
typedef __PACKED_STRUCT {
  sli_memprof_evt_hdr_t header;                       ///< The common event header
  uint32_t tracker_handle;                            ///< Handle of the tracker that was created
  uint8_t  flags;                                     ///< Reserved for future flag bits. Set to 0 for now.
  uint8_t  description[MAX_TRACKER_DESCRIPTION_LEN];  ///< Short human-readable description
} sli_memprof_evt_create_tracker_t;

/**
 * @brief Event structure for the "describe_tracker" event
 */
typedef __PACKED_STRUCT {
  sli_memprof_evt_hdr_t header;                       ///< The common event header
  uint32_t tracker_handle;                            ///< Handle of the tracker
  uint8_t  description[MAX_TRACKER_DESCRIPTION_LEN];  ///< Short human-readable description
} sli_memprof_evt_describe_tracker_t;

/**
 * @brief Event structure for the "delete_tracker" event
 */
typedef __PACKED_STRUCT {
  sli_memprof_evt_hdr_t header; ///< The common event header
  uint32_t tracker_handle;      ///< Handle of the pool tracker that was deleted
} sli_memprof_evt_delete_tracker_t;

/**
 * @brief Event structure for the "track_alloc" event
 */
typedef __PACKED_STRUCT {
  sli_memprof_evt_hdr_t header; ///< The common event header
  uint32_t tracker_handle;      ///< Handle of the tracker
  uint32_t ptr;                 ///< Pointer to the allocated memory or NULL if allocation failed
  uint32_t size;                ///< The number of bytes allocated, or attempted to allocate
  uint32_t pc;                  ///< The program counter at the location of the owner
} sli_memprof_evt_track_alloc_t;

/**
 * @brief Event structure for the "track_realloc" event
 */
typedef __PACKED_STRUCT {
  sli_memprof_evt_hdr_t header; ///< The common event header
  uint32_t tracker_handle;      ///< Handle of the tracker
  uint32_t ptr;                 ///< Pointer to the original memory block
  uint32_t realloced_ptr;       ///< Pointer to the resized or allocated memory
  uint32_t size;                ///< The size that the block was reallocated to
} sli_memprof_evt_track_realloc_t;

/**
 * @brief Event structure for the "track_free" event
 */
typedef __PACKED_STRUCT {
  sli_memprof_evt_hdr_t header; ///< The common event header
  uint32_t tracker_handle;      ///< Handle of the tracker
  uint32_t ptr;                 ///< Pointer to the free'd memory
} sli_memprof_evt_track_free_t;

/**
 * @brief Event structure for the "track_ownership" event
 */
typedef __PACKED_STRUCT {
  sli_memprof_evt_hdr_t header; ///< The common event header

  /// Handle of the tracker level at which the ownership is taken. This is used
  /// to disambiguate in cases where nested allocations start at the same memory
  /// location, and the caller is specifically taking ownership of one of the
  /// outer blocks that may contain smaller allocations that have their own
  /// (more detailed) owners. If set to `SLI_INVALID_MEMORY_TRACKER_HANDLE`, the
  /// ownership of the innermost allocation is taken.
  uint32_t tracker_handle;
  uint32_t ptr;  ///< Pointer to the allocated memory for which ownership is taken
  uint32_t pc;   ///< The program counter at the location that took ownership
} sli_memprof_evt_track_ownership_t;

/**
 * @brief Event structure for the "take_snapshot" event
 */
typedef __PACKED_STRUCT {
  sli_memprof_evt_hdr_t header;         ///< The common event header
  uint8_t  name[MAX_SNAPSHOT_NAME_LEN]; ///< Short human-readable name for the snapshot
} sli_memprof_evt_take_snapshot_t;

/**
 * @brief Event structure for the "log" event
 */
typedef __PACKED_STRUCT {
  sli_memprof_evt_hdr_t header; ///< The common event header
  uint32_t  log_id;             ///< The unique ID of the log
  uint32_t  arg1;               ///< ID-specific argument 1
  uint32_t  arg2;               ///< ID-specific argument 2
  uint32_t  arg3;               ///< ID-specific argument 3
  uint32_t  pc;                 ///< The program counter at the location of the log call
} sli_memprof_evt_log_t;

/** brief Set to true when the Memory Profiler has initialized */
static bool memory_profiler_initialized = false;

/** @brief The sequence number for the next event to send */
static uint8_t next_sequence_number = 0;

/**
 * @brief RTT buffer for sending events
 */
static uint8_t rtt_buffer[SLI_MEMORY_PROFILER_RTT_BUFFER_SIZE];

/**
 * @brief Send a memory profiler event over RTT
 *
 * This function fills the event header of the specified event structure and
 * sends the event over the RTT.
 *
 * @param[in] header Pointer to the header structure. The payload must follow
 *   right after the header in contiguous memory addresses
 * @param[in] id ID of the event
 * @param[in] event_len Total length of the event header and payload
 */
static void send_memory_profiler_event(sli_memprof_evt_hdr_t *header,
                                       uint8_t id,
                                       size_t event_len)
{
  // Initialize the header
  EFM_ASSERT(event_len >= sizeof(*header));
  header->type = id;
  header->len = event_len - sizeof(*header);
  header->seq_num = next_sequence_number++;

  // Send the event
  SEGGER_RTT_Write(SL_MEMORY_PROFILER_RTT_BUFFER_INDEX, header, event_len);
}

/**
 * @brief Initialize the memory profiler
 */
static void init_memory_profiler()
{
  // Callers must ensure that this internal init happens only once
  EFM_ASSERT(!memory_profiler_initialized);

  // Configure the RTT buffer for our events
  SEGGER_RTT_ConfigUpBuffer(SL_MEMORY_PROFILER_RTT_BUFFER_INDEX,
                            "sli_memory_profiler",
                            rtt_buffer,
                            sizeof(rtt_buffer),
                            SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);

  // Send the init event
  next_sequence_number = 0;
  sli_memprof_evt_init_t event;
  event.format_version = SLI_MEMPROF_EVENT_FORMAT_VERSION;
  send_memory_profiler_event(&event.header, SLI_MEMPROF_EVENT_INIT_ID, sizeof(event));

  memory_profiler_initialized = true;
}

/* Initialize the memory profiler */
void sli_memory_profiler_init()
{
  CORE_DECLARE_IRQ_STATE;

  // If we haven't initialized already, do it now
  CORE_ENTER_ATOMIC();
  if (!memory_profiler_initialized) {
    init_memory_profiler();
  }
  CORE_EXIT_ATOMIC();
}

/* Create a pool memory tracker */
sl_status_t sli_memory_profiler_create_pool_tracker(sli_memory_tracker_handle_t tracker_handle,
                                                    const char *description,
                                                    void* ptr,
                                                    size_t size)
{
  CORE_DECLARE_IRQ_STATE;

  // Fill the event structure
  sli_memprof_evt_create_pool_tracker_t event;
  event.tracker_handle = (uint32_t) (uintptr_t) tracker_handle;
  event.ptr = (uint32_t) (uintptr_t) ptr;
  event.size = (uint32_t) size;
  event.flags = 0;
  size_t description_len = 0;
  if (description != NULL) {
    description_len = strlen(description);
    if (description_len > sizeof(event.description)) {
      description_len = sizeof(event.description);
    }
    memcpy(event.description, description, description_len);
  }
  size_t event_len = sizeof(event) - sizeof(event.description) + description_len;

  CORE_ENTER_ATOMIC();

  // If we haven't initialized already, do it now
  if (!memory_profiler_initialized) {
    init_memory_profiler();
  }

  // Send the event
  send_memory_profiler_event(&event.header, SLI_MEMPROF_EVENT_CREATE_POOL_TRACKER_ID, event_len);

  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/* Create a memory tracker */
sl_status_t sli_memory_profiler_create_tracker(sli_memory_tracker_handle_t tracker_handle,
                                               const char *description)
{
  CORE_DECLARE_IRQ_STATE;

  // Fill the event structure
  sli_memprof_evt_create_tracker_t event;
  event.tracker_handle = (uint32_t) (uintptr_t) tracker_handle;
  event.flags = 0;
  size_t description_len = 0;
  if (description != NULL) {
    description_len = strlen(description);
    if (description_len > sizeof(event.description)) {
      description_len = sizeof(event.description);
    }
    memcpy(event.description, description, description_len);
  }
  size_t event_len = sizeof(event) - sizeof(event.description) + description_len;

  CORE_ENTER_ATOMIC();

  // If we haven't initialized already, do it now
  if (!memory_profiler_initialized) {
    init_memory_profiler();
  }

  // Send the event
  send_memory_profiler_event(&event.header, SLI_MEMPROF_EVENT_CREATE_TRACKER_ID, event_len);

  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/* Add or update a description to a previously created memory tracker */
void sli_memory_profiler_describe_tracker(sli_memory_tracker_handle_t tracker_handle,
                                          const char *description)
{
  CORE_DECLARE_IRQ_STATE;

  // Fill the event structure
  sli_memprof_evt_describe_tracker_t event;
  event.tracker_handle = (uint32_t) (uintptr_t) tracker_handle;
  size_t description_len = 0;
  if (description != NULL) {
    description_len = strlen(description);
    if (description_len > sizeof(event.description)) {
      description_len = sizeof(event.description);
    }
    memcpy(event.description, description, description_len);
  }
  size_t event_len = sizeof(event) - sizeof(event.description) + description_len;

  // Send the event atomically
  CORE_ENTER_ATOMIC();
  send_memory_profiler_event(&event.header, SLI_MEMPROF_EVENT_DESCRIBE_TRACKER_ID, event_len);
  CORE_EXIT_ATOMIC();
}

/* Delete a memory tracker */
void sli_memory_profiler_delete_tracker(sli_memory_tracker_handle_t tracker_handle)
{
  CORE_DECLARE_IRQ_STATE;

  // Fill the event structure
  sli_memprof_evt_delete_tracker_t event;
  event.tracker_handle = (uint32_t) (uintptr_t) tracker_handle;

  // Send the event atomically
  CORE_ENTER_ATOMIC();
  send_memory_profiler_event(&event.header, SLI_MEMPROF_EVENT_DELETE_TRACKER_ID, sizeof(event));
  CORE_EXIT_ATOMIC();
}

/* Track the allocation of a memory block */
void sli_memory_profiler_track_alloc(sli_memory_tracker_handle_t tracker_handle,
                                     void * ptr,
                                     size_t size)
{
  // Assign the ownership to the caller by default
  sli_memory_profiler_track_alloc_with_ownership(tracker_handle, ptr, size,
                                                 sli_memory_profiler_get_return_address());
}

/* Track the allocation of a memory block and record ownership */
void sli_memory_profiler_track_alloc_with_ownership(sli_memory_tracker_handle_t tracker_handle,
                                                    void * ptr,
                                                    size_t size,
                                                    void * pc)
{
  CORE_DECLARE_IRQ_STATE;

  // Fill the event structure
  sli_memprof_evt_track_alloc_t event;
  event.tracker_handle = (uint32_t) (uintptr_t) tracker_handle;
  event.ptr = (uint32_t) (uintptr_t) ptr;
  event.size = (uint32_t) size;
  event.pc = (uint32_t) (uintptr_t) pc;

  // Send atomically
  CORE_ENTER_ATOMIC();
  send_memory_profiler_event(&event.header, SLI_MEMPROF_EVENT_TRACK_ALLOC_ID, sizeof(event));
  CORE_EXIT_ATOMIC();
}

/* Track the reallocation of a memory block */
void sli_memory_profiler_track_realloc(sli_memory_tracker_handle_t tracker_handle,
                                       void * ptr,
                                       void * realloced_ptr,
                                       size_t size)
{
  CORE_DECLARE_IRQ_STATE;

  // Fill the event structure
  sli_memprof_evt_track_realloc_t event;
  event.tracker_handle = (uint32_t) (uintptr_t) tracker_handle;
  event.ptr = (uint32_t) (uintptr_t) ptr;
  event.realloced_ptr = (uint32_t) (uintptr_t) realloced_ptr;
  event.size = (uint32_t) size;

  // Send atomically
  CORE_ENTER_ATOMIC();
  send_memory_profiler_event(&event.header, SLI_MEMPROF_EVENT_TRACK_REALLOC_ID, sizeof(event));
  CORE_EXIT_ATOMIC();
}

/* Track the freeing of a memory block */
void sli_memory_profiler_track_free(sli_memory_tracker_handle_t tracker_handle,
                                    void * ptr)
{
  CORE_DECLARE_IRQ_STATE;

  // Fill the event structure
  sli_memprof_evt_track_free_t event;
  event.tracker_handle = (uint32_t) (uintptr_t) tracker_handle;
  event.ptr = (uint32_t) (uintptr_t) ptr;

  // Send atomically
  CORE_ENTER_ATOMIC();
  send_memory_profiler_event(&event.header, SLI_MEMPROF_EVENT_TRACK_FREE_ID, sizeof(event));
  CORE_EXIT_ATOMIC();
}

/* Track the transfer of memory allocation ownership */
void sli_memory_profiler_track_ownership(sli_memory_tracker_handle_t tracker_handle,
                                         void * ptr,
                                         void * pc)
{
  CORE_DECLARE_IRQ_STATE;

  // Fill the event structure
  sli_memprof_evt_track_ownership_t event;
  event.tracker_handle = (uint32_t) (uintptr_t) tracker_handle;
  event.ptr = (uint32_t) (uintptr_t) ptr;
  event.pc = (uint32_t) (uintptr_t) pc;

  // Send atomically
  CORE_ENTER_ATOMIC();
  send_memory_profiler_event(&event.header, SLI_MEMPROF_EVENT_TRACK_OWNERSHIP_ID, sizeof(event));
  CORE_EXIT_ATOMIC();
}

/* Trigger the creation of a snapshot of the current state */
void sli_memory_profiler_take_snapshot(const char *name)
{
  CORE_DECLARE_IRQ_STATE;

  // Fill the event structure
  sli_memprof_evt_take_snapshot_t event;
  size_t name_len = strlen(name);
  if (name_len > sizeof(event.name)) {
    name_len = sizeof(event.name);
  }
  memcpy(event.name, name, name_len);
  size_t event_len = sizeof(event) - sizeof(event.name) + name_len;

  // Send atomically
  CORE_ENTER_ATOMIC();
  send_memory_profiler_event(&event.header, SLI_MEMPROF_EVENT_TAKE_SNAPSHOT_ID, event_len);
  CORE_EXIT_ATOMIC();
}

/* Send a generic log */
void sli_memory_profiler_log(uint32_t log_id, uint32_t arg1, uint32_t arg2, uint32_t arg3, void * pc)
{
  CORE_DECLARE_IRQ_STATE;

  // Fill the event structure
  sli_memprof_evt_log_t event;
  event.log_id = log_id;
  event.arg1 = arg1;
  event.arg2 = arg2;
  event.arg3 = arg3;
  event.pc = (uint32_t) (uintptr_t) pc;

  // Send atomically
  CORE_ENTER_ATOMIC();
  send_memory_profiler_event(&event.header, SLI_MEMPROF_EVENT_LOG_ID, sizeof(event));
  CORE_EXIT_ATOMIC();
}
