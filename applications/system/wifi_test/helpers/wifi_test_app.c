#include "wifi_test_app.h"
#include "wifi_scan.h"
#include "wifi_async_socket_server_tcp_rx.h"
#include "wifi_async_socket_client_tcp_tx.h"
#include "wifi_async_socket_server_echo.h"

#include <furi.h>

#include <sl_status.h>
#include <sl_wifi.h>
#include <sl_net_wifi_types.h>
#include <sl_net.h>
#include <sl_si91x_driver.h>
#include <sl_wifi_callback_framework.h>

#include <args.h>
#include <strint.h>

#define TAG                 "WifiTestApp"
#define WIFI_TEST_SERVER_IP "192.168.10.2"

#define CHANNEL_NUMBER 6

#define WIFI_AP_PROFILE_SSID "FlipperBsbAp"
#define WIFI_AP_CREDENTIAL   "12345678"

#define WIFI_CLIENT_PROFILE_SSID    "Zyxel24"
#define WIFI_CLIENT_CREDENTIAL      "1qa2wszz"
#define WIFI_CLIENT_SECURITY_TYPE   SL_WIFI_WPA_WPA2_MIXED
#define WIFI_CLIENT_ENCRYPTION_TYPE SL_WIFI_CCMP_ENCRYPTION

//! IP address of the module
//! E.g: 0x0A0AA8C0 == 192.168.11.10
#define DEFAULT_WIFI_MODULE_IP_ADDRESS 0x0A0BA8C0

//! IP address of netmask
//! E.g: 0x00FFFFFF == 255.255.255.0
#define DEFAULT_WIFI_SN_MASK_ADDRESS 0x00FFFFFF

//! IP address of Gateway
//! E.g: 0x0A0AA8C0 == 192.168.11.10
#define DEFAULT_WIFI_GATEWAY_ADDRESS 0x0A0BA8C0

static const sl_net_wifi_ap_profile_t wifi_ap_profile = {
    .config =
        {
            .ssid.value = WIFI_AP_PROFILE_SSID,
            .ssid.length = sizeof(WIFI_AP_PROFILE_SSID) - 1,
            .channel.channel = CHANNEL_NUMBER, //SL_WIFI_AUTO_CHANNEL,
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
            .is_11n_enabled = 0,
        },

    .ip =
        {
            .mode = SL_IP_MANAGEMENT_STATIC_IP,
            .type = SL_IPV4,
            .host_name = NULL,
            .ip =
                {
                    .v4.ip_address.value = DEFAULT_WIFI_MODULE_IP_ADDRESS,
                    .v4.gateway.value = DEFAULT_WIFI_GATEWAY_ADDRESS,
                    .v4.netmask.value = DEFAULT_WIFI_SN_MASK_ADDRESS,
                },
        },
};

static const sl_net_wifi_psk_credential_entry_t wifi_ap_credential = {
    .type = SL_NET_WIFI_PSK,
    .data_length = sizeof(WIFI_AP_CREDENTIAL) - 1,
    .data = WIFI_AP_CREDENTIAL,
};

static const sl_net_wifi_client_profile_t wifi_client_profile = {
    .config =
        {
            .ssid.value = WIFI_CLIENT_PROFILE_SSID,
            .ssid.length = sizeof(WIFI_CLIENT_PROFILE_SSID) - 1,
            .channel.channel = SL_WIFI_AUTO_CHANNEL,
            .channel.band = SL_WIFI_AUTO_BAND,
            .channel.bandwidth = SL_WIFI_AUTO_BANDWIDTH,
            .bssid = {{0}},
            .bss_type = SL_WIFI_BSS_TYPE_INFRASTRUCTURE,
            .security = WIFI_CLIENT_SECURITY_TYPE,
            .encryption = WIFI_CLIENT_ENCRYPTION_TYPE,
            .client_options = 0,
            .credential_id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID,
        },
    .ip = {
        .mode = SL_IP_MANAGEMENT_DHCP,
        .type = SL_IPV4,
        .host_name = NULL,
        .ip = {{{0}}},

    }};

static const sl_net_wifi_psk_credential_entry_t wifi_client_credential = {
    .type = SL_NET_WIFI_PSK,
    .data_length = sizeof(WIFI_CLIENT_CREDENTIAL) - 1,
    .data = WIFI_CLIENT_CREDENTIAL};

typedef enum {
    WifiTestCmdTypeHelp,
    WifiTestCmdTypeHelpHelp,
    WifiTestCmdTypeScan,
    WifiTestCmdTypeApUp,
    WifiTestCmdTypeApDown,
    WifiTestCmdTypeStaUp,
    WifiTestCmdTypeStaDown,
    WifiTestCmdTypeTestTcpRx,
    WifiTestCmdTypeTestTcpTx,
    WifiTestCmdTypeTestEcho,

    WifiTestCmdTypeMax,
} WifiTestCmdType;

typedef enum {
    WifiTestStateIdle,
    //WifiTestStateDriverInit,
    WifiTestStateApUp,
    WifiTestStateStaUp,
} WifiTestState;

typedef struct {
    char* cmd;
} WifiTestCmd;

const WifiTestCmd wifi_test_cmd[WifiTestCmdTypeMax] = {
    {"?"},
    {"help"},
    {"scan"},
    {"ap_up"},
    {"ap_down"},
    {"sta_up"},
    {"sta_down"},
    {"test_tcp_rx"},
    {"test_tcp_tx"},
    {"test_echo"}};

struct WifiTestApp {
    FuriString* msg;
    CliWorker* worker;
    WifiTestState state;

    bool exit;
    bool ap_client_connected;
};

static void wifi_test_app_cmd_usage(WifiTestApp* instance);

static void wifi_test_app_send_msg(WifiTestApp* instance) {
    cli_worker_add_rx_data(
        instance->worker,
        (uint8_t*)furi_string_get_cstr(instance->msg),
        furi_string_utf8_length(instance->msg));
}

void wifi_test_app_send_text(WifiTestApp* instance, FuriString* text) {
    cli_worker_add_rx_data(
        instance->worker, (uint8_t*)furi_string_get_cstr(text), furi_string_utf8_length(text));
}

static void wifi_test_app_send_msg_invalid_arg(WifiTestApp* instance) {
    furi_string_printf(instance->msg, "Invalid argument\r\n");
    wifi_test_app_send_msg(instance);
}

static sl_status_t wifi_test_app_connected_event_handler(
    sl_wifi_event_t event,
    void* data,
    uint32_t data_length,
    void* arg) {
    UNUSED_PARAMETER(data_length);
    UNUSED_PARAMETER(event);
    UNUSED_PARAMETER(data);

    WifiTestApp* instance = (WifiTestApp*)arg;
    sl_mac_address_t* mac = (sl_mac_address_t*)data;
    furi_string_printf(
        instance->msg,
        "Remote Client connected:  %2X:%2X:%2X:%2X:%2X:%2X\r\n",
        mac->octet[0],
        mac->octet[1],
        mac->octet[2],
        mac->octet[3],
        mac->octet[4],
        mac->octet[5]);
    wifi_test_app_send_msg(instance);

    instance->ap_client_connected = true;
    return SL_STATUS_OK;
}

static sl_status_t wifi_test_app_disconnected_event_handler(
    sl_wifi_event_t event,
    void* data,
    uint32_t data_length,
    void* arg) {
    UNUSED_PARAMETER(data_length);
    UNUSED_PARAMETER(event);
    UNUSED_PARAMETER(data);

    WifiTestApp* instance = (WifiTestApp*)arg;
    sl_mac_address_t* mac = (sl_mac_address_t*)data;
    furi_string_printf(
        instance->msg,
        "Remote Client disconnect:  %2X:%2X:%2X:%2X:%2X:%2X\r\n",
        mac->octet[0],
        mac->octet[1],
        mac->octet[2],
        mac->octet[3],
        mac->octet[4],
        mac->octet[5]);
    wifi_test_app_send_msg(instance);
    instance->ap_client_connected = false;
    return SL_STATUS_OK;
}

void* wifi_test_app_start(CliWorker* worker) {
    FURI_LOG_I(TAG, "Starting");

    WifiTestApp* instance = malloc(sizeof(WifiTestApp));
    instance->msg = furi_string_alloc();
    instance->worker = worker;
    instance->state = WifiTestStateIdle;

    instance->exit = false;
    instance->ap_client_connected = false;

    sl_status_t status = SL_STATUS_FAIL;
    do {
        // Initialize Wi-Fi APSTA interface
        status = sl_net_init(
            SL_NET_WIFI_AP_INTERFACE, &sl_wifi_default_concurrent_configuration, NULL, NULL);
        if(status != SL_STATUS_OK) {
            furi_string_printf(
                instance->msg, "Failed to start Wi-Fi APSTA interface: 0x%lx\r\n", status);
            wifi_test_app_send_msg(instance);
            break;
        }

        furi_string_printf(instance->msg, "Wi-Fi APSTA interface init\r\n");
        wifi_test_app_send_msg(instance);

        wifi_test_app_cmd_usage(instance);
    } while(0);

    if(status != SL_STATUS_OK) {
        wifi_test_app_stop(instance);
        return NULL;
    }
    return (void*)instance;
}

void wifi_test_app_stop(void* app_handle) {
    furi_check(app_handle);
    FURI_LOG_I(TAG, "Stopping");
    WifiTestApp* instance = (WifiTestApp*)app_handle;

    if(instance) {
        instance->exit = true;
        furi_delay_ms(100);
        if(instance->state == WifiTestStateApUp) {
            sl_net_down(SL_NET_WIFI_AP_INTERFACE);
        } else if(instance->state == WifiTestStateStaUp) {
            sl_net_down(SL_NET_WIFI_CLIENT_INTERFACE);
        }

        furi_string_free(instance->msg);
        free(instance);
        instance = NULL;
    }

    sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
}

static sl_status_t wifi_test_app(WifiTestApp* instance, uint8_t cmd_index, FuriString* args) {
    sl_status_t status = SL_STATUS_FAIL;

    char* args_cstr = (char*)furi_string_get_cstr(args);
    UNUSED(args_cstr);
    FuriString* arg = furi_string_alloc();

    switch(cmd_index) {
    case WifiTestCmdTypeHelp:
    case WifiTestCmdTypeHelpHelp:
        wifi_test_app_cmd_usage(instance);
        break;
    case WifiTestCmdTypeScan:
        if(instance->state == WifiTestStateIdle) {
            status = wifi_scan(instance->msg);
            if(status != SL_STATUS_OK) {
                furi_string_printf(instance->msg, "Scan failed\r\n");
                wifi_test_app_send_msg(instance);
            } else {
                wifi_test_app_send_msg(instance);
            }
        } else {
            furi_string_printf(instance->msg, "AP is up, scan not allowed\r\n");
            wifi_test_app_send_msg(instance);
        }
        break;
    case WifiTestCmdTypeApUp:
        if(instance->state == WifiTestStateIdle) {
            // Set Wi-Fi callbacks
            sl_wifi_set_callback(
                SL_WIFI_CLIENT_CONNECTED_EVENTS, wifi_test_app_connected_event_handler, instance);
            sl_wifi_set_callback(
                SL_WIFI_CLIENT_DISCONNECTED_EVENTS,
                wifi_test_app_disconnected_event_handler,
                instance);

            // Set Wi-Fi AP profile
            status = sl_net_set_profile(
                SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1, &wifi_ap_profile);
            if(status != SL_STATUS_OK) {
                furi_string_printf(instance->msg, "Failed to set AP profile: 0x%lx\r\n", status);
                wifi_test_app_send_msg(instance);
                break;
            }
            furi_string_printf(instance->msg, "Success to set AP profile\r\n");
            wifi_test_app_send_msg(instance);

            // Set Wi-Fi AP credential
            status = sl_net_set_credential(
                SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID,
                wifi_ap_credential.type,
                &wifi_ap_credential.data,
                wifi_ap_credential.data_length);
            if(status != SL_STATUS_OK) {
                furi_string_printf(instance->msg, "Failed to set credentials: 0x%lx\r\n", status);
                wifi_test_app_send_msg(instance);
                break;
            }
            furi_string_printf(instance->msg, "Wi-Fi set credential success\r\n");
            wifi_test_app_send_msg(instance);

            // Bring Wi-Fi AP interface up
            status = sl_net_up(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1);
            if(status != SL_STATUS_OK) {
                furi_string_printf(
                    instance->msg, "Failed to bring Wi-Fi AP interface up: 0x%lx\r\n", status);
                wifi_test_app_send_msg(instance);
                break;
            }
            furi_string_printf(instance->msg, "AP started\r\n");
            wifi_test_app_send_msg(instance);
            instance->state = WifiTestStateApUp;
        } else {
            furi_string_printf(instance->msg, "AP or STA is already up\r\n");
            wifi_test_app_send_msg(instance);
        }
        break;
    case WifiTestCmdTypeApDown:
        if(instance->state == WifiTestStateApUp) {
            status = sl_net_down(SL_NET_WIFI_AP_INTERFACE);
            if(status != SL_STATUS_OK) {
                furi_string_printf(
                    instance->msg, "Failed to bring Wi-Fi AP interface down: 0x%lx\r\n", status);
                wifi_test_app_send_msg(instance);
                break;
            }
            furi_string_printf(instance->msg, "AP stopped\r\n");
            wifi_test_app_send_msg(instance);
            instance->state = WifiTestStateIdle;
        } else {
            furi_string_printf(instance->msg, "AP is not up\r\n");
            wifi_test_app_send_msg(instance);
        }
        break;
    case WifiTestCmdTypeStaUp:
        if(instance->state == WifiTestStateIdle) {
            sl_net_wifi_client_profile_t client_profile = {0};
            sl_ip_address_t ip_address = {0};
            //! Set Wi-Fi client profile
            status = sl_net_set_profile(
                SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &wifi_client_profile);
            if(status != SL_STATUS_OK) {
                furi_string_printf(
                    instance->msg,
                    "Failed to store the Wi-Fi client network profile: 0x%lx\r\n",
                    status);
                wifi_test_app_send_msg(instance);
                break;
            }

            furi_string_printf(
                instance->msg, "Successfully stored the Wi-Fi client network profile\r\n");
            wifi_test_app_send_msg(instance);

            //! Set network credentials
            status = sl_net_set_credential(
                SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID,
                wifi_client_credential.type,
                &wifi_client_credential.data,
                wifi_client_credential.data_length);
            if(status != SL_STATUS_OK) {
                furi_string_printf(
                    instance->msg,
                    "Failed to configure Wi-Fi client credentials: 0x%lx\r\n",
                    status);
                wifi_test_app_send_msg(instance);
                break;
            }

            furi_string_printf(
                instance->msg, "Configuring Wi-Fi client credentials is successful\r\n");
            wifi_test_app_send_msg(instance);

            //! Bring up Wi-Fi client interface
            status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1);
            if(status != SL_STATUS_OK) {
                furi_string_printf(
                    instance->msg,
                    "Failed to bring up Wi-Fi client interface up: 0x%lx\r\n",
                    status);
                wifi_test_app_send_msg(instance);
                break;
            }

            furi_string_printf(instance->msg, "Wi-Fi client interface up\r\n");
            wifi_test_app_send_msg(instance);

            //! Get profile
            status = sl_net_get_profile(
                SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &client_profile);
            if(status != SL_STATUS_OK) {
                furi_string_printf(
                    instance->msg, "Failed to get client profile: 0x%lx\r\n", status);
                wifi_test_app_send_msg(instance);
                break;
            }

            furi_string_printf(instance->msg, "Get client profile is successful\r\n");
            wifi_test_app_send_msg(instance);

            ip_address.type = SL_IPV4;
            memcpy(
                &ip_address.ip.v4.bytes,
                &client_profile.ip.ip.v4.ip_address.bytes,
                sizeof(sl_ipv4_address_t));

            if(ip_address.type == SL_IPV4) {
                furi_string_printf(
                    instance->msg,
                    "Ip address of client: %d.%d.%d.%d\r\nWi-Fi client connected\r\n",
                    ip_address.ip.v4.bytes[0],
                    ip_address.ip.v4.bytes[1],
                    ip_address.ip.v4.bytes[2],
                    ip_address.ip.v4.bytes[3]);
                wifi_test_app_send_msg(instance);
            }
            instance->state = WifiTestStateStaUp;

        } else {
            furi_string_printf(instance->msg, "AP or STA is already up\r\n");
            wifi_test_app_send_msg(instance);
        }
        break;
    case WifiTestCmdTypeStaDown:
        if(instance->state == WifiTestStateStaUp) {
            status = sl_net_down(SL_NET_WIFI_CLIENT_INTERFACE);
            if(status != SL_STATUS_OK) {
                furi_string_printf(
                    instance->msg,
                    "Failed to bring Wi-Fi client interface down: 0x%lx\r\n",
                    status);
                wifi_test_app_send_msg(instance);
                break;
            }
            furi_string_printf(instance->msg, "Wi-Fi client interface down\r\n");
            wifi_test_app_send_msg(instance);
            instance->state = WifiTestStateIdle;
        } else {
            furi_string_printf(instance->msg, "STA is not up\r\n");
            wifi_test_app_send_msg(instance);
        }
        break;
    case WifiTestCmdTypeTestTcpRx:
        if(instance->state == WifiTestStateStaUp || instance->state == WifiTestStateApUp) {
            wifi_async_socket_server_tcp_rx_init(instance, instance->msg, 5005);
        } else {
            furi_string_printf(instance->msg, "AP or STA is not up\r\n");
            wifi_test_app_send_msg(instance);
        }
        break;
    case WifiTestCmdTypeTestTcpTx:
        if(instance->state == WifiTestStateStaUp || instance->state == WifiTestStateApUp) {
            if(!args_read_string_and_trim(args, arg)) {
                wifi_async_socket_client_tcp_tx_init(
                    instance, instance->msg, WIFI_TEST_SERVER_IP, 5000);
            } else {
                wifi_async_socket_client_tcp_tx_init(
                    instance, instance->msg, (char*)furi_string_get_cstr(arg), 5000);
            }
        } else {
            furi_string_printf(instance->msg, "AP or STA is not up\r\n");
            wifi_test_app_send_msg(instance);
        }
        break;
    case WifiTestCmdTypeTestEcho:
        if(instance->state == WifiTestStateStaUp || instance->state == WifiTestStateApUp) {
            wifi_async_socket_server_echo_init(instance, instance->msg, 5005);
        } else {
            furi_string_printf(instance->msg, "AP or STA is not up\r\n");
            wifi_test_app_send_msg(instance);
        }
        break;
    default:
        wifi_test_app_send_msg_invalid_arg(instance);
        break;
    }

    furi_string_free(arg);
    return SL_STATUS_OK;
}

void wifi_test_app_parse_msg(void* app_handle, uint8_t* data, size_t size) {
    WifiTestApp* instance = (WifiTestApp*)app_handle;
    uint8_t i = 0;
    uint8_t cmd_index = 0;
    bool cmd_valid = false;

    FuriString* args = furi_string_alloc();
    furi_string_set_strn(args, (const char*)data, size);
    FuriString* cmd = furi_string_alloc();

    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(args));

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            break;
        }

        for(i = 0; i < WifiTestCmdTypeMax; i++) {
            if(furi_string_cmp_str(cmd, (char*)wifi_test_cmd[i].cmd) == 0) {
                cmd_index = i;
                cmd_valid = true;
                break;
            }
        }
        if(cmd_valid) {
            if(wifi_test_app(instance, cmd_index, args) != SL_STATUS_OK) {
                furi_string_printf(instance->msg, "Command failed\r\n");
                wifi_test_app_send_msg(instance);
            }
        } else {
            furi_string_printf(instance->msg, "Invalid command\r\n");
            wifi_test_app_send_msg(instance);
        }
    } while(false);

    furi_string_free(args);
    furi_string_free(cmd);
}

static void wifi_test_app_cmd_usage(WifiTestApp* instance) {
    furi_string_printf(instance->msg, "%s commands usage:\r\n", TAG);
    furi_string_cat_printf(
        instance->msg,
        "*************************************************************************************************************"
        "***********************************************\r\n");
    furi_string_cat_printf(instance->msg, "?\r\n");
    furi_string_cat_printf(instance->msg, "help\r\n");
    furi_string_cat_printf(
        instance->msg,
        "scan WiFi scan ap. Scanning is possible when the access point is not running\r\n");
    furi_string_cat_printf(instance->msg, "ap_up Start AP.\r\n");
    furi_string_cat_printf(instance->msg, "ap_down Stop AP.\r\n");
    furi_string_cat_printf(
        instance->msg,
        "sta_up Start STA. SSID:%s PASS:%s\r\n",
        WIFI_CLIENT_PROFILE_SSID,
        WIFI_CLIENT_CREDENTIAL);
    furi_string_cat_printf(instance->msg, "sta_down Stop STA.\r\n");
    furi_string_cat_printf(
        instance->msg,
        "test_tcp_tx [ip] Start TCP TX iPref test \"iperf.exe -s -p 5000 -i 1\". Default IP:%s\r\n",
        WIFI_TEST_SERVER_IP);
    furi_string_cat_printf(
        instance->msg,
        "test_tcp_rx Start TCP RX iPref test \"iperf.exe -c 192.168.11.10 -p 5005 -i 1 -b70M -t 30\".\r\n");
    furi_string_cat_printf(
        instance->msg, "test_echo Start TCP echo test port 5005. Work time 90 sec\r\n");
    furi_string_cat_printf(
        instance->msg,
        "*************************************************************************************************************"
        "***********************************************\r\n");
    wifi_test_app_send_msg(instance);
}
