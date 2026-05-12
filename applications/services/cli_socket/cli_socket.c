#include <furi.h>
#include <lwip/tcp.h>
#include <lwip/netif.h>
#include <network/network.h>
#include "cli_socket.h"
#include "cli_socket_client.h"
#include "settings/sysctl_settings.h"

#include <lwip/tcpip.h>

#define CLI_SOCKET_PORT 23
#define TAG             "CliSocketServer"

/* Cached WiFi CLI enable state. Updated at startup and by cli_socket_set_wifi_enabled(). */
static volatile bool cli_socket_wifi_enabled = false;

void cli_socket_set_wifi_enabled(bool enabled) {
    cli_socket_wifi_enabled = enabled;
}

/**
 * @brief Returns true if the accepted connection arrived on the WiFi netif ("WL0").
 *
 * Called from within the lwIP thread (accept callback), so netif_find is safe.
 */
static bool cli_socket_is_wifi_connection(struct tcp_pcb* client_socket) {
    if(!IP_IS_V4_VAL(client_socket->local_ip)) return false;
    struct netif* wifi_netif = network_find_netif(NetworkNetifWifi);
    if(!wifi_netif) return false;
    return ip4_addr_eq(ip_2_ip4(&client_socket->local_ip), netif_ip4_addr(wifi_netif));
}

static err_t cli_socket_accept_callback(void* context, struct tcp_pcb* client_socket, err_t err) {
    struct tcp_pcb* listen_socket = context;
    if(err != ERR_OK) {
        FURI_LOG_E(TAG, "Error accepting connection: %d", err);
        return err;
    }

    if(cli_socket_is_wifi_connection(client_socket) && !cli_socket_wifi_enabled) {
        FURI_LOG_I(
            TAG,
            "Rejected WiFi CLI from %s:%d (disabled by sysctl)",
            ipaddr_ntoa(&client_socket->remote_ip),
            client_socket->remote_port);
        return ERR_MEM; /* lwIP aborts the PCB for us */
    }

    FURI_LOG_I(
        TAG, "Accepted %s:%d", ipaddr_ntoa(&client_socket->remote_ip), client_socket->remote_port);
    cli_socket_client_start(client_socket);
    tcp_accepted(listen_socket);
    return ERR_OK;
}

static void cli_socket_init_callback(void* context) {
    UNUSED(context);

    struct tcp_pcb* server_socket = tcp_new();
    furi_check(tcp_bind(server_socket, IP_ADDR_ANY, CLI_SOCKET_PORT) == ERR_OK);

    struct tcp_pcb* listen_socket = tcp_listen(server_socket);
    tcp_arg(listen_socket, listen_socket);
    tcp_accept(listen_socket, cli_socket_accept_callback);

    FURI_LOG_I(TAG, "Started");
}

void cli_socket_on_system_start(void) {
    SysctlSettings sysctl_settings;
    sysctl_settings_load(&sysctl_settings);
    cli_socket_wifi_enabled = sysctl_settings.cli_wifi_enabled;

    furi_record_open(RECORD_NETWORK);
    tcpip_callback(cli_socket_init_callback, NULL);
}
