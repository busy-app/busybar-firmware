#include "busy_timer_saved_state.h"

#include <storage/storage.h>

#define BUSY_TIMER_SAVED_STATE_FILE_PATH APP_DATA_PATH("state.json")
#define BUSY_TIMER_SAVED_STATE_VERSION   1
#define BUSY_TIMER_SAVED_STATE_ROOT      busy_timer_saved_state_v1_root

void busy_timer_saved_state_load(BusyTimerSavedState* saved_state) {
    furi_check(saved_state);

    SettingProvider* provider = setting_provider_alloc(
        BUSY_TIMER_SAVED_STATE_FILE_PATH, BUSY_TIMER_SAVED_STATE_VERSION, NULL, 0);
    setting_provider_load(provider, &BUSY_TIMER_SAVED_STATE_ROOT, saved_state);
    setting_provider_free(provider);
}

void busy_timer_saved_state_save(const BusyTimerSavedState* saved_state) {
    furi_check(saved_state);

    SettingProvider* provider = setting_provider_alloc(
        BUSY_TIMER_SAVED_STATE_FILE_PATH, BUSY_TIMER_SAVED_STATE_VERSION, NULL, 0);
    setting_provider_save(provider, &BUSY_TIMER_SAVED_STATE_ROOT, saved_state);
    setting_provider_free(provider);
}
