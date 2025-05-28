#pragma once
#include <furi.h>
#include "../wifi_test.h"
#include <cli/shell/cli_shell.h>

void wifi_async_socket_client_udp_tx_init(
    CliShell* shell,
    char* ip,
    uint16_t port);
void wifi_async_socket_client_udp_tx_stop(void);
