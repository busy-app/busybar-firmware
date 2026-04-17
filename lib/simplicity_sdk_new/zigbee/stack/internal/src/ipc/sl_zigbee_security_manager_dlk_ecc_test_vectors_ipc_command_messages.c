/***************************************************************************//**
 * @file sl_zigbee_security_manager_dlk_ecc_test_vectors_ipc_command_messages.c
 * @brief internal wrappers for 'sl_zigbee_security_manager_dlk_ecc_test_vectors' ipc commands
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
// automatically generated from sl_zigbee_security_manager_dlk_ecc_test_vectors.h.  Do not manually edit
#include "stack/include/sl_zigbee_security_manager_dlk_ecc_test_vectors.h"
#include "stack/internal/inc/sl_zigbee_security_manager_dlk_ecc_test_vectors_internal_def.h"
#include "stack/internal/src/ipc/sl_zigbee_security_manager_dlk_ecc_test_vectors_ipc_command_messages.h"
#include "stack/internal/src/ipc/zigbee_ipc_command_messages.h"

// ipc command dispatch

void sli_zigbee_stack_dlk_ecc_test_vector_get_profile_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.dlk_ecc_test_vector_get_profile.response.result = sli_zigbee_stack_dlk_ecc_test_vector_get_profile();
}

void sli_zigbee_stack_dlk_ecc_test_vector_set_profile_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  sli_zigbee_stack_dlk_ecc_test_vector_set_profile(&msg->data.dlk_ecc_test_vector_set_profile.request.profile);
}

void sli_zigbee_stack_sec_man_ecc_init_with_test_vector_process_ipc_command(sli_zigbee_ipc_cmd_t *msg)
{
  msg->data.sec_man_ecc_init_with_test_vector.response.result = sli_zigbee_stack_sec_man_ecc_init_with_test_vector(&msg->data.sec_man_ecc_init_with_test_vector.request.dlk_ecc_ctx,
                                                                                                                   &msg->data.sec_man_ecc_init_with_test_vector.request.config,
                                                                                                                   msg->data.sec_man_ecc_init_with_test_vector.request.psk,
                                                                                                                   &msg->data.sec_man_ecc_init_with_test_vector.request.vector_data);
}

// public entrypoints

sl_zigbee_dlk_ecc_test_vector_profile_data_t * sl_zigbee_dlk_ecc_test_vector_get_profile(void)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_dlk_ecc_test_vector_get_profile_process_ipc_command, &msg);

  return msg.data.dlk_ecc_test_vector_get_profile.response.result;
}

void sl_zigbee_dlk_ecc_test_vector_set_profile(const sl_zigbee_dlk_ecc_test_vector_profile_data_t *profile)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (profile != NULL) {
    msg.data.dlk_ecc_test_vector_set_profile.request.profile = *profile;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_dlk_ecc_test_vector_set_profile_process_ipc_command, &msg);
}

sl_status_t sl_zigbee_sec_man_ecc_init_with_test_vector(sl_zigbee_sec_man_dlk_ecc_context_t *dlk_ecc_ctx,
                                                        sl_zb_dlk_ecc_config_t *config,
                                                        uint8_t *psk,
                                                        sl_zigbee_dlk_ecc_test_vector_bundle_t *vector_data)
{
  sli_zigbee_ipc_cmd_t msg = { 0, };

  if (dlk_ecc_ctx != NULL) {
    msg.data.sec_man_ecc_init_with_test_vector.request.dlk_ecc_ctx = *dlk_ecc_ctx;
  }

  if (config != NULL) {
    msg.data.sec_man_ecc_init_with_test_vector.request.config = *config;
  }

  if (psk != NULL) {
    memmove(msg.data.sec_man_ecc_init_with_test_vector.request.psk, psk, sizeof(uint8_t) * 16);
  }

  if (vector_data != NULL) {
    msg.data.sec_man_ecc_init_with_test_vector.request.vector_data = *vector_data;
  }

  sli_zigbee_send_ipc_cmd(sli_zigbee_stack_sec_man_ecc_init_with_test_vector_process_ipc_command, &msg);

  if (dlk_ecc_ctx != NULL) {
    *dlk_ecc_ctx = msg.data.sec_man_ecc_init_with_test_vector.request.dlk_ecc_ctx;
  }

  if (config != NULL) {
    *config = msg.data.sec_man_ecc_init_with_test_vector.request.config;
  }

  if (psk != NULL) {
    memmove(psk, msg.data.sec_man_ecc_init_with_test_vector.request.psk, sizeof(uint8_t) * 16);
  }

  if (vector_data != NULL) {
    *vector_data = msg.data.sec_man_ecc_init_with_test_vector.request.vector_data;
  }

  return msg.data.sec_man_ecc_init_with_test_vector.response.result;
}
