#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include <core/string.h>

#define UPDATE_EXECUTOR_DELAY_OPERATION_OK    10
#define UPDATE_EXECUTOR_DELAY_OPERATION_ERROR INT_MAX

typedef enum {
    UpdateExecutorTaskStageProgress = 0,

    UpdateExecutorTaskStageReadManifest,

    UpdateExecutorTaskStageValidateDFUImage,
    UpdateExecutorTaskStageFlashWrite,
    UpdateExecutorTaskStageFlashValidate,

    UpdateExecutorTaskStage917RadioWrite,
    UpdateExecutorTaskStage917RadioInstall,

    UpdateExecutorTaskStage917Write,
    UpdateExecutorTaskStage917Install,

    UpdateExecutorTaskStageResourcesFileCleanup,
    UpdateExecutorTaskStageResourcesDirCleanup,
    UpdateExecutorTaskStageResourcesFileUnpack,

    UpdateExecutorTaskStageCompleted,
    UpdateExecutorTaskStageError,
    UpdateExecutorTaskStageMAX
} UpdateExecutorTaskStage;

static inline bool update_executor_task_stage_is_error(const UpdateExecutorTaskStage stage) {
    return stage >= UpdateExecutorTaskStageError;
}

typedef enum {
    UpdateExecutorTaskStageGroupMisc = 0,
    UpdateExecutorTaskStageGroupPrepare = 1 << 1,
    UpdateExecutorTaskStageGroupFirmware = 1 << 2,
    UpdateExecutorTaskStageGroup917Radio = 1 << 3,
    UpdateExecutorTaskStageGroup917 = 1 << 4,
    UpdateExecutorTaskStageGroupResources = 1 << 5,
} UpdateExecutorTaskStageGroup;

typedef struct {
    UpdateExecutorTaskStage stage;
    uint8_t overall_progress, stage_progress;
    FuriString* status;
    UpdateExecutorTaskStageGroup groups;
    uint32_t total_progress_points;
    uint32_t completed_stages_points;
} UpdateExecutorTaskState;

typedef struct UpdateExecutorTask UpdateExecutorTask;

typedef void (*UpdateExecutorTaskProgressCallback)(
    const char* status,
    UpdateExecutorTaskStage stage,
    uint8_t percent,
    void* context);

UpdateExecutorTask* update_executor_task_alloc(void);

void update_executor_task_free(UpdateExecutorTask* update_task);

void update_executor_task_set_progress_callback(
    UpdateExecutorTask* update_task,
    UpdateExecutorTaskProgressCallback callback,
    void* context);

void update_executor_task_start(UpdateExecutorTask* update_task);

bool update_executor_task_is_running(UpdateExecutorTask* update_task);

const UpdateExecutorTaskState* update_executor_task_get_state(UpdateExecutorTask* update_task);

// const UpdateManifest* update_executor_task_get_manifest(UpdateExecutorTask* update_task);

#ifdef __cplusplus
}
#endif
