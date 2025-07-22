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
} FuriHalNwp;

static FuriHalNwp furi_hal_nwp_is_nwp_initialized = {FuriHalNwpIdle, 0};

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

    sl_status_t status =
        sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &wifi_config_client, NULL, NULL);
    if(status == SL_STATUS_ALREADY_INITIALIZED) {
        furi_hal_nwp_is_nwp_initialized.state = FuriHalNwpIsInitialized;
        furi_hal_nwp_is_nwp_initialized.holders_count++;
        FURI_LOG_D(TAG, "NWP already initialized");
        return true;
    } else if(status == SL_STATUS_OK) {
        furi_hal_nwp_is_nwp_initialized.state = FuriHalNwpInit;
        furi_hal_nwp_is_nwp_initialized.holders_count++;
        FURI_LOG_D(TAG, "NWP initialized");
        return true;
    } else {
        FURI_LOG_E(TAG, "Failed to initialise NWP: 0x%08lx", status);
        furi_hal_nwp_is_nwp_initialized.state = FuriHalNwpError;
    }
    return false;
}

bool furi_hal_nwp_deinit(void) {
    if(furi_hal_nwp_is_nwp_initialized.holders_count > 0) {
        furi_hal_nwp_is_nwp_initialized.holders_count--;
    }

    if(furi_hal_nwp_is_nwp_initialized.holders_count > 0) {
        FURI_LOG_D(TAG, "NWP is still in use by other components");
        return true;
    }

    if(furi_hal_nwp_is_nwp_initialized.state == FuriHalNwpIdle) {
        FURI_LOG_D(TAG, "NWP is not initialized");
        return true;
    }

    if(furi_hal_nwp_is_nwp_initialized.state == FuriHalNwpError) {
        FURI_LOG_E(TAG, "NWP is in error state, cannot deinitialize");
        return false;
    }

    sl_status_t status = sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
    if(status != SL_STATUS_OK) {
        FURI_LOG_E(TAG, "Failed to deinitialise NWP: 0x%08lx", status);
        furi_hal_nwp_is_nwp_initialized.state = FuriHalNwpError;
        return false;
    }
    FURI_LOG_D(TAG, "NWP deinitialized");
    furi_hal_nwp_is_nwp_initialized.state = FuriHalNwpIdle;
    return true;
}
