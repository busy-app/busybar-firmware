#include "wifi_config.h"

#include "ble_config.h"

#ifndef TX_POOL_RATIO
#define TX_POOL_RATIO 1
#endif

#ifndef RX_POOL_RATIO
#define RX_POOL_RATIO 1
#endif

#ifndef GLOBAL_POOL_RATIO
#define GLOBAL_POOL_RATIO 1
#endif

const sl_wifi_device_configuration_t wifi_config_client = {
    .boot_option = LOAD_NWP_FW,
    .mac_address = NULL,
    .band = SL_SI91X_WIFI_BAND_2_4GHZ,
    .region_code = WORLD_DOMAIN,
    .boot_config =
        {
            .oper_mode = SL_SI91X_CLIENT_MODE,
            .coex_mode = SL_SI91X_WLAN_BLE_MODE,
            .feature_bit_map =
                (SL_SI91X_FEAT_SECURITY_OPEN | SL_SI91X_FEAT_SECURITY_PSK |
                 SL_SI91X_FEAT_AGGREGATION | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE |
                 SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1),
            .tcp_ip_feature_bit_map =
                (SL_SI91X_TCP_IP_FEAT_BYPASS | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
            .custom_feature_bit_map =
                (SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID |
                 SL_SI91X_CUSTOM_FEAT_SOC_CLK_CONFIG_120MHZ |
                 SL_SI91X_CUSTOM_FEAT_ASYNC_CONNECTION_STATUS),
            .ext_custom_feature_bit_map =
                (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_SI91X_EXT_FEAT_IEEE_80211W | MEMORY_CONFIG
#ifdef SLI_SI917
                 | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                 | SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE),
            .bt_feature_bit_map = (SL_SI91X_BT_RF_TYPE | SL_SI91X_ENABLE_BLE_PROTOCOL),
#ifdef RSI_PROCESS_MAX_RX_DATA
            .ext_tcp_ip_feature_bit_map =
                (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID | SL_SI91X_EXT_TCP_MAX_RECV_LENGTH),
#else
            .ext_tcp_ip_feature_bit_map = (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
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
            .config_feature_bit_map = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP,
        },
    .ta_pool =
        {
            .tx_ratio_in_buffer_pool = TX_POOL_RATIO,
            .rx_ratio_in_buffer_pool = RX_POOL_RATIO,
            .global_ratio_in_buffer_pool = GLOBAL_POOL_RATIO,
        },
};
