#pragma once

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <furi.h>

#define MG_ARCH                     MG_ARCH_CUSTOM
#define MG_TLS                      MG_TLS_MBED
#define MG_MBEDTLS_DEBUG_LEVEL      0 // Set to 4 for verbose mbedtls debug
#define MG_ENABLE_LWIP              1
#define MG_ENABLE_SOCKET            1
#define MG_ENABLE_TCPIP             0
#define MG_ENABLE_TCPIP_DRIVER_INIT 0
#define MG_ENABLE_CUSTOM_MILLIS     1
#define MG_ENABLE_CUSTOM_RANDOM     1
#define MG_ENABLE_CUSTOM_LOG        1
#define MG_ENABLE_LOG               1
#define MG_ENABLE_POSIX_FS          0
#define MG_ENABLE_DIRLIST           1
#define MG_ENABLE_CUSTOM_RANDOM     1

#define MG_DATA_SIZE 32

#define MG_MAX_RECV_SIZE (256 * 1024)
#define MG_IO_SIZE       1460
