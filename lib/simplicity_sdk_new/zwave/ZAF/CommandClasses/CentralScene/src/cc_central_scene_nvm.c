/**
 * @file
 * Command Class Central Scene I/O handling with NVM
 * @copyright 2022 Silicon Laboratories Inc.
 */

#include <string.h>
#include "cc_central_scene_io.h"
#include <ZAF_nvm_app.h>
#include <ZAF_nvm.h>
#include <ZAF_file_ids.h>
#include <zpal_misc.h>

// Matches FILE_ID_APPLICATIONDATA from WallController sample app
#define CENTRAL_SCENE_DATA_LEGACY_FILE_ID      (0x00000)

bool
cc_central_scene_read(central_scene_configuration_t* pConfig)
{
  zpal_status_t status = ZAF_nvm_read(ZAF_FILE_ID_CENTRAL_SCENE_CONFIG, pConfig, sizeof(central_scene_configuration_t));
  return ZPAL_STATUS_OK == status;
}

bool
cc_central_scene_write(const central_scene_configuration_t* pConfig)
{
  zpal_status_t status = ZAF_nvm_write(ZAF_FILE_ID_CENTRAL_SCENE_CONFIG, pConfig, sizeof(central_scene_configuration_t));
  return ZPAL_STATUS_OK == status;
}

void
cc_central_scene_migrate(void)
{
  #ifdef ZW_MIGRATION_TO_7_19

  zpal_status_t status;
  central_scene_configuration_t application_data = { 0 };
  // On initial implementation of CentralScene Command Class
  // This information belonged to the application
  // From SDK 7.19.0 this information was moved to the command class
  // In order to migrate, we need to read the `CENTRAL_SCENE_DATA_LEGACY_FILE_ID` which points to
  // `FILE_ID_APPLICATIONDATA`. Compare `slowRefresh` attribute,
  //  If the constant matches, migrate the file to the new location
  status = ZAF_nvm_app_read(CENTRAL_SCENE_DATA_LEGACY_FILE_ID, &application_data, sizeof(central_scene_configuration_t));
  if (status != ZPAL_STATUS_OK) {
    return;
  }

  // Force slow refresh
  application_data.slowRefresh = 1; // CC:005B.03.00.21.001 - Slow refresh must be enabled after inclusion

  ZAF_nvm_write(ZAF_FILE_ID_CENTRAL_SCENE_CONFIG, &application_data, sizeof(central_scene_configuration_t));

  ZAF_nvm_app_erase_object(CENTRAL_SCENE_DATA_LEGACY_FILE_ID);

  #endif
}
