#pragma once

#include "intercom.h"
#include "intercom_frame.h"

#include <furi.h>

#include <furi_hal_serial.h>
#include <furi_hal_serial_control.h>

#define INTERCOM_TX_TIMEOUT_MS                 (1000UL)
#define INTERCOM_INITIAL_SYNC_RETRY_LOCKOUT_MS (500UL)
#define INTERCOM_INITIAL_SYNC_TIMEOUT_MS       (10000UL)

#ifdef INTERCOM_DEBUG
#define INTERCOM_LOG_D(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define INTERCOM_LOG_D(...)
#endif

#ifndef INTERCOM_BAUD_RATE
#define INTERCOM_BAUD_RATE (11250000UL)
#endif

#if defined(BSB_MCU_U5)
#define INTERCOM_SERIAL FuriHalSerialIdUsart1
#elif defined(BSB_MCU_SI917)
#define INTERCOM_SERIAL FuriHalSerialIdUsart0
#else
#error "Unsupported MCU"
#endif

#define INTERCOM_MAGIC_DELAY (100UL)

typedef enum {
    IntercomCustomEventSyncRequested = 1UL << 0,
    IntercomCustomEventFrameSent = 1UL << 1,
    IntercomCustomEventDataAvailable = 1UL << 2,
} IntercomCustomEvent;

/**
 * @brief Flags for `IntercomChannel` `flags` field
 */
typedef enum {
    IntercomChannelFlagPeerReady = (1 << 0), //<! Same channel on other chip is ready to receive
} IntercomChannelFlag;

struct IntercomChannel {
    Intercom* intercom;
    FuriEventFlag* flags;
    IntercomRxCallback rx_callback;
    void* callback_context;
};

struct Intercom {
    FuriEventLoop* event_loop;
    FuriSemaphore* tx_semaphore;
    FuriEventLoopTimer* tx_timer;
    FuriHalSerialHandle* serial;
    FuriPubSub* pubsub;
    bool error_handling_disabled;
    _Atomic bool is_in_sync;
    IntercomChannel handles[IntercomChannelIdMax];
    IntercomFrame tx_frame;
    IntercomFrame rx_frame;
};

// intercom.c:

IntercomFrame* intercom_do_acquire_tx(Intercom* intercom);
void intercom_do_tx(Intercom* intercom);
void intercom_dump_frame(const IntercomFrame* frame);

// intercom_rx.c:

void intercom_start_rx_thread(Intercom* instance);

// intercom_sync.c:

bool intercom_sync_serial(FuriHalSerialHandle* serial);

// intercom_channel.c:

void intercom_channel_init(IntercomChannel* channel, Intercom* intercom);

void intercom_channel_set_callback(
    IntercomChannel* channel,
    IntercomRxCallback callback,
    void* context);
void intercom_channel_call_callback(const IntercomChannel* channel, const IntercomFrame* rx_frame);

void intercom_channel_send_ready(IntercomChannel* channel);
