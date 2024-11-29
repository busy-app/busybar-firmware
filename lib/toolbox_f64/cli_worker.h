#pragma once
#include <cli/cli.h>

typedef struct CliWorker CliWorker;

typedef void* (*CliWorkerAppStartCallback)(CliWorker* worker_handle);
typedef void (*CliWorkerAppProcessCallback)(void* app_handle, uint8_t* data, size_t size);
typedef void (*CliWorkerAppExitCallback)(void* app_handle);

CliWorker* cli_worker_alloc(const char* workspace_name, Cli* cli);
void cli_worker_free(CliWorker* instance);
bool cli_worker_start(CliWorker* instance);
void cli_worker_run(CliWorker* instance);
void cli_worker_stop(CliWorker* instance);
bool cli_worker_is_running(CliWorker* instance);

size_t cli_worker_add_rx_data(CliWorker* instance, uint8_t* data, size_t size);

void cli_worker_set_callback(
    CliWorker* instance,
    CliWorkerAppStartCallback app_start_callback,
    CliWorkerAppProcessCallback app_process_callback,
    CliWorkerAppExitCallback app_exit_callback);
