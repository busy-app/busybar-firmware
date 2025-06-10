#include "update_task_i.h"

#include <toolbox/update_lib/dfu_file.h>
#include <toolbox/update_lib/resources/manifest.h>
#include <toolbox/tar/tar_archive.h>
#include <toolbox/path.h>

#define TAG "UpdateTask"

#define STM_DFU_VENDOR_ID   0x0483
#define STM_DFU_PRODUCT_ID  0xDF11
#define BSB_DFU_DEVICE_CODE 0xFFFF

static const DfuValidationParams bsb_dfu_params = {
    .device = BSB_DFU_DEVICE_CODE,
    .product = STM_DFU_PRODUCT_ID,
    .vendor = STM_DFU_VENDOR_ID,
};

static bool update_task_flash_program_page(
    const uint8_t i_page,
    const uint8_t* update_block,
    uint16_t update_block_len) {
    FURI_LOG_D(
        TAG,
        "Programming flash page %u at address 0x%08X with update block of length %u",
        i_page,
        (unsigned int)(furi_hal_flash_get_base() + furi_hal_flash_get_page_size() * i_page),
        update_block_len);
    furi_hal_flash_program_page(i_page, update_block, update_block_len);
    return true;
}

static bool pdate_task_compare_flash(
    const uint8_t i_page,
    const uint8_t* update_block,
    uint16_t update_block_len) {
    const size_t page_addr = furi_hal_flash_get_base() + furi_hal_flash_get_page_size() * i_page;
    FURI_LOG_D(
        TAG,
        "Comparing flash page %u at address 0x%08X with update block of length %u",
        i_page,
        (unsigned int)page_addr,
        update_block_len);
    return memcmp(update_block, (void*)page_addr, update_block_len) == 0;
}

static bool check_address_boundaries(const size_t address) {
    const size_t min_allowed_address = furi_hal_flash_get_base();
    const size_t max_allowed_address = (size_t)furi_hal_flash_get_free_end_address();
    FURI_LOG_D(
        TAG,
        "Checking address 0x%08X against boundaries: 0x%08X - 0x%08X",
        (unsigned int)address,
        (unsigned int)min_allowed_address,
        (unsigned int)max_allowed_address);
    return (address >= min_allowed_address) && (address < max_allowed_address);
}

static void update_task_file_progress(const uint8_t progress, void* context) {
    UpdateTask* update_task = context;
    update_task_set_progress(update_task, UpdateTaskStageProgress, progress);
}

static bool update_task_write_dfu(UpdateTask* update_task) {
    DfuUpdateTask page_task = {
        .address_cb = &check_address_boundaries,
        .progress_cb = &update_task_file_progress,
        .task_cb = &update_task_flash_program_page,
        .context = update_task,
    };

    bool success = false;
    do {
        update_task_set_progress(update_task, UpdateTaskStageValidateDFUImage, 0);
        CHECK_RESULT(update_task_open_file(
            update_task,
            updater_manifest_get_path(
                update_config_get_manifest(update_task->config), UpdateManifestPathDfu)));

        File* dfu_file = update_task->file;
        CHECK_RESULT(dfu_file_validate_crc(dfu_file, update_task_file_progress, update_task));

        uint8_t n_targets = dfu_file_validate_headers(dfu_file, &bsb_dfu_params);
        FURI_LOG_I(TAG, "DFU targets: %u", n_targets);
        if(n_targets == 0) {
            break;
        }

        update_task_set_progress(update_task, UpdateTaskStageFlashWrite, 0);
        page_task.task_cb = &update_task_flash_program_page;
        CHECK_RESULT(dfu_file_process_targets(&page_task, dfu_file, n_targets));

        update_task_set_progress(update_task, UpdateTaskStageFlashValidate, 0);
        page_task.task_cb = &pdate_task_compare_flash;
        CHECK_RESULT(dfu_file_process_targets(&page_task, dfu_file, n_targets));

        success = true;
    } while(false);

    return success;
}

typedef struct {
    UpdateTask* update_task;
    TarArchive* archive;
} TarUnpackProgress;

static bool update_task_resource_unpack_cb(const char* name, bool is_directory, void* context) {
    UNUSED(name);
    UNUSED(is_directory);
    TarUnpackProgress* unpack_progress = context;
    int32_t progress = 0, total = 0;
    tar_archive_get_read_progress(unpack_progress->archive, &progress, &total);
    update_task_set_progress(
        unpack_progress->update_task, UpdateTaskStageProgress, (progress * 100) / (total + 1));
    return true;
}

static void update_task_cleanup_resources(UpdateTask* update_task) {
    ResourceManifestReader* manifest_reader = resource_manifest_reader_alloc(update_task->storage);
    do {
        FURI_LOG_D(TAG, "Cleaning up old manifest");
        if(!resource_manifest_reader_open(manifest_reader, EXT_PATH("Manifest"))) {
            FURI_LOG_W(TAG, "No existing manifest");
            break;
        }

        ResourceManifestEntry* entry_ptr = NULL;
        /* Iterate over manifest and calculate entries count */
        uint32_t n_file_entries = 1, n_dir_entries = 1;
        while((entry_ptr = resource_manifest_reader_next(manifest_reader))) {
            if(entry_ptr->type == ResourceManifestEntryTypeFile) {
                n_file_entries++;
            } else if(entry_ptr->type == ResourceManifestEntryTypeDirectory) {
                n_dir_entries++;
            }
        }
        resource_manifest_rewind(manifest_reader);

        update_task_set_progress(update_task, UpdateTaskStageResourcesFileCleanup, 0);
        uint32_t n_processed_file_entries = 0;
        while((entry_ptr = resource_manifest_reader_next(manifest_reader))) {
            if(entry_ptr->type == ResourceManifestEntryTypeFile) {
                update_task_set_progress(
                    update_task,
                    UpdateTaskStageProgress,
                    (n_processed_file_entries++ * 100) / n_file_entries);

                FuriString* file_path = furi_string_alloc();
                path_concat(
                    STORAGE_EXT_PATH_PREFIX, furi_string_get_cstr(entry_ptr->name), file_path);
                FURI_LOG_D(TAG, "Removing %s", furi_string_get_cstr(file_path));

                FS_Error result =
                    storage_common_remove(update_task->storage, furi_string_get_cstr(file_path));
                if(result != FSE_OK && result != FSE_EXIST) {
                    FURI_LOG_E(
                        TAG,
                        "%s remove failed, cause %s",
                        furi_string_get_cstr(file_path),
                        storage_error_get_desc(result));
                }
                furi_string_free(file_path);
            }
        }

        update_task_set_progress(update_task, UpdateTaskStageResourcesDirCleanup, 0);
        uint32_t n_processed_dir_entries = 0;
        while((entry_ptr = resource_manifest_reader_previous(manifest_reader))) {
            if(entry_ptr->type == ResourceManifestEntryTypeDirectory) {
                update_task_set_progress(
                    update_task,
                    UpdateTaskStageProgress,
                    (n_processed_dir_entries++ * 100) / n_dir_entries);

                FuriString* folder_path = furi_string_alloc();

                do {
                    path_concat(
                        STORAGE_EXT_PATH_PREFIX,
                        furi_string_get_cstr(entry_ptr->name),
                        folder_path);

                    FURI_LOG_D(TAG, "Removing folder %s", furi_string_get_cstr(folder_path));
                    FS_Error result = storage_common_remove(
                        update_task->storage, furi_string_get_cstr(folder_path));
                    if(result != FSE_OK && result != FSE_EXIST) {
                        FURI_LOG_E(
                            TAG,
                            "%s remove failed, cause %s",
                            furi_string_get_cstr(folder_path),
                            storage_error_get_desc(result));
                    }
                } while(false);

                furi_string_free(folder_path);
            }
        }
    } while(false);
    resource_manifest_reader_free(manifest_reader);
}

static bool update_task_handle_resources(UpdateTask* update_task) {
    const FuriString* resources_path = updater_manifest_get_path(
        update_config_get_manifest(update_task->config), UpdateManifestPathResources);

    TarArchive* archive = tar_archive_alloc(update_task->storage);
    bool success = false;
    do {
        if(update_task->state.groups & UpdateTaskStageGroupResources) {
            TarUnpackProgress progress = {
                .update_task = update_task,
                .archive = archive,
            };

            CHECK_RESULT(
                tar_archive_open(archive, furi_string_get_cstr(resources_path), TarOpenModeRead));

            update_task_cleanup_resources(update_task);

            update_task_set_progress(update_task, UpdateTaskStageResourcesFileUnpack, 0);
            tar_archive_set_file_callback(archive, update_task_resource_unpack_cb, &progress);
            CHECK_RESULT(tar_archive_unpack_to(archive, STORAGE_EXT_PATH_PREFIX, NULL));
        }
        success = true;

    } while(false);
    tar_archive_free(archive);

    return success;
}

int32_t update_task_worker_general(void* context) {
    furi_assert(context);
    UpdateTask* update_task = context;
    bool success = false;

    do {
        update_task_set_progress(update_task, UpdateTaskStageReadManifest, 0);
        CHECK_RESULT(update_task_parse_manifest(update_task));

        if(update_task->state.groups & UpdateTaskStageGroupFirmware) {
            CHECK_RESULT(update_task_write_dfu(update_task));
        }

        if(update_task->state.groups & UpdateTaskStageGroupResources) {
            CHECK_RESULT(update_task_handle_resources(update_task));
        }

        update_task_set_progress(update_task, UpdateTaskStageCompleted, 100);
        success = true;
    } while(false);

    if(!success) {
        update_task_set_progress(update_task, UpdateTaskStageError, 0);
        return UPDATE_TASK_FAILED;
    }

    return UPDATE_TASK_NOERR;
}
