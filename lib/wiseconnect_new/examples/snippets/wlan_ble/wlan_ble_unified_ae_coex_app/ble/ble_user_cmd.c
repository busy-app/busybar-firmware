/*******************************************************************************
 * @file  ble_user_cmd.c
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
/**
 * @file    ble_user_cmd.c
 * @version 0.1
 * @date    24 Jan 2023
 * @author yajain
 *  @brief : This file contains command handlers for BLE.
 *
 *  @section Description  This file handles the BLE commands and responses to that commands.*
 *
 */

/*=======================================================================*/
//   ! INCLUDES
/*=======================================================================*/
#include <stdio.h>
#include <rsi_common_app.h>
#include "ble_cmd_hdlr_auto_gen.h"
#include "event_loop.h"
#include "ble_private.h"
#include "ble_config.h"
#include "rsi_common_apis.h"
/*=======================================================================*/
//   ! function protoypes
/*=======================================================================*/

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

void ble_module_req_adv_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here

  printf("\n start adv command called \n");
  int32_t status;

  if (adv_state_dut == adv_disabled) {

    if (central_count == RSI_BLE_MAX_NBR_CENTRALS) {

      status = rsi_ble_start_advertising_with_values(&change_adv_param);
      if (status != RSI_SUCCESS) {
        printf("\r\n ble_module_req_adv_cmd_handler : error status 0x%lx \n", status);
      } else {
        adv_state_dut = adv_enabled_non_connectable;
        printf("\r\n ble_module_req_adv_cmd_handler : successful \n");
      }
    } else if (central_count < RSI_BLE_MAX_NBR_CENTRALS) {
      status = rsi_ble_start_advertising();
      if (status != RSI_SUCCESS) {
        printf("\r\n ble_module_req_adv_cmd_handler : error status 0x%lx \n", status);
      } else {
        adv_state_dut = adv_enabled;
        printf("\r\n ble_module_req_adv_cmd_handler : successful \n");
      }
    }
  } else if (adv_state_dut == adv_enabled) {
    printf("\r\n ble_connectable_adv_already_in_progress \n");
  } else if (adv_state_dut == adv_enabled_non_connectable) {
    printf("\r\n ble_non_connectable_adv_already_in_progress \n");
  }

  generic_command_message->response_status = 0;
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

void ble_module_req_scan_cmd_handler(generic_command_message_t *generic_command_message)
{

  printf("\n start scan command called \n");
  int32_t status;

  if (scan_state_dut == scan_off) {

    if (peripheral_count == RSI_BLE_MAX_NBR_PERIPHERALS) {

      status = rsi_ble_start_scanning_with_values(&change_scan_param);
      if (status != RSI_SUCCESS) {
        printf("\r\n ble_module_req_scan_cmd_handler : error status 0x%lx \n", status);
      } else {
        scan_state_dut = non_connectable_scan;
        printf("\r\n ble_module_req_scan_cmd_handler : successful \n");
      }
    } else if (peripheral_count < RSI_BLE_MAX_NBR_PERIPHERALS) {
      status = rsi_ble_start_scanning();
      if (status != RSI_SUCCESS) {
        printf("\r\n ble_module_req_scan_cmd_handler : error status 0x%lx \n", status);
      } else {
        scan_state_dut = connectable_scan;
        printf("\r\n ble_module_req_scan_cmd_handler : successful \n");
      }
    }
  } else if (scan_state_dut == connectable_scan) {
    printf("\r\n ble_connectable_scan_already_in_progress \n");
  } else if (scan_state_dut == non_connectable_scan) {
    printf("\r\n ble_non_connectable_scan_already_in_progress \n");
  }

  generic_command_message->response_status = 0;
}
/*==============================================*/
/**
 * @fn          void ble_module_req_adv_stop_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_adv to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_adv to be executed in ble task context
 *
 */

void ble_module_req_adv_stop_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here

  printf("\n stop adv command called \n");
  int32_t status;
  if (adv_state_dut == adv_disabled) {
    printf("\n adv  already not in progress   \n");
  } else {
    status = rsi_ble_stop_advertising();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_adv_stop_cmd_handler : error status 0x%lx \n", status);
    } else {
      adv_state_dut = adv_disabled;
      printf("\r\n ble_module_req_adv_cmd_stop_handler : successful \n");
    }
  }

  generic_command_message->response_status = 0;
}

/*==============================================*/
/**
 * @fn          void ble_module_req_scan_stop_cmd_handler(generic_command_message_t *generic_command_message)
 * @brief       handler for req_scan to be executed in ble task context
 * @param[in]   generic_command_message_t, generic_command_message 
 * @param[out]  None
 * @return      None
 *
 * @section description
 * handler for req_scan to be executed in ble task context
 *
 */

void ble_module_req_scan_stop_cmd_handler(generic_command_message_t *generic_command_message)
{

  printf("\n stop scan command called \n");
  int32_t status;
  if (scan_state_dut == scan_off) {
    printf("\n scan  already not in progress   \n");
  } else {
    status = rsi_ble_stop_scanning();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_scan_stop_cmd_handler : error status 0x%lx \n", status);
    } else {
      scan_state_dut = scan_off;
      printf("\r\n ble_module_req_scan_cmd_stop_handler : successful \n");
    }
  }

  generic_command_message->response_status = 0;
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

void ble_module_req_conn_cmd_handler(generic_command_message_t *generic_command_message)
{
  uint8_t dev_addr[BD_ADDR_LEN] = { 0 };

  //! Add handling here
  int32_t status;
  uint8_t dev_addr_type = ascii_to_dec(generic_command_message->argv[1]);
  LOG_PRINT_D("BLE connect addr type print: %d \r\n", dev_addr_type);

  rsi_ascii_dev_address_to_6bytes_rev(dev_addr, (int8_t *)generic_command_message->argv[0]);
  LOG_PRINT_D("BLE connect addr type print: %s \r\n", generic_command_message->argv[1]);
  LOG_PRINT_D("\r\n ble_module_req_conn_cmd_handler \n");
  /* generic_command_message->argv[1] is the pointer to the next string */
  status = rsi_ble_connect(dev_addr_type, (int8_t *)dev_addr);
  if (status != RSI_SUCCESS) {
    printf("\r\n ble_module_req_conn_cmd_handler : error status 0x%lx \n", status);
  } else {

    printf("\r\n ble_module_req_conn_cmd_handler : successful \n");
  }

  generic_command_message->response_status = 0;

  /* if conwithvalues 

 status = rsi_ble_connect_with_params(uint8_t remote_dev_addr_type,
                                    int8_t *remote_dev_addr,
                                    uint16_t scan_interval,
                                    uint16_t scan_window,
                                    uint16_t conn_interval_max,
                                    uint16_t conn_interval_min,
                                    uint16_t conn_latency,
                                    uint16_t supervision_tout)

    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_adv_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_adv_cmd_handler : successful \n");
    }


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

void ble_module_req_disconnect_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
  uint8_t remote_device_role    = 0;
  uint8_t dev_addr[BD_ADDR_LEN] = { 0 };

  rsi_ascii_dev_address_to_6bytes_rev(dev_addr, (int8_t *)generic_command_message->argv[0]);
  printf("BLE disconnect triggered: %s \r\n", generic_command_message->argv[0]);
  remote_device_role = rsi_get_remote_device_role(generic_command_message->argv[0]);

  int32_t status;

  status = rsi_ble_disconnect((int8_t *)dev_addr);
  if (status != RSI_SUCCESS) {
    printf("\r\n ble_module_req_disconnect_cmd_handler : error status 0x%lx \n", status);
  } else {
    if (remote_device_role == PERIPHERAL_ROLE) {
      peripheral_count--;
    } else {
      central_count--;
    }
    printf("\r\n ble_module_req_disconnect_cmd_handler : successful \n");
  }

  generic_command_message->response_status = 0;
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

void ble_module_cmd_conn_params_update_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
  int32_t status;

  uint8_t dev_addr[RSI_REM_DEV_ADDR_LEN] = { 0 };

  rsi_ascii_dev_address_to_6bytes_rev(dev_addr, (int8_t *)generic_command_message->argv[0]);
  uint16_t min_int = rsi_ascii_hex2num(*generic_command_message->argv[1]);
  uint16_t max_int = rsi_ascii_hex2num(*generic_command_message->argv[2]);
  uint16_t latency = rsi_ascii_hex2num(*generic_command_message->argv[3]);
  uint16_t timeout = rsi_ascii_hex2num(*generic_command_message->argv[4]);

  status = rsi_ble_conn_params_update(dev_addr, min_int, max_int, latency, timeout);
  if (status != RSI_SUCCESS) {
    printf("\r\n ble_module_cmd_conn_params_update_cmd_handler : error status 0x%lx \n", status);
  } else {

    printf("\r\n ble_module_cmd_conn_params_update_cmd_handler : successful \n");
  }

  generic_command_message->response_status = 0;
}

#if 0
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

void ble_module_get_dev_state_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_get_device_state(uint8_t *resp);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_dev_state_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_get_dev_state_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cmd_conn_params_update_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_conn_params_update(uint8_t *remote_dev_address,
                                   uint16_t min_int,
                                   uint16_t max_int,
                                   uint16_t latency,
                                   uint16_t timeout);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_conn_params_update_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_conn_params_update_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_start_encryption_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    /* Nothing in the definition
    int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_start_encryption_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_start_encryption_cmd_handler : successful \n");
    }
*/
    generic_command_message->response_status = 0;

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

void ble_module_req_smp_pair_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_smp_pair_request(uint8_t *remote_dev_address, uint8_t io_capability, uint8_t mitm_req);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_smp_pair_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_smp_pair_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_smp_pair_response_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status =  rsi_ble_smp_pair_response(uint8_t *remote_dev_address, uint8_t io_capability, uint8_t mitm_req);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_smp_pair_response_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_smp_pair_response_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_smp_passkey_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_smp_passkey(uint8_t *remote_dev_address, uint32_t passkey);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_smp_passkey_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_smp_passkey_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_profiles_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status =  rsi_ble_get_profiles(uint8_t *dev_addr,
                             uint16_t start_handle,
                             uint16_t end_handle,
                             rsi_ble_resp_profiles_list_t *p_prof_list);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_profiles_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_profiles_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_profile_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_get_profile(uint8_t *dev_addr, uuid_t profile_uuid, profile_descriptors_t *p_profile);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_profile_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_profile_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_char_services_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_get_char_services(uint8_t *dev_addr,
                                  uint16_t start_handle,
                                  uint16_t end_handle,
                                  rsi_ble_resp_char_services_t *p_char_serv_list);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_char_services_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_char_services_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_inc_services_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_get_inc_services(uint8_t *dev_addr,
                                 uint16_t start_handle,
                                 uint16_t end_handle,
                                 rsi_ble_resp_inc_services_t *p_inc_serv_list);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_inc_services_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_inc_services_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_read_by_uuid_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_get_char_value_by_uuid(uint8_t *dev_addr,
                                       uint16_t start_handle,
                                       uint16_t end_handle,
                                       uuid_t char_uuid,
                                       rsi_ble_resp_att_value_t *p_char_val);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_read_by_uuid_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_read_by_uuid_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_desc_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_get_att_descriptors(uint8_t *dev_addr,
                                    uint16_t start_handle,
                                    uint16_t end_handle,
                                    rsi_ble_resp_att_descs_t *p_att_desc);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_desc_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_desc_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_read_val_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_get_att_value(uint8_t *dev_addr, uint16_t handle, rsi_ble_resp_att_value_t *p_att_val);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_read_val_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_read_val_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_multiple_read_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_get_multiple_att_values(uint8_t *dev_addr,
                                        uint8_t num_of_handlers,
                                        uint16_t *handles,
                                        rsi_ble_resp_att_value_t *p_att_vals);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_multiple_read_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_multiple_read_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_long_read_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_get_long_att_value(uint8_t *dev_addr,
                                   uint16_t handle,
                                   uint16_t offset,
                                   rsi_ble_resp_att_value_t *p_att_vals);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_long_read_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_long_read_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_write_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_set_att_value(uint8_t *dev_addr, uint16_t handle, uint8_t data_len, uint8_t *p_data);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_write_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_write_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_write_no_ack_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_set_att_cmd(uint8_t *dev_addr, uint16_t handle, uint8_t data_len, uint8_t *p_data);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_write_no_ack_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_write_no_ack_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_long_write_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_set_long_att_value(uint8_t *dev_addr,
                                   uint16_t handle,
                                   uint16_t offset,
                                   uint8_t data_len,
                                   uint8_t *p_data);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_long_write_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_long_write_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_prepare_write_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status =  rsi_ble_prepare_write(uint8_t *dev_addr, uint16_t handle, uint16_t offset, uint8_t data_len, uint8_t *p_data);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_prepare_write_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_prepare_write_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_execute_write_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_execute_write(uint8_t *dev_addr, uint8_t exe_flag);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_execute_write_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_execute_write_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_add_service_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_add_service(uuid_t service_uuid, rsi_ble_resp_add_serv_t *p_resp_serv);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_add_service_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_add_service_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_add_attribute_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_add_attribute(rsi_ble_req_add_att_t *p_attribute);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_add_attribute_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_add_attribute_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_set_local_att_value_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_set_local_att_value(uint16_t handle, uint16_t data_len, uint8_t *p_data);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_local_att_value_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_set_local_att_value_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_get_local_att_value_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_get_local_att_value(uint16_t handle, rsi_ble_resp_local_att_value_t *p_resp_local_att_val);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_local_att_value_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_get_local_att_value_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cmd_notify_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status =  rsi_ble_notify_value(uint8_t *dev_addr, uint16_t handle, uint16_t data_len, uint8_t *p_data);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_notify_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_notify_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cmd_indicate_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_indicate_value(uint8_t *dev_addr, uint16_t handle, uint16_t data_len, uint8_t *p_data);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_indicate_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_indicate_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_set_advertise_data_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_set_advertise_data(uint8_t *data, uint16_t data_len);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_advertise_data_cmd_handler : error status 0x%lx \n", status);
    } else {

      printf("\r\n ble_module_set_advertise_data_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_get_le_ping_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status =  rsi_ble_get_le_ping_timeout(uint8_t *remote_dev_address, uint16_t *time_out);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_le_ping_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_get_le_ping_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_set_le_ping_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_set_le_ping_timeout(uint8_t *remote_dev_address, uint16_t time_out);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_le_ping_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_set_le_ping_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_set_random_address_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_set_random_address();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_random_address_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_set_random_address_cmd_handler : successful \n");
    }
/* setrandomadresswithvalues

 status = rsi_ble_set_random_address_with_value(uint8_t *random_addr);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_random_address_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_set_random_address_cmd_handler : successful \n");
    }

*/
    generic_command_message->response_status = 0;

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

void ble_module_encrypt_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_encrypt(uint8_t *key, uint8_t *data, uint8_t *resp);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_encrypt_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_encrypt_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cmd_read_resp_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status =  rsi_ble_gatt_read_response(uint8_t *dev_addr,
                                   uint8_t read_type,
                                   uint16_t handle,
                                   uint16_t offset,
                                   uint16_t length,
                                   uint8_t *p_data);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_read_resp_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_read_resp_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_set_scan_response_data_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_set_scan_response_data(uint8_t *data, uint16_t data_len);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_scan_response_data_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_set_scan_response_data_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_le_accept_list_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_clear_acceptlist();
/*
    rsi_ble_addto_acceptlist(int8_t *dev_address, uint8_t dev_addr_type)
rsi_ble_deletefrom_acceptlist(int8_t *dev_address, uint8_t dev_addr_type)
    */
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_le_accept_list_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_le_accept_list_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cmd_remove_service_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    // no definition or calls 
    int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_remove_service_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_remove_service_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cmd_remove_attribute_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    // no definition or calls 
    int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_remove_attribute_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_remove_attribute_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_process_resolv_list_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_resolvlist(uint8_t process_type,
                           uint8_t remote_dev_addr_type,
                           uint8_t *remote_dev_address,
                           uint8_t *peer_irk,
                           uint8_t *local_irk);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_process_resolv_list_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_process_resolv_list_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_get_resolving_list_size_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_get_resolving_list_size(uint8_t *resp);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_resolving_list_size_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_get_resolving_list_size_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_set_address_resolution_enable_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = crsi_ble_set_addr_resolution_enable(uint8_t enable, uint16_t tout);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_address_resolution_enable_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_set_address_resolution_enable_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_read_phy_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_readphy(int8_t *remote_dev_address, rsi_ble_resp_read_phy_t *resp);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_read_phy_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_read_phy_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_set_phy_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_setphy(int8_t *remote_dev_address, uint8_t tx_phy, uint8_t rx_phy, uint16_t coded_phy);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_set_phy_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_set_phy_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_set_data_len_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_set_data_len(uint8_t *remote_dev_address, uint16_t tx_octets, uint16_t tx_time);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_data_len_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_set_data_len_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_read_max_data_len_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_read_max_data_len(rsi_ble_read_max_data_length_t *blereaddatalen);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_read_max_data_len_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_read_max_data_len_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_set_privacy_mode_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_set_privacy_mode(uint8_t remote_dev_addr_type, uint8_t *remote_dev_address, uint8_t privacy_mode);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_privacy_mode_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_set_privacy_mode_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cbfc_conn_req_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
      // no def
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cbfc_conn_req_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cbfc_conn_req_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cbfc_conn_resp_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
      // no def
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cbfc_conn_resp_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cbfc_conn_resp_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cbfc_tx_data_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    // no def
    int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cbfc_tx_data_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cbfc_tx_data_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cbfc_disconn_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    // no def
    int32_t status;
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cbfc_disconn_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cbfc_disconn_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_le_ltk_req_reply_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_ltk_req_reply(uint8_t *remote_dev_address, uint8_t reply_type, uint8_t *ltk);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_le_ltk_req_reply_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_le_ltk_req_reply_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_rx_test_mode_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_rx_test_mode(uint8_t rx_channel, uint8_t phy, uint8_t modulation);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_rx_test_mode_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_rx_test_mode_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_tx_test_mode_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_tx_test_mode(uint8_t tx_channel, uint8_t phy, uint8_t tx_len, uint8_t mode);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_tx_test_mode_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_tx_test_mode_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_end_test_mode_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_end_test_mode(uint16_t *num_of_pkts);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_end_test_mode_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_end_test_mode_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_per_tx_mode_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
      //no def
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_per_tx_mode_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_per_tx_mode_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_per_rx_mode_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
      // nodef 
    status = call_api();
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_per_rx_mode_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_per_rx_mode_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cmd_att_error_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_att_error_response(uint8_t *dev_addr, uint16_t handle, uint8_t opcode, uint8_t err);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_att_error_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_att_error_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_profiles_async_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_get_profiles_async(uint8_t *dev_addr,
                                   uint16_t start_handle,
                                   uint16_t end_handle,
                                   rsi_ble_resp_profiles_list_t *p_prof_list);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_profiles_async_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_profiles_async_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_req_profile_async_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_get_profile_async(uint8_t *dev_addr, uuid_t profile_uuid, profile_descriptors_t *p_profile);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_req_profile_async_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_req_profile_async_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_get_charservices_async_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status =  rsi_ble_get_char_services_async(uint8_t *dev_addr,
                                        uint16_t start_handle,
                                        uint16_t end_handle,
                                        rsi_ble_resp_char_services_t *p_char_serv_list);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_charservices_async_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_get_charservices_async_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_get_includeservices_async_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_get_inc_services_async(uint8_t *dev_addr,
                                       uint16_t start_handle,
                                       uint16_t end_handle,
                                       rsi_ble_resp_inc_services_t *p_inc_serv_list);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_includeservices_async_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_get_includeservices_async_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_readcharvaluebyuuid_async_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_get_char_value_by_uuid_async(uint8_t *dev_addr,
                                             uint16_t start_handle,
                                             uint16_t end_handle,
                                             uuid_t char_uuid,
                                             rsi_ble_resp_att_value_t *p_char_val);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_readcharvaluebyuuid_async_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_readcharvaluebyuuid_async_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_get_attribute_async_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_get_att_descriptors_async(uint8_t *dev_addr,
                                          uint16_t start_handle,
                                          uint16_t end_handle,
                                          rsi_ble_resp_att_descs_t *p_att_desc);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_attribute_async_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_get_attribute_async_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_get_descriptorvalue_async_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_get_att_value_async(uint8_t *dev_addr, uint16_t handle, rsi_ble_resp_att_value_t *p_att_val);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_descriptorvalue_async_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_get_descriptorvalue_async_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_get_multiplevalues_async_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status =  rsi_ble_get_multiple_att_values_async(uint8_t *dev_addr,
                                              uint8_t num_of_handlers,
                                              uint16_t *handles,
                                              rsi_ble_resp_att_value_t *p_att_vals);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_multiplevalues_async_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_get_multiplevalues_async_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_get_longdescvalues_async_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_get_long_att_value_async(uint8_t *dev_addr,
                                         uint16_t handle,
                                         uint16_t offset,
                                         rsi_ble_resp_att_value_t *p_att_vals);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_get_longdescvalues_async_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_get_longdescvalues_async_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_set_descvalue_async_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_set_att_value_async(uint8_t *dev_addr, uint16_t handle, uint8_t data_len, uint8_t *p_data);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_descvalue_async_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_set_descvalue_async_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_set_preparewrite_async_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status =  rsi_ble_prepare_write_async(uint8_t *dev_addr,
                                    uint16_t handle,
                                    uint16_t offset,
                                    uint8_t data_len,
                                    uint8_t *p_data);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_preparewrite_async_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_set_preparewrite_async_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_execute_longdescwrite_async_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_execute_write_async(uint8_t *dev_addr, uint8_t exe_flag);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_execute_longdescwrite_async_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_execute_longdescwrite_async_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_set_smp_pairing_capability_data_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status =  rsi_ble_set_smp_pairing_cap_data(rsi_ble_set_smp_pairing_capabilty_data_t *smp_pair_cap_data);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_set_smp_pairing_capability_data_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_set_smp_pairing_capability_data_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_conn_param_resp_cmd_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_conn_param_resp(uint8_t *remote_dev_address, uint8_t status);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_conn_param_resp_cmd_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_conn_param_resp_cmd_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cmd_indicate_confirmation_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status =  rsi_ble_indicate_confirm(uint8_t *dev_addr);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_indicate_confirmation_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_indicate_confirmation_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_mtu_exchange_request_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_mtu_exchange_event(uint8_t *dev_addr, uint8_t mtu_size);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_mtu_exchange_request_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_mtu_exchange_request_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cmd_set_wwo_resp_notify_buf_info_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_set_wo_resp_notify_buf_info(uint8_t *dev_addr, uint8_t buf_mode, uint8_t buf_cnt);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_set_wwo_resp_notify_buf_info_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_set_wwo_resp_notify_buf_info_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cmd_write_resp_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_gatt_write_response(uint8_t *dev_addr, uint8_t type);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_write_resp_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_write_resp_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cmd_prepare_write_resp_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_gatt_prepare_write_response(uint8_t *dev_addr,
                                            uint16_t handle,
                                            uint16_t offset,
                                            uint16_t length,
                                            uint8_t *data);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_prepare_write_resp_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_prepare_write_resp_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cmd_set_local_irk_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_set_local_irk_value(uint8_t *l_irk);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_set_local_irk_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_set_local_irk_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cmd_set_prop_protocol_ble_bandedge_txpower_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_set_prop_protocol_ble_bandedge_tx_power(uint8_t protocol, int8_t bandedge_tx_power);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_set_prop_protocol_ble_bandedge_txpower_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_set_prop_protocol_ble_bandedge_txpower_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cmd_mtu_exchange_resp_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_mtu_exchange_resp(uint8_t *dev_addr, uint8_t mtu_size);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_mtu_exchange_resp_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_mtu_exchange_resp_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cmd_set_ble_tx_power_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_set_ble_tx_power(int8_t tx_power);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_set_ble_tx_power_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_set_ble_tx_power_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_cmd_indicate_sync_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ble_indicate_value_sync(uint8_t *dev_addr, uint16_t handle, uint16_t data_len, uint8_t *p_data);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_cmd_indicate_sync_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_cmd_indicate_sync_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_prop_protocol_cmd_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ant_send_cmd(void *ant_cmd, void *ant_cmd_resp);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_prop_protocol_cmd_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_prop_protocol_cmd_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

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

void ble_module_prop_protocol_cmd_per_cmd_handler(generic_command_message_t *generic_command_message)
{
  //! Add handling here
    int32_t status;
    status = rsi_ant_send_cmd_per(void *ant_cmd, void *ant_cmd_resp);
    if (status != RSI_SUCCESS) {
      printf("\r\n ble_module_prop_protocol_cmd_per_cmd_handler : error status 0x%lx \n", status);
    } else {
    
      printf("\r\n ble_module_prop_protocol_cmd_per_cmd_handler : successful \n");
    }

    generic_command_message->response_status = 0;

}

#endif
