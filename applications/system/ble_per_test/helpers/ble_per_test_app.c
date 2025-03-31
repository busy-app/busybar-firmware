#include "ble_per_test_app.h"

#include <furi.h>

#include <sl_status.h>
#include <sl_wifi.h>
#include <sl_wifi_callback_framework.h>
#include "sl_si91x_driver.h"

#include "ble_config.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"

#include <args.h>
#include <strint.h>

#define TAG "BLEPerTestApp"

#define RSI_BLE_LOCAL_NAME (void*)"BLE_PERIPHERAL"

#define DISABLE 0
#define ENABLE  1

#define RSI_BLE_PER_TRANSMIT_MODE 1
#define RSI_BLE_PER_RECEIVE_MODE  2
#define RSI_PER_STATS             3

#define RSI_CONFIG_PER_MODE RSI_BLE_PER_TRANSMIT_MODE

#define RSI_BLE_1MBPS   1
#define RSI_BLE_2MBPS   2
#define RSI_BLE_125KBPS 4
#define RSI_BLE_500KBPS 8

#define PRBS9_SEQ            0x0 //PRBS9 sequence '11111111100000111101...' \n
#define FOUR_ONES_FOUR_ZEROS 0x1 //Repeated '11110000' \n
#define ALT_ONES_AND_ZEROS   0x2 //Repeated '10101010' \n
#define PRBS15_SEQ           0x3 //PRBS15 \n
#define ALL_ONES             0x4 //Repeated '11111111' \n
#define ALL_ZEROS            0x5 //Repeated '00000000' \n
#define FOUR_ZEROS_FOUR_ONES 0x6 //Repeated '00001111' \n
#define ALT_ZERO_ALT_ONE     0x7 //Repeated '01010101' \n

#define LE_ADV_CHNL_TYPE  0
#define LE_DATA_CHNL_TYPE 1

#define BURST_MODE     0
#define CONTIUOUS_MODE 1
#define CW_MODE        2

#define NO_HOPPING     0
#define FIXED_HOPPING  1
#define RANDOM_HOPPING 2

#define BT_PER_STATS_CMD_ID 0x08
#define BLE_TRANSMIT_CMD_ID 0x13
#define BLE_RECEIVE_CMD_ID  0x14

#define BLE_ACCESS_ADDR    0x71764129
#define BLE_TX_PKT_LEN     32
#define BLE_PHY_RATE       RSI_BLE_2MBPS
#define BLE_RX_CHNL_NUM    10
#define BLE_TX_CHNL_NUM    10
#define BLE_TX_POWER_INDEX 127
#define SCRAMBLER_SEED     0
#define NUM_PKTS           0
#define RSI_INTER_PKT_GAP  0

#define ONBOARD_ANT_SEL 2
#define EXT_ANT_SEL     3

#define BLE_EXTERNAL_RF 0
#define BLE_INTERNAL_RF 1

#define NO_CHAIN_SEL      0
#define WLAN_HP_CHAIN_BIT 0
#define WLAN_LP_CHAIN_BIT 1
#define BT_HP_CHAIN_BIT   2
#define BT_LP_CHAIN_BIT   3

#define PLL_MODE_0 0
#define PLL_MODE_1 1

#define LOOP_BACK_MODE_DISABLE 0
#define LOOP_BACK_MODE_ENABLE  1

#define EXT_DATA_LEN_IND 1

#define DUTY_CYCLING_DISABLE 0
#define DUTY_CYCLING_ENABLE  1
#define LOCAL_DEV_ADDR_LEN   18 // Length of the local device address

#define GAIN_TABLE_AND_MAX_POWER_UPDATE_ENABLE \
    0 //! To update gain table and max tx power and offsets

#if GAIN_TABLE_AND_MAX_POWER_UPDATE_ENABLE

#define FCC       0
#define ETSI      1
#define TELEC     2
#define WORLDWIDE 3
#define KCC       4

#define BLE_GAIN_TABLE_MAXPOWER_UPDATE              0
#define BLE_GAIN_TABLE_OFFSET_UPDATE                1
#define BLE_GAIN_TABLE_LP_CHAIN_0DBM_OFFSET_UPDATE  2
#define BLE_GAIN_TABLE_LP_CHAIN_10DBM_OFFSET_UPDATE 3

// clang-format off
//! structure for the MAXPOWER
uint8_t Si917_BLE_REGION_BASED_MAXPOWER_XX[16] = {//{{{
	// BLE Max Power Index,
	FCC,        16,
	ETSI,       8,
	TELEC,      10,
	WORLDWIDE,  16,
	KCC,        10,
};//}}}

//! structure for the MAXPOWER OFFSET
uint8_t Si917_BLE_REGION_BASED_MAXPOWER_VS_OFFSET_XX[128] = {//{{{
	5,//NUM_OF_REGIONS
	FCC,
	4,//NUM_OF_CHANNELS
	//chan_num   1M   2M   125kbps 500kbps
	255,	    0,   0,      6,   0,
	0,          0,   0,      6,   0,
	38,         0,   2,      6,   0,
	39,         0,  16,      6,   0,
	ETSI,
	4,//NUM_OF_CHANNELS
	255,	    0,   0,      0,   0,
	0,          0,   0,      0,   0,
	19,         0,   0,      0,   0,
	39,         0,   0,      0,   0,
	TELEC,
	4,//NUM_OF_CHANNELS
	255,	    0,   0,      0,   0,
	0,	    0,   0,      0,   0,
	19,         0,   0,      0,   0,
	39,         0,   0,      0,   0,
	WORLDWIDE,
	4,//NUM_OF_CHANNELS
	255,	    0,   0,      0,   0,
	0,          0,   0,      0,   0,
	19,         0,   0,      0,   0,
	39,         0,   0,      0,   0,
	KCC,
	4,//NUM_OF_CHANNELS
	255,	    0,   0,      0,   0,
	0,	    0,   0,      0,   0,
	19,         0,   0,      0,   0,
	39,         0,   0,      0,   0
};//}}}

//! structure for the LP_CHAIN 0dBm OFFSET
uint8_t Si917_BLE_REGION_BASED_LP_CHAIN_0DBM_OFFSET_XX[128] = {//{{{
	5,//NUM_OF_REGIONS
	FCC,
	4,//NUM_OF_CHANNELS
	//chan_num    1M   2M   125kbps 500kbps
	255,	      31,  31,      31,   31,
	0,            31,  31,      31,   31,
	19,           31,  31,      31,   31,
	39,           31,   8,      31,   31,
	ETSI,
	4,//NUM_OF_CHANNELS
	255,	      31,  31,      31,   31,
	0,            31,  31,      31,   31,
	19,           31,  31,      31,   31,
	39,           31,  31,      31,   31,
	TELEC,
	4,//NUM_OF_CHANNELS
	255,	      31,  31,      31,   31,
	0,            31,  31,      31,   31,
	19,           31,  31,      31,   31,
	39,           31,  31,      31,   31,
	WORLDWIDE,
	4,//NUM_OF_CHANNELS
	255,	      31,  31,      31,   31,
	0,            31,  31,      31,   31,
	19,           31,  31,      31,   31,
	39,           31,  31,      31,   31,
	KCC,
	4,//NUM_OF_CHANNELS
	255,	      31,  31,      31,   31,
	0,            31,  31,      31,   31,
	19,           31,  31,      31,   31,
	39,           31,  31,      31,   31,
};//}}}

//! structure for the LP_CHAIN 10dBm OFFSET
uint8_t Si917_BLE_REGION_BASED_LP_CHAIN_10DBM_OFFSET_XX[128] = {//{{{
	5,//NUM_OF_REGIONS
	FCC,
	4,//NUM_OF_CHANNELS
	//chan_num    1M   2M   125kbps 500kbps
	255,	      63,  63,      63,   63,
	0,            63,  63,      63,   63,
	19,           63,  63,      63,   63,
	39,           63,  35,      63,   63,
	ETSI,
	4,//NUM_OF_CHANNELS
	255,	      63,  63,      63,   63,
	0,            63,  63,      63,   63,
	19,           63,  63,      63,   63,
	39,           63,  63,      63,   63,
	TELEC,
	4,//NUM_OF_CHANNELS
	255,	      63,  63,      63,   63,
	0,            63,  63,      63,   63,
	19,           63,  63,      63,   63,
	39,           63,  63,      63,   63,
	WORLDWIDE,
	4,//NUM_OF_CHANNELS
	255,	      63,  63,      63,   63,
	0,            63,  63,      63,   63,
	19,           63,  63,      63,   63,
	39,           63,  63,      63,   63,
	KCC,
	4,//NUM_OF_CHANNELS
	255,	      63,  63,      63,   63,
	0,            63,  63,      63,   63,
	19,           63,  63,      63,   63,
	39,           63,  63,      63,   63,
};//}}}

// clang-format on
#endif

#define RSI_FEATURE_BIT_MAP                   \
    (SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE | \
     SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1) //! To set wlan feature select bit map
#define RSI_TCP_IP_FEATURE_BIT_MAP 0 //! TCP/IP feature select bitmap for selecting TCP/IP features
#define RSI_CUSTOM_FEATURE_BIT_MAP \
    SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID //! To set custom feature select bit map
#define RSI_EXT_TCPIP_FEATURE_BITMAP 0
#define RSI_BT_FEATURE_BITMAP        (SL_SI91X_BT_RF_TYPE | SL_SI91X_ENABLE_BLE_PROTOCOL)
#define RSI_CONFIG_FEATURE_BITMAP    0

static const sl_wifi_device_configuration_t config = {
    .boot_option = LOAD_NWP_FW,
    .mac_address = NULL,
    .band = SL_SI91X_WIFI_BAND_2_4GHZ,
    .region_code = US,
    .boot_config = {
        .oper_mode = SL_SI91X_CLIENT_MODE,
        .coex_mode = SL_SI91X_WLAN_BLE_MODE,
#ifdef SLI_SI91X_MCU_INTERFACE
        .feature_bit_map = (SL_SI91X_FEAT_WPS_DISABLE | RSI_FEATURE_BIT_MAP),
#else
        .feature_bit_map = RSI_FEATURE_BIT_MAP,
#endif
#if RSI_TCP_IP_BYPASS
        .tcp_ip_feature_bit_map = RSI_TCP_IP_FEATURE_BIT_MAP,
#else
        .tcp_ip_feature_bit_map =
            (RSI_TCP_IP_FEATURE_BIT_MAP | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
#endif
        .custom_feature_bit_map =
            (SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID | RSI_CUSTOM_FEATURE_BIT_MAP),
        .ext_custom_feature_bit_map =
            (SL_SI91X_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK | MEMORY_CONFIG
#ifdef SLI_SI917
             | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
             | SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE),
        .bt_feature_bit_map = (RSI_BT_FEATURE_BITMAP),
#ifdef RSI_PROCESS_MAX_RX_DATA
        .ext_tcp_ip_feature_bit_map =
            (RSI_EXT_TCPIP_FEATURE_BITMAP | SL_SI91X_CONFIG_FEAT_EXTENTION_VALID |
             SL_SI91X_EXT_TCP_MAX_RECV_LENGTH),
#else
        .ext_tcp_ip_feature_bit_map =
            (RSI_EXT_TCPIP_FEATURE_BITMAP | SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
#endif
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
        .config_feature_bit_map =
            (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP | RSI_CONFIG_FEATURE_BITMAP)}};

typedef enum {
    BLEPerTestCmdTypeHelp,
    BLEPerTestCmdTypeHelpHelp,
    BLEPerTestCmdTypeModeTx,
    BLEPerTestCmdTypeModeRx,
    BLEPerTestCmdTypeModeTxRxStop,
    BLEPerTestCmdTypeSetChannel,
    BLEPerTestCmdTypeSetPhyRate,
    BLEPerTestCmdTypeSetPayloadLen,
    BLEPerTestCmdTypeSetPayloadType,
    BLEPerTestCmdTypeSetMode,
    BLEPerTestCmdTypeSetHopping,
    BLEPerTestCmdTypeSetTxPower,

    BLEPerTestCmdTypeMax,
} BLEPerTestCmdType;

typedef enum {
    BLEPerTestStateIdle,
    BLEPerTestStateTx,
    BLEPerTestStateRx,
} BLEPerTestState;

typedef struct {
    char* cmd;
} BLEPerTestCmd;

const BLEPerTestCmd ble_per_test_cmd[BLEPerTestCmdTypeMax] = {
    {"?"},
    {"help"},
    {"tx"},
    {"rx"},
    {"stop"},
    {"channel"},
    {"phy_rate"},
    {"payload_len"},
    {"payload_type"},
    {"mode"},
    {"hopping"},
    {"tx_power"},

};

typedef struct {
    char* rate_name;
    uint8_t rate_value;
} BLEPerTestPhy;
#define BLE_PER_TEST_PHY_RATE_MAX 4
const BLEPerTestPhy ble_per_test_phy_rate[BLE_PER_TEST_PHY_RATE_MAX] = {
    {"1Mbps", RSI_BLE_1MBPS},
    {"2Mbps", RSI_BLE_2MBPS},
    {"125Kbps", RSI_BLE_125KBPS},
    {"500Kbps", RSI_BLE_500KBPS},
};

typedef struct {
    char* payload_type_name;
    uint8_t payload_type_value;
} BLEPerTestPayloadType;
#define BLE_PER_TEST_PAYLOAD_TYPE_MAX 8
const BLEPerTestPayloadType ble_per_test_payload_type[BLE_PER_TEST_PAYLOAD_TYPE_MAX] = {
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
    char* mode_name;
    uint8_t mode_value;
} BLEPerTestTransmitMode;
#define BLE_PER_TEST_TRANSMIT_MODE_MAX 3
const BLEPerTestTransmitMode ble_per_test_transmit_mode[BLE_PER_TEST_TRANSMIT_MODE_MAX] = {
    {"burst", BURST_MODE},
    {"continuous", CONTIUOUS_MODE},
    {"cw", CW_MODE},
};

typedef struct {
    char* hopping_name;
    uint8_t hopping_value;
} BLEPerTestHoppingMode;
#define BLE_PER_TEST_HOPPING_MODE_MAX 3
const BLEPerTestHoppingMode ble_per_test_hopping_mode[BLE_PER_TEST_HOPPING_MODE_MAX] = {
    {"no_hopping", NO_HOPPING},
    {"fixed_hopping", FIXED_HOPPING},
    {"random_hopping", RANDOM_HOPPING},
};

struct BLEPerTestApp {
    FuriString* msg;
    CliWorker* worker;
    BLEPerTestState state;

    rsi_bt_resp_get_local_name_t rsi_app_resp_get_local_name;
    uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN];
    rsi_ble_per_transmit_t rsi_ble_per_tx;
    rsi_ble_per_receive_t rsi_ble_per_rx;
    rsi_bt_per_stats_t per_stats;

    FuriThread* thread;

    bool exit;
};

static void ble_per_test_app_cmd_usage(BLEPerTestApp* instance);
void ble_per_test_app_show_status_start(void* app_handle);
void ble_per_test_app_show_status_stop(void* app_handle);

static void ble_per_test_app_send_msg(BLEPerTestApp* instance) {
    cli_worker_add_rx_data(
        instance->worker,
        (uint8_t*)furi_string_get_cstr(instance->msg),
        furi_string_utf8_length(instance->msg));
}

void ble_per_test_app_send_text(BLEPerTestApp* instance, FuriString* text) {
    cli_worker_add_rx_data(
        instance->worker, (uint8_t*)furi_string_get_cstr(text), furi_string_utf8_length(text));
}

static void ble_per_test_app_send_msg_invalid_arg(BLEPerTestApp* instance) {
    furi_string_printf(instance->msg, "Invalid argument\r\n");
    ble_per_test_app_send_msg(instance);
}

void* ble_per_test_app_start(CliWorker* worker) {
    FURI_LOG_I(TAG, "Starting");

    BLEPerTestApp* instance = malloc(sizeof(BLEPerTestApp));
    instance->msg = furi_string_alloc();
    instance->worker = worker;
    instance->state = BLEPerTestStateIdle;

    memset(&instance->rsi_app_resp_get_local_name, 0, sizeof(rsi_bt_resp_get_local_name_t));
    memset(instance->rsi_app_resp_get_dev_addr, 0, RSI_DEV_ADDR_LEN);

    //! set the default values for the transmit parameters
    instance->rsi_ble_per_tx.cmd_ix = BLE_TRANSMIT_CMD_ID;
    instance->rsi_ble_per_tx.transmit_enable = ENABLE;
    *(uint32_t*)&instance->rsi_ble_per_tx.access_addr[0] = BLE_ACCESS_ADDR;
    *(uint16_t*)&instance->rsi_ble_per_tx.pkt_len[0] = BLE_TX_PKT_LEN;
    instance->rsi_ble_per_tx.phy_rate = BLE_PHY_RATE;
    instance->rsi_ble_per_tx.rx_chnl_num = BLE_RX_CHNL_NUM;
    instance->rsi_ble_per_tx.tx_chnl_num = BLE_TX_CHNL_NUM;
    instance->rsi_ble_per_tx.scrambler_seed = SCRAMBLER_SEED;
    instance->rsi_ble_per_tx.payload_type = PRBS9_SEQ;
    instance->rsi_ble_per_tx.le_chnl_type = LE_DATA_CHNL_TYPE;
    instance->rsi_ble_per_tx.tx_power = BLE_TX_POWER_INDEX; //1..10
    instance->rsi_ble_per_tx.transmit_mode = BURST_MODE;
    instance->rsi_ble_per_tx.freq_hop_en = NO_HOPPING;
    instance->rsi_ble_per_tx.ant_sel = ONBOARD_ANT_SEL;
    instance->rsi_ble_per_tx.inter_pkt_gap = RSI_INTER_PKT_GAP;
    instance->rsi_ble_per_tx.pll_mode = PLL_MODE_0;
    instance->rsi_ble_per_tx.rf_type = BLE_INTERNAL_RF;
    instance->rsi_ble_per_tx.rf_chain = BT_HP_CHAIN_BIT;

    //! set the default values for the receive parameters
    instance->rsi_ble_per_rx.cmd_ix = BLE_RECEIVE_CMD_ID;
    instance->rsi_ble_per_rx.receive_enable = ENABLE;
    *(uint32_t*)&instance->rsi_ble_per_rx.access_addr[0] = BLE_ACCESS_ADDR;
    instance->rsi_ble_per_rx.ext_data_len_indication = EXT_DATA_LEN_IND;
    instance->rsi_ble_per_rx.phy_rate = BLE_PHY_RATE;
    instance->rsi_ble_per_rx.rx_chnl_num = BLE_RX_CHNL_NUM;
    instance->rsi_ble_per_rx.tx_chnl_num = BLE_TX_CHNL_NUM;
    instance->rsi_ble_per_rx.scrambler_seed = SCRAMBLER_SEED;
    instance->rsi_ble_per_rx.le_chnl_type = LE_DATA_CHNL_TYPE;
    instance->rsi_ble_per_rx.loop_back_mode = LOOP_BACK_MODE_DISABLE;
    instance->rsi_ble_per_rx.freq_hop_en = NO_HOPPING;
    instance->rsi_ble_per_rx.ant_sel = ONBOARD_ANT_SEL;
    instance->rsi_ble_per_rx.duty_cycling_en = DUTY_CYCLING_DISABLE;
    instance->rsi_ble_per_rx.pll_mode = PLL_MODE_0;
    instance->rsi_ble_per_rx.rf_type = BLE_INTERNAL_RF;
    instance->rsi_ble_per_rx.rf_chain = BT_HP_CHAIN_BIT;

    instance->exit = false;

    sl_status_t status;
    sl_wifi_firmware_version_t version = {0};
    uint8_t local_dev_addr[LOCAL_DEV_ADDR_LEN] = {0};
    do {
        status = sl_wifi_init(&config, NULL, sl_wifi_default_event_handler);
        if(status != SL_STATUS_OK) {
            furi_string_printf(
                instance->msg, "Wireless initialization failed, error code: 0x%08lX\r\n", status);
            ble_per_test_app_send_msg(instance);
            break;
        }
        furi_string_printf(instance->msg, "Wireless Initialization Success\r\n");
        ble_per_test_app_send_msg(instance);

        //! set region support
        status = sl_si91x_set_device_region(
            config.boot_config.oper_mode, config.band, config.region_code);
        if(status != SL_STATUS_OK) {
            furi_string_printf(
                instance->msg, "Failed to set region, error code: 0x%08lX\r\n", status);
            ble_per_test_app_send_msg(instance);
            break;
        } else {
            furi_string_printf(instance->msg, "Set Region Success\r\n");
            ble_per_test_app_send_msg(instance);
        }

        //!  WLAN radio deinit
        status = sl_si91x_disable_radio();
        if(status != SL_STATUS_OK) {
            furi_string_printf(
                instance->msg, "Failed to disable WLAN radio, error code: 0x%08lX\r\n", status);
            ble_per_test_app_send_msg(instance);
            break;
        } else {
            furi_string_printf(instance->msg, "Disable WLAN radio success\r\n");
            ble_per_test_app_send_msg(instance);
        }

        //! Firmware version Prints
        status = sl_wifi_get_firmware_version(&version);
        if(status != SL_STATUS_OK) {
            furi_string_printf(
                instance->msg,
                "Failed to fetch firmware version, error code: 0x%08lX\r\n",
                status);
            ble_per_test_app_send_msg(instance);
            break;
        } else {
            print_firmware_version(&version);
            furi_string_printf(
                instance->msg,
                "Firmware version is: %x%x.%d.%d.%d.%d.%d.%d\r\n",
                version.chip_id,
                version.rom_id,
                version.major,
                version.minor,
                version.security_version,
                version.patch_num,
                version.customer_id,
                version.build_num);
            ble_per_test_app_send_msg(instance);
        }

        //! get the local device MAC address.
        status = rsi_bt_get_local_device_address(instance->rsi_app_resp_get_dev_addr);
        if(status != RSI_SUCCESS) {
            furi_string_printf(
                instance->msg,
                "Failed to get local device address, error code: 0x%08lX\r\n",
                status);
            ble_per_test_app_send_msg(instance);
            break;
        } else {
            rsi_6byte_dev_address_to_ascii(local_dev_addr, instance->rsi_app_resp_get_dev_addr);
            furi_string_printf(instance->msg, "Local device address : %s\r\n", local_dev_addr);
            ble_per_test_app_send_msg(instance);
        }

        //! set the local device name
        status = rsi_bt_set_local_name(RSI_BLE_LOCAL_NAME);
        if(status != RSI_SUCCESS) {
            furi_string_printf(
                instance->msg, "Failed to set local name, error code: 0x%08lX\r\n", status);
            ble_per_test_app_send_msg(instance);
            break;
        }

        //! get the local device name
        status = rsi_bt_get_local_name(&instance->rsi_app_resp_get_local_name);
        if(status != RSI_SUCCESS) {
            furi_string_printf(
                instance->msg, "Failed to get local name, error code: 0x%08lX\r\n", status);
            ble_per_test_app_send_msg(instance);
            break;
        }
        furi_string_printf(
            instance->msg,
            "Local name set to: %s\r\n",
            instance->rsi_app_resp_get_local_name.name);
        ble_per_test_app_send_msg(instance);

#if GAIN_TABLE_AND_MAX_POWER_UPDATE_ENABLE

        //! structure update for the MAXPOWER
        status = rsi_bt_cmd_update_gain_table_offset_or_max_pwr(
            0,
            sizeof(Si917_BLE_REGION_BASED_MAXPOWER_XX),
            Si917_BLE_REGION_BASED_MAXPOWER_XX,
            BLE_GAIN_TABLE_MAXPOWER_UPDATE);
        if(status != RSI_SUCCESS) {
            furi_string_printf(
                instance->msg,
                "Failed to update gain table for max power, error code: 0x%08lX\r\n",
                status);
            ble_per_test_app_send_msg(instance);
            break;
        } else {
            furi_string_printf(
                instance->msg, "Updation of gain table max tx power command is successful\r\n");
            ble_per_test_app_send_msg(instance);
        }

        //! structure update for the MAXPOWER OFFSET
        status = rsi_bt_cmd_update_gain_table_offset_or_max_pwr(
            0,
            sizeof(Si917_BLE_REGION_BASED_MAXPOWER_VS_OFFSET_XX),
            Si917_BLE_REGION_BASED_MAXPOWER_VS_OFFSET_XX,
            BLE_GAIN_TABLE_OFFSET_UPDATE);
        if(status != RSI_SUCCESS) {
            furi_string_printf(
                instance->msg,
                "Failed to update gain table offset, error code: 0x%08lX\r\n",
                status);
            ble_per_test_app_send_msg(instance);
            break;
        } else {
            furi_string_printf(
                instance->msg, "Updation of gain table offset command is successful\r\n");
        }

        //! structure update for the LP_CHAIN 0dBm OFFSET
        status = rsi_bt_cmd_update_gain_table_offset_or_max_pwr(
            0,
            sizeof(Si917_BLE_REGION_BASED_LP_CHAIN_0DBM_OFFSET_XX),
            Si917_BLE_REGION_BASED_LP_CHAIN_0DBM_OFFSET_XX,
            BLE_GAIN_TABLE_LP_CHAIN_0DBM_OFFSET_UPDATE);
        if(status != RSI_SUCCESS) {
            furi_string_printf(
                instance->msg,
                "Failed to update gain table LP-Chain 0dBm offset, error code: 0x%08lX\r\n",
                status);
            ble_per_test_app_send_msg(instance);
            break;
        } else {
            printf("\r\n Updation of gain table LP-Chain 0dBm offset command is successful \r\n");
            furi_string_printf(
                instance->msg,
                "Updation of gain table LP-Chain 0dBm offset command is successful\r\n");
            ble_per_test_app_send_msg(instance);
        }

        //! structure update for the LP_CHAIN 10dBm OFFSET
        status = rsi_bt_cmd_update_gain_table_offset_or_max_pwr(
            0,
            sizeof(Si917_BLE_REGION_BASED_LP_CHAIN_10DBM_OFFSET_XX),
            Si917_BLE_REGION_BASED_LP_CHAIN_10DBM_OFFSET_XX,
            BLE_GAIN_TABLE_LP_CHAIN_10DBM_OFFSET_UPDATE);
        if(status != RSI_SUCCESS) {
            furi_string_printf(
                instance->msg,
                "Failed to update gain table LP-Chain 10dBm offset, error code: 0x%08lX\r\n",
                status);
            ble_per_test_app_send_msg(instance);
            break;
        } else {
            furi_string_printf(
                instance->msg,
                "Updation of gain table LP-Chain 10dBm offset command is successful\r\n");
            ble_per_test_app_send_msg(instance);
        }

#endif
        ble_per_test_app_cmd_usage(instance);
    } while(false);

    if(status != SL_STATUS_OK) {
        furi_string_printf(instance->msg, "Failed to start BLE PER test\r\n");
        ble_per_test_app_send_msg(instance);
        ble_per_test_app_stop(instance);
        return NULL;
    }

    return (void*)instance;
}

void ble_per_test_app_stop(void* app_handle) {
    furi_check(app_handle);
    FURI_LOG_I(TAG, "Stopping");
    BLEPerTestApp* instance = (BLEPerTestApp*)app_handle;

    if(instance) {
        if(instance->thread) {
            ble_per_test_app_show_status_stop(instance);
        }
        furi_string_free(instance->msg);
        free(instance);
        instance = NULL;
    }
    sl_wifi_deinit();
}

static sl_status_t ble_per_test_app(BLEPerTestApp* instance, uint8_t cmd_index, FuriString* args) {
    sl_status_t status = SL_STATUS_FAIL;

    char* args_cstr = (char*)furi_string_get_cstr(args);
    UNUSED(args_cstr);
    FuriString* arg = furi_string_alloc();
    uint8_t arg_uint8 = 0;
    StrintParseError parse_err = StrintParseNoError;

    switch(cmd_index) {
    case BLEPerTestCmdTypeHelp:
    case BLEPerTestCmdTypeHelpHelp:
        ble_per_test_app_cmd_usage(instance);
        break;
    case BLEPerTestCmdTypeModeTx:
        if(instance->state == BLEPerTestStateIdle) {
            instance->rsi_ble_per_tx.transmit_enable = ENABLE;
            status = rsi_ble_per_transmit(&instance->rsi_ble_per_tx);
            if(status != RSI_SUCCESS) {
                furi_string_printf(
                    instance->msg,
                    "Failed to start BLE PER TX test, error code: 0x%08lX\r\n",
                    status);
                ble_per_test_app_send_msg(instance);
            } else {
                instance->state = BLEPerTestStateTx;
                if(instance->rsi_ble_per_tx.transmit_mode == BURST_MODE) {
                    ble_per_test_app_show_status_start(instance);
                }

                furi_string_printf(
                    instance->msg,
                    "BLE PER test Tx started\r\n"
                    "cmd id: 0x%X\r\n"
                    "enable: %d\r\n"
                    "access_addr: 0x%lX\r\n"
                    "pkt_len: %d\r\n"
                    "phy_rate: %d\r\n"
                    "rx_chnl_num: %d\r\n"
                    "tx_chnl_num: %d\r\n"
                    "scrambler_seed: %d\r\n"
                    "payload_type: %d\r\n"
                    "le_chnl_type: %d\r\n"
                    "tx_power: %d\r\n"
                    "transmit_mode: %d\r\n"
                    "freq_hop_en: %d\r\n"
                    "ant_sel: %d\r\n"
                    "inter_pkt_gap: %d\r\n"
                    "pll_mode: %d\r\n"
                    "rf_type: %d\r\n"
                    "rf_chain: %d\r\n",
                    instance->rsi_ble_per_tx.cmd_ix,
                    instance->rsi_ble_per_tx.transmit_enable,
                    *(uint32_t*)&instance->rsi_ble_per_tx.access_addr[0],
                    (*(uint16_t*)&instance->rsi_ble_per_tx.pkt_len[0]),
                    instance->rsi_ble_per_tx.phy_rate,
                    instance->rsi_ble_per_tx.rx_chnl_num,
                    instance->rsi_ble_per_tx.tx_chnl_num,
                    instance->rsi_ble_per_tx.scrambler_seed,
                    instance->rsi_ble_per_tx.payload_type,
                    instance->rsi_ble_per_tx.le_chnl_type,
                    instance->rsi_ble_per_tx.tx_power,
                    instance->rsi_ble_per_tx.transmit_mode,
                    instance->rsi_ble_per_tx.freq_hop_en,
                    instance->rsi_ble_per_tx.ant_sel,
                    instance->rsi_ble_per_tx.inter_pkt_gap,
                    instance->rsi_ble_per_tx.pll_mode,
                    instance->rsi_ble_per_tx.rf_type,
                    instance->rsi_ble_per_tx.rf_chain);
                ble_per_test_app_send_msg(instance);
            }

        } else {
            furi_string_printf(instance->msg, "Invalid state\r\n");
            ble_per_test_app_send_msg(instance);
        }

        break;
    case BLEPerTestCmdTypeModeRx:
        if(instance->state == BLEPerTestStateIdle) {
            instance->rsi_ble_per_rx.receive_enable = ENABLE;
            status = rsi_ble_per_receive(&instance->rsi_ble_per_rx);
            if(status != RSI_SUCCESS) {
                furi_string_printf(
                    instance->msg,
                    "Failed to start BLE PER RX test, error code: 0x%08lX\r\n",
                    status);
                ble_per_test_app_send_msg(instance);
            } else {
                instance->state = BLEPerTestStateRx;
                ble_per_test_app_show_status_start(instance);
                furi_string_printf(
                    instance->msg,
                    "BLE PER test Rx started\r\n"
                    "cmd id: 0x%X\r\n"
                    "enable: %d\r\n"
                    "access_addr: 0x%lX\r\n"
                    "ext_data_len_indication: %d\r\n"
                    "phy_rate: %d\r\n"
                    "rx_chnl_num: %d\r\n"
                    "tx_chnl_num: %d\r\n"
                    "scrambler_seed: %d\r\n"
                    "le_chnl_type: %d\r\n"
                    "loop_back_mode: %d\r\n"
                    "freq_hop_en: %d\r\n"
                    "ant_sel: %d\r\n"
                    "duty_cycling_en: %d\r\n"
                    "pll_mode: %d\r\n"
                    "rf_type: %d\r\n"
                    "rf_chain: %d\r\n",
                    instance->rsi_ble_per_rx.cmd_ix,
                    instance->rsi_ble_per_rx.receive_enable,
                    *(uint32_t*)&instance->rsi_ble_per_rx.access_addr[0],
                    instance->rsi_ble_per_rx.ext_data_len_indication,
                    instance->rsi_ble_per_rx.phy_rate,
                    instance->rsi_ble_per_rx.rx_chnl_num,
                    instance->rsi_ble_per_rx.tx_chnl_num,
                    instance->rsi_ble_per_rx.scrambler_seed,
                    instance->rsi_ble_per_rx.le_chnl_type,
                    instance->rsi_ble_per_rx.loop_back_mode,
                    instance->rsi_ble_per_rx.freq_hop_en,
                    instance->rsi_ble_per_rx.ant_sel,
                    instance->rsi_ble_per_rx.duty_cycling_en,
                    instance->rsi_ble_per_rx.pll_mode,
                    instance->rsi_ble_per_rx.rf_type,
                    instance->rsi_ble_per_rx.rf_chain);
                ble_per_test_app_send_msg(instance);
            }

        } else {
            furi_string_printf(instance->msg, "Invalid state\r\n");
            ble_per_test_app_send_msg(instance);
        }
        break;
    case BLEPerTestCmdTypeModeTxRxStop:
        if(instance->state == BLEPerTestStateTx) {
            ble_per_test_app_show_status_stop(instance);
            instance->rsi_ble_per_tx.transmit_enable = DISABLE;
            status = rsi_ble_per_transmit(&instance->rsi_ble_per_tx);
            if(status != RSI_SUCCESS) {
                furi_string_printf(
                    instance->msg,
                    "Failed to stop BLE PER TX test, error code: 0x%08lX\r\n",
                    status);
                ble_per_test_app_send_msg(instance);
            } else {
                furi_string_printf(instance->msg, "BLE PER test Tx stopped\r\n");
                ble_per_test_app_send_msg(instance);
            }
            instance->state = BLEPerTestStateIdle;
        } else if(instance->state == BLEPerTestStateRx) {
            ble_per_test_app_show_status_stop(instance);
            instance->rsi_ble_per_rx.receive_enable = DISABLE;
            status = rsi_ble_per_receive(&instance->rsi_ble_per_rx);
            if(status != RSI_SUCCESS) {
                furi_string_printf(
                    instance->msg,
                    "Failed to stop BLE PER RX test, error code: 0x%08lX\r\n",
                    status);
                ble_per_test_app_send_msg(instance);
            } else {
                furi_string_printf(instance->msg, "BLE PER test Rx stopped\r\n");
                ble_per_test_app_send_msg(instance);
            }
            instance->state = BLEPerTestStateIdle;
        } else {
            furi_string_printf(instance->msg, "Invalid state\r\n");
            ble_per_test_app_send_msg(instance);
        }
        break;
    case BLEPerTestCmdTypeSetChannel:
        if(instance->state == BLEPerTestStateIdle && furi_string_size(args)) {
            parse_err |= strint_to_uint8(args_cstr, &args_cstr, &arg_uint8, 10);

            if(parse_err == StrintParseNoError) {
                if(arg_uint8 <= 39) {
                    instance->rsi_ble_per_tx.tx_chnl_num = arg_uint8;
                    instance->rsi_ble_per_tx.rx_chnl_num = arg_uint8;
                    instance->rsi_ble_per_rx.tx_chnl_num = arg_uint8;
                    instance->rsi_ble_per_rx.rx_chnl_num = arg_uint8;
                    furi_string_printf(instance->msg, "BLE channel set to %d\r\n", arg_uint8);
                    ble_per_test_app_send_msg(instance);
                } else {
                    furi_string_printf(instance->msg, "Invalid channel\r\n");
                    ble_per_test_app_send_msg(instance);
                }
            } else {
                furi_string_printf(instance->msg, "Invalid argument\r\n");
                ble_per_test_app_send_msg(instance);
            }
        } else {
            furi_string_printf(instance->msg, "Invalid argument or state != IDLE\r\n");
            ble_per_test_app_send_msg(instance);
        }
        break;
    case BLEPerTestCmdTypeSetPhyRate:
        if(instance->state == BLEPerTestStateIdle && furi_string_size(args)) {
            parse_err |= strint_to_uint8(args_cstr, &args_cstr, &arg_uint8, 10);
            if(parse_err == StrintParseNoError) {
                if(arg_uint8 <= BLE_PER_TEST_PHY_RATE_MAX - 1) {
                    instance->rsi_ble_per_tx.phy_rate =
                        ble_per_test_phy_rate[arg_uint8].rate_value;
                    instance->rsi_ble_per_rx.phy_rate =
                        ble_per_test_phy_rate[arg_uint8].rate_value;
                    furi_string_printf(instance->msg, "PHY rate set to %d\r\n", arg_uint8);
                    ble_per_test_app_send_msg(instance);
                } else {
                    furi_string_printf(instance->msg, "Invalid PHY rate\r\n");
                    ble_per_test_app_send_msg(instance);
                }
            } else {
                furi_string_printf(instance->msg, "Invalid argument\r\n");
                ble_per_test_app_send_msg(instance);
            }
        } else {
            furi_string_printf(instance->msg, "Invalid argument or state != IDLE\r\n");
            ble_per_test_app_send_msg(instance);
        }
        break;
    case BLEPerTestCmdTypeSetPayloadLen:
        if(instance->state == BLEPerTestStateIdle && furi_string_size(args)) {
            parse_err |= strint_to_uint8(args_cstr, &args_cstr, &arg_uint8, 10);
            if(parse_err == StrintParseNoError) {
                if(arg_uint8 >= 1) {
                    instance->rsi_ble_per_tx.pkt_len[0] = arg_uint8;
                    furi_string_printf(instance->msg, "Payload length set to %d\r\n", arg_uint8);
                    ble_per_test_app_send_msg(instance);
                } else {
                    furi_string_printf(instance->msg, "Invalid payload length\r\n");
                    ble_per_test_app_send_msg(instance);
                }
            } else {
                furi_string_printf(instance->msg, "Invalid argument\r\n");
                ble_per_test_app_send_msg(instance);
            }
        } else {
            furi_string_printf(instance->msg, "Invalid argument or state != IDLE\r\n");
            ble_per_test_app_send_msg(instance);
        }
        break;
    case BLEPerTestCmdTypeSetPayloadType:
        if(instance->state == BLEPerTestStateIdle && furi_string_size(args)) {
            parse_err |= strint_to_uint8(args_cstr, &args_cstr, &arg_uint8, 10);
            if(parse_err == StrintParseNoError) {
                if(arg_uint8 <= BLE_PER_TEST_PAYLOAD_TYPE_MAX - 1) {
                    instance->rsi_ble_per_tx.payload_type =
                        ble_per_test_payload_type[arg_uint8].payload_type_value;
                    furi_string_printf(instance->msg, "Payload type set to %d\r\n", arg_uint8);
                    ble_per_test_app_send_msg(instance);
                } else {
                    furi_string_printf(instance->msg, "Invalid payload type\r\n");
                    ble_per_test_app_send_msg(instance);
                }
            } else {
                furi_string_printf(instance->msg, "Invalid argument\r\n");
                ble_per_test_app_send_msg(instance);
            }
        } else {
            furi_string_printf(instance->msg, "Invalid argument or state != IDLE\r\n");
            ble_per_test_app_send_msg(instance);
        }
        break;
    case BLEPerTestCmdTypeSetMode:
        if(instance->state == BLEPerTestStateIdle && furi_string_size(args)) {
            parse_err |= strint_to_uint8(args_cstr, &args_cstr, &arg_uint8, 10);
            if(parse_err == StrintParseNoError) {
                if(arg_uint8 <= BLE_PER_TEST_TRANSMIT_MODE_MAX - 1) {
                    instance->rsi_ble_per_tx.transmit_mode =
                        ble_per_test_transmit_mode[arg_uint8].mode_value;
                    furi_string_printf(instance->msg, "Transmit mode set to %d\r\n", arg_uint8);
                    ble_per_test_app_send_msg(instance);
                } else {
                    furi_string_printf(instance->msg, "Invalid transmit mode\r\n");
                    ble_per_test_app_send_msg(instance);
                }
            } else {
                furi_string_printf(instance->msg, "Invalid argument\r\n");
                ble_per_test_app_send_msg(instance);
            }
        } else {
            furi_string_printf(instance->msg, "Invalid argument or state != IDLE\r\n");
            ble_per_test_app_send_msg(instance);
        }
        break;
    case BLEPerTestCmdTypeSetTxPower:
        if(instance->state == BLEPerTestStateIdle && furi_string_size(args)) {
            parse_err |= strint_to_uint8(args_cstr, &args_cstr, &arg_uint8, 10);
            if(parse_err == StrintParseNoError) {
                if((arg_uint8 >= 1 && arg_uint8 <= 10) || arg_uint8 == 127) {
                    instance->rsi_ble_per_tx.tx_power = arg_uint8;
                    furi_string_printf(instance->msg, "Tx power set to %d\r\n", arg_uint8);
                    ble_per_test_app_send_msg(instance);
                } else {
                    furi_string_printf(instance->msg, "Invalid Tx power\r\n");
                    ble_per_test_app_send_msg(instance);
                }
            } else {
                furi_string_printf(instance->msg, "Invalid argument\r\n");
                ble_per_test_app_send_msg(instance);
            }
        } else {
            furi_string_printf(instance->msg, "Invalid argument or state != IDLE\r\n");
            ble_per_test_app_send_msg(instance);
        }
        break;
    case BLEPerTestCmdTypeSetHopping:
        if(instance->state == BLEPerTestStateIdle && furi_string_size(args)) {
            parse_err |= strint_to_uint8(args_cstr, &args_cstr, &arg_uint8, 10);
            if(parse_err == StrintParseNoError) {
                if(arg_uint8 <= BLE_PER_TEST_HOPPING_MODE_MAX - 1) {
                    instance->rsi_ble_per_tx.freq_hop_en =
                        ble_per_test_hopping_mode[arg_uint8].hopping_value;
                    furi_string_printf(instance->msg, "Hopping set to %d\r\n", arg_uint8);
                    ble_per_test_app_send_msg(instance);
                } else {
                    furi_string_printf(instance->msg, "Invalid hopping\r\n");
                    ble_per_test_app_send_msg(instance);
                }
            } else {
                furi_string_printf(instance->msg, "Invalid argument\r\n");
                ble_per_test_app_send_msg(instance);
            }
        } else {
            furi_string_printf(instance->msg, "Invalid argument or state != IDLE\r\n");
            ble_per_test_app_send_msg(instance);
        }
        break;
    default:
        ble_per_test_app_send_msg_invalid_arg(instance);
        break;
    }

    furi_string_free(arg);
    return SL_STATUS_OK;
}

void ble_per_test_app_parse_msg(void* app_handle, uint8_t* data, size_t size) {
    BLEPerTestApp* instance = (BLEPerTestApp*)app_handle;
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

        for(i = 0; i < BLEPerTestCmdTypeMax; i++) {
            if(furi_string_cmp_str(cmd, (char*)ble_per_test_cmd[i].cmd) == 0) {
                cmd_index = i;
                cmd_valid = true;
                break;
            }
        }
        if(cmd_valid) {
            if(ble_per_test_app(instance, cmd_index, args) != SL_STATUS_OK) {
                furi_string_printf(instance->msg, "Command failed\r\n");
                ble_per_test_app_send_msg(instance);
            }
        } else {
            furi_string_printf(instance->msg, "Invalid command\r\n");
            ble_per_test_app_send_msg(instance);
        }
    } while(false);

    furi_string_free(args);
    furi_string_free(cmd);
}

static void ble_per_test_app_cmd_usage(BLEPerTestApp* instance) {
    furi_string_printf(instance->msg, "%s commands usage:\r\n", TAG);
    furi_string_cat_printf(
        instance->msg,
        "*************************************************************************************************************"
        "*********\r\n");
    furi_string_cat_printf(instance->msg, "Tests the BLE PER peripheral role.\r\n");
    furi_string_cat_printf(
        instance->msg,
        "https://github.com/SiliconLabs/wiseconnect/tree/master/examples/featured/ble_per \r\n");

    furi_string_cat_printf(instance->msg, "?\r\n");
    furi_string_cat_printf(instance->msg, "help\r\n");
    furi_string_cat_printf(instance->msg, "tx Tx Start\r\n");
    furi_string_cat_printf(instance->msg, "rx Rx Start\r\n");
    furi_string_cat_printf(instance->msg, "stop Tx/Rx Stop\r\n");
    furi_string_cat_printf(
        instance->msg, "channel <0..39> BLE channels 2402MHz to 2480MHz with 2MHz spacing\r\n");
    furi_string_cat_printf(
        instance->msg, "phy_rate <0..4> PHY 0: 1Mbps, 1: 2Mbps, 2: 125Kbps, 3: 500Kbps\r\n");
    furi_string_cat_printf(instance->msg, "payload_len <1..255> Payload length\r\n");
    furi_string_cat_printf(
        instance->msg,
        "payload_type <0..7> Payload type 0: PRBS9, 1: 11110000, 2: 10101010, 3: PRBS15, 4: 11111111, 5: 00000000, 6: 00001111, 7: 01010101\r\n");
    furi_string_cat_printf(
        instance->msg, "mode <0..3> Transmit mode 0: Burst, 1: Continuous 2: Cw\r\n");
    furi_string_cat_printf(
        instance->msg,
        "hopping <0..2> Frequency hopping 0: No hopping, 1: Fixed Hopping, 2: Random Hopping\r\n");
    furi_string_cat_printf(
        instance->msg,
        "tx_power <1..10 | 127> Transmit power 1..10: 1dBm..10dBm, 127: Max Power Supported by Country regio\r\n");

    furi_string_cat_printf(
        instance->msg,
        "*************************************************************************************************************"
        "*********\r\n");
    ble_per_test_app_send_msg(instance);
}

//############ BLE PER Test App Show status #############################

static int32_t ble_per_test_app_thread_callback(void* context) {
    BLEPerTestApp* instance = (BLEPerTestApp*)context;
    FuriString* msg = furi_string_alloc();
    while(!instance->exit) {
        rsi_bt_per_stats(BT_PER_STATS_CMD_ID, &instance->per_stats);
        if(instance->state == BLEPerTestStateTx) {
            furi_string_printf(
                msg,
                "Tx Stats\r\n"
                "tx_dones: %d\r\n",
                instance->per_stats.tx_dones);
            ble_per_test_app_send_text(instance, msg);
        } else if(instance->state == BLEPerTestStateRx) {
            furi_string_printf(
                msg,
                "Rx Stats\r\n"
                "crc_fail_cnt: %d\r\n"
                "crc_pass_cnt: %d\r\n"
                "rssi: %d\r\n",
                instance->per_stats.crc_fail_cnt,
                instance->per_stats.crc_pass_cnt,
                instance->per_stats.rssi);

            ble_per_test_app_send_text(instance, msg);
        }
        furi_delay_ms(500);
    }
    furi_string_free(msg);
    return 0;
}

void ble_per_test_app_show_status_start(void* app_handle) {
    BLEPerTestApp* instance = (BLEPerTestApp*)app_handle;
    instance->thread = furi_thread_alloc_ex(
        "BLEPerTestShowStatus", 2048, ble_per_test_app_thread_callback, instance);
    instance->exit = false;
    furi_thread_start(instance->thread);
}

void ble_per_test_app_show_status_stop(void* app_handle) {
    BLEPerTestApp* instance = (BLEPerTestApp*)app_handle;
    if(instance->thread) {
        instance->exit = true;
        furi_thread_join(instance->thread);
        furi_thread_free(instance->thread);
        instance->thread = NULL;
    }
}
