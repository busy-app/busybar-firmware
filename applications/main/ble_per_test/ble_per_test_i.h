#pragma once

typedef struct BlePerTest BlePerTest;

void ble_per_test_update(
    BlePerTest* instance,
    uint32_t tx_dones,
    uint32_t crc_fail_cnt,
    uint32_t crc_pass_cnt,
    int32_t rssi);
