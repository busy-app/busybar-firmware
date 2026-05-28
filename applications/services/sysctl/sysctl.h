#pragma once

#include <stdbool.h>

/* Public sysctl API — use these instead of directly touching sysctl settings. */

bool sysctl_get_cli_wifi_enabled(void);
void sysctl_set_cli_wifi_enabled(bool enabled);

int sysctl_get_websrv_accesslog_level(void);
void sysctl_set_websrv_accesslog_level(int level);

int sysctl_get_ui_debug_mode(void);
void sysctl_set_ui_debug_mode(int mode);

void sysctl_set_debug_enabled(bool enabled);
