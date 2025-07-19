#include <furi.h>
#include <furi_hal_nwp.h>
#include <sl_net.h>
#include "wifi_config.h"

#define TAG "FuriHalNwp"

typedef enum {
    FuriHalNwpIdle,
    FuriHalNwpIsInitialized,
    FuriHalNwpInit,
} FuriHalNwp;
static FuriHalNwp furi_hal_nwp_is_nwp_initialized = FuriHalNwpIdle;

bool furi_hal_nwp_is_initialized(void) {
    return !(furi_hal_nwp_is_nwp_initialized == FuriHalNwpIdle);
}

bool furi_hal_nwp_init(void) {
    if(furi_hal_nwp_is_nwp_initialized == FuriHalNwpInit ||
       furi_hal_nwp_is_nwp_initialized == FuriHalNwpIsInitialized) {
        FURI_LOG_D(TAG, "NWP is already initialized");
        return true;
    }
    sl_status_t status =
        sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &wifi_config_client, NULL, NULL);
    if(status == SL_STATUS_ALREADY_INITIALIZED) {
        furi_hal_nwp_is_nwp_initialized = FuriHalNwpIsInitialized;
        FURI_LOG_D(TAG, "NWP already initialized");
        return true;
    } else if(status == SL_STATUS_OK) {
        furi_hal_nwp_is_nwp_initialized = FuriHalNwpInit;
        FURI_LOG_D(TAG, "NWP initialized");
        return true;
    } else {
        FURI_LOG_E(TAG, "Failed to initialise NWP: 0x%08lx", status);
        furi_hal_nwp_is_nwp_initialized = FuriHalNwpIdle;
    }
    return false;
}

void furi_hal_nwp_deinit(void) {
    if(furi_hal_nwp_is_nwp_initialized == FuriHalNwpInit) {
        sl_status_t status = sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to deinitialise Wifi: 0x%08lx", status);
        } else {
            FURI_LOG_D(TAG, "NWP deinitialized");
            furi_hal_nwp_is_nwp_initialized = FuriHalNwpIdle;
        }

    } else if(furi_hal_nwp_is_nwp_initialized == FuriHalNwpIsInitialized) {
        FURI_LOG_D(TAG, "NWP is already initialized, no need to deinitialize");
    } else {
        FURI_LOG_E(TAG, "NWP is not initialized");
    }
}
