/*************************************************************************/ /**
* @file
* @brief CPC Journal
******************************************************************************
* # License
* <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
******************************************************************************
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

#ifndef SL_CPC_JOURNAL_H
#define SL_CPC_JOURNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************/ /**
 * @addtogroup cpc CPC Journal
 * @{
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "sl_enum.h"
#include "sl_cpc_journal_config.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#if defined(SL_CATALOG_CPC_JOURNAL_CLI_PRESENT)
#include "sl_cli.h"
#endif

#if defined(SL_CATALOG_IOSTREAM_PRESENT)
#include "sl_iostream.h"
#endif

/**
 * @brief Record a CPC journal entry.
 *
 * @param[in] string The message string to record.
 * @param[in] value The value associated with the message.
 */
#define SL_CPC_JOURNAL_RECORD_ERROR(string, value) \
  sl_cpc_journal_record(SL_CPC_JOURNAL_ERROR_LEVEL, string, value)
#define SL_CPC_JOURNAL_RECORD_WARNING(string, value) \
  sl_cpc_journal_record(SL_CPC_JOURNAL_WARNING_LEVEL, string, value)
#define SL_CPC_JOURNAL_RECORD_INFO(string, value) \
  sl_cpc_journal_record(SL_CPC_JOURNAL_INFO_LEVEL, string, value)
#define SL_CPC_JOURNAL_RECORD_DEBUG(string, value) \
  sl_cpc_journal_record(SL_CPC_JOURNAL_DEBUG_LEVEL, string, value)

/// @brief Enumeration of journal entry levels.
SL_ENUM_GENERIC(sl_cpc_journal_level_t, uint8_t){
  SL_CPC_JOURNAL_ERROR_LEVEL = 0x00,
  SL_CPC_JOURNAL_WARNING_LEVEL = 0x40,
  SL_CPC_JOURNAL_INFO_LEVEL = 0x80,
  SL_CPC_JOURNAL_DEBUG_LEVEL = 0xC0,
  SL_CPC_JOURNAL_TRACE_LEVEL = 0xC0
};

/**************************************************************************//**
 * Initialize the CPC Journal Module.
 *
 * This function sets up the journal's circular buffers and configures the
 * maximum number of entries. The journal supports four log levels: Error,
 * Warning, Info, and Debug.
 *
 * @note To prevent overwriting critical log entries, Debug logs are stored
 * in a separate circular buffer.
 *****************************************************************************/
void sl_cpc_journal_init(void);

/**************************************************************************//**
 * @brief Push an entry in the Journal.
 *
 * @param[in] level The level of the entry (Error, Warning, Info, Debug).
 * @param[in] string The message string to log.
 * @param[in] value The value associated with the message.
 *
 * This function logs a journal entry with a specified severity level, message
 * string, and associated value.
 *****************************************************************************/
void sl_cpc_journal_record_internal(sl_cpc_journal_level_t level, const char *string, uint32_t value);

/**************************************************************************//**
 * @brief Record a Journal Entry.
 *
 * @param[in] level The level of the entry (Error, Warning, Info, Debug).
 * @param[in] string The message string to log.
 * @param[in] value The value associated with the message.
 *
 * @note If the provided journal level is superior to the configured max level,
 * the journal entry will be ignored.
 *****************************************************************************/
static inline void sl_cpc_journal_record(sl_cpc_journal_level_t level, const char *string, uint32_t value)
{
  if (level <= SL_CPC_JOURNAL_LEVEL) {
    sl_cpc_journal_record_internal(level, string, value);
  }
}

#if defined(SL_CATALOG_IOSTREAM_PRESENT) || defined(DOXYGEN)
/**
 * @brief Print the contents of the CPC journal via the default IOStream.
 *
 * @param[in] print_header Whether to print a CSV header.
 *
 * @param[in] stream The IOStream used to print the journal entries.
 *
 * @note Journal entries will be consumed when called
 */
void sl_cpc_journal_print(bool print_csv_header, sl_iostream_t *stream);
#endif

#if defined(SL_CATALOG_CPC_JOURNAL_CLI_PRESENT) || defined(DOXYGEN)
/**************************************************************************//**
 * @brief Command Handler to Print the Journal Contents.
 *
 * @param[in] arguments CLI arguments.
 *
 * This function handles a command to print the contents of the CPC journal.
 *
 * @note Journal entries will be consumed when this function is called.
 *****************************************************************************/
void sl_cpc_journal_print_cmd_handler(sl_cli_command_arg_t *arguments);
#endif

#ifdef __cplusplus
}
#endif

#endif // SL_CPC_JOURNAL_H
