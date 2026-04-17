/***************************************************************************//**
 * @file
 * @brief Configuration instance for "external bonding database"
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

#include "sl_component_catalog.h"

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_EXTERNAL_BONDING_DATABASE_PRESENT)
#include "sl_bluetooth_config.h"
#include "sl_bluetooth_external_bondingdb_config.h"
#include "sli_bt_external_bondingdb_config.h"
#include MBEDTLS_CONFIG_FILE  // For MBEDTLS_PSA_CRYPTO_STORAGE_C

#if (SL_BT_CONFIG_EXTERNAL_LOCAL_IRK == 0) && !defined MBEDTLS_PSA_CRYPTO_STORAGE_C
#error "Component psa_its is required when external local IRK is not used"
#endif

const sli_bt_external_bondingdb_config_t sli_bt_external_bondingdb_config = {
  .external_local_irk = (SL_BT_CONFIG_EXTERNAL_LOCAL_IRK != 0)
};

#endif // SL_CATALOG_BLUETOOTH_FEATURE_EXTERNAL_BONDING_DATABASE_PRESENT
