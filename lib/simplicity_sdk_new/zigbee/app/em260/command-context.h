/***************************************************************************//**
 * @file
 * @brief Declarations of command context structs.
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

#ifndef SILABS_COMMAND_CONTEXT_H
#define SILABS_COMMAND_CONTEXT_H

#include "app/util/ezsp/ezsp-enum.h"

typedef struct {
  bool consumed;
  sl_status_t status;

  union {
    sl_zigbee_ezsp_value_id_t valueId;
    sl_zigbee_ezsp_extended_value_id_t extendedValueId;
  };

  uint32_t characteristics;

  uint8_t valueLength;
  uint8_t value[128];
} sl_zigbee_af_plugin_ezsp_value_command_context_t;

typedef struct {
  bool consumed;

  sl_zigbee_ezsp_policy_id_t policyId;
  sl_status_t status;
  sl_zigbee_ezsp_decision_id_t decisionId;
} sl_zigbee_af_plugin_ezsp_policy_command_context_t;

typedef struct {
  bool consumed;
  sl_zigbee_ezsp_config_id_t configId;
  sl_status_t status;
  uint16_t value;
} sl_zigbee_af_plugin_ezsp_configuration_value_command_context_t;

#endif /* __COMMAND_CONTEXT_H__ */
