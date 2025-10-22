#pragma once

#include "wifi_i.h"

void wifi_state_transition(Wifi* instance, WifiState new_state, ...);
