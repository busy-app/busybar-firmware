#pragma once
#include <furi.h>
#include "../wifi_sta_test_i.h"

typedef struct {
    //Todo add if needed
} WifiStaCliSettings;

bool wifi_sta_cli_start(WifiStaTest* app_hendle, WifiStaCliSettings settings);
void wifi_sta_cli_stop(void);
