/*
 *  Copyright (c) 2018, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#define _GNU_SOURCE
#include <stdint.h>
#include <stdbool.h>

#include "openthread-posix-config.h"

#include "openthread/config.h"

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <limits.h>
#ifdef __linux__
#include <sys/prctl.h>
#endif

#ifndef HAVE_LIBEDIT
#define HAVE_LIBEDIT 0
#endif

#ifndef HAVE_LIBREADLINE
#define HAVE_LIBREADLINE 0
#endif

#include "openthread/platform/radio.h"

#include "common/code_utils.hpp"
#include "common/logging.hpp"
#include "lib/platform/exit_code.h"
#include "openthread/openthread-system.h"
#include "sl_zigbee_types.h"
#include "serial_adapter.h"

#include "stack/include/sl_zigbee_types.h"

#ifndef OPENTHREAD_ENABLE_COVERAGE
#define OPENTHREAD_ENABLE_COVERAGE 0
#endif

#ifndef UNUSED_VAR
#define UNUSED_VAR(x) (void)(x)
#endif

typedef struct PosixConfig{
  otPlatformConfig mPlatformConfig;      ///< Platform configuration.
  otLogLevel       mLogLevel;            ///< Debug level of logging.
  bool             mIsVerbose;           ///< Whether to print log to stderr.
} PosixConfig;

extern jmp_buf gResetJump;

#define RADIO_URL_MAX_LEN 150
char radioUrl[RADIO_URL_MAX_LEN];

#define DEFAULT_CONF_PATH "/usr/local/etc/zigbeed.conf"
char confFilePath[PATH_MAX] = DEFAULT_CONF_PATH;

/**
 * This enumeration defines the argument return values.
 *
 */
enum {
  OT_POSIX_OPT_RADIO_URL      = 'r',
  OT_POSIX_OPT_DEBUG_LEVEL    = 'd',
  OT_POSIX_OPT_EZSP_INTERFACE = 'p',
  OT_POSIX_OPT_HELP           = 'h',
  OT_POSIX_OPT_VERBOSE        = 'v',
  OT_POSIX_OPT_CONF           = 'c',
};

#define GETOPT_OPTION_STRING "r:d:p:hv:c:"
static const struct option kOptions[] = {
  { "radio-url", required_argument, NULL, OT_POSIX_OPT_RADIO_URL },
  { "debug-level", required_argument, NULL, OT_POSIX_OPT_DEBUG_LEVEL },
  { "ezsp-interface", required_argument, NULL, OT_POSIX_OPT_EZSP_INTERFACE },
  { "help", no_argument, NULL, OT_POSIX_OPT_HELP },
  { "verbose", required_argument, NULL, OT_POSIX_OPT_VERBOSE },
  { "conf", required_argument, NULL, OT_POSIX_OPT_CONF },
  { 0, 0, 0, 0 }
};

static void PrintUsage(const char *aProgramName, FILE *aStream, int aExitCode)
{
  fprintf(aStream,
          "Syntax:\n"
          "    %s [Options]\n"
          "Options:\n"
          "    -r  --radio-url <RadioURL>    URL to the radio device. Must include iid=n with n=1,2, or 3.\n"
          "    -d  --debug-level <level>     Debug level for Spinel syslog logging.\n"
          "    -p  --ezsp-interface <name>   EZSP interface name.\n"
          "    -h  --help                    Display this usage information.\n"
          "    -v  --verbose <value>         Also log Spinel to stderr, value: 1=enable; 0=disable.\n"
          "    -c  --conf <path>             Path to the configuration file.\n",
          aProgramName);
  exit(aExitCode);
}

// these're consulted from cpc - config.c
static inline bool is_nul(char c)
{
  return c == '\0';
}

static inline bool is_white_space(char c)
{
  return c == ' ' || c == '\t';
}

static inline bool is_line_break(char c)
{
  return c == '\n' || c == '\r';
}

static inline bool is_comment(char c)
{
  return c == '#';
}

static int32_t non_leading_whitespaces_index(const char *str)
{
  int32_t i = 0;
  while (!is_nul(str[i])) {
    if (!is_white_space(str[i])) {
      break;
    }
    ++i;
  }
  return i;
}

static bool is_comment_or_newline(const char* line)
{
  char c = line[non_leading_whitespaces_index(line)];
  return is_nul(c) || is_line_break(c) || is_comment(c);
}

// returns number of arguments read. Negative numbers are errors.
static ssize_t ParseEtcConf(char *confFilePath, int aArgVectorSize, char *aArgVector[])
{
  UNUSED_VAR(aArgVectorSize);
  ssize_t argCount = 0;
  FILE *fp = fopen(confFilePath, "r");
  if (fp == NULL) {
    fprintf(stderr, "Cannot open file %s\n", confFilePath);
    return 0; // no config file
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t read = 0;
  char key[128] = { 0 };
  char val[128] = { 0 };
  while ((read = getline(&line, &len, fp)) != -1) {
    if (is_comment_or_newline(line)) { // skip comment or invalid line
      continue;
    }

    // Extract 'key: value' pair
    if (sscanf(line, "%127[^: ]: %127[^\r\n #]%*c", key, val) != 2) {
      fprintf(stderr, "Config file line \"%s\" doesn't respect syntax. Expecting YAML format (key: value). Please refer to the provided zigbeed.conf", line);
    } else {
      // This is to make sure both key and val are not null-terminated strings
      key[127] = '\0';
      val[127] = '\0';

      aArgVector[argCount] = malloc(sizeof(char) * (strlen(key) + 2 + 1)); // make room for "--" & null terminated character.
      if (aArgVector[argCount] == NULL) {
        return ENOMEM;
      }
      sprintf(aArgVector[argCount], key[1] == '\0' ? "-%s" : "--%s", key); // add "--" to argument
      argCount++;

      aArgVector[argCount] = malloc(sizeof(char) * (strlen(val) + 1)); // make room for null terminated character.
      if (aArgVector[argCount] == NULL) {
        return ENOMEM;
      }
      strncpy(aArgVector[argCount], val, strlen(val));
      argCount++;
    }
  }

  fclose(fp);
  if (line) {
    free(line);
  }

  return argCount;
}

static void ParseArg(int aArgCount, char *aArgVector[], PosixConfig *aConfig)
{
  optind = 1;

  while (true) {
    int index  = 0;
    int option = getopt_long(aArgCount, aArgVector,
                             GETOPT_OPTION_STRING,
                             kOptions, &index);

    if (option == -1) {
      break;
    }

    switch (option) {
      case OT_POSIX_OPT_DEBUG_LEVEL:
        aConfig->mLogLevel = (otLogLevel)atoi(optarg);
        break;
      case OT_POSIX_OPT_HELP:
        PrintUsage(aArgVector[0], stdout, OT_EXIT_SUCCESS);
        break;
      case OT_POSIX_OPT_EZSP_INTERFACE:
        if (strlen(optarg) > SERIAL_PORT_NAME_MAX_LEN) {
          fprintf(stderr, "Error: ezsp-interface name too long\n");
          exit(OT_EXIT_INVALID_ARGUMENTS);
        }
        strcpy(serialPort, optarg);
        break;
      case OT_POSIX_OPT_RADIO_URL:
        if (strlen(optarg) > RADIO_URL_MAX_LEN) {
          fprintf(stderr, "Error: Radio URL too long\n");
          exit(OT_EXIT_INVALID_ARGUMENTS);
        }
        strcpy(radioUrl, optarg);
        aConfig->mPlatformConfig.mCoprocessorUrls.mUrls[aConfig->mPlatformConfig.mCoprocessorUrls.mNum++] = radioUrl;
        break;
      case OT_POSIX_OPT_VERBOSE:
        aConfig->mIsVerbose = (bool)atoi(optarg);
        break;
      case OT_POSIX_OPT_CONF:
        strncpy(confFilePath, optarg, sizeof(confFilePath) - 1);
        confFilePath[sizeof(confFilePath) - 1] = '\0';
        break;
      case '?':
        fprintf(stderr, "Error: Unrecognized option (%s)\n", optarg);
        PrintUsage(aArgVector[0], stderr, OT_EXIT_INVALID_ARGUMENTS);
        break;
      default:
        assert(false);
        break;
    }
  }
}

void app_process_args(int argc, char *argv[])
{
  PosixConfig config = { 0 };
  config.mPlatformConfig.mSpeedUpFactor = 1;
  config.mLogLevel                      = OT_LOG_LEVEL_CRIT;

  int confArgc = 30;
  char *confArgv[confArgc + 1];
  for (int i = confArgc + 1; i; i--) {
    confArgv[i] = NULL;
  }
  confArgv[0] = argv[0];
  confArgc = ParseEtcConf(confFilePath, confArgc, confArgv + 1);
  if (confArgc < 0) {
    PrintUsage(argv[0], stderr, OT_EXIT_INVALID_ARGUMENTS);
  }
  if (confArgc == ENOMEM) {
    fprintf(stderr, "Cannot allocate memory for arguments. \n");
  }
  ParseArg(confArgc + 1, confArgv, &config);
  for (int i = confArgc + 1; i; i--) {
    free(confArgv[i]);
  }

  ParseArg(argc, argv, &config);

  if (config.mPlatformConfig.mCoprocessorUrls.mNum == 0) {
    fprintf(stderr, "RadioURL is undefined.\n");
    PrintUsage(argv[0], stderr, OT_EXIT_INVALID_ARGUMENTS);
  }

  openlog("zigbeed", LOG_PID | (config.mIsVerbose ? LOG_PERROR : 0), LOG_DAEMON);
  setlogmask(setlogmask(0) & LOG_UPTO(LOG_DEBUG));
  // Sleep added for successful restart of zigbeed when Host application resets
  sleep(1);
  otInstance *instance = otSysInit(&config.mPlatformConfig);
  IgnoreError(otLoggingSetLevel(config.mLogLevel));
  syslog(LOG_INFO, "Zigbeed started");
  syslog(LOG_INFO, "RCP version: %s", otPlatRadioGetVersionString(instance));
  syslog(LOG_INFO, "Zigbeed Version: GSDK %d.%d.%d - %s - %s",
         sl_zigbee_version.major,
         sl_zigbee_version.minor,
         sl_zigbee_version.patch,
         sl_zigbee_stack_build_date_time.date,
         sl_zigbee_stack_build_date_time.time);
}
