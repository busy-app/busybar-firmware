/***************************************************************************//**
 * @file
 * @brief CPC Journal configuration file.
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
#include "sl_cpc_journal.h"

// <<< Use Configuration Wizard in Context Menu >>>

#ifndef SL_CPC_JOURNAL_CONFIG_H
#define SL_CPC_JOURNAL_CONFIG_H

// <h>CPC Journal Configuration

// <o SL_CPC_JOURNAL_MAX_ENTRY_COUNT>Maximum CPC journal entries
// <i> Default: 128
// <i> Maximum amount of entries for the CPC Journal
#define SL_CPC_JOURNAL_MAX_ENTRY_COUNT 128

// <o SL_CPC_JOURNAL_LEVEL> Journal entry level
// <SL_CPC_JOURNAL_ERROR_LEVEL=> Record only errors
// <SL_CPC_JOURNAL_WARNING_LEVEL=> Record warnings and anything above
// <SL_CPC_JOURNAL_INFO_LEVEL=> Record info and anything above
// <SL_CPC_JOURNAL_DEBUG_LEVEL=> Record debug and anything above
// <SL_CPC_JOURNAL_TRACE_LEVEL=> Record traces and anything above
// <i> Default: SL_CPC_JOURNAL_INFO_LEVEL
// <i> Minimum journal entry level for CPC Journal
#define SL_CPC_JOURNAL_LEVEL SL_CPC_JOURNAL_INFO_LEVEL

// </h>

// <<< end of configuration section >>>

#endif /* SL_CPC_JOURNAL_CONFIG_H */
