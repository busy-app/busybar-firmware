#include "ble_advertise_config.h"

const BleAdvertiseConfig advertise_config = {
    .flags =
        {
            .header = {.length = 2, .type = 1},
            .data = 6,
        },
    .appearance =
        {
            .header = {.type = 0x19, .length = 3},
            .data = 0x0880, //0x00C0,
        },
    .local_name =
        {
            .header = {.type = 0x9, .length = sizeof(BLE_LOCAL_NAME) + 1},
            .data = BLE_LOCAL_NAME,
        },
    .manufacturer =
        {
            .header = {.type = 0xFF, .length = 3},
            .data = 0x0E29,
        },
    .service_class =
        {
            .header = {.type = 0x02, .length = 3},
            .data = 0x308A,
        },
};

static_assert(sizeof(advertise_config) <= BLE_ADVERTISE_PACKET_MAX_SIZE);
//==========================================================
