#include "ble_test_app.h"

#include <furi.h>

#include <sl_status.h>
#include <sl_wifi.h>
#include <sl_wifi_callback_framework.h>

#include "ble_config.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"

#include <args.h>
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
    BLETestCmdTypeHelp,
    BLETestCmdTypeHelpHelp,
    BLETestCmdTypeModeTx,
    BLETestCmdTypeModeRx,
    BLETestCmdTypeModeTxRxStop,
    BLETestCmdTypeSetChannel,
    BLETestCmdTypeSetPhy,
    BLETestCmdTypeSetPayloadLen,
    BLETestCmdTypeSetPayloadType,

    BLETestCmdMax,
} BLETestCmdType;

typedef enum {
    BLETestStateIdle,
    BLETestStateTx,
    BLETestStateRx,
} BLETestState;

typedef struct {
    char* cmd;
} BLETestCmd;

const BLETestCmd ble_test_cmd[BLETestCmdMax] = {
    {"?"},
    {"help"},
    {"tx"},
    {"rx"},
    {"stop"},
    {"channel"},
    {"phy_rate"},
    {"payload_len"},
    {"payload_type"},

};

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

struct BLETestApp {
    FuriString* msg;
    CliWorker* worker;
    BLETestState state;

    sl_wifi_firmware_version_t fw_version;
    uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN];
    uint8_t local_dev_addr[LOCAL_DEV_ADDR_LEN];

    uint8_t channel;
    uint8_t phy;
    uint8_t payload_len;
    uint8_t payload_type;
    bool exit;
};

static void ble_test_app_cmd_usage(BLETestApp* instance);

static void ble_test_app_send_msg(BLETestApp* instance) {
    cli_worker_add_rx_data(
        instance->worker,
        (uint8_t*)furi_string_get_cstr(instance->msg),
        furi_string_utf8_length(instance->msg));
}

void ble_test_app_send_text(BLETestApp* instance, FuriString* text) {
    cli_worker_add_rx_data(
        instance->worker, (uint8_t*)furi_string_get_cstr(text), furi_string_utf8_length(text));
}

static void ble_test_app_send_msg_invalid_arg(BLETestApp* instance) {
    furi_string_printf(instance->msg, "Invalid argument\r\n");
    ble_test_app_send_msg(instance);
}

void* ble_test_app_start(CliWorker* worker) {
    FURI_LOG_I(TAG, "Starting");

    BLETestApp* instance = malloc(sizeof(BLETestApp));
    instance->msg = furi_string_alloc();
    instance->worker = worker;
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
                instance->msg, "Wi-Fi Initialization Failed, Error Code : 0x%lX\r\n", status);
            ble_test_app_send_msg(instance);
            break;
        }
        furi_string_printf(instance->msg, "Wi-Fi initialization is successful\n");
        ble_test_app_send_msg(instance);

        //! Firmware version Prints
        status = sl_wifi_get_firmware_version(&instance->fw_version);
        if(status != SL_STATUS_OK) {
            furi_string_printf(
                instance->msg, "Firmware version Failed, Error Code : 0x%lX\r\n", status);
            ble_test_app_send_msg(instance);
        } else {
            furi_string_printf(
                instance->msg,
                "Firmware version is: %x%x.%d.%d.%d.%d.%d.%d\r\n",
                instance->fw_version.chip_id,
                instance->fw_version.rom_id,
                instance->fw_version.major,
                instance->fw_version.minor,
                instance->fw_version.security_version,
                instance->fw_version.patch_num,
                instance->fw_version.customer_id,
                instance->fw_version.build_num);
            ble_test_app_send_msg(instance);
        }

        //! get the local device MAC address.
        status = rsi_bt_get_local_device_address(instance->rsi_app_resp_get_dev_addr);
        if(status != RSI_SUCCESS) {
            furi_string_printf(instance->msg, "Get local device address failed = %lx\r\n", status);
            ble_test_app_send_msg(instance);
            break;
        } else {
            furi_string_printf(
                instance->msg, "Local device address %s \r\n", instance->local_dev_addr);
            ble_test_app_send_msg(instance);
        }
        ble_test_app_cmd_usage(instance);
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

static sl_status_t ble_test_app(BLETestApp* instance, uint8_t cmd_index, FuriString* args) {
    sl_status_t status = SL_STATUS_FAIL;

    char* args_cstr = (char*)furi_string_get_cstr(args);
    FuriString* arg = furi_string_alloc();
    uint8_t arg_uint8 = 0;
    StrintParseError parse_err = StrintParseNoError;

    switch(cmd_index) {
    case BLETestCmdTypeHelp:
    case BLETestCmdTypeHelpHelp:
        ble_test_app_cmd_usage(instance);
        break;
    case BLETestCmdTypeModeTx:
        if(instance->state == BLETestStateIdle) {
            instance->state = BLETestStateTx;
            status = rsi_ble_tx_test_mode(
                instance->channel, /* channel number*/
                instance->phy, /* phy - 1Mbps selected */
                instance->payload_len, //255,  /* data_length */
                instance->payload_type); /* packet payload sequence */
            if(status != SL_STATUS_OK) {
                furi_string_printf(instance->msg, "Failed to start Tx test mode\r\n");
                ble_test_app_send_msg(instance);
            } else {
                furi_string_printf(
                    instance->msg,
                    "Tx test mode started. Channel:%dMhz, Phy:%s, Payload_len:%d, Payload_type:%s\r\n",
                    (2402 + (2 * instance->channel)),
                    ble_test_phy_rate[instance->phy].rate_name,
                    instance->payload_len,
                    ble_test_payload_type[instance->payload_type].payload_type_name);
                ble_test_app_send_msg(instance);
            }
        } else {
            furi_string_printf(instance->msg, "Invalid state\r\n");
            ble_test_app_send_msg(instance);
        }
        break;
    case BLETestCmdTypeModeRx:
        if(instance->state == BLETestStateIdle) {
            instance->state = BLETestStateRx;
            status = rsi_ble_rx_test_mode(
                instance->channel, /* channel number*/
                instance->phy, /* phy - 1Mbps selected */
                0x00); /* standard modulation */
            if(status != SL_STATUS_OK) {
                furi_string_printf(instance->msg, "Failed to start Rx test mode\r\n");
                ble_test_app_send_msg(instance);
            } else {
                furi_string_printf(
                    instance->msg,
                    "Rx test mode started. Channel:%dMhz, Phy:%s\r\n",
                    (2402 + (2 * instance->channel)),
                    ble_test_phy_rate[instance->phy].rate_name);
                ble_test_app_send_msg(instance);
            }
        } else {
            furi_string_printf(instance->msg, "Invalid state\r\n");
            ble_test_app_send_msg(instance);
        }
        break;
    case BLETestCmdTypeModeTxRxStop:
        if(instance->state == BLETestStateTx || instance->state == BLETestStateRx) {
            uint16_t num_of_pkts = 0;
            status = rsi_ble_end_test_mode(&num_of_pkts);
            if(status != SL_STATUS_OK) {
                furi_string_printf(instance->msg, "Failed to stop test mode\r\n");
                ble_test_app_send_msg(instance);
            } else {
                if(instance->state == BLETestStateTx) {
                    furi_string_printf(instance->msg, "Tx mode stopped\r\n");
                } else if(instance->state == BLETestStateRx) {
                    furi_string_printf(
                        instance->msg, "Rx mode stopped, number of packets: %d\r\n", num_of_pkts);
                }
                ble_test_app_send_msg(instance);
            }
            instance->state = BLETestStateIdle;
        } else {
            furi_string_printf(instance->msg, "Invalid state\r\n");
            ble_test_app_send_msg(instance);
        }
        break;
    case BLETestCmdTypeSetChannel:
        if(instance->state == BLETestStateIdle && furi_string_size(args)) {
            parse_err |= strint_to_uint8(args_cstr, &args_cstr, &arg_uint8, 10);

            if(parse_err == StrintParseNoError) {
                if(arg_uint8 <= 39) {
                    instance->channel = arg_uint8;
                } else {
                    furi_string_printf(instance->msg, "Invalid channel\r\n");
                    ble_test_app_send_msg(instance);
                }
            }

        } else {
            furi_string_printf(instance->msg, "Invalid argument or state != IDLE \r\n");
            ble_test_app_send_msg(instance);
        }
        break;
    case BLETestCmdTypeSetPhy:
        if(instance->state == BLETestStateIdle && furi_string_size(args)) {
            parse_err |= strint_to_uint8(args_cstr, &args_cstr, &arg_uint8, 10);

            if(parse_err == StrintParseNoError) {
                if((arg_uint8 <= BLE_TEST_PHY_RATE_MAX-1)) {
                    instance->phy = ble_test_phy_rate[arg_uint8].rate_value;
                } else {
                    furi_string_printf(instance->msg, "Invalid PHY Rate\r\n");
                    ble_test_app_send_msg(instance);
                }
            }

        } else {
            furi_string_printf(instance->msg, "Invalid argument or state != IDLE \r\n");
            ble_test_app_send_msg(instance);
        }
        break;
    case BLETestCmdTypeSetPayloadLen:
        if(instance->state == BLETestStateIdle && furi_string_size(args)) {
            parse_err |= strint_to_uint8(args_cstr, &args_cstr, &arg_uint8, 10);

            if(parse_err == StrintParseNoError) {
                if((arg_uint8 >= 1) && (arg_uint8 <= 251)) {
                    instance->payload_len = arg_uint8;
                } else {
                    furi_string_printf(instance->msg, "Invalid payload length\r\n");
                    ble_test_app_send_msg(instance);
                }
            }

        } else {
            furi_string_printf(instance->msg, "Invalid argument or state != IDLE \r\n");
            ble_test_app_send_msg(instance);
        }
        break;
    case BLETestCmdTypeSetPayloadType:
        if(instance->state == BLETestStateIdle && furi_string_size(args)) {
            parse_err |= strint_to_uint8(args_cstr, &args_cstr, &arg_uint8, 10);

            if(parse_err == StrintParseNoError) {
                if(arg_uint8 <= 7) {
                    instance->payload_type = arg_uint8;
                } else {
                    furi_string_printf(instance->msg, "Invalid payload type\r\n");
                    ble_test_app_send_msg(instance);
                }
            }

        } else {
            furi_string_printf(instance->msg, "Invalid argument or state != IDLE \r\n");
            ble_test_app_send_msg(instance);
        }
        break;

    default:
        ble_test_app_send_msg_invalid_arg(instance);
        break;
    }

    furi_string_free(arg);
    return SL_STATUS_OK;
}

void ble_test_app_parse_msg(void* app_handle, uint8_t* data, size_t size) {
    BLETestApp* instance = (BLETestApp*)app_handle;
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

        for(i = 0; i < BLETestCmdMax; i++) {
            if(furi_string_cmp_str(cmd, (char*)ble_test_cmd[i].cmd) == 0) {
                cmd_index = i;
                cmd_valid = true;
                break;
            }
        }
        if(cmd_valid) {
            if(ble_test_app(instance, cmd_index, args) != SL_STATUS_OK) {
                furi_string_printf(instance->msg, "Command failed\r\n");
                ble_test_app_send_msg(instance);
            }
        } else {
            furi_string_printf(instance->msg, "Invalid command\r\n");
            ble_test_app_send_msg(instance);
        }
    } while(false);

    furi_string_free(args);
    furi_string_free(cmd);
}

static void ble_test_app_cmd_usage(BLETestApp* instance) {
    furi_string_printf(instance->msg, "%s commands usage:\r\n", TAG);
    furi_string_cat_printf(
        instance->msg,
        "*************************************************************************************************************"
        "*********\r\n");
    furi_string_cat_printf(instance->msg, "Tests the BLE GAP peripheral role.\r\n");

    furi_string_cat_printf(instance->msg, "?\r\n");
    furi_string_cat_printf(instance->msg, "help\r\n");
    furi_string_cat_printf(instance->msg, "tx Tx Start\r\n");
    furi_string_cat_printf(instance->msg, "rx Rx Start\r\n");
    furi_string_cat_printf(instance->msg, "stop Tx/Rx Stop\r\n");
    furi_string_cat_printf(
        instance->msg, "channel <0..39> BLE channels 2402MHz to 2480MHz with 2MHz spacing\r\n");
    furi_string_cat_printf(
        instance->msg, "phy_rate <0..3> PHY 0: 1Mbps, 1: 2Mbps, 2: 125Kbps, 3: 500Kbps\r\n");
    furi_string_cat_printf(instance->msg, "payload_len <1..251> Payload length\r\n");
    furi_string_cat_printf(
        instance->msg,
        "payload_type <0..7> Payload type 0: PRBS9, 1: 11110000, 2: 10101010, 3: PRBS15, 4: 11111111, 5: 00000000, 6: 00001111, 7: 01010101\r\n");
    furi_string_cat_printf(
        instance->msg,
        "*************************************************************************************************************"
        "*********\r\n");
    ble_test_app_send_msg(instance);
}
