#include "update_task.h"
#include "update_task_i.h"

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_version.h>
#include <storage/storage.h>
#include <toolbox/path.h>

#define TAG "UpdExecWorker"

static const char* update_task_stage_descr[] = {
    [UpdateExecutorTaskStageProgress] = "...",
    [UpdateExecutorTaskStageReadManifest] = "Loading update manifest",
    [UpdateExecutorTaskStageValidateDFUImage] = "Checking DFU file",
    [UpdateExecutorTaskStageFlashWrite] = "Writing flash",
    [UpdateExecutorTaskStageFlashValidate] = "Validating flash",
    [UpdateExecutorTaskStage917Write] = "Writing 917 FW",
    [UpdateExecutorTaskStage917Install] = "Installing 917 FW",
    [UpdateExecutorTaskStage917RadioWrite] = "Writing 917 Radio FW",
    [UpdateExecutorTaskStage917RadioInstall] = "Installing 917 Radio FW",
    [UpdateExecutorTaskStageResourcesFileCleanup] = "Cleaning up files",
    [UpdateExecutorTaskStageResourcesDirCleanup] = "Cleaning up folders",
    [UpdateExecutorTaskStageResourcesFileUnpack] = "Extracting resources",
    [UpdateExecutorTaskStageCompleted] = "Restarting...",
    [UpdateExecutorTaskStageError] = "Error",
};

static const struct {
    UpdateExecutorTaskStage stage;
    uint8_t percent_min, percent_max;
    const char* descr;
} update_task_error_detail[] = {
    {
        .stage = UpdateExecutorTaskStageReadManifest,
        .percent_min = 0,
        .percent_max = 13,
        .descr = "Wrong Updater HW",
    },
    {
        .stage = UpdateExecutorTaskStageReadManifest,
        .percent_min = 14,
        .percent_max = 20,
        .descr = "Manifest pointer error",
    },
    {
        .stage = UpdateExecutorTaskStageReadManifest,
        .percent_min = 21,
        .percent_max = 30,
        .descr = "Manifest load error",
    },
    {
        .stage = UpdateExecutorTaskStageReadManifest,
        .percent_min = 31,
        .percent_max = 40,
        .descr = "Wrong package version",
    },
    {
        .stage = UpdateExecutorTaskStageReadManifest,
        .percent_min = 41,
        .percent_max = 50,
        .descr = "HW Target mismatch",
    },
    {
        .stage = UpdateExecutorTaskStageReadManifest,
        .percent_min = 51,
        .percent_max = 60,
        .descr = "No DFU file",
    },
    {
        .stage = UpdateExecutorTaskStageReadManifest,
        .percent_min = 61,
        .percent_max = 80,
        .descr = "No Radio file",
    },
    {
        .stage = UpdateExecutorTaskStage917Write,
        .percent_min = 0,
        .percent_max = 100,
        .descr = "917 FW write: error",
    },
    {
        .stage = UpdateExecutorTaskStage917Install,
        .percent_min = 0,
        .percent_max = 10,
        .descr = "917 FW write: error",
    },
    {
        .stage = UpdateExecutorTaskStage917RadioWrite,
        .percent_min = 11,
        .percent_max = 100,
        .descr = "Stack install: wait failed",
    },
    {
        .stage = UpdateExecutorTaskStage917RadioInstall,
        .percent_min = 0,
        .percent_max = 10,
        .descr = "Failed to start C2",
    },
    {
        .stage = UpdateExecutorTaskStageValidateDFUImage,
        .percent_min = 0,
        .percent_max = 1,
        .descr = "Failed to open DFU file",
    },
    {
        .stage = UpdateExecutorTaskStageValidateDFUImage,
        .percent_min = 1,
        .percent_max = 97,
        .descr = "DFU file read error",
    },
    {
        .stage = UpdateExecutorTaskStageValidateDFUImage,
        .percent_min = 98,
        .percent_max = 100,
        .descr = "DFU file CRC mismatch",
    },
    {
        .stage = UpdateExecutorTaskStageFlashWrite,
        .percent_min = 0,
        .percent_max = 100,
        .descr = "Flash write error",
    },
    {
        .stage = UpdateExecutorTaskStageFlashValidate,
        .percent_min = 0,
        .percent_max = 100,
        .descr = "Flash compare error",
    },
    {
        .stage = UpdateExecutorTaskStageResourcesFileCleanup,
        .percent_min = 0,
        .percent_max = 100,
        .descr = "SD I/O error",
    },
    {
        .stage = UpdateExecutorTaskStageResourcesDirCleanup,
        .percent_min = 0,
        .percent_max = 100,
        .descr = "SD I/O error",
    },
    {
        .stage = UpdateExecutorTaskStageResourcesFileUnpack,
        .percent_min = 0,
        .percent_max = 100,
        .descr = "SD I/O error",
    },
};

static const char* update_task_get_error_message(UpdateExecutorTaskStage stage, uint8_t percent) {
    for(size_t i = 0; i < COUNT_OF(update_task_error_detail); i++) {
        if(update_task_error_detail[i].stage == stage &&
           percent >= update_task_error_detail[i].percent_min &&
           percent <= update_task_error_detail[i].percent_max) {
            return update_task_error_detail[i].descr;
        }
    }
    return "Unknown error";
}

typedef struct {
    UpdateExecutorTaskStageGroup group;
    uint8_t weight;
} UpdateTaskStageGroupMap;

#define STAGE_DEF(GROUP, WEIGHT) \
    {                            \
        .group = (GROUP),        \
        .weight = (WEIGHT),      \
    }

// Stage                 Measured time  Weight
// ReadManifest:         252 ms         0
// ValidateDFUImage:     5536 ms        15
// FlashWrite:           1669 ms        5
// FlashValidate:        1323 ms        4
// 917RadioWrite:        70893 ms       202
// 917RadioInstall:      18049 ms       52
// 917Write:             34993 ms       98
// 917Install:           9272 ms        26
// ResourcesFileCleanup: 1124 ms        3
// ResourcesDirCleanup:  1459 ms        4
// ResourcesFileUnpack:  6173 ms        18

static const UpdateTaskStageGroupMap update_task_stage_progress[] = {
    [UpdateExecutorTaskStageProgress] = STAGE_DEF(UpdateExecutorTaskStageGroupMisc, 0),

    [UpdateExecutorTaskStageReadManifest] = STAGE_DEF(UpdateExecutorTaskStageGroupPrepare, 0),

    [UpdateExecutorTaskStageValidateDFUImage] =
        STAGE_DEF(UpdateExecutorTaskStageGroupFirmware, 15),
    [UpdateExecutorTaskStageFlashWrite] = STAGE_DEF(UpdateExecutorTaskStageGroupFirmware, 5),
    [UpdateExecutorTaskStageFlashValidate] = STAGE_DEF(UpdateExecutorTaskStageGroupFirmware, 4),

    [UpdateExecutorTaskStage917RadioWrite] = STAGE_DEF(UpdateExecutorTaskStageGroup917Radio, 202),
    [UpdateExecutorTaskStage917RadioInstall] = STAGE_DEF(UpdateExecutorTaskStageGroup917Radio, 52),

    [UpdateExecutorTaskStage917Write] = STAGE_DEF(UpdateExecutorTaskStageGroup917, 98),
    [UpdateExecutorTaskStage917Install] = STAGE_DEF(UpdateExecutorTaskStageGroup917, 26),

    [UpdateExecutorTaskStageResourcesFileCleanup] =
        STAGE_DEF(UpdateExecutorTaskStageGroupResources, 3),
    [UpdateExecutorTaskStageResourcesDirCleanup] =
        STAGE_DEF(UpdateExecutorTaskStageGroupResources, 4),
    [UpdateExecutorTaskStageResourcesFileUnpack] =
        STAGE_DEF(UpdateExecutorTaskStageGroupResources, 18),

    [UpdateExecutorTaskStageCompleted] = STAGE_DEF(UpdateExecutorTaskStageGroupMisc, 1),
    [UpdateExecutorTaskStageError] = STAGE_DEF(UpdateExecutorTaskStageGroupMisc, 1),
};

static UpdateExecutorTaskStageGroup update_task_get_task_groups(UpdateExecutorTask* update_task) {
    UpdateExecutorTaskStageGroup ret = UpdateExecutorTaskStageGroupPrepare;
    const UpdateManifest* manifest = update_config_get_manifest(update_task->config);

    if(update_task->session_config.do_update_u5_firmware &&
       !furi_string_empty(updater_manifest_get_path(manifest, UpdateManifestPathDfu))) {
        ret |= UpdateExecutorTaskStageGroupFirmware;
    }

    if(update_task->session_config.do_update_917_firmware &&
       !furi_string_empty(updater_manifest_get_path(manifest, UpdateManifestPath917))) {
        ret |= UpdateExecutorTaskStageGroup917;
    }

    if(update_task->session_config.do_update_917_radio_stack &&
       !furi_string_empty(updater_manifest_get_path(manifest, UpdateManifestPath917Radio))) {
        ret |= UpdateExecutorTaskStageGroup917Radio;
    }

    if(update_task->session_config.do_update_resources &&
       !furi_string_empty(updater_manifest_get_path(manifest, UpdateManifestPathResources))) {
        ret |= UpdateExecutorTaskStageGroupResources;
    }

    return ret;
}

static void update_task_calc_completed_stages(UpdateExecutorTask* update_task) {
    uint32_t completed_stages_points = 0;
    for(UpdateExecutorTaskStage past_stage = UpdateExecutorTaskStageProgress;
        past_stage < update_task->state.stage;
        ++past_stage) {
        const UpdateTaskStageGroupMap* grp_descr = &update_task_stage_progress[past_stage];
        if((grp_descr->group & update_task->state.groups) == 0) {
            continue;
        }
        completed_stages_points += grp_descr->weight;
    }
    update_task->state.completed_stages_points = completed_stages_points;
}

void update_executor_task_set_progress(
    UpdateExecutorTask* update_task,
    UpdateExecutorTaskStage stage,
    uint8_t progress) {
    if(stage != UpdateExecutorTaskStageProgress) {
        FURI_LOG_I(TAG, "Stage %d, progress %d", stage, progress);
        /* do not override more specific error states */
        if((stage >= UpdateExecutorTaskStageError) &&
           (update_task->state.stage >= UpdateExecutorTaskStageError)) {
            return;
        }
        /* Build error message with code "[stage_idx-stage_percent]" */
        if(stage >= UpdateExecutorTaskStageError) {
            furi_string_printf(
                update_task->state.status,
                "%s\n#[%d-%d]",
                update_task_get_error_message(
                    update_task->state.stage, update_task->state.stage_progress),
                update_task->state.stage,
                update_task->state.stage_progress);
        } else {
            furi_string_set(update_task->state.status, update_task_stage_descr[stage]);
        }
        /* Store stage update */
        update_task->state.stage = stage;
        /* If we are still alive, sum completed stages weights */
        if((stage > UpdateExecutorTaskStageProgress) &&
           (stage < UpdateExecutorTaskStageCompleted)) {
            update_task_calc_completed_stages(update_task);
        }
    }

    /* Store stage progress for all non-error updates - to provide details on error state */
    if(!update_executor_task_stage_is_error(stage)) {
        update_task->state.stage_progress = progress;
    }

    /* Calculate "overall" progress, based on stage weights */
    uint32_t adapted_progress = 1;
    if(update_task->state.total_progress_points != 0) {
        if(stage < UpdateExecutorTaskStageCompleted) {
            adapted_progress = MIN(
                (update_task->state.completed_stages_points +
                 (update_task_stage_progress[update_task->state.stage].weight * progress / 100)) *
                    100 / (update_task->state.total_progress_points),
                100u);

        } else {
            adapted_progress = update_task->state.overall_progress;
        }
    }
    update_task->state.overall_progress = adapted_progress;

    if(update_task->status_change_callback) {
        update_task->status_change_callback(
            furi_string_get_cstr(update_task->state.status),
            update_task->state.stage,
            adapted_progress,
            update_task->status_change_callback_context);
    }
}

static void update_task_close_file(UpdateExecutorTask* update_task) {
    furi_assert(update_task);
    if(!storage_file_is_open(update_task->file)) {
        return;
    }

    storage_file_close(update_task->file);
}

static bool
    update_task_check_file_exists(UpdateExecutorTask* update_task, const FuriString* filename) {
    return storage_file_exists(update_task->storage, furi_string_get_cstr(filename));
}

bool update_executor_task_open_file(UpdateExecutorTask* update_task, const FuriString* filename) {
    update_task_close_file(update_task);

    FURI_LOG_I(TAG, "Opening file: %s", furi_string_get_cstr(filename));

    return storage_file_open(
        update_task->file, furi_string_get_cstr(filename), FSAM_READ, FSOM_OPEN_EXISTING);
}

static void
    update_task_worker_thread_cb(FuriThread* thread, FuriThreadState state, void* context) {
    UNUSED(context);

    if(state != FuriThreadStateStopped) {
        return;
    }

    if(furi_thread_get_return_code(thread) == UPDATE_EXECUTOR_TASK_NOERR) {
        updater_session_config_delete();

        furi_delay_ms(UPDATE_EXECUTOR_DELAY_OPERATION_OK);
        furi_hal_power_reset();
    }
}

UpdateExecutorTask* update_executor_task_alloc(void) {
    UpdateExecutorTask* update_task = malloc(sizeof(UpdateExecutorTask));

    update_task->state.stage = UpdateExecutorTaskStageProgress;
    update_task->state.stage_progress = 0;
    update_task->state.overall_progress = 0;
    update_task->state.status = furi_string_alloc();

    update_task->config = update_config_alloc();
    update_task->storage = furi_record_open(RECORD_STORAGE);
    update_task->file = storage_file_alloc(update_task->storage);
    update_task->status_change_callback = NULL;

    FuriThread* thread = update_task->thread =
        furi_thread_alloc_ex("UpdateWorker", 5120, NULL, update_task);

    furi_thread_set_state_callback(thread, update_task_worker_thread_cb);

    furi_thread_set_callback(thread, update_executor_task_worker_general);

    return update_task;
}

void update_executor_task_free(UpdateExecutorTask* update_task) {
    furi_assert(update_task);

    furi_thread_join(update_task->thread);

    furi_thread_free(update_task->thread);
    update_task_close_file(update_task);
    storage_file_free(update_task->file);
    update_config_free(update_task->config);

    furi_record_close(RECORD_STORAGE);

    free(update_task);
}

bool update_executor_task_parse_manifest(UpdateExecutorTask* update_task) {
    furi_assert(update_task);
    update_task->state.stage_progress = 0;
    update_task->state.overall_progress = 0;
    update_task->state.total_progress_points = 0;
    update_task->state.completed_stages_points = 0;
    update_task->state.groups = 0;

    update_executor_task_set_progress(update_task, UpdateExecutorTaskStageReadManifest, 0);
    bool result = false;
    FuriString* manifest_path = furi_string_alloc();

    do {
        update_executor_task_set_progress(update_task, UpdateExecutorTaskStageProgress, 13);
        CHECK_RESULT(furi_hal_version_do_i_belong_here());

        CHECK_RESULT(update_config_read_pointer_file(update_task->storage, manifest_path));
        // furi_string_set(update_task->update_path, manifest_path);

        updater_session_config_load(&update_task->session_config);

        update_executor_task_set_progress(update_task, UpdateExecutorTaskStageProgress, 20);
        UpdateConfigValidation res =
            update_config_load(update_task->config, furi_string_get_cstr(manifest_path));

        if(res != UpdateConfigValidationOK) {
            // TODO: message
            break;
        }

        const UpdateManifest* manifest = update_config_get_manifest(update_task->config);
        if(manifest == NULL) {
            break;
        }

        update_executor_task_set_progress(update_task, UpdateExecutorTaskStageProgress, 50);

        update_task->state.groups = update_task_get_task_groups(update_task);
        for(size_t stage_counter = 0; stage_counter < COUNT_OF(update_task_stage_progress);
            ++stage_counter) {
            const UpdateTaskStageGroupMap* grp_descr = &update_task_stage_progress[stage_counter];
            if((grp_descr->group & update_task->state.groups) != 0) {
                update_task->state.total_progress_points += grp_descr->weight;
            }
        }

        update_executor_task_set_progress(update_task, UpdateExecutorTaskStageProgress, 60);
        if((update_task->state.groups & UpdateExecutorTaskStageGroupFirmware) &&
           !update_task_check_file_exists(
               update_task, updater_manifest_get_path(manifest, UpdateManifestPathDfu))) {
            break;
        }

        update_executor_task_set_progress(update_task, UpdateExecutorTaskStageProgress, 70);
        if((update_task->state.groups & UpdateExecutorTaskStageGroup917) &&
           (!update_task_check_file_exists(
               update_task, updater_manifest_get_path(manifest, UpdateManifestPath917))
            // TODO:  || (manifest->radio_version.version.type == 0)
            )) {
            break;
        }

        update_executor_task_set_progress(update_task, UpdateExecutorTaskStageProgress, 80);
        if((update_task->state.groups & UpdateExecutorTaskStageGroup917Radio) &&
           (!update_task_check_file_exists(
               update_task, updater_manifest_get_path(manifest, UpdateManifestPath917))
            // TODO:  || (manifest->radio_version.version.type == 0)
            )) {
            break;
        }

        update_executor_task_set_progress(update_task, UpdateExecutorTaskStageProgress, 80);
        if((update_task->state.groups & UpdateExecutorTaskStageGroupResources) &&
           (!update_task_check_file_exists(
               update_task, updater_manifest_get_path(manifest, UpdateManifestPathResources)))) {
            break;
        }

        update_executor_task_set_progress(update_task, UpdateExecutorTaskStageProgress, 100);
        result = true;
    } while(false);

    furi_string_free(manifest_path);
    return result;
}

void update_executor_task_set_progress_callback(
    UpdateExecutorTask* update_task,
    UpdateExecutorTaskProgressCallback callback,
    void* context) {
    update_task->status_change_callback = callback;
    update_task->status_change_callback_context = context;
}

void update_executor_task_start(UpdateExecutorTask* update_task) {
    furi_assert(update_task);
    furi_assert(!update_executor_task_is_running(update_task));

    furi_thread_start(update_task->thread);
}

bool update_executor_task_is_running(UpdateExecutorTask* update_task) {
    furi_assert(update_task);
    furi_assert(update_task->thread);
    return furi_thread_get_state(update_task->thread) == FuriThreadStateRunning;
}

UpdateExecutorTaskState const* update_executor_task_get_state(UpdateExecutorTask* update_task) {
    furi_assert(update_task);
    return &update_task->state;
}

UpdateManifest const* update_executor_task_get_manifest(UpdateExecutorTask* update_task) {
    furi_assert(update_task);
    return update_config_get_manifest(update_task->config);
}
