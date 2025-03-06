#pragma once
#include <cli/cli.h>
#include <cli_worker.h>

typedef struct CryptoTestApp CryptoTestApp;

void* crypto_test_app_start(CliWorker* worker);
void crypto_test_app_stop(void* app_handle);
void crypto_test_app_parse_msg(void* app_handle, uint8_t* data, size_t size);
void crypto_test_app_send_text(CryptoTestApp* instance, FuriString* text);
