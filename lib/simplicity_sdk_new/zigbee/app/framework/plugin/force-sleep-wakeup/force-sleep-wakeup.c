/***************************************************************************//**
 * @brief Zigbee Force sleep component source code
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "app/framework/include/af.h"
#include "force-sleep-wakeup.h"
#include "sl_rail.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_RAIL_MULTIPLEXER_PRESENT
#include "sl_rail_mux_rename.h"
#endif
#include "sl_zigbee_debug_print.h"

extern sli_zigbee_event_queue_t sli_zigbee_stack_event_queue;
//externed from lower-mac-rail-802.15.4.c
extern void sl_mac_lower_mac_force_sleep(bool sleep);

//externed from mac-dispatch.h
extern void sl_802154_cancel_polls(void);
extern void sl_802154_purge_transmit_queue(void);
extern void sl_802154_purge_incoming_queue(void);

bool force_sleep = false;
static sl_zigbee_af_event_t force_sleep_wakeup_event;

static void force_sleep_wakeup_event_handler(sl_zigbee_af_event_t *event)
{
  sl_mac_lower_mac_force_sleep(false);
  sl_zigbee_core_debug_print("Wakeup\n");
}

void sli_zigbee_af_force_sleep_wakeup_init_callback(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_isr_event_init(&force_sleep_wakeup_event, force_sleep_wakeup_event_handler);
}

#ifdef SL_CATALOG_KERNEL_PRESENT
extern osMutexId_t sli_zigbee_af_event_mutex_id;
#endif // SL_CATALOG_KERNEL_PRESENT
void sl_zigbee_app_framework_force_stop(void)
{
  sl_zigbee_core_debug_print("Sleep\n");

  sl_802154_cancel_polls();
  sl_802154_purge_transmit_queue();
  sl_802154_purge_incoming_queue();

  sli_zigbee_cancel_all_events(&sli_zigbee_stack_event_queue);
  sli_zigbee_cancel_all_events(&sli_zigbee_af_app_event_queue);

  sl_mac_lower_mac_force_sleep(true);
  force_sleep = true;
}

void sl_zigbee_app_framework_force_wakeup(void)
{
  // This function run in an interrupt context, we have 2 stack and app framework tasks running.
  // The bellow event only run in app framwork task but force_sleep is being accessed by both stack
  // and app framework task so we need to clear force_sleep here.
  force_sleep = false;
  sl_zigbee_af_event_set_active(&force_sleep_wakeup_event);
  #ifdef SL_CATALOG_CMSIS_OS_COMMON_PRESENT
  sl_zigbee_wakeup_common_task();
  #endif //SL_CATALOG_CMSIS_OS_COMMON_PRESENT
}
// When force_sleep is set, this function will bypass power manager logic and immediately
// setup zigbee so it is ready to sleep
bool sli_zigbee_app_framework_get_force_sleep_flag(void)
{
  return force_sleep;
}

#ifdef SL_CATALOG_CLI_PRESENT
void sli_zigbee_app_framework_force_sleep_cli_handler(sl_cli_command_arg_t *arguments)
{
  // Do other things like turn off LEDs etc in the callback implemented in the app
  sli_zigbee_app_framework_force_sleep_callback();
  sl_zigbee_app_framework_force_stop();
}
#endif //SL_CATALOG_CLI_PRESENT

WEAK(void sli_zigbee_app_framework_force_sleep_callback(void)) {
}
