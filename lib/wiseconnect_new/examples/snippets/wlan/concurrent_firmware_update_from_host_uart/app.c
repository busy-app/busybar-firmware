/***************************************************************************/ /**
 * @file
 * @brief Concurrent Firmware Update From Host UART Example Application
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
 ******************************da************************************************/

#include "sl_net.h"
#include "app.h"
#include "errno.h"
#include "sl_utility.h"
#include "sl_wifi.h"
#include "sl_net_wifi_types.h"
#include "sl_si91x_socket_support.h"
#include "sl_si91x_socket_constants.h"
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_socket.h"
#include "sl_net_si91x.h"
#include "login.h"
#include "jsmn.h"
#include "sl_http_server.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_eusart.h"
#include "sl_uartdrv_eusart_exp_config.h"
#include "sl_si91x_core_utilities.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define CLEAN_HTTP_CLIENT_IF_FAILED(status, client_handle, is_sync) \
  {                                                                 \
    if (status != SL_STATUS_OK) {                                   \
      sl_http_client_deinit(client_handle);                         \
      return ((is_sync == 0) ? status : callback_status);           \
    }                                                               \
  }

#define HTTP_SERVER_PORT 80

#define HTTP_SERVER_RESPONSE 0
#define HTTP_SERVER_ENABLE   1

#define WIFI_AP_PROFILE_SSID "MY_DUAL_AP_SSID"
#define WIFI_AP_CREDENTIAL   "MY_AP_PASSPHRASE"

// Memory length for send buffer
#define TCP_BUFFER_SIZE 1380
#define UDP_BUFFER_SIZE 1390

//IPv6 address of STA connected to ThirdParty AP
#define SERVER_IP "2401:4290:1245:11ed::4"

// Server port number
#define SERVER_PORT 5000

// Module port number
#define LISTENING_PORT 5005

#define BACK_LOG 1

#define AP_VAP 1

#define BYTES_TO_SEND (1 << 29)              // 512MB.
#define TEST_TIMEOUT  (10000 * tick_count_s) // 10sec

#define HIGH_PERFORMANCE_SOCKET BIT(7)

/******************************************************
 *               Variable Definitions
 ******************************************************/

uint8_t has_data_received = 0;
uint32_t bytes_read       = 0;
uint32_t now              = 0;
uint8_t retry             = 5;
uint8_t is_provisioning   = 0;
uint32_t tick_count_s     = 1;
uint8_t ap_vap            = AP_VAP;
static char response[300] = { 0 };

volatile bool is_server_running = false;

uint8_t data_buffer[TCP_BUFFER_SIZE];
uint8_t address_buffer[SL_IPV6_ADDRESS_LENGTH];

char WIFI_CLIENT_PROFILE_SSID[32]; // Assuming SSID can be up to 32 characters long
char WIFI_CLIENT_CREDENTIAL[64];   // Assuming Password can be up to 64 characters long
char WIFI_CLIENT_SECURITY_TYPE[32];

static jsmntok_t t[128]; /* We expect no more than 128 tokens */
static sl_http_server_t server_handle = { 0 };

sl_status_t callback_status = SL_STATUS_OK;

const osThreadAttr_t thread_attributes = {
  .name       = "app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityNormal,
  .tz_module  = 0,
  .reserved   = 0,
};

const osThreadAttr_t fw_up_thread_attributes = {
  .name       = "fw_up_thread",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityNormal,
  .tz_module  = 0,
  .reserved   = 0,
};

osThreadId_t fw_up_thread_id = NULL;

app_state_t app_cb; //! application control block

//! IP address of the module as AP
#define DEFAULT_WIFI_AP_MODULE_IP4_ADDRESS "192.168.10.10"

//! IP address of netmask
#define DEFAULT_WIFI_AP_NS_MODULE_IP4_ADDRESS "255.255.255.0"

//! IP address of Gateway
#define DEFAULT_WIFI_AP_GATEWAY4_ADDRESS "192.168.10.10"

//! IPv6 address of the module
#define DEFAULT_WIFI_AP_MODULE_IP6_ADDRESS "2001:db8:0:1::121"

//! IP address of Gateway
#define DEFAULT_WIFI_AP_GATEWAY6_ADDRESS "2001:db8:0:1::121"

static sl_net_wifi_client_profile_t wifi_client_profile_4 = {
    .config = {
        .channel.channel = SL_WIFI_AUTO_CHANNEL,
        .channel.band = SL_WIFI_AUTO_BAND,
        .channel.bandwidth = SL_WIFI_AUTO_BANDWIDTH,
        .bssid = {{0}},
        .bss_type = SL_WIFI_BSS_TYPE_INFRASTRUCTURE,
        .client_options = 0,
        .credential_id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID,
    },
    .ip = {
        .mode = SL_IP_MANAGEMENT_DHCP,
        .type = SL_IPV4,
        .host_name = NULL,
        .ip = {{{0}}},
    }
};

static sl_net_wifi_client_profile_t wifi_client_profile_6 = {
    .config = {
        .channel.channel = SL_WIFI_AUTO_CHANNEL,
        .channel.band = SL_WIFI_AUTO_BAND,
        .channel.bandwidth = SL_WIFI_AUTO_BANDWIDTH,
        .bssid = {{0}},
        .bss_type = SL_WIFI_BSS_TYPE_INFRASTRUCTURE,
        .client_options = 0,
        .credential_id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID,
    },
    .ip = {
        .mode = SL_IP_MANAGEMENT_DHCP,
        .type = SL_IPV6,
        .host_name = NULL,
        .ip = {{{0}}},
    }
};

static sl_net_wifi_ap_profile_t wifi_ap_profile_4 = {
    .config = {
        .ssid.value = WIFI_AP_PROFILE_SSID,
        .ssid.length = sizeof(WIFI_AP_PROFILE_SSID)-1,
        .channel.channel = SL_WIFI_AUTO_CHANNEL,
        .channel.band = SL_WIFI_AUTO_BAND,
        .channel.bandwidth = SL_WIFI_AUTO_BANDWIDTH,
        .security = SL_WIFI_WPA2,
        .encryption = SL_WIFI_CCMP_ENCRYPTION,
        .rate_protocol = SL_WIFI_RATE_PROTOCOL_AUTO,
        .options = 0,
        .credential_id = SL_NET_USER_CREDENTIAL_ID,
        .keepalive_type = SL_SI91X_AP_NULL_BASED_KEEP_ALIVE,
        .beacon_interval = 100,
        .client_idle_timeout = 0xFF,
        .dtim_beacon_count = 3,
        .maximum_clients = 3,
        .beacon_stop = 0,
    },
    .ip = {
        .mode = SL_IP_MANAGEMENT_STATIC_IP,
        .type = SL_IPV4,
        .host_name = NULL,
        .ip = {{{0}}},
    }
};

static sl_net_wifi_ap_profile_t wifi_ap_profile_6 = {
    .config = {
        .ssid.value = WIFI_AP_PROFILE_SSID,
        .ssid.length = sizeof(WIFI_AP_PROFILE_SSID)-1,
        .channel.channel = SL_WIFI_AUTO_CHANNEL,
        .channel.band = SL_WIFI_AUTO_BAND,
        .channel.bandwidth = SL_WIFI_AUTO_BANDWIDTH,
        .security = SL_WIFI_WPA2,
        .encryption = SL_WIFI_CCMP_ENCRYPTION,
        .rate_protocol = SL_WIFI_RATE_PROTOCOL_AUTO,
        .options = 0,
        .credential_id = SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID,
        .keepalive_type = SL_SI91X_AP_NULL_BASED_KEEP_ALIVE,
        .beacon_interval = 100,
        .client_idle_timeout = 0xFF,
        .dtim_beacon_count = 3,
        .maximum_clients = 3,
        .beacon_stop = 0,
    },
    .ip = {
        .mode = SL_IP_MANAGEMENT_STATIC_IP,
        .type = SL_IPV6,
        .host_name = NULL,
        .ip = {{{0}}},
    }
};

sl_net_wifi_psk_credential_entry_t wifi_ap_credential = { .type        = SL_NET_WIFI_PSK,
                                                          .data_length = sizeof(WIFI_AP_CREDENTIAL) - 1,
                                                          .data        = WIFI_AP_CREDENTIAL };

/******************************************************
 *               Function Declarations
 ******************************************************/

void receive_data_from_tcp_client(void);
void send_data_to_udp_server(void);
void parse_json_response(void);
void sl_fw_task_up(void);
void eusart2_init(void);
extern void fw_up_configurator_task(void *argument);
static void application_start(void *argument);
sl_wifi_security_t string_to_security_type(const char *securityType);
sl_status_t login_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
sl_status_t default_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
sl_status_t connect_data_handler(sl_http_server_t *handle, sl_http_server_request_t *req);
static sl_status_t ap_connected_event_handler(sl_wifi_event_t event,
                                              sl_status_t status_code,
                                              void *data,
                                              uint32_t data_length,
                                              void *arg);
static sl_status_t ap_disconnected_event_handler(sl_wifi_event_t event,
                                                 sl_status_t status_code,
                                                 void *data,
                                                 uint32_t data_length,
                                                 void *arg);

/******************************************************
 *               Function Definitions
 ******************************************************/
osMutexId_t printf_mutex = 0;

sl_status_t initialize_printf_mutex()
{
  printf_mutex = osMutexNew(NULL);
  if (printf_mutex == NULL) {
    LOG_PRINT("\r\nFailed to create printf_mutex\r\n");
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

osMutexId_t fw_update_status_mutex = 0;
volatile bool fw_update_done       = false;

sl_status_t initialize_fw_update_status_mutex()
{
  fw_update_status_mutex = osMutexNew(NULL);
  if (fw_update_status_mutex == NULL) {
    LOG_PRINT("\r\nFailed to create fw_update_status_mutex\r\n");
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

void fw_update_status(bool status)
{
  osMutexAcquire(fw_update_status_mutex, 0xFFFFFFFFUL);
  fw_update_done = status;
  osMutexRelease(fw_update_status_mutex);
}

bool check_fw_update_status()
{
  bool done_status = false;

  osMutexAcquire(fw_update_status_mutex, 0xFFFFFFFFUL);
  done_status = fw_update_done;
  osMutexRelease(fw_update_status_mutex);

  return done_status;
}
void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

sl_status_t join_callback_handler(sl_wifi_event_t event,
                                  sl_status_t status_code,
                                  char *result,
                                  uint32_t result_length,
                                  void *arg)
{
  UNUSED_PARAMETER(result);
  UNUSED_PARAMETER(arg);

  LOG_PRINT("in join CB\r\n");
  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    LOG_PRINT("F: Join Event received with %lu bytes payload\n", result_length);
    app_cb = MODULE_DISCONNECT_STATE;
    return status_code;
  }
  return SL_STATUS_OK;
}

void print_client_mac_address(sl_mac_address_t *mac_address)
{
  if (mac_address == NULL) {
    return;
  }
  LOG_PRINT("%2X:%2X:%2X:%2X:%2X:%2X",
            mac_address->octet[0],
            mac_address->octet[1],
            mac_address->octet[2],
            mac_address->octet[3],
            mac_address->octet[4],
            mac_address->octet[5]);
}

void print_sl_client_ip_address(const sl_ip_address_t *sl_ip_address)
{
  if (sl_ip_address == NULL) {
    return;
  }

  if (sl_ip_address->type == SL_IPV4) {
    print_sl_client_ipv4_address(&sl_ip_address->ip.v4);
  } else if (sl_ip_address->type == SL_IPV6) {
    print_sl_client_ipv6_address(&sl_ip_address->ip.v6);
  }
}

void print_sl_client_ipv4_address(const sl_ipv4_address_t *ip_address)
{
  LOG_PRINT("%d.%d.%d.%d", ip_address->bytes[0], ip_address->bytes[1], ip_address->bytes[2], ip_address->bytes[3]);
}

void print_sl_client_ipv6_address(const sl_ipv6_address_t *ip_address)
{
  char temp_buffer[46] = { 0 };
  sl_inet_ntop6((const unsigned char *)(ip_address), (char *)temp_buffer, sizeof(temp_buffer));
  LOG_PRINT("%s\r\n", temp_buffer);
}

#if defined _SILICON_LABS_32B_SERIES_2_CONFIG_5
//! Initialization of eusart interface for debug prints
void eusart2_init(void)
{
  // Chip errata
  CHIP_Init();

  // Initialize GPIO and EUSART2
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(cmuClock_EUSART2, true);

  // Configure the EUSART TX pin to the board controller as an output
  GPIO_PinModeSet(SL_UARTDRV_EUSART_EXP_TX_PORT, SL_UARTDRV_EUSART_EXP_TX_PIN, gpioModePushPull, 1);

  // Default asynchronous initializer (115.2 Kbps, 8N1, no flow control)
  EUSART_UartInit_TypeDef init = EUSART_UART_INIT_DEFAULT_HF;

  // Route EUSART2 TX and RX to the board controller TX and RX pins
  GPIO->EUSARTROUTE[2].TXROUTE = (SL_UARTDRV_EUSART_EXP_TX_PORT << _GPIO_EUSART_TXROUTE_PORT_SHIFT)
                                 | (SL_UARTDRV_EUSART_EXP_TX_PIN << _GPIO_EUSART_TXROUTE_PIN_SHIFT);

  // Enable RX and TX signals now that they have been routed
  GPIO->EUSARTROUTE[2].ROUTEEN = GPIO_EUSART_ROUTEEN_TXPEN;

  // Configure and enable EUSART2 for high-frequency (EM0/1) operation
  EUSART_UartInitHf(EUSART2, &init);

  // Enable NVIC USART sources
  NVIC_ClearPendingIRQ(EUSART2_TX_IRQn);
  NVIC_EnableIRQ(EUSART2_TX_IRQn);
}
#endif
static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  eusart2_init();
  sl_status_t status                             = SL_STATUS_OK;
  sl_wifi_channel_t client_channel               = { 0 };
  sl_net_wifi_client_profile_t profile           = { 0 };
  sl_ip_address_t ip_address                     = { 0 };
  unsigned char hex_addr[SL_IPV6_ADDRESS_LENGTH] = { 0 };
  uint32_t ip_addr[4]                            = { 0 };
  uint32_t gateway[4]                            = { 0 };
  uint8_t fw_up_thread_create                    = 1;

  static sl_http_server_handler_t request_handlers[2] = { { .uri = "/login", .handler = login_request_handler },
                                                          { .uri = "/connect", .handler = connect_data_handler } };

  sl_http_server_config_t server_config = { 0 };

  int return_value;

  while (1) {
    switch (app_cb) {

      case INITIAL_STATE:

        status = sl_net_init(SL_NET_WIFI_AP_INTERFACE, &sl_wifi_default_concurrent_v6_configuration, NULL, NULL);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to start Wi-Fi AP interface: 0x%lx\r\n", status);
          return;
        }

        sl_wifi_set_callback_v2(SL_WIFI_CLIENT_CONNECTED_EVENTS, ap_connected_event_handler, NULL);
        sl_wifi_set_callback_v2(SL_WIFI_CLIENT_DISCONNECTED_EVENTS, ap_disconnected_event_handler, NULL);
        LOG_PRINT("\r\nWi-Fi AP interface init Success");

        status = sl_net_up(SL_NET_WIFI_AP_INTERFACE, SL_NET_DEFAULT_WIFI_AP_PROFILE_ID);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to bring Wi-Fi AP interface up: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nAP started\r\n");

        is_provisioning = 1;
        if (app_cb == INITIAL_STATE)
          app_cb = WAIT_STATE;

        break;

      case PROVISION_STATE:

        server_config.port            = HTTP_SERVER_PORT;
        server_config.default_handler = default_handler;
        server_config.handlers_list   = request_handlers;
        server_config.handlers_count  = 2;

        status = sl_http_server_init(&server_handle, &server_config);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nHTTP server init failed:%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nHttp Init done\r\n");

        status = sl_http_server_start(&server_handle);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nServer start fail:%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nServer start done\r\n");

        osDelay(10000);

        is_server_running = true;

        //! Server is exited when we receive the credentials
        while (is_server_running) {
          osDelay(100);
        }

        parse_json_response();

        SL_DEBUG_LOG("SSID stored:%s\r\n", WIFI_CLIENT_PROFILE_SSID);
        SL_DEBUG_LOG("Password stored:%s\r\n", WIFI_CLIENT_CREDENTIAL);
        SL_DEBUG_LOG("Security type stored:%s\r\n", WIFI_CLIENT_SECURITY_TYPE);

        status = sl_http_server_stop(&server_handle);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nServer stop fail:%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nServer stop done\r\n");

        status = sl_http_server_deinit(&server_handle);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nServer deinit fail:%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nServer deinit done\r\n");

        status = sl_net_deinit(SL_NET_WIFI_AP_INTERFACE);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nAp deinit : 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nAP Deinit success\r\n");

        app_cb = STA_BRINGUP_STATE;

        break;

      case STA_BRINGUP_STATE:

        status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_default_concurrent_v6_configuration, NULL, NULL);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to start Wi-Fi client interface: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nWi-Fi Client interface init\r\n");
        if (fw_up_thread_create == 1) {
          //! Create separate thread to handle firmware update
          sl_fw_task_up();
          fw_up_thread_create = 0;
        }

        sl_wifi_set_callback_v2(SL_WIFI_CLIENT_CONNECTED_EVENTS, ap_connected_event_handler, NULL);
        sl_wifi_set_callback_v2(SL_WIFI_CLIENT_DISCONNECTED_EVENTS, ap_disconnected_event_handler, NULL);

        wifi_client_profile_4.config.ssid.length = strlen((char *)WIFI_CLIENT_PROFILE_SSID);
        memcpy(&wifi_client_profile_4.config.ssid.value,
               WIFI_CLIENT_PROFILE_SSID,
               wifi_client_profile_4.config.ssid.length);
        wifi_client_profile_4.config.security = string_to_security_type(WIFI_CLIENT_SECURITY_TYPE);

        wifi_client_profile_6.config.ssid.length = strlen((char *)WIFI_CLIENT_PROFILE_SSID);
        memcpy(&wifi_client_profile_6.config.ssid.value,
               WIFI_CLIENT_PROFILE_SSID,
               wifi_client_profile_6.config.ssid.length);
        wifi_client_profile_6.config.security = string_to_security_type(WIFI_CLIENT_SECURITY_TYPE);

        //  Keeping the station ipv4 record in profile_id_0
        status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_0, &wifi_client_profile_4);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to set client profile: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nWi-Fi set client profile v4 success\r\n");

        //  Keeping the station ipv6 record in profile_id_1
        status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &wifi_client_profile_6);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to set client profile: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nWi-Fi set client profile v6 success\r\n");

        SL_DEBUG_LOG("\r\nSTA credentials through http server");
        sl_wifi_credential_t cred  = { 0 };
        sl_wifi_credential_id_t id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;
        cred.type                  = SL_WIFI_PSK_CREDENTIAL;
        memcpy(cred.psk.value, WIFI_CLIENT_CREDENTIAL, strlen((char *)WIFI_CLIENT_CREDENTIAL));

        status =
          sl_net_set_credential(id, SL_NET_WIFI_PSK, WIFI_CLIENT_CREDENTIAL, strlen((char *)WIFI_CLIENT_CREDENTIAL));
        if (status == SL_STATUS_OK) {
          SL_DEBUG_LOG("Credentials set, id : %lu\n", id);
          sl_wifi_client_configuration_t access_point = { 0 };
          access_point.ssid.length                    = strlen((char *)WIFI_CLIENT_PROFILE_SSID);
          memcpy(access_point.ssid.value, WIFI_CLIENT_PROFILE_SSID, access_point.ssid.length);
          access_point.security      = string_to_security_type(WIFI_CLIENT_SECURITY_TYPE);
          access_point.encryption    = SL_WIFI_CCMP_ENCRYPTION;
          access_point.credential_id = id;

          SL_DEBUG_LOG("SSID %s\n", access_point.ssid.value);

          sl_wifi_set_join_callback_v2(join_callback_handler, NULL);
          status = sl_wifi_connect(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &access_point, 25000);
        } else {
          LOG_PRINT("\r\nFailed to set credentials; status: %lu\n", status);
        }
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
          //! If scan failed retry again after 5 seconds
          if (status == 0x10003) {
            if (retry) {
              retry--;
              osDelay(5000);
              break;
            } else {
              retry = 5;
            }
          }
          //! Before resetting the module stop the python script
          firmware_update_operation_complete();
          //! Before resetting the module terminate the firmware update thread
          status = osThreadTerminate(fw_up_thread_id);
          if (status == SL_STATUS_OK) {
            LOG_PRINT("\r\nthread terminated");
            fw_up_thread_create = 1;
          }
          status = sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
          if (status != SL_STATUS_OK) {
            LOG_PRINT("\r\nSTA deinit : 0x%lx\r\n", status);
          }
          app_cb = INITIAL_STATE;
          break;
        }

        status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_0, &profile);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
          return;
        }

        status = sl_si91x_configure_ip_address(&profile.ip, SL_SI91X_WIFI_CLIENT_VAP_ID);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nIPv4 address configuration is failed : 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nIPv4 address configuration is done");

        if (profile.ip.type == SL_IPV4) {
          ip_address.type = SL_IPV4;
          memcpy(&ip_address.ip.v4.bytes, &profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
          LOG_PRINT("\r\nClient IPv4 : ");
          print_sl_client_ip_address(&ip_address);
        }

        //  Station ipv6 config using profile_id_1
        status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &profile);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
          return;
        }

        status = sl_si91x_configure_ip_address(&profile.ip, SL_SI91X_WIFI_CLIENT_VAP_ID);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nIPv6 address configuration is failed : 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nIPv6 address configuration is done");

        status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &profile);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to set client profile: 0x%lx\r\n", status);
          return;
        }

        status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &profile);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to get client_v6 profile: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nClient IPv6 : ");

        if (profile.ip.type == SL_IPV6) {
          sl_ip_address_t link_local_address = { 0 };
          link_local_address.ip.v6           = profile.ip.ip.v6.link_local_address;
          link_local_address.type            = SL_IPV6;
          LOG_PRINT("\nLink Local Address: ");
          print_sl_client_ip_address(&link_local_address);

          sl_ip_address_t global_address = { 0 };
          global_address.ip.v6           = profile.ip.ip.v6.global_address;
          global_address.type            = SL_IPV6;
          LOG_PRINT("Global Address: ");
          print_sl_client_ip_address(&global_address);

          sl_ip_address_t gateway = { 0 };
          gateway.ip.v6           = profile.ip.ip.v6.gateway;
          gateway.type            = SL_IPV6;
          LOG_PRINT("Gateway Address: ");
          print_sl_client_ip_address(&gateway);
        }

        app_cb = AP_BRINGUP_STATE;

        break;

      case AP_BRINGUP_STATE:

        status = sl_net_init(SL_NET_WIFI_AP_INTERFACE, &sl_wifi_default_concurrent_v6_configuration, NULL, NULL);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to start Wi-Fi AP interface: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nWi-Fi AP interface init\r\n");

        sl_net_inet_addr(DEFAULT_WIFI_AP_MODULE_IP4_ADDRESS, (uint32_t *)&wifi_ap_profile_4.ip.ip.v4.ip_address.bytes);
        sl_net_inet_addr(DEFAULT_WIFI_AP_NS_MODULE_IP4_ADDRESS, (uint32_t *)&wifi_ap_profile_4.ip.ip.v4.netmask.bytes);
        sl_net_inet_addr(DEFAULT_WIFI_AP_GATEWAY4_ADDRESS, (uint32_t *)&wifi_ap_profile_4.ip.ip.v4.gateway.bytes);

        return_value = sl_inet_pton6(DEFAULT_WIFI_AP_MODULE_IP6_ADDRESS,
                                     DEFAULT_WIFI_AP_MODULE_IP6_ADDRESS + strlen(DEFAULT_WIFI_AP_MODULE_IP6_ADDRESS),
                                     hex_addr,
                                     (unsigned int *)ip_addr);
        if (return_value != 0x1) {
          LOG_PRINT("\r\nIPv6 conversion failed.\r\n");
          return;
        }
        return_value = sl_inet_pton6(DEFAULT_WIFI_AP_GATEWAY6_ADDRESS,
                                     DEFAULT_WIFI_AP_GATEWAY6_ADDRESS + strlen(DEFAULT_WIFI_AP_GATEWAY6_ADDRESS),
                                     hex_addr,
                                     (unsigned int *)gateway);
        if (return_value != 0x1) {
          LOG_PRINT("\r\nIPv6 conversion failed.\r\n");
          return;
        }

        memcpy(wifi_ap_profile_6.ip.ip.v6.global_address.value, ip_addr, sizeof(sl_ipv6_address_t));
        memcpy(wifi_ap_profile_6.ip.ip.v6.gateway.value, gateway, sizeof(sl_ipv6_address_t));

        status = sl_wifi_get_channel(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &client_channel);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to get client channel: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nSuccess to get client channel\r\n");

        wifi_ap_profile_4.config.channel.channel = client_channel.channel;
        wifi_ap_profile_6.config.channel.channel = client_channel.channel;

        status = sl_net_set_profile(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_0, &wifi_ap_profile_4);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to set AP profile: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nSuccess to set AP v4 profile \r\n");

        status = sl_net_set_profile(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1, &wifi_ap_profile_6);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to set AP profile: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nSuccess to set AP v6 profile \r\n");

        status = sl_net_set_credential(SL_NET_USER_CREDENTIAL_ID,
                                       wifi_ap_credential.type,
                                       &wifi_ap_credential.data,
                                       wifi_ap_credential.data_length);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to set credentials: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nWi-Fi set credential success\n");

        status = sl_si91x_configure_ip_address(&wifi_ap_profile_4.ip, SL_SI91X_WIFI_AP_VAP_ID);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nIPv4 address configuration is failed : 0x%lx\r\n", status);
          return;
        }
        status = sl_net_get_profile(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_0, &wifi_ap_profile_4);

        if (wifi_ap_profile_4.ip.type == SL_IPV4) {
          ip_address.type = SL_IPV4;
          LOG_PRINT("\r\nAP IPv4 : ");
          memcpy(&ip_address.ip.v4.bytes, &wifi_ap_profile_4.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
          print_sl_client_ip_address(&ip_address);
        }

        status = sl_net_get_profile(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1, &wifi_ap_profile_6);

        status = sl_si91x_configure_ip_address(&wifi_ap_profile_6.ip, SL_SI91X_WIFI_AP_VAP_ID);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nIPv4 address configuration is failed : 0x%lx\r\n", status);
          return;
        }

        LOG_PRINT("\r\nAP IPv6 : ");
        if (wifi_ap_profile_6.ip.type == SL_IPV6) {
          sl_ip_address_t link_local_address = { 0 };
          link_local_address.ip.v6           = wifi_ap_profile_6.ip.ip.v6.link_local_address;
          link_local_address.type            = SL_IPV6;
          LOG_PRINT("\nLink Local Address: ");
          print_sl_client_ip_address(&link_local_address);
          sl_ip_address_t global_address = { 0 };
          global_address.ip.v6           = wifi_ap_profile_6.ip.ip.v6.global_address;
          global_address.type            = SL_IPV6;
          LOG_PRINT("Global Address: ");
          print_sl_client_ip_address(&global_address);

          sl_ip_address_t gateway = { 0 };
          gateway.ip.v6           = wifi_ap_profile_6.ip.ip.v6.gateway;
          gateway.type            = SL_IPV6;
          LOG_PRINT("Gateway Address: ");
          print_sl_client_ip_address(&gateway);
        }

        status = sl_wifi_start_ap(SL_WIFI_AP_2_4GHZ_INTERFACE, &wifi_ap_profile_4.config);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nAp start fail: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nAP started\r\n");

#if HTTP_SERVER_ENABLE
        server_config.port            = HTTP_SERVER_PORT;
        server_config.default_handler = default_handler;
        server_config.handlers_list   = request_handlers;
        server_config.handlers_count  = 2;

        status = sl_http_server_init(&server_handle, &server_config);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nHTTP server init failed:%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nHttp Init done\r\n");

        status = sl_http_server_start(&server_handle);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nServer start fail:%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nServer start done\r\n");
#if HTTP_SERVER_RESPONSE
        is_server_running = true;
        while (is_server_running) { //Server is exited when we receive the credentials
          osDelay(100);
        }

        parse_json_response();

        SL_DEBUG_LOG("SSID stored:%s\r\n", WIFI_CLIENT_PROFILE_SSID);
        SL_DEBUG_LOG("Password stored:%s\r\n", WIFI_CLIENT_CREDENTIAL);
        SL_DEBUG_LOG("Security type stored:%s\r\n", WIFI_CLIENT_SECURITY_TYPE);

#endif
#endif

        is_provisioning = 0;
        app_cb          = DATA_TRANSFER_STATE;

        break;

      case DATA_TRANSFER_STATE:

        for (size_t i = 0; i < sizeof(data_buffer); i++)
          data_buffer[i] = 'A' + (i % 26);

        LOG_PRINT("\r\nSending UDP Data over IPv6 on STA VAP");
        send_data_to_udp_server();

        LOG_PRINT("\r\nReceving TCP Data over IPv4 on AP VAP");
        receive_data_from_tcp_client();

        if (app_cb == DATA_TRANSFER_STATE)
          app_cb = WAIT_STATE;

        break;

      case WAIT_STATE:
        break;

      case MODULE_DISCONNECT_STATE:

        //! Upon rejoin failure MODULE_DISCONNECT_STATE is called. Hence, pulling down HTTP server & AP
        status = sl_http_server_stop(&server_handle);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n Server stop fail:%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\n Server stop done\r\n");

        status = sl_http_server_deinit(&server_handle);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\n Server deinit fail:%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nServer deinit done\r\n");

        osDelay(2000);

        status = sl_net_down(SL_NET_WIFI_AP_INTERFACE);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nAp stop fail: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nAP Disconnected\r\n");

        app_cb = STA_BRINGUP_STATE;

        break;
    }
  }
  while (1) {
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    // Let the CPU go to sleep if the system allows it.
    sl_power_manager_sleep();
#else
    osDelay(osWaitForever);
#endif
  }
}

static sl_status_t ap_connected_event_handler(sl_wifi_event_t event,
                                              sl_status_t status_code,
                                              void *data,
                                              uint32_t data_length,
                                              void *arg)
{
  UNUSED_PARAMETER(data_length);
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    return status_code;
  }

  LOG_PRINT("Remote Client connected\n");
  print_client_mac_address((sl_mac_address_t *)data);
  if (is_provisioning)
    app_cb = PROVISION_STATE;

  return SL_STATUS_OK;
}

static sl_status_t ap_disconnected_event_handler(sl_wifi_event_t event,
                                                 sl_status_t status_code,
                                                 void *data,
                                                 uint32_t data_length,
                                                 void *arg)
{
  UNUSED_PARAMETER(data_length);
  UNUSED_PARAMETER(arg);

  if (SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
    return status_code;
  }

  LOG_PRINT("\r\nRemote Client disconnected\r\n");
  print_client_mac_address((sl_mac_address_t *)data);
  return SL_STATUS_OK;
}

sl_status_t login_request_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_server_response_t http_response = { 0 };

  LOG_PRINT("Got request %s with data length : %lu\n", req->uri.path, req->request_data_length);

  // Set the response code to 200 (OK)
  http_response.response_code = SL_HTTP_RESPONSE_OK;

  // Set the content type to plain text
  http_response.content_type = SL_HTTP_CONTENT_TYPE_TEXT_HTML;

  // Set the response data which is the connect page
  http_response.data                 = (uint8_t *)login_content;
  http_response.current_data_length  = strlen((const char *)login_content);
  http_response.expected_data_length = http_response.current_data_length;
  sl_http_server_send_response(handle, &http_response);
  return SL_STATUS_OK;
}

sl_status_t connect_data_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_recv_req_data_t recvData        = { 0 };
  sl_http_server_response_t http_response = { 0 };

  LOG_PRINT("Got request %s with data length : %lu\n", req->uri.path, req->request_data_length);
  if (req->request_data_length > 0) {
    recvData.request       = req;
    recvData.buffer        = (uint8_t *)response;
    recvData.buffer_length = 1024;

    sl_http_server_read_request_data(handle, &recvData);
    response[recvData.received_data_length] = 0;
    LOG_PRINT("Got request data as : %s\n", response);
  }

  // Set the response code to 200 (OK)
  http_response.response_code = SL_HTTP_RESPONSE_OK;

  // Set the content type to plain text
  http_response.content_type = SL_HTTP_CONTENT_TYPE_TEXT_HTML;

  // Set the response data as "Done"
  char *response_data                = "Done";
  http_response.data                 = (uint8_t *)response_data;
  http_response.current_data_length  = strlen(response_data);
  http_response.expected_data_length = http_response.current_data_length;
  sl_http_server_send_response(handle, &http_response);
  is_server_running = false;
  return SL_STATUS_OK;
}

sl_status_t default_handler(sl_http_server_t *handle, sl_http_server_request_t *req)
{
  sl_http_recv_req_data_t recvData        = { 0 };
  sl_http_server_response_t http_response = { 0 };
  sl_http_header_t header                 = { .key = "Server", .value = "SI917-HTTPServer" };

  LOG_PRINT("Got request %s with data length : %lu\n", req->uri.path, req->request_data_length);
  if (req->request_data_length > 0) {
    recvData.request       = req;
    recvData.buffer        = (uint8_t *)response;
    recvData.buffer_length = 1024;

    sl_http_server_read_request_data(handle, &recvData);
    response[recvData.received_data_length] = 0;
    LOG_PRINT("Got request data as : %s\n", response);
  }

  http_response.response_code = SL_HTTP_RESPONSE_NOT_FOUND;
  http_response.content_type  = SL_HTTP_CONTENT_TYPE_TEXT_PLAIN;
  http_response.headers       = &header;
  http_response.header_count  = 1;

  // Set the response data to "Hello, World!"
  char *response_data                = "Invalid request!";
  http_response.data                 = (uint8_t *)response_data;
  http_response.current_data_length  = strlen(response_data);
  http_response.expected_data_length = http_response.current_data_length;
  sl_http_server_send_response(handle, &http_response);

  return SL_STATUS_OK;
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start
      && strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

void parse_json_response()
{
  int i;
  int r;
  jsmn_parser p;

  jsmn_init(&p);
  r = jsmn_parse(&p, response, strlen(response), t, sizeof(t) / sizeof(t[0]));
  if (r < 0) {
    LOG_PRINT("Failed to parse JSON: %d\n", r);
    return;
  }

  /* Assume the top-level element is an object */
  if (r < 1 || t[0].type != JSMN_OBJECT) {
    LOG_PRINT("Object expected\n");
    return;
  }

  /* Loop over all keys of the root object */
  for (i = 1; i < r; i++) {
    if (jsoneq(response, &t[i], "id") == 0) {
      /* We may use strndup() to fetch string value */
      SL_DEBUG_LOG("- SSID: %.*s\n", t[i + 1].end - t[i + 1].start, response + t[i + 1].start);
      snprintf(WIFI_CLIENT_PROFILE_SSID,
               sizeof(WIFI_CLIENT_PROFILE_SSID),
               "%.*s",
               t[i + 1].end - t[i + 1].start,
               response + t[i + 1].start);
      i++;
    } else if (jsoneq(response, &t[i], "password") == 0) {
      /* We may additionally check if the value is either "true" or "false" */
      SL_DEBUG_LOG("- Password: %.*s\n", t[i + 1].end - t[i + 1].start, response + t[i + 1].start);
      snprintf(WIFI_CLIENT_CREDENTIAL,
               sizeof(WIFI_CLIENT_CREDENTIAL),
               "%.*s",
               t[i + 1].end - t[i + 1].start,
               response + t[i + 1].start);
      i++;
    } else if (jsoneq(response, &t[i], "securityType") == 0) {
      /* We may want to do strtol() here to get numeric value */
      SL_DEBUG_LOG("- Security Type: %.*s\n", t[i + 1].end - t[i + 1].start, response + t[i + 1].start);
      snprintf(WIFI_CLIENT_SECURITY_TYPE,
               sizeof(WIFI_CLIENT_SECURITY_TYPE),
               "%.*s",
               t[i + 1].end - t[i + 1].start,
               response + t[i + 1].start);
      i++;
    } else {
      LOG_PRINT("Unexpected key: %.*s\n", t[i].end - t[i].start, response + t[i].start);
    }
  }
}

sl_wifi_security_t string_to_security_type(const char *securityType)
{
  if (strcmp(securityType, "WPA") == 0) {
    return SL_WIFI_WPA;
  } else if (strcmp(securityType, "WPA2") == 0) {
    return SL_WIFI_WPA2;
  } else if (strcmp(securityType, "WPA3") == 0) {
    return SL_WIFI_WPA3;
  } else if (strcmp(securityType, "Mixed Mode") == 0) {
    return SL_WIFI_WPA_WPA2_MIXED;
  } else {
    return SL_WIFI_SECURITY_UNKNOWN;
  }
}

void receive_data_from_tcp_client(void)
{
  sl_status_t status                = SL_STATUS_OK;
  int server_socket                 = -1;
  int client_socket                 = -1;
  uint32_t start                    = 0;
  int socket_return_value           = 0;
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);
  uint8_t high_performance_socket   = HIGH_PERFORMANCE_SOCKET;

  int read_bytes                = 1;
  uint32_t total_bytes_received = 0;

  server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_socket < 0) {
    LOG_PRINT("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }
  LOG_PRINT("\r\nServer Socket ID : %d\r\n", server_socket);

  socket_return_value = setsockopt(server_socket,
                                   SOL_SOCKET,
                                   SL_SO_HIGH_PERFORMANCE_SOCKET,
                                   &high_performance_socket,
                                   sizeof(high_performance_socket));
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSet Socket option failed with bsd error: %d\r\n", errno);
    close(client_socket);
    return;
  }

  socket_return_value =
    sl_si91x_setsockopt(server_socket, SOL_SOCKET, SL_SI91X_SO_SOCK_VAP_ID, &ap_vap, sizeof(ap_vap));

  server_address.sin_family = AF_INET;
  server_address.sin_port   = LISTENING_PORT;

  socket_return_value = bind(server_socket, (struct sockaddr *)&server_address, socket_length);
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSocket bind failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }

  socket_return_value = listen(server_socket, BACK_LOG);
  if (socket_return_value < 0) {
    LOG_PRINT("\r\nSocket listen failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }
  LOG_PRINT("\r\nListening on Local Port : %d\r\n", LISTENING_PORT);

  client_socket = accept(server_socket, NULL, NULL);
  if (client_socket < 0) {
    LOG_PRINT("\r\nSocket accept failed with bsd error: %d\r\n", errno);
    close(server_socket);
    return;
  }
  LOG_PRINT("\r\nClient Socket ID : %d\r\n", client_socket);

  LOG_PRINT("\r\nTCP_RX Throughput test start\r\n");
  start = osKernelGetTickCount();
  while (1) {
    read_bytes = recv(client_socket, data_buffer, sizeof(data_buffer), 0);
    if (read_bytes < 0) {
      if (errno == 0) {
        // get the error code returned by the firmware
        status = sl_wifi_get_saved_firmware_status();
        if (status == SL_STATUS_SI91X_MEMORY_FAILED_FROM_MODULE) {
          continue;
        } else {
          printf("\r\nrecv failed with BSD error = %d and status = 0x%lx\r\n", errno, status);
        }
      } else {
        printf("\r\nrecv failed with BSD error = %d\r\n", errno);
      }
      break;
    }
    total_bytes_received = total_bytes_received + read_bytes;
    now                  = osKernelGetTickCount();

    if ((now - start) > TEST_TIMEOUT) {
      LOG_PRINT("\r\nTest Time Out: %ld ms\r\n", (now - start));
      break;
    }
  }
  LOG_PRINT("\r\nTCP_RX Throughput test finished\r\n");
  LOG_PRINT("\r\nTotal bytes received : %ld\r\n", total_bytes_received);

  close(client_socket);
  close(server_socket);
}

void send_data_to_udp_server(void)
{
  int client_socket                   = -1;
  uint32_t total_bytes_sent           = 0;
  struct sockaddr_in6 server_address6 = { 0 };
  socklen_t socket_length             = sizeof(struct sockaddr_in6);
  int sent_bytes                      = 1;
  uint32_t start                      = 0;
  uint32_t now                        = 0;

  client_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
  if (client_socket < 0) {
    LOG_PRINT("\r\nSocket creation failed with bsd error: %d\r\n", errno);
    return;
  }
  LOG_PRINT("\r\nSocket ID : %d\r\n", client_socket);

  server_address6.sin6_family = AF_INET6;
  server_address6.sin6_port   = SERVER_PORT;

  int status = sl_inet_pton6(SERVER_IP,
                             SERVER_IP + strlen(SERVER_IP),
                             address_buffer,
                             (unsigned int *)server_address6.sin6_addr.__u6_addr.__u6_addr32);
  if (status != 0x1) {
    LOG_PRINT("\r\nIPv6 conversion failed.\r\n");
    return;
  }

  LOG_PRINT("\r\nUDP_TX Throughput test start\r\n");
  start = osKernelGetTickCount();
  while (total_bytes_sent < BYTES_TO_SEND) {
    sent_bytes =
      sendto(client_socket, data_buffer, UDP_BUFFER_SIZE, 0, (struct sockaddr *)&server_address6, socket_length);
    now = osKernelGetTickCount();

    if ((now - start) > TEST_TIMEOUT) {
      LOG_PRINT("\r\nTime Out: %ld\r\n", (now - start));
      break;
    }
    if (sent_bytes < 0) {
      if (errno == ENOBUFS)
        continue;
      LOG_PRINT("\r\nSocket send failed with bsd error: %d\r\n", errno);
      close(client_socket);
      break;
    }
    total_bytes_sent = total_bytes_sent + sent_bytes;
  }
  LOG_PRINT("\r\nUDP_TX Throughput test finished\r\n");
  LOG_PRINT("\r\nTotal bytes sent : %ld\r\n", total_bytes_sent);

  close(client_socket);
}

void sl_fw_task_up()
{
  sl_status_t status                          = SL_STATUS_OK;
  sl_wifi_firmware_version_t firmware_version = { 0 };

  fw_update_status(false);
  status = sl_wifi_get_firmware_version(&firmware_version);
  if (status != SL_STATUS_OK) {
    LOG_PRINT("\r\nFailed to fetch firmware version: 0x%lx\r\n", status);
    return;
  } else {
    LOG_PRINT("\r\nFirmware version is: %x%x.%d.%d.%d.%d.%d.%d\r\n",
              firmware_version.chip_id,
              firmware_version.rom_id,
              firmware_version.major,
              firmware_version.minor,
              firmware_version.security_version,
              firmware_version.patch_num,
              firmware_version.customer_id,
              firmware_version.build_num);
  }

  //! Create separate thread to handle firmware update
  fw_up_thread_id = osThreadNew((osThreadFunc_t)fw_up_configurator_task, NULL, &fw_up_thread_attributes);
  if (fw_up_thread_id == NULL) {
    LOG_PRINT("Failed to create Firmware update thread\n");
    return;
  }
}
