/*******************************************************************************
 * @file  rsi_bt_ble.c
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#ifdef __ZEPHYR__
#include "rsi_bt_common_config.h"
#else
#include "ble_config.h"
#endif

#include "rsi_common.h"
#include "cmsis_os2.h"

#include "sl_wifi_host_interface.h"
#include "sl_si91x_driver.h"

#include "rsi_bt_common.h"
#include "rsi_ble.h"
#include "stdio.h"

#include "sl_si91x_host_interface.h"
#include "rsi_ble_common_config.h"

sl_status_t sli_si91x_allocate_command_buffer(sl_wifi_buffer_t **host_buffer,
                                              void **buffer,
                                              uint32_t requested_buffer_size,
                                              uint32_t wait_duration_ms);
uint32_t rsi_get_bt_state(const rsi_bt_cb_t *bt_cb);

#define BT_SEM     0x1
#define BT_CMD_SEM 0x2

/*
 * Global Variables
 * */

// rsi_bt_ble.c function declarations
void rsi_bt_common_register_callbacks(rsi_bt_get_ber_pkt_t rsi_bt_get_ber_pkt_from_app);
uint32_t rsi_bt_get_timeout(uint16_t cmd_type, uint16_t protocol_type);
uint32_t rsi_bt_get_status(const rsi_bt_cb_t *bt_cb);
void rsi_ble_update_le_dev_buf(const rsi_ble_event_le_dev_buf_ind_t *rsi_ble_event_le_dev_buf_ind);
void rsi_add_remote_ble_dev_info(const rsi_ble_event_enhance_conn_status_t *remote_dev_info);
void rsi_remove_remote_ble_dev_info(const rsi_ble_event_disconnect_t *remote_dev_info);
int32_t rsi_driver_process_bt_resp(
  rsi_bt_cb_t *bt_cb,
  sl_wifi_system_packet_t *pkt,
  void (*rsi_bt_async_callback_handler)(rsi_bt_cb_t *cb, uint16_t type, uint8_t *data, uint16_t length),
  uint16_t protocol_type);
void rsi_ble_on_chip_memory_status_callbacks_register(chip_ble_buffers_stats_handler_t ble_on_chip_memory_status_event);
uint16_t rsi_bt_prepare_common_pkt(uint16_t cmd_type, void *cmd_struct, sl_wifi_system_packet_t *pkt);
uint16_t rsi_bt_prepare_le_pkt(uint16_t cmd_type, void *cmd_struct, sl_wifi_system_packet_t *pkt);
static void rsi_ble_allocate_tx_buffer_check(rsi_bt_cb_t *le_cb, uint8_t inx);
static void rsi_ble_update_buff_for_err_resp(int32_t status);

/*
 Global Variables
 */
/** @addtogroup DRIVER14
* @{
*/
/**
 * @brief       Register the bt-common callbacks
 * @param[in]   rsi_bt_get_ber_pkt_from_app - BER Call back
 * @return      void
 *
 *
 */

void rsi_bt_common_register_callbacks(rsi_bt_get_ber_pkt_t rsi_bt_get_ber_pkt_from_app)
{
  // Get bt cb struct pointer
  rsi_bt_common_specific_cb_t *bt_common_specific_cb = rsi_driver_cb->bt_global_cb->bt_common_specific_cb;

  // Assign the call backs to the respective call back
  bt_common_specific_cb->rsi_bt_get_ber_pkt = rsi_bt_get_ber_pkt_from_app;
}

/**
 * @brief       Determine the BT protocol (BT COMMON / BT classic / BLE ) using the packet type
 * @param[in]   rsp_type - Packet type
 * @param[in]   bt_cb    - BT control back
 * @return      0              - Success \n
 *              Non-Zero Value - Failure
 *
 *
 */

uint16_t rsi_bt_get_proto_type(uint16_t rsp_type, rsi_bt_cb_t **bt_cb)
{
  SL_PRINTF(SL_RSI_BT_GET_PROTO_TYPE_TRIGGER, BLUETOOTH, LOG_INFO, "RESPONSE_TYPE: %2x", rsp_type);
  uint16_t return_value = 0xFF;

  if (rsp_type == RSI_BLE_EVENT_DISCONNECT) {
    if (rsi_driver_cb->bt_common_cb->dev_type == RSI_BT_LE_DEVICE) {
      return_value = RSI_PROTO_BLE;
      *bt_cb       = rsi_driver_cb->ble_cb;
    }

    return return_value;
  }
  /** @} */
  // Determine the protocol type by looking at the packet type
  if ((rsp_type == RSI_BT_EVENT_CARD_READY)
      || ((rsp_type >= RSI_BT_SET_LOCAL_NAME) && (rsp_type <= RSI_BT_GET_LOCAL_DEV_ADDR))
      || ((rsp_type >= RSI_BT_REQ_INIT) && (rsp_type <= RSI_BT_SET_ANTENNA_SELECT))
      || (rsp_type == RSI_BT_SET_FEATURES_BITMAP) || (rsp_type == RSI_BT_SET_ANTENNA_TX_POWER_LEVEL)
      || (rsp_type == RSI_BT_SET_BD_ADDR_REQ) || (rsp_type == RSI_BLE_ONLY_OPER_MODE)
      || (rsp_type == RSI_BLE_REQ_PWRMODE) || (rsp_type == RSI_BLE_REQ_SOFTRESET) || (rsp_type == RSI_BT_REQ_PER_CMD)
      || (rsp_type == RSI_BT_VENDOR_SPECIFIC) || (rsp_type == RSI_BT_GET_BT_STACK_VERSION)
      || (rsp_type == RSI_BT_SET_GAIN_TABLE_OFFSET_OR_MAX_POWER_UPDATE)) {
    return_value = RSI_PROTO_BT_COMMON;
    *bt_cb       = rsi_driver_cb->bt_common_cb;
  } else if (((rsp_type >= RSI_BLE_REQ_ADV) && (rsp_type <= RSI_BLE_REQ_EXECUTE_WRITE))
             || ((rsp_type >= RSI_BLE_ADD_SERVICE) && (rsp_type <= RSI_BLE_CMD_INDICATE))
             || (rsp_type == RSI_BLE_SET_ADVERTISE_DATA)
             || ((rsp_type >= RSI_BLE_GET_LE_PING) && (rsp_type <= RSI_BLE_CMD_READ_RESP))
             || (rsp_type == RSI_BLE_SET_SCAN_RESPONSE_DATA)
             || ((rsp_type >= RSI_BLE_LE_ACCEPT_LIST) && (rsp_type <= RSI_BLE_CBFC_DISCONN))
             || ((rsp_type >= RSI_BLE_LE_LTK_REQ_REPLY) && (rsp_type <= RSI_BLE_PER_RX_MODE))
             || (rsp_type == RSI_BLE_CMD_ATT_ERROR) || (rsp_type == RSI_BLE_CMD_SET_BLE_TX_POWER)
             || (rsp_type == RSI_BLE_CMD_INDICATE_SYNC) || (rsp_type == RSI_BLE_CMD_AE)
             || ((rsp_type >= RSI_BLE_REQ_PROFILES_ASYNC) && (rsp_type <= RSI_BLE_EXECUTE_LONGDESCWRITE_ASYNC))
             || (rsp_type == RSI_BLE_SET_SMP_PAIRING_CAPABILITY_DATA) || (rsp_type == RSI_BLE_REQ_SMP_PAIRING_FAILED)
             || (rsp_type == RSI_BLE_REQ_HCI_RAW) || (rsp_type == RSI_BLE_EVENT_RCP_DATA_RCVD)
             || ((rsp_type >= RSI_BLE_CONN_PARAM_RESP_CMD) && (rsp_type <= RSI_BLE_CMD_MTU_EXCHANGE_RESP))
             || ((rsp_type >= RSI_BLE_EVENT_GATT_ERROR_RESPONSE) && (rsp_type <= RSI_BLE_EVENT_SCAN_REQ_RECVD))
             || (rsp_type == RSI_BLE_REQ_CONN_ENHANCE) || (rsp_type == RSI_BLE_EVENT_REMOTE_DEVICE_INFORMATION)
             || ((rsp_type >= RSI_BLE_CMD_READ_TRANSMIT_POWER) && (rsp_type <= RSI_BLE_CMD_WRITE_RF_PATH_COMP))
             || (rsp_type == RSI_BT_EVT_CONTROLLER_LOGS)) {

    return_value = RSI_PROTO_BLE;
    *bt_cb       = rsi_driver_cb->ble_cb;
  }

  return return_value;
}
/** @addtogroup DRIVER14
* @{
*/

/**
 * @brief      Calculate semaphore wait time for a protocol (BT COMMON / BT classic / BLE )
 * using the packet type
 * @param[in]  cmd_type      - Command Type
 * @param[in]  protocol_type - Protocol type , whether it is BT Common/BT Classic/BLE
 * @return     0              - Success \n
 *             Non-Zero Value - Failure
 *
 *
 */

uint32_t rsi_bt_get_timeout(uint16_t cmd_type, uint16_t protocol_type)
{

  SL_PRINTF(SL_RSI_BT_GET_TIMEOUT_TRIGGER,
            BLUETOOTH,
            LOG_INFO,
            "COMMAND_TYPE: %2x, PROTOCOL_TYPE: %2x",
            cmd_type,
            protocol_type);
  uint32_t return_value = 0; // 0 means RSI_WAIT_FOREVER

  switch (protocol_type) {
    case RSI_PROTO_BT_COMMON: {
      return_value = RSI_BT_COMMON_CMD_RESP_WAIT_TIME;
    } break;
    case RSI_PROTO_BLE: {
      if (((cmd_type >= RSI_BLE_REQ_ADV) && (cmd_type <= RSI_BLE_SMP_PASSKEY))
          || ((cmd_type >= RSI_BLE_SET_ADVERTISE_DATA) && (cmd_type <= RSI_BLE_PER_RX_MODE))
          || (cmd_type == RSI_BLE_CONN_PARAM_RESP_CMD)
          || ((cmd_type == RSI_BLE_MTU_EXCHANGE_REQUEST) || (cmd_type == RSI_BLE_CMD_MTU_EXCHANGE_RESP))) {
        return_value = RSI_BLE_GAP_CMD_RESP_WAIT_TIME;
      } else if (((cmd_type >= RSI_BLE_REQ_PROFILES) && (cmd_type <= RSI_BLE_CMD_INDICATE))
                 || ((cmd_type >= RSI_BLE_CMD_ATT_ERROR) && (cmd_type <= RSI_BLE_SET_SMP_PAIRING_CAPABILITY_DATA))
                 || (cmd_type == RSI_BLE_CMD_INDICATE_CONFIRMATION) || (cmd_type == RSI_BLE_CMD_INDICATE_SYNC)) {
        return_value = RSI_BLE_GATT_CMD_RESP_WAIT_TIME;
      } else {
        return_value = RSI_BT_BLE_CMD_MAX_RESP_WAIT_TIME;
      }
    } break;

    default: {
      return_value = RSI_BT_BLE_CMD_MAX_RESP_WAIT_TIME;
    } break;
  }
  return return_value;
}

/**
 * @brief       Handle BT data transfer completion.
 * @param[in]   pkt - Pointer to packet
 * @return      void
 */

void rsi_bt_common_tx_done(const sl_wifi_system_packet_t *pkt, int32_t status)
{

  SL_PRINTF(SL_RSI_BT_COMMON_TX_DONE, BLUETOOTH, LOG_INFO);
  const uint8_t *host_desc = NULL;
  uint8_t protocol_type    = 0;
  uint16_t rsp_type        = 0;
  rsi_bt_cb_t *bt_cb       = NULL;

  // Get Host Descriptor
  host_desc = pkt->desc;

  // Get Command response Type
  rsp_type = rsi_bytes2R_to_uint16(host_desc + RSI_BT_RSP_TYPE_OFFSET);

  // Get the protocol Type
  protocol_type = (uint8_t)rsi_bt_get_proto_type(rsp_type, &bt_cb);

  if (protocol_type == 0xFF) {
    return;
  }

  // If the command is not a synchronous/blocking one
  if (!bt_cb->sync_rsp || status != RSI_SUCCESS) {
    // Set bt_common status as success
    rsi_bt_set_status(bt_cb, status);

    // Post the semaphore which is waiting on driver_send API
    osSemaphoreRelease(bt_cb->bt_sem);
  }
}
/** @} */
/** @addtogroup DRIVER14
* @{
*/
/**
 *
 * @brief      Return BT status
 * @param[in]  bt_cb - BT control block
 * @return     0              - Success \n
 *             Non-Zero Value - Failure
 */

uint32_t rsi_get_bt_state(const rsi_bt_cb_t *bt_cb)
{
  SL_PRINTF(SL_RSI_BT_STATE_TRIGGER, BLUETOOTH, LOG_INFO);
  return bt_cb->state;
}

/*==============================================*/
/**
 * @brief       Set BT status
 * @param[in]   bt_cb  - BT control block
 * @param[in]   status - Status value to be set
 * @param[out]  None
 * @return      void
 */

void rsi_bt_set_status(rsi_bt_cb_t *bt_cb, int32_t status)
{

  SL_PRINTF(SL_RSI_BT_SET_STATUS_TRIGGER, BLUETOOTH, LOG_INFO, "STATUS: %4x", status);
  bt_cb->status = status;
}

/*==============================================*/
/**
 * @brief       Get bt status
 * @param[in]   bt_cb - BT control block
 * @return      0              - Success \n
 *              Non-Zero Value - Failure
 */
uint32_t rsi_bt_get_status(const rsi_bt_cb_t *bt_cb)
{
  return bt_cb->status;
}

/**
 * @brief       Update local Device buffer availability per peripheral in global ble cb structure
 * @param[in]   void
 * @return      void
 *
 */

void rsi_ble_update_le_dev_buf(const rsi_ble_event_le_dev_buf_ind_t *rsi_ble_event_le_dev_buf_ind)
{

  SL_PRINTF(SL_RSI_BT_UPDATE_LE_DEV_BUF_TRIGGER, BLUETOOTH, LOG_INFO);
  rsi_bt_cb_t *le_cb = rsi_driver_cb->ble_cb;

  for (uint8_t inx = 0; inx < (RSI_BLE_MAX_NBR_PERIPHERALS + RSI_BLE_MAX_NBR_CENTRALS); inx++) {
    if (!memcmp(rsi_ble_event_le_dev_buf_ind->remote_dev_bd_addr,
                le_cb->remote_ble_info[inx].remote_dev_bd_addr,
                RSI_DEV_ADDR_LEN)) {
      if (le_cb->remote_ble_info[inx].ble_buff_mutex) {
        osMutexAcquire(le_cb->remote_ble_info[inx].ble_buff_mutex, 0xFFFFFFFFUL);
        /*get the available buffer count in TA*/
        le_cb->remote_ble_info[inx].avail_buf_cnt += rsi_ble_event_le_dev_buf_ind->avail_buf_cnt;
        osMutexRelease(le_cb->remote_ble_info[inx].ble_buff_mutex);
      }

      if (le_cb->ble_buff_total_mutex) {
        osMutexAcquire(le_cb->ble_buff_total_mutex, 0xFFFFFFFFUL);
        /*get the total available buffer count in TA*/
        le_cb->ble_buff_total_avail_cnt = rsi_ble_event_le_dev_buf_ind->avail_buff_total_cnt;
        osMutexRelease(le_cb->ble_buff_total_mutex);
      }
      break;
    }
  }
}

/**
 * @brief       Update Remote BLE Device info in global ble cb structure
 * @param[in]   void
 * @return      void
 *
 *
 */

void rsi_add_remote_ble_dev_info(const rsi_ble_event_enhance_conn_status_t *remote_dev_info)
{

  SL_PRINTF(SL_RSI_ADD_REMOTE_BLE_DEV_INFO_TRIGGER, BLUETOOTH, LOG_INFO);
  rsi_bt_cb_t *le_cb = rsi_driver_cb->ble_cb;

  for (uint8_t inx = 0; inx < (RSI_BLE_MAX_NBR_PERIPHERALS + RSI_BLE_MAX_NBR_CENTRALS); inx++) {
    if (!le_cb->remote_ble_info[inx].used) {
      memcpy(le_cb->remote_ble_info[inx].remote_dev_bd_addr, remote_dev_info->dev_addr, RSI_DEV_ADDR_LEN);
      le_cb->remote_ble_info[inx].used = 1;
      /* On connection default values are set as follows*/
      le_cb->remote_ble_info[inx].max_buf_cnt    = 1;
      le_cb->remote_ble_info[inx].avail_buf_cnt  = 1;
      le_cb->remote_ble_info[inx].mode           = 1;
      le_cb->remote_ble_info[inx].ble_buff_mutex = osMutexNew(NULL);

      break;
    }
  }
}

/**
 * @brief       Remove Remote BLE Device info in global ble cb structure
 * @param[in]   remote_dev_info - Remote device information
 * @return      void
 *
 *
 */

void rsi_remove_remote_ble_dev_info(const rsi_ble_event_disconnect_t *remote_dev_info)
{

  SL_PRINTF(SL_RSI_REMOVE_REMOTE_BLE_DEV_INFO_TRIGGER, BLUETOOTH, LOG_INFO);
  rsi_bt_cb_t *le_cb = rsi_driver_cb->ble_cb;

  for (uint8_t inx = 0; inx < (RSI_BLE_MAX_NBR_PERIPHERALS + RSI_BLE_MAX_NBR_CENTRALS); inx++) {
    if (!memcmp(remote_dev_info->dev_addr, le_cb->remote_ble_info[inx].remote_dev_bd_addr, RSI_DEV_ADDR_LEN)) {
      memset(le_cb->remote_ble_info[inx].remote_dev_bd_addr, 0, RSI_DEV_ADDR_LEN);
      le_cb->remote_ble_info[inx].used                 = 0;
      le_cb->remote_ble_info[inx].avail_buf_cnt        = 0;
      le_cb->remote_ble_info[inx].cmd_in_use           = 0;
      le_cb->remote_ble_info[inx].max_buf_cnt          = 0;
      le_cb->remote_ble_info[inx].expected_resp        = 0;
      le_cb->remote_ble_info[inx].mode                 = 0;
      le_cb->remote_ble_info[inx].remote_dev_addr_type = 0;
      le_cb->remote_ble_info[inx].max_buf_len          = 0;
      if (le_cb->remote_ble_info[inx].ble_buff_mutex) {
        osMutexDelete(le_cb->remote_ble_info[inx].ble_buff_mutex);
      }
      break;
    }
  }
}

/**
 * @brief       Process BT RX packets
 * @param[in ]  bt_cb    - BT control block
 * @param[in]   pkt      - Pointer to received RX packet
 * @param[in]   bt_aync_callback_handler
 * @return      0              - Success \n
 *              Non-Zero Value - Failure
 *
 */

int32_t rsi_driver_process_bt_resp(
  rsi_bt_cb_t *bt_cb,
  sl_wifi_system_packet_t *pkt,
  void (*rsi_bt_async_callback_handler)(rsi_bt_cb_t *cb, uint16_t type, uint8_t *data, uint16_t length),
  uint16_t protocol_type)
{
  UNUSED_PARAMETER(protocol_type);

  SL_PRINTF(SL_RSI_DRIVER_PROCESS_BT_RESPONSE_TRIGGER, BLUETOOTH, LOG_INFO, "PROTOCOL_TYPE: %2x", protocol_type);
  uint16_t rsp_type        = 0;
  int16_t status           = RSI_SUCCESS;
  const uint8_t *host_desc = NULL;
  uint8_t *payload;
  uint16_t payload_length;
  uint16_t expected_resp = 0;

  // Get Host Descriptor
  host_desc = pkt->desc;

  // Get Command response Type
  rsp_type = rsi_bytes2R_to_uint16(host_desc + RSI_BT_RSP_TYPE_OFFSET);

  // Get Payload start pointer
  payload = pkt->data;

  // Get Payload length
  payload_length = (rsi_bytes2R_to_uint16(host_desc) & 0xFFF);

  // Get Status
  status = rsi_bytes2R_to_uint16(host_desc + RSI_BT_STATUS_OFFSET);

  // Check bt_cb for any task is waiting for response
  if (bt_cb->expected_response_type == rsp_type) {
    // Update the status in bt_cb
    rsi_bt_set_status(bt_cb, status);
    if (bt_cb->expected_response_type == RSI_BT_EVENT_CARD_READY) {
      bt_cb->state = RSI_BT_STATE_OPERMODE_DONE;
    }

    //To not allow BT SetAddress after these states are triggered
    if ((status == RSI_SUCCESS)
        && (bt_cb->expected_response_type == RSI_BLE_REQ_ADV || bt_cb->expected_response_type == RSI_BLE_REQ_SCAN
            || bt_cb->expected_response_type == RSI_BLE_REQ_CONN)) {
      rsi_driver_cb->bt_common_cb->state = RSI_BT_STATE_NONE;
    }
    expected_resp = bt_cb->expected_response_type;
    // Clear expected response type
    bt_cb->expected_response_type = 0;

    // Copy the expected response to response structure/buffer, if any, passed in API
    if (bt_cb->expected_response_buffer != NULL) {
      memcpy(bt_cb->expected_response_buffer, payload, payload_length);

      // Save expected_response pointer to a local variable, since it is being cleared below
      payload = bt_cb->expected_response_buffer;

      // Clear the expected response pointer
      bt_cb->expected_response_buffer = NULL;
    }

    // Check if it is sync response
    if (bt_cb->sync_rsp) {

      /* handling this for the new buf configuration */
      if ((expected_resp == RSI_BLE_RSP_SET_WWO_RESP_NOTIFY_BUF_INFO) && (status == RSI_SUCCESS)) {
        const rsi_ble_set_wo_resp_notify_buf_info_t *buf_info = (rsi_ble_set_wo_resp_notify_buf_info_t *)payload;

        bt_cb->remote_ble_info[bt_cb->remote_ble_index].mode = buf_info->buf_mode;

        if (buf_info->buf_mode == 0) /* small buf cnt */
        {
          bt_cb->remote_ble_info[bt_cb->remote_ble_index].max_buf_cnt   = (buf_info->buf_count * 10);
          bt_cb->remote_ble_info[bt_cb->remote_ble_index].avail_buf_cnt = (buf_info->buf_count * 10);
        } else /* big buf cnt */
        {
          bt_cb->remote_ble_info[bt_cb->remote_ble_index].max_buf_cnt   = buf_info->buf_count;
          bt_cb->remote_ble_info[bt_cb->remote_ble_index].avail_buf_cnt = buf_info->buf_count;
        }
        bt_cb->remote_ble_index = 0; /* assigning value to 0 after successful response */
      }
      // Signal the bt semaphore
      osSemaphoreRelease(bt_cb->bt_sem);
    } else if (rsi_bt_async_callback_handler != NULL) {

      bt_cb->async_status = status;
      // Call callbacks handler
      rsi_bt_async_callback_handler(bt_cb, rsp_type, payload, payload_length);
    }
  } else if (rsi_bt_async_callback_handler != NULL) {
    bt_cb->async_status = status;
    // Call callbacks handler
    rsi_bt_async_callback_handler(bt_cb, rsp_type, payload, payload_length);
  }

  return status;
}

/**
 * @brief       Process BT RX packets
 * @param[in]   pkt      - Pointer to received RX packet
 * @return      0              - Success \n
 *              Non-Zero Value - Failure
 */

uint16_t rsi_driver_process_bt_resp_handler(void *rx_pkt)
{

  SL_PRINTF(SL_RSI_DRIVER_PROCESS_BT_RESP_HANDLER_TRIGGER, BLUETOOTH, LOG_INFO);
  sl_wifi_system_packet_t *pkt                = (sl_wifi_system_packet_t *)rx_pkt;
  const uint8_t *host_desc                    = NULL;
  uint8_t protocol_type                       = 0;
  uint16_t rsp_type                           = 0;
  int16_t status                              = RSI_SUCCESS;
  rsi_bt_cb_t *bt_cb                          = NULL;
  const rsi_ble_event_disconnect_t *temp_data = NULL;

  // Get Host Descriptor
  host_desc = pkt->desc;

  // Get Command response Type
  rsp_type = rsi_bytes2R_to_uint16(host_desc + RSI_BT_RSP_TYPE_OFFSET);

  if (rsp_type == RSI_BLE_EVENT_DISCONNECT) {

    // rsi_driver_cb->bt_common_cb->dev_type = ((rsi_ble_event_disconnect_t *)pkt->data)->dev_type;
    temp_data = (rsi_ble_event_disconnect_t *)pkt->data;
    rsi_driver_cb->bt_common_cb->dev_type =
      ((temp_data->dev_type) & LOWERNIBBLE); //Getting the dev_type from lower nibble
  }

  // Get the protocol Type
  protocol_type = (uint8_t)rsi_bt_get_proto_type(rsp_type, &bt_cb);

  SL_PRINTF(SL_RSI_BT_DRIVER_PROCESS_BT_RESP_HANDLER_TRIGGER, BLUETOOTH, LOG_INFO, "PROTOCOL_TYPE: %1x", protocol_type);
  if (protocol_type == 0xFF) {
    return 0;
  }
  // Call the corresponding protocol process rsp handler
  if (protocol_type == RSI_PROTO_BT_COMMON) {
    // Call BT common process rsp handler
    status = (int16_t)rsi_driver_process_bt_resp(bt_cb, pkt, NULL, protocol_type);
  } else {
    // Call BLE process response handler
    status = (int16_t)rsi_driver_process_bt_resp(bt_cb, pkt, rsi_ble_callbacks_handler, protocol_type);
  }
  return status;
}

/**
 * @brief      Initialize bt control block structure
 * @param[in]  bt_cb - Pointer to bt_cb structure
 * @param[in]  protocol_type - Protocol type
 * @return     0              - Success \n
 *             Non-Zero Value - Failure
 *
 */

int8_t rsi_bt_cb_init(rsi_bt_cb_t *bt_cb, uint16_t protocol_type)
{
  UNUSED_PARAMETER(protocol_type);

  SL_PRINTF(SL_RSI_BT_CB_INIT_TRIGGER, BLUETOOTH, LOG_INFO, "PROTOCOL_TYPE: %2x", protocol_type);

  int8_t retval = 0;

  // validate input parameter
  if (bt_cb == NULL) {
    return RSI_ERROR_INVALID_PARAM;
  }

  // Initialize bt control block with default values
  bt_cb->state  = 0;
  bt_cb->status = 0;

  // Create bt mutex
  bt_cb->expected_response_type   = 0;
  bt_cb->expected_response_buffer = NULL;

  // Create common/bt/ble sync semaphore
  bt_cb->bt_sem = osSemaphoreNew(1, 0, NULL);
  if (bt_cb->bt_sem == NULL) {
    retval = RSI_ERROR_SEMAPHORE_CREATE_FAILED;
  }

  // Create common/bt/ble command semaphore
  bt_cb->bt_cmd_sem = osSemaphoreNew(1, 0, NULL);
  if (bt_cb->bt_cmd_sem == NULL) {
    retval = RSI_ERROR_SEMAPHORE_CREATE_FAILED;
  }

  osSemaphoreRelease(bt_cb->bt_cmd_sem);
  bt_cb->app_buffer = 0;

  return retval;
}

/**
 * @brief      Initialize bt global control block
 * @param[in]  driver_cb - Pointer to bt_cb structure
 * @param[in]  buffer - Buffer
 * @return     0              - Success \n
 *             Non-Zero Value - Failure
 *
 */
uint16_t rsi_bt_global_cb_init(rsi_driver_cb_t *driver_cb, uint8_t *buffer)
{

  SL_PRINTF(SL_RSI_BT_GLOBAL_CB_INIT_TRIGGER, BLUETOOTH, LOG_INFO);
  rsi_bt_global_cb_t *bt_global_cb = driver_cb->bt_global_cb;
  uint16_t total_size              = 0;

  bt_global_cb->ble_specific_cb = (rsi_ble_cb_t *)buffer;

  driver_cb->ble_cb->bt_global_cb = bt_global_cb;
  total_size += sizeof(rsi_ble_cb_t);

  return total_size;
}
/** @} */
/** @addtogroup DRIVER14
* @{
*/
/**
 * @brief      Wait for BT card ready
 * @param[in]  void
 * @return     void
 * */

void rsi_bt_common_init(void)
{
  // Get bt_common_cb structure pointer
  rsi_bt_cb_t *bt_common_cb = rsi_driver_cb->bt_common_cb;

  // Save expected response type
  bt_common_cb->expected_response_type = RSI_BT_EVENT_CARD_READY;
  bt_common_cb->sync_rsp               = 1;

  // Wait on BLE semaphore
  if (bt_common_cb->bt_sem) {
    osSemaphoreAcquire(bt_common_cb->bt_sem, osWaitForever);
  }

  // BT card ready is received
}
/** @} */

/** @addtogroup BT-LOW-ENERGY7
* @{
*/
/*==============================================*/
/**
 * @brief      Register GAP callbacks.
 * @param[in]  ble_on_adv_report_event                  - Callback function for Advertise events
 * @param[in]  ble_on_connect                           - Callback function for Connect events
 * @param[in]  ble_on_disconnect                        - Callback function for Disconnect events
 * @param[in]  timeout_expired_event                    - Callback function for le ping timeout events
 * @param[in]  ble_on_phy_update_complete_event         - Callback function for phy update complete events
 * @param[in]  ble_on_data_length_update_complete_event - Callback function for data length update events
 * @param[in]  ble_on_enhance_conn_status_event         - Callback function for enhanced connection status events
 * @param[in]  ble_on_directed_adv_report_event         - Callback function for directed advertiseing report events
 * @param[in]  ble_on_conn_update_complete_event        - Callback function for conn update complete events
 * @param[in]  ble_on_remote_conn_params_request_event  - Callback function to remote conn params request events
 * @return      void
 */

void rsi_ble_gap_register_callbacks(rsi_ble_on_adv_report_event_t ble_on_adv_report_event,
                                    rsi_ble_on_connect_t ble_on_conn_status_event,
                                    rsi_ble_on_disconnect_t ble_on_disconnect_event,
                                    rsi_ble_on_le_ping_payload_timeout_t ble_on_le_ping_time_expired_event,
                                    rsi_ble_on_phy_update_complete_t ble_on_phy_update_complete_event,
                                    rsi_ble_on_data_length_update_t rsi_ble_on_data_length_update_event,
                                    rsi_ble_on_enhance_connect_t ble_on_enhance_conn_status_event,
                                    rsi_ble_on_directed_adv_report_event_t ble_on_directed_adv_report_event,
                                    rsi_ble_on_conn_update_complete_t ble_on_conn_update_complete_event,
                                    rsi_ble_on_remote_conn_params_request_t ble_on_remote_conn_params_request_event)
{

  SL_PRINTF(SL_RSI_BLE_GAP_REGISTER_CALLBACKS_TRIGGER, BLE, LOG_INFO);
  // Get ble cb struct pointer
  rsi_ble_cb_t *ble_specific_cb = rsi_driver_cb->ble_cb->bt_global_cb->ble_specific_cb;

  // Assign the call backs to the respective call back
  ble_specific_cb->ble_on_adv_report_event                 = ble_on_adv_report_event;
  ble_specific_cb->ble_on_conn_status_event                = ble_on_conn_status_event;
  ble_specific_cb->ble_on_disconnect_event                 = ble_on_disconnect_event;
  ble_specific_cb->ble_on_le_ping_time_expired_event       = ble_on_le_ping_time_expired_event;
  ble_specific_cb->ble_on_phy_update_complete_event        = ble_on_phy_update_complete_event;
  ble_specific_cb->rsi_ble_on_data_length_update_event     = rsi_ble_on_data_length_update_event;
  ble_specific_cb->ble_on_enhance_conn_status_event        = ble_on_enhance_conn_status_event;
  ble_specific_cb->ble_on_directed_adv_report_event        = ble_on_directed_adv_report_event;
  ble_specific_cb->ble_on_conn_update_complete_event       = ble_on_conn_update_complete_event;
  ble_specific_cb->ble_on_remote_conn_params_request_event = ble_on_remote_conn_params_request_event;
}
/*==============================================*/
/**
 * @brief      Register GAP Extended responses/events callbacks.
 * @pre        Device should be initialized before calling this API.
 * @param[in]  ble_on_remote_features_event  - Call back function for Remote feature request
 * @param[in]  ble_on_le_more_data_req_event - Call back function for LE More data request
 * @note        For more information about each callback, please refer to GAP Extended callbacks description section.
 * @return      void
 */

void rsi_ble_gap_extended_register_callbacks(rsi_ble_on_remote_features_t ble_on_remote_features_event,
                                             rsi_ble_on_le_more_data_req_t ble_on_le_more_data_req_event)
{

  SL_PRINTF(SL_RSI_BLE_GAP_EXTENDED_REGISTER_CALLBACKS_TRIGGER, BLE, LOG_INFO);
  // Get ble cb struct pointer
  rsi_ble_cb_t *ble_specific_cb = rsi_driver_cb->ble_cb->bt_global_cb->ble_specific_cb;

  // Assign the call backs to the respective call back
  ble_specific_cb->ble_on_remote_features_event  = ble_on_remote_features_event;
  ble_specific_cb->ble_on_le_more_data_req_event = ble_on_le_more_data_req_event;
}
/** @} */

/** @addtogroup BT-LOW-ENERGY7
* @{
*/
/**
 * @brief         Register the SMP callbacks
 * @param[in]     ble_on_smp_request_event           - smp request callback
 * @param[in]     ble_on_smp_response_event          - smp response callback
 * @param[in]	  ble_on_smp_passkey_event           - smp passkey callback
 * @param[in]	  ble_on_smp_failed_event            - smp failed callback
 * @param[in]	  ble_on_smp_encryptrd               - encription enabled callback
 * @param[in]	  ble_on_smp_passkey_display_event   - smp passkey display callback
 * @param[in]	  ble_sc_passkey_event               - sc passkey display callback
 * @param[in]     ble_on_le_ltk_req_event            - This is the SMP ltk request callback
 * @param[in]     ble_on_le_security_keys_event      -  This is the SMP security keys callback
 * @param[in]	  ble_on_sc_method_event             - sc method display callback
 * @return      void
 *
 */

void rsi_ble_smp_register_callbacks(rsi_ble_on_smp_request_t ble_on_smp_request_event,
                                    rsi_ble_on_smp_response_t ble_on_smp_response_event,
                                    rsi_ble_on_smp_passkey_t ble_on_smp_passkey_event,
                                    rsi_ble_on_smp_failed_t ble_on_smp_failed_event,
                                    rsi_ble_on_encrypt_started_t ble_on_smp_encryptrd,
                                    rsi_ble_on_smp_passkey_display_t ble_on_smp_passkey_display_event,
                                    rsi_ble_on_sc_passkey_t ble_sc_passkey_event,
                                    rsi_ble_on_le_ltk_req_event_t ble_on_le_ltk_req_event,
                                    rsi_ble_on_le_security_keys_t ble_on_le_security_keys_event,
                                    rsi_ble_on_smp_response_t ble_on_cli_smp_response_event,
                                    rsi_ble_on_sc_method_t ble_on_sc_method_event)
{

  SL_PRINTF(SL_RSI_BLE_SMP_REGISTER_CALLBACKS_TRIGGER, BLE, LOG_INFO);
  // Get ble cb struct pointer
  rsi_ble_cb_t *ble_specific_cb = rsi_driver_cb->ble_cb->bt_global_cb->ble_specific_cb;

  // Assign the call backs to the respective call back
  ble_specific_cb->ble_on_smp_request_event      = ble_on_smp_request_event;
  ble_specific_cb->ble_on_smp_response_event     = ble_on_smp_response_event;
  ble_specific_cb->ble_on_smp_passkey_event      = ble_on_smp_passkey_event;
  ble_specific_cb->ble_on_smp_fail_event         = ble_on_smp_failed_event;
  ble_specific_cb->ble_on_smp_encrypt_started    = ble_on_smp_encryptrd;
  ble_specific_cb->ble_on_smp_passkey_display    = ble_on_smp_passkey_display_event;
  ble_specific_cb->ble_on_sc_passkey             = ble_sc_passkey_event;
  ble_specific_cb->ble_on_le_ltk_req_event       = ble_on_le_ltk_req_event;
  ble_specific_cb->ble_on_le_security_keys_event = ble_on_le_security_keys_event;
  ble_specific_cb->ble_on_cli_smp_response_event = ble_on_cli_smp_response_event;
  ble_specific_cb->ble_on_sc_method_event        = ble_on_sc_method_event;
}

/**
 * @brief       Register the GATT callbacks.
 * @param[in]   rsi_ble_on_profiles_list_resp_t         ble_on_profiles_list_resp         - Callback for rsi_ble_get_profiles command
 * @param[in]   rsi_ble_on_profile_resp_t               ble_on_profile_resp               - Callback for rsi_ble_get_profile command
 * @param[in]   rsi_ble_on_char_services_resp_t         ble_on_char_services_resp         - Callback for rsi_ble_get_char_services command
 * @param[in]   rsi_ble_on_inc_services_resp_t          ble_on_inc_services_resp          - Callback for rsi_ble_get_inc_services command
 * @param[in]   rsi_ble_on_att_desc_resp_t              ble_on_att_desc_resp              - Callback for rsi_ble_get_att_descriptors command
 * @param[in]   rsi_ble_on_read_resp_t                  ble_on_read_resp                  - Callback for all read requests command
 * @param[in]   rsi_ble_on_write_resp_t                 ble_on_write_resp                 - Callback for all write commands
 * @param[in]   rsi_ble_on_gatt_write_event_t           blw_on_gatt_event                 - Callback for all GATT events
 * @param[in]   rsi_ble_on_gatt_error_resp_t            ble_on_gatt_error_resp_event      - Callback for GATT error events
 * @param[in]   rsi_ble_on_gatt_desc_val_event_t        ble_on_gatt_desc_val_resp_event   - Callback for GATT descriptor value event
 * @param[in]   rsi_ble_on_event_profiles_list_t        ble_on_profiles_list_event        - Callback function for profiles list event
 * @param[in]   rsi_ble_on_event_profile_by_uuid_t      ble_on_profile_by_uuid_event      - Callback function for profile event
 * @param[in]  rsi_ble_on_event_read_by_char_services_t ble_on_read_by_char_services_event- Callback function for char services event
 * @param[in]  rsi_ble_on_event_read_by_inc_services_t  ble_on_read_by_inc_services_event - Callback function for inc services event
 * @param[in]  rsi_ble_on_event_read_att_value_t        ble_on_read_att_value_event       - Callback function for read att value event
 * @param[in]  rsi_ble_on_event_read_resp_t             ble_on_read_resp_event            - Callback function for read att event
 * @param[in]  rsi_ble_on_event_write_resp_t            ble_on_write_resp_event           - Callback function for write event
 * @param[in]  rsi_ble_on_event_indicate_confirmation_t ble_on_indicate_confirmation_event- Callback function for indicate confirmation event
 * @param[in]  rsi_ble_on_event_prepare_write_resp_t    ble_on_prepare_write_resp_event   - Callback function for prepare write event
 * @return  void
 *
 */

void rsi_ble_gatt_register_callbacks(rsi_ble_on_profiles_list_resp_t ble_on_profiles_list_resp,
                                     rsi_ble_on_profile_resp_t ble_on_profile_resp,
                                     rsi_ble_on_char_services_resp_t ble_on_char_services_resp,
                                     rsi_ble_on_inc_services_resp_t ble_on_inc_services_resp,
                                     rsi_ble_on_att_desc_resp_t ble_on_att_desc_resp,
                                     rsi_ble_on_read_resp_t ble_on_read_resp,
                                     rsi_ble_on_write_resp_t ble_on_write_resp,
                                     rsi_ble_on_gatt_write_event_t ble_on_gatt_event,
                                     rsi_ble_on_gatt_prepare_write_event_t ble_on_gatt_prepare_write_event,
                                     rsi_ble_on_execute_write_event_t ble_on_execute_write_event,
                                     rsi_ble_on_read_req_event_t ble_on_read_req_event,
                                     rsi_ble_on_mtu_event_t ble_on_mtu_event,
                                     rsi_ble_on_gatt_error_resp_t ble_on_gatt_error_resp_event,
                                     rsi_ble_on_gatt_desc_val_event_t ble_on_gatt_desc_val_resp_event,
                                     rsi_ble_on_event_profiles_list_t ble_on_profiles_list_event,
                                     rsi_ble_on_event_profile_by_uuid_t ble_on_profile_by_uuid_event,
                                     rsi_ble_on_event_read_by_char_services_t ble_on_read_by_char_services_event,
                                     rsi_ble_on_event_read_by_inc_services_t ble_on_read_by_inc_services_event,
                                     rsi_ble_on_event_read_att_value_t ble_on_read_att_value_event,
                                     rsi_ble_on_event_read_resp_t ble_on_read_resp_event,
                                     rsi_ble_on_event_write_resp_t ble_on_write_resp_event,
                                     rsi_ble_on_event_indicate_confirmation_t ble_on_indicate_confirmation_event,
                                     rsi_ble_on_event_prepare_write_resp_t ble_on_prepare_write_resp_event)
{

  SL_PRINTF(SL_RSI_BLE_GATT_REGISTER_CALLBACKS_TRIGGER, BLE, LOG_INFO);
  // Get ble specific cb struct pointer
  rsi_ble_cb_t *ble_specific_cb = rsi_driver_cb->ble_cb->bt_global_cb->ble_specific_cb;

  // Assign the call backs to the respective call back
  ble_specific_cb->ble_on_profiles_list_resp          = ble_on_profiles_list_resp;
  ble_specific_cb->ble_on_profile_resp                = ble_on_profile_resp;
  ble_specific_cb->ble_on_char_services_resp          = ble_on_char_services_resp;
  ble_specific_cb->ble_on_inc_services_resp           = ble_on_inc_services_resp;
  ble_specific_cb->ble_on_att_desc_resp               = ble_on_att_desc_resp;
  ble_specific_cb->ble_on_read_resp                   = ble_on_read_resp;
  ble_specific_cb->ble_on_write_resp                  = ble_on_write_resp;
  ble_specific_cb->ble_on_gatt_events                 = ble_on_gatt_event;
  ble_specific_cb->ble_on_prepare_write_event         = ble_on_gatt_prepare_write_event;
  ble_specific_cb->ble_on_execute_write_event         = ble_on_execute_write_event;
  ble_specific_cb->ble_on_read_req_event              = ble_on_read_req_event;
  ble_specific_cb->ble_on_mtu_event                   = ble_on_mtu_event;
  ble_specific_cb->ble_on_gatt_error_resp_event       = ble_on_gatt_error_resp_event;
  ble_specific_cb->ble_on_profiles_list_event         = ble_on_profiles_list_event;
  ble_specific_cb->ble_on_gatt_desc_val_resp_event    = ble_on_gatt_desc_val_resp_event;
  ble_specific_cb->ble_on_profile_by_uuid_event       = ble_on_profile_by_uuid_event;
  ble_specific_cb->ble_on_read_by_char_services_event = ble_on_read_by_char_services_event;
  ble_specific_cb->ble_on_read_by_inc_services_event  = ble_on_read_by_inc_services_event;
  ble_specific_cb->ble_on_read_att_value_event        = ble_on_read_att_value_event;
  ble_specific_cb->ble_on_read_resp_event             = ble_on_read_resp_event;
  ble_specific_cb->ble_on_write_resp_event            = ble_on_write_resp_event;
  ble_specific_cb->ble_on_indicate_confirmation_event = ble_on_indicate_confirmation_event;
  ble_specific_cb->ble_on_prepare_write_resp_event    = ble_on_prepare_write_resp_event;

  return;
}
/**
 * @brief       Register the GATT Extended responses/events callbacks.
 * @param[in]   rsi_ble_on_mtu_exchange_info_t         ble_on_mtu_exchange_info_event         - Call back function for MTU Exchange information Event
 * @return      void
 * 
 */

void rsi_ble_gatt_extended_register_callbacks(rsi_ble_on_mtu_exchange_info_t ble_on_mtu_exchange_info_event)
{
  // Get ble cb struct pointer
  rsi_ble_cb_t *ble_specific_cb = rsi_driver_cb->ble_cb->bt_global_cb->ble_specific_cb;

  // Assign the call backs to the respective call back
  ble_specific_cb->ble_on_mtu_exchange_info_event = ble_on_mtu_exchange_info_event;
}

/**
 * @fn       uint16_t rsi_ble_enhanced_gap_extended_register_callbacks(uint16_t callback_id, void (*callback_handler_ptr)(uint16_t status,
 *                                                uint8_t *buffer))
 * @brief    Register the BLE call back functions.
 * @param[in]  callback_id                       - This is the Id of the call back function following ids are supported:
 * @param[in]  void (*callback_handler_ptr)(void - This is the Call back handler
 * @param[in]  status                            - status of the asynchronous response
 * @param[in]  buffer                            - payload of the asynchronous response
 * @return      0 - Success \n
 *              -53 - Failure \n
 *             If call_back_id is greater than the maximum callbacks to register, returns ref/ RSI_ERROR_BLE_INVALID_CALLBACK_CNT.
 * @note        In callbacks, application should not initiate any TX operation to the module.
 */
uint32_t rsi_ble_enhanced_gap_extended_register_callbacks(uint16_t callback_id,
                                                          void (*callback_handler_ptr)(uint16_t status,
                                                                                       uint8_t *buffer))
{
  // Get BLE cb struct pointer
  rsi_ble_cb_t *ble_specific_cb = rsi_driver_cb->ble_cb->bt_global_cb->ble_specific_cb;

  if (callback_id > RSI_BLE_MAX_NUM_GAP_EXT_CALLBACKS) {

    /*Return , if the callback number exceeds the RSI_BT_COMMON_MAX_NUM_EXT_CALLBACKS */

    return RSI_ERROR_BLE_INVALID_CALLBACK_CNT;
  }
  switch (callback_id) {
    case RSI_BLE_ON_REMOTE_DEVICE_INFORMATION: {
      ble_specific_cb->ble_on_remote_device_info_event = (rsi_ble_on_remote_device_info_t)callback_handler_ptr;
    } break;
    case RSI_BLE_ON_RCP_EVENT: {
      ble_specific_cb->ble_on_rcp_resp_rcvd_event = (rsi_ble_on_rcp_resp_rcvd_t)callback_handler_ptr;
    } break;
    default: {
      LOG_PRINT("\nInvalid Callback ID\n");
      return RSI_ERROR_BLE_INVALID_CALLBACK_CNT;
    }
  }
  return RSI_SUCCESS;
}

/**
 * @fn       uint16_t  rsi_ble_adv_ext_events_register_callbacks  (uint16_t callback_id, void (*callback_handler_ptr)(uint16_t status,
 *                                                uint8_t *buffer))
 * @brief    Register the BLE call back functions.
 * @param[in]  callback_id                       - This is the Id of the call back function following ids are supported:
 * @param[in]  void (*callback_handler_ptr)(void - This is the Call back handler
 * @param[in]  status                            - status of the asynchronous response
 * @param[in]  buffer                            - payload of the asynchronous response
 * @return      0 - Success \n
 *              -53 - Failure \n
 *             If call_back_id is greater than the maximum callbacks to register, returns ref/ RSI_ERROR_BLE_INVALID_CALLBACK_CNT.
 * @note        In callbacks, application should not initiate any TX operation to the module.
 */

int32_t rsi_ble_adv_ext_events_register_callbacks(uint16_t callback_id,
                                                  void (*callback_handler_ptr)(uint16_t status, uint8_t *buffer))
{
  //Get ble cb struct pointer
  rsi_ble_cb_t *ble_specific_cb = rsi_driver_cb->ble_cb->bt_global_cb->ble_specific_cb;

  if (callback_id > RSI_BLE_MAX_NUM_ADV_EXT_EVENT_CALLBACKS) {
    /*
     *Return , if the callback number exceeds the RSI_BLE_MAX_NUM_CALLBACKS ,or
     * the callback is already registered
     */
    return RSI_ERROR_BLE_INVALID_CALLBACK_CNT;
  }

  switch (callback_id) {
    case RSI_BLE_ON_ADV_EXT_ADVERTISE_REPORT_EVENT: {
      ble_specific_cb->ble_ae_report_complete_event = (rsi_ble_ae_report_complete_t)callback_handler_ptr;
    } break;
    case RSI_BLE_ON_ADV_EXT_PERIODIC_ADV_SYNC_ESTBL_EVENT: {
      ble_specific_cb->ble_ae_per_adv_sync_estbl_event = (rsi_ble_ae_per_adv_sync_estbl_t)callback_handler_ptr;
    } break;
    case RSI_BLE_ON_ADV_EXT_PERIODIC_ADVERTISE_REPORT_EVENT: {
      ble_specific_cb->ble_ae_per_adv_report_event = (rsi_ble_ae_per_adv_report_t)callback_handler_ptr;
    } break;
    case RSI_BLE_ON_ADV_EXT_PERIODIC_ADV_SYNC_LOST_EVENT: {
      ble_specific_cb->ble_ae_per_adv_sync_lost_event = (rsi_ble_ae_per_adv_sync_lost_t)callback_handler_ptr;
    } break;
    case RSI_BLE_ON_ADV_EXT_SCAN_TIMEOUT_EVENT: {
      ble_specific_cb->ble_ae_scan_timeout_event = (rsi_ble_ae_scan_timeout_t)callback_handler_ptr;
    } break;
    case RSI_BLE_ON_ADV_EXT_ADVERTISE_SET_TERMINATED_EVENT: {
      ble_specific_cb->ble_ae_adv_set_terminated_event = (rsi_ble_ae_adv_set_terminated_t)callback_handler_ptr;
    } break;
    case RSI_BLE_ON_ADV_EXT_SCAN_REQUEST_RECEIVED_EVENT: {
      ble_specific_cb->ble_ae_scan_req_recvd_event = (rsi_ble_ae_scan_req_recvd_t)callback_handler_ptr;
    } break;
    default:
      return RSI_ERROR_BLE_INVALID_CALLBACK_CNT;
  }

  return RSI_SUCCESS;
}

/** @} */

/*==============================================*/
/**
 * @brief      Register the function pointers for GATT responses
 * @param[in]  ble_on_cbsc_conn_req      - Callback function for CBFC connection request event
 * @param[in]  ble_on_cbsc_conn_complete - Callback function for CBFC connection complete status event
 * @param[in]  ble_on_cbsc_rx_data       - Callback function for CBFC data receive event
 * @param[in]  ble_on_cbsc_disconn       - Callback function for CBFC disconnect event
 * @return     void
 */

void rsi_ble_l2cap_cbsc_register_callbacks(rsi_ble_on_cbfc_conn_req_event_t ble_on_cbsc_conn_req,
                                           rsi_ble_on_cbfc_conn_complete_event_t ble_on_cbsc_conn_complete,
                                           rsi_ble_on_cbfc_rx_data_event_t ble_on_cbsc_rx_data,
                                           rsi_ble_on_cbfc_disconn_event_t ble_on_cbsc_disconn)
{

  SL_PRINTF(SL_RSI_BLE_L2CAP_CBSC_REGISTER_CALLBACKS_TRIGGER, BLE, LOG_INFO);
  // Get ble specific cb struct pointer
  rsi_ble_cb_t *ble_specific_cb = rsi_driver_cb->ble_cb->bt_global_cb->ble_specific_cb;

  // Assign the call backs to the respective call back
  ble_specific_cb->ble_on_cbfc_conn_req_event      = ble_on_cbsc_conn_req;
  ble_specific_cb->ble_on_cbfc_conn_complete_event = ble_on_cbsc_conn_complete;
  ble_specific_cb->ble_on_cbfc_rx_data_event       = ble_on_cbsc_rx_data;
  ble_specific_cb->ble_on_cbfc_disconn_event       = ble_on_cbsc_disconn;

  return;
}

/**
 * @fn       uint16_t rsi_bt_debug_logs_register_callbacks(uint16_t callback_id, void (*callback_handler_ptr)(uint16_t status,
 *                                                uint8_t *buffer))
 * @brief    Register the BT Debug Logs callback functions.
 * @param[in]  callback_id                       - This is the ID of the callback function. The following IDs are supported:
 * @param[in]  void (*callback_handler_ptr)(void - This is the callback handler function.
 * @param[in]  status                            - Status of the asynchronous response.
 * @param[in]  buffer                            - Payload of the asynchronous response.
 * @return      0 - Success \n
 *              -53 - Failure \n
 *              If callback_id is greater than the maximum callbacks to register, returns RSI_ERROR_BLE_INVALID_CALLBACK_CNT.
 * @note        In callbacks, the application should not initiate any TX operation to the module.
 */
uint32_t rsi_bt_debug_logs_register_callbacks(uint16_t callback_id,
                                              void (*callback_handler_ptr)(uint16_t status, uint8_t *buffer))
{
  // Get bt cb struct pointer
  rsi_ble_cb_t *ble_specific_cb = rsi_driver_cb->ble_cb->bt_global_cb->ble_specific_cb;

  if (callback_id > RSI_BT_MAX_NUM_DEBUG_LOGS_CALLBACKS) {
    /*
  *Return , if the callback number exceeds the RSI_BT_MAX_NUM_DEBUG_LOGS_CALLBACKS ,or
  * the callback is already registered
  */
    return RSI_ERROR_BLE_INVALID_CALLBACK_CNT;
  }
  if (callback_id == RSI_BT_ON_CONTROLLER_LOGS) {
    /* Register controller logs call back handler */
    ble_specific_cb->bt_on_controller_logs_event = (rsi_bt_on_controller_logs_t)callback_handler_ptr;
  } else {
    return RSI_ERROR_BLE_INVALID_CALLBACK_CNT;
  }

  return RSI_SUCCESS;
}

/** @addtogroup DRIVER14
* @{
*/
/**
 * @brief      Initailize the BT callbacks register.
 * @param[in]  ble_cb   - BLE control back
 * @param[in]  rsp_type - BLE Packet type
 * @param[in]  payload - Payload
 * @param[in]  payload_length - Payload length
 * @return     void
 *
 */
void rsi_ble_callbacks_handler(rsi_bt_cb_t *ble_cb, uint16_t rsp_type, uint8_t *payload, uint16_t payload_length)
{
  SL_PRINTF(SL_RSI_BLE_CALLBACKS_HANDLER_TRIGGER, BLE, LOG_INFO, "RESPONSE_TYPE: %2x", rsp_type);
  // This statement is added only to resolve compilation warning, value is unchanged
  UNUSED_PARAMETER(payload_length);
  // Get ble cb struct pointer
  const rsi_ble_cb_t *ble_specific_cb = ble_cb->bt_global_cb->ble_specific_cb;
  uint16_t status                     = 0;
  uint16_t sync_status                = 0;
  uint8_t le_cmd_inuse_check          = 0;

  // updating the response status;
  status = (uint16_t)ble_cb->async_status;

  sync_status = (uint16_t)rsi_bt_get_status(ble_cb);

  SL_PRINTF(SL_RSI_BLE_CALLBACKS_HANDLER_STATUS, BLE, LOG_INFO, "STATUS: %2x", status);

  // Check each cmd_type like decode_resp_handler and call the respective callback
  switch (rsp_type) {
    case RSI_BLE_EVENT_ADV_REPORT: {
      if (ble_specific_cb->ble_on_adv_report_event != NULL) {
        ble_specific_cb->ble_on_adv_report_event((rsi_ble_event_adv_report_t *)payload);
      }
    } break;
    case RSI_BLE_EVENT_CONN_STATUS: {
      if (ble_specific_cb->ble_on_conn_status_event != NULL) {
        ((rsi_ble_event_conn_status_t *)payload)->status = status;
        ble_specific_cb->ble_on_conn_status_event((rsi_ble_event_conn_status_t *)payload);
      }
      rsi_add_remote_ble_dev_info((rsi_ble_event_enhance_conn_status_t *)payload);
    } break;

    case RSI_BLE_EVENT_ENHANCE_CONN_STATUS: {
      if (ble_specific_cb->ble_on_enhance_conn_status_event != NULL) {
        ((rsi_ble_event_enhance_conn_status_t *)payload)->status = status;
        ble_specific_cb->ble_on_enhance_conn_status_event((rsi_ble_event_enhance_conn_status_t *)payload);
      }
      rsi_add_remote_ble_dev_info((rsi_ble_event_enhance_conn_status_t *)payload);
    } break;

    case RSI_BLE_EVENT_DISCONNECT: {
      if (ble_specific_cb->ble_on_disconnect_event != NULL) {
        ble_specific_cb->ble_on_disconnect_event((rsi_ble_event_disconnect_t *)payload, status);
      }
      rsi_remove_remote_ble_dev_info((rsi_ble_event_disconnect_t *)payload);
    } break;
    case RSI_BLE_EVENT_GATT_ERROR_RESPONSE: {
      if (ble_specific_cb->ble_on_gatt_error_resp_event != NULL) {
        ble_specific_cb->ble_on_gatt_error_resp_event(status, (rsi_ble_event_error_resp_t *)payload);
      }
      le_cmd_inuse_check = 1;
    } break;
    case RSI_BLE_EVENT_GATT_DESC_VAL_RESPONSE: {
      if (ble_specific_cb->ble_on_gatt_desc_val_resp_event != NULL) {
        ble_specific_cb->ble_on_gatt_desc_val_resp_event(status, (rsi_ble_event_gatt_desc_t *)payload);
      }
      le_cmd_inuse_check = 1;
    } break;

    case RSI_BLE_EVENT_GATT_PRIMARY_SERVICE_LIST: {
      if (ble_specific_cb->ble_on_profiles_list_event != NULL) {
        ble_specific_cb->ble_on_profiles_list_event(status, (rsi_ble_event_profiles_list_t *)payload);
      }
      le_cmd_inuse_check = 1;
    } break;
    case RSI_BLE_EVENT_GATT_PRIMARY_SERVICE_BY_UUID: {
      if (ble_specific_cb->ble_on_profile_by_uuid_event != NULL) {
        ble_specific_cb->ble_on_profile_by_uuid_event(status, (rsi_ble_event_profile_by_uuid_t *)payload);
      }
      le_cmd_inuse_check = 1;
    } break;
    case RSI_BLE_EVENT_GATT_READ_CHAR_SERVS: {
      if (ble_specific_cb->ble_on_read_by_char_services_event != NULL) {
        ble_specific_cb->ble_on_read_by_char_services_event(status, (rsi_ble_event_read_by_type1_t *)payload);
      }
      le_cmd_inuse_check = 1;
    } break;
    case RSI_BLE_EVENT_GATT_READ_INC_SERVS: {
      if (ble_specific_cb->ble_on_read_by_inc_services_event != NULL) {
        ble_specific_cb->ble_on_read_by_inc_services_event(status, (rsi_ble_event_read_by_type2_t *)payload);
      }
      le_cmd_inuse_check = 1;
    } break;
    case RSI_BLE_EVENT_GATT_READ_VAL_BY_UUID: {
      if (ble_specific_cb->ble_on_read_att_value_event != NULL) {
        ble_specific_cb->ble_on_read_att_value_event(status, (rsi_ble_event_read_by_type3_t *)payload);
      }
      le_cmd_inuse_check = 1;
    } break;
    case RSI_BLE_EVENT_GATT_READ_RESP:
    case RSI_BLE_EVENT_GATT_READ_BLOB_RESP:
    case RSI_BLE_EVENT_GATT_READ_MULTIPLE_RESP: {
      if (ble_specific_cb->ble_on_read_resp_event != NULL) {
        ble_specific_cb->ble_on_read_resp_event(status, (rsi_ble_event_att_value_t *)payload);
      }
      le_cmd_inuse_check = 1;
    } break;
    case RSI_BLE_EVENT_GATT_WRITE_RESP:
    case RSI_BLE_EVENT_GATT_EXECUTE_WRITE_RESP: {
      if (ble_specific_cb->ble_on_write_resp_event != NULL) {
        ble_specific_cb->ble_on_write_resp_event(status, (rsi_ble_set_att_resp_t *)payload);
      }
      le_cmd_inuse_check = 1;
    } break;
    case RSI_BLE_EVENT_GATT_INDICATE_CONFIRMATION: {
      if (ble_specific_cb->ble_on_indicate_confirmation_event != NULL) {
        ble_specific_cb->ble_on_indicate_confirmation_event(status, (rsi_ble_set_att_resp_t *)payload);
      }
      le_cmd_inuse_check = 1;
    } break;
    case RSI_BLE_EVENT_GATT_PREPARE_WRITE_RESP: {
      if (ble_specific_cb->ble_on_prepare_write_resp_event != NULL) {
        ble_specific_cb->ble_on_prepare_write_resp_event(status, (rsi_ble_prepare_write_resp_t *)payload);
      }
      le_cmd_inuse_check = 1;
    } break;
    case RSI_BLE_EVENT_SMP_REQUEST: {
      if (ble_specific_cb->ble_on_smp_request_event != NULL) {
        ble_specific_cb->ble_on_smp_request_event((rsi_bt_event_smp_req_t *)payload);
      }
    } break;

    case RSI_BLE_EVENT_SMP_RESPONSE: {
      if (ble_specific_cb->ble_on_smp_response_event != NULL) {
        ble_specific_cb->ble_on_smp_response_event((rsi_bt_event_smp_resp_t *)payload);
      }
    } break;

    case RSI_BLE_EVENT_CLI_SMP_RESPONSE: {
      if (ble_specific_cb->ble_on_cli_smp_response_event != NULL) {
        ble_specific_cb->ble_on_cli_smp_response_event((rsi_bt_event_smp_resp_t *)payload);
      }
    } break;
    case RSI_BLE_EVENT_CHIP_MEMORY_STATS: {
      if (ble_specific_cb->ble_on_chip_memory_status_event != NULL) {
        ble_specific_cb->ble_on_chip_memory_status_event((chip_ble_buffers_stats_t *)payload);
      }
    } break;

    case RSI_BLE_EVENT_SMP_PASSKEY: {
      if (ble_specific_cb->ble_on_smp_passkey_event != NULL) {
        ble_specific_cb->ble_on_smp_passkey_event((rsi_bt_event_smp_passkey_t *)payload);
      }
    } break;

    case RSI_BLE_EVENT_SMP_FAILED: {
      if (ble_specific_cb->ble_on_smp_fail_event != NULL) {
        ble_specific_cb->ble_on_smp_fail_event(status, (rsi_bt_event_smp_failed_t *)payload);
      }
    } break;

    case RSI_BLE_EVENT_SC_METHOD: {
      if (ble_specific_cb->ble_on_sc_method_event != NULL) {
        ble_specific_cb->ble_on_sc_method_event((rsi_bt_event_sc_method_t *)payload);
      }
    } break;

    case RSI_BLE_EVENT_ENCRYPT_STARTED: {
      if (ble_specific_cb->ble_on_smp_encrypt_started != NULL) {
        ble_specific_cb->ble_on_smp_encrypt_started(status, (rsi_bt_event_encryption_enabled_t *)payload);
      }
    } break;
    case RSI_BLE_EVENT_SMP_PASSKEY_DISPLAY_EVENT: {
      if (ble_specific_cb->ble_on_smp_passkey_display != NULL) {
        ble_specific_cb->ble_on_smp_passkey_display((rsi_bt_event_smp_passkey_display_t *)payload);
      }
    } break;
    case RSI_BLE_RSP_PROFILES: {
      if (ble_specific_cb->ble_on_profiles_list_resp != NULL) {
        ble_specific_cb->ble_on_profiles_list_resp(sync_status, (rsi_ble_resp_profiles_list_t *)payload);
      }
    } break;
    case RSI_BLE_RSP_PROFILE: {
      if (ble_specific_cb->ble_on_profile_resp != NULL) {
        ble_specific_cb->ble_on_profile_resp(sync_status, (profile_descriptors_t *)payload);
      }
    } break;
    case RSI_BLE_RSP_CHAR_SERVICES: {
      if (ble_specific_cb->ble_on_char_services_resp != NULL) {
        ble_specific_cb->ble_on_char_services_resp(sync_status, (rsi_ble_resp_char_services_t *)payload);
      }
    } break;
    case RSI_BLE_RSP_INC_SERVICES: {
      if (ble_specific_cb->ble_on_inc_services_resp != NULL) {
        ble_specific_cb->ble_on_inc_services_resp(sync_status, (rsi_ble_resp_inc_services_t *)payload);
      }
    } break;
    case RSI_BLE_RSP_DESC: {
      if (ble_specific_cb->ble_on_att_desc_resp != NULL) {
        ble_specific_cb->ble_on_att_desc_resp(sync_status, (rsi_ble_resp_att_descs_t *)payload);
      }
    } break;
    case RSI_BLE_RSP_READ_BY_UUID:
    case RSI_BLE_RSP_READ_VAL:
    case RSI_BLE_RSP_MULTIPLE_READ:
    case RSI_BLE_RSP_LONG_READ: {
      if (ble_specific_cb->ble_on_read_resp != NULL) {
        ble_specific_cb->ble_on_read_resp(sync_status, rsp_type, (rsi_ble_resp_att_value_t *)payload);
      }
    } break;
    case RSI_BLE_RSP_WRITE:
    case RSI_BLE_RSP_WRITE_NO_ACK:
    case RSI_BLE_RSP_LONG_WRITE:
    case RSI_BLE_RSP_PREPARE_WRITE:
    case RSI_BLE_RSP_EXECUTE_WRITE: {
      if (ble_specific_cb->ble_on_write_resp != NULL) {
        ble_specific_cb->ble_on_write_resp(sync_status, rsp_type);
      }
    } break;
    case RSI_BLE_EVENT_GATT_NOTIFICATION:
    case RSI_BLE_EVENT_GATT_INDICATION:
    case RSI_BLE_EVENT_GATT_WRITE:
      if (ble_specific_cb->ble_on_gatt_events != NULL) {
        ble_specific_cb->ble_on_gatt_events(rsp_type, (rsi_ble_event_write_t *)payload);
      }
      break;
    case RSI_BLE_EVENT_MTU:
      if (ble_specific_cb->ble_on_mtu_event != NULL) {
        ble_specific_cb->ble_on_mtu_event((rsi_ble_event_mtu_t *)payload);
      }
      le_cmd_inuse_check = 1;
      break;
    case RSI_BLE_EVENT_MTU_EXCHANGE_INFORMATION:
      if (ble_specific_cb->ble_on_mtu_exchange_info_event != NULL) {
        ble_specific_cb->ble_on_mtu_exchange_info_event((rsi_ble_event_mtu_exchange_information_t *)payload);
      }
      le_cmd_inuse_check = 1;
      break;
    case RSI_BLE_EVENT_LE_PING_TIME_EXPIRED:
      if (ble_specific_cb->ble_on_le_ping_time_expired_event != NULL) {
        ble_specific_cb->ble_on_le_ping_time_expired_event((rsi_ble_event_le_ping_time_expired_t *)payload);
      }
      break;

    case RSI_BLE_EVENT_PREPARE_WRITE:
      if (ble_specific_cb->ble_on_prepare_write_event != NULL) {
        ble_specific_cb->ble_on_prepare_write_event(rsp_type, (rsi_ble_event_prepare_write_t *)payload);
      }
      break;

    case RSI_BLE_EVENT_EXECUTE_WRITE:
      if (ble_specific_cb->ble_on_execute_write_event != NULL) {
        ble_specific_cb->ble_on_execute_write_event(rsp_type, (rsi_ble_execute_write_t *)payload);
      }
      break;

    case RSI_BLE_EVENT_READ_REQ:
      if (ble_specific_cb->ble_on_read_req_event != NULL) {
        ble_specific_cb->ble_on_read_req_event(rsp_type, (rsi_ble_read_req_t *)payload);
      }
      break;

    case RSI_BLE_EVENT_PHY_UPDATE_COMPLETE: {
      if (ble_specific_cb->ble_on_phy_update_complete_event != NULL) {
        ble_specific_cb->ble_on_phy_update_complete_event((rsi_ble_event_phy_update_t *)payload);
      }
    } break;

    case RSI_BLE_EVENT_DATA_LENGTH_UPDATE_COMPLETE:
      if (ble_specific_cb->rsi_ble_on_data_length_update_event != NULL) {
        ble_specific_cb->rsi_ble_on_data_length_update_event((rsi_ble_event_data_length_update_t *)payload);
      }
      break;

    case RSI_BLE_EVENT_SC_PASSKEY: {
      if (ble_specific_cb->ble_on_sc_passkey != NULL) {
        ble_specific_cb->ble_on_sc_passkey((rsi_bt_event_sc_passkey_t *)payload);
      }
    } break;

    case RSI_BLE_EVENT_DIRECTED_ADV_REPORT: {
      if (ble_specific_cb->ble_on_directed_adv_report_event != NULL) {
        ble_specific_cb->ble_on_directed_adv_report_event((rsi_ble_event_directedadv_report_t *)payload);
      }
    } break;

    case RSI_BLE_EVENT_LE_LTK_REQUEST:
      if (ble_specific_cb->ble_on_le_ltk_req_event != NULL) {
        ble_specific_cb->ble_on_le_ltk_req_event((rsi_bt_event_le_ltk_request_t *)payload);
      }
      break;

    case RSI_BLE_EVENT_SECURITY_KEYS: {
      if (ble_specific_cb->ble_on_le_security_keys_event != NULL) {
        ble_specific_cb->ble_on_le_security_keys_event((rsi_bt_event_le_security_keys_t *)payload);
      }
    } break;

    case RSI_BLE_EVENT_PSM_CONN_REQ: {
      if (ble_specific_cb->ble_on_cbfc_conn_req_event != NULL) {
        ble_specific_cb->ble_on_cbfc_conn_req_event((rsi_ble_event_cbfc_conn_req_t *)payload);
      }
    } break;

    case RSI_BLE_EVENT_PSM_CONN_COMPLETE: {
      if (ble_specific_cb->ble_on_cbfc_conn_complete_event != NULL) {
        ble_specific_cb->ble_on_cbfc_conn_complete_event((rsi_ble_event_cbfc_conn_complete_t *)payload, status);
      }
    } break;

    case RSI_BLE_EVENT_PSM_RX_DATA: {
      if (ble_specific_cb->ble_on_cbfc_rx_data_event != NULL) {
        ble_specific_cb->ble_on_cbfc_rx_data_event((rsi_ble_event_cbfc_rx_data_t *)payload);
      }
    } break;

    case RSI_BLE_EVENT_PSM_DISCONNECT: {
      if (ble_specific_cb->ble_on_cbfc_disconn_event != NULL) {
        ble_specific_cb->ble_on_cbfc_disconn_event((rsi_ble_event_cbfc_disconn_t *)payload);
      }
    } break;

    case RSI_BLE_EVENT_CONN_UPDATE_COMPLETE: {
      if (ble_specific_cb->ble_on_conn_update_complete_event != NULL) {
        ble_specific_cb->ble_on_conn_update_complete_event((rsi_ble_event_conn_update_t *)payload, status);
      }
    } break;
    case RSI_BLE_EVENT_REMOTE_FEATURES: {
      if (ble_specific_cb->ble_on_remote_features_event != NULL) {
        ble_specific_cb->ble_on_remote_features_event((rsi_ble_event_remote_features_t *)payload);
      }
    } break;
    case RSI_BLE_EVENT_LE_MORE_DATA_REQ: {
      rsi_ble_update_le_dev_buf((rsi_ble_event_le_dev_buf_ind_t *)payload);
      if (ble_specific_cb->ble_on_le_more_data_req_event != NULL) {
        ble_specific_cb->ble_on_le_more_data_req_event((rsi_ble_event_le_dev_buf_ind_t *)payload);
      }
    } break;
    case RSI_BLE_EVENT_REMOTE_CONN_PARAMS_REQUEST: {
      if (ble_specific_cb->ble_on_remote_conn_params_request_event != NULL) {
        ble_specific_cb->ble_on_remote_conn_params_request_event((rsi_ble_event_remote_conn_param_req_t *)payload,
                                                                 status);
      }
    } break;
    case RSI_BLE_EVENT_REMOTE_DEVICE_INFORMATION: {
      if (ble_specific_cb->ble_on_remote_device_info_event != NULL) {
        ble_specific_cb->ble_on_remote_device_info_event(status, (rsi_ble_event_remote_device_info_t *)payload);
      }
    } break;

    case RSI_BLE_EVENT_AE_ADVERTISING_REPORT: {
      if (ble_specific_cb->ble_ae_report_complete_event != NULL) {
        ble_specific_cb->ble_ae_report_complete_event(status, (rsi_ble_ae_adv_report_t *)payload);
      }
    } break;
    case RSI_BLE_EVENT_PER_ADV_SYNC_ESTBL: {
      if (ble_specific_cb->ble_ae_per_adv_sync_estbl_event != NULL) {
        ble_specific_cb->ble_ae_per_adv_sync_estbl_event(status, (rsi_ble_per_adv_sync_estbl_t *)payload);
      }
    } break;
    case RSI_BLE_EVENT_PER_ADV_REPORT: {
      if (ble_specific_cb->ble_ae_per_adv_report_event != NULL) {
        ble_specific_cb->ble_ae_per_adv_report_event(status, (rsi_ble_per_adv_report_t *)payload);
      }
    } break;
    case RSI_BLE_EVENT_PER_ADV_SYNC_LOST: {
      if (ble_specific_cb->ble_ae_per_adv_sync_lost_event != NULL) {
        ble_specific_cb->ble_ae_per_adv_sync_lost_event(status, (rsi_ble_per_adv_sync_lost_t *)payload);
      }
    } break;
    case RSI_BLE_EVENT_SCAN_TIMEOUT: {
      if (ble_specific_cb->ble_ae_scan_timeout_event != NULL) {
        ble_specific_cb->ble_ae_scan_timeout_event(status, (rsi_ble_scan_timeout_t *)payload);
      }
    } break;
    case RSI_BLE_EVENT_ADV_SET_TERMINATED: {
      if (ble_specific_cb->ble_ae_adv_set_terminated_event != NULL) {
        ble_specific_cb->ble_ae_adv_set_terminated_event(status, (rsi_ble_adv_set_terminated_t *)payload);
      }
    } break;
    case RSI_BLE_EVENT_SCAN_REQ_RECVD: {
      if (ble_specific_cb->ble_ae_scan_req_recvd_event != NULL) {
        ble_specific_cb->ble_ae_scan_req_recvd_event(status, (rsi_ble_scan_req_recvd_t *)payload);
      }
    } break;
    case RSI_BLE_EVENT_RCP_DATA_RCVD: {
      if (ble_specific_cb->ble_on_rcp_resp_rcvd_event != NULL) {
        ble_specific_cb->ble_on_rcp_resp_rcvd_event(status, (rsi_ble_event_rcp_rcvd_info_t *)payload);
      }
    } break;
    case RSI_BT_EVT_CONTROLLER_LOGS: {
      if (ble_specific_cb->bt_on_controller_logs_event != NULL) {
        ble_specific_cb->bt_on_controller_logs_event(status, (void *)payload);
      }
    } break;
    default:
      break;
  }

  if (le_cmd_inuse_check) {
    const uint8_t *remote_dev_bd_addr = payload;
    for (uint8_t inx = 0; inx < (RSI_BLE_MAX_NBR_PERIPHERALS + RSI_BLE_MAX_NBR_CENTRALS); inx++) {
      if (!memcmp(ble_cb->remote_ble_info[inx].remote_dev_bd_addr, remote_dev_bd_addr, RSI_DEV_ADDR_LEN)) {
        if ((ble_cb->remote_ble_info[inx].cmd_in_use)
            && ((rsp_type == RSI_BLE_EVENT_GATT_ERROR_RESPONSE)
                || (rsp_type == ble_cb->remote_ble_info[inx].expected_resp))) {
          ble_cb->remote_ble_info[inx].cmd_in_use    = 0;
          ble_cb->remote_ble_info[inx].expected_resp = 0;
        }
        break;
      }
    }
  }
}
/**
 * @brief       Chip memory status
 * @param[in]   ble_on_chip_memory_status_event     - Memory status
 * @return      0              - Success \n
 *              Non-Zero Value - Failure
 *
 */
void rsi_ble_on_chip_memory_status_callbacks_register(chip_ble_buffers_stats_handler_t ble_on_chip_memory_status_event)
{

  SL_PRINTF(SL_RSI_BLE_CHIP_MEMORY_STATUS_CALLBACKS_REGISTER, BLE, LOG_INFO);
  // Get ble cb struct pointer
  rsi_ble_cb_t *ble_specific_cb = rsi_driver_cb->ble_cb->bt_global_cb->ble_specific_cb;

  // Assign the call backs to the respective call back
  ble_specific_cb->ble_on_chip_memory_status_event = ble_on_chip_memory_status_event;
}

/**
 * @brief       Form the payload of the BT command packet
 * @param[in]   cmd_type     - Type of the command
 * @param[in]   cmd_stuct    - Pointer of the command structure
 * @param[out]  pkt          - Pointer of the packet to fill the contents of the payload
 * @return      0              - Success \n
 *              Non-Zero Value - Failure
 *
 */

uint16_t rsi_bt_prepare_common_pkt(uint16_t cmd_type, void *cmd_struct, sl_wifi_system_packet_t *pkt)
{

  SL_PRINTF(SL_RSI_BT_PREPARE_COMMON_PACKET_TRIGGER, BLUETOOTH, LOG_INFO, "COMMAND_TYPE: %2x", cmd_type);
  uint16_t payload_size = 0;

  switch (cmd_type) {
    case RSI_BT_SET_LOCAL_NAME: {
      payload_size = sizeof(rsi_bt_req_set_local_name_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;
    case RSI_BT_SET_BD_ADDR_REQ: {
      rsi_bt_cb_t *bt_cb = rsi_driver_cb->bt_common_cb;
      if (bt_cb->state == RSI_BT_STATE_OPERMODE_DONE) {
        bt_cb->state = RSI_BT_STATE_NONE;
        payload_size = sizeof(rsi_bt_set_local_bd_addr_t);
        memcpy(pkt->data, cmd_struct, payload_size);
      }
    } break;

    case RSI_BT_GET_RSSI: {
      payload_size = sizeof(rsi_bt_get_rssi_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;
    case RSI_BT_GET_LOCAL_DEV_ADDR:
    case RSI_BT_GET_LOCAL_NAME:
    case RSI_BT_REQ_INIT:
    case RSI_BT_REQ_DEINIT:
    case RSI_BT_GET_BT_STACK_VERSION:
      break;
    case RSI_BT_SET_ANTENNA_SELECT: {
      payload_size = sizeof(rsi_ble_set_antenna_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;
    case RSI_BT_SET_FEATURES_BITMAP: {
      payload_size = sizeof(rsi_bt_set_feature_bitmap_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;
    case RSI_BT_SET_ANTENNA_TX_POWER_LEVEL: {
      payload_size = sizeof(rsi_bt_set_antenna_tx_power_level_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_ONLY_OPER_MODE: {
      payload_size = sizeof(rsi_ble_oper_mode_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_REQ_PWRMODE: {
      payload_size = sizeof(rsi_ble_power_mode_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;
    case RSI_BLE_REQ_SOFTRESET:
      break;
    case RSI_BT_REQ_PER_CMD: {
      pkt->data[0] = *(uint8_t *)cmd_struct;
      switch (pkt->data[0]) {
        case HCI_BT_PER_STATS_CMD_ID:
          payload_size = 1;
          pkt->data[0] = *(uint8_t *)cmd_struct;
          break;
        case HCI_BLE_TRANSMIT_CMD_ID:
          payload_size = sizeof(rsi_ble_per_transmit_t);
          memcpy(pkt->data, cmd_struct, payload_size);
          break;
        case HCI_BLE_RECEIVE_CMD_ID:
          payload_size = sizeof(rsi_ble_per_receive_t);
          memcpy(pkt->data, cmd_struct, payload_size);
          break;
        default:
          return RSI_ERROR_INVALID_PARAM;
      }
    } break;
    case RSI_BT_VENDOR_SPECIFIC: {
      pkt->data[0] = ((uint8_t *)cmd_struct)[0];
      pkt->data[1] = ((uint8_t *)cmd_struct)[1];
      switch (pkt->data[0] | (pkt->data[1] << 8)) {
        case BLE_VENDOR_RF_TYPE_CMD_OPCODE:
          payload_size = sizeof(rsi_ble_vendor_rf_type_t);
          memcpy(pkt->data, cmd_struct, payload_size);
          break;
        case BLE_VENDOR_ACCEPTLIST_USING_ADV_DATA_PAYLOAD:
          payload_size = sizeof(rsi_ble_req_acceptlist_using_payload_t);
          memcpy(pkt->data, cmd_struct, payload_size);
          break;
        case BLE_VENDOR_ACCEPTLIST_ON_TYPE:
          payload_size = sizeof(rsi_ble_req_acceptlist_on_type_t);
          memcpy(pkt->data, cmd_struct, payload_size);
          break;
        case BLE_VENDOR_SET_COEX_ROLE_PRIORITY:
          payload_size = sizeof(rsi_ble_set_coex_roles_priority_t);
          memcpy(pkt->data, cmd_struct, payload_size);
          break;
        default:
          break;
      }
    } break;
    case RSI_BT_SET_GAIN_TABLE_OFFSET_OR_MAX_POWER_UPDATE: {
      payload_size = sizeof(rsi_bt_cmd_update_gain_table_offset_or_maxpower_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;
    default:
      break;
  }

  // Return payload_size
  return payload_size;
}

/** @} */

/**
 * @brief       Form the payload of the BT Classic command packet.
 * @param[in]   cmd_type     - Type of the command
 * @param[in]   cmd_stuct    - Pointer of the command structure
 * @param[out]  pkt          - Pointer of the packet to fill the contents of the payload
 * @return      0              - Success \n
 *              Non-Zero Value - Failure
 */

uint16_t rsi_bt_prepare_le_pkt(uint16_t cmd_type, void *cmd_struct, sl_wifi_system_packet_t *pkt)
{

  SL_PRINTF(SL_RSI_BT_PREPARE_LE_PKT_TRIGGER, BLUETOOTH, LOG_INFO, "COMMAND_TYPE: %2x", cmd_type);
  uint16_t payload_size       = 0;
  uint8_t le_buf_check        = 0;
  uint8_t le_cmd_inuse_check  = 0;
  uint8_t le_buf_in_use_check = 0;
  uint8_t le_buf_config_check = 0;
  uint8_t pkt_type            = 0;
  uint16_t expected_resp      = 0;
  uint8_t is_it_legacy_cmd    = 0;
  rsi_bt_cb_t *le_cb          = rsi_driver_cb->ble_cb;

  switch (cmd_type) {
    case RSI_BLE_REQ_ADV: {
      payload_size = sizeof(rsi_ble_req_adv_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      is_it_legacy_cmd = 1;
    } break;
    case RSI_BLE_SET_ADVERTISE_DATA: {
      payload_size = sizeof(rsi_ble_req_adv_data_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      is_it_legacy_cmd = 1;
    } break;
    case RSI_BLE_SET_SCAN_RESPONSE_DATA: {
      payload_size = sizeof(rsi_ble_req_scanrsp_data_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      is_it_legacy_cmd = 1;
    } break;
    case RSI_BLE_REQ_SCAN: {
      payload_size = sizeof(rsi_ble_req_scan_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      is_it_legacy_cmd = 1;
    } break;
    case RSI_BLE_REQ_CONN: {
      payload_size = sizeof(rsi_ble_req_conn_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      is_it_legacy_cmd = 1;
    } break;
    case RSI_BLE_REQ_CONN_ENHANCE: {
      payload_size = sizeof(rsi_ble_req_enhance_conn_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      is_it_legacy_cmd = 1;
    } break;
    case RSI_BLE_CMD_CONN_PARAMS_UPDATE: {
      payload_size = sizeof(rsi_ble_cmd_conn_params_update_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_REQ_DISCONNECT: {
      payload_size = sizeof(rsi_ble_req_disconnect_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;
    case RSI_BLE_REQ_START_ENCRYPTION: {
      payload_size = sizeof(rsi_ble_strat_encryption_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_REQ_SMP_PAIR: {
      payload_size = sizeof(rsi_ble_req_smp_pair_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_SMP_PAIR_RESPONSE: {
      payload_size = sizeof(rsi_ble_smp_response_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_SMP_PASSKEY: {
      payload_size = sizeof(rsi_ble_smp_passkey_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_REQ_SMP_PAIRING_FAILED: {
      payload_size = sizeof(rsi_ble_req_smp_pair_failed_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_REQ_HCI_RAW: {
      payload_size = *((uint8_t *)cmd_struct + 3);
      pkt_type     = *(uint8_t *)cmd_struct;
      if (pkt_type == 0x01) {
        payload_size += 3;
      } else {
        payload_size |= ((uint16_t)(*((uint8_t *)cmd_struct + 4)) << 8);
        payload_size += 4;
      }
      memcpy(pkt->data, ((uint8_t *)cmd_struct + 1), payload_size);
      le_cb->sync_rsp = 0;
    } break;

      //GATT cases
    case RSI_BLE_REQ_PROFILES: {
      payload_size = sizeof(rsi_ble_req_profiles_list_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cb->sync_rsp = 0;
    } break;

    case RSI_BLE_REQ_PROFILE: {
      payload_size = sizeof(rsi_ble_req_profile_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cb->sync_rsp = 0;
    } break;

    case RSI_BLE_REQ_CHAR_SERVICES: {
      payload_size = sizeof(rsi_ble_req_char_services_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cb->sync_rsp = 0;
    } break;

    case RSI_BLE_REQ_INC_SERVICES: {
      payload_size = sizeof(rsi_ble_req_inc_services_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cb->sync_rsp = 0;
    } break;

    case RSI_BLE_REQ_READ_BY_UUID: {
      payload_size = sizeof(rsi_ble_req_char_val_by_uuid_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cb->sync_rsp = 0;
    } break;

    case RSI_BLE_REQ_DESC: {
      payload_size = sizeof(rsi_ble_req_att_descs_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cb->sync_rsp = 0;
    } break;

    case RSI_BLE_REQ_READ_VAL: {
      payload_size = sizeof(rsi_ble_req_att_value_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cb->sync_rsp = 0;
    } break;

    case RSI_BLE_REQ_MULTIPLE_READ: {
      payload_size = sizeof(rsi_ble_req_multi_att_values_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cb->sync_rsp = 0;
    } break;

    case RSI_BLE_REQ_LONG_READ: {
      payload_size = sizeof(rsi_ble_req_long_att_value_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cb->sync_rsp = 0;
    } break;

    case RSI_BLE_REQ_WRITE: {
      payload_size = sizeof(rsi_ble_set_att_value_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cb->sync_rsp = 0;
    } break;

    case RSI_BLE_REQ_WRITE_NO_ACK: {
      payload_size = sizeof(rsi_ble_set_att_cmd_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_buf_check = 1;
    } break;

    case RSI_BLE_REQ_LONG_WRITE: {
      payload_size = sizeof(rsi_ble_set_long_att_value_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cb->sync_rsp = 0;
    } break;

    case RSI_BLE_REQ_PREPARE_WRITE: {
      payload_size = sizeof(rsi_ble_req_prepare_write_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cb->sync_rsp = 0;
    } break;

    case RSI_BLE_REQ_EXECUTE_WRITE: {
      payload_size = sizeof(rsi_ble_req_execute_write_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cb->sync_rsp = 0;
    } break;
    case RSI_BLE_ADD_SERVICE: {
      payload_size = sizeof(rsi_ble_req_add_serv_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;
    case RSI_BLE_ADD_ATTRIBUTE: {
      payload_size = sizeof(rsi_ble_req_add_att_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;
    case RSI_BLE_SET_LOCAL_ATT_VALUE: {
      payload_size = sizeof(rsi_ble_set_local_att_value_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;
    case RSI_BLE_CMD_SET_WWO_RESP_NOTIFY_BUF_INFO: {
      payload_size = sizeof(rsi_ble_set_wo_resp_notify_buf_info_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_buf_in_use_check = 1;
      le_buf_config_check = 1;
    } break;
    case RSI_BLE_CMD_NOTIFY: {
      payload_size = sizeof(rsi_ble_notify_att_value_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_buf_check = 1;
    } break;
    case RSI_BLE_CMD_INDICATE: {
      payload_size = sizeof(rsi_ble_notify_att_value_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cmd_inuse_check = 1;
      expected_resp      = RSI_BLE_EVENT_GATT_INDICATE_CONFIRMATION;
    } break;
    case RSI_BLE_CMD_INDICATE_CONFIRMATION: {
      payload_size = sizeof(rsi_ble_indicate_confirm_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_GET_LOCAL_ATT_VALUE: {
      payload_size = sizeof(rsi_ble_get_local_att_value_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;
    case RSI_BLE_GET_LE_PING: {
      payload_size = sizeof(rsi_ble_get_le_ping_timeout_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_SET_LE_PING: {
      payload_size = sizeof(rsi_ble_set_le_ping_timeout_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_ENCRYPT: {
      payload_size = sizeof(rsi_ble_encrypt_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_SET_RANDOM_ADDRESS: {
      uint8_t dummy_rand_addr[6]       = { 0 };
      rsi_ble_req_rand_t *rsi_ble_rand = (rsi_ble_req_rand_t *)pkt->data;
      memcpy(rsi_ble_rand, cmd_struct, sizeof(rsi_ble_req_rand_t));
      if (memcmp(rsi_ble_rand->rand_addr, dummy_rand_addr, 6) == 0) {
        rsi_ascii_dev_address_to_6bytes_rev(rsi_ble_rand->rand_addr, (int8_t *)RSI_BLE_SET_RAND_ADDR);
      }
      // fill payload size
      payload_size = sizeof(rsi_ble_req_rand_t);
    } break;

    case RSI_BLE_CMD_READ_RESP: {
      payload_size = sizeof(rsi_ble_gatt_read_response_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_LE_ACCEPT_LIST: {
      payload_size = sizeof(rsi_ble_accept_list_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_RSP_REMOVE_SERVICE: {
      payload_size = sizeof(rsi_ble_gatt_remove_serv_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_RSP_REMOVE_ATTRIBUTE: {
      payload_size = sizeof(rsi_ble_gatt_remove_att_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_PROCESS_RESOLV_LIST: {
      payload_size = sizeof(rsi_ble_resolvlist_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_GET_RESOLVING_LIST_SIZE:
      break;

    case RSI_BLE_SET_ADDRESS_RESOLUTION_ENABLE: {
      payload_size = sizeof(rsi_ble_set_addr_resolution_enable_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_SET_PRIVACY_MODE: {
      payload_size = sizeof(rsi_ble_set_privacy_mode_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_REQ_READ_PHY: {
      payload_size = sizeof(rsi_ble_req_read_phy_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_REQ_SET_PHY: {
      payload_size = sizeof(rsi_ble_set_phy_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_SET_DATA_LEN: {
      payload_size = sizeof(rsi_ble_setdatalength_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_CBFC_CONN_REQ: {
      payload_size = sizeof(rsi_ble_cbfc_conn_req_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_CBFC_CONN_RESP: {
      payload_size = sizeof(rsi_ble_cbfc_conn_resp_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_CBFC_TX_DATA: {
      payload_size = sizeof(rsi_ble_cbfc_data_tx_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_CBFC_DISCONN: {
      payload_size = sizeof(rsi_ble_cbfc_disconn_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_CMD_ATT_ERROR: {
      payload_size = sizeof(rsi_ble_att_error_response_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_LE_LTK_REQ_REPLY: {
      payload_size = sizeof(rsi_ble_set_le_ltkreqreply_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_RX_TEST_MODE: {
      payload_size = sizeof(rsi_ble_rx_test_mode_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_TX_TEST_MODE: {
      payload_size = sizeof(rsi_ble_tx_test_mode_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;
    case RSI_BLE_REQ_PROFILES_ASYNC: {
      payload_size = sizeof(rsi_ble_req_profiles_list_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cmd_inuse_check = 1;
      expected_resp      = RSI_BLE_EVENT_GATT_PRIMARY_SERVICE_LIST;
    } break;

    case RSI_BLE_REQ_PROFILE_ASYNC: {
      payload_size = sizeof(rsi_ble_req_profile_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cmd_inuse_check = 1;
      expected_resp      = RSI_BLE_EVENT_GATT_PRIMARY_SERVICE_BY_UUID;
    } break;
    case RSI_BLE_GET_CHARSERVICES_ASYNC: {
      payload_size = sizeof(rsi_ble_req_char_services_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cmd_inuse_check = 1;
      expected_resp      = RSI_BLE_EVENT_GATT_READ_CHAR_SERVS;
    } break;

    case RSI_BLE_GET_INCLUDESERVICES_ASYNC: {
      payload_size = sizeof(rsi_ble_req_inc_services_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cmd_inuse_check = 1;
      expected_resp      = RSI_BLE_EVENT_GATT_READ_INC_SERVS;
    } break;

    case RSI_BLE_READCHARVALUEBYUUID_ASYNC: {
      payload_size = sizeof(rsi_ble_req_char_val_by_uuid_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cmd_inuse_check = 1;
      expected_resp      = RSI_BLE_EVENT_GATT_READ_VAL_BY_UUID;
    } break;

    case RSI_BLE_GET_ATTRIBUTE_ASYNC: {
      payload_size = sizeof(rsi_ble_req_att_descs_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cmd_inuse_check = 1;
      expected_resp      = RSI_BLE_EVENT_GATT_DESC_VAL_RESPONSE;
    } break;

    case RSI_BLE_GET_DESCRIPTORVALUE_ASYNC: {
      payload_size = sizeof(rsi_ble_req_att_value_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cmd_inuse_check = 1;
      expected_resp      = RSI_BLE_EVENT_GATT_READ_RESP;
    } break;

    case RSI_BLE_GET_MULTIPLEVALUES_ASYNC: {
      payload_size = sizeof(rsi_ble_req_multi_att_values_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cmd_inuse_check = 1;
      expected_resp      = RSI_BLE_EVENT_GATT_READ_MULTIPLE_RESP;
    } break;

    case RSI_BLE_GET_LONGDESCVALUES_ASYNC: {
      payload_size = sizeof(rsi_ble_req_long_att_value_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cmd_inuse_check = 1;
      expected_resp      = RSI_BLE_EVENT_GATT_READ_BLOB_RESP;
    } break;
    case RSI_BLE_SET_DESCVALUE_ASYNC: {
      payload_size = sizeof(rsi_ble_set_att_value_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cmd_inuse_check = 1;
      expected_resp      = RSI_BLE_EVENT_GATT_WRITE_RESP;
    } break;
    case RSI_BLE_SET_PREPAREWRITE_ASYNC: {
      payload_size = sizeof(rsi_ble_req_prepare_write_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cmd_inuse_check = 1;
      expected_resp      = RSI_BLE_EVENT_GATT_PREPARE_WRITE_RESP;
    } break;
    case RSI_BLE_EXECUTE_LONGDESCWRITE_ASYNC: {
      payload_size = sizeof(rsi_ble_req_execute_write_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cmd_inuse_check = 1;
      expected_resp      = RSI_BLE_EVENT_GATT_EXECUTE_WRITE_RESP;
    } break;
    case RSI_BLE_SET_SMP_PAIRING_CAPABILITY_DATA: {
      payload_size = sizeof(rsi_ble_set_smp_pairing_capabilty_data_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;
    case RSI_BLE_CONN_PARAM_RESP_CMD: {
      payload_size = sizeof(rsi_ble_cmd_conn_param_resp_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_END_TEST_MODE:
      break;
    case RSI_BLE_MTU_EXCHANGE_REQUEST: {
      payload_size = sizeof(rsi_ble_mtu_exchange_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cmd_inuse_check = 1;
      expected_resp      = RSI_BLE_EVENT_MTU;
    } break;
    case RSI_BLE_CMD_MTU_EXCHANGE_RESP: {
      payload_size = sizeof(rsi_ble_mtu_exchange_resp_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;
    case RSI_BLE_CMD_WRITE_RESP: {
      payload_size = sizeof(rsi_ble_gatt_write_response_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_CMD_PREPARE_WRITE_RESP: {
      payload_size = sizeof(rsi_ble_gatt_prepare_write_response_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    case RSI_BLE_CMD_SET_LOCAL_IRK: {
      payload_size = sizeof(rsi_ble_set_local_irk_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;
    case RSI_BLE_CMD_SET_BLE_TX_POWER: {
      payload_size = sizeof(rsi_ble_set_ble_tx_power_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;
    case RSI_BLE_CMD_INDICATE_SYNC: {
      payload_size = sizeof(rsi_ble_notify_att_value_t);
      memcpy(pkt->data, cmd_struct, payload_size);
      le_cb->sync_rsp = 0;
    } break;

    //AE ENABLE
    case RSI_BLE_CMD_AE: {
      payload_size = sizeof(rsi_ble_ae_pdu_t);
      memcpy(pkt->data, cmd_struct, payload_size);

      if (le_cb->state & RSI_BLE_LEGACY_CMD_USED) {
        le_cb->state |= RSI_BLE_CHECK_CMD;
      } else {
        le_cb->state |= RSI_BLE_ADV_EXT_CMD_USED;
      }
    } break;

    case RSI_BLE_CMD_READ_TRANSMIT_POWER: {
      // No TX payload
    } break;

    case RSI_BLE_CMD_READ_RF_PATH_COMP: {
      // No Tx Payload
    } break;

    case RSI_BLE_CMD_WRITE_RF_PATH_COMP: {
      payload_size = sizeof(rsi_ble_write_rf_path_comp_t);
      memcpy(pkt->data, cmd_struct, payload_size);
    } break;

    default:
      break;
  }

  if (le_buf_check || le_cmd_inuse_check || le_buf_in_use_check) {
    const uint8_t *remote_dev_bd_addr = (uint8_t *)cmd_struct;
    for (uint8_t inx = 0; inx < (RSI_BLE_MAX_NBR_PERIPHERALS + RSI_BLE_MAX_NBR_CENTRALS); inx++) {
      if (!memcmp(le_cb->remote_ble_info[inx].remote_dev_bd_addr, remote_dev_bd_addr, RSI_DEV_ADDR_LEN)) {

        /* ERROR PRONE : Do not changes if else checks order */
        if (le_buf_config_check) {
          if (le_cb->remote_ble_info[inx].ble_buff_mutex) {
            osMutexAcquire(le_cb->remote_ble_info[inx].ble_buff_mutex, 0xFFFFFFFFUL);
          }
          if ((le_cb->remote_ble_info[inx].avail_buf_cnt) != (le_cb->remote_ble_info[inx].max_buf_cnt)) {
            le_cb->buf_status = 2; //return error based on the status

            if (le_cb->remote_ble_info[inx].ble_buff_mutex) {
              osMutexRelease(le_cb->remote_ble_info[inx].ble_buff_mutex);
            }
            return payload_size;
          }
          if (le_cb->remote_ble_info[inx].ble_buff_mutex) {
            osMutexRelease(le_cb->remote_ble_info[inx].ble_buff_mutex);
          }
        }

        if (le_buf_in_use_check) {
          le_cb->remote_ble_index = inx;
          break;
        } else if (le_buf_check) {
#if 0
          if (le_cb->remote_ble_info[inx].ble_buff_mutex) {
            osMutexAcquire(le_cb->remote_ble_info[inx].ble_buff_mutex, 0xFFFFFFFFUL);
          }
          if (le_cb->remote_ble_info[inx].avail_buf_cnt == 0) {
            le_cb->buf_status = SI_LE_BUFFER_FULL;
            if (le_cb->remote_ble_info[inx].ble_buff_mutex) {
              osMutexRelease(le_cb->remote_ble_info[inx].ble_buff_mutex);
            }
            break;
          } else {
            le_cb->buf_status = SI_LE_BUFFER_AVL;
            le_cb->remote_ble_info[inx].avail_buf_cnt -= 1;
            if (le_cb->remote_ble_info[inx].ble_buff_mutex) {
              osMutexRelease(le_cb->remote_ble_info[inx].ble_buff_mutex);
            }
            break;
          }
#endif
          rsi_ble_allocate_tx_buffer_check(le_cb, inx);
        } else if (le_cmd_inuse_check) {
          if (le_cb->remote_ble_info[inx].cmd_in_use) {
            le_cb->cmd_status = RSI_TRUE;
          } else {
            le_cb->cmd_status                         = RSI_FALSE;
            le_cb->remote_ble_info[inx].cmd_in_use    = RSI_TRUE;
            le_cb->remote_ble_info[inx].expected_resp = expected_resp;
          }
        }
        break;
      }
    }
  }

  if (is_it_legacy_cmd) {
    if (le_cb->state & RSI_BLE_ADV_EXT_CMD_USED) {
      le_cb->state |= RSI_BLE_CHECK_CMD; //bt_cb->state
    } else {
      le_cb->state |= RSI_BLE_LEGACY_CMD_USED; //bt_cb->state
    }
  }
  // Return payload_size
  return payload_size;
}

/**
 * @brief       Fill commands and places into Bt TX queue
 * @param[in]   cmd          - Type of the command to send
 * @param[in]   cmd_stuct    - Pointer of the packet structure to send
 * @param[in]   resp         - Pointer of the packet to fill the contents of the payload
 * @return      0              - Success \n
 *              Non-Zero Value - Failure
 */

int32_t rsi_bt_driver_send_cmd(uint16_t cmd, void *cmd_struct, void *resp)
{

  SL_PRINTF(SL_RSI_BT_SEND_CMD_TRIGGER, BLUETOOTH, LOG_INFO, "COMMAND: %2x", cmd);
  uint16_t payload_size         = 0;
  uint16_t protocol_type        = 0;
  int32_t status                = RSI_SUCCESS;
  sl_wifi_system_packet_t *pkt  = NULL;
  uint8_t *host_desc            = NULL;
  rsi_bt_cb_t *bt_cb            = NULL;
  uint32_t calculate_timeout_ms = 0;

  sl_wifi_buffer_t *buffer = NULL;

  protocol_type = rsi_bt_get_proto_type(cmd, &bt_cb);

  SL_PRINTF(SL_RSI_BT_SEND_CMD_PROTOCOL_TYPE, BLUETOOTH, LOG_INFO, "PROTOCOL_TYPE: %2x", protocol_type);
  if (protocol_type == 0xFF) {
    // Return packet allocation failure error
    SL_PRINTF(SL_RSI_ERROR_PACKET_ALLOCATION_FAILURE,
              BLUETOOTH,
              LOG_ERROR,
              "COMMAND: %2x , PROTOCOL_TYPE: %2x",
              protocol_type,
              cmd);

    return RSI_ERROR_PKT_ALLOCATION_FAILURE;
  }

  if ((cmd == RSI_BT_SET_BD_ADDR_REQ) && (rsi_driver_cb->bt_common_cb->state != RSI_BT_STATE_OPERMODE_DONE)) {
    SL_PRINTF(SL_RSI_ERROR_COMMAND_GIVEN_IN_WORNG_STATE, BLUETOOTH, LOG_ERROR, "COMMAND: %2x", cmd);

    return RSI_ERROR_COMMAND_GIVEN_IN_WRONG_STATE;
  }
  // Get timeout based on cmd
  calculate_timeout_ms = rsi_bt_get_timeout(cmd, protocol_type);
  if (bt_cb->bt_cmd_sem == NULL || (osSemaphoreAcquire(bt_cb->bt_cmd_sem, calculate_timeout_ms) != osOK)) {
    // LOG_PRINT("%s: Command ID:0x%x Command timed-out with:%d\n",__func__, cmd, calculate_timeout_ms);
    SL_PRINTF(SL_RSI_ERROR_BT_BLE_CMD_IN_PROGRESS,
              BLUETOOTH,
              LOG_ERROR,
              "COMMAND: %2x, Calculate_timeout_ms: %4x",
              cmd,
              calculate_timeout_ms);

    return RSI_ERROR_BT_BLE_CMD_IN_PROGRESS;
  }

  // Allocate command buffer from ble pool
  status = sli_si91x_allocate_command_buffer(&buffer,
                                             (void **)&pkt,
                                             sizeof(sl_wifi_system_packet_t) + RSI_BT_COMMON_CMD_LEN,
                                             calculate_timeout_ms);
  // If allocation of packet fails
  if (pkt == NULL) {
    osSemaphoreRelease(bt_cb->bt_cmd_sem);

    // Return packet allocation failure error
    SL_PRINTF(SL_RSI_ERROR_PKT_ALLOCATION_FAILURE, BLUETOOTH, LOG_ERROR, "COMMAND: %2x", cmd);

    return RSI_ERROR_PKT_ALLOCATION_FAILURE;
  }

  // Memset host descriptor
  memset(pkt->desc, 0, sizeof(pkt->desc));

  // Get host descriptor pointer
  host_desc = (pkt->desc);

  bt_cb->sync_rsp = 1;
  payload_size    = 0;

  if (protocol_type == RSI_PROTO_BT_COMMON) {
    // Memset data
    memset(pkt->data, 0, (RSI_BT_COMMON_CMD_LEN - sizeof(sl_wifi_system_packet_t)));
    payload_size = rsi_bt_prepare_common_pkt(cmd, cmd_struct, pkt);
  } else if (protocol_type == RSI_PROTO_BLE) {
    // Memset data
    memset(pkt->data, 0, (RSI_BLE_CMD_LEN - sizeof(sl_wifi_system_packet_t)));
    payload_size = rsi_bt_prepare_le_pkt(cmd, cmd_struct, pkt);
    if (cmd == RSI_BLE_REQ_CONN_ENHANCE) {
      cmd = RSI_BLE_REQ_CONN;
    }
  }

  if (bt_cb->buf_status || bt_cb->cmd_status || (bt_cb->state & RSI_BLE_CHECK_CMD)) {
    sli_si91x_host_free_buffer(buffer);

    if (bt_cb->buf_status == SI_LE_BUFFER_IN_PROGRESS) {
      status = RSI_ERROR_BLE_DEV_BUF_IS_IN_PROGRESS;
    } else if (bt_cb->buf_status == SI_LE_BUFFER_FULL) {
      status = RSI_ERROR_BLE_DEV_BUF_FULL;
    } else if (bt_cb->cmd_status) {
      status = RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS;
    }

    if (bt_cb->state & RSI_BLE_CHECK_CMD) {
      if (bt_cb->state & RSI_BLE_LEGACY_CMD_USED) {
        status = RSI_ERROR_BLE_ADV_EXT_COMMAND_NOT_ALLOWED;
      } else if (bt_cb->state & RSI_BLE_ADV_EXT_CMD_USED) {
        status = RSI_ERROR_BLE_LEGACY_COMMAND_NOT_ALLOWED;
      }
    }

    bt_cb->buf_status = SI_LE_BUFFER_AVL;
    bt_cb->cmd_status = 0;
    osSemaphoreRelease(bt_cb->bt_cmd_sem);
    SL_PRINTF(SL_RSI_BLE_ERROR, BLUETOOTH, LOG_ERROR, "Status: %4x", status);

    return status;
  }
  if (cmd == RSI_BLE_REQ_HCI_RAW) {
    host_desc[14] = *(uint8_t *)cmd_struct;
  }

  rsi_uint16_to_2bytes(host_desc, (payload_size & 0xFFF));
  rsi_uint16_to_2bytes(&host_desc[2], cmd);

  // Save expected response type
  bt_cb->expected_response_type = cmd;

  // Save expected response type
  bt_cb->expected_response_buffer = resp;

  if (cmd == RSI_BLE_ONLY_OPER_MODE) {
    // Save expected response type
    bt_cb->expected_response_type = RSI_BT_EVENT_CARD_READY;
    bt_cb->sync_rsp               = 1;
  }

  status = sli_si91x_driver_send_bt_command(cmd, SLI_SI91X_BT_CMD, buffer, bt_cb->sync_rsp);
  if (status != SL_STATUS_OK && status != SL_STATUS_IN_PROGRESS) {
    rsi_bt_set_status(bt_cb, status);
    osSemaphoreRelease(bt_cb->bt_cmd_sem);
    SL_PRINTF(SL_RSI_ERROR_BT_COMMAND_SEND, BLUETOOTH, LOG_ERROR, "COMMAND: %2x, STATUS: %4x", cmd, status);
    return status;
  }

  if (bt_cb->bt_sem == NULL || (osSemaphoreAcquire(bt_cb->bt_sem, calculate_timeout_ms) != osOK)) {
    rsi_bt_set_status(bt_cb, RSI_ERROR_RESPONSE_TIMEOUT);
    SL_PRINTF(SL_RSI_SEMAPHORE_TIMEOUT,
              BLUETOOTH,
              LOG_ERROR,
              " Command: %2x , Calculate_timeout_ms: %4x",
              cmd,
              calculate_timeout_ms);
  }
  // Get command response status
  status = rsi_bt_get_status(bt_cb);

  if ((cmd == RSI_BLE_CMD_NOTIFY || cmd == RSI_BLE_REQ_WRITE_NO_ACK)) {
    rsi_ble_update_buff_for_err_resp(status);
  }

  SL_PRINTF(SL_RSI_BT_COMMAND_RESPONSE_STATUS, BLUETOOTH, LOG_INFO, "STATUS: %4x", status);
  // Clear sync rsp variable
  bt_cb->sync_rsp = 0;

  // Post the semaphore which is waiting on driver_send API
  osSemaphoreRelease(bt_cb->bt_cmd_sem);

  // Return status
  return status;
}
/**
 * @fn          static void rsi_ble_allocate_tx_buffer_check(rsi_bt_cb_t *le_cb, uint8_t inx)
 * @brief       Allocates transmit buffers for BLE connections with fair distribution and flow control.
 *              This function implements a sophisticated buffer allocation mechanism that ensures
 *              fair distribution of transmit buffers among all active BLE connections while
 *              preventing buffer starvation and implementing flow control to maintain system stability.
 *
 * @param[in]   le_cb - Pointer to the BLE control block containing connection information,
 *                      buffer counters, and synchronization primitives
 * @param[in]   inx   - Index of the current BLE connection (0 to RSI_BLE_MAX_NBR_SLAVES + RSI_BLE_MAX_NBR_MASTERS - 1)
 *
 * @return      None (void function)
 *
 * @note        This function performs the following operations:
 *              - Calculates buffer requirements for all other active connections
 *              - Implements fair buffer allocation based on connection usage patterns
 *              - Supports two buffer modes: small (20 min buffers) and big (2 min buffers)
 *              - Prevents buffer monopolization by enforcing minimum buffer guarantees
 *              - Updates global and per-connection buffer counters atomically
 *              - Sets buf_status flag to indicate buffer availability (0=available, RSI_TRUE=unavailable)
 *
 * @warning     This function uses mutex locks for thread safety and must be called in a context
 *              where mutex operations are allowed. The function modifies global buffer counters
 *              and connection-specific buffer states.
 */
static void rsi_ble_allocate_tx_buffer_check(rsi_bt_cb_t *le_cb, uint8_t inx)
{
  // Variable to track how many buffers are required for remaining connections
  uint8_t buffs_req_for_remaining_conns = 0;
  // Variable to hold the total available connection event (CE) buffers
  uint8_t total_buff_cp_avail = 0;
  // Flag to indicate if the global CE buffer count should be decremented
  uint8_t ce_dec_flag = 0;
  // Flag to indicate if memory allocation should be skipped for this connection
  uint8_t skip_mem_alloc        = 0;
  uint8_t per_conn_min_buff_cnt = 0;
  uint8_t min_ce_avail          = 0;

  // Input validation
  if (!le_cb || inx >= (RSI_BLE_MAX_NBR_PERIPHERALS + RSI_BLE_MAX_NBR_CENTRALS)) {
    return; // Invalid parameters
  }

  // Get the total available BLE connection event buffer count (global)
  if (le_cb->ble_buff_total_mutex) {
    osMutexAcquire(le_cb->ble_buff_total_mutex, 0xFFFFFFFFUL);
    total_buff_cp_avail = le_cb->ble_buff_total_avail_cnt;
    osMutexRelease(le_cb->ble_buff_total_mutex);
  }

  // Lock the buffer mutex for the current connection to ensure thread safety
  if (le_cb->remote_ble_info[inx].ble_buff_mutex) {
    osMutexAcquire(le_cb->remote_ble_info[inx].ble_buff_mutex, 0xFFFFFFFFUL);
  }

  // Iterate through all connections to count how many still need minimum buffers
  for (uint8_t ch = 0; ch < (RSI_BLE_MAX_NBR_PERIPHERALS + RSI_BLE_MAX_NBR_CENTRALS); ch++) {
    // Check if the connection is in use and not the current one
    if ((le_cb->remote_ble_info[ch].used) && (ch != inx)) {
      // Determine minimum buffer count based on buffer mode (small or big)
      if (le_cb->remote_ble_info[ch].mode == SMALL_BUFF_MODE) {
        per_conn_min_buff_cnt =
          RSI_BLE_PER_CONN_MIN_PACKET_BUFFS_REQUIRED * RSI_BLE_SMALL_BUFF_COUNT_PER_PACKET_CHUNK; // Small buffer mode
      } else {
        per_conn_min_buff_cnt = RSI_BLE_PER_CONN_MIN_PACKET_BUFFS_REQUIRED; // Big buffer mode
      }
      per_conn_min_buff_cnt = (le_cb->remote_ble_info[ch].max_buf_cnt < per_conn_min_buff_cnt)
                                ? le_cb->remote_ble_info[ch].max_buf_cnt
                                : per_conn_min_buff_cnt;

      // If the connection hasn't used any buffers yet
      if (le_cb->remote_ble_info[ch].avail_buf_cnt == le_cb->remote_ble_info[ch].max_buf_cnt) {
        if (le_cb->remote_ble_info[ch].mode == SMALL_BUFF_MODE) {
          per_conn_min_buff_cnt /= RSI_BLE_SMALL_BUFF_COUNT_PER_PACKET_CHUNK;
        }
        buffs_req_for_remaining_conns += per_conn_min_buff_cnt;
      } else {
        // If the connection has used less than the minimum required buffers
        if ((le_cb->remote_ble_info[ch].max_buf_cnt) - (le_cb->remote_ble_info[ch].avail_buf_cnt)
            < per_conn_min_buff_cnt) {
          buffs_req_for_remaining_conns++; // Reserve 1 buffer for partially used connections
        }
      }
    }
  }

  // Determine minimum buffer count for the current connection
  if (le_cb->remote_ble_info[inx].mode == SMALL_BUFF_MODE) {
    per_conn_min_buff_cnt = RSI_BLE_PER_CONN_MIN_PACKET_BUFFS_REQUIRED * RSI_BLE_SMALL_BUFF_COUNT_PER_PACKET_CHUNK;
    min_ce_avail          = RSI_BLE_MIN_CE_BUFFS_REQUIRED_INCASE_SMALL_BUFF_MODE;
  } else {
    per_conn_min_buff_cnt = RSI_BLE_PER_CONN_MIN_PACKET_BUFFS_REQUIRED;
    min_ce_avail          = RSI_BLE_MIN_CE_BUFFS_REQUIRED_INCASE_BIG_BUFF_MODE;
  }

  // Check if there are not enough global buffers left for all connections
  if (total_buff_cp_avail <= buffs_req_for_remaining_conns) {
    // If the current connection has already used enough buffers, skip allocation
    if (((le_cb->remote_ble_info[inx].max_buf_cnt) - (le_cb->remote_ble_info[inx].avail_buf_cnt)
         >= per_conn_min_buff_cnt)
        && ((le_cb->remote_ble_info[inx].max_buf_cnt > per_conn_min_buff_cnt))) {
      skip_mem_alloc = 1;
    }
  }
  // Check if buffer is available for this connection, globally, and not skipping allocation
  // Also ensure enough total buffers are available for all connections
  if ((le_cb->remote_ble_info[inx].avail_buf_cnt == 0) || (total_buff_cp_avail < min_ce_avail) || (skip_mem_alloc)) {
    // Buffer not available or allocation skipped, set error status and unlock mutex
    le_cb->buf_status = RSI_TRUE; // Set error status (buffer full)
    if (le_cb->remote_ble_info[inx].ble_buff_mutex) {
      osMutexRelease(le_cb->remote_ble_info[inx].ble_buff_mutex);
    }
  } else {
    le_cb->buf_status = 0; // Buffer allocation is OK
    // Decrement buffer counters based on buffer mode
    if (le_cb->remote_ble_info[inx].mode == SMALL_BUFF_MODE) { // Small buffer mode
      // In small buffer mode, decrement global buffer count every 10 packets
      // This happens when (max_buf_cnt - avail_buf_cnt) is a multiple of 10
      if (!((le_cb->remote_ble_info[inx].max_buf_cnt - le_cb->remote_ble_info[inx].avail_buf_cnt)
            % RSI_BLE_SMALL_BUFF_COUNT_PER_PACKET_CHUNK)) {
        ce_dec_flag = 1;
      }
    } else { // Big buffer mode
      // In big buffer mode, decrement global buffer count for every packet
      ce_dec_flag = 1;
    }
    // Decrement the available buffer count for this connection
    le_cb->remote_ble_info[inx].avail_buf_cnt -= 1;
    if (le_cb->remote_ble_info[inx].ble_buff_mutex) {
      osMutexRelease(le_cb->remote_ble_info[inx].ble_buff_mutex);
    }
    // If required, decrement the global total buffer count
    if (ce_dec_flag) {
      if (le_cb->ble_buff_total_mutex) {
        osMutexAcquire(le_cb->ble_buff_total_mutex, 0xFFFFFFFFUL);
        le_cb->ble_buff_total_avail_cnt -= 1;
        osMutexRelease(le_cb->ble_buff_total_mutex);
      }
    }
    le_cb->remote_ble_index = inx;
  }
}

static void rsi_ble_update_buff_for_err_resp(int32_t status)
{
  rsi_bt_cb_t *le_cb = rsi_driver_cb->ble_cb;
  if (status == SL_STATUS_SI91X_ERROR_BLE_HW_BUF_OVERFLOW) {
    if (le_cb->remote_ble_info[le_cb->remote_ble_index].ble_buff_mutex) {
      osMutexAcquire(le_cb->remote_ble_info[le_cb->remote_ble_index].ble_buff_mutex, 0xFFFFFFFFUL);
    }

    le_cb->remote_ble_info[le_cb->remote_ble_index].avail_buf_cnt += 1;

    if (le_cb->remote_ble_info[le_cb->remote_ble_index].ble_buff_mutex) {
      osMutexRelease(le_cb->remote_ble_info[le_cb->remote_ble_index].ble_buff_mutex);
    }

    le_cb->remote_ble_index = 0;
  }
}
/** @} */

/*==============================================*/
