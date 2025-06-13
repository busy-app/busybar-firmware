#pragma once

#include "update_task.h"

#include <furi_hal.h>
#include <storage/storage.h>
#include <toolbox/update_lib/update_config.h>

#define UPDATE_TASK_NOERR  0
#define UPDATE_TASK_FAILED -1

typedef struct UpdateTask {
    UpdateTaskState state;
    UpdateConfig* config;
    FuriThread* thread;
    Storage* storage;
    File* file;
    updateProgressCb status_change_cb;
    void* status_change_cb_state;
} UpdateTask;

void update_task_set_progress(UpdateTask* update_task, UpdateTaskStage stage, uint8_t progress);
bool update_task_parse_manifest(UpdateTask* update_task);
bool update_task_open_file(UpdateTask* update_task, const FuriString* filename);

int32_t update_task_worker_general(void* context);

#define CHECK_RESULT(x) \
    if(!(x)) {          \
        break;          \
    }
