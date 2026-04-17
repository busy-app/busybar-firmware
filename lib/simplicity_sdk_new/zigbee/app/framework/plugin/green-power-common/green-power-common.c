/***************************************************************************//**
 * @file
 * @brief Place for common functions / definitions shared by Green Power Client/Server
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
#else // !SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "green-power-adapter.h"
#endif //SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "stack/include/zigbee-device-stack.h"
#include "green-power-common.h"

extern bool sli_zigbee_gp_address_match(const sl_zigbee_gp_address_t *a1, const sl_zigbee_gp_address_t *a2);
extern void sli_zigbee_spoof_device_announcement(uint16_t shortId,
                                                 uint8_t *sourceEUI64,
                                                 sl_802154_long_addr_t deviceAnnounceEui,
                                                 uint8_t capabilities);
extern sl_802154_short_addr_t sl_zigbee_af_response_destination;
uint16_t sli_zigbee_af_copy_additional_info_block_structure_to_array(uint8_t commandId,
                                                                     sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *additionalInfoBlockIn,
                                                                     uint8_t *additionalInfoBlockOut)
{
  uint8_t charCount = 0;
  uint8_t *additionalInfoBlockOutPtr = additionalInfoBlockOut;

  // copy GPD Additional information block length
  sl_zigbee_af_copy_int8u(additionalInfoBlockOutPtr, 0, (uint8_t)(additionalInfoBlockIn->totalLengthOfAddInfoBlock));
  additionalInfoBlockOutPtr += sizeof(uint8_t);

  if ((additionalInfoBlockIn->totalLengthOfAddInfoBlock) != 0x00) {
    sl_zigbee_af_green_power_cluster_println("%d ", additionalInfoBlockIn->totalLengthOfAddInfoBlock);
    //if ((additionalInfoBlockIn->optionRecord.optionSelector != 0x00) && (recordIndex < additionalInfoBlockIn->totalNbOfOptionRecord) ) {
    // copy  option selector field
    //optionSelector = (additionalInfoBlockIn->optionRecord[recordIndex].optionID & 0xF0) | (additionalInfoBlockIn->optionRecord[recordIndex].optionLength & 0x0F);
    sl_zigbee_af_copy_int8u(additionalInfoBlockOutPtr, 0, (uint8_t)(additionalInfoBlockIn->optionSelector));
    additionalInfoBlockOutPtr += sizeof(uint8_t);

    if (commandId == SL_ZIGBEE_ZCL_GP_GPDF_COMPACT_ATTRIBUTE_REPORTING ) {
      sl_zigbee_af_copy_int8u(additionalInfoBlockOutPtr,
                              0,
                              (uint8_t)(additionalInfoBlockIn->optionData.compactAttr.reportIdentifier));
      additionalInfoBlockOutPtr += sizeof(uint8_t);
      sl_zigbee_af_copy_int8u(additionalInfoBlockOutPtr,
                              0,
                              (uint8_t)(additionalInfoBlockIn->optionData.compactAttr.attrOffsetWithinReport));
      additionalInfoBlockOutPtr += sizeof(uint8_t);
      sl_zigbee_af_copy_int16u(additionalInfoBlockOutPtr,
                               0,
                               (uint16_t)(additionalInfoBlockIn->optionData.compactAttr.clusterID));
      additionalInfoBlockOutPtr += sizeof(uint16_t);
      sl_zigbee_af_copy_int16u(additionalInfoBlockOutPtr,
                               0,
                               (uint16_t)(additionalInfoBlockIn->optionData.compactAttr.attributeID));
      additionalInfoBlockOutPtr += sizeof(uint16_t);
      sl_zigbee_af_copy_int8u(additionalInfoBlockOutPtr,
                              0,
                              (uint8_t)(additionalInfoBlockIn->optionData.compactAttr.attributeDataType));
      additionalInfoBlockOutPtr += sizeof(uint8_t);
      //attributeoptions = ((additionalInfoBlockIn->optionRecord[recordIndex].optionData.compactAttr.manufacturerIdPresent << 1) | (additionalInfoBlockIn->optionRecord[recordIndex].optionData.compactAttr.clientServer));
      sl_zigbee_af_copy_int8u(additionalInfoBlockOutPtr, 0, (uint8_t)(additionalInfoBlockIn->optionData.compactAttr.attributeOptions));
      additionalInfoBlockOutPtr += sizeof(uint8_t);
      //if ( additionalInfoBlockIn->optionRecord[recordIndex].optionData.compactAttr.manufacturerIdPresent) {
      if (additionalInfoBlockIn->optionData.compactAttr.attributeOptions & 0x02) {
        sl_zigbee_af_copy_int16u(additionalInfoBlockOutPtr,
                                 0,
                                 (uint16_t)(additionalInfoBlockIn->optionData.compactAttr.manufacturerID));
        additionalInfoBlockOutPtr += sizeof(uint16_t);
      }
      sl_zigbee_af_green_power_cluster_println("%d %d %d 0x%04X 0x%04X %d %d 0x%04X",
                                               additionalInfoBlockIn->optionSelector,
                                               additionalInfoBlockIn->optionData.compactAttr.reportIdentifier,
                                               additionalInfoBlockIn->optionData.compactAttr.attrOffsetWithinReport,
                                               additionalInfoBlockIn->optionData.compactAttr.clusterID,
                                               additionalInfoBlockIn->optionData.compactAttr.attributeID,
                                               additionalInfoBlockIn->optionData.compactAttr.attributeDataType,
                                               additionalInfoBlockIn->optionData.compactAttr.attributeOptions,
                                               additionalInfoBlockIn->optionData.compactAttr.manufacturerID);
    } else {
      sl_zigbee_af_copy_int8u(additionalInfoBlockOutPtr,
                              0,
                              (uint8_t)(additionalInfoBlockIn->optionData.genericSwitch.contactStatus));
      additionalInfoBlockOutPtr += sizeof(uint8_t);
      sl_zigbee_af_copy_int8u(additionalInfoBlockOutPtr,
                              0,
                              (uint8_t)(additionalInfoBlockIn->optionData.genericSwitch.contactBitmask));
      additionalInfoBlockOutPtr += sizeof(uint8_t);
    }

    sl_zigbee_af_green_power_cluster_println("%d 0x%04X 0x%04X",
                                             additionalInfoBlockIn->optionSelector,
                                             additionalInfoBlockIn->optionData.genericSwitch.contactBitmask,
                                             additionalInfoBlockIn->optionData.genericSwitch.contactStatus);
  }
  charCount = (uint16_t)(additionalInfoBlockOutPtr - additionalInfoBlockOut);
  if (additionalInfoBlockIn->totalLengthOfAddInfoBlock != (charCount - 1)) {
    sl_zigbee_af_green_power_cluster_println("[%s:%d] Error in Addiotional Information Block additionalInfoBlockIn->totalLengthOfAddInfoBlock = %d, charCount= %d",
                                             __FUNCTION__, __LINE__, additionalInfoBlockIn->totalLengthOfAddInfoBlock, charCount);
  }
  return charCount;
}

uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_notification_smart(uint16_t options,
                                                                             uint32_t gpdSrcId,
                                                                             uint8_t* gpdIeee,
                                                                             uint8_t  gpdEndpoint,
                                                                             uint32_t gpdSecurityFrameCounter,
                                                                             uint8_t  gpdCommandId,
                                                                             uint8_t gpdCommandPayloadLength,
                                                                             const uint8_t* gpdCommandPayload,
                                                                             uint16_t gppShortAddress,
                                                                             uint8_t  gppDistance)
{
  uint16_t charCount = 0;
  sl_zigbee_gp_application_id_t appId = sl_zigbee_af_green_power_get_application_id(options);

  if ((appId != SL_ZIGBEE_GP_APPLICATION_SOURCE_ID)
      && (appId != SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }

  charCount += sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                                  | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                                                  | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                                                 ZCL_GREEN_POWER_CLUSTER_ID,
                                                 ZCL_GP_NOTIFICATION_COMMAND_ID,
                                                 "v",
                                                 options);

  if (appId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int32u_in_resp(gpdSrcId));
    charCount += sizeof(uint32_t);
  } else if (appId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(gpdIeee, EUI64_SIZE));
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(gpdEndpoint));
    charCount += EUI64_SIZE + sizeof(uint8_t);
  }

  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int32u_in_resp(gpdSecurityFrameCounter));
  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(gpdCommandId));
  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(gpdCommandPayloadLength));
  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(gpdCommandPayload, gpdCommandPayloadLength));
  charCount += sizeof(uint32_t) + sizeof(uint8_t) + gpdCommandPayloadLength + sizeof(uint8_t);
  if (options & SL_ZIGBEE_AF_GP_NOTIFICATION_OPTION_PROXY_INFO_PRESENT) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int16u_in_resp(gppShortAddress));
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(gppDistance));
    charCount += sizeof(uint16_t) + sizeof(uint8_t);
  }

  return charCount;
}

uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_pairing_search_smart(uint16_t options,
                                                                               uint32_t gpdSrcId,
                                                                               uint8_t* gpdIeee,
                                                                               uint8_t endpoint)
{
  sl_zigbee_gp_application_id_t appId = sl_zigbee_af_green_power_get_application_id(options);
  uint16_t charCount = 0;

  if ((appId != SL_ZIGBEE_GP_APPLICATION_SOURCE_ID)
      && (appId != SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }

  charCount += sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND           \
                                                  | ZCL_DISABLE_DEFAULT_RESPONSE_MASK    \
                                                  | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), \
                                                 ZCL_GREEN_POWER_CLUSTER_ID,             \
                                                 ZCL_GP_PAIRING_SEARCH_COMMAND_ID,       \
                                                 "v",                                    \
                                                 options);

  if (appId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int32u_in_resp(gpdSrcId));
    charCount += sizeof(uint32_t);
  } else if (appId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(gpdIeee, EUI64_SIZE));
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(endpoint));
    charCount += EUI64_SIZE + sizeof(uint8_t);
  }

  return charCount;
}

uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_tunneling_stop_smart(uint8_t options,
                                                                               uint32_t gpdSrcId,
                                                                               uint8_t* gpdIeee,
                                                                               uint8_t endpoint,
                                                                               uint32_t gpdSecurityFrameCounter,
                                                                               uint16_t gppShortAddress,
                                                                               int8_t gppDistance)
{
  sl_zigbee_gp_application_id_t appId = sl_zigbee_af_green_power_get_application_id(options);
  uint16_t charCount = 0;

  if ((appId != SL_ZIGBEE_GP_APPLICATION_SOURCE_ID)
      && (appId != SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }

  charCount += sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND           \
                                                  | ZCL_DISABLE_DEFAULT_RESPONSE_MASK    \
                                                  | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), \
                                                 ZCL_GREEN_POWER_CLUSTER_ID,             \
                                                 ZCL_GP_TUNNELING_STOP_COMMAND_ID,       \
                                                 "u",                                    \
                                                 options);

  if (appId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int32u_in_resp(gpdSrcId));
    charCount += sizeof(uint32_t);
  } else if (appId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(gpdIeee, EUI64_SIZE));
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(endpoint));
    charCount += EUI64_SIZE + sizeof(uint8_t);
  }

  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int32u_in_resp(gpdSecurityFrameCounter));
  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int16u_in_resp(gppShortAddress));
  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(gppDistance));

  charCount += sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint8_t);

  return charCount;
}

uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_commissioning_notification_smart(uint16_t options,
                                                                                           uint32_t gpdSrcId,
                                                                                           uint8_t* gpdIeee,
                                                                                           uint8_t endpoint,
                                                                                           uint8_t macSequenceNumber,
                                                                                           sl_zigbee_gp_security_level_t gpdfSecurityLevel,
                                                                                           sl_zigbee_gp_security_frame_counter_t gpdSecurityFrameCounter,
                                                                                           uint8_t gpdCommandId,
                                                                                           uint8_t gpdCommandPayloadLength,
                                                                                           const uint8_t *gpdCommandPayload,
                                                                                           sl_802154_short_addr_t gppShortAddress,
                                                                                           uint8_t gppLink,
                                                                                           sl_zigbee_gp_mic_t mic)
{
  sl_zigbee_gp_application_id_t appId = sl_zigbee_af_green_power_get_application_id(options);
  uint16_t charCount = 0;
  uint32_t securityFrameCounter = 0;

  if ((appId != SL_ZIGBEE_GP_APPLICATION_SOURCE_ID)
      && (appId != SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }

  charCount += sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                                  | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                                                  | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                                                 ZCL_GREEN_POWER_CLUSTER_ID,
                                                 ZCL_GP_COMMISSIONING_NOTIFICATION_COMMAND_ID,
                                                 "v",
                                                 options);

  if (appId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int32u_in_resp(gpdSrcId));
    charCount += sizeof(uint32_t);
  } else if (appId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(gpdIeee, EUI64_SIZE));
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(endpoint));
    charCount += EUI64_SIZE + sizeof(uint8_t);
  }

  if (gpdfSecurityLevel == 0 ) {
    securityFrameCounter = (uint32_t)macSequenceNumber;
  } else {
    securityFrameCounter = gpdSecurityFrameCounter;
  }
  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int32u_in_resp(securityFrameCounter));
  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(gpdCommandId));
  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(gpdCommandPayloadLength));
  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(gpdCommandPayload, gpdCommandPayloadLength));
  charCount += sizeof(uint32_t) + sizeof(uint8_t) + gpdCommandPayloadLength + sizeof(uint8_t);
  if (options & SL_ZIGBEE_AF_GP_COMMISSIONING_NOTIFICATION_OPTION_PROXY_INFO_PRESENT) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int16u_in_resp(gppShortAddress));
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(gppLink));
    charCount += sizeof(uint16_t) + sizeof(uint8_t);
  }

  if (options & SL_ZIGBEE_AF_GP_COMMISSIONING_NOTIFICATION_OPTION_SECURITY_PROCESSING_FAILED) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int32u_in_resp(mic));
    charCount += sizeof(uint32_t);
  }

  return charCount;
}

uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_translation_table_update_smart(uint16_t options,
                                                                                         uint32_t gpdSrcId,
                                                                                         uint8_t* gpdIeee,
                                                                                         uint8_t endpoint,
                                                                                         uint8_t translationsLen,
                                                                                         sl_zigbee_zcl_gp_translation_table_update_translation_t* translations,
                                                                                         sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t* additionalInfoBlock)
{
  (void)translationsLen;
  sl_zigbee_gp_application_id_t appId = sl_zigbee_af_green_power_get_application_id(options);
  uint16_t charCount = 0;

  if (((appId != SL_ZIGBEE_GP_APPLICATION_SOURCE_ID)
       && (appId != SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS))
      || (translations == NULL)) {
    return 0;
  }

  charCount += sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND               \
                                                  | ZCL_DISABLE_DEFAULT_RESPONSE_MASK        \
                                                  | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),     \
                                                 ZCL_GREEN_POWER_CLUSTER_ID,                 \
                                                 ZCL_GP_TRANSLATION_TABLE_UPDATE_COMMAND_ID, \
                                                 "v",                                        \
                                                 options);

  if (appId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int32u_in_resp(gpdSrcId));
    charCount += sizeof(uint32_t);
  } else if (appId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(gpdIeee, EUI64_SIZE));
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(endpoint));
    charCount += EUI64_SIZE + sizeof(uint8_t);
  }

  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(translations->index));
  charCount += sizeof(uint8_t);
  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(translations->gpdCommandId));
  charCount += sizeof(uint8_t);
  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(translations->endpoint));
  charCount += sizeof(uint8_t);
  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int16u_in_resp(translations->profile));
  charCount += sizeof(uint16_t);
  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int16u_in_resp(translations->cluster));
  charCount += sizeof(uint16_t);
  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(translations->zigbeeCommandId));
  charCount += sizeof(uint8_t);
  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_string_in_resp(translations->zigbeeCommandPayload));
  charCount += sl_zigbee_af_string_length(translations->zigbeeCommandPayload) + 1;
  if (sl_zigbee_af_green_power_t_t_update_get_additional_info_block_present(options) != 0
      && additionalInfoBlock) {
    uint8_t tempBuffer[32] = { 0 }; // Two records maximum per cli
    uint16_t length = sli_zigbee_af_copy_additional_info_block_structure_to_array(translations->gpdCommandId,
                                                                                  additionalInfoBlock,
                                                                                  tempBuffer);
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(length));
    charCount += sizeof(uint8_t);
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(tempBuffer, length));
    charCount += length;
  }
  return charCount;
}

uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_pairing_configuration_smart(uint8_t actions,
                                                                                      uint16_t options,
                                                                                      uint32_t gpdSrcId,
                                                                                      uint8_t* gpdIeee,
                                                                                      uint8_t endpoint,
                                                                                      uint8_t deviceId,
                                                                                      uint8_t groupListCount,
                                                                                      uint8_t* groupList,
                                                                                      uint16_t gpdAssignedAlias,
                                                                                      uint8_t groupcastRadius,
                                                                                      uint8_t securityOptions,
                                                                                      uint32_t gpdSecurityFrameCounter,
                                                                                      uint8_t* gpdSecurityKey,
                                                                                      uint8_t numberOfPairedEndpoints,
                                                                                      uint8_t* pairedEndpoints,
                                                                                      uint8_t applicationInformation,
                                                                                      uint16_t manufacturerId,
                                                                                      uint16_t modeId,
                                                                                      uint8_t numberOfGpdCommands,
                                                                                      uint8_t * gpdCommandIdList,
                                                                                      uint8_t clusterIdListCount,
                                                                                      uint16_t * clusterListServer,
                                                                                      uint16_t * clusterListClient,
                                                                                      uint8_t switchInformationLength,
                                                                                      uint8_t genericSwitchConfiguration,
                                                                                      uint8_t currentContactStatus,
                                                                                      uint8_t totalNumberOfReports,
                                                                                      uint8_t numberOfReports,
                                                                                      uint8_t* reportDescriptorM)
{
  sl_zigbee_gp_application_id_t appId = sl_zigbee_af_green_power_get_application_id(options);
  uint16_t charCount = 0;

  if ((appId != SL_ZIGBEE_GP_APPLICATION_SOURCE_ID)
      && (appId != SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }

  charCount += sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND            \
                                                  | ZCL_DISABLE_DEFAULT_RESPONSE_MASK     \
                                                  | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),  \
                                                 ZCL_GREEN_POWER_CLUSTER_ID,              \
                                                 ZCL_GP_PAIRING_CONFIGURATION_COMMAND_ID, \
                                                 "uv",                                    \
                                                 actions,
                                                 options);

  if (appId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int32u_in_resp(gpdSrcId));
    charCount += sizeof(uint32_t);
  } else if (appId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(gpdIeee, EUI64_SIZE));
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(endpoint));
    charCount += EUI64_SIZE + sizeof(uint8_t);
  }

  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(deviceId));
  charCount += sizeof(uint8_t);

  uint8_t gpPairingConfigCommunicationMode = (options
                                              & SL_ZIGBEE_AF_GP_PAIRING_CONFIGURATION_OPTION_COMMUNICATION_MODE)
                                             >> SL_ZIGBEE_AF_GP_PAIRING_CONFIGURATION_OPTION_COMMUNICATION_MODE_OFFSET;
  if (gpPairingConfigCommunicationMode == SL_ZIGBEE_GP_SINK_TYPE_GROUPCAST) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(groupListCount));
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(groupList, groupListCount * sizeof(sl_zigbee_gp_sink_group_t)));
    charCount += sizeof(uint8_t) + groupListCount * sizeof(sl_zigbee_gp_sink_group_t);
  }

  if (options & SL_ZIGBEE_AF_GP_PAIRING_CONFIGURATION_OPTION_ASSIGNED_ALIAS) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int16u_in_resp(gpdAssignedAlias));
    charCount += sizeof(uint16_t);
  }

  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(groupcastRadius));
  charCount += sizeof(uint8_t);

  if (options & SL_ZIGBEE_AF_GP_PAIRING_CONFIGURATION_OPTION_SECURITY_USE) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(securityOptions));
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int32u_in_resp(gpdSecurityFrameCounter));
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(gpdSecurityKey, SL_ZIGBEE_ENCRYPTION_KEY_SIZE));
    charCount += sizeof(uint32_t) + sizeof(uint8_t) + SL_ZIGBEE_ENCRYPTION_KEY_SIZE;
  }

  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(numberOfPairedEndpoints));
  charCount += sizeof(uint8_t);
  if (numberOfPairedEndpoints < SL_ZIGBEE_AF_GP_TRANSLATION_TABLE_ZB_ENDPOINT_PASS_FRAME_TO_APLLICATION) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(pairedEndpoints, numberOfPairedEndpoints));
    charCount += numberOfPairedEndpoints;
  }

  if (options & SL_ZIGBEE_AF_GP_PAIRING_CONFIGURATION_OPTION_APPLICATION_INFORMATION_PRESENT) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(applicationInformation));
    if (applicationInformation & SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_MANUFACTURE_ID_PRESENT) {
      sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int16u_in_resp(manufacturerId));
      charCount += sizeof(uint16_t);
    }

    if (applicationInformation & SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_MODEL_ID_PRESENT) {
      sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int16u_in_resp(modeId));
      charCount += sizeof(uint16_t);
    }

    if (applicationInformation & SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_GPD_COMMANDS_PRESENT) {
      sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(numberOfGpdCommands));
      sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(gpdCommandIdList, numberOfGpdCommands));
      charCount += sizeof(uint8_t) + numberOfGpdCommands;
    }

    if (applicationInformation & SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_PRESENT) {
      sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(clusterIdListCount));
      sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp((uint8_t*)clusterListServer, sizeof(sl_zigbee_af_cluster_id_t) * (clusterIdListCount & SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_NUMBER_OF_SERVER_CLUSTER_MASK)));
      sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp((uint8_t*)clusterListClient, sizeof(sl_zigbee_af_cluster_id_t) * ((clusterIdListCount & SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_NUMBER_OF_CLIENT_CLUSTER_MASK) >> SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_NUMBER_OF_CLIENT_CLUSTER_MASK_OFFSET)));
      charCount += sizeof(uint8_t)
                   + sizeof(sl_zigbee_af_cluster_id_t) * (clusterIdListCount & SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_NUMBER_OF_SERVER_CLUSTER_MASK)
                   + sizeof(sl_zigbee_af_cluster_id_t) * ((clusterIdListCount & SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_NUMBER_OF_CLIENT_CLUSTER_MASK) >> SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_NUMBER_OF_CLIENT_CLUSTER_MASK_OFFSET);
    }
  }
  if (applicationInformation & SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_SWITCH_INFORMATION_PRESENT) { // Switch Information Present
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(switchInformationLength));
    charCount += sizeof(uint8_t);
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(genericSwitchConfiguration));
    charCount += sizeof(uint8_t);
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(currentContactStatus));
    charCount += sizeof(uint8_t);
  }
  // Application description should add the reports
  if (actions == SL_ZIGBEE_ZCL_GP_PAIRING_CONFIGURATION_ACTION_APPLICATION_DESCRIPTION) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(totalNumberOfReports));
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(numberOfReports));
    uint8_t nextOffset = 0;
    for (uint8_t index = 0; index < numberOfReports; index++) {
      uint8_t * descPtr;  // pointing to each report descriptor
      uint8_t descLength; // of each descriptor
      descPtr = &reportDescriptorM[nextOffset];
      // the report descriptor array has following meaning for indexs
      // descPtr[0] is reportId, descPtr[1] is options
      // the bit 0 of option indicates timeout Period Present (all other bits reserved)
      // if timeout(uint16 type)is present, the remaining length is present at index 5 else it is present
      // at index 3 of descPtr
      if (descPtr[1] & SL_ZIGBEE_AF_GP_GPD_APPLICATION_DESCRIPTION_COMMAND_REPORT_OPTIONS_TIMEOUT_PERIOD_PRESENT) {
        descLength = descPtr[4] + 5;
      } else {
        descLength = descPtr[2] + 3;
      }
      nextOffset += descLength;
    }
    // ptr to block and bytes to copy
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(reportDescriptorM, nextOffset));
    charCount += nextOffset;
  }
  return charCount;
}

uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_sink_table_request_smart(uint8_t options,
                                                                                   uint32_t gpdSrcId,
                                                                                   uint8_t* gpdIeee,
                                                                                   uint8_t endpoint,
                                                                                   uint8_t index)
{
  (void)endpoint;
  sl_zigbee_gp_application_id_t appId = sl_zigbee_af_green_power_get_application_id(options);
  uint16_t charCount = 0;

  if ((appId != SL_ZIGBEE_GP_APPLICATION_SOURCE_ID)
      && (appId != SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }

  charCount += sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND           \
                                                  | ZCL_DISABLE_DEFAULT_RESPONSE_MASK    \
                                                  | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER), \
                                                 ZCL_GREEN_POWER_CLUSTER_ID,             \
                                                 ZCL_GP_SINK_TABLE_REQUEST_COMMAND_ID,   \
                                                 "uw",                                   \
                                                 options,                                \
                                                 gpdSrcId);

  if ( ((options & SL_ZIGBEE_AF_GP_SINK_TABLE_REQUEST_OPTIONS_REQUEST_TYPE)
        >> SL_ZIGBEE_AF_GP_SINK_TABLE_REQUEST_OPTIONS_REQUEST_TYPE_OFFSET) == SL_ZIGBEE_ZCL_GP_SINK_TABLE_REQUEST_OPTIONS_REQUEST_TABLE_ENTRIES_BY_GPD_ID) {
    if (appId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
      sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int32u_in_resp(gpdSrcId));
      charCount += sizeof(uint32_t);
    } else if (appId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
      sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(gpdIeee, EUI64_SIZE));
      sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(index));
      charCount += EUI64_SIZE + sizeof(uint8_t);
    }
  }
  if ( ((options & SL_ZIGBEE_AF_GP_SINK_TABLE_REQUEST_OPTIONS_REQUEST_TYPE)
        >> SL_ZIGBEE_AF_GP_SINK_TABLE_REQUEST_OPTIONS_REQUEST_TYPE_OFFSET) == SL_ZIGBEE_ZCL_GP_SINK_TABLE_REQUEST_OPTIONS_REQUEST_TABLE_ENTRIES_BY_INDEX) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(index));
    charCount += sizeof(uint8_t);
  }

  return charCount;
}

uint32_t sl_zigbee_af_fill_command_green_power_cluster_gp_notification_response_smart(uint8_t options,
                                                                                      uint32_t gpdSrcId,
                                                                                      uint8_t* gpdIeee,
                                                                                      uint8_t endpoint,
                                                                                      uint32_t gpdSecurityFrameCounter)
{
  sl_zigbee_gp_application_id_t appId = sl_zigbee_af_green_power_get_application_id(options);
  uint16_t charCount = 0;

  if ((appId != SL_ZIGBEE_GP_APPLICATION_SOURCE_ID)
      && (appId != SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }

  charCount += sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND        \
                                                  | ZCL_DISABLE_DEFAULT_RESPONSE_MASK \
                                                  | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT),
                                                 ZCL_GREEN_POWER_CLUSTER_ID,
                                                 ZCL_GP_NOTIFICATION_RESPONSE_COMMAND_ID,
                                                 "u",
                                                 options);

  if (appId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int32u_in_resp(gpdSrcId));
    charCount += sizeof(uint32_t);
  } else if (appId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(gpdIeee, EUI64_SIZE));
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(endpoint));
    charCount += EUI64_SIZE + sizeof(uint8_t);
  }
  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int32u_in_resp(gpdSecurityFrameCounter));
  charCount += sizeof(uint32_t);

  return charCount;
}

uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_pairing_smart(uint32_t options,
                                                                        uint32_t gpdSrcId,
                                                                        uint8_t* gpdIeee,
                                                                        uint8_t endpoint,
                                                                        uint8_t* sinkIeeeAddress,
                                                                        uint16_t sinkNwkAddress,
                                                                        uint16_t sinkGroupId,
                                                                        uint8_t deviceId,
                                                                        uint32_t gpdSecurityFrameCounter,
                                                                        uint8_t* gpdKey,
                                                                        uint16_t assignedAlias,
                                                                        uint8_t groupcastRadius)
{
  sl_zigbee_gp_application_id_t appId = sl_zigbee_af_green_power_get_application_id(options);
  uint16_t charCount = 0;
  uint8_t commMode = sl_zigbee_af_green_power_pairing_options_get_comm_mode(options);

  if ((appId != SL_ZIGBEE_GP_APPLICATION_SOURCE_ID)
      && (appId != SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }
  // The Disable default response sub-field of the Frame Control Field of
  // the ZCL header SHALL be set to 0b1.
  charCount += sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                                  | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                                                  | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                                                 ZCL_GREEN_POWER_CLUSTER_ID,
                                                 ZCL_GP_PAIRING_COMMAND_ID,
                                                 "x",
                                                 options);

  if (appId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int32u_in_resp(gpdSrcId));
    charCount += sizeof(uint32_t);
  } else if (appId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(gpdIeee, EUI64_SIZE));
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(endpoint));
    charCount += EUI64_SIZE + sizeof(uint8_t);
  }

  if ((options & SL_ZIGBEE_AF_GP_PAIRING_OPTION_REMOVE_GPD) == 0x00) {
    if ((commMode == 0x00) || (commMode == 0x03)) {
      sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(sinkIeeeAddress, EUI64_SIZE));
      sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int16u_in_resp(sinkNwkAddress));
      charCount += EUI64_SIZE + sizeof(uint16_t);
    } else if (commMode == 0x01 || commMode == 0x02) {
      sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int16u_in_resp(sinkGroupId));
      charCount += sizeof(uint16_t);
    }
  }

  if (sl_zigbee_af_green_power_pairing_options_get_add_sink(options)) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(deviceId));
    charCount += sizeof(uint8_t);
  }
  if (options & SL_ZIGBEE_AF_GP_PAIRING_OPTION_GPD_SECURITY_FRAME_COUNTER_PRESENT) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int32u_in_resp(gpdSecurityFrameCounter));
    charCount += sizeof(uint32_t);
  }
  if (options & SL_ZIGBEE_AF_GP_PAIRING_OPTION_GPD_SECURITY_KEY_PRESENT) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(gpdKey, SL_ZIGBEE_ENCRYPTION_KEY_SIZE));
    charCount += SL_ZIGBEE_ENCRYPTION_KEY_SIZE;
  }
  if (options & SL_ZIGBEE_AF_GP_PAIRING_OPTION_ASSIGNED_ALIAS_PRESENT) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int16u_in_resp(assignedAlias));
    charCount += sizeof(uint16_t);
  }
  if (options & SL_ZIGBEE_AF_GP_PAIRING_OPTION_GROUPCAST_RADIUS_PRESENT) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(groupcastRadius));
    charCount += sizeof(uint8_t);
  }

  return charCount;
}

uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_proxy_commissioning_mode_smart(uint8_t options,
                                                                                         uint16_t commissioningWindow,
                                                                                         uint8_t channel)
{
  // docs-14-0563-08: "In the current version of the GP specification,
  // the Channel present sub-field SHALL always be set to 0b0 and
  // the Channel field SHALL NOT be present."
  uint16_t charCount = 0;

  options &= ~SL_ZIGBEE_AF_GP_PROXY_COMMISSIONING_MODE_OPTION_CHANNEL_PRESENT;
  charCount += sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                                  | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
                                                  | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                                                 ZCL_GREEN_POWER_CLUSTER_ID,
                                                 ZCL_GP_PROXY_COMMISSIONING_MODE_COMMAND_ID,
                                                 "u",
                                                 options);

  if (options & SL_ZIGBEE_AF_GP_PROXY_COMMISSIONING_MODE_EXIT_MODE_ON_COMMISSIONING_WINDOW_EXPIRATION) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int16u_in_resp(commissioningWindow));
    charCount += sizeof(uint16_t);
  }

  if (options & SL_ZIGBEE_AF_GP_PROXY_COMMISSIONING_MODE_OPTION_CHANNEL_PRESENT) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(channel));
    charCount += sizeof(uint8_t);
  }

  return charCount;
}

uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_response_smart(uint8_t  options,
                                                                         uint16_t tempMasterShortAddress,
                                                                         uint8_t  tempMasterTxChannel,
                                                                         uint32_t gpdSrcId,
                                                                         uint8_t* gpdIeee,
                                                                         uint8_t  endpoint,
                                                                         uint8_t  gpdCommandId,
                                                                         uint8_t gpdCommandPayloadLength,
                                                                         uint8_t* gpdCommandPayload)
{
  sl_zigbee_gp_application_id_t appId = sl_zigbee_af_green_power_get_application_id(options);
  uint16_t charCount = 0;
  if ((appId != SL_ZIGBEE_GP_APPLICATION_SOURCE_ID)
      && (appId != SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }

  charCount += sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                                  | ZCL_DISABLE_DEFAULT_RESPONSE_MASK
                                                  | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT),
                                                 ZCL_GREEN_POWER_CLUSTER_ID,
                                                 ZCL_GP_RESPONSE_COMMAND_ID,
                                                 "uvu",
                                                 options,
                                                 tempMasterShortAddress,
                                                 tempMasterTxChannel);

  if (appId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int32u_in_resp(gpdSrcId));
    charCount += sizeof(uint32_t);
  } else if (appId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(gpdIeee, EUI64_SIZE));
    charCount += EUI64_SIZE;
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(endpoint));
    charCount += sizeof(uint8_t);
  }

  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(gpdCommandId));
  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(gpdCommandPayloadLength));
  sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(gpdCommandPayload, gpdCommandPayloadLength));
  charCount += sizeof(uint8_t) + gpdCommandPayloadLength + sizeof(uint8_t);

  return charCount;
}

uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_proxy_table_request_smart(uint8_t options,
                                                                                    uint32_t gpdSrcId,
                                                                                    uint8_t* gpdIeee,
                                                                                    uint8_t endpoint,
                                                                                    uint8_t index)
{
  (void)index;
  sl_zigbee_gp_application_id_t appId = sl_zigbee_af_green_power_get_application_id(options);
  uint8_t requestType = (options & SL_ZIGBEE_AF_GP_PROXY_TABLE_REQUEST_OPTIONS_REQUEST_TYPE) >> 3;
  uint16_t charCount = 0;
  if ((appId != SL_ZIGBEE_GP_APPLICATION_SOURCE_ID)
      && (appId != SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS)) {
    return 0;
  }

  charCount += sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                                  | ZCL_DISABLE_DEFAULT_RESPONSE_MASK
                                                  | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT),
                                                 ZCL_GREEN_POWER_CLUSTER_ID,
                                                 ZCL_GP_PROXY_TABLE_REQUEST_COMMAND_ID,
                                                 "u",
                                                 options);

  if (appId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int32u_in_resp(gpdSrcId));
    charCount += sizeof(uint32_t);
  } else if (appId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_block_in_resp(gpdIeee, EUI64_SIZE));
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(endpoint));
    charCount += EUI64_SIZE + sizeof(uint8_t);
  }

  if (requestType == 0x01) {
    sl_zigbee_af_green_power_check_return_of_put_data_in_response(sl_zigbee_af_put_int8u_in_resp(endpoint));
    charCount += sizeof(uint8_t);
  }

  return charCount;
}

/*
 * Comparing two GP address.
 *
 * return - true if addresses are the same. otherwise, false.
 *
 */
bool sl_zigbee_af_green_power_common_gp_addr_compare(const sl_zigbee_gp_address_t * a1,
                                                     const sl_zigbee_gp_address_t * a2)
{
#ifndef EZSP_HOST
  return sli_zigbee_gp_address_match(a1, a2);
#else
  if (a1->applicationId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID
      && a2->applicationId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
    if (a1->id.sourceId == a2->id.sourceId) {
      return true;
    }
  } else if (a1->applicationId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS
             && a2->applicationId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
    if (!memcmp(a1->id.gpdIeeeAddress, a2->id.gpdIeeeAddress, EUI64_SIZE)) {
      if (a1->endpoint == a2->endpoint
          || a1->endpoint == GREEN_POWER_SERVER_ALL_SINK_ENDPOINTS
          || a2->endpoint == GREEN_POWER_SERVER_ALL_SINK_ENDPOINTS
          || a1->endpoint == GREEN_POWER_SERVER_NO_PAIRED_ENDPOINTS
          || a2->endpoint == GREEN_POWER_SERVER_NO_PAIRED_ENDPOINTS) {
        return true;
      }
    }
  }
  return false;
#endif
}

bool sli_zigbee_af_gp_make_addr(sl_zigbee_gp_address_t *addr,
                                sl_zigbee_gp_application_id_t appId,
                                sl_zigbee_gp_source_id_t srcId,
                                uint8_t *gpdIeee,
                                uint8_t endpoint)
{
  if (addr == NULL
      || (appId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS
          && (endpoint > GREEN_POWER_SERVER_MAX_VALID_APP_ENDPOINT)
          && (endpoint < GREEN_POWER_SERVER_RAW_GPD_PROCESS_IN_APP_ENDPOINTS))) {
    return false;
  }

  if (appId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
    if (IS_RESERVED_GPD_SRC_ID(srcId)) {
      return false;
    }
    addr->id.sourceId = srcId;
  } else if (appId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS
             && gpdIeee != NULL) {
    if (((sl_zigbee_af_memory_byte_compare(gpdIeee, EUI64_SIZE, 0)))) {
      return false;
    }
    memcpy(addr->id.gpdIeeeAddress, gpdIeee, EUI64_SIZE);
    addr->endpoint = endpoint;
  } else {
    return false;
  }
  addr->applicationId = appId;
  return true;
}

void sli_zigbee_af_gp_spoof_device_annce(uint16_t nodeId,
                                         sl_802154_long_addr_t eui64,
                                         uint8_t capabilities)
{
  sl_zigbee_aps_frame_t apsFrameDevAnnce;
  apsFrameDevAnnce.sourceEndpoint = SL_ZIGBEE_ZDO_ENDPOINT;
  apsFrameDevAnnce.destinationEndpoint = SL_ZIGBEE_ZDO_ENDPOINT;
  apsFrameDevAnnce.clusterId = END_DEVICE_ANNOUNCE;
  apsFrameDevAnnce.profileId = SL_ZIGBEE_ZDO_PROFILE_ID;
  apsFrameDevAnnce.options = SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  apsFrameDevAnnce.options |= SL_ZIGBEE_APS_OPTION_USE_ALIAS_SEQUENCE_NUMBER;
  apsFrameDevAnnce.groupId = 0;
  uint8_t messageContents[GP_DEVICE_ANNOUNCE_SPOOF_MSG_SIZE];
  uint8_t apsSequence = 0;
  // Form the APS message for Bcast
  messageContents[0] = apsSequence; //Sequence
  messageContents[1] = (uint8_t)nodeId; //NodeId
  messageContents[2] = (uint8_t)(nodeId >> 8); //NodeId
  memcpy(&messageContents[3], eui64, EUI64_SIZE); //IEEE Address
  messageContents[11] = capabilities; // Capability
  sl_zigbee_send_broadcast(nodeId,//sl_802154_short_addr_t source,
                           0xFFFD, //sl_802154_short_addr_t destination,
                           0, //uint8_t nwkSequence,
                           &apsFrameDevAnnce, //sl_zigbee_aps_frame_t *apsFrame,
                           0xFF, //uint8_t radius,
                           0xFF, // Tag Id
                           sizeof(messageContents), //uint8_t messageLength,
                           messageContents, //uint8_t *messageContents,
                           &apsSequence);
}

sl_status_t sli_zigbee_af_gp_send_response_unicast(void)
{
  if (sl_zigbee_af_current_command()->interPanHeader == NULL) {
    sl_zigbee_af_response_destination = sl_zigbee_af_current_command()->source;
  }
  return sl_zigbee_af_send_response();
}
