#include "crypto_backup_client.h"
#include "crypto_backup_common.h"

#include <furi.h>
#include <cli/args.h>
#include <cli/cli_ansi.h>
#include <intercom/intercom.h>

#include <storage/storage_backup.h>

#define TAG "CryptoBackup"

#define CRYPTO_BACKUP_FILE_NAME "crypto_backup.bin"
#define CRYPTO_BACKUP_FILE_PATH BACKUP_PATH(CRYPTO_BACKUP_FILE_NAME)

#define CRYPTO_BACKUP_TIMEOUT 15000 // 15 seconds timeout

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
    furi_check(data_size == CRYPTO_BACKUP_COMMON_USERDATA_SIZE);

    bool ret = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

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

    furi_record_close(RECORD_STORAGE);
    return ret;
}

static bool crypto_backup_client_file_read(uint8_t* data, size_t data_size) {
    furi_check(data);
    furi_check(data_size == CRYPTO_BACKUP_COMMON_USERDATA_SIZE);

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

    do {
        if(!storage_simply_remove(storage, CRYPTO_BACKUP_FILE_PATH)) {
            FURI_LOG_E(TAG, "Failed to delete file");
            break;
        }
        ret = true;
    } while(false);
    storage_file_free(file);

    furi_record_close(RECORD_STORAGE);
    return ret;
}

static void crypto_backup_client_rx_callback(const void* data, size_t data_size, void* context) {
    furi_check(data);
    furi_check(context);
    furi_check(data_size == sizeof(CryptoBackupEvent));

    CryptoBackup* instance = context;
    CryptoBackupEvent* event_rx = (CryptoBackupEvent*)data;

    furi_check(
        furi_stream_buffer_send(
            instance->rx_buffer, event_rx->data, event_rx->data_size, FuriWaitForever) ==
        event_rx->data_size);
}

// static void crypto_backup_client_data_tx(uint8_t* data, size_t data_size, CryptoBackup* instance) {
//     furi_check(data);
//     furi_check(data_size);
//     furi_check(instance);

//     const size_t tx_size = intercom_tx(
//         instance->intercom, IntercomChannelCryptoBackup, data, data_size, FuriWaitForever);
//     furi_assert(tx_size == data_size);
// }

static bool crypto_backup_client_get_917_user_data(uint8_t* data, size_t data_size) {
    furi_check(data);
    furi_check(data_size == CRYPTO_BACKUP_COMMON_USERDATA_SIZE);

    bool ret = false;
    CryptoBackup* instance = malloc(sizeof(CryptoBackup));
    instance->intercom = furi_record_open(RECORD_INTERCOM);
    intercom_set_rx_callback(
        instance->intercom,
        IntercomChannelCryptoBackup,
        crypto_backup_client_rx_callback,
        instance);
    instance->rx_buffer = furi_stream_buffer_alloc(data_size, 1);

    CryptoBackupEvent event = {.cmd = CryptoBackupCmdGet, .data_size = 0};

    size_t tx_size = intercom_tx(
        instance->intercom,
        IntercomChannelCryptoBackup,
        &event,
        sizeof(CryptoBackupEvent),
        FuriWaitForever);
    furi_check(tx_size == sizeof(CryptoBackupEvent), "Failed to send data");

    uint8_t percent = 0;
    int32_t timeout = CRYPTO_BACKUP_TIMEOUT;

    while((furi_stream_buffer_bytes_available(instance->rx_buffer) < data_size) && (timeout > 0)) {
        furi_delay_ms(100);
        percent =
            (uint8_t)((furi_stream_buffer_bytes_available(instance->rx_buffer) * 100) / data_size);
        printf("\rProgress: %d%%, timeout: %ld s ", percent, timeout / 1000);
        fflush(stdout);
        timeout -= 100;
    }

    if(furi_stream_buffer_bytes_available(instance->rx_buffer) == data_size) {
        printf("\rProgress: 100%% ");
        fflush(stdout);

        printf("\r\n");

        size_t rx_size =
            furi_stream_buffer_receive(instance->rx_buffer, data, data_size, FuriWaitForever);
        if(rx_size != data_size) {
            printf(
                ANSI_FG_RED
                "Error: Failed to receive all data, received only %d bytes\r\n" ANSI_RESET,
                rx_size);
        } else {
            ret = true;
        }

    } else {
        printf(
            "\r\n" ANSI_FG_RED
            "Error: Failed to receive all data, received only %d bytes\r\n" ANSI_RESET,
            furi_stream_buffer_bytes_available(instance->rx_buffer));
    }

    intercom_set_rx_callback(instance->intercom, IntercomChannelCryptoBackup, NULL, NULL);
    furi_record_close(RECORD_INTERCOM);
    furi_stream_buffer_free(instance->rx_buffer);
    free(instance);
    instance = NULL;
    return ret;
}

void crypto_backup_client_create(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    UNUSED(args);

    uint8_t* buf = malloc(CRYPTO_BACKUP_COMMON_USERDATA_SIZE);

    if(crypto_backup_client_get_917_user_data(buf, CRYPTO_BACKUP_COMMON_USERDATA_SIZE)) {
        if(crypto_backup_client_file_write(buf, CRYPTO_BACKUP_COMMON_USERDATA_SIZE)) {
            printf(
                ANSI_FG_GREEN "Data successfully written to %s\r\n" ANSI_RESET,
                CRYPTO_BACKUP_FILE_PATH);
            printf("RET: 0\r\n");
        } else {
            printf(
                ANSI_FG_RED "Error: Failed to write data to %s\r\n" ANSI_RESET,
                CRYPTO_BACKUP_FILE_PATH);
            printf("RET: 1\r\n");
        }
    }

    free(buf);
}

void crypto_backup_client_remove(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    UNUSED(args);
    if(furi_string_cmp_str(args, "Yes") == 0) {
        if(crypto_backup_client_file_delete()) {
            printf(
                ANSI_FG_GREEN "File %s removed successfully\r\n" ANSI_RESET,
                CRYPTO_BACKUP_FILE_PATH);
            printf("RET: 0\r\n");
        } else {
            printf(
                ANSI_FG_RED "Error: Failed to remove file %s\r\n" ANSI_RESET,
                CRYPTO_BACKUP_FILE_PATH);
            printf("RET: 1\r\n");
        }
    } else {
        printf(
            ANSI_FG_RED
            "Deleting this file in the future may result in the device not functioning properly.\r\n" ANSI_RESET);
        printf("RET: 1\r\n");
        return;
    }
}

void crypto_backup_client_restore(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    UNUSED(args);

    uint8_t* buf = malloc(CRYPTO_BACKUP_COMMON_USERDATA_SIZE);

    if(crypto_backup_client_file_read(buf, CRYPTO_BACKUP_COMMON_USERDATA_SIZE)) {
        printf(
            ANSI_FG_GREEN "Data successfully read from %s\r\n" ANSI_RESET,
            CRYPTO_BACKUP_FILE_PATH);
        printf("RET: 0\r\n");

        for(size_t i = 0; i < CRYPTO_BACKUP_COMMON_USERDATA_SIZE; i++) {
            printf("%02x ", buf[i]);
            if((i + 1) % 32 == 0) {
                printf("\r\n");
            }
        }
    } else {
        printf(
            ANSI_FG_RED "Error: Failed to read data from %s\r\n" ANSI_RESET,
            CRYPTO_BACKUP_FILE_PATH);
        printf("RET: 1\r\n");
    }

    free(buf);
}

void crypto_backup_client_verify(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    UNUSED(args);
    bool ret = false;
    uint8_t* buf_reference = malloc(CRYPTO_BACKUP_COMMON_USERDATA_SIZE);
    uint8_t* buf = malloc(CRYPTO_BACKUP_COMMON_USERDATA_SIZE);

    do {
        if(!crypto_backup_client_get_917_user_data(
               buf_reference, CRYPTO_BACKUP_COMMON_USERDATA_SIZE)) {
            printf(ANSI_FG_RED "Error: Failed to get user data\r\n" ANSI_RESET);
            break;
        }
        if(!crypto_backup_client_file_read(buf, CRYPTO_BACKUP_COMMON_USERDATA_SIZE)) {
            printf(ANSI_FG_RED "Error: Failed to read data from file\r\n" ANSI_RESET);
            break;
        }

        if(memcmp(buf, buf_reference, CRYPTO_BACKUP_COMMON_USERDATA_SIZE)) {
            break;
        }
        ret = true;
    } while(false);

    free(buf);
    free(buf_reference);

    if(ret) {
        printf(ANSI_FG_GREEN "Data verification successful\r\n" ANSI_RESET);
        printf("RET: 0\r\n");
    } else {
        printf(ANSI_FG_RED "Error: Data verification failed\r\n" ANSI_RESET);
        printf("RET: 1\r\n");
    }
}

static const CryptoBackupClient crypto_backup_client_cli_commands[] = {
    {
        "create",
        "create user_data NWP",
        &crypto_backup_client_create,
    },
    {
        "remove",
        "remove user_data NWP",
        &crypto_backup_client_remove,
    },
    {
        "restore",
        "restore user_data NWP",
        &crypto_backup_client_restore,
    },
    {
        "verify",
        "verify user_data NWP",
        &crypto_backup_client_verify,
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
