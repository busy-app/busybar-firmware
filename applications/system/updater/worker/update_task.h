#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include <core/string.h>

#define UPDATE_DELAY_OPERATION_OK    10
#define UPDATE_DELAY_OPERATION_ERROR INT_MAX

typedef enum {
    UpdateTaskStageProgress = 0,

    UpdateTaskStageReadManifest,

    UpdateTaskStageValidateDFUImage,
    UpdateTaskStageFlashWrite,
    UpdateTaskStageFlashValidate,

    UpdateTaskStage917RadioWrite,
    UpdateTaskStage917RadioInstall,

    UpdateTaskStage917Write,
    UpdateTaskStage917Install,

    UpdateTaskStageResourcesFileCleanup,
    UpdateTaskStageResourcesDirCleanup,
    UpdateTaskStageResourcesFileUnpack,

    UpdateTaskStageCompleted,
    UpdateTaskStageError,
    UpdateTaskStageMAX
} UpdateTaskStage;

inline bool update_stage_is_error(const UpdateTaskStage stage) {
    return stage >= UpdateTaskStageError;
}

typedef enum {
    UpdateTaskStageGroupMisc = 0,
    UpdateTaskStageGroupFirmware = 1 << 1,
    UpdateTaskStageGroup917Radio = 1 << 2,
    UpdateTaskStageGroup917 = 1 << 3,
    UpdateTaskStageGroupResources = 1 << 4,
} UpdateTaskStageGroup;

typedef struct {
    UpdateTaskStage stage;
    uint8_t overall_progress, stage_progress;
    FuriString* status;
    UpdateTaskStageGroup groups;
    uint32_t total_progress_points;
    uint32_t completed_stages_points;
} UpdateTaskState;

typedef struct UpdateTask UpdateTask;

typedef void (
    *updateProgressCb)(const char* status, const uint8_t stage_pct, bool failed, void* state);

UpdateTask* update_task_alloc(void);

void update_task_free(UpdateTask* update_task);

void update_task_set_progress_cb(UpdateTask* update_task, updateProgressCb cb, void* state);

void update_task_start(UpdateTask* update_task);

bool update_task_is_running(UpdateTask* update_task);

const UpdateTaskState* update_task_get_state(UpdateTask* update_task);

// const UpdateManifest* update_task_get_manifest(UpdateTask* update_task);

#ifdef __cplusplus
}
#endif
