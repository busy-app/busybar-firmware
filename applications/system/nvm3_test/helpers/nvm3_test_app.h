#pragma once
#include <cli/cli.h>
#include <cli_worker.h>

typedef struct NVM3TestApp NVM3TestApp;

void* nvm3_test_app_start(CliWorker* worker);
void nvm3_test_app_stop(void* app_handle);
void nvm3_test_app_parse_msg(void* app_handle, uint8_t* data, size_t size);
void nvm3_test_app_send_text(NVM3TestApp* instance, FuriString* text);
