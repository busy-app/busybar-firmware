
/*=======================================================================*/
//   ! INCLUDES
/*=======================================================================*/
#include <rsi_common_app.h>
#include "ble_private.h"
#include "ble_event_hdlr_auto_gen.h"
#include "rsi_common_utils.h"
#include "rsi_ble.h"
#include "rsi_ble_common_config.h"
#include "sl_si91x_status.h"
#include "rsi_common_apis.h"

extern rsi_parsed_conf_t rsi_parsed_conf;

ble_confg_info_t ble_confgs;
profile_dummy_data_t profile_dummy_data;
uint8_t central_count    = 0;
uint8_t peripheral_count = 0;
adv_state_t adv_state_dut;
scan_state_t scan_state_dut;
rsi_ble_dev_ltk_list_t ble_dev_ltk_list[TOTAL_CONNECTIONS];

static uint8_t remote_dev_addr_conn[RSI_REM_DEV_ADDR_LEN] = { 0 };
static uint8_t remote_dev_addr[RSI_REM_DEV_ADDR_LEN]      = { 0 };
static int8_t central_conn_id[2];
static uint8_t remote_name[31];
static rsi_bt_event_le_security_keys_t temp_le_sec_keys;
static rsi_bt_event_encryption_enabled_t glbl_enc_enabled;
static rsi_ble_event_ctkd_t ble_ctkd;

tx_generic_event_message_t transmit_event_message[TOTAL_CONNECTIONS];

more_data_state_beta_t more_data_state_beta[TOTAL_CONNECTIONS];

void rsi_ble_event_data_transmit_driver_callback(uint8_t conn_id)

{
  LOG_PRINT_D(" \n in rsi_ble_event_data_transmit_driver_callback \n");

  transmit_event_message[conn_id].next          = NULL;
  transmit_event_message[conn_id].event_id      = data_transmit_event_id;
  transmit_event_message[conn_id].free_callback = NULL;

  //! copy event data to msg2
  memcpy((void *)&transmit_event_message[conn_id].event_data, &conn_id, sizeof(uint8_t));

  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[conn_id],
                                 (rsi_app_pkt_t *)&transmit_event_message[conn_id],
                                 &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}

/**

 * @fn          void rsi_ble_event_scan_restart_driver_callback(rsi_ble_conn_info_t *rsi_ble_conn_info)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_conn_info_t *rsi_ble_conn_info 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

void rsi_ble_event_scan_restart_driver_callback(void)
{
  LOG_PRINT(" \n in rsi_ble_event_scan_restart_driver_callback \n");
  generic_event_message_t *msg3;

  //! allocate message

  msg3 = malloc(sizeof(generic_event_message_t));

  //! assert if malloc failed
  if (msg3 == NULL) {
    LOG_PRINT("Out of Memory //assert1\n");
    //assert("Out Of Memory\n");
  } else {
    LOG_PRINT_D("malloc passed\n");
  }
  //! init messag details
  msg3->next     = NULL;
  msg3->event_id = scan_restart_event_id;
  //! function to be called to free this message
  msg3->free_callback = free;
  //msg2->status = status;

  //! copy event data to msg2
  //memcpy((void *)&msg3->event_data[0], (void *)rsi_ble_conn_info, sizeof(rsi_ble_conn_info_t));

  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg3, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}
/*==============================================*/
/*==============================================*/
/**

 * @fn          void rsi_ble_event_advertisement_restart_driver_callback(rsi_ble_conn_info_t *rsi_ble_conn_info)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_conn_info_t *rsi_ble_conn_info 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

void rsi_ble_event_advertisement_restart_driver_callback(void)

{
  LOG_PRINT_D(" \n in rsi_ble_event_advertisement_restart_driver_callback \n");
  generic_event_message_t *msg3;

  //! allocate message

  msg3 = malloc(sizeof(generic_event_message_t));

  //! assert if malloc failed
  if (msg3 == NULL) {
    LOG_PRINT("Out of Memory assert//assert 55\n");
    //assert("Out Of Memory\n");
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg3->next     = NULL;
  msg3->event_id = advertisement_restart_event_id;
  //! function to be called to free this message
  msg3->free_callback = free;

  //! enqueue message to ble_generic_cb.event_queues[0]
  rsi_app_enqueue_pkt_with_mutex(&ble_generic_cb.event_queues[0], (rsi_app_pkt_t *)msg3, &ble_generic_cb.event_mutex);
  osSemaphoreRelease(ble_generic_cb.semaphore);
}
/*==============================================*/
/**
 * @fn         add_security_keys_to_device_list
 * @brief      this function will add device to resolvlistwith updated irks 
 * @param[out] ix, index of resolvlist
 * @return     none.
 * @section description
 * add device to resolvlistwith updated irks 
 * */
int8_t add_security_keys_to_device_list(rsi_ble_dev_ltk_list_t *ble_dev_ltk_list,
                                        rsi_bt_event_le_security_keys_t le_sec_keys)
{
  uint8_t status = RSI_SUCCESS;
  uint8_t ix;
  for (ix = 0; ix < TOTAL_CONNECTIONS; ix++) {

    if ((ble_dev_ltk_list[ix].used == 1) && (!memcmp(ble_dev_ltk_list[ix].remote_dev_addr, le_sec_keys.dev_addr, 6))
        && (ble_dev_ltk_list[ix].remote_dev_addr_type == le_sec_keys.dev_addr_type)) {
      memcpy(ble_dev_ltk_list[ix].local_irk, le_sec_keys.local_irk, 16);
      memcpy(ble_dev_ltk_list[ix].peer_irk, le_sec_keys.remote_irk, 16);
      memcpy(ble_dev_ltk_list[ix].remote_rand, le_sec_keys.remote_rand, 8);
      memcpy(ble_dev_ltk_list[ix].remote_ltk, le_sec_keys.remote_ltk, 16);
      memcpy(ble_dev_ltk_list[ix].Identity_addr, le_sec_keys.Identity_addr, 6);
      ble_dev_ltk_list[ix].remote_ediv        = le_sec_keys.remote_ediv;
      ble_dev_ltk_list[ix].Identity_addr_type = le_sec_keys.Identity_addr_type;
      break;
    }
  }
  if (ix >= TOTAL_CONNECTIONS) {
    return -1;
  }

  return status;
}
/*==============================================*/
/**
 * @fn         add_device_to_ltk_key_list
 * @brief      this function will add device to the ltk key list
 * @param[in]  ltk device list pointer
 * @param[in]  encrypt start event pointer
 * @param[out] status
 * @section description
 * add device to ltk key list
 * */
int8_t add_device_to_ltk_key_list(rsi_ble_dev_ltk_list_t *ble_dev_ltk_list,
                                  rsi_bt_event_encryption_enabled_t *enc_enabled)
{
  int8_t status = 0;
  uint8_t ix;
  uint8_t index_found = 0;

  for (ix = 0; ix < TOTAL_CONNECTIONS; ix++) {

    if (ble_dev_ltk_list[ix].used == 1) {
      if ((enc_enabled->dev_addr_type > 1) && (!memcmp(enc_enabled->dev_addr, ble_dev_ltk_list[ix].Identity_addr, 6))) {
        index_found = 1;
        LOG_PRINT("index found = %d\n", ix);
        break;
      }

      if ((enc_enabled->dev_addr_type <= 1)
          && (!memcmp(enc_enabled->dev_addr, ble_dev_ltk_list[ix].remote_dev_addr, 6))) {
        index_found = 1;
        LOG_PRINT("index found = %d\n", ix);
        break;
      }
    }

    if (ble_dev_ltk_list[ix].used == 0) {
      ble_dev_ltk_list[ix].used       = 1;
      ble_dev_ltk_list[ix].enc_enable = enc_enabled->enabled;
      ble_dev_ltk_list[ix].sc_enable  = enc_enabled->sc_enable;
      memcpy(ble_dev_ltk_list[ix].remote_dev_addr, enc_enabled->dev_addr, 6);
      memcpy(ble_dev_ltk_list[ix].localltk, enc_enabled->localltk, 16);
      memcpy(ble_dev_ltk_list[ix].localrand, enc_enabled->localrand, 8);
      ble_dev_ltk_list[ix].local_ediv = enc_enabled->localediv;
      break;
    }
  }
  if (index_found == 1) {
    LOG_PRINT_D("\n Add device to LTK list and its index = %d \n", status);
    ble_dev_ltk_list[ix].enc_enable = enc_enabled->enabled;
    ble_dev_ltk_list[ix].sc_enable  = enc_enabled->sc_enable;
    memcpy(ble_dev_ltk_list[ix].remote_dev_addr, enc_enabled->dev_addr, 6);
    memcpy(ble_dev_ltk_list[ix].localltk, enc_enabled->localltk, 16);
    memcpy(ble_dev_ltk_list[ix].localrand, enc_enabled->localrand, 8);
    ble_dev_ltk_list[ix].local_ediv = enc_enabled->localediv;
    index_found                     = 0;
  }
  if (ix >= TOTAL_CONNECTIONS) {
    return -1;
  }
  return status;
}
/*==============================================*/
/**
 * @fn         add_derived_key_to_ltk_list
 * @brief      invoked when security keys event is received
 * @param[in]  ble_dev_ltk_list device list pointer
 * @param[in]  ble_ctkd structure pointer
 * @return     none.
 * @section description
 * This callback function adds the derived linkkey to the list based on address
 */
int32_t add_derived_key_to_ltk_list(rsi_ble_dev_ltk_list_t *ble_dev_ltk_list, rsi_ble_event_ctkd_t *ble_ctkd)
{
  int8_t status = 0;
  int ix;
  for (ix = 0; ix < TOTAL_CONNECTIONS; ix++) {
    if (!(memcmp(ble_dev_ltk_list[ix].remote_dev_addr, ble_ctkd->dev_addr, 6))) {
      memcpy(ble_dev_ltk_list[ix].derived_linkkey, ble_ctkd->key, 16);
      LOG_PRINT("\n bd addr mapped  with inx = %x \n", ix);
      break;
    }
  }
  if (ix == TOTAL_CONNECTIONS) {

    status = -1;
  }
  return status;
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_profile_discovery(uint16_t status, void *event_data)
 * @brief       handler for event_advertisement_restart to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_advertisement_restart to be executed in ble task context
 *
 */
void rsi_ble_event_profile_dummy(uint16_t status, void *event_data)
{
  uint8_t ble_conn_id;
  uint8_t temp_switch_count                      = 0;
  profile_dummy_data_t *profile_dummy_data_event = (profile_dummy_data_t *)event_data;
  //! Add handling here
  ble_conn_id       = profile_dummy_data_event->ble_con_id;
  temp_switch_count = profile_dummy_data_event->swtch_cnt;
  LOG_PRINT("\r\nIn rsi_ble_event_profile_dummy \r\n");

  if (temp_switch_count == 1) {

    if (!rsi_ble_conn_info[ble_conn_id].profile_mem_init) {
      if (rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_info_uuid == NULL) {
        rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_info_uuid =
          (rsi_ble_event_profile_by_uuid_t *)malloc(sizeof(rsi_ble_event_profile_by_uuid_t)
                                                    * rsi_ble_conn_info[ble_conn_id].total_remote_profiles);
      }
      if (rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_info_uuid == NULL) {
        LOG_PRINT("failed to allocate memory for "
                  "rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn[%d].profile_info_uuid \r\n",
                  ble_conn_id);
        return;
      }
      memset(rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_info_uuid,
             0,
             sizeof(rsi_ble_event_profile_by_uuid_t) * rsi_ble_conn_info[ble_conn_id].total_remote_profiles);
      rsi_ble_conn_info[ble_conn_id].profile_mem_init = true;
    }
    // copy total searched profiles in local buffer
    if (rsi_ble_conn_info[ble_conn_id].prof_evt_cnt < rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {

      //! copy to central buffer
      memcpy(&rsi_ble_conn_info[ble_conn_id]
                .rsi_ble_profile_list_by_conn.profile_info_uuid[rsi_ble_conn_info[ble_conn_id].prof_evt_cnt],
             &rsi_ble_conn_info[ble_conn_id].get_profile,
             sizeof(rsi_ble_event_profile_by_uuid_t));
      rsi_ble_conn_info[ble_conn_id].prof_resp_recvd = false;
#if RSI_DEBUG_EN
      LOG_PRINT_D("\r\n Gatt profile:\nStart handle: 0x%x  \nEnd handle:0x%x -conn%d \r\n",
                  *(uint16_t *)rsi_ble_conn_info[ble_conn_id]
                     .rsi_ble_profile_list_by_conn.profile_info_uuid[rsi_ble_conn_info[ble_conn_id].prof_evt_cnt]
                     .start_handle,
                  *(uint16_t *)rsi_ble_conn_info[ble_conn_id]
                     .rsi_ble_profile_list_by_conn.profile_info_uuid[rsi_ble_conn_info[ble_conn_id].prof_evt_cnt]
                     .end_handle,
                  ble_conn_id);

#endif
      rsi_ble_conn_info[ble_conn_id].prof_evt_cnt++;
      LOG_PRINT_D("\n rsi_ble_conn_info[ble_conn_id].prof_evt_cnt value = %d ",
                  rsi_ble_conn_info[ble_conn_id].prof_evt_cnt);
      if (rsi_ble_conn_info[ble_conn_id].prof_cnt < rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {
        status = rsi_ble_get_profile_async(
          rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
          rsi_ble_conn_info[ble_conn_id].search_serv_conn[rsi_ble_conn_info[ble_conn_id].prof_cnt],
          NULL);
        rsi_ble_conn_info[ble_conn_id].prof_cnt++;
      }

      else if (rsi_ble_conn_info[ble_conn_id].prof_cnt == rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {
#if RSI_DEBUG_EN
        LOG_PRINT_D("\r\n completed gatt profile -conn%d\n", ble_conn_id);
#endif
        if (!rsi_ble_conn_info[ble_conn_id].char_resp_recvd) {
          if (rsi_ble_conn_info[ble_conn_id].profile_index_for_char_query
              < rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {
            rsi_ble_conn_info[ble_conn_id].char_resp_recvd = true;
#if RSI_DEBUG_EN
            LOG_PRINT_D(
              "\r\n search for profile characteristics :0x%x -conn%d \r\n",
              rsi_ble_conn_info[ble_conn_id]
                .rsi_ble_profile_list_by_conn.profile_desc[rsi_ble_conn_info[ble_conn_id].profile_index_for_char_query]
                .profile_uuid.val.val16,
              ble_conn_id);
#endif
            uint16_t start_handle = *(uint16_t *)rsi_ble_conn_info[ble_conn_id]
                                       .rsi_ble_profile_list_by_conn
                                       .profile_info_uuid[rsi_ble_conn_info[ble_conn_id].profile_index_for_char_query]
                                       .start_handle;
            uint16_t end_handle = *(uint16_t *)rsi_ble_conn_info[ble_conn_id]
                                     .rsi_ble_profile_list_by_conn
                                     .profile_info_uuid[rsi_ble_conn_info[ble_conn_id].profile_index_for_char_query]
                                     .end_handle;

            //! Get characteristic services of searched profile
            status = rsi_ble_get_char_services_async(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
                                                     start_handle,
                                                     end_handle,
                                                     NULL);
            rsi_ble_conn_info[ble_conn_id].char_cnt++;
            if (status != RSI_SUCCESS) {
              //! check for procedure already in progress error
              if (status == (uint16_t)RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
                rsi_ble_conn_info[ble_conn_id].char_resp_recvd = false;

#if RSI_DEBUG_EN
                LOG_PRINT_D("\r\n rsi_ble_get_char_services_async procedure is already in progress -conn%d \r\n",
                            ble_conn_id);
#endif
              }
              //! check for buffer full error, which is not expected for this procedure
              else if (status == (uint16_t)RSI_ERROR_BLE_DEV_BUF_FULL) {
                LOG_PRINT("\r\n rsi_ble_get_char_services_async failed with buffer full error -conn%d \r\n",
                          ble_conn_id);
              } else {
                LOG_PRINT(
                  "\r\n failed to get service characteristics of the remote GATT server with error:0x%x -conn%d \r\n",
                  status,
                  ble_conn_id);
              }
            }
            rsi_ble_conn_info[ble_conn_id].profile_index_for_char_query++;
          } else {
            //! discovery of complete characteristics in each profile is completed
            if (!rsi_ble_conn_info[ble_conn_id].service_char_mem_init) {
              if (rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_char_info == NULL) {
                rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_char_info =
                  (rsi_ble_event_read_by_type1_t *)malloc(sizeof(rsi_ble_event_read_by_type1_t)
                                                          * rsi_ble_conn_info[ble_conn_id].total_remote_profiles);
              }
              if (rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_char_info == NULL) {
                LOG_PRINT("failed to allocate memory for "
                          "rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn[%d].profile_char_info \r\n",
                          ble_conn_id);
                return;
              }
              memset(rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_char_info,
                     0,
                     sizeof(rsi_ble_event_read_by_type1_t) * rsi_ble_conn_info[ble_conn_id].total_remote_profiles);
              rsi_ble_conn_info[ble_conn_id].service_char_mem_init = true;
            }
            if (rsi_ble_conn_info[ble_conn_id].char_for_serv_cnt
                < rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {

              memcpy(
                &rsi_ble_conn_info[ble_conn_id]
                   .rsi_ble_profile_list_by_conn.profile_char_info[rsi_ble_conn_info[ble_conn_id].char_for_serv_cnt],
                &rsi_ble_conn_info[ble_conn_id].get_char_services,
                sizeof(rsi_ble_event_read_by_type1_t));
              LOG_PRINT("\n attribute handle from characteristic discovery = %d \n ",
                        rsi_ble_conn_info[ble_conn_id]
                          .get_char_services.char_services[rsi_ble_conn_info[ble_conn_id].char_for_serv_cnt]
                          .handle);

              rsi_ble_conn_info[ble_conn_id].char_for_serv_cnt++;
              rsi_ble_conn_info[ble_conn_id].char_resp_recvd = false;
              if (rsi_ble_conn_info[ble_conn_id].char_cnt < rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {
                uint16_t start_handle =
                  *(uint16_t *)rsi_ble_conn_info[ble_conn_id]
                     .rsi_ble_profile_list_by_conn.profile_info_uuid[rsi_ble_conn_info[ble_conn_id].char_cnt]
                     .start_handle;
                uint16_t end_handle =
                  *(uint16_t *)rsi_ble_conn_info[ble_conn_id]
                     .rsi_ble_profile_list_by_conn.profile_info_uuid[rsi_ble_conn_info[ble_conn_id].char_cnt]
                     .end_handle;
                status = rsi_ble_get_char_services_async(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
                                                         start_handle,
                                                         end_handle,
                                                         NULL);
                rsi_ble_conn_info[ble_conn_id].char_cnt++;
              } else if (rsi_ble_conn_info[ble_conn_id].char_cnt
                         == rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {

                rsi_ble_event_profile_discovery_driver_callback(&ble_conn_id);
              }
            }
          }
        }
      } else {
        LOG_PRINT("\n ERROR CONDITION \n");
      }
    }

  } else if (temp_switch_count == 2) {
    LOG_PRINT("\n get_char_services \n");
    //set RSI_BLE_GATT_PROFILE
    // rsi_ble_char_services_event(temp_status, &rsi_ble_conn_info[ble_conn_id].get_char_services);

  } else if (temp_switch_count == 3) {

    //set RSI_BLE_GATT_PROFILE
    // rsi_ble_event_profiles_list(temp_status, &rsi_ble_conn_info[ble_conn_id].get_allprofiles);
  }
}
void rsi_scan_restart_event()
{
  rsi_ble_ae_set_scan_enable_t ae_set_scan_enable = { 0 };

  LOG_PRINT_D("\r\nIn Scan Re-Start evt = %d \r\n", scan_state_dut);

  if (scan_state_dut != scan_off) {

    ae_set_scan_enable.enable            = RSI_BLE_STOP_SCAN;
    ae_set_scan_enable.filter_duplicates = BLE_AE_SCAN_ENABLE_FILTER_DUP;
    ae_set_scan_enable.duration          = BLE_AE_SCAN_DUR;
    ae_set_scan_enable.period            = BLE_AE_SCAN_PERIOD;
    status                               = rsi_ble_ae_set_scan_enable(&ae_set_scan_enable);
    if (status != RSI_SUCCESS) {
      LOG_PRINT(" \n set ae scan disable failed with 0x%lX \n", status);
    } else {
      scan_state_dut = scan_off;
      LOG_PRINT(" \n set ae scan disable success \n");
    }
#if WLAN_TRANSIENT_CASE
    ble_scanning_is_there = 0;
#endif
  }
  LOG_PRINT("\r\n Restarting scanning \n");
  if (scan_state_dut == scan_off) {
    if (peripheral_count <= RSI_BLE_MAX_NBR_PERIPHERALS) {
      status = ble_ext_scan_enable();
      if (status != RSI_SUCCESS) {
        LOG_PRINT("\r\n scanning start failed, cmd status = %lx -conn\n", status);
        rsi_ble_event_scan_restart_driver_callback();
      } else {
        scan_state_dut = connectable_scan;
        LOG_PRINT("\n scan restart enable success\n");
      }
    }

#if WLAN_TRANSIENT_CASE
    ble_scanning_is_there = 1;
#endif
  }
}
/*==============================================*/
/**
 * @fn          void rsi_ble_event_scan_restart(uint16_t status, void *event_data)
 * @brief       handler for event_advertisement_restart to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_advertisement_restart to be executed in ble task context
 *
 */
void rsi_ble_event_scan_restart(uint16_t __attribute__((unused)) status, void __attribute__((unused)) * event_data)
{
  //! Add handling here
  if (SCAN_ENABLED_DEFAULT) {
    rsi_scan_restart_event();
  }
}

void rsi_adv_restart_event()
{
  if ((adv_state_dut == adv_disabled) && (central_count <= RSI_BLE_MAX_NBR_CENTRALS)) {
    // By default advertising role will be disabled in the firmware once remote device got connected.
    LOG_PRINT("\r\n Number of central devices connected:%d -\n", central_count);

    //! advertise device with default interval
    if (adv_state_dut == adv_enabled) {
      rsi_ble_ae_adv_enable_t ble_ae_adv = { 0 };

      ble_ae_adv.enable        = RSI_BLE_STOP_ADV;
      ble_ae_adv.no_of_sets    = 1;
      ble_ae_adv.adv_handle    = BLE_AE_ADV_HNDL_SET_1;
      ble_ae_adv.duration      = BLE_AE_ADV_DUR_SET_1;
      ble_ae_adv.max_ae_events = BLE_AE_ADV_MAX_AE_EVENTS_SET_1;

      //SAPI function call for enabling extended advertising for set 1
      status = rsi_ble_start_ae_advertising(&ble_ae_adv);
      if (status != RSI_SUCCESS) {
        LOG_PRINT("\r\n advertising failed to stop, with status = 0x%lx -conn\n", status);

      } else {
        adv_state_dut = adv_disabled;
      }
#if WLAN_TRANSIENT_CASE
      ble_adv_is_there = 0;
#endif
    }

    if ((adv_state_dut == adv_disabled)) {
      //! advertise device with default interval
      status = ble_ae_set_1_advertising_enable();
      LOG_PRINT("\r\n Advertising Restarted \n");
      if (status != RSI_SUCCESS) {
        LOG_PRINT("\r\n advertising failed with status = 0x%lx -conn \n", status);
      } else {
#if WLAN_TRANSIENT_CASE
        ble_adv_is_there = 1;
#endif
        adv_state_dut = adv_enabled;
      }
    }

#if WLAN_TRANSIENT_CASE
    ble_adv_is_there = 1;
#endif
    LOG_PRINT_D("\r\n advertising device -conn\n");
  }
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_advertisement_restart(uint16_t status, void *event_data)
 * @brief       handler for event_advertisement_restart to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_advertisement_restart to be executed in ble task context
 *
 */
void rsi_ble_event_advertisement_restart(uint16_t __attribute__((unused)) status,
                                         void __attribute__((unused)) * event_data)
{
  if (ADV_ENABLED_DEFAULT) {
    rsi_adv_restart_event();
  }
}

/**
 * @fn          void rsi_ble_prepare_filtered_adv_report(rsi_ble_event_adv_report_t *rsi_ble_event_adv_report)
 * @brief       function enqueues filtered advertising reports event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_event_adv_report_t *rsi_ble_event_adv_report, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues the filtered advertising reports event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */
void rsi_ble_prepare_filtered_adv_report(rsi_ble_event_adv_report_t *rsi_ble_event_adv_report)
{
  LOG_PRINT("in rsi_ble_prepare_filtered_adv_report");

  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_adv_report_t));
  //! assert if malloc failed
  if (msg == NULL) {
    LOG_PRINT("Out of Memory assert//assert 56\n");
    //assert("Out Of Memory\n");
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
/**
* @fn          void rsi_ble_event_adv_report(uint16_t status, void *event_data)
* @brief       handler for event_adv_report to be executed in ble task context
* @param[in]   uint16_t , event_status
* @param[in]   void, event_data
* @return      None
*
* @section description
* handler for event_adv_report to be executed in ble task context
*
*/
void rsi_ble_event_adv_report(uint16_t status, void *event_data)
{
  LOG_PRINT("\n received the filtered adv report\n");
  uint8_t ble_conn_id;

  rsi_ble_event_adv_report_t *adv_report = (rsi_ble_event_adv_report_t *)event_data;

  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, adv_report->dev_addr);
  memset(remote_name, 0, sizeof(remote_name));
  BT_LE_ADPacketExtract(remote_name, adv_report->adv_data, adv_report->adv_data_len);

  //! get conn_id
#if (CONNECT_OPTION == CONN_BY_NAME)
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, remote_name, strlen((const char *)remote_name));
#else
  ble_conn_id              = rsi_get_ble_conn_id(remote_dev_addr_conn);
#endif

  memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_app_adv_reports_to_app, adv_report, sizeof(rsi_ble_event_adv_report_t));

  rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[ble_conn_id].conn_id = ble_conn_id;
  ble_confgs.ble_conn_configuration[ble_conn_id] = rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[ble_conn_id];
  LOG_PRINT_D("\nBLE conn ID: %d\n", ble_conn_id);

  LOG_PRINT_D("\r\n Connect command - conn%d \r\n", ble_conn_id);

  status = rsi_ble_stop_scanning();
  if (status != RSI_SUCCESS) {
    LOG_PRINT("Scan stopping failed with status : %d - conn%d \r\n", status, ble_conn_id);
  } else {
    LOG_PRINT("\n scan stopped \n ");
    scan_state_dut = scan_off;
  }

  status = rsi_ble_connect_with_params(rsi_ble_conn_info[ble_conn_id].rsi_app_adv_reports_to_app.dev_addr_type,
                                       (int8_t *)rsi_ble_conn_info[ble_conn_id].rsi_app_adv_reports_to_app.dev_addr,
                                       LE_SCAN_INTERVAL_CONN,
                                       LE_SCAN_WINDOW_CONN,
                                       C2P12_CONNECTION_INTERVAL_MAX,
                                       C2P12_CONNECTION_INTERVAL_MIN,
                                       C2P12_CONNECTION_LATENCY,
                                       C2P12_SUPERVISION_TIMEOUT);
  LOG_PRINT("\r\n connecting to device :  %s -conn%d \n",
            (int8_t *)remote_dev_addr_conn /*rsi_ble_conn_info[ble_conn_id].remote_dev_addr*/,
            ble_conn_id);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Connecting failed with status : 0x%x -conn%d\n", status, ble_conn_id);

    rsi_ble_event_scan_restart_driver_callback();
  } else {

    adv_pkt_processing_pending = 0;
    peripheral_con_req_pending = 1;

    osSemaphoreAcquire(ble_wait_on_connect, 10000);

    if (peripheral_con_req_pending) {
      LOG_PRINT("\r\n Initiating connect cancel command in -conn%d \n", ble_conn_id);
      status = rsi_ble_connect_cancel((int8_t *)rsi_ble_conn_info[ble_conn_id].rsi_app_adv_reports_to_app.dev_addr);
      if (status != RSI_SUCCESS) {
        LOG_PRINT("\r\n ble connect cancel cmd status = %x \n", status);
      } else {

        peripheral_count++;
        rsi_ble_event_disconnect(status, (int8_t *)rsi_ble_conn_info[ble_conn_id].rsi_app_adv_reports_to_app.dev_addr);
      }
      peripheral_con_req_pending = 0;
    }
  }
}
void rsi_ble_event_enhance_conn_status_driver_callback(
  rsi_ble_event_enhance_conn_status_t *rsi_ble_event_enhance_conn_status)
{
  uint8_t remote_device_role = 0;
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_event_enhance_conn_status->dev_addr);
  LOG_PRINT("\n start connect adddr RSI CONNECTEED ADDRESS : %s \n", remote_dev_addr_conn);
  remote_device_role = rsi_get_remote_device_role(remote_dev_addr_conn);
  if (remote_device_role == PERIPHERAL_ROLE) {
    peripheral_con_req_pending = 0;
    osSemaphoreRelease(ble_wait_on_connect);
  }
  LOG_PRINT_D(" \n in rsi_ble_event_enhance_conn_status_driver_callback \n");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_event_enhance_conn_status_t));
  //! assert if malloc failed
  if (msg == NULL) {
    LOG_PRINT("Out of Memory assert//assert 57\n");
    //assert("Out Of Memory\n");
  } else {
    LOG_PRINT_D("Malloc passed\n");
  }
  //! init messag details
  msg->next     = NULL;
  msg->event_id = event_enhance_conn_status_event_id;
  LOG_PRINT_D("%d", msg->event_id);
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
/**
 * @fn          void rsi_ble_event_adv_report_driver_callback(rsi_ble_event_adv_report_t *rsi_ble_event_adv_report)
 * @brief       function enqueues received event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_event_adv_report_t *rsi_ble_event_adv_report, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function filters the adv_reports received and then enqueues event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */

void rsi_ble_event_adv_report_driver_callback(rsi_ble_event_adv_report_t *rsi_ble_event_adv_report)
{
  uint8_t device_already_connected = 0;
  LOG_PRINT_D("\n On Event Adv Report \n");
  rsi_ble_event_adv_report_t *adv_report = (rsi_ble_event_adv_report_t *)rsi_ble_event_adv_report;

  //! Check weather the received address type is connectable or not
  if (adv_report->report_type == 0x02) {
    return; // return for NON CONN ADV PACKETS
  }

  //! Check any on-going connection process is pending
  if (adv_pkt_processing_pending) {
    return;
  }

  //! Check no of connected devices
  if (num_of_connected_peripheral_devices >= RSI_BLE_MAX_NBR_PERIPHERALS) {
    LOG_PRINT("\n reached the max num_of_connected_peripheral_devices \n");
    return;
  }

  rsi_6byte_dev_address_to_ascii(remote_dev_addr, (uint8_t *)adv_report->dev_addr);

#if (CONNECT_OPTION == CONN_BY_NAME)
  memset(remote_name, 0, sizeof(remote_name));
  BT_LE_ADPacketExtract(remote_name, adv_report->adv_data, adv_report->adv_data_len);
  device_already_connected = rsi_check_dev_list_driver(remote_name, remote_dev_addr);
  LOG_PRINT_D("\n device_already_connected value: %d \n", device_already_connected);
#else
  device_already_connected = rsi_check_dev_list_driver(NULL, remote_dev_addr);
  LOG_PRINT_D("\n device_already_connected value: %d \n", device_already_connected);
#endif

  if (device_already_connected == PERIPHERAL_NOT_CONNECTED) {
    LOG_PRINT("\r\n advertised details remote_name = %s, dev_addr = %s \r\n", remote_name, (int8_t *)remote_dev_addr);

    LOG_PRINT("\n rsi_ble_prepare_filtered_adv_report\n");

    //! prepare the packet and send it to app task for the connection procedure
    rsi_ble_prepare_filtered_adv_report(adv_report);

    adv_pkt_processing_pending = 1;
  }
  return;
}

void rsi_ble_on_data_recieve(uint8_t conn_id)
{
  uint8_t ble_conn_id = conn_id;
  if (rsi_ble_conn_info[ble_conn_id].notify_handle_found
      && ble_confgs.ble_conn_configuration[ble_conn_id].rx_notifications) {
    uint8_t notify_data[2] = { 1, 0 };
    LOG_PRINT("\r\n in receive notification event -conn%d \n", ble_conn_id);

    status = rsi_ble_set_att_value(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr, // enable the notifications
                                   rsi_ble_conn_info[ble_conn_id].notify_handle + 1,
                                   2,
                                   notify_data);
    if (status != RSI_SUCCESS) {
      if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
        LOG_PRINT("\r\n notify failed with buffer error -conn%d \r\n", ble_conn_id);
      } else {
        LOG_PRINT("\r\n notify value failed with status = %lx -conn%d \r\n", status, ble_conn_id);
      }
    }
  }
  //! Transmit Gatt indications to remote device
  else if (rsi_ble_conn_info[ble_conn_id].indication_handle_found
           && ble_confgs.ble_conn_configuration[ble_conn_id].rx_indications) {
    uint8_t indicate_data[2] = { 2, 0 };
    LOG_PRINT("\r\n in receive indication event -conn%d \n", ble_conn_id);

    status =
      rsi_ble_set_att_value_async(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr, // enable the indications
                                  rsi_ble_conn_info[ble_conn_id].indication_handle + 1,
                                  2,
                                  indicate_data);
    if (status != RSI_SUCCESS) {
      if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
        LOG_PRINT("\r\n indication failed with buffer error -conn%d \r\n", ble_conn_id);
      } else {
        LOG_PRINT("\r\n indication failed with status = %lx -conn%d \r\n", status, ble_conn_id);
      }
    }
  }

  return;
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_on_data_recieve(uint16_t status, void *event_data)
 * @brief       handler for event_advertisement_restart to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_advertisement_restart to be executed in ble task context
 *
 */
void rsi_ble_event_on_data_recieve(uint16_t __attribute__((unused)) status, void *event_data)
{
  //! Add handling here
  uint8_t *conn_id    = (uint8_t *)event_data;
  uint8_t ble_conn_id = *conn_id;

  rsi_ble_on_data_recieve(ble_conn_id);
}

void rsi_ble_on_data_transmit(uint8_t ble_conn_id)
{

  LOG_PRINT_D("\r\n in write with response event -conn%d \n", ble_conn_id);
  if (rsi_ble_conn_info[ble_conn_id].write_handle_found && ble_confgs.ble_conn_configuration[ble_conn_id].tx_write) {

#if RSI_DEBUG_EN
    LOG_PRINT_D("\r\n in write with response event -conn%d \n", ble_conn_id);
#endif
    rsi_ble_conn_info[ble_conn_id].read_data1[0] = rsi_ble_conn_info[ble_conn_id].write_cnt;
    rsi_ble_conn_info[ble_conn_id].max_data_length =
      ble_confgs.ble_conn_configuration[ble_conn_id].buff_mode_sel.max_data_length;
    status = rsi_ble_set_att_value_async(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
                                         rsi_ble_conn_info[ble_conn_id].write_handle,
                                         rsi_ble_conn_info[ble_conn_id].max_data_length,
                                         rsi_ble_conn_info[ble_conn_id].read_data1);
    if (status != RSI_SUCCESS) {
      //! check for procedure already in progress error
      if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {

        more_data_state_beta[ble_conn_id].data_transmit = 1;
#if RSI_DEBUG_EN
        LOG_PRINT_D("\r\n rsi_ble_set_att_value_async procedure is already in progress -conn%d \r\n", ble_conn_id);
#endif
      } else if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
        LOG_PRINT("\r\n write with response failed with buffer error -conn%d \r\n", ble_conn_id);

        more_data_state_beta[ble_conn_id].data_transmit = 1;
      } else {
        LOG_PRINT("\r\n write with response failed with status = %lx -conn%d \r\n", status, ble_conn_id);
      }
    } else {
      rsi_ble_conn_info[ble_conn_id].write_cnt++;
    }
  }

  //! Gatt write no response to remote device continuously
  if (rsi_ble_conn_info[ble_conn_id].write_wwr_handle_found
      && ble_confgs.ble_conn_configuration[ble_conn_id].tx_write_no_response) {
    rsi_ble_conn_info[ble_conn_id].read_data1[0] = rsi_ble_conn_info[ble_conn_id].wwr_count;
    rsi_ble_conn_info[ble_conn_id].max_data_length =
      ble_confgs.ble_conn_configuration[ble_conn_id].buff_mode_sel.max_data_length;

#if RSI_DEBUG_EN
    LOG_PRINT_D("\r\n in write without response event -conn%d \n", ble_conn_id);
#endif
    status = rsi_ble_set_att_cmd(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
                                 rsi_ble_conn_info[ble_conn_id].write_wwr_handle,
                                 rsi_ble_conn_info[ble_conn_id].max_data_length,
                                 (uint8_t *)rsi_ble_conn_info[ble_conn_id].read_data1);
    if (status != RSI_SUCCESS) {
      if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
#if RSI_DEBUG_EN
        LOG_PRINT_D("\r\n write without response failed with buffer error -conn%d \r\n", ble_conn_id);
#endif

        more_data_state_beta[ble_conn_id].data_transmit = 1;
      } else if (status == RSI_ERROR_IN_BUFFER_ALLOCATION) //! TO-DO, add proper error code
      {
        LOG_PRINT("\r\n cannot transmit %d bytes in small buffer configuration mode -conn%d\n",
                  rsi_ble_conn_info[ble_conn_id].max_data_length,
                  ble_conn_id);

        status = rsi_ble_disconnect((int8_t *)rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\ndisconnect command failed with reason %lx\n", status);
        }
      } else {
        LOG_PRINT("\r\n write without response failed with status = 0x%lx -conn%d \r\n", status, ble_conn_id);
      }
    } else {
      rsi_ble_conn_info[ble_conn_id].wwr_count++;
      rsi_ble_event_data_transmit_driver_callback(ble_conn_id);
    }
  }

  //! indicate to remote device continuously
  if (ble_confgs.ble_conn_configuration[ble_conn_id].tx_indications) {

    if (rsi_ble_conn_info[ble_conn_id].indications_enabled == true) {
      //! prepare the data to set as local attribute value.
      rsi_ble_conn_info[ble_conn_id].read_data1[0] = rsi_ble_conn_info[ble_conn_id].indication_cnt;
      rsi_ble_conn_info[ble_conn_id].read_data1[1] = rsi_ble_conn_info[ble_conn_id].indication_cnt >> 8;
      rsi_ble_conn_info[ble_conn_id].max_data_length =
        ble_confgs.ble_conn_configuration[ble_conn_id].buff_mode_sel.max_data_length;
      status = rsi_ble_indicate_value(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
                                      rsi_ble_att3_val_hndl,
                                      rsi_ble_conn_info[ble_conn_id].max_data_length,
                                      (uint8_t *)rsi_ble_conn_info[ble_conn_id].read_data1);
      if (status != RSI_SUCCESS) {
        if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
#if RSI_DEBUG_EN
          LOG_PRINT_D("\r\n indicate %d failed with buffer full error -conn%d \r\n",
                      rsi_ble_conn_info[ble_conn_id].indication_cnt,
                      ble_conn_id);
          more_data_state_beta[ble_conn_id].data_transmit = 1;
#endif
        } else if (status == RSI_ERROR_IN_BUFFER_ALLOCATION) //! TO-DO, add proper error code
        {
          LOG_PRINT("\r\n cannot transmit %d bytes in small buffer configuration mode -conn%d\n",
                    rsi_ble_conn_info[ble_conn_id].max_data_length,
                    ble_conn_id);

          status = rsi_ble_disconnect((int8_t *)rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\ndisconnect command failed with reason %lx\n", status);
          }
        } else {
          LOG_PRINT("\r\n indication %d failed with error code %lx -conn%d\n",
                    rsi_ble_conn_info[ble_conn_id].indication_cnt,
                    status,
                    ble_conn_id);
        }
      } else {
        rsi_ble_conn_info[ble_conn_id].indication_cnt++;
      }
    }
  }

  //! Notify to remote device continuously
  if (ble_confgs.ble_conn_configuration[ble_conn_id].tx_notifications) {

    if (rsi_ble_conn_info[ble_conn_id].notifications_enabled == true) {
      //! prepare the data to set as local attribute value.
      rsi_ble_conn_info[ble_conn_id].read_data1[0] = rsi_ble_conn_info[ble_conn_id].notfy_cnt;
      rsi_ble_conn_info[ble_conn_id].read_data1[1] = rsi_ble_conn_info[ble_conn_id].notfy_cnt >> 8;

#if RSI_DEBUG_EN
      LOG_PRINT("\r\n sending notify :%d\n", rsi_ble_conn_info[ble_conn_id].notfy_cnt);
#endif
      //! set the local attribute value.
      rsi_ble_conn_info[ble_conn_id].max_data_length =
        ble_confgs.ble_conn_configuration[ble_conn_id].buff_mode_sel.max_data_length;
      status = rsi_ble_notify_value(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
                                    rsi_ble_att1_val_hndl,
                                    rsi_ble_conn_info[ble_conn_id].max_data_length,
                                    (uint8_t *)rsi_ble_conn_info[ble_conn_id].read_data1);
      if (status != RSI_SUCCESS) {
        if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
#if RSI_DEBUG_EN
          LOG_PRINT_D("\r\n notify %d failed with buffer full error -conn%d \r\n",
                      rsi_ble_conn_info[ble_conn_id].notfy_cnt,
                      ble_conn_id);
#endif

          more_data_state_beta[ble_conn_id].data_transmit = 1;

          LOG_PRINT_D("\r\n more_data_state_beta  for conn %d\n", ble_conn_id);

          return;
        } else if (status == RSI_ERROR_IN_BUFFER_ALLOCATION) //! TO-DO, add proper error code
        {
          LOG_PRINT("\r\n cannot transmit %d bytes in small buffer configuration mode -conn%d\n",
                    rsi_ble_conn_info[ble_conn_id].max_data_length,
                    ble_conn_id);

          status = rsi_ble_disconnect((int8_t *)rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\ndisconnect command failed with reason %lx\n", status);
          }
        } else {
          LOG_PRINT("\r\n notify %d failed with error code %lx  -conn%d\n",
                    rsi_ble_conn_info[ble_conn_id].notfy_cnt,
                    status,
                    ble_conn_id);
        }
      } else {
#if RSI_DEBUG_EN
        LOG_PRINT_D("\r\n notify %d success -conn%d \r\n", rsi_ble_conn_info[ble_conn_id].notfy_cnt, ble_conn_id);
#endif
        rsi_ble_conn_info[ble_conn_id].notfy_cnt++;
        if (rsi_ble_conn_info[ble_conn_id].transmit) {
          LOG_PRINT_D("\r\n enquing transmit callback again %d \r\n", ble_conn_id);
          rsi_ble_event_data_transmit_driver_callback(ble_conn_id);
        }
      }
    }
  }
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_data_transmit(uint16_t status, void *event_data)
 * @brief       handler for event_data_transmit to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_data_transmit to be executed in ble task context
 *
 */
void rsi_ble_event_data_transmit(uint16_t __attribute__((unused)) status, void *event_data)

{

  uint8_t conn_id = *(uint8_t *)event_data;
  LOG_PRINT_D("\r\n in transmit  -conn%d \r\n", conn_id);
  rsi_ble_on_data_transmit(conn_id);
}

void rsi_conn_update_req_event(uint8_t conn_id)
{
  uint8_t ble_conn_id = conn_id;
  if (!rsi_ble_conn_info[ble_conn_id].conn_param_req_given) {
#if RSI_DEBUG_EN
    LOG_PRINT_D("\r\n in connection update req event -conn%d \r\n", ble_conn_id);
#endif
    //! update connection interval to 45ms, latency 0, supervision timeout 4s
    status =
      rsi_ble_conn_params_update(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
                                 ble_confgs.ble_conn_configuration[ble_conn_id].conn_param_update.conn_int,
                                 ble_confgs.ble_conn_configuration[ble_conn_id].conn_param_update.conn_int,
                                 ble_confgs.ble_conn_configuration[ble_conn_id].conn_param_update.conn_latncy,
                                 ble_confgs.ble_conn_configuration[ble_conn_id].conn_param_update.supervision_to);
    if (status != RSI_SUCCESS) {
      //! check for procedure already in progress error
      if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
        LOG_PRINT("\r\n rsi_ble_conn_params_update procedure is already in progress -conn%d \r\n", ble_conn_id);
      } else {
        LOG_PRINT("\r\n failed to update connection paramaters error:0x%lx -conn%d \r\n", status, ble_conn_id);
      }
    } else {
      LOG_PRINT("\r\n connection params request was successfull -conn%d \n", ble_conn_id);
      rsi_ble_conn_info[ble_conn_id].conn_param_req_given = true;
    }
  }

  //! set RSI_DATA_RECEIVE_EVENT to receive data from remote device
  if (rsi_ble_conn_info[ble_conn_id].rsi_rx_from_rem_dev) {
    rsi_ble_conn_info[ble_conn_id].rsi_rx_from_rem_dev = false;
    LOG_PRINT("\r\n start receiving data from - conn%d\r\n", ble_conn_id);

    rsi_ble_event_on_data_recieve_driver_callback(&ble_conn_id);
  }
  //! set RSI_DATA_TRANSMIT_EVENT to transmit data to remote device
  if (rsi_ble_conn_info[ble_conn_id].rsi_tx_to_rem_dev) {
    rsi_ble_conn_info[ble_conn_id].rsi_tx_to_rem_dev = false;
    LOG_PRINT("\r\n start transmitting data to - conn%d\r\n", ble_conn_id);

    rsi_ble_event_data_transmit_driver_callback(ble_conn_id);
  }

  return;
}
/*==============================================*/
/**
 * @fn          void rsi_ble_event_conn_update_req(uint16_t status, void *event_data)
 * @brief       handler for event_advertisement_restart to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_advertisement_restart to be executed in ble task context
 *
 */
void rsi_ble_event_conn_update_req(uint16_t __attribute__((unused)) status, void __attribute__((unused)) * event_data)
{
  return;
#if 0
  //! Add handling here
  uint8_t *conn_id    = (uint8_t *)event_data;
  uint8_t ble_conn_id = *conn_id;

  //rsi_conn_update_req_event(ble_conn_id);
#endif
}

void rsi_set_ble_buff_conf(uint8_t conn_id)
{
  uint8_t ble_conn_id = conn_id;

#if RSI_DEBUG_EN
  LOG_PRINT_D("\r\n in gatt RSI_BLE_BUFF_CONF_EVENT -conn%d \r\n", ble_conn_id);
#endif
  if (!rsi_ble_conn_info[ble_conn_id].buff_config_done) {

    LOG_PRINT_D("\r\n setting the buffer configuration -conn%d :  buffer_mode %d: , buffer_count %d: \r\n",
                ble_conn_id,
                ble_confgs.ble_conn_configuration[ble_conn_id].buff_mode_sel.buffer_mode,
                ble_confgs.ble_conn_configuration[ble_conn_id].buff_mode_sel.buffer_cnt);

    status =
      rsi_ble_set_wo_resp_notify_buf_info(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
                                          ble_confgs.ble_conn_configuration[ble_conn_id].buff_mode_sel.buffer_mode,
                                          ble_confgs.ble_conn_configuration[ble_conn_id].buff_mode_sel.buffer_cnt);
    if (status != RSI_SUCCESS) {
#if RSI_DEBUG_EN
      LOG_PRINT("\r\n failed to set the buffer configuration mode, error:0x%x -conn%d \r\n", status, ble_conn_id);
#endif
    } else {
      rsi_ble_conn_info[ble_conn_id].buff_config_done = true;
      LOG_PRINT("\r\n buffer configuration done successfully \n\r");
    }
  }
  rsi_conn_update_req_event(ble_conn_id);
  return;
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_set_buffer_config(uint16_t status, void *event_data)
 * @brief       handler for event_advertisement_restart to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_advertisement_restart to be executed in ble task context
 *
 */
void rsi_ble_event_set_buffer_config(uint16_t __attribute__((unused)) status, void *event_data)
{

  uint8_t *conn_id    = (uint8_t *)event_data;
  uint8_t ble_conn_id = *conn_id;

  rsi_set_ble_buff_conf(ble_conn_id);
}
/*==============================================*/
/**
 * @fn          void rsi_ble_on_gatt_write_event(uint16_t status, void *event_data)
 * @brief       handler for event_write to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_write to be executed in ble task context
 *
 */
void rsi_ble_on_gatt_write_event(uint16_t status, void *event_data)
{
  uint8_t ble_conn_id;
  rsi_ble_event_write_t *rsi_ble_write = (rsi_ble_event_write_t *)event_data;
  LOG_PRINT_D("On GATT WRITE");
#if RSI_DEBUG_EN
  LOG_PRINT_D("\r\n in write event \r\n");
#endif
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_write->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].app_ble_write_event, rsi_ble_write, sizeof(rsi_ble_event_write_t));

  //! process the received 'write response' data packet
  if ((*(uint16_t *)(rsi_ble_conn_info[ble_conn_id].app_ble_write_event.handle)) == rsi_ble_att1_val_hndl) {
    rsi_ble_att_list_t *attribute = NULL;
    uint8_t opcode = 0x12, err = 0x00;
    attribute =
      rsi_gatt_get_attribute_from_list(&att_list,
                                       (*(uint16_t *)(rsi_ble_conn_info[ble_conn_id].app_ble_write_event.handle)));

    //! Check if value has write properties
    if ((attribute != NULL) && (attribute->value != NULL)) {
      if (!(attribute->char_val_prop & 0x08)) //! If no write property, send error response
      {
        err = 0x03; //! Error - Write not permitted
      }
    } else {
      //! Error = No such handle exists
      err = 0x01;
    }

    //! Update the value based6 on the offset and length of the value
    if ((err == 0) && ((rsi_ble_conn_info[ble_conn_id].app_ble_write_event.length) <= attribute->max_value_len)) {
      memset(attribute->value, 0, attribute->max_value_len);

      //! Check if value exists for the handle. If so, maximum length of the value.
      memcpy(attribute->value,
             rsi_ble_conn_info[ble_conn_id].app_ble_write_event.att_value,
             rsi_ble_conn_info[ble_conn_id].app_ble_write_event.length);

      //! Update value length
      attribute->value_len = rsi_ble_conn_info[ble_conn_id].app_ble_write_event.length;

      LOG_PRINT("\r\n received data from remote device: %s \n", (uint8_t *)attribute->value);

      //! Send gatt write response
      rsi_ble_gatt_write_response(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr, 0);
    } else {
      //! Error : 0x07 - Invalid request,  0x0D - Invalid attribute value length
      err = 0x07;
    }

    if (err) {
      //! Send error response
      rsi_ble_att_error_response(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
                                 *(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_write_event.handle,
                                 opcode,
                                 err);
    }
  }

  //! print the received 'write no response' data packet
  if ((*(uint16_t *)(rsi_ble_conn_info[ble_conn_id].app_ble_write_event.handle)) == rsi_ble_att2_val_hndl) {
    LOG_PRINT("\r\n received data from remote device: %s \n",
              rsi_ble_conn_info[ble_conn_id].app_ble_write_event.att_value);
  }

  //! when remote device enabled the notifications
  if (((*(uint16_t *)(rsi_ble_conn_info[ble_conn_id].app_ble_write_event.handle) - 1) == rsi_ble_att1_val_hndl)) {
    if (ble_confgs.ble_conn_configuration[ble_conn_id].tx_notifications) {
      // check for valid notifications
      if (rsi_ble_conn_info[ble_conn_id].app_ble_write_event.att_value[0] == NOTIFY_ENABLE) {
        LOG_PRINT("\r\n Remote device enabled the notification -conn%d\n", ble_conn_id);
        rsi_ble_conn_info[ble_conn_id].rsi_tx_to_rem_dev     = true;
        rsi_ble_conn_info[ble_conn_id].transmit              = true;
        rsi_ble_conn_info[ble_conn_id].notifications_enabled = true;
        //! configure the buffer configuration mode
        rsi_ble_event_set_buffer_config_driver_callback(&ble_conn_id);
      } else if (rsi_ble_conn_info[ble_conn_id].app_ble_write_event.att_value[0] == NOTIFY_DISABLE) {
        LOG_PRINT("\r\n Remote device disabled the notification -conn%d\n", ble_conn_id);
        rsi_ble_conn_info[ble_conn_id].notifications_enabled = false;
        rsi_ble_conn_info[ble_conn_id].transmit              = false;
      }
    }
  }

  else if (((*(uint16_t *)(rsi_ble_conn_info[ble_conn_id].app_ble_write_event.handle) - 1) == rsi_ble_att3_val_hndl)) {
    if (ble_confgs.ble_conn_configuration[ble_conn_id].tx_indications) {
      // check for valid indications
      if (rsi_ble_conn_info[ble_conn_id].app_ble_write_event.att_value[0] == INDICATION_ENABLE) {
        LOG_PRINT("\r\n Remote device enabled the indications -conn%d\n", ble_conn_id);
        rsi_ble_conn_info[ble_conn_id].rsi_tx_to_rem_dev   = true;
        rsi_ble_conn_info[ble_conn_id].indications_enabled = true;
        //! configure the buffer configuration mode
        rsi_ble_event_set_buffer_config_driver_callback(&ble_conn_id);
      } else if (rsi_ble_conn_info[ble_conn_id].app_ble_write_event.att_value[0] == INDICATION_DISABLE) {
        LOG_PRINT("\r\n Remote device disabled the indications -conn%d\n", ble_conn_id);
        rsi_ble_conn_info[ble_conn_id].indications_enabled = false;
      }
    }
  }
  //! code to handle remote device indications
  //! send acknowledgement to the received indication packet
  if (*(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_write_event.handle
      == (rsi_ble_conn_info[ble_conn_id].indication_handle)) {
    if (ble_confgs.ble_conn_configuration[ble_conn_id].rx_indications) {
      LOG_PRINT("\r\n received indication packet from remote device, data= %s \n",
                rsi_ble_conn_info[ble_conn_id].app_ble_write_event.att_value);
#if RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST
      //! Send indication acknowledgement to remote device
      status = rsi_ble_indicate_confirm(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr);
      if (status != RSI_SUCCESS) {
        LOG_PRINT("\r\n indication confirm failed \t reason = %x -conn%d\n", status, ble_conn_id);
      } else {
        LOG_PRINT("\r\n indication confirm response sent -conn%d\n", ble_conn_id);
      }
#endif
    }
  }

  //! code to handle remote device notifications
  else if (*(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_write_event.handle
           == (rsi_ble_conn_info[ble_conn_id].notify_handle)) {
    if ((!rsi_ble_conn_info[ble_conn_id].notification_received)
        && (ble_confgs.ble_conn_configuration[ble_conn_id].rx_notifications)) {
      //! stop printing the logs after receiving first notification
      rsi_ble_conn_info[ble_conn_id].notification_received = true;
      LOG_PRINT("\r\n receiving notifications from remote device -conn%d\r\n", ble_conn_id);
    } else {
      //! do nothing as received notifications not required to print
    }
  }
}
/*==============================================*/
/**
 * @fn          void rsi_ble_on_gatt_prepare_write_event(uint16_t status, void *event_data)
 * @brief       handler for event_prepare_write to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_prepare_write to be executed in ble task context
 *
 */
void rsi_ble_on_gatt_prepare_write_event(uint16_t status, void *event_data)
{

  // This statement is added only to resolve compilation warning  : [-Wunused-parameter] , value is unchanged
  uint8_t ble_conn_id;
  rsi_ble_event_prepare_write_t *rsi_app_ble_prepared_write_event = (rsi_ble_event_prepare_write_t *)event_data;
  UNUSED_PARAMETER(status);
#if RSI_DEBUG_EN
  LOG_PRINT_D("\r\n in rsi_ble_on_gatt_prepare_write_event \r\n");
#endif
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_app_ble_prepared_write_event->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event,
         rsi_app_ble_prepared_write_event,
         sizeof(rsi_ble_event_prepare_write_t));

  LOG_PRINT("\nPWE\n");
  uint8_t err = 0;
  //! clear the served event
  if (*(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.handle == rsi_ble_att1_val_hndl) {
    rsi_ble_att_list_t *attribute = NULL;
    uint8_t opcode                = 0x16;
    attribute =
      rsi_gatt_get_attribute_from_list(&att_list,
                                       *(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.handle);

    //! Check if value has write properties
    if ((attribute != NULL) && (attribute->value != NULL)) {
      if (!(attribute->char_val_prop & 0x08)) //! If no write property, send error response
      {
        err = 0x03; //! Error - Write not permitted
      }
    } else {
      //! Error = No such handle exists
      err = 0x01;
    }

    if (err) {
      //! Send error response
      rsi_ble_att_error_response(rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.dev_addr,
                                 *(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.handle,
                                 opcode,
                                 err);
    }

    //! Update the value based6 on the offset and length of the value
    if ((err == 0)
        && ((*(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.offset)
            <= attribute->max_value_len)) {
      LOG_PRINT_D("PWE - offset : %d\n",
                  (*(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.offset));
      //! Hold the value to update it
      memcpy(&rsi_ble_conn_info[ble_conn_id]
                .temp_prepare_write_value[rsi_ble_conn_info[ble_conn_id].temp_prepare_write_value_len],
             rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.att_value,
             rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.length);
      rsi_ble_conn_info[ble_conn_id].temp_prepare_write_value_len +=
        rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.length;
    } else {
      //! Error : 0x07 - Invalid offset,  0x0D - Invalid attribute value length
      rsi_ble_conn_info[ble_conn_id].prep_write_err = 0x07;
    }
    //! Send gatt write response
    rsi_ble_gatt_prepare_write_response(
      rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.dev_addr,
      *(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.handle,
      (*(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.offset),
      rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.length,
      rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.att_value);
  }
}

void rsi_ble_req_gatt_profile(uint8_t conn_id)
{

  uint8_t ble_conn_id = conn_id;
  if (rsi_ble_conn_info[ble_conn_id].mtu_exchange_done) {
    //! continue profile query only after pairing and that too if smp is selected in configuration
    if (ble_confgs.ble_conn_configuration[ble_conn_id].smp_enable) {
      if (!rsi_ble_conn_info[ble_conn_id].neg_rply) {
        if (!rsi_ble_conn_info[ble_conn_id].smp_done) {
        }
      }
    }
    //! get remote device profiles
    LOG_PRINT("\r\n remote device profile discovery started -conn%d \r\n", ble_conn_id);
    status = rsi_ble_get_profiles_async(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr, 1, 0xffff, NULL);
    if (status != RSI_SUCCESS) {
      //! check for procedure already in progress error
      if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
        LOG_PRINT("\r\n rsi_ble_get_profiles_async procedure is already in progress -conn%d \r\n", ble_conn_id);
      }
      //! check for buffer full error, which is not expected for this procedure
      else if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
        LOG_PRINT("\r\n rsi_ble_get_profiles_async failed with buffer full error -conn%d \r\n", ble_conn_id);
      } else {
        LOG_PRINT("\r\n get profile async call failed with error code :%lx -conn%d \r\n", status, ble_conn_id);
      }
    }
  }
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_conn_status(uint16_t status, void *event_data)
 * @brief       handler for event_conn_status to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_conn_status to be executed in ble task context
 *
 */

void rsi_ble_event_conn_status(uint16_t status, void *event_data)
{
  uint8_t ble_conn_id                    = 0;
  rsi_ble_event_conn_status_t *resp_conn = (rsi_ble_event_conn_status_t *)event_data;
  LOG_PRINT("On Event COnnect");
  uint8_t i;
  uint8_t remote_device_role    = 0;
  uint8_t RSI_NULL_BLE_ADDR[18] = { 0 };

#if RSI_DEBUG_EN
  LOG_PRINT_D("\r\nIn on conn cb\r\n");
#endif
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, resp_conn->dev_addr);

  //! Check whether the received connected event came from remote peripheral or central
  remote_device_role = rsi_get_remote_device_role(remote_dev_addr_conn);

  if (resp_conn->status != 0) {
    LOG_PRINT_D("\r\n On connect event status report : %d", resp_conn->status);
    //! Restarting scan
    rsi_ble_event_scan_restart_driver_callback();
    return;
  }

  if (remote_device_role == PERIPHERAL_ROLE) // check for the connection is from peripheral or central
  {
    //! get conn_id
#if (CONNECT_OPTION != CONN_BY_NAME)
    //! get conn_id
    ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
    ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

    //! copy to conn specific buffer
    memcpy(&rsi_ble_conn_info[ble_conn_id].conn_event_to_app, resp_conn, sizeof(rsi_ble_event_conn_status_t));

    //! set conn specific event
    rsi_ble_conn_info[ble_conn_id].conn_state = connected_state;
  } else if (remote_device_role == CENTRAL_ROLE) {
    //! Check if already connected
    for (i = 0; i < RSI_BLE_MAX_NBR_CENTRALS; i++) {
      if (!memcmp(rsi_ble_conn_info[RSI_BLE_MAX_NBR_PERIPHERALS + i].remote_dev_addr,
                  RSI_NULL_BLE_ADDR,
                  RSI_REM_DEV_ADDR_LEN)) {

        memcpy(rsi_ble_conn_info[RSI_BLE_MAX_NBR_PERIPHERALS + i].remote_dev_addr,
               remote_dev_addr_conn,
               sizeof(remote_dev_addr_conn));
        memcpy(&rsi_ble_conn_info[RSI_BLE_MAX_NBR_PERIPHERALS + i].conn_event_to_app,
               resp_conn,
               sizeof(rsi_ble_event_conn_status_t));
        rsi_ble_conn_info[RSI_BLE_MAX_NBR_PERIPHERALS + i].is_enhanced_conn   = false;
        rsi_ble_conn_info[RSI_BLE_MAX_NBR_PERIPHERALS + i].conn_id            = RSI_BLE_MAX_NBR_PERIPHERALS + i;
        rsi_ble_conn_info[RSI_BLE_MAX_NBR_PERIPHERALS + i].remote_device_role = CENTRAL_ROLE;
        central_conn_id[i]                                                    = RSI_BLE_MAX_NBR_PERIPHERALS + i;

        break;
      }
    }
  } else {
    LOG_PRINT("\r\n CHECK WHY THIS STATE OCCURS IN CONNECTION");
    while (1)
      ;
  }

#if RSI_DEBUG_EN
  //! event invokes when connection was completed
  LOG_PRINT_D("\r\n In on conn evt - task%d \r\n", ble_conn_id);
#endif

  rsi_ble_conn_info[ble_conn_id].conn_status = 1;
  //! copy connected remote device address in to local buffer
  memcpy(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
         rsi_ble_conn_info[ble_conn_id].conn_event_to_app.dev_addr,
         RSI_DEV_ADDR_LEN);
  //! start SDP query if profile discovery is enabled in connection specific buffer
  if (ble_confgs.ble_conn_configuration[ble_conn_id].profile_discovery) {

    rsi_ble_req_gatt_profile(ble_conn_id);
  }
  //! add device to acceptlist
  if (ble_confgs.ble_conn_configuration[ble_conn_id].add_to_acceptlist) {
    //! clear the acceptlist completely
    status = rsi_ble_clear_acceptlist();
    if (status != RSI_SUCCESS) {
      LOG_PRINT("\r\n Failed to clear the accept list, error: 0x%x -conn%d\r\n", status, ble_conn_id);
    }
    status =
      rsi_ble_addto_acceptlist((int8_t *)rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr, LE_RANDOM_ADDRESS);
    rsi_6byte_dev_address_to_ascii(rsi_ble_conn_info[ble_conn_id].str_remote_address,
                                   rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr);
    if (status != RSI_SUCCESS) {
      LOG_PRINT("\r\n Failed to add the device:%s to acceptlist -conn%d\r\n",
                rsi_ble_conn_info[ble_conn_id].str_remote_address,
                ble_conn_id);
    }
    LOG_PRINT("\r\n device %s added to acceptlist -conn%d \r\n",
              rsi_ble_conn_info[ble_conn_id].str_remote_address,
              ble_conn_id);
  }

  //! perform MTU exchange with remote device
  if (RSI_BLE_MTU_EXCHANGE_FROM_HOST) {
    rsi_ascii_dev_address_to_6bytes_rev(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
                                        (int8_t *)rsi_ble_conn_info[ble_conn_id].remote_dev_addr);

    status = rsi_ble_mtu_exchange_event(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr, MAX_MTU_SIZE);
    if (status != 0) {
      LOG_PRINT("\r\n MTU Exchange request failed -conn%d\n", ble_conn_id);
    }
  }
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_disconnect(uint16_t status, void *event_data)
 * @brief       handler for event_disconnect to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_disconnect to be executed in ble task context
 *
 */
void rsi_ble_event_disconnect(uint16_t status, void *event_data)
{
  uint8_t ble_conn_id;
  rsi_ble_event_disconnect_t *resp_disconnect = (rsi_ble_event_disconnect_t *)event_data;
  LOG_PRINT("On Event Disconnect");
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, resp_disconnect->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_ble_disconn_resp, resp_disconnect, sizeof(rsi_ble_event_disconnect_t));

  //! clear the served event

  rsi_ble_conn_info[ble_conn_id].indication_handle            = 0;
  rsi_ble_conn_info[ble_conn_id].indication_handle_found      = false;
  rsi_ble_conn_info[ble_conn_id].notify_handle_found          = false;
  rsi_ble_conn_info[ble_conn_id].notify_handle                = 0;
  rsi_ble_conn_info[ble_conn_id].write_handle                 = 0;
  rsi_ble_conn_info[ble_conn_id].write_handle_found           = false;
  rsi_ble_conn_info[ble_conn_id].write_cnt                    = 0;
  rsi_ble_conn_info[ble_conn_id].write_wwr_handle_found       = false;
  rsi_ble_conn_info[ble_conn_id].write_wwr_handle             = 0;
  rsi_ble_conn_info[ble_conn_id].wwr_count                    = 0;
  rsi_ble_conn_info[ble_conn_id].smp_done                     = 0;
  rsi_ble_conn_info[ble_conn_id].neg_rply                     = 0;
  rsi_ble_conn_info[ble_conn_id].mtu_exchange_done            = 0;
  rsi_ble_conn_info[ble_conn_id].prof_resp_recvd              = false;
  rsi_ble_conn_info[ble_conn_id].done_profiles_query          = false;
  rsi_ble_conn_info[ble_conn_id].char_resp_recvd              = false;
  rsi_ble_conn_info[ble_conn_id].char_desc_resp_recvd         = false;
  rsi_ble_conn_info[ble_conn_id].profile_index_for_char_query = 0;
  rsi_ble_conn_info[ble_conn_id].prof_evt_cnt                 = 0;
  rsi_ble_conn_info[ble_conn_id].profs_evt_cnt                = 0;
  rsi_ble_conn_info[ble_conn_id].char_for_serv_cnt            = 0;
  rsi_ble_conn_info[ble_conn_id].buff_config_done             = false;
  more_data_state_beta[ble_conn_id].data_transmit             = false;

  //! clear the profile data
#if (CONNECT_OPTION == CONN_BY_NAME)
  free(rsi_ble_conn_info[ble_conn_id].rsi_remote_name);
  rsi_ble_conn_info[ble_conn_id].rsi_remote_name = NULL;
#endif
  free(rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_desc);
  free(rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_info_uuid);
  free(rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_char_info);
  memset(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr, 0, RSI_DEV_ADDR_LEN);

  //! check whether disconnection is from central
  if (ble_conn_id >= RSI_BLE_MAX_NBR_PERIPHERALS) {
    LOG_PRINT("\r\n central is disconnected, reason : 0x%x -conn%d \r\n", status, ble_conn_id);
    rsi_ble_conn_info[ble_conn_id].conn_state = disconnected_state;
    //! decrement the central count

    central_count--;
    adv_state_dut = adv_enabled;
    rsi_ble_event_advertisement_restart_driver_callback();
  } else {
    LOG_PRINT("\r\n peripheral is disconnected, reason : 0x%x -conn%d \r\n", status, ble_conn_id);
    rsi_ble_conn_info[ble_conn_id].conn_state = disconnected_state;

    peripheral_count--;

    rsi_ble_event_scan_restart_driver_callback();
  }

  LOG_PRINT("\r\n Number of connected central devices:%d\n", central_count);
  LOG_PRINT("\r\n Number of connected peripheral devices:%d\n", peripheral_count);

  memset(&rsi_ble_conn_info[ble_conn_id], 0, sizeof(rsi_ble_conn_info_t));
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_enhance_conn_status(uint16_t status, void *event_data)
 * @brief       handler for event_enhnace_conn_status to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_enhnace_conn_status to be executed in ble task context
 *
 */
void rsi_ble_event_enhance_conn_status(uint16_t status, void *event_data)
{

  rsi_ble_event_enhance_conn_status_t *resp_enh_conn = (rsi_ble_event_enhance_conn_status_t *)event_data;
  LOG_PRINT("On Enhance Connect Event ");
  // This statement is added only to resolve compilation warning  : [-Wunused-parameter] , value is unchanged
  UNUSED_PARAMETER(status);
  uint8_t i;
  uint8_t remote_device_role        = 0;
  uint8_t RSI_NULL_BLE_ADDR[18]     = { 0 };
  uint8_t debug_remote_dev_addr[18] = { 0 };

  uint8_t ble_conn_id;

#if RSI_DEBUG_EN
  LOG_PRINT_D("\r\nIn on_enhance_conn cb\r\n");
#endif

  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, resp_enh_conn->dev_addr);

  LOG_PRINT("\n Connected Address: %s \n", remote_dev_addr_conn);

  //! Check whether the received connected event came from remote peripheral or central
  remote_device_role = rsi_get_remote_device_role(remote_dev_addr_conn);

  if (resp_enh_conn->status != 0 && resp_enh_conn->status != 63) {
    LOG_PRINT("\r\n On enhanced connect event status report : %d", resp_enh_conn->status);
    peripheral_count--;
    rsi_ble_event_scan_restart_driver_callback();
    return;
  }
  if (remote_device_role == PERIPHERAL_ROLE) // check for the connection is from peripheral or central
  {
#if (CONNECT_OPTION != CONN_BY_NAME)
    //! get conn_id
    ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
    ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

    LOG_PRINT("\nBLE CONN ID : %d\n", ble_conn_id);

    //! copy to conn specific buffer
    memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_enhc_conn_status,
           resp_enh_conn,
           sizeof(rsi_ble_event_enhance_conn_status_t));

    rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, resp_enh_conn->dev_addr);
    LOG_PRINT_D("\n enhance conn copying buffer RSI CONNECTED ADDRESS : %s \n", remote_dev_addr_conn);
    peripheral_count++;
    if (!ble_confgs.ble_conn_configuration[ble_conn_id].smp_enable) {
      rsi_ble_event_scan_restart_driver_callback();
    }
  } else if (remote_device_role == CENTRAL_ROLE) {
    adv_state_dut = adv_disabled;
    for (i = 0; i < (RSI_BLE_MAX_NBR_CENTRALS); i++) {
      if (!memcmp(rsi_ble_conn_info[RSI_BLE_MAX_NBR_PERIPHERALS + i].remote_dev_addr,
                  RSI_NULL_BLE_ADDR,
                  RSI_REM_DEV_ADDR_LEN)) {
        LOG_PRINT_D("\n\n\n  Central Index = %d \n\n\n", i);

        memcpy(rsi_ble_conn_info[RSI_BLE_MAX_NBR_PERIPHERALS + i].remote_dev_addr,
               remote_dev_addr_conn,
               sizeof(remote_dev_addr_conn));
        memcpy(&rsi_ble_conn_info[RSI_BLE_MAX_NBR_PERIPHERALS + i].rsi_enhc_conn_status,
               resp_enh_conn,
               sizeof(rsi_ble_event_enhance_conn_status_t));

        rsi_ble_conn_info[RSI_BLE_MAX_NBR_PERIPHERALS + i].is_enhanced_conn   = true;
        rsi_ble_conn_info[RSI_BLE_MAX_NBR_PERIPHERALS + i].conn_id            = RSI_BLE_MAX_NBR_PERIPHERALS + i;
        rsi_ble_conn_info[RSI_BLE_MAX_NBR_PERIPHERALS + i].remote_device_role = CENTRAL_ROLE;

        ble_conn_id = rsi_ble_conn_info[RSI_BLE_MAX_NBR_PERIPHERALS + i].conn_id;
        central_count++;
        LOG_PRINT_D("\n COnn ID from central : %d\n", rsi_ble_conn_info[RSI_BLE_MAX_NBR_PERIPHERALS + i].conn_id);

        //! set below flag to use whether the connection requested from remote device is enhanced connection or normal connection
        break;
      }
    }
    rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, resp_enh_conn->dev_addr);
    LOG_PRINT("\n enhance conn copying buffer RSI CONNECTEED ADDRESS : %s \n", remote_dev_addr_conn);
  } else {
    LOG_PRINT("\r\n CHECK WHY THIS STATE OCCURS IN CONNECTION");
    while (1)
      ;
  }
  LOG_PRINT("\r\n In on_enhance_conn evt - conn%d\r\n", ble_conn_id);
  if (remote_device_role == CENTRAL_ROLE) {
    rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[ble_conn_id].conn_id = ble_conn_id;
    ble_confgs.ble_conn_configuration[ble_conn_id] = rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[ble_conn_id];
    LOG_PRINT("\nBLE conn ID: %d\n", ble_conn_id);
    rsi_ble_event_advertisement_restart_driver_callback();
  }
  LOG_PRINT_D("\r\n after of loop In on_enhance_conn evt - conn%d\r\n", ble_conn_id);

  rsi_ble_conn_info[ble_conn_id].conn_status = 1;
  //! copy connected remote device address in to local buffer
  memcpy(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
         rsi_ble_conn_info[ble_conn_id].rsi_enhc_conn_status.dev_addr,
         RSI_DEV_ADDR_LEN);

  LOG_PRINT_D("\n MTU Exchange Event Trigerred \n");
  if (RSI_BLE_MTU_EXCHANGE_FROM_HOST) {
    status = rsi_ble_mtu_exchange_event(rsi_ble_conn_info[ble_conn_id].rsi_enhc_conn_status.dev_addr, MAX_MTU_SIZE);
    if (status != 0) {
      LOG_PRINT("\r\n MTU Exchange request failed -conn%d\n", ble_conn_id);
    }
  }
  memcpy(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
         rsi_ble_conn_info[ble_conn_id].rsi_enhc_conn_status.dev_addr,
         RSI_DEV_ADDR_LEN);
  rsi_6byte_dev_address_to_ascii(debug_remote_dev_addr, rsi_ble_conn_info[ble_conn_id].rsi_enhc_conn_status.dev_addr);
  LOG_PRINT_D("\n dev_addr RSI CONNECTEED ADDRESS : %s \n", debug_remote_dev_addr);

  rsi_6byte_dev_address_to_ascii(debug_remote_dev_addr,
                                 rsi_ble_conn_info[ble_conn_id].rsi_enhc_conn_status.local_resolvlable_addr);
  LOG_PRINT_D("\n local_resolvlable_addr RSI CONNECTEED ADDRESS : %s \n", debug_remote_dev_addr);
  rsi_6byte_dev_address_to_ascii(debug_remote_dev_addr,
                                 rsi_ble_conn_info[ble_conn_id].rsi_enhc_conn_status.peer_resolvlable_addr);
  LOG_PRINT_D("\n peer_resolvlable_addr RSI CONNECTEED ADDRESS : %s \n", debug_remote_dev_addr);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_more_data_req(uint16_t status, void *event_data)
 * @brief       handler for event_le_dev_buf_ind to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_le_dev_buf_ind to be executed in ble task context
 *
 */
void rsi_ble_event_more_data_req(uint16_t __attribute((unused)) status, void *event_data)
{
  uint8_t ble_conn_id;

  rsi_ble_event_le_dev_buf_ind_t *rsi_ble_more_data_evt = (rsi_ble_event_le_dev_buf_ind_t *)event_data;
#if RSI_DEBUG_EN
  LOG_PRINT_D("\r\n Received more data event in main task\r\n");
#endif
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_more_data_evt->remote_dev_bd_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  // memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_ble_more_data_evt, rsi_ble_more_data_evt, sizeof(rsi_ble_event_le_dev_buf_ind_t));
  if (more_data_state_beta[ble_conn_id].data_transmit) {
    more_data_state_beta[ble_conn_id].data_transmit = 0;

    rsi_ble_on_data_transmit(ble_conn_id);
  }
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_phy_update(uint16_t status, void *event_data)
 * @brief       handler for event_phy_update to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_phy_update to be executed in ble task context
 *
 */
void rsi_ble_event_phy_update(uint16_t __attribute((unused)) status, void *event_data)
{

  rsi_ble_event_phy_update_t *rsi_ble_event_phy_update_complete = (rsi_ble_event_phy_update_t *)event_data;
  LOG_PRINT_D("\r\nIn event phy update cb\r\n");
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_event_phy_update_complete->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif
}

/*==============================================*/
/**
 * @fn          void rsi_ble_gatt_error_event(uint16_t status, void *event_data)
 * @brief       handler for event_error_resp to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_error_resp to be executed in ble task context
 *
 */
void rsi_ble_gatt_error_event(uint16_t status, void *event_data)
{
  uint8_t ble_conn_id;

  rsi_ble_event_error_resp_t *rsi_ble_gatt_error = (rsi_ble_event_error_resp_t *)event_data;
  LOG_PRINT_D("\r\nIn event error resp cb\r\n");
  // This statement is added only to resolve compilation warning  : [-Wunused-parameter] , value is unchanged
  UNUSED_PARAMETER(status);
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_gatt_error->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_ble_gatt_err_resp, rsi_ble_gatt_error, sizeof(rsi_ble_event_error_resp_t));

  if ((*(uint16_t *)rsi_ble_conn_info[ble_conn_id].rsi_ble_gatt_err_resp.error) == RSI_END_OF_PROFILE_QUERY) {
    if (rsi_ble_conn_info[ble_conn_id].total_remote_profiles != 0) //! If any profiles exists
    {
      if ((rsi_ble_conn_info[ble_conn_id].profile_index_for_char_query - 1)
          < rsi_ble_conn_info[ble_conn_id].total_remote_profiles) //! Error received for any profile
      {
        rsi_ble_conn_info[ble_conn_id].char_resp_recvd = false;
        rsi_ble_conn_info[ble_conn_id].char_for_serv_cnt++;
        profile_dummy_data.ble_con_id = ble_conn_id;
        profile_dummy_data.swtch_cnt  = 1;

        rsi_ble_event_profile_dummy_driver_callback(&profile_dummy_data);

      } else //! Error received for last profile
      {
        profile_dummy_data.ble_con_id = ble_conn_id;
        profile_dummy_data.swtch_cnt  = 2;

        rsi_ble_event_profile_dummy_driver_callback(&profile_dummy_data);
#if 0
        rsi_ble_conn_info[ble_conn_id].skip_end_profile = 1 ;
         status = rsi_ble_get_profiles_async(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr, 1, 0xffff, NULL);
          if (status != RSI_SUCCESS) {
            //! check for procedure already in progress error
            if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
              LOG_PRINT("\r\n rsi_ble_get_profiles_async procedure is already in progress -conn%d \r\n", ble_conn_id);

            }
            //! check for buffer full error, which is not expected for this procedure
            else if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
              LOG_PRINT("\r\n rsi_ble_get_profiles_async failed with buffer full error -conn%d \r\n", ble_conn_id);

            } else {
              LOG_PRINT("\r\n get profile async call failed with error code :%x -conn%d \r\n", status, ble_conn_id);
            }
          }

#endif
      }
    } else //! Check for profiles pending, else done profile querying
    {
      //! first level profile query completed
      rsi_ble_conn_info[ble_conn_id].done_profiles_query = true;
      profile_dummy_data.ble_con_id                      = ble_conn_id;
      profile_dummy_data.swtch_cnt                       = 3;

      rsi_ble_event_profile_dummy_driver_callback(&profile_dummy_data);
    }
  } else {
    LOG_PRINT("\r\nGATT ERROR REASON:0x%x -conn%d \n",
              *(uint16_t *)rsi_ble_conn_info[ble_conn_id].rsi_ble_gatt_err_resp.error,
              ble_conn_id);
  }
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_conn_update(uint16_t status, void *event_data)
 * @brief       handler for event_conn_update to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_conn_update to be executed in ble task context
 *
 */
void rsi_ble_event_conn_update(uint16_t __attribute((unused)) status, void __attribute((unused)) * event_data)
{
  return;
#if 0
  uint8_t ble_conn_id;
  rsi_ble_event_conn_update_t *resp_conn_update = (rsi_ble_event_conn_update_t *)event_data;
  LOG_PRINT_D("\r\nIn conn update cb\r\n");

  if (status != 0) {
    LOG_PRINT("\r\n RSI_BLE_CONN_UPDATE_EVENT FAILED : %d\r\n", status);
    return;
  } else {
    //! convert to ascii
    rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, resp_conn_update->dev_addr);

    //! get conn_id
#if (CONNECT_OPTION != CONN_BY_NAME)
    //! get conn_id
    ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
    ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

    //! copy to conn specific buffer
    memcpy(&rsi_ble_conn_info[ble_conn_id].conn_update_resp, resp_conn_update, sizeof(rsi_ble_event_conn_update_t));

    LOG_PRINT("\r\n conn updated device address : %s\n conn_interval:%d\n supervision timeout:%d -conn%d",
              rsi_ble_conn_info[ble_conn_id].remote_dev_addr,
              rsi_ble_conn_info[ble_conn_id].conn_update_resp.conn_interval,
              rsi_ble_conn_info[ble_conn_id].conn_update_resp.timeout,
              ble_conn_id);
#if UPDATE_CONN_PARAMETERS
    status = rsi_conn_update_request();
#endif

#if BLE_DYNAMIC_BUFF_CONFIGURATION_BASED_ON_INTERVAL
    if (ble_confgs.ble_conn_configuration[ble_conn_id].buff_mode_sel.buffer_mode == 1) {
      if ((rsi_ble_conn_info[ble_conn_id].conn_update_resp.conn_interval) <= 12) {
        //! Updated Feasible Buffer Count based on the Connection Interval
        if (rsi_ble_conn_info[ble_conn_id].remote_device_role == CENTRAL_ROLE) {
          ble_confgs.ble_conn_configuration[ble_conn_id].buff_mode_sel.buffer_cnt = 3;
        }
      } else if ((rsi_ble_conn_info[ble_conn_id].conn_update_resp.conn_interval) <= 40) {
        //! Updated Feasible Buffer Count based on the Connection Interval
        if (rsi_ble_conn_info[ble_conn_id].remote_device_role == CENTRAL_ROLE) {
          ble_confgs.ble_conn_configuration[ble_conn_id].buff_mode_sel.buffer_cnt = 5;
        }
      } else {
        //! Updated Feasible Buffer Count based on the Connection Interval
        if (rsi_ble_conn_info[ble_conn_id].remote_device_role == CENTRAL_ROLE) {
          ble_confgs.ble_conn_configuration[ble_conn_id].buff_mode_sel.buffer_cnt = 7;
        }
      }

      if (rsi_ble_conn_info[ble_conn_id].remote_device_role == CENTRAL_ROLE) {
        // take backup of the Data transfer bitmap
        if (rsi_ble_check_bitmap_based_on_conn(ble_conn_id, RSI_DATA_RECEIVE_EVENT)) {
          rsi_ble_conn_info[ble_conn_id].rsi_rx_from_rem_dev = true;
          rsi_ble_clear_event_based_on_conn(ble_conn_id, RSI_DATA_RECEIVE_EVENT);
        }
        if (rsi_ble_check_bitmap_based_on_conn(ble_conn_id, RSI_DATA_TRANSMIT_EVENT)) {
          rsi_ble_conn_info[ble_conn_id].rsi_tx_to_rem_dev = true;
          rsi_ble_clear_event_based_on_conn(ble_conn_id, RSI_DATA_TRANSMIT_EVENT);
        }
        rsi_ble_conn_info[ble_conn_id].buff_config_done = false;
        //! Re-configure the buffer configuration mode
        rsi_ble_set_event_based_on_conn(ble_conn_id, RSI_BLE_BUFF_CONF_EVENT);
      }
    }
#endif
  }
#endif
}
void rsi_ble_select_data_transfer(uint8_t conn_id)
{
  uint8_t ble_conn_id = conn_id;

  //! if receive 'write' is configured, then check for client characteristic handle having Gatt 'write' property
  if (ble_confgs.ble_conn_configuration[ble_conn_id].tx_write) {
    //! check for required client service and characteristic UUID
    for (i = 0; i < rsi_ble_conn_info[ble_conn_id].total_remote_profiles; i++) {
      //! check for configured client service UUID
      if (rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_desc[i].profile_uuid.val.val16
          == ble_confgs.ble_conn_configuration[ble_conn_id].tx_write_clientservice_uuid) {
        for (uint8_t ix = 0;
             ix < rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_char_info[i].num_of_services;
             ix++) {
          //! check for configured client characteristic UUID
          if (rsi_ble_conn_info[ble_conn_id]
                .rsi_ble_profile_list_by_conn.profile_char_info[i]
                .char_services[ix]
                .char_data.char_uuid.val.val16
              == ble_confgs.ble_conn_configuration[ble_conn_id].tx_write_client_char_uuid) {
            //! check for 'write' characteristic property
            if ((!rsi_ble_conn_info[ble_conn_id].write_handle_found)
                && ((rsi_ble_conn_info[ble_conn_id]
                       .rsi_ble_profile_list_by_conn.profile_char_info[i]
                       .char_services[ix]
                       .char_data.char_property)
                    & RSI_BLE_ATT_PROPERTY_WRITE)) {
              LOG_PRINT("\r\n write handle found -conn%d \n", ble_conn_id);
              rsi_ble_conn_info[ble_conn_id].write_handle_found = true;
              rsi_ble_conn_info[ble_conn_id].write_handle       = rsi_ble_conn_info[ble_conn_id]
                                                              .rsi_ble_profile_list_by_conn.profile_char_info[i]
                                                              .char_services[ix]
                                                              .char_data.char_handle;
              rsi_ble_conn_info[ble_conn_id].rsi_tx_to_rem_dev = true;
              //! configure the buffer configuration mode
              rsi_ble_event_set_buffer_config_driver_callback(&ble_conn_id);
              break;
            }
          }
        }
      }
    }
    if (!rsi_ble_conn_info[ble_conn_id].write_handle_found) {
      LOG_PRINT("\r\n Client Gatt write service not found -conn%d \n", ble_conn_id);
    }
  }

  //! if receive 'write no response' is configured, then check for client characteristic handle having Gatt 'write No response' property
  else if (ble_confgs.ble_conn_configuration[ble_conn_id].tx_write_no_response) {
    //! check for required client service and characteristic UUID
    for (i = 0; i < rsi_ble_conn_info[ble_conn_id].total_remote_profiles; i++) {
      //! check for configured client service UUID
      if (rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_desc[i].profile_uuid.val.val16
          == ble_confgs.ble_conn_configuration[ble_conn_id].tx_wnr_client_service_uuid) {
        //! check for configured client service UUID
        for (uint8_t ix = 0;
             ix < rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_char_info[i].num_of_services;
             ix++) {
          //! check for configured client characteristic UUID
          if (rsi_ble_conn_info[ble_conn_id]
                .rsi_ble_profile_list_by_conn.profile_char_info[i]
                .char_services[ix]
                .char_data.char_uuid.val.val16
              == ble_confgs.ble_conn_configuration[ble_conn_id].tx_wnr_client_char_uuid) {
            //! check for 'write no response' characteristic property
            if ((!rsi_ble_conn_info[ble_conn_id].write_wwr_handle_found)
                && (rsi_ble_conn_info[ble_conn_id]
                      .rsi_ble_profile_list_by_conn.profile_char_info[i]
                      .char_services[ix]
                      .char_data.char_property
                    & RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE)) {
#if RSI_DEBUG_EN
              LOG_PRINT_D("\r\n write without response handle found -conn%d \n", ble_conn_id);
#endif
              rsi_ble_conn_info[ble_conn_id].write_wwr_handle_found = true; //! write handle found
              rsi_ble_conn_info[ble_conn_id].write_wwr_handle       = rsi_ble_conn_info[ble_conn_id]
                                                                  .rsi_ble_profile_list_by_conn.profile_char_info[i]
                                                                  .char_services[ix]
                                                                  .char_data.char_handle;
              rsi_ble_conn_info[ble_conn_id].rsi_tx_to_rem_dev = true;
              rsi_ble_event_set_buffer_config_driver_callback(&ble_conn_id);
              break;
            }
          }
        }
      }
    }
    if (!rsi_ble_conn_info[ble_conn_id].write_wwr_handle_found) {
      LOG_PRINT("\r\n Client Gatt write no response service not found -conn%d \n", ble_conn_id);
    }
  }

  //! if receive 'Indications' is configured, then check for client characteristic handle having Gatt 'Indication' property
  if (ble_confgs.ble_conn_configuration[ble_conn_id].rx_indications) {
    //! check for required client service and characteristic UUID
    for (i = 0; i < rsi_ble_conn_info[ble_conn_id].total_remote_profiles; i++) {
      //! check for configured client service UUID
      if (rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_desc[i].profile_uuid.val.val16
          == ble_confgs.ble_conn_configuration[ble_conn_id].rx_indi_client_service_uuid) {
        for (uint8_t ix = 0;
             ix < rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_char_info[i].num_of_services;
             ix++) {
          //! check for configured client characteristic UUID
          if (rsi_ble_conn_info[ble_conn_id]
                .rsi_ble_profile_list_by_conn.profile_char_info[i]
                .char_services[ix]
                .char_data.char_uuid.val.val16
              == ble_confgs.ble_conn_configuration[ble_conn_id].rx_indi_client_char_uuid) {
            //! check for 'Indication' characteristic property
            if ((!rsi_ble_conn_info[ble_conn_id].indication_handle_found)
                && ((rsi_ble_conn_info[ble_conn_id]
                       .rsi_ble_profile_list_by_conn.profile_char_info[i]
                       .char_services[ix]
                       .char_data.char_property)
                    & RSI_BLE_ATT_PROPERTY_INDICATE)) {
              LOG_PRINT("\r\n indicate handle found -conn%d \n", ble_conn_id);
              rsi_ble_conn_info[ble_conn_id].indication_handle_found = true;
              rsi_ble_conn_info[ble_conn_id].indication_handle       = rsi_ble_conn_info[ble_conn_id]
                                                                   .rsi_ble_profile_list_by_conn.profile_char_info[i]
                                                                   .char_services[ix]
                                                                   .char_data.char_handle;
              rsi_ble_conn_info[ble_conn_id].rsi_rx_from_rem_dev = true;
              rsi_conn_update_req_event(ble_conn_id);
              break;
            }
          }
        }
      }
    }
    if (!rsi_ble_conn_info[ble_conn_id].indication_handle_found) {
      LOG_PRINT("\r\n Client Gatt Indication service not found -conn%d \n", ble_conn_id);
    }
  }

  //! if receive 'Notifications' is configured, then check for client characteristic handle having Gatt 'Notification' property
  else if (ble_confgs.ble_conn_configuration[ble_conn_id].rx_notifications) {
    //! check for required client service and characteristic UUID
    for (i = 0; i < rsi_ble_conn_info[ble_conn_id].total_remote_profiles; i++) {
      if (rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_desc[i].profile_uuid.val.val16
          == ble_confgs.ble_conn_configuration[ble_conn_id].rx_notif_client_service_uuid) {
        //! check for configured client service UUID
        for (uint8_t ix = 0;
             ix < rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_char_info[i].num_of_services;
             ix++) {
          //! check for configured client characteristic UUID
          if (rsi_ble_conn_info[ble_conn_id]
                .rsi_ble_profile_list_by_conn.profile_char_info[i]
                .char_services[ix]
                .char_data.char_uuid.val.val16
              == ble_confgs.ble_conn_configuration[ble_conn_id].rx_notif_client_char_uuid) {
            //! check for 'Notification' characteristic property
            if ((!rsi_ble_conn_info[ble_conn_id].notify_handle_found)
                && ((rsi_ble_conn_info[ble_conn_id]
                       .rsi_ble_profile_list_by_conn.profile_char_info[i]
                       .char_services[ix]
                       .char_data.char_property)
                    & RSI_BLE_ATT_PROPERTY_NOTIFY)) {
              rsi_ble_conn_info[ble_conn_id].notify_handle_found = true;
              rsi_ble_conn_info[ble_conn_id].notify_handle       = rsi_ble_conn_info[ble_conn_id]
                                                               .rsi_ble_profile_list_by_conn.profile_char_info[i]
                                                               .char_services[ix]
                                                               .char_data.char_handle;
              rsi_ble_conn_info[ble_conn_id].rsi_rx_from_rem_dev = true;
              rsi_conn_update_req_event(ble_conn_id);
              break;
            }
          }
        }
      }
    }
    if (!rsi_ble_conn_info[ble_conn_id].notify_handle_found) {
      LOG_PRINT("\r\n Client Gatt Notification service not found \n");
    }
  }
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_select_data_transfer(uint16_t status, void *event_data)
 * @brief       handler for event_advertisement_restart to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_advertisement_restart to be executed in ble task context
 *
 */
void rsi_ble_event_select_data_transfer(uint16_t __attribute((unused)) status, void *event_data)
{

  uint8_t *conn_id    = (uint8_t *)event_data;
  uint8_t ble_conn_id = *conn_id;

  rsi_ble_select_data_transfer(ble_conn_id);
}
void rsi_ble_profile_discovery(uint8_t conn_id)
{

  uint8_t ble_conn_id = conn_id;

  if (!rsi_ble_conn_info[ble_conn_id].char_desc_resp_recvd) {
    rsi_ble_conn_info[ble_conn_id].char_desc_resp_recvd = true;
    //! search for all characteristics descriptor in all profiles

    if (rsi_ble_conn_info[ble_conn_id].temp1 < rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {
      rsi_ble_conn_info[ble_conn_id].l_num_of_services =
        rsi_ble_conn_info[ble_conn_id]
          .rsi_ble_profile_list_by_conn.profile_char_info[rsi_ble_conn_info[ble_conn_id].temp1]
          .num_of_services;

      //! search for all characteristics descriptor in each profile
      if (rsi_ble_conn_info[ble_conn_id].temp2 < rsi_ble_conn_info[ble_conn_id].l_num_of_services) {
        rsi_ble_conn_info[ble_conn_id].l_char_property =
          rsi_ble_conn_info[ble_conn_id]
            .rsi_ble_profile_list_by_conn.profile_char_info[rsi_ble_conn_info[ble_conn_id].temp1]
            .char_services[rsi_ble_conn_info[ble_conn_id].temp2]
            .char_data.char_property;
        if ((rsi_ble_conn_info[ble_conn_id].l_char_property == RSI_BLE_ATT_PROPERTY_INDICATE)
            || (rsi_ble_conn_info[ble_conn_id].l_char_property == RSI_BLE_ATT_PROPERTY_NOTIFY)) {
          // LOG_PRINT("\r\n query for profile service1 %d -conn%d \n",rsi_ble_conn_info[ble_conn_id].temp1);
          status = rsi_ble_get_att_value_async(
            rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
            rsi_ble_conn_info[ble_conn_id]
              .rsi_ble_profile_list_by_conn.profile_char_info[rsi_ble_conn_info[ble_conn_id].temp1]
              .char_services[rsi_ble_conn_info[ble_conn_id].temp2]
              .handle,
            NULL);

          if (status != RSI_SUCCESS) {
            //! check for procedure already in progress error
            if (status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
              rsi_ble_conn_info[ble_conn_id].char_desc_resp_recvd = false;
#if RSI_DEBUG_EN
              LOG_PRINT_D("\r\n rsi_ble_get_att_value_async procedure is already in progress -m1 \r\n");
#endif
              return;
            }
            //! check for buffer full error, which is not expected for this procedure
            else if (status == RSI_ERROR_BLE_DEV_BUF_FULL) {
              LOG_PRINT("\r\n rsi_ble_get_att_value_async failed with buffer full error -m1 \r\n");
              return;
            } else {
              LOG_PRINT("\r\n failed to get characteristics descriptor of the remote GATT server with "
                        "error:0x%lx -m1 \r\n",
                        status);
              return;
            }
          }
        } else {
          rsi_ble_conn_info[ble_conn_id].temp2++;
          rsi_ble_conn_info[ble_conn_id].char_desc_resp_recvd = false;
          rsi_ble_event_profile_discovery_driver_callback(&ble_conn_id);
        }
      }

      if (rsi_ble_conn_info[ble_conn_id].temp2 == rsi_ble_conn_info[ble_conn_id].l_num_of_services) {
        rsi_ble_conn_info[ble_conn_id].temp2                = 0; //!  to start searching from starting of next profile
        rsi_ble_conn_info[ble_conn_id].char_desc_resp_recvd = false;
        rsi_ble_conn_info[ble_conn_id].temp1++;
        rsi_ble_event_profile_discovery_driver_callback(
          &ble_conn_id); //! look for next profile, after completion of searching all characteristic descriptors in one profile
      }
    }

    else if (rsi_ble_conn_info[ble_conn_id].temp1 == rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {
      LOG_PRINT("\r\n remote device profiles discovery completed -conn%d \n", ble_conn_id);
      //! if data transfer is configured
      if (ble_confgs.ble_conn_configuration[ble_conn_id].data_transfer) {
        LOG_PRINT("\r\n data transfer req done \r\n");
        rsi_ble_event_select_data_transfer_driver_callback(&ble_conn_id);
      }
    }
  }
  return;
}
/*==============================================*/
/**
 * @fn          void rsi_ble_event_profile_discovery(uint16_t status, void *event_data)
 * @brief       handler for event_advertisement_restart to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_advertisement_restart to be executed in ble task context
 *
 */
void rsi_ble_event_profile_discovery(uint16_t __attribute__((unused)) status, void *event_data)
{
  //! Add handling here

  uint8_t *conn_id    = (uint8_t *)event_data;
  uint8_t ble_conn_id = *conn_id;

  rsi_ble_profile_discovery(ble_conn_id);
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_gatt_desc(uint16_t status, void *event_data)
 * @brief       handler for event_gatt_desc to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_gatt_desc to be executed in ble task context
 *
 */
void rsi_ble_event_gatt_desc(uint16_t status, void *event_data)
{

  uint8_t ble_conn_id;
  rsi_ble_event_att_value_t *rsi_ble_event_att_val = (rsi_ble_event_att_value_t *)event_data;
  // This statement is added only to resolve compilation warning  : [-Wunused-parameter] , value is unchanged
  UNUSED_PARAMETER(status);
#if RSI_DEBUG_EN
  LOG_PRINT_D("\r\n in Gatt descriptor response event \r\n");
#endif

  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_event_att_val->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_char_descriptors,
         rsi_ble_event_att_val,
         sizeof(rsi_ble_event_att_value_t));
#if RSI_DEBUG_EN
  LOG_PRINT_D("\r\n in gatt test:RSI_BLE_GATT_DESC_SERVICES : rsi_ble_conn_info[ble_conn_id].temp1 %d -conn%d \r\n",
              rsi_ble_conn_info[ble_conn_id].temp1,
              ble_conn_id);
#endif
  if (rsi_ble_conn_info[ble_conn_id].temp1 < rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {
    rsi_ble_conn_info[ble_conn_id].temp2++;
    rsi_ble_conn_info[ble_conn_id].char_desc_cnt++;
    rsi_ble_conn_info[ble_conn_id].char_desc_resp_recvd = false;
  } else {
  }
  if (rsi_ble_conn_info[ble_conn_id].temp1 == rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {
    LOG_PRINT("\r\n remote device profiles discovery completed -conn%d \n", ble_conn_id);
    //! if data transfer is configured
    if (ble_confgs.ble_conn_configuration[ble_conn_id].data_transfer) {
      rsi_ble_event_select_data_transfer_driver_callback(&ble_conn_id);
    }
  }
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_profiles_list(uint16_t status, void *event_data)
 * @brief       handler for event_profiles_list to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_profiles_list to be executed in ble task context
 *
 */
void rsi_ble_event_profiles_list(uint16_t status, void *event_data)
{
  uint8_t ble_conn_id;

  rsi_ble_event_profiles_list_t *rsi_ble_event_profiles = (rsi_ble_event_profiles_list_t *)event_data;
  if (status == 0x4A0A) {
    return;
  }
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_event_profiles->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].get_allprofiles,
         rsi_ble_event_profiles,
         sizeof(rsi_ble_event_profiles_list_t));

#if RSI_DEBUG_EN
  //! prof_resp_recvd is set to false for every profile query response
  LOG_PRINT_D("\r\n in gatt profiles -conn%d\n", ble_conn_id);
#endif
  if (!rsi_ble_conn_info[ble_conn_id].prof_resp_recvd) {
    //! check until completion of first level query
    if (!rsi_ble_conn_info[ble_conn_id].done_profiles_query) {
#if RSI_DEBUG_EN
      LOG_PRINT_D("\r\n in profile query -conn%d \n", ble_conn_id);
#endif
      rsi_ble_conn_info[ble_conn_id].no_of_profiles = rsi_ble_conn_info[ble_conn_id].get_allprofiles.number_of_profiles;
      //! copy the end of handle of last searched profile
      rsi_ble_conn_info[ble_conn_id].profiles_endhandle =
        *(uint16_t *)(rsi_ble_conn_info[ble_conn_id]
                        .get_allprofiles.profile_desc[rsi_ble_conn_info[ble_conn_id].no_of_profiles - 1]
                        .end_handle);
#if RSI_DEBUG_EN
      LOG_PRINT_D("\r\n profiles end handle = 0x%x -conn%d\n",
                  rsi_ble_conn_info[ble_conn_id].profiles_endhandle,
                  ble_conn_id);
#endif

      //! allocate memory
      if (rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_desc == NULL) {
        rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_desc = (profile_descriptors_t *)malloc(
          sizeof(profile_descriptors_t) * rsi_ble_conn_info[ble_conn_id].no_of_profiles);
        memset(rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_desc,
               0,
               sizeof(profile_descriptors_t) * rsi_ble_conn_info[ble_conn_id].no_of_profiles);
      } else {
        void *temp = NULL;
        temp       = (void *)malloc(
          sizeof(profile_descriptors_t)
          * (rsi_ble_conn_info[ble_conn_id].total_remote_profiles + rsi_ble_conn_info[ble_conn_id].no_of_profiles));
        if (temp == NULL) {
          LOG_PRINT("failed to allocate memory for "
                    "rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_desc \r\n, conn_id:%d",
                    ble_conn_id);
          return;
        }
        //! fill the allocated buffer with '0'
        memset(
          temp,
          0,
          sizeof(profile_descriptors_t)
            * (rsi_ble_conn_info[ble_conn_id].total_remote_profiles + rsi_ble_conn_info[ble_conn_id].no_of_profiles));
        //! copy the already stored data in to this buffer
        memcpy(temp,
               rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_desc,
               (sizeof(profile_descriptors_t) * rsi_ble_conn_info[ble_conn_id].total_remote_profiles));
        //! free the old buffer which holds data
        free(rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_desc);
        //! assign the new buffer to old buffer
        rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_desc = temp;
      }
      if (rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_desc == NULL) {
        LOG_PRINT("failed to allocate memory for "
                  "rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_desc \r\n, conn_id:%d",
                  ble_conn_id);
        return;
      }
      //! copy retrieved profiles in local central buffer
      for (i = 0; i < rsi_ble_conn_info[ble_conn_id].no_of_profiles; i++) {
        memcpy(&rsi_ble_conn_info[ble_conn_id]
                  .rsi_ble_profile_list_by_conn.profile_desc[i + rsi_ble_conn_info[ble_conn_id].total_remote_profiles],
               &rsi_ble_conn_info[ble_conn_id].get_allprofiles.profile_desc[i],
               sizeof(profile_descriptors_t));
        LOG_PRINT_D("\n profile end handle  recieved: %x \n",
                    *(uint16_t *)(rsi_ble_conn_info[ble_conn_id].get_allprofiles.profile_desc[i].end_handle));
      }
      rsi_ble_conn_info[ble_conn_id].total_remote_profiles += rsi_ble_conn_info[ble_conn_id].no_of_profiles;
      /* profile discovery: if number of profile exceeds the max count stop after searching for top (RSI_MAX_PROFILE_CNT - 1) profiles*/
      if (rsi_ble_conn_info[ble_conn_id].total_remote_profiles >= RSI_MAX_PROFILE_CNT) {

        rsi_ble_conn_info[ble_conn_id].total_remote_profiles = RSI_MAX_PROFILE_CNT - 1;
        rsi_ble_conn_info[ble_conn_id].profiles_endhandle    = 0xffff;
      }
      //! check for end of profiles
      if (rsi_ble_conn_info[ble_conn_id].profiles_endhandle != 0xffff) {
        status = rsi_ble_get_profiles_async(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
                                            rsi_ble_conn_info[ble_conn_id].profiles_endhandle + 1,
                                            0xffff,
                                            NULL);
        if (status != RSI_SUCCESS) {
          //! check for procedure already in progress error
          if (status == (uint16_t)RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {

#if RSI_DEBUG_EN
            LOG_PRINT_D("\r\n rsi_ble_get_profiles_async procedure is already in progress -conn%d \r\n", ble_conn_id);
#endif
          }
          //! check for buffer full error, which is not expected for this procedure
          else if (status == (uint16_t)RSI_ERROR_BLE_DEV_BUF_FULL) {
            LOG_PRINT("\r\n rsi_ble_get_profiles_async failed with buffer full error -conn%d \r\n", ble_conn_id);
          } else {
            LOG_PRINT("\r\n get profile async call failed with error code :%x -conn%d \r\n", status, ble_conn_id);
            return;
          }
        }
      } else {
        //! first level profile query completed
        rsi_ble_conn_info[ble_conn_id].done_profiles_query = true;
      }
    }
    if (rsi_ble_conn_info[ble_conn_id].done_profiles_query) {
      rsi_ble_conn_info[ble_conn_id].prof_resp_recvd = true;
      //! check until completion of second level profiles query
      if (rsi_ble_conn_info[ble_conn_id].profs_evt_cnt < rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {
        for (i = 0; i < rsi_ble_conn_info[ble_conn_id].total_remote_profiles;
             i++, rsi_ble_conn_info[ble_conn_id].profs_evt_cnt++) {
          //! search handles for all retrieved profiles
          rsi_ble_conn_info[ble_conn_id].search_serv_conn[i].size =
            rsi_ble_conn_info[ble_conn_id]
              .rsi_ble_profile_list_by_conn.profile_desc[rsi_ble_conn_info[ble_conn_id].profs_evt_cnt]
              .profile_uuid.size;

          if (rsi_ble_conn_info[ble_conn_id].search_serv_conn[i].size == 2) //! check for 16 bit(2 bytes) UUID value
          {
            rsi_ble_conn_info[ble_conn_id].search_serv_conn[i].val.val16 =
              rsi_ble_conn_info[ble_conn_id]
                .rsi_ble_profile_list_by_conn.profile_desc[rsi_ble_conn_info[ble_conn_id].profs_evt_cnt]
                .profile_uuid.val.val16;
#if RSI_DEBUG_EN
            LOG_PRINT("\r\n search for profile :0x%x -conn%d \r\n",
                      rsi_ble_conn_info[ble_conn_id].search_serv_conn[i].val.val16,
                      ble_conn_id);
#endif
          } else if (rsi_ble_conn_info[ble_conn_id].search_serv_conn[i].size == 4) {
            rsi_ble_conn_info[ble_conn_id].search_serv_conn[i].val.val32 =
              rsi_ble_conn_info[ble_conn_id]
                .rsi_ble_profile_list_by_conn.profile_desc[rsi_ble_conn_info[ble_conn_id].profs_evt_cnt]
                .profile_uuid.val.val32;
#if RSI_DEBUG_EN
            LOG_PRINT("\r\n search for profile :0x%x -conn%d \r\n",
                      rsi_ble_conn_info[ble_conn_id].search_serv_conn[i].val.val32,
                      ble_conn_id);
#endif
          } else if (rsi_ble_conn_info[ble_conn_id].search_serv_conn[i].size == 16) //! 128 bit(16 byte) UUID value
          {
            rsi_ble_conn_info[ble_conn_id].search_serv_conn[i].val.val128 =
              rsi_ble_conn_info[ble_conn_id]
                .rsi_ble_profile_list_by_conn.profile_desc[rsi_ble_conn_info[ble_conn_id].profs_evt_cnt]
                .profile_uuid.val.val128;
#if RSI_DEBUG_EN
            LOG_PRINT("\r\n search for profile :0x%x -conn%d \r\n",
                      rsi_ble_conn_info[ble_conn_id].search_serv_conn[i].val.val128,
                      ble_conn_id);
#endif
          }
        }
        status = rsi_ble_get_profile_async(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
                                           rsi_ble_conn_info[ble_conn_id].search_serv_conn[0],
                                           NULL);
        if (status != RSI_SUCCESS) {
          //! check for procedure already in progress error
          if (status == (uint16_t)RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
            rsi_ble_conn_info[ble_conn_id].prof_resp_recvd = false;

#if RSI_DEBUG_EN
            LOG_PRINT_D("\r\n rsi_ble_get_profile_async procedure is already in progress -conn%d \r\n", ble_conn_id);
#endif
          }
          //! check for buffer full error, which is not expected for this procedure
          else if (status == (uint16_t)RSI_ERROR_BLE_DEV_BUF_FULL) {
            LOG_PRINT("\r\n rsi_ble_get_profiles_async failed with buffer full error -conn%d \r\n", ble_conn_id);
          } else {
            LOG_PRINT("\r\n get profile async call failed with error code :%x -conn%d \r\n", status, ble_conn_id);
          }
        } else {
          rsi_ble_conn_info[ble_conn_id].profs_evt_cnt++;
          rsi_ble_conn_info[ble_conn_id].prof_cnt++;
        }
      } else {
      }
    }
  }
}

/*==============================================*/
/**
 * @fn          void rsi_ble_prepare_write_resp(uint16_t status, void *event_data)
 * @brief       handler for prepare_write_resp to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for prepare_write_resp to be executed in ble task context
 *
 */
void rsi_ble_prepare_write_resp(uint16_t status, void *event_data)
{

  uint8_t ble_conn_id;
  rsi_ble_event_prepare_write_t *rsi_app_ble_prepared_write_event = (rsi_ble_event_prepare_write_t *)event_data;
  //! convert to ascii

  LOG_PRINT_D("\r\n in preapre write resp event\r\n");
  // This statement is added only to resolve compilation warning  : [-Wunused-parameter] , value is unchanged
  UNUSED_PARAMETER(status);
#if RSI_DEBUG_EN
  LOG_PRINT_D("\r\n in rsi_ble_on_gatt_prepare_write_event \r\n");
#endif
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_app_ble_prepared_write_event->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event,
         rsi_app_ble_prepared_write_event,
         sizeof(rsi_ble_event_prepare_write_t));

  LOG_PRINT("\nPWE\n");
  uint8_t err = 0;
  if (*(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.handle == rsi_ble_att1_val_hndl) {
    rsi_ble_att_list_t *attribute = NULL;
    uint8_t opcode                = 0x16;
    attribute =
      rsi_gatt_get_attribute_from_list(&att_list,
                                       *(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.handle);

    //! Check if value has write properties
    if ((attribute != NULL) && (attribute->value != NULL)) {
      if (!(attribute->char_val_prop & 0x08)) //! If no write property, send error response
      {
        err = 0x03; //! Error - Write not permitted
      }
    } else {
      //! Error = No such handle exists
      err = 0x01;
    }

    if (err) {
      //! Send error response
      rsi_ble_att_error_response(rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.dev_addr,
                                 *(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.handle,
                                 opcode,
                                 err);
    }

    //! Update the value based6 on the offset and length of the value
    if ((err == 0)
        && ((*(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.offset)
            <= attribute->max_value_len)) {
      // LOG_PRINT("PWE - offset : %d\n",(*(uint16_t *)app_ble_prepared_write_event.offset));
      //! Hold the value to update it
      memcpy(&rsi_ble_conn_info[ble_conn_id]
                .temp_prepare_write_value[rsi_ble_conn_info[ble_conn_id].temp_prepare_write_value_len],
             rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.att_value,
             rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.length);
      rsi_ble_conn_info[ble_conn_id].temp_prepare_write_value_len +=
        rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.length;
    } else {
      //! Error : 0x07 - Invalid offset,  0x0D - Invalid attribute value length
      rsi_ble_conn_info[ble_conn_id].prep_write_err = 0x07;
    }
    //! Send gatt write response
    rsi_ble_gatt_prepare_write_response(
      rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.dev_addr,
      *(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.handle,
      (*(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.offset),
      rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.length,
      rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.att_value);
  }
}

/*==============================================*/
/**
 * @fn          void rsi_ble_set_att_resp(uint16_t status, void *event_data)
 * @brief       handler for set_att_resp to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for set_att_resp to be executed in ble task context
 *
 */
void rsi_ble_set_att_resp(uint16_t status, void *event_data)
{
  uint8_t ble_conn_id;
  rsi_ble_set_att_resp_t *rsi_ble_event_set_att_rs = (rsi_ble_set_att_resp_t *)event_data;
  //! convert to ascii

  LOG_PRINT_D("\r\n in write resp event\r\n");

  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_event_set_att_rs->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_ble_write_resp_event,
         rsi_ble_event_set_att_rs,
         sizeof(rsi_ble_set_att_resp_t));

  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n write event response failed , error : 0x%x\r\n", status);
  } else {
    //! set conn specific event
    LOG_PRINT_D("\r\n in write resp event succesfulr\n");
    if (ble_confgs.ble_conn_configuration[ble_conn_id].tx_write) {
      rsi_ble_on_data_transmit(ble_conn_id);

#if RSI_DEBUG_EN
      LOG_PRINT("\r\n write response received -conn%d\n", ble_conn_id);
#endif
    }
  }
}
/*==============================================*/
/**
 * @fn          void rsi_ble_event_indication_confirmation(uint16_t status, void *event_data)
 * @brief       handler for event_advertisement_restart to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_advertisement_restart to be executed in ble task context
 *
 */
void rsi_ble_event_indication_confirmation(uint16_t __attribute__((unused)) status, void *event_data)
{
  LOG_PRINT_D("\r\n rsi_ble_event_indication_confirmation   \n");

  uint8_t ble_conn_id;
  rsi_ble_set_att_resp_t *rsi_ble_event_set_att_rsp = (rsi_ble_set_att_resp_t *)event_data;
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_event_set_att_rsp->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif
  //! set conn specific event
  if (ble_confgs.ble_conn_configuration[ble_conn_id].tx_indications) {
    rsi_ble_on_data_transmit(ble_conn_id);
#if RSI_DEBUG_EN
    LOG_PRINT("\r\nIn rsi_ble_on_event_indication_confirmation event\n");
#endif
  }
}
/*==============================================*/
/**
 * @fn          void rsi_ble_on_event_write_resp(uint16_t status, void *event_data)
 * @brief       Handler for the BLE write response event to be executed in the BLE task context.
 * @param[in]   uint16_t status   The status of the write response (e.g., success or error code).
 * @param[in]   void *event_data  Pointer to the data associated with the BLE write response event.
 * @return      None
 *
 * @section description
 * This function is called when a BLE write response event occurs. It is executed in the BLE task context.
 * The function processes the event, which includes checking the status and using the event data to take
 * appropriate actions. The function does not return any value.
 *
 */
void rsi_ble_on_event_write_resp(uint16_t status, void *event_data)
{
  uint8_t ble_conn_id;
  rsi_ble_set_att_resp_t *rsi_ble_set_att_resp = (rsi_ble_set_att_resp_t *)event_data;
  //! convert to ascii

  // This statement is added only to resolve compilation warning  : [-Wunused-parameter] , value is unchanged
  UNUSED_PARAMETER(status);

  LOG_PRINT_D("\r\n in write response event \r\n");

  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_set_att_resp->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  if (ble_confgs.ble_conn_configuration[ble_conn_id].tx_write) {
    rsi_ble_on_data_transmit(ble_conn_id);
#if RSI_DEBUG_EN
    LOG_PRINT("\r\nIn rsi_ble_on_event_write_resp event\n");
#endif
  }
}
/*==============================================*/
/**
 * @fn          void rsi_ble_event_mtu_exchange_information(uint16_t status, void *event_data)
 * @brief       handler for event_mtu_exchange_information to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_mtu_exchange_information to be executed in ble task context
 *
 */
void rsi_ble_event_mtu_exchange_information(uint16_t status, void *event_data)
{
  uint8_t ble_conn_id;
  rsi_ble_event_mtu_exchange_information_t *rsi_ble_event_mtu_exchange_info =
    (rsi_ble_event_mtu_exchange_information_t *)event_data;
  LOG_PRINT_D("\r\n Received MTU EXCHANGE Information Event in main task\r\n");
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_event_mtu_exchange_info->dev_addr);
#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif
  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].mtu_exchange_info,
         rsi_ble_event_mtu_exchange_info,
         sizeof(rsi_ble_event_mtu_exchange_information_t));
  LOG_PRINT("Remote Device Address : %s\n", remote_dev_addr_conn);
  LOG_PRINT("RemoteMTU : %d \r\n", rsi_ble_conn_info[ble_conn_id].mtu_exchange_info.remote_mtu_size);
  LOG_PRINT("LocalMTU : %d\r\n", rsi_ble_conn_info[ble_conn_id].mtu_exchange_info.local_mtu_size);
  LOG_PRINT("Initated Role : 0x%x \r\n", rsi_ble_conn_info[ble_conn_id].mtu_exchange_info.initiated_role);
  //! set conn specific event

  LOG_PRINT("\r\n MTU EXCHANGE INFORMATION - in subtask -conn%d \r\n", ble_conn_id);
  if ((rsi_ble_conn_info[ble_conn_id].mtu_exchange_info.initiated_role == PEER_DEVICE_INITATED_MTU_EXCHANGE)
      && (RSI_BLE_MTU_EXCHANGE_FROM_HOST)) {
    status = rsi_ble_mtu_exchange_resp(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr, LOCAL_MTU_SIZE);
    //! check for procedure already in progress error
    if (status == (uint16_t)RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {

      LOG_PRINT("\r\n rsi_ble_mtu_exchange_resp procedure is already in progress -conn%d \r\n", ble_conn_id);
    }
    if (status != RSI_SUCCESS) {
      LOG_PRINT("MTU EXCHANGE RESP Failed status : 0x%x \n", status);
    } else {
      LOG_PRINT("MTU EXCHANGE RESP SUCCESS status : 0x%x \n", status);
    }
  }
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_mtu(uint16_t status, void *event_data)
 * @brief       handler for event_mtu to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_mtu to be executed in ble task context
 *
 */
void rsi_ble_event_mtu(uint16_t status, void *event_data)
{
  uint8_t ble_conn_id;
  rsi_ble_event_mtu_t *rsi_ble_mtu = (rsi_ble_event_mtu_t *)event_data;
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_mtu->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].app_ble_mtu_event, rsi_ble_mtu, sizeof(rsi_ble_event_mtu_t));

  rsi_6byte_dev_address_to_ascii(rsi_ble_conn_info[ble_conn_id].str_remote_address,
                                 rsi_ble_conn_info[ble_conn_id].app_ble_mtu_event.dev_addr);
  LOG_PRINT("\r\n MTU size from remote device(%s), %d - conn%d\r\n",
            rsi_ble_conn_info[ble_conn_id].str_remote_address,
            rsi_ble_conn_info[ble_conn_id].app_ble_mtu_event.mtu_size,
            ble_conn_id);

  rsi_ble_conn_info[ble_conn_id].mtu_exchange_done = 1;
  if ((rsi_ble_conn_info[ble_conn_id].mtu_exchange_done)
      && (ble_confgs.ble_conn_configuration[ble_conn_id].smp_enable)) {
    if (!smp_in_progress) {
      status = rsi_ble_smp_pair_request(rsi_ble_conn_info[ble_conn_id].rsi_enhc_conn_status.dev_addr,
                                        RSI_BLE_SMP_IO_CAPABILITY,
                                        MITM_ENABLE);
      rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr);

      LOG_PRINT("\n ADRESS given to SMP: %s \n", remote_dev_addr_conn);
      if (status != RSI_SUCCESS) {
        LOG_PRINT("\r\n RSI_BLE_SMP_REQ_EVENT: failed to initiate the SMP pairing process: 0x%x \r\n -conn%d",
                  status,
                  ble_conn_id);
      } else {
        smp_in_progress                                     = 1;
        rsi_ble_conn_info[ble_conn_id].smp_pairing_initated = true;
        rsi_6byte_dev_address_to_ascii(rsi_ble_conn_info[ble_conn_id].str_remote_address,
                                       rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr);
        LOG_PRINT("\r\n smp pairing request initiated to %s - conn%d \r\n",
                  rsi_ble_conn_info[ble_conn_id].str_remote_address,
                  ble_conn_id);
      }
    } else {
      rsi_ble_conn_info[ble_conn_id].smp_state = smp_pending;
    }
  } else {
    status = rsi_ble_get_profiles_async(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr, 1, 0xffff, NULL);
    if (status != RSI_SUCCESS) {
      //! check for procedure already in progress error
      if (status == (uint16_t)RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
        //rsi_current_state[ble_conn_id] |= BIT64(RSI_BLE_REQ_GATT_PROFILE);
        LOG_PRINT("\r\n rsi_ble_get_profiles_async procedure is already in progress -conn%d \r\n", ble_conn_id);

      }
      //! check for buffer full error, which is not expected for this procedure
      else if (status == (uint16_t)RSI_ERROR_BLE_DEV_BUF_FULL) {
        LOG_PRINT("\r\n rsi_ble_get_profiles_async failed with buffer full error -conn%d \r\n", ble_conn_id);

      } else {
        LOG_PRINT("\r\n get profile async call failed with error code :%x -conn%d \r\n", status, ble_conn_id);
      }
    }
  }
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_data_length_update(uint16_t status, void *event_data)
 * @brief       handler for event_data_length_update to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_data_length_update to be executed in ble task context
 *
 */
void rsi_ble_event_data_length_update(uint16_t __attribute__((unused)) status,
                                      void __attribute__((unused)) * event_data)
{

  LOG_PRINT_D("\r\n Received rsi_ble_event_data_length_updatet in main task\r\n");
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_le_dev_buf_ind(uint16_t status, void *event_data)
 * @brief       handler for event_le_dev_buf_ind to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_le_dev_buf_ind to be executed in ble task context
 *
 */
void rsi_ble_event_le_dev_buf_ind(uint16_t __attribute__((unused)) status, void *event_data)
{
#if (RSI_DEBUG_EN)
  uint8_t ble_conn_id;
#endif
  rsi_ble_event_le_dev_buf_ind_t *rsi_ble_more_data_evt = (rsi_ble_event_le_dev_buf_ind_t *)event_data;
#if RSI_DEBUG_EN
  LOG_PRINT_D("\r\n Received more data event in main task\r\n");
#endif
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_more_data_evt->remote_dev_bd_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
#if RSI_DEBUG_EN
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  rsi_get_ble_conn_id(remote_dev_addr_conn);
#endif
#else
#if RSI_DEBUG_EN
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#else
  rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif
#endif
#if RSI_DEBUG_EN
  LOG_PRINT_D("\r\n more data request -conn%d \r\n", ble_conn_id);
#endif
#if 0
        if (rsi_current_state[ble_conn_id] & BIT64(RSI_DATA_TRANSMIT_EVENT)) {
          rsi_current_state[ble_conn_id] &= ~BIT64(RSI_DATA_TRANSMIT_EVENT);
          rsi_ble_set_event_based_on_conn(ble_conn_id, RSI_DATA_TRANSMIT_EVENT);
        }
        if (rsi_current_state[ble_conn_id] & BIT64(RSI_BLE_REQ_GATT_PROFILE)) {
          rsi_current_state[ble_conn_id] &= ~BIT64(RSI_BLE_REQ_GATT_PROFILE);
          rsi_ble_set_event_based_on_conn(ble_conn_id, RSI_BLE_REQ_GATT_PROFILE);
        }
        if (rsi_current_state[ble_conn_id] & BIT64(RSI_BLE_GATT_PROFILES)) {
          rsi_current_state[ble_conn_id] &= ~BIT64(RSI_BLE_GATT_PROFILES);
          rsi_ble_set_event_based_on_conn(ble_conn_id, RSI_BLE_GATT_PROFILES);
        }
        if (rsi_current_state[ble_conn_id] & BIT64(RSI_BLE_GATT_PROFILE)) {
          rsi_current_state[ble_conn_id] &= ~BIT64(RSI_BLE_GATT_PROFILE);
          rsi_ble_set_event_based_on_conn(ble_conn_id, RSI_BLE_GATT_PROFILE);
        }
        if (rsi_current_state[ble_conn_id] & BIT64(RSI_BLE_GATT_CHAR_SERVICES)) {
          rsi_current_state[ble_conn_id] &= ~BIT64(RSI_BLE_GATT_CHAR_SERVICES);
          rsi_ble_set_event_based_on_conn(ble_conn_id, RSI_BLE_GATT_CHAR_SERVICES);
        }
        if (rsi_current_state[ble_conn_id] & BIT64(RSI_BLE_READ_REQ_EVENT)) {
          rsi_current_state[ble_conn_id] &= ~BIT64(RSI_BLE_READ_REQ_EVENT);
          rsi_ble_set_event_based_on_conn(ble_conn_id, RSI_BLE_READ_REQ_EVENT);
        }
        if (rsi_current_state[ble_conn_id] & BIT64(RSI_BLE_BUFF_CONF_EVENT)) {
          rsi_current_state[ble_conn_id] &= ~BIT64(RSI_BLE_BUFF_CONF_EVENT);
          rsi_ble_set_event_based_on_conn(ble_conn_id, RSI_BLE_BUFF_CONF_EVENT);
        }
#endif
}

/*==============================================*/
/**
 * @fn          void rsi_ble_simple_peripheral_on_remote_features_event(uint16_t status, void *event_data)
 * @brief       handler for event_remote_features to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_remote_features to be executed in ble task context
 *
 */
void rsi_ble_simple_peripheral_on_remote_features_event(uint16_t status, void *event_data)
{
  uint8_t ble_conn_id;
  rsi_ble_event_remote_features_t *rsi_ble_event_remote_features = (rsi_ble_event_remote_features_t *)event_data;
  LOG_PRINT_D("rsi_ble_simple_peripheral_on_remote_features_event");
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_event_remote_features->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].remote_dev_feature,
         rsi_ble_event_remote_features,
         sizeof(rsi_ble_event_remote_features_t));

  if (ble_confgs.ble_conn_configuration[ble_conn_id].smp_enable) {
    if (rsi_ble_conn_info[ble_conn_id].mtu_exchange_done) {
      if ((!rsi_ble_conn_info[ble_conn_id].smp_pairing_initated)
          && (!rsi_ble_conn_info[ble_conn_id].smp_pairing_request_received)) {
        //! initiating the SMP pairing process
        status = rsi_ble_smp_pair_request(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
                                          RSI_BLE_SMP_IO_CAPABILITY,
                                          MITM_ENABLE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\n start of SMP pairing process failed with error code %x -conn%d \r\n", status, ble_conn_id);
        } else {
          rsi_ble_conn_info[ble_conn_id].smp_pairing_initated = true;
          rsi_6byte_dev_address_to_ascii(rsi_ble_conn_info[ble_conn_id].str_remote_address,
                                         rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr);
          LOG_PRINT("\r\n smp pairing request initiated to %s - conn%d \r\n",
                    rsi_ble_conn_info[ble_conn_id].str_remote_address,
                    ble_conn_id);
        }
      }
    }
  }
}

/*==============================================*/
/**
 * @fn          void rsi_ble_on_remote_conn_params_request_event(uint16_t status, void *event_data)
 * @brief       handler for event_remote_conn_param_req to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_remote_conn_param_req to be executed in ble task context
 *
 */
void rsi_ble_on_remote_conn_params_request_event(uint16_t status, void *event_data)
{
  uint8_t ble_conn_id;
  rsi_ble_event_remote_conn_param_req_t *remote_conn_param = (rsi_ble_event_remote_conn_param_req_t *)event_data;
  UNUSED_PARAMETER(status);
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, remote_conn_param->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_app_remote_device_conn_params,
         remote_conn_param,
         sizeof(rsi_ble_event_remote_conn_param_req_t));
  status = rsi_ble_conn_param_resp(rsi_ble_conn_info[ble_conn_id].rsi_app_remote_device_conn_params.dev_addr, 0);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("conn param resp status: 0x%X\r\n", status);
  }
}
/**
 * @fn          void rsi_ble_event_remote_conn_param_req(uint16_t status, void *event_data)
 * @brief       handler for event_remote_conn_param_req to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_remote_conn_param_req to be executed in ble task context
 *
 */
void rsi_ble_on_execute_write_event(uint16_t status, void *event_data)
{
  uint8_t ble_conn_id;
  rsi_ble_execute_write_t *rsi_app_ble_execute_write_event = (rsi_ble_execute_write_t *)event_data;
  UNUSED_PARAMETER(status);

  LOG_PRINT_D("\r\n in rsi_ble_on_execute_write_event \r\n");

  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_app_ble_execute_write_event->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].app_ble_execute_write_event,
         rsi_app_ble_execute_write_event,
         sizeof(rsi_ble_execute_write_t));

  LOG_PRINT("\nEWE\n");
  if (*(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.handle == rsi_ble_att1_val_hndl) {
    rsi_ble_att_list_t *attribute = NULL;
    uint8_t opcode = 0x18, err = 0x00;
    attribute =
      rsi_gatt_get_attribute_from_list(&att_list,
                                       *(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.handle);

    //! Check if value has write properties
    if ((attribute != NULL) && (attribute->value != NULL)) {
      if (!(attribute->char_val_prop & 0x08)) //! If no write property, send error response
      {
        err = 0x03; //! Error - Write not permitted
      }
    } else {
      //! Error = No such handle exists
      err = 0x01;
    }

    //! Update the value based on the offset and length of the value
    if ((!err) && (rsi_ble_conn_info[ble_conn_id].app_ble_execute_write_event.exeflag == 0x1)
        && (rsi_ble_conn_info[ble_conn_id].temp_prepare_write_value_len <= attribute->max_value_len)
        && !rsi_ble_conn_info[ble_conn_id].prep_write_err) {
      //! Hold the value to update it
      memcpy((uint8_t *)attribute->value,
             rsi_ble_conn_info[ble_conn_id].temp_prepare_write_value,
             rsi_ble_conn_info[ble_conn_id].temp_prepare_write_value_len);
      attribute->value_len = rsi_ble_conn_info[ble_conn_id].temp_prepare_write_value_len;

      //! Send gatt write response
      rsi_ble_gatt_write_response(rsi_ble_conn_info[ble_conn_id].app_ble_execute_write_event.dev_addr, 1);
    } else {
      err = 0x0D; // Invalid attribute value length
    }
    if (rsi_ble_conn_info[ble_conn_id].prep_write_err) {
      //! Send error response
      rsi_ble_att_error_response(rsi_ble_conn_info[ble_conn_id].app_ble_execute_write_event.dev_addr,
                                 *(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.handle,
                                 opcode,
                                 rsi_ble_conn_info[ble_conn_id].prep_write_err);
    } else if (err) {
      //! Send error response
      rsi_ble_att_error_response(rsi_ble_conn_info[ble_conn_id].app_ble_execute_write_event.dev_addr,
                                 *(uint16_t *)rsi_ble_conn_info[ble_conn_id].app_ble_prepared_write_event.handle,
                                 opcode,
                                 err);
    }
    rsi_ble_conn_info[ble_conn_id].prep_write_err = 0;
    err                                           = 0;
    memset(rsi_ble_conn_info[ble_conn_id].temp_prepare_write_value,
           0,
           rsi_ble_conn_info[ble_conn_id].temp_prepare_write_value_len);
    rsi_ble_conn_info[ble_conn_id].temp_prepare_write_value_len = 0;
  }
}
/**
 * @fn          void rsi_ble_read_req(uint16_t status, void *event_data)
 * @brief       handler for event_remote_conn_param_req to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_remote_conn_param_req to be executed in ble task context
 *
 */
void rsi_ble_read_req(uint16_t status, void *event_data)
{
  uint8_t ble_conn_id;
  rsi_ble_read_req_t *rsi_ble_read_req = (rsi_ble_read_req_t *)event_data;
  // This statement is added only to resolve compilation warning  : [-Wunused-parameter] , value is unchanged
  UNUSED_PARAMETER(status);
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_read_req->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].app_ble_read_event, rsi_ble_read_req, sizeof(rsi_ble_read_req_t));
  LOG_PRINT("\r\n Read request initiated by remote device -conn%d \n", ble_conn_id);

  rsi_ble_conn_info[ble_conn_id].type   = rsi_ble_conn_info[ble_conn_id].app_ble_read_event.type;
  rsi_ble_conn_info[ble_conn_id].handle = rsi_ble_conn_info[ble_conn_id].app_ble_read_event.handle;
  rsi_ble_conn_info[ble_conn_id].offset = rsi_ble_conn_info[ble_conn_id].app_ble_read_event.offset;

  if (rsi_ble_conn_info[ble_conn_id].type == 1) {
    status = rsi_ble_gatt_read_response(
      rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
      1,
      rsi_ble_conn_info[ble_conn_id].handle,
      rsi_ble_conn_info[ble_conn_id].offset,
      (sizeof(rsi_ble_conn_info[ble_conn_id].read_data1) - rsi_ble_conn_info[ble_conn_id].offset),
      &(rsi_ble_conn_info[ble_conn_id].read_data1[rsi_ble_conn_info[ble_conn_id].offset]));
    rsi_ble_conn_info[ble_conn_id].offset = 0;
  } else {

    status = rsi_ble_gatt_read_response(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
                                        0,
                                        rsi_ble_conn_info[ble_conn_id].handle,
                                        0,
                                        (sizeof(rsi_ble_conn_info[ble_conn_id].read_data1)),
                                        rsi_ble_conn_info[ble_conn_id].read_data1);
  }
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n read response failed, error:0x%x -conn%d \r\n", status, ble_conn_id);
  } else {
    LOG_PRINT("\r\n response to read request initiated by remote device was successfull -conn%d \n", ble_conn_id);
  }
}
/**
 * @fn          void rsi_ble_profile_event(uint16_t status, void *event_data)
 * @brief       handler for event_remote_conn_param_req to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_remote_conn_param_req to be executed in ble task context
 *
 */
void rsi_ble_profile_event(uint16_t status, void *event_data)
{

  uint8_t ble_conn_id;
  rsi_ble_event_profile_by_uuid_t *rsi_ble_event_profile = (rsi_ble_event_profile_by_uuid_t *)event_data;
  // This statement is added only to resolve compilation warning  : [-Wunused-parameter] , value is unchanged
  UNUSED_PARAMETER(status);
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_event_profile->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].get_profile, rsi_ble_event_profile, sizeof(rsi_ble_event_profile_by_uuid_t));

#if RSI_DEBUG_EN
  LOG_PRINT_D("\r\n in gatt profile -conn%d\n", ble_conn_id);
#endif
  if (!rsi_ble_conn_info[ble_conn_id].profile_mem_init) {
    if (rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_info_uuid == NULL) {
      rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_info_uuid =
        (rsi_ble_event_profile_by_uuid_t *)malloc(sizeof(rsi_ble_event_profile_by_uuid_t)
                                                  * rsi_ble_conn_info[ble_conn_id].total_remote_profiles);
    }
    if (rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_info_uuid == NULL) {
      LOG_PRINT("failed to allocate memory for "
                "rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn[%d].profile_info_uuid \r\n",
                ble_conn_id);
      return;
    }
    memset(rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_info_uuid,
           0,
           sizeof(rsi_ble_event_profile_by_uuid_t) * rsi_ble_conn_info[ble_conn_id].total_remote_profiles);
    rsi_ble_conn_info[ble_conn_id].profile_mem_init = true;
  }
  // copy total searched profiles in local buffer
  if (rsi_ble_conn_info[ble_conn_id].prof_evt_cnt < rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {

    //! copy to central buffer
    memcpy(&rsi_ble_conn_info[ble_conn_id]
              .rsi_ble_profile_list_by_conn.profile_info_uuid[rsi_ble_conn_info[ble_conn_id].prof_evt_cnt],
           &rsi_ble_conn_info[ble_conn_id].get_profile,
           sizeof(rsi_ble_event_profile_by_uuid_t));
    rsi_ble_conn_info[ble_conn_id].prof_resp_recvd = false;
#if RSI_DEBUG_EN
    LOG_PRINT_D("\r\n Gatt profile:\nStart handle: 0x%x  \nEnd handle:0x%x -conn%d \r\n",
                *(uint16_t *)rsi_ble_conn_info[ble_conn_id]
                   .rsi_ble_profile_list_by_conn.profile_info_uuid[rsi_ble_conn_info[ble_conn_id].prof_evt_cnt]
                   .start_handle,
                *(uint16_t *)rsi_ble_conn_info[ble_conn_id]
                   .rsi_ble_profile_list_by_conn.profile_info_uuid[rsi_ble_conn_info[ble_conn_id].prof_evt_cnt]
                   .end_handle,
                ble_conn_id);

#endif
    rsi_ble_conn_info[ble_conn_id].prof_evt_cnt++;
    LOG_PRINT_D("\n rsi_ble_conn_info[ble_conn_id].prof_evt_cnt value = %d ",
                rsi_ble_conn_info[ble_conn_id].prof_evt_cnt);
    if (rsi_ble_conn_info[ble_conn_id].prof_cnt < rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {
      status = rsi_ble_get_profile_async(
        rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
        rsi_ble_conn_info[ble_conn_id].search_serv_conn[rsi_ble_conn_info[ble_conn_id].prof_cnt],
        NULL);
      rsi_ble_conn_info[ble_conn_id].prof_cnt++;
    }

    else if (rsi_ble_conn_info[ble_conn_id].prof_cnt == rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {
#if RSI_DEBUG_EN
      LOG_PRINT_D("\r\n completed gatt profile -conn%d\n", ble_conn_id);
#endif
      if (!rsi_ble_conn_info[ble_conn_id].char_resp_recvd) {
        if (rsi_ble_conn_info[ble_conn_id].profile_index_for_char_query
            < rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {
          rsi_ble_conn_info[ble_conn_id].char_resp_recvd = true;
#if RSI_DEBUG_EN
          LOG_PRINT_D(
            "\r\n search for profile characteristics :0x%x -conn%d \r\n",
            rsi_ble_conn_info[ble_conn_id]
              .rsi_ble_profile_list_by_conn.profile_desc[rsi_ble_conn_info[ble_conn_id].profile_index_for_char_query]
              .profile_uuid.val.val16,
            ble_conn_id);
#endif
          uint16_t start_handle = *(uint16_t *)rsi_ble_conn_info[ble_conn_id]
                                     .rsi_ble_profile_list_by_conn
                                     .profile_info_uuid[rsi_ble_conn_info[ble_conn_id].profile_index_for_char_query]
                                     .start_handle;
          uint16_t end_handle = *(uint16_t *)rsi_ble_conn_info[ble_conn_id]
                                   .rsi_ble_profile_list_by_conn
                                   .profile_info_uuid[rsi_ble_conn_info[ble_conn_id].profile_index_for_char_query]
                                   .end_handle;

          //! Get characteristic services of searched profile
          status = rsi_ble_get_char_services_async(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
                                                   start_handle,
                                                   end_handle,
                                                   NULL);
          rsi_ble_conn_info[ble_conn_id].char_cnt++;
          if (status != RSI_SUCCESS) {
            //! check for procedure already in progress error
            if (status == (uint16_t)RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
              rsi_ble_conn_info[ble_conn_id].char_resp_recvd = false;

#if RSI_DEBUG_EN
              LOG_PRINT_D("\r\n rsi_ble_get_char_services_async procedure is already in progress -conn%d \r\n",
                          ble_conn_id);
#endif
            }
            //! check for buffer full error, which is not expected for this procedure
            else if (status == (uint16_t)RSI_ERROR_BLE_DEV_BUF_FULL) {
              LOG_PRINT("\r\n rsi_ble_get_char_services_async failed with buffer full error -conn%d \r\n", ble_conn_id);
            } else {
              LOG_PRINT(
                "\r\n failed to get service characteristics of the remote GATT server with error:0x%x -conn%d \r\n",
                status,
                ble_conn_id);
            }
          }
          rsi_ble_conn_info[ble_conn_id].profile_index_for_char_query++;
        } else {
          //! discovery of complete characteristics in each profile is completed
        }
      }
    } else {
      LOG_PRINT("\n ERROR CONDITION \n");
    }
  }
}

/**
 * @fn          void rsi_ble_char_services_event(uint16_t status, void *event_data)
 * @brief       handler for event_remote_conn_param_req to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_remote_conn_param_req to be executed in ble task context
 *
 */
void rsi_ble_char_services_event(uint16_t status, void *event_data)
{

  uint8_t ble_conn_id;
  rsi_ble_event_read_by_type1_t *rsi_ble_event_char_services = (rsi_ble_event_read_by_type1_t *)event_data;
  // This statement is added only to resolve compilation warning  : [-Wunused-parameter] , value is unchanged
  UNUSED_PARAMETER(status);
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_event_char_services->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].get_char_services,
         rsi_ble_event_char_services,
         sizeof(rsi_ble_event_read_by_type1_t));
#if RSI_DEBUG_EN
  LOG_PRINT_D("\r\n in gatt test:RSI_BLE_GATT_CHAR_SERVICES -conn%d \r\n", ble_conn_id);
#endif
  if (!rsi_ble_conn_info[ble_conn_id].service_char_mem_init) {
    if (rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_char_info == NULL) {
      rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_char_info =
        (rsi_ble_event_read_by_type1_t *)malloc(sizeof(rsi_ble_event_read_by_type1_t)
                                                * rsi_ble_conn_info[ble_conn_id].total_remote_profiles);
    }
    if (rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_char_info == NULL) {
      LOG_PRINT("failed to allocate memory for "
                "rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn[%d].profile_char_info \r\n",
                ble_conn_id);
      return;
    }
    memset(rsi_ble_conn_info[ble_conn_id].rsi_ble_profile_list_by_conn.profile_char_info,
           0,
           sizeof(rsi_ble_event_read_by_type1_t) * rsi_ble_conn_info[ble_conn_id].total_remote_profiles);
    rsi_ble_conn_info[ble_conn_id].service_char_mem_init = true;
  }
  if (rsi_ble_conn_info[ble_conn_id].char_for_serv_cnt < rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {

    memcpy(&rsi_ble_conn_info[ble_conn_id]
              .rsi_ble_profile_list_by_conn.profile_char_info[rsi_ble_conn_info[ble_conn_id].char_for_serv_cnt],
           &rsi_ble_conn_info[ble_conn_id].get_char_services,
           sizeof(rsi_ble_event_read_by_type1_t));
    LOG_PRINT_D("\n attribute handle from characteristic discovery = %d \n ",
                rsi_ble_conn_info[ble_conn_id]
                  .get_char_services.char_services[rsi_ble_conn_info[ble_conn_id].char_for_serv_cnt]
                  .handle);

    rsi_ble_conn_info[ble_conn_id].char_for_serv_cnt++;
    rsi_ble_conn_info[ble_conn_id].char_resp_recvd = false;
    if (rsi_ble_conn_info[ble_conn_id].char_cnt < rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {
      uint16_t start_handle =
        *(uint16_t *)rsi_ble_conn_info[ble_conn_id]
           .rsi_ble_profile_list_by_conn.profile_info_uuid[rsi_ble_conn_info[ble_conn_id].char_cnt]
           .start_handle;
      uint16_t end_handle = *(uint16_t *)rsi_ble_conn_info[ble_conn_id]
                               .rsi_ble_profile_list_by_conn.profile_info_uuid[rsi_ble_conn_info[ble_conn_id].char_cnt]
                               .end_handle;
      status = rsi_ble_get_char_services_async(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr,
                                               start_handle,
                                               end_handle,
                                               NULL);
      rsi_ble_conn_info[ble_conn_id].char_cnt++;
    } else if (rsi_ble_conn_info[ble_conn_id].char_cnt == rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {
      LOG_PRINT_D("BLE CONN ID FOR CALLBACK: %d", *&ble_conn_id);
      rsi_ble_event_profile_discovery_driver_callback(&ble_conn_id);
    }
  }
}

/**
 * @fn          void rsi_ble_event_remote_conn_param_req(uint16_t status, void *event_data)
 * @brief       handler for event_remote_conn_param_req to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_remote_conn_param_req to be executed in ble task context
 *
 */
void rsi_ble_on_read_resp_event(uint16_t eventid, void *event_data)
{
  uint8_t ble_conn_id;

  rsi_ble_event_att_value_t *rsi_ble_event_att_val = (rsi_ble_event_att_value_t *)event_data;
  // This statement is added only to resolve compilation warning  : [-Wunused-parameter] , value is unchanged
  UNUSED_PARAMETER(eventid);

  LOG_PRINT_D("\r\n in Gatt descriptor response event \r\n");

  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_event_att_val->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_char_descriptors,
         rsi_ble_event_att_val,
         sizeof(rsi_ble_event_att_value_t));

  LOG_PRINT_D("\n Remote dev length = %x \n", rsi_ble_event_att_val->length);
  LOG_PRINT("\n Remote dev addr in read resp = %s \n", remote_dev_addr_conn);

  //! set conn specific event
  //! signal conn specific task

  if (rsi_ble_conn_info[ble_conn_id].temp1 < rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {
    rsi_ble_conn_info[ble_conn_id].temp2++;
    rsi_ble_conn_info[ble_conn_id].char_desc_cnt++;
    rsi_ble_conn_info[ble_conn_id].char_desc_resp_recvd = false;
    rsi_ble_event_profile_discovery_driver_callback(&ble_conn_id);
  } else if (rsi_ble_conn_info[ble_conn_id].temp1 == rsi_ble_conn_info[ble_conn_id].total_remote_profiles) {
    LOG_PRINT("\r\n remote device profiles discovery completed -conn%d \n", ble_conn_id);
    //! if data transfer is configured
    if (ble_confgs.ble_conn_configuration[ble_conn_id].data_transfer) {
      LOG_PRINT("\r\n data transfer req done \r\n");
      rsi_ble_event_select_data_transfer_driver_callback(&ble_conn_id);
    }
  }
}

/*==============================================*/
/**
 * @fn          void rsi_bt_event_smp_req(uint16_t status, void *event_data)
 * @brief       handler for event_smp_req to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_smp_req to be executed in ble task context
 *
 */
void rsi_bt_event_smp_req(uint16_t status, void *event_data)
{
  uint8_t ble_conn_id;
  rsi_bt_event_smp_req_t *remote_smp = (rsi_bt_event_smp_req_t *)event_data;
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, remote_smp->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_ble_event_smp_req, remote_smp, sizeof(rsi_bt_event_smp_req_t));
  LOG_PRINT("\r\n in smp request \r\n -conn%d \r\n", ble_conn_id);
  rsi_ble_conn_info[ble_conn_id].conn_state = on_connect_state;
  if (ble_confgs.ble_conn_configuration[ble_conn_id].smp_enable) {
    rsi_ble_conn_info[ble_conn_id].smp_pairing_request_received = true;

    if (rsi_ble_conn_info[ble_conn_id].first_connect == 0) {
      if (!rsi_ble_conn_info[ble_conn_id].smp_pairing_initated) {
        //! initiating the SMP pairing process
        status = rsi_ble_smp_pair_request(rsi_ble_conn_info[ble_conn_id].rsi_ble_event_smp_req.dev_addr,
                                          RSI_BLE_SMP_IO_CAPABILITY,
                                          MITM_ENABLE);
        if (status != RSI_SUCCESS) {
          LOG_PRINT("\r\n RSI_BLE_SMP_REQ_EVENT: failed to initiate the SMP pairing process: 0x%x \r\n -conn%d",
                    status,
                    ble_conn_id);
        } else {
          rsi_ble_conn_info[ble_conn_id].smp_pairing_initated = true;
          // rsi_6byte_dev_address_to_ascii(rsi_ble_conn_info[ble_conn_id].str_remote_address, rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr);
          LOG_PRINT("\r\n smp pairing request initiated to %s - conn%d \r\n",
                    rsi_ble_conn_info[ble_conn_id].str_remote_address,
                    ble_conn_id);
        }
      }
    } else {
      status = rsi_ble_start_encryption(rsi_ble_conn_info[ble_conn_id].rsi_ble_event_smp_req.dev_addr,
                                        rsi_ble_conn_info[ble_conn_id].rsi_encryption_enabled.localediv,
                                        rsi_ble_conn_info[ble_conn_id].rsi_encryption_enabled.localrand,
                                        rsi_ble_conn_info[ble_conn_id].rsi_encryption_enabled.localltk);
    }
  }
}

/*==============================================*/
/**
 * @fn          void rsi_bt_event_smp_resp(uint16_t status, void *event_data)
 * @brief       handler for event_smp_resp to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_smp_resp to be executed in ble task context
 *
 */
void rsi_bt_event_smp_resp(uint16_t __attribute__((unused)) status, void *event_data)
{
  uint8_t ble_conn_id;
  rsi_bt_event_smp_resp_t *remote_smp = (rsi_bt_event_smp_resp_t *)event_data;

  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, remote_smp->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif
  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_ble_event_smp_resp, remote_smp, sizeof(rsi_bt_event_smp_resp_t));
  rsi_ble_conn_info[ble_conn_id].conn_state = on_connect_state;

  LOG_PRINT("\r\n in smp response -conn%d \r\n", ble_conn_id);
  if (ble_confgs.ble_conn_configuration[ble_conn_id].smp_enable) {
    //! initiating the SMP pairing process
#if RSI_DEBUG_EN
    status = rsi_ble_smp_pair_response(rsi_ble_conn_info[ble_conn_id].rsi_ble_event_smp_resp.dev_addr,
                                       RSI_BLE_SMP_IO_CAPABILITY,
                                       MITM_ENABLE);
#else
    rsi_ble_smp_pair_response(rsi_ble_conn_info[ble_conn_id].rsi_ble_event_smp_resp.dev_addr,
                              RSI_BLE_SMP_IO_CAPABILITY,
                              MITM_ENABLE);
#endif
  }
#if RSI_DEBUG_EN
  LOG_PRINT_D("Status = %x", status);
#endif
}

/*==============================================*/
/**
 * @fn          void rsi_bt_event_smp_passkey(uint16_t status, void *event_data)
 * @brief       handler for event_smp_passkey to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_smp_passkey to be executed in ble task context
 *
 */
void rsi_bt_event_smp_passkey(uint16_t __attribute__((unused)) status, void *event_data)
{
  uint8_t ble_conn_id;
  rsi_bt_event_smp_passkey_t *smp_pass_key = (rsi_bt_event_smp_passkey_t *)event_data;
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, smp_pass_key->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_ble_event_smp_passkey, smp_pass_key, sizeof(rsi_bt_event_smp_passkey_t));

  LOG_PRINT("\r\n in smp_passkey - rsi_ble_conn_info[ble_conn_id].str_remote_address : %s\r\n",
            rsi_ble_conn_info[ble_conn_id].remote_dev_addr);
  rsi_ble_conn_info[ble_conn_id].conn_state = on_connect_state;

  //! initiating the SMP pairing process
#if RSI_DEBUG_EN
  status =
    rsi_ble_smp_passkey(rsi_ble_conn_info[ble_conn_id].rsi_ble_event_smp_passkey.dev_addr, RSI_BLE_APP_SMP_PASSKEY);
#else
  rsi_ble_smp_passkey(rsi_ble_conn_info[ble_conn_id].rsi_ble_event_smp_passkey.dev_addr, RSI_BLE_APP_SMP_PASSKEY);
#endif
#if RSI_DEBUG_EN
  LOG_PRINT_D("Status = %x", status);
#endif
}

/*==============================================*/
/**
 * @fn          void rsi_bt_event_smp_failed(uint16_t status, void *event_data)
 * @brief       handler for event_smp_failed to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_smp_failed to be executed in ble task context
 *
 */
void rsi_bt_event_smp_failed(uint16_t status, void *event_data)
{
  uint8_t ble_conn_id;
  rsi_bt_event_smp_failed_t *remote_dev_address = (rsi_bt_event_smp_failed_t *)event_data;
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, remote_dev_address->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id              = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif
  smp_in_progress = 0;

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_ble_smp_failed, remote_dev_address, sizeof(rsi_bt_event_smp_failed_t));
  rsi_ble_conn_info[ble_conn_id].conn_state = on_connect_state;

  LOG_PRINT("\r\n SMP failed for remote address : %s with status : %x", remote_dev_addr_conn, status);
  LOG_PRINT("\r\n in smp failed remote address: %s -conn%d \r\n",
            rsi_ble_conn_info[ble_conn_id].remote_dev_addr,
            ble_conn_id);

  LOG_PRINT("\r\n Initiating a disconnect as the security failed \r\n ");
  status = rsi_ble_disconnect((int8_t *)rsi_ble_conn_info[ble_conn_id].rsi_ble_smp_failed.dev_addr);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\ndisconnect command failed with reason %x\n", status);
  }
}

/*==============================================*/
/**
 * @fn          void rsi_bt_event_encryption_enabled(uint16_t status, void *event_data)
 * @brief       handler for event_encryption_enabled to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_encryption_enabled to be executed in ble task context
 *
 */
void rsi_bt_event_encryption_enabled(uint16_t status, void *event_data)
{

  uint8_t ble_conn_id;
  uint8_t remote_device_role                     = 0;
  rsi_bt_event_encryption_enabled_t *enc_enabled = (rsi_bt_event_encryption_enabled_t *)event_data;
  // This statement is added only to resolve compilation warning  : [-Wunused-parameter] , value is unchanged
  UNUSED_PARAMETER(status);
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, enc_enabled->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id              = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  smp_in_progress = 0;

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_encryption_enabled,
         enc_enabled,
         sizeof(rsi_bt_event_encryption_enabled_t));
  memcpy(&rsi_ble_conn_info[ble_conn_id].l_rsi_encryption_enabled,
         &rsi_ble_conn_info[ble_conn_id].rsi_encryption_enabled,
         sizeof(rsi_bt_event_encryption_enabled_t));
  rsi_ble_conn_info[ble_conn_id].smp_pairing_initated = false;
  rsi_ble_conn_info[ble_conn_id].smp_state            = smp_process_done;
  uint8_t ix;

  memcpy(&glbl_enc_enabled, enc_enabled, sizeof(rsi_bt_event_encryption_enabled_t));
  LOG_PRINT("\n ************************** \n");
  LOG_PRINT("\n LE LTK \n");

  for (ix = 0; ix < 16; ix++) {
    LOG_PRINT("%x ", enc_enabled->localltk[ix]);
  }

  LOG_PRINT("\n ************************** \n");
  status = add_device_to_ltk_key_list(ble_dev_ltk_list, &glbl_enc_enabled);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n Failed to add LTK to the device");
  }
  rsi_ble_conn_info[ble_conn_id].conn_state = on_connect_state;
  LOG_PRINT("\r\n in smp encrypt event -conn%d \r\n", ble_conn_id);

  remote_device_role = rsi_get_remote_device_role(remote_dev_addr_conn);
  if (remote_device_role == PERIPHERAL_ROLE) {
    if (ble_confgs.ble_conn_configuration[ble_conn_id].smp_enable) {
      rsi_ble_event_scan_restart_driver_callback();
    }
  }

  LOG_PRINT_D("\n Triggering SMP pending callback\n");

  rsi_ble_event_smp_pending_driver_callback(&ble_conn_id);

  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr);

  LOG_PRINT_D("\n address sent from encryption to rpfoile disc RSI CONNECTEED ADDRESS : %s \n", remote_dev_addr_conn);
  if (rsi_ble_conn_info[ble_conn_id].smp_state == smp_process_done) {
    status = rsi_ble_get_profiles_async(rsi_ble_conn_info[ble_conn_id].rsi_connected_dev_addr, 1, 0xffff, NULL);
    if (status != RSI_SUCCESS) {
      //! check for procedure already in progress error
      if (status == (uint16_t)RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {

        LOG_PRINT("\r\n rsi_ble_get_profiles_async procedure is already in progress -conn%d \r\n", ble_conn_id);

      }
      //! check for buffer full error, which is not expected for this procedure
      else if (status == (uint16_t)RSI_ERROR_BLE_DEV_BUF_FULL) {
        LOG_PRINT("\r\n rsi_ble_get_profiles_async failed with buffer full error -conn%d \r\n", ble_conn_id);

      } else {
        LOG_PRINT("\r\n get profile async call failed with error code :%x -conn%d \r\n", status, ble_conn_id);
      }
    }
  }
}

/*==============================================*/
/**
 * @fn          void rsi_bt_event_smp_passkey_display(uint16_t status, void *event_data)
 * @brief       handler for event_smp_passkey_display to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_smp_passkey_display to be executed in ble task context
 *
 */
void rsi_bt_event_smp_passkey_display(uint16_t __attribute__((unused)) status, void *event_data)
{

  uint8_t ble_conn_id;
  rsi_bt_event_smp_passkey_display_t *smp_passkey_display = (rsi_bt_event_smp_passkey_display_t *)event_data;
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, smp_passkey_display->dev_addr);

  //! get conn_id
#if (CONNECT_OPTION != CONN_BY_NAME)
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id              = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif
  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_ble_smp_passkey_display,
         smp_passkey_display,
         sizeof(rsi_bt_event_smp_passkey_display_t));
  LOG_PRINT("\r\nremote addr: %s, passkey: %s \r\n",
            rsi_ble_conn_info[ble_conn_id].remote_dev_addr,
            rsi_ble_conn_info[ble_conn_id].rsi_ble_smp_passkey_display.passkey);
  rsi_ble_conn_info[ble_conn_id].conn_state = on_connect_state;
}

/*==============================================*/
/**
 * @fn          void rsi_bt_event_sc_passkey(uint16_t status, void *event_data)
 * @brief       handler for event_sc_passkey to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_sc_passkey to be executed in ble task context
 *
 */
void rsi_bt_event_sc_passkey(uint16_t __attribute__((unused)) status, void *event_data)
{

  uint8_t ble_conn_id;
  rsi_bt_event_sc_passkey_t *sc_passkey = (rsi_bt_event_sc_passkey_t *)event_data;
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, sc_passkey->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id              = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif
  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_event_sc_passkey, sc_passkey, sizeof(rsi_bt_event_sc_passkey_t));
  LOG_PRINT("\r\n in smp sc passkey event -conn%d \r\n", ble_conn_id);

  LOG_PRINT("\r\n In passkey event, remote addr: %s, passkey: %lu -conn%u \r\n",
            rsi_ble_conn_info[ble_conn_id].remote_dev_addr,
            rsi_ble_conn_info[ble_conn_id].rsi_event_sc_passkey.passkey,
            ble_conn_id);

  rsi_ble_smp_passkey(rsi_ble_conn_info[ble_conn_id].rsi_event_sc_passkey.dev_addr,
                      rsi_ble_conn_info[ble_conn_id].rsi_event_sc_passkey.passkey);
  rsi_ble_conn_info[ble_conn_id].conn_state = on_connect_state;
}

/*==============================================*/
/**
 * @fn          void rsi_bt_event_le_ltk_request(uint16_t status, void *event_data)
 * @brief       handler for event_le_ltk_request to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_le_ltk_request to be executed in ble task context
 *
 */
void rsi_bt_event_le_ltk_request(uint16_t status, void *event_data)
{
  uint8_t ble_conn_id;

  rsi_bt_event_le_ltk_request_t *le_ltk_req = (rsi_bt_event_le_ltk_request_t *)event_data;
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, le_ltk_req->dev_addr);

#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id              = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_le_ltk_resp, le_ltk_req, sizeof(rsi_bt_event_le_ltk_request_t));
  rsi_ble_conn_info[ble_conn_id].conn_state = on_connect_state;
  LOG_PRINT("\r\n in LTK  request -conn%d \r\n", ble_conn_id);

  if (0) {
    LOG_PRINT("\r\n positive reply\n");
    //! give le ltk req reply cmd with positive reply
    status = rsi_ble_ltk_req_reply(rsi_ble_conn_info[ble_conn_id].rsi_le_ltk_resp.dev_addr,
                                   1,
                                   rsi_ble_conn_info[ble_conn_id].l_rsi_encryption_enabled.localltk);
    if (status != RSI_SUCCESS) {
      LOG_PRINT("\r\n failed to restart smp pairing with status: 0x%x -conn%d\r\n", status, ble_conn_id);
    }
  } else {
    LOG_PRINT("\r\n negative reply\n");
    //! give le ltk req reply cmd with negative reply
    status = rsi_ble_ltk_req_reply(rsi_ble_conn_info[ble_conn_id].rsi_le_ltk_resp.dev_addr, 0, NULL);
    if (status != RSI_SUCCESS) {
      LOG_PRINT("\r\n failed to restart smp pairing with status: 0x%x \r\n", status);
    }
    rsi_ble_conn_info[ble_conn_id].neg_rply = 1;
  }
}

/*==============================================*/
/**
 * @fn          void rsi_bt_event_le_security_keys(uint16_t status, void *event_data)
 * @brief       handler for event_le_security_keys to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_le_security_keys to be executed in ble task context
 *
 */
void rsi_bt_event_le_security_keys(uint16_t status, void *event_data)
{

  uint8_t ble_conn_id;
  rsi_bt_event_le_security_keys_t *le_sec_keys = (rsi_bt_event_le_security_keys_t *)event_data;
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, le_sec_keys->dev_addr);
#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id              = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif
  rsi_ble_conn_info[ble_conn_id].conn_state = on_connect_state;

  //! copy to conn specific buffer
  memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_le_security_keys, le_sec_keys, sizeof(rsi_bt_event_le_security_keys_t));
  LOG_PRINT("\r\n in smp security keys event  -conn%d \r\n", ble_conn_id);
  rsi_ble_conn_info[ble_conn_id].conn_state = connected_state;
  memcpy(&temp_le_sec_keys, le_sec_keys, sizeof(rsi_bt_event_le_security_keys_t));
  rsi_ble_conn_info[ble_conn_id].smp_done = 1;

  status = add_security_keys_to_device_list(ble_dev_ltk_list, temp_le_sec_keys);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n Failed to add Security keys to list \n");
  }
}

/*==============================================*/
/**
 * @fn          void rsi_ble_on_ctkd(uint16_t status, void *event_data)
 * @brief       handler for event_ctkd to be executed in ble task context
 * @param[in]   uint16_t , event_status 
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for event_ctkd to be executed in ble task context
 *
 */
void rsi_ble_on_ctkd(uint16_t status, void *event_data)
{
  //! Add handling here
  uint8_t ble_conn_id;
  rsi_ble_event_ctkd_t *ctkd = (rsi_ble_event_ctkd_t *)event_data;
  uint8_t ix;
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, ctkd->dev_addr);
#if (CONNECT_OPTION != CONN_BY_NAME)
  //! get conn_id
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn);
#else
  ble_conn_id              = rsi_get_ble_conn_id(remote_dev_addr_conn, NULL, 0);
#endif
  LOG_PRINT(" \n Received ctkd event and its status =  %x \n", status);
  memcpy(&ble_ctkd, ctkd, sizeof(rsi_ble_event_ctkd_t));
#if 1
  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(rsi_ble_conn_info[ble_conn_id].str_remote_address, ctkd->dev_addr);
  LOG_PRINT("\n CTKD remote dev addr = %s \n", rsi_ble_conn_info[ble_conn_id].str_remote_address);
#endif
  LOG_PRINT("\n Derived LinkKey from CTKD :\n");
  for (ix = 0; ix < 16; ix++) {
    LOG_PRINT("%x ", ctkd->key[ix]);
  }

  status = add_derived_key_to_ltk_list(ble_dev_ltk_list, &ble_ctkd);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\n Failed to add_derived_key_to_ltk_list");
  }
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_ext_adv_report(uint16_t status, void *event_data)
 * @brief       handler for ae_adv_report to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for ae_adv_report to be executed in ble task context
 *
 */
void rsi_ble_event_ext_adv_report(uint16_t status, void *event_data)
{
  uint8_t ble_conn_id;
  rsi_ble_ae_adv_report_t *ae_adv_report                        = (rsi_ble_ae_adv_report_t *)event_data;
  uint8_t remote_dev_addr_conn[18]                              = { 0 };
  rsi_ble_ae_extended_create_connect_t ble_extended_create_conn = { 0 };
  rsi_ble_ae_set_scan_enable_t ae_set_scan_enable               = { 0 };
  uint8_t ix;

  //! convert to ascii
  rsi_6byte_dev_address_to_ascii(remote_dev_addr_conn, ae_adv_report->remote_addr);
  memset(remote_name, 0, sizeof(remote_name));
  BT_LE_ADPacketExtract(remote_name, ae_adv_report->data, ae_adv_report->data_len);

  //! get conn_id
#if (CONNECT_OPTION == CONN_BY_NAME)
  ble_conn_id = rsi_get_ble_conn_id(remote_dev_addr_conn, remote_name, strlen((const char *)remote_name));
#else
  ble_conn_id              = rsi_get_ble_conn_id(remote_dev_addr_conn);
#endif

  memcpy(&rsi_ble_conn_info[ble_conn_id].rsi_app_ae_adv_reports_to_app, ae_adv_report, sizeof(rsi_ble_ae_adv_report_t));

  rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[ble_conn_id].conn_id = ble_conn_id;
  ble_confgs.ble_conn_configuration[ble_conn_id] = rsi_parsed_conf.rsi_ble_config.rsi_ble_conn_config[ble_conn_id];
  LOG_PRINT_D("\nBLE conn ID: %d\n", ble_conn_id);

  LOG_PRINT_D("\r\n Connect command - conn%d \r\n", ble_conn_id);

  ae_set_scan_enable.enable            = RSI_BLE_STOP_SCAN;
  ae_set_scan_enable.filter_duplicates = BLE_AE_SCAN_ENABLE_FILTER_DUP;
  ae_set_scan_enable.duration          = BLE_AE_SCAN_DUR;
  ae_set_scan_enable.period            = BLE_AE_SCAN_PERIOD;
  status                               = rsi_ble_ae_set_scan_enable(&ae_set_scan_enable);
  if (status != RSI_SUCCESS) {
    LOG_PRINT(" \n set ae scan disable failed with 0x%x \n", status);
  } else {
    scan_state_dut = scan_off;
    LOG_PRINT(" \n set ae scan disable success \n");
  }

  ble_extended_create_conn.own_addr_type    = LE_PUBLIC_ADDRESS;
  ble_extended_create_conn.remote_addr_type = ae_adv_report->remote_addr_type;
  memcpy(ble_extended_create_conn.remote_addr, (int8_t *)ae_adv_report->remote_addr, RSI_DEV_ADDR_LEN);
  ble_extended_create_conn.initiator_filter_policy = ALLOW_SCAN_REQ_ANY_CONN_REQ_ANY;
  ble_extended_create_conn.init_phys               = INITIATING_PHYS;

  for (ix = 0; ix < 3; ix++) {
    ble_extended_create_conn.init_params[ix].ScanInterval    = PRI_PHY_LE_AE_SCAN_INTERVAL;
    ble_extended_create_conn.init_params[ix].ScanWindow      = PRI_PHY_LE_AE_SCAN_WINDOW;
    ble_extended_create_conn.init_params[ix].ConnIntervalMin = CONNECTION_INTERVAL_MIN;
    ble_extended_create_conn.init_params[ix].ConnIntervalMax = CONNECTION_INTERVAL_MAX;
    ble_extended_create_conn.init_params[ix].ConnLatency     = CONNECTION_LATENCY;
    ble_extended_create_conn.init_params[ix].ConnSTO         = SUPERVISION_TIMEOUT;
    ble_extended_create_conn.init_params[ix].MinCELen        = CONNECTION_EVENT_LEN_MIN;
    ble_extended_create_conn.init_params[ix].MaxCELen        = CONNECTION_EVENT_LEN_MAX;
  }

  LOG_PRINT("\r\n Initiating extended connect command \n");
  status = rsi_ble_extended_connect_with_params(&ble_extended_create_conn);

  LOG_PRINT("\r\n connecting to device :  %s -conn%d \n",
            (int8_t *)remote_dev_addr_conn /*rsi_ble_conn_info[ble_conn_id].remote_dev_addr*/,
            ble_conn_id);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("\r\n Connecting failed with status : 0x%x -conn%d\n", status, ble_conn_id);

    rsi_ble_event_scan_restart_driver_callback();
  } else {

    adv_pkt_processing_pending = 0;
    peripheral_con_req_pending = 1;

    osSemaphoreAcquire(ble_wait_on_connect, 10000);

    if (peripheral_con_req_pending) {
      LOG_PRINT("\r\n Initiating connect cancel command in -conn%d \n", ble_conn_id);
      status =
        rsi_ble_connect_cancel((int8_t *)rsi_ble_conn_info[ble_conn_id].rsi_app_ae_adv_reports_to_app.remote_addr);
      if (status != RSI_SUCCESS) {
        LOG_PRINT("\r\n ble connect cancel cmd status = %x \n", status);
      } else {

        peripheral_count++;
        rsi_ble_event_disconnect(status,
                                 (int8_t *)rsi_ble_conn_info[ble_conn_id].rsi_app_ae_adv_reports_to_app.remote_addr);
      }
      peripheral_con_req_pending = 0;
    }
  }
}

/**
 * @fn          void rsi_ble_prepare_filtered_adv_report(rsi_ble_event_adv_report_t *rsi_ble_event_adv_report)
 * @brief       function enqueues filtered advertising reports event data in driver context to ble_generic_cb.event_queues to be processed in ble task context
 * @param[in]   rsi_ble_event_adv_report_t *rsi_ble_event_adv_report, event_data
 * @param[out]  None
 * @return      None
 *
 * @section description
 * This function enqueues the filtered advertising reports event data received in driver context to ble_generic_cb.event_queues to be processed in ble task context
 *
 */
void rsi_ble_prepare_filtered_ae_adv_report(rsi_ble_ae_adv_report_t *rsi_ble_ae_adv_report)
{
  LOG_PRINT("in rsi_ble_prepare_filtered_ae_adv_report");
  generic_event_message_t *msg;

  //! allocate message
  msg = malloc(sizeof(generic_event_message_t) + sizeof(rsi_ble_ae_adv_report_t));
  if (msg == NULL) {
    LOG_PRINT("Out of Memory assert//assert 58\n");
    //assert("Out Of Memory\n");
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

void rsi_ble_ae_adv_report_driver_callback(uint16_t __attribute__((unused)) status,
                                           rsi_ble_ae_adv_report_t *rsi_ble_ae_adv_report)
{
  uint8_t device_already_connected = 0;
  LOG_PRINT_D("\n On Event Adv Report \n");
  rsi_ble_ae_adv_report_t *ae_adv_report = (rsi_ble_ae_adv_report_t *)rsi_ble_ae_adv_report;

  //! Check weather the received address type is connectable or not
  if (ae_adv_report->event_type == 0x02) {
    return; // return for NON CONN ADV PACKETS
  }

  //! Check any on-going connection process is pending
  if (adv_pkt_processing_pending) {
    return;
  }

  //! Check no of connected devices
  if (peripheral_count >= RSI_BLE_MAX_NBR_PERIPHERALS) {
    LOG_PRINT_D("\n reached the max num_of_connected_peripheral_devices \n");
    return;
  }

  rsi_6byte_dev_address_to_ascii(remote_dev_addr, (uint8_t *)ae_adv_report->remote_addr);

#if (CONNECT_OPTION == CONN_BY_NAME)
  memset(remote_name, 0, sizeof(remote_name));
  BT_LE_ADPacketExtract(remote_name, ae_adv_report->data, ae_adv_report->data_len);
  device_already_connected = rsi_check_dev_list_driver(remote_name, remote_dev_addr);
  LOG_PRINT_D("\n device_already_connected value: %d \n", device_already_connected);
#else
  device_already_connected = rsi_check_dev_list_driver(NULL, remote_dev_addr);
  LOG_PRINT_D("\n device_already_connected value: %d \n", device_already_connected);
#endif

  if (device_already_connected == PERIPHERAL_NOT_CONNECTED) {
    LOG_PRINT("\r\n ae advertised details remote_name = %s, dev_addr = %s \r\n",
              remote_name,
              (int8_t *)remote_dev_addr);

    LOG_PRINT("\n rsi_ble_prepare_filtered_ae_adv_report\n");

    //! prepare the packet and send it to app task for the connection procedure
    rsi_ble_prepare_filtered_ae_adv_report(ae_adv_report);

    adv_pkt_processing_pending = 1;
  }
  return;
}

/*==============================================*/
/**
 * @fn          void rsi_ble_on_terminate(uint16_t status, void *event_data)
 * @brief       handler for adv_set_terminated to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for adv_set_terminated to be executed in ble task context
 *
 */
void rsi_ble_on_terminate(uint16_t __attribute__((unused)) status, void __attribute__((unused)) * event_data)
{
  //! Add handling here
  LOG_PRINT("\n Terminate event received \n");
#if 0
  //re triggering ae advertiser enable for set 1
  rsi_ble_ae_adv_enable_t ble_ae_adv = { 0 };

  ble_ae_adv.enable        = RSI_BLE_START_ADV;
  ble_ae_adv.no_of_sets    = 1;
  ble_ae_adv.adv_handle    = terminated_set->adv_handle;
  ble_ae_adv.duration      = 0;
  ble_ae_adv.max_ae_events = 0x00;

  status = rsi_ble_start_ae_advertising(&ble_ae_adv);
  if (status != RSI_SUCCESS) {
    LOG_PRINT("set ae adv enable failed with 0x%lX \n", status);
  } else {
    LOG_PRINT("set ae adv enable success \n");
  }
#endif
}

/*==============================================*/
/**
 * @fn          void rsi_ble_event_extended_scan_req_received(uint16_t status, void *event_data)
 * @brief       handler for scan_req_recvd to be executed in ble task context
 * @param[in]   uint16_t , event_status
 * @param[in]   void, event_data
 * @return      None
 *
 * @section description
 * handler for scan_req_recvd to be executed in ble task context
 *
 */
void rsi_ble_event_extended_scan_req_received(uint16_t status, void *event_data)
{
  UNUSED_PARAMETER(status);
  uint8_t remote_dev_str_addr[18] = { 0 };

  rsi_ble_scan_req_recvd_t *scan_req_recvd = (rsi_ble_scan_req_recvd_t *)event_data;

  LOG_PRINT("\n ExScnReq \n");
  LOG_PRINT_D("\n status = %x \n", status);
  LOG_PRINT_D("\n adv_handle = %x \n", scan_req_recvd->adv_handle);
  LOG_PRINT_D("\n scanner_addr_type = %x \n", scan_req_recvd->scanner_addr_type);
  rsi_6byte_dev_address_to_ascii(remote_dev_str_addr, scan_req_recvd->scanner_addr);
  LOG_PRINT_D("\n adv_handle = %s \n", remote_dev_str_addr);

  LOG_PRINT("\n***************************\n");
}

void rsi_ble_event_smp_pending(uint16_t status, void *event_data)
{
  uint8_t *conn_id             = (uint8_t *)event_data;
  uint8_t ble_conn_id          = *conn_id;
  uint8_t BD_ADDR[BD_ADDR_LEN] = { 0 };
  for (i = 0; i < TOTAL_CONNECTIONS; i++) {
    if ((rsi_ble_conn_info[i].mtu_state) && (ble_confgs.ble_conn_configuration[ble_conn_id].smp_enable)
        && (!smp_in_progress)) {
      if (!rsi_ble_conn_info[ble_conn_id].smp_pairing_initated) {
        if (rsi_ble_conn_info[i].smp_state == smp_pending) {
          memcpy(BD_ADDR, rsi_ble_conn_info[i].rsi_enhc_conn_status.dev_addr, BD_ADDR_LEN);
          status = rsi_ble_smp_pair_request(BD_ADDR, RSI_BLE_SMP_IO_CAPABILITY, MITM_ENABLE);
          if (status != RSI_SUCCESS) {
            LOG_PRINT("\r\n RSI_BLE_SMP_REQ_EVENT: failed to initiate the SMP pairing process: 0x%x \r\n -conn%d",
                      status,
                      ble_conn_id);
          } else {
            smp_in_progress                                     = 1;
            rsi_ble_conn_info[ble_conn_id].smp_pairing_initated = true;
            rsi_ble_conn_info[i].smp_state                      = smp_triggered;
            rsi_6byte_dev_address_to_ascii(rsi_ble_conn_info[ble_conn_id].str_remote_address, BD_ADDR);
            LOG_PRINT("\r\n smp pairing request initiated to %s - conn%d \r\n",
                      rsi_ble_conn_info[ble_conn_id].str_remote_address,
                      ble_conn_id);
            break;
          }
        }
      }
    }
  }
}
