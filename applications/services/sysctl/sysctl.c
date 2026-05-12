#include "sysctl.h"
#include "settings/sysctl_settings.h"

static SysctlSettings s_settings;

bool sysctl_get_cli_wifi_enabled(void) {
    return s_settings.cli_wifi_enabled;
}

void sysctl_set_cli_wifi_enabled(bool enabled) {
    s_settings.cli_wifi_enabled = enabled;
    sysctl_settings_save(&s_settings);
}

int sysctl_get_websrv_accesslog_level(void) {
    return s_settings.websrv_accesslog_level;
}

void sysctl_set_websrv_accesslog_level(int level) {
    s_settings.websrv_accesslog_level = level;
    sysctl_settings_save(&s_settings);
}

void sysctl_on_system_start(void) {
    sysctl_settings_load(&s_settings);
}
