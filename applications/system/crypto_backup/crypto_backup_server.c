#include <furi.h>
#include <cli/args.h>
#include <cli/cli_ansi.h>
#include <intercom/intercom.h>

#include "sl_si91x_driver.h"
#include <furi_hal_nwp.h>

#include <furi_hal_crypto_storage.h>

#define TAG "CryptoBackupServer"

static int32_t crypto_backup_server_thread_callback(void* context) {
    furi_assert(context);
    Intercom* intercom = context;
    FURI_LOG_D(TAG, "Start");

    if(!furi_hal_nwp_init()) {
        FURI_LOG_E(TAG, "NWP is not initialized");
        return 0;
    }

    uint8_t* buf = malloc(512);
    uint8_t counter = 0;
    sl_status_t status = SL_STATUS_FAIL;

    for(uint32_t i = FURI_HAL_CRYPTO_STORAGE_START_ADDRESS;
        i < FURI_HAL_CRYPTO_STORAGE_END_ADDRESS;
        i += 512) {
        status = sl_si91x_command_to_read_common_flash(i, 512, buf);
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx", status);
            break;
        }
        size_t tx_size =
            intercom_tx(intercom, IntercomChannelCryptoBackup, buf, 512, FuriWaitForever);
        furi_check(tx_size == 512, "Failed to send data");
        FURI_LOG_T(TAG, "Transmitted %d packets, %ld  bytes", ++counter, i + 512);
    }
    free(buf);

    furi_hal_nwp_deinit();

    FURI_LOG_D(TAG, "Stoping thread");

    return 0;
}

static void crypto_backup_server_thread_state_callback(
    FuriThread* thread,
    FuriThreadState state,
    void* context) {
    furi_assert(thread);
    UNUSED(context);

    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
    }
    FURI_LOG_D(TAG, "Stop");
}

static void crypto_backup_server_startup(void* context) {
    Intercom* intercom = context;
    FuriThread* startup_thread = furi_thread_alloc_ex(
        "CryptoBackup", 1024 * 2, crypto_backup_server_thread_callback, intercom);
    furi_thread_set_state_callback(startup_thread, crypto_backup_server_thread_state_callback);
    FURI_LOG_T(TAG, "Starting thread");
    furi_thread_start(startup_thread);
}

static void crypto_backup_server_rx_callback(const void* data, size_t data_size, void* context) {
    furi_check(data);
    furi_check(context);
    Intercom* intercom = context;
    UNUSED(intercom);
    UNUSED(data_size);

    //todo: add get/set commands
    crypto_backup_server_startup(context);
}

int32_t crypto_backup_server_init(void* arg) {
    UNUSED(arg);
    FURI_LOG_I(TAG, "Server Init");

    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    intercom_set_rx_callback(
        intercom, IntercomChannelCryptoBackup, crypto_backup_server_rx_callback, intercom);
    return 0;
}
