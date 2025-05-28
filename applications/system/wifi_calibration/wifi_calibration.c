#include "wifi_calibration.h"
#include <furi.h>

#include <sl_status.h>
#include <sl_wifi.h>
#include <sl_net.h>
#include <sl_si91x_driver.h>

#include <cli/args.h>
#include <cli/shell/cli_shell.h>
#include <cli/cli_ansi.h>
#include <strint.h>
#include <stdlib.h>
#include <string.h>

#define TAG "Calibration"

#define BURST_MODE      0 // Burst Mode
#define CONTINUOUS_MODE 1 // Continuous Mode
#define CW_MODE         2 // Wave Mode (non modulation) in DC mode
#define CW_MODE_25      3 // Wave Mode (non modulation) in single tone mode (center frequency -2.5 MHz)
#define CW_MODE_50      4 // Wave Mode (non modulation) in single tone mode (center frequency +5.0 MHz)
#define TX_TEST_MODE    CW_MODE // Test mode is changed here

/* 
ch  min     typ     max
1	2.401   2.412	2.423
2	2.406   2.417	2.428
3	2.411   2.422	2.433
4	2.416   2.427	2.438
5	2.421   2.432	2.443
6	2.426   2.437	2.448
7	2.431   2.442	2.453
8	2.436   2.447	2.458
9	2.441   2.452	2.463
10	2.446   2.457	2.468
11	2.451   2.462	2.473
12	2.456   2.467	2.478
13	2.461   2.472	2.483
*/
#define CHANNEL 1

enum CalibCmdType {
    CalibCmdTypeHelp,
    CalibCmdTypeHelpHelp,
    CalibCmdTypeFreqOffset,
    CalibCmdTypeCalibWrite,
    CalibCmdTypeEvmOffest,
    CalibCmdTypeEvmWrite,
    CalibCmdTypeSetMode,
    CalibCmdTypeSetCannel,
    CalibCmdTypeSetPower,
    CalibCmdTypeSetRate,
#ifdef SLI_SI917
    DPD_CALIB_WRITE,
#endif
    CalibCmdTypeMaxCmd,
};

typedef struct {
    const char* str_val;
    uint16_t int_val;
} CalibOption;

const CalibOption calib_rate_options[] = {
    {"1", 0},      {"2", 2},      {"5.5", 4},       {"11", 6},     {"6", 139},    {"9", 143},
    {"12", 138},   {"18", 142},   {"24", 137},      {"36", 141},   {"48", 136},   {"54", 140},
    {"MCS0", 256}, {"MCS1", 257}, {"MCS2", 258},    {"MCS3", 259}, {"MCS4", 260}, {"MCS5", 261},
    {"MCS6", 262}, {"MCS7", 263}, {"MCS7_SG", 775},
};

typedef enum {
    CalibStateIdle,
    CalibStateTx,
} CalibState;

typedef struct {
    FuriString* msg;
    CliShell* shell;
    sl_si91x_calibration_read_t target;
    sl_si91x_calibration_read_t calib_read_pkt;
    sl_si91x_calibration_write_t calib_pkt;
    sl_si91x_freq_offset_t freq_calib_pkt;
    sl_si91x_evm_offset_t evm_offset_pkt;
    sl_si91x_evm_write_t evm_write_pkt;
    //sl_si91x_efuse_read_t efuse_read_pkt;
    sl_si91x_get_dpd_calib_data_t dpd_calib_pkt;

    CalibState state;
} CalibApp;

const sl_wifi_data_rate_t rate = SL_WIFI_DATA_RATE_6;
/*
https://www.silabs.com/documents/public/application-notes/an1436-siwx917-qms-crystal-calibration-application-note.pdf
 Note:
 • Tx burst mode can only be used if test instrument supports Modulation analysis measurement, where the Freq error can be reported
 by the instrument.
 • Tx CW mode is not implemented in this example. To access CW mode, modify the app.c file based as mentioned in the following
 section.
*/
sl_si91x_request_tx_test_info_t tx_test_info = {
    .enable = 1,
    .power = 12, // Sets TX power in dBm. The valid values are from (2 to 18)dBm and 127.
    .rate = rate,
    .length =
        30, // Configures length of the TX packet. Valid values are in the range of 24 to 1500 bytes in the burst mode.
    .mode = CONTINUOUS_MODE,
    .channel = CHANNEL,
    .aggr_enable = 0,
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

static const sl_wifi_device_configuration_t calibration_configuration = {
    .boot_option = LOAD_NWP_FW,
    .mac_address = NULL,
    .band = SL_SI91X_WIFI_BAND_2_4GHZ,
    .region_code = WORLD_DOMAIN,
    .boot_config =
        {.oper_mode = SL_SI91X_TRANSMIT_TEST_MODE,
         .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
         .feature_bit_map = (SL_SI91X_FEAT_SECURITY_PSK | SL_SI91X_FEAT_AGGREGATION),
         .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT),
         .custom_feature_bit_map = (SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID),
         .ext_custom_feature_bit_map =
             (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS |
              MEMORY_CONFIG | SL_SI91X_EXT_FEAT_DISABLE_XTAL_CORRECTION
#ifdef SLI_SI917B0
              | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
              ),
         .bt_feature_bit_map = 0,
         .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
         .ble_feature_bit_map = 0,
         .ble_ext_feature_bit_map = 0,
         .config_feature_bit_map = 0},
};

void calibration_app_stop(void* app_handle);

#define MAX_DPD_TRAINING_CHANNELS 6
uint8_t channel_sel[MAX_DPD_TRAINING_CHANNELS] = {1, 3, 6, 8, 11, 13};

sl_status_t
    sl_process_dpd_calibration(CalibApp* instance, sl_si91x_get_dpd_calib_data_t* dpd_power_inx) {
    uint8_t i;
    sl_status_t status = SL_STATUS_OK;
    sl_si91x_calibration_write_t calib_pkt = {0};
    calib_pkt.target = 1;
    calib_pkt.flags = 256;
    status = sl_si91x_transmit_test_stop();
    if(status != SL_STATUS_OK) {
        printf("Transmit failed to stop %lx\r\n", status);
        return status;
    } else {
        printf("Transmit command stopped\r\n");
        instance->state = CalibStateIdle;
    }

    for(i = 0; i < MAX_DPD_TRAINING_CHANNELS; i++) {
        //! Checking the region code if the channel number is above 11
        //!
        if(!((channel_sel[i] > 11) && (calibration_configuration.region_code < 2))) {
            tx_test_info.channel = channel_sel[i];
            status = sl_si91x_transmit_test_start(&tx_test_info);
            if(status != SL_STATUS_OK) {
                printf("Transmit failed with channel num %lx\r\n", status);
                return status;
            } else {
                printf(
                    "Transmit command started with channel num %x\r\n",
                    channel_sel[i]);
                instance->state = CalibStateTx;
            }
            furi_delay_ms(1000);

            status = sl_si91x_transmit_test_stop();
            if(status != SL_STATUS_OK) {
                printf("Transmit failed to stop %lx\r\n", status);
                return status;
            } else {
                printf("Transmit command stopped\r\n");
                instance->state = CalibStateIdle;
            }
            furi_delay_ms(1000);
        }
        if(i == MAX_DPD_TRAINING_CHANNELS - 1) {
            status = sl_si91x_dpd_calibration(dpd_power_inx);
            if(status != SL_STATUS_OK) {
                printf("rsi_calibration_dpd_failed %lx\r\n", status);
                return status;
            } else {
                printf("calib val collect\r\n");
            }
            furi_delay_ms(1000);
            status = sl_si91x_calibration_write(calib_pkt);
            if(status != SL_STATUS_OK) {
                printf("rsi_calib_write failed with error %lx\r\n", status);
                return status;
            } else {
                printf("calib-write pass\n");
            }
        } else {
            status = sl_si91x_dpd_calibration(dpd_power_inx);
            if(status != SL_STATUS_OK) {
                printf("rsi_calibration_dpd_failed %lx\r\n", status);
                return status;
            } else {
                printf("calib val collect\r\n");
            }
        }
        furi_delay_ms(1000);
    }
    return status;
}

sl_status_t calibration_app_flow_transmit_tx(CalibApp* instance) {
    sl_status_t status = SL_STATUS_FAIL;
    uint16_t mode_temp = tx_test_info.mode;
    do {
        status = sl_si91x_transmit_test_stop();
        if(status != SL_STATUS_OK) {
            break;
        }
        furi_delay_ms(200);
        tx_test_info.mode = CONTINUOUS_MODE;
        status = sl_si91x_transmit_test_start(&tx_test_info);
        if(status != SL_STATUS_OK) {
            break;
        }
        furi_delay_ms(200);
        status = sl_si91x_transmit_test_stop();
        if(status != SL_STATUS_OK) {
            break;
        }
        furi_delay_ms(200);
        tx_test_info.mode = mode_temp;
        status = sl_si91x_transmit_test_start(&tx_test_info);
    } while(0);
    if(status != SL_STATUS_OK) {
        printf("Transmit test start failed: 0x%lx\r\n", status);
        return status;
    } else {
        printf("Transmit test started\r\n");
        instance->state = CalibStateTx;
    }

    return status;
}

static bool calibration_generic_enum_arg_command_guard(FuriString* args, const CalibOption* options, uint16_t* arg_out) {
    FuriString* arg = furi_string_alloc();
    bool ret_val = false;
    do {
        if(!args_read_string_and_trim(args, arg)) {
            printf(ANSI_FG_RED "expected string argument\r\n" ANSI_RESET);
            break;
        }

        const CalibOption* iter = options;
        while(iter->str_val) {
            if(furi_string_cmp_str(arg, iter->str_val) == 0) {
                *arg_out = iter->int_val;
                break;
            }
            iter++;
        }

        if(iter->str_val) {
            ret_val = true;
        } else {
            printf(ANSI_FG_RED "unrecognized variant\r\n" ANSI_RESET);
        }
    } while(0);
    furi_string_free(arg);

    if(!ret_val) {
        printf("available variants:\r\n");

        const CalibOption* iter = options;
        while(iter->str_val) {
            printf("  ");
            printf(iter->str_val);
            iter++;
        }

        printf("\r\n");
    }

    return ret_val;
}

static void calibration_sl_freq_offset_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    CalibApp* instance = context;
    const char* args_cstr = furi_string_get_cstr(args);
        StrintParseError parse_err = strint_to_int32(
            args_cstr, NULL, &instance->freq_calib_pkt.frequency_offset_in_khz, 10);

        if(parse_err == StrintParseNoError) {
            sl_status_t status = sl_si91x_frequency_offset(&instance->freq_calib_pkt);
            if(status != SL_STATUS_OK) {
                printf("Frequency offset correction failed: 0x%lx\r\n", status);
                //return status;
            } else {
                printf("Frequency offset correction successful\r\n");
            }
        } else {
            printf("sl_freq_offset <freq_offset_in_KHz>\r\n");
        }
}

static void calibration_sl_calib_write_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    CalibApp* instance = context;
    char* args_cstr = (char*)furi_string_get_cstr(args);
        StrintParseError parse_err = StrintParseNoError;
        parse_err |= strint_to_uint8(args_cstr, &args_cstr, &instance->calib_pkt.target, 10);
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &instance->calib_pkt.flags, 10);
        parse_err |=
            strint_to_int8(args_cstr, &args_cstr, &instance->calib_pkt.gain_offset[0], 10);
        parse_err |=
            strint_to_int8(args_cstr, &args_cstr, &instance->calib_pkt.gain_offset[1], 10);
        parse_err |=
            strint_to_int8(args_cstr, &args_cstr, &instance->calib_pkt.gain_offset[2], 10);
        if(*args_cstr != 0x00) {
            parse_err |=
                strint_to_int8(args_cstr, &args_cstr, &instance->calib_pkt.xo_ctune, 10);
            parse_err |=
                strint_to_int8(args_cstr, NULL, &instance->calib_pkt.gain_offset_ch14, 10);
        }
        if(parse_err == StrintParseNoError) {
            sl_status_t status = sl_si91x_calibration_write(instance->calib_pkt);
            //furi_delay_ms(500);
            if(status != SL_STATUS_OK) {
                printf("Calibration data write failed: 0x%lx\r\n", status);
                return;
            } else {
                printf("Calibration data write successful\r\n");
            }
            instance->target.target = instance->calib_pkt.target;
            status = sl_si91x_calibration_read(instance->target, &instance->calib_read_pkt);
            if(status != SL_STATUS_OK) {
                printf("Calibration data read failed: 0x%lx\r\n", status);
                return;
            } else {
                printf("Calibration data read successful\r\n");
                printf(
                    "target %d, gain_offset_low:%d, gain_offset_2:%d, gain_offset_3:%d,xo_tune:%d,gain_offset_ch14:%d\r\n",
                    instance->calib_read_pkt.target,
                    instance->calib_read_pkt.gain_offset[0],
                    instance->calib_read_pkt.gain_offset[1],
                    instance->calib_read_pkt.gain_offset[2],
                    instance->calib_read_pkt.xo_ctune,
                    instance->calib_read_pkt.gain_offset_ch14);
            }
        } else {
            printf("sl_calib_write <target> <flags> <gain_offset_low> <gain_offset_mid> <gain_offset_high> <xo_ctune> <gain_"
        "offset_ch14>\r\n");
        }
}

static void calibration_sl_evm_offset_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    CalibApp* instance = context;
    char* args_cstr = (char*)furi_string_get_cstr(args);
        StrintParseError parse_err = StrintParseNoError;
        parse_err |=
            strint_to_uint8(args_cstr, &args_cstr, &instance->evm_offset_pkt.evm_index, 10);
        parse_err |= strint_to_int8(
            args_cstr, &args_cstr, &instance->evm_offset_pkt.evm_offset_val, 10);

        if(parse_err == StrintParseNoError) {
            sl_status_t status = sl_si91x_transmit_test_stop();
            if(status != SL_STATUS_OK) {
                printf("Transmit test stop failed: 0x%lx\r\n", status);
                return;
            } else {
                printf("Transmit test stopped\r\n");
                instance->state = CalibStateIdle;
            }

            status = sl_si91x_evm_offset(&instance->evm_offset_pkt);
            if(status != SL_STATUS_OK) {
                printf("EVM offset correction failed: 0x%lx\r\n", status);
                return;
            } else {
                printf("EVM offset correction successful\r\n");
            }

            status = sl_si91x_transmit_test_start(&tx_test_info);
            if(status != SL_STATUS_OK) {
                printf("Transmit test start failed: 0x%lx\r\n", status);
                return;
            } else {
                printf("Transmit test started\r\n");
                instance->state = CalibStateTx;
            }
        } else {
            printf("sl_evm_offset <index> <evm_offset>\r\n");
        }
}

static void calibration_sl_evm_write_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    CalibApp* instance = context;
    char* args_cstr = (char*)furi_string_get_cstr(args);
        StrintParseError parse_err = StrintParseNoError;
        parse_err |=
            strint_to_uint8(args_cstr, &args_cstr, &instance->evm_write_pkt.target, 10);
        parse_err |=
            strint_to_uint32(args_cstr, &args_cstr, &instance->evm_write_pkt.flags, 10);
        parse_err |= strint_to_uint8(
            args_cstr, &args_cstr, &instance->evm_write_pkt.evm_offset_11B, 10);
        parse_err |= strint_to_uint8(
            args_cstr,
            &args_cstr,
            &instance->evm_write_pkt.evm_offset_11G_36M_54M_11N_MCS3_MCS7,
            10);
        parse_err |= strint_to_uint8(
            args_cstr,
            &args_cstr,
            &instance->evm_write_pkt.evm_offset_11G_6M_24M_11N_MCS0_MCS2,
            10);
        parse_err |= strint_to_uint8(
            args_cstr, &args_cstr, &instance->evm_write_pkt.evm_offset_11N_MCS0, 10);
        parse_err |=
            strint_to_uint8(args_cstr, NULL, &instance->evm_write_pkt.evm_offset_11N_MCS7, 10);

        if(parse_err == StrintParseNoError) {
            sl_status_t status = sl_si91x_evm_write(&instance->evm_write_pkt);
            if(status != SL_STATUS_OK) {
                printf("EVM offset correction failed: 0x%lx\r\n", status);
                return;
            } else {
                printf("EVM offset correction successful\r\n");
            }
        } else {
            printf("sl_evm_write <target> <flags> <evm_offset_11B> <evm_offset_11G_36M_54M_11N_MCS3_MCS7> <evm_offset_11G_6M_24M_"
        "11N_MCS0_MCS2> <evm_offset_11N_MCS0>,<evm_offset_11N_MCS7>\r\n");
        }
}

static void calibration_sl_process_dpd_calibration_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    CalibApp* instance = context;
    sl_status_t status = sl_process_dpd_calibration(instance, &instance->dpd_calib_pkt);
    if(status != SL_STATUS_OK) {
        printf("DPD calibration failed: 0x%lx\r\n", status);
    } else {
        printf("DPD calibration successful\r\n");
    }
}

static void calibration_set_mode_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    CalibApp* instance = context;
    char* args_cstr = (char*)furi_string_get_cstr(args);
        StrintParseError parse_err = strint_to_uint16(args_cstr, NULL, &tx_test_info.mode, 10);
        if(parse_err == StrintParseNoError) {
            calibration_app_flow_transmit_tx(instance);
        } else {
            printf("set_mode <index> %d-Burst, %d-Continuos, %d-CW, %d-CW-2,5, %d-CW+5\r\n",
        BURST_MODE,
        CONTINUOUS_MODE,
        CW_MODE,
        CW_MODE_25,
        CW_MODE_50);
        }
}

static void calibration_set_channel_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    CalibApp* instance = context;
    char* args_cstr = (char*)furi_string_get_cstr(args);
        uint16_t channel = 0;
        StrintParseError parse_err = strint_to_uint16(args_cstr, NULL, &channel, 10);
        if(parse_err == StrintParseNoError) {
            if(channel > 13 || channel < 1) {
                printf("Invalid channel number\r\n");
                return;
            } else {
                tx_test_info.channel = channel;
                calibration_app_flow_transmit_tx(instance);
            }
        } else {
            printf("set_channel <channel> 1-13, 1-2412Mhz, 6-2437Mhz, 13-2472Mhz\r\n");
        }
}

static void calibration_set_power_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    CalibApp* instance = context;
    char* args_cstr = (char*)furi_string_get_cstr(args);
        uint16_t power = 0;
        StrintParseError parse_err = strint_to_uint16(args_cstr, NULL, &power, 10);
        if(parse_err == StrintParseNoError) {
            if(((power > 18) && (power != 127)) || power < 2) {
                printf("Invalid power value\r\n");
                return;
            } else {
                tx_test_info.power = power;
                calibration_app_flow_transmit_tx(instance);
            }
        } else {
            printf("set_power <power>  default=%d, 2-18dBm, 127-max power\r\n", tx_test_info.power);
        }
}

static void calibration_set_rate_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    CalibApp* instance = context;
    uint16_t rate;
    if(calibration_generic_enum_arg_command_guard(args, calib_rate_options, &rate)) {
        tx_test_info.rate = rate;
        calibration_app_flow_transmit_tx(instance);
        printf("Rate set\r\n");
    }
}

void* calibration_app_start(CliShell* shell) {
    FURI_LOG_I(TAG, "Starting");

    CalibApp* instance = malloc(sizeof(CalibApp));
    instance->msg = furi_string_alloc();
    instance->shell = shell;

    memset(&instance->target, 0, sizeof(sl_si91x_calibration_read_t));
    memset(&instance->calib_read_pkt, 0, sizeof(sl_si91x_calibration_read_t));
    memset(&instance->calib_pkt, 0, sizeof(sl_si91x_calibration_write_t));
    memset(&instance->freq_calib_pkt, 0, sizeof(sl_si91x_freq_offset_t));
    memset(&instance->evm_offset_pkt, 0, sizeof(sl_si91x_evm_offset_t));
    memset(&instance->evm_write_pkt, 0, sizeof(sl_si91x_evm_write_t));
    //memset(&instance->efuse_read_pkt, 0, sizeof(sl_si91x_efuse_read_t));
    memset(&instance->dpd_calib_pkt, 0, sizeof(sl_si91x_get_dpd_calib_data_t));

    instance->target.target = 1;
    instance->calib_pkt.target = 1;
    instance->calib_pkt.flags = 6;
    instance->calib_pkt.xo_ctune = 80;

    instance->freq_calib_pkt.frequency_offset_in_khz = 30;

    instance->evm_offset_pkt.evm_offset_val = 12;
    instance->evm_offset_pkt.evm_index = 1;

    instance->evm_write_pkt.target = 1;
    instance->evm_write_pkt.flags = 2;
    instance->evm_write_pkt.evm_offset_11G_6M_24M_11N_MCS0_MCS2 = 10;

    //instance->efuse_read_pkt.efuse_read_addr_offset = 602;
    //instance->efuse_read_pkt.efuse_read_data_len    = 20;

    instance->dpd_calib_pkt.dpd_power_index = 127;

    instance->state = CalibStateIdle;

    sl_status_t status = SL_STATUS_FAIL;
    do {
        status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &calibration_configuration, NULL, NULL);
        if(status != SL_STATUS_OK) {
            furi_string_printf(
                instance->msg, "Failed to start Wi-Fi client interface: 0x%lx", status);
            cli_shell_notification_print(instance->shell, instance->msg);
            furi_delay_ms(200);
            break;
        } else {
            furi_string_printf(instance->msg, "Wi-Fi initialization successful");
            cli_shell_notification_print(instance->shell, instance->msg);
        }
        FURI_LOG_D(TAG, "Wi-Fi initialization successful");

        tx_test_info.mode = CONTINUOUS_MODE;
        status = sl_si91x_transmit_test_start(&tx_test_info);
        if(status != SL_STATUS_OK) {
            furi_string_printf(instance->msg, "Transmit test start failed: 0x%lx", status);
            cli_shell_notification_print(instance->shell, instance->msg);
            break;
        }
        FURI_LOG_D(TAG, "Wi-Fi transmit CONTINUOUS_MODE started");
        instance->state = CalibStateTx;
        furi_delay_ms(200);
        status = sl_si91x_transmit_test_stop();
        if(status != SL_STATUS_OK) {
            furi_string_printf(instance->msg, "Transmit test stop failed: 0x%lx", status);
            cli_shell_notification_print(instance->shell, instance->msg);
            break;
        }
        FURI_LOG_D(TAG, "Wi-Fi transmit CONTINUOUS_MODE stopped");
        instance->state = CalibStateIdle;
        furi_delay_ms(200);
        tx_test_info.mode = TX_TEST_MODE;

        status = sl_si91x_transmit_test_start(&tx_test_info);
        if(status != SL_STATUS_OK) {
            furi_string_printf(instance->msg, "Transmit test start failed: 0x%lx", status);
            cli_shell_notification_print(instance->shell, instance->msg);
            break;
        } else {
            furi_string_printf(instance->msg, "Transmit test started");
            cli_shell_notification_print(instance->shell, instance->msg);
        }
        instance->state = CalibStateTx;
        FURI_LOG_D(TAG, "Wi-Fi transmit started");
    } while(0);

    if(status != SL_STATUS_OK) {
        calibration_app_stop(instance);
        return NULL;
    }

    return (void*)instance;
}

void calibration_app_stop(void* app_handle) {
    CalibApp* instance = (CalibApp*)app_handle;
    FURI_LOG_I(TAG, "Stopping");

    if(instance) {
        if(instance->state == CalibStateTx) {
            sl_si91x_transmit_test_stop();
            FURI_LOG_D(TAG, "Wi-Fi transmit stopped");
        }

        furi_string_free(instance->msg);
        free(instance);
        instance = NULL;
    }

    sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
    FURI_LOG_D(TAG, "Wi-Fi deinitialization successful");
}

static void wifi_calibration_motd(void* context) {
    UNUSED(context);
    printf("\r\n+-------------------------------------+\r\n");
    printf("| Welcome to Wi-Fi calibration shell! |\r\n");
    printf("+-------------------------------------+\r\n\r\n");
    printf("Read the manual: https://www.silabs.com/documents/public/application-notes/an1440-siwx917-gain-offset-calibration.pdf\r\n");
    printf("Read the manual: https://www.silabs.com/documents/public/application-notes/an1436-siwx917-qms-crystal-calibration-application-note.pdf\r\n");
    printf("Read the manual: https://github.com/SiliconLabs/wiseconnect/tree/master/examples/snippets/wlan/calibration_app\r\n");
}

void wifi_calibration_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

    CliRegistry* registry = cli_registry_alloc();
    CliShell* shell = cli_shell_alloc(wifi_calibration_motd, NULL, pipe, registry, NULL);
    cli_shell_set_prompt(shell, "wifi_calibration");
    cli_shell_start(shell);

    CalibApp* app = calibration_app_start(shell);
    cli_registry_add_command(registry, "sl_freq_offset", CliCommandFlagDefault, calibration_sl_freq_offset_command, app);
    cli_registry_add_command(registry, "sl_calib_write", CliCommandFlagDefault, calibration_sl_calib_write_command, app);
    cli_registry_add_command(registry, "sl_evm_offset", CliCommandFlagDefault, calibration_sl_evm_offset_command, app);
    cli_registry_add_command(registry, "sl_evm_write", CliCommandFlagDefault, calibration_sl_evm_write_command, app);
#ifdef SLI_SI917
    cli_registry_add_command(registry, "sl_process_dpd_calibration", CliCommandFlagDefault, calibration_sl_process_dpd_calibration_command, app);
#endif
    cli_registry_add_command(registry, "set_mode", CliCommandFlagDefault, calibration_set_mode_command, app);
    cli_registry_add_command(registry, "set_channel", CliCommandFlagDefault, calibration_set_channel_command, app);
    cli_registry_add_command(registry, "set_power", CliCommandFlagDefault, calibration_set_power_command, app);
    cli_registry_add_command(registry, "set_rate", CliCommandFlagDefault, calibration_set_rate_command, app);
    cli_shell_join(shell);
    calibration_app_stop(app);

    cli_shell_free(shell);
    cli_registry_free(registry);
}
