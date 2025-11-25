#include "ble_i.h"

#include "ble_system_command.h"

#define TAG "BleAPI"

static void ble_send_message(
    Ble* instance,
    const BleSystemCommand command,
    const void* data,
    const size_t data_size,
    void* const output,
    const size_t max_output_size,
    bool* result) {
    furi_mutex_acquire(instance->current_message_lock, FuriWaitForever);
    api_lock_relock(instance->current_message_api_lock);

    const size_t new_msg_size = sizeof(BleIntercomFrameHeader) + data_size + sizeof(bool);
    if(new_msg_size > instance->current_message_size) {
        free(instance->current_message);

        instance->current_message = malloc(new_msg_size);
        furi_check(instance->current_message);
        instance->current_message_size = new_msg_size;
    }

    BleIntercomFrameHeader* header = &instance->current_message->header;
    header->frame_type = BleIntercomFrameTypeRequest;
    header->command = command;
    header->source = BleIntercomFrameSourceSystem;
    header->data_size = data_size;
    if(data_size > 0) memcpy(instance->current_message->data, data, data_size);

    furi_event_loop_set_custom_event(instance->event_loop, BleEventTypeIncomingMessage);

    api_lock_wait_unlock(instance->current_message_api_lock);

    *result = instance->current_message->result;
    if(output && max_output_size > 0) {
        memcpy(output, instance->current_message->data, max_output_size);
    }
    memset(instance->current_message, 0, instance->current_message_size);
    furi_mutex_release(instance->current_message_lock);
}

void ble_set_name(Ble* ble) {
    bool result = false;
    ble_send_message(ble, BleCommandSetDeviceName, NULL, 0, NULL, 0, &result);
}

bool ble_init(Ble* ble) {
    furi_assert(ble);

    BleServiceState state = ble_get_state(ble);

    bool result = false;
    if(state == BleServiceStateReset) {
        ble_send_message(ble, BleCommandInit, NULL, 0, NULL, 0, &result);
        ble_set_name(ble);

    } else if(
        state == BleServiceStateReady || state == BleServiceStateAdvertising ||
        state == BleServiceStateConnected) {
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

    BleServiceState state = BleServiceStateReset;
    uint8_t dummy = 0;
    bool result = false;
    ble_send_message(
        ble, BleCommandGetState, &dummy, sizeof(dummy), &state, sizeof(BleServiceState), &result);

    return state;
}

bool ble_start(Ble* ble) {
    furi_assert(ble);
    ble_init(ble);

    BleServiceState state = ble_get_state(ble);

    bool result = false;
    if(state == BleServiceStateReady) {
        ble_send_message(ble, BleCommandEnable, NULL, 0, NULL, 0, &result);
    } else {
        BLE_LOG_W("No start, wrong state: %d", state);
    }

    return result;
}

bool ble_stop(Ble* ble) {
    furi_assert(ble);
    bool result = false;
    ble_init(ble);

    BLE_LOG_I("ble_stop");
    ble_send_message(ble, BleCommandDisable, NULL, 0, NULL, 0, &result);
    return result;
}

bool ble_forget(Ble* ble) {
    furi_assert(ble);

    BleServiceState state = ble_get_state(ble);

    bool result = false;
    if(state != BleServiceStateError && state != BleServiceStateReset) {
        ble_send_message(ble, BleCommandForgetPairing, NULL, 0, NULL, 0, &result);
    }
    return result;
}

BlePairingState ble_pairing_get_state(Ble* ble) {
    furi_assert(ble);
    BleServiceState state = ble_get_state(ble);

    BlePairingState pairing = BlePairingStateUnkown;
    bool result = false;
    if(state != BleServiceStateError && state != BleServiceStateReset) {
        ble_send_message(ble, BleCommandGetPairing, NULL, 0, &pairing, sizeof(pairing), &result);
    }
    return pairing;
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
