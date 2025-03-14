#pragma once

#include <sl_wifi_device.h>

extern const sl_wifi_device_configuration_t wifi_config_client;

// default wifi STA IP address
#define STA_IP_ADDR0_DEFAULT      192
#define STA_IP_ADDR1_DEFAULT      168
#define STA_IP_ADDR2_DEFAULT      10
#define STA_IP_ADDR3_DEFAULT      100
// default wifi STA netmask
#define STA_NETMASK_ADDR0_DEFAULT 255
#define STA_NETMASK_ADDR1_DEFAULT 255
#define STA_NETMASK_ADDR2_DEFAULT 255
#define STA_NETMASK_ADDR3_DEFAULT 0
// default wifi STA gateway
#define STA_GW_ADDR0_DEFAULT      192
#define STA_GW_ADDR1_DEFAULT      168
#define STA_GW_ADDR2_DEFAULT      10
#define STA_GW_ADDR3_DEFAULT      1
