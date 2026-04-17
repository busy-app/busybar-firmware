/**
 * @file
 *
 * ZAF PrintAppInfo module source file
 * @copyright 2023 Silicon Laboratories Inc.
 */

#include "ZAF_PrintAppInfo.h"
#include "ZAF_AppName.h"
#include "ZAF_version.h"
#include "zpal_log.h"

void ZAF_PrintAppInfo(void)
{
  #if defined(ZPAL_LOG_APP)
  char app_name[ZAF_FILE_SIZE_APP_NAME];
  ZAF_AppName_Read(app_name);
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP,
                 "\n\n----------------------------------\n");
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP,
                 "Z-Wave Sample App: %s \n", app_name);
  ZPAL_LOG_DEBUG(ZPAL_LOG_APP,
                 "SDK: %d.%d.%d ZAF: %d.%d.%d.%d\n",
                 SDK_GetVersionMajor(), SDK_GetVersionMinor(), SDK_GetVersionPatch(),
                 ZAF_GetVersionMajor(), ZAF_GetVersionMinor(), ZAF_GetVersionPatch(),
                 ZAF_GetBuildNumber());
  #endif // defined(ZPAL_LOG_APP)
}
