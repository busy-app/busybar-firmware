/***************************************************************************/ /**
 * @file app.c
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_status.h"
#include "sl_board_configuration.h"
#include "cmsis_os2.h"
#include "errno.h"
#include "sl_wifi.h"
#include "sl_net.h"
#include "socket.h"
#include "sl_utility.h"
#include "sl_net_si91x.h"
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_driver.h"
#include <string.h>
#include <stdint.h>
#include "rsi_debug.h"
#include "sl_si91x_fw_fallback.h"

sl_wifi_device_configuration_t sl_wifi_firmware_update_configuration = {
  .boot_option         = LOAD_NWP_FW,
  .nwp_fw_image_number = SL_SI91X_NWP_FW_IMAGE_NUMBER_0,
  .mac_address         = NULL,
  .band                = SL_SI91X_WIFI_BAND_2_4GHZ,
  .boot_config         = { .oper_mode = SL_SI91X_CLIENT_MODE,
                           .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                           .feature_bit_map =
                             (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
#ifdef SLI_SI91X_MCU_INTERFACE
                      | SL_WIFI_FEAT_WPS_DISABLE
#endif
                      ),
                   .tcp_ip_feature_bit_map     = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT
                                              | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map     = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map = (SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK
                                                  | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS | MEMORY_CONFIG
#if defined(SLI_SI917) || defined(SLI_SI915)
                                                  | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                  ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP | SL_WIFI_ENABLE_ENHANCED_MAX_PSP) }
};
/**
 * @brief Structure to hold active slot information for both M4 and NWP
 */
typedef struct {
  uint8_t m4_active_slot;           ///< Active M4 slot (SLOT_A or SLOT_B)
  uint8_t nwp_active_slot;          ///< Active NWP slot (SLOT_A or SLOT_B)
  uint32_t m4_active_slot_address;  ///< Memory address of active M4 slot
  uint32_t nwp_active_slot_address; ///< Memory address of active NWP slot
} sl_si91x_active_slot_info_t;

static void application_start(void *argument);
void sl_si91x_m4_updater_app();
sl_status_t sl_si91x_app_get_active_slot_addresses(sl_si91x_active_slot_info_t *active_slot_info);
/******************************************************
  *               Variable Definitions
  ******************************************************/

const osThreadAttr_t thread_attributes = {
  .name       = "app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityLow,
  .tz_module  = 0,
  .reserved   = 0,
};

/*
  * @brief Application initialization function
  * @param unused Pointer to the unused parameter
  * @return None
  */
void app_init(const void *unused)
{
  UNUSED_PARAMETER(unused);
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}
/*
  * @brief Application start function
  * @param argument Pointer to the argument
  * @return None
  */
static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;
  sl_wifi_firmware_version_t version           = { 0 };
  sl_si91x_active_slot_info_t active_slot_info = { 0 };
  sl_si91x_fw_ab_slot_management_t app_ab_slot_info;

  // This function checks the current active NWP slot information retrieved from the slot management structure.
  // If the active slot is valid (either SLOT_A or SLOT_B), it assigns the corresponding firmware image number
  // to the Wi-Fi firmware update configuration.
  status = sl_si91x_ab_get_slot_info(&app_ab_slot_info);
  if (app_ab_slot_info.nwp_slot_info.current_active_nwp_slot != 0xFF
      && (app_ab_slot_info.nwp_slot_info.current_active_nwp_slot == SLOT_A
          || app_ab_slot_info.nwp_slot_info.current_active_nwp_slot == SLOT_B)) {
    sl_wifi_firmware_update_configuration.nwp_fw_image_number =
      app_ab_slot_info.nwp_slot_info.current_active_nwp_slot - 1;
    DEBUGOUT(" \r\n Assigned NWP Image Number from Slot Info: %d \r\n",
             sl_wifi_firmware_update_configuration.nwp_fw_image_number);
  }

  // Select the default NWP firmware image based on the current fw_image_number.
  status = sl_si91x_select_default_nwp_fw(sl_wifi_firmware_update_configuration.nwp_fw_image_number);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\n Failed to select default NWP firmware: %d\r\n", (int16_t)status);
    while (1)
      ; // Halt execution in case of failure
  }

  // Initialize the Wi-Fi client interface
  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_firmware_update_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("Failed to start Wi-Fi Client interface: 0x%lx\r\n", status);
    return;
  }

  // Get the firmware version
  status = sl_wifi_get_firmware_version(&version);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\nFailed to fetch firmware version: 0x%lx\r\n", status);
    while (1)
      ; // Halt execution in case of failure
  } else {
    print_firmware_version(&version);
  }

  // Get active slot addresses
  status = sl_si91x_app_get_active_slot_addresses(&active_slot_info);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\nFailed to get active slot addresses: 0x%lx\r\n", status);
    while (1)
      ; // Halt execution in case of failure
  } else {
    DEBUGOUT("\r\nActive M4 Slot: %s (Address: 0x%08lx)\r\n",
             (active_slot_info.m4_active_slot == SLOT_A) ? "A" : "B",
             active_slot_info.m4_active_slot_address);
    DEBUGOUT("Active NWP Slot: %s (Address: 0x%08lx)\r\n",
             (active_slot_info.nwp_active_slot == SLOT_A) ? "A" : "B",
             active_slot_info.nwp_active_slot_address);
  }

  // Verify the image for the NWP active slot
  status = sl_si91x_verify_image(active_slot_info.nwp_active_slot_address);
  if (status != SL_STATUS_SI91X_FW_UPDATE_DONE) {
    DEBUGOUT("\r\nFailed to verify image: 0x%lx 0x%lx\r\n", status, active_slot_info.nwp_active_slot_address);
    while (1)
      ; // Halt execution in case of failure
  } else {
    DEBUGOUT("\r\nImage verified successfully: 0x%lx\r\n", active_slot_info.nwp_active_slot_address);
  }

  // Verify the image for the M4 active slot
  status = sl_si91x_verify_image(active_slot_info.m4_active_slot_address);
  if (status != SL_STATUS_SI91X_FW_UPDATE_DONE) {
    DEBUGOUT("\r\nFailed to verify image: 0x%lx 0x%lx\r\n", status, active_slot_info.m4_active_slot_address);
    while (1)
      ; // Halt execution in case of failure
  } else {
    DEBUGOUT("\r\nImage verified successfully: 0x%lx\r\n", active_slot_info.m4_active_slot_address);
  }

  //Request to load QSPI keys
  status = sl_si91x_fallback_load_qspi_keys(active_slot_info.m4_active_slot_address);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\nFailed to load keys: 0x%lx 0x%lx\r\n", status, active_slot_info.m4_active_slot_address);
    while (1)
      ; // Halt execution in case of failure
  } else {
    DEBUGOUT("\r\nLoad Keys successfully: 0x%lx\r\n", active_slot_info.m4_active_slot_address);
  }

  // Perform a soft reset NWP FW
  sl_si91x_nwp_soft_reset_from_updater(active_slot_info.m4_active_slot_address);

  // Call the M4 updater application
  sl_si91x_m4_updater_app();
  while (1)
    ;
}

/*
  * @brief Get active slot addresses for M4 and NWP
  * @param active_slot_info Pointer to structure that will be populated with active slot information
  * @return sl_status_t Status of the operation
  */
sl_status_t sl_si91x_app_get_active_slot_addresses(sl_si91x_active_slot_info_t *active_slot_info)
{
  sl_status_t status;
  sl_si91x_fw_ab_slot_management_t slot_info = { 0 };

  // Validate input parameter
  if (active_slot_info == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  // Get the current slot information
  status = sl_si91x_ab_get_slot_info(&slot_info);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Get M4 active slot information
  active_slot_info->m4_active_slot = slot_info.m4_slot_info.current_active_M4_slot;
  if (active_slot_info->m4_active_slot == SLOT_A) {
    active_slot_info->m4_active_slot_address = slot_info.m4_slot_info.m4_slot_A.slot_image_offset;
  } else if (active_slot_info->m4_active_slot == SLOT_B) {
    active_slot_info->m4_active_slot_address = slot_info.m4_slot_info.m4_slot_B.slot_image_offset;
  } else {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Get NWP active slot information
  active_slot_info->nwp_active_slot = slot_info.nwp_slot_info.current_active_nwp_slot;
  if (active_slot_info->nwp_active_slot == SLOT_A) {
    active_slot_info->nwp_active_slot_address = slot_info.nwp_slot_info.nwp_slot_A.slot_image_offset;
  } else if (active_slot_info->nwp_active_slot == SLOT_B) {
    active_slot_info->nwp_active_slot_address = slot_info.nwp_slot_info.nwp_slot_B.slot_image_offset;
  } else {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return SL_STATUS_OK;
}
