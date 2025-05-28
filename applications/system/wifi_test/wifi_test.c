#include "wifi_test.h"
#include "helpers/wifi_scan.h"
#include "helpers/wifi_async_socket_server_tcp_rx.h"
#include "helpers/wifi_async_socket_client_tcp_tx.h"
#include "helpers/wifi_async_socket_server_echo.h"
#include "helpers/wifi_async_socket_client_udp_tx.h"

#include <furi.h>

#include <sl_status.h>
#include <sl_wifi.h>
#include <sl_net_wifi_types.h>
#include <sl_net.h>
#include <sl_si91x_driver.h>
#include <sl_wifi_callback_framework.h>

#include <cli/args.h>
#include <cli/shell/cli_shell.h>
#include <cli/cli_ansi.h>
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
    WifiTestStateIdle,
    //WifiTestStateDriverInit,
    WifiTestStateApUp,
    WifiTestStateStaUp,
} WifiTestState;

struct WifiTestApp {
    FuriString* msg;
    CliShell* shell;
    WifiTestState state;

    bool exit;
    bool udp_test_running;
    bool ap_client_connected;
};

void wifi_test_app_stop(void* app_handle);

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
        "Remote Client connected:  %2X:%2X:%2X:%2X:%2X:%2X",
        mac->octet[0],
        mac->octet[1],
        mac->octet[2],
        mac->octet[3],
        mac->octet[4],
        mac->octet[5]);
    cli_shell_notification_print(instance->shell, instance->msg);

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
        "Remote Client disconnect:  %2X:%2X:%2X:%2X:%2X:%2X",
        mac->octet[0],
        mac->octet[1],
        mac->octet[2],
        mac->octet[3],
        mac->octet[4],
        mac->octet[5]);
    cli_shell_notification_print(instance->shell, instance->msg);
    instance->ap_client_connected = false;
    return SL_STATUS_OK;
}

void* wifi_test_app_start(CliShell* shell) {
    FURI_LOG_I(TAG, "Starting");

    WifiTestApp* instance = malloc(sizeof(WifiTestApp));
    instance->msg = furi_string_alloc();
    instance->shell = shell;
    instance->state = WifiTestStateIdle;

    instance->exit = false;
    instance->ap_client_connected = false;

    sl_status_t status = SL_STATUS_FAIL;
    do {
        // Initialize Wi-Fi APSTA interface
        status = sl_net_init(
            SL_NET_WIFI_AP_INTERFACE, &sl_wifi_default_concurrent_configuration, NULL, NULL);
        if(status != SL_STATUS_OK) {
            printf(ANSI_FG_RED "Failed to start Wi-Fi APSTA interface: 0x%lx\r\n" ANSI_RESET, status);
            break;
        }

        printf("Wi-Fi APSTA interface init\r\n");
        printf("start_app: 1\r\n");
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
        if(instance->udp_test_running) {
            wifi_async_socket_client_udp_tx_stop();
        }
        instance->exit = true;
        furi_delay_ms(100);
        if(instance->state == WifiTestStateApUp) {
            sl_net_down(SL_NET_WIFI_AP_INTERFACE);
        } else if(instance->state == WifiTestStateStaUp) {
            sl_net_down(SL_NET_WIFI_CLIENT_INTERFACE);
        }
        furi_delay_ms(1000);

        furi_string_free(instance->msg);
        free(instance);
        instance = NULL;
    }

    sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
}

static void wifi_test_scan_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    WifiTestApp* instance = context;
    if(instance->state == WifiTestStateIdle) {
        sl_status_t status = wifi_scan();
        if(status != SL_STATUS_OK) {
            printf("Scan failed\r\n");
        }
    } else {
        printf("AP is up, scan not allowed\r\n");
    }
}

static void wifi_test_ap_up_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    WifiTestApp* instance = context;
    sl_status_t status;
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
            printf(ANSI_FG_RED "Failed to set AP profile: 0x%lx\r\n" ANSI_RESET, status);
            return;
        }
        printf("Success to set AP profile\r\n");

        // Set Wi-Fi AP credential
        status = sl_net_set_credential(
            SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID,
            wifi_ap_credential.type,
            &wifi_ap_credential.data,
            wifi_ap_credential.data_length);
        if(status != SL_STATUS_OK) {
            printf(ANSI_FG_RED "Failed to set credentials: 0x%lx\r\n" ANSI_RESET, status);
            return;
        }
        printf("Wi-Fi set credential success\r\n");

        // Bring Wi-Fi AP interface up
        status = sl_net_up(SL_NET_WIFI_AP_INTERFACE, SL_NET_PROFILE_ID_1);
        if(status != SL_STATUS_OK) {
            printf(ANSI_FG_RED "Failed to bring Wi-Fi AP interface up: 0x%lx\r\n" ANSI_RESET, status);
            return;
        }
        printf("AP started\r\n");
        instance->state = WifiTestStateApUp;
    } else {
        printf("AP or STA is already up\r\n");
    }
}

static void wifi_test_ap_down_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    WifiTestApp* instance = context;
    if(instance->state == WifiTestStateApUp) {
        sl_status_t status = sl_net_down(SL_NET_WIFI_AP_INTERFACE);
        if(status != SL_STATUS_OK) {
            printf(ANSI_FG_RED "Failed to bring Wi-Fi AP interface down: 0x%lx\r\n" ANSI_RESET, status);
            return;
        }
        printf("AP stopped\r\n");
        instance->state = WifiTestStateIdle;
    } else {
        printf("AP is not up\r\n");
    }
}

static void wifi_test_sta_up_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);

    WifiTestApp* instance = context;
    sl_status_t status;
    if(instance->state == WifiTestStateIdle) {
        printf("Connecting to SSID:%s PASS:%s\r\n", WIFI_CLIENT_PROFILE_SSID, WIFI_CLIENT_CREDENTIAL);

        sl_net_wifi_client_profile_t client_profile = {0};
        sl_ip_address_t ip_address = {0};
        //! Set Wi-Fi client profile
        status = sl_net_set_profile(
            SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &wifi_client_profile);
        if(status != SL_STATUS_OK) {
            printf(
                ANSI_FG_RED "Failed to store the Wi-Fi client network profile: 0x%lx\r\n" ANSI_RESET,
                status);
                return;
        }

        printf("Successfully stored the Wi-Fi client network profile\r\n");

        //! Set network credentials
        status = sl_net_set_credential(
            SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID,
            wifi_client_credential.type,
            &wifi_client_credential.data,
            wifi_client_credential.data_length);
        if(status != SL_STATUS_OK) {
            printf(
                ANSI_FG_RED "Failed to configure Wi-Fi client credentials: 0x%lx\r\n" ANSI_RESET,
                status);
                return;
        }

        printf("Configuring Wi-Fi client credentials is successful\r\n");

        //! Bring up Wi-Fi client interface
        status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1);
        if(status != SL_STATUS_OK) {
            printf(
                ANSI_FG_RED "Failed to bring up Wi-Fi client interface up: 0x%lx\r\n" ANSI_RESET,
                status);
            return;
        }

        printf("Wi-Fi client interface up\r\n");

        //! Get profile
        status = sl_net_get_profile(
            SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &client_profile);
        if(status != SL_STATUS_OK) {
            printf(ANSI_FG_RED "Failed to get client profile: 0x%lx\r\n" ANSI_RESET, status);
            return;
        }

        printf("Get client profile is successful\r\n");

        ip_address.type = SL_IPV4;
        memcpy(
            &ip_address.ip.v4.bytes,
            &client_profile.ip.ip.v4.ip_address.bytes,
            sizeof(sl_ipv4_address_t));

        if(ip_address.type == SL_IPV4) {
            printf(
                "ip_address_of_client: %d.%d.%d.%d\r\nWi-Fi client connected\r\n",
                ip_address.ip.v4.bytes[0],
                ip_address.ip.v4.bytes[1],
                ip_address.ip.v4.bytes[2],
                ip_address.ip.v4.bytes[3]);
        }
        instance->state = WifiTestStateStaUp;

    } else {
        printf("AP or STA is already up\r\n");
    }
}

static void wifi_test_sta_down_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    WifiTestApp* instance = context;
    if(instance->state == WifiTestStateStaUp) {
        sl_status_t status = sl_net_down(SL_NET_WIFI_CLIENT_INTERFACE);
        if(status != SL_STATUS_OK) {
            printf(
                ANSI_FG_RED "Failed to bring Wi-Fi client interface down: 0x%lx\r\n" ANSI_RESET,
                status);
            return;
        }
        printf("Wi-Fi client interface down\r\n");
        instance->state = WifiTestStateIdle;
    } else {
        printf("STA is not up\r\n");
    }
}

static void wifi_test_tcp_rx_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    WifiTestApp* instance = context;
    if(instance->state == WifiTestStateStaUp || instance->state == WifiTestStateApUp) {
        printf("iPerf test: \"iperf.exe -c 192.168.11.10 -p 5005 -i 1 -b70M -t 30\"\r\n");
        wifi_async_socket_server_tcp_rx_init(5005);
    } else {
        printf("AP or STA is not up\r\n");
    }
}

static void wifi_test_tcp_tx_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    WifiTestApp* instance = context;
    if(instance->state == WifiTestStateStaUp || instance->state == WifiTestStateApUp) {
        FuriString* arg = furi_string_alloc();
        printf("iPerf test: \"iperf.exe -s -p 5000 -i 1\"\r\n");
        if(!args_read_string_and_trim(args, arg)) {
            printf("Using default IP: %s\r\n", WIFI_TEST_SERVER_IP);
            wifi_async_socket_client_tcp_tx_init(WIFI_TEST_SERVER_IP, 5000);
        } else {
            wifi_async_socket_client_tcp_tx_init((char*)furi_string_get_cstr(arg), 5000);
        }
        furi_string_free(arg);
    } else {
        printf("AP or STA is not up\r\n");
    }
}

static void wifi_test_udp_tx_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    WifiTestApp* instance = context;
    if(instance->state == WifiTestStateStaUp || instance->state == WifiTestStateApUp) {
        FuriString* arg = furi_string_alloc();
        printf("iPerf test: \"iperf.exe -s -u -p 5001 -i 1\"\r\n");
        if(!args_read_string_and_trim(args, arg)) {
            printf("Using default IP: %s\r\n", WIFI_TEST_SERVER_IP);
            wifi_async_socket_client_udp_tx_init(instance->shell, WIFI_TEST_SERVER_IP, 5001);
        } else {
            wifi_async_socket_client_udp_tx_init(
                instance->shell, (char*)furi_string_get_cstr(arg), 5001);
        }
        instance->udp_test_running = true;
        furi_string_free(arg);
    } else {
        printf("AP or STA is not up\r\n");
    }
}

static void wifi_test_udp_tx_stop_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    WifiTestApp* instance = context;
    if(instance->state == WifiTestStateStaUp || instance->state == WifiTestStateApUp) {
        wifi_async_socket_client_udp_tx_stop();
        printf("UDP TX stopped\r\n");
        instance->udp_test_running = false;
    } else {
        printf("AP or STA is not up\r\n");
    }
}

static void wifi_test_echo_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    WifiTestApp* instance = context;
    if(instance->state == WifiTestStateStaUp || instance->state == WifiTestStateApUp) {
        wifi_async_socket_server_echo_init(5005);
        printf("Started TCP echo on port 5005. Work time 90 sec\r\n");
    } else {
        printf("AP or STA is not up\r\n");
    }
}

static void crypto_test_motd(void* context) {
    UNUSED(context);
    printf("\r\n+------------------------------+\r\n");
    printf("| Welcome to Wi-Fi test shell! |\r\n");
    printf("+------------------------------+\r\n\r\n");
}

void wifi_test_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

    CliRegistry* registry = cli_registry_alloc();
    CliShell* shell = cli_shell_alloc(crypto_test_motd, NULL, pipe, registry, NULL);
    cli_shell_set_prompt(shell, "wifi_test");

    WifiTestApp* app = wifi_test_app_start(shell);
    cli_registry_add_command(registry, "scan", CliCommandFlagDefault, wifi_test_scan_command, app);
    cli_registry_add_command(registry, "ap_up", CliCommandFlagDefault, wifi_test_ap_up_command, app);
    cli_registry_add_command(registry, "ap_down", CliCommandFlagDefault, wifi_test_ap_down_command, app);
    cli_registry_add_command(registry, "sta_up", CliCommandFlagDefault, wifi_test_sta_up_command, app);
    cli_registry_add_command(registry, "sta_down", CliCommandFlagDefault, wifi_test_sta_down_command, app);
    cli_registry_add_command(registry, "tcp_tx", CliCommandFlagDefault, wifi_test_tcp_tx_command, app);
    cli_registry_add_command(registry, "tcp_rx", CliCommandFlagDefault, wifi_test_tcp_rx_command, app);
    cli_registry_add_command(registry, "udp_tx", CliCommandFlagDefault, wifi_test_udp_tx_command, app);
    cli_registry_add_command(registry, "udp_tx_stop", CliCommandFlagDefault, wifi_test_udp_tx_stop_command, app);
    cli_registry_add_command(registry, "echo", CliCommandFlagDefault, wifi_test_echo_command, app);

    cli_shell_start(shell);
    cli_shell_join(shell);
    wifi_test_app_stop(app);

    cli_shell_free(shell);
    cli_registry_free(registry);
}
