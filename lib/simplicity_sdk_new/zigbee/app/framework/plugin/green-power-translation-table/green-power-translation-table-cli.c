/***************************************************************************//**
 * @file
 * @brief CLI for the Green Power Translation Table plugin.
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
#else // !SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "green-power-adapter.h"
#endif //SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "green-power-translation-table.h"

#include "green-power-common.h"

#define  GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ADDITIONAL_INFO_LEN  36

static void print_additional_info_block(uint8_t gpdCommand, uint8_t addInfoOffset)
{
  sl_zigbee_gp_translation_table_additional_info_block_field_t *additionalInfoTable = sli_zigbee_af_gp_get_additional_info_table();
  sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t * addInfo = &(additionalInfoTable->additionalInfoBlock[addInfoOffset]);
  if (gpdCommand == SL_ZIGBEE_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING) {
    sl_zigbee_af_core_println("%d %d %d %d 0x%04X 0x%04X %d %d 0x%04X",
                              addInfo->totalLengthOfAddInfoBlock,
                              addInfo->optionSelector,
                              addInfo->optionData.compactAttr.reportIdentifier,
                              addInfo->optionData.compactAttr.attrOffsetWithinReport,
                              addInfo->optionData.compactAttr.clusterID,
                              addInfo->optionData.compactAttr.attributeID,
                              addInfo->optionData.compactAttr.attributeDataType,
                              addInfo->optionData.compactAttr.attributeOptions,
                              addInfo->optionData.compactAttr.manufacturerID);
  } else {
    sl_zigbee_af_core_println("%d %d 0x%04X 0x%04X",
                              addInfo->totalLengthOfAddInfoBlock,
                              addInfo->optionSelector,
                              addInfo->optionData.genericSwitch.contactBitmask,
                              addInfo->optionData.genericSwitch.contactStatus);
  }
}

void sl_zigbee_af_green_power_server_cli_clear_additional_info_block_table(SL_CLI_COMMAND_ARG)
{
  (void)arguments;
  sli_zigbee_gp_clear_additional_info_block_table();
}

void sl_zigbee_af_green_power_server_cli_clear_customized_table(SL_CLI_COMMAND_ARG)
{
  (void)arguments;
  sli_zigbee_af_gp_clear_customized_table();
}

WEAK(void sl_zigbee_af_green_power_server_cli_clear_translation_translation_table(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
  sli_zigbee_af_gp_trans_table_clear_translation_table();
}

void sl_zigbee_af_green_power_server_cli_clear_additional_info_block_table_entry(SL_CLI_COMMAND_ARG)
{
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_gp_translation_table_additional_info_block_field_t *additionalInfoTable = sli_zigbee_af_gp_get_additional_info_table();

  if (index < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE
      && additionalInfoTable->validEntry[index]) {
    additionalInfoTable->validEntry[index]--;
    if (additionalInfoTable->validEntry[index] == 0) {
      memset(&(additionalInfoTable->additionalInfoBlock[index]), 0x00, sizeof(sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t));
      additionalInfoTable->totlaNoOfEntries--;
    }
    sli_zigbee_af_gp_set_additional_info_block_table_entry(index);
  }
}

void sl_zigbee_af_green_power_server_cli_clear_customized_table_entry(SL_CLI_COMMAND_ARG)
{
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t* customizedTable = sli_zigbee_af_gp_get_customized_table();

  if (index < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE) {
    memset(&(customizedTable[index]),
           0x00,
           sizeof(sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t));
    sli_zigbee_af_gp_set_customized_table_entry(index);
  }
}

WEAK(void sl_zigbee_af_green_power_server_cli_clear_translation_table_entry(SL_CLI_COMMAND_ARG))
{
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  sli_zigbee_af_gp_command_translation_table * translationtable = sli_zigbee_af_gp_trans_table_get_translation_table();

  if ((index < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE) && (translationtable->totalNoOfEntries > 0)) {
    memset(&translationtable->TableEntry[index], 0x00, sizeof(sli_zigbee_af_gp_command_translation_table_entry));
    translationtable->TableEntry[index].entry = NO_ENTRY;
    translationtable->TableEntry[index].offset = 0xFF;
    translationtable->totalNoOfEntries--;
    sli_zigbee_af_gp_set_translation_table_entry(index);
  }
}

void sl_zigbee_af_green_power_server_cli_set_additional_info_block_table_entry(SL_CLI_COMMAND_ARG)
{
  sl_zigbee_gp_translation_table_additional_info_block_field_t *additionalInfoTable = sli_zigbee_af_gp_get_additional_info_table();
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t gpdCommandId = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t additionalInfoLength = sl_cli_get_argument_uint8(arguments, 2);

  uint8_t additionalInfoBlockIn[GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ADDITIONAL_INFO_LEN];
  sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t additionalInfoBlockOut;

  if (additionalInfoLength < GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ADDITIONAL_INFO_LEN) {
    sl_zigbee_copy_hex_arg(arguments, 3, additionalInfoBlockIn, additionalInfoLength, false);
  }
  if (index < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE) {
    sli_zigbee_af_gp_copy_additional_info_block_array_to_structure(additionalInfoBlockIn, &additionalInfoBlockOut, gpdCommandId);
    memcpy(&additionalInfoTable->additionalInfoBlock[index],
           &additionalInfoBlockOut,
           sizeof(sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t));
    additionalInfoTable->validEntry[index]++;
    additionalInfoTable->totlaNoOfEntries++;
    sli_zigbee_af_gp_set_additional_info_block_table_entry(index);
  }
}

void sl_zigbee_af_green_power_server_cli_set_customized_table_entry(SL_CLI_COMMAND_ARG)
{
  sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t customizedTableEntryIn;
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  customizedTableEntryIn.validEntry = true;
  customizedTableEntryIn.gpdCommand    = sl_cli_get_argument_uint8(arguments, 1);
  customizedTableEntryIn.endpoint    = sl_cli_get_argument_uint8(arguments, 2);
  customizedTableEntryIn.zigbeeProfile = sl_cli_get_argument_uint16(arguments, 3);
  customizedTableEntryIn.zigbeeCluster = sl_cli_get_argument_uint16(arguments, 4);
  customizedTableEntryIn.serverClient  = sl_cli_get_argument_uint8(arguments, 5);
  customizedTableEntryIn.zigbeeCommandId = sl_cli_get_argument_uint8(arguments, 6);
  customizedTableEntryIn.payloadSrc    = sl_cli_get_argument_uint8(arguments, 7);
  uint8_t payloadLength = sl_cli_get_argument_uint8(arguments, 8);

  if (payloadLength < SL_ZIGBEE_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN) {
    sl_zigbee_copy_hex_arg(arguments,
                           9,
                           customizedTableEntryIn.zclPayloadDefault,
                           SL_ZIGBEE_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN,
                           false);
  }
  if (index < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE) {
    sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t* customizedTable = sli_zigbee_af_gp_get_customized_table();
    memcpy(&(customizedTable[index]),
           &customizedTableEntryIn,
           sizeof(sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t));
    sli_zigbee_af_gp_set_customized_table_entry(index);
  }
}

void sl_zigbee_af_green_power_server_cli_set_translation_table(SL_CLI_COMMAND_ARG)
{
  //assign default values wherever possible since CLI has a limitation on number of arguments
  uint8_t retval;
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  uint16_t options = sl_cli_get_argument_uint16(arguments, 1);
  bool infoBlockPresent = ((options >> 0x03) & 0x01);
  uint8_t gpApplicationId = (options & 0x07);
  uint32_t  gpdSrcId = 0;
  sl_802154_long_addr_t gpdIeeeAddr;
  uint8_t gpdEndpoint = 1;
  uint8_t zbEndpoint = sl_cli_get_argument_uint8(arguments, 5);
  uint8_t gpdCommandId = sl_cli_get_argument_uint8(arguments, 6);
  uint16_t zigbeeProfile = sl_cli_get_argument_uint16(arguments, 7);
  uint16_t zigbeeCluster = sl_cli_get_argument_uint16(arguments, 8);
  uint8_t  zigbeeCommandId = sl_cli_get_argument_uint8(arguments, 9);
  uint8_t payloadSrc  = SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED;//sl_zigbee_unsigned_command_argument(10);
  uint8_t payloadLength = sl_cli_get_argument_uint8(arguments, 10);
  uint8_t payload[SL_ZIGBEE_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN];
  uint8_t additionalInfoLength = sl_cli_get_argument_uint8(arguments, 12);
  uint8_t additionalInfoBlockIn[GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ADDITIONAL_INFO_LEN];
  sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t additionalInfoBlockOut;

  if (index < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE) {
    if (gpApplicationId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
      sl_zigbee_copy_eui64_arg(arguments, 2, gpdIeeeAddr, true);
      gpdEndpoint = sl_cli_get_argument_uint8(arguments, 3);
    } else if (gpApplicationId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
      gpdSrcId = sl_cli_get_argument_uint32(arguments, 4);
    }
    if (payloadLength < SL_ZIGBEE_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN) {
      sl_zigbee_copy_hex_arg(arguments,
                             11,
                             payload,
                             SL_ZIGBEE_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN,
                             false);
    }

    if ( infoBlockPresent) {
      if (additionalInfoLength < GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ADDITIONAL_INFO_LEN) {
        sl_zigbee_copy_hex_arg(arguments,
                               13,
                               additionalInfoBlockIn,
                               additionalInfoLength,
                               false);
      }
      sli_zigbee_af_gp_copy_additional_info_block_array_to_structure(additionalInfoBlockIn, &additionalInfoBlockOut, gpdCommandId);
    }
    sl_zigbee_gp_address_t gpdAddr;
    if (!sli_zigbee_af_gp_make_addr(&gpdAddr, gpApplicationId, gpdSrcId, gpdIeeeAddr, gpdEndpoint)) {
      sl_zigbee_af_core_println("GPD addr Error");
      return;
    }
    retval = sli_zigbee_af_gp_trans_table_add_translation_table_entry_update_command(index,
                                                                                     infoBlockPresent,
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
      if (retval == GP_TRANSLATION_TABLE_STATUS_ENTRY_NOT_EMPTY) {
        sl_zigbee_af_core_println("Entry @Index [%d] is not empty, trying replace", index);
        retval = sli_zigbee_af_gp_trans_table_replace_translation_table_entry_update_command(index,
                                                                                             infoBlockPresent,
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
        sl_zigbee_af_core_println("Replace Ststus  = %d ", retval);
      } else if (retval == GP_TRANSLATION_TABLE_STATUS_PARAM_DOES_NOT_MATCH) {
        sl_zigbee_af_core_println("Parameter does not match @Index [%d]", index);
        sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_FAILURE); //send failure notification immediately
      }
    }
  } else {
    sl_zigbee_af_core_println("invalid Translation Table index!");
  }
}

void sl_zigbee_af_green_power_server_cli_additional_info_block_table_print(SL_CLI_COMMAND_ARG)
{
  (void)arguments;
  uint8_t gpdCommandId = SL_ZIGBEE_ZCL_GP_GPDF_8BITS_VECTOR_PRESS;
  sl_zigbee_gp_translation_table_additional_info_block_field_t *addInfoTable = sli_zigbee_af_gp_get_additional_info_table();
  for (int i = 0; i < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE; i++) {
    if (addInfoTable->additionalInfoBlock[i].totalLengthOfAddInfoBlock > 3) {
      gpdCommandId = SL_ZIGBEE_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING;
    }
    print_additional_info_block(gpdCommandId, i);
  }
}
void sl_zigbee_af_green_power_server_cli_customized_table_print(SL_CLI_COMMAND_ARG)
{
  (void)arguments;
  sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t  * entry = NULL;
  // print customized table.
  sl_zigbee_af_core_println("Print Sink Proxy Server Customized Translation Table:");
  sl_zigbee_af_core_println("index:\tvalid\topt\tgpEp\tieee\t\t\tsrcId\t\tep\tgpdCommandId\tprofile\tcluster\tcmdId\tpayloadType\tpayload");
  uint8_t index;
  uint8_t len;
  sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t* customizedTable = sli_zigbee_af_gp_get_customized_table();
  for (index = 0; index < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_CUSTOMIZED_GPD_TRANSLATION_TABLE_SIZE; index++) {
    entry = &customizedTable[index];
    sl_zigbee_af_core_print("%d  0x%02X\t\t  0x%04X\t  0x%04X\t  0x%02X\t",
                            entry->endpoint,
                            entry->gpdCommand,
                            entry->zigbeeProfile,
                            entry->zigbeeCluster,
                            entry->zigbeeCommandId);
    sl_zigbee_af_core_print("0x%02X", entry->payloadSrc);

    len = sl_zigbee_af_string_length(entry->zclPayloadDefault);
    if (len >= SL_ZIGBEE_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN) {
      len = SL_ZIGBEE_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN - 1;
    }
    sl_zigbee_af_print_buffer(SL_ZIGBEE_AF_PRINT_CORE, entry->zclPayloadDefault, len + 1, false);

    sl_zigbee_af_core_print(" \t");
    sl_zigbee_af_core_println("");
  }
}

void sl_zigbee_af_green_power_server_cli_translation_table_print(SL_CLI_COMMAND_ARG)
{
  (void)arguments;
  uint8_t status;
  sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t TranslationTableEntry;
  sli_zigbee_af_gp_command_translation_table * sli_zigbee_gp_translation_table = sli_zigbee_af_gp_trans_table_get_translation_table();
  if (!(sli_zigbee_gp_translation_table->totalNoOfEntries)) {
    sl_zigbee_af_core_println("translation table is Empty");
  } else {
    sl_zigbee_af_core_println("\nIndex Valid AppId Ieee                Ep gpdSrc   gdpCmd zEp zProf zCluster zCmd EType Info ");
    for (uint8_t entryIndex = 0; entryIndex < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE; entryIndex++ ) {
      if (sli_zigbee_gp_translation_table->TableEntry[entryIndex].entry == NO_ENTRY) {
        continue;
      }
      status = sli_zigbee_af_gp_trans_table_get_translation_table_entry(entryIndex, &TranslationTableEntry);
      if (status == GP_TRANSLATION_TABLE_STATUS_SUCCESS) {
        sl_zigbee_af_core_print("%02X    ", entryIndex);
        sl_zigbee_af_core_print("%02X    ", TranslationTableEntry.validEntry);
        sl_zigbee_af_core_print("%02X    ", sli_zigbee_gp_translation_table->TableEntry[entryIndex].gpAddr.applicationId);
        if ((sli_zigbee_gp_translation_table->TableEntry[entryIndex].gpAddr.applicationId & SL_ZIGBEE_AF_GP_NOTIFICATION_OPTION_APPLICATION_ID) == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
          sl_zigbee_af_print_big_endian_eui64(sli_zigbee_gp_translation_table->TableEntry[entryIndex].gpAddr.id.gpdIeeeAddress);
          sl_zigbee_af_core_print(" ");
          sl_zigbee_af_core_print("%02X ", sli_zigbee_gp_translation_table->TableEntry[entryIndex].gpAddr.endpoint);
          sl_zigbee_af_core_print("         ");
        } else {
          sl_zigbee_af_core_print("                       ");
          sl_zigbee_af_core_print("%08X ", sli_zigbee_gp_translation_table->TableEntry[entryIndex].gpAddr.id.sourceId);
        }
        sl_zigbee_af_core_print("%02X     ", TranslationTableEntry.gpdCommand);
        sl_zigbee_af_core_print("%02X  ", TranslationTableEntry.endpoint);
        sl_zigbee_af_core_print("%04X  ", TranslationTableEntry.zigbeeProfile);
        sl_zigbee_af_core_print("%04X     ", TranslationTableEntry.zigbeeCluster);
        sl_zigbee_af_core_print("%02X   ", TranslationTableEntry.zigbeeCommandId);
        sl_zigbee_af_core_print("%02X    ", sli_zigbee_gp_translation_table->TableEntry[entryIndex].entry);
        sl_zigbee_af_core_print("%02X   ", sli_zigbee_gp_translation_table->TableEntry[entryIndex].infoBlockPresent);
        if (sli_zigbee_gp_translation_table->TableEntry[entryIndex].infoBlockPresent == true) {
          print_additional_info_block(TranslationTableEntry.gpdCommand, sli_zigbee_gp_translation_table->TableEntry[entryIndex].additionalInfoOffset);
        }
        sl_zigbee_af_core_print("\n");
      } else {
        sl_zigbee_af_core_println("[%s] sli_zigbee_af_gp_trans_table_get_translation_table FAILED", __FUNCTION__);
      }
    }
    sl_zigbee_af_core_print("\n");
  }
}

void sl_zigbee_af_green_power_server_cli_remove_gpd_endpoint(SL_CLI_COMMAND_ARG)
{
  //assign default values wherever possible since CLI has a limitation on number of arguments
  uint8_t gpApplicationId = sl_cli_get_argument_uint8(arguments, 0);
  uint32_t  gpdSrcId = 0;
  sl_802154_long_addr_t gpdIeeeAddr;
  uint8_t gpdEndpoint = 1;
  uint8_t zbEndpoint = sl_cli_get_argument_uint8(arguments, 4);

  if (gpApplicationId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
    gpdSrcId = sl_cli_get_argument_uint32(arguments, 1);
  } else if (gpApplicationId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
    sl_zigbee_copy_eui64_arg(arguments, 2, gpdIeeeAddr, true);
    gpdEndpoint = sl_cli_get_argument_uint8(arguments, 3);
  }

  sl_zigbee_gp_address_t gpdAddr;
  if (!sli_zigbee_af_gp_make_addr(&gpdAddr, gpApplicationId, gpdSrcId, gpdIeeeAddr, gpdEndpoint)) {
    sl_zigbee_af_green_power_cluster_println("GPD addr Error");
    return;
  }
  sl_zigbee_af_green_power_cluster_print("%02X    ", gpApplicationId);
  if ((gpApplicationId & SL_ZIGBEE_AF_GP_NOTIFICATION_OPTION_APPLICATION_ID) == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
    sl_zigbee_af_print_big_endian_eui64(gpdAddr.id.gpdIeeeAddress);
    sl_zigbee_af_green_power_cluster_print(" ");
    sl_zigbee_af_green_power_cluster_print("%02X ", gpdAddr.endpoint);
    sl_zigbee_af_green_power_cluster_print("         ");
  } else {
    sl_zigbee_af_green_power_cluster_print("                       ");
    sl_zigbee_af_green_power_cluster_print("%08X ", gpdAddr.id.sourceId);
  }
  sl_zigbee_af_green_power_cluster_println("%02X  ", zbEndpoint);
  sli_zigbee_af_gp_remove_gpd_endpoint_from_translation_table(&gpdAddr, zbEndpoint);
}
