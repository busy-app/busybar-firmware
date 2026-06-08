#include "sysctl.h"
#include "settings/sysctl_settings.h"
#include <furi.h>
#include <furi_hal_nvm.h>

typedef struct {
    SysctlSettings settings;
    FuriMutex* mutex;
} SysctlState;

static SysctlState s_state;

bool sysctl_get_cli_wifi_enabled(void) {
    furi_mutex_acquire(s_state.mutex, FuriWaitForever);
    bool value = s_state.settings.cli_wifi_enabled;
    furi_mutex_release(s_state.mutex);
    return value;
}

void sysctl_set_cli_wifi_enabled(bool enabled) {
    furi_mutex_acquire(s_state.mutex, FuriWaitForever);
    s_state.settings.cli_wifi_enabled = enabled;
    sysctl_settings_save(&s_state.settings);
    furi_mutex_release(s_state.mutex);
}

int sysctl_get_websrv_accesslog_level(void) {
    furi_mutex_acquire(s_state.mutex, FuriWaitForever);
    int value = s_state.settings.websrv_accesslog_level;
    furi_mutex_release(s_state.mutex);
    return value;
}

void sysctl_set_websrv_accesslog_level(int level) {
    furi_mutex_acquire(s_state.mutex, FuriWaitForever);
    s_state.settings.websrv_accesslog_level = level;
    sysctl_settings_save(&s_state.settings);
    furi_mutex_release(s_state.mutex);
}

int sysctl_get_ui_debug_mode(void) {
    furi_mutex_acquire(s_state.mutex, FuriWaitForever);
    int value = s_state.settings.ui_debug_mode;
    furi_mutex_release(s_state.mutex);
    return value;
}

void sysctl_set_ui_debug_mode(int mode) {
    furi_mutex_acquire(s_state.mutex, FuriWaitForever);
    s_state.settings.ui_debug_mode = mode;
    sysctl_settings_save(&s_state.settings);
    furi_mutex_release(s_state.mutex);
}

void sysctl_on_system_start(void) {
    s_state.mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    sysctl_settings_load(&s_state.settings);
    if(s_state.settings.debug_enabled) {
        furi_hal_nvm_set_flag(FuriHalNvmFlagDebug);
    } else {
        furi_hal_nvm_reset_flag(FuriHalNvmFlagDebug);
    }
}

void sysctl_set_debug_enabled(bool enabled) {
    furi_mutex_acquire(s_state.mutex, FuriWaitForever);
    s_state.settings.debug_enabled = enabled;
    if(enabled) {
        furi_hal_nvm_set_flag(FuriHalNvmFlagDebug);
    } else {
        furi_hal_nvm_reset_flag(FuriHalNvmFlagDebug);
    }
    sysctl_settings_save(&s_state.settings);
    furi_mutex_release(s_state.mutex);
}
