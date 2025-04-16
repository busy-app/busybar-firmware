#pragma once
#include <furi.h>
#include "../wifi_adaptive_test_i.h"

typedef struct {
    char* ip;
} WifiAdaptiveCliSettings;

bool wifi_adaptive_cli_start(WifiAdaptiveTest* app_hendle, WifiAdaptiveCliSettings settings);
void wifi_adaptive_cli_stop(void);
