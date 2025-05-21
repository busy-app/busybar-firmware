#pragma once
#include <cli/cli.h>
#include <cli_worker.h>

typedef struct BLETestApp BLETestApp;

void* ble_test_app_start(CliWorker* worker);
void ble_test_app_stop(void* app_handle);
void ble_test_app_parse_msg(void* app_handle, uint8_t* data, size_t size);
void ble_test_app_send_text(BLETestApp* instance, FuriString* text);
