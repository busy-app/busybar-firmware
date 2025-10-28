#include "ble_i.h"
#include "http/ble_http_repeater.h"

#include "ble_system_command.h"

#define TAG "BleAPI"

static void ble_send_message(Ble* instance, BleMessage* message) {
    message->lock = api_lock_alloc_locked();

    instance->current_message = message;
    furi_event_loop_set_custom_event(instance->event_loop, BleEventTypeIncomingMessage);

    api_lock_wait_unlock_and_free(message->lock);
}

bool ble_init(Ble* ble) {
    furi_assert(ble);

    BleServiceState state = ble_get_state(ble);

    bool result = false;
    if(state == BleServiceStateReset) {
        BleMessage msg = {0};
        msg.header.frame_type = BleIntercomFrameTypeRequest;
        msg.header.command = BleCommandInit;
        msg.header.data_size = 0;
        ble_send_message(ble, &msg);
        result = msg.result;
    } else if(state == BleServiceStateReady) {
        ///TODO: possibly this should be done by actually executing command and
        /// if state is Ready then just do nothing and return true;
        /// But for now let's left it as it is
        result = true;
    } else {
        BLE_LOG_W("No init, wrong state: %d", state);
    }

    return result;
}

BleServiceState ble_get_state(Ble* ble) {
    furi_assert(ble);
    size_t msg_size = sizeof(BleMessage) + sizeof(BleServiceState);
    BLE_LOG_D("Alloc BleMessage: %d", msg_size);

    BleMessage* msg = malloc(msg_size);
    msg->header.frame_type = BleIntercomFrameTypeRequest;
    msg->header.command = BleCommandGetState;
    msg->header.data_size = sizeof(BleServiceState);
    msg->header.source = BleIntercomFrameSourceSystem;

    ble_send_message(ble, msg);
    BleServiceState state = msg->result ? *((BleServiceState*)msg->data) : BleServiceStateError;
    free(msg);

    return state;
}

bool ble_start(Ble* ble) {
    furi_assert(ble);
    ble_init(ble);

    BleServiceState state = ble_get_state(ble);

    bool result = false;
    if(state == BleServiceStateReady) {
        BleMessage msg = {0};
        msg.header.frame_type = BleIntercomFrameTypeRequest;
        msg.header.command = BleCommandEnable;
        msg.header.data_size = 0;
        msg.header.source = BleIntercomFrameSourceSystem;

        ble_send_message(ble, &msg);
        result = msg.result;
    } else {
        BLE_LOG_W("No start, wrong state: %d", state);
    }

    return result;
}

bool ble_stop(Ble* ble) {
    furi_assert(ble);
    BleMessage msg = {0};
    msg.header.frame_type = BleIntercomFrameTypeRequest;
    msg.header.command = BleCommandDisable;
    msg.header.data_size = 0;
    msg.header.source = BleIntercomFrameSourceSystem;

    ble_send_message(ble, &msg);
    return msg.result;
}

bool ble_forget(Ble* ble) {
    furi_assert(ble);

    BleServiceState state = ble_get_state(ble);

    BleMessage msg = {0};
    if(state == BleServiceStateReady) {
        msg.header.frame_type = BleIntercomFrameTypeRequest;
        msg.header.command = BleCommandForgetPairing;
        msg.header.data_size = 0;
        msg.header.source = BleIntercomFrameSourceSystem;

        ble_send_message(ble, &msg);
    }
    return msg.result;
}

void ble_uart_tx_data(Ble* ble, BleUartChannel channel, const void* data, const size_t data_size) {
    furi_assert(ble);
    furi_assert(channel < BleUartChannelCount);

    BleServiceIndex index = (channel == BleUartChannelHM10) ? BleServiceIndexHm10Uart :
                                                              BleServiceIndexNordicUart;
    BleServiceObject* service = ble->services[index];

    ble_service_write_data(service, 1, data, data_size);
}

void ble_uart_set_rx_callback(
    Ble* ble,
    BleUartChannel channel,
    BleDataUpdatedCallback rx_cb,
    void* ctx) {
    furi_assert(ble);
    furi_assert(channel < BleUartChannelCount);

    BleServiceIndex index = (channel == BleUartChannelHM10) ? BleServiceIndexHm10Uart :
                                                              BleServiceIndexNordicUart;
    BleServiceObject* service = ble->services[index];
    ble_service_register_update_callback(service, 0, rx_cb, ctx);
}

void ble_uart_set_tx_done_callback(
    Ble* ble,
    BleUartChannel channel,
    BleDataTransmitDoneCallback tx_done_cb,
    void* ctx) {
    furi_assert(ble);
    furi_assert(channel < BleUartChannelCount);

    BleServiceIndex index = (channel == BleUartChannelHM10) ? BleServiceIndexHm10Uart :
                                                              BleServiceIndexNordicUart;
    BleServiceObject* service = ble->services[index];
    ble_service_register_transmission_done_callback(service, 1, tx_done_cb, ctx);
}
