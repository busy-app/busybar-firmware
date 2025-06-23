#include "wifi_cli_test.h"
#include "wifi_cli_wifi_commands.h"

#include <furi.h>
#include <cli/args.h>
#include <strint.h>

void wifi_cli_test_command_wifi_init(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    console_args_t arg = {.arg[0] = 2, .bitmap = 0x7f};
    if(furi_string_size(args)) {
        if(strint_to_uint32(furi_string_get_cstr(args), NULL, &arg.arg[0], 10) !=
           StrintParseNoError) {
            cli_print_usage(
                "wifi_cli_test wifi_init",
                "wifi_init Init Wi-Fi interface \"ap-1\", \"apsta-2\", \"ble_coex\", \"client-0\", \"client_ipv6\", \"eap-3\", \"transmit_test-6\"\r\n",
                furi_string_get_cstr(args));
        }
    }
    sl_status_t status = wifi_init_command_handler(&arg);
    if(status != SL_STATUS_OK) {
        printf("Failed to start Wi-Fi interface: 0x%lx\r\n", status);
    }
}

void wifi_cli_test_command_wifi_deinit(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    UNUSED(args);

    sl_status_t status = wifi_deinit_command_handler(NULL);
    if(status != SL_STATUS_OK) {
        printf("Failed to stop Wi-Fi interface: 0x%lx\r\n", status);
    }
}

void wifi_cli_test_command_wifi_scan(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    UNUSED(pipe);
    UNUSED(args);
    if(furi_string_size(args)) {
        printf("wifi_cli_test wifi_scan not used arg\r\n");
    }

    sl_status_t status = wifi_scan_command_handler(NULL);
    if(status != SL_STATUS_OK) {
        printf("Failed to scan Wi-Fi: 0x%lx\r\n", status);
    }
}

static void wifi_cli_test_command_print_usage(void) {
    printf("Usage:\r\n");
    printf("wifi_cli_test <cmd> <args>\r\n");
    printf("Cmd list:\r\n");

    printf(
        "\twifi_init Init Wi-Fi interface \"ap-1\", \"apsta-2\", \"ble_coex\", \"client-0\", \"client_ipv6\", \"eap-3\", \"transmit_test-6\"\r\n");
    printf("\twifi_deinit Deinit Wi-Fi interface\r\n");
    printf("\twifi_scan Start Wi-Fi scanning\r\n");
}

void wifi_cli_test_command(PipeSide* pipe, FuriString* args, void* context) {
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            wifi_cli_test_command_print_usage();
            break;
        }

        if(furi_string_cmp_str(cmd, "wifi_init") == 0) {
            wifi_cli_test_command_wifi_init(pipe, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "wifi_deinit") == 0) {
            wifi_cli_test_command_wifi_deinit(pipe, args, context);
            break;
        }
        if(furi_string_cmp_str(cmd, "wifi_scan") == 0) {
            wifi_cli_test_command_wifi_scan(pipe, args, context);
            break;
        }

        wifi_cli_test_command_print_usage();
    } while(false);

    furi_string_free(cmd);
}
