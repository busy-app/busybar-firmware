#include <furi.h>

#include <sl_status.h>
#include <sl_wifi.h>
#include <sl_wifi_callback_framework.h>

#include "ble_config.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"

#include <cli/args.h>
#include <cli/shell/cli_shell.h>
#include <cli/cli_ansi.h>
#include <strint.h>

#define TAG "BLETestApp"

#define RSI_BLE_1MBPS   0x1
#define RSI_BLE_2MBPS   0x2
#define RSI_BLE_125KBPS 0x3
#define RSI_BLE_500KBPS 0x4

#define PRBS9_SEQ            0x0 //PRBS9 sequence '11111111100000111101...' \n
#define FOUR_ONES_FOUR_ZEROS 0x1 //Repeated '11110000' \n
#define ALT_ONES_AND_ZEROS   0x2 //Repeated '10101010' \n
#define PRBS15_SEQ           0x3 //PRBS15 \n
#define ALL_ONES             0x4 //Repeated '11111111' \n
#define ALL_ZEROS            0x5 //Repeated '00000000' \n
#define FOUR_ZEROS_FOUR_ONES 0x6 //Repeated '00001111' \n
#define ALT_ZERO_ALT_ONE     0x7 //Repeated '01010101' \n

#define RSI_SEL_ANTENNA               RSI_SEL_INTERNAL_ANTENNA
#define BLE_TEST_CHANNEL_DEFAULT      0x10 //0..39 BLE channels 2402MHz to 2480MHz with 2MHz spacing
#define BLE_TESET_PHY_DEFAULT         RSI_BLE_1MBPS
#define BLE_TEST_PAYLOAD_LEN_DEFAULT  0x20
#define LOCAL_DEV_ADDR_LEN            18 // Length of the local device address
#define BLE_TEST_PAYLOAD_TYPE_DEFAULT PRBS9_SEQ

static const sl_wifi_device_configuration_t config = {
    .boot_option = LOAD_NWP_FW,
    .mac_address = NULL,
    .band = SL_SI91X_WIFI_BAND_2_4GHZ,
    .region_code = US,
    .boot_config = {
        .oper_mode = SL_SI91X_CLIENT_MODE,
        .coex_mode = SL_SI91X_BLE_MODE,
        .feature_bit_map =
            (SL_SI91X_FEAT_WPS_DISABLE | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE |
             SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1),
        .tcp_ip_feature_bit_map =
            (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
        .custom_feature_bit_map = (SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID),
        .ext_custom_feature_bit_map =
            (SL_SI91X_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
#ifdef SLI_SI917
             | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
             | (SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE)),
        .bt_feature_bit_map = (SL_SI91X_BT_RF_TYPE | SL_SI91X_ENABLE_BLE_PROTOCOL),
        .ext_tcp_ip_feature_bit_map = (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
        //!ENABLE_BLE_PROTOCOL in bt_feature_bit_map
        .ble_feature_bit_map =
            ((SL_SI91X_BLE_MAX_NBR_PERIPHERALS(RSI_BLE_MAX_NBR_PERIPHERALS) |
              SL_SI91X_BLE_MAX_NBR_CENTRALS(RSI_BLE_MAX_NBR_CENTRALS) |
              SL_SI91X_BLE_MAX_NBR_ATT_SERV(RSI_BLE_MAX_NBR_ATT_SERV) |
              SL_SI91X_BLE_MAX_NBR_ATT_REC(RSI_BLE_MAX_NBR_ATT_REC)) |
             SL_SI91X_FEAT_BLE_CUSTOM_FEAT_EXTENTION_VALID |
             SL_SI91X_BLE_PWR_INX(RSI_BLE_PWR_INX) |
             SL_SI91X_BLE_PWR_SAVE_OPTIONS(RSI_BLE_PWR_SAVE_OPTIONS) |
             SL_SI91X_916_BLE_COMPATIBLE_FEAT_ENABLE
#if RSI_BLE_GATT_ASYNC_ENABLE
             | SL_SI91X_BLE_GATT_ASYNC_ENABLE
#endif
             ),
        .ble_ext_feature_bit_map =
            ((SL_SI91X_BLE_NUM_CONN_EVENTS(RSI_BLE_NUM_CONN_EVENTS) |
              SL_SI91X_BLE_NUM_REC_BYTES(RSI_BLE_NUM_REC_BYTES))
#if RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST
             | SL_SI91X_BLE_INDICATE_CONFIRMATION_FROM_HOST //indication response from app
#endif
#if RSI_BLE_MTU_EXCHANGE_FROM_HOST
             | SL_SI91X_BLE_MTU_EXCHANGE_FROM_HOST //MTU Exchange request initiation from app
#endif
#if RSI_BLE_SET_SCAN_RESP_DATA_FROM_HOST
             | (SL_SI91X_BLE_SET_SCAN_RESP_DATA_FROM_HOST) //Set SCAN Resp Data from app
#endif
#if RSI_BLE_DISABLE_CODED_PHY_FROM_HOST
             | (SL_SI91X_BLE_DISABLE_CODED_PHY_FROM_HOST) //Disable Coded PHY from app
#endif
#if BLE_SIMPLE_GATT
             | SL_SI91X_BLE_GATT_INIT
#endif
             ),
        .config_feature_bit_map = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP)}};

typedef enum {
    BLETestStateIdle,
    BLETestStateTx,
    BLETestStateRx,
} BLETestState;

typedef struct {
    char* cmd;
} BLETestCmd;

typedef struct {
    char* rate_name;
    uint8_t rate_value;
} BLETestPhy;
#define BLE_TEST_PHY_RATE_MAX 4
const BLETestPhy ble_test_phy_rate[BLE_TEST_PHY_RATE_MAX] = {
    {"1Mbps", RSI_BLE_1MBPS},
    {"2Mbps", RSI_BLE_2MBPS},
    {"125Kbps", RSI_BLE_125KBPS},
    {"500Kbps", RSI_BLE_500KBPS},
};

typedef struct {
    char* payload_type_name;
    uint8_t payload_type_value;
} BLETestPayloadType;
#define BLE_TEST_PAYLOAD_TYPE_MAX 8
const BLETestPayloadType ble_test_payload_type[BLE_TEST_PAYLOAD_TYPE_MAX] = {
    {"PRBS9", PRBS9_SEQ},
    {"Four Ones Four Zeros", FOUR_ONES_FOUR_ZEROS},
    {"Alt Ones and Zeros", ALT_ONES_AND_ZEROS},
    {"PRBS15", PRBS15_SEQ},
    {"All Ones", ALL_ONES},
    {"All Zeros", ALL_ZEROS},
    {"Four Zeros Four Ones", FOUR_ZEROS_FOUR_ONES},
    {"Alt Zero Alt One", ALT_ZERO_ALT_ONE},
};

typedef struct {
    FuriString* msg;
    CliShell* shell;
    BLETestState state;

    sl_wifi_firmware_version_t fw_version;
    uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN];
    uint8_t local_dev_addr[LOCAL_DEV_ADDR_LEN];

    uint8_t channel;
    uint8_t phy;
    uint8_t payload_len;
    uint8_t payload_type;
    bool exit;
} BLETestApp;

void ble_test_app_stop(void* app_handle);

void* ble_test_app_start(CliShell* shell) {
    FURI_LOG_I(TAG, "Starting");

    BLETestApp* instance = malloc(sizeof(BLETestApp));
    instance->msg = furi_string_alloc();
    instance->shell = shell;
    instance->state = BLETestStateIdle;

    instance->channel = BLE_TEST_CHANNEL_DEFAULT;
    instance->phy = BLE_TESET_PHY_DEFAULT;
    instance->payload_len = BLE_TEST_PAYLOAD_LEN_DEFAULT;
    instance->payload_type = BLE_TEST_PAYLOAD_TYPE_DEFAULT;

    instance->exit = false;

    sl_status_t status = SL_STATUS_FAIL;
    do {
        status = sl_wifi_init(&config, NULL, sl_wifi_default_event_handler);
        if(status != SL_STATUS_OK) {
            furi_string_printf(
                instance->msg, "Wi-Fi Initialization Failed, Error Code : 0x%lX", status);
            cli_shell_notification_print(instance->shell, instance->msg);
            break;
        }
        furi_string_printf(instance->msg, "Wi-Fi initialization is successful");
        cli_shell_notification_print(instance->shell, instance->msg);

        //! Firmware version Prints
        status = sl_wifi_get_firmware_version(&instance->fw_version);
        if(status != SL_STATUS_OK) {
            furi_string_printf(
                instance->msg, "Firmware version Failed, Error Code : 0x%lX", status);
            cli_shell_notification_print(instance->shell, instance->msg);
        } else {
            furi_string_printf(
                instance->msg,
                "Firmware version is: %x%x.%d.%d.%d.%d.%d.%d",
                instance->fw_version.chip_id,
                instance->fw_version.rom_id,
                instance->fw_version.major,
                instance->fw_version.minor,
                instance->fw_version.security_version,
                instance->fw_version.patch_num,
                instance->fw_version.customer_id,
                instance->fw_version.build_num);
            cli_shell_notification_print(instance->shell, instance->msg);
        }

        //! get the local device MAC address.
        status = rsi_bt_get_local_device_address(instance->rsi_app_resp_get_dev_addr);
        if(status != RSI_SUCCESS) {
            furi_string_printf(instance->msg, "Get local device address failed = %lx", status);
            cli_shell_notification_print(instance->shell, instance->msg);
            break;
        } else {
            rsi_6byte_dev_address_to_ascii(
                instance->local_dev_addr, instance->rsi_app_resp_get_dev_addr);
            furi_string_printf(instance->msg, "Local device address %s", instance->local_dev_addr);
            cli_shell_notification_print(instance->shell, instance->msg);
        }
    } while(0);

    if(status != SL_STATUS_OK) {
        ble_test_app_stop(instance);
        return NULL;
    }

    return (void*)instance;
}

void ble_test_app_stop(void* app_handle) {
    furi_check(app_handle);
    FURI_LOG_I(TAG, "Stopping");
    BLETestApp* instance = (BLETestApp*)app_handle;

    if(instance) {
        instance->exit = true;
        furi_string_free(instance->msg);
        free(instance);
        instance = NULL;
    }
    sl_wifi_deinit();
}

static void ble_test_tx_cmd(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    BLETestApp* instance = context;
    if(instance->state == BLETestStateIdle) {
        instance->state = BLETestStateTx;
        sl_status_t status = rsi_ble_tx_test_mode(
            instance->channel, /* channel number*/
            instance->phy, /* phy - 1Mbps selected */
            instance->payload_len, //255,  /* data_length */
            instance->payload_type); /* packet payload sequence */
        if(status != SL_STATUS_OK) {
            printf(ANSI_FG_RED "Failed to start Tx test mode" ANSI_RESET);
        } else {
            printf(

                "Tx test mode started. Channel:%dMhz, Phy:%s, Payload_len:%d, Payload_type:%s",
                (2402 + (2 * instance->channel)),
                ble_test_phy_rate[instance->phy].rate_name,
                instance->payload_len,
                ble_test_payload_type[instance->payload_type].payload_type_name);
        }
    } else {
        printf(ANSI_FG_RED "Invalid state" ANSI_RESET);
    }
}

static void ble_test_rx_cmd(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    BLETestApp* instance = context;
    if(instance->state == BLETestStateIdle) {
        instance->state = BLETestStateRx;
        sl_status_t status = rsi_ble_rx_test_mode(
            instance->channel, /* channel number*/
            instance->phy, /* phy - 1Mbps selected */
            0x00); /* standard modulation */
        if(status != SL_STATUS_OK) {
            printf(ANSI_FG_RED "Failed to start Rx test mode" ANSI_RESET);
        } else {
            printf(

                "Rx test mode started. Channel:%dMhz, Phy:%s",
                (2402 + (2 * instance->channel)),
                ble_test_phy_rate[instance->phy].rate_name);
        }
    } else {
        printf(ANSI_FG_RED "Invalid state" ANSI_RESET);
    }
}

static void ble_test_stop_cmd(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    BLETestApp* instance = context;
    if(instance->state == BLETestStateTx || instance->state == BLETestStateRx) {
        uint16_t num_of_pkts = 0;
        sl_status_t status = rsi_ble_end_test_mode(&num_of_pkts);
        if(status != SL_STATUS_OK) {
            printf(ANSI_FG_RED "Failed to stop test mode" ANSI_RESET);
        } else {
            if(instance->state == BLETestStateTx) {
                printf("Tx mode stopped");
            } else if(instance->state == BLETestStateRx) {
                printf("Rx mode stopped, number of packets: %d", num_of_pkts);
            }
        }
        instance->state = BLETestStateIdle;
    } else {
        printf(ANSI_FG_RED "Invalid state" ANSI_RESET);
    }
}

static bool ble_test_generic_command_guard(
    BLETestApp* instance,
    FuriString* args,
    BLETestState expected_state,
    uint8_t min_arg,
    uint8_t max_arg,
    uint8_t* arg_out) {
    if(instance->state != expected_state) {
        printf(
            ANSI_FG_RED "invalid state %d; expected to be in state %d\r\n" ANSI_RESET,
            instance->state,
            expected_state);
        return false;
    }

    int arg = 0;
    if(!args_read_int_and_trim(args, &arg)) {
        printf(ANSI_FG_RED "expected numeric argument\r\n" ANSI_RESET);
        return false;
    }

    if(arg < (int)min_arg || arg > (int)max_arg) {
        printf(
            ANSI_FG_RED "argument out of bounds; expected >= %d, <= %d\r\n" ANSI_RESET,
            min_arg,
            max_arg);
        return false;
    }

    *arg_out = arg;
    return true;
}

static void ble_test_channel_cmd(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    BLETestApp* instance = context;
    uint8_t arg_uint8;
    if(ble_test_generic_command_guard(instance, args, BLETestStateIdle, 0, 39, &arg_uint8)) {
        instance->channel = arg_uint8;
    } else {
        printf("Usage: channel <0..39>\r\n  BLE channels 2402MHz to 2480MHz with 2MHz spacing");
    }
}

static void ble_test_phy_rate_cmd(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    BLETestApp* instance = context;
    uint8_t arg_uint8;
    if(ble_test_generic_command_guard(instance, args, BLETestStateIdle, 0, 3, &arg_uint8)) {
        instance->phy = ble_test_phy_rate[arg_uint8].rate_value;
    } else {
        printf("Usage: phy_rate <0..3>\r\n  PHY 0: 1Mbps, 1: 2Mbps, 2: 125Kbps, 3: 500Kbps");
    }
}

static void ble_test_payload_len_cmd(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    BLETestApp* instance = context;
    uint8_t arg_uint8;
    if(ble_test_generic_command_guard(instance, args, BLETestStateIdle, 1, 251, &arg_uint8)) {
        instance->payload_len = arg_uint8;
    } else {
        printf("Usage: payload_len <1..251>\r\n  Payload length");
    }
}

static void ble_test_payload_type_cmd(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    BLETestApp* instance = context;
    uint8_t arg_uint8;
    if(ble_test_generic_command_guard(instance, args, BLETestStateIdle, 0, 7, &arg_uint8)) {
        instance->payload_type = arg_uint8;
    } else {
        printf(
            "Usage: payload_type <0..7>\r\n  Payload type\r\n  0: PRBS9, 1: 11110000, 2: 10101010, 3: PRBS15, 4: 11111111, 5: 00000000, 6: 00001111, 7: 01010101");
    }
}

static void ble_test_motd(void* context) {
    UNUSED(context);
    printf("\r\n+----------------------------+\r\n");
    printf("| Welcome to BLE Test shell! |\r\n");
    printf("+----------------------------+\r\n\r\n");
    printf("Tests the BLE GAP peripheral role.\r\n");
}

void ble_test_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

    CliRegistry* registry = cli_registry_alloc();
    CliShell* shell = cli_shell_alloc(ble_test_motd, NULL, pipe, registry, NULL);
    cli_shell_set_prompt(shell, "ble_per");

    cli_shell_start(shell);
    BLETestApp* app = ble_test_app_start(shell);
    cli_registry_add_command(registry, "tx", CliCommandFlagDefault, ble_test_tx_cmd, app);
    cli_registry_add_command(registry, "rx", CliCommandFlagDefault, ble_test_rx_cmd, app);
    cli_registry_add_command(registry, "stop", CliCommandFlagDefault, ble_test_stop_cmd, app);
    cli_registry_add_command(
        registry, "channel", CliCommandFlagDefault, ble_test_channel_cmd, app);
    cli_registry_add_command(
        registry, "phy_rate", CliCommandFlagDefault, ble_test_phy_rate_cmd, app);
    cli_registry_add_command(
        registry, "payload_len", CliCommandFlagDefault, ble_test_payload_len_cmd, app);
    cli_registry_add_command(
        registry, "payload_type", CliCommandFlagDefault, ble_test_payload_type_cmd, app);

    cli_shell_join(shell);
    ble_test_app_stop(app);

    cli_shell_free(shell);
    cli_registry_free(registry);
}
