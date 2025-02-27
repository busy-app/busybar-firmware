#include "nvm3_test_app.h"

#include <furi.h>

#include <sl_status.h>
#include <sl_net.h>
#include <sl_wifi.h>
#include <sl_wifi_callback_framework.h>

#include <nvm3_default.h>
#include <ecode.h>
#include <nvm3_default_config.h>

#include <args.h>
#include <strint.h>

#define TAG "NVM3 Test"

#define NVM3_DEFAULT_HANDLE nvm3_defaultHandle
// Maximum number of data objects saved
#define MAX_OBJECT_COUNT    10

// Max and min keys for data objects
#define MIN_DATA_KEY NVM3_KEY_MIN
#define MAX_DATA_KEY (MIN_DATA_KEY + MAX_OBJECT_COUNT - 1)

// Key of write counter object
#define WRITE_COUNTER_KEY MAX_OBJECT_COUNT

// Key of delete counter object
#define DELETE_COUNTER_KEY (WRITE_COUNTER_KEY + 1)

static char buffer[NVM3_DEFAULT_MAX_OBJECT_SIZE];
uint8_t write_data1[12] = {"Silicon labs"};
uint8_t write_data2[4] = {"NVM3"};

const sl_wifi_device_configuration_t client_configuration = {
    .boot_option = LOAD_NWP_FW,
    .mac_address = NULL,
    .band = SL_SI91X_WIFI_BAND_2_4GHZ,
    .region_code = US,
    .boot_config = {
        .oper_mode = SL_SI91X_CLIENT_MODE,
        .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
        .feature_bit_map =
            (SL_SI91X_FEAT_SECURITY_PSK | SL_SI91X_FEAT_AGGREGATION
#ifdef SLI_SI91X_MCU_INTERFACE
             | SL_SI91X_FEAT_WPS_DISABLE
#endif
             ),
        .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT),
        .custom_feature_bit_map = (SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID),
        .ext_custom_feature_bit_map = (
#ifdef SLI_SI91X_MCU_INTERFACE
            SL_SI91X_RAM_LEVEL_NWP_ADV_MCU_BASIC
#else
            SL_SI91X_RAM_LEVEL_NWP_ALL_AVAILABLE
#endif
#if defined(SLI_SI917) || defined(SLI_SI915)
            | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
            ),
        .bt_feature_bit_map = 0,
        .ext_tcp_ip_feature_bit_map = 0,
        .ble_feature_bit_map = 0,
        .ble_ext_feature_bit_map = 0,
        .config_feature_bit_map = 0}};

typedef enum {
    NVM3TestCmdTypeHelp,
    NVM3TestCmdTypeHelpHelp,

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

static void nvm3_app_read(nvm3_ObjectKey_t key) {
    uint32_t type;
    size_t len;
    Ecode_t err;

    do {
        // check for NVM3 maximum key value
        if(key > MAX_DATA_KEY) {
            printf("Invalid key\r\n");
            break;
        }
        err = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, key, &type, &len);
        if(err != NVM3_OBJECTTYPE_DATA || type != NVM3_OBJECTTYPE_DATA) {
            printf("Key does not contain data object\r\n");
            break;
        }
        err = nvm3_readData(NVM3_DEFAULT_HANDLE, key, buffer, len);
        // check for error code
        if(ECODE_NVM3_OK == err) {
            buffer[len] = '\0';
            printf("Read data from key %lu:\r\n", key);
            printf("%s\r\n", buffer);
        } else {
            printf("Error reading data from key %lu\r\n", key);
        }
    } while(false);

    return;
}

void nvm3_app_write(uint32_t key, unsigned char* data, uint32_t len) {
    do {
        // check for NVM3 Maximum object size
        //    if (len > NVM3_DEFAULT_MAX_OBJECT_SIZE) {
        //      printf("Maximum object size exceeded\r\n");
        //      break;
        //    }
        // check for NVM3 maximum key value
        if(key > MAX_DATA_KEY) {
            printf("Invalid key\r\n");
            break;
        }
        // check for NVM3 write success or not
        if(ECODE_NVM3_OK == nvm3_writeData(NVM3_DEFAULT_HANDLE, key, (unsigned char*)data, len)) {
            printf("Stored data at key %lu\r\n", key);
            // Track number of writes in counter object
            nvm3_incrementCounter(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY, NULL);
        } else {
            printf("Error storing data\r\n");
        }
    } while(false);

    return;
}

void nvm3_app_delete(uint32_t key) {
    if(key > MAX_DATA_KEY) {
        printf("Invalid key\r\n");
    } else {
        // check for NVM3 delete object success or not
        if(ECODE_NVM3_OK == nvm3_deleteObject(NVM3_DEFAULT_HANDLE, key)) {
            printf("Deleted data at key %lu\r\n", key);
            // Track number or deletes in counter object
            nvm3_incrementCounter(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY, NULL);
        } else {
            printf("Error deleting key\r\n");
        }
    }
    return;
}

static void initialise_counters(void) {
    uint32_t type;
    size_t len;
    Ecode_t err;

    // check if the designated keys contain counters, and initialise if needed.
    err = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY, &type, &len);
    if((err != ECODE_NVM3_OK) || (type != NVM3_OBJECTTYPE_COUNTER)) {
        nvm3_writeCounter(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY, 0);
    }

    err = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY, &type, &len);
    if((err != ECODE_NVM3_OK) || (type != NVM3_OBJECTTYPE_COUNTER)) {
        nvm3_writeCounter(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY, 0);
    }
}

static void nvm3_app_display(void) {
    nvm3_ObjectKey_t keys[MAX_OBJECT_COUNT];
    size_t len, objects_count;
    uint32_t type;
    Ecode_t err;
    uint32_t counter = 0;
    size_t i;

    objects_count = nvm3_enumDeletedObjects(
        NVM3_DEFAULT_HANDLE,
        (uint32_t*)keys,
        sizeof(keys) / sizeof(keys[0]),
        MIN_DATA_KEY,
        MAX_DATA_KEY);
    // check for NVM3 deleted object count
    if(objects_count == 0) {
        printf("No deleted objects found\r\n");
    } else {
        printf("Keys of objects deleted from NVM3:\r\n");
        for(i = 0; i < objects_count; i++) {
            printf("> %lu\r\n", keys[i]);
        }
    }

    // Retrieve the keys of stored data
    objects_count = nvm3_enumObjects(
        NVM3_DEFAULT_HANDLE,
        (uint32_t*)keys,
        sizeof(keys) / sizeof(keys[0]),
        MIN_DATA_KEY,
        MAX_DATA_KEY);

    // check for NVM3 stored object count
    if(objects_count == 0) {
        printf("No stored objects found\r\n");
    } else {
        printf("Keys and contents of objects stored in NVM3:\r\n");
        for(i = 0; i < objects_count; i++) {
            nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, keys[i], &type, &len);
            if(type == NVM3_OBJECTTYPE_DATA) {
                err = nvm3_readData(NVM3_DEFAULT_HANDLE, keys[i], buffer, len);
                EFM_ASSERT(ECODE_NVM3_OK == err);
                buffer[len] = '\0';
                printf("> %lu: %s\r\n", keys[i], buffer);
            }
        }
    }
    // Display and reset counters
    err = nvm3_readCounter(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY, &counter);
    if(ECODE_NVM3_OK == err) {
        printf("%lu objects have been deleted since last display\r\n", counter);
    }
    nvm3_writeCounter(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY, 0);
    err = nvm3_readCounter(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY, &counter);
    if(ECODE_NVM3_OK == err) {
        printf("%lu objects have been written since last display\r\n", counter);
    }
    nvm3_writeCounter(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY, 0);
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

        Ecode_t err = nvm3_initDefault();
        if(err != ECODE_OK) {
            furi_string_printf(nvm3_test_app_instance->msg, "Failed to init NVM3: 0x%lx\r\n", err);
            nvm3_test_app_send_msg(nvm3_test_app_instance);
            break;
        }
        initialise_counters();
        printf("\nwrite key 1 data\r\n");
        // nvm3_app_write(1, write_data1, 12);
        nvm3_app_read(1);
        printf("\nwrite key 2 data\r\n");
        //nvm3_app_write(2, write_data2, 4);
        nvm3_app_read(2);
        printf("\nwrite key 3 data\r\n");
        //nvm3_app_write(3, write_data2, 4);
        nvm3_app_read(3);
        printf("\nwrite key 4 data\r\n");
        // nvm3_app_write(4, write_data1, 12);
        nvm3_app_read(4);
        nvm3_app_display();
        // printf("\nDeleting all keys\r\n");
        // nvm3_app_delete(1);
        // nvm3_app_delete(2);
        // nvm3_app_delete(3);
        // nvm3_app_delete(4);
        // nvm3_app_display();
        // // Delete all data in NVM3.
        // err = nvm3_eraseAll(NVM3_DEFAULT_HANDLE);
        // if (ECODE_NVM3_OK == err) {
        //   printf("Deleting all data stored in NVM3\r\n");
        // }

        furi_string_printf(nvm3_test_app_instance->msg, "Wi-Fi APSTA interface init\r\n");
        nvm3_test_app_send_msg(nvm3_test_app_instance);

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
        sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
    }

    if(instance) {
        instance->exit = true;

        furi_string_free(instance->msg);
        free(instance);
        instance = NULL;
    }
}

static sl_status_t nvm3_test_app(NVM3TestApp* instance, uint8_t cmd_index, FuriString* args) {
    //sl_status_t status = SL_STATUS_FAIL;

    char* args_cstr = (char*)furi_string_get_cstr(args);
    UNUSED(args_cstr);
    FuriString* arg = furi_string_alloc();

    switch(cmd_index) {
    case NVM3TestCmdTypeHelp:
    case NVM3TestCmdTypeHelpHelp:
        nvm3_test_app_cmd_usage(instance);
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
    furi_string_cat_printf(instance->msg, "?\r\n");
    furi_string_cat_printf(instance->msg, "help\r\n");

    furi_string_cat_printf(
        instance->msg,
        "*************************************************************************************************************"
        "******\r\n");
    nvm3_test_app_send_msg(instance);
}
