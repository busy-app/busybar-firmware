#include "wifi_state.h"

static void wifi_state_reset_info(WifiInfo* info) {
    memset(info, 0, sizeof(WifiInfo));
    info->rssi = -99;
}

void wifi_state_transition(Wifi* instance, WifiState new_state, ...) {
    with_furi_state(instance->state, WifiInfo * info, {
        va_list args;
        va_start(args, new_state);

        const WifiState current_state = info->state;

        if(current_state == WifiStateUnknown) {
            if(new_state == WifiStateDisconnected) {
                wifi_state_reset_info(info);
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

        va_end(args);

        info->state = new_state;
    });
}

void wifi_state_update_backend_info(Wifi* instance, const WifiBackendInfo* backend_info) {
    with_furi_state(instance->state, WifiInfo * info, {
        memcpy(info->bssid, backend_info->bssid, HW_ADDRESS_LEN);
        info->channel = backend_info->channel;
        info->rssi = backend_info->rssi;
    });
}

WifiStatus wifi_state_check_request_type(Wifi* instance, WifiRequestType request_type) {
    WifiStatus status = WifiStatusOk;

    with_furi_state(instance->state, WifiInfo * info, {
        const WifiState current_state = info->state;

        if(request_type == WifiRequestTypeInit) {
            if(current_state != WifiStateUnknown) {
                status = WifiStatusError;
            }
        } else if(request_type == WifiRequestTypeScan) {
            if(current_state != WifiStateDisconnected) {
                status = WifiStatusScanNotPossible;
            }
        } else if(request_type == WifiRequestTypeConnect) {
            if(current_state != WifiStateDisconnected) {
                status = WifiStatusAlreadyConnected;
            }
        } else if(request_type == WifiRequestTypeDisconnect) {
            if(current_state != WifiStateConnected) {
                status = WifiStatusAlreadyDisconnected;
            }
        } else if(request_type == WifiRequestTypeBackendInfo) {
            if(current_state != WifiStateConnected) {
                status = WifiStatusError;
            }
        } else {
            furi_crash("Invalid WifiRequestType");
        }
    });

    return status;
}
