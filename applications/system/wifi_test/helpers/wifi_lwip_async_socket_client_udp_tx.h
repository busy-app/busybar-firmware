#pragma once
#include <furi.h>
#include "wifi_test_app.h"

void wifi_lwip_async_socket_client_udp_tx_init(
    WifiTestApp* app,
    FuriString* msg,
    char* ip,
    uint16_t port);
