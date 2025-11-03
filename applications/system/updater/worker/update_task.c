#include "update_task.h"
#include "update_task_i.h"

#include <furi.h>
#include <furi_hal.h>
#include <storage/storage.h>
#include <toolbox/path.h>

#define TAG "UpdWorker"

static const char* update_task_stage_descr[] = {
    [UpdateTaskStageProgress] = "...",
    [UpdateTaskStageReadManifest] = "Loading update manifest",
    [UpdateTaskStageValidateDFUImage] = "Checking DFU file",
    [UpdateTaskStageFlashWrite] = "Writing flash",
    [UpdateTaskStageFlashValidate] = "Validating flash",
    [UpdateTaskStage917Write] = "Writing 917 FW",
    [UpdateTaskStage917Install] = "Installing 917 FW",
    [UpdateTaskStage917RadioWrite] = "Writing 917 Radio FW",
    [UpdateTaskStage917RadioInstall] = "Installing 917 Radio FW",
    [UpdateTaskStageResourcesFileCleanup] = "Cleaning up files",
    [UpdateTaskStageResourcesDirCleanup] = "Cleaning up folders",
    [UpdateTaskStageResourcesFileUnpack] = "Extracting resources",
    [UpdateTaskStageCompleted] = "Restarting...",
    [UpdateTaskStageError] = "Error",
};

static const struct {
    UpdateTaskStage stage;
    uint8_t percent_min, percent_max;
    const char* descr;
} update_task_error_detail[] = {
    {
        .stage = UpdateTaskStageReadManifest,
        .percent_min = 0,
        .percent_max = 13,
        .descr = "Wrong Updater HW",
    },
    {
        .stage = UpdateTaskStageReadManifest,
        .percent_min = 14,
        .percent_max = 20,
        .descr = "Manifest pointer error",
    },
    {
        .stage = UpdateTaskStageReadManifest,
        .percent_min = 21,
        .percent_max = 30,
        .descr = "Manifest load error",
    },
    {
        .stage = UpdateTaskStageReadManifest,
        .percent_min = 31,
        .percent_max = 40,
        .descr = "Wrong package version",
    },
    {
        .stage = UpdateTaskStageReadManifest,
        .percent_min = 41,
        .percent_max = 50,
        .descr = "HW Target mismatch",
    },
    {
        .stage = UpdateTaskStageReadManifest,
        .percent_min = 51,
        .percent_max = 60,
        .descr = "No DFU file",
    },
    {
        .stage = UpdateTaskStageReadManifest,
        .percent_min = 61,
        .percent_max = 80,
        .descr = "No Radio file",
    },
    {
        .stage = UpdateTaskStage917Write,
        .percent_min = 0,
        .percent_max = 100,
        .descr = "917 FW write: error",
    },
    {
        .stage = UpdateTaskStage917Install,
        .percent_min = 0,
        .percent_max = 10,
        .descr = "917 FW write: error",
    },
    {
        .stage = UpdateTaskStage917RadioWrite,
        .percent_min = 11,
        .percent_max = 100,
        .descr = "Stack install: wait failed",
    },
    {
        .stage = UpdateTaskStage917RadioInstall,
        .percent_min = 0,
        .percent_max = 10,
        .descr = "Failed to start C2",
    },
    {
        .stage = UpdateTaskStageValidateDFUImage,
        .percent_min = 0,
        .percent_max = 1,
        .descr = "Failed to open DFU file",
    },
    {
        .stage = UpdateTaskStageValidateDFUImage,
        .percent_min = 1,
        .percent_max = 97,
        .descr = "DFU file read error",
    },
    {
        .stage = UpdateTaskStageValidateDFUImage,
        .percent_min = 98,
        .percent_max = 100,
        .descr = "DFU file CRC mismatch",
    },
    {
        .stage = UpdateTaskStageFlashWrite,
        .percent_min = 0,
        .percent_max = 100,
        .descr = "Flash write error",
    },
    {
        .stage = UpdateTaskStageFlashValidate,
        .percent_min = 0,
        .percent_max = 100,
        .descr = "Flash compare error",
    },
    {
        .stage = UpdateTaskStageResourcesFileCleanup,
        .percent_min = 0,
        .percent_max = 100,
        .descr = "SD I/O error",
    },
    {
        .stage = UpdateTaskStageResourcesDirCleanup,
        .percent_min = 0,
        .percent_max = 100,
        .descr = "SD I/O error",
    },
    {
        .stage = UpdateTaskStageResourcesFileUnpack,
        .percent_min = 0,
        .percent_max = 100,
        .descr = "SD I/O error",
    },
};

static const char* update_task_get_error_message(UpdateTaskStage stage, uint8_t percent) {
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
    UpdateTaskStageGroup group;
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
    [UpdateTaskStageProgress] = STAGE_DEF(UpdateTaskStageGroupMisc, 0),

    [UpdateTaskStageReadManifest] = STAGE_DEF(UpdateTaskStageGroupPrepare, 0),

    [UpdateTaskStageValidateDFUImage] = STAGE_DEF(UpdateTaskStageGroupFirmware, 15),
    [UpdateTaskStageFlashWrite] = STAGE_DEF(UpdateTaskStageGroupFirmware, 5),
    [UpdateTaskStageFlashValidate] = STAGE_DEF(UpdateTaskStageGroupFirmware, 4),

    [UpdateTaskStage917RadioWrite] = STAGE_DEF(UpdateTaskStageGroup917Radio, 202),
    [UpdateTaskStage917RadioInstall] = STAGE_DEF(UpdateTaskStageGroup917Radio, 52),

    [UpdateTaskStage917Write] = STAGE_DEF(UpdateTaskStageGroup917, 98),
    [UpdateTaskStage917Install] = STAGE_DEF(UpdateTaskStageGroup917, 26),

    [UpdateTaskStageResourcesFileCleanup] = STAGE_DEF(UpdateTaskStageGroupResources, 3),
    [UpdateTaskStageResourcesDirCleanup] = STAGE_DEF(UpdateTaskStageGroupResources, 4),
    [UpdateTaskStageResourcesFileUnpack] = STAGE_DEF(UpdateTaskStageGroupResources, 18),

    [UpdateTaskStageCompleted] = STAGE_DEF(UpdateTaskStageGroupMisc, 1),
    [UpdateTaskStageError] = STAGE_DEF(UpdateTaskStageGroupMisc, 1),
};

static UpdateTaskStageGroup update_task_get_task_groups(UpdateTask* update_task) {
    UpdateTaskStageGroup ret = UpdateTaskStageGroupPrepare;
    const UpdateManifest* manifest = update_config_get_manifest(update_task->config);

    if(update_task->session_config.do_update_u5_firmware &&
       !furi_string_empty(updater_manifest_get_path(manifest, UpdateManifestPathDfu))) {
        ret |= UpdateTaskStageGroupFirmware;
    }

    if(update_task->session_config.do_update_917_firmware &&
       !furi_string_empty(updater_manifest_get_path(manifest, UpdateManifestPath917))) {
        ret |= UpdateTaskStageGroup917;
    }

    if(update_task->session_config.do_update_917_radio_stack &&
       !furi_string_empty(updater_manifest_get_path(manifest, UpdateManifestPath917Radio))) {
        ret |= UpdateTaskStageGroup917Radio;
    }

    if(update_task->session_config.do_update_resources &&
       !furi_string_empty(updater_manifest_get_path(manifest, UpdateManifestPathResources))) {
        ret |= UpdateTaskStageGroupResources;
    }

    return ret;
}

static void update_task_calc_completed_stages(UpdateTask* update_task) {
    uint32_t completed_stages_points = 0;
    for(UpdateTaskStage past_stage = UpdateTaskStageProgress;
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

void update_task_set_progress(UpdateTask* update_task, UpdateTaskStage stage, uint8_t progress) {
    if(stage != UpdateTaskStageProgress) {
        FURI_LOG_I(TAG, "Stage %d, progress %d", stage, progress);
        /* do not override more specific error states */
        if((stage >= UpdateTaskStageError) && (update_task->state.stage >= UpdateTaskStageError)) {
            return;
        }
        /* Build error message with code "[stage_idx-stage_percent]" */
        if(stage >= UpdateTaskStageError) {
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
        if((stage > UpdateTaskStageProgress) && (stage < UpdateTaskStageCompleted)) {
            update_task_calc_completed_stages(update_task);
        }
    }

    /* Store stage progress for all non-error updates - to provide details on error state */
    if(!update_stage_is_error(stage)) {
        update_task->state.stage_progress = progress;
    }

    /* Calculate "overall" progress, based on stage weights */
    uint32_t adapted_progress = 1;
    if(update_task->state.total_progress_points != 0) {
        if(stage < UpdateTaskStageCompleted) {
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

static void update_task_close_file(UpdateTask* update_task) {
    furi_assert(update_task);
    if(!storage_file_is_open(update_task->file)) {
        return;
    }

    storage_file_close(update_task->file);
}

static bool update_task_check_file_exists(UpdateTask* update_task, const FuriString* filename) {
    return storage_file_exists(update_task->storage, furi_string_get_cstr(filename));
}

bool update_task_open_file(UpdateTask* update_task, const FuriString* filename) {
    update_task_close_file(update_task);

    FURI_LOG_I(TAG, "Opening file: %s", furi_string_get_cstr(filename));

    return storage_file_open(
        update_task->file, furi_string_get_cstr(filename), FSAM_READ, FSOM_OPEN_EXISTING);
}

static void
    update_task_worker_thread_cb(FuriThread* thread, FuriThreadState state, void* context) {
    UpdateTask* update_task = context;

    if(state != FuriThreadStateStopped) {
        return;
    }

    if(furi_thread_get_return_code(thread) == UPDATE_TASK_NOERR) {
        updater_session_config_delete(furi_string_get_cstr(update_task->update_dir_path));

        furi_delay_ms(UPDATE_DELAY_OPERATION_OK);
        furi_hal_power_reset();
    }
}

UpdateTask* update_task_alloc(void) {
    UpdateTask* update_task = malloc(sizeof(UpdateTask));

    update_task->state.stage = UpdateTaskStageProgress;
    update_task->state.stage_progress = 0;
    update_task->state.overall_progress = 0;
    update_task->state.status = furi_string_alloc();

    update_task->config = update_config_alloc();
    update_task->storage = furi_record_open(RECORD_STORAGE);
    update_task->file = storage_file_alloc(update_task->storage);
    update_task->status_change_callback = NULL;
    update_task->update_dir_path = furi_string_alloc();

    FuriThread* thread = update_task->thread =
        furi_thread_alloc_ex("UpdateWorker", 5120, NULL, update_task);

    furi_thread_set_state_context(thread, update_task);
    furi_thread_set_state_callback(thread, update_task_worker_thread_cb);

    furi_thread_set_callback(thread, update_task_worker_general);

    return update_task;
}

void update_task_free(UpdateTask* update_task) {
    furi_assert(update_task);

    furi_thread_join(update_task->thread);

    furi_thread_free(update_task->thread);
    update_task_close_file(update_task);
    storage_file_free(update_task->file);
    update_config_free(update_task->config);
    furi_string_free(update_task->update_dir_path);

    furi_record_close(RECORD_STORAGE);

    free(update_task);
}

bool update_task_parse_manifest(UpdateTask* update_task) {
    furi_assert(update_task);
    update_task->state.stage_progress = 0;
    update_task->state.overall_progress = 0;
    update_task->state.total_progress_points = 0;
    update_task->state.completed_stages_points = 0;
    update_task->state.groups = 0;

    update_task_set_progress(update_task, UpdateTaskStageReadManifest, 0);
    bool result = false;
    FuriString* manifest_path = furi_string_alloc();

    do {
        update_task_set_progress(update_task, UpdateTaskStageProgress, 13);
        CHECK_RESULT(furi_hal_version_do_i_belong_here());

        CHECK_RESULT(update_config_read_pointer_file(update_task->storage, manifest_path));
        // furi_string_set(update_task->update_path, manifest_path);

        path_extract_dirname(furi_string_get_cstr(manifest_path), update_task->update_dir_path);
        updater_session_config_load(
            furi_string_get_cstr(update_task->update_dir_path), &update_task->session_config);

        update_task_set_progress(update_task, UpdateTaskStageProgress, 20);
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

        update_task_set_progress(update_task, UpdateTaskStageProgress, 50);

        update_task->state.groups = update_task_get_task_groups(update_task);
        for(size_t stage_counter = 0; stage_counter < COUNT_OF(update_task_stage_progress);
            ++stage_counter) {
            const UpdateTaskStageGroupMap* grp_descr = &update_task_stage_progress[stage_counter];
            if((grp_descr->group & update_task->state.groups) != 0) {
                update_task->state.total_progress_points += grp_descr->weight;
            }
        }

        update_task_set_progress(update_task, UpdateTaskStageProgress, 60);
        if((update_task->state.groups & UpdateTaskStageGroupFirmware) &&
           !update_task_check_file_exists(
               update_task, updater_manifest_get_path(manifest, UpdateManifestPathDfu))) {
            break;
        }

        update_task_set_progress(update_task, UpdateTaskStageProgress, 70);
        if((update_task->state.groups & UpdateTaskStageGroup917) &&
           (!update_task_check_file_exists(
               update_task, updater_manifest_get_path(manifest, UpdateManifestPath917))
            // TODO:  || (manifest->radio_version.version.type == 0)
            )) {
            break;
        }

        update_task_set_progress(update_task, UpdateTaskStageProgress, 80);
        if((update_task->state.groups & UpdateTaskStageGroup917Radio) &&
           (!update_task_check_file_exists(
               update_task, updater_manifest_get_path(manifest, UpdateManifestPath917))
            // TODO:  || (manifest->radio_version.version.type == 0)
            )) {
            break;
        }

        update_task_set_progress(update_task, UpdateTaskStageProgress, 80);
        if((update_task->state.groups & UpdateTaskStageGroupResources) &&
           (!update_task_check_file_exists(
               update_task, updater_manifest_get_path(manifest, UpdateManifestPathResources)))) {
            break;
        }

        update_task_set_progress(update_task, UpdateTaskStageProgress, 100);
        result = true;
    } while(false);

    furi_string_free(manifest_path);
    return result;
}

void update_task_set_progress_callback(
    UpdateTask* update_task,
    UpdateProgressCallback callback,
    void* context) {
    update_task->status_change_callback = callback;
    update_task->status_change_callback_context = context;
}

void update_task_start(UpdateTask* update_task) {
    furi_assert(update_task);
    furi_assert(!update_task_is_running(update_task));

    furi_thread_start(update_task->thread);
}

bool update_task_is_running(UpdateTask* update_task) {
    furi_assert(update_task);
    furi_assert(update_task->thread);
    return furi_thread_get_state(update_task->thread) == FuriThreadStateRunning;
}

UpdateTaskState const* update_task_get_state(UpdateTask* update_task) {
    furi_assert(update_task);
    return &update_task->state;
}

UpdateManifest const* update_task_get_manifest(UpdateTask* update_task) {
    furi_assert(update_task);
    return update_config_get_manifest(update_task->config);
}
