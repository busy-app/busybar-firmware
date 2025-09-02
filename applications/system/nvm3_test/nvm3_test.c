#include "nvm3_test.h"

#include <furi.h>
#include <strint.h>

#include <cli/args.h>
#include <cli/cli_ansi.h>
#include <cli/shell/cli_shell.h>

#include <nvm/nvm.h>

#define TAG "NvmTest"

#define COUNTER_INIT_VAL (5UL)
#define COUNTER_INC_MAX  (3UL)
#define MAX_DATA_LEN     (100UL)

typedef struct {
    Nvm* nvm;
} NvmTestApp;

enum {
    NvmTestKeyData = NvmKeyUser1Min,
    NvmTestKeyCounter,
};

NvmTestApp* nvm_test_app_alloc(void) {
    NvmTestApp* instance = malloc(sizeof(NvmTestApp));
    instance->nvm = furi_record_open(RECORD_NVM);
    return instance;
}

void nvm_test_app_free(NvmTestApp* instance) {
    furi_check(instance);

    furi_record_close(RECORD_NVM);

    free(instance);
}

void nvm3_test_test_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);

    furi_assert(context);
    NvmTestApp* instance = context;

    const uint8_t test_data[] = "Hello World";

    uint8_t buffer[sizeof(test_data)];
    uint32_t count;

    // write - read test
    do {
        if(!nvm_write(instance->nvm, NvmTestKeyData, test_data, sizeof(test_data))) {
            printf("Failed to write data to key 0x%X\r\n", NvmTestKeyData);
            break;
        }
        if(!nvm_read(instance->nvm, NvmTestKeyData, buffer, sizeof(test_data))) {
            printf("Failed to read data from key 0x%X\r\n", NvmTestKeyData);
            break;
        }
        if(memcmp(test_data, buffer, sizeof(test_data)) != 0) {
            printf("Data mismatch at key 0x%X", NvmTestKeyData);
            break;
        }

        printf("Write/read key 0x%X OK\r\n", NvmTestKeyData);

    } while(false);

    // delete test
    do {
        if(!nvm_delete(instance->nvm, NvmTestKeyData)) {
            printf("Failed to delete key 0x%X\r\n", NvmTestKeyData);
            break;
        }
        if(nvm_read(instance->nvm, NvmTestKeyData, buffer, sizeof(test_data))) {
            printf("Data read from key 1 after delete\r\n");
            break;
        }

        printf("Delete key 0x%X OK\r\n", NvmTestKeyData);
    } while(false);

    // counter test
    do {
        if(!nvm_write_counter(instance->nvm, NvmTestKeyCounter, COUNTER_INIT_VAL)) {
            printf("Failed to write counter 0x%X\r\n", NvmTestKeyCounter);
            break;
        }
        if(!nvm_read_counter(instance->nvm, NvmTestKeyCounter, &count)) {
            printf("Failed to read counter 0x%X\r\n", NvmTestKeyCounter);
            break;
        }
        if(count != COUNTER_INIT_VAL) {
            printf("Counter 0x%X: %lu != %lu\r\n", NvmTestKeyCounter, count, COUNTER_INIT_VAL);
            break;
        }

        uint32_t i;

        for(i = 1; i < COUNTER_INC_MAX; ++i) {
            if(!nvm_increment_counter(instance->nvm, NvmTestKeyCounter, NULL)) {
                printf("Failed to increment counter 0x%X\r\n", NvmTestKeyCounter);
                break;
            }
            if(!nvm_read_counter(instance->nvm, NvmTestKeyCounter, &count)) {
                printf("Failed to read counter 0x%X\r\n", NvmTestKeyCounter);
                break;
            }
            if(count != COUNTER_INIT_VAL + i) {
                printf(
                    "Counter 0x%X: %lu != %lu\r\n", NvmTestKeyCounter, count, COUNTER_INIT_VAL + i);
                break;
            }
        }

        if(i != COUNTER_INC_MAX) {
            break;
        }

        printf("Counter 0x%X OK\r\n", NvmTestKeyCounter);
    } while(false);
}

static void nvm3_test_write_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);

    furi_assert(context);
    NvmTestApp* instance = context;

    static const char* usage = "Usage: write <key> <hex data>\r\n";

    do {
        uint32_t key;
        if(!args_read_int_and_trim(args, (int*)&key)) {
            printf("%s", usage);
            break;
        }

        const size_t arg_len = args_get_first_word_length(args);

        if(arg_len < 2 || arg_len % 2) {
            printf("%s", usage);
            break;
        }

        const size_t data_len = arg_len / 2;

        if(data_len > MAX_DATA_LEN) {
            printf("Data too long: %zu > %lu\r\n", data_len, MAX_DATA_LEN);
            break;
        }

        uint8_t data[MAX_DATA_LEN];

        if(!args_read_hex_bytes(args, data, data_len)) {
            printf("%s", usage);
            break;
        }

        if(!nvm_write(instance->nvm, key, data, data_len)) {
            printf("Failed to write %zu bytes to key 0x%lX\r\n", data_len, key);
            break;
        }

        printf("Wrote %zu bytes to key 0x%lX\r\n", data_len, key);

    } while(false);
}

static void nvm3_test_read_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);

    furi_assert(context);
    NvmTestApp* instance = context;

    static const char* usage = "Usage: read <key>\r\n";

    do {
        uint32_t key;
        if(!args_read_int_and_trim(args, (int*)&key)) {
            printf("%s", usage);
            break;
        }

        size_t data_len;
        if(!nvm_exists(instance->nvm, key, &data_len)) {
            printf("Failed to find key 0x%lX\r\n", key);
            break;
        }

        data_len = MIN(data_len, MAX_DATA_LEN);

        uint8_t data[MAX_DATA_LEN];

        if(!nvm_read(instance->nvm, key, data, data_len)) {
            printf("Failed to read %zu bytes from key 0x%lX\r\n", data_len, key);
            break;
        }

        printf("Read %zu bytes: ", data_len);

        for(uint32_t i = 0; i < data_len; ++i) {
            printf("%hhX", data[i]);
        }

        printf("\r\n");

    } while(false);
}

static void nvm3_test_del_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);

    furi_assert(context);
    NvmTestApp* instance = context;

    static const char* usage = "Usage: del <key>\r\n";

    do {
        uint32_t key;
        if(!args_read_int_and_trim(args, (int*)&key)) {
            printf("%s", usage);
            break;
        }

        if(!nvm_exists(instance->nvm, key, NULL)) {
            printf("Failed to find key 0x%lX\r\n", key);
            break;
        }

        if(!nvm_delete(instance->nvm, key)) {
            printf("Failed to delete key 0x%lX\r\n", key);
            break;
        }

        printf("Deleted key 0x%lX\r\n", key);

    } while(false);
}

static void nvm3_test_erase_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);

    furi_assert(context);
    NvmTestApp* instance = context;

    printf("Erasing nvm storage...\r\n");

    if(nvm_erase_all(instance->nvm)) {
        printf("Successfully erased nvm storage\r\n");
    } else {
        printf("Failed to erase nvm storage\r\n");
    }
}

static void nvm3_test_motd(void* context) {
    UNUSED(context);
    printf("\r\n+-----------------------------+\r\n");
    printf("| Welcome to nvm3 test shell! |\r\n");
    printf("+-----------------------------+\r\n\r\n");
    printf(
        "Read the manual: https://docs.silabs.com/gecko-platform/latest/platform-driver/nvm3\r\n");
    printf(
        "Read the manual: https://docs.silabs.com/gecko-platform/3.0/driver/api/group-nvm3\r\n");
}

void nvm3_test_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

    NvmTestApp* instance = nvm_test_app_alloc();
    CliRegistry* registry = cli_registry_alloc();

    cli_registry_add_command(
        registry, "read", CliCommandFlagDefault, nvm3_test_read_command, instance);
    cli_registry_add_command(
        registry, "write", CliCommandFlagDefault, nvm3_test_write_command, instance);
    cli_registry_add_command(
        registry, "del", CliCommandFlagDefault, nvm3_test_del_command, instance);
    cli_registry_add_command(
        registry, "test", CliCommandFlagDefault, nvm3_test_test_command, instance);
    cli_registry_add_command(
        registry, "erase", CliCommandFlagDefault, nvm3_test_erase_command, instance);

    CliShell* shell = cli_shell_alloc(nvm3_test_motd, NULL, pipe, registry, NULL);
    cli_shell_set_prompt(shell, "nvm_test");

    cli_shell_start(shell);
    cli_shell_join(shell);
    cli_shell_free(shell);

    cli_registry_free(registry);

    nvm_test_app_free(instance);
}
