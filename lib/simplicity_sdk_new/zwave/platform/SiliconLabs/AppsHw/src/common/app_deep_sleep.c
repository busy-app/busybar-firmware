/**
 * @file
 * Platform abstraction for all sleeping reporting applications
 *
 * @copyright 2021 Silicon Laboratories Inc.
 */
#include "app_hw.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_SIMPLE_BUTTON_PRESENT
#include "app_button_handler.h"
#endif

void app_hw_deep_sleep_wakeup_handler(void)
{
#ifdef SL_CATALOG_SIMPLE_BUTTON_PRESENT
  app_button_press_stay_awake_until_release();
#endif
}
