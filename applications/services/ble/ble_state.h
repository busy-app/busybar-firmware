#pragma once

#include <furi.h>

typedef enum {
    BleServiceStateReset, /*Service was just created. Will move to BleServiceStateInitialization when it will create all inner objects*/
    BleServiceStateInitialization, /* Service performs initialization sequence for all inner ble services. 
    U5 also sends init data to 917 to help him create its services */
    BleServiceStateReady, /*All init sequences are done. All inner services configured, and both u5 and 917 ready to work. But ble still disabled*/
    BleServiceStateAdvertising, /*User enabled ble, device start advertising.*/
    BleServiceStateConnected, /*Remote device connected to bsb over ble*/
    BleServiceStateError, /*Error occured.*/

    BleServiceStateCount, /*Total amount of states. Used in some cyclic operations*/
} BleServiceState;

typedef void (*BleDataUpdatedCallback)(size_t data_size, void* data, void* context);
typedef void (*BleDataTransmitDoneCallback)(void* context);
