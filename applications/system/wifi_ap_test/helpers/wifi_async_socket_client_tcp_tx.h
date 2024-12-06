#pragma once
#include <furi.h>
#include "wifi_ap_test_app.h"

void wifi_async_socket_client_tcp_tx_init(
    WifiApTestApp* app,
    FuriString* msg,
    char* ip,
    uint16_t port);
