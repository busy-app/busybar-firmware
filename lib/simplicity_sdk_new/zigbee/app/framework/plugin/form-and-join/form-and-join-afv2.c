/***************************************************************************//**
 * @file
 * @brief This provides a compatibility layer between the legacy form and join code
 * and the App Framework v2 code.  The legacy form and join code is agnostic
 * of the Application Framework and thus we cannot put lots of App Framework
 * specific code in there.
 * More specifically, the sensor/sink app doesn't use the framework and thus
 * cannot compile form-and-join code with Afv2 references in it.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "app/util/common/form-and-join.h"

// ****************************************************************************
// Globals
sl_zigbee_af_event_t sl_zigbee_af_form_and_join_cleanup_event;
#define cleanupEvent (&sl_zigbee_af_form_and_join_cleanup_event)
void sl_zigbee_af_form_and_join_cleanup_event_handler(sl_zigbee_af_event_t * event);

// ****************************************************************************
// Forward Declarations

// ****************************************************************************

void sl_zigbee_joinable_network_found_handler(sl_zigbee_zigbee_network_t *networkFound,
                                              uint8_t lqi,
                                              int8_t rssi)
{
  sl_zigbee_af_form_and_join_network_found_cb(networkFound, lqi, rssi);
}

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#if (!defined(SL_CATALOG_ZIGBEE_NETWORK_FIND_PRESENT)              \
  && !defined(SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_NETWORK_PRESENT) \
  && !defined(EZSP_HOST))
#define PERFORM_JOIN_CLEANUP
#endif

void sl_zigbee_af_form_and_join_cleanup_event_handler(sl_zigbee_af_event_t * event)
{
  (void)event;
  // This takes a bit of explaining.
  // Prior to this release the form-and-join library was not a plugin and was
  // hardcoded in the project templates, thus there was no way to turn it off.
  // We needed to be able to enable/disable it and so we made it into a plugin.

  // The Network Find was another plugin layered on top of the form-and-join library.
  // Any application that used form-and-join without the network-find plugin
  // would need to properly cleanup the form-and-join code.  On the SOC this
  // was done automatically by a timer that fired after 30 seconds, assuming
  // you called sl_zigbee_form_and_join_tick() regularly.  On the host, there was nno
  // timer and thus an explicit call to sl_zigbee_form_and_join_cleanup() was required.

  // Now if the network-find plugin was enabled it had its own cleanup
  // (that also called sl_zigbee_form_and_join_cleanup()), therefore it was
  // unnecessary to schedule an event.

  // To maintain backwards compatibility we will run the cleanup code
  // only on SOC when it is not disabled by another piece of code
  // (e.g. network-find plugin)
  UNUSED_VAR(event);
#ifdef PERFORM_JOIN_CLEANUP
  sl_zigbee_form_and_join_cleanup(SL_STATUS_OK);
#else
  // This is a workaround put in place to handle bug EMAPPFWKV2-1379.  There are
  // certain circumstances in which the CleanupEvent is set active while the
  // above #if conditions are not met.  This results in the eventHandler taking
  // no action, leaving the event in an active state.  The event then has no
  // means by which it can be made inactive, so the scheduler indefinitely
  // calls the empty event, which prevents the device from sleeping.
  sl_zigbee_af_event_set_inactive(cleanupEvent);
#endif
}

void sli_zigbee_af_form_and_join_init_callback(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_event_init(cleanupEvent,
                          sl_zigbee_af_form_and_join_cleanup_event_handler);
}
