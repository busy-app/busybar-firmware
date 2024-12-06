#pragma once
#include <furi.h>
#include "wifi_ap_test_app.h"

void wifi_async_socket_server_tcp_rx_init(WifiApTestApp* app, FuriString* msg, uint16_t port);
