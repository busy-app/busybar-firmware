#pragma once

#include "wifi_i.h"

/*
 * Variable arguments:
 * - WifiStateConnecting -> WifiStateConnected:
 *   - const WifiCredentials*, const WifiIpConfig*
 * - WifiStateReconnecting -> WifiStateConnected:
 *   - const WifiIpConfig*
 * no variable arguments for any other state transition
 */
void wifi_state_transition(Wifi* instance, WifiState new_state, ...);

void wifi_state_update_backend_info(Wifi* instance, const WifiBackendInfo* backend_info);

WifiStatus wifi_state_check_request_type(Wifi* instance, WifiRequestType request_type);
