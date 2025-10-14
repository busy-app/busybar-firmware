#include <furi.h>
#include <cli/args.h>
#include <cli/cli_ansi.h>
#include <wifi/wifi_common.h>
#include <intercom/intercom.h>

#include <sl_si91x_driver.h>
#include <furi_hal_crypto_storage.h>

#include "crypto_backup_common.h"

#define TAG "CryptoBackupServer"

typedef struct {
    IntercomChannel* intercom;
    FuriSemaphore* access_semaphore;
    uint32_t buffer_size;
    uint8_t* buffer;
} CryptoBackupServer;

static CryptoBackupServer crypto_backup_server;

static void crypto_backup_server_tx(CryptoBackupServer* instance, CryptoBackupEvent* event_tx) {
    furi_check(instance);
    furi_check(event_tx);

    size_t tx_size =
        intercom_tx(instance->intercom, event_tx, sizeof(CryptoBackupEvent), FuriWaitForever);
    furi_check(tx_size == sizeof(CryptoBackupEvent), "Failed to send data");
}

static int32_t crypto_backup_server_thread_callback(void* context) {
    furi_assert(context);
    CryptoBackupServer* instance = context;
    FURI_LOG_D(TAG, "Start");

    furi_record_open(RECORD_WIFI);

    CryptoBackupEvent* event_rx = (CryptoBackupEvent*)instance->buffer;
    furi_assert(event_rx);
    sl_status_t status = SL_STATUS_FAIL;
    CryptoBackupEvent* event_tx = malloc(sizeof(CryptoBackupEvent));
    FURI_LOG_D(TAG, "Cmd: %d", event_rx->cmd);

    switch(event_rx->cmd) {
    case CryptoBackupCmdWrite:
        FURI_LOG_D(TAG, "Set command received");

        event_tx->cmd = CryptoBackupCmdAsk;
        event_tx->data_size = 0;

        status = sl_si91x_command_to_write_common_flash(
            event_rx->address + FURI_HAL_CRYPTO_STORAGE_START_ADDRESS,
            event_rx->data,
            event_rx->data_size,
            0);
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to write to NWP flash: 0x%08lx", status);
            event_tx->cmd = CryptoBackupCmdNack;
        }
        crypto_backup_server_tx(instance, event_tx);

        break;
    case CryptoBackupCmdRead:
        FURI_LOG_D(TAG, "Get command received");

        event_tx->data_size = 0;
        event_tx->cmd = CryptoBackupCmdAsk;
        crypto_backup_server_tx(instance, event_tx);
        // ToDo add some delay to avoid flooding
        furi_delay_ms(10);

        event_tx->cmd = CryptoBackupCmdRead;
        event_tx->data_size = CRYPTO_BACKUP_COMMON_BUFFER_SIZE;

        uint8_t counter = 0;
        for(uint32_t i = FURI_HAL_CRYPTO_STORAGE_START_ADDRESS;
            i < FURI_HAL_CRYPTO_STORAGE_END_ADDRESS;
            i += CRYPTO_BACKUP_COMMON_BUFFER_SIZE) {
            status = sl_si91x_command_to_read_common_flash(
                i, CRYPTO_BACKUP_COMMON_BUFFER_SIZE, event_tx->data);
            if(status != SL_STATUS_OK) {
                FURI_LOG_E(TAG, "Failed to read from NWP flash: 0x%08lx", status);
                break;
            }
            crypto_backup_server_tx(instance, event_tx);
            FURI_LOG_T(
                TAG,
                "Transmitted %d packets, %ld  bytes",
                ++counter,
                i + CRYPTO_BACKUP_COMMON_BUFFER_SIZE);
            // ToDo add some delay to avoid flooding
            furi_delay_ms(10);
        }
        break;
    // TODO: Remove below 2 commands from client and from here
    case CryptoBackupCmdNwpInit:
        FURI_LOG_D(TAG, "NWP Init command received");
        event_tx->cmd = CryptoBackupCmdAsk;
        event_tx->data_size = 0;
        crypto_backup_server_tx(instance, event_tx);
        break;
    case CryptoBackupCmdNwpDeinit:
        FURI_LOG_D(TAG, "NWP Deinit command received");
        event_tx->cmd = CryptoBackupCmdAsk;
        event_tx->data_size = 0;
        crypto_backup_server_tx(instance, event_tx);
        break;
    case CryptoBackupCmdUserDataWipe:
        FURI_LOG_D(TAG, "User Data Wipe command received");
        event_tx->cmd = CryptoBackupCmdAsk;
        event_tx->data_size = 0;

        status = sl_si91x_command_to_write_common_flash(
            FURI_HAL_CRYPTO_STORAGE_START_ADDRESS, NULL, FURI_HAL_CRYPTO_STORAGE_END_ADDRESS, 1);
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to wipe NWP flash: 0x%08lx", status);
            event_tx->cmd = CryptoBackupCmdNack;
        } else {
            FURI_LOG_D(TAG, "Wiped NWP flash");
        }
        crypto_backup_server_tx(instance, event_tx);
        break;

    default:
        FURI_LOG_E(TAG, "Unknown command: %d", event_rx->cmd);
        event_tx->cmd = CryptoBackupCmdError;
        event_tx->data_size = 0;
        break;
    }

    free(event_tx);

    FURI_LOG_D(TAG, "Stopping thread");

    furi_record_close(RECORD_WIFI);

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

    Intercom* intercom = furi_record_open(RECORD_INTERCOM);

    crypto_backup_server.intercom = intercom_channel_open(
        intercom,
        IntercomChannelIdCryptoBackup,
        crypto_backup_server_rx_callback,
        &crypto_backup_server);

    return 0;
}
