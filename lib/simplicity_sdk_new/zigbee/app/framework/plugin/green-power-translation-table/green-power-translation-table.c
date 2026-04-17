/***************************************************************************//**
 * @file
 * @brief Routines for the Green Power Translation Table plugin.
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
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/common.h"
#include "zap-cluster-command-parser.h"
#include "zap-type.h"
#else // !SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "green-power-adapter.h"
#endif //SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

#include "green-power-translation-table.h"

#include "green-power-server.h"
#include "green-power-common.h"
#include "sl_zigbee_token.h"
#include "stack/config/sl_zigbee_token_defines.h"
#include "sl_token_manager_api.h"

#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USER_HAS_DEFAULT_TRANSLATION_TABLE == 1)
#define USER_HAS_DEFAULT_TRANSLATION_TABLE
#endif
#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USER_HAS_DEFAULT_GENERIC_SWITCH_TRANSLATION_TABLE == 1)
#define USER_HAS_DEFAULT_GENERIC_SWITCH_TRANSLATION_TABLE
#endif

#define isClusterInManufactureSpeceficRange(clusterId) (0xFC00 <= clusterId)
#define isAttributeInManufactureSpecificRange(attributeId) (0x5000 <= attributeId)

#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
// wrapper for common token manager APIs if GP adapter isn't present
#define sl_zigbee_gp_set_token(token, data, length) (void)sl_token_manager_set_data(token, data, length)
#define sl_zigbee_gp_get_token(token, data, length) (void)sl_token_manager_get_data(token, data, length)
#endif

#ifndef USER_HAS_DEFAULT_TRANSLATION_TABLE
// Following GPDF to Zcl Command only applies for following ApplicationID
// SL_ZIGBEE_GP_APPLICATION_SOURCE_ID(0b000) and SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS (0b010)
// NOTE: all mapped ZCL commands have frame control of ZCL_FRAME_CONTROL_CLIENT_TO_SERVER.
extern const sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t sl_zigbee_gp_default_translation_table[];
extern uint16_t sli_zigbee_af_gp_default_table_size;
static const sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t *getSystemDefinedDefaultTable(uint16_t *tableSize)
{
  *tableSize = sli_zigbee_af_gp_default_table_size;
  return sl_zigbee_gp_default_translation_table;
}
#endif // !USER_HAS_DEFAULT_TRANSLATION_TABLE

#ifndef USER_HAS_DEFAULT_GENERIC_SWITCH_TRANSLATION_TABLE
sl_zigbee_af_green_power_server_defaut_generic_sw_translation_t sl_zigbee_gp_switch_translation_table[] = {
  // default table for switch configuration :
  // switchType, nbOfIdentifiedContacts, nbOfTTEntriesNeeded, indicativeBitmask, gpdCommand, endpoint, zigbeeProfile, zigbeeCluster, zigbeeCommandId, payloadSrc, zclPayloadDefault
  // 1bp --> Toggle
  // b3, b0, b1, b0 TOGGLE
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 1, 1, 0x0F }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_TOGGLE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },

  // 2bp --> on off   b2, b0 OFF /  b3, b1 ON
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 2, 2, 0x0A }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 2, 2, 0x05 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_OFF_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },

  // 3bp --> dim+/stop dim-/stop toggle   b0 UP /b1 DOWN /b3 TOGGLE
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 3, 3, 0x08 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_TOGGLE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 3, 3, 0x02 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_MOVE_MODE_DOWN } } },
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 3, 3, 0x01 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_WITH_ON_OFF_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_MOVE_MODE_UP } } },
  // b0, b1 stop
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 3, 1, 0x03 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STOP_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },

  // 4bp --> on/off dim+/stop dim-/stop
  // b0 UP /b1 DOWN/ b2 OFF/b3 ON
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 4, 4, 0x08 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 4, 4, 0x04 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_OFF_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 4, 4, 0x02 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_MOVE_MODE_DOWN } } },
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 4, 4, 0x01 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_WITH_ON_OFF_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_MOVE_MODE_UP } } },
  // b0, b1 STOP
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 4, 1, 0x03 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STOP_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },

  // 5bp to 8bp --> pass to application
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 5, 1, 0x01 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFC, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 5, 1, 0x01 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STOP_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 6, 1, 0x20 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFC, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 6, 1, 0x01 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STOP_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 8, 1, 0x80 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFC, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 8, 1, 0x01 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STOP_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 7, 1, 0x40 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFC, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE, 7, 1, 0x01 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STOP_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },

  // 1 rocker --> on/off
  //  b2, b0 OFF /  b3, b1 ON
  { { SL_ZIGBEE_ZCL_GP_ROCKER_SWITCH_TYPE, 2, 2, 0x0A }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { SL_ZIGBEE_ZCL_GP_ROCKER_SWITCH_TYPE, 2, 2, 0x05 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_OFF_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },

  // 2 rocker --> on/off dim+/stop dim-/stop
  // b0 UP /b1 DOWN/ b2 OFF/b3 ON
  { { SL_ZIGBEE_ZCL_GP_ROCKER_SWITCH_TYPE, 2, 4, 0x08 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { SL_ZIGBEE_ZCL_GP_ROCKER_SWITCH_TYPE, 2, 4, 0x04 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_OFF_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { SL_ZIGBEE_ZCL_GP_ROCKER_SWITCH_TYPE, 2, 4, 0x02 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_MOVE_MODE_DOWN } } },
  { { SL_ZIGBEE_ZCL_GP_ROCKER_SWITCH_TYPE, 2, 4, 0x01 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_MOVE_WITH_ON_OFF_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED | SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD, { 0x01, SL_ZIGBEE_ZCL_MOVE_MODE_UP } } },
  { { SL_ZIGBEE_ZCL_GP_ROCKER_SWITCH_TYPE, 2, 1, 0x03 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE, 0xFF, HA_PROFILE_ID, ZCL_LEVEL_CONTROL_CLUSTER_ID, 1, ZCL_STOP_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },

  // 3 to 4 rocker --> pass to application
  { { SL_ZIGBEE_ZCL_GP_ROCKER_SWITCH_TYPE, 3, 1, 0x20 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFC, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { SL_ZIGBEE_ZCL_GP_ROCKER_SWITCH_TYPE, 3, 1, 0x20 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE, 0xFC, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },

  { { SL_ZIGBEE_ZCL_GP_ROCKER_SWITCH_TYPE, 4, 1, 0x80 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS, 0xFC, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
  { { SL_ZIGBEE_ZCL_GP_ROCKER_SWITCH_TYPE, 4, 1, 0x80 }, { true, SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE, 0xFC, HA_PROFILE_ID, ZCL_ON_OFF_CLUSTER_ID, 1, ZCL_ON_COMMAND_ID, SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA, { 0 } } },
};

static const sl_zigbee_af_green_power_server_defaut_generic_sw_translation_t*getSystemDefinedDefaultGenericSwitchTable(uint16_t *tableSize)
{
  *tableSize = (sizeof(sl_zigbee_gp_switch_translation_table) / sizeof(sl_zigbee_af_green_power_server_defaut_generic_sw_translation_t));
  return &sl_zigbee_gp_switch_translation_table[0];
}
#endif // !USER_HAS_DEFAULT_GENERIC_SWITCH_TRANSLATION_TABLE

static const sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t *sli_zigbee_af_gp_default_table_ptr = NULL;
static const sl_zigbee_af_green_power_server_defaut_generic_sw_translation_t *sli_zigbee_af_gp_default_generic_switch_table_ptr = NULL;
static uint16_t sli_zigbee_af_gp_default_generic_switch_table_size = 0;
static sli_zigbee_af_gp_command_translation_table sli_zigbee_af_gp_translation_table = { 0 };
static sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t customizedTranslationTable[SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE] =  { 0 };
static sl_zigbee_gp_translation_table_additional_info_block_field_t sli_zigbee_af_gp_additional_info_table = { 0 };

// Static functions
static uint8_t createCustomizedTranslationTableEntry(bool infoBlockPresent,
                                                     sl_zigbee_gp_address_t *gpdAddr,
                                                     uint8_t gpdCommandId,
                                                     uint8_t zbEndpoint,
                                                     uint16_t zigbeeProfile,
                                                     uint16_t zigbeeCluster,
                                                     uint8_t zigbeeCommandId,
                                                     uint8_t payloadLength,
                                                     uint8_t *payload,
                                                     uint8_t payloadSrc,
                                                     uint8_t *outIndex)
{
  UNUSED_VAR(infoBlockPresent);
  UNUSED_VAR(gpdAddr);

  uint8_t cTableIndex;
  sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t *customizedTable = sli_zigbee_af_gp_get_customized_table();
  sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t *customizedTableEntry = NULL;
  if (customizedTable == NULL) {
    return GP_TRANSLATION_TABLE_STATUS_FAILED;
  }
  for (cTableIndex = 0; cTableIndex < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE; cTableIndex++) {
    customizedTableEntry = &(customizedTable[cTableIndex]);
    if (customizedTableEntry->validEntry == false) {
      break;
    }
  }
  if (cTableIndex == SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE) {
    return GP_TRANSLATION_TABLE_STATUS_FULL;
  }
  customizedTableEntry->gpdCommand = gpdCommandId;
  customizedTableEntry->endpoint = zbEndpoint;
  customizedTableEntry->zigbeeProfile = zigbeeProfile;
  customizedTableEntry->zigbeeCluster = zigbeeCluster;
  customizedTableEntry->zigbeeCommandId = zigbeeCommandId;
  if (payloadSrc) {
    customizedTableEntry->payloadSrc = payloadSrc;
  } else {
    customizedTableEntry->payloadSrc = SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA;
  }
  customizedTableEntry->zclPayloadDefault[0] = payloadLength;
  if (payloadLength > 0 && payloadLength < 0xFE
      &&  payloadLength < SL_ZIGBEE_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN) {
    memcpy(&customizedTableEntry->zclPayloadDefault[1], payload, payloadLength);
  }
  customizedTableEntry->validEntry = true;
  *outIndex = cTableIndex;
  sli_zigbee_af_gp_set_customized_table_entry(cTableIndex);
  return GP_TRANSLATION_TABLE_STATUS_SUCCESS;
}

static uint8_t compareTranslationTableAdditionalInfoBlockEntry(uint8_t gpdCommand,
                                                               sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *srcAddlInfoBlock,
                                                               sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *dstAddlInfoBlock)
{
  if (gpdCommand == SL_ZIGBEE_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING) {
    if ((srcAddlInfoBlock->optionData.compactAttr.reportIdentifier == dstAddlInfoBlock->optionData.compactAttr.reportIdentifier)
        && (srcAddlInfoBlock->optionData.compactAttr.attrOffsetWithinReport == dstAddlInfoBlock->optionData.compactAttr.attrOffsetWithinReport)
        && (srcAddlInfoBlock->optionData.compactAttr.clusterID == dstAddlInfoBlock->optionData.compactAttr.clusterID)
        && (srcAddlInfoBlock->optionData.compactAttr.attributeID == dstAddlInfoBlock->optionData.compactAttr.attributeID)
        && (srcAddlInfoBlock->optionData.compactAttr.attributeDataType == dstAddlInfoBlock->optionData.compactAttr.attributeDataType)
        && (srcAddlInfoBlock->optionData.compactAttr.attributeOptions == dstAddlInfoBlock->optionData.compactAttr.attributeOptions)
        && (srcAddlInfoBlock->optionData.compactAttr.manufacturerID == dstAddlInfoBlock->optionData.compactAttr.manufacturerID)) {
      return true;
    }
  } else if (gpdCommand == SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS
             || gpdCommand == SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE) {
    if ((srcAddlInfoBlock->optionData.genericSwitch.contactStatus == dstAddlInfoBlock->optionData.genericSwitch.contactStatus)
        && (srcAddlInfoBlock->optionData.genericSwitch.contactBitmask == dstAddlInfoBlock->optionData.genericSwitch.contactBitmask)) {
      return true;
    }
  }
  return false;
}

static uint8_t deleteTanslationTableAdditionalInfoBlockEntry(uint8_t gpdCommand,
                                                             sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *additionalInfoBlock)
{
  uint8_t optionRecord = 0;
  sl_zigbee_gp_translation_table_additional_info_block_field_t *additionalInfoTable = sli_zigbee_af_gp_get_additional_info_table();
  sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *addInfo = NULL;
  for (optionRecord = 0; optionRecord < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE; optionRecord++) {
    addInfo = &(additionalInfoTable->additionalInfoBlock[optionRecord]);
    if (compareTranslationTableAdditionalInfoBlockEntry(gpdCommand, addInfo, additionalInfoBlock)) {
      if (additionalInfoTable->validEntry[optionRecord]) {
        additionalInfoTable->validEntry[optionRecord]--;
        if (additionalInfoTable->validEntry[optionRecord] == 0) {
          memset(addInfo, 0x00, sizeof(sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t));
          additionalInfoTable->totlaNoOfEntries--;
        }
        sli_zigbee_af_gp_set_additional_info_block_table_entry(optionRecord);
        return optionRecord;
      }
    }
  }
  return 0xFF;
}

static uint8_t createTanslationTableAdditionalInfoBlockEntry(uint8_t gpdCommand,
                                                             sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *additionalInfoBlock)
{
  sl_zigbee_gp_translation_table_additional_info_block_field_t *additionalInfoTable = sli_zigbee_af_gp_get_additional_info_table();
  sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *addInfo = NULL;
  for (uint8_t optionRecord = 0; optionRecord < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE; optionRecord++) {
    addInfo = &(additionalInfoTable->additionalInfoBlock[optionRecord]);
    if (compareTranslationTableAdditionalInfoBlockEntry(gpdCommand, addInfo, additionalInfoBlock)) {
      additionalInfoTable->validEntry[optionRecord]++;
      sli_zigbee_af_gp_set_additional_info_block_table_entry(optionRecord);
      return optionRecord;
    }
  }
  for (uint8_t optionRecord = 0; optionRecord < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE; optionRecord++) {
    addInfo = &(additionalInfoTable->additionalInfoBlock[optionRecord]);
    if (additionalInfoTable->validEntry[optionRecord] == 0) {
      memcpy(addInfo, additionalInfoBlock, sizeof(sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t));
      additionalInfoTable->validEntry[optionRecord] += 1;
      additionalInfoTable->totlaNoOfEntries++;
      sli_zigbee_af_gp_set_additional_info_block_table_entry(optionRecord);
      return optionRecord;
    }
  }
  return 0xFF;
}

static bool isCustomizedTableEntryReferenced(uint8_t index, uint8_t offset)
{
  sli_zigbee_af_gp_command_translation_table *sli_zigbee_gp_translation_table = sli_zigbee_af_gp_trans_table_get_translation_table();
  if ((sli_zigbee_gp_translation_table != NULL) && (offset != 0xFF)) {
    for (uint8_t i = 0; i < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE; i++) {
      if ((sli_zigbee_gp_translation_table->TableEntry[i].entry == CUSTOMIZED_TABLE_ENTRY)
          && (sli_zigbee_gp_translation_table->TableEntry[i].offset == offset)
          && (index != i)) {
        return true;           //do not delete as reference is found for this customized table entry
      }
    }
  }
  return false;
}

static void deleteCustomizednTableEntry(uint8_t index)
{
  sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t *customizedTable = sli_zigbee_af_gp_get_customized_table();
  if (index < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE) {
    memset(&(customizedTable[index]),
           0x00,
           sizeof(sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t));
    sli_zigbee_af_gp_set_customized_table_entry(index);
  }
}

static uint8_t addTranslationTableEntryByIndex(uint8_t incomingReqType,
                                               uint8_t index,
                                               bool infoBlockPresent,
                                               sl_zigbee_gp_address_t *gpdAddr,
                                               uint8_t gpdCommandId,
                                               uint8_t zbEndpoint,
                                               uint16_t zigbeeProfile,
                                               uint16_t zigbeeCluster,
                                               uint8_t zigbeeCommandId,
                                               uint8_t payloadLength,
                                               uint8_t *payload,
                                               uint8_t payloadSrc,
                                               uint8_t additionalInfoLength,
                                               sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *additionalInfoBlock)
{
  UNUSED_VAR(additionalInfoLength);
  uint8_t ret = GP_TRANSLATION_TABLE_STATUS_FAILED;
  sli_zigbee_af_gp_command_translation_table *sli_zigbee_gp_translation_table = sli_zigbee_af_gp_trans_table_get_translation_table();
  uint8_t outIndex = 0xFF;
  uint8_t entryType = NO_ENTRY;
  for (entryType = CUSTOMIZED_TABLE_ENTRY; entryType > NO_ENTRY; entryType--) {
    // First search for the matching entry in the Customized table. If not found then search in the Default Table.
    ret = sli_zigbee_af_gp_find_matching_generic_translation_table_entry(entryType,
                                                                         incomingReqType,
                                                                         0xFF,
                                                                         infoBlockPresent,
                                                                         gpdCommandId,
                                                                         zigbeeProfile,
                                                                         zigbeeCluster,
                                                                         zigbeeCommandId,
                                                                         payloadLength,
                                                                         payload,
                                                                         &outIndex);
    if (ret == GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
      break;
    }
  }
  if ((outIndex == 0xFF) && (entryType == NO_ENTRY)) {
    ret = createCustomizedTranslationTableEntry(infoBlockPresent,
                                                gpdAddr,
                                                gpdCommandId,
                                                zbEndpoint,
                                                zigbeeProfile,
                                                zigbeeCluster,
                                                zigbeeCommandId,
                                                payloadLength,
                                                payload,
                                                payloadSrc,
                                                &outIndex);

    if (ret != GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
      return ret;
    } else {
      entryType = CUSTOMIZED_TABLE_ENTRY;
    }
  }
  if (outIndex != 0xFF) {
    if (sli_zigbee_gp_translation_table->TableEntry[index].entry == NO_ENTRY) {
      //Do not increment total number of entries while replacing Table entry
      sli_zigbee_gp_translation_table->totalNoOfEntries++;
      memset(&sli_zigbee_gp_translation_table->TableEntry[index], 0x00, sizeof(sli_zigbee_af_gp_command_translation_table_entry));
      sli_zigbee_gp_translation_table->TableEntry[index].offset = 0xFF;
      sli_zigbee_gp_translation_table->TableEntry[index].additionalInfoOffset = 0xFF;
    }
    if (infoBlockPresent) {
      uint8_t addInfoOffset = sli_zigbee_gp_translation_table->TableEntry[index].additionalInfoOffset;
      if ((sli_zigbee_gp_translation_table->TableEntry[index].entry != NO_ENTRY) && (addInfoOffset != 0xFF)) {
        sl_zigbee_gp_translation_table_additional_info_block_field_t *additionalInfoTable = sli_zigbee_af_gp_get_additional_info_table();
        sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *addInfo = &(additionalInfoTable->additionalInfoBlock[addInfoOffset]);
        uint8_t status = deleteTanslationTableAdditionalInfoBlockEntry(gpdCommandId, addInfo);
        if (status != 0xFF) {
          sli_zigbee_gp_translation_table->TableEntry[index].additionalInfoOffset = 0xFF;
        }
      }
      sli_zigbee_gp_translation_table->TableEntry[index].additionalInfoOffset = createTanslationTableAdditionalInfoBlockEntry(gpdCommandId, additionalInfoBlock);
    }
    if ((entryType == CUSTOMIZED_TABLE_ENTRY)
        && (sli_zigbee_gp_translation_table->TableEntry[index].offset != 0xFF)
        && (sli_zigbee_gp_translation_table->TableEntry[index].offset != outIndex)) {
      if (!isCustomizedTableEntryReferenced(index, sli_zigbee_gp_translation_table->TableEntry[index].offset)) {
        deleteCustomizednTableEntry(sli_zigbee_gp_translation_table->TableEntry[index].offset);
        sli_zigbee_gp_translation_table->TableEntry[index].offset = 0xFF;
      }
    }

    sli_zigbee_gp_translation_table->TableEntry[index].infoBlockPresent = infoBlockPresent;
    sli_zigbee_gp_translation_table->TableEntry[index].gpApplicationId = gpdAddr->applicationId;
    memcpy(&(sli_zigbee_gp_translation_table->TableEntry[index].gpAddr), gpdAddr, sizeof(sl_zigbee_gp_address_t));
    sli_zigbee_gp_translation_table->TableEntry[index].gpdCommand = gpdCommandId;
    sli_zigbee_gp_translation_table->TableEntry[index].zbEndpoint = zbEndpoint;
    sli_zigbee_gp_translation_table->TableEntry[index].offset = outIndex;
    sli_zigbee_gp_translation_table->TableEntry[index].entry = entryType;
    sli_zigbee_af_gp_set_translation_table_entry(index);
    ret = GP_TRANSLATION_TABLE_STATUS_SUCCESS;
  }
  return ret;
}

static uint8_t addPairedDeviceToTranslationTable(uint8_t incomingReqType,
                                                 bool infoBlockPresent,
                                                 sl_zigbee_gp_address_t *gpdAddr,
                                                 uint8_t gpdCommandId,
                                                 uint8_t zbEndpoint,
                                                 uint16_t zigbeeProfile,
                                                 uint16_t zigbeeCluster,
                                                 uint8_t zigbeeCommandId,
                                                 uint8_t payloadLength,
                                                 uint8_t *payload,
                                                 uint8_t payloadSrc,
                                                 uint8_t additionalInfoLength,
                                                 sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *additionalInfoBlock,
                                                 uint8_t *outNewTTEntryIndex)
{
  int tableIndex;
  uint8_t ret = GP_TRANSLATION_TABLE_STATUS_FAILED;
  sli_zigbee_af_gp_command_translation_table *sli_zigbee_gp_translation_table = sli_zigbee_af_gp_trans_table_get_translation_table();

  if ((sli_zigbee_gp_translation_table->totalNoOfEntries) >= SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE) {
    return GP_TRANSLATION_TABLE_STATUS_FULL;
  }

  for (tableIndex = 0; tableIndex < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE; tableIndex++) {
    if (sli_zigbee_gp_translation_table->TableEntry[tableIndex].entry == NO_ENTRY ) {
      break;
    }
  }

  if (tableIndex == SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE) {
    return GP_TRANSLATION_TABLE_STATUS_FULL;
  }

  ret = addTranslationTableEntryByIndex(incomingReqType,
                                        tableIndex,
                                        infoBlockPresent,
                                        gpdAddr,
                                        gpdCommandId,
                                        zbEndpoint,
                                        zigbeeProfile,
                                        zigbeeCluster,
                                        zigbeeCommandId,
                                        payloadLength,
                                        payload,
                                        payloadSrc,
                                        additionalInfoLength,
                                        additionalInfoBlock);
  if (ret != GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
    memset(&sli_zigbee_gp_translation_table->TableEntry[tableIndex], 0x00, sizeof(sli_zigbee_af_gp_command_translation_table_entry));
    sli_zigbee_gp_translation_table->TableEntry[tableIndex].offset = 0xFF;
    sli_zigbee_gp_translation_table->TableEntry[tableIndex].additionalInfoOffset = 0xFF;
    sli_zigbee_gp_translation_table->TableEntry[tableIndex].entry = NO_ENTRY;
  } else {
    *outNewTTEntryIndex = tableIndex;
  }
  return ret;
}

static void updateSwitchCurrentContact(sl_zigbee_gp_address_t *gpdAddr)
{
  sli_zigbee_gp_comm_data_saved_t *commissioningGpd = sl_zigbee_af_green_power_server_find_commissioning_gpd_instance(gpdAddr);
  // The following is to ensure the rocker gets both the entries in a pair even if one of the bit is set.
  if (commissioningGpd != NULL
      && commissioningGpd->switchInformationStruct.switchType == SL_ZIGBEE_ZCL_GP_ROCKER_SWITCH_TYPE) {
    // Realign the current contacts as a pair of bits.
    // If one bit of the bit pairs (b7,b6),(b5,b4),(b3,b2) or (b1,b0), is set then set the bit pair
    for (uint8_t i = 0; i < commissioningGpd->switchInformationStruct.nbOfContacts; i += 2) {
      if (commissioningGpd->switchInformationStruct.currentContact & (0x03 << i)) {
        commissioningGpd->switchInformationStruct.currentContact |= (0x03 << i);
      }
    }
  }
}

WEAK(void sl_zigbee_af_green_power_translation_table_stack_status_cb(sl_status_t status))
{
  sl_zigbee_af_green_power_cluster_println("Green Power Translation Table Stack Status Callback status = %02X", status);
  if (status == SL_STATUS_NETWORK_DOWN
      && sl_zigbee_is_performing_rejoin() == FALSE) {
    // Clear the additional info, translation table and sink table in order.
    sli_zigbee_gp_clear_additional_info_block_table();
    sli_zigbee_af_gp_clear_customized_table();
    sli_zigbee_af_gp_trans_table_clear_translation_table();
  }
}

sl_status_t sl_zigbee_af_green_power_translation_table_token_init(void)
{
  sl_status_t status = SL_STATUS_OK;
  sli_zigbee_af_gp_command_translation_table_entry tokTranslationTableDefault = TOKEN_TRANSLATION_TABLE_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_TRANSLATION_TABLE, &tokTranslationTableDefault, sizeof(sli_zigbee_af_gp_command_translation_table_entry), SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE);
  sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t tokCustomizedTableDefault = TOKEN_CUSTOMIZED_TABLE_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_CUSTOMIZED_TABLE, &tokCustomizedTableDefault, sizeof(sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t), SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE);
  sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t tokAdditionalInfoTableDefault = TOKEN_ADDITIONALINFO_TABLE_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_ADDITIONALINFO_TABLE, &tokAdditionalInfoTableDefault, sizeof(sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t), SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE);
  uint8_t tokAdditionalInfoTableValidEntriesDefault = TOKEN_ADDITIONALINFO_TABLE_VALID_ENTRIES_DEFAULT;
  status = sl_zigbee_initialize_index_token(COMMON_TOKEN_ADDITIONALINFO_TABLE_VALID_ENTRIES, &tokAdditionalInfoTableValidEntriesDefault, sizeof(uint8_t), SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE);
  uint8_t tokTranslationTableTotalEntriesDefault = TOKEN_TRANSLATION_TABLE_TOTAL_ENTRIES_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_TRANSLATION_TABLE_TOTAL_ENTRIES, &tokTranslationTableTotalEntriesDefault, sizeof(uint8_t));
  uint8_t tokAdditionalInfoTableTotalEntriesDefault = TOKEN_ADDITIONALINFO_TABLE_TOTAL_ENTRIES_DEFAULT;
  status = sl_zigbee_initialize_basic_token(COMMON_TOKEN_ADDITIONALINFO_TABLE_TOTAL_ENTRIES, &tokAdditionalInfoTableTotalEntriesDefault, sizeof(uint8_t));
  return status;
}

WEAK(void sl_zigbee_af_green_power_translation_table_init_cb(uint8_t init_level))
{
  UNUSED_VAR(init_level);

#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS == 1) && !defined(EZSP_HOST)
  // On device initialization, Read Translation Table / Customized Table
  // and the Aditional Info Block Table from the persistent memory.

  //sli_zigbee_af_gp_get_customized_table();

  for (int i = 0; i < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE; i++) {
    sl_zigbee_gp_get_token(COMMON_TOKEN_CUSTOMIZED_TABLE + i,
                           (void *)&customizedTranslationTable[i],
                           sizeof(sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t));
  }

  //sli_zigbee_af_gp_trans_table_get_translation_table();
  for (int i = 0; i < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE; i++) {
    sl_zigbee_gp_get_token(COMMON_TOKEN_TRANSLATION_TABLE + i,
                           (void *)&sli_zigbee_af_gp_translation_table.TableEntry[i],
                           sizeof(sli_zigbee_af_gp_command_translation_table_entry));
  }
  sl_zigbee_gp_get_token(COMMON_TOKEN_TRANSLATION_TABLE_TOTAL_ENTRIES,
                         (void *)&sli_zigbee_af_gp_translation_table.totalNoOfEntries,
                         sizeof(uint8_t));

  // sli_zigbee_af_gp_get_additional_info_table();
  for (int i = 0; i < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE; i++) {
    sl_zigbee_gp_get_token(COMMON_TOKEN_ADDITIONALINFO_TABLE + i,
                           (void *)&sli_zigbee_af_gp_additional_info_table.additionalInfoBlock[i],
                           sizeof(sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t));

    sl_zigbee_gp_get_token(COMMON_TOKEN_ADDITIONALINFO_TABLE_VALID_ENTRIES + i,
                           (void *)&sli_zigbee_af_gp_additional_info_table.validEntry[i],
                           sizeof(uint8_t));
  }
  sl_zigbee_gp_get_token(COMMON_TOKEN_ADDITIONALINFO_TABLE_TOTAL_ENTRIES,
                         (void *)&sli_zigbee_af_gp_additional_info_table.totlaNoOfEntries,
                         sizeof(uint8_t));

#endif // (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS == 1) && !defined(EZSP_HOST)
  assert(SL_STATUS_OK == sl_zigbee_af_green_power_translation_table_token_init());
  sl_zigbee_af_table_init_cb();
}

// Get the entire translation table
sli_zigbee_af_gp_command_translation_table *sli_zigbee_af_gp_trans_table_get_translation_table(void)
{
  return &sli_zigbee_af_gp_translation_table;
}

// Clears the entire translation table
void sli_zigbee_af_gp_trans_table_clear_translation_table(void)
{
  memset(&sli_zigbee_af_gp_translation_table, 0x00, sizeof(sli_zigbee_af_gp_command_translation_table));
  for (int i = 0; i < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE; i++) {
    sli_zigbee_af_gp_translation_table.TableEntry[i].offset = 0xFF;
    sli_zigbee_af_gp_translation_table.TableEntry[i].additionalInfoOffset = 0xFF;
#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS == 1) && !defined(EZSP_HOST)
    sl_zigbee_gp_set_token(COMMON_TOKEN_TRANSLATION_TABLE + i, (void *)&sli_zigbee_af_gp_translation_table.TableEntry[i], sizeof(sli_zigbee_af_gp_command_translation_table_entry));
#endif // (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS == 1) && !defined(EZSP_HOST)
  }
  sli_zigbee_af_gp_translation_table.totalNoOfEntries = 0;
#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS) && !defined(EZSP_HOST)
  sl_zigbee_gp_set_token(COMMON_TOKEN_TRANSLATION_TABLE_TOTAL_ENTRIES, (void *)&sli_zigbee_af_gp_translation_table.totalNoOfEntries, sizeof(uint8_t));
#endif // (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS == 1) && !defined(EZSP_HOST)
}

// Set the translation table entry at index
void sli_zigbee_af_gp_set_translation_table_entry(uint8_t index)
{
#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS == 1) && !defined(EZSP_HOST)
  if (index < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE) {
    sl_zigbee_gp_set_token(COMMON_TOKEN_TRANSLATION_TABLE_TOTAL_ENTRIES, (void *)&sli_zigbee_af_gp_translation_table.totalNoOfEntries, sizeof(uint8_t));
    sl_zigbee_gp_set_token(COMMON_TOKEN_TRANSLATION_TABLE + index, (void *)&sli_zigbee_af_gp_translation_table.TableEntry[index], sizeof(sli_zigbee_af_gp_command_translation_table_entry));
  }
#endif // (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS == 1) && !defined(EZSP_HOST)
}
// Get the entire customized table
sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t *sli_zigbee_af_gp_get_customized_table(void)
{
  if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE > 0) {
    return &customizedTranslationTable[0];
  } else {
    return NULL;
  }
}

// Clears the entire customized table
void sli_zigbee_af_gp_clear_customized_table(void)
{
  memset(customizedTranslationTable,
         0x00,
         sizeof(sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t)
         * SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE);
#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS == 1) && !defined(EZSP_HOST)
  for (int i = 0; i < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE; i++) {
    sl_zigbee_gp_set_token(COMMON_TOKEN_CUSTOMIZED_TABLE + i, (void *)&customizedTranslationTable[i], sizeof(sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t));
  }
#endif // (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS == 1) && !defined(EZSP_HOST)
}

// Set the custmized table entry at index
void sli_zigbee_af_gp_set_customized_table_entry(uint8_t index)
{
#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS == 1) && !defined(EZSP_HOST)
  if (index < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE) {
    sl_zigbee_gp_set_token(COMMON_TOKEN_CUSTOMIZED_TABLE + index, (void *)&customizedTranslationTable[index], sizeof(sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t));
  }
#endif // (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS == 1) && !defined(EZSP_HOST)
}

// Get the entire  additional info block table
sl_zigbee_gp_translation_table_additional_info_block_field_t *sli_zigbee_af_gp_get_additional_info_table(void)
{
  return &sli_zigbee_af_gp_additional_info_table;
}

// Clears the entire additional info block table
void sli_zigbee_gp_clear_additional_info_block_table(void)
{
  memset(&sli_zigbee_af_gp_additional_info_table, 0x00, sizeof(sl_zigbee_gp_translation_table_additional_info_block_field_t));
#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS == 1) && !defined(EZSP_HOST)
  for (int i = 0; i < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE; i++) {
    sl_zigbee_gp_set_token(COMMON_TOKEN_ADDITIONALINFO_TABLE + i, (void *)&(sli_zigbee_af_gp_additional_info_table.additionalInfoBlock[i]), sizeof(sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t));
    sl_zigbee_gp_set_token(COMMON_TOKEN_ADDITIONALINFO_TABLE_VALID_ENTRIES + i, (void *)&(sli_zigbee_af_gp_additional_info_table.validEntry[i]), sizeof(uint8_t));
  }
  sl_zigbee_gp_set_token(COMMON_TOKEN_ADDITIONALINFO_TABLE_TOTAL_ENTRIES, (void *)&(sli_zigbee_af_gp_additional_info_table.totlaNoOfEntries), sizeof(uint8_t));
#endif // (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS == 1) && !defined(EZSP_HOST)
}

// Set the additional info block entry at index
void sli_zigbee_af_gp_set_additional_info_block_table_entry(uint8_t index)
{
#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS == 1) && !defined(EZSP_HOST)
  if (index < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE) {
    sl_zigbee_gp_set_token(COMMON_TOKEN_ADDITIONALINFO_TABLE + index, (void *)&(sli_zigbee_af_gp_additional_info_table.additionalInfoBlock[index]), sizeof(sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t));
    sl_zigbee_gp_set_token(COMMON_TOKEN_ADDITIONALINFO_TABLE_VALID_ENTRIES + index, (void *)&(sli_zigbee_af_gp_additional_info_table.validEntry[index]), sizeof(uint8_t));
    sl_zigbee_gp_set_token(COMMON_TOKEN_ADDITIONALINFO_TABLE_TOTAL_ENTRIES, (void *)&(sli_zigbee_af_gp_additional_info_table.totlaNoOfEntries), sizeof(uint8_t));
  }
#endif // (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_USE_TOKENS == 1) && !defined(EZSP_HOST)
}

// This function, validates and changes the option to fit the check if success
static bool manufactureIdValidation(uint8_t *attributeOptions,
                                    uint16_t manufacturerID,
                                    uint16_t clusterId,
                                    uint16_t attributeId)
{
  // Check if the cluster or attribute is in range of Manufacture Specific range
  if (((*attributeOptions) & 0x02)
      && (isClusterInManufactureSpeceficRange(clusterId)
          || isAttributeInManufactureSpecificRange(attributeId))) {
    // Check if the manufactureId is supported in sink for the attribute/cluster
    if (manufacturerID != SL_ZIGBEE_ZCL_MANUFACTURER_CODE) {
      // manufacturId present but does not match the manufactureCode in sink
      // TODO : call sl_zigbee_af_locate_attribute_metadata to locate if any such
      // attribute of a cluster is present in sink or not.
      return false;
    } else {
      return true;
    }
  }
  // Neither the cluster not attribute is manufacture specific, hence ensure the
  // manufacture specific flag in option;
  *attributeOptions &= ~(0x02);
  return true;
}

static void addReportstoTranslationTable(sl_zigbee_gp_address_t *gpdAddr,
                                         uint8_t gpdCommandId,
                                         uint8_t endpoint)
{
  UNUSED_VAR(gpdCommandId);
  // none entry match, create TT entries for New GPD
  sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t additionalInfo = { 0 };
  uint8_t additionalInfoReportLen = 0;
  uint8_t additionalInfoRecordLen = 0;
  uint8_t additionalInfoAttrLen = 0;
  uint8_t noOfattributes = 0;
  uint8_t length = 0;
  uint8_t reportLength = 0;
  uint8_t recordRemainingLength = 0;
  uint8_t attributeRemainingLength = 0;

  uint8_t index = 0;
  const uint8_t *report = NULL;
  const sli_zigbee_gp_comm_data_saved_t *gpdCommDataPtr = sl_zigbee_af_green_power_server_find_commissioning_gpd_instance(gpdAddr);

  if (gpdCommDataPtr == NULL) {
    return;
  }
  // Populate all the reports as option records of the additional block
  for (index = 0; index < gpdCommDataPtr->numberOfReports; index++) {
    report = sli_zigbee_af_gp_find_report_id(index,
                                             gpdCommDataPtr->numberOfReports,
                                             gpdCommDataPtr->reportsStorage);
    if (report != NULL) {
      length = 0;
      reportLength = 0;
      additionalInfoReportLen = 0;

      additionalInfo.optionData.compactAttr.reportIdentifier = report[length]; //reportId
      length += 1; //reportId
      additionalInfoReportLen++;
      if (report[length] & SL_ZIGBEE_AF_GP_GPD_APPLICATION_DESCRIPTION_COMMAND_REPORT_OPTIONS_TIMEOUT_PERIOD_PRESENT) {
        length += 3; // option and 2 byte timeout
      } else {
        length += 1; // just the option byte
      }
      recordRemainingLength = report[length];
      length += 1; // length
      additionalInfoReportLen++;

      reportLength = length; // length
      do {
        sl_zigbee_af_print_buffer(SL_ZIGBEE_AF_PRINT_CORE, &report[length - 4], (recordRemainingLength + 4), true);
        sl_zigbee_af_green_power_cluster_println("");
        additionalInfoRecordLen = 0;
        uint8_t tempAttributeOption = ((report[length] & 0x1F) >> 3); // attribute option
        noOfattributes = ( (report[length] & 0x07) + 1);
        length += 1; //data point options
        additionalInfoRecordLen++;
        additionalInfo.optionData.compactAttr.clusterID = report[length] + ((uint16_t)report[length + 1] << 8);
        length += 2; // clusterid
        additionalInfoRecordLen += 2;
        if (tempAttributeOption & 0x02) { // ManufactureId present
          additionalInfo.optionData.compactAttr.manufacturerID = report[length] + ((uint16_t)report[length + 1] << 8);
          length += 2;
        }
        additionalInfoRecordLen = (additionalInfoReportLen + additionalInfoRecordLen);
        for (uint8_t attrRecordIndex = 0; attrRecordIndex < noOfattributes; attrRecordIndex++) {
          additionalInfo.optionData.compactAttr.attributeOptions = tempAttributeOption;
          additionalInfoAttrLen = 0;
          additionalInfo.optionData.compactAttr.attributeID = report[length] + ((uint16_t)report[length + 1] << 8);
          length += 2;
          additionalInfoAttrLen += 2;
          additionalInfo.optionData.compactAttr.attributeDataType = report[length];
          length += 1;
          additionalInfoAttrLen++;
          uint8_t reportDescAttributeOption = report[length];
          length += 1;
          attributeRemainingLength = (reportDescAttributeOption & 0x0F) + 1;
          if (reportDescAttributeOption & 0x10) {
            if ((attributeRemainingLength) && (recordRemainingLength > (length - reportLength))) {
              additionalInfo.optionData.compactAttr.attrOffsetWithinReport = report[length];
              length += 1;
              attributeRemainingLength--;
            }
          } else {
            additionalInfo.optionData.compactAttr.attrOffsetWithinReport = 0x00;
          }
          additionalInfoAttrLen++;
          if (reportDescAttributeOption & 0x20) {
            if ((attributeRemainingLength) && (recordRemainingLength > (length - reportLength))) {
              length += attributeRemainingLength;
            }
          }
          if (reportDescAttributeOption & 0x10) { // Reported bit set
            if (sli_zigbee_af_gp_endpoint_and_cluster_id_validation(endpoint,
                                                                    !(additionalInfo.optionData.compactAttr.attributeOptions & 0x01), // Mask for server/Client
                                                                    additionalInfo.optionData.compactAttr.clusterID)
                && manufactureIdValidation(&(additionalInfo.optionData.compactAttr.attributeOptions),
                                           additionalInfo.optionData.compactAttr.manufacturerID,
                                           additionalInfo.optionData.compactAttr.clusterID,
                                           additionalInfo.optionData.compactAttr.attributeID)) {
              if ((additionalInfo.optionData.compactAttr.attributeOptions) & 0x02) {
                additionalInfoRecordLen += 2;
              }
              additionalInfo.optionSelector = (additionalInfoRecordLen + additionalInfoAttrLen - additionalInfoReportLen);//(endRecord - startRecord - 1);
              sl_zigbee_af_green_power_cluster_println(" AdditionalInfo.optionSelector : %d additionalInfoReportdLen = %d additionalInfoRecordLen = %d additionalInfoAttrLen",
                                                       additionalInfo.optionSelector,
                                                       additionalInfoReportLen,
                                                       additionalInfoRecordLen,
                                                       additionalInfoAttrLen);
              additionalInfo.totalLengthOfAddInfoBlock = (additionalInfoRecordLen + additionalInfoAttrLen);

              uint8_t status = 0xFF;
              uint8_t outIndex;
              status = sli_zigbee_af_gp_trans_table_find_matching_translation_table_entry((GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID
                                                                                           | GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_CMD_ID
                                                                                           | GP_TRANSLATION_TABLE_SCAN_LEVEL_ZB_ENDPOINT
                                                                                           | GP_TRANSLATION_TABLE_SCAN_LEVEL_ADDITIONAL_INFO_BLOCK),//uint8_t levelOfScan,
                                                                                          true,
                                                                                          gpdAddr,
                                                                                          SL_ZIGBEE_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING,
                                                                                          endpoint,
                                                                                          NULL,
                                                                                          &additionalInfo,
                                                                                          &outIndex,
                                                                                          0);
              if (status != GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
                status = addPairedDeviceToTranslationTable(ADD_PAIRED_DEVICE,
                                                           true,
                                                           gpdAddr,
                                                           SL_ZIGBEE_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING,
                                                           endpoint,
                                                           0,
                                                           additionalInfo.optionData.compactAttr.clusterID,
                                                           0,
                                                           0,
                                                           NULL,
                                                           0,
                                                           additionalInfo.totalLengthOfAddInfoBlock,
                                                           &additionalInfo,
                                                           &outIndex);
                sl_zigbee_af_green_power_cluster_println("Status %d attrRecord index = %d ", status, attrRecordIndex);
              }
            }
          }
        }
        sl_zigbee_af_green_power_cluster_println("record index = %d ", index);
      } while (recordRemainingLength > (length - reportLength));
    }
  }
}

// Gets switch configuration from configured switches in switch translation table
static const sl_zigbee_af_green_power_server_defaut_generic_sw_translation_t *getSwConfig(uint8_t gpdCommandId,
                                                                                          uint8_t switchType,
                                                                                          uint8_t noOfPairedBits,
                                                                                          uint8_t currentContactStatus)
{
  const sl_zigbee_af_green_power_server_defaut_generic_sw_translation_t *genericSwitchTable = sli_zigbee_af_gp_default_generic_switch_table_ptr;
  if (genericSwitchTable == NULL) {
    return NULL;
  }
  for (int i = 0; i < sli_zigbee_af_gp_default_table_size; i++) {
    if (switchType == genericSwitchTable[i].SwitchType.switchType
        && (gpdCommandId == genericSwitchTable[i].genericSwitchDefaultTableEntry.gpdCommand)
        && noOfPairedBits == genericSwitchTable[i].SwitchType.nbOfIdentifiedContacts) {
      if (currentContactStatus & genericSwitchTable[i].SwitchType.indicativeBitmask) {
        return &(genericSwitchTable[i]);
      }
    }
  }
  return NULL;
}

// Prepares an additional info block from the switch configuration and received
// contact status
static void swInfoToAdditionalInfo(sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *additionalInfo,
                                   uint8_t currentContact)
{
  additionalInfo->totalLengthOfAddInfoBlock = 3;
  additionalInfo->optionSelector = 1;
  additionalInfo->optionData.genericSwitch.contactBitmask = currentContact;
  additionalInfo->optionData.genericSwitch.contactStatus = currentContact;
}

static void ignoreAnyBitSetHigherThanNoOfContacts(uint8_t noOfContacts, uint8_t *currentContact)
{
  uint8_t BitMask = 0;
  for (uint8_t iContact = 0; iContact < noOfContacts; iContact++) {
    BitMask |=  (0x01 << iContact);
  }
  (*currentContact) &= BitMask;
}

static uint8_t calculatePairedBits(uint8_t noOfContacts, uint8_t *savedContact)
{
  uint8_t noOfPairedBits = 0;
  ignoreAnyBitSetHigherThanNoOfContacts(noOfContacts,
                                        savedContact);

  for (uint8_t iContact = 0; iContact < noOfContacts; iContact++) {
    if ((*savedContact) & (0x01 << iContact)) {
      noOfPairedBits++;
    }
  }
  return noOfPairedBits;
}

static void countSwitchEntries(sl_zigbee_gp_address_t *gpdAddr, uint8_t *savedContact)
{
  sli_zigbee_af_gp_command_translation_table *sli_zigbee_gp_translation_table = sli_zigbee_af_gp_trans_table_get_translation_table();
  sl_zigbee_gp_translation_table_additional_info_block_field_t *additionalInfoTable = sli_zigbee_af_gp_get_additional_info_table();
  sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *addInfo = NULL;
  for ( uint8_t tableIndex = 0; tableIndex < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE; tableIndex++) {
    if (sli_zigbee_gp_translation_table->TableEntry[tableIndex].entry == NO_ENTRY) {
      continue;
    }
    if (sl_zigbee_af_green_power_common_gp_addr_compare(&sli_zigbee_gp_translation_table->TableEntry[tableIndex].gpAddr, gpdAddr)) {
      if (sli_zigbee_gp_translation_table->TableEntry[tableIndex].additionalInfoOffset != 0xFF) {
        addInfo = &(additionalInfoTable->additionalInfoBlock[sli_zigbee_gp_translation_table->TableEntry[tableIndex].additionalInfoOffset]);
        *savedContact |= addInfo->optionData.genericSwitch.contactStatus;
      }
    }
  }
}

static uint8_t deleteTranslationTableEntryByIndex(uint8_t index,
                                                  bool infoBlockPresent,
                                                  uint8_t gpdCommandId,
                                                  uint16_t zigbeeProfile,
                                                  uint16_t zigbeeCluster,
                                                  uint8_t zigbeeCommandId,
                                                  uint8_t payloadLength,
                                                  uint8_t *payload,
                                                  uint8_t additionalInfoLength,
                                                  sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *additionalInfoBlock)
{
  UNUSED_VAR(additionalInfoLength);
  UNUSED_VAR(additionalInfoBlock);
  uint8_t ret = GP_TRANSLATION_TABLE_STATUS_FAILED;
  sli_zigbee_af_gp_command_translation_table *sli_zigbee_gp_translation_table = sli_zigbee_af_gp_trans_table_get_translation_table();
  uint8_t status = 0xFF;
  uint8_t outIndex = 0xFF;
  ret = sli_zigbee_af_gp_find_matching_generic_translation_table_entry(sli_zigbee_gp_translation_table->TableEntry[index].entry,
                                                                       DELETE_PAIRED_DEVICE,
                                                                       sli_zigbee_gp_translation_table->TableEntry[index].offset,
                                                                       infoBlockPresent,
                                                                       gpdCommandId,
                                                                       zigbeeProfile,
                                                                       zigbeeCluster,
                                                                       zigbeeCommandId,
                                                                       payloadLength,
                                                                       payload,
                                                                       &outIndex);
  if (ret == GP_TRANSLATION_TABLE_STATUS_SUCCESS && outIndex != 0xFF) {
    sl_zigbee_gp_translation_table_additional_info_block_field_t *additionalInfoTable = sli_zigbee_af_gp_get_additional_info_table();
    uint8_t addInfoOffset = sli_zigbee_gp_translation_table->TableEntry[index].additionalInfoOffset;
    if (addInfoOffset != 0xFF) {
      status = deleteTanslationTableAdditionalInfoBlockEntry(gpdCommandId, &(additionalInfoTable->additionalInfoBlock[addInfoOffset]));
      if (status != 0xFF) {
        sli_zigbee_gp_translation_table->TableEntry[index].additionalInfoOffset = 0xFF;
      }
    }
    if ((sli_zigbee_gp_translation_table->TableEntry[index].entry == CUSTOMIZED_TABLE_ENTRY) && (sli_zigbee_gp_translation_table->TableEntry[index].offset != 0xFF)) {
      if (!isCustomizedTableEntryReferenced(index, sli_zigbee_gp_translation_table->TableEntry[index].offset)) {
        deleteCustomizednTableEntry(sli_zigbee_gp_translation_table->TableEntry[index].offset);
      }
    }
    memset(&sli_zigbee_gp_translation_table->TableEntry[index], 0x00, sizeof(sli_zigbee_af_gp_command_translation_table_entry));
    sli_zigbee_gp_translation_table->TableEntry[index].entry = NO_ENTRY;
    sli_zigbee_gp_translation_table->TableEntry[index].offset = 0xFF;
    sli_zigbee_gp_translation_table->TableEntry[index].additionalInfoOffset = 0xFF;
    sli_zigbee_gp_translation_table->totalNoOfEntries--;
    sli_zigbee_af_gp_set_translation_table_entry(index);
    ret = GP_TRANSLATION_TABLE_STATUS_SUCCESS;
  }
  return ret;
}

static void addSwitchToTranslationTable(sl_zigbee_gp_address_t *gpdAddr,
                                        uint8_t gpdCommandId,
                                        uint8_t endpoint)
{
  UNUSED_VAR(endpoint);
  uint8_t savedContact = 0;
  uint8_t bitShift = 0;
  sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t additionalInfo = { 0 };
  const sli_zigbee_gp_comm_data_saved_t *gpdCommDataPtr = sl_zigbee_af_green_power_server_find_commissioning_gpd_instance(gpdAddr);

  if (gpdCommDataPtr == NULL) {
    return;
  }
  uint8_t alignSwitchType = gpdCommDataPtr->switchInformationStruct.switchType;
  // look at the switch type and supported no of contacts to find how many
  //commands and what are the commands need to be added in TT.
  const sl_zigbee_af_green_power_server_defaut_generic_sw_translation_t *swConfig;
  // Find the first entry of the configuration, that will tell how many entries
  // to follow.
  //
  if (gpdCommDataPtr->switchInformationStruct.switchType == SL_ZIGBEE_ZCL_GP_UNKNOWN_SWITCH_TYPE) {
    alignSwitchType = SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE;
  } else {
    alignSwitchType = gpdCommDataPtr->switchInformationStruct.switchType;
    updateSwitchCurrentContact(gpdAddr);
  }

  countSwitchEntries(gpdAddr, &savedContact);
  if (savedContact == (savedContact | gpdCommDataPtr->switchInformationStruct.currentContact)) {
    return; //Ignore the contact if it is already commissioned
  }
  if (savedContact) {   // check if the switch already commissioned!
    for (bitShift = 0; bitShift < gpdCommDataPtr->switchInformationStruct.nbOfContacts; bitShift++) {
      uint8_t contactStatus = savedContact & (0x01 << bitShift);
      if (contactStatus) {
        sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t additionalInfo = { 0 };
        swInfoToAdditionalInfo(&additionalInfo, contactStatus);
        uint8_t outIndex = 0xFF;
        uint8_t status = sli_zigbee_af_gp_trans_table_find_matching_translation_table_entry((GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID
                                                                                             | GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_CMD_ID
                                                                                             | GP_TRANSLATION_TABLE_SCAN_LEVEL_ADDITIONAL_INFO_BLOCK),//uint8_t levelOfScan,
                                                                                            true,//bool infoBlockPresent,
                                                                                            gpdAddr,
                                                                                            gpdCommandId,
                                                                                            0,//uint8_t zbEndpoint,
                                                                                            0,
                                                                                            &additionalInfo,
                                                                                            &outIndex,
                                                                                            0);
        if (status == GP_TRANSLATION_TABLE_STATUS_SUCCESS
            && outIndex != 0xFF) {
          deleteTranslationTableEntryByIndex(outIndex,
                                             true,
                                             gpdCommandId,
                                             0,
                                             0,
                                             0,
                                             0x00,
                                             NULL,
                                             0x00,
                                             NULL);
        }
      }
    }
  }
  savedContact |= gpdCommDataPtr->switchInformationStruct.currentContact;
  uint8_t pairedBits = calculatePairedBits(gpdCommDataPtr->switchInformationStruct.nbOfContacts, &savedContact);
  //addpaired
  for (bitShift = 0; bitShift < gpdCommDataPtr->switchInformationStruct.nbOfContacts; bitShift++) {
    uint8_t contactStatus = (savedContact & (0x01 << bitShift));
    swConfig = getSwConfig(gpdCommandId, alignSwitchType, pairedBits, contactStatus);
    if (NULL != swConfig) {
      memset(&additionalInfo, 0x00, sizeof(sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t));
      swInfoToAdditionalInfo(&additionalInfo, contactStatus);
      uint8_t outIndex = 0xFF;
      const sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t *entry;
      entry = &(swConfig->genericSwitchDefaultTableEntry);
      (void)addPairedDeviceToTranslationTable(TRANSLATION_TABLE_UPDATE,
                                              true,
                                              gpdAddr,
                                              entry->gpdCommand,
                                              endpoint,
                                              entry->zigbeeProfile,
                                              entry->zigbeeCluster,
                                              entry->zigbeeCommandId,
                                              entry->zclPayloadDefault[0],
                                              (uint8_t *)&(entry->zclPayloadDefault[1]),
                                              entry->payloadSrc,
                                              additionalInfo.totalLengthOfAddInfoBlock,
                                              &additionalInfo,
                                              &outIndex);
    }
  }
}

// This function Copies the addiitional information block structure to an array to be
// used for the Translation Table request/update request reponses.
uint16_t sli_zigbee_af_gp_copy_additional_info_block_array_to_structure(uint8_t *additionalInfoBlockIn,
                                                                        sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *additionalInfoBlockOut,
                                                                        uint8_t gpdCommandId)
{
  UNUSED_VAR(gpdCommandId);
  uint8_t charCount = 0;
  uint8_t *additionalInfoBlockInPtr = additionalInfoBlockIn;
  uint8_t recordIndex = 0;
  uint8_t optionSelector = 0;
  uint8_t attributeoptions = 0;
  uint8_t totalLengthOfAddInfoBlockCnt = 0;
  sl_zigbee_af_green_power_cluster_println("GP SERVER - STORE ADDITIONAL INFOROMATION BLOCK into a structure");

  // copy GPD Additional information block length
  additionalInfoBlockOut->totalLengthOfAddInfoBlock = sl_zigbee_af_get_int8u(additionalInfoBlockInPtr, 0, 1);
  additionalInfoBlockInPtr += sizeof(uint8_t);
  totalLengthOfAddInfoBlockCnt = additionalInfoBlockOut->totalLengthOfAddInfoBlock;

  if ((additionalInfoBlockOut->totalLengthOfAddInfoBlock) != 0x00) {
    optionSelector = sl_zigbee_af_get_int8u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlock);
    additionalInfoBlockInPtr += sizeof(uint8_t);
    totalLengthOfAddInfoBlockCnt -= 1;

    if ( ((optionSelector & 0x0F) != 0) && (totalLengthOfAddInfoBlockCnt > 0)) {
      // copy  option selector field
      additionalInfoBlockOut->optionSelector = optionSelector;

      if (gpdCommandId  == SL_ZIGBEE_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING ) {
        additionalInfoBlockOut->optionData.compactAttr.reportIdentifier = sl_zigbee_af_get_int8u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlockCnt);
        additionalInfoBlockInPtr += sizeof(uint8_t);
        totalLengthOfAddInfoBlockCnt -= 1;

        additionalInfoBlockOut->optionData.compactAttr.attrOffsetWithinReport = sl_zigbee_af_get_int8u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlockCnt);
        additionalInfoBlockInPtr += sizeof(uint8_t);
        totalLengthOfAddInfoBlockCnt -= 1;

        additionalInfoBlockOut->optionData.compactAttr.clusterID = sl_zigbee_af_get_int16u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlockCnt);
        additionalInfoBlockInPtr += sizeof(uint16_t);
        totalLengthOfAddInfoBlockCnt -= 2;

        additionalInfoBlockOut->optionData.compactAttr.attributeID = sl_zigbee_af_get_int16u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlockCnt);
        additionalInfoBlockInPtr += sizeof(uint16_t);
        totalLengthOfAddInfoBlockCnt -= 2;

        additionalInfoBlockOut->optionData.compactAttr.attributeDataType = sl_zigbee_af_get_int8u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlockCnt);
        additionalInfoBlockInPtr += sizeof(uint8_t);
        totalLengthOfAddInfoBlockCnt -= 1;

        attributeoptions = sl_zigbee_af_get_int8u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlockCnt);
        additionalInfoBlockInPtr += sizeof(uint8_t);
        totalLengthOfAddInfoBlockCnt -= 1;
        //additionalInfoBlockOut->optionRecord[recordIndex].optionData.compactAttr.manufacturerIdPresent  = ( (attributeoptions >> 1) & 0x01);
        //additionalInfoBlockOut->optionRecord[recordIndex].optionData.compactAttr.clientServer = (attributeoptions & 0x01);
        // Until the above is uncommented out, void cast recordIndex to avoid an unused variable warning
        (void)recordIndex;
        additionalInfoBlockOut->optionData.compactAttr.attributeOptions  = attributeoptions;
        if ( additionalInfoBlockOut->optionData.compactAttr.attributeOptions & 0x02 ) {
          additionalInfoBlockOut->optionData.compactAttr.manufacturerID = sl_zigbee_af_get_int16u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlockCnt);
          additionalInfoBlockInPtr += sizeof(uint16_t);
          totalLengthOfAddInfoBlockCnt -= 1;
        }
        recordIndex++;
      } else {
        additionalInfoBlockOut->optionData.genericSwitch.contactStatus = sl_zigbee_af_get_int8u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlockCnt);
        additionalInfoBlockInPtr += sizeof(uint8_t);
        totalLengthOfAddInfoBlockCnt -= 1;

        additionalInfoBlockOut->optionData.genericSwitch.contactBitmask = sl_zigbee_af_get_int8u(additionalInfoBlockInPtr, 0, totalLengthOfAddInfoBlockCnt);
        additionalInfoBlockInPtr += sizeof(uint8_t);
        totalLengthOfAddInfoBlockCnt -= 1;
      }
    }
  }
  charCount = (uint16_t)(additionalInfoBlockInPtr - additionalInfoBlockIn);
  if (additionalInfoBlockOut->totalLengthOfAddInfoBlock != (charCount - 1)) {
    sl_zigbee_af_green_power_cluster_println("[%s:%d] Error in Addiotional Information Block", __FUNCTION__, __LINE__);
  }
  return charCount;
}

static uint16_t copyTranslationTableEntryToBuffer(uint8_t entryIndex,
                                                  sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t *translationTableEntry,
                                                  uint8_t *tempDatabuffer,
                                                  uint8_t tempDatabufferLen)
{
  UNUSED_VAR(tempDatabufferLen);
  uint8_t charCount = 0;
  uint8_t *bufPtr = tempDatabuffer;
  sli_zigbee_af_gp_command_translation_table *sli_zigbee_gp_translation_table = sli_zigbee_af_gp_trans_table_get_translation_table();

  sl_zigbee_af_green_power_cluster_println("GP SERVER - STORE TRANSLATION TABLE ENTRY into a buffer");
  // no more ID field

  // copy 32bit or 64bit address field
  if (sli_zigbee_gp_translation_table->TableEntry[entryIndex].gpAddr.applicationId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
    sl_zigbee_af_copy_int32u(bufPtr, 0, sli_zigbee_gp_translation_table->TableEntry[entryIndex].gpAddr.id.sourceId);
    bufPtr += sizeof(uint32_t);
  } else if (sli_zigbee_gp_translation_table->TableEntry[entryIndex].gpAddr.applicationId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
    memmove(bufPtr, sli_zigbee_gp_translation_table->TableEntry[entryIndex].gpAddr.id.gpdIeeeAddress, EUI64_SIZE);
    bufPtr += EUI64_SIZE;

    // copy ieee endpoint field
    sl_zigbee_af_copy_int8u(bufPtr, 0, sli_zigbee_gp_translation_table->TableEntry[entryIndex].gpAddr.endpoint);
    bufPtr += sizeof(uint8_t);
  }

  // copy GPD command ID field
  sl_zigbee_af_copy_int8u(bufPtr, 0, (uint8_t)(translationTableEntry->gpdCommand));
  bufPtr += sizeof(uint8_t);

  // copy zbEndpoint field
  sl_zigbee_af_copy_int8u(bufPtr, 0, (uint8_t)(translationTableEntry->endpoint));
  bufPtr += sizeof(uint8_t);

  // copy profile field
  sl_zigbee_af_copy_int16u(bufPtr, 0, (uint16_t)(translationTableEntry->zigbeeProfile));
  bufPtr += sizeof(uint16_t);

  // copy cluster field
  sl_zigbee_af_copy_int16u(bufPtr, 0, (uint16_t)(translationTableEntry->zigbeeCluster));
  bufPtr += sizeof(uint16_t);

  // copy Zigbee Command ID field
  sl_zigbee_af_copy_int8u(bufPtr, 0, (uint8_t)(translationTableEntry->zigbeeCommandId));
  bufPtr += sizeof(uint8_t);

  uint8_t zclPayloadLen = translationTableEntry->zclPayloadDefault[0];
  // copy Zigbee payload length ID field
  sl_zigbee_af_copy_int8u(bufPtr, 0, (uint8_t)(zclPayloadLen));
  bufPtr += sizeof(uint8_t);

  // copy Zigbee payload - first byte is a length and rest is a payload
  if (zclPayloadLen > 0 && zclPayloadLen < 0xFE) {
    memmove(bufPtr, &(translationTableEntry->zclPayloadDefault[1]), zclPayloadLen);
    bufPtr += zclPayloadLen;
  }

  if (sli_zigbee_gp_translation_table->TableEntry[entryIndex].infoBlockPresent == TRUE) {
    sl_zigbee_gp_translation_table_additional_info_block_field_t *additionalInfoTable = sli_zigbee_af_gp_get_additional_info_table();
    uint8_t additionalInfoOffset = sli_zigbee_gp_translation_table->TableEntry[entryIndex].additionalInfoOffset;
    sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *addInfo = &(additionalInfoTable->additionalInfoBlock[additionalInfoOffset]);
    charCount = sli_zigbee_af_copy_additional_info_block_structure_to_array(translationTableEntry->gpdCommand,
                                                                            addInfo,
                                                                            bufPtr);
    if (charCount) {
      bufPtr += charCount;
    }
  }
  charCount = (uint16_t)(bufPtr - tempDatabuffer);
  return charCount;
}

uint8_t sli_zigbee_af_gp_trans_table_find_matching_translation_table_entry(uint8_t levelOfScan,
                                                                           bool infoBlockPresent,
                                                                           sl_zigbee_gp_address_t *gpAddr,
                                                                           uint8_t gpdCommandId,
                                                                           uint8_t zbEndpoint,
                                                                           uint8_t *gpdCmdPayload,
                                                                           sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *addInfoBlock,
                                                                           uint8_t *outIndex,
                                                                           uint8_t startIndex)
{
  UNUSED_VAR(infoBlockPresent);
  uint8_t ret = GP_TRANSLATION_TABLE_STATUS_FAILED;
  uint8_t scanLevelMatchFound = 0xFF;
  sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t const *genericTranslationTable = NULL;  //this pointer either point to default table or customied table
  sli_zigbee_af_gp_command_translation_table *sli_zigbee_gp_translation_table = sli_zigbee_af_gp_trans_table_get_translation_table();

  if (!sli_zigbee_gp_translation_table->totalNoOfEntries) {
    return GP_TRANSLATION_TABLE_STATUS_EMPTY;
  }
  if (startIndex >= SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE) {
    return GP_TRANSLATION_TABLE_STATUS_FAILED;
  }
  for ( uint8_t tableIndex = startIndex; tableIndex < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE; tableIndex++) {
    scanLevelMatchFound = 0;
    if (sli_zigbee_gp_translation_table->TableEntry[tableIndex].entry == NO_ENTRY) {
      continue;
    }
    if (levelOfScan & GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID) {
      if (sl_zigbee_af_green_power_common_gp_addr_compare(&sli_zigbee_gp_translation_table->TableEntry[tableIndex].gpAddr, gpAddr)) {
        scanLevelMatchFound |= GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID;
      }
    }
    if (levelOfScan & GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_CMD_ID) {
      if (sli_zigbee_gp_translation_table->TableEntry[tableIndex].gpdCommand == gpdCommandId) {
        scanLevelMatchFound |= GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_CMD_ID;
      }
    }
    if (levelOfScan & GP_TRANSLATION_TABLE_SCAN_LEVEL_ZB_ENDPOINT) {
      if (sli_zigbee_gp_translation_table->TableEntry[tableIndex].entry == DEFAULT_TABLE_ENTRY) {
        sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t const *defaultTable = sli_zigbee_af_gp_get_default_table();
        if (defaultTable == NULL) {
          return GP_TRANSLATION_TABLE_STATUS_FAILED;
        }
        genericTranslationTable = &(defaultTable[sli_zigbee_gp_translation_table->TableEntry[tableIndex].offset]);
      } else if (sli_zigbee_gp_translation_table->TableEntry[tableIndex].entry == CUSTOMIZED_TABLE_ENTRY) {
        sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t *customizedTable = sli_zigbee_af_gp_get_customized_table();
        if (customizedTable == NULL) {
          return GP_TRANSLATION_TABLE_STATUS_FAILED;
        }
        genericTranslationTable = &(customizedTable[sli_zigbee_gp_translation_table->TableEntry[tableIndex].offset]);
      }
      if (genericTranslationTable == NULL) {
        return GP_TRANSLATION_TABLE_STATUS_FAILED;
      }
      if (genericTranslationTable->endpoint == SL_ZIGBEE_AF_GP_TRANSLATION_TABLE_ZB_ENDPOINT_PASS_FRAME_TO_APLLICATION) {
        scanLevelMatchFound |= GP_TRANSLATION_TABLE_SCAN_LEVEL_ZB_ENDPOINT;
      }
      if (sli_zigbee_gp_translation_table->TableEntry[tableIndex].zbEndpoint == zbEndpoint) {
        scanLevelMatchFound |= GP_TRANSLATION_TABLE_SCAN_LEVEL_ZB_ENDPOINT;
      }
    }
    if (sli_zigbee_gp_translation_table->TableEntry[tableIndex].infoBlockPresent == true) {
      uint8_t additionalInfoOffset = sli_zigbee_gp_translation_table->TableEntry[tableIndex].additionalInfoOffset;
      if (additionalInfoOffset == 0xFF) {
        continue;
      }
      sl_zigbee_gp_translation_table_additional_info_block_field_t *additionalInfoTable = sli_zigbee_af_gp_get_additional_info_table();
      sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *addInfo = &(additionalInfoTable->additionalInfoBlock[additionalInfoOffset]);

      if (levelOfScan & GP_TRANSLATION_TABLE_SCAN_LEVEL_ADDITIONAL_INFO_BLOCK) {
        if (compareTranslationTableAdditionalInfoBlockEntry(gpdCommandId, addInfo, addInfoBlock)) {
          scanLevelMatchFound |= GP_TRANSLATION_TABLE_SCAN_LEVEL_ADDITIONAL_INFO_BLOCK;
        }
      }
      if (levelOfScan & GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_PAYLOAD) {
        if (gpdCmdPayload == NULL) {
          continue;
        }
        if ((gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING)
            && (addInfo->optionData.compactAttr.reportIdentifier  == gpdCmdPayload[0])) {
          // end of choice1 and choice2
          scanLevelMatchFound |= GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_PAYLOAD;
        } else if ((gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS
                    || gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE)
                   && (addInfo->optionData.genericSwitch.contactStatus == (addInfo->optionData.genericSwitch.contactBitmask & gpdCmdPayload[0]))) {
          scanLevelMatchFound |= GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_PAYLOAD;
        }
      }
    }  //end of InfoBlockPresent
    if (levelOfScan == scanLevelMatchFound) {
      *outIndex = tableIndex;
      ret = GP_TRANSLATION_TABLE_STATUS_SUCCESS;
      break;
    } else {
      continue;
    }
  }
  return ret;
}

uint8_t sli_zigbee_af_gp_trans_table_delete_paired_devicefrom_translation_table_entry(sl_zigbee_gp_address_t *gpdAddr)
{
  sli_zigbee_af_gp_command_translation_table *sli_zigbee_gp_translation_table = sli_zigbee_af_gp_trans_table_get_translation_table();

  if (!sli_zigbee_gp_translation_table->totalNoOfEntries) {
    return GP_TRANSLATION_TABLE_STATUS_EMPTY;
  }
  uint8_t entryIndex;
  do {
    entryIndex = GP_TRANSLATION_TABLE_ENTRY_INVALID_INDEX;
    uint8_t status = sli_zigbee_af_gp_trans_table_find_matching_translation_table_entry(GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID,
                                                                                        false,
                                                                                        gpdAddr,
                                                                                        0x00,
                                                                                        0x00,
                                                                                        NULL,
                                                                                        NULL,
                                                                                        &entryIndex,
                                                                                        0);
    if (status == GP_TRANSLATION_TABLE_STATUS_SUCCESS
        && (entryIndex != 0xFF)) {
      sl_zigbee_gp_translation_table_additional_info_block_field_t *additionalInfoTable = sli_zigbee_af_gp_get_additional_info_table();
      uint8_t addInfoOffset = sli_zigbee_gp_translation_table->TableEntry[entryIndex].additionalInfoOffset;
      sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *addInfo = &(additionalInfoTable->additionalInfoBlock[addInfoOffset]);
      if (addInfoOffset != 0xFF) {
        status = deleteTanslationTableAdditionalInfoBlockEntry(sli_zigbee_gp_translation_table->TableEntry[entryIndex].gpdCommand,
                                                               addInfo);
        if (status != 0xFF) {
          sli_zigbee_gp_translation_table->TableEntry[entryIndex].additionalInfoOffset = 0xFF;
        }
      }
      if ((sli_zigbee_gp_translation_table->TableEntry[entryIndex].entry == CUSTOMIZED_TABLE_ENTRY) && (sli_zigbee_gp_translation_table->TableEntry[entryIndex].offset != 0xFF)) {
        if (!isCustomizedTableEntryReferenced(entryIndex, sli_zigbee_gp_translation_table->TableEntry[entryIndex].offset)) {
          deleteCustomizednTableEntry(sli_zigbee_gp_translation_table->TableEntry[entryIndex].offset);
        }
      }
      memset(&sli_zigbee_gp_translation_table->TableEntry[entryIndex], 0x00, sizeof(sli_zigbee_af_gp_command_translation_table_entry));
      sli_zigbee_gp_translation_table->TableEntry[entryIndex].entry = NO_ENTRY;
      sli_zigbee_gp_translation_table->TableEntry[entryIndex].offset = 0xFF;
      sli_zigbee_gp_translation_table->TableEntry[entryIndex].additionalInfoOffset = 0xFF;
      sli_zigbee_gp_translation_table->totalNoOfEntries--;
      sli_zigbee_af_gp_set_translation_table_entry(entryIndex);
    }
  } while (entryIndex != 0xFF);
  return GP_TRANSLATION_TABLE_STATUS_SUCCESS;
}

uint8_t sli_zigbee_af_gp_trans_table_get_translation_table_entry(uint8_t entryIndex,
                                                                 sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t *TranslationTableEntry)
{
  sli_zigbee_af_gp_command_translation_table const *sli_zigbee_gp_translation_table = sli_zigbee_af_gp_trans_table_get_translation_table();
  sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t const *genericTranslationTable = NULL;
  if (TranslationTableEntry == NULL || (entryIndex >= SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE)) {
    return GP_TRANSLATION_TABLE_STATUS_FAILED;
  }

  if (entryIndex != 0xFF ) {
    if (sli_zigbee_gp_translation_table->TableEntry[entryIndex].entry == NO_ENTRY) {
      return GP_TRANSLATION_TABLE_STATUS_FAILED;
    }
    memset(TranslationTableEntry, 0, sizeof(sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t));
    TranslationTableEntry->endpoint = sli_zigbee_gp_translation_table->TableEntry[entryIndex].zbEndpoint;
    TranslationTableEntry->gpdCommand = sli_zigbee_gp_translation_table->TableEntry[entryIndex].gpdCommand;
    if (sli_zigbee_gp_translation_table->TableEntry[entryIndex].entry == DEFAULT_TABLE_ENTRY) {
      sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t const *defaultTable = sli_zigbee_af_gp_get_default_table();
      if (defaultTable == NULL) {
        return SL_STATUS_FAIL;
      }
      genericTranslationTable = &(defaultTable[sli_zigbee_gp_translation_table->TableEntry[entryIndex].offset]);
    } else if (sli_zigbee_gp_translation_table->TableEntry[entryIndex].entry == CUSTOMIZED_TABLE_ENTRY) {
      sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t *customizedTable = sli_zigbee_af_gp_get_customized_table();
      if (customizedTable == NULL) {
        return SL_STATUS_FAIL;
      }
      genericTranslationTable = &(customizedTable[sli_zigbee_gp_translation_table->TableEntry[entryIndex].offset]);
    }
    if (genericTranslationTable != NULL) {
      TranslationTableEntry->validEntry = genericTranslationTable->validEntry;
      TranslationTableEntry->zigbeeProfile = genericTranslationTable->zigbeeProfile;
      TranslationTableEntry->zigbeeCluster = genericTranslationTable->zigbeeCluster;
      TranslationTableEntry->zigbeeCommandId = genericTranslationTable->zigbeeCommandId;
      memcpy(&(TranslationTableEntry->payloadSrc),
             &(genericTranslationTable->payloadSrc),
             sizeof(uint8_t));
      memcpy(&(TranslationTableEntry->zclPayloadDefault),
             &(genericTranslationTable->zclPayloadDefault),
             sizeof(SL_ZIGBEE_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN));
      return GP_TRANSLATION_TABLE_STATUS_SUCCESS;
    } else {
      return GP_TRANSLATION_TABLE_STATUS_FAILED;
    }
  }
  return GP_TRANSLATION_TABLE_STATUS_FAILED;
}

uint8_t sli_zigbee_af_gp_trans_table_add_translation_table_entry_update_command(uint8_t index,
                                                                                bool infoBlockPresent,
                                                                                sl_zigbee_gp_address_t *gpdAddr,
                                                                                uint8_t gpdCommandId,
                                                                                uint8_t zbEndpoint,
                                                                                uint16_t zigbeeProfile,
                                                                                uint16_t zigbeeCluster,
                                                                                uint8_t zigbeeCommandId,
                                                                                uint8_t payloadLength,
                                                                                uint8_t *payload,
                                                                                uint8_t payloadSrc,
                                                                                uint8_t additionalInfoLength,
                                                                                sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *additionalInfoBlock)
{
  bool ret = FALSE;
  sli_zigbee_af_gp_command_translation_table *sli_zigbee_gp_translation_table = sli_zigbee_af_gp_trans_table_get_translation_table();

  if (index == 0xFF) {
    //If index is 0xff, the sink SHALL choose any free entry
    uint8_t newTTEntryIndex;
    ret = addPairedDeviceToTranslationTable(TRANSLATION_TABLE_UPDATE,
                                            infoBlockPresent,
                                            gpdAddr,
                                            gpdCommandId,
                                            zbEndpoint,
                                            zigbeeProfile,
                                            zigbeeCluster,
                                            zigbeeCommandId,
                                            payloadLength,
                                            payload,
                                            payloadSrc,
                                            additionalInfoLength,
                                            additionalInfoBlock,
                                            &newTTEntryIndex);
  } else {
    if (index >= SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE) {
      return GP_TRANSLATION_TABLE_STATUS_FAILED;
    }
    if (sli_zigbee_gp_translation_table->TableEntry[index].entry != NO_ENTRY) {
      return GP_TRANSLATION_TABLE_STATUS_ENTRY_NOT_EMPTY;
    } else {
      ret = addTranslationTableEntryByIndex(TRANSLATION_TABLE_UPDATE,
                                            index,
                                            infoBlockPresent,
                                            gpdAddr,
                                            gpdCommandId,
                                            zbEndpoint,
                                            zigbeeProfile,
                                            zigbeeCluster,
                                            zigbeeCommandId,
                                            payloadLength,
                                            payload,
                                            payloadSrc,
                                            additionalInfoLength,
                                            additionalInfoBlock);
    }
  }
  return ret;
}

uint8_t sli_zigbee_af_gp_trans_table_replace_translation_table_entry_update_command(uint8_t index,
                                                                                    bool infoBlockPresent,
                                                                                    sl_zigbee_gp_address_t *gpdAddr,
                                                                                    uint8_t gpdCommandId,
                                                                                    uint8_t zbEndpoint,
                                                                                    uint16_t zigbeeProfile,
                                                                                    uint16_t zigbeeCluster,
                                                                                    uint8_t zigbeeCommandId,
                                                                                    uint8_t payloadLength,
                                                                                    uint8_t *payload,
                                                                                    uint8_t payloadSrc,
                                                                                    uint8_t additionalInfoLength,
                                                                                    sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *additionalInfoBlock)
{
  uint8_t ret = GP_TRANSLATION_TABLE_STATUS_FAILED;
  bool found = FALSE;
  int tableIndex = 0;
  sli_zigbee_af_gp_command_translation_table *sli_zigbee_gp_translation_table = sli_zigbee_af_gp_trans_table_get_translation_table();
  if (index != 0xFF ) {
    if (index >= SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE) {
      return GP_TRANSLATION_TABLE_STATUS_FAILED;
    }
    if (sli_zigbee_gp_translation_table->TableEntry[index].entry == NO_ENTRY) {
      return GP_TRANSLATION_TABLE_STATUS_ENTRY_EMPTY;
    }
    ret = addTranslationTableEntryByIndex(TRANSLATION_TABLE_UPDATE,
                                          index,
                                          infoBlockPresent,
                                          gpdAddr,
                                          gpdCommandId,
                                          zbEndpoint,
                                          zigbeeProfile,
                                          zigbeeCluster,
                                          zigbeeCommandId,
                                          payloadLength,
                                          payload,
                                          payloadSrc,
                                          additionalInfoLength,
                                          additionalInfoBlock);
  } else {
    const sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t *genericTranslationTable = NULL;
    /*If index is 0xff, the sink replacing any number of translation entry(s) for the same
       (GPD ID, GPD Endpoint, GPD CommandID, EndPoint, Profile, Cluster) quintuple by the supplied number of entries*/

    for (tableIndex = 0; tableIndex < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE; tableIndex++) {
      if (!sli_zigbee_gp_translation_table->TableEntry[tableIndex].entry) {
        continue;
      }
      if (sli_zigbee_gp_translation_table->TableEntry[tableIndex].entry == CUSTOMIZED_TABLE_ENTRY) {
        genericTranslationTable = sli_zigbee_af_gp_get_customized_table();
      } else if (sli_zigbee_gp_translation_table->TableEntry[tableIndex].entry == DEFAULT_TABLE_ENTRY) {
        genericTranslationTable = sli_zigbee_af_gp_get_default_table();
      }
      if (genericTranslationTable == NULL) {
        return GP_TRANSLATION_TABLE_STATUS_FAILED;
      }
      genericTranslationTable = &genericTranslationTable[sli_zigbee_gp_translation_table->TableEntry[tableIndex].offset];
      if ((sl_zigbee_af_green_power_common_gp_addr_compare(&sli_zigbee_gp_translation_table->TableEntry[tableIndex].gpAddr, gpdAddr)
           && (uint8_t)sli_zigbee_gp_translation_table->TableEntry[tableIndex].gpdCommand == (uint8_t)gpdCommandId)
          && (sli_zigbee_gp_translation_table->TableEntry[tableIndex].zbEndpoint == zbEndpoint)
          && (genericTranslationTable->zigbeeProfile == zigbeeProfile)
          && (genericTranslationTable->zigbeeCluster == zigbeeCluster)) {
        ret = addTranslationTableEntryByIndex(TRANSLATION_TABLE_UPDATE,
                                              tableIndex,
                                              infoBlockPresent,
                                              gpdAddr,
                                              gpdCommandId,
                                              zbEndpoint,
                                              zigbeeProfile,
                                              zigbeeCluster,
                                              zigbeeCommandId,
                                              payloadLength,
                                              payload,
                                              payloadSrc,
                                              additionalInfoLength,
                                              additionalInfoBlock);
        if (ret == GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
          found = TRUE;
        }
      }
    }
    if (!found) { //If Cannot replace, add new entry to the table
      for (tableIndex = 0; tableIndex < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE; tableIndex++) {
        if (sli_zigbee_gp_translation_table->TableEntry[tableIndex].entry) {
          continue;
        }
        ret = addTranslationTableEntryByIndex(TRANSLATION_TABLE_UPDATE,
                                              tableIndex,
                                              infoBlockPresent,
                                              gpdAddr,
                                              gpdCommandId,
                                              zbEndpoint,
                                              zigbeeProfile,
                                              zigbeeCluster,
                                              zigbeeCommandId,
                                              payloadLength,
                                              payload,
                                              payloadSrc,
                                              additionalInfoLength,
                                              additionalInfoBlock);
        break;
      }
    }
  }
  return ret;
}

static uint8_t removeTranslationTableEntryUpdateCommand(uint8_t index,
                                                        bool infoBlockPresent,
                                                        sl_zigbee_gp_address_t *gpdAddr,
                                                        uint8_t gpdCommandId,
                                                        uint8_t zbEndpoint,
                                                        uint16_t zigbeeProfile,
                                                        uint16_t zigbeeCluster,
                                                        uint8_t zigbeeCommandId,
                                                        uint8_t payloadLength,
                                                        uint8_t *payload,
                                                        uint8_t payloadSrc,
                                                        uint8_t additionalInfoLength,
                                                        sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *additionalInfoBlock)
{
  UNUSED_VAR(payloadLength);
  UNUSED_VAR(payload);
  UNUSED_VAR(payloadSrc);
  UNUSED_VAR(additionalInfoLength);
  UNUSED_VAR(additionalInfoBlock);
  uint8_t ret = GP_TRANSLATION_TABLE_STATUS_FAILED;
  int tableIndex = 0;
  sli_zigbee_af_gp_command_translation_table *sli_zigbee_gp_translation_table = sli_zigbee_af_gp_trans_table_get_translation_table();

  if (index != 0xFF ) {
    if (index >= SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE) {
      return GP_TRANSLATION_TABLE_STATUS_FAILED;
    }
    if (sli_zigbee_gp_translation_table->TableEntry[index].entry == NO_ENTRY) {
      return GP_TRANSLATION_TABLE_STATUS_ENTRY_EMPTY;
    }
    ret = deleteTranslationTableEntryByIndex(index,
                                             infoBlockPresent,
                                             gpdCommandId,
                                             zigbeeProfile,
                                             zigbeeCluster,
                                             zigbeeCommandId,
                                             0x00,
                                             NULL,
                                             0x00,
                                             NULL);
  } else {
    const sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t *genericTranslationTable = NULL;
    // If index is 0xff, the sink removes any number of translation entry(s) for the same
    // (GPD ID, GPD Endpoint, GPD CommandID, EndPoint, Profile, Cluster) quintuple by the supplied number of entries
    for (tableIndex = 0; tableIndex < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE; tableIndex++) {
      if (sli_zigbee_gp_translation_table->TableEntry[tableIndex].entry == NO_ENTRY ) {
        continue;
      }
      if (sli_zigbee_gp_translation_table->TableEntry[tableIndex].entry == CUSTOMIZED_TABLE_ENTRY) {
        genericTranslationTable = sli_zigbee_af_gp_get_customized_table();
      } else if (sli_zigbee_gp_translation_table->TableEntry[tableIndex].entry == DEFAULT_TABLE_ENTRY) {
        genericTranslationTable = sli_zigbee_af_gp_get_default_table();
      }
      if (genericTranslationTable == NULL) {
        return GP_TRANSLATION_TABLE_STATUS_FAILED;
      }
      genericTranslationTable = &genericTranslationTable[sli_zigbee_gp_translation_table->TableEntry[tableIndex].offset];
      if (sl_zigbee_af_green_power_common_gp_addr_compare(&sli_zigbee_gp_translation_table->TableEntry[tableIndex].gpAddr, gpdAddr)
          && (sli_zigbee_gp_translation_table->TableEntry[tableIndex].gpdCommand == gpdCommandId)
          && (sli_zigbee_gp_translation_table->TableEntry[tableIndex].zbEndpoint == zbEndpoint)
          && (genericTranslationTable->zigbeeProfile == zigbeeProfile)
          && (genericTranslationTable->zigbeeCluster == zigbeeCluster)) {
        ret = deleteTranslationTableEntryByIndex(tableIndex,
                                                 infoBlockPresent,
                                                 gpdCommandId,
                                                 zigbeeProfile,
                                                 zigbeeCluster,
                                                 zigbeeCommandId,
                                                 0x00,
                                                 NULL,
                                                 0x00,
                                                 NULL);
      }
    }
  }
  return ret;
}

void sli_zigbee_af_gp_remove_gpd_endpoint_from_translation_table(sl_zigbee_gp_address_t *gpdAddr, uint8_t zbEndpoint)
{
  uint8_t outIndex = 0xFF;
  uint8_t startIndex = 0x00;
  sli_zigbee_af_gp_command_translation_table *translationTable = sli_zigbee_af_gp_trans_table_get_translation_table();
  sl_zigbee_af_green_power_cluster_println("removeGpdEndpointFromTranslationTable zbEndpoint %d ", zbEndpoint);

  // by default : remove one particular GPD from this combo edp
  uint8_t levelOfScanMask = (GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID | GP_TRANSLATION_TABLE_SCAN_LEVEL_ZB_ENDPOINT);
  if ((gpdAddr == NULL)
      || ((gpdAddr->applicationId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID
           && gpdAddr->id.sourceId == GP_ADDR_SRC_ID_WILDCARD)
          && ((gpdAddr->applicationId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS)
              && (sl_zigbee_af_memory_byte_compare(gpdAddr->id.gpdIeeeAddress, EUI64_SIZE, 0xFF))))) {
    // WildcardId thus remove AllGpd from this combo edp
    levelOfScanMask = GP_TRANSLATION_TABLE_SCAN_LEVEL_ZB_ENDPOINT;
  }

  do {
    outIndex = 0xFF;
    uint8_t status = sli_zigbee_af_gp_trans_table_find_matching_translation_table_entry(levelOfScanMask,
                                                                                        false,//bool infoBlockPresent,
                                                                                        gpdAddr,
                                                                                        0,
                                                                                        zbEndpoint,//uint8_t zbEndpoint,
                                                                                        0,
                                                                                        NULL,
                                                                                        &outIndex,
                                                                                        startIndex);
    if (status == GP_TRANSLATION_TABLE_STATUS_SUCCESS
        && outIndex != 0xFF) {
      deleteTranslationTableEntryByIndex(outIndex,
                                         true,
                                         translationTable->TableEntry[outIndex].gpdCommand,
                                         0,
                                         0,
                                         0,
                                         0x00,
                                         NULL,
                                         0x00,
                                         NULL);
      startIndex = outIndex + 1;
    }
  } while (outIndex != 0xFF);
}

static uint8_t findTTEntriesByStartIndex(sl_zigbee_gp_address_t *addr,
                                         uint8_t gpdCommandId,
                                         uint8_t *gpdCommandPayload,
                                         uint8_t *outIndex,
                                         uint8_t startIndex)
{
  uint8_t status = GP_TRANSLATION_TABLE_STATUS_FAILED;
  if (gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS
      || gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE
      || gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING) {
    uint8_t *payload = NULL;
    // In incomming GPDF has following interpretation for the command payload
    // gpdCommandPayload [0] = length (has special meaning 0xFF)
    // gpdCommandPayload [1] = data start
    if (gpdCommandPayload != NULL
        && gpdCommandPayload[0]
        && gpdCommandPayload[0] != GREEN_POWER_SERVER_GPD_COMMAND_PAYLOAD_UNSPECIFIED_LENGTH
        && gpdCommandPayload[0] != GREEN_POWER_SERVER_GPD_COMMAND_PAYLOAD_TT_DERIVED_LENGTH) {
      payload = &gpdCommandPayload[1];
    }
    // this is special case as there may be multiple configuration for the same
    // switch and command - hence the payload which is the currentStaus of contact
    // need to match as well.
    status = sli_zigbee_af_gp_trans_table_find_matching_translation_table_entry((GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID
                                                                                 | GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_CMD_ID
                                                                                 | GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_PAYLOAD),
                                                                                true,
                                                                                addr,
                                                                                gpdCommandId,
                                                                                0,
                                                                                payload,
                                                                                NULL,
                                                                                outIndex,
                                                                                startIndex);
  } else {
    status = sli_zigbee_af_gp_trans_table_find_matching_translation_table_entry((GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID
                                                                                 | GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_CMD_ID),
                                                                                false,
                                                                                addr,
                                                                                gpdCommandId,
                                                                                0,
                                                                                NULL,
                                                                                NULL,
                                                                                outIndex,
                                                                                startIndex);
    if (status != GP_TRANSLATION_TABLE_STATUS_SUCCESS
        || *outIndex == GP_TRANSLATION_TABLE_ENTRY_INVALID_INDEX) {
      if (gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_ATTRIBUTE_REPORTING
          || gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_MFR_SP_ATTR_RPTG
          || gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_MULTI_CLUSTER_RPTG
          || gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_MFR_SP_MULTI_CLUSTER_RPTG) {
        status = sli_zigbee_af_gp_trans_table_find_matching_translation_table_entry((GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID
                                                                                     | GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_CMD_ID),
                                                                                    false,
                                                                                    addr,
                                                                                    SL_ZIGBEE_ZCL_GP_GPDF_ANY_GPD_SENSOR_CMD,
                                                                                    0,
                                                                                    NULL,
                                                                                    NULL,
                                                                                    outIndex,
                                                                                    startIndex);
      }
    }
  }
  return status;
}

static sl_status_t filloutCommandAndForward(sl_zigbee_gp_address_t *addr,
                                            uint8_t gpdCommandId,
                                            uint8_t *gpdCommandPayload,
                                            sli_zigbee_af_gp_command_translation_table_entry *tableEntry)
{
  if (tableEntry->entry != NO_ENTRY) {
    sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t const *genericTranslationTable = NULL;
    if (tableEntry->entry == DEFAULT_TABLE_ENTRY) {
      sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t const *defaultTable = sli_zigbee_af_gp_get_default_table();
      genericTranslationTable = &(defaultTable[tableEntry->offset]);
      if (defaultTable == NULL) {
        return SL_STATUS_FAIL;
      }
    } else if (tableEntry->entry == CUSTOMIZED_TABLE_ENTRY) {
      sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t *customizedTable = sli_zigbee_af_gp_get_customized_table();
      if (customizedTable == NULL) {
        return SL_STATUS_FAIL;
      }
      genericTranslationTable = &(customizedTable[tableEntry->offset]);
    }
    sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *addInfo = NULL;
    if (tableEntry->infoBlockPresent) {
      sl_zigbee_gp_translation_table_additional_info_block_field_t *additionalInfoTable = sli_zigbee_af_gp_get_additional_info_table();
      addInfo = &(additionalInfoTable->additionalInfoBlock[tableEntry->additionalInfoOffset]);
      if (addInfo == NULL) {
        return SL_STATUS_FAIL;
      }
    }
    // Pass the frame to Application
    if (genericTranslationTable->endpoint == SL_ZIGBEE_AF_GP_TRANSLATION_TABLE_ZB_ENDPOINT_PASS_FRAME_TO_APLLICATION) {
      sl_zigbee_af_green_power_cluster_pass_frame_without_translation_cb(addr,
                                                                         gpdCommandId,
                                                                         gpdCommandPayload);
      return SL_STATUS_OK;
    }

    if (gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING) {
      if (addInfo == NULL) {
        return SL_STATUS_FAIL;
      }
      uint8_t dataSize = sl_zigbee_af_get_data_size(addInfo->optionData.compactAttr.attributeDataType);
      (void) sl_zigbee_af_fill_external_buffer((ZCL_GLOBAL_COMMAND                                                              \
                                                | ((addInfo->optionData.compactAttr.attributeOptions & 0x01)                    \
                                                   ? ZCL_FRAME_CONTROL_SERVER_TO_CLIENT : ZCL_FRAME_CONTROL_CLIENT_TO_SERVER)), \
                                               addInfo->optionData.compactAttr.clusterID,                                       \
                                               genericTranslationTable->zigbeeCommandId,                                        \
                                               "");
      sl_zigbee_af_append_to_external_buffer((uint8_t *)(&addInfo->optionData.compactAttr.attributeID), 2);
      sl_zigbee_af_append_to_external_buffer((uint8_t *)(&addInfo->optionData.compactAttr.attributeDataType), 1);
      sl_zigbee_af_append_to_external_buffer((uint8_t *)(gpdCommandPayload       \
                                                         + CAR_DATA_POINT_OFFSET \
                                                         + addInfo->optionData.compactAttr.attrOffsetWithinReport), dataSize);
      sl_zigbee_af_set_command_endpoints(tableEntry->zbEndpoint,
                                         tableEntry->zbEndpoint);
      return sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, sl_zigbee_af_get_node_id());
    } else {
      return sli_zigbee_af_gp_forward_gpd_to_mapped_endpoint(addr,
                                                             gpdCommandId,
                                                             gpdCommandPayload,
                                                             genericTranslationTable,
                                                             tableEntry->zbEndpoint);
    }
  }
  return SL_STATUS_FAIL;
}

void sli_zigbee_af_gp_forward_gpd_command_based_on_translation_table(sl_zigbee_gp_address_t *addr,
                                                                     uint8_t gpdCommandId,
                                                                     uint8_t *gpdCommandPayload)
{
  // Get translation table head
  sli_zigbee_af_gp_command_translation_table *translationTable = sli_zigbee_af_gp_trans_table_get_translation_table();
  if (translationTable == NULL) {
    // Error or No TT suported
    return;
  }
  // Loop through the TT entries and forward the commands to the endpoints that are commissionined
  uint8_t startIndex = 0;
  while (true) {
    uint8_t outIndex = GP_TRANSLATION_TABLE_ENTRY_INVALID_INDEX;
    uint8_t status = GP_TRANSLATION_TABLE_STATUS_FAILED;
    status = findTTEntriesByStartIndex(addr,
                                       gpdCommandId,
                                       gpdCommandPayload,
                                       &outIndex,
                                       startIndex);
    if (status != GP_TRANSLATION_TABLE_STATUS_SUCCESS
        || outIndex == GP_TRANSLATION_TABLE_ENTRY_INVALID_INDEX) {
      // Error or end of TT
      return;
    }
    filloutCommandAndForward(addr,
                             gpdCommandId,
                             gpdCommandPayload,
                             &translationTable->TableEntry[outIndex]);
    startIndex = outIndex + 1;
  }
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_green_power_cluster_gp_translation_table_update_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_green_power_cluster_gp_translation_table_update_command_t cmd_data;
  uint8_t retval = 0;
  uint16_t payloadOffset = 0;
  uint8_t gpApplicationId;
  sl_zigbee_gp_address_t gpdAddr;
  uint8_t action;
  uint8_t noOfTranslations;
  bool additionalInfoBlockPresent;
  uint8_t index = 0;
  uint8_t gpdCommandId = 0;
  uint8_t zbEndpoint = 0;
  uint16_t zigbeeProfile;
  uint16_t zigbeeCluster = 0;
  uint8_t zigbeeCommandId = 0;
  uint8_t payloadLength = 0;
  uint8_t payload[SL_ZIGBEE_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN] = { 0 };
  uint8_t payloadSrc = 0;
  uint8_t additionalInfoLength = 0;
  uint8_t *additionalInfoBlockIn = NULL;
  uint8_t *translationsEntryPtr;
  sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t additionalInfoBlockOut;

  if (zcl_decode_green_power_cluster_gp_translation_table_update_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  gpApplicationId = (cmd_data.options & 0x0007);
  if (!sli_zigbee_af_gp_make_addr(&gpdAddr, gpApplicationId, cmd_data.gpdSrcId, cmd_data.gpdIeee, cmd_data.endpoint)) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }
  action = ((cmd_data.options >> 3) & 0x0003); //3..4bits
  noOfTranslations = ((cmd_data.options >> 5) & 0x0003); //5..7 bits
  additionalInfoBlockPresent = ((cmd_data.options >> 8) & 0x0001); //8 bits
  translationsEntryPtr = cmd_data.translations;

  for (uint8_t i = 0; i < noOfTranslations; i++) {
    index = (uint8_t)sl_zigbee_af_get_int8u(translationsEntryPtr, payloadOffset, payloadOffset + 1); //1byte
    payloadOffset += 1u;
    gpdCommandId = (uint8_t)sl_zigbee_af_get_int8u(translationsEntryPtr, payloadOffset, (payloadOffset + 1)); //1byte
    payloadOffset += 1u;
    zbEndpoint = (uint8_t)sl_zigbee_af_get_int8u(translationsEntryPtr, payloadOffset, (payloadOffset + 1)); //1byte
    payloadOffset += 1u;
    zigbeeProfile = (uint16_t)sl_zigbee_af_get_int16u(translationsEntryPtr, payloadOffset, (payloadOffset + 2)); //2byte
    payloadOffset += 2u;
    zigbeeCluster = (uint16_t)sl_zigbee_af_get_int16u(translationsEntryPtr, payloadOffset, (payloadOffset + 2)); //2byte
    payloadOffset += 2u;
    zigbeeCommandId = (uint8_t)sl_zigbee_af_get_int8u(translationsEntryPtr, payloadOffset, (payloadOffset + 1)); //1byte
    payloadOffset += 1u;
    payloadLength = (uint8_t)sl_zigbee_af_get_int8u(translationsEntryPtr, payloadOffset, (payloadOffset + 1)); //1byte
    payloadOffset += 1u;
    if (payloadLength > 0) {
      //Do not copy the payload when payloadLength is 0xFF 0r 0xFE
      if ( payloadLength <= SL_ZIGBEE_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN) {
        memcpy(payload, &translationsEntryPtr[payloadOffset], payloadLength);
        payloadOffset += payloadLength;
      }
    }
    if (additionalInfoBlockPresent) {
      additionalInfoBlockIn = &translationsEntryPtr[payloadOffset];
      retval = sli_zigbee_af_gp_copy_additional_info_block_array_to_structure(additionalInfoBlockIn, &additionalInfoBlockOut, gpdCommandId);
      additionalInfoLength = retval;
      payloadOffset += retval;
    }

    index = 0xFF; //In the current version of the specification, the Index field SHALL always be set to 0xff
                  //upon transmis-sion and SHALL always be ignored on reception
    if ( action == 0x00) {
      retval = sli_zigbee_af_gp_trans_table_add_translation_table_entry_update_command(index,
                                                                                       additionalInfoBlockPresent,
                                                                                       &gpdAddr,
                                                                                       gpdCommandId,
                                                                                       zbEndpoint,
                                                                                       zigbeeProfile,
                                                                                       zigbeeCluster,
                                                                                       zigbeeCommandId,
                                                                                       payloadLength,
                                                                                       payload,
                                                                                       payloadSrc,
                                                                                       additionalInfoLength,
                                                                                       &additionalInfoBlockOut);
      if (retval != GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
        if ( retval == GP_TRANSLATION_TABLE_STATUS_ENTRY_NOT_EMPTY ) {
          sl_zigbee_af_green_power_cluster_println("Entry @Index [%d] is not empty", index);
        } else if ( retval == GP_TRANSLATION_TABLE_STATUS_PARAM_DOES_NOT_MATCH) {
          sl_zigbee_af_green_power_cluster_println("Parameter does not match @Index [%d]", index);
          sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_FAILURE); //send failure notification immediately
        }
      }
    } else if (action == 0x01) {
      retval = sli_zigbee_af_gp_trans_table_replace_translation_table_entry_update_command(index,
                                                                                           additionalInfoBlockPresent,
                                                                                           &gpdAddr,
                                                                                           gpdCommandId,
                                                                                           zbEndpoint,
                                                                                           zigbeeProfile,
                                                                                           zigbeeCluster,
                                                                                           zigbeeCommandId,
                                                                                           payloadLength,
                                                                                           payload,
                                                                                           payloadSrc,
                                                                                           additionalInfoLength,
                                                                                           &additionalInfoBlockOut);
      if (retval != GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
        if ( retval == GP_TRANSLATION_TABLE_STATUS_ENTRY_EMPTY) {
          sl_zigbee_af_green_power_cluster_println("Entry @Index [%d] is empty", index);
        } else if (retval == GP_TRANSLATION_TABLE_STATUS_PARAM_DOES_NOT_MATCH) {
          sl_zigbee_af_green_power_cluster_println("Parameter does not match @Index [%d]", index);
          sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_FAILURE);
        }
      }
    } else if (action == 0x02) {
      retval = removeTranslationTableEntryUpdateCommand(index,
                                                        additionalInfoBlockPresent,
                                                        &gpdAddr,
                                                        gpdCommandId,
                                                        zbEndpoint,
                                                        zigbeeProfile,
                                                        zigbeeCluster,
                                                        zigbeeCommandId,
                                                        payloadLength,
                                                        payload,
                                                        payloadSrc,
                                                        additionalInfoLength,
                                                        &additionalInfoBlockOut);
      if (retval != GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
        if (retval == GP_TRANSLATION_TABLE_STATUS_ENTRY_EMPTY) {
          sl_zigbee_af_green_power_cluster_println("Entry @Index [%d] is empty", index);
        } else if (retval == GP_TRANSLATION_TABLE_STATUS_PARAM_DOES_NOT_MATCH) {
          sl_zigbee_af_green_power_cluster_println("Parameter does not match @Index [%d]", index);
          sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_FAILURE);
        }
      }
    } else {
      return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
    }
    translationsEntryPtr += payloadOffset; //If successful move the pointer to the next translation
    payloadOffset = 0;
  }
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_green_power_cluster_gp_translation_table_request_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_green_power_cluster_gp_translation_table_request_command_t cmd_data;
  uint8_t retval;
  sli_zigbee_af_gp_command_translation_table *sli_zigbee_gp_translation_table = sli_zigbee_af_gp_trans_table_get_translation_table();
  uint8_t entryIndex = 0;
  uint8_t options = 0;

  if (zcl_decode_green_power_cluster_gp_translation_table_request_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_green_power_cluster_println("Got translation table request with index %02X sli_zigbee_gp_translation_table->totalNoOfEntries = %d",
                                           cmd_data.startIndex,
                                           sli_zigbee_gp_translation_table->totalNoOfEntries);
  // only respond to unicast messages.
  if (sl_zigbee_af_current_command()->type != SL_ZIGBEE_INCOMING_UNICAST) {
    sl_zigbee_af_green_power_cluster_println("Not unicast");
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  // the device SHALL check if it implements a Translation Table.
  if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE == 0) {
    sl_zigbee_af_green_power_cluster_println("Unsup cluster command");
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  if (sl_zigbee_af_current_endpoint() != SL_ZIGBEE_GP_ENDPOINT) {
    sl_zigbee_af_green_power_cluster_println("Drop frame due to unknown endpoint: %02X", sl_zigbee_af_current_endpoint());
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  if ((sli_zigbee_gp_translation_table->totalNoOfEntries == 0)
      || (cmd_data.startIndex >= SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE)) {
    // "index" is already 0xFF if search by ID
    // or already set to the value from "SinkTableRequest" triggered frame in case it is search by INDEX
    sl_zigbee_af_fill_command_green_power_cluster_gp_translation_table_response_smart(SL_ZIGBEE_ZCL_GP_TRANSLATION_TABLE_RESPONSE_STATUS_NOT_FOUND,
                                                                                      0x00, //options
                                                                                      sli_zigbee_gp_translation_table->totalNoOfEntries, //totalNoOfEntries
                                                                                      0x00, //startIndex
                                                                                      0x00, //entryCount
                                                                                      NULL,
                                                                                      0);
    sli_zigbee_af_gp_send_response_unicast();
    goto kickout;
  } else {
    for (entryIndex = cmd_data.startIndex; entryIndex < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE; entryIndex++) {
      if (!sli_zigbee_gp_translation_table->TableEntry[entryIndex].entry) {
        continue;
      } else {
        break;
      }
    }
    if (entryIndex >= SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE) {
      sl_zigbee_af_fill_command_green_power_cluster_gp_translation_table_response_smart(SL_ZIGBEE_ZCL_GP_TRANSLATION_TABLE_RESPONSE_STATUS_NOT_FOUND,
                                                                                        0x00, //options
                                                                                        sli_zigbee_gp_translation_table->totalNoOfEntries, //totalNoOfEntries
                                                                                        0x00, //startIndex
                                                                                        0x00, //entryCount
                                                                                        NULL,
                                                                                        0);
      sli_zigbee_af_gp_send_response_unicast();
      goto kickout;
    }
    uint16_t entriesCount = 0;
    uint8_t tempDatabuffer[SL_ZIGBEE_AF_RESPONSE_BUFFER_LEN];
    uint16_t tempDataLength = 0;
    uint8_t entryCountOffset = (appResponseLength + 7);
    sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t TranslationTableEntry = { 0 };

    for (entryIndex = cmd_data.startIndex; entryIndex < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE; entryIndex++) {
      if (!sli_zigbee_gp_translation_table->TableEntry[entryIndex].entry) {
        continue;
      }
      sl_zigbee_af_green_power_cluster_println("Send the response with translation table entries -- entryIndex = %d", entryIndex);
      retval  = sli_zigbee_af_gp_trans_table_get_translation_table_entry(entryIndex, &TranslationTableEntry);
      if (retval == GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
        if (!entriesCount) {
          //Fill the options with first translation entry found after startIndex
          options = (sli_zigbee_gp_translation_table->TableEntry[entryIndex].infoBlockPresent << 0x03 | sli_zigbee_gp_translation_table->TableEntry[entryIndex].gpAddr.applicationId);
        }
        if ( options == ((sli_zigbee_gp_translation_table->TableEntry[entryIndex].infoBlockPresent << 0x03)
                         | (sli_zigbee_gp_translation_table->TableEntry[entryIndex].gpAddr.applicationId))) {
          // Copy to a temp buffer and add if there is space
          tempDataLength = copyTranslationTableEntryToBuffer(entryIndex, &TranslationTableEntry, tempDatabuffer, SL_ZIGBEE_AF_RESPONSE_BUFFER_LEN);
          // If space add to buffer
          if ((sizeof(appResponseData) - (entryCountOffset + 1)) >= (appResponseLength + tempDataLength)) {
            if (!entriesCount) {
              sl_zigbee_af_fill_command_green_power_cluster_gp_translation_table_response_smart(SL_ZIGBEE_ZCL_GP_SINK_TABLE_RESPONSE_STATUS_SUCCESS,
                                                                                                options, //options
                                                                                                sli_zigbee_gp_translation_table->totalNoOfEntries, //totalNoOfEntries
                                                                                                cmd_data.startIndex, //startIndex
                                                                                                0x00, //entryCount
                                                                                                NULL,
                                                                                                0);
            }
            memmove(&appResponseData[appResponseLength], tempDatabuffer, tempDataLength);
            appResponseLength +=  tempDataLength;
            entriesCount++;
          } else {
            break;
          }
        }
      }
    }
    if (entriesCount == 0) {
      sl_zigbee_af_green_power_cluster_println("INSUFFICIENT SPACE"); //send Insufficient space message if not even one entry could fit in the response
      sl_zigbee_af_fill_command_green_power_cluster_gp_translation_table_response_smart(SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE,
                                                                                        0x00, //options
                                                                                        sli_zigbee_gp_translation_table->totalNoOfEntries, //totalNoOfEntries
                                                                                        0x00, //startIndex
                                                                                        0x00, //entryCount
                                                                                        NULL,
                                                                                        0);
      sli_zigbee_af_gp_send_response_unicast();
      goto kickout;
    } else {
      //Insert the number of entries actually included @ entryCountOffset
      appResponseData[entryCountOffset] = entriesCount;
      sl_status_t status = sli_zigbee_af_gp_send_response_unicast();
      if (status == SL_STATUS_MESSAGE_TOO_LONG) {
        sl_zigbee_af_fill_command_green_power_cluster_gp_translation_table_response_smart(SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE,
                                                                                          0x00, //options
                                                                                          sli_zigbee_gp_translation_table->totalNoOfEntries, //totalNoOfEntries
                                                                                          0x00, //startIndex
                                                                                          0x00, //entryCount
                                                                                          NULL,
                                                                                          0);
        sli_zigbee_af_gp_send_response_unicast();
      }
    }
  }           //end of else
  kickout: return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

uint8_t sli_zigbee_af_gp_find_matching_generic_translation_table_entry(uint8_t entryType,
                                                                       uint8_t incomingReqType,
                                                                       uint8_t offset,
                                                                       bool infoBlockPresent,
                                                                       uint8_t gpdCommandId,
                                                                       uint16_t zigbeeProfile,
                                                                       uint16_t zigbeeCluster,
                                                                       uint8_t zigbeeCommandId,
                                                                       uint8_t payloadLength,
                                                                       uint8_t *payload,
                                                                       uint8_t *outIndex)
{
  UNUSED_VAR(infoBlockPresent);
  UNUSED_VAR(payload);
  uint8_t tableSize = 0xFF;
  sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t const *genericTable = NULL;
  sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t const *genericTableEntry = NULL;

  if (entryType == CUSTOMIZED_TABLE_ENTRY) {
    genericTable = sli_zigbee_af_gp_get_customized_table();
    tableSize = SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE;
  } else if (entryType == DEFAULT_TABLE_ENTRY) {
    genericTable = sli_zigbee_af_gp_get_default_table();
    tableSize = sli_zigbee_af_gp_default_table_size;
  }

  if (genericTable == NULL || tableSize == 0xFF) {
    return GP_TRANSLATION_TABLE_STATUS_FAILED;
  }
  if (offset == 0xFF) {
    //If the offset is 0xFF then search entire Customized Table to get the matching entry
    for (uint8_t index = 0; (index < tableSize); index++) {
      genericTableEntry = &(genericTable[index]);
      if (genericTableEntry->validEntry != true) {
        continue;
      }
      if ((genericTableEntry->gpdCommand == gpdCommandId)
          && ((incomingReqType == ADD_PAIRED_DEVICE)
              || ((incomingReqType == TRANSLATION_TABLE_UPDATE)
                  && (genericTableEntry->zigbeeProfile == zigbeeProfile)
                  && (genericTableEntry->zigbeeCluster == zigbeeCluster)
                  && (genericTableEntry->zigbeeCommandId == zigbeeCommandId)
                  && (genericTableEntry->zclPayloadDefault[0] == payloadLength)))) {
        *outIndex = index;
        break;
      }
    }           // end of for
  } else if (offset < tableSize) {
    genericTableEntry = &(genericTable[offset]);
    //If the offset is not 0xFF then look for matching entry in customized Table[offset]
    if (genericTableEntry->validEntry == true) {
      if ((genericTableEntry->gpdCommand == gpdCommandId)
          && ((incomingReqType == ADD_PAIRED_DEVICE)
              || ((incomingReqType == TRANSLATION_TABLE_UPDATE)
                  && (genericTableEntry->zigbeeProfile == zigbeeProfile)
                  && (genericTableEntry->zigbeeCluster == zigbeeCluster)))) {
        *outIndex = offset;
      }
    }
  }

  if (*outIndex != 0xFF) {
    return GP_TRANSLATION_TABLE_STATUS_SUCCESS;
  }
  return GP_TRANSLATION_TABLE_STATUS_FAILED;
}

void sl_zigbee_af_table_init_cb(void)
{
#ifndef USER_HAS_DEFAULT_TRANSLATION_TABLE
  // Get user/system defined default table
  sli_zigbee_af_gp_default_table_ptr  = getSystemDefinedDefaultTable(&sli_zigbee_af_gp_default_table_size);
#endif //USER_HAS_DEFAULT_TRANSLATION_TABLE
#ifndef USER_HAS_DEFAULT_GENERIC_SWITCH_TRANSLATION_TABLE
  sli_zigbee_af_gp_default_generic_switch_table_ptr = getSystemDefinedDefaultGenericSwitchTable(&sli_zigbee_af_gp_default_generic_switch_table_size);
#endif //USER_HAS_DEFAULT_GENERIC_SWITCH_TRANSLATION_TABLE
}

//After the pairing session, manage the customized translation table
void sli_zigbee_af_gp_pairing_done_thus_set_customized_translation_table(sl_zigbee_gp_address_t *gpdAddr,
                                                                         uint8_t gpdCommandId,
                                                                         uint8_t endpoint)
{
  // If switch command and recievd within the commissioning time
  if ((gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS
       || gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_RELEASE)
      && sl_zigbee_af_event_is_scheduled(genericSwitchCommissioningTimeout)) {
    addSwitchToTranslationTable(gpdAddr,
                                gpdCommandId,
                                endpoint);
    return;
  }
  // Multisensor or CAR
  if (gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING) {
    addReportstoTranslationTable(gpdAddr,
                                 gpdCommandId,
                                 endpoint);
    return;
  }
  uint8_t outIndex = GP_TRANSLATION_TABLE_ENTRY_INVALID_INDEX;
  uint8_t status = sli_zigbee_af_gp_trans_table_find_matching_translation_table_entry((GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID
                                                                                       | GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_CMD_ID
                                                                                       | GP_TRANSLATION_TABLE_SCAN_LEVEL_ZB_ENDPOINT),
                                                                                      false,
                                                                                      gpdAddr,
                                                                                      gpdCommandId,
                                                                                      endpoint,
                                                                                      NULL,
                                                                                      NULL,
                                                                                      &outIndex,
                                                                                      0);
  if (outIndex == GP_TRANSLATION_TABLE_ENTRY_INVALID_INDEX
      && status != GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
    uint8_t newTTEntryIndex = 0xFF;
    addPairedDeviceToTranslationTable(ADD_PAIRED_DEVICE,
                                      false,
                                      gpdAddr,
                                      gpdCommandId,
                                      endpoint,
                                      0,
                                      0,
                                      0,
                                      0,
                                      NULL,
                                      0,
                                      0,
                                      NULL,
                                      &newTTEntryIndex);
  }
}

// Get the default table entry
const sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t *sli_zigbee_af_gp_get_default_table(void)
{
  return sli_zigbee_af_gp_default_table_ptr;
}
