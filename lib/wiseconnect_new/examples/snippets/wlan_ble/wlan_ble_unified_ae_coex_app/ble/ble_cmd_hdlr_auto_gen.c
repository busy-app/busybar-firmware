#include <stdio.h>
#include "ble_cmd_hdlr_auto_gen.h"
#include "event_loop.h"
#include <stdlib.h>
#include "rsi_common_apis.h"

generic_command_handler_lut_entry_t ble_commands_hdlr_lut[] = {
  { ble_module_req_adv_cmd_handler },
  { ble_module_req_scan_cmd_handler },
  { ble_module_req_conn_cmd_handler },
  { ble_module_req_adv_stop_cmd_handler },
  { ble_module_req_scan_stop_cmd_handler },
  { ble_module_req_disconnect_cmd_handler },
  { ble_module_get_dev_state_cmd_handler },
  { ble_module_cmd_conn_params_update_cmd_handler },
  { ble_module_req_start_encryption_cmd_handler },
  { ble_module_req_smp_pair_cmd_handler },
  { ble_module_smp_pair_response_cmd_handler },
  { ble_module_smp_passkey_cmd_handler },
  { ble_module_req_profiles_cmd_handler },
  { ble_module_req_profile_cmd_handler },
  { ble_module_req_char_services_cmd_handler },
  { ble_module_req_inc_services_cmd_handler },
  { ble_module_req_read_by_uuid_cmd_handler },
  { ble_module_req_desc_cmd_handler },
  { ble_module_req_read_val_cmd_handler },
  { ble_module_req_multiple_read_cmd_handler },
  { ble_module_req_long_read_cmd_handler },
  { ble_module_req_write_cmd_handler },
  { ble_module_req_write_no_ack_cmd_handler },
  { ble_module_req_long_write_cmd_handler },
  { ble_module_req_prepare_write_cmd_handler },
  { ble_module_req_execute_write_cmd_handler },
  { ble_module_add_service_cmd_handler },
  { ble_module_add_attribute_cmd_handler },
  { ble_module_set_local_att_value_cmd_handler },
  { ble_module_get_local_att_value_cmd_handler },
  { ble_module_cmd_notify_cmd_handler },
  { ble_module_cmd_indicate_cmd_handler },
  { ble_module_set_advertise_data_cmd_handler },
  { ble_module_get_le_ping_cmd_handler },
  { ble_module_set_le_ping_cmd_handler },
  { ble_module_set_random_address_cmd_handler },
  { ble_module_encrypt_cmd_handler },
  { ble_module_cmd_read_resp_cmd_handler },
  { ble_module_set_scan_response_data_cmd_handler },
  { ble_module_le_accept_list_cmd_handler },
  { ble_module_cmd_remove_service_cmd_handler },
  { ble_module_cmd_remove_attribute_cmd_handler },
  { ble_module_process_resolv_list_cmd_handler },
  { ble_module_get_resolving_list_size_cmd_handler },
  { ble_module_set_address_resolution_enable_cmd_handler },
  { ble_module_req_read_phy_cmd_handler },
  { ble_module_req_set_phy_cmd_handler },
  { ble_module_set_data_len_cmd_handler },
  { ble_module_read_max_data_len_cmd_handler },
  { ble_module_set_privacy_mode_cmd_handler },
  { ble_module_cbfc_conn_req_cmd_handler },
  { ble_module_cbfc_conn_resp_cmd_handler },
  { ble_module_cbfc_tx_data_cmd_handler },
  { ble_module_cbfc_disconn_cmd_handler },
  { ble_module_le_ltk_req_reply_cmd_handler },
  { ble_module_rx_test_mode_cmd_handler },
  { ble_module_tx_test_mode_cmd_handler },
  { ble_module_end_test_mode_cmd_handler },
  { ble_module_per_tx_mode_cmd_handler },
  { ble_module_per_rx_mode_cmd_handler },
  { ble_module_cmd_att_error_cmd_handler },
  { ble_module_req_profiles_async_cmd_handler },
  { ble_module_req_profile_async_cmd_handler },
  { ble_module_get_charservices_async_cmd_handler },
  { ble_module_get_includeservices_async_cmd_handler },
  { ble_module_readcharvaluebyuuid_async_cmd_handler },
  { ble_module_get_attribute_async_cmd_handler },
  { ble_module_get_descriptorvalue_async_cmd_handler },
  { ble_module_get_multiplevalues_async_cmd_handler },
  { ble_module_get_longdescvalues_async_cmd_handler },
  { ble_module_set_descvalue_async_cmd_handler },
  { ble_module_set_preparewrite_async_cmd_handler },
  { ble_module_execute_longdescwrite_async_cmd_handler },
  { ble_module_set_smp_pairing_capability_data_cmd_handler },
  { ble_module_conn_param_resp_cmd_cmd_handler },
  { ble_module_cmd_indicate_confirmation_cmd_handler },
  { ble_module_mtu_exchange_request_cmd_handler },
  { ble_module_cmd_set_wwo_resp_notify_buf_info_cmd_handler },
  { ble_module_cmd_write_resp_cmd_handler },
  { ble_module_cmd_prepare_write_resp_cmd_handler },
  { ble_module_cmd_set_local_irk_cmd_handler },
  { ble_module_cmd_set_prop_protocol_ble_bandedge_txpower_cmd_handler },
  { ble_module_cmd_mtu_exchange_resp_cmd_handler },
  { ble_module_cmd_set_ble_tx_power_cmd_handler },
  { ble_module_cmd_indicate_sync_cmd_handler },
  { ble_module_prop_protocol_cmd_cmd_handler },
  { ble_module_prop_protocol_cmd_per_cmd_handler },
  { NULL }
};

/*==============================================*/
/**
 * @fn          void ui_task_req_adv_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_adv_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_adv_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_scan_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_scan_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_scan_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_conn_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_conn_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_conn_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_adv_stop_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_adv_stop_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_adv_stop_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_scan_stop_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_scan_stop_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_scan_stop_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_disconnect_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_disconnect_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_disconnect_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_get_dev_state_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_get_dev_state_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = get_dev_state_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cmd_conn_params_update_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cmd_conn_params_update_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cmd_conn_params_update_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_start_encryption_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_start_encryption_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_start_encryption_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_smp_pair_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_smp_pair_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_smp_pair_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_smp_pair_response_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_smp_pair_response_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = smp_pair_response_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_smp_passkey_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_smp_passkey_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = smp_passkey_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_profiles_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_profiles_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_profiles_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_profile_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_profile_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_profile_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_char_services_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_char_services_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_char_services_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_inc_services_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_inc_services_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_inc_services_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_read_by_uuid_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_read_by_uuid_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_read_by_uuid_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_desc_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_desc_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_desc_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_read_val_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_read_val_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_read_val_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_multiple_read_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_multiple_read_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_multiple_read_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_long_read_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_long_read_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_long_read_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_write_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_write_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_write_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_write_no_ack_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_write_no_ack_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_write_no_ack_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_long_write_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_long_write_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_long_write_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_prepare_write_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_prepare_write_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_prepare_write_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_execute_write_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_execute_write_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_execute_write_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_add_service_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_add_service_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = add_service_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_add_attribute_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_add_attribute_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = add_attribute_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_set_local_att_value_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_set_local_att_value_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = set_local_att_value_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_get_local_att_value_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_get_local_att_value_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = get_local_att_value_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cmd_notify_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cmd_notify_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cmd_notify_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cmd_indicate_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cmd_indicate_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cmd_indicate_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_set_advertise_data_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_set_advertise_data_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = set_advertise_data_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_get_le_ping_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_get_le_ping_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = get_le_ping_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_set_le_ping_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_set_le_ping_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = set_le_ping_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_set_random_address_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_set_random_address_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = set_random_address_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_encrypt_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_encrypt_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = encrypt_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cmd_read_resp_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cmd_read_resp_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cmd_read_resp_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_set_scan_response_data_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_set_scan_response_data_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = set_scan_response_data_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_le_accept_list_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_le_accept_list_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = le_accept_list_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cmd_remove_service_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cmd_remove_service_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cmd_remove_service_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cmd_remove_attribute_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cmd_remove_attribute_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cmd_remove_attribute_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_process_resolv_list_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_process_resolv_list_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = process_resolv_list_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_get_resolving_list_size_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_get_resolving_list_size_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = get_resolving_list_size_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_set_address_resolution_enable_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_set_address_resolution_enable_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = set_address_resolution_enable_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_read_phy_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_read_phy_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_read_phy_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_set_phy_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_set_phy_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_set_phy_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_set_data_len_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_set_data_len_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = set_data_len_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_read_max_data_len_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_read_max_data_len_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = read_max_data_len_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_set_privacy_mode_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_set_privacy_mode_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = set_privacy_mode_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cbfc_conn_req_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cbfc_conn_req_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cbfc_conn_req_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cbfc_conn_resp_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cbfc_conn_resp_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cbfc_conn_resp_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cbfc_tx_data_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cbfc_tx_data_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cbfc_tx_data_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cbfc_disconn_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cbfc_disconn_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cbfc_disconn_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_le_ltk_req_reply_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_le_ltk_req_reply_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = le_ltk_req_reply_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_rx_test_mode_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_rx_test_mode_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = rx_test_mode_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_tx_test_mode_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_tx_test_mode_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = tx_test_mode_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_end_test_mode_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_end_test_mode_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = end_test_mode_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_per_tx_mode_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_per_tx_mode_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = per_tx_mode_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_per_rx_mode_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_per_rx_mode_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = per_rx_mode_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cmd_att_error_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cmd_att_error_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cmd_att_error_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_profiles_async_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_profiles_async_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_profiles_async_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_req_profile_async_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_req_profile_async_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = req_profile_async_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_get_charservices_async_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_get_charservices_async_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = get_charservices_async_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_get_includeservices_async_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_get_includeservices_async_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = get_includeservices_async_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_readcharvaluebyuuid_async_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_readcharvaluebyuuid_async_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = readcharvaluebyuuid_async_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_get_attribute_async_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_get_attribute_async_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = get_attribute_async_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_get_descriptorvalue_async_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_get_descriptorvalue_async_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = get_descriptorvalue_async_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_get_multiplevalues_async_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_get_multiplevalues_async_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = get_multiplevalues_async_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_get_longdescvalues_async_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_get_longdescvalues_async_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = get_longdescvalues_async_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_set_descvalue_async_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_set_descvalue_async_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = set_descvalue_async_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_set_preparewrite_async_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_set_preparewrite_async_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = set_preparewrite_async_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_execute_longdescwrite_async_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_execute_longdescwrite_async_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = execute_longdescwrite_async_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_set_smp_pairing_capability_data_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_set_smp_pairing_capability_data_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = set_smp_pairing_capability_data_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_conn_param_resp_cmd_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_conn_param_resp_cmd_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = conn_param_resp_cmd_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cmd_indicate_confirmation_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cmd_indicate_confirmation_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cmd_indicate_confirmation_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_mtu_exchange_request_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_mtu_exchange_request_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = mtu_exchange_request_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cmd_set_wwo_resp_notify_buf_info_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cmd_set_wwo_resp_notify_buf_info_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cmd_set_wwo_resp_notify_buf_info_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cmd_write_resp_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cmd_write_resp_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cmd_write_resp_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cmd_prepare_write_resp_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cmd_prepare_write_resp_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cmd_prepare_write_resp_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cmd_set_local_irk_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cmd_set_local_irk_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cmd_set_local_irk_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cmd_set_prop_protocol_ble_bandedge_txpower_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cmd_set_prop_protocol_ble_bandedge_txpower_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cmd_set_prop_protocol_ble_bandedge_txpower_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cmd_mtu_exchange_resp_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cmd_mtu_exchange_resp_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cmd_mtu_exchange_resp_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cmd_set_ble_tx_power_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cmd_set_ble_tx_power_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cmd_set_ble_tx_power_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_cmd_indicate_sync_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_cmd_indicate_sync_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = cmd_indicate_sync_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_prop_protocol_cmd_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_prop_protocol_cmd_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = prop_protocol_cmd_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}
/*==============================================*/
/**
 * @fn          void ui_task_prop_protocol_cmd_per_command_handler(uint8_t argc, uint8_t **argv)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.command_queue to be processed in ble task context
 * @param[in]   uint8_t, argc 
 * @param[in]   uint8_t, argv
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues command data received in driver context to ble_generic_cb.command_queue to be processed in ble task context
 *
 */

APP_WEAK void ui_task_prop_protocol_cmd_per_cmd_handler(uint8_t argc, uint8_t **argv)
{
  generic_command_message_t cmd_msg = { 0 };
  osStatus_t sem_return_status;

  //! init messag details
  cmd_msg.next       = NULL;
  cmd_msg.argc       = argc;
  cmd_msg.argv       = argv;
  cmd_msg.command_id = prop_protocol_cmd_per_command_id;
  cmd_msg.semaphore  = &ui_task_response_semaphore;
  //cmd_msg.free_handler = rsi_free_memory;

  //! enqueue message to ble_generic_cb.command_queue
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.command_queue, (rsi_app_pkt_t *)&cmd_msg, &ble_generic_cb.cmd_mutex);

  //! post the ble_generic_cb.semaphore to excute event loop
  osSemaphoreRelease(ble_generic_cb.semaphore);

  //! wait ui_task_response_semaphore until that particular command handler excutes
  sem_return_status = osSemaphoreAcquire(ui_task_response_semaphore, 0);
  if (sem_return_status != 0) {
    _assert((uint8_t *)"ui_task_response_semaphore failed!!!", __LINE__);
  }

  if (cmd_msg.response_status == FIXED_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.fixed_length_response_buffer);
  } else if (cmd_msg.response_status == VARIABLE_LEN_RESPONSE_BUFFER_VALID) {
    printf("\r\ncommand response: %s\r\n", cmd_msg.variable_length_response_buffer);
    if (cmd_msg.free_handler) {
      cmd_msg.free_handler(cmd_msg.variable_length_response_buffer);
    } else {
      _assert((uint8_t *)"free_handler missing!!!", __LINE__);
    }
  }
}

/*==============================================*/
/**
 * @fn          void ble_module_req_adv_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_adv to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_adv to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_adv_cmd_handler(generic_command_message_t __attribute__((unused))
                                             * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_adv_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_adv_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_scan_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_scan to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_scan to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_scan_cmd_handler(generic_command_message_t __attribute__((unused))
                                              * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_scan_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_scan_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_conn_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_conn to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_conn to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_conn_cmd_handler(generic_command_message_t __attribute__((unused))
                                              * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_conn_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_conn_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_adv_stop_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_adv_stop to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_adv_stop to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_adv_stop_cmd_handler(generic_command_message_t __attribute__((unused))
                                                  * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_adv_stop_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_adv_stop_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_scan_stop_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_scan_stop to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_scan_stop to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_scan_stop_cmd_handler(generic_command_message_t __attribute__((unused))
                                                   * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_scan_stop_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_scan_stop_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_disconnect_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_disconnect to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_disconnect to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_disconnect_cmd_handler(generic_command_message_t __attribute__((unused))
                                                    * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_disconnect_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_disconnect_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_get_dev_state_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for get_dev_state to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for get_dev_state to be executed in ble task context
 *
 */

APP_WEAK void ble_module_get_dev_state_cmd_handler(generic_command_message_t __attribute__((unused))
                                                   * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_dev_state_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_get_dev_state_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cmd_conn_params_update_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cmd_conn_params_update to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cmd_conn_params_update to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cmd_conn_params_update_cmd_handler(generic_command_message_t __attribute__((unused))
                                                            * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_conn_params_update_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_conn_params_update_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_start_encryption_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_start_encryption to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_start_encryption to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_start_encryption_cmd_handler(generic_command_message_t __attribute__((unused))
                                                          * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_start_encryption_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_start_encryption_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_smp_pair_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_smp_pair to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_smp_pair to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_smp_pair_cmd_handler(generic_command_message_t __attribute__((unused))
                                                  * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_smp_pair_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_smp_pair_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_smp_pair_response_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for smp_pair_response to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for smp_pair_response to be executed in ble task context
 *
 */

APP_WEAK void ble_module_smp_pair_response_cmd_handler(generic_command_message_t __attribute__((unused))
                                                       * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_smp_pair_response_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_smp_pair_response_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_smp_passkey_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for smp_passkey to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for smp_passkey to be executed in ble task context
 *
 */

APP_WEAK void ble_module_smp_passkey_cmd_handler(generic_command_message_t __attribute__((unused))
                                                 * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_smp_passkey_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_smp_passkey_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_profiles_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_profiles to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_profiles to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_profiles_cmd_handler(generic_command_message_t __attribute__((unused))
                                                  * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_profiles_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_profiles_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_profile_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_profile to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_profile to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_profile_cmd_handler(generic_command_message_t __attribute__((unused))
                                                 * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_profile_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_profile_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_char_services_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_char_services to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_char_services to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_char_services_cmd_handler(generic_command_message_t __attribute__((unused))
                                                       * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_char_services_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_char_services_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_inc_services_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_inc_services to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_inc_services to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_inc_services_cmd_handler(generic_command_message_t __attribute__((unused))
                                                      * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_inc_services_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_inc_services_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_read_by_uuid_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_read_by_uuid to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_read_by_uuid to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_read_by_uuid_cmd_handler(generic_command_message_t __attribute__((unused))
                                                      * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_read_by_uuid_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_read_by_uuid_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_desc_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_desc to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_desc to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_desc_cmd_handler(generic_command_message_t __attribute__((unused))
                                              * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_desc_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_desc_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_read_val_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_read_val to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_read_val to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_read_val_cmd_handler(generic_command_message_t __attribute__((unused))
                                                  * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_read_val_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_read_val_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_multiple_read_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_multiple_read to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_multiple_read to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_multiple_read_cmd_handler(generic_command_message_t __attribute__((unused))
                                                       * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_multiple_read_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_multiple_read_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_long_read_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_long_read to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_long_read to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_long_read_cmd_handler(generic_command_message_t __attribute__((unused))
                                                   * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_long_read_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_long_read_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_write_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_write to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_write to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_write_cmd_handler(generic_command_message_t __attribute__((unused))
                                               * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_write_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_write_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_write_no_ack_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_write_no_ack to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_write_no_ack to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_write_no_ack_cmd_handler(generic_command_message_t __attribute__((unused))
                                                      * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_write_no_ack_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_write_no_ack_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_long_write_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_long_write to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_long_write to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_long_write_cmd_handler(generic_command_message_t __attribute__((unused))
                                                    * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_long_write_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_long_write_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_prepare_write_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_prepare_write to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_prepare_write to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_prepare_write_cmd_handler(generic_command_message_t __attribute__((unused))
                                                       * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_prepare_write_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_prepare_write_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_execute_write_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_execute_write to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_execute_write to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_execute_write_cmd_handler(generic_command_message_t __attribute__((unused))
                                                       * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_execute_write_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_execute_write_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_add_service_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for add_service to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for add_service to be executed in ble task context
 *
 */

APP_WEAK void ble_module_add_service_cmd_handler(generic_command_message_t __attribute__((unused))
                                                 * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_add_service_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_add_service_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_add_attribute_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for add_attribute to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for add_attribute to be executed in ble task context
 *
 */

APP_WEAK void ble_module_add_attribute_cmd_handler(generic_command_message_t __attribute__((unused))
                                                   * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_add_attribute_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_add_attribute_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_set_local_att_value_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for set_local_att_value to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for set_local_att_value to be executed in ble task context
 *
 */

APP_WEAK void ble_module_set_local_att_value_cmd_handler(generic_command_message_t __attribute__((unused))
                                                         * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_local_att_value_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_set_local_att_value_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_get_local_att_value_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for get_local_att_value to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for get_local_att_value to be executed in ble task context
 *
 */

APP_WEAK void ble_module_get_local_att_value_cmd_handler(generic_command_message_t __attribute__((unused))
                                                         * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_local_att_value_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_get_local_att_value_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cmd_notify_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cmd_notify to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cmd_notify to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cmd_notify_cmd_handler(generic_command_message_t __attribute__((unused))
                                                * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_notify_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_notify_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cmd_indicate_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cmd_indicate to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cmd_indicate to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cmd_indicate_cmd_handler(generic_command_message_t __attribute__((unused))
                                                  * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_indicate_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_indicate_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_set_advertise_data_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for set_advertise_data to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for set_advertise_data to be executed in ble task context
 *
 */

APP_WEAK void ble_module_set_advertise_data_cmd_handler(generic_command_message_t __attribute__((unused))
                                                        * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_advertise_data_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_set_advertise_data_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_get_le_ping_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for get_le_ping to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for get_le_ping to be executed in ble task context
 *
 */

APP_WEAK void ble_module_get_le_ping_cmd_handler(generic_command_message_t __attribute__((unused))
                                                 * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_le_ping_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_get_le_ping_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_set_le_ping_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for set_le_ping to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for set_le_ping to be executed in ble task context
 *
 */

APP_WEAK void ble_module_set_le_ping_cmd_handler(generic_command_message_t __attribute__((unused))
                                                 * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_le_ping_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_set_le_ping_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_set_random_address_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for set_random_address to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for set_random_address to be executed in ble task context
 *
 */

APP_WEAK void ble_module_set_random_address_cmd_handler(generic_command_message_t __attribute__((unused))
                                                        * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_random_address_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_set_random_address_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_encrypt_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for encrypt to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for encrypt to be executed in ble task context
 *
 */

APP_WEAK void ble_module_encrypt_cmd_handler(generic_command_message_t __attribute__((unused))
                                             * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_encrypt_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_encrypt_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cmd_read_resp_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cmd_read_resp to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cmd_read_resp to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cmd_read_resp_cmd_handler(generic_command_message_t __attribute__((unused))
                                                   * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_read_resp_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_read_resp_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_set_scan_response_data_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for set_scan_response_data to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for set_scan_response_data to be executed in ble task context
 *
 */

APP_WEAK void ble_module_set_scan_response_data_cmd_handler(generic_command_message_t __attribute__((unused))
                                                            * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_scan_response_data_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_set_scan_response_data_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_le_accept_list_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for le_accept_list to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for le_accept_list to be executed in ble task context
 *
 */

APP_WEAK void ble_module_le_accept_list_cmd_handler(generic_command_message_t __attribute__((unused))
                                                    * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_le_accept_list_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_le_accept_list_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cmd_remove_service_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cmd_remove_service to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cmd_remove_service to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cmd_remove_service_cmd_handler(generic_command_message_t __attribute__((unused))
                                                        * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_remove_service_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_remove_service_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cmd_remove_attribute_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cmd_remove_attribute to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cmd_remove_attribute to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cmd_remove_attribute_cmd_handler(generic_command_message_t __attribute__((unused))
                                                          * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_remove_attribute_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_remove_attribute_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_process_resolv_list_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for process_resolv_list to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for process_resolv_list to be executed in ble task context
 *
 */

APP_WEAK void ble_module_process_resolv_list_cmd_handler(generic_command_message_t __attribute__((unused))
                                                         * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_process_resolv_list_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_process_resolv_list_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_get_resolving_list_size_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for get_resolving_list_size to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for get_resolving_list_size to be executed in ble task context
 *
 */

APP_WEAK void ble_module_get_resolving_list_size_cmd_handler(generic_command_message_t __attribute__((unused))
                                                             * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_resolving_list_size_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_get_resolving_list_size_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_set_address_resolution_enable_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for set_address_resolution_enable to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for set_address_resolution_enable to be executed in ble task context
 *
 */

APP_WEAK void ble_module_set_address_resolution_enable_cmd_handler(generic_command_message_t __attribute__((unused))
                                                                   * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_address_resolution_enable_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_set_address_resolution_enable_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_read_phy_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_read_phy to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_read_phy to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_read_phy_cmd_handler(generic_command_message_t __attribute__((unused))
                                                  * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_read_phy_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_read_phy_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_set_phy_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_set_phy to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_set_phy to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_set_phy_cmd_handler(generic_command_message_t __attribute__((unused))
                                                 * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_set_phy_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_set_phy_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_set_data_len_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for set_data_len to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for set_data_len to be executed in ble task context
 *
 */

APP_WEAK void ble_module_set_data_len_cmd_handler(generic_command_message_t __attribute__((unused))
                                                  * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_data_len_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_set_data_len_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_read_max_data_len_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for read_max_data_len to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for read_max_data_len to be executed in ble task context
 *
 */

APP_WEAK void ble_module_read_max_data_len_cmd_handler(generic_command_message_t __attribute__((unused))
                                                       * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_read_max_data_len_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_read_max_data_len_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_set_privacy_mode_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for set_privacy_mode to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for set_privacy_mode to be executed in ble task context
 *
 */

APP_WEAK void ble_module_set_privacy_mode_cmd_handler(generic_command_message_t __attribute__((unused))
                                                      * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_privacy_mode_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_set_privacy_mode_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cbfc_conn_req_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cbfc_conn_req to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cbfc_conn_req to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cbfc_conn_req_cmd_handler(generic_command_message_t __attribute__((unused))
                                                   * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cbfc_conn_req_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cbfc_conn_req_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cbfc_conn_resp_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cbfc_conn_resp to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cbfc_conn_resp to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cbfc_conn_resp_cmd_handler(generic_command_message_t __attribute__((unused))
                                                    * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cbfc_conn_resp_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cbfc_conn_resp_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cbfc_tx_data_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cbfc_tx_data to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cbfc_tx_data to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cbfc_tx_data_cmd_handler(generic_command_message_t __attribute__((unused))
                                                  * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cbfc_tx_data_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cbfc_tx_data_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cbfc_disconn_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cbfc_disconn to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cbfc_disconn to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cbfc_disconn_cmd_handler(generic_command_message_t __attribute__((unused))
                                                  * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cbfc_disconn_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cbfc_disconn_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_le_ltk_req_reply_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for le_ltk_req_reply to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for le_ltk_req_reply to be executed in ble task context
 *
 */

APP_WEAK void ble_module_le_ltk_req_reply_cmd_handler(generic_command_message_t __attribute__((unused))
                                                      * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_le_ltk_req_reply_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_le_ltk_req_reply_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_rx_test_mode_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for rx_test_mode to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for rx_test_mode to be executed in ble task context
 *
 */

APP_WEAK void ble_module_rx_test_mode_cmd_handler(generic_command_message_t __attribute__((unused))
                                                  * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_rx_test_mode_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_rx_test_mode_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_tx_test_mode_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for tx_test_mode to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for tx_test_mode to be executed in ble task context
 *
 */

APP_WEAK void ble_module_tx_test_mode_cmd_handler(generic_command_message_t __attribute__((unused))
                                                  * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_tx_test_mode_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_tx_test_mode_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_end_test_mode_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for end_test_mode to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for end_test_mode to be executed in ble task context
 *
 */

APP_WEAK void ble_module_end_test_mode_cmd_handler(generic_command_message_t __attribute__((unused))
                                                   * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_end_test_mode_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_end_test_mode_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_per_tx_mode_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for per_tx_mode to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for per_tx_mode to be executed in ble task context
 *
 */

APP_WEAK void ble_module_per_tx_mode_cmd_handler(generic_command_message_t __attribute__((unused))
                                                 * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_per_tx_mode_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_per_tx_mode_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_per_rx_mode_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for per_rx_mode to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for per_rx_mode to be executed in ble task context
 *
 */

APP_WEAK void ble_module_per_rx_mode_cmd_handler(generic_command_message_t __attribute__((unused))
                                                 * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_per_rx_mode_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_per_rx_mode_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cmd_att_error_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cmd_att_error to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cmd_att_error to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cmd_att_error_cmd_handler(generic_command_message_t __attribute__((unused))
                                                   * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_att_error_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_att_error_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_profiles_async_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_profiles_async to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_profiles_async to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_profiles_async_cmd_handler(generic_command_message_t __attribute__((unused))
                                                        * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_profiles_async_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_profiles_async_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_req_profile_async_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_profile_async to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_profile_async to be executed in ble task context
 *
 */

APP_WEAK void ble_module_req_profile_async_cmd_handler(generic_command_message_t __attribute__((unused))
                                                       * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_profile_async_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_req_profile_async_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_get_charservices_async_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for get_charservices_async to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for get_charservices_async to be executed in ble task context
 *
 */

APP_WEAK void ble_module_get_charservices_async_cmd_handler(generic_command_message_t __attribute__((unused))
                                                            * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_charservices_async_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_get_charservices_async_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_get_includeservices_async_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for get_includeservices_async to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for get_includeservices_async to be executed in ble task context
 *
 */

APP_WEAK void ble_module_get_includeservices_async_cmd_handler(generic_command_message_t __attribute__((unused))
                                                               * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_includeservices_async_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_get_includeservices_async_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_readcharvaluebyuuid_async_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for readcharvaluebyuuid_async to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for readcharvaluebyuuid_async to be executed in ble task context
 *
 */

APP_WEAK void ble_module_readcharvaluebyuuid_async_cmd_handler(generic_command_message_t __attribute__((unused))
                                                               * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_readcharvaluebyuuid_async_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_readcharvaluebyuuid_async_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_get_attribute_async_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for get_attribute_async to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for get_attribute_async to be executed in ble task context
 *
 */

APP_WEAK void ble_module_get_attribute_async_cmd_handler(generic_command_message_t __attribute__((unused))
                                                         * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_attribute_async_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_get_attribute_async_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_get_descriptorvalue_async_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for get_descriptorvalue_async to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for get_descriptorvalue_async to be executed in ble task context
 *
 */

APP_WEAK void ble_module_get_descriptorvalue_async_cmd_handler(generic_command_message_t __attribute__((unused))
                                                               * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_descriptorvalue_async_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_get_descriptorvalue_async_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_get_multiplevalues_async_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for get_multiplevalues_async to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for get_multiplevalues_async to be executed in ble task context
 *
 */

APP_WEAK void ble_module_get_multiplevalues_async_cmd_handler(generic_command_message_t __attribute__((unused))
                                                              * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_multiplevalues_async_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_get_multiplevalues_async_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_get_longdescvalues_async_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for get_longdescvalues_async to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for get_longdescvalues_async to be executed in ble task context
 *
 */

APP_WEAK void ble_module_get_longdescvalues_async_cmd_handler(generic_command_message_t __attribute__((unused))
                                                              * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_longdescvalues_async_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_get_longdescvalues_async_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_set_descvalue_async_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for set_descvalue_async to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for set_descvalue_async to be executed in ble task context
 *
 */

APP_WEAK void ble_module_set_descvalue_async_cmd_handler(generic_command_message_t __attribute__((unused))
                                                         * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_descvalue_async_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_set_descvalue_async_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_set_preparewrite_async_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for set_preparewrite_async to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for set_preparewrite_async to be executed in ble task context
 *
 */

APP_WEAK void ble_module_set_preparewrite_async_cmd_handler(generic_command_message_t __attribute__((unused))
                                                            * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_preparewrite_async_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_set_preparewrite_async_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_execute_longdescwrite_async_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for execute_longdescwrite_async to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for execute_longdescwrite_async to be executed in ble task context
 *
 */

APP_WEAK void ble_module_execute_longdescwrite_async_cmd_handler(generic_command_message_t __attribute__((unused))
                                                                 * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_execute_longdescwrite_async_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_execute_longdescwrite_async_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_set_smp_pairing_capability_data_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for set_smp_pairing_capability_data to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for set_smp_pairing_capability_data to be executed in ble task context
 *
 */

APP_WEAK void ble_module_set_smp_pairing_capability_data_cmd_handler(generic_command_message_t __attribute__((unused))
                                                                     * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_smp_pairing_capability_data_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_set_smp_pairing_capability_data_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_conn_param_resp_cmd_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for conn_param_resp_cmd to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for conn_param_resp_cmd to be executed in ble task context
 *
 */

APP_WEAK void ble_module_conn_param_resp_cmd_cmd_handler(generic_command_message_t __attribute__((unused))
                                                         * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_conn_param_resp_cmd_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_conn_param_resp_cmd_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cmd_indicate_confirmation_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cmd_indicate_confirmation to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cmd_indicate_confirmation to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cmd_indicate_confirmation_cmd_handler(generic_command_message_t __attribute__((unused))
                                                               * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_indicate_confirmation_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_indicate_confirmation_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_mtu_exchange_request_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for mtu_exchange_request to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for mtu_exchange_request to be executed in ble task context
 *
 */

APP_WEAK void ble_module_mtu_exchange_request_cmd_handler(generic_command_message_t __attribute__((unused))
                                                          * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_mtu_exchange_request_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_mtu_exchange_request_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cmd_set_wwo_resp_notify_buf_info_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cmd_set_wwo_resp_notify_buf_info to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cmd_set_wwo_resp_notify_buf_info to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cmd_set_wwo_resp_notify_buf_info_cmd_handler(generic_command_message_t __attribute__((unused))
                                                                      * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_set_wwo_resp_notify_buf_info_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_set_wwo_resp_notify_buf_info_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cmd_write_resp_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cmd_write_resp to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cmd_write_resp to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cmd_write_resp_cmd_handler(generic_command_message_t __attribute__((unused))
                                                    * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_write_resp_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_write_resp_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cmd_prepare_write_resp_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cmd_prepare_write_resp to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cmd_prepare_write_resp to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cmd_prepare_write_resp_cmd_handler(generic_command_message_t __attribute__((unused))
                                                            * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_prepare_write_resp_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_prepare_write_resp_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cmd_set_local_irk_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cmd_set_local_irk to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cmd_set_local_irk to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cmd_set_local_irk_cmd_handler(generic_command_message_t __attribute__((unused))
                                                       * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_set_local_irk_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_set_local_irk_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cmd_set_prop_protocol_ble_bandedge_txpower_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cmd_set_prop_protocol_ble_bandedge_txpower to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cmd_set_prop_protocol_ble_bandedge_txpower to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cmd_set_prop_protocol_ble_bandedge_txpower_cmd_handler(generic_command_message_t
                                                                                __attribute__((unused))
                                                                                * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_set_prop_protocol_ble_bandedge_txpower_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_set_prop_protocol_ble_bandedge_txpower_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cmd_mtu_exchange_resp_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cmd_mtu_exchange_resp to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cmd_mtu_exchange_resp to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cmd_mtu_exchange_resp_cmd_handler(generic_command_message_t __attribute__((unused))
                                                           * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_mtu_exchange_resp_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_mtu_exchange_resp_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cmd_set_ble_tx_power_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cmd_set_ble_tx_power to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cmd_set_ble_tx_power to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cmd_set_ble_tx_power_cmd_handler(generic_command_message_t __attribute__((unused))
                                                          * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_set_ble_tx_power_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_set_ble_tx_power_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_cmd_indicate_sync_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for cmd_indicate_sync to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for cmd_indicate_sync to be executed in ble task context
 *
 */

APP_WEAK void ble_module_cmd_indicate_sync_cmd_handler(generic_command_message_t __attribute__((unused))
                                                       * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_indicate_sync_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_indicate_sync_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_prop_protocol_cmd_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for prop_protocol_cmd to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for prop_protocol_cmd to be executed in ble task context
 *
 */

APP_WEAK void ble_module_prop_protocol_cmd_cmd_handler(generic_command_message_t __attribute__((unused))
                                                       * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_prop_protocol_cmd_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_prop_protocol_cmd_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
/*==============================================*/
/**
 * @fn          void ble_module_prop_protocol_cmd_per_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for prop_protocol_cmd_per to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for prop_protocol_cmd_per to be executed in ble task context
 *
 */

APP_WEAK void ble_module_prop_protocol_cmd_per_cmd_handler(generic_command_message_t __attribute__((unused))
                                                           * generic_command_message)
{
  //! Add handling here
  /*  int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_prop_protocol_cmd_per_cmd_handler : error status 0x%x \n", status);
    } else {
    
      printf("\r\n ble_module_prop_protocol_cmd_per_cmd_handler : successful \n");
    }

	  generic_command_message->response_status = 0;
  */
}
