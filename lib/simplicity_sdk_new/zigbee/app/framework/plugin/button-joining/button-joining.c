/***************************************************************************//**
 * @file
 * @brief Routines for forming/joining using the hardware buttons.
 *   button 0: if not joined: FORM if the device is capable of forming
 *             (a.k.a. a coordinator).  Otherwise form a network.
 *             if joined: broadcast ZDO MGMT Permit Join in network.
 *             Hold for 5 seconds and release: leave network
 *   button 1: Unused (Callback executed)
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
#include "button-joining.h"

//------------------------------------------------------------------------------
// Forward Declaration

#include "button-joining-config.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_NETWORK_CREATOR_SECURITY_PRESENT
#include "network-creator-security.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_NETWORK_CREATOR_PRESENT
#include "network-creator.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_NETWORK_STEERING_PRESENT
#include "network-steering.h"
#endif
sl_zigbee_af_event_t sl_zigbee_af_button_joining_button0_event;
#define buttonEvent0 (&sl_zigbee_af_button_joining_button0_event)
void sl_zigbee_af_button_joining_button0_event_handler(sl_zigbee_af_event_t * event);
sl_zigbee_af_event_t sl_zigbee_af_button_joining_button1_event;
#define buttonEvent1 (&sl_zigbee_af_button_joining_button1_event)
void sl_zigbee_af_button_joining_button1_event_handler(sl_zigbee_af_event_t * event);
#define BUTTON0         0
#define BUTTON1         1

static bool buttonPress(uint8_t button, uint8_t state);
static uint32_t  buttonPressDurationMs = 0;

#define BUTTON_HOLD_DURATION_MS 5000

//------------------------------------------------------------------------------
// Globals

#define PERMIT_JOIN_TIMEOUT SL_ZIGBEE_AF_PLUGIN_BUTTON_JOINING_PERMIT_JOIN_TIMEOUT

//------------------------------------------------------------------------------

// Event init only.
void sli_zigbee_af_button_joining_init_callback(uint8_t init_level)
{
  UNUSED_VAR(init_level);

  sl_zigbee_af_isr_event_init(&sl_zigbee_af_button_joining_button0_event,
                              sl_zigbee_af_button_joining_button0_event_handler);
  sl_zigbee_af_isr_event_init(&sl_zigbee_af_button_joining_button1_event,
                              sl_zigbee_af_button_joining_button1_event_handler);
}

void sl_zigbee_af_button_joining_button0_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sl_zigbee_af_event_set_inactive(buttonEvent0);

  if (buttonPressDurationMs >= BUTTON_HOLD_DURATION_MS) {
    sl_zigbee_af_core_println("Leaving network due to button press.");
    sl_zigbee_leave_network(SL_ZIGBEE_LEAVE_NWK_WITH_NO_OPTION);
    return;
  }

  if (sl_zigbee_af_network_state() == SL_ZIGBEE_JOINED_NETWORK) {
#ifdef SL_CATALOG_ZIGBEE_NETWORK_CREATOR_SECURITY_PRESENT
    // The Network Creator Security plugin Open process is more comprehensive
    // and also takes care of broadcasting pjoin
    sl_zigbee_af_network_creator_security_open_network();
#else
    sl_zigbee_af_broadcast_permit_join(PERMIT_JOIN_TIMEOUT);
#endif // SL_CATALOG_ZIGBEE_NETWORK_CREATOR_SECURITY_PRESENT
  } else if (sl_zigbee_af_network_state() == SL_ZIGBEE_NO_NETWORK) {
#ifdef SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK
    sl_zigbee_af_core_println("%s: nwk down: do form", "button0");
    // Use Z3.0 network formation if we have the plugin for it; else, fall back
    // to legacy / custom formation methods
#ifdef SL_CATALOG_ZIGBEE_NETWORK_CREATOR_PRESENT
    sl_zigbee_af_network_creator_start(true);
#else // !SL_CATALOG_ZIGBEE_NETWORK_CREATOR_PRESENT
    sl_zigbee_af_find_unused_pan_id_and_form();
#endif // SL_CATALOG_ZIGBEE_NETWORK_CREATOR_PRESENT
#else // !SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK
    // Use Z3.0 network steering if we have the plugin for it; else, fall back
    // to legacy / custom joining method
    sl_zigbee_af_core_println("%s: nwk down: do join", "button0");
#ifdef SL_CATALOG_ZIGBEE_NETWORK_STEERING_PRESENT
    sl_zigbee_af_network_steering_start();
#else // !SL_CATALOG_ZIGBEE_NETWORK_STEERING_PRESENT
    sl_zigbee_af_start_search_for_joinable_network();
#endif // SL_CATALOG_ZIGBEE_NETWORK_STEERING_PRESENT
#endif // SL_ZIGBEE_AF_HAS_COORDINATOR_NETWORK
  } else {
    // MISRA requires ..else if.. to have terminating else.
  }
}

void sl_zigbee_af_button_joining_button1_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sl_zigbee_af_event_set_inactive(buttonEvent1);
  sl_zigbee_af_button_joining_button_event_cb(1, // button 1 is pressed
                                              buttonPressDurationMs);
}

void sl_zigbee_af_button_joining_press_button(uint8_t button)
{
  // We don't bother to check the button press both times
  // since the only reason it fails is invalid button.
  // No point in doing that twice.
  bool result = buttonPress(button, BUTTON_PRESSED);
  buttonPress(button, BUTTON_RELEASED);
  if (!result) {
    sl_zigbee_af_core_println("Invalid button %d", button);
  }
}

// ISR context functions!!!

// WARNING: these functions are in ISR so we must do minimal
// processing and not make any blocking calls (like printf)
// or calls that take a long time.

static bool buttonPress(uint8_t button, uint8_t state)
{
  // ISR CONTEXT!!!
  static uint32_t timeMs;
  sl_zigbee_af_event_t *eventControl;
  if (button == BUTTON0) {
    eventControl = buttonEvent0;
  } else if (button == BUTTON1) {
    eventControl = buttonEvent1;
  } else {
    return false;
  }

  if (state == BUTTON_PRESSED) {
    buttonPressDurationMs = 0;
    timeMs = halCommonGetInt32uMillisecondTick();
  } else {
    buttonPressDurationMs = elapsedTimeInt32u(timeMs, halCommonGetInt32uMillisecondTick());
    sl_zigbee_af_event_set_active(eventControl);
  }

  return true;
}

void sl_zigbee_af_hal_button_isr_cb(uint8_t button, uint8_t state)
{
  // ISR CONTEXT!!!
  buttonPress(button, state);
}
