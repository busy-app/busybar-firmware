#include "ble_i.h"
#include "service/uart/ble_uart_characteristic.h"

#define TAG "BleAPI"

bool ble_get_state(Ble* ble, BleState* const output) {
    furi_assert(ble);
    furi_assert(output);

    bool result =
        ble_command_engine_put_command(ble->engine, BleCommandGetStatus, output, sizeof(BleState));
    return result;
}

bool ble_start(Ble* ble) {
    furi_assert(ble);

    bool result = ble_command_engine_put_command(ble->engine, BleCommandEnable, NULL, 0);
    return result;
}

bool ble_stop(Ble* ble) {
    furi_assert(ble);

    bool result = ble_command_engine_put_command(ble->engine, BleCommandDisable, NULL, 0);
    return result;
}

bool ble_forget(Ble* ble) {
    furi_assert(ble);

    bool result = false;
    do {
        BleState state = {0};
        if(!ble_get_state(ble, &state)) break;

        if(state.status == BleServiceStatusConnectable ||
           state.status == BleServiceStatusConnected) {
            result = ble_command_engine_put_command(ble->engine, BleCommandForgetPairing, NULL, 0);
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
    ble_service_register_update_callback(service, BleUartCharacteristicIndexRx, rx_cb, ctx);
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
    ble_service_register_transmission_done_callback(
        service, BleUartCharacteristicIndexTx, tx_done_cb, ctx);
}

void ble_uart_set_session_callback(Ble* ble, BleDataUpdatedCallback session_update_cb, void* ctx) {
    furi_assert(ble);

    BleServiceObject* service = ble->services[BleServiceIndexNordicUart];
    ble_service_register_update_callback(
        service, BleUartCharacteristicIndexSession, session_update_cb, ctx);
}

void ble_uart_session_set_value(Ble* ble, const uint32_t session) {
    furi_assert(ble);

    BleServiceObject* service = ble->services[BleServiceIndexNordicUart];
    ble_service_write_data(service, BleUartCharacteristicIndexSession, &session, sizeof(uint32_t));
}
