/***************************************************************************/ /**
 * @file
 * @brief CPC Journal implementation.
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
#include <stdint.h>

#include "sl_compiler.h"
#include "sl_cpc_journal.h"
#include "sl_sleeptimer.h"
#include "sl_status.h"
#include "sli_cpc.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

typedef __PACKED_STRUCT {
  const char *str;
  uint32_t value;
  uint32_t timestamp;
  uint8_t level_seq_id;
} sl_cpc_journal_object_t;

typedef struct {
  uint16_t head;
  uint16_t tail;
  uint32_t size;
  sl_cpc_journal_object_t buffer[SL_CPC_JOURNAL_MAX_ENTRY_COUNT];
} sl_cpc_journal_circular_buffer_t;

// Made volatile to ensure it does not get optimized out and is accessible by a debugger
volatile sl_cpc_journal_circular_buffer_t sl_cpc_journal_cb[1 + (SL_CPC_JOURNAL_LEVEL >= SL_CPC_JOURNAL_DEBUG_LEVEL)];
static uint32_t last_timestamp;
static uint8_t persistant_sequence_id;

/// PRIVATE FUNCTION PROTOTYPES
#if defined(SL_CATALOG_IOSTREAM_PRESENT)
static sl_status_t sl_cpc_journal_pop(uint8_t depth_index, sl_cpc_journal_object_t *object);
static const char* level_to_string(sl_cpc_journal_level_t level);
#endif
static volatile sl_cpc_journal_circular_buffer_t* get_circular_buffer(sl_cpc_journal_level_t level);

/// PUBLIC FUNCTIONS
void sl_cpc_journal_init(void)
{
  for (uint8_t i = 0; i < ARRAY_SIZE(sl_cpc_journal_cb); i++) {
    sl_cpc_journal_cb[i].head = 0;
    sl_cpc_journal_cb[i].tail = 0;
    sl_cpc_journal_cb[i].size = SL_CPC_JOURNAL_MAX_ENTRY_COUNT;
  }

  persistant_sequence_id = 0;
  last_timestamp = 0;
  SL_CPC_JOURNAL_RECORD_INFO("CPC journal Init", 0);
}

void sl_cpc_journal_record_internal(sl_cpc_journal_level_t level, const char *string, uint32_t value)
{
  MCU_DECLARE_IRQ_STATE;

  // Fetch the proper circular buffer according to the entry level.
  volatile sl_cpc_journal_circular_buffer_t *journal = get_circular_buffer(level);

  MCU_ENTER_ATOMIC();

  uint32_t event_timestamp = sl_sleeptimer_tick_to_ms(sl_sleeptimer_get_tick_count());

  if (last_timestamp != event_timestamp) {
    persistant_sequence_id = 0;
  }
  last_timestamp = event_timestamp;

  uint16_t next_head = (journal->head + 1) % SL_CPC_JOURNAL_MAX_ENTRY_COUNT;

  if (next_head == journal->tail) {
    // Buffer is full
    journal->tail = (journal->tail + 1) % SL_CPC_JOURNAL_MAX_ENTRY_COUNT;
  }

  journal->buffer[journal->head].str = string;
  journal->buffer[journal->head].value = value;
  journal->buffer[journal->head].level_seq_id = level | persistant_sequence_id;
  journal->buffer[journal->head].timestamp = event_timestamp;

  journal->head = next_head;

  persistant_sequence_id = (persistant_sequence_id + 1) % 64;

  MCU_EXIT_ATOMIC();
}

#if defined(SL_CATALOG_IOSTREAM_PRESENT)
void sl_cpc_journal_print(bool print_csv_header, sl_iostream_t *stream)
{
  sl_status_t status = SL_STATUS_OK;
  sl_cpc_journal_object_t object;

  if (print_csv_header) {
    sl_iostream_printf(stream, "Timestamp,SequenceID,Level,Message,Value(Hex)\n");
  }

  for (uint8_t i = 0; i < ARRAY_SIZE(sl_cpc_journal_cb); i++) {
    status = sl_cpc_journal_pop(i, &object);
    while (status != SL_STATUS_EMPTY) {
      sl_iostream_printf(stream, "%lu,%d,%s,%s,0x%lx\n",
                         (unsigned long)object.timestamp,
                         (unsigned short)(object.level_seq_id & 0x3F),
                         level_to_string((object.level_seq_id & 0xC0)),
                         object.str,
                         (unsigned long)object.value);

      status = sl_cpc_journal_pop(i, &object);
    }
  }
}
#endif

#if defined(SL_CATALOG_CPC_JOURNAL_CLI_PRESENT)
void sl_cpc_journal_print_cmd_handler(sl_cli_command_arg_t *arguments)
{
  sl_cpc_journal_print(true, arguments->handle->iostream_handle);
}
#endif

/// PRIVATE FUNCTIONS
#if defined(SL_CATALOG_IOSTREAM_PRESENT)
/**************************************************************************//**
 * @brief Pop an entry from the circular journal buffer.
 *
 * This function removes the oldest entry from the circular journal buffer
 * and updates the tail index. If the journal buffer is empty, it returns
 * an empty status. The function operates atomically to ensure thread safety.
 *
 * @param[in] depth_index Circular buffer depth.
 * @param[out] object Return a copy of a buffer entry.
 * @return Global status code.
 *****************************************************************************/
static sl_status_t sl_cpc_journal_pop(uint8_t depth_index, sl_cpc_journal_object_t *object)
{
  MCU_DECLARE_IRQ_STATE;
  if (object == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  MCU_ENTER_ATOMIC();
  // Since sl_cpc_journal_cb is volatile, access order needs to be defined
  uint16_t head_temp = sl_cpc_journal_cb[depth_index].head;
  uint16_t tail_temp = sl_cpc_journal_cb[depth_index].tail;

  if (head_temp == tail_temp) {
    MCU_EXIT_ATOMIC();
    return SL_STATUS_EMPTY;
  }

  *object = sl_cpc_journal_cb[depth_index].buffer[tail_temp];
  tail_temp = (tail_temp + 1) % SL_CPC_JOURNAL_MAX_ENTRY_COUNT;
  sl_cpc_journal_cb[depth_index].tail = tail_temp;
  MCU_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/**************************************************************************//**
 * @brief Convert journal levels to their corresponding string representations.
 *
 * @param[in] level The journal level to convert.
 * @return Returns the string representation of the journal level.
 *
 * This function takes a journal level and returns the corresponding string
 * representation for that level.
 *****************************************************************************/
static const char* level_to_string(sl_cpc_journal_level_t level)
{
  switch (level) {
    case SL_CPC_JOURNAL_ERROR_LEVEL:   return "ERROR";
    case SL_CPC_JOURNAL_WARNING_LEVEL: return "WARNING";
    case SL_CPC_JOURNAL_INFO_LEVEL:    return "INFO";
    case SL_CPC_JOURNAL_DEBUG_LEVEL:   return "DEBUG";
    default:                           return "UNKNOWN";
  }
}
#endif

/**************************************************************************//**
 * @brief Retrieve the appropriate circular buffer based on the journal entry level.
 *
 * @param[in] level The journal entry level.
 * @return Pointer to the corresponding circular buffer.
 *
 * This function returns a pointer to the circular buffer that corresponds to
 * the specified journal entry level.
 *****************************************************************************/
static volatile sl_cpc_journal_circular_buffer_t* get_circular_buffer(sl_cpc_journal_level_t level)
{
  return &sl_cpc_journal_cb[SL_MIN(level >= SL_CPC_JOURNAL_DEBUG_LEVEL, ARRAY_SIZE(sl_cpc_journal_cb) > 1)];
}
