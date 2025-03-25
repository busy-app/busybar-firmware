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
} kermit_io_t;

typedef struct kermit_t kermit_t;

kermit_t* kermit_alloc(const kermit_io_t* io, void* context);

void kermit_free(kermit_t* kermit);

bool kermit_run(kermit_t* kermit);

bool kermit_is_running(kermit_t* kermit);

int32_t kermit_feed_serial_data(kermit_t* kermit, const uint8_t* data, size_t length);

#ifdef __cplusplus
}
#endif
