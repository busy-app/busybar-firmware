#pragma once

#include "sl_matter_config.h"

#define CHIP_DEVICE_CONFIG_THREAD_FTD           0
#define CHIP_DEVICE_CONFIG_ENABLE_WPA           0
#define CHIP_WITH_GIO                           0
#define OPENTHREAD_CONFIG_ENABLE_TOBLE          0
#define CHIP_STACK_LOCK_TRACKING_ENABLED        0
#define CHIP_STACK_LOCK_TRACKING_ERROR_FATAL    0
#define CHIP_DISABLE_PLATFORM_KVS               0
#define CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR 0
//#define CHIP_DEVICE_PROJECT_CONFIG_INCLUDE      <CHIPProjectConfig.h>
#define CHIP_DEVICE_PLATFORM_CONFIG_INCLUDE     <platform/bsb/CHIPDevicePlatformConfig.h>
// #define CHIP_DEVICE_LAYER_TARGET_EFR32          1
#define CHIP_DEVICE_LAYER_TARGET                bsb

#define CHIP_USE_TRANSITIONAL_COMMISSIONABLE_DATA_PROVIDER 0
