#include "wifi_state.h"

static void wifi_state_reset_info(WifiInfo* info) {
    memset(info->ssid, 0, SSID_MAX_LEN);
    memset(&info->ip_config, 0, sizeof(info->ip_config));

    info->security_mode = WifiSecurityModeOpen;
    info->channel = 0;
    info->rssi = -99;
}

void wifi_state_transition(Wifi* instance, WifiState new_state, ...) {
    va_list args;
    va_start(args, new_state);

    WifiInfo* info = &instance->info;
    const WifiState current_state = info->state;

    if(current_state == WifiStateUnknown) {
        if(new_state == WifiStateDisconnected) {
            wifi_state_reset_info(info);
            info->bssid = *(va_arg(args, const WifiHardwareAddress*));
        } else {
            furi_crash("Invalid transition from WifiStateUnknown");
        }

    } else if(current_state == WifiStateDisconnected) {
        if(new_state == WifiStateConnecting) {
            /* Nothing */
        } else {
            furi_crash("Invalid transition from WifiStateDisconnected");
        }

    } else if(current_state == WifiStateConnecting) {
        if(new_state == WifiStateConnected) {
            const WifiCredentials* credentials = va_arg(args, const WifiCredentials*);
            const WifiIpConfig* ip_config = va_arg(args, const WifiIpConfig*);

            strncpy(info->ssid, credentials->ssid, SSID_MAX_LEN);
            info->security_mode = credentials->security_mode;
            info->ip_config = *ip_config;

        } else if(new_state == WifiStateDisconnected) {
            /* Nothing */
        } else {
            furi_crash("Invalid transition from WifiStateConnecting");
        }

    } else if(current_state == WifiStateConnected) {
        if(new_state == WifiStateDisconnecting) {
            /* Nothing */
        } else {
            furi_crash("Invalid transition from WifiStateConnected");
        }

    } else if(current_state == WifiStateDisconnecting) {
        if(new_state == WifiStateDisconnected) {
            wifi_state_reset_info(info);
        } else {
            furi_crash("Invalid transition from WifiStateDisconnecting");
        }

    } else {
        furi_crash("Invalid WifiState value");
    }

    info->state = new_state;

    va_end(args);
}
