/***************************************************************************//**
 * @file trust-center-backup.h
 * @brief Definitions for the Trust Center Backup plugin.
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

#ifdef SL_CATALOG_ZIGBEE_TRUST_CENTER_BACKUP_PRESENT
#include "trust-center-backup-config.h"
#else
#include "config/trust-center-backup-config.h"
#endif
#if (SL_ZIGBEE_AF_PLUGIN_TRUST_CENTER_BACKUP_POSIX_FILE_BACKUP_SUPPORT == 1)
#define POSIX_FILE_BACKUP_SUPPORT
#endif

/**
 * @defgroup trust-center-backup Trust Center Backup
 * @ingroup component
 * @brief API and Callbacks for the Trust Center Backup Component
 *
 * This component is only for a trust center.  It provides a set of APIs
 * for importing and exporting the backup info for a Smart Energy trust center.
 * It requires extending to hook up import/export routines into an external
 * storage device or network, where the data may be saved to and restored from.
 *
 */

/**
 * @addtogroup trust-center-backup
 * @{
 */

/**
 * @name API
 * @{
 */

/** @brief export backup data
 *
 * @param backup Ver.: always
 *
 * @return sl_status_t status code
 *
 */
sl_status_t sl_zigbee_trust_center_export_backup_data(sl_zigbee_af_trust_center_backup_data_t* backup);

/** @brief Import backup and start network
 *
 * @param nbackup Ver.: always
 *
 * @return sl_status_t status code
 *
 */
sl_status_t sl_zigbee_trust_center_import_backup_and_start_network(const sl_zigbee_af_trust_center_backup_data_t* backup);

/** @brief Import backup from file
 *
 * @param filepath Ver.: always
 *
 * @return sl_status_t status code
 *
 * @note Available only for SL_ZIGBEE_AF_PLUGIN_TRUST_CENTER_BACKUP_POSIX_FILE_BACKUP_SUPPORT
 *
 */
sl_status_t sl_zigbee_af_trust_center_import_backup_from_file(const char* filepath);

/** @brief export backup to a file
 *
 * @param filepath Ver.: always
 *
 * @return sl_status_t status code
 *
 * @note Available only for SL_ZIGBEE_AF_PLUGIN_TRUST_CENTER_BACKUP_POSIX_FILE_BACKUP_SUPPORT
 *
 */
sl_status_t sl_zigbee_af_trust_center_export_backup_to_file(const char* filepath);

/** @brief saves tokens to a file
 *
 * @param filepath Ver.: always
 *
 * @return sl_status_t status code
 *
 * @note Available only for SL_ZIGBEE_AF_PLUGIN_TRUST_CENTER_BACKUP_POSIX_FILE_BACKUP_SUPPORT
 *
 */
sl_status_t sl_zigbee_af_trust_center_backup_save_tokens_to_file(const char* filepath);

/** @brief restores tokens from a file
 *
 * @param filepath Ver.: always
 *
 * @return sl_status_t status code
 *
 * @note Available only for SL_ZIGBEE_AF_PLUGIN_TRUST_CENTER_BACKUP_POSIX_FILE_BACKUP_SUPPORT
 *
 */
sl_status_t sl_zigbee_af_trust_center_backup_restore_tokens_from_file(const char* filepath);

/** @brief Updates zigbeed tokens from a file that stores NCP tokens.
 *
 * @param filepath Ver.: always
 *
 * @return sl_status_t status code
 *
 * @note Available only for SL_ZIGBEE_AF_PLUGIN_TRUST_CENTER_BACKUP_POSIX_FILE_BACKUP_SUPPORT
 *
 */
sl_status_t sl_zigbee_af_trust_center_backup_write_ncp_token_to_zigbeed_tokens(const char* filepath);

/** @} */ // end of name API
/** @} */ // end of trust-center-backup

#ifdef SL_CLI_TYPES_H
//requires CLI support; avoid having scripted unit test try to compile

sl_status_t sli_zigbee_af_trust_center_backup_save_keys_to_data(sl_zigbee_token_data_t* data_s, uint32_t nvm3Key, uint8_t index);
sl_status_t sli_zigbee_af_trust_center_backup_restore_keys_from_data(sl_zigbee_token_data_t* data_s, uint32_t nvm3Key, uint8_t index);

void sli_zigbee_af_tc_export_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_af_tc_import_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_af_trust_center_backup_save_tokens_to_file_cli(sl_cli_command_arg_t *arguments);
void sli_zigbee_af_trust_center_backup_restore_tokens_from_file_cli(sl_cli_command_arg_t *arguments);

#endif //SL_CLI_TYPES_H
