#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Kermit protocol */

typedef struct {
    int32_t (*src_file_read)(void* context, uint8_t* buffer, size_t length);
    int32_t (*comms_send)(void* context, const uint8_t* buffer, size_t length);
    int32_t (*comms_receive)(void* context, uint8_t* buffer, size_t length);
} kermit_io_t;

typedef struct kermit_t kermit_t;

kermit_t* kermit_alloc(void* context, const kermit_io_t* io);

void kermit_free(kermit_t* kermit);

bool kermit_run(kermit_t* kermit);

#ifdef __cplusplus
}
#endif
