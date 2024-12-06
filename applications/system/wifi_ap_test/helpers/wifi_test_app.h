#pragma once
#include <cli/cli.h>
#include <cli_worker.h>

typedef struct WifiTestApp WifiTestApp;

void* wifi_test_app_start(CliWorker* worker);
void wifi_test_app_stop(void* app_handle);
void wifi_test_app_parse_msg(void* app_handle, uint8_t* data, size_t size);
void wifi_test_app_send_text(WifiTestApp* instance, FuriString* text);
