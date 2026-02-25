/**
 * @brief Si917 info client
 */

#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SI917_NWP_VERSION_STR_LEN_MAX (4 * 7 + 5)

typedef struct {
    uint8_t wifi_mac[6];
    uint8_t ble_mac[6];
    char nwp_version[SI917_NWP_VERSION_STR_LEN_MAX + 1];
} Si917InfoData;

typedef struct Si917InfoClient Si917InfoClient;

#define RECORD_SI917_INFO_CLIENT "si917_info"

bool si917_info_get(Si917InfoClient* client, Si917InfoData* info);

#ifdef __cplusplus
}
#endif
