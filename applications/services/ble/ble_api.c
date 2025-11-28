#include "ble_i.h"

#include "ble_system_command.h"

#define TAG "BleAPI"

static void ble_send_message(
    Ble* instance,
    const BleSystemCommand command,
    void* data,
    size_t data_size,
    bool* result) {
    furi_mutex_acquire(instance->current_command_lock, FuriWaitForever);
    api_lock_relock(instance->current_command_api_lock);

    const size_t new_msg_size = sizeof(BleIntercomFrameHeader) + data_size + sizeof(bool);
    if(new_msg_size > instance->current_command_size) {
        free(instance->current_command);

        instance->current_command = malloc(new_msg_size);
        furi_check(instance->current_command);
        instance->current_command_size = new_msg_size;
    }

    BleIntercomFrameHeader* header = &instance->current_command->header;
    header->frame_type = BleIntercomFrameTypeRequest;
    header->command = command;
    header->source = BleIntercomFrameSourceSystem;
    header->data_size = data_size;
    if(data_size > 0) memcpy(instance->current_command->data, data, data_size);

    furi_event_loop_set_custom_event(instance->event_loop, BleEventTypeIncomingMessage);

    api_lock_wait_unlock(instance->current_command_api_lock);

    *result = instance->current_command->result;
    if(data && data_size > 0) {
        memcpy(data, instance->current_command->data, data_size);
    }
    memset(instance->current_command, 0, instance->current_command_size);
    furi_mutex_release(instance->current_command_lock);
}

bool ble_init(Ble* ble) {
    furi_assert(ble);

    bool result = false;
    ble_send_message(ble, BleCommandInit, NULL, 0, &result);

    return result;
}

bool ble_get_status(Ble* ble, BleStatus* const output) {
    furi_assert(ble);
    furi_assert(output);

    bool result = false;
    ble_send_message(ble, BleCommandGetStatus, output, sizeof(BleStatus), &result);

    return result;
}

bool ble_start(Ble* ble) {
    furi_assert(ble);

    bool result = false;
    do {
        if(!ble_init(ble)) break;

        ble_send_message(ble, BleCommandEnable, NULL, 0, &result);
    } while(false);

    return result;
}

bool ble_stop(Ble* ble) {
    furi_assert(ble);

    bool result = false;
    do {
        if(!ble_init(ble)) break;

        ble_send_message(ble, BleCommandDisable, NULL, 0, &result);
    } while(false);

    return result;
}

bool ble_forget(Ble* ble) {
    furi_assert(ble);

    bool result = false;
    do {
        BleStatus status = {0};
        if(!ble_get_status(ble, &status)) break;

        if(status.state != BleServiceStateError && status.state != BleServiceStateReset) {
            ble_send_message(ble, BleCommandForgetPairing, NULL, 0, &result);
        }
    } while(false);

    return result;
}

FuriPubSub* ble_get_pubsub(Ble* ble) {
    furi_assert(ble);
    return ble->on_status_change;
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
