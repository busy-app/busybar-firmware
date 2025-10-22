/**
 * @file wifi.h
 * @brief API for controlling WiFi networks.
 *
 * All of the below functions are synchronous (will block the calling thread until completion).
 * Additionally, the Wifi system only serves one thread at a time, so attempting to access it from
 * multiple threads will put them in a blocked state until the previous thread is done with it.
 */
#pragma once

#include "wifi_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief TBD
 *
 * @param[in,out] instance pointer to the Wifi instance
 * @returns TBD
 */
FuriState* wifi_get_state(Wifi* instance);

/**
 * @brief Scan for available Wifi access points nearby.
 *
 * The array pointed to by the results parameter MUST be allocated by the user code.
 * Naturally, it is also responsible for freeing the array when it is no longer needed.
 *
 * @note Scanning is only possible when the Wifi is disconnected from a network.
 *
 * @param[in,out] instance pointer to the Wifi instance
 * @param[out] results pointer to the array to contain the scan results
 * @param[out] result_count pointer to the variable to contain the result count
 * @param[in] max_result_count maximum capacity of the scan results array
 * @returns WifiStatusOk on success, error code otherwise
 */
WifiStatus wifi_scan(
    Wifi* instance,
    WifiScanResult* results,
    uint8_t* result_count,
    uint8_t max_result_count);

/**
 * @brief Connect to an access point using the specified configuration.
 *
 * @param[in,out] instance pointer to the Wifi instance
 * @param[in] credentials pointer to the structure containing the connection credentials
 * @param[in] ip_config pointer to the structure containing the connection configuration
 * @returns WifiStatusOk on success, error code otherwise
 */
WifiStatus
    wifi_connect(Wifi* instance, const WifiCredentials* credentials, const WifiIpConfig* ip_config);

/**
 * @brief Disconnect from the access point.
 *
 * @param[in,out] instance pointer to the Wifi instance
 * @returns WifiStatusOk on success, error code otherwise
 */
WifiStatus wifi_disconnect(Wifi* instance);

/**
 * @brief Get the information about current state of the Wifi system.
 *
 * @param[in,out] instance pointer to the Wifi instance
 * @param[out] info pointer to the structure to contain the information
 * @returns WifiStatusOk on success, error code otherwise
 */
WifiStatus wifi_get_info(Wifi* instance, WifiInfo* info);

#ifdef __cplusplus
}
#endif
