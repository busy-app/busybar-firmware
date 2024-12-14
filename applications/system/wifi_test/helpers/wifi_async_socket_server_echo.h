#pragma once
#include <furi.h>
#include "wifi_test_app.h"

void wifi_async_socket_server_echo_init(WifiTestApp* app, FuriString* msg, uint16_t port);
