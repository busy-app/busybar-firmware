#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// See https://www.kermitproject.org/kpackets.html for some details on Kermit protocol

typedef struct {
    int32_t (*src_file_read)(void* context, uint8_t* buffer, size_t length);
    int32_t (*comms_send)(void* context, const uint8_t* buffer, size_t length);
} kermit_io_t;

typedef struct kermit_t kermit_t;

kermit_t* kermit_alloc(const kermit_io_t* io, void* context);

void kermit_free(kermit_t* kermit);

bool kermit_start(kermit_t* kermit, const uint8_t timeout_seconds);

// Returns true if the session is established and the file transfer is in progress
bool kermit_is_active(kermit_t* kermit);

// Returns true if data was processed successfully, false otherwise.
// If false is returned, state of the session is invalid, and operation should be aborted.
bool kermit_feed_serial_data(kermit_t* kermit, const uint8_t* data, size_t length);

#ifdef __cplusplus
}
#endif
