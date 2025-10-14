#include "crypto_backup_client.h"
#include "crypto_backup_common.h"

#include <furi.h>
#include <cli/args.h>
#include <cli/cli_ansi.h>
#include <cli/cli_status.h>
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
    IntercomChannel* intercom;
    FuriStreamBuffer* rx_buffer;
    CryptoBackupCmd cmd;
    FuriSemaphore* access_semaphore;
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
    instance->cmd = event_rx->cmd;
    switch(event_rx->cmd) {
    case CryptoBackupCmdAsk:
        // Handle ask command
        break;
    case CryptoBackupCmdNack:
        // Handle nack command
        break;
    case CryptoBackupCmdRead:
        // Handle get command
        furi_check(
            furi_stream_buffer_send(
                instance->rx_buffer, event_rx->data, event_rx->data_size, FuriWaitForever) ==
            event_rx->data_size);
        break;
    case CryptoBackupCmdWrite:
        // Handle set command
        break;
    default:
        furi_crash();
        break;
    }
    furi_semaphore_release(instance->access_semaphore);
}

static void crypto_backup_client_data_tx(CryptoBackup* instance, CryptoBackupEvent* event) {
    size_t tx_size =
        intercom_tx(instance->intercom, event, sizeof(CryptoBackupEvent), FuriWaitForever);
    furi_check(tx_size == sizeof(CryptoBackupEvent), "Failed to send data");
}

static CryptoBackup* crypto_backup_client_init() {
    CryptoBackup* instance = malloc(sizeof(CryptoBackup));
    instance->access_semaphore = furi_semaphore_alloc(1, 0);
    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    instance->intercom = intercom_channel_open(
        intercom,
        IntercomChannelIdCryptoBackup,
        FuriWaitForever,
        crypto_backup_client_rx_callback,
        instance);
    instance->rx_buffer = furi_stream_buffer_alloc(CRYPTO_BACKUP_COMMON_USERDATA_SIZE, 1);
    return instance;
}

static void crypto_backup_client_deinit(CryptoBackup* instance) {
    furi_check(instance);
    intercom_channel_close(instance->intercom);
    furi_record_close(RECORD_INTERCOM);
    furi_semaphore_free(instance->access_semaphore);
    furi_stream_buffer_free(instance->rx_buffer);
    free(instance);
    instance = NULL;
}

static bool crypto_backup_client_read_917_user_data(uint8_t* data, size_t data_size) {
    furi_check(data);
    furi_check(data_size == CRYPTO_BACKUP_COMMON_USERDATA_SIZE);

    bool ret = false;
    CryptoBackup* instance = crypto_backup_client_init();

    do {
        // initialize the NWP
        CryptoBackupEvent event = {.cmd = CryptoBackupCmdNwpInit, .data_size = 0, .address = 0};
        crypto_backup_client_data_tx(instance, &event);
        // wait for the NWP to be initialized
        if(furi_semaphore_acquire(instance->access_semaphore, CRYPTO_BACKUP_TIMEOUT) !=
           FuriStatusOk) {
            printf(ANSI_FG_RED "Error: NWP initialization timeout\r\n" ANSI_RESET);
            break;
        }
        if(instance->cmd != CryptoBackupCmdAsk) {
            printf(ANSI_FG_RED "Error: NWP initialization failed\r\n" ANSI_RESET);
            break;
        }
        // read the user data
        event.cmd = CryptoBackupCmdRead;
        crypto_backup_client_data_tx(instance, &event);
        // wait for the user data to be received
        if(furi_semaphore_acquire(instance->access_semaphore, CRYPTO_BACKUP_TIMEOUT) !=
           FuriStatusOk) {
            printf(ANSI_FG_RED "Error: User data read timeout\r\n" ANSI_RESET);
            break;
        }
        if(instance->cmd != CryptoBackupCmdAsk) {
            printf(ANSI_FG_RED "Error: User data read failed\r\n" ANSI_RESET);
            break;
        }

        uint8_t percent = 0;
        int32_t timeout = CRYPTO_BACKUP_TIMEOUT;

        while((furi_stream_buffer_bytes_available(instance->rx_buffer) < data_size) &&
              (timeout > 0)) {
            furi_delay_ms(100);
            percent = (uint8_t)((furi_stream_buffer_bytes_available(instance->rx_buffer) * 100) /
                                data_size);
            printf("\rProgress: %d%%, timeout: %ld s ", percent, timeout / 1000);
            fflush(stdout);
            timeout -= 100;
        }
        furi_semaphore_acquire(instance->access_semaphore, CRYPTO_BACKUP_TIMEOUT);
        // deinit the NWP
        event.cmd = CryptoBackupCmdNwpDeinit;
        crypto_backup_client_data_tx(instance, &event);
        if(furi_semaphore_acquire(instance->access_semaphore, CRYPTO_BACKUP_TIMEOUT) !=
           FuriStatusOk) {
            printf("\r\n" ANSI_FG_RED "Error: NWP deinitialization timeout\r\n" ANSI_RESET);
            break;
        }
        if(instance->cmd != CryptoBackupCmdAsk) {
            printf("\r\n" ANSI_FG_RED "Error: NWP deinitialization failed\r\n" ANSI_RESET);
            break;
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
                ANSI_FG_RED
                "Error: Failed to receive all data, received only %d bytes\r\n" ANSI_RESET,
                furi_stream_buffer_bytes_available(instance->rx_buffer));
        }
    } while(false);

    crypto_backup_client_deinit(instance);
    return ret;
}

static bool crypto_backup_client_write_917_user_data(uint8_t* data, size_t data_size) {
    furi_check(data);
    furi_check(data_size == CRYPTO_BACKUP_COMMON_USERDATA_SIZE);

    bool ret = false;
    CryptoBackup* instance = crypto_backup_client_init();

    do {
        // initialize the NWP
        CryptoBackupEvent event = {.cmd = CryptoBackupCmdNwpInit, .data_size = 0, .address = 0};
        crypto_backup_client_data_tx(instance, &event);
        // wait for the NWP to be initialized
        if(furi_semaphore_acquire(instance->access_semaphore, CRYPTO_BACKUP_TIMEOUT) !=
           FuriStatusOk) {
            printf(ANSI_FG_RED "Error: NWP initialization timeout\r\n" ANSI_RESET);
            break;
        }
        if(instance->cmd != CryptoBackupCmdAsk) {
            printf(ANSI_FG_RED "Error: NWP initialization failed\r\n" ANSI_RESET);
            break;
        }

        // wipe the user data
        event.cmd = CryptoBackupCmdUserDataWipe;
        crypto_backup_client_data_tx(instance, &event);
        if(furi_semaphore_acquire(instance->access_semaphore, CRYPTO_BACKUP_TIMEOUT) !=
           FuriStatusOk) {
            printf(ANSI_FG_RED "Error: User data wipe timeout\r\n" ANSI_RESET);
            break;
        }
        if(instance->cmd != CryptoBackupCmdAsk) {
            printf(ANSI_FG_RED "Error: User data wipe failed\r\n" ANSI_RESET);
            break;
        }

        // write the user data
        event.cmd = CryptoBackupCmdWrite;
        event.data_size = CRYPTO_BACKUP_COMMON_BUFFER_SIZE;
        size_t offset = 0;
        uint8_t percent = 0;
        bool write_error = true;
        while(offset < data_size) {
            size_t chunk_size = (data_size - offset > CRYPTO_BACKUP_COMMON_BUFFER_SIZE) ?
                                    CRYPTO_BACKUP_COMMON_BUFFER_SIZE :
                                    data_size - offset;
            event.address = offset;
            memcpy(event.data, data + offset, chunk_size);
            crypto_backup_client_data_tx(instance, &event);
            // wait for the user data to be set
            if(furi_semaphore_acquire(instance->access_semaphore, CRYPTO_BACKUP_TIMEOUT) !=
               FuriStatusOk) {
                printf(ANSI_FG_RED "Error: User data write timeout\r\n" ANSI_RESET);
                break;
            }
            if(instance->cmd != CryptoBackupCmdAsk) {
                printf(ANSI_FG_RED "Error: User data write failed\r\n" ANSI_RESET);
                break;
            }
            percent = (uint8_t)((offset + chunk_size) * 100 / data_size);
            printf("\rProgress: %d%%, write: %u bytes ", percent, offset);
            fflush(stdout);
            offset += chunk_size;
        }
        if(offset == data_size) {
            printf("\rProgress: 100%%, write: %u bytes\r\n", offset);
            write_error = false;
        }

        // deinit the NWP
        event.cmd = CryptoBackupCmdNwpDeinit;
        crypto_backup_client_data_tx(instance, &event);
        if(furi_semaphore_acquire(instance->access_semaphore, CRYPTO_BACKUP_TIMEOUT) !=
           FuriStatusOk) {
            printf("\r\n" ANSI_FG_RED "Error: NWP deinitialization timeout\r\n" ANSI_RESET);
            break;
        }
        if(instance->cmd != CryptoBackupCmdAsk) {
            printf("\r\n" ANSI_FG_RED "Error: NWP deinitialization failed\r\n" ANSI_RESET);
            break;
        }

        if(!write_error) {
            ret = true;
        }
    } while(false);

    crypto_backup_client_deinit(instance);
    return ret;
}

void crypto_backup_client_create(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    UNUSED(args);

    uint8_t* buf = malloc(CRYPTO_BACKUP_COMMON_USERDATA_SIZE);

    if(crypto_backup_client_read_917_user_data(buf, CRYPTO_BACKUP_COMMON_USERDATA_SIZE)) {
        if(crypto_backup_client_file_write(buf, CRYPTO_BACKUP_COMMON_USERDATA_SIZE)) {
            printf(
                ANSI_FG_GREEN "Data successfully created to %s\r\n" ANSI_RESET,
                CRYPTO_BACKUP_FILE_PATH);
            printf(CLI_STATUS_OK);
        } else {
            printf(
                ANSI_FG_RED "Error: Failed to create to %s\r\n" ANSI_RESET,
                CRYPTO_BACKUP_FILE_PATH);
            printf(CLI_STATUS_ERROR);
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
            printf(CLI_STATUS_OK);
        } else {
            printf(
                ANSI_FG_RED "Error: Failed to remove file %s\r\n" ANSI_RESET,
                CRYPTO_BACKUP_FILE_PATH);
            printf(CLI_STATUS_ERROR);
        }
    } else {
        printf(
            ANSI_FG_RED
            "Deleting this file in the future may result in the device not functioning properly.\r\n" ANSI_RESET);
        printf(CLI_STATUS_ERROR);
        return;
    }
}

void crypto_backup_client_restore(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    UNUSED(args);

    uint8_t* buf = malloc(CRYPTO_BACKUP_COMMON_USERDATA_SIZE);

    if(crypto_backup_client_file_read(buf, CRYPTO_BACKUP_COMMON_USERDATA_SIZE)) {
        if(crypto_backup_client_write_917_user_data(buf, CRYPTO_BACKUP_COMMON_USERDATA_SIZE)) {
            printf(
                ANSI_FG_GREEN "Data successfully restored to %s\r\n" ANSI_RESET,
                CRYPTO_BACKUP_FILE_PATH);
            printf(CLI_STATUS_OK);
        } else {
            printf(
                ANSI_FG_RED "Error: Failed to restore data to %s\r\n" ANSI_RESET,
                CRYPTO_BACKUP_FILE_PATH);
            printf(CLI_STATUS_ERROR);
        }
    } else {
        printf(
            ANSI_FG_RED "Error: Failed to read data from %s\r\n" ANSI_RESET,
            CRYPTO_BACKUP_FILE_PATH);
        printf(CLI_STATUS_ERROR);
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
        if(!crypto_backup_client_read_917_user_data(
               buf_reference, CRYPTO_BACKUP_COMMON_USERDATA_SIZE)) {
            printf(ANSI_FG_RED "Error: Failed to read user data\r\n" ANSI_RESET);
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
        printf(
            ANSI_FG_GREEN "Data verification successful to %s\r\n" ANSI_RESET,
            CRYPTO_BACKUP_FILE_PATH);
        printf(CLI_STATUS_OK);
    } else {
        printf(
            ANSI_FG_RED "Error: Data verification failed to %s\r\n" ANSI_RESET,
            CRYPTO_BACKUP_FILE_PATH);
        printf(CLI_STATUS_ERROR);
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
