#pragma once

#include "intercom.h"
#include "intercom_frame.h"

#include <furi.h>

#include <furi_hal_serial.h>
#include <furi_hal_serial_control.h>
#include <furi_hal_resources.h>

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
    FuriThread* rx_thread;
    FuriSemaphore* sync_semaphore;
    FuriEventLoop* event_loop;
    FuriSemaphore* tx_semaphore;
    FuriEventLoopTimer* tx_timer;
    FuriHalSerialHandle* serial;
    FuriPubSub* pubsub;
    bool is_initial_sync_done;
    bool error_handling_disabled;
    _Atomic bool is_in_sync;
    IntercomChannel handles[IntercomChannelIdMax];
    IntercomFrame tx_frame;
    IntercomFrame rx_frame;
};

// intercom.c:

IntercomFrame* intercom_do_acquire_tx(Intercom* intercom);
void intercom_do_tx(Intercom* intercom);

// intercom_sync.c:

void intercom_sync_request(const GpioPin* gpio);
bool intercom_sync_serial(FuriHalSerialHandle* serial);

// intercom_channel.c:

void intercom_channel_init(IntercomChannel* channel, Intercom* intercom);

void intercom_channel_set_callback(
    IntercomChannel* channel,
    IntercomRxCallback callback,
    void* context);
void intercom_channel_call_callback(IntercomChannel* channel, const IntercomFrame* rx_frame);

void intercom_channel_send_ready(IntercomChannel* channel);
bool intercom_channel_await_peer_ready(IntercomChannel* channel, FuriWait timeout);
