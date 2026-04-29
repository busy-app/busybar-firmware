#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
    uint32_t val;
    uint8_t bytes[4];
} UsbNetworkIpAddress;

typedef struct {
    UsbNetworkIpAddress address;
    UsbNetworkIpAddress netmask;
    UsbNetworkIpAddress gateway;
} UsbNetworkIpConfig;

#ifdef __cplusplus
}
#endif
