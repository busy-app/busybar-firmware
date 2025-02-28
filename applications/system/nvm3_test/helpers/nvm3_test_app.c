#include "nvm3_test_app.h"
#include "nvm3_test.h"

#include <furi.h>

#include <sl_status.h>
#include <sl_net.h>
#include <sl_wifi.h>

#include <args.h>
#include <strint.h>

#define TAG "NVM3 Test"

typedef enum {
    NVM3TestCmdTypeHelp,
    NVM3TestCmdTypeHelpHelp,
    NVM3TestCmdTypeNVM3,
    NVM3TestCmdTypeNVM3Print,
    NVM3TestCmdTypeMax,
} NVM3TestCmdType;

typedef enum {
    NVM3TestStateIdle,
    NVM3TestStateInit,
} NVM3TestState;

typedef struct {
    char* cmd;
} NVM3TestCmd;

const NVM3TestCmd nvm3_test_cmd[NVM3TestCmdTypeMax] = {
    {"?"},
    {"help"},
    {"nvm3_test"},
    {"nvm3_print"},
};

struct NVM3TestApp {
    FuriString* msg;
    CliWorker* worker;
    NVM3TestState state;

    bool exit;
};

static NVM3TestApp* nvm3_test_app_instance = NULL;

static void nvm3_test_app_cmd_usage(NVM3TestApp* instance);

static void nvm3_test_app_send_msg(NVM3TestApp* instance) {
    cli_worker_add_rx_data(
        instance->worker,
        (uint8_t*)furi_string_get_cstr(instance->msg),
        furi_string_utf8_length(instance->msg));
}

void nvm3_test_app_send_text(NVM3TestApp* instance, FuriString* text) {
    cli_worker_add_rx_data(
        instance->worker, (uint8_t*)furi_string_get_cstr(text), furi_string_utf8_length(text));
}

static void nvm3_test_app_send_msg_invalid_arg(NVM3TestApp* instance) {
    furi_string_printf(instance->msg, "Invalid argument\r\n");
    nvm3_test_app_send_msg(instance);
}

void* nvm3_test_app_start(CliWorker* worker) {
    FURI_LOG_I(TAG, "Starting");

    nvm3_test_app_instance = malloc(sizeof(NVM3TestApp));
    nvm3_test_app_instance->msg = furi_string_alloc();
    nvm3_test_app_instance->worker = worker;
    nvm3_test_app_instance->state = NVM3TestStateIdle;

    nvm3_test_app_instance->exit = false;
    sl_status_t status = SL_STATUS_FAIL;
    do {
        status = sl_net_init(
            SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_default_concurrent_configuration, NULL, NULL);
        if(status != SL_STATUS_OK) {
            furi_string_printf(
                nvm3_test_app_instance->msg,
                "Failed to start Wi-Fi client interface: 0x%lx\r\n",
                status);
            nvm3_test_app_send_msg(nvm3_test_app_instance);
            break;
        }
        furi_string_printf(nvm3_test_app_instance->msg, "Wi-Fi APSTA interface init\r\n");
        nvm3_test_app_send_msg(nvm3_test_app_instance);

        if(!nvm3_test_init()) {
            furi_string_printf(nvm3_test_app_instance->msg, "Failed to init NVM3\r\n");
            nvm3_test_app_send_msg(nvm3_test_app_instance);
            break;
        }

        nvm3_test_app_cmd_usage(nvm3_test_app_instance);
        nvm3_test_app_instance->state = NVM3TestStateInit;
    } while(0);

    if(status != SL_STATUS_OK) {
        nvm3_test_app_stop(nvm3_test_app_instance);
        return NULL;
    }
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

void nvm3_test_app_test(NVM3TestApp* instance) {
    uint8_t test_data[] = "Hello World\0";
    uint8_t buffer[256];
    uint32_t count;

    // write - read test
    do {
        if(!nvm3_test_write(2, (uint8_t*)"BSB NVM3 Test\0", 14)) {
            furi_string_printf(instance->msg, "Failed to write data to key 2\r\n");
            nvm3_test_app_send_msg(instance);
            break;
        }

        if(!nvm3_test_write(1, test_data, sizeof(test_data))) {
            furi_string_printf(instance->msg, "Failed to write data to key 1\r\n");
            nvm3_test_app_send_msg(instance);
            break;
        }
        if(!nvm3_test_read(1, buffer, sizeof(test_data))) {
            furi_string_printf(instance->msg, "Failed to read data from key 1\r\n");
            nvm3_test_app_send_msg(instance);
            break;
        }

        if(memcmp(test_data, buffer, sizeof(test_data)) != 0) {
            furi_string_printf(
                instance->msg, "Data read from key 1 is not the same as written\r\n");
            nvm3_test_app_send_msg(instance);
            break;
        }

        furi_string_printf(instance->msg, "Wrire-Read key 1 OK\r\n");
        nvm3_test_app_send_msg(instance);
    } while(false);

    // delete test
    do {
        if(!nvm3_test_delete(1)) {
            furi_string_printf(instance->msg, "Failed to delete key 1\r\n");
            nvm3_test_app_send_msg(instance);
            break;
        }
        if(nvm3_test_read(1, buffer, sizeof(test_data))) {
            furi_string_printf(instance->msg, "Data read from key 1 after delete\r\n");
            nvm3_test_app_send_msg(instance);
            break;
        }

        furi_string_printf(instance->msg, "Delete key 1 OK\r\n");
        nvm3_test_app_send_msg(instance);
    } while(false);

    // counter test
    do {
        if(!nvm3_test_write_counter(10, 5)) {
            furi_string_printf(instance->msg, "Failed to write counter 10\r\n");
            nvm3_test_app_send_msg(instance);
            break;
        }
        if(!nvm3_test_read_counter(10, &count)) {
            furi_string_printf(instance->msg, "Failed to read counter 10\r\n");
            nvm3_test_app_send_msg(instance);
            break;
        }

        if(count != 5) {
            furi_string_printf(instance->msg, "Counter 10 is not 5\r\n");
            nvm3_test_app_send_msg(instance);
            break;
        }

        if(!nvm3_test_increment_counter(10, NULL)) {
            furi_string_printf(instance->msg, "Failed to increment counter 10\r\n");
            nvm3_test_app_send_msg(instance);
            break;
        }

        if(!nvm3_test_read_counter(10, &count)) {
            furi_string_printf(instance->msg, "Failed to read counter 10\r\n");
            nvm3_test_app_send_msg(instance);
            break;
        }

        if(count != 6) {
            furi_string_printf(instance->msg, "Counter 10 is not 6\r\n");
            nvm3_test_app_send_msg(instance);
            break;
        }

        if(!nvm3_test_increment_counter(10, NULL)) {
            furi_string_printf(instance->msg, "Failed to increment counter 10\r\n");
            nvm3_test_app_send_msg(instance);
            break;
        }

        if(!nvm3_test_read_counter(10, &count)) {
            furi_string_printf(instance->msg, "Failed to read counter 10\r\n");
            nvm3_test_app_send_msg(instance);
            break;
        }

        if(count != 7) {
            furi_string_printf(instance->msg, "Counter 10 is not 7\r\n");
            nvm3_test_app_send_msg(instance);
            break;
        }

        furi_string_printf(instance->msg, "Counter 10 OK\r\n");
        nvm3_test_app_send_msg(instance);
    } while(false);
}

static sl_status_t nvm3_test_app(NVM3TestApp* instance, uint8_t cmd_index, FuriString* args) {
    char* args_cstr = (char*)furi_string_get_cstr(args);
    UNUSED(args_cstr);
    FuriString* arg = furi_string_alloc();

    switch(cmd_index) {
    case NVM3TestCmdTypeHelp:
    case NVM3TestCmdTypeHelpHelp:
        nvm3_test_app_cmd_usage(instance);
        break;
    case NVM3TestCmdTypeNVM3:
        nvm3_test_app_test(instance);
        break;
    case NVM3TestCmdTypeNVM3Print:
        nvm3_test_print_objects(instance->msg);
        nvm3_test_app_send_msg(instance);
        break;

    default:
        nvm3_test_app_send_msg_invalid_arg(instance);
        break;
    }

    furi_string_free(arg);
    return SL_STATUS_OK;
}

void nvm3_test_app_parse_msg(void* app_handle, uint8_t* data, size_t size) {
    NVM3TestApp* instance = (NVM3TestApp*)app_handle;
    uint8_t i = 0;
    uint8_t cmd_index = 0;
    bool cmd_valid = false;

    FuriString* args = furi_string_alloc();
    furi_string_set_strn(args, (const char*)data, size);
    FuriString* cmd = furi_string_alloc();

    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(args));

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            break;
        }

        for(i = 0; i < NVM3TestCmdTypeMax; i++) {
            if(furi_string_cmp_str(cmd, (char*)nvm3_test_cmd[i].cmd) == 0) {
                cmd_index = i;
                cmd_valid = true;
                break;
            }
        }
        if(cmd_valid) {
            if(nvm3_test_app(instance, cmd_index, args) != SL_STATUS_OK) {
                furi_string_printf(instance->msg, "Command failed\r\n");
                nvm3_test_app_send_msg(instance);
            }
        } else {
            furi_string_printf(instance->msg, "Invalid command\r\n");
            nvm3_test_app_send_msg(instance);
        }
    } while(false);

    furi_string_free(args);
    furi_string_free(cmd);
}

static void nvm3_test_app_cmd_usage(NVM3TestApp* instance) {
    furi_string_printf(instance->msg, "%s commands usage:\r\n", TAG);
    furi_string_cat_printf(
        instance->msg,
        "*************************************************************************************************************"
        "******\r\n");
    furi_string_cat_printf(
        instance->msg,
        "Read the manual:  https://docs.silabs.com/gecko-platform/latest/platform-driver/nvm3 \r\n");
    furi_string_cat_printf(
        instance->msg,
        "Read the manual:  https://docs.silabs.com/gecko-platform/3.0/driver/api/group-nvm3 \r\n");
    furi_string_cat_printf(instance->msg, "?\r\n");
    furi_string_cat_printf(instance->msg, "help\r\n");
    furi_string_cat_printf(instance->msg, "nvm3_test\r\n");
    furi_string_cat_printf(instance->msg, "nvm3_print\r\n");

    furi_string_cat_printf(
        instance->msg,
        "*************************************************************************************************************"
        "******\r\n");
    nvm3_test_app_send_msg(instance);
}
