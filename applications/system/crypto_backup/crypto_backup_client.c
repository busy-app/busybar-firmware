#include "crypto_backup_client.h"

#include <furi.h>
#include <cli/args.h>
#include <cli/cli_ansi.h>
#include <intercom/intercom.h>

#include <storage/storage_backup.h>

#define TAG "CryptoBackup"

#define CRYPTO_BACKUP_FILE_NAME "crypto_backup.bin"
#define CRYPTO_BACKUP_FILE_PATH BACKUP_PATH(CRYPTO_BACKUP_FILE_NAME)

#define CRYPTO_BACKUP_BUFFER_SIZE 1024 * 20

typedef void (*CryptoBackupClientCallback)(PipeSide* pipe, FuriString* args, void* context);
typedef struct {
    const char* command;
    const char* help;
    const CryptoBackupClientCallback impl;
} CryptoBackupClient;

typedef struct {
    Intercom* intercom;
    FuriStreamBuffer* rx_buffer;
    FuriThread* thread;
} CryptoBackup;

static bool crypto_backup_client_file_write(uint8_t* data, size_t data_size) {
    furi_check(data);
    furi_check(data_size == CRYPTO_BACKUP_BUFFER_SIZE);

    bool ret = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    // unlock backup storage
    storage_backup_set_readonly(storage, false);

    do {
        if(!storage_file_open(file, CRYPTO_BACKUP_FILE_PATH, FSAM_WRITE, FSOM_CREATE_NEW)) {
            FURI_LOG_E(TAG, "File already exists");
            break;
        }

        size_t bytes_written = storage_file_write(file, data, data_size);
        storage_file_close(file);

        if(bytes_written != data_size) {
            FURI_LOG_E(TAG, "Failed to write data to file");
            break;
        }
        ret = true;
    } while(false);
    storage_file_free(file);
    // lock backup storage
    storage_backup_set_readonly(storage, true);

    furi_record_close(RECORD_STORAGE);
    return ret;
}

static bool crypto_backup_client_file_read(uint8_t* data, size_t data_size) {
    furi_check(data);
    furi_check(data_size == CRYPTO_BACKUP_BUFFER_SIZE);

    bool ret = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        if(!storage_file_open(file, CRYPTO_BACKUP_FILE_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Failed to open file");
            break;
        }

        size_t bytes_read = storage_file_read(file, data, data_size);
        storage_file_close(file);

        if(bytes_read != data_size) {
            FURI_LOG_E(TAG, "Failed to read data from file");
            break;
        }
        ret = true;
    } while(false);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return ret;
}

static bool crypto_backup_client_file_delete(void) {
    bool ret = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    // unlock backup storage
    storage_backup_set_readonly(storage, false);

    do {
        if(!storage_simply_remove(storage, CRYPTO_BACKUP_FILE_PATH)) {
            FURI_LOG_E(TAG, "Failed to delete file");
            break;
        }
        ret = true;
    } while(false);
    storage_file_free(file);
    // lock backup storage
    storage_backup_set_readonly(storage, true);

    furi_record_close(RECORD_STORAGE);
    return ret;
}

static void crypto_backup_client_rx_callback(const void* data, size_t data_size, void* context) {
    furi_check(data);
    furi_check(context);

    CryptoBackup* instance = context;
    furi_check(
        furi_stream_buffer_send(instance->rx_buffer, data, data_size, FuriWaitForever) ==
        data_size);
    // furi_thread_flags_set(
    //     furi_thread_get_id(ble_per_cli_instance->thread), BlePerCliThreadEventRxData);
}

// static void crypto_backup_client_data_tx(uint8_t* data, size_t data_size, CryptoBackup* instance) {
//     furi_check(data);
//     furi_check(data_size);
//     furi_check(instance);

//     const size_t tx_size = intercom_tx(
//         instance->intercom, IntercomChannelCryptoBackup, data, data_size, FuriWaitForever);
//     furi_assert(tx_size == data_size);
// }

void crypto_backup_client_get(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    UNUSED(args);

    printf(ANSI_FG_GREEN "Read data from NWP flash address: " ANSI_RESET "\r\n");

    CryptoBackup* instance = malloc(sizeof(CryptoBackup));
    instance->intercom = furi_record_open(RECORD_INTERCOM);
    intercom_set_rx_callback(
        instance->intercom,
        IntercomChannelCryptoBackup,
        crypto_backup_client_rx_callback,
        instance);
    instance->rx_buffer = furi_stream_buffer_alloc(CRYPTO_BACKUP_BUFFER_SIZE, 1);

    uint8_t data[] = "Hello from crypto backup client!";
    size_t tx_size = intercom_tx(
        instance->intercom, IntercomChannelCryptoBackup, data, sizeof(data), FuriWaitForever);
    furi_check(tx_size == sizeof(data), "Failed to send data");

    uint8_t percent = 0;
    int32_t timeout = 15000; // 15 seconds timeout

    while((furi_stream_buffer_bytes_available(instance->rx_buffer) < CRYPTO_BACKUP_BUFFER_SIZE) &&
          (timeout > 0)) {
        furi_delay_ms(100);
        percent = (uint8_t)((furi_stream_buffer_bytes_available(instance->rx_buffer) * 100) /
                            CRYPTO_BACKUP_BUFFER_SIZE);
        printf("\rProgress: %d%%, timeout: %ld s ", percent, timeout / 1000);
        fflush(stdout);
        timeout -= 100;
    }
    if(furi_stream_buffer_bytes_available(instance->rx_buffer) == CRYPTO_BACKUP_BUFFER_SIZE) {
        printf("\rProgress: 100%% ");
        fflush(stdout);

        printf("\r\n");

        uint8_t* buf = malloc(CRYPTO_BACKUP_BUFFER_SIZE);
        size_t rx_size = furi_stream_buffer_receive(
            instance->rx_buffer, buf, CRYPTO_BACKUP_BUFFER_SIZE, FuriWaitForever);

        for(size_t i = 0; i < rx_size; i++) {
            printf("%02x ", buf[i]);
            if((i + 1) % 32 == 0) {
                printf("\r\n");
            }
        }
        printf("\r\n\r\n\r\n");

        //crypto_backup_client_file_delete();

        if(crypto_backup_client_file_write(buf, rx_size)) {
            printf(
                "\r\n" ANSI_FG_GREEN "Data successfully written to %s\r\n" ANSI_RESET,
                CRYPTO_BACKUP_FILE_PATH);
        } else {
            printf(
                "\r\n" ANSI_FG_RED "Failed to write data to %s\r\n" ANSI_RESET,
                CRYPTO_BACKUP_FILE_PATH);
        }

        memset(buf, 0, CRYPTO_BACKUP_BUFFER_SIZE);
        crypto_backup_client_file_read(buf, CRYPTO_BACKUP_BUFFER_SIZE);

        for(size_t i = 0; i < CRYPTO_BACKUP_BUFFER_SIZE; i++) {
            printf("%02x ", buf[i]);
            if((i + 1) % 32 == 0) {
                printf("\r\n");
            }
        }
        printf("\r\n\r\n\r\n");

        free(buf);

    } else {
        printf(
            "\r\n" ANSI_FG_RED "Failed to receive all data, received only %d bytes\r\n" ANSI_RESET,
            furi_stream_buffer_bytes_available(instance->rx_buffer));
    }

    intercom_set_rx_callback(instance->intercom, IntercomChannelCryptoBackup, NULL, NULL);
    furi_record_close(RECORD_INTERCOM);
    furi_stream_buffer_free(instance->rx_buffer);
    free(instance);
    instance = NULL;
}

void crypto_backup_client_remove(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    UNUSED(args);
    if(furi_string_cmp_str(args, "Yes") == 0) {
        crypto_backup_client_file_delete();
        printf(ANSI_FG_RED "File %s removed successfully\r\n" ANSI_RESET, CRYPTO_BACKUP_FILE_PATH);
    } else {
        printf(
            ANSI_FG_RED
            "Deleting this file in the future may result in the device not functioning properly.\r\n" ANSI_RESET);
        return;
    }
}
static const CryptoBackupClient crypto_backup_client_cli_commands[] = {
    {
        "get",
        "backup user_data NWP",
        &crypto_backup_client_get,
    },
    {
        "remove",
        "remove user_data NWP",
        &crypto_backup_client_remove,
    },
};

static void crypto_backup_client_command_print_usage(void) {
    printf("Usage:\r\n");
    printf("crypto_backup <cmd> <args>\r\n");
    printf("Cmd list:\r\n");

    for(size_t i = 0; i < COUNT_OF(crypto_backup_client_cli_commands); ++i) {
        const CryptoBackupClient* command_descr = &crypto_backup_client_cli_commands[i];
        const char* cli_cmd = command_descr->command;
        printf(
            "\t%s%s - %s\r\n", cli_cmd, strlen(cli_cmd) > 8 ? "\t" : "\t\t", command_descr->help);
    }
}

void crypto_backup_client_command(PipeSide* pipe, FuriString* args, void* context) {
    FuriString* cmd = furi_string_alloc();
    FuriString* path = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            crypto_backup_client_command_print_usage();
            break;
        }

        size_t i = 0;
        for(; i < COUNT_OF(crypto_backup_client_cli_commands); ++i) {
            const CryptoBackupClient* command_descr = &crypto_backup_client_cli_commands[i];
            if(furi_string_cmp_str(cmd, command_descr->command) == 0) {
                command_descr->impl(pipe, args, context);
                break;
            }
        }

        if(i == COUNT_OF(crypto_backup_client_cli_commands)) {
            crypto_backup_client_command_print_usage();
        }
    } while(false);

    furi_string_free(path);
    furi_string_free(cmd);
}
