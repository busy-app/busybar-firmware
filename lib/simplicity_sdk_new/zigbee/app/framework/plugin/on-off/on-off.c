/***************************************************************************//**
 * @file
 * @brief Routines for the On-Off plugin, which implements the On-Off server
 *        cluster.
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
#include "on-off.h"

#include "zap-cluster-command-parser.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_REPORTING_PRESENT
#include "app/framework/plugin/reporting/reporting.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_SCENES_PRESENT
#include "scenes.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_ZLL_ON_OFF_SERVER_PRESENT
#include "zll-on-off-server.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_ZLL_LEVEL_CONTROL_SERVER_PRESENT
  #include "zll-level-control-server.h"
#endif

#ifdef ZCL_USING_ON_OFF_CLUSTER_START_UP_ON_OFF_ATTRIBUTE
static bool areStartUpOnOffServerAttributesTokenized(uint8_t endpoint);
#endif

sl_zigbee_af_status_t sl_zigbee_af_on_off_cluster_set_value_cb(uint8_t endpoint,
                                                               uint8_t command,
                                                               bool initiatedByLevelChange)
{
  sl_zigbee_af_status_t status;
  bool currentValue, newValue;

  sl_zigbee_af_on_off_cluster_println("On/Off set value: %02X %02X", endpoint, command);

  // read current on/off value
  status = sl_zigbee_af_read_attribute(endpoint,
                                       ZCL_ON_OFF_CLUSTER_ID,
                                       ZCL_ON_OFF_ATTRIBUTE_ID,
                                       CLUSTER_MASK_SERVER,
                                       (uint8_t *)&currentValue,
                                       sizeof(currentValue),
                                       NULL); // data type
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_on_off_cluster_println("ERR: reading on/off %02X", status);
    return status;
  }

  // if the value is already what we want to set it to then do nothing
  if ((!currentValue && command == ZCL_OFF_COMMAND_ID)
      || (currentValue && command == ZCL_ON_COMMAND_ID)) {
    sl_zigbee_af_on_off_cluster_println("On/off already set to new value");
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  // we either got a toggle, or an on when off, or an off when on,
  // so we need to swap the value
  newValue = !currentValue;
  sl_zigbee_af_on_off_cluster_println("Toggle on/off from %02X to %02X", currentValue, newValue);

  // the sequence of updating on/off attribute and kick off level change effect should
  // be depend on whether we are turning on or off. If we are turning on the light, we
  // should update the on/off attribute before kicking off level change, if we are
  // turning off the light, we should do the opposite,cmd_data that is kick off level change
  // before updating the on/off attribute.
  if (newValue) {
    // write the new on/off value
    status = sl_zigbee_af_write_attribute(endpoint,
                                          ZCL_ON_OFF_CLUSTER_ID,
                                          ZCL_ON_OFF_ATTRIBUTE_ID,
                                          CLUSTER_MASK_SERVER,
                                          (uint8_t *)&newValue,
                                          ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_af_on_off_cluster_println("ERR: writing on/off %02X", status);
      return status;
    }

    // If initiatedByLevelChange is false, then we assume that the level change
    // ZCL stuff has not happened and we do it here
    if (!initiatedByLevelChange
        && sl_zigbee_af_contains_server(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID)) {
      sl_zigbee_af_on_off_cluster_level_control_effect_cb(endpoint,
                                                          newValue);
    }
  } else {
    // If initiatedByLevelChange is false, then we assume that the level change
    // ZCL stuff has not happened and we do it here
    if (!initiatedByLevelChange
        && sl_zigbee_af_contains_server(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID)) {
      sl_zigbee_af_on_off_cluster_level_control_effect_cb(endpoint,
                                                          newValue);
    }

    // write the new on/off value
    status = sl_zigbee_af_write_attribute(endpoint,
                                          ZCL_ON_OFF_CLUSTER_ID,
                                          ZCL_ON_OFF_ATTRIBUTE_ID,
                                          CLUSTER_MASK_SERVER,
                                          (uint8_t *)&newValue,
                                          ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_af_on_off_cluster_println("ERR: writing on/off %02X", status);
      return status;
    }
  }

#ifdef SL_CATALOG_ZIGBEE_ZLL_ON_OFF_SERVER_PRESENT
  if (initiatedByLevelChange) {
    sl_zigbee_af_zll_on_off_server_level_control_zll_extensions(endpoint);
  }
#endif // SL_CATALOG_ZIGBEE_ZLL_ON_OFF_SERVER_PRESENT

  // the scene has been changed (the value of on/off has changed) so
  // the current scene as descibed in the attribute table is invalid,
  // so mark it as invalid (just writes the valid/invalid attribute)
  if (sl_zigbee_af_contains_server(endpoint, ZCL_SCENES_CLUSTER_ID)) {
    sl_zigbee_af_scenes_cluster_make_invalid_cb(endpoint);
  }

  // The returned status is based solely on the On/Off cluster.  Errors in the
  // Level Control and/or Scenes cluster are ignored.
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_on_off_cluster_off_cb(void)
{
  sl_zigbee_af_status_t status = sl_zigbee_af_on_off_cluster_set_value_cb(sl_zigbee_af_current_endpoint(),
                                                                          ZCL_OFF_COMMAND_ID,
                                                                          false);
#ifdef SL_CATALOG_ZIGBEE_ZLL_ON_OFF_SERVER_PRESENT
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_zll_on_off_server_off_zll_extensions(sl_zigbee_af_current_command());
  }
#endif
  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_on_off_cluster_on_cb(void)
{
  sl_zigbee_af_status_t status = sl_zigbee_af_on_off_cluster_set_value_cb(sl_zigbee_af_current_endpoint(),
                                                                          ZCL_ON_COMMAND_ID,
                                                                          false);
#ifdef SL_CATALOG_ZIGBEE_ZLL_ON_OFF_SERVER_PRESENT
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_zll_on_off_server_on_zll_extensions(sl_zigbee_af_current_command());
  }
#endif
  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_on_off_cluster_toggle_cb(void)
{
  sl_zigbee_af_status_t status = sl_zigbee_af_on_off_cluster_set_value_cb(sl_zigbee_af_current_endpoint(),
                                                                          ZCL_TOGGLE_COMMAND_ID,
                                                                          false);
#ifdef SL_CATALOG_ZIGBEE_ZLL_ON_OFF_SERVER_PRESENT
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_zll_on_off_server_toggle_zll_extensions(sl_zigbee_af_current_command());
  }
#endif
  return status;
}

void sl_zigbee_af_on_off_cluster_server_init_cb(uint8_t endpoint)
{
#ifdef ZCL_USING_ON_OFF_CLUSTER_START_UP_ON_OFF_ATTRIBUTE
  // StartUp behavior relies on OnOff and StartUpOnOff attributes being tokenized.
  if (areStartUpOnOffServerAttributesTokenized(endpoint)) {
    // Read the StartUpOnOff attribute and set the OnOff attribute as per
    // following from zcl 7 14-0127-20i-zcl-ch-3-general.doc.
    // 3.8.2.2.5	StartUpOnOff Attribute
    // The StartUpOnOff attribute SHALL define the desired startup behavior of a
    // lamp device when it is supplied with power and this state SHALL be
    // reflected in the OnOff attribute.  The values of the StartUpOnOff
    // attribute are listed below.
    // Table 3 46. Values of the StartUpOnOff Attribute
    // Value      Action on power up
    // 0x00       Set the OnOff attribute to 0 (off).
    // 0x01       Set the OnOff attribute to 1 (on).
    // 0x02       If the previous value of the OnOff attribute is equal to 0,
    //            set the OnOff attribute to 1.If the previous value of the OnOff
    //            attribute is equal to 1, set the OnOff attribute to 0 (toggle).
    // 0x03-0xfe  These values are reserved.  No action.
    // 0xff       Set the OnOff attribute to its previous value.

    // Initialize startUpOnOff to No action value 0xFE
    uint8_t startUpOnOff = 0xFE;
    sl_zigbee_af_status_t status = sl_zigbee_af_read_attribute(endpoint,
                                                               ZCL_ON_OFF_CLUSTER_ID,
                                                               ZCL_START_UP_ON_OFF_ATTRIBUTE_ID,
                                                               CLUSTER_MASK_SERVER,
                                                               (uint8_t *)&startUpOnOff,
                                                               sizeof(startUpOnOff),
                                                               NULL);
    if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      // Initialise updated value to 0
      bool updatedOnOff = 0;
      status = sl_zigbee_af_read_attribute(endpoint,
                                           ZCL_ON_OFF_CLUSTER_ID,
                                           ZCL_ON_OFF_ATTRIBUTE_ID,
                                           CLUSTER_MASK_SERVER,
                                           (uint8_t *)&updatedOnOff,
                                           sizeof(updatedOnOff),
                                           NULL);
      if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
        switch (startUpOnOff) {
          case SL_ZIGBEE_ZCL_START_UP_ON_OFF_VALUE_SET_TO_OFF:
            updatedOnOff = 0; // Off
            break;
          case SL_ZIGBEE_ZCL_START_UP_ON_OFF_VALUE_SET_TO_ON:
            updatedOnOff = 1; //On
            break;
          case SL_ZIGBEE_ZCL_START_UP_ON_OFF_VALUE_SET_TO_TOGGLE:
            updatedOnOff = !updatedOnOff;
            break;
          case SL_ZIGBEE_ZCL_START_UP_ON_OFF_VALUE_SET_TO_PREVIOUS:
          default:
            // All other values 0x03- 0xFE are reserved - no action.
            // When value is 0xFF - update with last value - that is as good as
            // no action.
            break;
        }
        status = sl_zigbee_af_write_attribute(endpoint,
                                              ZCL_ON_OFF_CLUSTER_ID,
                                              ZCL_ON_OFF_ATTRIBUTE_ID,
                                              CLUSTER_MASK_SERVER,
                                              (uint8_t *)&updatedOnOff,
                                              ZCL_BOOLEAN_ATTRIBUTE_TYPE);
      }
    }
  }
#endif
  sl_zigbee_af_on_off_cluster_server_post_init_cb(endpoint);
}

#ifdef ZCL_USING_ON_OFF_CLUSTER_START_UP_ON_OFF_ATTRIBUTE
static bool areStartUpOnOffServerAttributesTokenized(uint8_t endpoint)
{
  sl_zigbee_af_attribute_metadata_t *metadata;

  metadata = sl_zigbee_af_locate_attribute_metadata(endpoint,
                                                    ZCL_ON_OFF_CLUSTER_ID,
                                                    ZCL_ON_OFF_ATTRIBUTE_ID,
                                                    CLUSTER_MASK_SERVER,
                                                    SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);
  if (!sl_zigbee_af_attribute_is_tokenized(metadata)) {
    return false;
  }

  metadata = sl_zigbee_af_locate_attribute_metadata(endpoint,
                                                    ZCL_ON_OFF_CLUSTER_ID,
                                                    ZCL_START_UP_ON_OFF_ATTRIBUTE_ID,
                                                    CLUSTER_MASK_SERVER,
                                                    SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);
  if (!sl_zigbee_af_attribute_is_tokenized(metadata)) {
    return false;
  }

  return true;
}
#endif

uint32_t sl_zigbee_af_on_off_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                          sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_OFF_COMMAND_ID:
      {
        status = sl_zigbee_af_on_off_cluster_off_cb();
        break;
      }
      case ZCL_ON_COMMAND_ID:
      {
        status = sl_zigbee_af_on_off_cluster_on_cb();
        break;
      }
      case ZCL_TOGGLE_COMMAND_ID:
      {
        status = sl_zigbee_af_on_off_cluster_toggle_cb();
        break;
      }
    }
  }

  return status;
}
