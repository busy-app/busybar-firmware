/***************************************************************************/ /**
 * @file
 * @brief AWS Device Shadow Application
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <string.h>
#include "cmsis_os2.h"
#include "sl_status.h"
#include "sl_net.h"
#include "sl_wifi.h"
#include "sl_wifi_device.h"
#include "sl_net_wifi_types.h"
#include "sl_utility.h"
#include "sl_si91x_driver.h"

//! Certificates to be loaded
#include "aws_client_certificate.pem.crt.h"
#include "aws_client_private_key.pem.key.h"
#include "aws_starfield_ca.pem.h"

//! AWS files
#include "aws_iot_error.h"
#include "aws_iot_config.h"
#include "aws_iot_shadow_interface.h"

/******************************************************
*                    Constants
******************************************************/
#define JSON_DOC_BUFFER_LENGTH       300
#define ROOM_TEMPERATURE_UPPER_LIMIT 32
#define ROOM_TEMPERATURE_LOWER_LIMIT 25
#define STARTING_ROOM_TEMPERATURE    ROOM_TEMPERATURE_LOWER_LIMIT
#define ENABLE_NWP_POWER_SAVE        0

/******************************************************
 *               Function Declarations
 ******************************************************/
static void application_start(void *argument);
sl_status_t load_certificates_in_flash();
sl_status_t start_aws_device_shadow();
static void simulate_room_temperature(float *p_room_temperature, bool *p_window_open);
static void window_actuate_callback(const char *p_json_string, uint32_t json_string_data_len, jsonStruct_t *p_context);
static void shadow_update_status_callback(const char *p_thing_name,
                                          ShadowActions_t action,
                                          Shadow_Ack_Status_t status,
                                          const char *p_received_json_document,
                                          void *p_context_data);

/******************************************************
 *               Variable Definitions
 ******************************************************/
const osThreadAttr_t thread_attributes = {
  .name       = "app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3584,
  .priority   = osPriorityLow,
  .tz_module  = 0,
  .reserved   = 0,
};

static const sl_wifi_device_configuration_t client_init_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .boot_config = { .oper_mode       = SL_SI91X_CLIENT_MODE,
                   .coex_mode       = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map = (SL_WIFI_FEAT_SECURITY_PSK
#if ENABLE_NWP_POWER_SAVE
                                       | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
#endif
                                       ),
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT
                                              | SL_SI91X_TCP_IP_FEAT_SSL | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_RSA_KEY_WITH_4096_SUPPORT | SL_SI91X_EXT_FEAT_SSL_CERT_WITH_4096_KEY_SUPPORT
                      | SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
#if ENABLE_NWP_POWER_SAVE
                      | SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE
#endif
#ifdef SLI_SI917
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = (
#if ENABLE_NWP_POWER_SAVE
                     (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP | SL_WIFI_ENABLE_ENHANCED_MAX_PSP)
#else
                     0
#endif
                       ) }
};

char json_document_buffer[JSON_DOC_BUFFER_LENGTH];

/******************************************************
 *               Function Definitions
 ******************************************************/
void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_net_wifi_client_profile_t profile = { 0 };
  sl_ip_address_t ip_address           = { 0 };

  sl_status_t status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &client_init_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nUnexpected error while doing init: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWiFi Init Success\r\n");

#ifdef SLI_SI91X_MCU_INTERFACE
  uint8_t xtal_enable = 1;
  status              = sl_si91x_m4_ta_secure_handshake(SL_SI91X_ENABLE_XTAL, 1, &xtal_enable, 0, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to bring m4_ta_secure_handshake: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nm4_ta_secure_handshake Success\r\n");
#endif

  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf("\r\nError while connecting to Access point: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nConnected to Access point\r\n");

  status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &profile);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to get client profile: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nClient profile fetched successfully\r\n");

  ip_address.type = SL_IPV4;
  memcpy(&ip_address.ip.v4.bytes, &profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
  print_sl_ip_address(&ip_address);

  status = load_certificates_in_flash();
  if (status != SL_STATUS_OK) {
    printf("\r\nUnexpected error while loading certificates: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nCertificate loading success\r\n");

#if ENABLE_NWP_POWER_SAVE
  sl_wifi_performance_profile_v2_t performance_profile = { .profile = ASSOCIATED_POWER_SAVE_LOW_LATENCY };
  status                                               = sl_wifi_set_performance_profile_v2(&performance_profile);
  if (status != SL_STATUS_OK) {
    printf("\r\nPower save configuration Failed, Error Code : 0x%lX\r\n", status);
    return;
  }
  printf("\r\nAssociated Power Save Enabled\n");
#endif

  status = start_aws_device_shadow();
  if (status != SL_STATUS_OK) {
    printf("\r\nUnexpected error occurred in AWS Device shadow: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nAWS Device shadow Success\r\n");
}

sl_status_t load_certificates_in_flash()
{
  sl_status_t status = SL_STATUS_FAIL;

  status = sl_net_set_credential(SL_NET_TLS_SERVER_CREDENTIAL_ID(0),
                                 SL_NET_SIGNING_CERTIFICATE,
                                 aws_starfield_ca,
                                 sizeof(aws_starfield_ca));
  if (status != SL_STATUS_OK) {
    printf("\r\nUnexpected error while loading CA certificate: 0x%lx\r\n", status);
    return status;
  }

  status = sl_net_set_credential(SL_NET_TLS_CLIENT_CREDENTIAL_ID(0),
                                 SL_NET_CERTIFICATE,
                                 aws_client_certificate,
                                 sizeof(aws_client_certificate));
  if (status != SL_STATUS_OK) {
    printf("\r\nUnexpected error while loading Client certificate: 0x%lx\r\n", status);
    return status;
  }

  status = sl_net_set_credential(SL_NET_TLS_CLIENT_CREDENTIAL_ID(0),
                                 SL_NET_PRIVATE_KEY,
                                 aws_client_private_key,
                                 sizeof(aws_client_private_key));
  if (status != SL_STATUS_OK) {
    printf("\r\nUnexpected error while loading private key: 0x%lx\r\n", status);
  } else {
    printf("\r\nCertificate loading success\r\n");
  }

  return status;
}

sl_status_t start_aws_device_shadow()
{
  IoT_Error_t rc                                      = FAILURE;
  AWS_IoT_Client mqtt_client                          = { 0 };
  ShadowInitParameters_t shadow_init_parameters       = ShadowInitParametersDefault;
  ShadowConnectParameters_t shadow_connect_parameters = ShadowConnectParametersDefault;
  bool window_open_state                              = false;
  jsonStruct_t window_actuator                        = { 0 };
  float temperature                                   = 0.0;

  jsonStruct_t temperature_handler    = { 0 };
  size_t size_of_json_document_buffer = sizeof(json_document_buffer) / sizeof(json_document_buffer[0]);

  shadow_init_parameters.pRootCA             = (char *)aws_starfield_ca;
  shadow_init_parameters.pClientCRT          = (char *)aws_client_certificate;
  shadow_init_parameters.pClientKey          = (char *)aws_client_private_key;
  shadow_init_parameters.enableAutoReconnect = true;

  sl_mac_address_t mac_addr = { 0 };
  char mac_id[18]           = { 0 };
  char client_id[25]        = { 0 };

  sl_status_t status = sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, &mac_addr);
  if (status != SL_STATUS_OK) {
    printf("\r\nError while fetching mac address: 0x%lx\r\n", status);
    return status;
  }
  print_mac_address(&mac_addr);

  sprintf(mac_id,
          "%x:%x:%x:%x:%x:%x",
          mac_addr.octet[0],
          mac_addr.octet[1],
          mac_addr.octet[2],
          mac_addr.octet[3],
          mac_addr.octet[4],
          mac_addr.octet[5]);
  sprintf(client_id, "silabs_%s", mac_id);
  printf("\r\nClient ID: %s\r\n", client_id);

  rc = aws_iot_shadow_init(&mqtt_client, &shadow_init_parameters);
  if (rc < 0) {
    printf("\r\nShadow Initialization failed with aws error: %d\r\n", rc);
    return rc;
  }
  printf("\r\nShadow Initialization Success\r\n");

  shadow_connect_parameters.pMyThingName    = AWS_IOT_MY_THING_NAME;
  shadow_connect_parameters.pMqttClientId   = client_id;
  shadow_connect_parameters.mqttClientIdLen = (uint16_t)strlen(client_id);

  rc = aws_iot_shadow_connect(&mqtt_client, &shadow_connect_parameters);
  if (rc < 0) {
    printf("\r\nShadow Connection failed with aws error: %d\r\n", rc);
    return rc;
  }
  printf("\r\nShadow Connection Success\r\n");

  window_actuator.pKey       = "Window Open";
  window_actuator.pData      = &window_open_state;
  window_actuator.dataLength = sizeof(bool);
  window_actuator.type       = SHADOW_JSON_BOOL;
  window_actuator.cb         = window_actuate_callback;

  temperature_handler.pKey       = "Temperature";
  temperature_handler.pData      = &temperature;
  temperature_handler.dataLength = sizeof(float);
  temperature_handler.type       = SHADOW_JSON_FLOAT;
  temperature_handler.cb         = NULL;

  rc = aws_iot_shadow_register_delta(&mqtt_client, &window_actuator);
  if (rc != SUCCESS) {
    printf("\r\nShadow Register Delta failed with aws error: %d\r\n", rc);
    return rc;
  }
  printf("\r\nShadow Register Delta\r\n");

  temperature = STARTING_ROOM_TEMPERATURE;

  while ((NETWORK_ATTEMPTING_RECONNECT == rc) || (NETWORK_RECONNECTED == rc) || (SUCCESS == rc)) {
    rc = aws_iot_shadow_yield(&mqtt_client, 2000);
    if (rc == NETWORK_ATTEMPTING_RECONNECT) {
      continue;
    }
    simulate_room_temperature(&temperature, &window_open_state);
    rc = aws_iot_shadow_init_json_document(json_document_buffer, size_of_json_document_buffer);
    if (rc != SUCCESS) {
      printf("\r\nFailed to initialize JSON buffer with error: %d\r\n", rc);
      continue;
    }

    rc = aws_iot_shadow_add_reported(json_document_buffer,
                                     size_of_json_document_buffer,
                                     2,
                                     &temperature_handler,
                                     &window_actuator);
    if (rc != SUCCESS) {
      printf("\r\nFailed to add reported value in JSON buffer with error: %d\r\n", rc);
      continue;
    }
    printf("\r\nAdding reported value in JSON buffer success\r\n");

    rc = aws_iot_finalize_json_document(json_document_buffer, size_of_json_document_buffer);
    if (rc != SUCCESS) {
      printf("\r\nFailed to finalize JSON buffer with error: %d\r\n", rc);
      continue;
    }
    printf("\r\nJSON finalization buffer Success\r\n");

    printf("\r\nUpdate Shadow: %s\r\n", json_document_buffer);

    rc = aws_iot_shadow_update(&mqtt_client,
                               AWS_IOT_MY_THING_NAME,
                               json_document_buffer,
                               shadow_update_status_callback,
                               NULL,
                               40,
                               true);
    if (rc != SUCCESS) {
      printf("\r\nFailed to update JSON buffer with error: %d\r\n", rc);
      continue;
    }
  }
  printf("\r\nSuccess to update JSON buffer\r\n");

  return rc;
}
static void simulate_room_temperature(float *p_room_temperature, bool *p_window_open)
{
  static float delta_change;

  if (*p_room_temperature >= ROOM_TEMPERATURE_UPPER_LIMIT) {
    delta_change = -0.5f;
  } else if (*p_room_temperature <= ROOM_TEMPERATURE_LOWER_LIMIT) {
    delta_change = +0.5f;
  }

  *p_room_temperature += delta_change;
  *p_window_open = false;

  if (*p_room_temperature >= (ROOM_TEMPERATURE_UPPER_LIMIT + ROOM_TEMPERATURE_LOWER_LIMIT) / 2) {
    *p_window_open = true;
  }
}

static void shadow_update_status_callback(const char *p_thing_name,
                                          ShadowActions_t action,
                                          Shadow_Ack_Status_t status,
                                          const char *p_received_json_document,
                                          void *p_context_data)
{
  UNUSED_PARAMETER(p_thing_name);
  UNUSED_PARAMETER(action);
  UNUSED_PARAMETER(p_received_json_document);
  UNUSED_PARAMETER(p_context_data);

  if (SHADOW_ACK_TIMEOUT == status) {
    printf("\r\nUpdate Timed out!!\r\n");
  } else if (SHADOW_ACK_REJECTED == status) {
    printf("\r\nUpdate Rejected!!\r\n");
  } else if (SHADOW_ACK_ACCEPTED == status) {
    printf("\r\nUpdate Accepted!!\r\n");
  }
}

static void window_actuate_callback(const char *p_json_string, uint32_t json_string_data_len, jsonStruct_t *p_context)
{
  UNUSED_PARAMETER(p_json_string);
  UNUSED_PARAMETER(json_string_data_len);

  if (p_context != NULL) {
    printf("Delta - Window state changed to %d", *(bool *)(p_context->pData));
  }
}