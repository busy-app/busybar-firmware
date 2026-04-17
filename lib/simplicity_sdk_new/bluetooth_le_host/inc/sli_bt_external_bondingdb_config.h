/***************************************************************************//**
 * @file
 * @brief Configuration types for "external bonding database"
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_BT_EXTERNAL_BONDINGDB_CONFIG_H
#define SLI_BT_EXTERNAL_BONDINGDB_CONFIG_H

#include "sli_bt_config_defs.h"
#include <stdbool.h>

/**
 * @brief Configuration structure used by "external bonding database"
 */
typedef struct {
  bool external_local_irk; ///< Set to value of SL_BT_CONFIG_EXTERNAL_LOCAL_IRK
} sli_bt_external_bondingdb_config_t;

#endif // SLI_BT_EXTERNAL_BONDINGDB_CONFIG_H
