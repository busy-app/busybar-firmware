/*******************************************************************************
 * @file sl_si91x_power_manager_debug.c
 * @brief Power Manager Debug API Implementation
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

#include "sl_si91x_power_manager.h"
#include "sl_si91x_power_manager_debug_config.h"
#include "sl_si91x_power_manager_debug.h"
#include "sli_si91x_power_manager.h"
#include "rsi_debug.h"
#include "sl_slist.h"

#if defined(SL_SI91X_POWER_MANAGER_DEBUG) && (SL_SI91X_POWER_MANAGER_DEBUG == ENABLE)

/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/
#define POWER_STATE_TABLE_SIZE 5 // Power State Requirement Table size

/*******************************************************************************
 ***************************  Local Types  ********************************
 ******************************************************************************/
// Debug entry
typedef struct {
  sl_slist_node_t node;
  const char *module_name;
} sli_si91x_power_debug_requirement_entry_t;

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static sl_slist_node_t *power_manager_debug_requirement_ps_table[POWER_STATE_TABLE_SIZE];
static sli_si91x_power_debug_requirement_entry_t power_debug_entry_table[SL_SI91X_POWER_MANAGER_DEBUG_POOL_SIZE];
static sl_slist_node_t *power_debug_free_entry_list = NULL;
static bool power_debug_ran_out_of_entry            = false;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
static void power_manager_log_add_requirement(sl_slist_node_t **p_list, bool add, const char *name);

/*******************************************************************************
***********************  Global function Definitions *************************
 ******************************************************************************/
/***************************************************************************/ /**
 * Initialize debugging feature.
 ******************************************************************************/
void sli_si91x_power_manager_init_debug(void)
{
  uint32_t i;

  for (i = 0; i < SL_SI91X_POWER_MANAGER_DEBUG_POOL_SIZE; i++) {
    sli_si91x_power_debug_requirement_entry_t *entry = &power_debug_entry_table[i];
    sl_slist_push(&power_debug_free_entry_list, &entry->node);
  }
}

/***************************************************************************/ /**
 * Print a fancy table that describes the current requirements on each energy
 * mode and their owner.
 ******************************************************************************/
void sl_si91x_power_manager_debug_print_ps_requirements(void)
{
  uint8_t i;
  sli_si91x_power_debug_requirement_entry_t *entry;

  if (power_debug_ran_out_of_entry) {
    DEBUGOUT("WARNING: The system ran out of Debug Entry; This report is likely to be incomplete. Increase "
             "SL_POWER_MANAGER_DEBUG_POOL_SIZE\n\n");
  }
  DEBUGOUT("------------------------------------------\n");
  DEBUGOUT("| Power State requirements\n");
  DEBUGOUT("------------------------------------------\n");
  for (i = 0; i < POWER_STATE_TABLE_SIZE; i++) {
    if (power_manager_debug_requirement_ps_table[i] != NULL) {
      DEBUGOUT("| PS%d requirement module owners:\n", i + 1);
    }
    SL_SLIST_FOR_EACH_ENTRY(power_manager_debug_requirement_ps_table[i],
                            entry,
                            sli_si91x_power_debug_requirement_entry_t,
                            node)
    {
      DEBUGOUT("|     %s\n", entry->module_name);
    }
    if (power_manager_debug_requirement_ps_table[i] != NULL) {
      DEBUGOUT("------------------------------------------\n");
    }
  }
}

/***************************************************************************/ /**
 * Log requirement to a list
 *
 * @param p_list  List where to push or remove the requirement.
 *
 * @param add     Add (true) or remove (false) the requirement.
 *
 * @param name    Module name that acquired or remove the requirement.
 ******************************************************************************/
static void power_manager_log_add_requirement(sl_slist_node_t **p_list, bool add, const char *name)
{
  sl_slist_node_t *node;
  sli_si91x_power_debug_requirement_entry_t *entry;

  if (add == true) {
    // Get entry from free list
    node = sl_slist_pop(&power_debug_free_entry_list);
    if (node == NULL) {
      power_debug_ran_out_of_entry = true;
      return;
    }

    // Push entry to the EMx requirement debug list
    entry              = SL_SLIST_ENTRY(node, sli_si91x_power_debug_requirement_entry_t, node);
    entry->module_name = name;
    sl_slist_push(p_list, &entry->node);
  } else {
    sli_si91x_power_debug_requirement_entry_t *entry_remove = NULL;

    // Search in the EMx requirement debug list
    SL_SLIST_FOR_EACH_ENTRY(*p_list, entry, sli_si91x_power_debug_requirement_entry_t, node)
    {
      // Current module name and entry module name
      if (strcmp(entry->module_name, name) == 0) {
        entry_remove = entry;
        break;
      }
    }

    if (entry_remove == NULL) {
      return;
    }

    sl_slist_remove(p_list, &entry_remove->node);
    sl_slist_push(&power_debug_free_entry_list, &entry_remove->node);
  }
}

#undef sli_si91x_power_manager_debug_log_ps_requirement

/***************************************************************************/ /**
 * Log Power State (EM) requirement
 *
 * @param ps    Power State added or removed.
 *
 * @param add   Add (true) or remove (false) the requirement.
 *
 * @param name  Module name that adds or removes the requirement.
 ******************************************************************************/
void sli_si91x_power_manager_debug_log_ps_requirement(sl_power_state_t ps, bool add, const char *name)
{
#if defined(SL_SI91X_POWER_MANAGER_DEBUG) && (SL_SI91X_POWER_MANAGER_DEBUG == 1)
  power_manager_log_add_requirement(&power_manager_debug_requirement_ps_table[ps - 1], add, name);
#else
  (void)ps;
  (void)add;
  (void)name;
#endif
}

#endif // SL_SI91X_POWER_MANAGER_DEBUG