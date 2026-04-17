#include <stdio.h>
#include "ble_event_hdlr_auto_gen.h"
#include "ble_private.h"
#include "event_loop.h"
#include <stdlib.h>
#include "rsi_common_utils.h"
#include "rsi_common_apis.h"
#include "sl_constants.h"

generic_task_cb_t ble_generic_cb;

generic_event_handler_lut_entry_t ble_events_lut[] = { { rsi_ble_event_adv_report },
                                                       { rsi_ble_event_disconnect },
                                                       { rsi_ble_event_conn_status },
                                                       { rsi_ble_event_enhance_conn_status },
                                                       { rsi_ble_event_mtu_exchange_information },
                                                       { rsi_ble_event_mtu },
                                                       { rsi_ble_event_more_data_req },
                                                       { rsi_ble_simple_peripheral_on_remote_features_event },
                                                       { rsi_ble_event_data_length_update },
                                                       { rsi_ble_event_phy_update },
                                                       { rsi_ble_on_remote_conn_params_request_event },
                                                       { rsi_ble_event_conn_update },
                                                       { rsi_ble_read_req },
                                                       { rsi_ble_gatt_error_event },
                                                       { rsi_ble_event_gatt_desc },
                                                       { rsi_ble_char_services_event },
                                                       { rsi_ble_profile_event },
                                                       { rsi_ble_event_profiles_list },
                                                       { rsi_ble_on_gatt_write_event },
                                                       { rsi_ble_on_gatt_prepare_write_event },
                                                       { rsi_ble_on_execute_write_event },
                                                       { rsi_ble_on_event_write_resp },
                                                       { rsi_ble_prepare_write_resp },
                                                       { rsi_ble_on_read_resp_event },
                                                       { rsi_ble_event_indication_confirmation },
                                                       { rsi_ble_event_ext_adv_report },
                                                       { rsi_ble_event_extended_per_sync_establish },
                                                       { rsi_ble_event_extended_per_adv_report },
                                                       { rsi_ble_event_extended_per_sync_lost },
                                                       { rsi_ble_event_extended_scan_timeout },
                                                       { rsi_ble_on_terminate },
                                                       { rsi_ble_event_extended_scan_req_received },
                                                       { rsi_bt_event_smp_req },
                                                       { rsi_bt_event_smp_resp },
                                                       { rsi_bt_event_smp_passkey },
                                                       { rsi_bt_event_smp_failed },
                                                       { rsi_bt_event_encryption_enabled },
                                                       { rsi_bt_event_smp_passkey_display },
                                                       { rsi_bt_event_sc_passkey },
                                                       { rsi_bt_event_le_ltk_request },
                                                       { rsi_bt_event_le_security_keys },
                                                       { rsi_ble_event_data_transmit },
                                                       { rsi_ble_event_smp_pending },
                                                       { rsi_ble_event_advertisement_restart },
                                                       { rsi_ble_event_scan_restart },
                                                       { rsi_ble_event_on_data_recieve },
                                                       { rsi_ble_event_conn_update_req },
                                                       { rsi_ble_event_set_buffer_config },
                                                       { rsi_ble_event_select_data_transfer },
                                                       { rsi_ble_event_profile_discovery },
                                                       { rsi_ble_event_profile_dummy },
                                                       { rsi_ble_event_connection_procedure_timeout },
                                                       { NULL } };

/*==============================================*/
/**
 * @fn          void rsi_ble_event_adv_report_driver_callback(rsi_ble_event_adv_report_t *rsi_ble_event_adv_report)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_event_adv_report_t *rsi_ble_event_adv_report, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_adv_report_driver_callback(rsi_ble_event_adv_report_t *rsi_ble_event_adv_report)
{
  LOG_PRINT_D("\n in rsi_ble_event_adv_report_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_adv_report_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_adv_report_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_adv_report, sizeof(rsi_ble_event_adv_report_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_disconnect_driver_callback(rsi_ble_event_disconnect_t *rsi_ble_event_disconnect, uint16_t status)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_event_disconnect_t *rsi_ble_event_disconnect, event_data
 * @param[in]   uint16_t , event_status 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_disconnect_driver_callback(rsi_ble_event_disconnect_t *rsi_ble_event_disconnect,
                                                       uint16_t status)
{
  LOG_PRINT_D("\n in rsi_ble_event_disconnect_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_disconnect_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_disconnect_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_disconnect, sizeof(rsi_ble_event_disconnect_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_conn_status_driver_callback(rsi_ble_event_conn_status_t *rsi_ble_event_conn_status)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_event_conn_status_t *rsi_ble_event_conn_status, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_conn_status_driver_callback(rsi_ble_event_conn_status_t *rsi_ble_event_conn_status)
{
  LOG_PRINT_D("\n in rsi_ble_event_conn_status_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_conn_status_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_conn_status_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_conn_status, sizeof(rsi_ble_event_conn_status_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_enhance_conn_status_driver_callback(rsi_ble_event_enhance_conn_status_t *rsi_ble_event_enhance_conn_status)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_event_enhance_conn_status_t *rsi_ble_event_enhance_conn_status, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_enhance_conn_status_driver_callback(
  rsi_ble_event_enhance_conn_status_t *rsi_ble_event_enhance_conn_status)
{
  LOG_PRINT_D("\n in rsi_ble_event_enhance_conn_status_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_enhance_conn_status_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_enhance_conn_status_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0],
         (void *)rsi_ble_event_enhance_conn_status,
         sizeof(rsi_ble_event_enhance_conn_status_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_mtu_exchange_information_driver_callback(rsi_ble_event_mtu_exchange_information_t *rsi_ble_event_mtu_exchange_information)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_event_mtu_exchange_information_t *rsi_ble_event_mtu_exchange_information, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_mtu_exchange_information_driver_callback(
  rsi_ble_event_mtu_exchange_information_t *rsi_ble_event_mtu_exchange_information)
{
  LOG_PRINT_D("\n in rsi_ble_event_mtu_exchange_information_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_mtu_exchange_information_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_mtu_exchange_information_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0],
         (void *)rsi_ble_event_mtu_exchange_information,
         sizeof(rsi_ble_event_mtu_exchange_information_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_mtu_driver_callback(rsi_ble_event_mtu_t *rsi_ble_event_mtu)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_event_mtu_t *rsi_ble_event_mtu, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_mtu_driver_callback(rsi_ble_event_mtu_t *rsi_ble_event_mtu)
{
  LOG_PRINT_D("\n in rsi_ble_event_mtu_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_mtu_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_mtu_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_mtu, sizeof(rsi_ble_event_mtu_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_le_dev_buf_ind_driver_callback(rsi_ble_event_le_dev_buf_ind_t *rsi_ble_event_le_dev_buf_ind)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_event_le_dev_buf_ind_t *rsi_ble_event_le_dev_buf_ind, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_le_dev_buf_ind_driver_callback(rsi_ble_event_le_dev_buf_ind_t *rsi_ble_event_le_dev_buf_ind)
{
  LOG_PRINT_D("\n in rsi_ble_event_le_dev_buf_ind_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_le_dev_buf_ind_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_le_dev_buf_ind_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_le_dev_buf_ind, sizeof(rsi_ble_event_le_dev_buf_ind_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_remote_features_driver_callback(rsi_ble_event_remote_features_t *rsi_ble_event_remote_features)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_event_remote_features_t *rsi_ble_event_remote_features, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_remote_features_driver_callback(
  rsi_ble_event_remote_features_t *rsi_ble_event_remote_features)
{
  LOG_PRINT_D("\n in rsi_ble_event_remote_features_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_remote_features_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_remote_features_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_remote_features, sizeof(rsi_ble_event_remote_features_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_data_length_update_driver_callback(rsi_ble_event_data_length_update_t *rsi_ble_event_data_length_update)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_event_data_length_update_t *rsi_ble_event_data_length_update, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_data_length_update_driver_callback(
  rsi_ble_event_data_length_update_t *rsi_ble_event_data_length_update)
{
  LOG_PRINT_D("\n in rsi_ble_event_data_length_update_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_data_length_update_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_data_length_update_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0],
         (void *)rsi_ble_event_data_length_update,
         sizeof(rsi_ble_event_data_length_update_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_phy_update_driver_callback(rsi_ble_event_phy_update_t *rsi_ble_event_phy_update)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_event_phy_update_t *rsi_ble_event_phy_update, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_phy_update_driver_callback(rsi_ble_event_phy_update_t *rsi_ble_event_phy_update)
{
  LOG_PRINT_D("\n in rsi_ble_event_phy_update_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_phy_update_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_phy_update_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_phy_update, sizeof(rsi_ble_event_phy_update_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_remote_conn_param_req_driver_callback(rsi_ble_event_remote_conn_param_req_t *rsi_ble_event_remote_conn_param_req, uint16_t status)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_event_remote_conn_param_req_t *rsi_ble_event_remote_conn_param_req, event_data
 * @param[in]   uint16_t , event_status 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_remote_conn_param_req_driver_callback(
  rsi_ble_event_remote_conn_param_req_t *rsi_ble_event_remote_conn_param_req,
  uint16_t status)
{
  LOG_PRINT_D("\n in rsi_ble_event_remote_conn_param_req_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_remote_conn_param_req_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_remote_conn_param_req_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0],
         (void *)rsi_ble_event_remote_conn_param_req,
         sizeof(rsi_ble_event_remote_conn_param_req_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_conn_update_driver_callback(rsi_ble_event_conn_update_t *rsi_ble_event_conn_update, uint16_t status)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_event_conn_update_t *rsi_ble_event_conn_update, event_data
 * @param[in]   uint16_t , event_status 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_conn_update_driver_callback(rsi_ble_event_conn_update_t *rsi_ble_event_conn_update,
                                                        uint16_t status)
{
  LOG_PRINT_D("\n in rsi_ble_event_conn_update_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_conn_update_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_conn_update_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_conn_update, sizeof(rsi_ble_event_conn_update_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_read_req_driver_callback(uint16_t status, rsi_ble_read_req_t *rsi_ble_read_req)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_ble_read_req_t *rsi_ble_read_req, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_read_req_driver_callback(uint16_t status, rsi_ble_read_req_t *rsi_ble_read_req)
{
  LOG_PRINT_D("\n in rsi_ble_read_req_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_read_req_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = read_req_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_read_req, sizeof(rsi_ble_read_req_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_error_resp_driver_callback(uint16_t status, rsi_ble_event_error_resp_t *rsi_ble_event_error_resp)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_ble_event_error_resp_t *rsi_ble_event_error_resp, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_error_resp_driver_callback(uint16_t status,
                                                       rsi_ble_event_error_resp_t *rsi_ble_event_error_resp)
{
  LOG_PRINT_D("\n in rsi_ble_event_error_resp_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_error_resp_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_error_resp_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_error_resp, sizeof(rsi_ble_event_error_resp_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_gatt_desc_driver_callback(rsi_ble_event_gatt_desc_t *rsi_ble_event_gatt_desc)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_event_gatt_desc_t *rsi_ble_event_gatt_desc, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_gatt_desc_driver_callback(rsi_ble_event_gatt_desc_t *rsi_ble_event_gatt_desc)
{
  LOG_PRINT_D("\n in rsi_ble_event_gatt_desc_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_gatt_desc_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_gatt_desc_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_gatt_desc, sizeof(rsi_ble_event_gatt_desc_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_read_by_type1_driver_callback(uint16_t status, rsi_ble_event_read_by_type1_t *rsi_ble_event_read_by_type1)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_ble_event_read_by_type1_t *rsi_ble_event_read_by_type1, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_read_by_type1_driver_callback(uint16_t status,
                                                          rsi_ble_event_read_by_type1_t *rsi_ble_event_read_by_type1)
{
  UNUSED_PARAMETER(status);
  LOG_PRINT_D("\n in rsi_ble_event_read_by_type1_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_read_by_type1_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_read_by_type1_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_read_by_type1, sizeof(rsi_ble_event_read_by_type1_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_profile_by_uuid_driver_callback(uint16_t status, rsi_ble_event_profile_by_uuid_t *rsi_ble_event_profile_by_uuid)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_ble_event_profile_by_uuid_t *rsi_ble_event_profile_by_uuid, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_profile_by_uuid_driver_callback(
  uint16_t status,
  rsi_ble_event_profile_by_uuid_t *rsi_ble_event_profile_by_uuid)
{
  LOG_PRINT_D("\n in rsi_ble_event_profile_by_uuid_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_profile_by_uuid_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_profile_by_uuid_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_profile_by_uuid, sizeof(rsi_ble_event_profile_by_uuid_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_profiles_list_driver_callback(uint16_t status, rsi_ble_event_profiles_list_t *rsi_ble_event_profiles_list)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_ble_event_profiles_list_t *rsi_ble_event_profiles_list, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_profiles_list_driver_callback(uint16_t status,
                                                          rsi_ble_event_profiles_list_t *rsi_ble_event_profiles_list)
{
  LOG_PRINT_D("\n in rsi_ble_event_profiles_list_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_profiles_list_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_profiles_list_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_profiles_list, sizeof(rsi_ble_event_profiles_list_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_write_driver_callback(uint16_t status, rsi_ble_event_write_t *rsi_ble_event_write)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_ble_event_write_t *rsi_ble_event_write, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_write_driver_callback(uint16_t status, rsi_ble_event_write_t *rsi_ble_event_write)
{
  LOG_PRINT_D("\n in rsi_ble_event_write_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_write_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_write_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_write, sizeof(rsi_ble_event_write_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_prepare_write_driver_callback(uint16_t status, rsi_ble_event_prepare_write_t *rsi_ble_event_prepare_write)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_ble_event_prepare_write_t *rsi_ble_event_prepare_write, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_prepare_write_driver_callback(uint16_t status,
                                                          rsi_ble_event_prepare_write_t *rsi_ble_event_prepare_write)
{
  LOG_PRINT_D("\n in rsi_ble_event_prepare_write_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_prepare_write_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_prepare_write_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_prepare_write, sizeof(rsi_ble_event_prepare_write_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_execute_write_driver_callback(uint16_t status, rsi_ble_execute_write_t *rsi_ble_execute_write)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_ble_execute_write_t *rsi_ble_execute_write, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_execute_write_driver_callback(uint16_t status, rsi_ble_execute_write_t *rsi_ble_execute_write)
{
  LOG_PRINT_D("\n in rsi_ble_execute_write_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_execute_write_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = execute_write_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_execute_write, sizeof(rsi_ble_execute_write_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_set_att_resp_driver_callback(uint16_t status, rsi_ble_set_att_resp_t *rsi_ble_set_att_resp)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_ble_set_att_resp_t *rsi_ble_set_att_resp, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_set_att_resp_driver_callback(uint16_t status, rsi_ble_set_att_resp_t *rsi_ble_set_att_resp)
{
  LOG_PRINT_D("\n in rsi_ble_set_att_resp_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_set_att_resp_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = set_att_resp_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_set_att_resp, sizeof(rsi_ble_set_att_resp_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_prepare_write_resp_driver_callback(rsi_ble_prepare_write_resp_t *rsi_ble_prepare_write_resp)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_prepare_write_resp_t *rsi_ble_prepare_write_resp, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_prepare_write_resp_driver_callback(rsi_ble_prepare_write_resp_t *rsi_ble_prepare_write_resp)
{
  LOG_PRINT_D("\n in rsi_ble_prepare_write_resp_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_prepare_write_resp_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = prepare_write_resp_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_prepare_write_resp, sizeof(rsi_ble_prepare_write_resp_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_att_value_driver_callback(uint16_t status, rsi_ble_event_att_value_t *rsi_ble_event_att_value)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_ble_event_att_value_t *rsi_ble_event_att_value, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_att_value_driver_callback(uint16_t status,
                                                      rsi_ble_event_att_value_t *rsi_ble_event_att_value)
{
  LOG_PRINT_D("\n in rsi_ble_event_att_value_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_att_value_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_att_value_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_att_value, sizeof(rsi_ble_event_att_value_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_set_att_resp_driver_callback(uint16_t status, rsi_ble_set_att_resp_t *rsi_ble_set_att_resp)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_ble_set_att_resp_t *rsi_ble_set_att_resp, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_indication_confirmation_driver_callback(uint16_t status,
                                                                    rsi_ble_set_att_resp_t *rsi_ble_event_set_att_rsp)
{
  LOG_PRINT_D(" \n in rsi_ble_event_indication_confirmation driver_callback \n");
  generic_event_message_t *msg;
  UNUSED_PARAMETER(status);
  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_set_att_resp_t));
  //! assert if malloc failed
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = indication_confirmation_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_set_att_rsp, sizeof(rsi_ble_set_att_resp_t));
  //! enqueue message to ble_generic_cb.event_queues[0]

  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}
/*==============================================*/
/**
 * @fn          void rsi_ble_ae_adv_report_driver_callback(uint16_t status, rsi_ble_ae_adv_report_t *rsi_ble_ae_adv_report)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_ble_ae_adv_report_t *rsi_ble_ae_adv_report, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_ae_adv_report_driver_callback(uint16_t status, rsi_ble_ae_adv_report_t *rsi_ble_ae_adv_report)
{
  LOG_PRINT_D("\n in rsi_ble_ae_adv_report_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_ae_adv_report_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = ae_adv_report_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_ae_adv_report, sizeof(rsi_ble_ae_adv_report_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_per_adv_sync_estbl_driver_callback(uint16_t status, rsi_ble_per_adv_sync_estbl_t *rsi_ble_per_adv_sync_estbl)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_ble_per_adv_sync_estbl_t *rsi_ble_per_adv_sync_estbl, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_per_adv_sync_estbl_driver_callback(uint16_t status,
                                                         rsi_ble_per_adv_sync_estbl_t *rsi_ble_per_adv_sync_estbl)
{
  LOG_PRINT_D("\n in rsi_ble_per_adv_sync_estbl_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_per_adv_sync_estbl_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = per_adv_sync_estbl_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_per_adv_sync_estbl, sizeof(rsi_ble_per_adv_sync_estbl_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_per_adv_report_driver_callback(uint16_t status, rsi_ble_per_adv_report_t *rsi_ble_per_adv_report)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_ble_per_adv_report_t *rsi_ble_per_adv_report, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_per_adv_report_driver_callback(uint16_t status, rsi_ble_per_adv_report_t *rsi_ble_per_adv_report)
{
  LOG_PRINT_D("\n in rsi_ble_per_adv_report_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_per_adv_report_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = per_adv_report_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_per_adv_report, sizeof(rsi_ble_per_adv_report_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_per_adv_sync_lost_driver_callback(uint16_t status, rsi_ble_per_adv_sync_lost_t *rsi_ble_per_adv_sync_lost)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_ble_per_adv_sync_lost_t *rsi_ble_per_adv_sync_lost, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_per_adv_sync_lost_driver_callback(uint16_t status,
                                                        rsi_ble_per_adv_sync_lost_t *rsi_ble_per_adv_sync_lost)
{
  LOG_PRINT_D("\n in rsi_ble_per_adv_sync_lost_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_per_adv_sync_lost_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = per_adv_sync_lost_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_per_adv_sync_lost, sizeof(rsi_ble_per_adv_sync_lost_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_scan_timeout_driver_callback(uint16_t status, rsi_ble_scan_timeout_t *rsi_ble_scan_timeout)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_ble_scan_timeout_t *rsi_ble_scan_timeout, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_scan_timeout_driver_callback(uint16_t status, rsi_ble_scan_timeout_t *rsi_ble_scan_timeout)
{
  LOG_PRINT_D("\n in rsi_ble_scan_timeout_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_scan_timeout_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = scan_timeout_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_scan_timeout, sizeof(rsi_ble_scan_timeout_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_adv_set_terminated_driver_callback(uint16_t status, rsi_ble_adv_set_terminated_t *rsi_ble_adv_set_terminated)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_ble_adv_set_terminated_t *rsi_ble_adv_set_terminated, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_adv_set_terminated_driver_callback(uint16_t status,
                                                         rsi_ble_adv_set_terminated_t *rsi_ble_adv_set_terminated)
{
  LOG_PRINT_D("\n in rsi_ble_adv_set_terminated_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_adv_set_terminated_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = adv_set_terminated_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_adv_set_terminated, sizeof(rsi_ble_adv_set_terminated_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_scan_req_recvd_driver_callback(uint16_t status, rsi_ble_scan_req_recvd_t *rsi_ble_scan_req_recvd)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_ble_scan_req_recvd_t *rsi_ble_scan_req_recvd, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_scan_req_recvd_driver_callback(uint16_t status, rsi_ble_scan_req_recvd_t *rsi_ble_scan_req_recvd)
{
  LOG_PRINT_D("\n in rsi_ble_scan_req_recvd_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_scan_req_recvd_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = scan_req_recvd_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_scan_req_recvd, sizeof(rsi_ble_scan_req_recvd_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_smp_req_driver_callback(rsi_bt_event_smp_req_t *rsi_ble_event_smp_req)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_bt_event_smp_req_t *rsi_ble_event_smp_req, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_smp_req_driver_callback(rsi_bt_event_smp_req_t *rsi_ble_event_smp_req)
{
  LOG_PRINT_D("\n in rsi_ble_event_smp_req_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_bt_event_smp_req_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_smp_req_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_smp_req, sizeof(rsi_bt_event_smp_req_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_smp_resp_driver_callback(rsi_bt_event_smp_resp_t *rsi_ble_event_smp_resp)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_bt_event_smp_resp_t *rsi_ble_event_smp_resp, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_smp_resp_driver_callback(rsi_bt_event_smp_resp_t *rsi_ble_event_smp_resp)
{
  LOG_PRINT_D("\n in rsi_ble_event_smp_resp_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_bt_event_smp_resp_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_smp_resp_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_smp_resp, sizeof(rsi_bt_event_smp_resp_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_smp_passkey_driver_callback(rsi_bt_event_smp_passkey_t *rsi_ble_event_smp_passkey)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_bt_event_smp_passkey_t *rsi_ble_event_smp_passkey, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_smp_passkey_driver_callback(rsi_bt_event_smp_passkey_t *rsi_ble_event_smp_passkey)
{
  LOG_PRINT_D("\n in rsi_ble_event_smp_passkey_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_bt_event_smp_passkey_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_smp_passkey_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_smp_passkey, sizeof(rsi_bt_event_smp_passkey_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_smp_failed_driver_callback(uint16_t status, rsi_bt_event_smp_failed_t *rsi_ble_event_smp_failed)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_bt_event_smp_failed_t *rsi_ble_event_smp_failed, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_smp_failed_driver_callback(uint16_t status,
                                                       rsi_bt_event_smp_failed_t *rsi_ble_event_smp_failed)
{
  LOG_PRINT_D("\n in rsi_ble_event_smp_failed_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_bt_event_smp_failed_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_smp_failed_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_smp_failed, sizeof(rsi_bt_event_smp_failed_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_encryption_enabled_driver_callback(uint16_t status, rsi_bt_event_encryption_enabled_t *rsi_ble_event_encryption_enabled)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   rsi_bt_event_encryption_enabled_t *rsi_ble_event_encryption_enabled, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_encryption_enabled_driver_callback(
  uint16_t status,
  rsi_bt_event_encryption_enabled_t *rsi_ble_event_encryption_enabled)
{
  LOG_PRINT_D("\n in rsi_ble_event_encryption_enabled_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_bt_event_encryption_enabled_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_encryption_enabled_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  msg->status        = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0],
         (void *)rsi_ble_event_encryption_enabled,
         sizeof(rsi_bt_event_encryption_enabled_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_smp_passkey_display_driver_callback(rsi_bt_event_smp_passkey_display_t *rsi_ble_event_smp_passkey_display)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_bt_event_smp_passkey_display_t *rsi_ble_event_smp_passkey_display, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_smp_passkey_display_driver_callback(
  rsi_bt_event_smp_passkey_display_t *rsi_ble_event_smp_passkey_display)
{
  LOG_PRINT_D("\n in rsi_ble_event_smp_passkey_display_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_bt_event_smp_passkey_display_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_smp_passkey_display_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0],
         (void *)rsi_ble_event_smp_passkey_display,
         sizeof(rsi_bt_event_smp_passkey_display_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_sc_passkey_driver_callback(rsi_bt_event_sc_passkey_t *rsi_ble_event_sc_passkey)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_bt_event_sc_passkey_t *rsi_ble_event_sc_passkey, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_sc_passkey_driver_callback(rsi_bt_event_sc_passkey_t *rsi_ble_event_sc_passkey)
{
  LOG_PRINT_D("\n in rsi_ble_event_sc_passkey_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_bt_event_sc_passkey_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_sc_passkey_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_sc_passkey, sizeof(rsi_bt_event_sc_passkey_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_le_ltk_request_driver_callback(rsi_bt_event_le_ltk_request_t *rsi_ble_event_le_ltk_request)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_bt_event_le_ltk_request_t *rsi_ble_event_le_ltk_request, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_le_ltk_request_driver_callback(rsi_bt_event_le_ltk_request_t *rsi_ble_event_le_ltk_request)
{
  LOG_PRINT_D("\n in rsi_ble_event_le_ltk_request_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_bt_event_le_ltk_request_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_le_ltk_request_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_le_ltk_request, sizeof(rsi_bt_event_le_ltk_request_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_le_security_keys_driver_callback(rsi_bt_event_le_security_keys_t *rsi_ble_event_le_security_keys)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_bt_event_le_security_keys_t *rsi_ble_event_le_security_keys, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_le_security_keys_driver_callback(
  rsi_bt_event_le_security_keys_t *rsi_ble_event_le_security_keys)
{
  LOG_PRINT_D("\n in rsi_ble_event_le_security_keys_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_bt_event_le_security_keys_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_le_security_keys_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;
  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_event_le_security_keys, sizeof(rsi_bt_event_le_security_keys_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**

 * @fn          void rsi_ble_event_data_transmit_driver_callback(uint8_t conn_id)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint8_t conn_id
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_data_transmit_driver_callback(uint8_t conn_id)
{
  LOG_PRINT_D("\n in rsi_ble_data_transmit_driver_callback \n");
  generic_event_message_t *msg;
  UNUSED_PARAMETER(conn_id);

  //! allocate message

  msg = malloc(sizeof(generic_event_message_t) + sizeof(uint8_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = data_transmit_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;

  memcpy((void *)&msg->event_data[0], (void *)&conn_id, sizeof(uint8_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**

 * @fn          void rsi_ble_event_smp_pending_driver_callback(uint8_t *conn_id)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint8_t *conn_id
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_smp_pending_driver_callback(uint8_t *conn_id)
{
  LOG_PRINT_D("\n in rsi_ble_smp_pending_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message

  msg = malloc(sizeof(generic_event_message_t) + sizeof(uint8_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = smp_pending_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;

  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)conn_id, sizeof(uint8_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**

 * @fn          void rsi_ble_event_advertisement_restart_driver_callback(void)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   NULL
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_advertisement_restart_driver_callback(void)
{
  LOG_PRINT_D("\n in rsi_ble_advertisement_restart_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message

  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_conn_info_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = advertisement_restart_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;

  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_conn_info, sizeof(rsi_ble_conn_info_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**

 * @fn          void rsi_ble_event_scan_restart_driver_callback(void)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   NULL
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_scan_restart_driver_callback(void)
{
  LOG_PRINT_D("\n in rsi_ble_scan_restart_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message

  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_conn_info_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = scan_restart_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;

  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)rsi_ble_conn_info, sizeof(rsi_ble_conn_info_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**

 * @fn          void rsi_ble_event_on_data_recieve_driver_callback(uint8_t conn_id)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint8_t conn_id
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_on_data_recieve_driver_callback(uint8_t conn_id)
{
  LOG_PRINT_D("\n in rsi_ble_on_data_recieve_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message

  msg = malloc(sizeof(generic_event_message_t) + sizeof(uint8_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = on_data_recieve_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;

  memcpy((void *)&msg->event_data[0], (void *)&conn_id, sizeof(uint8_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**

 * @fn          void rsi_ble_event_conn_update_req_driver_callback(uint8_t *conn_id)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint8_t *conn_id
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_conn_update_req_driver_callback(uint8_t *conn_id)
{
  LOG_PRINT_D("\n in rsi_ble_conn_update_req_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message

  msg = malloc(sizeof(generic_event_message_t) + sizeof(uint8_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = conn_update_req_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;

  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)conn_id, sizeof(uint8_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**

 * @fn          void rsi_ble_event_set_buffer_config_driver_callback(uint8_t *conn_id)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint8_t *conn_id
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_set_buffer_config_driver_callback(uint8_t *conn_id)
{
  LOG_PRINT_D("\n in rsi_ble_set_buffer_config_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message

  msg = malloc(sizeof(generic_event_message_t) + sizeof(uint8_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = set_buffer_config_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;

  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)conn_id, sizeof(uint8_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**

 * @fn          void rsi_ble_event_select_data_transfer_driver_callback(uint8_t *conn_id)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint8_t *conn_id
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_select_data_transfer_driver_callback(uint8_t *conn_id)
{
  LOG_PRINT_D("\n in rsi_ble_select_data_transfer_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message

  msg = malloc(sizeof(generic_event_message_t) + sizeof(uint8_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = select_data_transfer_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;

  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)conn_id, sizeof(uint8_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**

 * @fn          void rsi_ble_event_profile_discovery_driver_callback(uint8_t *conn_id)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint8_t *conn_id
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_profile_discovery_driver_callback(uint8_t *conn_id)
{
  LOG_PRINT_D("\n in rsi_ble_profile_discovery_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message

  msg = malloc(sizeof(generic_event_message_t) + sizeof(uint8_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = profile_discovery_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;

  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)conn_id, sizeof(uint8_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**

 * @fn          void rsi_ble_event_profile_dummy_driver_callback(profile_dummy_data_t *profile_dummy_data)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint8_t *conn_id, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_profile_dummy_driver_callback(profile_dummy_data_t *profile_dummy_data)
{
  LOG_PRINT_D("\n in rsi_ble_profile_dummy_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message

  msg = malloc(sizeof(generic_event_message_t) + sizeof(profile_dummy_data_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = profile_dummy_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;

  //! copy event data to msg
  memcpy((void *)&msg->event_data[0], (void *)profile_dummy_data, sizeof(profile_dummy_data_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**

 * @fn          void rsi_ble_event_connection_procedure_timeout_driver_callback(uint8_t conn_id)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   uint8_t conn_id
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

APP_WEAK void rsi_ble_event_connection_procedure_timeout_driver_callback(uint8_t conn_id)
{
  LOG_PRINT_D("\n in rsi_ble_connection_procedure_timeout_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message

  msg = malloc(sizeof(generic_event_message_t) + sizeof(uint8_t));
  if (msg == NULL) {
    printf("Out of Memory assert\n");
    _assert((uint8_t *)"Out Of Memory\n", __LINE__);
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = connection_procedure_timeout_event_id;
  //! function to be called to free this message
  msg->free_callback = free;
  //msg->status = status;

  memcpy((void *)&msg->event_data[0], (void *)&conn_id, sizeof(uint8_t));
  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_adv_report(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_adv_report to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_adv_report to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_adv_report(uint16_t __attribute__((unused)) status,
                                       void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_disconnect(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_disconnect to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_disconnect to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_disconnect(uint16_t __attribute__((unused)) status,
                                       void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_conn_status(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_conn_status to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_conn_status to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_conn_status(uint16_t __attribute__((unused)) status,
                                        void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_enhance_conn_status(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_enhance_conn_status to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_enhance_conn_status to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_enhance_conn_status(uint16_t __attribute__((unused)) status,
                                                void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_mtu_exchange_information(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_mtu_exchange_information to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_mtu_exchange_information to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_mtu_exchange_information(uint16_t __attribute__((unused)) status,
                                                     void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_mtu(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_mtu to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_mtu to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_mtu(uint16_t __attribute__((unused)) status, void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_more_data_req(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_le_dev_buf_ind to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_le_dev_buf_ind to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_more_data_req(uint16_t __attribute__((unused)) status,
                                          void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_simple_peripheral_on_remote_features_event(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_remote_features to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_remote_features to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_simple_peripheral_on_remote_features_event(uint16_t __attribute__((unused)) status,
                                                                 void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_data_length_update(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_data_length_update to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_data_length_update to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_data_length_update(uint16_t __attribute__((unused)) status,
                                               void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_phy_update(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_phy_update to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_phy_update to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_phy_update(uint16_t __attribute__((unused)) status,
                                       void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_on_remote_conn_params_request_event(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_remote_conn_param_req to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_remote_conn_param_req to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_on_remote_conn_params_request_event(uint16_t __attribute__((unused)) status,
                                                          void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_conn_update(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_conn_update to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_conn_update to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_conn_update(uint16_t __attribute__((unused)) status,
                                        void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_read_req(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for read_req to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for read_req to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_read_req(uint16_t __attribute__((unused)) status, void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_gatt_error_event(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_error_resp to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_error_resp to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_gatt_error_event(uint16_t __attribute__((unused)) status,
                                       void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_gatt_desc(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_gatt_desc to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_gatt_desc to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_gatt_desc(uint16_t __attribute__((unused)) status,
                                      void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_char_services_event(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_read_by_type1 to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_read_by_type1 to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_char_services_event(uint16_t __attribute__((unused)) status,
                                          void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_profile_event(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_profile_by_uuid to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_profile_by_uuid to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_profile_event(uint16_t __attribute__((unused)) status, void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_profiles_list(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_profiles_list to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_profiles_list to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_profiles_list(uint16_t __attribute__((unused)) status,
                                          void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_on_gatt_write_event(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_write to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_write to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_on_gatt_write_event(uint16_t __attribute__((unused)) status,
                                          void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_on_gatt_prepare_write_event(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_prepare_write to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_prepare_write to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_on_gatt_prepare_write_event(uint16_t __attribute__((unused)) status,
                                                  void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_on_execute_write_event(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for execute_write to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for execute_write to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_on_execute_write_event(uint16_t __attribute__((unused)) status,
                                             void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_on_event_write_resp(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for set_att_resp to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for set_att_resp to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_on_event_write_resp(uint16_t __attribute__((unused)) status,
                                          void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_prepare_write_resp(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for prepare_write_resp to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for prepare_write_resp to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_prepare_write_resp(uint16_t __attribute__((unused)) status,
                                         void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_on_read_resp_event(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_att_value to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_att_value to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_on_read_resp_event(uint16_t __attribute__((unused)) status,
                                         void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_indication_confirmation(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for indication_confirmation to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for indication_confirmation to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_indication_confirmation(uint16_t __attribute__((unused)) status,
                                                    void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_ext_adv_report(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for ae_adv_report to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for ae_adv_report to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_ext_adv_report(uint16_t __attribute__((unused)) status,
                                           void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_extended_per_sync_establish(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for per_adv_sync_estbl to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for per_adv_sync_estbl to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_extended_per_sync_establish(uint16_t __attribute__((unused)) status,
                                                        void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_extended_per_adv_report(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for per_adv_report to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for per_adv_report to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_extended_per_adv_report(uint16_t __attribute__((unused)) status,
                                                    void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_extended_per_sync_lost(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for per_adv_sync_lost to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for per_adv_sync_lost to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_extended_per_sync_lost(uint16_t __attribute__((unused)) status,
                                                   void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_extended_scan_timeout(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for scan_timeout to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for scan_timeout to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_extended_scan_timeout(uint16_t __attribute__((unused)) status,
                                                  void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_on_terminate(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for adv_set_terminated to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for adv_set_terminated to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_on_terminate(uint16_t __attribute__((unused)) status, void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_extended_scan_req_received(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for scan_req_recvd to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for scan_req_recvd to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_extended_scan_req_received(uint16_t __attribute__((unused)) status,
                                                       void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_bt_event_smp_req(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_smp_req to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_smp_req to be executed in ble task context
 *
 */
APP_WEAK void rsi_bt_event_smp_req(uint16_t __attribute__((unused)) status, void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_bt_event_smp_resp(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_smp_resp to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_smp_resp to be executed in ble task context
 *
 */
APP_WEAK void rsi_bt_event_smp_resp(uint16_t __attribute__((unused)) status, void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_bt_event_smp_passkey(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_smp_passkey to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_smp_passkey to be executed in ble task context
 *
 */
APP_WEAK void rsi_bt_event_smp_passkey(uint16_t __attribute__((unused)) status,
                                       void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_bt_event_smp_failed(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_smp_failed to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_smp_failed to be executed in ble task context
 *
 */
APP_WEAK void rsi_bt_event_smp_failed(uint16_t __attribute__((unused)) status,
                                      void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_bt_event_encryption_enabled(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_encryption_enabled to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_encryption_enabled to be executed in ble task context
 *
 */
APP_WEAK void rsi_bt_event_encryption_enabled(uint16_t __attribute__((unused)) status,
                                              void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_bt_event_smp_passkey_display(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_smp_passkey_display to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_smp_passkey_display to be executed in ble task context
 *
 */
APP_WEAK void rsi_bt_event_smp_passkey_display(uint16_t __attribute__((unused)) status,
                                               void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_bt_event_sc_passkey(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_sc_passkey to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_sc_passkey to be executed in ble task context
 *
 */
APP_WEAK void rsi_bt_event_sc_passkey(uint16_t __attribute__((unused)) status,
                                      void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_bt_event_le_ltk_request(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_le_ltk_request to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_le_ltk_request to be executed in ble task context
 *
 */
APP_WEAK void rsi_bt_event_le_ltk_request(uint16_t __attribute__((unused)) status,
                                          void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_bt_event_le_security_keys(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for event_le_security_keys to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_le_security_keys to be executed in ble task context
 *
 */
APP_WEAK void rsi_bt_event_le_security_keys(uint16_t __attribute__((unused)) status,
                                            void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_data_transmit(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for data_transmit to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for data_transmit to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_data_transmit(uint16_t __attribute__((unused)) status,
                                          void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_smp_pending(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for smp_pending to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for smp_pending to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_smp_pending(uint16_t __attribute__((unused)) status,
                                        void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_advertisement_restart(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for advertisement_restart to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for advertisement_restart to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_advertisement_restart(uint16_t __attribute__((unused)) status,
                                                  void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_scan_restart(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for scan_restart to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for scan_restart to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_scan_restart(uint16_t __attribute__((unused)) status,
                                         void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_on_data_recieve(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for on_data_recieve to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for on_data_recieve to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_on_data_recieve(uint16_t __attribute__((unused)) status,
                                            void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_conn_update_req(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for conn_update_req to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for conn_update_req to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_conn_update_req(uint16_t __attribute__((unused)) status,
                                            void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_set_buffer_config(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for set_buffer_config to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for set_buffer_config to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_set_buffer_config(uint16_t __attribute__((unused)) status,
                                              void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_select_data_transfer(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for select_data_transfer to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for select_data_transfer to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_select_data_transfer(uint16_t __attribute__((unused)) status,
                                                 void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_profile_discovery(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for profile_discovery to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for profile_discovery to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_profile_discovery(uint16_t __attribute__((unused)) status,
                                              void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_profile_dummy(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for profile_dummy to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for profile_dummy to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_profile_dummy(uint16_t __attribute__((unused)) status,
                                          void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_connection_procedure_timeout(uint16_t __attribute__((unused))status, void __attribute__((unused))*event_data)
 * @brief       handler for connection_procedure_timeout to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for connection_procedure_timeout to be executed in ble task context
 *
 */
APP_WEAK void rsi_ble_event_connection_procedure_timeout(uint16_t __attribute__((unused)) status,
                                                         void __attribute__((unused)) * event_data)
{
  //! Add handling here
}

void sl_btc_ble_gap_register_callbacks_wrapper()
{
  rsi_ble_gap_register_callbacks(rsi_ble_event_adv_report_driver_callback,
                                 rsi_ble_event_conn_status_driver_callback,
                                 rsi_ble_event_disconnect_driver_callback,
                                 NULL, /* ble_on_le_ping_time_expired_event */
                                 NULL, /* rsi_ble_event_phy_update_driver_callback*/
                                 NULL, /*rsi_ble_event_data_length_update_driver_callback*/
                                 rsi_ble_event_enhance_conn_status_driver_callback,
                                 NULL, /* ble_on_directed_adv_report_event */
                                 rsi_ble_event_conn_update_driver_callback,
                                 rsi_ble_event_remote_conn_param_req_driver_callback);
}
void sl_btc_ble_gatt_register_callbacks_wrapper()
{
  rsi_ble_gatt_register_callbacks(
    NULL, /* rsi_ble_on_profiles_list_resp */
    NULL, /* ble_on_profile_resp */
    NULL, /* ble_on_char_services_resp */
    NULL, /* ble_on_inc_services_resp */
    NULL, /* ble_on_att_desc_resp */
    NULL, /* ble_on_read_resp */
    NULL, /* ble_on_write_resp */
    rsi_ble_event_write_driver_callback,
    rsi_ble_event_prepare_write_driver_callback,
    rsi_ble_execute_write_driver_callback,
    rsi_ble_read_req_driver_callback,
    rsi_ble_event_mtu_driver_callback,
    rsi_ble_event_error_resp_driver_callback,
    NULL, /* rsi_ble_on_gatt_desc_val_event */
    rsi_ble_event_profiles_list_driver_callback,
    rsi_ble_event_profile_by_uuid_driver_callback, //!add profile event
    rsi_ble_event_read_by_type1_driver_callback,
    NULL, /* ble_on_read_by_inc_services_event */
    NULL, /* ble_on_read_att_value_event */
    rsi_ble_event_att_value_driver_callback,
    rsi_ble_set_att_resp_driver_callback,
    rsi_ble_event_indication_confirmation_driver_callback, /* rsi_ble_on_event_indication_confirmation */
    NULL /* rsi_ble_prepare_write_resp_driver_callback */);
}
void sl_btc_ble_gatt_extended_register_callbacks_wrapper()
{
  rsi_ble_gatt_extended_register_callbacks(rsi_ble_event_mtu_exchange_information_driver_callback);
}
void sl_btc_ble_gap_extended_register_callbacks_wrapper()
{
  rsi_ble_gap_extended_register_callbacks(rsi_ble_event_remote_features_driver_callback,
                                          rsi_ble_event_le_dev_buf_ind_driver_callback);
}

void sl_btc_ble_smp_register_callbacks_wrapper()
{
  rsi_ble_smp_register_callbacks(rsi_ble_event_smp_req_driver_callback,
                                 rsi_ble_event_smp_resp_driver_callback,
                                 rsi_ble_event_smp_passkey_driver_callback,
                                 rsi_ble_event_smp_failed_driver_callback,
                                 rsi_ble_event_encryption_enabled_driver_callback,
                                 rsi_ble_event_smp_passkey_display_driver_callback,
                                 rsi_ble_event_sc_passkey_driver_callback,
                                 rsi_ble_event_le_ltk_request_driver_callback,
                                 rsi_ble_event_le_security_keys_driver_callback,
                                 NULL, /* ble_on_cli_smp_response_event */
                                 NULL /* ble_on_sc_method_event not required in this app */);
}