/***************************************************************************//**
 * @file
 * @brief APIs and defines for the Green Power Translation Table plugin.
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

#ifndef _SILABS_GREEN_POWER_TRANSLATION_TABLE_H_
#define _SILABS_GREEN_POWER_TRANSLATION_TABLE_H_

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
// This translation table header is included by the green power server to perform default
// command translation, even without the translation table component. Hence the
// the following configuration header inclusion is conditionalized.
#ifdef SL_CATALOG_ZIGBEE_GREEN_POWER_TRANSLATION_TABLE_PRESENT
#include "green-power-translation-table-config.h"
#endif // SL_CATALOG_ZIGBEE_GREEN_POWER_TRANSLATION_TABLE_PRESENT

/**
 * @defgroup green-power-translation-table Green Power Translation Table
 * @ingroup component
 * @brief API and Callbacks for the Green Power Translation Table Component
 *
 * A component implementing the translation table functionality of the Green Power cluster.
 *
 */

/**
 * @addtogroup green-power-translation-table
 * @{
 */
#define SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE

#define SL_ZIGBEE_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN (7)

// defines macros for GPD To ZCL CMD Mapping Payload SRC
#define  SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_PRECONFIGURED 1
#define  SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_NA            2
#define  SL_ZIGBEE_AF_GREEN_POWER_ZCL_PAYLOAD_SRC_GPD_CMD       4

#define GREEN_POWER_SERVER_GPD_COMMAND_PAYLOAD_UNSPECIFIED_LENGTH 0xFF
#define GREEN_POWER_SERVER_GPD_COMMAND_PAYLOAD_TT_DERIVED_LENGTH  0xFE

#define GP_TRANSLATION_TABLE_STATUS_SUCCESS                0x00
#define GP_TRANSLATION_TABLE_STATUS_FAILED                 0xFF
#define GP_TRANSLATION_TABLE_STATUS_FULL                   0xFE
#define GP_TRANSLATION_TABLE_STATUS_EMPTY                  0xFD
#define GP_TRANSLATION_TABLE_STATUS_ENTRY_EMPTY            0xFC
#define GP_TRANSLATION_TABLE_STATUS_ENTRY_NOT_EMPTY        0xFB
#define GP_TRANSLATION_TABLE_STATUS_PARAM_DOES_NOT_MATCH   0xFA
#define GP_TRANSLATION_TABLE_STATUS_CUSTOMIZED_TABLE_FULL  0xF9

#define GP_TRANSLATION_TABLE_ENTRY_INVALID_INDEX 0xFF

typedef uint8_t sli_zigbee_gp_table_type_t;
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum sli_zigbee_af_gp_table_type
#else
enum
#endif //DOXYGEN_SHOULD_SKIP_THIS
{
  NO_ENTRY,
  DEFAULT_TABLE_ENTRY,
  CUSTOMIZED_TABLE_ENTRY,
}; //sli_zigbee_af_gp_table_type

#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum IncomingReqType
#else
enum
#endif //DOXYGEN_SHOULD_SKIP_THIS
{
  ADD_PAIRED_DEVICE = 1,
  DELETE_PAIRED_DEVICE = 1,
  TRANSLATION_TABLE_UPDATE = 2,
}; //IncommingReqType

typedef struct {
  bool            validEntry;
  uint8_t         gpdCommand;
  uint8_t         endpoint;
  uint16_t        zigbeeProfile;
  uint16_t        zigbeeCluster;
  uint8_t         serverClient;
  uint8_t         zigbeeCommandId;
  uint8_t         payloadSrc;

  // This is a Zigbee Octate String, fist byte is length sub-field followed by payload.
  // The length sub-field has following special meanings
  // 1. If the Length sub-field of the ZigBee Command payload field is set to 0x00,
  //    the Payload sub-field is not present, and the ZigBee command is sent without payload.
  // 2. If the Length sub-field of the ZigBee Command payload field is set to 0xff,
  //    the Payload sub-field is not present, and the payload from the triggering
  //    GPD command is to be copied verbatim into the ZigBee command.
  // 3. If the Length sub-field of the ZigBee Command payload field is set to 0xfe,
  //    the Payload sub-field is not present, and the payload from the triggering GPD command
  //    needs to be parsed. For all other values of the Length sub-field,
  uint8_t zclPayloadDefault[SL_ZIGBEE_AF_GREEN_POWER_SERVER_TRANSLATION_TABLE_ENTRY_ZCL_PAYLOAD_LEN];
} sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t;

// The following declaration/definitions are only needed if the translation table is installed
// by the user.
#ifdef SL_CATALOG_ZIGBEE_GREEN_POWER_TRANSLATION_TABLE_PRESENT
typedef struct {
  uint8_t applicationId;
  uint8_t additionalInfoBlockPresent;
} sli_zigbee_gp_translation_table_option_field_t;

typedef struct {
  uint8_t       switchType;
  uint8_t       nbOfIdentifiedContacts;
  uint8_t       nbOfTTEntriesNeeded;
  uint8_t       indicativeBitmask;
} sli_zigbee_af_gp_switch_type_data;

typedef struct {
  bool            infoBlockPresent;
  uint8_t         gpdCommand;
  uint8_t         zbEndpoint;
  uint8_t         offset;
  sli_zigbee_gp_table_type_t     entry;
  sl_zigbee_gp_address_t  gpAddr;
  sl_zigbee_gp_application_id_t gpApplicationId;
  uint8_t         additionalInfoOffset;
} sli_zigbee_af_gp_command_translation_table_entry;

typedef struct {
  uint8_t       endpoint;
  uint8_t       gpdCommand;
  uint16_t      zigbeeProfile;
} sl_zigbee_af_green_power_server_multi_sensor_translation_t;

typedef struct {
  sli_zigbee_af_gp_command_translation_table_entry TableEntry[SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_TRANSLATION_TABLE_TRANSLATION_TABLE_SIZE];
  uint8_t totalNoOfEntries;
} sli_zigbee_af_gp_command_translation_table;

typedef struct {
  sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t additionalInfoBlock[SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE];
  uint8_t validEntry[SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_ADDITIONALINFO_TABLE_SIZE];
  uint8_t totlaNoOfEntries;
} sl_zigbee_gp_translation_table_additional_info_block_field_t;

typedef struct {
  sli_zigbee_af_gp_switch_type_data  SwitchType;
  sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t genericSwitchDefaultTableEntry;
} sl_zigbee_af_green_power_server_defaut_generic_sw_translation_t;

/**
 * @name Callbacks
 * @{
 */

/**
 * @defgroup gp_translation_table_cb Green Power Translation Table
 * @ingroup af_callback
 * @brief Callbacks for Green Power Translation Table Component
 *
 */

/**
 * @addtogroup gp_translation_table_cb
 * @{
 */

/** @brief Green power server translation table init callback.
 *
 * This function is called by the green power server translation table plugin
 * upon initializing the translation tables.
 *
 */
void sl_zigbee_af_table_init_cb(void);

/** @brief Green power server pass frame without translation callback.
 *
 * This function is called by the green power server translation table plugin
 * to notify the application of a green power notification without translation
 * (when the endpoint is set as 0xFC).
 *
 * @param addr GPD address in the notification   Ver.: always
 * @param gpdCommandId GPD command id Ver.: always
 * @param gpdCommandPayload GPD command payload Ver.: always
 */
void sl_zigbee_af_green_power_cluster_pass_frame_without_translation_cb(sl_zigbee_gp_address_t * addr,
                                                                        uint8_t gpdCommandId,
                                                                        uint8_t * gpdCommandPayload);

/** @} */ // end of gp_translation_table_cb
/** @} */ // end of name Callbacks
/** @} */ // end of green-power-translation-table

sli_zigbee_af_gp_command_translation_table* sli_zigbee_af_gp_trans_table_get_translation_table(void);

void sli_zigbee_af_gp_trans_table_clear_translation_table(void);
void sli_zigbee_af_gp_set_translation_table_entry(uint8_t index);
sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t* sli_zigbee_af_gp_get_customized_table(void);
void sli_zigbee_af_gp_clear_customized_table(void);
void sli_zigbee_af_gp_set_customized_table_entry(uint8_t index);
sl_zigbee_gp_translation_table_additional_info_block_field_t * sli_zigbee_af_gp_get_additional_info_table(void);
void sli_zigbee_gp_clear_additional_info_block_table(void);
void sli_zigbee_af_gp_set_additional_info_block_table_entry(uint8_t index);
uint8_t sli_zigbee_af_gp_trans_table_get_translation_table_entry(uint8_t entryIndex,
                                                                 sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t *TranslationTableEntry);
uint8_t sli_zigbee_af_gp_trans_table_replace_translation_table_entry_update_command(uint8_t Index,
                                                                                    bool infoBlockPresent,
                                                                                    sl_zigbee_gp_address_t * gpdAddr,
                                                                                    uint8_t gpdCommandId,
                                                                                    uint8_t ZbEndpoint,
                                                                                    uint16_t zigbeeProfile,
                                                                                    uint16_t zigbeeCluster,
                                                                                    uint8_t  zigbeeCommandId,
                                                                                    uint8_t payloadLength,
                                                                                    uint8_t* payload,
                                                                                    uint8_t payloadSrc,
                                                                                    uint8_t additionalInfoLength,
                                                                                    sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t* AdditionalInfoBlock);
uint8_t sli_zigbee_af_gp_trans_table_add_translation_table_entry_update_command(uint8_t Index,
                                                                                bool infoBlockPresent,
                                                                                sl_zigbee_gp_address_t * gpdAddr,
                                                                                uint8_t gpdCommandId,
                                                                                uint8_t ZbEndpoint,
                                                                                uint16_t zigbeeProfile,
                                                                                uint16_t zigbeeCluster,
                                                                                uint8_t  zigbeeCommandId,
                                                                                uint8_t payloadLength,
                                                                                uint8_t* payload,
                                                                                uint8_t payloadSrc,
                                                                                uint8_t additionalInfoLength,
                                                                                sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t* AdditionalInfoBlock);
uint16_t sli_zigbee_af_gp_copy_additional_info_block_array_to_structure(uint8_t * additionalInfoBlockIn,
                                                                        sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t * additionalInfoBlockOut,
                                                                        uint8_t gpdCommandId);
uint8_t sli_zigbee_af_gp_trans_table_find_matching_translation_table_entry(uint8_t levelOfScan,
                                                                           bool infoBlockPresent,
                                                                           sl_zigbee_gp_address_t  * gpAddr,
                                                                           uint8_t gpdCommandId,
                                                                           uint8_t zbEndpoint,
                                                                           uint8_t * gpdCmdPayload,
                                                                           sl_zigbee_gp_translation_table_additional_info_block_option_record_field_t* additionalInfoBlock,
                                                                           uint8_t *outIndex,
                                                                           uint8_t startIndex);
uint8_t sli_zigbee_af_gp_trans_table_delete_paired_devicefrom_translation_table_entry(sl_zigbee_gp_address_t * gpdAddr);
void sli_zigbee_af_gp_remove_gpd_endpoint_from_translation_table (sl_zigbee_gp_address_t *gpdAddr, uint8_t zbEndpoint);
void sli_zigbee_af_gp_forward_gpd_command_based_on_translation_table(sl_zigbee_gp_address_t *addr,
                                                                     uint8_t gpdCommandId,
                                                                     uint8_t * gpdCommandPayload);
uint8_t sli_zigbee_af_gp_find_matching_generic_translation_table_entry(uint8_t entryType,
                                                                       uint8_t incomingReqType,
                                                                       uint8_t offset,
                                                                       bool infoBlockPresent,
                                                                       uint8_t gpdCommandId,
                                                                       uint16_t zigbeeProfile,
                                                                       uint16_t zigbeeCluster,
                                                                       uint8_t  zigbeeCommandId,
                                                                       uint8_t payloadLength,
                                                                       uint8_t* payload,
                                                                       uint8_t* outIndex);

const sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t* sli_zigbee_af_gp_get_default_table(void);
void sli_zigbee_af_gp_pairing_done_thus_set_customized_translation_table(sl_zigbee_gp_address_t * gpdAddr,
                                                                         uint8_t gpdCommandId,
                                                                         uint8_t endpoint);
extern sl_status_t sli_zigbee_af_gp_forward_gpd_to_mapped_endpoint(sl_zigbee_gp_address_t *addr,
                                                                   uint8_t gpdCommandId,
                                                                   uint8_t * gpdCommandPayload,
                                                                   const sl_zigbee_af_green_power_server_gpd_sub_translation_table_entry_t * genericTranslationTable,
                                                                   uint8_t endpoint);
#endif // SL_CATALOG_ZIGBEE_GREEN_POWER_TRANSLATION_TABLE_PRESENT
#endif //_SILABS_GREEN_POWER_TRANSLATION_TABLE_H_
