/***************************************************************************//**
 * @file
 * @brief Provides the host utility functions for counters.
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

#include "sl_code_classification.h"
#include "app/framework/include/af.h"
#include "app/util/common/common.h"
#include "counters.h"
#include "counters-ota.h"

uint16_t sl_zigbee_counters[SL_ZIGBEE_COUNTER_TYPE_COUNT];
uint16_t sl_zigbee_counters_thresholds[SL_ZIGBEE_COUNTER_TYPE_COUNT];

void sl_zigbee_af_counters_init_cb(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_counters_clear();
  sl_zigbee_af_counters_reset_thresholds();
}

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_ZIGBEE_STACK, SL_CODE_CLASS_TIME_CRITICAL)
void sli_zigbee_af_counter_rollover_callback(sl_zigbee_counter_type_t type)
{
  (void) type;
}

void sl_zigbee_af_counters_clear(void)
{
  sl_status_t status;
  #ifdef EZSP_HOST
  status = sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_CLEAR_COUNTERS, 0, NULL);
  #else
  status = sl_zigbee_clear_counters();
  #endif
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_core_println("ezsp counters failed %u", status);
  }
}

void sl_zigbee_af_counters_reset_thresholds(void)
{
  //Clear local copy of thresholds.
  sl_status_t status;
  #ifdef EZSP_HOST
  status = sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_RESET_COUNTER_THRESHOLDS, 0, NULL);
  #else
  status = sl_zigbee_reset_counters_thresholds();
  #endif
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_core_println("ezsp Reset Threshold error %u", status);
  } else {
    memset(sl_zigbee_counters_thresholds, 0xFF, sizeof(sl_zigbee_counters_thresholds));
  }
}

void sl_zigbee_af_counters_set_threshold(sl_zigbee_counter_type_t type, uint16_t threshold)
{
  sl_status_t status;
  #ifdef EZSP_HOST
  uint8_t ezspThreshold[3];
  ezspThreshold[0] = type;
  ezspThreshold[1] = LOW_BYTE(threshold);
  ezspThreshold[2] = HIGH_BYTE(threshold);
  status = sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_SET_COUNTER_THRESHOLD, sizeof(ezspThreshold), ezspThreshold);
  #else
  status = sl_zigbee_set_counter_threshold(type, threshold);
  #endif

  if (status != SL_STATUS_OK) {
    sl_zigbee_af_core_println("ezsp Set Threshold error %u", status);
  } else {
    sl_zigbee_counters_thresholds[type] = threshold;
  }
}
