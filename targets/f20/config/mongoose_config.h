#pragma once

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <furi.h>

#define MG_ARCH                     MG_ARCH_CUSTOM
#define MG_ENABLE_LWIP              1
#define MG_ENABLE_TCPIP_DRIVER_INIT 0
#define MG_ENABLE_CUSTOM_MILLIS     1
#define MG_ENABLE_CUSTOM_LOG        1
#define MG_ENABLE_LOG               1
#define MG_ENABLE_POSIX_FS          0
#define MG_ENABLE_DIRLIST           1
#define MG_DATA_SIZE                32

#define MG_IO_SIZE 512
