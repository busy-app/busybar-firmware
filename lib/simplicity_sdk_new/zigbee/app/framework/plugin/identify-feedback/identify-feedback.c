/***************************************************************************//**
 * @file
 * @brief Routines for the Identify Feedback plugin, which implements the
 *        feedback component of the Identify cluster.
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

// this file contains all the common includes for clusters in the util
#include "app/framework/include/af.h"

#include "app/framework/util/common.h"
#if !defined(EZSP_HOST)
#include "hal/hal.h"
#endif // !defined(EZSP_HOST)

#include "sl_component_catalog.h"
#ifdef SL_CATALOG_SIMPLE_LED_PRESENT
#include "sl_simple_led_instances.h"
#endif // SL_CATALOG_SIMPLE_LED_PRESENT

#include "identify-feedback-config.h"
sl_zigbee_af_event_t sl_zigbee_af_identify_feedback_provide_feedback_event;
#define provideFeedbackEventControl (&sl_zigbee_af_identify_feedback_provide_feedback_event)
void sl_zigbee_af_identify_feedback_provide_feedback_event_handler(sl_zigbee_af_event_t * event);
#if (SL_ZIGBEE_AF_PLUGIN_IDENTIFY_FEEDBACK_LED_FEEDBACK == 1)
#define LED_FEEDBACK
#endif

static bool identifyTable[SL_ZIGBEE_ZCL_IDENTIFY_CLUSTER_SERVER_ENDPOINT_COUNT] = { false };

// Verify if there is at least one ongoing endpoint verification
static bool haveIdentifyingEndpoint(void)
{
  for (int i = 0; i < SL_ZIGBEE_ZCL_IDENTIFY_CLUSTER_SERVER_ENDPOINT_COUNT; i++) {
    if (identifyTable[i]) {
      return true;
    }
  }
  return false;
}

void sli_zigbee_af_identify_init_callback(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_event_init(provideFeedbackEventControl,
                          sl_zigbee_af_identify_feedback_provide_feedback_event_handler);
}

void sl_zigbee_af_identify_feedback_provide_feedback_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
#if !defined(EZSP_HOST)
#ifdef LED_FEEDBACK
#ifdef SL_CATALOG_SIMPLE_LED_LED0_PRESENT
  sl_led_toggle(&sl_led_led0);
#endif // SL_CATALOG_SIMPLE_LED_LED0_PRESENT
#ifdef SL_CATALOG_SIMPLE_LED_LED1_PRESENT
  sl_led_toggle(&sl_led_led1);
#endif // SL_CATALOG_SIMPLE_LED_LED1_PRESENT
#ifdef SL_CATALOG_SIMPLE_LED_LED2_PRESENT
  sl_led_toggle(&sl_led_led2);
#endif // SL_CATALOG_SIMPLE_LED_LED2_PRESENT
#ifdef SL_CATALOG_SIMPLE_LED_LED3_PRESENT
  sl_led_toggle(&sl_led_led3);
#endif // SL_CATALOG_SIMPLE_LED_LED3_PRESENT
#endif // LED_FEEDBACK
#endif
  sl_zigbee_af_event_set_delay_ms(provideFeedbackEventControl,
                                  MILLISECOND_TICKS_PER_SECOND);
}

void sl_zigbee_af_identify_start_feedback_cb(uint8_t endpoint,
                                             uint16_t identifyTime)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_IDENTIFY_CLUSTER_ID);

  if (ep == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX) {
    sl_zigbee_af_identify_cluster_println("ERR: invalid endpoint supplied for identification.");
    return;
  }

  sl_zigbee_af_identify_cluster_println("Starting identifying on endpoint 0x%02X, identify time is %d sec",
                                        endpoint,
                                        identifyTime);

  if (!haveIdentifyingEndpoint()) {
    sl_zigbee_af_event_set_delay_ms(provideFeedbackEventControl,
                                    MILLISECOND_TICKS_PER_SECOND);
  }
  identifyTable[ep] = true;
}

void sl_zigbee_af_identify_stop_feedback_cb(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_IDENTIFY_CLUSTER_ID);

  if (ep == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX) {
    sl_zigbee_af_identify_cluster_println("ERR: invalid endpoint supplied for identification.");
    return;
  }

  sl_zigbee_af_identify_cluster_println("Stopping identifying on endpoint 0x%02X", endpoint);

  identifyTable[ep] = false;

  // Do not set provideFeedback event to inactive if there is at least one ongoing endpoint identification
  if (haveIdentifyingEndpoint()) {
    return;
  }

  sl_zigbee_af_identify_cluster_println("No endpoints identifying; stopping identification feedback.");
  sl_zigbee_af_event_set_inactive(provideFeedbackEventControl);
}
