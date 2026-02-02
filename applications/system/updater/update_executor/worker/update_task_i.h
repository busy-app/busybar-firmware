#pragma once

#include "update_task.h"
#include "../../session/session_config.h"

#include <furi_hal.h>
#include <storage/storage.h>
#include <toolbox/update_lib/update_config.h>

#define UPDATE_EXECUTOR_TASK_NOERR  0
#define UPDATE_EXECUTOR_TASK_FAILED -1

struct UpdateExecutorTask {
    UpdateExecutorTaskState state;
    UpdateConfig* config;
    UpdaterSessionConfig session_config;
    FuriThread* thread;
    Storage* storage;
    File* file;
    UpdateExecutorTaskProgressCallback status_change_callback;
    void* status_change_callback_context;
};

void update_executor_task_set_progress(
    UpdateExecutorTask* update_task,
    UpdateExecutorTaskStage stage,
    uint8_t progress);
bool update_executor_task_parse_manifest(UpdateExecutorTask* update_task);
bool update_executor_task_open_file(UpdateExecutorTask* update_task, const FuriString* filename);

int32_t update_executor_task_worker_general(void* context);

#define CHECK_RESULT(x) \
    if(!(x)) {          \
        break;          \
    }
