#pragma once

#include "wifi_i.h"

void wifi_state_transition(Wifi* instance, WifiState new_state, ...);

void wifi_state_update_backend_info(Wifi* instance, const WifiBackendInfo* backend_info);
