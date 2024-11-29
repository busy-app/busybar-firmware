#include <furi.h>

#include "sl_status.h"
#include "sl_wifi.h"
#include "sl_net.h"
#include "sl_si91x_driver.h"
#include "sl_wifi_callback_framework.h"
#include <cmsis_os2.h>

#define TAG "DemoWifiTest"

typedef struct {
    FuriEventLoop* event_loop;
    FuriEventLoopTimer* timer;
} DemoWifiTest;

static const sl_wifi_data_rate_t rate = SL_WIFI_DATA_RATE_6;
static const sl_wifi_tx_test_mode_t mode = SL_WIFI_TEST_BURST_MODE;

#define RECEIVE_STATS           1
#define MAX_RECEIVE_STATS_COUNT 5
#define CHANNEL                 1 // 1ch - 2412MHz 6ch - 2437MHz 11ch - 2462MHz

static const sl_wifi_device_configuration_t transmit_test_configuration = {
    .boot_option = LOAD_NWP_FW,
    .mac_address = NULL,
    .band = SL_SI91X_WIFI_BAND_2_4GHZ,
    .region_code = WORLD_DOMAIN,
    .boot_config = {
        .oper_mode = SL_SI91X_TRANSMIT_TEST_MODE,
        .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
        .feature_bit_map =
#ifdef SLI_SI91X_MCU_INTERFACE
            (SL_SI91X_FEAT_SECURITY_OPEN | SL_SI91X_FEAT_WPS_DISABLE),
#else
            (SL_SI91X_FEAT_SECURITY_OPEN),
#endif
        .tcp_ip_feature_bit_map =
            (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
        .custom_feature_bit_map = SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID,
        .ext_custom_feature_bit_map =
            (MEMORY_CONFIG
#ifdef SLI_SI917
             | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
             ),
        .bt_feature_bit_map = SL_SI91X_BT_RF_TYPE,
        .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
        .ble_feature_bit_map = 0,
        .ble_ext_feature_bit_map = 0,
        .config_feature_bit_map = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP}};

static sl_si91x_request_tx_test_info_t tx_test_info = {
    .enable = 1,
    .power = 127,
    .rate = rate,
    .length = 100,
    .mode = mode,
    .channel = CHANNEL,
    .aggr_enable = 0,
    .no_of_pkts = 0,
#ifdef SLI_SI917
    .enable_11ax = 0,
    .coding_type = 0,
    .nominal_pe = 0,
    .ul_dl = 0,
    .he_ppdu_type = 0,
    .beam_change = 0,
    .bw = 0,
    .stbc = 0,
    .tx_bf = 0,
    .gi_ltf = 0,
    .dcm = 0,
    .nsts_midamble = 0,
    .spatial_reuse = 0,
    .bss_color = 0,
    .he_siga2_reserved = 0,
    .ru_allocation = 0,
    .n_heltf_tot = 0,
    .sigb_dcm = 0,
    .sigb_mcs = 0,
    .user_sta_id = 0,
    .user_idx = 0,
    .sigb_compression_field = 0,
#endif
};

static float pass_avg = 0;
static float fail_avg = 0;
// static uint32_t rssi_avg = 0;
// static uint32_t crc_pass = 0;
// static uint32_t crc_fail = 0;
// static uint32_t cal_rssi = 0;
static uint16_t total_crc_pass = 0;
static uint16_t total_crc_fail = 0;

#if RECEIVE_STATS
static uint8_t stats_count = 0;
#endif
static volatile sl_status_t callback_status = SL_STATUS_OK;

sl_status_t wifi_stats_receive_handler(
    sl_wifi_event_t event,
    void* reponse,
    uint32_t result_length,
    void* arg);

void demo_wifi_test(void* p) {
    UNUSED(p);
    FURI_LOG_I(TAG, "Starting");

    DemoWifiTest* instance = malloc(sizeof(DemoWifiTest));
    instance->event_loop = furi_event_loop_alloc();
    // instance->timer = furi_event_loop_timer_alloc(
    //     instance->event_loop,
    //     notification_timer_callback,
    //     FuriEventLoopTimerTypePeriodic,
    //     instance);

    sl_status_t status;

    do {
        status =
            sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &transmit_test_configuration, NULL, NULL);
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to start Wi-Fi client interface: 0x%lx", status);
            break;
        }
        FURI_LOG_I(TAG, "Wi-Fi Init Done");

#if RECEIVE_STATS
        // Register WLAN receive stats call back handler
        sl_wifi_set_stats_callback(wifi_stats_receive_handler, NULL);
#endif

        status = sl_wifi_set_antenna(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, SL_WIFI_ANTENNA_INTERNAL);
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to start set Antenna: 0x%lx", status);
            break;
        }
        FURI_LOG_I(TAG, "Antenna Command Frame Success");

        if((mode == SL_WIFI_TEST_CONTINOUS_WAVE_MODE) ||
           (mode == SL_WIFI_TEST_CONTINOUS_WAVE_MODE_OFF_CENTER_LOW) ||
           (mode == SL_WIFI_TEST_CONTINOUS_WAVE_MODE_OFF_CENTER_HIGH)) {
            tx_test_info.mode = SL_WIFI_TEST_CONTINOUS_MODE;
            status = sl_si91x_transmit_test_start(&tx_test_info);
            if(status != SL_STATUS_OK) {
                FURI_LOG_E(TAG, "Transmit test start Failed, Error Code : 0x%lX", status);
                break;
            }
            FURI_LOG_I(TAG, "Transmit test start Success");

            // Add delay here to see the TX packets on AIR
            furi_delay_ms(10000);

            status = sl_si91x_transmit_test_stop();
            if(status != SL_STATUS_OK) {
                FURI_LOG_E(TAG, "Transmit test stop Failed, Error Code : 0x%lX", status);
                break;
            }
            FURI_LOG_I(TAG, "Transmit test stop Success");

            furi_delay_ms(5000);
        }

        tx_test_info.mode = mode;
        status = sl_si91x_transmit_test_start(&tx_test_info);
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Transmit test start Failed, Error Code : 0x%lX", status);
            break;
        }
        FURI_LOG_I(TAG, "Transmit test start Success");

        // Add delay here to see the TX packets on AIR
        furi_delay_ms(10000);

        status = sl_si91x_transmit_test_stop();
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Transmit test stop Failed, Error Code : 0x%lX", status);
            break;
        }
        FURI_LOG_I(TAG, "Transmit test stop Success");

#if RECEIVE_STATS
        ////////////////////////////////////////
        // Transmit data/TX from the peer//////
        ////////////////////////////////////////

        // Start/Receive publishing RX stats
        sl_wifi_channel_t channel = {0};
        channel.channel = CHANNEL;
        status = sl_wifi_start_statistic_report(SL_WIFI_CLIENT_INTERFACE, channel);
        if(SL_STATUS_IN_PROGRESS == status) {
            callback_status = SL_STATUS_IN_PROGRESS;
            FURI_LOG_I(TAG, "Receive Statistics...");
            bool ret = true;
            do {
                while(stats_count <= MAX_RECEIVE_STATS_COUNT) {
                    osThreadYield();
                    if(stats_count == MAX_RECEIVE_STATS_COUNT &&
                       callback_status != SL_STATUS_IN_PROGRESS) {
                        FURI_LOG_I(TAG, "Stop Statistics Report");
                        sl_wifi_stop_statistic_report(SL_WIFI_CLIENT_INTERFACE);
                        FURI_LOG_I(TAG, "Start Statistic Report Success");
                        ret = false;
                        break;
                    }
                }
            } while(0);
            if(ret == false) break;
            status = callback_status;
        }
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Start Statistic Report Failed, Error Code : 0x%lX", status);
            break;
        }
#endif
    } while(0);
    furi_event_loop_run(instance->event_loop);
}

#if RECEIVE_STATS
sl_status_t wifi_stats_receive_handler(
    sl_wifi_event_t event,
    void* reponse,
    uint32_t result_length,
    void* arg) {
    UNUSED_PARAMETER(result_length);
    UNUSED_PARAMETER(arg);
    if(SL_WIFI_CHECK_IF_EVENT_FAILED(event)) {
        callback_status = *(sl_status_t*)reponse;
        return SL_STATUS_FAIL;
    }

    if(event == SL_WIFI_STATS_AYSNC_EVENT) {
        sl_si91x_async_stats_response_t* result = (sl_si91x_async_stats_response_t*)reponse;

        FURI_LOG_I(TAG, "WIFI STATS Recieved packet# %d", stats_count);
        FURI_LOG_I(
            TAG,
            "stats : crc_pass %d, crc_fail %d, cal_rssi :%d",
            result->crc_pass,
            result->crc_fail,
            result->cal_rssi);
        float p = result->crc_pass;
        float f = result->crc_fail;
        float t = p + f;

        float per_pass = (p * 100 / t);
        float per_fail = (f * 100 / t);

        pass_avg += per_pass;
        fail_avg += per_fail;

        total_crc_pass += result->crc_pass;
        total_crc_fail += result->crc_fail;

        if(stats_count == MAX_RECEIVE_STATS_COUNT - 1) {
            FURI_LOG_I(
                TAG,
                "CRC Average pass%% = %.6f,         CRC Average fail%% = %.6f",
                (double)pass_avg / MAX_RECEIVE_STATS_COUNT,
                (double)fail_avg / MAX_RECEIVE_STATS_COUNT);
            FURI_LOG_I(
                TAG,
                "Total : total_crc_pass %d, total_crc_fail %d",
                total_crc_pass,
                total_crc_fail);
            pass_avg = 0;
            fail_avg = 0;
        }
        stats_count++;
        callback_status = SL_STATUS_OK;
    }
    return SL_STATUS_OK;
}
#endif
