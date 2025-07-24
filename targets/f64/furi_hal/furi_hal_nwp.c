#include <furi.h>
#include <furi_hal_nwp.h>
#include <sl_net.h>
#include "wifi_config.h"

#define TAG "FuriHalNwp"

typedef enum {
    FuriHalNwpIdle,
    FuriHalNwpIsInitialized,
    FuriHalNwpInit,
    FuriHalNwpError,
} FuriHalNwpState;

typedef struct {
    FuriHalNwpState state;
    uint32_t holders_count; // Count of holders to track if NWP is initialized
    FuriMutex* mutex;
} FuriHalNwp;

static FuriHalNwp furi_hal_nwp_is_nwp_initialized = {FuriHalNwpIdle, 0, NULL};
static FURI_ALWAYS_INLINE void furi_hal_nwp_lock(void) {
    furi_check(
        furi_mutex_acquire(furi_hal_nwp_is_nwp_initialized.mutex, FuriWaitForever) ==
        FuriStatusOk);
}

static FURI_ALWAYS_INLINE void furi_hal_nwp_unlock(void) {
    furi_check(furi_mutex_release(furi_hal_nwp_is_nwp_initialized.mutex) == FuriStatusOk);
}

static FURI_ALWAYS_INLINE void furi_hal_nwp_mutex_init(void) {
    furi_hal_nwp_is_nwp_initialized.mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    furi_check(furi_hal_nwp_is_nwp_initialized.mutex != NULL);
}

bool furi_hal_nwp_is_initialized(void) {
    return (
        (furi_hal_nwp_is_nwp_initialized.state != FuriHalNwpIdle) &&
        (furi_hal_nwp_is_nwp_initialized.state != FuriHalNwpError));

    return !(furi_hal_nwp_is_nwp_initialized.state == FuriHalNwpIdle);
}

bool furi_hal_nwp_init(void) {
    if(furi_hal_nwp_is_initialized()) {
        FURI_LOG_D(TAG, "NWP is already initialized");
        furi_hal_nwp_is_nwp_initialized.holders_count++;
        return true;
    }

    if(!furi_hal_nwp_is_nwp_initialized.mutex) {
        furi_hal_nwp_mutex_init();
    }

    furi_hal_nwp_lock();
    bool ret = false;
    do {
        sl_status_t status =
            sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &wifi_config_client, NULL, NULL);
        if(status == SL_STATUS_ALREADY_INITIALIZED) {
            furi_hal_nwp_is_nwp_initialized.state = FuriHalNwpIsInitialized;
            furi_hal_nwp_is_nwp_initialized.holders_count++;
            FURI_LOG_D(TAG, "NWP already initialized");
            ret = true;
            break;
        }
        if(status == SL_STATUS_OK) {
            furi_hal_nwp_is_nwp_initialized.state = FuriHalNwpInit;
            furi_hal_nwp_is_nwp_initialized.holders_count++;
            FURI_LOG_D(TAG, "NWP initialized");
            ret = true;
            break;
        }
        FURI_LOG_E(TAG, "Failed to initialise NWP: 0x%08lx", status);
        furi_hal_nwp_is_nwp_initialized.state = FuriHalNwpError;
    } while(false);
    furi_hal_nwp_unlock();
    return ret;
}

bool furi_hal_nwp_deinit(void) {
    if(!furi_hal_nwp_is_initialized()) {
        FURI_LOG_D(TAG, "NWP is not initialized");
        return true;
    }

    furi_hal_nwp_lock();
    bool ret = false;
    do {
        if(furi_hal_nwp_is_nwp_initialized.holders_count > 0) {
            furi_hal_nwp_is_nwp_initialized.holders_count--;
        }

        if(furi_hal_nwp_is_nwp_initialized.holders_count > 0) {
            FURI_LOG_D(TAG, "NWP is still in use by other components");
            ret = true;
            break;
        }

        if(furi_hal_nwp_is_nwp_initialized.state == FuriHalNwpIdle) {
            FURI_LOG_D(TAG, "NWP is not initialized");
            ret = true;
            break;
        }

        if(furi_hal_nwp_is_nwp_initialized.state == FuriHalNwpError) {
            FURI_LOG_E(TAG, "NWP is in error state, cannot deinitialize");
            break;
        }

        sl_status_t status = sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to deinitialise NWP: 0x%08lx", status);
            furi_hal_nwp_is_nwp_initialized.state = FuriHalNwpError;
            break;
        }

        FURI_LOG_D(TAG, "NWP deinitialized");
        furi_hal_nwp_is_nwp_initialized.state = FuriHalNwpIdle;
        ret = true;
    } while(false);

    furi_hal_nwp_unlock();
    return ret;
}
