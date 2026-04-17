/*******************************************************************************
 * @file
 * @brief Calibration Application
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include "sl_iostream.h"
#include "sl_iostream_init_instances.h"
#include "sl_iostream_handles.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "sl_board_configuration.h"
#include "cmsis_os2.h"
#include "sl_wifi.h"
#include "sl_net.h"
#include "sl_utility.h"
#include "sl_si91x_driver.h"
#include <strings.h>
/******************************************************
 *                      Macros
 ******************************************************/
/******************************************************
 *                    Constants
 ******************************************************/
#define BURST_MODE      0
#define CONTINUOUS_MODE 1

#define MAX_CALIB_COMMAND_LENGTH 200
#define MIN_CALIB_COMMAND_LENGTH (strlen("sl_evm_write=") + 2 /*atleast one char of input data and CR/LF */)
#define NO_OF_CALIB_COMMANDS     5

#define PARSE_1_BYTE  1
#define PARSE_2_BYTES 2
#define PARSE_4_BYTES 4

typedef struct calib_commands_t {
  uint8_t cmd[MAX_CALIB_COMMAND_LENGTH];
} calib_commands_t;

enum calib_cmd_types {
  FREQ_OFFSET,
  CALIB_WRITE,
  EVM_OFFSET,
  EVM_WRITE,
#ifdef SLI_SI917
  DPD_CALIB_WRITE
#endif
};

calib_commands_t calib_commands[NO_OF_CALIB_COMMANDS] = { { "sl_freq_offset=" },
                                                          { "sl_calib_write=" },
                                                          { "sl_evm_offset=" },
                                                          { "sl_evm_write=" },
                                                          { "sl_process_dpd_calibration=" } };

/******************************************************
 *               Global Variable
 ******************************************************/

const osThreadAttr_t thread_attributes = {
  .name       = "app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityLow,
  .tz_module  = 0,
  .reserved   = 0,
};

const sl_wifi_data_rate_t rate              = SL_WIFI_DATA_RATE_1;
sl_wifi_request_tx_test_info_t tx_test_info = {
  .enable      = 1,
  .power       = 18,
  .rate        = rate,
  .length      = 30,
  .mode        = BURST_MODE,
  .channel     = 1,
  .aggr_enable = 0,
#ifdef SLI_SI917
  .enable_11ax            = 0,
  .coding_type            = 0,
  .nominal_pe             = 0,
  .ul_dl                  = 0,
  .he_ppdu_type           = 0,
  .beam_change            = 0,
  .bw                     = 0,
  .stbc                   = 0,
  .tx_bf                  = 0,
  .gi_ltf                 = 0,
  .dcm                    = 0,
  .nsts_midamble          = 0,
  .spatial_reuse          = 0,
  .bss_color              = 0,
  .he_siga2_reserved      = 0,
  .ru_allocation          = 0,
  .n_heltf_tot            = 0,
  .sigb_dcm               = 0,
  .sigb_mcs               = 0,
  .user_sta_id            = 0,
  .user_idx               = 0,
  .sigb_compression_field = 0,
#endif
};

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

#ifndef BUFSIZE
#define BUFFER_SIZE 200 //Input buffer size
#endif

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

/* Input buffer */
static char buffer[BUFFER_SIZE];
uint32_t status   = 0;
bool end_of_cmd   = false;
bool cmd_valid    = true;
uint8_t cmd_index = -1;

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
sl_status_t sl_process_dpd_calibration(sl_si91x_get_dpd_calib_data_t *dpd_power_inx);
/***************************************************************************/ /**
 * Initialize example.
 ******************************************************************************/

static const sl_wifi_device_configuration_t calibration_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = WORLD_DOMAIN,
  .boot_config = { .oper_mode              = SL_SI91X_TRANSMIT_TEST_MODE,
                   .coex_mode              = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map        = (SL_WIFI_FEAT_SECURITY_PSK | SL_WIFI_FEAT_AGGREGATION),
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT),
                   .custom_feature_bit_map = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS | MEMORY_CONFIG
                      | SL_SI91X_EXT_FEAT_DISABLE_XTAL_CORRECTION
#if defined(SLI_SI917B0)
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = 0 }
};

/******************************************************
 *               Function Declarations
 ******************************************************/
static void application_start(void *argument);
sl_status_t calibration_app(void);
void iostream_usart_init(void);
void iostream_rx(void);
void display_calib_cmd_usage(void);
void validate_input_cmd(void);

/******************************************************
 *               Function Definitions
 ******************************************************/

void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

void iostream_usart_init()
{
  /* Prevent buffering of output/input.*/
#if !defined(__CROSSWORKS_ARM) && defined(__GNUC__)
  setvbuf(stdout, NULL, _IONBF, 0); /*Set unbuffered mode for stdout (newlib)*/
  setvbuf(stdin, NULL, _IONBF, 0);  /*Set unbuffered mode for stdin (newlib)*/
#endif
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;
  printf("\r\n initializing usart \r\n");
  iostream_usart_init();
  printf("\r\n initialised usart \r\n");

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &calibration_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  } else {
    printf("Wi-Fi initialization successful\r\n");
  }

  status = sl_si91x_transmit_test_start(&tx_test_info);
  if (status != SL_STATUS_OK) {
    printf("Transmit test start failed: 0x%lx\r\n", status);
    return;
  } else {
    printf("Transmit test started\r\n");
  }

  status = calibration_app();

  if (status != SL_STATUS_OK) {
    printf("Calibration test failed with status 0x%lx\r\n", status);
    return;
  }
}

void iostream_rx()
{
  char c               = 0;
  static uint8_t index = 0;
  sl_iostream_getchar(SL_IOSTREAM_STDIN, &c);
  if (c > 0) {
    if ((c == '\n')) {
      if (buffer[index - 1] == '\r') {
        buffer[index - 1] = '\0';
        index             = 0;
        end_of_cmd        = true;
      }
    } else {
      if (index < BUFFER_SIZE - 1) {
        buffer[index] = (char)c;
        index++;
      }
    }
  }
}

void display_calib_cmd_usage()
{
  printf("Calibration commands usage:\r\n");
  printf("*************************************************************************************************************"
         "**************************************************************\r\n");
  printf("sl_freq_offset=<freq_offset_in_KHz>\r\n");
  printf("sl_calib_write=<target>,<flags>,<gain_offset_low>,<gain_offset_mid>,<gain_offset_high>,<xo_ctune>,<gain_"
         "offset_ch14>\r\n");
  printf("sl_evm_offset=<index>,<evm_offset>\r\n");
  printf("sl_evm_write=<target>,<flags>,<evm_offset_11B>,<evm_offset_11G_36M_54M_11N_MCS3_MCS7>,<evm_offset_11G_6M_24M_"
         "11N_MCS0_MCS2>,<evm_offset_11N_MCS0>,<evm_offset_11N_MCS7>\r\n");
  printf("*************************************************************************************************************"
         "**************************************************************\r\n");
}

void validate_input_cmd()
{
  size_t buffer_len = strlen(buffer);

  if (((strncasecmp((const char *)buffer, (const char *)"sl_", strlen("sl_")) != 0))
      || (buffer_len < MIN_CALIB_COMMAND_LENGTH) || (buffer_len > MAX_CALIB_COMMAND_LENGTH)) {
    cmd_valid = false;
  } else {
    // Mark command as valid if all conditions are met.
    cmd_valid = true;
  }
}

int32_t validate_and_set_cmd_index()
{
  uint8_t i       = 0;
  int32_t cmd_len = 0;

  validate_input_cmd();

  if (!cmd_valid)
    return -1;

  for (i = 0; i < NO_OF_CALIB_COMMANDS; i++) {
    cmd_len = strlen((const char *)calib_commands[i].cmd);
    if (!strncasecmp((const char *)&buffer, (char *)calib_commands[i].cmd, cmd_len)) {
      cmd_index = i;
      cmd_valid = true;
      return cmd_len;
    }
  }

  cmd_valid = false;
  return -1;
}

int16_t parse_cmd(void *address, uint16_t length, uint8_t *value)
{
  uint8_t temp_buff[15];
  uint16_t temp_offset = 0;
  uint16_t offset;

  for (offset = 0; offset < length; offset++) {
    uint16_t index = 0;
    /* Copy the bytes until '.' ',' '\0'  is reached */
    while ((value[temp_offset] != '.') && (value[temp_offset] != ',') && (value[temp_offset] != '\0')) {
      if (index >= (sizeof(temp_buff) - 1))
        break;
      temp_buff[index++] = value[temp_offset++];
    }
    temp_buff[index] = '\0';
    temp_offset++;
    if (length == PARSE_2_BYTES) {
      *((uint16_t *)address) = atoi((const char *)temp_buff);
      break;
    } else if (length == PARSE_4_BYTES) {
      *((uint32_t *)address) = atoi((const char *)temp_buff);
      break;
    } else
      *((uint8_t *)address + offset) = atoi((const char *)temp_buff);
  }

  return temp_offset;
}

sl_status_t calibration_app()
{
  sl_status_t status;
  int32_t cmd_len                            = 0;
  int8_t temp1B                              = 0;
  int32_t temp4B                             = 0;
  int32_t offset                             = 0;
  sl_si91x_calibration_read_t target         = { 0 };
  sl_si91x_calibration_read_t calib_read_pkt = { 0 };
  sl_si91x_calibration_write_t calib_pkt     = { 0 };
  sl_si91x_freq_offset_t freq_calib_pkt      = { 0 };
  sl_si91x_evm_offset_t evm_offset_pkt       = { 0 };
  sl_si91x_evm_write_t evm_write_pkt         = { 0 };
  //sl_si91x_efuse_read_t efuse_read_pkt        = { 0 };
  sl_si91x_get_dpd_calib_data_t dpd_calib_pkt = { 0 };

  target.target      = 1;
  calib_pkt.target   = 1;
  calib_pkt.flags    = 6;
  calib_pkt.xo_ctune = 80;

  freq_calib_pkt.frequency_offset_in_khz = 30;

  evm_offset_pkt.evm_offset_val = 12;
  evm_offset_pkt.evm_index      = 1;

  evm_write_pkt.target                              = 1;
  evm_write_pkt.flags                               = 2;
  evm_write_pkt.evm_offset_11G_6M_24M_11N_MCS0_MCS2 = 10;

  // efuse_read_pkt.efuse_read_addr_offset = 602;
  // efuse_read_pkt.efuse_read_data_len    = 20;

  dpd_calib_pkt.dpd_power_index = 127;

  while (1) {
    display_calib_cmd_usage();

    printf("Enter the calibration command:\r\n");

    while (!end_of_cmd) {
      iostream_rx();
    }
    end_of_cmd = false;

    printf("Command read complete\r\n");

    cmd_len = validate_and_set_cmd_index();
    offset  = cmd_len;
    if (cmd_valid) {
      switch (cmd_index) {
        case FREQ_OFFSET:
          offset += parse_cmd(&temp4B, PARSE_4_BYTES, (uint8_t *)&buffer[offset]);
          freq_calib_pkt.frequency_offset_in_khz = temp4B;
          status                                 = sl_si91x_frequency_offset(&freq_calib_pkt);
          if (status != SL_STATUS_OK) {
            printf("Frequency offset correction failed: 0x%lx\r\n", status);
            //return status;
          } else {
            printf("Frequency offset correction successful\r\n");
          }
          offset = 0;
          temp4B = 0;
          break;
        case CALIB_WRITE:
          if (buffer[offset] != '\0') {
            offset += parse_cmd(&temp1B, PARSE_1_BYTE, (uint8_t *)&buffer[offset]);
            calib_pkt.target = temp1B;
          }
          if (buffer[offset] != '\0') {
            offset += parse_cmd(&temp4B, PARSE_4_BYTES, (uint8_t *)&buffer[offset]);
            calib_pkt.flags = temp4B;
          }
          if (buffer[offset] != '\0') {
            offset += parse_cmd(&temp1B, PARSE_1_BYTE, (uint8_t *)&buffer[offset]);
            calib_pkt.gain_offset[0] = temp1B;
          }
          if (buffer[offset] != '\0') {
            offset += parse_cmd(&temp1B, PARSE_1_BYTE, (uint8_t *)&buffer[offset]);
            calib_pkt.gain_offset[1] = temp1B;
          }
          if (buffer[offset] != '\0') {
            offset += parse_cmd(&temp1B, PARSE_1_BYTE, (uint8_t *)&buffer[offset]);
            calib_pkt.gain_offset[2] = temp1B;
          }
          if (buffer[offset] != '\0') {
            offset += parse_cmd(&temp1B, PARSE_1_BYTE, (uint8_t *)&buffer[offset]);
            calib_pkt.xo_ctune = temp1B;
          }
          if (buffer[offset] != '\0') {
            offset += parse_cmd(&temp1B, PARSE_1_BYTE, (uint8_t *)&buffer[offset]);
            calib_pkt.gain_offset_ch14 = temp1B;
          }
          status = sl_si91x_calibration_write(calib_pkt);
          if (status != SL_STATUS_OK) {
            printf("Calibration data write failed: 0x%lx\r\n", status);
            return status;
          } else {
            printf("Calibration data write successful\r\n");
          }
          target.target = calib_pkt.target;
          status        = sl_si91x_calibration_read(target, &calib_read_pkt);
          if (status != SL_STATUS_OK) {
            printf("Calibration data read failed: 0x%lx\r\n", status);
            return status;
          } else {
            printf("Calibration data read successful\r\n");
            printf(
              "target %d, gain_offset_low:%d, gain_offset_2:%d, gain_offset_3:%d,xo_tune:%d,gain_offset_ch14:%d\r\n",
              calib_read_pkt.target,
              calib_read_pkt.gain_offset[0],
              calib_read_pkt.gain_offset[1],
              calib_read_pkt.gain_offset[2],
              calib_read_pkt.xo_ctune,
              calib_read_pkt.gain_offset_ch14);
          }
          offset = 0;
          temp1B = 0;
          temp4B = 0;
          break;
        case EVM_OFFSET:
          status = sl_si91x_transmit_test_stop();
          if (status != SL_STATUS_OK) {
            printf("Transmit test stop failed: 0x%lx\r\n", status);
            return status;
          } else {
            printf("Transmit test stopped\r\n");
          }
          if (buffer[offset] != '\0') {
            offset += parse_cmd(&temp1B, PARSE_1_BYTE, (uint8_t *)&buffer[offset]);
            evm_offset_pkt.evm_index = temp1B;
          }
          if (buffer[offset] != '\0') {
            offset += parse_cmd(&temp1B, PARSE_1_BYTE, (uint8_t *)&buffer[offset]);
            evm_offset_pkt.evm_offset_val = temp1B;
          }
          status = sl_si91x_evm_offset(&evm_offset_pkt);
          if (status != SL_STATUS_OK) {
            printf("EVM offset correction failed: 0x%lx\r\n", status);
            return status;
          } else {
            printf("EVM offset correction successful\r\n");
          }
          offset = 0;
          temp1B = 0;
          status = sl_si91x_transmit_test_start(&tx_test_info);
          if (status != SL_STATUS_OK) {
            printf("Transmit test start failed: 0x%lx\r\n", status);
            return status;
          } else {
            printf("Transmit test started\r\n");
          }
          break;
        case EVM_WRITE:
          if (buffer[offset] != '\0') {
            offset += parse_cmd(&temp1B, PARSE_1_BYTE, (uint8_t *)&buffer[offset]);
            evm_write_pkt.target = temp1B;
          }
          if (buffer[offset] != '\0') {
            offset += parse_cmd(&temp4B, PARSE_4_BYTES, (uint8_t *)&buffer[offset]);
            evm_write_pkt.flags = temp4B;
          }
          if (buffer[offset] != '\0') {
            offset += parse_cmd(&temp1B, PARSE_1_BYTE, (uint8_t *)&buffer[offset]);
            evm_write_pkt.evm_offset_11B = temp1B;
          }
          if (buffer[offset] != '\0') {
            offset += parse_cmd(&temp1B, PARSE_1_BYTE, (uint8_t *)&buffer[offset]);
            evm_write_pkt.evm_offset_11G_36M_54M_11N_MCS3_MCS7 = temp1B;
          }
          if (buffer[offset] != '\0') {
            offset += parse_cmd(&temp1B, PARSE_1_BYTE, (uint8_t *)&buffer[offset]);
            evm_write_pkt.evm_offset_11G_6M_24M_11N_MCS0_MCS2 = temp1B;
          }
          if (buffer[offset] != '\0') {
            offset += parse_cmd(&temp1B, PARSE_1_BYTE, (uint8_t *)&buffer[offset]);
            evm_write_pkt.evm_offset_11N_MCS0 = temp1B;
          }
          if (buffer[offset] != '\0') {
            offset += parse_cmd(&temp1B, PARSE_1_BYTE, (uint8_t *)&buffer[offset]);
            evm_write_pkt.evm_offset_11N_MCS7 = temp1B;
          }
          status = sl_si91x_evm_write(&evm_write_pkt);
          if (status != SL_STATUS_OK) {
            printf("EVM offset correction failed: 0x%lx\r\n", status);
            return status;
          } else {
            printf("EVM offset correction successful\r\n");
          }
          break;
        case DPD_CALIB_WRITE:
          status = sl_process_dpd_calibration(&dpd_calib_pkt);
          if (status != SL_STATUS_OK) {
            printf("DPD calibration failed: 0x%lx\r\n", status);
            return status;
          } else {
            printf("DPD claibration successful\r\n");
          }
          break;
        default:
          printf("Invalid command\r\n");
          break;
      }
    } else {
      printf("Invalid command\r\n");
    }

    memset(buffer, 0, sizeof(buffer));
  }
}
#define MAX_DPD_TRAINING_CHANNELS 6
uint8_t channel_sel[MAX_DPD_TRAINING_CHANNELS] = { 1, 3, 6, 8, 11, 13 };
sl_status_t sl_process_dpd_calibration(sl_si91x_get_dpd_calib_data_t *dpd_power_inx)
{
  uint8_t i;
  sl_status_t status                     = SL_STATUS_OK;
  sl_si91x_calibration_write_t calib_pkt = { 0 };
  calib_pkt.target                       = 1;
  calib_pkt.flags                        = 256;
  status                                 = sl_si91x_transmit_test_stop();
  if (status != SL_STATUS_OK) {
    printf("Transmit failed to stop %lx\r\n", status);
    return status;
  } else {
    printf("Transmit command stopped\n");
  }

  for (i = 0; i < MAX_DPD_TRAINING_CHANNELS; i++) {
    //! Checking the region code if the channel number is above 11
    //!
    if (!((channel_sel[i] > 11) && (calibration_configuration.region_code < 2))) {
      tx_test_info.channel = channel_sel[i];
      status               = sl_si91x_transmit_test_start(&tx_test_info);
      if (status != SL_STATUS_OK) {
        printf("Transmit failed with channel num %lx\r\n", status);
        return status;
      } else {
        printf("Transmit command started with channel num %x\r\n", channel_sel[i]);
      }
      osDelay(1000);

      status = sl_si91x_transmit_test_stop();
      if (status != SL_STATUS_OK) {
        printf("Transmit failed to stop %lx\r\n", status);
        return status;
      } else {
        printf("Transmit command stopped\n");
      }
      osDelay(1000);
    }
    if (i == MAX_DPD_TRAINING_CHANNELS - 1) {
      status = sl_si91x_dpd_calibration(dpd_power_inx);
      if (status != SL_STATUS_OK) {
        printf("rsi_calibration_dpd_failed %lx\r\n", status);
        return status;
      } else {
        printf("calib-val coellecting\n");
      }
      osDelay(1000);
      status = sl_si91x_calibration_write(calib_pkt);
      if (status != SL_STATUS_OK) {
        printf("rsi_calib_write failed with error %lx\r\n", status);
        return status;
      } else {
        printf("calib-write pass\n");
      }
    } else {
      status = sl_si91x_dpd_calibration(dpd_power_inx);
      if (status != SL_STATUS_OK) {
        printf("rsi_calibration_dpd_failed %lx\r\n", status);
        return status;
      } else {
        printf("calib val collect\n");
      }
    }
    osDelay(1000);
  }
  return status;
}
