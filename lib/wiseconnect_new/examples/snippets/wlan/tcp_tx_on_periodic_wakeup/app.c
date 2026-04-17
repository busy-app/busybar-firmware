/***************************************************************************/ /**
 * @file
 * @brief TCP tx on Periodic Wakeup Application
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

#include <string.h>
#include "cmsis_os2.h"
#include "sl_status.h"
#include "sl_net.h"
#include "sl_wifi.h"
#include "sl_wifi_device.h"
#include "sl_net_wifi_types.h"
#include "sl_utility.h"
#include "sl_si91x_driver.h"

#include "sl_board_configuration.h"
#include "errno.h"
#include "socket.h"
#include "sl_net_si91x.h"
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_core_utilities.h"

#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_m4_ps.h"
#include "sl_si91x_driver_gpio.h"
#endif

/******************************************************
 *                    Constants
 ******************************************************/

#define ENABLE_NWP_POWER_SAVE 1

/******************************************************
 *                      Macros
 ******************************************************/

// Memory length for send buffer
#define TCP_BUFFER_SIZE 1460
#define BUFFER_SIZE     TCP_BUFFER_SIZE

#define SERVER_IP "192.168.0.206"

#define SERVER_PORT 5001

#define BYTES_TO_SEND (1 << 29) //512MB
#define TEST_TIMEOUT  (10000)   //10sec

#define TX_POOL_RATIO     1
#define RX_POOL_RATIO     1
#define GLOBAL_POOL_RATIO 1

#define ALARM_TIMEOUT 30000 //! Publish periodicity in milliseconds
/******************************************************
 *               Function Declarations
 ******************************************************/
void send_data_to_tcp_server(void);
static void application_start(void *argument);

/******************************************************
 *               Variable Definitions
 ******************************************************/

osSemaphoreId_t button_semaphore;

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
uint8_t data_buffer[BUFFER_SIZE];
sl_ip_address_t ip_address           = { 0 };
sl_net_wifi_client_profile_t profile = { 0 };

static const sl_wifi_device_configuration_t sl_wifi_throughput_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                   .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map =
#ifdef SLI_SI91X_MCU_INTERFACE
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE),
#else
                     (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION
#if ENABLE_NWP_POWER_SAVE
                      | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
#endif
                      ),
#endif
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT
                                              | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                   .custom_feature_bit_map = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS
                      | SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE | MEMORY_CONFIG
#ifdef SLI_SI917
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      ),
                   .bt_feature_bit_map = 0,
                   .ext_tcp_ip_feature_bit_map =
                     (SL_SI91X_EXT_TCP_IP_WINDOW_SCALING | SL_SI91X_EXT_TCP_IP_TOTAL_SELECTS(1)
                      | SL_SI91X_CONFIG_FEAT_EXTENTION_VALID | SL_SI91X_EXT_TCP_IP_BI_DIR_ACK_UPDATE),
                   .ble_feature_bit_map     = 0,
                   .ble_ext_feature_bit_map = 0,
#if defined(SLI_SI91X_MCU_INTERFACE) || ENABLE_NWP_POWER_SAVE
                   .config_feature_bit_map = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP | SL_WIFI_ENABLE_ENHANCED_MAX_PSP)
#else
                   .config_feature_bit_map = 0
#endif
  },
  .ta_pool = { .tx_ratio_in_buffer_pool     = TX_POOL_RATIO,
               .rx_ratio_in_buffer_pool     = RX_POOL_RATIO,
               .global_ratio_in_buffer_pool = GLOBAL_POOL_RATIO }
};

/******************************************************
 *               Function Definitions
 ******************************************************/

#if defined(SLI_SI91X_MCU_INTERFACE) && (SL_SI91X_TICKLESS_MODE == ENABLE)
void gpio_uulp_pin_interrupt_callback(uint32_t pin_intr)
{
  while (sl_si91x_gpio_get_uulp_npss_pin(pin_intr) == LOW)
    ; // waiting for the button release
  osSemaphoreRelease(button_semaphore);
}
#endif

void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;
  button_semaphore = osSemaphoreNew(1, 0, NULL);

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_throughput_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to start Wi-Fi Client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client interface init success\r\n");

#ifdef SLI_SI91X_MCU_INTERFACE
  uint8_t xtal_enable = 1;
  status              = sl_si91x_m4_ta_secure_handshake(SL_SI91X_ENABLE_XTAL, 1, &xtal_enable, 0, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to bring m4_ta_secure_handshake: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nm4_ta_secure_handshake Success\r\n");
#endif

  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to connect to AP: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi client connected\r\n");

  status = sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &profile);
  if (status != SL_STATUS_OK) {
    printf("Failed to get client profile: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nSuccess to get client profile\r\n");

  ip_address.type = SL_IPV4;
  memcpy(&ip_address.ip.v4.bytes, &profile.ip.ip.v4.ip_address.bytes, sizeof(sl_ipv4_address_t));
  print_sl_ip_address(&ip_address);

  for (size_t i = 0; i < sizeof(data_buffer); i++)
    data_buffer[i] = 'A' + (i % 26);

  send_data_to_tcp_server();
}

void send_data_to_tcp_server(void)
{
  int client_socket                 = -1;
  uint32_t total_bytes_sent         = 0;
  int socket_return_value           = 0;
  int sent_bytes                    = 1;
  uint32_t start                    = 0;
  struct sockaddr_in server_address = { 0 };
  socklen_t socket_length           = sizeof(struct sockaddr_in);
  sl_status_t rc                    = SL_STATUS_FAIL;

  server_address.sin_family = AF_INET;
  server_address.sin_port   = SERVER_PORT;
  sl_net_inet_addr(SERVER_IP, &server_address.sin_addr.s_addr);

  client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (client_socket < 0) {
    printf("\r\nSocket Create failed with bsd error: %d\r\n", errno);
    return;
  }
  printf("\r\nSocket ID : %d\r\n", client_socket);

  socket_return_value = connect(client_socket, (struct sockaddr *)&server_address, socket_length);
  if (socket_return_value < 0) {
    printf("\r\nSocket Connect failed with bsd error: %d\r\n", errno);
    close(client_socket);
    return;
  }
  printf("\r\nSocket connected to TCP server\r\n");

  printf("\r\nTCP_TX Throughput test start\r\n");

  sl_wifi_performance_profile_v2_t performance_profile = { .profile         = ASSOCIATED_POWER_SAVE_LOW_LATENCY,
                                                           .listen_interval = 1000 };
#if ENABLE_NWP_POWER_SAVE
  rc = sl_wifi_set_performance_profile_v2(&performance_profile);
  if (rc != SL_STATUS_OK) {
    printf("\r\nPower save configuration Failed, Error Code : 0x%lX\r\n", rc);
  } else {
    printf("\r\nAssociated Power Save Enabled\n");
  }
#endif

  while (1) {
    start = osKernelGetTickCount();
    while (total_bytes_sent < BYTES_TO_SEND) {
      sent_bytes = send(client_socket, data_buffer, TCP_BUFFER_SIZE, 0);
      if (sent_bytes < 0) {
        if (errno == ENOBUFS)
          continue;
        printf("\r\nTCP send failed with BSD error = %d\r\n", errno);
        break;
      }
      total_bytes_sent = total_bytes_sent + sent_bytes;

      if ((osKernelGetTickCount() - start) > TEST_TIMEOUT) {
        printf("\r\nData transfer has completed\r\n");
        break;
      }
    }
    total_bytes_sent = 0;

#if ENABLE_NWP_POWER_SAVE
#ifdef SLI_SI91X_MCU_INTERFACE
    osStatus_t semaphore_status = osSemaphoreAcquire(button_semaphore, ALARM_TIMEOUT);
    if (semaphore_status == osOK) {
      printf("\rM4 wake up button pressed\r\n");
    } else if (semaphore_status == osErrorTimeout) {
      printf("\rM4 alarm based wakeup\r\n");
    }
#endif
#endif
  }
  printf("\r\nTCP_TX test finished\r\n");

  close(client_socket);
}
