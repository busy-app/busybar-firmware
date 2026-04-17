/**
 * @file
 * @brief Weak implementation for Command Class User Credential I/O
 *
 * @copyright 2023 Silicon Laboratories Inc.
 */

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/

#include "cc_user_credential_io.h"

/****************************************************************************/
/*                           GENERAL API FUNCTIONS                          */
/****************************************************************************/

ZW_WEAK void CC_UserCredential_factory_reset(void)
{
}

ZW_WEAK void CC_UserCredential_init_database(void)
{
}

/****************************************************************************/
/*                        USER RELATED API FUNCTIONS                        */
/****************************************************************************/

ZW_WEAK u3c_db_operation_result CC_UserCredential_get_user(
  __attribute__((unused)) uint16_t unique_identifier,
  __attribute__((unused)) u3c_user_t * user,
  __attribute__((unused)) uint8_t * name)
{
  return U3C_DB_OPERATION_RESULT_ERROR;
}

ZW_WEAK uint16_t CC_UserCredential_get_next_user(
  __attribute__((unused)) uint16_t unique_identifier)
{
  return 0;
}

ZW_WEAK u3c_db_operation_result CC_UserCredential_add_user(
  __attribute__((unused)) u3c_user_t * user,
  __attribute__((unused)) uint8_t * name)
{
  return U3C_DB_OPERATION_RESULT_ERROR;
}

ZW_WEAK u3c_db_operation_result CC_UserCredential_modify_user(
  __attribute__((unused)) u3c_user_t * user,
  __attribute__((unused)) uint8_t * name)
{
  return U3C_DB_OPERATION_RESULT_ERROR;
}

ZW_WEAK u3c_db_operation_result CC_UserCredential_delete_user(
  __attribute__((unused)) uint16_t user_unique_identifier)
{
  return U3C_DB_OPERATION_RESULT_ERROR;
}

/****************************************************************************/
/*                     CREDENTIAL RELATED API FUNCTIONS                     */
/****************************************************************************/

ZW_WEAK u3c_db_operation_result CC_UserCredential_get_credential(
  __attribute__((unused)) uint16_t user_unique_identifier,
  __attribute__((unused)) u3c_credential_type credential_type,
  __attribute__((unused)) uint16_t credential_slot,
  __attribute__((unused)) u3c_credential_metadata_t * credential_metadata,
  __attribute__((unused)) uint8_t * credential_data)
{
  return U3C_DB_OPERATION_RESULT_ERROR;
}

ZW_WEAK bool CC_UserCredential_get_next_credential(
  __attribute__((unused)) uint16_t user_unique_identifier,
  __attribute__((unused)) u3c_credential_type credential_type,
  __attribute__((unused)) uint16_t credential_slot,
  __attribute__((unused)) u3c_credential_type * next_credential_type,
  __attribute__((unused)) uint16_t * next_credential_slot)
{
  return false;
}

ZW_WEAK u3c_db_operation_result CC_UserCredential_add_credential(
  __attribute__((unused)) u3c_credential_t * credential)
{
  return U3C_DB_OPERATION_RESULT_ERROR;
}

ZW_WEAK u3c_db_operation_result CC_UserCredential_modify_credential(
  __attribute__((unused)) u3c_credential_t * credential)
{
  return U3C_DB_OPERATION_RESULT_ERROR;
}

ZW_WEAK u3c_db_operation_result CC_UserCredential_delete_credential(
  __attribute__((unused)) u3c_credential_type credential_type,
  __attribute__((unused)) uint16_t credential_slot)
{
  return U3C_DB_OPERATION_RESULT_ERROR;
}

ZW_WEAK u3c_db_operation_result CC_UserCredential_move_credential(
  __attribute__((unused)) u3c_credential_type credential_type,
  __attribute__((unused)) uint16_t source_credential_slot,
  __attribute__((unused)) uint16_t destination_user_uid,
  __attribute__((unused)) uint16_t destination_credential_slot)
{
  return U3C_DB_OPERATION_RESULT_ERROR;
}

ZW_WEAK u3c_db_operation_result CC_UserCredential_get_admin_code_info(
  __attribute__((unused)) u3c_admin_code_metadata_t *code)
{
  return U3C_DB_OPERATION_RESULT_ERROR;
}

ZW_WEAK u3c_db_operation_result CC_UserCredential_set_admin_code(
  __attribute__((unused)) u3c_admin_code_metadata_t *code)
{
  return U3C_DB_OPERATION_RESULT_ERROR;
}
