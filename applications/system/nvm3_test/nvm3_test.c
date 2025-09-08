#include "nvm3_test.h"
#include "helpers/nvm3_test.h"

#include <furi.h>

#include <sl_status.h>
#include <sl_net.h>
#include <sl_wifi.h>

#include <cli/args.h>
#include <cli/shell/cli_shell.h>
#include <cli/cli_ansi.h>
#include <strint.h>

#define TAG "NVM3 Test"

typedef enum {
    NVM3TestStateIdle,
    NVM3TestStateInit,
} NVM3TestState;

typedef struct {
    FuriString* msg;
    CliShell* shell;
    NVM3TestState state;

    bool exit;
} NVM3TestApp;

static NVM3TestApp* nvm3_test_app_instance = NULL;

void nvm3_test_app_stop(void* app_handle);

void* nvm3_test_app_start(CliShell* shell) {
    FURI_LOG_I(TAG, "Starting");

    nvm3_test_app_instance = malloc(sizeof(NVM3TestApp));
    nvm3_test_app_instance->msg = furi_string_alloc();
    nvm3_test_app_instance->shell = shell;
    nvm3_test_app_instance->state = NVM3TestStateIdle;

    nvm3_test_app_instance->exit = false;
    sl_status_t status = SL_STATUS_FAIL;
    do {
        status = sl_net_init(
            SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_default_concurrent_configuration, NULL, NULL);
        if((status != SL_STATUS_OK) && (status != SL_STATUS_ALREADY_INITIALIZED)) {
            furi_string_printf(
                nvm3_test_app_instance->msg,
                ANSI_FG_RED "Failed to start Wi-Fi client interface: 0x%lx" ANSI_RESET,
                status);
            cli_shell_notification_print(
                nvm3_test_app_instance->shell, nvm3_test_app_instance->msg);
            break;
        }
        furi_string_printf(nvm3_test_app_instance->msg, "Wi-Fi APSTA interface init");
        cli_shell_notification_print(nvm3_test_app_instance->shell, nvm3_test_app_instance->msg);

        if(!nvm3_test_init()) {
            furi_string_printf(
                nvm3_test_app_instance->msg, ANSI_FG_RED "Failed to init NVM3" ANSI_RESET);
            cli_shell_notification_print(
                nvm3_test_app_instance->shell, nvm3_test_app_instance->msg);
            break;
        }

        nvm3_test_app_instance->state = NVM3TestStateInit;
    } while(0);

    return (void*)nvm3_test_app_instance;
}

void nvm3_test_app_stop(void* app_handle) {
    furi_check(app_handle);
    FURI_LOG_I(TAG, "Stopping");
    NVM3TestApp* instance = (NVM3TestApp*)app_handle;

    if(instance->state == NVM3TestStateInit) {
        nvm3_test_deinit();
        sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
    }

    if(instance) {
        instance->exit = true;

        furi_string_free(instance->msg);
        free(instance);
        instance = NULL;
    }
}

void nvm3_test_test_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    NVM3TestApp* instance = context;

    uint8_t test_data[] = "Hello World\0";
    uint8_t buffer[256];
    uint32_t count;

    // write - read test
    do {
        if(!nvm3_test_write(2, (uint8_t*)"BSB NVM3 Test\0", 14)) {
            printf("Failed to write data to key 2\r\n");
            break;
        }

        if(!nvm3_test_write(1, test_data, sizeof(test_data))) {
            printf("Failed to write data to key 1\r\n");
            break;
        }
        if(!nvm3_test_read(1, buffer, sizeof(test_data))) {
            printf("Failed to read data from key 1\r\n");
            break;
        }

        if(memcmp(test_data, buffer, sizeof(test_data)) != 0) {
            furi_string_printf(
                instance->msg, "Data read from key 1 is not the same as written\r\n");
            break;
        }

        printf("Wrire-Read key 1 OK\r\n");
    } while(false);

    // delete test
    do {
        if(!nvm3_test_delete(1)) {
            printf("Failed to delete key 1\r\n");
            break;
        }
        if(nvm3_test_read(1, buffer, sizeof(test_data))) {
            printf("Data read from key 1 after delete\r\n");
            break;
        }

        printf("Delete key 1 OK\r\n");
    } while(false);

    // counter test
    do {
        if(!nvm3_test_write_counter(10, 5)) {
            printf("Failed to write counter 10\r\n");
            break;
        }
        if(!nvm3_test_read_counter(10, &count)) {
            printf("Failed to read counter 10\r\n");
            break;
        }

        if(count != 5) {
            printf("Counter 10 is not 5\r\n");
            break;
        }

        if(!nvm3_test_increment_counter(10, NULL)) {
            printf("Failed to increment counter 10\r\n");
            break;
        }

        if(!nvm3_test_read_counter(10, &count)) {
            printf("Failed to read counter 10\r\n");
            break;
        }

        if(count != 6) {
            printf("Counter 10 is not 6\r\n");
            break;
        }

        if(!nvm3_test_increment_counter(10, NULL)) {
            printf("Failed to increment counter 10\r\n");
            break;
        }

        if(!nvm3_test_read_counter(10, &count)) {
            printf("Failed to read counter 10\r\n");
            break;
        }

        if(count != 7) {
            printf("Counter 10 is not 7\r\n");
            break;
        }

        printf("Counter 10 OK\r\n");
    } while(false);
}

static void nvm3_test_erase_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    printf("Erasing nvm storage\r\n");

    if(nvm3_test_erase_all()) {
        printf("nvm storage erased successfully\r\n");
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

    CliRegistry* registry = cli_registry_alloc();
    cli_registry_add_command(
        registry, "test", CliCommandFlagDefault, nvm3_test_test_command, NULL);
    cli_registry_add_command(
        registry, "print", CliCommandFlagDefault, nvm3_test_print_command, NULL);
    cli_registry_add_command(
        registry, "erase", CliCommandFlagDefault, nvm3_test_erase_command, NULL);

    CliShell* shell = cli_shell_alloc(nvm3_test_motd, NULL, pipe, registry, NULL);
    cli_shell_set_prompt(shell, "crypto_test");

    cli_shell_start(shell);
    NVM3TestApp* app = nvm3_test_app_start(shell);
    cli_shell_join(shell);
    nvm3_test_app_stop(app);

    cli_shell_free(shell);
    cli_registry_free(registry);
}
