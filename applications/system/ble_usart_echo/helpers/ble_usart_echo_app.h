#pragma once
#include <cli/cli.h>
#include <cli_worker.h>

typedef struct BLEUsartEchoApp BLEUsartEchoApp;

void* ble_usart_echo_app_start(CliWorker* worker);
void ble_usart_echo_app_stop(void* app_handle);
void ble_usart_echo_app_parse_msg(void* app_handle, uint8_t* data, size_t size);
void ble_usart_echo_app_send_text(BLEUsartEchoApp* instance, FuriString* text);
