#pragma once

#include "fatfs/ff.h"
#include "fatfs/ff_gen_drv.h"
#include "user_diskio.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Init file system driver */
void fatfs_init(char* path, size_t logical_unit_number);

#ifdef __cplusplus
}
#endif
