/**
 * @file intercom.h
 * @brief Chip-to-chip intercommunication via U(S)ART service
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_INTERCOM "intercom"

typedef struct Intercom Intercom;

typedef enum {
    IntercomChannelInput,
    /* Add more channels here as needed */
    IntercomChannelDebug = 15,
    IntercomChannelMax,
} IntercomChannel;

typedef void (*IntercomRxCallback)(const void* data, size_t data_size, void* context);

void intercom_set_rx_callback(
    Intercom* instance,
    IntercomChannel channel,
    IntercomRxCallback callback,
    void* context);

size_t intercom_tx(
    Intercom* instance,
    IntercomChannel channel,
    const void* data,
    size_t data_size,
    uint32_t timeout);

#ifdef __cplusplus
}
#endif
