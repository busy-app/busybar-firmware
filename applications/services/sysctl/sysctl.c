#include "sysctl.h"
#include "settings/sysctl_settings.h"
#include <furi.h>
#include <storage/storage.h>

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

void sysctl_on_system_start(void) {
    s_state.mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_simply_mkdir(storage, EXT_PATH("apps_data/sysctl"));
    furi_record_close(RECORD_STORAGE);

    sysctl_settings_load(&s_state.settings);
}
