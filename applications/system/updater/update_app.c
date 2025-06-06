#include "updater_core.h"

#include <furi.h>
#include <furi_hal_nvm.h>

#define TAG "UpdaterApp"

static void updater_execute(const char* update_path) {
    UpdaterState* state = updater_state_alloc(update_path);

    bool config_ok = updater_load_configuration(state);
    FURI_LOG_I(TAG, "Updater config loaded: %s", config_ok ? "OK" : "FAIL");
    config_ok = updater_validate_config(state);
    FURI_LOG_I(TAG, "Updater config validated: %s", config_ok ? "OK" : "FAIL");

    updater_state_free(state);
}

int32_t updater_srv(void* arg) {
    UNUSED(arg);

    FURI_LOG_I(TAG, "Updater service started");

    // if(furi_hal_nvm_get_boot_mode() == FuriHalNvmBootModeNormal) {
    //     FURI_LOG_E(TAG, "Updater service in normal boot mode");
    //     furi_hal_nvm_set_boot_mode(FuriHalNvmBootModeNormal);
    //     furi_thread_suspend(furi_thread_get_current_id());
    // }

    updater_execute("/ext/update");
    furi_thread_suspend(furi_thread_get_current_id());

    return 0;
}
