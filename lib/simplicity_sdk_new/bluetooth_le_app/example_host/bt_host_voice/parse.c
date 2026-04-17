/***************************************************************************//**
 * @file
 * @brief Parse source file
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <string.h>
#include <stdbool.h>
#include <stddef.h>

// BG stack headers
#include "app_log.h"

// Own header
#include "parse.h"
#include "config.h"

/*******************************************************************************
 * Static Function Declarations
 ******************************************************************************/
static bool parse_address(const char *str, bd_addr *addr);

/***************************************************************************//**
 *  \brief  Initialize adc_sample_rate variable in configuration structure by
 *          data from argument list.
 *  \param[in]  sample rate
 ******************************************************************************/
void init_sample_rate(adc_sample_rate_t sr)
{
  switch (sr) {
    case sr_8k:
      get_config()->adc_sample_rate = sr_8k;
      break;
    case sr_16k:
    default:
      get_config()->adc_sample_rate = sr_16k;
      break;
  }
}

/***************************************************************************//**
 *  \brief  Parse bluetooth address.
 *  \param[in]  data to parse
 *  \param[out] parsed bluetooth address
 *  \return true if success, otherwise false
 ******************************************************************************/
static bool parse_address(const char *str, bd_addr *addr)
{
  int a[6];
  int i;
  i = sscanf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
             &a[5],
             &a[4],
             &a[3],
             &a[2],
             &a[1],
             &a[0]);

  if (i != 6) {
    return false;
  }

  for (i = 0; i < 6; i++) {
    addr->addr[i] = (uint8_t)a[i];
  }

  return true;
}

/***************************************************************************//**
 *  \brief  Print bluetooth address on stdout.
 *  \param[in]  bluetooth address
 ******************************************************************************/
void print_address(bd_addr *addr)
{
  if (app_log_check_level(APP_LOG_LEVEL_DEBUG)) {
    app_log("%02x:%02x:%02x:%02x:%02x:%02x" APP_LOG_NL,
            addr->addr[5],
            addr->addr[4],
            addr->addr[3],
            addr->addr[2],
            addr->addr[1],
            addr->addr[0]);
  }
}

/***************************************************************************//**
 *  \brief  Print help on stdout.
 ******************************************************************************/
void help(void)
{
  app_log("Help:" APP_LOG_NL);
  app_log(NCP_HOST_OPTIONS);
  app_log(APP_LOG_OPTIONS);
  app_log("    -o  Output file name." APP_LOG_NL);
  app_log("        <file_name>      File name, audio data send to stdout by default." APP_LOG_NL);
  app_log("    -a  Remote device bluetooth address." APP_LOG_NL);
  app_log("        <bt_address>     No default bluetooth address." APP_LOG_NL);
  app_log("    -s  ADC sampling rate." APP_LOG_NL);
  app_log("        <8/16>           8 or 16 kHz sampling rate can be used. Default - 16 kHz." APP_LOG_NL);
  app_log("    -x  Enable filtering, default: disabled. When filtering enabled HPF filter is used." APP_LOG_NL);
  app_log("    -e  Disable encoding, default: enabled" APP_LOG_NL);
  app_log("    -t  Disable transfer status, default: enabled" APP_LOG_NL);
  app_log("    -h  Print help message." APP_LOG_NL);
}

/***************************************************************************//**
 *  \brief  Print configuration parameters on stdout.
 ******************************************************************************/
void print_configuration(void)
{
  app_log_debug("Parameters:" APP_LOG_NL);
  if (get_config()->output_to_stdout == true) {
    app_log_debug("  Audio data send to:      stdout" APP_LOG_NL);
  } else {
    app_log_debug("  Audio data store into:   %s" APP_LOG_NL, get_config()->out_file_name);
  }
  app_log_debug("  Remote address:          "); print_address(&get_config()->remote_address);
  app_log_debug("  Audio data notification: %s" APP_LOG_NL, get_config()->audio_data_notification ? "Enabled" : "Disabled");
  app_log_debug("  ADC sample rate:         %d[kHz]" APP_LOG_NL, get_config()->adc_sample_rate);
  app_log_debug("  Filtering:               %s" APP_LOG_NL, get_config()->filter_enabled ? "Enabled" : "Disabled");
  app_log_debug("  Encoding:                %s" APP_LOG_NL, get_config()->encoding_enabled ? "Enabled" : "Disabled");
  app_log_debug("  Transfer status:         %s" APP_LOG_NL, get_config()->transfer_status ? "Enabled" : "Disabled");
  app_log_nl();
  return;
}

/***************************************************************************//**
 *  \brief  Add extension to output file depending on application parameters.
 ******************************************************************************/
static void add_extension_to_file(void)
{
  char *ptr = NULL;

  if (strcmp(get_config()->out_file_name, "-") == 0) {
    get_config()->output_to_stdout = true;
    return;
  }

  size_t ptr_len = 1 + strlen(get_config()->out_file_name) + 4;
  ptr = (char *)malloc(ptr_len);

  if (ptr == NULL) {
    app_log_error("Memory allocation failed. Exiting." APP_LOG_NL);
    exit(EXIT_FAILURE);
  }

  strcpy(ptr, get_config()->out_file_name);

  if (get_config()->encoding_enabled) {
    strcat(ptr, IMA_FILE_EXTENSION);
  } else {
    strcat(ptr, S16_FILE_EXTENSION);
  }

  free(get_config()->out_file_name);
  get_config()->out_file_name = NULL;

  get_config()->out_file_name = malloc(1 + strlen(ptr));
  strcpy(get_config()->out_file_name, ptr);

  free(ptr);
}

/***************************************************************************//**
 *  \brief  Parse application parameters.
 *  \param[in] argc Argument count.
 *  \param[in] argv Buffer contaning application parameters.
 ******************************************************************************/
void parse_arguments(int argc, char **argv)
{
  int opt;
  sl_status_t sc;
  size_t fLen;

  while ((opt = getopt(argc, argv, OPTSTRING)) != -1) {
    switch (opt) {
      case 'o':
        fLen = strlen(optarg) + 1;
        get_config()->out_file_name = malloc(fLen);
        memcpy(get_config()->out_file_name, optarg, fLen);
        break;

      case 'a':
        if (parse_address(optarg, &get_config()->remote_address)) {
          get_config()->remote_address_set = true;
        } else {
          app_log_error("Unable to parse address %s", optarg);
          exit(EXIT_FAILURE);
        }
        break;

      case 's':
        init_sample_rate(atoi(optarg));
        break;

      case 'x':
        get_config()->filter_enabled = true;
        break;

      case 'e':
        get_config()->encoding_enabled = false;
        break;

      case 't':
        get_config()->transfer_status = false;
        break;

      case 'h':
        help();
        exit(EXIT_SUCCESS);

      default:
        sc = ncp_host_set_option((char)opt, optarg);
        if (sc == SL_STATUS_NOT_FOUND) {
          sc = app_log_set_option((char)opt, optarg);
        }
        if (sc != SL_STATUS_OK) {
          app_log(USAGE, argv[0]);
          exit(EXIT_FAILURE);
        }
    }
  }

  add_extension_to_file();
}
