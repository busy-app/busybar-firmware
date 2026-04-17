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

#ifndef _SILABS_GREEN_POWER_COMMON_H_
#define _SILABS_GREEN_POWER_COMMON_H_

/**
 * @defgroup green-power-common Green Power Common
 * @ingroup component
 * @brief API and Callbacks for the Green Power Common Component
 *
 * A component that provides common functionalities between client
 * and server sides of the Green Power components.
 */

/**
 * @addtogroup green-power-common
 * @{
 */
#define GREEN_POWER_SERVER_NO_PAIRED_ENDPOINTS                    0x00
#define GREEN_POWER_SERVER_RAW_GPD_PROCESS_IN_APP_ENDPOINTS       0xFD
#define GREEN_POWER_SERVER_RESERVED_ENDPOINTS                     0xFD
#define GREEN_POWER_SERVER_SINK_DERIVES_ENDPOINTS                 0xFE
#define GREEN_POWER_SERVER_ALL_SINK_ENDPOINTS                     0xFF

#define GREEN_POWER_SERVER_MIN_VALID_APP_ENDPOINT  1
#define GREEN_POWER_SERVER_MAX_VALID_APP_ENDPOINT  240

#define SL_ZIGBEE_AF_GP_GPD_CHANNEL_REQUEST_CHANNEL_TOGGLING_BEHAVIOR_RX_CHANNEL_NEXT_ATTEMPT (0x0F)
#define SL_ZIGBEE_AF_GP_GPD_CHANNEL_REQUEST_CHANNEL_TOGGLING_BEHAVIOR_RX_CHANNEL_SECOND_NEXT_ATTEMPT (0xF0)
#define SL_ZIGBEE_AF_GP_GPD_CHANNEL_REQUEST_CHANNEL_TOGGLING_BEHAVIOR_RX_CHANNEL_SECOND_NEXT_ATTEMPT_OFFSET (4)

#define SL_ZIGBEE_AF_GP_GPD_APPLICATION_DESCRIPTION_COMMAND_REPORT_OPTIONS_TIMEOUT_PERIOD_PRESENT           (0x01)
#define SL_ZIGBEE_AF_GP_TRANSLATION_TABLE_ZB_ENDPOINT_PASS_FRAME_TO_APLLICATION (0xFC)

#define SL_ZIGBEE_AF_GP_SINK_TABLE_ENTRY_OPTIONS_MASK                      (0x03FF)

// bitmap of how the translation table is scanned when a gpd entry
// is search into it
#define GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_ID                          (0x01)
#define GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_CMD_ID                      (0x02)
#define GP_TRANSLATION_TABLE_SCAN_LEVEL_GPD_PAYLOAD                     (0x04)
#define GP_TRANSLATION_TABLE_SCAN_LEVEL_ZB_ENDPOINT                     (0x08)
#define GP_TRANSLATION_TABLE_SCAN_LEVEL_ADDITIONAL_INFO_BLOCK           (0x10)

#define SL_ZIGBEE_GP_NOTIFICATION_COMMISSIONED_GROUPCAST_SEQUENCE_NUMBER_OFFSET  (9)

#define SL_ZIGBEE_AF_GP_GPP_GPD_LINK_RSSI         (0x3F)
#define SL_ZIGBEE_AF_GP_GPP_GPD_LINK_LINK_QUALITY (0xC0)
#define SL_ZIGBEE_AF_GP_GPP_GPD_LINK_LINK_QUALITY_OFFSET (6)

#define SL_ZIGBEE_AF_GP_PAIRING_CONFIGURATION_ACTIONS_MASK          (0x0F)
#define SL_ZIGBEE_AF_GP_PAIRING_CONFIGURATION_OPTION_MASK           (0x07FF)

#define SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_NUMBER_OF_SERVER_CLUSTER_MASK (0x0F)
#define SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_NUMBER_OF_CLIENT_CLUSTER_MASK (0xF0)
#define SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_CLUSTER_LIST_NUMBER_OF_CLIENT_CLUSTER_MASK_OFFSET (4)

#define SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_GPD_APPLICATION_DESCRIPTION_COMMAND_FOLLOWS (0x20)
#define SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_GPD_APPLICATION_DESCRIPTION_COMMAND_FOLLOWS_OFFSET (5)

#define SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_SWITCH_INFORMATION_CONFIGURATION_NB_OF_CONTACT   (0x0F)
#define SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_SWITCH_INFORMATION_CONFIGURATION_SWITCH_TYPE     (0x30)
#define SL_ZIGBEE_AF_GP_APPLICATION_INFORMATION_SWITCH_INFORMATION_CONFIGURATION_SWITCH_TYPE_OFFSET (4)

#define GP_COMMISSIONING_SECURITY_LEVEL_TO_OPTIONS_SHIFT (4)
#define GP_COMMISSIONING_SECURITY_KEY_TYPE_TO_OPTIONS_SHIFT (6)
#define SL_ZIGBEE_GP_NOTIFICATION_COMMISSIONED_GROUPCAST_SEQUENCE_NUMBER_OFFSET (9)
#define SL_ZIGBEE_GP_COMMISSIONING_NOTIFICATION_SEQUENCE_NUMBER_OFFSET (12)

#define SL_ZIGBEE_AF_GREEN_POWER_SERVER_GPDF_SCENE_DERIVED_GROUP_ID (0xFFFF)

#define GP_DEVICE_ANNOUNCE_SPOOF_MSG_SIZE 12

/** @brief If GPD source ID is reserved */
#define IS_RESERVED_GPD_SRC_ID(srcId)       (((srcId) >=  GP_GPD_SRC_ID_RESERVED_FFFFFF9) \
                                             && ((srcId) <= GP_GPD_SRC_ID_RESERVED_FFFFFFE))
/** @brief If GPD source ID is 0 */
#define IS_GPD_SRC_ID_ZERO(appId, srcId) (((appId) == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) \
                                          && ((srcId) == GP_GPD_SRC_ID_RESERVED_0))

/** @brief Translation Table command - Options field: "Additional information block present" flag */
#define sl_zigbee_af_green_power_t_t_get_additional_info_block_present(options) ((uint8_t) (options & 0x08))
/** @brief Translation Table Update command - Options field: "Additional information block present" flag */
#define sl_zigbee_af_green_power_t_t_update_get_additional_info_block_present(options) ((uint16_t) (options & 0x0100))
/** @brief Pairing Configuration command - Options field: "Application information present" flag */
#define sl_zigbee_af_green_power_pairing_config_get_application_info_present(options) ((uint16_t) (options & 0x0400))
/** @brief Pairing command - Options field: "Communication Mode" value */
#define sl_zigbee_af_green_power_pairing_options_get_comm_mode(options) (((options) & SL_ZIGBEE_AF_GP_PAIRING_OPTION_COMMUNICATION_MODE) >> 5)
/** @brief Pairing command - Options field: "Add Sink" value */
#define sl_zigbee_af_green_power_pairing_options_get_add_sink(options) (((options) & SL_ZIGBEE_AF_GP_PAIRING_OPTION_ADD_SINK) >> 3)
/** @brief Any command - Options field: "Application ID" value */
#define sl_zigbee_af_green_power_get_application_id(options) ((sl_zigbee_gp_application_id_t) ((options) & 0x07))

/** @brief Proxy table entry - Sequence number capability */
#define sl_zigbee_af_green_power_gpd_seq_num_cap(entry)      ((entry->options >> 8) & 0x01)
/** @brief Proxy table entry - Security level */
#define sl_zigbee_af_green_power_security_level(entry)     ((entry->options >> 9) & 0x03)

// If the value of this sub-field is 0b1, then the GPD is not expected to change
// its position during its operation in the network.
/** @brief If GPD has a fixed location during operation */
#define sl_zigbee_af_green_power_fixed_during_operation(options) ((((options) & GP_PAIRING_OPTIONS_GPD_FIXED) >> SL_ZIGBEE_AF_GP_PAIRING_OPTION_GPD_FIXED_OFFSET) & 0x01)
/** @brief If GPD does not have a fixed location during operation */
#define sl_zigbee_af_green_power_mobile_cap(options)          (!((((options) & GP_PAIRING_OPTIONS_GPD_FIXED) >> SL_ZIGBEE_AF_GP_PAIRING_OPTION_GPD_FIXED_OFFSET) & 0x01))
/** @brief If GPD has a fixed location during operation */
#define sl_zigbee_af_green_power_portable_cap(options)     ((((options) & GP_PAIRING_OPTIONS_GPD_FIXED) >> 7) & 0x01)
/** @brief Any command - Options field: "Security key type" value */
#define sl_zigbee_af_green_power_security_key_type(options) ((((options) & GP_PAIRING_OPTIONS_SECURITY_KEY_TYPE) >> 11) & 0x07)

#define sl_zigbee_af_green_power_check_return_of_put_data_in_response(ret) \
  ({  if ((ret) == NULL) {                                                 \
        return 0;                                                          \
      }                                                                    \
   })                                                                      \

/** @brief Green Power link quality */
typedef enum {
  SL_ZIGBEE_ZCL_GP_GPD_GPP_LINK_QUALITY_POOR       = 0x00, /**< Poor link quality */
  SL_ZIGBEE_ZCL_GP_GPD_GPP_LINK_QUALITY_MODERATE   = 0x01, /**< Moderate link quality */
  SL_ZIGBEE_ZCL_GP_GPD_GPP_LINK_QUALITY_HIGH       = 0x02, /**< High link quality */
  SL_ZIGBEE_ZCL_GP_GPD_GPP_LINK_QUALITY_EXCELLENT  = 0x03, /**< Excellent link quality */
}sl_zigbee_af_g_p_gpd_gpp_link_quality_t;

/** @brief Green Power switch type */
typedef enum {
  SL_ZIGBEE_ZCL_GP_UNKNOWN_SWITCH_TYPE        = 0x00, /**< Unknown switch */
  SL_ZIGBEE_ZCL_GP_BUTTON_SWITCH_TYPE         = 0x01, /**< Button switch */
  SL_ZIGBEE_ZCL_GP_ROCKER_SWITCH_TYPE         = 0x02, /**< Rocker switch */
  SL_ZIGBEE_ZCL_GP_RESERVED_SWITCH_TYPE       = 0x03, /**< Reserved switch */
} sl_zigbee_af_g_p_generic_sw_switch_type_t;

/** @brief Green Power device ID */
typedef enum {
  SL_ZIGBEE_GP_DEVICE_ID_GPD_SIMPLE_GENERIC_ONE_STATE_SWITCH   = 0x00, /**< Simple generic one-state switch */
  SL_ZIGBEE_GP_DEVICE_ID_GPD_SIMPLE_GENERIC_TWO_STATE_SWITCH   = 0x01, /**< Simple generic two-state switch */
  SL_ZIGBEE_GP_DEVICE_ID_GPD_ON_OFF_SWITCH                     = 0x02, /**< On-off switch */
  SL_ZIGBEE_GP_DEVICE_ID_GPD_LEVEL_CONTROL_SWITCH              = 0x03, /**< Level control switch */
  SL_ZIGBEE_GP_DEVICE_ID_GPD_SIMPLE_SENSOR_SWITCH              = 0x04, /**< Simple sensor switch */
  SL_ZIGBEE_GP_DEVICE_ID_GPD_ADVANCED_GENERIC_ONE_STATE_SWITCH = 0x05, /**< Advanced generic one-state switch */
  SL_ZIGBEE_GP_DEVICE_ID_GPD_ADVANCED_GENERIC_TWO_STATE_SWITCH = 0x06, /**< Advanced generic two-state switch */
  SL_ZIGBEE_GP_DEVICE_ID_GPD_GENERIC_SWITCH                    = 0x07, /**< Generic switch */
  SL_ZIGBEE_GP_DEVICE_ID_GPD_COLOR_DIMMER_SWITCH               = 0x10, /**< Color dimmer switch */
  SL_ZIGBEE_GP_DEVICE_ID_GPD_LIGHT_SENSOR_SWITCH               = 0x11, /**< Light sensor switch */
  SL_ZIGBEE_GP_DEVICE_ID_GPD_OCCUPANCY_SENSOR_SWITCH           = 0x12, /**< Occupancy sensor switch */
  SL_ZIGBEE_GP_DEVICE_ID_GPD_DOOR_LOCK_CONTROLLER_SWITCH       = 0x20, /**< Door lock controller switch */
  SL_ZIGBEE_GP_DEVICE_ID_GPD_TEMPERATURE_SENSOR_SWITCH         = 0x30, /**< Temperature sensor switch */
  SL_ZIGBEE_GP_DEVICE_ID_GPD_PRESSURE_SENSOR_SWITCH            = 0x31, /**< Pressure sensor switch */
  SL_ZIGBEE_GP_DEVICE_ID_GPD_FLOW_SENSOR_SWITCH                = 0x32, /**< Flow sensor switch */
  SL_ZIGBEE_GP_DEVICE_ID_GPD_INDOOR_ENVIRONMENT_SENSOR         = 0x33, /**< Indoor environment sensor */
  SL_ZIGBEE_GP_DEVICE_ID_GPD_SCENCES                           = 0xFE, /**< Scenes */
  SL_ZIGBEE_GP_DEVICE_ID_GPD_UNDEFINED                         = 0xFE, /**< Undefined */
} sl_zigbee_af_gp_device_id_gpd_t;

/**
 * @name API
 * @{
 */

/** @brief Prepare a GP proxy commissioning mode command buffer.
 *
 * This function prepares an application framework ZCL command buffer for the GP
 * proxy table response command with supplied arguments and returns the buffer length.
 *
 * @param[in] status Status of the GP table response
 * @param[in] totalNumberOfNonEmptyProxyTableEntries Total number of entries in the proxy table
 * @param[in] startIndex Start index in the response
 * @param[in] entriesCount Number of entries in the response
 *
 * @returns Length of the constructed command buffer
 */
#define sl_zigbee_af_fill_command_green_power_cluster_gp_proxy_table_response_smart(status,                                 \
                                                                                    totalNumberOfNonEmptyProxyTableEntries, \
                                                                                    startIndex,                             \
                                                                                    entriesCount)                           \
  sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND                                                           \
                                     | ZCL_DISABLE_DEFAULT_RESPONSE_MASK                                                    \
                                     | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),                                                 \
                                    ZCL_GREEN_POWER_CLUSTER_ID,                                                             \
                                    ZCL_GP_PROXY_TABLE_RESPONSE_COMMAND_ID,                                                 \
                                    "uuuu",                                                                                 \
                                    status,                                                                                 \
                                    totalNumberOfNonEmptyProxyTableEntries,                                                 \
                                    startIndex,                                                                             \
                                    entriesCount)

/** @brief Prepare a GP sink table response command buffer.
 *
 * This function prepares an application framework ZCL command buffer for the GP
 * sink table response command with supplied arguments and returns the buffer length.
 *
 * @param[in] status Sink table response status
 * @param[in] totalNumberofNonEmptySinkTableEntries Total number of entries in the sink table
 * @param[in] startIndex Start index in the response
 * @param[in] sinkTableEntriesCount Number of entries in the response
 *
 * @returns Length of the constructed command buffer
 */
#define sl_zigbee_af_fill_command_green_power_cluster_gp_sink_table_response_smart(status,                                \
                                                                                   totalNumberofNonEmptySinkTableEntries, \
                                                                                   startIndex,                            \
                                                                                   sinkTableEntriesCount)                 \
  sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND                                                         \
                                     | ZCL_DISABLE_DEFAULT_RESPONSE_MASK                                                  \
                                     | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT),                                               \
                                    ZCL_GREEN_POWER_CLUSTER_ID,                                                           \
                                    ZCL_GP_SINK_TABLE_RESPONSE_COMMAND_ID,                                                \
                                    "uuuu",                                                                               \
                                    status,                                                                               \
                                    totalNumberofNonEmptySinkTableEntries,                                                \
                                    startIndex,                                                                           \
                                    sinkTableEntriesCount)

/** @brief To provide GPD Command Translation Table content.
 *
 * Cluster: Green Power, The Green Power cluster defines the format of the commands exchanged when handling GPDs.
 * Command: GpTranslationTableResponse
 * @param[in] status GPD Command Translation Table status
 * @param[in] options Options
 * @param[in] totalNumberOfEntries Total number of entries
 * @param[in] startIndex Start index
 * @param[in] entriesCount Number of entries
 * @param[in] translationTableList Translation table list
 * @param[in] translationTableListLen  Translation table length
 */
#define sl_zigbee_af_fill_command_green_power_cluster_gp_translation_table_response_smart(status,                  \
                                                                                          options,                 \
                                                                                          totalNumberOfEntries,    \
                                                                                          startIndex,              \
                                                                                          entriesCount,            \
                                                                                          translationTableList,    \
                                                                                          translationTableListLen) \
  sl_zigbee_af_fill_external_buffer((ZCL_CLUSTER_SPECIFIC_COMMAND                                                  \
                                     | ZCL_DISABLE_DEFAULT_RESPONSE_MASK                                           \
                                     | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT),                                        \
                                    ZCL_GREEN_POWER_CLUSTER_ID,                                                    \
                                    ZCL_GP_TRANSLATION_TABLE_RESPONSE_COMMAND_ID,                                  \
                                    "uuuuub",                                                                      \
                                    status,                                                                        \
                                    options,                                                                       \
                                    totalNumberOfEntries,                                                          \
                                    startIndex,                                                                    \
                                    entriesCount,                                                                  \
                                    translationTableList,                                                          \
                                    translationTableListLen)

/** @brief Prepare a GP notification command buffer.
 *
 * This function prepares an application framework ZCL command buffer for the GP
 * notification command with supplied arguments and returns the buffer length.
 *
 * @param[in] options Options
 * @param[in] gpdSrcId GPD Source Id
 * @param[in] gpdIeee GPD IEEE address
 * @param[in] gpdEndpoint GPD endpoint
 * @param[in] gpdSecurityFrameCounter GPD security frame counter
 * @param[in] gpdCommandId GPD command Id
 * @param[in] gpdCommandPayloadLength Command payload length
 * @param[in] gpdCommandPayload GPD command payload
 * @param[in] gppShortAddress GP Proxy short address
 * @param[in] gppDistance GP Proxy distance
 *
 * @returns Length of the constructed command buffer
 */
uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_notification_smart(uint16_t options,
                                                                             uint32_t gpdSrcId,
                                                                             uint8_t* gpdIeee,
                                                                             uint8_t  gpdEndpoint,
                                                                             uint32_t gpdSecurityFrameCounter,
                                                                             uint8_t  gpdCommandId,
                                                                             uint8_t gpdCommandPayloadLength,
                                                                             const uint8_t* gpdCommandPayload,
                                                                             uint16_t gppShortAddress,
                                                                             uint8_t  gppDistance);
/** @brief Prepare a GP pairing search command buffer.
 *
 * This function prepares an application framework ZCL command buffer for the GP
 * pairing search command with supplied arguments and returns the buffer length.
 *
 * @param[in] options Options
 * @param[in] gpdSrcId GPD Source Id
 * @param[in] gpdIeee GPD IEEE address
 * @param[in] gpdEndpoint GPD endpoint
 *
 * @returns Length of the constructed command buffer
 */
uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_pairing_search_smart(uint16_t options,
                                                                               uint32_t gpdSrcId,
                                                                               uint8_t* gpdIeee,
                                                                               uint8_t gpdEndpoint);
/** @brief Prepare a GP tunneling stop command buffer.
 *
 * This function prepares an application framework ZCL command buffer for the GP
 * tunneling stop command with supplied arguments and returns the buffer length.
 *
 * @param[in] options Options
 * @param[in] gpdSrcId GPD Source Id
 * @param[in] gpdIeee GPD IEEE address
 * @param[in] gpdEndpoint GPD endpoint
 * @param[in] gpdSecurityFrameCounter GPD security frame counter
 * @param[in] gppShortAddress GP Proxy short address
 * @param[in] gppDistance GP Proxy distance
 *
 * @returns Length of the constructed command buffer
 */
uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_tunneling_stop_smart(uint8_t options,
                                                                               uint32_t gpdSrcId,
                                                                               uint8_t* gpdIeee,
                                                                               uint8_t gpdEndpoint,
                                                                               uint32_t gpdSecurityFrameCounter,
                                                                               uint16_t gppShortAddress,
                                                                               int8_t gppDistance);
/** @brief Prepare a GP commissioning notification command buffer.
 *
 * This function prepares an application framework ZCL command buffer for the GP
 * commissioning notification command with supplied arguments and returns the
 * buffer length.
 *
 * @param[in] options Options
 * @param[in] gpdSrcId GPD Source Id
 * @param[in] gpdIeee GPD IEEE address
 * @param[in] gpdEndpoint GPD endpoint
 * @param[in] sequenceNumber MAC Sequence to be used in security counter field for GPD security level 0
 * @param[in] gpdfSecurityLevel GPD security level
 * @param[in] gpdSecurityFrameCounter GPD security frame counter
 * @param[in] gpdCommandId GPD command Id
 * @param[in] gpdCommandPayloadLength Command payload length
 * @param[in] gpdCommandPayload GPD command payload
 * @param[in] gppShortAddress GP Proxy short address
 * @param[in] gppLink GPD-GP Proxy link
 * @param[in] mic Message Integrity Code when security level is 2 or 3
 *
 * @returns Length of the constructed command buffer
 */
uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_commissioning_notification_smart(uint16_t options,
                                                                                           uint32_t gpdSrcId,
                                                                                           uint8_t* gpdIeee,
                                                                                           uint8_t gpdEndpoint,
                                                                                           uint8_t sequenceNumber,
                                                                                           sl_zigbee_gp_security_level_t gpdfSecurityLevel,
                                                                                           sl_zigbee_gp_security_frame_counter_t gpdSecurityFrameCounter,
                                                                                           uint8_t gpdCommandId,
                                                                                           uint8_t gpdCommandPayloadLength,
                                                                                           const uint8_t *gpdCommandPayload,
                                                                                           sl_802154_short_addr_t gppShortAddress,
                                                                                           uint8_t gppLink,
                                                                                           sl_zigbee_gp_mic_t mic);
/** @brief Prepare a GP translation table update command buffer.
 *
 * This function prepares an application framework ZCL command buffer for the GP
 * translation table update command with supplied arguments and returns the
 * buffer length.
 *
 * @param[in] options Options
 * @param[in] gpdSrcId GPD Source Id
 * @param[in] gpdIeee GPD IEEE address
 * @param[in] gpdEndpoint GPD endpoint
 * @param[in] translationsLength Length of the translations packet
 * @param[in] translations One or more number of translations
 * @param[in] additionnalInfoBlock Additional Information block used along with the translations
 *
 * @returns Length of the constructed command buffer
 */
uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_translation_table_update_smart(uint16_t options,
                                                                                         uint32_t gpdSrcId,
                                                                                         uint8_t* gpdIeee,
                                                                                         uint8_t gpdEndpoint,
                                                                                         uint8_t translationsLength,
                                                                                         sl_zigbee_zcl_gp_translation_table_update_translation_t* translations,
                                                                                         sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t* additionnalInfoBlock);
/** @brief Prepare a GP pairing configuration command buffer.
 *
 * This function prepares an application framework ZCL command buffer for the GP
 * pairing configuration command with supplied arguments and returns the
 * buffer length.
 *
 * @param[in] actions GP Pairing actions field
 * @param[in] options Options
 * @param[in] gpdSrcId GPD Source Id
 * @param[in] gpdIeee GPD IEEE address
 * @param[in] gpdEndpoint GPD endpoint
 * @param[in] gpdDeviceId GPD Device Id
 * @param[in] groupListCount Group list count
 * @param[in] groupList Group list
 * @param[in] gpdAssignedAlias GPD assigned alias
 * @param[in] groupcastRadius Group cast radius
 * @param[in] securityOptions Security options
 * @param[in] gpdSecurityFrameCounter Security frame counter
 * @param[in] gpdSecurityKey Security key
 * @param[in] numberOfPairedEndpoints Number of paired endpoints
 * @param[in] pairedEndpoints Paired endpoint list
 * @param[in] applicationInformation Application information field
 * @param[in] manufacturerId GPD Manufacture Id
 * @param[in] modeId GPD Model Id
 * @param[in] numberOfGpdCommands Number of GPD commands
 * @param[in] gpdCommandIdList GPD commands list
 * @param[in] clusterIdListCount Cluster List count
 * @param[in] clusterListServer Server cluster list
 * @param[in] clusterListClient Client cluster list
 * @param[in] switchInformationLength Generic switch information length
 * @param[in] genericSwitchConfiguration Generic switch configuration
 * @param[in] currentContactStatus Current contact status
 * @param[in] totalNumberOfReports Total number of reports configured
 * @param[in] numberOfReports Number of reports in the report present in descriptor
 * @param[in] reportDescriptor Report descriptors list
 *
 * @returns Length of the constructed command buffer
 */
uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_pairing_configuration_smart(uint8_t actions,
                                                                                      uint16_t options,
                                                                                      uint32_t gpdSrcId,
                                                                                      uint8_t* gpdIeee,
                                                                                      uint8_t gpdEndpoint,
                                                                                      uint8_t gpdDeviceId,
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
                                                                                      uint8_t* reportDescriptor);
/** @brief Prepare a GP sink table request command buffer.
 *
 * This function prepares an application framework ZCL command buffer for the GP
 * sink table request command with supplied arguments and returns the
 * buffer length.
 *
 * @param[in] options Options
 * @param[in] gpdSrcId GPD Source Id
 * @param[in] gpdIeee GPD IEEE address
 * @param[in] gpdEndpoint GPD endpoint
 * @param[in] index Requested table index to start
 *
 * @returns Length of the constructed command buffer
 */
uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_sink_table_request_smart(uint8_t options,
                                                                                   uint32_t gpdSrcId,
                                                                                   uint8_t* gpdIeee,
                                                                                   uint8_t gpdEndpoint,
                                                                                   uint8_t index);
/** @brief Prepare a GP notification response command buffer.
 *
 * This function prepares an application framework ZCL command buffer for the GP
 * notification response command with supplied arguments and returns the
 * buffer length.
 *
 * @param[in] options Options
 * @param[in] gpdSrcId GPD Source Id
 * @param[in] gpdIeee GPD IEEE address
 * @param[in] gpdEndpoint GPD endpoint
 * @param[in] gpdSecurityFrameCounter GPD security frame counter
 *
 * @returns Length of the constructed command buffer
 */
uint32_t sl_zigbee_af_fill_command_green_power_cluster_gp_notification_response_smart(uint8_t options,
                                                                                      uint32_t gpdSrcId,
                                                                                      uint8_t* gpdIeee,
                                                                                      uint8_t gpdEndpoint,
                                                                                      uint32_t gpdSecurityFrameCounter);
/** @brief Prepare a GP pairing command buffer.
 *
 * This function prepares an application framework ZCL command buffer for the GP
 * pairing command with supplied arguments and returns the buffer length.
 *
 * @param[in] options Options
 * @param[in] gpdSrcId GPD Source Id
 * @param[in] gpdIeee GPD IEEE address
 * @param[in] gpdEndpoint GPD endpoint
 * @param[in] sinkIeeeAddress Sink IEEE address
 * @param[in] sinkNwkAddress Sink network address
 * @param[in] sinkGroupId Sink group Id
 * @param[in] deviceId GPD Device Id
 * @param[in] gpdSecurityFrameCounter GPD security frame counter
 * @param[in] gpdKey GPD security key
 * @param[in] assignedAlias GPD assigned alias
 * @param[in] groupcastRadius Group cast radius of this message
 *
 * @returns Length of the constructed command buffer
 */
uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_pairing_smart(uint32_t options,
                                                                        uint32_t gpdSrcId,
                                                                        uint8_t* gpdIeee,
                                                                        uint8_t gpdEndpoint,
                                                                        uint8_t* sinkIeeeAddress,
                                                                        uint16_t sinkNwkAddress,
                                                                        uint16_t sinkGroupId,
                                                                        uint8_t deviceId,
                                                                        uint32_t gpdSecurityFrameCounter,
                                                                        uint8_t* gpdKey,
                                                                        uint16_t assignedAlias,
                                                                        uint8_t groupcastRadius);
/** @brief Prepare a GP proxy commissioning mode command buffer.
 *
 * This function prepares an application framework ZCL command buffer for the GP
 * proxy commissioning mode command with supplied arguments and returns the buffer length.
 *
 * @param[in] options Options
 * @param[in] commissioningWindow Commissioning window in seconds
 * @param[in] channel Proxy channel field
 *
 * @returns Length of the constructed command buffer
 */
uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_proxy_commissioning_mode_smart(uint8_t options,
                                                                                         uint16_t commissioningWindow,
                                                                                         uint8_t channel);
/** @brief Prepare a GP response command buffer.
 *
 * This function prepares an application framework ZCL command buffer for the GP
 * response command with supplied arguments and returns the buffer length.
 *
 * @param[in] options Options
 * @param[in] tempMasterShortAddress Proxy Temp Master short address
 * @param[in] tempMasterTxChannel Proxy Temp Master transmit channel
 * @param[in] gpdSrcId GPD Source Id
 * @param[in] gpdIeee GPD IEEE address
 * @param[in] gpdEndpoint GPD endpoint
 * @param[in] gpdCommandId GPD command id
 * @param[in] gpdCommandPayloadLength GPD command length
 * @param[in] gpdCommandPayload GPD command payload
 *
 * @returns Length of the constructed command buffer
 */
uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_response_smart(uint8_t  options,
                                                                         uint16_t tempMasterShortAddress,
                                                                         uint8_t  tempMasterTxChannel,
                                                                         uint32_t gpdSrcId,
                                                                         uint8_t* gpdIeee,
                                                                         uint8_t  gpdEndpoint,
                                                                         uint8_t  gpdCommandId,
                                                                         uint8_t gpdCommandPayloadLength,
                                                                         uint8_t* gpdCommandPayload);
/** @brief Prepare a GP proxy table request command buffer.
 *
 * This function prepares an application framework ZCL command buffer for the GP
 * proxy table request command with supplied arguments and returns the buffer length.
 *
 * @param[in] options Options
 * @param[in] gpdSrcId GPD Source Id
 * @param[in] gpdIeee GPD IEEE address
 * @param[in] gpdEndpoint GPD endpoint
 * @param[in] index Requested table index to start
 *
 * @returns Length of the constructed command buffer
 */
uint16_t sl_zigbee_af_fill_command_green_power_cluster_gp_proxy_table_request_smart(uint8_t options,
                                                                                    uint32_t gpdSrcId,
                                                                                    uint8_t* gpdIeee,
                                                                                    uint8_t gpdEndpoint,
                                                                                    uint8_t index);
/** @brief Compare two GP Addresses.
 *
 * This function compares two GP Addresses and returns true if they are the same.
 *
 * @param[in] a1 First address to compare
 * @param[in] a2 Second address to compare
 *
 * @returns True if the address are same
 */
bool sl_zigbee_af_green_power_common_gp_addr_compare(const sl_zigbee_gp_address_t * a1,
                                                     const sl_zigbee_gp_address_t * a2);

/** @} */ // end of name API
/** @} */ // end of green-power-common

sl_802154_short_addr_t sli_zigbee_gpd_alias(sl_zigbee_gp_address_t *addr);

bool sli_zigbee_af_gp_make_addr(sl_zigbee_gp_address_t *addr,
                                sl_zigbee_gp_application_id_t appId,
                                sl_zigbee_gp_source_id_t srcId,
                                uint8_t *gpdIeee,
                                uint8_t endpoint);
uint16_t sli_zigbee_af_copy_additional_info_block_structure_to_array(uint8_t commandId,
                                                                     sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t *additionalInfoBlockIn,
                                                                     uint8_t *additionalInfoBlockOut);
void sli_zigbee_af_gp_spoof_device_annce(uint16_t nodeId,
                                         sl_802154_long_addr_t eui64,
                                         uint8_t capabilities);

sl_status_t sli_zigbee_af_gp_send_response_unicast(void);
#endif //_GREEN_POWER_COMMON_H_
