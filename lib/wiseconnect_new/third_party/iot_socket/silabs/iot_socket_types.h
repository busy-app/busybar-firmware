/*
 * EVALUATION AND USE OF THIS SOFTWARE IS SUBJECT TO THE TERMS AND
 * CONDITIONS OF THE CONTROLLING LICENSE AGREEMENT FOUND AT LICENSE.md
 * IN THIS SDK. IF YOU DO NOT AGREE TO THE LICENSE TERMS AND CONDITIONS,
 * PLEASE RETURN ALL SOURCE FILES TO SILICON LABORATORIES.
 * (c) Copyright 2018, Silicon Laboratories Inc.  All rights reserved.
 */

#pragma once
#include <stdint.h>
#include <stdio.h>

#define IOT_SOCKET_NO_ERROR 0
#define NO_FLAGS            0
#define SIN_ZERO_LEN        8

typedef struct sockaddr_store {
  uint8_t        socket_length;
  uint8_t        socket_family;
  uint8_t        socket_port[2];
  uint32_t       socket_data[3];
#if defined(SLI_SI91X_ENABLE_IPV6)
  uint32_t       socket_data_extended[3];
#endif /* LWIP_IPV6 */
} sockaddr_storage_t ;
