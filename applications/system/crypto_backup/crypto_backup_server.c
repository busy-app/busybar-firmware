#include <furi.h>
#include <cli/args.h>
#include <cli/cli_ansi.h>
#include <intercom/intercom.h>

#include "sl_si91x_driver.h"
#include <furi_hal_nwp.h>

#include <furi_hal_crypto_storage.h>

#include "crypto_backup_common.h"

#define TAG "CryptoBackupServer"

typedef struct {
    Intercom* intercom;
    FuriSemaphore* access_semaphore;
    uint32_t buffer_size;
    uint8_t* buffer;
} CryptoBackupServer;

static CryptoBackupServer crypto_backup_server;

static int32_t crypto_backup_server_thread_callback(void* context) {
    furi_assert(context);
    CryptoBackupServer* instance = context;
    FURI_LOG_D(TAG, "Start");

    if(!furi_hal_nwp_init()) {
        FURI_LOG_E(TAG, "NWP is not initialized");
        return 0;
    }

    CryptoBackupEvent* event_rx = (CryptoBackupEvent*)instance->buffer;
    furi_assert(event_rx);
    CryptoBackupEvent* event_tx = malloc(sizeof(CryptoBackupEvent));
    FURI_LOG_D(TAG, "Cmd: %d", event_rx->cmd);

    if(event_rx->cmd == CryptoBackupCmdGet) {
        //uint8_t* buf = malloc(CRYPTO_BACKUP_SERVER_READ_BUFFER_SIZE);
        event_tx->cmd = CryptoBackupCmdGet;
        event_tx->data_size = CRYPTO_BACKUP_COMMON_BUFFER_SIZE;

        uint8_t counter = 0;
        sl_status_t status = SL_STATUS_FAIL;

        for(uint32_t i = FURI_HAL_CRYPTO_STORAGE_START_ADDRESS;
            i < FURI_HAL_CRYPTO_STORAGE_END_ADDRESS;
            i += CRYPTO_BACKUP_COMMON_BUFFER_SIZE) {
            status = sl_si91x_command_to_read_common_flash(
                i, CRYPTO_BACKUP_COMMON_BUFFER_SIZE, event_tx->data);
            if(status != SL_STATUS_OK) {
                FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx", status);
                break;
            }
            size_t tx_size = intercom_tx(
                instance->intercom,
                IntercomChannelCryptoBackup,
                event_tx,
                sizeof(CryptoBackupEvent),
                FuriWaitForever);
            furi_check(tx_size == sizeof(CryptoBackupEvent), "Failed to send data");
            FURI_LOG_T(
                TAG,
                "Transmitted %d packets, %ld  bytes",
                ++counter,
                i + CRYPTO_BACKUP_COMMON_BUFFER_SIZE);
            // ToDo add some delay to avoid flooding
            furi_delay_ms(10);
        }

    } else if(event_rx->cmd == CryptoBackupCmdSet) {
        FURI_LOG_D(TAG, "Set command received");
        // Handle set command logic here
    } else {
        FURI_LOG_E(TAG, "Unknown command: %d", event_rx->cmd);
    }

    free(event_tx);

    furi_hal_nwp_deinit();

    FURI_LOG_D(TAG, "Stopping thread");

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
        FURI_LOG_D(TAG, "Stop");
        free(crypto_backup_server.buffer);
        crypto_backup_server.buffer = NULL;
        crypto_backup_server.buffer_size = 0;
        furi_semaphore_release(crypto_backup_server.access_semaphore);
    }
}

static void crypto_backup_server_startup(void* context) {
    CryptoBackupServer* instance = context;
    FuriThread* startup_thread = furi_thread_alloc_ex(
        "CryptoBackup", 1024 * 2, crypto_backup_server_thread_callback, instance);
    furi_thread_set_state_callback(startup_thread, crypto_backup_server_thread_state_callback);
    FURI_LOG_T(TAG, "Starting thread");

    furi_thread_start(startup_thread);
}

static void crypto_backup_server_rx_callback(const void* data, size_t data_size, void* context) {
    furi_check(data);
    furi_check(context);
    CryptoBackupServer* instance = context;
    UNUSED(data_size);

    furi_semaphore_acquire(instance->access_semaphore, FuriWaitForever);
    instance->buffer_size = data_size;
    instance->buffer = malloc(data_size);
    memcpy(instance->buffer, data, data_size);

    crypto_backup_server_startup(context);
}

int32_t crypto_backup_server_init(void* arg) {
    UNUSED(arg);
    FURI_LOG_I(TAG, "Server Init");
    crypto_backup_server.access_semaphore = furi_semaphore_alloc(1, 1);
    crypto_backup_server.buffer_size = 0;
    crypto_backup_server.buffer = NULL;
    crypto_backup_server.intercom = furi_record_open(RECORD_INTERCOM);

    intercom_set_rx_callback(
        crypto_backup_server.intercom,
        IntercomChannelCryptoBackup,
        crypto_backup_server_rx_callback,
        &crypto_backup_server);
    return 0;
}
