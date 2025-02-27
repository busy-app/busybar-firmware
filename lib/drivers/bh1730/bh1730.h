#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FuriHalI2cBusHandle FuriHalI2cBusHandle;

bool bh1730_init(FuriHalI2cBusHandle* handle);

bool bh1730_read_lux(FuriHalI2cBusHandle* handle, float* value);

#ifdef __cplusplus
}
#endif
